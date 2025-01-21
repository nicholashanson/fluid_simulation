#include "collision_step.hpp"

namespace fs {

    namespace lbm {

        extern "C" {
            void collision_step_dpcpp_c( double* gd_cell_states, double* cell_states, double tau, double* result ) {
                
                const size_t num_elements = settings::grid_height * settings::grid_width * 9;

                std::vector<double> cell_states_( cell_states, cell_states + num_elements );

                std::vector<double> gd_cell_states_( gd_cell_states, gd_cell_states + num_elements );

                grid<std::vector<double>, lb_grid> gd( gd_cell_states_ );

                gd = collision_step_dpcpp( gd, cell_states_, tau );

                std::copy( gd.get_data_handle(), gd.get_data_handle() + num_elements, result );
            }

            void collision_step_dpcpp_c_opt( double* gd_cell_states, double* cell_states, double tau, double* result ) {

                const size_t num_elements = settings::grid_height * settings::grid_width * 9;

                std::vector<double> cell_states_( cell_states, cell_states + num_elements );
                
                grid<std::vector<double>, lb_grid> gd( gd_cell_states );

                gd = collision_step_dpcpp( gd, cell_states_, tau );

                std::copy( gd.get_data_handle(), gd.get_data_handle() + num_elements, result );
            }

            void collision_step_dpcpp_c_opt_0( double* gd_cell_states, double* cell_states, double tau ) {

                collision_step_dpcpp( gd_cell_states, cell_states, tau );
            }

            char** get_available_devices( int* num_devices ) {

                std::vector<std::string> device_names;

                for ( auto platform : sycl::platform::get_platforms() ) {
                    for ( auto device : platform.get_devices() ) {

                        device_names.push_back( device.get_info<sycl::info::device::name>() );

                    }
                }

                *num_devices = device_names.size();

                char** device_list = new char*[ device_names.size() ];

                for ( size_t i = 0; i < device_names.size(); ++i ) {

                    device_list[ i ] = new char[ device_names[ i ].size() + 1 ];

                    std::copy( device_names[i].begin(), device_names[ i ].end(), device_list[ i ] );

                    device_list[ i ][ device_names[ i ].size() ] = '\0';
                }

                return device_list;
            }

            void free_device_list( char** device_list, int num_devices ) {

                if ( device_list != nullptr ) {

                    for ( int i = 0; i < num_devices; ++i ) {
                        delete[] device_list[i];
                    }

                    delete[] device_list;  
                }
            }
        }
    
    } // lbm

} // fs
