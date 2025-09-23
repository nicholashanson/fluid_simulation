#include <gtest/gtest.h>

#include <geometry/triangulation.hpp>

TEST( GeometryTest, GetLatticeBoundary ) {
    auto lattice_boundary = geometry::get_lattice_boundary( 50, 50 );
    EXPECT_EQ( lattice_boundary.size(), 196 );
}