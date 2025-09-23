#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetSquaredTriangleLengths_ScaleneTriangle ) {
    std::pair<double,double> p{ 0, 0 };
    std::pair<double,double> q{ 2, 0 };
    std::pair<double,double> r{ 2, 1 };
    auto [ l_min, l_med, l_max ] = geometry::get_squared_triangle_lengths( p, q, r );
    EXPECT_DOUBLE_EQ( l_min, 1.0 );  
    EXPECT_DOUBLE_EQ( l_med, 4.0 );   
    EXPECT_DOUBLE_EQ( l_max, 5.0 );  
}