<p align="center">
  <img height="90" src="img/logo.png"/>  
</p>

<p align="center">
  Unix-style pathname pattern expansion
</p>

## Table of Contents

- [Quick Start](#quick-start)
- [API](#api)
- [Wildcards](#wildcards)
- [Example](#example)
  * [Match file extensions](#match-file-extensions)
  * [Match files in absolute pathnames](#match-files-in-absolute-pathnames)
  * [Wildcards: Match a range of characters listed in brackets ('[]')](#wildcards-match-a-range-of-characters-listed-in-brackets-)
  * [Exclude files from the matching](#exclude-files-from-the-matching)
  * [Wildcards: Match any one character with question mark ('?')](#wildcards-match-any-one-character-with-question-mark-)
  * [Case sensitivity](#case-sensitivity)
- [Contributing](#contributing)
- [License)(#license)

## Quick Start

* This library has 2 files: `glob.h` and `glob.cpp`
  - No external dependencies - just the standard library
  - Requires C++17 `std::filesystem`
  - If you can't use `C++17`, you can integrate [gulrak/filesystem](https://github.com/gulrak/filesystem) with minimal effort.
* MIT License

```bash
git clone https://github.com/p-ranav/glob
cd glob
mkdir build && cd build
cmake .. && make
```

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

## Example

Below is a short program that runs `glob` and prints matching path names.

***NOTE*** Replace `glob` with `rglob` if you want to glob recursively.

```cpp
#include <glob/glob.h>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./exe <pattern>\n";
    return EXIT_FAILURE;
  }

  // Run glob on the vector of patterns
  for (auto &f : glob::glob(argv[1])) {
    std::cout << f << "\n";
  }
}
```

### Match file extensions

```console
foo@bar:~$ tree
.
├── include
│   └── foo
│       ├── bar.hpp
│       ├── baz.hpp
│       └── foo.hpp
└── test
    ├── bar.cpp
    ├── doctest.hpp
    ├── foo.cpp
    └── main.cpp

3 directories, 7 files

foo@bar:~$ ./main "**/*.hpp"
"test/doctest.hpp"

foo@bar:~$ ./main "**/**/*.hpp"
"include/foo/baz.hpp"
"include/foo/foo.hpp"
"include/foo/bar.hpp"
```

***NOTE*** If you use `rglob` instead of `glob`:

```console
foo@bar:~$ ./main "**/*.hpp"
"test/doctest.hpp"
"include/foo/baz.hpp"
"include/foo/foo.hpp"
"include/foo/bar.hpp"
```

### Match files in absolute pathnames

```console
foo@bar:~$ ./main '/usr/local/include/nc*.h'
"/usr/local/include/ncCheck.h"
"/usr/local/include/ncGroupAtt.h"
"/usr/local/include/ncUshort.h"
"/usr/local/include/ncByte.h"
"/usr/local/include/ncString.h"
"/usr/local/include/ncUint64.h"
"/usr/local/include/ncGroup.h"
"/usr/local/include/ncUbyte.h"
"/usr/local/include/ncvalues.h"
"/usr/local/include/ncInt.h"
"/usr/local/include/ncAtt.h"
"/usr/local/include/ncVar.h"
"/usr/local/include/ncUint.h"
```

### Wildcards: Match a range of characters listed in brackets ('[]')

```console
foo@bar:~$ ls test_files_02
1.txt 2.txt 3.txt 4.txt

foo@bar:~$ ./main 'test_files_02/[0-9].txt'
"test_files_02/4.txt"
"test_files_02/3.txt"
"test_files_02/2.txt"
"test_files_02/1.txt"

foo@bar:~$ ./main 'test_files_02/[1-2]*'
"test_files_02/2.txt"
"test_files_02/1.txt"
```

```console
foo@bar:~$ ls test_files_03
file1.txt file2.txt file3.txt file4.txt

foo@bar:~$ ./main 'test_files_03/file[0-9].*'
"test_files_03/file2.txt"
"test_files_03/file3.txt"
"test_files_03/file1.txt"
"test_files_03/file4.txt"
```

### Exclude files from the matching

```console
foo@bar:~$ ls test_files_01
__init__.py     bar.py      foo.py

foo@bar:~$ ./main 'test_files_01/*[!__init__].py'
"test_files_01/bar.py"
"test_files_01/foo.py"

foo@bar:~$ ./main 'test_files_01/*[!__init__][!bar].py'
"test_files_01/foo.py"

foo@bar:~$ ./main 'test_files_01/[!_]*.py'
"test_files_01/bar.py"
"test_files_01/foo.py"
```

### Wildcards: Match any one character with question mark ('?')

```console
foo@bar:~$ ls test_files_02
1.txt 2.txt 3.txt 4.txt

foo@bar:~$ ./main 'test_files_02/?.txt'
"test_files_02/4.txt"
"test_files_02/3.txt"
"test_files_02/2.txt"
"test_files_02/1.txt"
```

```console
foo@bar:~$ ls test_files_03
file1.txt file2.txt file3.txt file4.txt

foo@bar:~$ ./main 'test_files_03/????[3-4].txt'
"test_files_03/file3.txt"
"test_files_03/file4.txt"
```

### Case sensitivity

`glob` matching is case-sensitive:

```console
foo@bar:~$ ls test_files_05
file1.png file2.png file3.PNG file4.PNG

foo@bar:~$ ./main 'test_files_05/*.png'
"test_files_05/file2.png"
"test_files_05/file1.png"

foo@bar:~$ ./main 'test_files_05/*.PNG'
"test_files_05/file3.PNG"
"test_files_05/file4.PNG"
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
