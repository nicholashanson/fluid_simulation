#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetSquaredDistanceToSegment_PointOnSegment ) {
    double dist_2 = geometry::get_squared_distance_to_segment( 0.0, 0.0, 10.0, 0.0, 5.0, 0.0 );
    EXPECT_DOUBLE_EQ( dist_2, 0.0 );
}

TEST( GeometryTest, GetSquaredDistanceToSegment_PointOutsideSegment ) {
    double dist_2 = geometry::get_squared_distance_to_segment( 0.0, 0.0, 10.0, 0.0, 5.0, 5.0 );
    EXPECT_DOUBLE_EQ( dist_2, 25.0 );
}

TEST( GeometryTest, GetSquaredDistanceToSegment_PointClosestToEndpoint ) {
    double dist_2 = geometry::get_squared_distance_to_segment( 0.0, 0.0, 10.0, 0.0, 12.0, 5.0 );
    double expected = geometry::dist_sqr( std::pair<double, double>{ 10.0, 0.0 }, { 12.0, 5.0 } );
    EXPECT_DOUBLE_EQ( dist_2, expected );
}

TEST( GeometryTest, GetSquaredDistanceToSegment_ZeroLengthSegment ) {
    double dist_2 = geometry::get_squared_distance_to_segment( 2.0, 3.0, 2.0, 3.0, 5.0, 7.0 );
    double expected = geometry::dist_sqr( std::pair<double, double>{ 2.0, 3.0 }, { 5.0, 7.0 } );
    EXPECT_DOUBLE_EQ( dist_2, expected );
}