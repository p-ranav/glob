#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <stdlib.h>

#ifdef USE_SINGLE_HEADER
#include "glob/glob.hpp"
#else
#include "glob/glob.h"
#endif

namespace fs = std::filesystem;

fs::path mkdir_temp() {
  std::srand(std::time(nullptr));
  fs::path temp_dir = fs::temp_directory_path() / ("rglob_test_" + std::to_string(std::rand()));

  fs::create_directories(temp_dir);
  return temp_dir;
}

// regression test to avoid matching an non existing file
TEST(rglobTest, MatchNonExistent) {
  auto matches = glob::rglob("non-existent/**");
  EXPECT_EQ(matches.size(), 0);
}

// see https://github.com/p-ranav/glob/issues/3
TEST(rglobTest, Issue3) {
  auto temp_dir = mkdir_temp();
  std::cout << "Temporary directory: " << temp_dir << std::endl;

  fs::path sub1 = temp_dir / "sub";
  fs::path sub2 = sub1 / "sub";
  EXPECT_TRUE(fs::create_directory(sub1));
  EXPECT_TRUE(fs::create_directory(sub2));

  std::ofstream(temp_dir / "file.txt").close();
  std::ofstream(sub1 / "file.txt").close();
  std::ofstream(sub2 / "file.txt").close();

  auto pattern = temp_dir.string() + "/**/*.txt";
  std::cout << "Pattern: " << pattern << std::endl;

  auto matches = glob::rglob(pattern);
  EXPECT_EQ(matches.size(), 3);
  EXPECT_EQ(matches[0].string(), (temp_dir / "file.txt").string());
  EXPECT_EQ(matches[1].string(), (sub1 / "file.txt").string());
  EXPECT_EQ(matches[2].string(), (sub2 / "file.txt").string());
}
