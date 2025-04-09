/*
    this interface just provides a thin layer to the DPC++ DLL for a cleaner and more consistent API
*/
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

            extern "C" void collide_and_stream_MRT_c( double* gd_states, unsigned char* obstacle, const size_t steps );

            extern "C" void collide_and_stream_MRT_c_( double* gd_states, unsigned char* obstacle, const size_t steps );

            extern "C" void* init_cs_c( double* D2Q9, const unsigned char* obstacle, const size_t ydim, const size_t xdim, const double viscosity );

            extern "C" void stateful_collide_and_stream_c( void* cs_state, const size_t );

            extern "C" void terminate_cs_c( void* state );

            extern "C" size_t get_size_of_cs_state();

            template<typename DataStorage, typename View>
            void* init_cs( sim::grid<DataStorage, View>& gd, std::vector<unsigned char>& obstacle, double viscosity ) {

                return init_cs_c( gd.get_data_handle(), obstacle.data(), gd.get_dim( 0), gd.get_dim( 1 ), viscosity );
            }

            template<typename DataStorage, typename View>
            void collide_and_stream_tbb( sim::grid<DataStorage, View>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_tbb_c( gd.get_data_handle(), obstacle, steps );
            }

            template<typename DataStorage, typename View>
            void collide_and_stream( sim::grid<DataStorage, View>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_c( gd.get_data_handle(), obstacle, steps );
            }

            template<typename DataStorage, typename View>
            void collide_and_stream_MRT( sim::grid<DataStorage, View>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_MRT_c( gd.get_data_handle(), obstacle, steps );
            }

            template<typename DataStorage, typename View>
            void collide_and_stream_MRT_( sim::grid<DataStorage, View>& gd, unsigned char* obstacle, size_t steps ) {

                collide_and_stream_MRT_c_( gd.get_data_handle(), obstacle, steps );
            }

        } // lbm

    } // dpcxx

} // fs

#endif
