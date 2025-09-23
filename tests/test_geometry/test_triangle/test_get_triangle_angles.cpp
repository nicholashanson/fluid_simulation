#include <gtest/gtest.h>

#include <geometry/triangle.hpp>

TEST( GeometryTest, GetTriangleAngles ) {
    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 1.0, 0.0 };
    std::pair<double, double> r = { 0.0, 1.0 };
    auto angles = geometry::get_triangle_angles( p, q, r);
    double angle_1{};
    double angle_2{}; 
    double angle_3{};
    std::tie( angle_1, angle_2, angle_3 ) = angles;
    EXPECT_NEAR( angle_1, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_2, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_3, M_PI / 2.0, 0.01 );
}