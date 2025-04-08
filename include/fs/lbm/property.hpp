#ifndef LBM_PROPERTY_HPP
#define LBM_PROPERTY_HPP

#include <numeric>
#include <functional>

#include <vector>

#include <grid.hpp>

#include <fs/global_aliases.hpp>
#include <fs/lbm/common.hpp>

#include <tbb/tbb.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

namespace fs {

    namespace lbm {

        inline std::pair<double,double> calculate_u_v( const std::span<double> f ) {

            std::pair<double,double> u_v{};

            for ( size_t q = 0; q < 9; ++q ) {

                u_v.first += e[ q ].first * f[ q ];
                u_v.second += e[ q ].second * f[ q ];
            }

            return u_v;
        }

        // takes a Q9 cell and returns velocity as ( u_x, u_y )
        inline std::pair<double,double> calculate_u_v_a( const std::array<double, 9>& f ) {

            std::pair<double,double> u_v{};

            for ( size_t q = 0; q < 9; ++q ) {

                u_v.first += e[ q ].first * f[ q ];
                u_v.second += e[ q ].second * f[ q ];
            }

            return u_v;
        }

        inline double calculate_u( const std::span<double> f ) {

            std::pair<double,double> u_v = calculate_u_v( f );

            return std::sqrt( u_v.first * u_v.first + u_v.second * u_v.second );
        }

        // get velocity magnitude
        inline double calculate_u_a( const std::array<double, 9>& f ) {

            std::pair<double,double> u_v = calculate_u_v_a( f );

            return std::sqrt( u_v.first * u_v.first + u_v.second * u_v.second );
        }

        inline double calculate_rho( const std::span<double> f ) {

            double rho = std::accumulate( f.begin(), f.end(), 0.0 );

            return rho;
        }

        // get density
        inline double calculate_rho_a( const std::array<double, 9>& f ) {

            double rho = std::accumulate( f.begin(), f.end(), 0.0 );

            return rho;
        }

        inline double calculate_u_x( const std::span<double> f ) {

            double u_x{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_x += e[ q ].first * f[ q ]; 

            return u_x;
        }

        inline double calculate_u_x_a( const std::array<double, 9>& f ) {

            double u_x{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_x += e[ q ].first * f[ q ]; 

            return u_x;
        }

        inline double calculate_u_y( const std::span<double> f ) {

            double u_y{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_y += e[ q ].second * f[ q ]; 

            return u_y;
        }

        inline double calculate_u_y_a( const std::array<double, 9>& f ) {

            double u_y{};

            for ( size_t q = 0; q < 9; ++q ) 
                u_y += e[ q ].second * f[ q ]; 

            return u_y;
        }

        template<typename DataStorage, typename View>
        sim::grid<std::vector<double>, property_view>
        calculate_property_v_grid( const sim::grid<DataStorage, View>& gd, const std::vector<double>& property_states, 
                                   std::function<double( std::array<double,9>& )> calculate_property ) {

            sim::grid<std::vector<double>, property_view> property_grid( property_states );
            
            const size_t ydim = gd.get_dim( 0 );
            const size_t xdim = gd.get_dim( 1 );

            for ( size_t y = 0; y < ydim; ++y ) {
                for ( size_t x = 0; x < xdim; ++x ) {

                    std::array<double, 9> cell_state = gd.get_cell_state_array( y, x );

                    double property = calculate_property( cell_state );

                    property_grid.set_cell_state( property, y, x );
                }
            }

            return property_grid;
        }

        inline void calculate_property_v( double* D2Q9_data, double* property_data,
                                          std::function<double( std::span<double> )> calculate_property ) {
            
            for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
                for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                    const size_t offset = ( x + y * fs::settings::xdim ) * 9;

                    std::span<double> cell_state( &D2Q9_data[ offset ], 9 );

                    double property = calculate_property( cell_state );

                    property_data[ x + y * fs::settings::xdim ] = property;
                }
            }
        }

        inline void calculate_property_v_tbb( double* D2Q9_data, double* property_data,
                                              std::function<double( std::span<double> )> calculate_property ) {

            tbb::parallel_for(
                tbb::blocked_range<size_t>( 0, fs::settings::ydim ),
                [&]( const tbb::blocked_range<size_t>& range ) {

                    for ( size_t y = range.begin(); y < range.end(); ++y ) {
 
                        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                            const size_t offset = ( x + y * fs::settings::xdim ) * 9;

                            // non-owning view of cell's Q9
                            std::span<double> cell_state( &D2Q9_data[ offset ], 9 );

                            double property = calculate_property( cell_state );

                            property_data[ x + y * fs::settings::xdim ] = property;
                        }
                    }
                }   
            );
        }

