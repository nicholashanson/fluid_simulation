#include <gtest/gtest.h>

#include <array>

#include <fs/fs.hpp>

template <typename T, size_t Dim>
void print_matrix( const std::array<T, Dim * Dim>& matrix ) {
    for ( size_t i = 0; i < Dim; ++i ) {
        for ( size_t j = 0; j < Dim; ++j ) {
            std::cout << matrix[ j + i * Dim ] << " ";
        }
        std::cout << std::endl;
    }
}

// multiply two square matrices
template<typename T, size_t Dim>
std::array<T, Dim * Dim> matrix_mult( 
    const std::array<T, Dim * Dim>& A,
    const std::array<T, Dim * Dim>& B
) {

    std::array<T, Dim * Dim> result;

    for ( size_t i = 0; i < Dim; ++i ) {
        for ( size_t j = 0; j < Dim; ++j ) {
            for ( size_t k = 0; k < Dim; ++k ) {

                result[ j + i * Dim ] += A[ k + i * Dim ] * B[ j + k * Dim ];
            }
        }
    }

    return result;
}

// test if square matrix is the identity matrix
template<typename T, size_t Dim>
bool is_identity_matrix( const std::array<T, Dim * Dim> matrix, T tol = 1e-9 ) {

    for ( size_t i = 0; i < Dim; ++i ) {
        for ( size_t j = 0; j < Dim; ++ j ) {

            T expected = ( i == j ) ? ( T )1 : ( T )0;
            if ( std::fabs( matrix [ j + i * Dim ] - expected ) > tol ) {
                return false;
            }
        }
    }
    return true;

}

TEST( LBMTests, MInverse ) {

    std::array<double, 2 * 2> T = {
        4.0, 7.0, 2.0, 6.0
    };

    std::array<double, 2 * 2> T_inv = {
        0.6, -0.7, -0.2, 0.4
    };

    auto result_T = matrix_mult<double, 2>( T, T_inv );

    EXPECT_TRUE( ( is_identity_matrix<double, 2>( result_T ) ) ) << "T * T_inv is not identity";

    auto result_M = matrix_mult<fs::lbm::T, 9>( fs::lbm::M, fs::lbm::M_inv );

    EXPECT_TRUE( ( is_identity_matrix<fs::lbm::T, 9>( result_M ) ) ) << "M * M_inv is not identity";
} 
