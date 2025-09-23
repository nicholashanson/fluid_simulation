#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include <vector>

#include "test_constants.hpp"

#include <geometry/polygon.hpp>

TEST( GeometryTests, GetDistanceToPolygonWithMultipleSegments_CenterPoint ) {
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_multiple_segments( p, test::square_points, test::square_boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, 5.0 );
}

TEST( GeometryTests, GetDistanceToPolygonWithMultipleSegments_RightOfLeftEdge ) {
    std::pair<double, double> p = { 15.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_multiple_segments( p, test::square_points, test::square_boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

TEST( GeometryTests, GetDistanceToPolygonWithMultipleSegments_AboveUpperEdge ) {
    std::pair<double, double> p = { 5.0, 15.0 };
    double dist = geometry::get_distance_to_polygon_with_multiple_segments( p, test::square_points, test::square_boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

