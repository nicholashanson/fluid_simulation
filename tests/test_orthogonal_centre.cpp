#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

TEST( GeometryTests, TriangleOrthoCentre ) {

    auto tri = fs::fvm::triangulate_rectangle<int,double>( 
        0.0, 2.0, // xmin, xmax
        0.0, 2.0, // ymin, ymax
        50, 50    // lattic points in x-dimension, lattice points in y-direction   
    );

    auto triangles = tri.get_triangles();

    auto first_triangle = *triangles.begin();

    auto [ c_x, c_y ] = fs::fvm::triangle_orthocenter( tri, first_triangle );

    EXPECT_DOUBLE_EQ( c_x, 0.020408163265306117 );
    EXPECT_DOUBLE_EQ( c_y, 0.020408163265306124 );
}
