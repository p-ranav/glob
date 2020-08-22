<p align="center">
  <img height="90" src="img/logo.png"/>  
</p>

<p align="center">
  Unix-style pathname pattern expansion
</p>

## Quick Start

* This library has 2 files: `glob.hpp` and `glob.cpp`
  - No external dependencies - just the standard library
  - Requires C++17 `std::filesystem`
  - If you can't use `C++17`, you can integrate [gulrak/filesystem](https://github.com/gulrak/filesystem) with minimal effort.
* MIT License

```cpp
// Performs `glob` and prints all the matching results
#include <glob/glob.hpp>
#include <iostream>
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./exe <pattern>\n";
    return EXIT_FAILURE;
  }

  for (auto &f : glob::glob(argv[1])) {
    std::cout << fs::relative(f) << "\n";
  }
}
```

## API

```cpp
// `glob` finds all the pathnames matching a specified pattern according to the rules used
// by the Unix shell. No tilde expansion is done, but *, ?, and character ranges expressed
// with [] will be correctly matched. For a literal match, wrap the meta-characters in
// brackets. For example, '[?]' matches the character '?'.

// Return a possibly-empty list of path names that match pathname, which must be a string
// containing a path specification. pathname can be either absolute (like
// /usr/src/Python-1.5/Makefile) or relative (like ../../Tools/*/*.gif), and can contain
// shell-style wildcards. Broken symlinks are included in the results (as in the shell).
std::vector<std::filesystem::path> glob(const std::string &pathname);

// Similar to glob.
// The pattern “**” will match any files and zero or more directories, subdirectories and
// symbolic links to directories.
std::vector<std::filesystem::path> rglob(const std::string &pathname);
```
