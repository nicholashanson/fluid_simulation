#include <gtest/gtest.h>

#include <geometry/sphere.hpp>

TEST( GeometryTest, GetCircumsphere ) {
    geometry::three_d_point<double> p( 1.0, 0.0, 0.0 );
    geometry::three_d_point<double> q( 0.0, 1.0, 0.0 );
    geometry::three_d_point<double> r( 0.0, 0.0, 1.0 );
    auto [ center, radius ]  = geometry::get_circumsphere( p, q, r );
    double expected_radius = 0.816496580927726;
    double pos = 0.333333333333333;
    EXPECT_NEAR( radius, expected_radius, 1e-8 );
    EXPECT_NEAR( center.x, pos, 1e-8 );
    EXPECT_NEAR( center.y, pos, 1e-8 );
    EXPECT_NEAR( center.z, pos, 1e-8 );
}