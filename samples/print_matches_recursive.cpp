#include <glob/glob.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./exe <pattern>\n";
    return EXIT_FAILURE;
  }

  for (auto &f : glob::rglob(argv[1])) {
    std::cout << fs::relative(f) << "\n";
  }
}