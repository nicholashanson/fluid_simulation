#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, FastTwoSumTail_PositiveNumbers) {
    double a = 1e10;
    double b = 1.0;
    double x = a + b; 
    double y = adaptive_arithmetic::fast_two_sum_tail( a, b, x );
    double reconstructed = x + y;
    EXPECT_EQ( reconstructed, a + b );
}