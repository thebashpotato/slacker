#include "slacker_lock/utils/version.hpp"
#include <gtest/gtest.h>

TEST(Version, SlackerLogVersionTest) {
    EXPECT_EQ("0.1.0", slacker_lock::utils::getVersion());
}
