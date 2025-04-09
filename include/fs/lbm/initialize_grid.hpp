#ifndef LBM_INITIALIZE_GRID_HPP
#define LBM_INITIALIZE_GRID_HPP

#include <grid.hpp>

namespace fs {

    namespace lbm {

        template<typename DataStorage, typename View>
        void set_velocity( sim::grid<DataStorage, View>& gd, 
                           const size_t y, const size_t x, 
                           const double u_x, const double u_y ) {

            const double rho = 1.0;

            for ( size_t q = 0; q < 9; ++ q ) {

                double f_eq = calculate_f_eq( q, rho, u_x, u_y );

                gd.set_cell_state( f_eq, y, x, q );
            }
        }

        inline void set_velocity( T* D2Q9, 
                                  const size_t y, const size_t x,
                                  const size_t yidm, const size_t xdim,
                                  const T u_x, const T u_y ) {

            const T rho = 1.0;

            const size_t base_index = ( x + y * xdim ) * 9;

            for ( size_t q = 0; q < 9; ++ q ) {

                T f_eq = calculate_f_eq( q, rho, u_x, u_y );

                D2Q9[ base_index + q ] = f_eq;
            }
        }

        template<typename DataStorage, typename View>
        void initialize_grid( sim::grid<DataStorage, View>& gd ) {

            const size_t ydim = gd.get_dim( 0 );
            const size_t xdim = gd.get_dim( 1 );

            for ( size_t y = 0; y < ydim; ++y ) {
                for ( size_t x = 0; x < xdim; ++x ) {

                    set_velocity( gd, y, x, 0.1, 0.0 );
                }
            }
        }

        template<typename DataStorage, typename View>
        void set_boundaries( sim::grid<DataStorage, View>& gd ) {

            const size_t ydim = gd.get_dim( 0 );
            const size_t xdim = gd.get_dim( 1 );

            for ( size_t y = 0; y < ydim; ++y ) {

                set_velocity( gd, y, 0, 0.1, 0.0 );
                set_velocity( gd, y, xdim - 1, 0.1, 0.0 );
            } 
    
            for ( size_t x = 0; x < xdim; ++x ) {
                
                set_velocity( gd, 0, x, 0.1, 0.0 );
                set_velocity( gd, ydim - 1, x, 0.1, 0.0 );
            }
        }

    } // lbm

} // fs

#endif



