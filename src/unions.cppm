module;
#include <clang-c/Index.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

export module ctoh:unions;
import :shared;

namespace ctoh {

export {
  std::unique_ptr<std::fstream> dump_union(CXCursor cursor, std::unique_ptr<std::fstream> fstream) {
    WithFields data{};
    CXString struct_name = clang_getCursorSpelling(cursor);
    data.name = std::string(clang_getCString(struct_name));
    clang_disposeString(struct_name);

    clang_visitChildren(
        cursor,
        [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
          auto *out = reinterpret_cast<WithFields *>(client_data);
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

    fstream->write(LIT_UNION.data(), LIT_UNION.size());
    fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
    fstream->write(data.name.data(), data.name.size());
    fstream->write(LIT_BRACE_OPEN.data(), LIT_BRACE_OPEN.size());
    fstream->write(LIT_LF.data(), LIT_LF.size());
    for (const auto &type : data.types) {
      std::string type_spelling = handle_named_type(type.type, type.name);
      fstream->write(type_spelling.data(), type_spelling.size());
      fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
      fstream->write(LIT_LF.data(), LIT_LF.size());
    }
    fstream->write(LIT_BRACE_CLOSE.data(), LIT_BRACE_CLOSE.size());
    fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
    fstream->write(LIT_LF.data(), LIT_LF.size());
    return std::move(fstream);
  }
}
} // namespace ctoh