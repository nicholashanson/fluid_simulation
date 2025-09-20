#include <gtest/gtest.h>
#include <lin_alg/lin_alg.hpp>
#include "test_constants.hpp"

TEST( LinAlgTests, LUDecomposition ) {
    const size_t rows = 3;
    const size_t columns = 3;
    const std::array<double,rows*columns> elements = {
         2.0, -1.0, -2.0, 
        -4.0,  6.0,  3.0, 
        -4.0, -2.0,  8.0, 
    };

    lin_alg::matrix_<double,rows,columns> m( elements );
    auto result = lin_alg::LU_decomposition<double,rows,columns>( m, false );
    ASSERT_TRUE( result ) << "No valid LU decomposition found" << std::endl;

    auto [ lower_triangular, upper_triangular, permutations ] = result.value();
    const std::array<double,rows*columns> expected_lower_triangular = {
         1.0,  0.0, 0.0, 
        -2.0,  1.0, 0.0, 
        -2.0, -1.0, 1.0,
    };
    const std::array<double,rows*columns> expected_upper_triangular = {
        2.0, -1.0, -2.0, 
        0.0,  4.0, -1.0, 
        0.0,  0.0,  3.0,
    };

    EXPECT_EQ( upper_triangular.elements, expected_upper_triangular );
    EXPECT_EQ( lower_triangular.elements, expected_lower_triangular );
    auto product = test::matrix_mult<double,rows>( lower_triangular.elements, upper_triangular.elements );
    EXPECT_EQ( product, elements );
}