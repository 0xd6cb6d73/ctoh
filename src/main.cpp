#include "clang-c/Index.h"
#include <algorithm>
#include <format>
#include <fstream>
#include <memory>

import ctoh;

struct ParseData {
  CXTranslationUnit in_tu;
  std::unique_ptr<std::fstream> fstr;
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
  auto out = std::make_unique<std::fstream>(header_name, std::fstream::out | std::fstream::trunc);
  ParseData parse_data{.in_tu = tu, .fstr = std::move(out), .known = {}, .source = file_name};

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
        // TODO: fix header names
        clang_getPresumedLocation(loc, &fname, nullptr, nullptr);
        std::string source_loc(clang_getCString(fname));
        const auto last = source_loc.find_last_of('\\') + 1;
        if (!std::ranges::equal(parse_data->source, source_loc)) {
          if (parse_data->add(std::string_view(source_loc.begin() + last, source_loc.end()))) {
            // parse_data->fstr->write(LIT_POUND.data(), LIT_POUND.size());
            // parse_data->fstr->write(LIT_INCLUDE.data(), LIT_INCLUDE.size());
            // parse_data->fstr->write(LIT_SPACE.data(), LIT_SPACE.size());
            // parse_data->fstr->write(LIT_ANGLE_OPEN.data(), LIT_ANGLE_OPEN.size());
            // parse_data->fstr->write(parse_data->known.back().data(),
            //                         parse_data->known.back().size());
            // parse_data->fstr->write(LIT_ANGLE_CLOSE.data(), LIT_ANGLE_CLOSE.size());
            // parse_data->fstr->write(LIT_LF.data(), LIT_LF.size());
          }
        }
        clang_disposeString(fname);
        if (clang_Location_isFromMainFile(loc) == 0) {
          return CXChildVisit_Recurse;
        }

        if (cursor_kind == CXCursor_FunctionDecl) {
          parse_data->fstr =
              std::move(ctoh::dump_function(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_StructDecl) {
          parse_data->fstr =
              std::move(ctoh::dump_struct(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_TypedefDecl) {
          parse_data->fstr =
              std::move(ctoh::dump_typedef(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_EnumDecl) {
          parse_data->fstr =
              std::move(ctoh::dump_enum(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_UnionDecl) {
          parse_data->fstr =
              std::move(ctoh::dump_union(current_cursor, std::move(parse_data->fstr)));
        }
        return CXChildVisit_Recurse;
      },
      &parse_data);
}