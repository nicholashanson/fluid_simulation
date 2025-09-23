#ifndef TEST_CONSTANTS_HPP
#define TEST_CONSTANTS_HPP

#include <vector>

#include <iomanip>

namespace test {

    inline const std::vector<std::pair<double, double>> square_points = { {  0.0,  0.0 }, 
                                                                          { 10.0,  0.0 }, 
                                                                          { 10.0, 10.0 }, 
                                                                          {  0.0, 10.0 } };

    inline const std::vector<std::vector<size_t>> square_boundary_nodes = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };

    // multiply two square matrices
    template<typename T,size_t Dim>
    std::array<T, Dim * Dim> matrix_mult( const std::array<T, Dim * Dim>& A,const std::array<T, Dim * Dim>& B ) {
        std::array<T,Dim * Dim> result = { ( T )0 };
        for ( size_t i = 0; i < Dim; ++i ) {
            for ( size_t j = 0; j < Dim; ++j ) {
                for ( size_t k = 0; k < Dim; ++k ) {
                    result[ j + i * Dim ] += A[ k + i * Dim ] * B[ j + k * Dim ];
                }
            }
        }
        return result;
    }

    template<typename T, size_t Dim>
    void print_matrix( const std::array<T, Dim * Dim>& matrix ) {
        for ( size_t i = 0; i < Dim; ++i ) {
            for ( size_t j = 0; j < Dim; ++j ) {
                std::cout << std::setw( 16 ) << matrix[ j + i * Dim ] << " ";
            }
            std::cout << std::endl;
        }
    }

} // namespace test

#endif