#include <gtest/gtest.h>

#include <geometry/triangulation.hpp>

TEST( GeometryTest, GetTriangleCirucmcenter ) {
    std::pair<double,double> p = { 0.0, 0.0 }; 
    std::pair<double,double> q = { 4.0, 0.0 }; 
    std::pair<double,double> r = { 2.0, 4.0 }; 
    std::pair<double, double> expected_circumcenter = { 2.0, 1.5 };
    auto result = geometry::get_triangle_circumcenter( p, q, r );
    EXPECT_NEAR( result.first, expected_circumcenter.first, 1e-5 );
    EXPECT_NEAR( result.second, expected_circumcenter.second, 1e-5 );
}

TEST( GeometryTests, GetTriangleCircumCenter_Triangulation ) {
    auto tri = geometry::triangulate_rectangle<int,double>( 
        0.0, 2.0, // xmin, xmax
        0.0, 2.0, // ymin, ymax
        50, 50    // lattic points in x-dimension, lattice points in y-direction   
    );
    auto triangles = tri.get_triangles();
    ASSERT_FALSE( triangles.empty() );
    auto first_triangle = *triangles.begin();
    auto [ c_x, c_y ] = geometry::get_triangle_circumcenter( tri, first_triangle );
    EXPECT_DOUBLE_EQ( c_x, 0.020408163265306117 );
    EXPECT_DOUBLE_EQ( c_y, 0.020408163265306124 );
}