#include <gtest/gtest.h>

#include <lin_alg/lin_alg.hpp>

TEST( LinAlgTest, LUSolve ) {
    const size_t rows = 3;
    const size_t columns = 3;
    /*
        A = [ 1.0, 1.0, 1.0       x +  y + z = 6.0
              2.0, 3.0, 1.0      2x + 3y + z = 11.0
              3.0, 2.0, 1.0 ]    3x + 2y + z = 10.0

        solution:
            x = 1.0
            y = 2.0
            z = 3.0
    */
    const std::array<double,rows*columns> coefficients = {
        1.0, 1.0, 1.0, 
        2.0, 3.0, 1.0, 
        3.0, 2.0, 1.0,
    };
    const std::array<double,rows> b = {
        6.0,    //  x +  y + z = 6.0
        11.0,   // 2x + 3y + z = 11.0
        10.0,   // 3x + 2y + z = 10.0 
    };

    lin_alg::matrix_<double,rows,columns> A( coefficients );
    auto LU_decomp = lin_alg::LU_decomposition<double,rows,columns>( A );
    ASSERT_TRUE( LU_decomp );
    auto [ L, U, ps ] = LU_decomp.value();
    auto solution = lin_alg::LU_solve<double,rows>( L, U, b, ps );
    std::array<double,rows> expected_solution{ 1.0, 2.0, 3.0 };
    for ( size_t i = 0; i < rows; ++i ) {
        EXPECT_NEAR( solution[ i ], expected_solution[ i ], 1e-9 ); 
    }
}