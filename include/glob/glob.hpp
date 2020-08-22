
#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace glob {

/// \param pathname string containing a path specification
/// \return vector of paths that match the pathname
///
/// Pathnames can be absolute (/usr/src/Foo/Makefile) or relative (../../Tools/*/*.gif)
/// Pathnames can contain shell-style wildcards
/// Broken symlinks are included in the results (as in the shell)
std::vector<std::filesystem::path> glob(const std::string &pathname);

/// \param pathname string containing a path specification
/// \return vector of paths that match the pathname
///
/// Globs recursively.
/// The pattern “**” will match any files and zero or more directories, subdirectories and
/// symbolic links to directories.
std::vector<std::filesystem::path> rglob(const std::string &pathname);

} // namespace glob