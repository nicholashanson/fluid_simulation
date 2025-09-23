#include <geometry/geometry.hpp>
#include <lin_alg/lin_alg.hpp>

namespace geometry {

    // ==================
    //  Get Circumsphere 
    // ==================

    template<typename T>
    std::tuple<three_d_point<T>,T> get_circumsphere( const three_d_point<T>& p, const three_d_point<T>& q, 
                                                     const three_d_point<T>& r ) {
        const size_t rows = 3;
        const size_t cols = 3;

        auto q_p = get_difference( q, p );              // ( q - p )
        auto r_p = get_difference( r, p );              // ( r - p )
        auto norm = get_cross_product( q_p, r_p );      // ( q - p ) X ( r - p )

        auto [ a_00, a_01, a_02 ] = q_p;    // row 1 of coefficient matrix ( q - p ) 
        auto [ a_10, a_11, a_12 ] = r_p;    // row 2 of coefficient matrix ( r - p )
        auto [ a_20, a_21, a_22 ] = norm;   // row 3 of coefficient matrix ( q - p ) X ( r - p )

        // matrix of coefficients
        std::array<T,rows*cols> elements = {
            a_00, a_01, a_02,
            a_10, a_11, a_12,
            a_20, a_21, a_22,
        };

        lin_alg::matrix_<T,rows,cols> A( elements );             // construct matrix of coefficients

        std::array<T,3> b = {                           
            0.5 * get_dot_product( q_p, q_p ),          
            0.5 * get_dot_product( r_p, r_p ),          
            ( T )0,                                     
        };                                              

        auto LU_decomp = lin_alg::LU_decomposition( A );                // A = LU
        auto [ L, U, ps ] = LU_decomp.value();
        auto [ x, y, z ] = lin_alg::LU_solve( L, U, b, ps );            // solve Ax = b
        three_d_point<T> offset( x, y, z );                             // displacement between point p and circumcenter
        three_d_point<T> center = get_sum( p, offset );            		// add the offset vector to p to get the circumcenter  
        auto radius = std::sqrt( get_dot_product( offset, offset ) );   // radius of the circumsphere
        return std::make_tuple( center, radius );
    }

} // namespace geometry
