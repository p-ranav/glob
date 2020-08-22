
#include <functional>
#include <glob/fnmatch.hpp>

namespace glob {

namespace details {

bool has_magic(const std::string& pathname) {
  static const auto magic_check = std::regex("([*?[])");
  return std::regex_search(pathname, magic_check);
}

bool is_hidden(const std::string& pathname) {
  return pathname[0] == '.';
}

bool is_recursive(const std::string& pattern) {
  return pattern == "**";
}

std::vector<fs::path> iter_directory(const fs::path& dirname, bool dironly) {
  std::vector<fs::path> result;  

  auto current_directory = dirname;
  if (current_directory.empty()) {
    current_directory = fs::current_path();
  }

  for (auto& entry : fs::directory_iterator(current_directory)) {
    if (!dironly || entry.is_directory()) {
      result.push_back(entry.path());
    }
  }

  return result;
}

// Escape all special characters.
std::string escape(const std::string& pathname) {
  // Escaping is done by wrapping any of "*?[" between square brackets.
  // Meta-characters do not work in the drive part and shouldn't be escaped.

  // drive, pathname = os.path.splitdrive(pathname)
  auto path = fs::path(pathname);
  auto drive = path.root_name();
  auto relative_path = path.relative_path();

  static const auto magic_check = std::regex("([*?[])");
  // magic_check.sub(r'[\1]', pathname)
  relative_path = std::regex_replace(relative_path.string(), magic_check, R"([\1])");
  return drive / relative_path;
}

// Recursively yields relative pathnames inside a literal directory.
std::vector<fs::path> rlistdir(const fs::path& dirname, bool dironly) {
  std::vector<fs::path> result;
  auto names = iter_directory(dirname, dironly);
  for (auto& x : names) {
    if (!is_hidden(x.string())) {
      result.push_back(x);
      for (auto& y : rlistdir(x, dironly)) {
        result.push_back(y);
      }
    }
  }
  return result;
}

std::vector<fs::path> glob2(const fs::path& dirname, const std::string& pattern, bool dironly) {
  std::vector<fs::path> result;
  assert(is_recursive(pattern));
  for (auto& dir : rlistdir(dirname, dironly)) {
    result.push_back(dir);
  }
  return result;
}

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<fs::path> glob1(const fs::path& dirname, const std::string& pattern, bool dironly) {
  auto names = iter_directory(dirname, dironly);
  std::vector<fs::path> filtered_names;
  if (!is_hidden(pattern)) {
    for (auto& n : names) {
      if (!is_hidden(n.string())) {
        filtered_names.push_back(fs::relative(n));
      }
    }
  }
  return details::filter(filtered_names, pattern);
}

std::vector<fs::path> glob0(const fs::path& dirname, const fs::path& basename, bool dironly) {
  std::vector<fs::path> result;
  if (basename.empty()) {
    // 'q*x/' should match only directories.
    if (fs::is_directory(dirname)) {
      result = {basename};
    }
  }
  else {
    if (fs::exists(dirname / basename)) {
      result = {basename};
    }
  }
  return result;
}

std::vector<fs::path> glob(const std::string &pathname, bool recursive = false, bool dironly = false) {
  std::vector<fs::path> result;

  const auto path = fs::path(pathname);
  auto dirname = path.parent_path();
  const auto basename = path.filename();

  // if (dirname.empty()) {
  //   dirname = fs::current_path();
  // }

  if (!has_magic(pathname)) {
    assert(!dironly);
    if (!basename.empty()) {
      if (fs::exists(path)) {
        result.push_back(path);
      }
    } else {
      // Patterns ending with a slash should match only directories
      if (fs::is_directory(dirname)) {
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

  std::vector<fs::path> dirs;
  if (dirname != fs::path(pathname) && has_magic(dirname.string())) {
    dirs = details::glob(dirname, recursive, true);
  } else {
    dirs = {dirname};
  }

  std::function<std::vector<fs::path>(const fs::path&, const fs::path&, bool)> glob_in_dir;
  if (has_magic(basename.string())) {
    if (recursive && is_recursive(basename.string())) {
      glob_in_dir = details::glob2;
    } else {
      glob_in_dir = details::glob1;
    }
  } else {
    glob_in_dir = details::glob0;
  }

  for (auto& d : dirs) {
    for (auto& name : glob_in_dir(d, basename, dironly)) {
      result.push_back(fs::absolute(name));
    }
  }

  return result;
}

}

std::vector<fs::path> glob(const std::string &pathname, bool recursive = false) {
  return details::glob(pathname, recursive);
}

}