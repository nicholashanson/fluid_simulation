#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetSquaredTriangleLengthsWithSmallestIndex ) {
    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 3.0, 0.0 };
    std::pair<double, double> r = { 0.0, 4.0 };  
    double l_min{}; 
    double l_med{}; 
    double l_max{};
    std::size_t index{};
    auto result = geometry::get_squared_triangle_lengths_with_smallest_index( p, q, r );
    std::tie( l_min, l_med, l_max, index ) = result;
    EXPECT_NEAR( l_min, 9.0, 0.01 ); 
    EXPECT_NEAR( l_med, 16.0, 0.01 ); 
    EXPECT_NEAR( l_max, 25.0, 0.01 ); 
    EXPECT_EQ( index, 0 ); 
    result = geometry::get_squared_triangle_lengths_with_smallest_index( r, p, q );
    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;
    EXPECT_EQ( index, 1 ); 
    result = geometry::get_squared_triangle_lengths_with_smallest_index( q, r, p );
    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;
    EXPECT_EQ( index, 2 ); 
}

TEST( GeometryTest, GetSquaredTriangleLengthsWithSmallestIndex_ScaleneTriangle ) {
    std::pair<double,double> p{ 0,0 };
    std::pair<double,double> q{ 2,0 };
    std::pair<double,double> r{ 2,1 };
    auto [ l_min, l_med, l_max, index ] = geometry::get_squared_triangle_lengths_with_smallest_index(p,q,r);
    EXPECT_DOUBLE_EQ( l_min, 1.0 );  
    EXPECT_DOUBLE_EQ( l_med, 4.0 );   
    EXPECT_DOUBLE_EQ( l_max, 5.0 );  
    EXPECT_EQ( index, 1u );
}