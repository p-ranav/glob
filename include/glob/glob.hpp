
#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace glob {

// `glob` finds all the pathnames matching a specified pattern according to the rules used
// by the Unix shell. No tilde expansion is done, but *, ?, and character ranges expressed
// with [] will be correctly matched. For a literal match, wrap the meta-characters in
// brackets. For example, '[?]' matches the character '?'.

// Return a possibly-empty list of path names that match pathname, which must be a string
// containing a path specification. pathname can be either absolute (like
// /usr/src/Python-1.5/Makefile) or relative (like ../../Tools/*/*.gif), and can contain
// shell-style wildcards. Broken symlinks are included in the results (as in the shell).
std::vector<std::filesystem::path> glob(const std::string &pathname);

// Similar to glob.
// The pattern “**” will match any files and zero or more directories, subdirectories and
// symbolic links to directories.
std::vector<std::filesystem::path> rglob(const std::string &pathname);

} // namespace glob