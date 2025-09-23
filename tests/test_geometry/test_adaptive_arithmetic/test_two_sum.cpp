#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

// 1e16 and 1.0 are perfectly representable in binary ( both for float and double )
// 1e16 + 1.0 = 1e16
// so the error term should be 1.0
TEST( GeometryTest, TwoSum ) {
    double a = 1e16;
    double b = 1.0; 
    auto [ sum, err ] = adaptive_arithmetic::two_sum( a, b );
    ASSERT_EQ( sum, a );
    ASSERT_EQ( err, b );
}