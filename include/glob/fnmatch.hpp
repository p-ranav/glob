
#pragma once
#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
namespace fs = std::filesystem;

namespace glob {

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