#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

#include <cmath>
#include <limits>

#include <vector>

TEST( DistanceToPolygonMultipleSegments, CenterPoint ) {
    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<std::vector<size_t>> boundary_nodes = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, 5.0 );
}

TEST( DistanceToPolygonMultipleSegments, RightOfLeftEdge ) {
    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<std::vector<size_t>> boundary_nodes = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };

    std::pair<double, double> p = { 15.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments(p, points, boundary_nodes);

    EXPECT_DOUBLE_EQ(dist, -5.0);
}

TEST( DistanceToPolygonMultipleSegments, AboveUpperEdge ) {
    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<std::vector<size_t>> boundary_nodes = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };

    std::pair<double, double> p = { 5.0, 15.0 };

    double dist = fs::fvm::distance_to_polygon_multiple_segments(p, points, boundary_nodes);

    EXPECT_DOUBLE_EQ(dist, -5.0);
}

