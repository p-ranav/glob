
#pragma once
#include <functional>
#include <glob/fnmatch.hpp>

namespace glob {

namespace details {

bool has_magic(const std::string& pathname);

bool is_hidden(const std::string& pathname);

bool is_recursive(const std::string& pattern);

std::vector<fs::path> iter_directory(const fs::path& dirname, bool dironly);

// Escape all special characters.
std::string escape(const std::string& pathname);

// Recursively yields relative pathnames inside a literal directory.
std::vector<fs::path> rlistdir(const fs::path& dirname, bool dironly);

std::vector<fs::path> glob2(const fs::path& dirname, const std::string& pattern, bool dironly);

// These 2 helper functions non-recursively glob inside a literal directory.
// They return a list of basenames.  _glob1 accepts a pattern while _glob0
// takes a literal basename (so it only has to check for its existence).

std::vector<fs::path> glob1(const fs::path& dirname, const std::string& pattern, bool dironly);

std::vector<fs::path> glob0(const fs::path& dirname, const fs::path& basename, bool dironly);

std::vector<fs::path> glob(const std::string &pathname, bool recursive = false, bool dironly = false);

}

std::vector<fs::path> glob(const std::string &pathname, bool recursive = false);

}