
#include <cassert>
#include <functional>
#include <glob/glob.hpp>
#include <iostream>
#include <map>
#include <regex>

namespace glob {

namespace details {

bool string_replace(std::string &str, const std::string &from, const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::string translate(const std::string &pattern) {
  std::size_t i = 0, n = pattern.size();
  std::string result_string;

  while (i < n) {
    auto c = pattern[i];
    i += 1;
    if (c == '*') {
      result_string += ".*";
    } else if (c == '?') {
      result_string += ".";
    } else if (c == '[') {
      auto j = i;
      if (j < n && pattern[j] == '!') {
        j += 1;
      }
      if (j < n && pattern[j] == ']') {
        j += 1;
      }
      while (j < n && pattern[j] != ']') {
        j += 1;
      }
      if (j >= n) {
        result_string += "\\[";
      } else {
        auto stuff = std::string(pattern.begin() + i, pattern.begin() + j);
        if (stuff.find("--") == std::string::npos) {
          string_replace(stuff, std::string{"\\"}, std::string{R"(\\)"});
        } else {
          std::vector<std::string> chunks;
          std::size_t k = 0;
          if (pattern[i] == '!') {
            k = i + 2;
          } else {
            k = i + 1;
          }

          while (true) {
            k = pattern.find("-", k, j);
            if (k == std::string::npos) {
              break;
            }
            chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + k));
            i = k + 1;
            k = k + 3;
          }

          chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + j));
          // Escape backslashes and hyphens for set difference (--).
          // Hyphens that create ranges shouldn't be escaped.
          bool first = false;
          for (auto &s : chunks) {
            string_replace(s, std::string{"\\"}, std::string{R"(\\)"});
            string_replace(s, std::string{"-"}, std::string{R"(\-)"});
            if (first) {
              stuff += s;
              first = false;
            } else {
              stuff += "-" + s;
            }
          }
        }

        // Escape set operations (&&, ~~ and ||).
        std::string result;
        std::regex_replace(std::back_inserter(result),          // ressult
                           stuff.begin(), stuff.end(),          // string
                           std::regex(std::string{R"([&~|])"}), // pattern
                           std::string{R"(\\\1)"});             // repl
        stuff = result;

        i = j + 1;
        if (stuff[0] == '!') {
          stuff = "^" + std::string(stuff.begin() + 1, stuff.end());
        } else if (stuff[0] == '^' || stuff[0] == '[') {
          stuff = "\\\\" + stuff;
        }
        result_string = result_string + "[" + stuff + "]";
      }
    } else {
      // SPECIAL_CHARS
      // closing ')', '}' and ']'
      // '-' (a range in character set)
      // '&', '~', (extended character set operations)
      // '#' (comment) and WHITESPACE (ignored) in verbose mode
      static std::string special_characters = "()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
      static std::map<int, std::string> special_characters_map;
      if (special_characters_map.empty()) {
        for (auto &c : special_characters) {
          special_characters_map.insert(
              std::make_pair(static_cast<int>(c), std::string{"\\"} + std::string(1, c)));
        }
      }

      if (special_characters.find(c) != std::string::npos) {
        result_string += special_characters_map[static_cast<int>(c)];
      } else {
        result_string += c;
      }
    }
  }
  return std::string{"(("} + result_string + std::string{R"()|[\r\n])$)"};
}

std::regex compile_pattern(const std::string &pattern) {
  return std::regex(translate(pattern), std::regex::ECMAScript);
}

std::vector<std::filesystem::path> filter(const std::vector<std::filesystem::path> &names,
                                          const std::string &pattern) {
  std::vector<std::filesystem::path> result;
  auto regex = details::compile_pattern(pattern);
  for (auto &name : names) {
    if (std::regex_match(name.string(), regex)) {
      result.push_back(name);
    }
  }
  return result;
}

bool fnmatch_case(const std::filesystem::path &name, const std::string &pattern) {
  return std::regex_match(name.string(), details::compile_pattern(pattern));
}

bool fnmatch(const std::filesystem::path &name, const std::string &pattern) {
  auto name_normal = name.lexically_normal();
  auto pattern_normal = std::filesystem::path(pattern).lexically_normal();
  return fnmatch_case(name_normal, pattern_normal.string());
}

bool has_magic(const std::string &pathname) {
  static const auto magic_check = std::regex("([*?[])");
  return std::regex_search(pathname, magic_check);
}

bool is_hidden(const std::string &pathname) { return pathname[0] == '.'; }

bool is_recursive(const std::string &pattern) { return pattern == "**"; }

