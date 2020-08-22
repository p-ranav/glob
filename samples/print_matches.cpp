#include <glob/glob.hpp>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./exe <pattern>\n";
    return EXIT_FAILURE;
  }

  for (auto& f : glob::glob(argv[1])) {
    std::cout << glob::fs::relative(f) << "\n";
  }
}