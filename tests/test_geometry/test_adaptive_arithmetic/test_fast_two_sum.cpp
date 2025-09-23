#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, FastTwoSum_NoError ) {
    double a = 2.0;
    double b = 1.0;
    auto [ x, y ] = adaptive_arithmetic::fast_two_sum( a, b );
    EXPECT_EQ( x, 3.0 );
    EXPECT_EQ( y, 0.0 );
}

TEST( GeometryTest, FastTwoSum_DifferentMagnitude ) {
    double a = 1e16;
    double b = 1.0;
    auto [ x, y ] = adaptive_arithmetic::fast_two_sum( a, b );
    EXPECT_EQ( x, 1e16 );    
    EXPECT_EQ( y, 1.0  );
}