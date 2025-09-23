#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleArea ) {
    std::pair<double,double> p = { 0.0, 0.0 };  
    std::pair<double,double> q = { 1.0, 0.0 };  
    std::pair<double,double> r = { 1.0, 1.0 };  
    auto area = geometry::get_triangle_area( p, q, r );
    EXPECT_DOUBLE_EQ( area, 0.5 );
}