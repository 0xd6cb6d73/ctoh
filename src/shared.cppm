
module;
#include <clang-c/Index.h>
#include <string>
#include <vector>
export module ctoh:shared;

export {
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

  struct AstElement {
    std::string name;
    CXType type;
  };

  struct WithFields {
    std::string name;
    std::vector<AstElement> types;
  };
}
