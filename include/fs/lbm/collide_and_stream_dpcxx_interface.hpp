#ifndef LBM_COLLIDE_AND_STREAM_DPCXX_INTERFACE_HPP
#define LBM_COLLIDE_AND_STREAM_DPCXX_INTERFACE_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include <grid.hpp>

namespace fs {

    namespace dpcxx {

        namespace lbm {

            extern "C" void collide_and_stream_tbb_c( double* gd_states, unsigned char* obstacle, size_t steps );

            extern "C" void collide_and_stream_c( double* gd_states, unsigned char* obstacle, size_t steps );

            extern "C" void* init_cs_c( size_t ydim, size_t xdim, float viscosity );

            extern "C" size_t get_size_of_cs_state();

            template<typename Array, typename MDSpan>
            void collide_and_stream_tbb( sim::grid<Array, MDSpan>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_tbb_c( gd.get_data_handle(), obstacle, steps );
            }

            template<typename Array, typename MDSpan>
            void collide_and_stream( sim::grid<Array, MDSpan>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_c( gd.get_data_handle(), obstacle, steps );
            }

        } // lbm

    } // dpcxx

} // fs

#endif