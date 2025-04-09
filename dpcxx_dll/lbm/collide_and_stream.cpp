#include "fs_dpcxx.hpp"
#include <fs/lbm/collide_and_stream_dpcxx.hpp>
#include <fs/lbm/stateful_collide_and_stream_dpcxx.hpp>
#include <fs/lbm/collide_and_stream_tbb.hpp>

namespace fs {

    namespace dpcxx {

        namespace lbm {

            extern "C" {

                void* init_cs_c( double* D2Q9, const unsigned char* obstacle, const size_t ydim, const size_t xdim, const double viscosity ) {

                    return init_cs( D2Q9, obstacle, ydim, xdim, viscosity );
                }

                void stateful_collide_and_stream_c( void* cs_state, const size_t steps ) {

                    stateful_collide_and_stream( cs_state, steps );
                }


                void terminate_cs_c( void* state ) {

                    terminate_cs( state );
                }

                void collide_and_stream_c( T* D2Q9, unsigned char* obstacle, size_t steps ) {

                    collide_and_stream( D2Q9, obstacle, steps ); 
                }

                void collide_and_stream_tbb_c( T* D2Q9, unsigned char* obstacle, size_t steps ) {

                    fs::lbm::collide_and_stream_tbb( D2Q9, obstacle, steps );
                } 

                void collide_and_stream_MRT_c( T* D2Q9, unsigned char* obstacle, const size_t steps ) {

                    collide_and_stream_MRT( D2Q9, obstacle, steps );
                }

                void collide_and_stream_MRT_c_( T* D2Q9, unsigned char* obstacle, const size_t steps ) {

                    collide_and_stream_MRT_( D2Q9, obstacle, steps );
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

                size_t get_size_of_cs_state() {

                    return sizeof( cs_state );
                }
            }

        } // lbm
    
    } // dpcxx

} // fs

