#include <fs/lbm/common.hpp>
#include <fs/global_aliases.hpp>
#include <settings.hpp>

namespace fs {

    namespace lbm {

        /*
            transformation matrix for D2Q9 MRT
        */
        const std::array<T, 9 * 9> M = {
             1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,
            -4.0,  2.0, -1.0,  2.0, -1.0,  2.0, -1.0,  2.0, -1.0,
             4.0,  1.0, -2.0,  1.0, -2.0,  1.0, -2.0,  1.0, -2.0, 
             0.0, -1.0, -1.0, -1.0,  0.0,  1.0,  1.0,  1.0,  0.0,
             0.0, -1.0,  2.0, -1.0,  0.0,  1.0, -2.0,  1.0,  0.0,
             0.0,  1.0,  0.0, -1.0, -1.0, -1.0,  0.0,  1.0,  1.0,
             0.0,  1.0,  0.0, -1.0,  2.0, -1.0,  0.0,  1.0, -2.0,
             0.0,  0.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0, -1.0,
             0.0, -1.0,  0.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0 
        };

        /*
            inverse of transformation matrix for D2Q9 MRT
        */
        const std::array<T, 9 * 9> M_inv = {
            ( 1.0 / 9.0 ), -( 1.0 /  9.0 ),  ( 1.0 /  9.0 ),            0.0,             0.0,            0.0,             0.0,            0.0,             0.0,
            ( 1.0 / 9.0 ),  ( 1.0 / 18.0 ),  ( 1.0 / 36.0 ), -( 1.0 / 6.0 ), -( 1.0 / 12.0 ),  ( 1.0 / 6.0 ),  ( 1.0 / 12.0 ),            0.0,  -( 1.0 / 4.0 ),
            ( 1.0 / 9.0 ), -( 1.0 / 36.0 ), -( 1.0 / 18.0 ), -( 1.0 / 6.0 ),  ( 1.0 /  6.0 ),            0.0,             0.0,  ( 1.0 / 4.0 ),             0.0,
            ( 1.0 / 9.0 ),  ( 1.0 / 18.0 ),  ( 1.0 / 36.0 ), -( 1.0 / 6.0 ), -( 1.0 / 12.0 ), -( 1.0 / 6.0 ), -( 1.0 / 12.0 ),            0.0,   ( 1.0 / 4.0 ),
            ( 1.0 / 9.0 ), -( 1.0 / 36.0 ), -( 1.0 / 18.0 ),            0.0,             0.0, -( 1.0 / 6.0 ),  ( 1.0 /  6.0 ), -( 1.0 / 4.0 ),             0.0,
            ( 1.0 / 9.0 ),  ( 1.0 / 18.0 ),  ( 1.0 / 36.0 ),  ( 1.0 / 6.0 ),  ( 1.0 / 12.0 ), -( 1.0 / 6.0 ), -( 1.0 / 12.0 ),            0.0,  -( 1.0 / 4.0 ),
            ( 1.0 / 9.0 ), -( 1.0 / 36.0 ), -( 1.0 / 18.0 ),  ( 1.0 / 6.0 ), -( 1.0 /  6.0 ),            0.0,             0.0,  ( 1.0 / 4.0 ),             0.0,
            ( 1.0 / 9.0 ),  ( 1.0 / 18.0 ),  ( 1.0 / 36.0 ),  ( 1.0 / 6.0 ),  ( 1.0 / 12.0 ),  ( 1.0 / 6.0 ),  ( 1.0 / 12.0 ),            0.0,   ( 1.0 / 4.0 ),
            ( 1.0 / 9.0 ), -( 1.0 / 36.0 ), -( 1.0 / 18.0 ),            0.0,             0.0,  ( 1.0 / 6.0 ),  -( 1.0 / 6.0 ), -( 1.0 / 4.0 ),             0.0
        };

        // equilibrium distribution function
        T calculate_f_eq( const size_t q, const T rho, const T u_x, const T u_y ) {

            T e_dot_u = e[ q ].first * u_x + e[ q ].second * u_y;

            T u_dot_u = u_x * u_x + u_y * u_y;          // |u|^2

            T f_eq = w[ q ] * rho * ( ( T )1 + e_dot_u / c_s2 + ( e_dot_u * e_dot_u ) / ( 2 * c_s2 * c_s2 ) - u_dot_u / ( 2 * c_s2 ) );

            return f_eq;
        }

    } // lbm

} // fs