#include <glob/glob.hpp>
#include <iostream>
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: ./exe <pattern>\n";
    return EXIT_FAILURE;
  }

  std::vector<std::string> patterns;
  for (int i = 1; i < argc; i++) {
    patterns.push_back(argv[i]);
  }

  for (auto &f : glob::glob(patterns)) {
    std::cout << f << "\n";
  }
}