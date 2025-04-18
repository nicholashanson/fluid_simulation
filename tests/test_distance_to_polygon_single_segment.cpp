#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

TEST( DistanceToPolygonSingleSegment, CenterPointBottomEdge ) {

    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<size_t> boundary_nodes = { 0, 1 };

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_single_segment( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

TEST( DistanceToPolygonSingleSegment, CenterPointLeftEdge ) {

    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<size_t> boundary_nodes = { 0, 3 };

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_single_segment( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 );  
}

TEST( DistanceToPolygonSingleSegment, CenterPointRightEdge ) {

    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<size_t> boundary_nodes = { 1, 2 };

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_single_segment( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, 5.0 ); 
}

TEST( DistanceToPolygonSingleSegment, CenterPointUpperEdge ) {

    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<size_t> boundary_nodes = { 3, 2 };

    std::pair<double, double> p = { 5.0, 5.0 };

    double dist = fs::fvm::distance_to_polygon_single_segment( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, -5.0 ); 
}

TEST( DistanceToPolygonSingleSegment, PointOnBottomEdge ) {

    std::vector<std::pair<double, double>> points = { {  0.0,  0.0 }, 
                                                      { 10.0,  0.0 }, 
                                                      { 10.0, 10.0 }, 
                                                      {  0.0, 10.0 } };

    std::vector<size_t> boundary_nodes = { 0, 1 };

    std::pair<double, double> p = { 5.0, 0.0 };

    double dist = fs::fvm::distance_to_polygon_single_segment( p, points, boundary_nodes );

    EXPECT_DOUBLE_EQ( dist, 0.0 );  
}

