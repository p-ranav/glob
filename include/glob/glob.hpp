
#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace glob {

std::vector<std::filesystem::path> glob(const std::string &pathname);

std::vector<std::filesystem::path> rglob(const std::string &pathname);

} // namespace glob