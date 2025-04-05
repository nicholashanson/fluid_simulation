#ifndef LBM_COLLIDE_AND_STREAM_DPCXX_HPP
#define LBM_COLLIDE_AND_STREAM_DPCXX_HPP

#include <sycl/sycl.hpp>

#include <optional>

#include <fs/fs.hpp>
#include <fs/lbm/dpcxx_common.hpp>

#include <settings.hpp>

namespace fs {

    namespace dpcxx {

        namespace lbm {

            void check_for_extension() {

                return;
            }

            typedef struct cs_state cs_state;

            /*
                this is a handle to the state of the simulation on the GPU so we don't need to copy back and
                forth between the host and the GPU each time we call collide and stream.
            */
            struct cs_state {

                sycl::queue* gpu_queue;
                T* D2Q9;
                T* d_D2Q9;
                T* d_D2Q9_n;
                unsigned char * d_obstacle;
                size_t ydim;
                size_t xdim;
                size_t vec_len;
                T omega;
            };

            void* init_cs( double* D2Q9, const unsigned char* obstacle, 
                           const size_t ydim, const size_t xdim, const T viscosity ) {

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

                state->gpu_queue = new sycl::queue( *gpu );

                state->D2Q9 = D2Q9;
                state->d_D2Q9 = sycl::malloc_device<T>( state->vec_len * 9, *( state->gpu_queue ) );
                state->d_D2Q9_n = sycl::malloc_device<T>( state->vec_len * 9, *( state->gpu_queue ) );
                state->d_obstacle = sycl::malloc_device<unsigned char>( state->vec_len, *( state->gpu_queue ) );

                state->gpu_queue->memcpy( state->d_obstacle, obstacle, state->vec_len * sizeof( unsigned char ) );
                state->gpu_queue->memcpy( state->d_D2Q9, D2Q9, state->vec_len * 9 * sizeof( T ) );
                state->gpu_queue->wait();
                state->gpu_queue->memcpy( state->d_D2Q9_n, state->d_D2Q9, state->vec_len * 9 * sizeof( T ) );
                state->gpu_queue->wait();
                
                return state;
            }

            void collide( sycl::queue& gpu_queue, T* d_D2Q9, const size_t vec_len, const T omega ) {
                
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
            
                        local_Q9[ local_Q9_offset ] += omega * ( ( 4.0 / 9.0 ) * rho * ( 1 - u_215 ) - local_Q9[ local_Q9_offset ] );
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
            }

            void stream( sycl::queue& gpu_queue, T* d_D2Q9, T* d_D2Q9_n, const size_t ydim, const size_t xdim ) {

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
            }

            void bounce_back( sycl::queue& gpu_queue, fs::lbm::T* d_D2Q9_n, unsigned char* d_obstacle, const size_t ydim, const size_t xdim ) {                
                    
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
            }

