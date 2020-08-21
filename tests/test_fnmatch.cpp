#include <doctest.hpp>
#include <glob/fnmatch.hpp>

using doctest::test_suite;

TEST_CASE("fnmatch can check '*' patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch::fnmatch("foo/bar.txt", "*.txt"));
  REQUIRE(fnmatch::fnmatch("foo/bar.txt", "foo/*.txt"));
  REQUIRE_FALSE(fnmatch::fnmatch("foo/bar.txt", "foo/bar/*.txt"));
  REQUIRE_FALSE(fnmatch::fnmatch("foo/bar.txt", "foo/baz/*.txt"));
}