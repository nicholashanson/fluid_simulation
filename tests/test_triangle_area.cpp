#include <gtest/gtest.h>

#include <cmath>

#include <fs/fvm/triangulation.hpp>

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

TEST( GeometryTests, TriangleArea ) {

    std::pair<double,double> p = { 0.0, 0.0 };  
    std::pair<double,double> q = { 1.0, 0.0 };  
    std::pair<double,double> r = { 1.0, 1.0 };  

    auto area = fs::fvm::triangle_area( p, q, r );

    EXPECT_DOUBLE_EQ( area, 0.5 );
}

TEST( GeometryTests, RightTriangleAngleTest ) {

    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 1.0, 0.0 };
    std::pair<double, double> r = { 0.0, 1.0 };

    auto angles = fs::fvm::triangle_angles(p, q, r);

    double angle_1{};
    double angle_2{}; 
    double angle_3{};

    std::tie( angle_1, angle_2, angle_3 ) = angles;

    EXPECT_NEAR( angle_1, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_2, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_3, M_PI / 2.0, 0.01 );
}

TEST( GeometryTests, SquaredTriangleLengthsWithSmallestIndex ) {

    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 3.0, 0.0 };
    std::pair<double, double> r = { 0.0, 4.0 };  

    double l_min{}; 
    double l_med{}; 
    double l_max{};
    size_t index{};

    auto result = fs::fvm::squared_triangle_lengths_with_smallest_index( p, q, r );
    
    std::tie( l_min, l_med, l_max, index ) = result;

    EXPECT_NEAR( l_min, 9.0, 0.01 ); 
    EXPECT_NEAR( l_med, 16.0, 0.01 ); 
    EXPECT_NEAR( l_max, 25.0, 0.01 ); 
    EXPECT_EQ( index, 0 ); 
    
    result = fs::fvm::squared_triangle_lengths_with_smallest_index( r, p, q );

    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;

    EXPECT_EQ( index, 1 ); 

    result = fs::fvm::squared_triangle_lengths_with_smallest_index( q, r, p );

    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;

    EXPECT_EQ( index, 2 ); 
}