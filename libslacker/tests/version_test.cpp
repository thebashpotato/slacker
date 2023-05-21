#include "slacker/utils/version.hpp"
#include <gtest/gtest.h>

TEST(Version, SlackerVersionTest) {
    EXPECT_EQ("0.1.0", slacker::utils::getVersion());
}
