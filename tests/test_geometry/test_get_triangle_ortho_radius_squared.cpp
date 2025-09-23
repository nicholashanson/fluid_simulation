#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleOrthoRadiusSquared ) {
    std::pair<double,double> p = { 0.0, 0.0 }; 
    std::pair<double,double> q = { 1.0, 0.0 }; 
    std::pair<double,double> r = { 0.0, 1.0 }; 
    double expected_ortho_radius = 0.0;
    auto result = geometry::get_triangle_ortho_radius_squared( p, q, r );
    EXPECT_NEAR( result, expected_ortho_radius, 1e-5 );
}