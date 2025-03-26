#ifndef FS_DLL_HPP
#define FS_DLL_HPP

#define NOMINMAX
#include <Windows.h>

#ifdef DLL_EXPORTS
#define DLL_API __declspec( dllexport )
#else
#define DLL_API __declspec( dllimport )
#endif

#include <fs/global_aliases.hpp>
#include <settings.hpp>
#include <string>

#include <opencv.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

using T = fs::lbm::T;

namespace fs {

    namespace dpcxx {

        namespace lbm {

            extern "C" {

                DLL_API char** get_available_devices( int * num_devices );       

                DLL_API void free_device_list( char** device_list, int num_devices );

                DLL_API void grid_to_vertex_data_c( float* vertex_data, double* property_data );

                DLL_API void grid_to_vertex_data_cv_c( float* vertex_data, double* property_data, int colormap );

                DLL_API void collide_and_stream_c( T* D2Q9, unsigned char* obstacle, size_t steps );

                DLL_API void collide_and_stream_tbb_c( T* D2Q9, unsigned char* obstacle, size_t steps );

                DLL_API void* init_cs_c( double* D2Q9, unsigned char* obstacle, size_t ydim, size_t xdim, float viscosity );

                DLL_API void stateful_collide_and_stream_c( void* cs_state, const size_t steps );

                DLL_API void terminate_cs_c( void* state );

                DLL_API void collide_and_stream_MRT_c( T* D2Q9, unsigned char* obstacle, const size_t steps );

                DLL_API size_t get_size_of_cs_state();
            }

        } // lbm

    } // dpcxx

} // fs

#endif