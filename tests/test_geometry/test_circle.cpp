#include <gtest/gtest.h>

#include <cmath>

#include <geometry/circle.hpp>

/*
    TEST: InCirle

    test if three points ( 5.0, 5.0 ), ( 0.0, 0.0 ), ( 0.0, -1.0 ) are inside, outside or on the unit circle
*/
TEST( GeometryTests, InCircle ) {
    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    {
        std::pair<double,double> a( 5.0, 5.0 );
        auto expected_outside = geometry::point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_outside, geometry::in_circle::OUTSIDE );
    }
    {
        std::pair<double,double> a( 0.0, 0.0 );
        auto expected_inside = geometry::point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_inside, geometry::in_circle::INSIDE );
    }
    {
        std::pair<double,double> a( 0.0, -1.0 );
        auto expected_on = geometry::point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_on, geometry::in_circle::ON );
    }
}