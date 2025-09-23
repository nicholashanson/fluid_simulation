#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleInradius_EquilateralTriangle ) {
    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 2.0, 0.0 );
    std::pair<double,double> r( 1.0, std::sqrt( 3.0 ) );
    double expected_area = std::sqrt( 3.0 );
    double expected_perimeter = 6.0;
    double expected_inradius = expected_area / ( 0.5 * expected_perimeter );
    double actual_inradius = geometry::get_triangle_inradius( p, q, r );
    EXPECT_NEAR( actual_inradius, expected_inradius, 1e-6 );
}

TEST( GeometryTest, GetTrianlgeInradius_RightAngleTriangle ) {
    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 3.0, 0.0 );
    std::pair<double,double> r( 0.0, 4.0 );
    double expected_area = 6.0;
    double expected_perimeter = 12.0;
    double expected_inradius = expected_area / ( 0.5 * expected_perimeter );
    double actual_inradius = geometry::get_triangle_inradius( p, q, r );
    EXPECT_NEAR( actual_inradius, expected_inradius, 1e-6 );
}
