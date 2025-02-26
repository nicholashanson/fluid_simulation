#ifndef LBM_COLLIDE_AND_STREAM_DPCXX_HPP
#define LBM_COLLIDE_AND_STREAM_DPCXX_HPP

#include <sycl/sycl.hpp>

#include <optional>

#include <fs/lbm/dpcxx_common.hpp>
#include <fs/fs.hpp>
#include <settings.hpp>

namespace fs {

    namespace dpcxx {

        namespace lbm {

            typedef struct cs_state cs_state;

            struct cs_state {

                sycl::queue gpu_queue;
                T* d_D2Q9;
                T* d_D2Q9_n;
                unsigned char * d_obstacle;
                size_t ydim;
                size_t xdim;
                size_t vec_len;
                T omega;
            };

            void* init_cs( double* D2Q9, unsigned char* obstacle, size_t ydim, size_t xdim, T viscosity ) {

                cs_state* state = ( cs_state* )std::malloc( sizeof( cs_state ) );

                std::optional<sycl::device> gpu = get_gpu();

                if ( !gpu ) {
                    std::cout << "GPU not found!" << std::endl;
                    return nullptr;
                }

                state->ydim = ydim;
                state->xdim = xdim;
                state->vec_len = xdim * ydim;
                state->omega = 1.0 / ( 3.0 * viscosity + 0.5 );
                state->gpu_queue = sycl::queue( *gpu );

                state->d_D2Q9 = sycl::malloc_device<T>( state->vec_len * 9, state->gpu_queue );
                state->d_D2Q9_n = sycl::malloc_device<T>( state->vec_len * 9, state->gpu_queue );
                state->d_obstacle = sycl::malloc_device<unsigned char>( state->vec_len, state->gpu_queue );

                state->gpu_queue.memcpy( state->d_obstacle, obstacle, state->vec_len * sizeof( unsigned char ) );
                state->gpu_queue.memcpy( state->d_D2Q9, D2Q9, state->vec_len * 9 * sizeof( T ) );
                state->gpu_queue.wait();
                state->gpu_queue.memcpy( state->d_D2Q9_n, state->d_D2Q9, state->vec_len * 9 * sizeof( T ) );
                state->gpu_queue.wait();
                
                return state;
            }
            
            void terminate_cs( void* state ) {

                cs_state* cs = ( cs_state* )state;

                sycl::free( cs->d_D2Q9, cs->gpu_queue );
                sycl::free( cs->d_D2Q9_n, cs->gpu_queue );
                sycl::free( cs->d_obstacle, cs->gpu_queue );
            }

