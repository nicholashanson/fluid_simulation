#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

TEST( GeometryTest, FindDeepestContainingPolygonTree ) {
	std::vector<std::pair<double,double>> points = { 
		{ 0.0, 0.0 }, { 2.0, 0.0 }, { 2.0, 2.0 }, { 0.0, 2.0 } 
	}; 
	std::vector<std::vector<std::size_t>> boundaries = { { 0, 1, 2, 3 } };
	std::pair<double,double> point = { 1.0, 1.0 };
	geometry::polygon_heirarchy<std::size_t,double> heirarchy{};
	geometry::bounding_box bb{ 0.0, 2.0, 0.0, 2.0 };
	heirarchy.bounding_boxes.push_back( bb );
	auto* root = new geometry::polygon_tree<std::size_t>();
	root->index = 0;
	root->height = 0;
	heirarchy.trees[ root->index ] = root;
	geometry::polygon_heirarchy_context context( heirarchy, points, boundaries );
	ASSERT_EQ( geometry::find_deepest_containing_polygon_tree( context, root, point ), root );
}