            void stateful_collide_and_stream_inline( void* state, const size_t steps ) {

                cs_state* cs = ( cs_state* )state;

                T omega = cs->omega;

                T* d_D2Q9 = cs->d_D2Q9;
                T* d_D2Q9_n = cs->d_D2Q9_n;

                const size_t xdim = cs->xdim;
                const size_t ydim = cs->ydim;

                const T u_x = 0.1;
                const T u_y = 0.0;
                const T rho = 1.0;

                const T ux_3 = 3 * u_x;
                const T uy_3 = 3 * u_y;
                const T ux_2 = u_x * u_x;
                const T uy_2 = u_y * u_y;
                const T ux_uy_2 = 2 * u_x * u_y;
                const T u_2 = ux_2 + uy_2;
                const T u_2_15 = 1.5 * u_2;
                
                const T f_0 = ( 4.0 / 9.0 )  * rho * ( 1                                         - u_2_15 );
                const T f_1 = ( 1.0 / 9.0 )  * rho * ( 1 + ux_3        + 4.5 * ux_2              - u_2_15 );
                const T f_2 = ( 1.0 / 9.0 )  * rho * ( 1 + uy_3        + 4.5 * uy_2              - u_2_15 );
                const T f_3 = ( 1.0 / 9.0 )  * rho * ( 1 - ux_3        + 4.5 * ux_2              - u_2_15 );
                const T f_4 = ( 1.0 / 9.0 )  * rho * ( 1 - uy_3        + 4.5 * uy_2              - u_2_15 );
                const T f_5 = ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + ux_uy_2 ) - u_2_15 );
                const T f_6 = ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - ux_uy_2 ) - u_2_15 );
                const T f_7 = ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + ux_uy_2 ) - u_2_15 );
                const T f_8 = ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - ux_uy_2 ) - u_2_15 );

                for ( size_t z = 0; z < steps; ++z ) {
            
                    cs->gpu_queue->submit( [&]( sycl::handler& h ) {
            
                        sycl::local_accessor<T, 1> local_Q9( 9 * 32, h );
            
                        h.parallel_for( sycl::nd_range<1>{ { cs->vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                            
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

                    cs->gpu_queue->wait();

                    cs->gpu_queue->submit( [&]( sycl::handler& h ) {
            
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

                    cs->gpu_queue->wait();

                    unsigned char* d_obstacle = cs->d_obstacle;

                    cs->gpu_queue->submit( [&]( sycl::handler& h ) {
            
                        h.parallel_for( sycl::nd_range<2>{ { cs->ydim, cs->xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
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

                    cs->gpu_queue->wait();
            
                    std::swap( cs->d_D2Q9, cs->d_D2Q9_n ); 
                }

                /*
                cs->gpu_queue->submit( [&]( sycl::handler& h ) {

                    h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
                        const size_t y = item.get_global_id( 0 );
                        const size_t x = item.get_global_id( 1 );

                        const size_t base_index = ( x + y * xdim ) * 9;

                        bool is_edge_cell = ( y == 0 || x == 0 || y == ydim - 1 || x == xdim - 1 );

                        if ( is_edge_cell ) {

                            d_D2Q9[ base_index ] = f_0;
                            d_D2Q9[ base_index + 1 ] = f_1;
                            d_D2Q9[ base_index + 2 ] = f_2;
                            d_D2Q9[ base_index + 3 ] = f_3;
                            d_D2Q9[ base_index + 4 ] = f_4;
                            d_D2Q9[ base_index + 5 ] = f_5;
                            d_D2Q9[ base_index + 6 ] = f_6;
                            d_D2Q9[ base_index + 7 ] = f_7;
                            d_D2Q9[ base_index + 8 ] = f_8;
                        }              
                    });
                });
                */

                cs->gpu_queue->wait();
            
                cs->gpu_queue->memcpy( cs->D2Q9, cs->d_D2Q9, cs->vec_len * 9 * sizeof( T ) );

                cs->gpu_queue->wait();
            }

            void set_boundaries_( sycl::queue& gpu_queue, T* d_D2Q9, const size_t xdim, const size_t ydim ) {

                const T u_x = 0.1;
                const T u_y = 0.0;
                const T rho = 1.0;

                const T ux_3 = 3 * u_x;
                const T uy_3 = 3 * u_y;
                const T ux_2 = u_x * u_x;
                const T uy_2 = u_y * u_y;
                const T ux_uy_2 = 2 * u_x * u_y;
                const T u_2 = ux_2 + uy_2;
                const T u_2_15 = 1.5 * u_2;
                
                const T f_0 = ( 4.0 / 9.0 )  * rho * ( 1                                         - u_2_15 );
                const T f_1 = ( 1.0 / 9.0 )  * rho * ( 1 + ux_3        + 4.5 * ux_2              - u_2_15 );
                const T f_2 = ( 1.0 / 9.0 )  * rho * ( 1 + uy_3        + 4.5 * uy_2              - u_2_15 );
                const T f_3 = ( 1.0 / 9.0 )  * rho * ( 1 - ux_3        + 4.5 * ux_2              - u_2_15 );
                const T f_4 = ( 1.0 / 9.0 )  * rho * ( 1 - uy_3        + 4.5 * uy_2              - u_2_15 );
                const T f_5 = ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + ux_uy_2 ) - u_2_15 );
                const T f_6 = ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - ux_uy_2 ) - u_2_15 );
                const T f_7 = ( 1.0 / 36.0 ) * rho * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + ux_uy_2 ) - u_2_15 );
                const T f_8 = ( 1.0 / 36.0 ) * rho * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - ux_uy_2 ) - u_2_15 );

                gpu_queue.submit( [&]( sycl::handler& h ) {

                    h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
                        const size_t y = item.get_global_id( 0 );
                        const size_t x = item.get_global_id( 1 );

                        const size_t base_index = ( x + y * xdim ) * 9;

                        bool is_edge_cell = ( y == 0 || x == 0 || y == ydim - 1 || x == xdim - 1 );

                        if ( is_edge_cell ) {

                            d_D2Q9[ base_index ] = f_0;
                            d_D2Q9[ base_index + 1 ] = f_1;
                            d_D2Q9[ base_index + 2 ] = f_2;
                            d_D2Q9[ base_index + 3 ] = f_3;
                            d_D2Q9[ base_index + 4 ] = f_4;
                            d_D2Q9[ base_index + 5 ] = f_5;
                            d_D2Q9[ base_index + 6 ] = f_6;
                            d_D2Q9[ base_index + 7 ] = f_7;
                            d_D2Q9[ base_index + 8 ] = f_8;
                        }              
                    });
                });

                gpu_queue.wait();
            }

            void set_boundaries( sycl::queue& gpu_queue, T* d_D2Q9, const size_t ydim, const size_t xdim ) {

                gpu_queue.submit( [&]( sycl::handler& h ) {

                    h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                            
                        const size_t y = item.get_global_id( 0 );
                        const size_t x = item.get_global_id( 1 );

                        const size_t base_index = ( x + y * xdim ) * 9;

                        bool is_edge_cell = ( y == 0 || x == 0 || y == ydim - 1 || x == xdim - 1 );

                        if ( is_edge_cell ) {

                            fs::lbm::set_velocity( d_D2Q9, y, x, ydim, xdim, 0.1, 0.0 );
                        }              
                    });
                });

                gpu_queue.wait();
            }

            void set_grid_boundaries( void* state ) {

                cs_state* cs = ( cs_state* )state;

                set_boundaries( *( cs->gpu_queue ), cs->d_D2Q9, cs->ydim, cs->xdim );
            
                cs->gpu_queue->memcpy( cs->D2Q9, cs->d_D2Q9, cs->vec_len * 9 * sizeof( T ) );

                cs->gpu_queue->wait();
            }

            void stateful_collide_and_stream( void* state, const size_t steps ) {

                cs_state* cs = ( cs_state* )state;

                set_boundaries( *( cs->gpu_queue ), cs->d_D2Q9, cs->ydim, cs->xdim );

                for ( size_t z = 0; z < steps; ++z ) {
            
                    collide( *( cs->gpu_queue ), cs->d_D2Q9, cs->vec_len, cs->omega );

                    stream( *( cs->gpu_queue ), cs->d_D2Q9, cs->d_D2Q9_n, cs->ydim, cs->xdim );

                    bounce_back( *( cs->gpu_queue ), cs->d_D2Q9_n, cs->d_obstacle, cs->ydim, cs->xdim ); 
 
                    std::swap( cs->d_D2Q9, cs->d_D2Q9_n ); 
                }
            
                cs->gpu_queue->memcpy( cs->D2Q9, cs->d_D2Q9, cs->vec_len * 9 * sizeof( T ) );

                cs->gpu_queue->wait();
            }
            
            void terminate_cs( void* state ) {

                cs_state* cs = ( cs_state* )state;

                sycl::free( cs->d_D2Q9, *( cs->gpu_queue ) );
                sycl::free( cs->d_D2Q9_n, *( cs->gpu_queue ) );
                sycl::free( cs->d_obstacle, *( cs->gpu_queue ) );

                delete cs->gpu_queue;

                std::free( cs );
            }

            void collide_and_stream_inline( T* D2Q9, unsigned char* obstacle, size_t steps ) {

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
            
                            local_Q9[ local_Q9_offset ] += omega * ( ( 4.0 / 9.0 ) * rho * ( 1 - u_215 ) - local_Q9[ local_Q9_offset ] );
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
            
                    collide( gpu_queue, d_D2Q9, vec_len, omega );
            
                    stream( gpu_queue, d_D2Q9, d_D2Q9_n, ydim, xdim );

                    bounce_back( gpu_queue, d_D2Q9_n, d_obstacle, ydim, xdim );
            
                    std::swap( d_D2Q9, d_D2Q9_n ); 
                }
            
                gpu_queue.memcpy( D2Q9, d_D2Q9, vec_len * 9 * sizeof( T ) );

                gpu_queue.wait();
            
                sycl::free( d_D2Q9, gpu_queue );
                sycl::free( d_D2Q9_n, gpu_queue );
                sycl::free( d_obstacle, gpu_queue );
            }

            void collide_MRT( sycl::queue& gpu_queue, fs::lbm::T* d_D2Q9, const size_t vec_len, fs::lbm::T omega ) {
                
                /*
                std::array<fs::lbm::T, 9> s = { 0.0, 
                                                1.6, 
                                                1.8, 
                                                0.0,
                                                8.0 * ( 2.0 - omega ) / ( 8.0 - omega ),
                                                0.0,
                                                8.0 * ( 2.0 - omega ) / ( 8.0 - omega ),
                                                omega, 
                                                omega };
                */

                gpu_queue.submit( [&]( sycl::handler& h ) {
    
                    // transformation matrix
                    sycl::local_accessor<fs::lbm::T, 1> local_M( 9 * 9 , h );
    
                    // transformation matrix inverse
                    sycl::local_accessor<fs::lbm::T, 1> local_M_inv( 9 * 9, h );
    
                    h.parallel_for( sycl::nd_range<1>( { vec_len }, { 32 } ), [=]( sycl::nd_item<1> item ) {
                        
                        /*
                        const size_t i = item.get_group().get_group_id();
                        const size_t k = item.get_local_id();
    
                        const size_t d_D2Q9_offset = ( i * 32 + k ) * 9;

                        
                            first thead in the group copies transformation matrix from host memory to 
                            local memory.

                        if ( k == 0 ) {
    
                            for ( size_t i = 0; i < 9 * 9; ++i ) {
    
                                    local_M[ i ] = d_M[ i ];
                                    local_M_inv[ i ] = d_M_inv[ i ];
                            } 
                        }
    
                        item.barrier( sycl::access::fence_space::local_space );

                        m[ 0 ] =
                        */
                    });
                });
    
                gpu_queue.wait();
            }

            void collide_MRT_( sycl::queue& gpu_queue, fs::lbm::T* d_D2Q9, fs::lbm::T* d_M, fs::lbm::T* d_M_inv,
                               const size_t vec_len, fs::lbm::T omega ) {
    
                gpu_queue.submit( [&]( sycl::handler& h ) {

                    sycl::local_accessor<T, 1> local_Q9( 9 * 32, h );
    
                    // transformation matrix
                    sycl::local_accessor<fs::lbm::T, 1> local_M( 9 * 9, h );
    
                    // transformation matrix inverse
                    sycl::local_accessor<fs::lbm::T, 1> local_M_inv( 9 * 9, h );
    
                    h.parallel_for( sycl::nd_range<1>( { vec_len }, { 32 } ), [=]( sycl::nd_item<1> item ) {

                        const size_t i = item.get_group().get_group_id();
                        const size_t k = item.get_local_id();
            
                        const size_t d_D2Q9_offset = ( i * 32 + k ) * 9; 
                        const size_t local_Q9_offset = k * 9;

                        for ( size_t z = 0; z < 9; ++z ) 
                            local_Q9[ local_Q9_offset + z ] = d_D2Q9[ d_D2Q9_offset + z ];
            
                        fs::lbm::T rho{};
                        
                        for ( size_t z = 0; z < 9; ++ z ) 
                            rho += local_Q9[ local_Q9_offset + z ]; 
        
                        fs::lbm::T u_x{};
                        u_x = ( local_Q9[ local_Q9_offset + 1 ] + local_Q9[ local_Q9_offset + 5 ] +
                                local_Q9[ local_Q9_offset + 8 ] - local_Q9[ local_Q9_offset + 3 ] -
                                local_Q9[ local_Q9_offset + 6 ] - local_Q9[ local_Q9_offset + 7 ] ) / rho;
        
                        fs::lbm::T u_y{};
                        u_y = ( local_Q9[ local_Q9_offset + 2 ] + local_Q9[ local_Q9_offset + 5 ] +
                                local_Q9[ local_Q9_offset + 6 ] - local_Q9[ local_Q9_offset + 4 ] -
                                local_Q9[ local_Q9_offset + 7 ] - local_Q9[ local_Q9_offset + 8 ] ) / rho;
                    
                        /*
                            first nine threads in the group copy transformation matrix from global memory to 
                            local memory.
                        */
                        if ( k < 9 ) {

                            const size_t offset = k * 9;

                            for ( size_t i = offset; i < offset + 9; ++i ) {

                                local_M[ i ] = d_M[ i ]; 
                                local_M_inv[ i ] = d_M_inv[ i ];
                            }
                        }

                        item.barrier( sycl::access::fence_space::local_space );

                        fs::lbm::T m_eq[ 9 ] = { 0.0 };

                        fs::lbm::T p_x = rho * u_x;
                        fs::lbm::T p_y = rho * u_y;

                        m_eq[ 0 ] = rho;
                        m_eq[ 1 ] = -2.0 * rho + 3.0 * ( p_x * p_x + p_y * p_y );
                        m_eq[ 2 ] = rho - 3.0 * ( p_x * p_x + p_y * p_y );
                        m_eq[ 3 ] = p_x;
                        m_eq[ 4 ] = -p_x;
                        m_eq[ 5 ] = p_y;
                        m_eq[ 6 ] = -p_y;
                        m_eq[ 7 ] = ( p_x * p_x - p_y * p_y );
                        m_eq[ 8 ] = p_x * p_y;

                        fs::lbm::T m[ 9 ] = { 0.0 };
                        
                        for ( size_t i = 0; i < 9; ++i ) {
                            for ( size_t j = 0; j < 9; ++j ) {

                                m[ i ] += local_Q9[ local_Q9_offset + j ] * local_M[ j + i * 9 ];
                            }
                        }

                        {
                            fs::lbm::T s[ 9 ] = { 0.0, 
                                                  1.6, 
                                                  1.8, 
                                                  0.0,
                                                  8.0 * ( 2.0 - omega ) / ( 8.0 - omega ),
                                                  0.0,
                                                  8.0 * ( 2.0 - omega ) / ( 8.0 - omega ),
                                                  omega, 
                                                  omega };

                            for ( size_t i = 0; i < 9; ++i ) {
                                m[ i ] -= s[ i ] * ( m[ i ] - m_eq[ i ] );
                            }
                        }

                        for ( size_t i = 0; i < 9; ++i ) {

                            local_Q9[ local_Q9_offset + i ] = 0;

                            for ( size_t j = 0; j < 9; ++j ) {

                                local_Q9[ local_Q9_offset + i ] += local_M_inv[ j + i * 9 ] * m[ j ];
                            }

                            d_D2Q9[ d_D2Q9_offset + i ] = local_Q9[ local_Q9_offset + i ];
                        }                
                    });
                });
    
                gpu_queue.wait();
            }

            void collide_and_stream_MRT( fs::lbm::T* D2Q9, unsigned char* obstacle, const size_t steps ) {

                // const fs::lbm::T viscosity = 0.005;
            
                // const fs::lbm::T omega = 1 / ( 3 * viscosity + 0.5 );
    
                const size_t ydim = fs::settings::ydim;
                const size_t xdim = fs::settings::xdim;
                
                const size_t vec_len = ydim * xdim;
    
                std::optional<sycl::device> gpu = get_gpu();
            
                sycl::queue gpu_queue( *gpu );
            
                fs::lbm::T* d_D2Q9 = sycl::malloc_device<fs::lbm::T>( vec_len * 9, gpu_queue );
                // fs::lbm::T* d_D2Q9_n = sycl::malloc_device<fs::lbm::T>( vec_len * 9, gpu_queue );
                fs::lbm::T* d_M = sycl::malloc_device<fs::lbm::T>( 9 * 9, gpu_queue );
                fs::lbm::T* d_M_inv = sycl::malloc_device<fs::lbm::T>( 9 * 9, gpu_queue );
 
                gpu_queue.memcpy( d_M, fs::lbm::M.data(), 9 * 9 * sizeof( fs::lbm::T ) );
                    
                gpu_queue.memcpy( d_M_inv, fs::lbm::M_inv.data(), 9 * 9 * sizeof( fs::lbm::T ) ); 
    
                // unsigned char* d_obstacle = sycl::malloc_device<unsigned char>( vec_len, gpu_queue );
    
                // gpu_queue.memcpy( d_obstacle, obstacle, vec_len * sizeof( unsigned char ) );

                gpu_queue.memcpy( d_D2Q9, D2Q9, vec_len * 9 * sizeof( fs::lbm::T ) );
                gpu_queue.wait();
                
                // gpu_queue.memcpy( d_D2Q9_n, d_D2Q9, vec_len * 9 * sizeof( fs::lbm::T ) );
                // gpu_queue.wait();
            
                for ( size_t z = 0; z < steps; ++z ) {
            
                    // collide_MRT( gpu_queue, d_D2Q9, vec_len, omega );
                }

                sycl::free( d_D2Q9, gpu_queue );
                sycl::free( d_M, gpu_queue );
                sycl::free( d_M_inv, gpu_queue );
                
                // sycl::free( d_D2Q9_n, gpu_queue );
                // sycl::free( d_obstacle, gpu_queue );
            }

            void collide_and_stream_MRT_( fs::lbm::T* D2Q9, unsigned char* obstacle, const size_t steps ) {

                const fs::lbm::T viscosity = 0.005;
            
                const fs::lbm::T omega = 1 / ( 3 * viscosity + 0.5 );
    
                const size_t ydim = fs::settings::ydim;
                const size_t xdim = fs::settings::xdim;
                
                const size_t vec_len = ydim * xdim;
    
                std::optional<sycl::device> gpu = get_gpu();
            
                sycl::queue gpu_queue( *gpu );
            
                fs::lbm::T* d_D2Q9 = sycl::malloc_device<fs::lbm::T>( vec_len * 9, gpu_queue );
                fs::lbm::T* d_D2Q9_n = sycl::malloc_device<fs::lbm::T>( vec_len * 9, gpu_queue );

                fs::lbm::T* d_M = sycl::malloc_device<fs::lbm::T>( 9 * 9, gpu_queue );
                fs::lbm::T* d_M_inv = sycl::malloc_device<fs::lbm::T>( 9 * 9, gpu_queue );
 
                gpu_queue.memcpy( d_M, fs::lbm::M.data(), 9 * 9 * sizeof( fs::lbm::T ) );
                gpu_queue.memcpy( d_M_inv, fs::lbm::M_inv.data(), 9 * 9 * sizeof( fs::lbm::T ) ); 
    
                unsigned char* d_obstacle = sycl::malloc_device<unsigned char>( vec_len, gpu_queue );
    
                gpu_queue.memcpy( d_obstacle, obstacle, vec_len * sizeof( unsigned char ) );

                gpu_queue.memcpy( d_D2Q9, D2Q9, vec_len * 9 * sizeof( fs::lbm::T ) );
                gpu_queue.wait();
                
                gpu_queue.memcpy( d_D2Q9_n, d_D2Q9, vec_len * 9 * sizeof( fs::lbm::T ) );
                gpu_queue.wait();
            
                for ( size_t z = 0; z < steps; ++z ) {
            
                    collide_MRT_( gpu_queue, d_D2Q9, d_M, d_M_inv, vec_len, omega );

                    stream( gpu_queue, d_D2Q9, d_D2Q9_n, ydim, xdim );

                    bounce_back( gpu_queue, d_D2Q9_n, d_obstacle, ydim, xdim );
            
                    std::swap( d_D2Q9, d_D2Q9_n ); 
                }
            
                gpu_queue.memcpy( D2Q9, d_D2Q9, vec_len * 9 * sizeof( T ) );

                sycl::free( d_D2Q9, gpu_queue );
                sycl::free( d_M, gpu_queue );
                sycl::free( d_M_inv, gpu_queue );
                
                sycl::free( d_D2Q9_n, gpu_queue );
                sycl::free( d_obstacle, gpu_queue );
            }
        
        } // lbm

    } // dpcxx

} // fs

#endif

