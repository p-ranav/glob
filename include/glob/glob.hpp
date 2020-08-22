
#pragma once
#include <glob/fnmatch.hpp>

namespace glob {

std::vector<fs::path> glob(const std::string &pathname, bool recursive = false);

}