#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, FastExpansionSumZeroElim_TwoElementExpansions ) {
    using T = double;
    std::array<T, 8> h{}; 
    std::array<T, 4> e = { 1.0, 0.1, 0.0, 0.0 };
    std::array<T, 4> f = { 2.0, 0.2, 0.0, 0.0 };
    size_t e_len = 2;
    size_t f_len = 2;
    auto [ h_len, h_result ] = adaptive_arithmetic::fast_expansion_sum_zero_elim<8>( e_len, e, f_len, f, h );
    double reconstructed = 0.0;
    for ( std::size_t i = 0; i < h_len; ++i ) {
        reconstructed += h_result[ i ];
    }
    EXPECT_DOUBLE_EQ( reconstructed, 3.3 );
    EXPECT_GT( h_len, 1u ); 
}