        inline void calculate_curl_v_tbb( double* D2Q9_data, double* curl_data ) {

            sim::grid<std::vector<double>, fs::property_view> u_x( fs::lbm::property_states );
            sim::grid<std::vector<double>, fs::property_view> u_y( fs::lbm::property_states ); 

            calculate_property_v_tbb( D2Q9_data, u_x.get_data_handle(), calculate_u_x );
            calculate_property_v_tbb( D2Q9_data, u_y.get_data_handle(), calculate_u_y );

            double* u_x_data = u_x.get_data_handle();
            double* u_y_data = u_y.get_data_handle();

            tbb::parallel_for(
                tbb::blocked_range<size_t>( 1, fs::settings::ydim - 1 ),
                    [&]( const tbb::blocked_range<size_t>& range ) {

                    for ( size_t y = range.begin(); y < range.end(); ++y ) {

                        for ( size_t x = 1; x < fs::settings::xdim - 1; ++x ) {

                            double curl = u_y_data[ ( x + 1 ) + y * fs::settings::xdim ] 
                                          - u_y_data[ ( x - 1 ) + y * fs::settings::xdim ]
                                          - u_x_data[ x + ( y + 1 ) * fs::settings::xdim ] 
                                          + u_x_data[ x + ( y - 1 ) * fs::settings::xdim ];

                            curl_data[ x + y * fs::settings::xdim ] = curl;
                        }
                    }
                }   
            );
        }

        template<typename DataStorage, typename View>
        std::tuple<double, sim::grid<std::vector<double>, property_view>>
        calculate_curl_with_max( const sim::grid<DataStorage, View>& gd, const std::vector<double>& property_states ) {

            // calculate velocities and store as grid objects
            auto u_x = calculate_property_v( gd, property_states, calculate_u_x );
            auto u_y = calculate_property_v( gd, property_states, calculate_u_y );

            // grid for holding results
            sim::grid<std::vector<double>, property_view> curl( property_states );

            // get grid dimensions
            const size_t ydim = gd.get_dim( 0 );
            const size_t xdim = gd.get_dim( 1 );

            double max_curl{};

            // don't process edge cells
            for ( size_t y = 1; y < ydim - 1; ++y ) {
                for ( size_t x = 1; x < xdim - 1; ++x ) {

                    // use velocity values to calculate cul
                    double curl_ = u_y.get_cell_state( y, x + 1 ) - u_y.get_cell_state( y, x - 1 )
                                   - u_x.get_cell_state( y + 1, x ) + u_x.get_cell_state( y - 1, x );

                    // new maximum curl found
                    if ( curl_ > max_curl )
                        max_curl = curl_;

                    curl.set_cell_state( curl_, y, x );
                }
            }

            return std::make_tuple( max_curl, std::move( curl ) );
        }

        /*
            takes a D2Q9 grid and outputs a grid of a property values defined by calculate_property
            e.g. fs::lbm::calculate_property_v_with_max( ..., fs::lbm::calculate_rho ) will return a 
            two-element tuple containing a grid object of density values and the max density as a real
            number.
        */
        template<typename DataStorage, typename View>
        std::tuple<double, sim::grid<std::vector<double>, property_view>>
        calculate_property_v_with_max( const sim::grid<DataStorage, View>& gd, const std::vector<double>& property_states, 
                                       std::function<double( std::array<double,9>& )> calculate_property ) {

            // grid to return
            sim::grid<std::vector<double>, property_view> property_grid( property_states );

            // get grid dimensions
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

        inline void calculate_property_v_with_max_tbb( double* D2Q9_data,
                                                       double* property_data,
                                                       double& max_property,
                                                       std::function<double( std::span<double> )> calculate_property ) {

            tbb::combinable<double> local_max_property( []{ return -std::numeric_limits<double>::infinity(); } );

            tbb::parallel_for(
                tbb::blocked_range<size_t>( 0, fs::settings::ydim ),
                [&]( const tbb::blocked_range<size_t>& range ) {

                    double& thread_max = local_max_property.local();

                    for ( size_t y = range.begin(); y < range.end(); ++y ) {
    
                        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                            const size_t offset = ( x + y * fs::settings::xdim ) * 9;

                            // non-owning view of cell's Q9
                            std::span<double> cell_state( &D2Q9_data[ offset ], 9 );

                            double property = calculate_property( cell_state );

                            thread_max = std::max( thread_max, property );

                            property_data[ x + y * fs::settings::xdim ] = property;
                        }
                    }
                }   
            );

            max_property = local_max_property.combine( []( double a, double b ) { return std::max( a, b ); } );
        }

        inline void calculate_property_v_with_min_max_tbb( double* D2Q9_data,
                                                           double* property_data,
                                                           double& min_property,
                                                           double& max_property,
                                                           std::function<double( const std::span<double> )> calculate_property ) {

            tbb::combinable<double> local_min_property( []{ return std::numeric_limits<double>::infinity(); } );
            tbb::combinable<double> local_max_property( []{ return -std::numeric_limits<double>::infinity(); } );
            
            tbb::parallel_for(
                tbb::blocked_range<size_t>( 0, fs::settings::ydim ),
                    [&]( const tbb::blocked_range<size_t>& range ) {

                    double& thread_min = local_min_property.local();
                    double& thread_max = local_max_property.local();

                    for ( size_t y = range.begin(); y < range.end(); ++y ) {

                        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                            const size_t offset = ( x + y * fs::settings::xdim ) * 9;

                            // non-owning view of cell's Q9
                            std::span<double> cell_state( &D2Q9_data[ offset ], 9 );

                            double property = calculate_property( cell_state );

                            thread_min = std::min( thread_min, property );
                            thread_max = std::max( thread_max, property );

                            property_data[ x + y * fs::settings::xdim ] = property;
                        }
                    }
                }   
            );

            min_property = local_min_property.combine( []( double a, double b ) { return std::min( a, b ); } );
            max_property = local_max_property.combine( []( double a, double b ) { return std::max( a, b ); } );
        }

    }
}

#endif
