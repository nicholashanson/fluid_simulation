#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetCurvature_BSpline ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	auto result = geometry::get_curvature( spline, 0.2 );
	EXPECT_EQ( result, -0.7314378369627899 );
}

TEST( GeometryTest, GetCurvature_CircularArc ) {}