            void collide_and_stream( T* D2Q9, unsigned char* obstacle, size_t steps ) {

                const T viscosity = 0.005;
            
                const T omega = 1 / ( 3 * viscosity + 0.5 );

                const size_t ydim = fs::settings::ydim;
                const size_t xdim = fs::settings::xdim;
                const size_t vec_len = ydim * xdim;

                std::optional<sycl::device> gpu = get_gpu();
            
                sycl::queue gpu_queue( *gpu );
            
                T* d_D2Q9 = sycl::malloc_device<T>( vec_len * 9, gpu_queue );
                T* d_D2Q9_n = sycl::malloc_device<T>( vec_len * 9, gpu_queue ); 

                unsigned char* d_obstacle = sycl::malloc_device<unsigned char>( vec_len, gpu_queue );

                gpu_queue.memcpy( d_obstacle, obstacle, vec_len * sizeof( unsigned char ) );
                gpu_queue.memcpy( d_D2Q9, D2Q9, vec_len * 9 * sizeof( T ) );
                gpu_queue.wait();
                gpu_queue.memcpy( d_D2Q9_n, d_D2Q9, vec_len * 9 * sizeof( T ) );
                gpu_queue.wait();
            
                for ( size_t z = 0; z < steps; ++z ) {
            
                    gpu_queue.submit( [&]( sycl::handler& h ) {
            
                        sycl::local_accessor<T, 1> local_Q9( 9 * 32, h );
            
                        h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                            
                            const size_t i = item.get_group().get_group_id();
                            const size_t k = item.get_local_id();
            
                            const size_t d_D2Q9_offset = ( i * 32 + k ) * 9; 
                            const size_t local_Q9_offset = k * 9;
            
                            for ( size_t z = 0; z < 9; ++z ) 
                                local_Q9[ local_Q9_offset + z ] = d_D2Q9[ d_D2Q9_offset + z ];
            
                            T rho{};
                            
                            for ( size_t z = 0; z < 9; ++ z ) 
                                rho += local_Q9[ local_Q9_offset + z ]; 
            
                            T u_x{};
                            u_x = ( local_Q9[ local_Q9_offset + 1 ] + local_Q9[ local_Q9_offset + 5 ] +
                                    local_Q9[ local_Q9_offset + 8 ] - local_Q9[ local_Q9_offset + 3 ] -
                                    local_Q9[ local_Q9_offset + 6 ] - local_Q9[ local_Q9_offset + 7 ] ) / rho;
            
                            T u_y{};
                            u_y = ( local_Q9[ local_Q9_offset + 2 ] + local_Q9[ local_Q9_offset + 5 ] +
                                    local_Q9[ local_Q9_offset + 6 ] - local_Q9[ local_Q9_offset + 4 ] -
                                    local_Q9[ local_Q9_offset + 7 ] - local_Q9[ local_Q9_offset + 8 ] ) / rho;
            
                            const T ux_2 = u_x * u_x;
                            const T uy_2 = u_y * u_y;
                            const T u_215 = 1.5 * ( ( ux_2 ) + ( uy_2 ) );
                            const T ux_3 = 3 * u_x;
                            const T uy_3 = 3 * u_y;
            
                            local_Q9[ local_Q9_offset ] += omega * 
                                                           ( ( 4.0 / 9.0 ) * rho * ( 1 - u_215 ) 
                                                           - local_Q9[ local_Q9_offset ] );
                            d_D2Q9[ d_D2Q9_offset ] = local_Q9[ local_Q9_offset ];
            
                            local_Q9[ local_Q9_offset + 1 ] += omega * ( ( 1.0 / 9.0 ) * rho * ( 1 + ux_3 + 4.5 * ux_2 - u_215 ) - local_Q9[ local_Q9_offset + 1 ] );
                            d_D2Q9[ d_D2Q9_offset + 1 ] = local_Q9[ local_Q9_offset + 1 ];
            
                            local_Q9[ local_Q9_offset + 2 ] += omega * ( ( 1.0 / 9.0 ) * rho * ( 1 + uy_3 + 4.5 * uy_2 - u_215 ) - local_Q9[ local_Q9_offset + 2 ] );
                            d_D2Q9[ d_D2Q9_offset + 2 ] = local_Q9[ local_Q9_offset + 2 ];
            
                            local_Q9[ local_Q9_offset + 3 ] += omega * (  ( 1.0 / 9.0 ) * rho * ( 1 - ux_3 + 4.5 * ux_2 - u_215 )  - local_Q9[ local_Q9_offset + 3 ] );
                            d_D2Q9[ d_D2Q9_offset + 3 ] = local_Q9[ local_Q9_offset + 3 ]; 
            
                            local_Q9[ local_Q9_offset + 4 ] += omega * ( ( 1.0 / 9.0 ) * rho * ( 1 - uy_3 + 4.5 * uy_2 - u_215 )  - local_Q9[ local_Q9_offset + 4 ] );
                            d_D2Q9[ d_D2Q9_offset + 4 ] = local_Q9[ local_Q9_offset + 4 ];
            
                            const T uxuy_2 = 2 * u_x * u_y;
            
                            const T u_2 = ux_2 + uy_2;
            
                            local_Q9[ local_Q9_offset + 5 ] += omega * ( ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - local_Q9[ local_Q9_offset + 5 ] ); 
                            d_D2Q9[ d_D2Q9_offset + 5 ] = local_Q9[ local_Q9_offset + 5 ];
            
                            local_Q9[ local_Q9_offset + 6 ] += omega * ( ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - local_Q9[ local_Q9_offset + 6 ] ); 
                            d_D2Q9[ d_D2Q9_offset + 6 ] = local_Q9[ local_Q9_offset + 6 ];
            
                            local_Q9[ local_Q9_offset + 7 ] += omega * ( ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - local_Q9[ local_Q9_offset + 7 ] ); 
                            d_D2Q9[ d_D2Q9_offset + 7 ] = local_Q9[ local_Q9_offset + 7 ];
            
                            local_Q9[ local_Q9_offset + 8 ] += omega * ( ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - local_Q9[ local_Q9_offset + 8 ] ); 
                            d_D2Q9[ d_D2Q9_offset + 8 ] = local_Q9[ local_Q9_offset + 8 ];
                        });
                    });

