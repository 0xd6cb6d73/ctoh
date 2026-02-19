
module;
#include <clang-c/Index.h>

export module ctoh:typedefs;
import :shared;
import std;

namespace ctoh {

export std::unique_ptr<std::fstream> dump_typedef(CXCursor cursor,
                                                  std::unique_ptr<std::fstream> fstream) {
  CXString spelling = clang_getCursorSpelling(cursor);
  std::string spelling_str(clang_getCString(spelling));
  CXType param_type = clang_getTypedefDeclUnderlyingType(cursor);
  CXString type_name = clang_getTypeSpelling(param_type);
  std::string type_str(clang_getCString(type_name));
  fstream->write(LIT_TYPEDEF.data(), LIT_TYPEDEF.size());
  fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
  fstream->write(type_str.data(), type_str.size());
  fstream->write(LIT_SPACE.data(), LIT_SPACE.size());
  fstream->write(spelling_str.data(), spelling_str.size());
  fstream->write(LIT_SEMI_COLON.data(), LIT_SEMI_COLON.size());
  fstream->write(LIT_LF.data(), LIT_LF.size());
  clang_disposeString(type_name);
  clang_disposeString(spelling);
  return std::move(fstream);
}
} // namespace ctoh