#include <doctest.hpp>
#include <glob/fnmatch.hpp>

using doctest::test_suite;

TEST_CASE("fnmatch can check patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch::fnmatch("abc", "abc"));
  REQUIRE(fnmatch::fnmatch("abc", "?*?"));
  REQUIRE(fnmatch::fnmatch("abc", "???*"));
  REQUIRE(fnmatch::fnmatch("abc", "*???"));
  REQUIRE(fnmatch::fnmatch("abc", "???"));
  REQUIRE(fnmatch::fnmatch("abc", "*"));
  REQUIRE(fnmatch::fnmatch("abc", "ab[cd]"));
  REQUIRE(fnmatch::fnmatch("abc", "ab[!de]"));
  REQUIRE_FALSE(fnmatch::fnmatch("abc", "ab[de]"));
  REQUIRE_FALSE(fnmatch::fnmatch("a", "??"));
  REQUIRE_FALSE(fnmatch::fnmatch("a", "b"));

  // these test that '\' is handled correctly in character sets;
  REQUIRE(fnmatch::fnmatch("\\", R"([\])"));
  REQUIRE(fnmatch::fnmatch("a", R"([!\])"));
  REQUIRE_FALSE(fnmatch::fnmatch("\\", R"([!\])"));

  // test that filenames with newlines in them are handled correctly.
  REQUIRE(fnmatch::fnmatch(R"(foo\nbar)", "foo*"));
  REQUIRE(fnmatch::fnmatch(R"(foo\nbar\n)", "foo*"));
  REQUIRE_FALSE(fnmatch::fnmatch(R"(\nfoo)", "foo*"));
  REQUIRE(fnmatch::fnmatch(R"(\n)", "*"));
}

TEST_CASE("fnmatch can check '*' patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch::fnmatch("foo/bar.txt", "*.txt"));
  REQUIRE(fnmatch::fnmatch("foo/bar.txt", "foo/*.txt"));
  REQUIRE_FALSE(fnmatch::fnmatch("foo/bar.txt", "foo/bar/*.txt"));
  REQUIRE_FALSE(fnmatch::fnmatch("foo/bar.txt", "foo/baz/*.txt"));
}

TEST_CASE("fnmatch can check '*' patterns" * test_suite("fnmatch")) {
  const std::string pattern = "fnmatch_*.cpp";
  REQUIRE_FALSE(fnmatch::fnmatch("init.cpp", pattern));
  REQUIRE(fnmatch::fnmatch("fnmatch_filter.cpp", pattern));
  REQUIRE(fnmatch::fnmatch("fnmatch_fnmatch.cpp", pattern));
  REQUIRE(fnmatch::fnmatch("fnmatch_fnmatchcase.cpp", pattern));
  REQUIRE(fnmatch::fnmatch("fnmatch_translate.cpp", pattern));
  REQUIRE_FALSE(fnmatch::fnmatch("index.rst", pattern));
}

TEST_CASE("fnmatch_case can check '*' patterns" * test_suite("fnmatch")) {
  const std::string pattern = "FNMATCH_*.cpp";
  REQUIRE_FALSE(fnmatch::fnmatch_case("init.cpp", pattern));
  REQUIRE_FALSE(fnmatch::fnmatch_case("fnmatch_filter.cpp", pattern));
  REQUIRE(fnmatch::fnmatch_case("FNMATCH_FNMATCH.cpp", pattern));
  REQUIRE_FALSE(fnmatch::fnmatch_case("fnmatch_fnmatchcase.cpp", pattern));
  REQUIRE_FALSE(fnmatch::fnmatch_case("fnmatch_translate.cpp", pattern));
  REQUIRE_FALSE(fnmatch::fnmatch_case("index.rst", pattern));
}

TEST_CASE("fnmatch_case can check patterns" * test_suite("fnmatch")) {
  REQUIRE_FALSE(fnmatch::fnmatch_case("AbC", "abc"));
  REQUIRE_FALSE(fnmatch::fnmatch_case("abc", "AbC"));
}

TEST_CASE("fnmatch_case can filter for all items that match pattern" * test_suite("fnmatch")) {
  const std::string pattern = "fnmatch_*.py";
  const std::vector<fs::path> files = {
    fs::path("__init__.py"),
    fs::path("fnmatch_filter.py"),
    fs::path("fnmatch_fnmatch.py"),
    fs::path("fnmatch_fnmatchcase.py"),
    fs::path("fnmatch_translate.py"),
    fs::path("index.rst")
  };

  const auto filter = fnmatch::filter(files, pattern);
  REQUIRE(filter.size() == 4);
  REQUIRE(filter[0].string() == "fnmatch_filter.py");
  REQUIRE(filter[1].string() == "fnmatch_fnmatch.py");
  REQUIRE(filter[2].string() == "fnmatch_fnmatchcase.py");
  REQUIRE(filter[3].string() == "fnmatch_translate.py");
}