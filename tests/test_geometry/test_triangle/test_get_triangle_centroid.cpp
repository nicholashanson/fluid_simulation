#include <gtest/gtest.h>

#include <cmath>

#include <geometry/triangle.hpp>

TEST( GeometryTests, GetTriangleCentroid ) {
    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    std::pair<double,double> expected_centroid( 0.0, 0.3333333333333333 );
    auto actual_centroid = geometry::get_triangle_centroid( p, q, r );
    ASSERT_EQ( expected_centroid, actual_centroid );
}