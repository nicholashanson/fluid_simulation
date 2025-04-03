#ifndef LBM_COMMON_HPP
#define LBM_COMMON_HPP

#include <cmath>
#include <cstddef>

#include <set>
#include <array>
#include <vector>

#include <iostream>

#include <settings.hpp>
#include <fs/global_aliases.hpp>

#ifndef DPCPP_COMPILER
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

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

        /*
            transformation matrix for D2Q9 MRT
        */
        extern const std::array<T, 9 * 9> M;

        /*
            inverse of transformation matrix for D2Q9 MRT
        */
        extern const std::array<T, 9 * 9> M_inv;

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

        inline std::set<std::pair<size_t, size_t>> get_airfoil_coords( const double max_camber, 
                                                                       const double camber_position, 
                                                                       const double max_thickness ) {
        
            // object to return
            std::set<std::pair<size_t, size_t>> coords;

            // chord length in pixels
            const int chord_length = fs::settings::xdim / 4;

            // x is zero-indexed
            const int start_x = fs::settings::xdim / 8 - 1;

            const size_t center_y = fs::settings::ydim / 2;

            const double max_camber_pixels = max_camber * chord_length;
            const double max_thickness_pixels = max_thickness * chord_length;

            const double camber_position_pixels = chord_length * camber_position;

            for ( size_t dx = 0; dx < chord_length; ++dx ) {

                double d_dx = static_cast<double>( dx );

                double camber = 0.0;
                if ( dx < camber_position_pixels ) {

                    camber = ( max_camber_pixels / ( camber_position_pixels * camber_position_pixels ) ) *
                             ( 2 * camber_position_pixels * d_dx - d_dx * d_dx );
                } else {
                    camber = ( max_camber_pixels / ( ( 1 - camber_position_pixels ) * ( 1 - camber_position_pixels ) ) ) *
                             ( ( 1 - 2 * camber_position_pixels ) + 2 * camber_position_pixels * d_dx - d_dx * d_dx ); 
                }

                double dy_dx = 0.0;
                if ( dx < camber_position_pixels ) {

                    dy_dx = ( 2 * max_camber_pixels / ( camber_position_pixels * camber_position_pixels ) ) *
                            ( camber_position_pixels - d_dx );
                } else {

                    dy_dx = ( 2 * max_camber_pixels / ( ( 1 - camber_position_pixels ) * ( 1 - camber_position_pixels ) ) ) *
                            ( camber_position_pixels - d_dx ); 
                }

                double theta = std::atan( dy_dx );

                double d_dx_n = d_dx / chord_length;

                double y_t = 5.0 * max_thickness_pixels * 
                             ( 0.2969 * std::sqrt( d_dx_n ) - 0.1260 * d_dx_n - 0.3516 * d_dx_n * d_dx_n + 0.2843 * d_dx_n * d_dx_n * d_dx_n - 0.1036 * d_dx_n * d_dx_n * d_dx_n * d_dx_n );
                
                size_t x = start_x + dx;

                size_t x_u = static_cast<size_t>( std::round( x - y_t * std::sin( theta ) ) );
                size_t x_l = static_cast<size_t>( std::round( x + y_t * std::sin( theta ) ) );

                size_t y_u = static_cast<size_t>( std::round( center_y - camber - y_t * std::cos( theta ) ) );
                size_t y_l = static_cast<size_t>( std::round( center_y - camber + y_t * std::cos( theta ) ) );

                coords.insert( { y_u, x_u } );
                coords.insert( { y_l, x_l } );
            }

            return coords;
        }

        inline std::set<std::pair<size_t, size_t>> get_airfoil_coords_aoa( const double max_camber, 
                                                                           const double camber_position, 
                                                                           const double max_thickness, 
                                                                           double aoa ) {

            aoa = aoa * M_PI / 180.0;

            // object to return
            std::set<std::pair<size_t, size_t>> coords;

            // chord length in pixels
            const int chord_length = fs::settings::xdim / 4;

            // x is zero-indexed
            const int start_x = fs::settings::xdim / 8 - 1;

            double d_start_x = static_cast<double>( start_x );

            const size_t center_y = fs::settings::ydim / 2;

            double d_center_y = static_cast<double>( center_y );

            const double max_camber_pixels = max_camber * chord_length;
            const double max_thickness_pixels = max_thickness * chord_length;

            const double camber_position_pixels = chord_length * camber_position;

            const double cos_aoa = std::cos( aoa );
            const double sin_aoa = std::sin( aoa ); 

            const double pivot_y = fs::settings::ydim / 2;
            const double pivot_x = fs::settings::xdim / 4;

            for ( size_t dx = 0; dx < chord_length; ++dx ) {

                double d_dx = static_cast<double>( dx );

                double camber = 0.0;
                if ( dx < camber_position_pixels ) {

                    camber = ( max_camber_pixels / ( camber_position_pixels * camber_position_pixels ) ) *
                             ( 2 * camber_position_pixels * d_dx - d_dx * d_dx );
                } else {
                    camber = ( max_camber_pixels / ( ( 1 - camber_position_pixels ) * ( 1 - camber_position_pixels ) ) ) *
                             ( ( 1 - 2 * camber_position_pixels ) + 2 * camber_position_pixels * d_dx - d_dx * d_dx ); 
                }

                double dy_dx = 0.0;
                if ( dx < camber_position_pixels ) {

                    dy_dx = ( 2 * max_camber_pixels / ( camber_position_pixels * camber_position_pixels ) ) *
                            ( camber_position_pixels - d_dx );
                } else {

                    dy_dx = ( 2 * max_camber_pixels / ( ( 1 - camber_position_pixels ) * ( 1 - camber_position_pixels ) ) ) *
                            ( camber_position_pixels - d_dx ); 
                }

                double theta = std::atan( dy_dx );

                double d_dx_n = d_dx / chord_length;

                double y_t = 5.0 * max_thickness_pixels * 
                             ( 0.2969 * std::sqrt( d_dx_n ) - 0.1260 * d_dx_n - 0.3516 * d_dx_n * d_dx_n + 0.2843 * d_dx_n * d_dx_n * d_dx_n - 0.1036 * d_dx_n * d_dx_n * d_dx_n * d_dx_n );

                double x = d_start_x + d_dx;

                double x_u = x - y_t * std::sin( theta );
                double x_l = x + y_t * std::sin( theta );

                double y_u = d_center_y - camber - y_t * std::cos( theta );
                double y_l = d_center_y - camber + y_t * std::cos( theta );

                auto rotate = [&]( double x, double y ) {
                    double x_rel = x - pivot_x;
                    double y_rel = y - pivot_y;
                    
                    size_t x_rot = static_cast<size_t>( std::round( pivot_x + x_rel * cos_aoa - y_rel * sin_aoa ) );
                    size_t y_rot = static_cast<size_t>( std::round( pivot_y + x_rel * sin_aoa + y_rel * cos_aoa ) );    

                    coords.insert( { y_rot, x_rot } );
                };

                if ( aoa != 0.0 ) {
                    rotate( x_u, y_u );
                    rotate( x_l, y_l );
                } else {

                    coords.insert( { static_cast<size_t>( std::round( y_u ) ), static_cast<size_t>( std::round( x_u ) ) } );
                    coords.insert( { static_cast<size_t>( std::round( y_l ) ), static_cast<size_t>( std::round( x_l ) ) } );
                }
            }

            return coords;
        }

        inline void fill_gaps( std::set<std::pair<size_t, size_t>>& coords ) {

            cv::Mat img = cv::Mat::zeros( fs::settings::ydim, fs::settings::xdim, CV_8UC1 );

            for ( const auto& coord : coords ) {
                
                img.at<uchar>( coord.first, coord.second ) = 255;
            }

#ifndef NDEBUG
            cv::imwrite( "before_fill_gaps.png", img );
#endif

            const int kernel_size = 3;

            cv::Mat kernel = cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( kernel_size, kernel_size ) );

            cv::dilate( img, img, kernel );

            cv::morphologyEx( img, img, cv::MORPH_CLOSE, kernel );

            coords.clear();

            for ( size_t y = 0; y < img.rows; ++y ) {
                for ( size_t x = 0; x < img.cols; ++x ) {

                    if ( img.at<uchar>( y, x ) > 0 ) {

                        coords.insert( { y, x } );
                    }
                }
            }

