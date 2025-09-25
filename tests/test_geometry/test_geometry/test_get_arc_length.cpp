#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetArcLength_LinearSpline ) {
	auto spline = geometry::b_spline<double>( { { 0, 0 },{ 1, 0 } }, 1 );
	auto result = geometry::get_arc_length( spline, 0.0, 1.0 );
 	EXPECT_NEAR( result, 1.0, 1e-12 );
}

TEST( GeometryTest, GetArcLength ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	auto result = geometry::get_arc_length( spline, 0.0, 1.0 );
	EXPECT_NEAR( result, 5.38384992382059657, 2e-3 );
}
