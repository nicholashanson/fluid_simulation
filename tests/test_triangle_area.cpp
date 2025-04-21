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