                    gpu_queue.wait();
            
                    gpu_queue.submit( [&]( sycl::handler& h ) {
            
                        h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
                            const size_t y = item.get_global_id( 0 );

                            const size_t x = item.get_global_id( 1 );
            
                            const size_t base_index = ( x + y * xdim ) * 9;
            
                            bool is_boundary = ( y == 0 || x == 0 || y == ydim - 1 || x == xdim - 1 );
            
                            if ( !is_boundary ) {
                                
                                // rest particle frequency-distribution directly copied to the new grid
                                d_D2Q9_n[ base_index ] = d_D2Q9[ base_index ];

                                // get east from west cell
                                d_D2Q9_n[ base_index + 1 ] = d_D2Q9[ ( ( x - 1 ) + y * xdim ) * 9 + 1 ];

                                // get south from north cell 
                                d_D2Q9_n[ base_index + 4 ] = d_D2Q9[ ( x + ( y + 1 ) * xdim ) * 9 + 4 ];
                                
                                // get west from east cell
                                d_D2Q9_n[ base_index + 3 ] = d_D2Q9[ ( ( x + 1 ) + y * xdim ) * 9 + 3 ];

                                // get north from south cell
                                d_D2Q9_n[ base_index + 2 ] = d_D2Q9[ ( x + ( y - 1 ) * xdim ) * 9 + 2 ];
                                
                                // get south-east from north-west cell
                                d_D2Q9_n[ base_index + 8 ] = d_D2Q9[ ( ( x - 1 ) + ( y + 1 ) * xdim ) * 9 + 8 ];

                                // get south-west from north-east cell
                                d_D2Q9_n[ base_index + 7 ] = d_D2Q9[ ( ( x + 1 ) + ( y + 1 ) * xdim ) * 9 + 7 ];
                                
                                // get north-west from south-east cell
                                d_D2Q9_n[ base_index + 6 ] = d_D2Q9[ ( ( x + 1 ) + ( y - 1 ) * xdim ) * 9 + 6 ];
                                
                                // get north-east from south-west cell
                                d_D2Q9_n[ base_index + 5 ] = d_D2Q9[ ( ( x - 1 ) + ( y - 1 ) * xdim ) * 9 + 5 ];
                            }
                        });
                    });

                    gpu_queue.wait();

