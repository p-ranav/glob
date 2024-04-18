#include <glob/glob.h>

#include <cassert>

#include <algorithm>
#include <map>
#include <regex>
#include <string_view>

namespace glob {

namespace {

static constexpr auto SPECIAL_CHARACTERS = std::string_view{"()[]{}?*+-|^$\\.&~# \t\n\r\v\f"};
static const auto ESCAPE_SET_OPER = std::regex(std::string{R"([&~|])"});
static const auto ESCAPE_REPL_STR = std::string{R"(\\\1)"};

bool string_replace(std::string &str, std::string_view from, std::string_view to) {
  std::size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::string translate(std::string_view pattern) {
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
          string_replace(stuff, std::string_view{"\\"}, std::string_view{R"(\\)"});
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
            if (k == std::string_view::npos) {
              break;
            }
            chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + k));
            i = k + 1;
            k = k + 3;
          }

          chunks.push_back(std::string(pattern.begin() + i, pattern.begin() + j));
          // Escape backslashes and hyphens for set difference (--).
          // Hyphens that create ranges shouldn't be escaped.
          bool first = true;
          for (auto &chunk : chunks) {
            string_replace(chunk, std::string_view{"\\"}, std::string_view{R"(\\)"});
            string_replace(chunk, std::string_view{"-"}, std::string_view{R"(\-)"});
            if (first) {
              stuff += chunk;
              first = false;
            } else {
              stuff += "-" + chunk;
            }
          }
        }

        // Escape set operations (&&, ~~ and ||).
        std::string result{};
        std::regex_replace(std::back_inserter(result), // result
                           stuff.begin(), stuff.end(), // string
                           ESCAPE_SET_OPER,            // pattern
                           ESCAPE_REPL_STR);           // repl
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
      static std::map<int, std::string> special_characters_map;
      if (special_characters_map.empty()) {
        for (auto &&sc : SPECIAL_CHARACTERS) {
          special_characters_map.emplace(static_cast<int>(sc), std::string{"\\"} + std::string(1, sc));
        }
      }

      if (SPECIAL_CHARACTERS.find(c) != std::string_view::npos) {
        result_string += special_characters_map[static_cast<int>(c)];
      } else {
        result_string += c;
      }
    }
  }
  return std::string{"(("} + result_string + std::string{R"()|[\r\n])$)"};
}

std::regex compile_pattern(std::string_view pattern) {
  return std::regex(translate(pattern), std::regex::ECMAScript);
}

bool fnmatch(std::string&& name, const std::regex& pattern) {
  return std::regex_match(std::move(name), pattern);
}

std::vector<fs::path> filter(const std::vector<fs::path> &names,
                             std::string_view pattern) {
  // std::cout << "Pattern: " << pattern << "\n";
  const auto pattern_re = compile_pattern(pattern);
  std::vector<fs::path> result;
  std::copy_if(std::make_move_iterator(names.begin()), std::make_move_iterator(names.end()),
               std::back_inserter(result),
               [&pattern_re](const fs::path& name) { return fnmatch(name.string(), pattern_re); });
  return result;
}

fs::path expand_tilde(fs::path path) {
  if (path.empty()) return path;

#ifdef _WIN32
    const char * home_variable = "USERNAME";
#else
    const char * home_variable = "USER";
#endif
    const char * home = std::getenv(home_variable);

  if (home == nullptr) {
    throw std::invalid_argument("error: Unable to expand `~` - HOME environment variable not set.");
  }

  std::string s = path.string();
  if (s[0] == '~') {
    s = std::string{home} + s.substr(1, s.size() - 1);
    return fs::path(s);
  }
  return path;
}

bool has_magic(const std::string &pathname) {
  static const auto magic_check = std::regex("([*?[])");
  return std::regex_search(pathname, magic_check);
}

constexpr bool is_hidden(std::string_view pathname) noexcept { return pathname[0] == '.'; }

constexpr bool is_recursive(std::string_view pattern) noexcept { return pattern == std::string_view{"**"}; }

std::vector<fs::path> iter_directory(const fs::path &dirname, bool dironly) {
  std::vector<fs::path> result;

  auto current_directory = dirname;
  if (current_directory.empty()) {
    current_directory = fs::current_path();
  }

  if (fs::exists(current_directory)) {
    try {
      for (auto &entry : fs::directory_iterator(
              current_directory, fs::directory_options::follow_directory_symlink |
                                      fs::directory_options::skip_permission_denied)) {
        if (!dironly || entry.is_directory()) {
          if (dirname.is_absolute()) {
            result.push_back(entry.path());
          } else {
            result.push_back(fs::relative(entry.path()));
          }
        }
      }
    } catch (std::exception&) {
      // not a directory
      // do nothing
    }
  }

  return result;
}

