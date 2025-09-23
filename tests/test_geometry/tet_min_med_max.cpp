#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, MinMedMax ) {
    auto min_med_max = geometry::min_med_max( 1, 2, 3 );
    EXPECT_EQ( std::get<0>( min_med_max ), 1 );
    EXPECT_EQ( std::get<1>( min_med_max ), 2 );
    EXPECT_EQ( std::get<2>( min_med_max ), 3 );
}