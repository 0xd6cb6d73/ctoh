
module;
#include <clang-c/Index.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

export module ctoh:enums;
import :shared;

namespace ctoh {
struct EnumConstant {
  std::string name;
  unsigned long long value;
};

struct Enum {
  std::string name;
  std::vector<EnumConstant> constants;
};

export {
  std::unique_ptr<std::fstream> dump_enum(CXCursor cursor, std::unique_ptr<std::fstream> fstream) {
    Enum data{};
    CXString struct_name = clang_getCursorSpelling(cursor);
    data.name = std::string(clang_getCString(struct_name));
    clang_disposeString(struct_name);

    clang_visitChildren(
        cursor,
        [](CXCursor current_cursor, CXCursor parent, CXClientData client_data) {
          auto *out = reinterpret_cast<Enum *>(client_data);
          CXCursorKind kind = clang_getCursorKind(current_cursor);
          if (kind == CXCursor_EnumConstantDecl) {
            CXString field_name = clang_getCursorSpelling(current_cursor);
            out->constants.push_back(
                {.name = std::string(clang_getCString(field_name)),
                 .value = clang_getEnumConstantDeclUnsignedValue(current_cursor)});
            clang_disposeString(field_name);
          }
          return CXChildVisit_Recurse;
        },
        &data);

    fstream->write(LIT_ENUM.data(), LIT_ENUM.size());
    fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
    fstream->write(data.name.data(), data.name.size());
    fstream->write(LIT_BRACE_OPEN.data(), LIT_BRACE_OPEN.size());
    fstream->write(LIT_LF.data(), LIT_LF.size());
    for (const auto &type : data.constants) {
      fstream->write(type.name.data(), type.name.size());
      fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
      fstream->write(LIT_EQUAL.data(), LIT_EQUAL.size());
      fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
      std::string val_str = std::to_string(type.value);
      fstream->write(val_str.data(), val_str.size());
      fstream->write(LIT_COMMA.data(), LIT_COMMA.size());
      fstream->write(LIT_LF.data(), LIT_LF.size());
    }
    fstream->write(LIT_BRACE_CLOSE.data(), LIT_BRACE_CLOSE.size());
    fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
    fstream->write(LIT_LF.data(), LIT_LF.size());
    return std::move(fstream);
  }
}

} // namespace ctoh