#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleCircumRadius ) {
    std::pair<double,double> p{ 0.0, 0.0 };
    std::pair<double,double> q{ 4.0, 0.0 };
    std::pair<double,double> r{ 0.0, 3.0 };
    double expected_radius = 2.5; 
    double actual_radius = geometry::get_triangle_circumradius( p, q, r );
    EXPECT_NEAR( actual_radius, expected_radius, 1e-8 );
}