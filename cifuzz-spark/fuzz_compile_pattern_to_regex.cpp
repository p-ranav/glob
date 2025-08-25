#include <fuzzer/FuzzedDataProvider.h>
#include <cifuzz/cifuzz.h> // For FUZZ_TEST and FuzzedDataProvider
#include <regex>           // For std::regex and std::regex_match
#include <string_view>     // For std::string_view

#include "glob/glob.h"     // For glob::compile_pattern_to_regex

FUZZ_TEST(const uint8_t *data, size_t size) {
  if (size < 1) {
    return;
  }

  FuzzedDataProvider fdp(data, size);
  std::string glob_pattern = fdp.ConsumeRandomLengthString(50);
  try {
    std::regex compiled_regex = glob::compile_pattern_to_regex(glob_pattern);
    // Generate fuzzed file path to match against.
    std::string path = fdp.ConsumeRandomLengthString(50);
    std::regex_match(path, compiled_regex);
  } catch (const std::regex_error &) {
    // Ignore regex errors - these are expected for invalid glob patterns.
  } catch (const std::length_error &) {
    // Ignore length errors - these are expected for invalid glob patterns.
  }
}
