#ifndef LBM_COLLISION_STEP_DLL_HPP
#define LBM_COLLISION_STEP_DLL_HPP

#include <lbm/collision_step_dpcpp.hpp>
#include <global_aliases.hpp>

#define NOMINMAX
#include <Windows.h>

#ifdef DLL_EXPORTS
#define DLL_API __declspec( dllexport )
#else
#define DLL_API __declspec( dllimport )
#endif

#include <settings.hpp>

namespace fs {

    namespace lbm {

        extern "C" {
            DLL_API void collision_step_dpcpp_c( double* gd_cell_states, double* cell_states, double tau, double* result );

            DLL_API void collision_step_dpcpp_c_opt( double* gd_cell_states, double* cell_states, double tau, double* result );

            DLL_API void collision_step_dpcpp_c_opt_0( double* gd_cell_states, double* cell_states, double tau );

            DLL_API char** get_available_devices( int * num_devices );       

            DLL_API void free_device_list( char** device_list, int num_devices );
        }

    } // lbm

} // fs

#endif

