#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, BSpline ) {
	std::vector<std::pair<double, double>> control_points = {
        { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 2.0 }, { 3.0, 0.0 }
    };
    geometry::b_spline spline( control_points, 3 );
    EXPECT_EQ( spline.control_points.size(), 4 );
    EXPECT_EQ( spline.knots.size(), 8 );
}

TEST( GeometryTest, BSpline_Knots ) {
    auto spline = geometry::b_spline<double>( { { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
    std::vector<int> expected_knots = { 0, 0, 0, 0, 1, 2, 2, 2, 2 };
    EXPECT_EQ(spline.knots, expected_knots);
}

TEST( GeometryTest, BSpline_FindKnotSpan ) {
    auto spline = geometry::b_spline<double>( { { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
    EXPECT_EQ( geometry::find_knot_span( spline, 0.0 ), 3 );
}

TEST( GeometryTest, BSpline_Evaluate_Endpoints ) {
    auto spline = geometry::b_spline<double>( { { 0, 0 }, { 1, 2 }, { 2, 2 }, { 3, 0 } }, 3 );
    auto start = spline.evaluate( 0.0 );
    auto end = spline.evaluate( 1.0 );
    EXPECT_NEAR( start.first, 0.0, 1e-6 );
    EXPECT_NEAR( end.first, 3.0, 1e-6 );
}

TEST( GeometryTest, BSpline_Evaluate ) {
	auto spline = geometry::b_spline<double>( 
		{ { 0.0, 0.0 }, { 1.0, 2.0 }, { 2.0, 1.5 }, { 3.0, 1.0 }, { 4.0, 0.0 } }, 3 );
	std::array<std::pair<double,double>,11> results = { { 
		{                 0.0,                0.0 },
		{ 				0.544, 0.9480000000000002 },
		{  0.9920000000000002, 1.4640000000000004 },
		{  1.3679999999999999, 1.6560000000000001 },
		{  1.6960000000000002, 1.6320000000000001 },
		{                 2.0,             	  1.5 },
		{  2.3040000000000003, 				1.344 },
		{  2.6319999999999997, 1.1520000000000001 },
		{  				3.008,              0.888 },
		{  3.4560000000000004,              0.516 },
		{                 4.0,                0.0 } } }; 
	EXPECT_EQ( geometry::de_boor( spline, 0.0 ), results[ 0 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.1 ), results[ 1 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.2 ), results[ 2 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.3 ), results[ 3 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.4 ), results[ 4 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.5 ), results[ 5 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.6 ), results[ 6 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.7 ), results[ 7 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.8 ), results[ 8 ] );
	EXPECT_EQ( geometry::de_boor( spline, 0.9 ), results[ 9 ] );
	EXPECT_EQ( geometry::de_boor( spline, 1.0 ), results[ 10 ] );
}