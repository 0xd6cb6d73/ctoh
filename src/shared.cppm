module;
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <string>
#include <vector>

export module ctoh:shared;

export namespace ctoh {
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
const std::string LIT_ENUM("enum");
const std::string LIT_UNION("union");
const std::string LIT_BRACKET_OPEN("[");
const std::string LIT_BRACKET_CLOSE("]");

struct AstElement {
  std::string name;
  CXType type;
};

struct WithFields {
  std::string name;
  std::vector<AstElement> types;
};

// makes sure to handle array cases
std::string handle_named_type(CXType type, std::string name) {
  CXString spelling = clang_getTypeSpelling(type);
  std::string spelling_str(clang_getCString(spelling));
  clang_disposeString(spelling);
  auto marker = spelling_str.find("(*");
  if (const auto idx = marker; idx != std::string::npos) {
    return spelling_str.insert(idx + 2, name);
  }
  marker = spelling_str.find(LIT_BRACKET_OPEN);
  if (marker == std::string::npos) {
    return spelling_str.append(LIT_SPACE).append(name);
  }
  return spelling_str.insert(marker, LIT_SPACE).insert(marker + 1, name);
}
} // namespace ctoh
