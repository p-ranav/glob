#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <gtest/gtest.h>
#include <stdlib.h>

#ifdef USE_SINGLE_HEADER
#include "glob/glob.hpp"
#else
#include "glob/glob.h"
#endif

namespace fs = std::filesystem;

class ScopedEnvironment {
public:
  ScopedEnvironment(const char *name, const std::string &value) : name_(name) {
#ifdef _WIN32
    char *previous = nullptr;
    size_t size = 0;
    _dupenv_s(&previous, &size, name);
    had_value_ = previous != nullptr;
    if (had_value_) previous_ = previous;
    std::free(previous);
#else
    const char *previous = std::getenv(name);
    had_value_ = previous != nullptr;
    if (had_value_) previous_ = previous;
#endif
    set(value.c_str());
  }

  ~ScopedEnvironment() { set(had_value_ ? previous_.c_str() : nullptr); }

  void set(const char *value) {
#ifdef _WIN32
    EXPECT_EQ(_putenv_s(name_.c_str(), value ? value : ""), 0);
#else
    EXPECT_EQ(value ? setenv(name_.c_str(), value, 1) : unsetenv(name_.c_str()), 0);
#endif
  }

private:
  std::string name_;
  std::string previous_;
  bool had_value_;
};

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

TEST(rglobTest, TildeUsesHomeDirectory) {
  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto home = fs::temp_directory_path() / ("glob home " + std::to_string(suffix));
  fs::create_directories(home);
  const auto file = home / "file.txt";
  std::ofstream(file).close();
#ifdef _WIN32
  ScopedEnvironment home_env("USERPROFILE", home.string());
  ScopedEnvironment user_env("USERNAME", "glob_test_login");
#else
  ScopedEnvironment home_env("HOME", home.string());
  ScopedEnvironment user_env("USER", "glob_test_login");
#endif
  EXPECT_EQ(glob::glob("~"), std::vector<fs::path>{home});
  EXPECT_EQ(glob::rglob("~"), std::vector<fs::path>{home});
  EXPECT_EQ(glob::glob("~/file.txt"), std::vector<fs::path>{file});
  EXPECT_EQ(glob::glob("~/*.txt"), std::vector<fs::path>{file});
  EXPECT_EQ(glob::rglob("~/file.txt"), std::vector<fs::path>{file});
  EXPECT_EQ(glob::rglob("~/*.txt"), std::vector<fs::path>{file});
  const std::vector<std::string> patterns{"~/file.txt"};
  EXPECT_EQ(glob::glob(patterns), std::vector<fs::path>{file});
  EXPECT_EQ(glob::rglob(patterns), std::vector<fs::path>{file});
  EXPECT_EQ(glob::glob({"~/file.txt"}), std::vector<fs::path>{file});
  EXPECT_EQ(glob::rglob({"~/file.txt"}), std::vector<fs::path>{file});
  EXPECT_TRUE(glob::glob("~/missing.txt").empty());
  home_env.set(nullptr);
  EXPECT_THROW(glob::glob("~"), std::invalid_argument);
  EXPECT_THROW(glob::rglob("~/file.txt"), std::invalid_argument);
  EXPECT_EQ(glob::glob(file.string()), std::vector<fs::path>{file});
  home_env.set("");
  EXPECT_THROW(glob::glob("~"), std::invalid_argument);
  fs::remove_all(home);
}
