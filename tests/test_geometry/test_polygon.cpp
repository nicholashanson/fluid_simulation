#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

#include "test_constants.hpp"

#include <geometry/polygon.hpp>

TEST( GeometryTests, DistanceToSingleSegment1 ) {

    std::vector<size_t> boundary_nodes = { 0, 1 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::distance_to_polygon_single_segment( p, test::square_points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 );
}