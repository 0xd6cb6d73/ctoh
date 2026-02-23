module;
#include <clang-c/Index.h>
#include <cstdint>

export module ctoh:run;
import :enums;
import :functions;
import :structs;
import :unions;
import :typedefs;
import std;

namespace ctoh {
struct ParseData {
  CXTranslationUnit in_tu;
  std::unique_ptr<std::fstream> fstr;
  std::vector<std::string> known;
  bool add(const std::string_view str) {
    if (const auto res = std::ranges::find(this->known, str); res == this->known.end()) {
      this->known.emplace_back(std::string(str));
      return true;
    }
    return false;
  }
};

export void run(std::string file_name) {
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
  ParseData parse_data{
      .in_tu = tu,
      .fstr = std::move(out),
      .known = {},
  };

  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        CXType cursor_type = clang_getCursorType(current_cursor);
        CXCursorKind cursor_kind = clang_getCursorKind(current_cursor);
        ParseData *parse_data = reinterpret_cast<ParseData *>(client_data);

        CXSourceLocation loc = clang_getCursorLocation(current_cursor);
        bool in_main = clang_Location_isFromMainFile(loc);
        if (!in_main) {
          return CXChildVisit_Recurse;
        }

        if (cursor_kind == CXCursor_FunctionDecl) {
          parse_data->fstr = std::move(dump_function(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_StructDecl) {
          parse_data->fstr = std::move(dump_struct(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_TypedefDecl) {
          parse_data->fstr = std::move(dump_typedef(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_EnumDecl) {
          parse_data->fstr = std::move(dump_enum(current_cursor, std::move(parse_data->fstr)));
        } else if (cursor_kind == CXCursor_UnionDecl) {
          parse_data->fstr = std::move(dump_union(current_cursor, std::move(parse_data->fstr)));
        }
        return CXChildVisit_Recurse;
      },
      &parse_data);
}
} // namespace ctoh