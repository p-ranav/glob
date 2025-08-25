#include <gtest/gtest.h>
#include <stdlib.h>

#ifdef USE_SINGLE_HEADER
#include "glob/glob.hpp"
#else
#include "glob/glob.h"
#endif

TEST(globTest, simpleFileGlob) {
  auto pattern = glob::compile_pattern_to_regex("foo/*.txt");
  ASSERT_TRUE(std::regex_match("foo/bar.txt", pattern));
  ASSERT_TRUE(std::regex_match("foo/blub.bar.txt", pattern));
  ASSERT_FALSE(std::regex_match("blub.txt", pattern));
  ASSERT_FALSE(std::regex_match("foo/blub.json", pattern));
}

TEST(globTest, doubleStarGlob) {
  auto pattern = glob::compile_pattern_to_regex("foo/**/*.txt");
  ASSERT_TRUE(std::regex_match("foo/baz/bar.txt", pattern));
  ASSERT_TRUE(std::regex_match("foo/baz/blub/bar.txt", pattern));
  ASSERT_FALSE(std::regex_match("blub.txt", pattern));
  ASSERT_FALSE(std::regex_match("foo/blub.json", pattern));
  ASSERT_FALSE(std::regex_match("/home/user/foo/bar.txt", pattern));
  ASSERT_TRUE(std::regex_match("foo/bar.txt", pattern));
}

TEST(globTest, doubleStar_Star_FixedEnd) {
  auto pattern = glob::compile_pattern_to_regex("/home/*/**/test");
  ASSERT_TRUE(std::regex_match("/home/user/test", pattern));
  ASSERT_TRUE(std::regex_match("/home/user2/test", pattern));
  ASSERT_TRUE(std::regex_match("/home/test/a/b/c/test", pattern));
  ASSERT_FALSE(std::regex_match("/home/test/a/b/c/mytest", pattern));
}

TEST(globTest, doubleStar_Star_StarFixedEnd) {
  auto pattern = glob::compile_pattern_to_regex("/home/*/**/*stream");
  ASSERT_TRUE(std::regex_match("/home/user/stream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user2/stream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user/a/b/c/stream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user/a/b/c/stream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user/a/b/c/istream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user/a/b/c/my-stream", pattern));
  ASSERT_TRUE(std::regex_match("/home/user/a/b/c/youdontstream", pattern));
  ASSERT_FALSE(std::regex_match("/home/user/a/b/c/youdontstreamc", pattern));
}

TEST(globTest, exclude_directories_but_not_files) {
  auto pattern = glob::compile_pattern_to_regex("test*/**");
  ASSERT_TRUE(std::regex_match("test/whatever.c", pattern));
  ASSERT_FALSE(std::regex_match("test.c", pattern));
}
