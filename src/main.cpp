#include <algorithm>
#include <clang-c/CXErrorCode.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

const std::string LIT_STRUCT("struct");
const std::string LIT_BRACE_OPEN("{");
const std::string LIT_BRACE_CLOSE("}");
const std::string LIT_PAREN_OPEN("(");
const std::string LIT_PAREN_CLOSE(")");
const std::string LIT_SEMI_COLON(";");
const std::string LIT_COMMA(",");
const std::string LIT_LF("\n");
const std::string LIT_ANGLE_OPEN("<");
const std::string LIT_ANGLE_CLOSE(">");
const std::string LIT_POUND("#");
const std::string LIT_INCLUDE("include");
const std::string LIT_SPACE(" ");
const std::string LIT_QUOTE("\"");
const std::string LIT_EQUAL("=");
const std::string LIT_USING("using");
const std::string LIT_TYPEDEF("typedef");

struct ParseData {
  CXTranslationUnit in_tu;
  std::fstream *fstr;
  std::vector<std::string> known;
  std::string source;
  bool add(const std::string_view str) {
    if (const auto res = std::ranges::find(this->known, str); res == this->known.end()) {
      this->known.emplace_back(std::string(str));
      return true;
    }
    return false;
  }
};

struct HeaderType {
  std::string name;
  CXType type;
};

struct HeaderStruct {
  std::string name;
  std::vector<HeaderType> types;
};

struct FunctionArg {
  std::string name;
  CXType type;
};

struct Function {
  std::string name;
  CXType ret;
  std::vector<FunctionArg> args;
};

namespace ctoh {
HeaderStruct dump_struct(CXCursor cursor) {
  HeaderStruct data{};
  CXString struct_name = clang_getCursorSpelling(cursor);
  data.name = std::string(clang_getCString(struct_name));
  clang_disposeString(struct_name);

  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        HeaderStruct *out = reinterpret_cast<HeaderStruct *>(client_data);
        CXCursorKind kind = clang_getCursorKind(current_cursor);
        if (kind == CXCursor_FieldDecl) {
          CXString field_name = clang_getCursorSpelling(current_cursor);
          CXType field_type = clang_getCursorType(current_cursor);
          CXString type_name = clang_getTypeSpelling(field_type);
          out->types.push_back(
              {.name = std::string(clang_getCString(field_name)), .type = field_type});
          clang_disposeString(type_name);
          clang_disposeString(field_name);
        }
        return CXChildVisit_Recurse;
      },
      &data);
  return data;
}

Function dump_function(CXCursor cursor) {
  CXString fn_name = clang_getCursorSpelling(cursor);

  CXCursor args_cursor = clang_Cursor_getArgument(cursor, 0);
  Function data{};
  data.ret = clang_getCursorResultType(cursor);
  data.name = std::string(clang_getCString(fn_name));
  clang_disposeString(fn_name);
  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        Function *out = reinterpret_cast<Function *>(client_data);
        CXCursorKind kind = clang_getCursorKind(current_cursor);
        if (kind == CXCursor_ParmDecl) {
          CXString param_name = clang_getCursorSpelling(current_cursor);
          CXType param_type = clang_getCursorType(current_cursor);
          CXString type_name = clang_getTypeSpelling(param_type);
          out->args.push_back(
              {.name = std::string(clang_getCString(param_name)), .type = param_type});
          clang_disposeString(type_name);
          clang_disposeString(param_name);
        }
        return CXChildVisit_Recurse;
      },
      &data);
  return data;
  std::cout << "Function: " << clang_getCString(fn_name);
  clang_disposeString(fn_name);
  CXType cursor_type = clang_getCursorType(cursor);
  CXString type_spelling = clang_getTypeSpelling(cursor_type);
  std::cout << ", namely " << clang_getCString(type_spelling);
  clang_disposeString(type_spelling);
}

size_t is_fn_ptr(const std::string_view spelling) {
  const auto marker = spelling.find("(*");
  if (marker == std::string::npos) {
    return 0;
  }
  return marker + 2;
}

void dump_typedef(CXCursor cursor) {}
} // namespace ctoh

