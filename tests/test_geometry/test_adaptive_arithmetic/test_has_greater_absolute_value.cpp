#include <gtest/gtest.h>
#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, HasGreaterAbsoluteValue ) {
    EXPECT_TRUE( adaptive_arithmetic::has_greater_absolute_value( 3.0, 5.0 ) ); 
    EXPECT_FALSE( adaptive_arithmetic::has_greater_absolute_value( 10.0, 2.0 ) ); 
    EXPECT_FALSE( adaptive_arithmetic::has_greater_absolute_value( 4.0, 4.0 ) );  
}