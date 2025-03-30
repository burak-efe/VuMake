#include <gtest/gtest.h>
#include ""

// Simple test case that always passes
TEST(SimpleTest, AlwaysPasses) {
    EXPECT_EQ(1, 1);  // The test will pass as 1 == 1
}