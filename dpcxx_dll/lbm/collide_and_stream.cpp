#include "fs_dpcxx.hpp"
#include <fs/lbm/collide_and_stream_dpcxx.hpp>

namespace fs {

    namespace dpcxx {

        namespace lbm {

            extern "C" {

                void* init_cs_c( size_t ydim, size_t xdim, float viscosity ) {

                    return init_cs( ydim, xdim, viscosity );
                }

                void collide_and_stream_c( T* Af_32, unsigned char* obstacle, size_t steps ) {

                    std::cout << "inside collide and stream" << std::endl;

                    collide_and_stream( Af_32, obstacle, steps ); 
                }

                void collide_and_stream_tbb_c( T* A, unsigned char* obstacle, size_t steps ) {

                    collide_and_stream_tbb( A, obstacle, steps );
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

