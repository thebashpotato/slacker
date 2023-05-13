#include <gtest/gtest.h>
#include <slacker/version.hpp>

TEST(Version, StarterAppBasicAssertion) {
    EXPECT_EQ("0.1.0", slacker::get_version());
}
