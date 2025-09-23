#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, Orient2_CounterClockwiseTriangle ) {
    std::pair<double, double> p{ 0.0, 0.0 };
    std::pair<double, double> q{ 1.0, 0.0 };
    std::pair<double, double> r{ 0.0, 1.0 };
    double det = adaptive_arithmetic::orient_2( p, q, r );
    EXPECT_GT( det, 0.0 );  
}

TEST( GeometryTest, Orient2_ClockwiseTriangle ) {
    std::pair<double, double> p{ 0.0, 0.0 };
    std::pair<double, double> q{ 0.0, 1.0 };
    std::pair<double, double> r{ 1.0, 0.0 };
    double det = adaptive_arithmetic::orient_2( p, q, r );
    EXPECT_LT( det, 0.0 );  
}

TEST( GeometryTest, Orient2_XAxisColinear) {
    std::pair<double, double> p{ 0.0, 0.0 };
    std::pair<double, double> q{ 1.0, 0.0 };
    std::pair<double, double> r{ 2.0, 0.0 };
    double det = adaptive_arithmetic::orient_2(p, q, r);
    EXPECT_EQ(det, 0.0);  
}

TEST( GeometryTest, Orient2_YAxisColinear ) {
    std::pair<double, double> p{ 0.0, 0.0 };
    std::pair<double, double> q{ 0.0, 1.0 };
    std::pair<double, double> r{ 0.0, 2.0 };
    double det = adaptive_arithmetic::orient_2( p, q, r );
    EXPECT_EQ( det, 0.0 );
}

TEST( GeometryTest, Orient2_NearlyColinear ) {
    std::pair<double, double> p{ 0.0, 0.0 };
    std::pair<double, double> q{ 1e-12, 0.0 };
    std::pair<double, double> r{ 2e-12, 0.0 };
    double det = adaptive_arithmetic::orient_2( p, q, r );
    EXPECT_EQ( det, 0.0 );
}

TEST( GeometryTest, Orient2_LargeCooridnates ) {
    std::pair<double, double> p{ 1e9, 1e9 };
    std::pair<double, double> q{ 1e9 + 1.0, 1e9 };
    std::pair<double, double> r{ 1e9, 1e9 + 1.0 };
    double det = adaptive_arithmetic::orient_2( p, q, r );
    EXPECT_GT( det, 0.0 );
}