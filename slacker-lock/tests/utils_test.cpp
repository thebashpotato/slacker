#include <gtest/gtest.h>
#include <slacker_lock/utils/version.hpp>

TEST(Version, StarterAppBasicAssertion) {
    EXPECT_EQ("0.1.0", slacker_lock::get_version());
}
