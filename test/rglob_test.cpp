#include <algorithm>
#include <chrono>
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

namespace {
std::vector<fs::path> sorted_paths(std::vector<fs::path> paths) {
  std::sort(paths.begin(), paths.end());
  return paths;
}

class RecursiveCurrentDirectoryTest : public ::testing::Test {
protected:
  fs::path original_directory;
  fs::path root;
  bool created = false;

  void SetUp() override {
    original_directory = fs::current_path();
    root = fs::temp_directory_path() /
           ("glob_cwd_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    created = fs::create_directory(root);
    ASSERT_TRUE(created);
    fs::create_directories(root / "sub" / "nested");
    std::ofstream(root / "root.txt").close();
    std::ofstream(root / "sub" / "root.txt").close();
    std::ofstream(root / "sub" / "nested" / "deep.txt").close();
    fs::current_path(root);
  }

  void TearDown() override {
    fs::current_path(original_directory);
    if (created) fs::remove_all(root);
  }
};
} // namespace

TEST_F(RecursiveCurrentDirectoryTest, WildcardIncludesCurrentDirectory) {
  const auto expected = sorted_paths({"root.txt", "sub/root.txt", "sub/nested/deep.txt"});
  EXPECT_EQ(sorted_paths(glob::rglob("**/*.txt")), expected);
  EXPECT_EQ(sorted_paths(glob::rglob("./**/*.txt")), expected);
  EXPECT_EQ(sorted_paths(glob::rglob((root / "**" / "*.txt").string())),
            sorted_paths({root / "root.txt", root / "sub/root.txt", root / "sub/nested/deep.txt"}));
  EXPECT_EQ(sorted_paths(glob::glob("**/*.txt")), sorted_paths({"sub/root.txt"}));
  EXPECT_TRUE(glob::rglob("missing/**/*.txt").empty());
}

TEST_F(RecursiveCurrentDirectoryTest, LiteralIncludesCurrentDirectory) {
  EXPECT_EQ(sorted_paths(glob::rglob("**/root.txt")), sorted_paths({"root.txt", "sub/root.txt"}));
  EXPECT_TRUE(glob::rglob("**/absent.txt").empty());
}

TEST_F(RecursiveCurrentDirectoryTest, TerminalRecursivePatternIncludesBase) {
  EXPECT_EQ(sorted_paths(glob::rglob("**")),
            sorted_paths({".", "root.txt", "sub", "sub/root.txt", "sub/nested", "sub/nested/deep.txt"}));
  EXPECT_EQ(sorted_paths(glob::rglob("**/")), sorted_paths({".", "sub/", "sub/nested/"}));
  EXPECT_EQ(sorted_paths(glob::rglob("sub/**")),
            sorted_paths({"sub/", "sub/root.txt", "sub/nested", "sub/nested/deep.txt"}));
  EXPECT_TRUE(glob::rglob("missing/**").empty());
}
