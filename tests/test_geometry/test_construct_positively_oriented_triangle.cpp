#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, ConstructPositivelyOrientedTriangle ) {
    // ( p, q, r ) is a right-angle triangle
    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 1.0, 0.0 );
    std::pair<double,double> r( 0.0, 1.0 );
    std::pair<std::vector<double>,std::vector<double>> points( 
        { p.first, r.first, q.first },      // x components
        { p.second, r.second, q.second }    // y components
    );
    auto tri = geometry::construct_positively_oriented_triangle<double>( 
        points, 
        0, /* index of p in "points" */
        1, /* index of r in "points" */
        2  /* index of q in "points" */ );

    /*
        v, u, w are the order of traversal with respect to the indices 
        of p, r, q in "points", which should be:
            0, 1, 2
        because p, q, r are already positively-oriented
    */
    ASSERT_TRUE( tri );
    auto [ v, u, w ] = tri.value();
    std::tie( p, r, q ) = geometry::get_triangle_points( points, v, u, w );
    auto expected_positive = geometry::get_orient( p, r, q );
    ASSERT_EQ( expected_positive, geometry::orient::positive );
}