#include <gtest/gtest.h>


#ifdef USE_SINGLE_HEADER
#include "glob/glob.hpp"
#else
#include "glob/glob.h"
#endif


TEST(rglobTest, MatchNonExistent) {
    auto matches = glob::rglob("non-existent/**");
    EXPECT_EQ(matches.size(), 0);
}
