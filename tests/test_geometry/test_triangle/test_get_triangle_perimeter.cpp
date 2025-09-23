#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTrianglePerimeter_EquilateralTriangle ) {
    std::pair<double,double> p{   0, 0 };
    std::pair<double,double> q{   1, 0 };
    std::pair<double,double> r{ 0.5, std::sqrt( 3.0 ) / 2.0 };
    double perimeter = geometry::get_triangle_perimeter( p,q,r );
    EXPECT_NEAR( perimeter, 3.0, 1e-12 ); 
}