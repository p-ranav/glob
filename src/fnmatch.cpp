#if !(defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include))
#if __has_include(<filesystem>)
#include <glob/filesystem/fs_impl.hpp>
#endif
#endif

#include <glob/fnmatch.hpp>

namespace glob {

namespace details {

bool string_replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::string translate_to_regex_string(const std::string& pattern) {
  std::size_t i = 0, n = pattern.size();
  std::string result_string;

  while(i < n) {
      auto c = pattern[i];
      i += 1;
      if (c == '*') {
        result_string += ".*";
      } else if (c == '?') {
        result_string += ".";
      }
      else if (c == '[') {
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
        }
        else {
          auto stuff = std::string(pattern.begin() + i, pattern.begin() + j);
          if (stuff.find("--") == std::string::npos) {
            string_replace(stuff, std::string{"\\"}, std::string{R"(\\)"});
          }
          else {
            std::vector<std::string> chunks;
            std::size_t k = 0;
            if (pattern[i] == '!') {
              k = i + 2;
            } else {
              k = i + 1;
            }

            while(true) {
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
            for (auto& s : chunks) {
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
          std::regex_replace(std::back_inserter(result),   // ressult 
            stuff.begin(), stuff.end(),                    // string
            std::regex(std::string{R"([&~|])"}),           // pattern
            std::string{R"(\\\1)"});                       // repl
          stuff = result;

          i = j + 1;
          if (stuff[0] == '!') {
            stuff = "^" + std::string(stuff.begin() + 1, stuff.end());
          }
          else if (stuff[0] == '^' || stuff[0] == '[') {
            stuff = "\\\\" + stuff;
          }
          result_string = result_string + "[" + stuff + "]";
        }
      }
      else {
        // SPECIAL_CHARS
        // closing ')', '}' and ']'
        // '-' (a range in character set)
        // '&', '~', (extended character set operations)
        // '#' (comment) and WHITESPACE (ignored) in verbose mode
        static std::string special_characters = "()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
        static std::map<int, std::string> special_characters_map;
        if (special_characters_map.empty()) {
          for (auto& c : special_characters) {
            special_characters_map.insert(std::make_pair(static_cast<int>(c), std::string{"\\"} + std::string(1, c)));
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

std::regex compile_pattern(const std::string& pattern) {
  return std::regex(translate_to_regex_string(pattern), std::regex::ECMAScript);
}

std::vector<fs::path> filter(const std::vector<fs::path>& names, const std::string& pattern) {
  std::vector<fs::path> result;
  auto regex = compile_pattern(pattern);
  for (auto& name : names) {
    if (std::regex_match(name.string(), regex)) {
      result.push_back(name);
    }
  }
  return result;
}

// Test whether FILENAME matches PATTERN, including case.
// This is a version of fnmatch() which doesn't case-normalize
//    its arguments.
bool fnmatch_case(const fs::path& name, const std::string& pattern) {
  return std::regex_match(name.string(), compile_pattern(pattern));
}

// Test whether FILENAME matches PATTERN.
// Patterns are Unix shell style:
// *       matches everything
// ?       matches any single character
// [seq]   matches any character in seq
// [!seq]  matches any char not in seq
// An initial period in FILENAME is not special.
// Both FILENAME and PATTERN are first case-normalized
// if the operating system requires it.
// If you don't want this, use fnmatchcase(FILENAME, PATTERN).
bool fnmatch(const fs::path& name, const std::string& pattern) {
  auto name_normal = name.lexically_normal();
  auto pattern_normal = fs::path(pattern).lexically_normal();
  return fnmatch_case(name_normal, pattern_normal.string());
}

}

}