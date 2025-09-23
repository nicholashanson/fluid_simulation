#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, MinMax ) {
    auto min_max = geometry::min_max( 1, 2 );
    EXPECT_EQ( std::get<0>( min_max ), 1 );
    EXPECT_EQ( std::get<1>( min_max ), 2 );
}