
module;
#include <clang-c/Index.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

export module ctoh:enums;
import :shared;

namespace ctoh {
export {
  std::unique_ptr<std::fstream> dump_enum(CXCursor cursor, std::unique_ptr<std::fstream> fstream) {}
}

} // namespace ctoh