
#pragma once
#include <filesystem>
#include <string>
#include <vector>
namespace fs = std::filesystem;

namespace glob {

std::vector<fs::path> glob(const std::string &pathname);

std::vector<fs::path> rglob(const std::string &pathname);

} // namespace glob