#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, Orient2Adapt_CounterClockwise ) { 
	std::pair<double,double> p{ 0,0 }; 
	std::pair<double,double> q{ 1,0 }; 
	std::pair<double,double> r{ 0,1 }; 
	double det_sum = adaptive_arithmetic::calculate_det_sum( p, q, r ); 
	double det = geometry::orient_2_adapt( p, q, r, det_sum ); 
	EXPECT_GT( det, 0.0 );
}