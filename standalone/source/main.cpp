#include <glob/glob.h>
#include <glob/version.h>

#include <cxxopts.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  cxxopts::Options options(argv[0], "Run glob to find all the pathnames matching a specified pattern");

  bool recursive;
  std::vector<std::string> patterns;

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("r,recursive", "Run glob recursively", cxxopts::value<bool>(recursive)->default_value("false"))
    ("i,input", "Patterns to match", cxxopts::value<std::vector<std::string>>(patterns))
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  if (result["help"].as<bool>()) {
    std::cout << options.help() << std::endl;
    return 0;
  } else if (result["version"].as<bool>()) {
    std::cout << "glob, version " << GLOB_VERSION << std::endl;
    return 0;
  }

  if (patterns.empty()) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if (recursive) {
    for (auto& match: glob::rglob(patterns)) {
     std::cout << match << "\n";
    } 
  } else {
    for (auto& match: glob::glob(patterns)) {
      std::cout << match << "\n";
    }
  }

  return 0;
}
