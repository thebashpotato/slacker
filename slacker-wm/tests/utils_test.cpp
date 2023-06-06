#include "slacker_wm/utils/version.hpp"
#include <gtest/gtest.h>

TEST(Version, SlackerWmVersionTest) {
    EXPECT_EQ("0.1.0", slacker_wm::utils::getVersion());
}