                    gpu_queue.submit( [&]( sycl::handler& h ) {
            
                        h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
                            const size_t y = item.get_global_id( 0 );

                            const size_t x = item.get_global_id( 1 );
            
                            const size_t index = ( x + y * xdim ) * 9;
            
                            if ( d_obstacle[ x + y * xdim ] ) {
                                
                                d_D2Q9_n[ ( x + 1 + y * xdim ) * 9 + 1 ] = d_D2Q9_n[ index + 3 ];
                                
                                d_D2Q9_n[ ( x - 1 + y * xdim ) * 9 + 3 ] = d_D2Q9_n[ index + 1 ];
                                
                                d_D2Q9_n[ ( x + ( y + 1 ) * xdim ) * 9 + 2 ] = d_D2Q9_n[ index + 4 ];
                                
                                d_D2Q9_n[ ( x + ( y - 1 ) * xdim ) * 9 + 4 ] = d_D2Q9_n[ index + 2 ];
                                
                                d_D2Q9_n[ ( x + 1 + ( y + 1 ) * xdim ) * 9 + 5 ] = d_D2Q9_n[ index + 7 ];
                                
                                d_D2Q9_n[ ( x - 1 + ( y + 1 ) * xdim ) * 9 + 6 ] = d_D2Q9_n[ index + 8 ];
                                
                                d_D2Q9_n[ ( x + 1 + ( y - 1 ) * xdim ) * 9 + 8 ] = d_D2Q9_n[ index + 6 ];
                                
                                d_D2Q9_n[ ( x - 1 + ( y - 1 ) * xdim ) * 9 + 7 ] = d_D2Q9_n[ index + 5 ];
                            }  
                        });
                    });

                    gpu_queue.wait();
            
                    std::swap( d_D2Q9, d_D2Q9_n ); 
                }
            
                gpu_queue.memcpy( D2Q9, d_D2Q9, vec_len * 9 * sizeof( T ) );
            
                sycl::free( d_D2Q9, gpu_queue );
                sycl::free( d_D2Q9_n, gpu_queue );
                sycl::free( d_obstacle, gpu_queue );
            }

            void collide_and_stream_tbb( double* A, unsigned char* obstacle, size_t steps ) {

                const T viscosity = 0.005;
            
                const T omega = 1 / ( 3 * viscosity + 0.5 );

                const size_t ydim = fs::settings::ydim;

                const size_t xdim = fs::settings::xdim;

                const size_t vec_len = ydim * xdim;

                T* A_n = ( T* )std::malloc( ydim * xdim * 9 * sizeof( T ) );

                T* A_n_initial = A_n;

                std::memcpy( A_n, A, ydim * xdim * 9 * sizeof( T ) );
    
                for ( size_t z = 0; z < steps; ++z ) {

                    // collide
            
                    tbb::parallel_for( tbb::blocked_range<size_t>( 0, vec_len ),
                        [&]( const tbb::blocked_range<size_t>& r ) {
            
                            for ( size_t i = r.begin(); i < r.end(); ++i ) {
                    
                                const size_t index_offset = i * 9;
            
                                T rho_{};
                                    
                                for ( size_t k = 0; k < 9; ++k )
                                    rho_ += A[ index_offset + k ];

                                T ux_{};
            
                                ux_ = ( A[ index_offset + 1 ] + A[ index_offset + 5 ] +
                                        A[ index_offset + 8 ] - A[ index_offset + 3 ] -
                                        A[ index_offset + 6 ] - A[ index_offset + 7 ] ) / rho_;
    
                                T uy_{};
            
                                uy_ = ( A[ index_offset + 2 ] + A[ index_offset + 5 ] +
                                        A[ index_offset + 6 ] - A[ index_offset + 4 ] -
                                        A[ index_offset + 7 ] - A[ index_offset + 8 ] ) / rho_;
        
                                const T ux_2 = ux_ * ux_;
        
                                const T uy_2 = uy_ * uy_;
        
                                const T u_215 = 1.5 * ( ( ux_2 ) + ( uy_2 ) );
        
                                const T ux_3 = ux_ * 3;
        
                                const T uy_3 = uy_ * 3;
        
                                A[ index_offset ] += omega * ( ( 4.0 / 9.0 ) * rho_ * ( 1 - u_215 ) - A[ index_offset ] );
        
                                A[ index_offset + 1 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 + ux_3 + 4.5 * ux_2 - u_215 ) - A[ index_offset + 1 ] );
        
                                A[ index_offset + 2 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 + uy_3 + 4.5 * uy_2 - u_215 ) - A[ index_offset + 2 ] );
        
                                A[ index_offset + 3 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 - ux_3 + 4.5 * ux_2 - u_215 ) - A[ index_offset + 3 ] );    
                            
                                A[ index_offset + 4 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 - uy_3 + 4.5 * uy_2 - u_215 )  - A[ index_offset + 4 ] ); 
                            
                                const T uxuy_2 = 2 * ux_ * uy_;
        
                                const T u_2 = ux_2 + uy_2;
        
                                A[ index_offset + 5 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - A[ index_offset + 5 ] ); 
        
                                A[ index_offset + 6 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - A[ index_offset + 6 ] );
        
                                A[ index_offset + 7 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - A[ index_offset + 7 ] ); 
                        
                                A[ index_offset + 8 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - A[ index_offset + 8 ] ); 
                            }
                        }
                    );

                    // stream
    
                    tbb::parallel_for( tbb::blocked_range<size_t>( 1, ydim - 1 ),
                        [&]( const tbb::blocked_range<size_t>& r ) {
        
                            for ( size_t y = r.begin(); y < r.end(); ++y ) {
        
                                for ( size_t x = 1; x < xdim - 1; ++x ) {
        
                                    const size_t base_index = ( x + y * xdim ) * 9;

                                    A_n[ base_index ] = A[ base_index ];
        
                                    A_n[ base_index + 1 ] = A[ ( ( x - 1 ) + y * xdim ) * 9 + 1 ];

                                    A_n[ base_index + 4 ] = A[ ( x + ( y + 1 ) * xdim ) * 9 + 4 ];

                                    A_n[ base_index + 3 ] = A[ ( ( x + 1 ) + y * xdim ) * 9 + 3 ];

                                    A_n[ base_index + 2 ] = A[ ( x + ( y - 1 ) * xdim ) * 9 + 2 ];

                                    A_n[ base_index + 8 ] = A[ ( ( x - 1 ) + ( y + 1 ) * xdim ) * 9 + 8 ];

                                    A_n[ base_index + 7 ] = A[ ( ( x + 1 ) + ( y + 1 ) * xdim ) * 9 + 7 ];

                                    A_n[ base_index + 6 ] = A[ ( ( x + 1 ) + ( y - 1 ) * xdim ) * 9 + 6 ];

                                    A_n[ base_index + 5 ] = A[ ( ( x - 1 ) + ( y - 1 ) * xdim ) * 9 + 5 ];
                                }
                            }
                        }
                    );

                    for ( size_t y = 1; y < ydim; ++y ) {

                        for ( size_t x = 1; x < xdim - 1; ++x ) {

                            if ( obstacle[ x + y * xdim ] ) {

                                size_t index = ( x + y * xdim ) * 9;
                    
                                A_n[ ( x + 1 + y * xdim ) * 9 + 1 ] = A_n[ index + 3 ];

                                A_n[ ( x - 1 + y * xdim ) * 9 + 3 ] = A_n[ index + 1 ];

                                A_n[ ( x + ( y + 1 ) * xdim ) * 9 + 2 ] = A_n[ index + 4 ];

                                A_n[ ( x + ( y - 1 ) * xdim ) * 9 + 4 ] = A_n[ index + 2 ];

                                A_n[ ( x + 1 + ( y + 1 ) * xdim ) * 9 + 5 ] = A_n[ index + 7 ];

                                A_n[ ( x - 1 + ( y + 1 ) * xdim ) * 9 + 6 ] = A_n[ index + 8 ];

                                A_n[ ( x + 1 + ( y - 1 ) * xdim ) * 9 + 8 ] = A_n[ index + 6 ];

                                A_n[ ( x - 1 + ( y - 1 ) * xdim ) * 9 + 7 ] = A_n[ index + 5 ];
                            }
                        }  
                    }

                    std::swap( A, A_n );
                }

                if ( A_n != A_n_initial ) {

                    std::memcpy( A_n, A, ydim * xdim * 9 * sizeof( T ) );

                    std::swap( A, A_n );
                }

                std::free( A_n );
            } 
        
        } // lbm

    } // dpcxx

} // fs

#endif
