#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleOrthoCenter_RightAngleTriangle ) {
    std::pair<double,double> p = { 0.0, 0.0 }; 
    std::pair<double,double> q = { 1.0, 0.0 }; 
    std::pair<double,double> r = { 0.0, 1.0 }; 
    auto [ c_x, c_y ] = geometry::get_triangle_ortho_center( p, q, r );
    EXPECT_NEAR( c_x, 0.0, 1e-5 );
    EXPECT_NEAR( c_y, 0.0, 1e-5 );
}