#include <gtest/gtest.h>

#include <lin_alg/lin_alg.hpp>

TEST( LinAlgTest, SolveCramer ) {
    geometry::three_d_point<double> p( 1.0, 0.0, 0.0 );
    geometry::three_d_point<double> q( 0.0, 1.0, 0.0 );
    geometry::three_d_point<double> r( 0.0, 0.0, 1.0 );
    geometry::three_d_point<double> D( 1.0, 1.0, 1.0 );
    geometry::matrix<double> A( p, q, r );
    geometry::three_d_point<double> solution = lin_alg::solve_cramer( A, D );
    EXPECT_NEAR( solution.x, 1.0, 1e-9 );
    EXPECT_NEAR( solution.y, 1.0, 1e-9 );
    EXPECT_NEAR( solution.z, 1.0, 1e-9 );
}
