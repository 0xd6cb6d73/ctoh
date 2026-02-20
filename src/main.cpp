import ctoh;
import std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Missing filename");
  }
  std::string file_name = std::string(argv[1]);

  ctoh::run(std::move(file_name));
}