#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetSecondDerivative_BSpline ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	std::array<std::pair<double,double>,11> second_derivatives = { {
		{       		-12.0, 				 -54.0 },
		{  -9.600000000000001, 				 -43.2 },
		{  -7.199999999999999, 				 -32.4 },
		{  -4.800000000000001, 				 -21.6 },
		{ -2.3999999999999995, -10.799999999999997 },
		{ 				  0.0, 				   0.0 },
		{  2.3999999999999995,  -3.599999999999999 },
		{   4.799999999999999,  -7.199999999999998 },
		{   7.200000000000001, 			     -10.8 },
		{   9.600000000000001,      	     -14.4 },
		{ 			 	 12.0, 			     -18.0 } } };
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.0 ), second_derivatives[ 0 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.1 ), second_derivatives[ 1 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.2 ), second_derivatives[ 2 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.3 ), second_derivatives[ 3 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.4 ), second_derivatives[ 4 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.5 ), second_derivatives[ 5 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.6 ), second_derivatives[ 6 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.7 ), second_derivatives[ 7 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.8 ), second_derivatives[ 8 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 0.9 ), second_derivatives[ 9 ] );
	EXPECT_EQ( geometry::get_second_derivative( spline, 1.0 ), second_derivatives[ 10 ] );
}

TEST( GeometryTest, GetSecondDerivative_BezierCurve ) {}


