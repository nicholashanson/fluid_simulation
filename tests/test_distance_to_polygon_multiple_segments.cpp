#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

#include <cmath>
#include <limits>

#include <vector>

#include "test_constants.hpp"

TEST( DistanceToPolygonMultipleSegments, CenterPoint ) {

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments( p, test::square_points, test::square_boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, 5.0 );
}

TEST( DistanceToPolygonMultipleSegments, RightOfLeftEdge ) {

    std::pair<double, double> p = { 15.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments( p, test::square_points, test::square_boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

TEST( DistanceToPolygonMultipleSegments, AboveUpperEdge ) {

    std::pair<double, double> p = { 5.0, 15.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments( p, test::square_points, test::square_boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