int32_t main(int32_t argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Missing filename");
  }
  std::string file_name = std::string(argv[1]);

  CXIndex index = clang_createIndex(0, 1);
  CXTranslationUnit tu{};
  CXErrorCode err = clang_parseTranslationUnit2(index, file_name.data(), nullptr, 0, nullptr, 0,
                                                CXTranslationUnit_SkipFunctionBodies, &tu);
  if (err != 0) {
    throw std::runtime_error(std::format("TU parse failure: {}", int32_t(err)));
  }

  CXFile my_file = clang_getFile(tu, file_name.data());
  CXCursor cursor = clang_getTranslationUnitCursor(tu);

  std::string header_name = std::format("{}.h", file_name);
  std::fstream out(header_name, out.out | out.trunc);
  ParseData parse_data{.in_tu = tu, .fstr = &out, .known = {}, .source = file_name};

  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        CXType cursor_type = clang_getCursorType(current_cursor);
        CXCursorKind cursor_kind = clang_getCursorKind(current_cursor);
        ParseData *parse_data = reinterpret_cast<ParseData *>(client_data);

        CXSourceLocation loc = clang_getCursorLocation(current_cursor);
        // CXFile curr_file{};
        // clang_getSpellingLocation(loc, &curr_file, nullptr, nullptr, nullptr);
        // CXString fname = clang_getFileName(curr_file);
        CXString fname{};
        clang_getPresumedLocation(loc, &fname, nullptr, nullptr);
        std::string source_loc(clang_getCString(fname));
        if (!std::ranges::equal(parse_data->source, source_loc)) {
          if (parse_data->add(source_loc)) {
            parse_data->fstr->write(LIT_POUND.data(), LIT_POUND.size());
            parse_data->fstr->write(LIT_INCLUDE.data(), LIT_INCLUDE.size());
            parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
            parse_data->fstr->write(LIT_ANGLE_OPEN.data(), LIT_ANGLE_OPEN.size());
            parse_data->fstr->write(parse_data->known.back().data(),
                                    parse_data->known.back().size());
            parse_data->fstr->write(LIT_ANGLE_CLOSE.data(), LIT_ANGLE_CLOSE.size());
            parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
          }
        }
        clang_disposeString(fname);
        if (clang_Location_isFromMainFile(loc) == 0) {
          return CXChildVisit_Recurse;
        }

        if (cursor_kind == CXCursor_FunctionDecl) {
          auto data = ctoh::dump_function(current_cursor);
          CXString ret_spelling = clang_getTypeSpelling(data.ret);
          std::string ret_str(clang_getCString(ret_spelling));
          parse_data->fstr->write(ret_str.data(), ret_str.size());
          clang_disposeString(ret_spelling);
          parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
          parse_data->fstr->write(data.name.data(), data.name.size());
          parse_data->fstr->write(LIT_PAREN_OPEN.data(), LIT_PAREN_OPEN.size());
          for (const auto &type : data.args) {
            CXString spelling = clang_getTypeSpelling(type.type);
            std::string spelling_str(clang_getCString(spelling));
            parse_data->fstr->write(spelling_str.data(), spelling_str.size());
            parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
            parse_data->fstr->write(type.name.data(), type.name.size());
            parse_data->fstr->write(LIT_COMMA.data(), LIT_COMMA.size());
            clang_disposeString(spelling);
          }
          parse_data->fstr->seekp(-1, std::ios_base::seekdir::end);
          parse_data->fstr->write(LIT_PAREN_CLOSE.data(), LIT_PAREN_CLOSE.size());
          parse_data->fstr->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
        } else if (cursor_kind == CXCursor_StructDecl) {
          HeaderStruct data = ctoh::dump_struct(current_cursor);
          parse_data->fstr->write(LIT_STRUCT.data(), LIT_STRUCT.size());
          parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
          parse_data->fstr->write(data.name.data(), data.name.size());
          parse_data->fstr->write(LIT_BRACE_OPEN.data(), LIT_BRACE_OPEN.size());
          parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
          for (const auto &type : data.types) {
            CXString spelling = clang_getTypeSpelling(type.type);
            std::string spelling_str(clang_getCString(spelling));
            if (const auto idx = ctoh::is_fn_ptr(spelling_str)) {
              spelling_str.insert(idx, type.name);
            } else {
              spelling_str.append(" ").append(type.name);
            }
            parse_data->fstr->write(spelling_str.data(), spelling_str.size());
            parse_data->fstr->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
            parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
            clang_disposeString(spelling);
          }
          parse_data->fstr->write(LIT_BRACE_CLOSE.data(), LIT_BRACE_CLOSE.size());
          parse_data->fstr->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
          parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
        } else if (cursor_kind == CXCursor_TypedefDecl) {
          CXString spelling = clang_getCursorSpelling(current_cursor);
          std::string spelling_str(clang_getCString(spelling));
          std::cout << spelling_str << '\n';
          CXType param_type = clang_getTypedefDeclUnderlyingType(current_cursor);
          CXString type_name = clang_getTypeSpelling(param_type);
          std::string type_str(clang_getCString(type_name));
          parse_data->fstr->write(LIT_TYPEDEF.data(), LIT_TYPEDEF.size());
          parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
          parse_data->fstr->write(type_str.data(), type_str.size());
          parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
          parse_data->fstr->write(spelling_str.data(), spelling_str.size());
          parse_data->fstr->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
          parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
          std::cout << type_str << '\n';
          clang_disposeString(type_name);
          clang_disposeString(spelling);
        }
        return CXChildVisit_Recurse;
      },
      &parse_data);
}