
#pragma once
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
#if __has_include(<filesystem>)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs {
using namespace std::filesystem;
using ifstream = std::ifstream;
using ofstream = std::ofstream;
using fstream = std::fstream;
}
#endif
#endif
#ifndef GHC_USE_STD_FS
#include <glob/filesystem/fs_fwd.hpp>
namespace fs {
using namespace ghc::filesystem;
using ifstream = ghc::filesystem::ifstream;
using ofstream = ghc::filesystem::ofstream;
using fstream = ghc::filesystem::fstream;
} 
#endif

#include <cassert>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace glob {

namespace details {

bool string_replace(std::string& str, const std::string& from, const std::string& to);

std::string translate_to_regex_string(const std::string& pattern);

std::regex compile_pattern(const std::string& pattern);

std::vector<fs::path> filter(const std::vector<fs::path>& names, const std::string& pattern);

// Test whether FILENAME matches PATTERN, including case.
// This is a version of fnmatch() which doesn't case-normalize
//    its arguments.
bool fnmatch_case(const fs::path& name, const std::string& pattern);

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
bool fnmatch(const fs::path& name, const std::string& pattern);

}

}