std::vector<std::filesystem::path> iter_directory(const std::filesystem::path &dirname,
                                                  bool dironly) {
  std::vector<std::filesystem::path> result;

  auto current_directory = dirname;
  if (current_directory.empty()) {
    current_directory = std::filesystem::current_path();
  }

  for (auto &entry : std::filesystem::directory_iterator(current_directory)) {
    if (!dironly || entry.is_directory()) {
      result.push_back(entry.path());
    }
  }

  return result;
}

// Escape all special characters.
std::string escape(const std::string &pathname) {
  // Escaping is done by wrapping any of "*?[" between square brackets.
  // Meta-characters do not work in the drive part and shouldn't be escaped.

  // drive, pathname = os.path.splitdrive(pathname)
  auto path = std::filesystem::path(pathname);
  auto drive = path.root_name();
  auto relative_path = path.relative_path();

  static const auto magic_check = std::regex("([*?[])");
  // magic_check.sub(r'[\1]', pathname)
  relative_path = std::regex_replace(relative_path.string(), magic_check, R"([\1])");
  return drive / relative_path;
}

// Recursively yields relative pathnames inside a literal directory.
std::vector<std::filesystem::path> rlistdir(const std::filesystem::path &dirname,
                                            bool dironly) {
  std::vector<std::filesystem::path> result;
  auto names = iter_directory(dirname, dironly);
  for (auto &x : names) {
    if (!is_hidden(x.string())) {
      result.push_back(x);
      for (auto &y : rlistdir(x, dironly)) {
        result.push_back(y);
      }
    }
  }
  return result;
}

std::vector<std::filesystem::path> glob2(const std::filesystem::path &dirname,
                                         const std::string &pattern, bool dironly) {
  std::vector<std::filesystem::path> result;
  assert(is_recursive(pattern));
  for (auto &dir : rlistdir(dirname, dironly)) {
    result.push_back(dir);
  }
  return result;
}

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<std::filesystem::path> glob1(const std::filesystem::path &dirname,
                                         const std::string &pattern, bool dironly) {
  auto names = iter_directory(dirname, dironly);
  std::vector<std::filesystem::path> filtered_names;
  if (!is_hidden(pattern)) {
    for (auto &n : names) {
      if (!is_hidden(n.string())) {
        filtered_names.push_back(std::filesystem::relative(n));
      }
    }
  }
  return filter(filtered_names, pattern);
}

std::vector<std::filesystem::path> glob0(const std::filesystem::path &dirname,
                                         const std::filesystem::path &basename,
                                         bool dironly) {
  std::vector<std::filesystem::path> result;
  if (basename.empty()) {
    // 'q*x/' should match only directories.
    if (std::filesystem::is_directory(dirname)) {
      result = {basename};
    }
  } else {
    if (std::filesystem::exists(dirname / basename)) {
      result = {basename};
    }
  }
  return result;
}

std::vector<std::filesystem::path> glob(const std::string &pathname,
                                        bool recursive = false, bool dironly = false) {
  std::vector<std::filesystem::path> result;

  const auto path = std::filesystem::path(pathname);
  auto dirname = path.parent_path();
  const auto basename = path.filename();

  // if (dirname.empty()) {
  //   dirname = std::filesystem::current_path();
  // }

  if (!has_magic(pathname)) {
    assert(!dironly);
    if (!basename.empty()) {
      if (std::filesystem::exists(path)) {
        result.push_back(path);
      }
    } else {
      // Patterns ending with a slash should match only directories
      if (std::filesystem::is_directory(dirname)) {
        result.push_back(path);
      }
    }
    return result;
  }

  if (dirname.empty()) {
    if (recursive && is_recursive(basename.string())) {
      return glob2(dirname, basename, dironly);
    } else {
      return glob1(dirname, basename, dironly);
    }
  }

  std::vector<std::filesystem::path> dirs;
  if (dirname != std::filesystem::path(pathname) && has_magic(dirname.string())) {
    dirs = details::glob(dirname, recursive, true);
  } else {
    dirs = {dirname};
  }

  std::function<std::vector<std::filesystem::path>(const std::filesystem::path &,
                                                   const std::filesystem::path &, bool)>
      glob_in_dir;
  if (has_magic(basename.string())) {
    if (recursive && is_recursive(basename.string())) {
      glob_in_dir = details::glob2;
    } else {
      glob_in_dir = details::glob1;
    }
  } else {
    glob_in_dir = details::glob0;
  }

  for (auto &d : dirs) {
    for (auto &name : glob_in_dir(d, basename, dironly)) {
      result.push_back(std::filesystem::absolute(name));
    }
  }

  return result;
}

} // namespace details

std::vector<std::filesystem::path> glob(const std::string &pathname) {
  return details::glob(pathname, false);
}

std::vector<std::filesystem::path> rglob(const std::string &pathname) {
  return details::glob(pathname, true);
}

} // namespace glob