// Recursively yields relative pathnames inside a literal directory.
std::vector<fs::path> rlistdir(const fs::path &dirname, bool dironly) {
  std::vector<fs::path> result;
  auto names = iter_directory(dirname, dironly);
  for (auto &&name : names) {
    if (!is_hidden(name.string())) {
      result.push_back(name);
      auto matched_dirs = rlistdir(name, dironly);
      std::copy(std::make_move_iterator(matched_dirs.begin()), std::make_move_iterator(matched_dirs.end()), std::back_inserter(result));
    }
  }
  return result;
}

// This helper function recursively yields relative pathnames inside a literal
// directory.
std::vector<fs::path> glob2(const fs::path &dirname, [[maybe_unused]] const fs::path &pattern,
                            bool dironly) {
  // std::cout << "In glob2\n";
  std::vector<fs::path> result{"."};
  assert(is_recursive(pattern.string()));
  auto matched_dirs = rlistdir(dirname, dironly);
  std::copy(std::make_move_iterator(matched_dirs.begin()), std::make_move_iterator(matched_dirs.end()), std::back_inserter(result));
  return result;
}

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<fs::path> glob1(const fs::path &dirname, const fs::path &pattern,
                            bool dironly) {
  // std::cout << "In glob1\n";
  std::vector<fs::path> filtered_names;
  auto names = iter_directory(dirname, dironly);
  for (auto &&name : names) {
    if (!is_hidden(name.string())) {
      filtered_names.push_back(name.filename());
      // if (name.is_relative()) {
      //   // std::cout << "Filtered (Relative): " << name << "\n";
      //   filtered_names.push_back(fs::relative(name));
      // } else {
      //   // std::cout << "Filtered (Absolute): " << name << "\n";
      //   filtered_names.push_back(name.filename());
      // }
    }
  }
  return filter(filtered_names, pattern.string());
}

std::vector<fs::path> glob0(const fs::path &dirname, const fs::path &basename,
                            bool /*dironly*/) {
  // std::cout << "In glob0\n";

  // 'q*x/' should match only directories.
  if ((basename.empty() && fs::is_directory(dirname)) || (!basename.empty() && fs::exists(dirname / basename))) {
    return {basename};
  }
  return {};
}

std::vector<fs::path> glob(const fs::path &inpath, bool recursive = false,
                           bool dironly = false) {
  std::vector<fs::path> result;

  const auto pathname = inpath.string();
  auto path = fs::path(pathname);

  if (pathname[0] == '~') {
    // expand tilde
    path = expand_tilde(path);
  }

  auto dirname = path.parent_path();
  const auto basename = path.filename();

  if (!has_magic(pathname)) {
    assert(!dironly);

    // Patterns ending with a slash should match only directories
    if ((!basename.empty() && fs::exists(path)) || (basename.empty() && fs::is_directory(dirname))) {
      result.push_back(path);
    }
    return result;
  }

  if (dirname.empty()) {
    if (recursive && is_recursive(basename.string())) {
      return glob2(dirname, basename, dironly);
    }
    return glob1(dirname, basename, dironly);
  }

  std::vector<fs::path> dirs{dirname};
  if (dirname != fs::path(pathname) && has_magic(dirname.string())) {
    dirs = glob(dirname, recursive, true);
  }

  auto glob_in_dir = glob0;
  if (has_magic(basename.string())) {
    if (recursive && is_recursive(basename.string())) {
      glob_in_dir = glob2;
    } else {
      glob_in_dir = glob1;
    }
  }

  for (auto &d : dirs) {
    for (auto &&name : glob_in_dir(d, basename, dironly)) {
      fs::path subresult = name;
      if (name.parent_path().empty()) {
        subresult = d / name;
      }
      result.push_back(subresult.lexically_normal());
    }
  }

  return result;
}

} // namespace end

std::vector<fs::path> glob(const std::string &pathname) {
  return glob(pathname, false);
}

std::vector<fs::path> rglob(const std::string &pathname) {
  return glob(pathname, true);
}

std::vector<fs::path> glob(const std::vector<std::string> &pathnames) {
  std::vector<fs::path> result;
  for (const auto &pathname : pathnames) {
    auto matched_res = glob(pathname, false);
    std::copy(std::make_move_iterator(matched_res.begin()), std::make_move_iterator(matched_res.end()), std::back_inserter(result));
  }
  return result;
}

std::vector<fs::path> rglob(const std::vector<std::string> &pathnames) {
  std::vector<fs::path> result;
  for (const auto &pathname : pathnames) {
    auto matched_res = glob(pathname, true);
    std::copy(std::make_move_iterator(matched_res.begin()), std::make_move_iterator(matched_res.end()), std::back_inserter(result));
  }
  return result;
}

std::vector<fs::path>
glob(const std::initializer_list<std::string> &pathnames) {
  return glob(std::vector<std::string>(pathnames));
}

std::vector<fs::path>
rglob(const std::initializer_list<std::string> &pathnames) {
  return rglob(std::vector<std::string>(pathnames));
}

} // namespace glob
