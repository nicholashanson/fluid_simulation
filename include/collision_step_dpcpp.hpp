#ifndef LBM_COLLISION_STEP_DPCPP
#define LBM_COLLISION_STEP_DPCPP

#ifdef DPCPP_COMPILER
#include <sycl/sycl.hpp>
#endif

#include <lbm_global_data.hpp>
#include <grid.hpp>

namespace fs {

    namespace lbm {

#ifndef DPCPP_COMPILER
        template<typename Array, typename MDSpan>
        grid<Array, MDSpan> collision_step_dpcpp( grid<Array, MDSpan>& gd, Array& cell_states_, const double tau );
#else
        template<typename Array, typename MDSpan>
        grid<Array, MDSpan> collision_step_dpcpp( grid<Array, MDSpan>& gd, Array& cell_states_, const double tau ) {

            grid<Array, MDSpan> new_state( cell_states_ );

            const size_t height = new_state.get_grid_height();
            const size_t width = new_state.get_grid_width();

            typename Array::pointer gd_data = gd.get_data_handle();
            typename Array::pointer new_state_data = new_state.get_data_handle();

            sycl::queue queue_( sycl::default_selector_v );

            sycl::buffer<typename Array::value_type, 3> gd_buffer( gd_data, sycl::range<3>( height, width, 9 ) );
            sycl::buffer<typename Array::value_type, 3> ns_buffer( new_state_data, sycl::range<3>( height, width, 9 ) );

            queue_.submit( [&]( sycl::handler& h ) {

                sycl::accessor gd_acc( gd_buffer, h, sycl::read_write );
                sycl::accessor ns_acc( ns_buffer, h, sycl::read_write );

                h.parallel_for<class collision_kernel>( sycl::range<2>( height, width ), [=]( sycl::id<2> idx ) {
                    size_t i = idx[ 0 ];
                    size_t j = idx[ 1 ];

                    if ( i == 0 || j == 0 || i == height - 1 || j == width - 1 )
                        return;

                    double rho{};
                    double u_x{};
                    double u_y{};

                    for ( size_t q = 0; q < 9; ++q ) {

                        double f_q = gd_acc[ i ][ j ][ q ];

                        rho += f_q;

                        u_x += e[ q ].first * f_q;
                        u_y += e[ q ].second * f_q;
                    }

                    if ( rho < 1e-6 )
                        rho = 1e-6;

                    u_x /= rho;
                    u_y /= rho;

                    for ( size_t q = 0; q < 9; ++q ) {

                        double f_eq = calculate_f_eq( q, rho, u_x, u_y );

                        double f_prev = gd_acc[ i ][ j ][ q ];

                        double f_current = f_prev + ( 1.0 / tau ) * ( f_eq - f_prev );

                        if ( f_current < 0.0 )
                            f_current = 0.0;

                        ns_acc[ i ][ j ][ q ] = f_current;
                    }

                } );

            } );

            queue_.wait();

            return new_state;

        }
#endif

    }
}

#endif
