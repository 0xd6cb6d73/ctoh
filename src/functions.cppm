module;
#include <clang-c/Index.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

export module ctoh:functions;
import :shared;

namespace ctoh {
struct Function {
  std::string name;
  CXType ret;
  std::vector<AstElement> args;
};

export std::unique_ptr<std::fstream> dump_function(CXCursor cursor,
                                                   std::unique_ptr<std::fstream> fstream) {
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

  CXString ret_spelling = clang_getTypeSpelling(data.ret);
  std::string ret_str(clang_getCString(ret_spelling));
  fstream->write(ret_str.data(), ret_str.size());
  clang_disposeString(ret_spelling);
  fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
  fstream->write(data.name.data(), data.name.size());
  fstream->write(LIT_PAREN_OPEN.data(), LIT_PAREN_OPEN.size());
  for (const auto &type : data.args) {
    CXString spelling = clang_getTypeSpelling(type.type);
    std::string spelling_str(clang_getCString(spelling));
    fstream->write(spelling_str.data(), spelling_str.size());
    fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
    fstream->write(type.name.data(), type.name.size());
    fstream->write(LIT_COMMA.data(), LIT_COMMA.size());
    clang_disposeString(spelling);
  }
  if (data.args.size() > 0) {
    fstream->seekp(-1, std::ios_base::seekdir::end);
  }
  fstream->write(LIT_PAREN_CLOSE.data(), LIT_PAREN_CLOSE.size());
  fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
  return std::move(fstream);
}
} // namespace ctoh