#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

TEST( GeometryTest, PointIsInPolygon ) {
	std::vector<std::pair<double,double>> points = { 
		{ 0.0, 0.0 }, { 2.0, 0.0 }, { 2.0, 2.0 }, { 0.0, 2.0 } 
	}; 
	std::vector<std::size_t> boundary = { 0, 1, 2, 3 };
	std::pair<double,double> point = { 1.0, 1.0 };
	ASSERT_TRUE( geometry::point_is_in_polygon( points, boundary, point ) );
}