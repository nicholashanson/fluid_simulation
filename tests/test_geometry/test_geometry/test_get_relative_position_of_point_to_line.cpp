#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetRelativePositionOfPointToLine ) {
    std::pair<double,double> a( 0.0, -1.0 );
    std::pair<double,double> b( 0.0, 1.0 );
    {
        std::pair<double,double> p( -1.0, 0.0 );
        auto expected_left = geometry::get_relaive_position_of_point_to_line( a, b, p );
        EXPECT_EQ( expected_left, geometry::relative_position::left );
    }
    {
        std::pair<double,double> p( 1.0, 0.0 );
        auto expected_right = geometry::get_relaive_position_of_point_to_line( a, b, p );
        EXPECT_EQ( expected_right, geometry::relative_position::right );
    }
    {
        std::pair<double,double> p = { 0.0, 0.0 };
        auto expected_on = geometry::get_relaive_position_of_point_to_line( a, b, p );
        EXPECT_EQ( expected_on, geometry::relative_position::on );
    }
}