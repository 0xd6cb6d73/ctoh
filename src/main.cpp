#include <cstdint>

import ctoh;
import std;

int32_t main(int32_t argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Missing filename");
  }
  std::string file_name = std::string(argv[1]);

  ctoh::run(file_name);
}