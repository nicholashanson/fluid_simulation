#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, Differentiate_BSpline ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	std::array<std::pair<double,double>,11> derivatives = { {
		{ 				 6.0, 				 12.0 },
		{ 				4.92,  7.1400000000000015 },
		{ 				4.08,  3.3599999999999994 },
		{ 3.4799999999999995,  0.6600000000000007 },
		{ 				3.12, -0.9600000000000004 },
		{ 				 3.0, 				 -1.5 },
		{ 				3.12, -1.6800000000000002 },
		{ 3.4799999999999995, -2.2199999999999998 },
		{ 				4.08, 			  	-3.12 },
		{ 				4.92, -	4.380000000000001 },
		{				 6.0, 			     -6.0 } } };
	EXPECT_EQ( geometry::differentiate( spline, 0.0 ), derivatives[ 0 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.1 ), derivatives[ 1 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.2 ), derivatives[ 2 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.3 ), derivatives[ 3 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.4 ), derivatives[ 4 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.5 ), derivatives[ 5 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.6 ), derivatives[ 6 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.7 ), derivatives[ 7 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.8 ), derivatives[ 8 ] );
	EXPECT_EQ( geometry::differentiate( spline, 0.9 ), derivatives[ 9 ] );
	EXPECT_EQ( geometry::differentiate( spline, 1.0 ), derivatives[ 10 ] );
}
