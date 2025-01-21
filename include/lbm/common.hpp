#ifndef LBM_COMMON_HPP
#define LBM_COMMON_HPP

#include <cstddef>
#include <cmath>
#include <array>
#include <vector>

#ifdef DPCPP_COMPILER
#include <sycl/sycl.hpp>
#endif

namespace fs {

    namespace lbm {

        /*
            lattice velocity directions for D2Q9

            the directions are listed in clockwise order. this
            is so we can stream in a certain general direction
            by selecting a range from the array.
        */
        constexpr std::array<std::pair<int,int>, 9> e = { {
            {  0,  0 },   // [ 0 ] rest particle
            {  0, -1 },   // [ 1 ] up
            {  1, -1 },   // [ 2 ] up-right
            {  1,  0 },   // [ 3 ] right
            {  1,  1 },   // [ 4 ] down-right
            {  0,  1 },   // [ 5 ] down
            { -1,  1 },   // [ 6 ] down-left
            { -1,  0 },   // [ 7 ] left
            { -1, -1 }    // [ 8 ] up-left
        } };

        /*
            look-up table used during bounce-back to flip the direction
            of streaming
        */
        constexpr std::array<size_t, 9> opposite_q = {
            0,  // [ 0 ] opposite( rest particle )  => rest particle
            5,  // [ 1 ] opposite( up )             => down
            6,  // [ 2 ] opposite( up-right )       => down-left
            7,  // [ 3 ] opposite( right )          => left
            8,  // [ 4 ] opposite( down-right )     => up-left
            1,  // [ 5 ] opposite( down )           => up
            2,  // [ 6 ] opposite( down-left )      => up-right
            3,  // [ 7 ] opposite( left )           => right
            4   // [ 8 ] opposite( up-left )        => down-right
        };

        /*
            weights:
            ( 1 ) rest particle:              4/9
            ( 2 ) cardinal directions:        1/9
        ``  ( 3 ) inter-cardinal directions:  1/36
        */
        constexpr std::array<double, 9> w = {
            4.0 / 9.0,      // [ 0 ] rest-partile
            1.0 / 9.0,      // [ 1 ] up ( cardinal )
            1.0 / 36.0,     // [ 2 ] up-right ( inter-cardinal )
            1.0 / 9.0,      // [ 3 ] right ( cardinal )
            1.0 / 36.0,     // [ 4 ] down-right ( inter-cardinal )
            1.0 / 9.0,      // [ 5 ] down ( cardinal )
            1.0 / 36.0,     // [ 6 ] down-left ( inter-cardinal )
            1.0 / 9.0,      // [ 7 ] left ( cardinal )
            1.0 / 36.0      // [ 8 ] up-left ( inter-cardinal )
        };

        extern std::vector<std::pair<size_t, size_t>> obstacle_coords;

        // zero-initialized vector used to initialize D2Q9 grids
        extern std::vector<double> cell_states_;

        // zero-initialized vector used to initialize grid of vector magnitudes
        extern std::vector<double> vec_mag_states;

#ifdef DPCPP_COMPILER
        // speed of sound
        constexpr double c_s = 0.57735;

        extern "C" {
            SYCL_EXTERNAL double calculate_f_eq( size_t q, double rho, double u_x, double u_y );
        }
#else
        constexpr double c_s = 1 / std::sqrt( 3 );

        double calculate_f_eq( size_t q, double rho, double u_x, double u_y );
#endif

        /*
            speed of sound squared
            pre-calculated for efficiency
        */
        constexpr double c_s2 = c_s * c_s;

    } // lbm

} // fs

#endif
