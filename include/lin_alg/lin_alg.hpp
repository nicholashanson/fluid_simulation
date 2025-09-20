#ifndef LIN_ALG_HPP
#define LIN_ALG_HPP

#include <cmath>

#include <algorithm>
#include <limits>
#include <iterator>
#include <optional>
#include <numeric>
#include <tuple>
#include <vector>
#include <ranges>

#include <concepts>
#include <type_traits>

namespace lin_alg {

    template<typename T,size_t rows,size_t cols>
    struct matrix_ {
        std::array<T,rows*cols> elements;

        matrix_() : elements{} {}

        matrix_( const std::array<T,rows*cols>& elements )
            : elements( elements ) {} 

        T& operator[]( size_t row, size_t col ) {
            return elements[ col + row * cols ];
        }

        const T& operator[]( size_t row, size_t col ) const {
            return elements[ col + row * cols ];
        }

        void swap_rows( size_t row_a, size_t row_b ) {
            if ( row_a == row_b ) return; 
            std::swap_ranges( elements.begin() + row_a * cols, 
                              elements.begin() + row_a * cols + cols, 
                              elements.begin() + row_b * cols );
        }
    };

    /*
        pivot()

        re-arranges the rows of matrix m so that the abosolute value of each element on the diagonal of
        the matrix is the largest value in that column

        re-arranging the rows of the matrix in this way helps to improve the stability of LU_solve

        the permutations vector represents the order of the rows in the new permutated matrix. It starts
        as [ 0, 1, 2, ... n ], representing the rows in their original order. Each time rows in the 
        matrix are swapped, the corresponding elements in the permutations vector are aslo swapped. For
        example, if row 0 is swapped with row 2, then the 0th element in the permuations vector is 
        swapped with the second.

        the matrix is modified in place. 
        
        returns the permuation vector
    */
    template<typename T,size_t R,size_t C>
    std::array<size_t,R> pivot( matrix_<T,R,C>& m ) {

        std::array<size_t,R> permutations;
        std::iota( permutations.begin(), permutations.end(), 0 );

        for ( size_t k = 0; k < R; ++k ) {
            T max = std::abs( m[ k, k ] );
            size_t max_index = k;
            for ( size_t i = k; i < R; ++i ) {
                if ( std::abs( m[ i, k ] ) > max ) {
                    max = std::abs( m[ i, k ] );
                    max_index = i;
                }
            }
            if ( max_index != k ) {
                m.swap_rows( max_index, k );
                std::swap( permutations[ max_index ], permutations[ k ] );
            }
        }
        return permutations;
    }

    /*
        LU_decomposition()

        decompose matrix A into the upper-triangular matrix U and the lower-triangular matrix L using 
        the Doolittle Algorithm:

            A = LU

        use a permutation vector to keep track of any row-swaps that take place during pivoting so that:

            PA = LU

        P is the permutation matrix that corresponds to the permutation vector
    */
    template<typename T,size_t R,size_t C>
    std::optional<
        std::tuple<
            matrix_<T,R,C>,         // lower-triangular matrix 
            matrix_<T,R,C>,         // upper-triangular matrix
            std::array<size_t,R>>>  // permutation vector
    LU_decomposition( matrix_<T,R,C>& A, bool pivot_ = true ) {
        if ( R != C ) {
            return std::nullopt;
        }
        matrix_<T,R,C> L;   // lower-triangular matrix 
        matrix_<T,R,C> U;   // upper-triangular matrix
        const size_t n = R;

        std::array<size_t,n> permutations;  // permutation vector
        if ( pivot_ == false ) {
            // [ 0, 1, 2, ... , n - 1 ]:
            // pivoting is switched-off so we return this dummy-vector
            // as the permutation vector
            std::iota( permutations.begin(), permutations.end(), 0 );
        } else {
            // pivot the matrix A
            permutations = pivot( A );
        }
        // construct U and L in an inter-leaved way
        // using Doolittle's algorithm
        for ( size_t i = 0; i < n; ++i ) {
            // construct U
            for ( size_t j = i; j < n; ++j ) {
                T sum{};
                for ( size_t k = 0; k < i; ++k ) {
                    sum += L[ i, k] * U[ k, j ];
                }
                U[ i, j ] = A[ i, j ] - sum;
            }
            // construct L
            for ( size_t j = i; j < n; ++j ) {
                if ( i == j ) {
                    L[ i, j ] = ( T )1;
                } else {
                    T sum{};
                    for ( size_t k = 0; k < i; ++k ) {
                        sum += L[ j, k ] * U[ k, i ]; 
                    }
                    L[ j, i ] = ( A[ j, i ] - sum ) / U[ i, i ];
                }
            }
        }
        return std::make_tuple( L, U, permutations );
    }

    /*
        LU_solve()
        take an LU decomposition of A and use forward and back-substitution to solve Ax = b
        
        before substitution, take the RHS vector b and re-order it according to the permutation vector 
        derived from the LU decomposition

        first we use forward substitution to solve for y in the equality:

                Ly = b
        
        then we take the intermediate result y and use back substitution to solve for x in the equality:

                Ux = y

        x is the solution vector in Ax = b
    */
    template<typename T,size_t N>
    std::array<T,N> LU_solve( 
        const matrix_<T,N,N>& L,        // lower-triangular matrix
        const matrix_<T,N,N>& U,        // upper-triangular matrix 
        const std::array<T,N>& b,       // RHS vector
        const std::array<size_t,N>& ps  // permutation vector
    ) {
        std::array<T,N> x;      // final solution vector
        std::array<T,N> y;      // intermediate solution vector
        std::array<T,N> bp;     // permutated RHS

        for ( size_t k = 0; k < N; ++k ) {  // re-odrer RHS according to the permutation vector
            bp[ k ] = b[ ps[ k ] ];
        } 
        // foward substitution with L:
        // solve for y in Ly = b
        for ( size_t i = 0; i < N; ++i ) {
            T sum{};
            for ( size_t j = 0; j < i; ++j ) {
                sum += L[ i, j ] * y[ j ];
            }
            y[ i ] = ( ( T )1 / L[ i, i ] ) * ( bp[ i ] - sum ); 
        }
        // backward substitution with U:
        // solve for x in Ux = y
        for ( size_t i = N - 1; i != size_t( -1 ); --i ) {
            T sum{};
            for ( size_t j = i + 1; j < N; ++j ) {
                sum += U[ i, j ] * x[ j ];
            }
            x[ i ] = ( y[ i ] - sum ) / U[ i, i ];
        }
        return x;
    }

} // namespace lin_alg

#endif