#ifndef NDEBUG
            cv::imwrite( "after_fill_gaps.png", img );
#endif
        }

        inline void complete_contour( std::set<std::pair<size_t, size_t>>& coords ) {

            cv::Mat img = cv::Mat::zeros( fs::settings::ydim, fs::settings::xdim, CV_8UC1 );

            for ( const auto& coord : coords ) {
                
                img.at<uchar>( coord.first, coord.second ) = 255;
            }

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours( img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
        
            cv::Mat filled = cv::Mat::zeros( img.size(), CV_8UC1 );
            cv::drawContours( filled, contours, -1, cv::Scalar(255), cv::FILLED );

            img = filled;

            coords.clear();

            for ( size_t y = 0; y < img.rows; ++y ) {
                for ( size_t x = 0; x < img.cols; ++x ) {

                    if ( img.at<uchar>( y, x ) > 0 ) {

                        coords.insert( { y, x } );
                    }
                }
            }

#ifndef NDEBUG
            cv::imwrite( "after_complete_contour.png", img );
#endif
        }

        inline std::set<std::pair<size_t, size_t>> obstacle_coords;

        inline std::vector<std::pair<size_t, size_t>> obstacle_coords_line;
#endif

        // zero-initialized vector used to initialize D2Q9 grids
        inline const std::vector<T> D2Q9_states( fs::settings::ydim * fs::settings::xdim * 9, ( T )0 );

        // zero-initialized vector used to initialize property grids
        inline const std::vector<T> property_states( fs::settings::ydim * fs::settings::xdim, ( T )0 );

#ifdef DPCPP_COMPILER

        // speed of sound
        constexpr double c_s = 0.57735;

        extern "C" {
            SYCL_EXTERNAL T calculate_f_eq( const size_t q, const T rho, const T u_x, const T u_y );
        }
#else
        constexpr double c_s = 1 / std::sqrt( 3 );

        T calculate_f_eq( const size_t q, const T rho, const T u_x, const T u_y );
#endif

        /*
            speed of sound squared
            pre-calculated for efficiency
        */
        constexpr double c_s2 = c_s * c_s;

    } // lbm

} // fs

#endif
