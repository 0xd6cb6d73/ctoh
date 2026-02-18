
module;
#include <clang-c/Index.h>
#include <fstream>
#include <string>
#include <vector>

export module ctoh:structs;
import :shared;

namespace ctoh {
struct HeaderType {
  std::string name;
  CXType type;
};

struct HeaderStruct {
  std::string name;
  std::vector<HeaderType> types;
};

size_t is_fn_ptr(const std::string_view spelling) {
  const auto marker = spelling.find("(*");
  if (marker == std::string::npos) {
    return 0;
  }
  return marker + 2;
}

export std::unique_ptr<std::fstream> dump_struct(CXCursor cursor,
                                                 std::unique_ptr<std::fstream> fstream) {
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

  fstream->write(LIT_STRUCT.data(), LIT_STRUCT.size());
  fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
  fstream->write(data.name.data(), data.name.size());
  fstream->write(LIT_BRACE_OPEN.data(), LIT_BRACE_OPEN.size());
  fstream->write(LIT_LF.data(), LIT_LF.size());
  for (const auto &type : data.types) {
    CXString spelling = clang_getTypeSpelling(type.type);
    std::string spelling_str(clang_getCString(spelling));
    if (const auto idx = ctoh::is_fn_ptr(spelling_str)) {
      spelling_str.insert(idx, type.name);
    } else {
      spelling_str.append(LIT_SPACE).append(type.name);
    }
    fstream->write(spelling_str.data(), spelling_str.size());
    fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
    fstream->write(LIT_LF.data(), LIT_LF.size());
    clang_disposeString(spelling);
  }
  fstream->write(LIT_BRACE_CLOSE.data(), LIT_BRACE_CLOSE.size());
  fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
  fstream->write(LIT_LF.data(), LIT_LF.size());
  return std::move(fstream);
}
} // namespace ctoh