#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

#include "test_constants.hpp"

TEST( GeometryTest, PolygonFeaturesSingleSegment ) {
	auto features = geometry::get_polygon_featues_single_segment( test::square_points, std::vector<std::size_t>{ 0, 1, 2, 3 } );
	EXPECT_DOUBLE_EQ( features.first, 100.0 );
	EXPECT_DOUBLE_EQ( features.second.first, 5.0 );
	EXPECT_DOUBLE_EQ( features.second.second, 5.0 );
}