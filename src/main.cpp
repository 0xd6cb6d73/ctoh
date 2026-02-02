#include <clang-c/CXErrorCode.h>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <cstdint>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

int32_t main(int32_t argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Missing filename");
  }
  std::string file_name = std::string(argv[1]);

  CXIndex index = clang_createIndex(0, 1);
  CXTranslationUnit tu{};
  CXErrorCode err = clang_parseTranslationUnit2(index, file_name.data(),
                                                nullptr, 0, nullptr, 0, 0, &tu);
  if (err != 0) {
    throw std::runtime_error(std::format("TU parse failure: {}", int32_t(err)));
  }

  CXFile my_file = clang_getFile(tu, file_name.data());
  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        CXType cursor_type = clang_getCursorType(current_cursor);
        CXFile my_file = reinterpret_cast<CXFile *>(client_data);
        CXFile file;

        CXSourceLocation loc = clang_getCursorLocation(current_cursor);
        clang_getExpansionLocation(loc, &file, nullptr, nullptr, nullptr);
        if (file != my_file) {
          return CXChildVisit_Recurse;
        }

        CXString type_kind_spelling =
            clang_getTypeKindSpelling(cursor_type.kind);
        std::cout << "TypeKind: " << clang_getCString(type_kind_spelling);
        clang_disposeString(type_kind_spelling);

        if (cursor_type.kind == CXType_Pointer || // If cursor_type is a pointer
            cursor_type.kind ==
                CXType_LValueReference || // or an LValue Reference (&)
            cursor_type.kind ==
                CXType_RValueReference) { // or an RValue Reference (&&),
          CXType pointed_to_type =
              clang_getPointeeType(cursor_type); // retrieve the pointed-to type

          CXString pointed_to_type_spelling =
              clang_getTypeSpelling(pointed_to_type); // Spell out the entire
          std::cout << "pointing to type: "
                    << clang_getCString(
                           pointed_to_type_spelling); // pointed-to type
          clang_disposeString(pointed_to_type_spelling);
        } else if (cursor_type.kind == CXType_Record) {
          CXString type_spelling = clang_getTypeSpelling(cursor_type);
          std::cout << ", namely " << clang_getCString(type_spelling);
          clang_disposeString(type_spelling);
        }
        std::cout << "\n";
        return CXChildVisit_Recurse;
      },
      my_file);

  clang_visitChildren(
      cursor,
      [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
        CXType cursor_type = clang_getCursorType(current_cursor);
        CXString cursor_spelling = clang_getCursorSpelling(current_cursor);
        CXSourceRange cursor_range = clang_getCursorExtent(current_cursor);
        CXFile file;
        CXFile my_file = reinterpret_cast<CXFile *>(client_data);
        unsigned start_line, start_column, start_offset;
        unsigned end_line, end_column, end_offset;

        clang_getExpansionLocation(clang_getRangeStart(cursor_range), &file,
                                   &start_line, &start_column, &start_offset);
        CXString cx_file_name = clang_getFileName(file);
        auto f_name = std::string(clang_getCString(cx_file_name));
        if (file != my_file) {
          return CXChildVisit_Recurse;
        }

        std::cout << "Cursor " << clang_getCString(cursor_spelling);
        clang_getExpansionLocation(clang_getRangeEnd(cursor_range), &file,
                                   &end_line, &end_column, &end_offset);
        std::cout << " spanning lines " << start_line << " to " << end_line;
        clang_disposeString(cursor_spelling);

        std::cout << " in file " << f_name;
        clang_disposeString(cx_file_name);

        std::cout << "\n";
        return CXChildVisit_Recurse;
      },
      my_file);
}