#include <gtest/gtest.h>

#include <geometry/circle.hpp>

TEST( GeometryTests, InCircle ) {
    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    {
        // a is outside the circle
        std::pair<double,double> a( 5.0, 5.0 );
        auto expected_outside = geometry::is_point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_outside, geometry::in_circle::outside );
    }
    {
        // a is the circle
        std::pair<double,double> a( 0.0, 0.0 );
        auto expected_inside = geometry::is_point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_inside, geometry::in_circle::inside );
    }
    {
        // a is on the circle
        std::pair<double,double> a( 0.0, -1.0 );
        auto expected_on = geometry::is_point_in_circle( p, q, r, a );
        EXPECT_EQ( expected_on, geometry::in_circle::on );
    }
}