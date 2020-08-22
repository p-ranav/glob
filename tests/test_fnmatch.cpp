#include <doctest.hpp>
#include <glob/fnmatch.hpp>
using doctest::test_suite;
using namespace glob;
using namespace glob::details;

TEST_CASE("fnmatch can check patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch("abc", "abc"));
  REQUIRE(fnmatch("abc", "?*?"));
  REQUIRE(fnmatch("abc", "???*"));
  REQUIRE(fnmatch("abc", "*???"));
  REQUIRE(fnmatch("abc", "???"));
  REQUIRE(fnmatch("abc", "*"));
  REQUIRE(fnmatch("abc", "ab[cd]"));
  REQUIRE(fnmatch("abc", "ab[!de]"));
  REQUIRE_FALSE(fnmatch("abc", "ab[de]"));
  REQUIRE_FALSE(fnmatch("a", "??"));
  REQUIRE_FALSE(fnmatch("a", "b"));

  // these test that '\' is handled correctly in character sets;
  REQUIRE(fnmatch("\\", R"([\])"));
  REQUIRE(fnmatch("a", R"([!\])"));
  REQUIRE_FALSE(fnmatch("\\", R"([!\])"));

  // test that filenames with newlines in them are handled correctly.
  REQUIRE(fnmatch(R"(foo\nbar)", "foo*"));
  REQUIRE(fnmatch(R"(foo\nbar\n)", "foo*"));
  REQUIRE_FALSE(fnmatch(R"(\nfoo)", "foo*"));
  REQUIRE(fnmatch(R"(\n)", "*"));
}

TEST_CASE("fnmatch can check '*' patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch("foo/bar.txt", "*.txt"));
  REQUIRE(fnmatch("foo/bar.txt", "foo/*.txt"));
  REQUIRE_FALSE(fnmatch("foo/bar.txt", "foo/bar/*.txt"));
  REQUIRE_FALSE(fnmatch("foo/bar.txt", "foo/baz/*.txt"));
}

TEST_CASE("fnmatch can check '*' patterns" * test_suite("fnmatch")) {
  const std::string pattern = "fnmatch_*.cpp";
  REQUIRE_FALSE(fnmatch("init.cpp", pattern));
  REQUIRE(fnmatch("fnmatch_filter.cpp", pattern));
  REQUIRE(fnmatch("fnmatch_fnmatch.cpp", pattern));
  REQUIRE(fnmatch("fnmatch_fnmatchcase.cpp", pattern));
  REQUIRE(fnmatch("fnmatch_translate.cpp", pattern));
  REQUIRE_FALSE(fnmatch("index.rst", pattern));
}

TEST_CASE("fnmatch_case can check '*' patterns" * test_suite("fnmatch")) {
  const std::string pattern = "FNMATCH_*.cpp";
  REQUIRE_FALSE(fnmatch_case("init.cpp", pattern));
  REQUIRE_FALSE(fnmatch_case("fnmatch_filter.cpp", pattern));
  REQUIRE(fnmatch_case("FNMATCH_FNMATCH.cpp", pattern));
  REQUIRE_FALSE(fnmatch_case("fnmatch_fnmatchcase.cpp", pattern));
  REQUIRE_FALSE(fnmatch_case("fnmatch_translate.cpp", pattern));
  REQUIRE_FALSE(fnmatch_case("index.rst", pattern));
}

TEST_CASE("fnmatch_case can check patterns" * test_suite("fnmatch")) {
  REQUIRE_FALSE(fnmatch_case("AbC", "abc"));
  REQUIRE_FALSE(fnmatch_case("abc", "AbC"));
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

  const auto filtered = details::filter(files, pattern);
  REQUIRE(filtered.size() == 4);
  REQUIRE(filtered[0].string() == "fnmatch_filter.py");
  REQUIRE(filtered[1].string() == "fnmatch_fnmatch.py");
  REQUIRE(filtered[2].string() == "fnmatch_fnmatchcase.py");
  REQUIRE(filtered[3].string() == "fnmatch_translate.py");
}

TEST_CASE("fnmatch_case can filter for all items that match pattern" * test_suite("fnmatch")) {
  const std::string pattern = "test[0-9].py";
  const std::vector<fs::path> files = {
    fs::path("__init__.py"),
    fs::path("test1.py"),
    fs::path("test2.py"),
    fs::path("index.rst")
  };

  const auto filtered = details::filter(files, pattern);
  REQUIRE(filtered.size() == 2);
  REQUIRE(filtered[0].string() == "test1.py");
  REQUIRE(filtered[1].string() == "test2.py");
}

TEST_CASE("fnmatch_case can check [] patterns" * test_suite("fnmatch")) {
  REQUIRE(fnmatch_case("test1.py", "test[0-9].py"));
  REQUIRE(fnmatch_case("test2.py", "test[0-9].py"));
  REQUIRE_FALSE(fnmatch_case("test17.py", "test[0-9].py"));
}