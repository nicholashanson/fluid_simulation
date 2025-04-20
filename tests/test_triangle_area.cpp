#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

TEST( GeometryTests, TriangleArea ) {

    std::pair<double,double> p = { 0.0, 0.0 };  
    std::pair<double,double> q = { 1.0, 0.0 };  
    std::pair<double,double> r = { 1.0, 1.0 };  

    auto area = fs::fvm::triangle_area( p, q, r );

    EXPECT_DOUBLE_EQ( area, 0.5 );
}