#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

TEST( GeometryTests, PlaneThroughThreePoints ) {

    fs::fvm::three_d_point<double> p = { 1.0, 0.0, 0.0 };  
    fs::fvm::three_d_point<double> q = { 0.0, 1.0, 0.0 };  
    fs::fvm::three_d_point<double> r = { 0.0, 0.0, 1.0 };  

    fs::fvm::three_d_plane<double> plane = fs::fvm::get_plane_through_three_points( p, q, r );

    double expected_alpha = 1.0;
    double expected_beta = 1.0;
    double expected_gamma = 1.0;
    double expected_delta = -1.0;

    EXPECT_DOUBLE_EQ( plane.alpha, expected_alpha );
    EXPECT_DOUBLE_EQ( plane.beta, expected_beta );
    EXPECT_DOUBLE_EQ( plane.gamma, expected_gamma );
    EXPECT_DOUBLE_EQ( plane.delta, expected_delta );
}