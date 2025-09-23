#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

TEST( GeometryTest, ConstructPolygonHierachyMultipleCurves_SingleCurve ) {
	std::vector<std::pair<double,double>> points = { 
		{ 0.0, 0.0 }, { 2.0, 0.0 }, { 2.0, 2.0 }, { 0.0, 2.0 } 
	}; 
	std::vector<std::vector<std::size_t>> boundaries = { { 0, 1, 2, 3 } };
	auto hierarchy = geometry::construct_polygon_hierarchy_multiple_curves( points, boundaries );
	EXPECT_EQ( hierarchy.trees.size(), 1 );
}

TEST( GeometryTest, ConstructPolygonHierachyMultipleCurves_NegativeDomain ) {
	std::vector<std::pair<double,double>> points = { 
		{ -1.0, -1.0 }, { 4.0, -1.0 }, { 4.0, 4.0 }, { -1.0, 4.0 }
	}; 
	std::vector<std::vector<std::size_t>> boundaries = { { 0, 1, 2, 3 } };
	auto hierarchy = geometry::construct_polygon_hierarchy_multiple_curves( points, boundaries );
	EXPECT_EQ( hierarchy.trees.size(), 1 );
}

TEST( GeometryTest, ConstructPolygonHierachyMultipleCurves ) {
	std::vector<std::pair<double,double>> points = { 
		{ -1.0, -1.0 }, { 4.0, -1.0 }, { 4.0, 4.0 }, { -1.0, 4.0 },
		{  0.0,  0.0 }, { 2.0,  0.0 }, { 2.0, 2.0 }, {  0.0, 2.0 }
	}; 
	std::vector<std::vector<std::size_t>> boundaries = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 } };
	auto hierarchy = geometry::construct_polygon_hierarchy_multiple_curves( points, boundaries );
	EXPECT_EQ( hierarchy.trees.size(), 2 );
}

TEST( GeometryTest, ConstructPolygonHierachyMultipleCurves_MultipleHoles ) {
	std::vector<std::pair<double,double>> points = { 
		{ -4.0, -4.0 }, {  4.0, -4.0 }, {  4.0,  4.0 }, { -4.0,  4.0 },
		{ -2.0, -2.0 }, { -1.0, -2.0 }, { -1.0, -1.0 }, { -2.0, -1.0 },
		{  1.0,  1.0 }, {  2.0,  1.0 }, {  2.0,  2.0 }, {  1.0,  2.0 }
	}; 
	std::vector<std::vector<std::size_t>> boundaries = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 8, 9, 10, 11 } };
	auto hierarchy = geometry::construct_polygon_hierarchy_multiple_curves( points, boundaries );
	EXPECT_EQ( hierarchy.trees.size(), 3 );
	EXPECT_EQ( hierarchy.trees[ 0 ]->children.size(), 2 ); 
	EXPECT_EQ( hierarchy.bounding_boxes[ 0 ], geometry::bounding_box( -4.0, 4.0, -4.0, 4.0 ) );
}