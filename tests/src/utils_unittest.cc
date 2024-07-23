#include <gtest/gtest.h>
#include "includes.h"

namespace{

// Utility functions

TEST(Utils, str_to_square){
    EXPECT_EQ(str_to_square("a1", false), 0);
    EXPECT_EQ(str_to_square("h8", false), 63);
    EXPECT_EQ(str_to_square("a8", false), 56);
    EXPECT_EQ(str_to_square("h1", false), 7);
    EXPECT_EQ(str_to_square("a8", true), 0);
    EXPECT_EQ(str_to_square("h1", true), 63);
    EXPECT_EQ(str_to_square("a1", true), 56);
    EXPECT_EQ(str_to_square("h8", true), 7);
    EXPECT_EQ(str_to_square("a0", true), -1);
    EXPECT_EQ(str_to_square("a", false), -1);
    EXPECT_EQ(str_to_square("a9", false), -1);
    EXPECT_EQ(str_to_square("i1", true), -1);
    EXPECT_EQ(str_to_square("i9", false), -1);
    EXPECT_EQ(str_to_square("i", true), -1);
}

TEST(Utils, square_to_str){
    EXPECT_EQ(square_to_str(0, false), "a1");
    EXPECT_EQ(square_to_str(63, false), "h8");
    EXPECT_EQ(square_to_str(56, false), "a8");
    EXPECT_EQ(square_to_str(7, false), "h1");
    EXPECT_EQ(square_to_str(0, true), "a8");
    EXPECT_EQ(square_to_str(63, true), "h1");
    EXPECT_EQ(square_to_str(56, true), "a1");
    EXPECT_EQ(square_to_str(7, true), "h8");
    EXPECT_EQ(square_to_str(-1, false), "??");
    EXPECT_EQ(square_to_str(64, true), "??");
}

} // namespace