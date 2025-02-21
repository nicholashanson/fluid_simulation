#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <vector>
#include <numeric>
#include <functional>

#include <grid.hpp>
#include <fs/global_aliases.hpp>
#include <fs/lbm/common.hpp>

namespace fs {

    namespace lbm {

        // takes a Q9 cell and returns velocity as ( u_x, u_y )
        inline std::pair<double,double> calculate_u_v( const std::array<double, 9>& f ) {

            std::pair<double,double> u_v{};

            for ( size_t q = 0; q < 9; ++q ) {

                u_v.first += e[ q ].first * f[ q ];
                u_v.second += e[ q ].second * f[ q ];
            }

            return u_v;
        }

        // get velocity magnitude
        inline double calculate_u( std::array<double, 9>& f) {

            std::pair<double,double> u_v = calculate_u_v( f );

            return std::sqrt( u_v.first * u_v.first + u_v.second * u_v.second );
        }

        // get density
        inline double calculate_rho( std::array<double, 9>& f ) {

            double rho = std::accumulate( f.begin(), f.end(), 0.0 );

            return rho;
        }

        inline double calculate_u_x( std::array<double, 9>& f ) {

            double u_x{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_x += e[ q ].first * f[ q ]; 

            return u_x;
        }

        inline double calculate_u_y( std::array<double, 9>& f ) {

            double u_y{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_y += e[ q ].second * f[ q ]; 

            return u_y;
        }

        // takes a D2Q9 grid and outputs a grid of a certain property defined by calculate_property
        // e.g. fs::lbm::calculate_property_v_with_max( ..., fs::lbm::calculate_rho ) will return a 
        // a gird object of densities and the max density as a tuple
        template<typename DataStorage, typename View>
        std::tuple<double, sim::grid<std::vector<double>, property_view>>
        calculate_property_v_with_max( const sim::grid<DataStorage, View>& gd, const std::vector<double>& property_states, 
                                       std::function<double( std::array<double,9>& )> calculate_property ) {

            sim::grid<std::vector<double>, property_view> property_grid( property_states );
            
            const size_t ydim = gd.get_dim( 0 );
            const size_t xdim = gd.get_dim( 1 );

            double max_property{};

            for ( size_t y = 0; y < ydim; ++y ) {
                for ( size_t x = 0; x < xdim; ++x ) {

                    std::array<double, 9> cell_state = gd.get_cell_state_array( y, x );

                    double property = calculate_property( cell_state );

                    if ( property > max_property )
                        max_property = property;

                    property_grid.set_cell_state( property, y, x );
                }
            }

            return std::make_tuple( max_property, std::move( property_grid ) );
        }

    }
}

#endif