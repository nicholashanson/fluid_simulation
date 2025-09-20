#include <gtest/gtest.h>

#include <lin_alg/lin_alg.hpp>

TEST( LinAlgTests, Pivot ) {
    const size_t rows = 3;
    const size_t columns = 3;
    const std::array<double,rows*columns> elemenents_before_pivot = {
        0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0,
    };

    lin_alg::matrix_<double,rows,columns> m( elemenents_before_pivot );
    auto permutations = lin_alg::pivot<double,rows,columns>( m );
    const std::array<double,rows*columns> elemenents_after_pivot = {
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
    };
    EXPECT_EQ( m.elements, elemenents_after_pivot );
    EXPECT_EQ( permutations, ( std::array<size_t,rows>{ 1, 2, 0 } ) );
}