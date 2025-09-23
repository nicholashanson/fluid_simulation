#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, TwoDiffTailTest_PositiveNumbers ) {
    double a = 5.0;
    double b = 3.0;
    double x = a - b;
    double y = adaptive_arithmetic::two_diff_tail( a, b, x );
    EXPECT_EQ( x + y, a - b ); 
}
