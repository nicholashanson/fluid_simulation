#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

/*
    TEST: ConstructPositivelyOrientedTriangle

    "points" is a pair of vectors:
    - the first vector represents the x components of the vertices
    - the second vector represents the y componnents of the vertices    

*/
TEST( GeometryTest, ConstructPositivelyOrientedTriangle ) {
    /*
        ( p, q, r ) is a right-angle triangle
        p: x = 0.0, y = 0.0
        q: x = 1.0, y = 0.0
        q: x = 0.0, y = 1.0
    */
    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 1.0, 0.0 );
    std::pair<double,double> r( 0.0, 1.0 );

    std::pair<std::vector<double>,std::vector<double>> points( 
        { p.first, r.first, q.first }, { p.second, r.second, q.second }
    );

    /*
        0, 1, 2 are the indices of p, q, r in "points"
    */
    auto tri = geometry::construct_positively_oriented_triangle<double>( points, 0, 1, 2 );

    /*
        v, u, w are the indices of the positively-oriented triangle, which should be:

            0, 1, 2

        because p, q, r are already positively-oriented
    */
    auto [ v, u, w ] = tri.value();
    std::tie( p, q, r ) = fs::fvm::get_triangle_points( points, v, u, w );
    auto expected_positive = fs::fvm::get_orient( p, q, r );
    ASSERT_EQ( expected_positive, fs::fvm::orient::POSITIVE );
}