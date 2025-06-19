#include <gtest/gtest.h>

#include <cmath>

#include <fs/fvm/triangulation.hpp>
#include <geometry/circle.hpp>

/*
    TEST: InCirle

    test if three points ( 5.0, 5.0 ), ( 0.0, 0.0 ), ( 0.0, -1.0 ) are inside, outside or on the unit circle
*/
TEST( GeometryTests, InCircle ) {

    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    std::pair<double,double> a(  5.0, 5.0 );

    auto expected_outside = geometry::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_outside, fs::fvm::in_circle::OUTSIDE );

    a = { 0.0, 0.0 };

    auto expected_inside = geometry::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_inside, fs::fvm::in_circle::INSIDE );

    a = { 0.0, -1.0 };

    auto expected_on = geometry::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_on, fs::fvm::in_circle::ON );
}