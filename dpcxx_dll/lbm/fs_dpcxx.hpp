#ifndef FS_DLL_HPP
#define FS_DLL_HPP

#define NOMINMAX
#include <Windows.h>

#ifdef DLL_EXPORTS
#define DLL_API __declspec( dllexport )
#else
#define DLL_API __declspec( dllimport )
#endif

#include <settings.hpp>
#include <string>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

using T = double;

namespace fs {

    namespace dpcxx {

        namespace lbm {

            extern "C" {

                DLL_API char** get_available_devices( int * num_devices );       

                DLL_API void free_device_list( char** device_list, int num_devices );

                DLL_API void collide_and_stream_c( T* Af_32, unsigned char* obstacle, size_t steps );

                DLL_API void collide_and_stream_tbb_c( T* A, unsigned char* obstacle, size_t steps );

                DLL_API void* init_cs_c( size_t ydim, size_t xdim, float viscosity );

                DLL_API size_t get_size_of_cs_state();
            }

        } // lbm

    } // dpcxx

} // fs

#endif