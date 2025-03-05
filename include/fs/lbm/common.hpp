#ifndef LBM_COMMON_HPP
#define LBM_COMMON_HPP

#include <cstddef>
#include <cmath>
#include <array>
#include <vector>

#include <settings.hpp>
#include <set>
#include <iostream>

#ifdef DPCPP_COMPILER
#include <sycl/sycl.hpp>
#endif

#ifndef M_PI
#define M_PI ( 4.0 * std::atan( 1.0 ) )
#endif

namespace fs {

    namespace lbm {

        // lattice velocity directions for D2Q9
        constexpr std::array<std::pair<int,int>, 9> e = { {
            {  0,  0 },   // [ 0 ] rest particle
            {  1,  0 },   // [ 1 ] right ( east )
            {  0,  1 },   // [ 2 ] up ( north )
            { -1,  0 },   // [ 3 ] left ( west ) 
            {  0, -1 },   // [ 4 ] down ( south )
            {  1,  1 },   // [ 5 ] up-right ( north-east )
            { -1,  1 },   // [ 6 ] up-left ( north-west )
            { -1, -1 },   // [ 7 ] down-left ( south-west )
            {  1, -1 }    // [ 8 ] down-right ( south-east )
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
            ( 3 ) inter-cardinal directions:  1/36
        */
        constexpr std::array<double, 9> w = {
            4.0 / 9.0,      // [ 0 ] rest-particle
            1.0 / 9.0,      // [ 1 ] right ( cardinal )
            1.0 / 9.0,      // [ 2 ] up ( cardinal )
            1.0 / 9.0,      // [ 3 ] left ( cardinal )
            1.0 / 9.0,      // [ 4 ] down ( cardinal )
            1.0 / 36.0,     // [ 5 ] up-right ( inter-cardinal )
            1.0 / 36.0,     // [ 6 ] up-left ( inter-cardinal )
            1.0 / 36.0,     // [ 7 ] down-left ( inter-cardinal )
            1.0 / 36.0      // [ 8 ] down-right ( inter-cardinal )
        };

#ifndef DPCPP_COMPILER
        inline std::set<std::pair<size_t, size_t>> get_obstacle_coords() {

            std::set<std::pair<size_t, size_t>> coords;

            int radius = settings::ydim / 8;

            const size_t center_x = settings::xdim / 4;
            const size_t center_y = settings::ydim / 2;
            
            for ( double theta = 0.0; theta < 360.0; theta += ( 45.0 / radius ) ) {

                size_t edge_cell_x = center_x + radius * std::cos( theta * M_PI / 180.0 );
                size_t edge_cell_y = center_y + radius * std::sin( theta * M_PI / 180.0 );

                coords.insert( { edge_cell_y, edge_cell_x } );
            } 

            return coords;
        }

        inline std::set<std::pair<size_t, size_t>> obstacle_coords;

        inline std::vector<std::pair<size_t, size_t>> obstacle_coords_line;
#endif

        // zero-initialized vector used to initialize D2Q9 grids
        inline const std::vector<double> D2Q9_states( fs::settings::ydim * fs::settings::xdim * 9, 0.0 );

        // zero-initialized vector used to initialize property grids
        inline const std::vector<double> property_states( fs::settings::ydim * fs::settings::xdim, 0.0 );

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
