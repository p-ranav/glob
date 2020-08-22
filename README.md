<p align="center">
  <img height="90" src="img/logo.png"/>  
</p>

<p align="center">
  Unix-style pathname pattern expansion
</p>

* This library has 2 files: `glob.hpp` and `glob.cpp`
  - No external dependencies - just the standard library
  - Requires C++17 `std::filesystem`
  - If you can't use `C++17`, you can integrate [gulrak/filesystem](https://github.com/gulrak/filesystem) with minimal effort.
* MIT License

## API

* No tilde expansion is done but `*`, `?`, and character ranges expressed with `[]` will be correctly matched.
* For a literal match, wrap the meta-characters in brackets. For example, `[?]` matches the character `?`.
* With `rglob`, the pattern `**` will match any files and zero or more directories, subdirectories and symbolic links to directories.

```cpp
/// e.g., glob("*.hpp")
/// e.g., glob("**/*.cpp")
/// e.g., glob("test_files_02/[0-9].txt")
/// e.g., glob("/usr/local/include/nc*.h")
/// e.g., glob("test_files_02/?.txt")
vector<filesystem::path> glob(string pathname);

/// Globs recursively
/// e.g., rglob("Documents/Projects/Foo/**/*.hpp")
/// e.g., rglob("test_files_02/*[0-9].txt")
vector<filesystem::path> rglob(string pathname);
```

There are also two convenience functions to `glob` on a list of patterns:

```cpp
/// e.g., glob({"*.png", "*.jpg"})
vector<filesystem::path> glob(vector<string> pathnames);

/// Globs recursively
/// e.g., rglob({"*.png", "*.jpg", "*.tiff"})
vector<filesystem::path> rglob(vector<string> pathnames);
```

## Wildcards

| Wildcard | Matches | Example
|--- |--- |--- |
| `*` | any characters | `*.txt` matches all files with the txt extension |
| `?` | any one character | `???` matches files with 3 characters long |
| `[]` | any character listed in the brackets | `[ABC]*` matches files starting with A,B or C | 
| `[-]` | any character in the range listed in brackets | `[A-Z]*` matches files starting with capital letters |
| `[!]` | any character listed in the brackets | `[!ABC]*` matches files that do not start with A,B or C |
