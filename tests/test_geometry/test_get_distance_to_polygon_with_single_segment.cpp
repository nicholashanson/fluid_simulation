#include <gtest/gtest.h>

#include "test_constants.hpp"

#include <geometry/polygon.hpp>

TEST( GeometryTest, GetDistanceToPolygonWithSingleSegment_CenterPointRightEdge ) {
    std::vector<std::size_t> boundary_nodes = { 1, 2 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( p, test::square_points, boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, 5.0 ); 
}

TEST( GeometryTest, GetDistanceToPolygonWithSingleSegment_CenterPointUpperEdge ) {
    std::vector<std::size_t> boundary_nodes = { 3, 2 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( p, test::square_points, boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, -5.0 ); 
}

TEST( GeometryTest, GetDistanceToPolygonWithSingleSegment_PointOnBottomEdge ) {
    std::vector<std::size_t> boundary_nodes = { 0, 1 };
    std::pair<double, double> p = { 5.0, 0.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( p, test::square_points, boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, 0.0 );  
}

TEST( GeometryTest, GetDistanceToPolygonWithSingleSegment_CenterPointAbove ) {
    std::vector<std::size_t> boundary_nodes = { 0, 1 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( p, test::square_points, boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, -5.0 );
}

TEST( GeometryTest, GetDistanceToPolygonWithSingleSegment_CenterPointRight ) {
    std::vector<std::size_t> boundary_nodes = { 0, 3 };
    std::pair<double, double> p = { 5.0, 5.0 };
    double dist = geometry::get_distance_to_polygon_with_single_segment( p, test::square_points, boundary_nodes );
    EXPECT_DOUBLE_EQ( dist, -5.0 );  
}
