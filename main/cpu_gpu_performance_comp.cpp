#define PERFORMANCE_PROFILE_MAIN
#define PROFILE_PRECISION 3
#include <performance_profile.hpp>
#include <sycl/sycl.hpp>
#include <vector>
#include <iostream>
#include <optional>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

using T = float;

const size_t vec_len = 32 * 30 * 32 * 90;

const size_t ydim = 32 * 90;
const size_t xdim = 32 * 30;

std::optional<sycl::device> get_gpu() {
    for ( const auto& platform : sycl::platform::get_platforms() ) {
        for ( const auto& device : platform.get_devices( sycl::info::device_type::gpu ) ) {

            if ( device.get_info<sycl::info::device::name>().find( "NVIDIA" ) != std::string::npos ) { 

                return device;
            }
        }
    }
    return std::nullopt;
}

struct f : profiler::base_fixture<10,1> {
    f() = default;
};

void set_equil( const sycl::device& device, std::vector<T>& A_f32 ) {

    sycl::queue gpu_queue( device );

    T* d_A = sycl::malloc_device<T>( vec_len * 9, gpu_queue );

    // Copy data to GPU
    gpu_queue.memcpy( d_A, A_f32.data(), vec_len * 9 * sizeof( T ) );
    
    gpu_queue.wait();  // Ensure data is copied before computation

    const double u_x = 0.1;
    const double u_y = 0.0;

    const double ux_3 = 3 * u_x;
    const double uy_3 = 3 * u_y;
    const double ux_2 = u_x * u_x;
    const double uy_2 = u_y * u_y;
    const double uxuy2 = 2 * u_x * u_y;
    const double u_2 = ux_2 + uy_2;
    const double u_215 = 1.5 * u_2;

    // Launch a kernel with one thread per element
    for ( size_t z = 0; z < 20; ++ z ) {
        gpu_queue.submit([&]( sycl::handler &h ) {

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                
                const size_t i = item.get_global_id();

                const size_t base_index = i * 9;

                d_A[ base_index ] = ( 4.0 / 9.0 ) * ( 1 - u_215 );
                d_A[ base_index + 1 ] = ( 1.0 / 9.0 ) * ( 1 + ux_3 + 4.5 * ux_2 - u_215 );
                d_A[ base_index + 2 ] = ( 1.0 / 9.0 ) * ( 1 + uy_3 + 4.5 * uy_2 - u_215 );
                d_A[ base_index + 3 ] = ( 1.0 / 9.0 ) * ( 1 - ux_3 + 4.5 * ux_2 - u_215 );
                d_A[ base_index + 4 ] = ( 1.0 / 9.0 ) * ( 1 - uy_3 + 4.5 * uy_2 - u_215 );
                d_A[ base_index + 5 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                d_A[ base_index + 6 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
                d_A[ base_index + 7 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                d_A[ base_index + 8 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
            });
        });
    }

    gpu_queue.wait();  // Ensure computation is finished

    // Copy result back to host
    gpu_queue.memcpy( A_f32.data(), d_A, vec_len * 9 * sizeof( T ) ).wait();

    // Free device memory
    sycl::free( d_A, gpu_queue );
}

void set_equil_mk( const sycl::device& device, std::vector<T>& A_f32 ) {

    sycl::queue gpu_queue( device );

    T* d_A = sycl::malloc_device<T>( vec_len * 9, gpu_queue );

    // Copy data to GPU
    gpu_queue.memcpy( d_A, A_f32.data(), vec_len * 9 * sizeof( T ) );
    
    gpu_queue.wait();  // Ensure data is copied before computation

    const double u_x = 0.1;
    const double u_y = 0.0;

    const double ux_3 = 3 * u_x;
    const double uy_3 = 3 * u_y;
    const double ux_2 = u_x * u_x;
    const double uy_2 = u_y * u_y;
    const double uxuy2 = 2 * u_x * u_y;
    const double u_2 = ux_2 + uy_2;
    const double u_215 = 1.5 * u_2;

    // Launch a kernel with one thread per element
    for ( size_t z = 0; z < 20; ++ z ) {
        gpu_queue.submit([&]( sycl::handler &h ) {

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                
                const size_t i = item.get_global_id();

                const size_t base_index = i * 9;

                d_A[ base_index ] = ( 4.0 / 9.0 ) * ( 1 - u_215 );
                d_A[ base_index + 1 ] = ( 1.0 / 9.0 ) * ( 1 + ux_3 + 4.5 * ux_2 - u_215 );
                d_A[ base_index + 2 ] = ( 1.0 / 9.0 ) * ( 1 + uy_3 + 4.5 * uy_2 - u_215 );
                d_A[ base_index + 3 ] = ( 1.0 / 9.0 ) * ( 1 - ux_3 + 4.5 * ux_2 - u_215 );
                d_A[ base_index + 4 ] = ( 1.0 / 9.0 ) * ( 1 - uy_3 + 4.5 * uy_2 - u_215 );
            });
        });

        gpu_queue.submit( [&]( sycl::handler & h) {

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                
                const size_t i = item.get_global_id();

                const size_t base_index = i * 9;

                d_A[ base_index + 5 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                d_A[ base_index + 6 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
                d_A[ base_index + 7 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                d_A[ base_index + 8 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
            });
        });
    }

    gpu_queue.wait();  // Ensure computation is finished

    // Copy result back to host
    gpu_queue.memcpy( A_f32.data(), d_A, vec_len * 9 * sizeof( T ) ).wait();

    // Free device memory
    sycl::free( d_A, gpu_queue );
}

void collide_and_stream( const sycl::device& device, std::vector<T>& A_f32 ) {

    const T viscosity = 0.005;

    const T omega = 1 / ( 3 * viscosity + 0.5 );

    sycl::queue gpu_queue( device );

    T* d_A = sycl::malloc_device<T>( vec_len * 9, gpu_queue );

    T* d_A_n = sycl::malloc_device<T>( vec_len * 9, gpu_queue ); 

    gpu_queue.memcpy( d_A, A_f32.data(), vec_len * 9 * sizeof( T ) );

    gpu_queue.wait();

    for ( size_t z = 0; z < 40; ++z ) {

        gpu_queue.submit( [&]( sycl::handler& h ) {

            sycl::local_accessor<T, 1> local_f( 9 * 32, h );

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                
                const size_t i = item.get_group().get_group_id();
                const size_t k = item.get_local_id();

                const size_t property_index = item.get_global_id();
                const size_t A_offset = ( i * 32 + k ) * 9;
                const size_t f_offset = k * 9;

                for ( size_t z = 0; z < 9; ++z ) 
                    local_f[ f_offset + z ] = d_A[ A_offset + z ];
                //item.barrier( sycl::access::fence_space::local_space );

                T rho{};
                
                for ( size_t z = 0; z < 9; ++ z ) {
                    rho += local_f[ f_offset + z ]; 
                }

                // d_rho[ property_index ] = rho;

                T u_x{};

                u_x = ( local_f[ f_offset + 1 ] + local_f[ f_offset + 5 ] +
                        local_f[ f_offset + 8 ] - local_f[ f_offset + 3 ] -
                        local_f[ f_offset + 6 ] - local_f[ f_offset + 7 ] ) / rho;

                // d_ux[ property_index ] = u_x;

                T u_y{};

                u_y = ( local_f[ f_offset + 2 ] + local_f[ f_offset + 5 ] +
                        local_f[ f_offset + 6 ] - local_f[ f_offset + 4 ] -
                        local_f[ f_offset + 7 ] - local_f[ f_offset + 8 ] ) / rho;

                // d_uy[ property_index ] = u_y;

                const T ux_2 = u_x * u_x;

                const T uy_2 = u_y * u_y;

                const T u_215 = 1.5f * ( ( ux_2 ) + ( uy_2 ) );

                const T ux_3 = 3 * u_x;

                const T uy_3 = 3 * u_y;

                local_f[ f_offset ] += ( 4.0f / 9.0f ) * rho * ( ( 1 - u_215 ) - local_f[ f_offset ] );
                d_A[ A_offset ] = local_f[ f_offset ];

                local_f[ f_offset + 1 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - ux_3 + 4.5 * ux_2 - u_215 ) - local_f[ f_offset + 1 ] );
                d_A[ A_offset + 1 ] = local_f[ f_offset + 1 ];

                local_f[ f_offset + 2 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 + uy_3 + 4.5 * uy_2 - u_215 ) - local_f[ f_offset + 2 ] );
                d_A[ A_offset + 2 ] = local_f[ f_offset + 2 ];

                local_f[ f_offset + 3 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - ux_3 + 4.5 * ux_2 - u_215 )  - local_f[ f_offset + 3 ] );
                d_A[ A_offset + 3 ] = local_f[ f_offset + 3 ]; 

                local_f[ f_offset + 4 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - uy_3 + 4.5 * uy_2 - u_215 )  - local_f[ f_offset + 4 ] );
                d_A[ A_offset + 4 ] = local_f[ f_offset + 4 ];

                const T uxuy_2 = 2 * u_x * u_y;

                const T u_2 = ux_2 + uy_2;

                local_f[ f_offset + 5 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - local_f[ f_offset + 5 ] ); 
                d_A[ A_offset + 5 ] = local_f[ f_offset + 5 ];

                local_f[ f_offset + 6 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - local_f[ f_offset + 6 ] ); 
                d_A[ A_offset + 6 ] = local_f[ f_offset + 6 ];

                local_f[ f_offset + 7 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - local_f[ f_offset + 7 ] ); 
                d_A[ A_offset + 7 ] = local_f[ f_offset + 7 ];

                local_f[ f_offset + 8 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - local_f[ f_offset + 8 ] ); 
                d_A[ A_offset + 8 ] = local_f[ f_offset + 8 ];
            });
        });

        gpu_queue.submit( [&]( sycl::handler& h ) {

            h.parallel_for( sycl::nd_range<2>{ { ydim, xdim }, { 32, 32 } }, [=]( sycl::nd_item<2> item ) {
                
                const size_t y = item.get_global_id( 0 );
                const size_t x = item.get_global_id( 1 );

                const size_t base_index = ( x + y * xdim ) * 9;

                bool is_boundary = ( y == 0 || x == 0 || y == ydim - 1 || x == xdim - 1 );

                if ( !is_boundary ) {
                    d_A_n[ base_index + 1 ] = d_A[ ( ( x - 1 ) + y * xdim ) * 9 + 1 ];
                    d_A_n[ base_index + 2 ] = d_A[ ( x + ( y - 1 ) * xdim ) * 9 + 2 ];
                    d_A_n[ base_index + 3 ] = d_A[ ( ( x + 1 ) + y * xdim ) * 9 + 3 ];
                    d_A_n[ base_index + 4 ] = d_A[ ( x + ( y + 1 ) * xdim ) * 9 + 4 ];
                    d_A_n[ base_index + 5 ] = d_A[ ( ( x - 1 ) + ( y + 1 ) * xdim ) * 9 + 5 ];
                    d_A_n[ base_index + 6 ] = d_A[ ( ( x + 1 ) + ( y + 1 ) * xdim ) * 9 + 6 ];
                    d_A_n[ base_index + 7 ] = d_A[ ( ( x + 1 ) + ( y - 1 ) * xdim ) * 9 + 7 ];
                    d_A_n[ base_index + 8 ] = d_A[ ( ( x - 1 ) + ( y - 1 ) * xdim ) * 9 + 8 ];
                }
            });
        });

        std::swap( d_A, d_A_n );    
    }

    gpu_queue.wait(); 

    gpu_queue.memcpy( A_f32.data(), d_A_n, vec_len * 9 * sizeof( T) );

    sycl::free( d_A, gpu_queue );
}

void collide_and_stream_no_ivs( const sycl::device& device, std::vector<T>& A_f32 ) {

    const T viscosity = 0.005;

    const T omega = 1 / ( 3 * viscosity + 0.5 );

    sycl::queue gpu_queue( device );

    T* d_A = sycl::malloc_device<T>( vec_len * 9, gpu_queue );

    gpu_queue.memcpy( d_A, A_f32.data(), vec_len * 9 * sizeof( T ) );

    gpu_queue.wait();

    for ( size_t z = 0; z < 20; ++z ) {

        gpu_queue.submit( [&]( sycl::handler& h ) {

            sycl::local_accessor<T, 1> local_f( 9 * 32, h );

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {
                
                const size_t i = item.get_group().get_group_id();
                const size_t k = item.get_local_id();

                const size_t property_index = item.get_global_id();
                const size_t A_offset = ( i * 32 + k ) * 9;
                const size_t f_offset = k * 9;

                for ( size_t z = 0; z < 9; ++z ) 
                    local_f[ f_offset + z ] = d_A[ A_offset + z ];
                //item.barrier( sycl::access::fence_space::local_space );

                T rho{};
                
                for ( size_t z = 0; z < 9; ++ z ) {
                    rho += local_f[ f_offset + z ]; 
                }

                // d_rho[ property_index ] = rho;

                T u_x{};

                u_x = ( local_f[ f_offset + 1 ] + local_f[ f_offset + 5 ] +
                        local_f[ f_offset + 8 ] - local_f[ f_offset + 3 ] -
                        local_f[ f_offset + 6 ] - local_f[ f_offset + 7 ] ) / rho;

                // d_ux[ property_index ] = u_x;

                T u_y{};

                u_y = ( local_f[ f_offset + 2 ] + local_f[ f_offset + 5 ] +
                        local_f[ f_offset + 6 ] - local_f[ f_offset + 4 ] -
                        local_f[ f_offset + 7 ] - local_f[ f_offset + 8 ] ) / rho;

                // d_uy[ property_index ] = u_y;

                const T ux_2 = u_x * u_x;

                const T uy_2 = u_y * u_y;

                const T u_215 = 1.5f * ( ( ux_2 ) + ( uy_2 ) );

                local_f[ f_offset ] += ( 4.0f / 9.0f ) * rho * ( ( 1 - u_215 ) - local_f[ f_offset ] );
                d_A[ A_offset ] = local_f[ f_offset ];

                local_f[ f_offset + 1 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - 3 * u_x + 4.5 * u_x * u_x - u_215 ) - local_f[ f_offset + 1 ] );
                d_A[ A_offset + 1 ] = local_f[ f_offset + 1 ];

                local_f[ f_offset + 2 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 + 3 * u_y + 4.5 * u_x * u_x - u_215 ) - local_f[ f_offset + 2 ] );
                d_A[ A_offset + 2 ] = local_f[ f_offset + 2 ];

                local_f[ f_offset + 3 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - 3 * u_x + 4.5 * u_x * u_x - u_215 )  - local_f[ f_offset + 3 ] );
                d_A[ A_offset + 3 ] = local_f[ f_offset + 3 ]; 

                local_f[ f_offset + 4 ] += ( 1.0f / 9.0f ) * rho * ( ( 1 - 3 * u_y + 4.5 * u_x * u_x - u_215 )  - local_f[ f_offset + 4 ] );
                d_A[ A_offset + 4 ] = local_f[ f_offset + 4 ];

                local_f[ f_offset + 5 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 + 3 * u_x + 3 * u_y + 4.5 * ( ( ux_2 + uy_2 ) + 2 * u_x * u_y ) - u_215 ) - local_f[ f_offset + 5 ] ); 
                d_A[ A_offset + 5 ] = local_f[ f_offset + 5 ];

                local_f[ f_offset + 6 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 - 3 * u_x + 3 * u_y + 4.5 * ( ( ux_2 + uy_2 ) - 2 * u_x * u_y ) - u_215 ) - local_f[ f_offset + 6 ] ); 
                d_A[ A_offset + 6 ] = local_f[ f_offset + 6 ];

                local_f[ f_offset + 7 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 - 3 * u_x - 3 * u_y + 4.5 * ( ( ux_2 + uy_2 ) + 2 * u_x * u_y ) - u_215 ) - local_f[ f_offset + 7 ] ); 
                d_A[ A_offset + 7 ] = local_f[ f_offset + 7 ];

                local_f[ f_offset + 8 ] += ( 1.0f / 36.0f ) * rho * ( ( 1 + 3 * u_x - 3 * u_y + 4.5 * ( ( ux_2 + uy_2 ) - 2 * u_x * u_y ) - u_215 ) - local_f[ f_offset + 8 ] ); 
                d_A[ A_offset + 8 ] = local_f[ f_offset + 8 ];
            });
        });

        /*
        gpu_queue.submit( [&]( sycl::handler& h ) {

            h.parallel_for( sycl::nd_range<1>{ { vec_len }, { 32 } }, [=]( sycl::nd_item<1> item ) {

                const size_t i = item.get_global_id();

                const size_t index_offset = i * 9;

                T u_x{};
            });
        });
        */
    }

    gpu_queue.wait(); 

    gpu_queue.memcpy( A_f32.data(), d_A, vec_len * 9 * sizeof( T) );

    sycl::free( d_A, gpu_queue );
}

/*
PROFILE_AVG( implicit, vector_increment, f, {

    START_TIMER
    vector_increment_no_group();
    END_TIMER
})

PROFILE_AVG( group_1, vector_increment, f, {

    START_TIMER
    vector_increment_1_group();
    END_TIMER
})
*/

/*
PROFILE_AVG( gpu, vector_addition, f, {
    START_TIMER
    std::optional<sycl::device> gpu = get_gpu();
    vector_addition( *gpu );
    END_TIMER
})
*/

const bool cpu_collide_and_stream = profiler::profile_avg<f>(
    "cpu",
    "collide_and_stream",
    []( auto& fx, auto& start_time, auto& end_time ) {

        std::cout << "before allocation" << std::endl;

        std::vector<T> A( vec_len * 9 );
        std::vector<T> A_n( vec_len * 9 ); 

        std::cout << "after allocation" << std::endl;

        for ( size_t i = 0; i < vec_len * 9; ++i ) 
            A[ i ] = i * 0.01;

        std::cout << "finished assigning values" << std::endl;

        START_TIMER
        for ( size_t z = 0; z < 40; ++z ) {

            tbb::parallel_for( tbb::blocked_range<size_t>( 0, vec_len ),
                [&]( const tbb::blocked_range<size_t>& r ) {

                    for ( size_t i = r.begin(); i < r.end(); ++i ) {
        
                        const size_t index_offset = i * 9;

                        T rho_{};
                        
                        for ( size_t k = 0; k < 9; ++ k )
                            rho_ += A[ index_offset + k ];

                        // rho[ i ] = rho_;

                        T ux_{};

                        ux_ = ( A[ index_offset + 1 ] + A[ index_offset + 5 ] +
                                A[ index_offset + 8 ] - A[ index_offset + 3 ] -
                                A[ index_offset + 6 ] - A[ index_offset + 7 ] ) / rho_;

                        // u_x[ i ] = ux_;

                        T uy_{};

                        uy_ = ( A[ index_offset + 2 ] + A[ index_offset + 5 ] +
                                A[ index_offset + 6 ] - A[ index_offset + 4 ] -
                                A[ index_offset + 7 ] - A[ index_offset + 8 ] ) / rho_;

                        // u_y[ i ] = uy_;

                        const T ux_2 = ux_ * ux_;

                        const T uy_2 = uy_ * uy_;

                        const T u_215 = 1.5f * ( ( ux_2 ) + ( uy_2 ) );

                        const T ux_3 = ux_ * 3;

                        const T uy_3 = uy_ * 3;

                        A[ index_offset ] += ( 4.0f / 9.0f ) * rho_ * ( ( 1 - u_215 ) - A[ index_offset ] );

                        A[ index_offset + 1 ] += ( 1.0f / 9.0f ) * rho_ * ( ( 1 - ux_3 + 4.5 * ux_2 - u_215 ) - A[ index_offset + 1 ] );

                        A[ index_offset + 2 ] += ( 1.0f / 9.0f ) * rho_ * ( ( 1 + uy_3 + 4.5 * uy_2 - u_215 ) - A[ index_offset + 2 ] );

                        A[ index_offset + 3 ] += ( 1.0f / 9.0f ) * rho_ * ( ( 1 - ux_3 + 4.5 * ux_2 - u_215 ) - A[ index_offset + 3 ] );    
                    
                        A[ index_offset + 4 ] += ( 1.0f / 9.0f ) * rho_ * ( ( 1 - uy_3 + 4.5 * uy_2 - u_215 )  - A[ index_offset + 4 ] ); 
                    
                        const T uxuy_2 = 2 * ux_ * uy_;

                        const T u_2 = ux_2 + uy_2;

                        A[ index_offset + 5 ] += ( 1.0f / 36.0f ) * rho_ * ( ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - A[ index_offset + 5 ] ); 

                        A[ index_offset + 6 ] += ( 1.0f / 36.0f ) * rho_ * ( ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - A[ index_offset + 6 ] );

                        A[ index_offset + 7 ] += ( 1.0f / 36.0f ) * rho_ * ( ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - A[ index_offset + 7 ] ); 
                
                        A[ index_offset + 8 ] += ( 1.0f / 36.0f ) * rho_ * ( ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - A[ index_offset + 8 ] ); 
                    }
                });

            tbb::parallel_for( tbb::blocked_range<size_t>( 1, ydim - 1 ),
                [&]( const tbb::blocked_range<size_t>& r ) {

                    for ( size_t y = r.begin(); y < r.end(); ++y ) {

                        for ( size_t x = 1; x < xdim - 1; ++x ) {

                            const size_t base_index = ( x + y * xdim ) * 9;

                            A_n[ base_index + 1 ] = A[ ( ( x - 1 ) + y * xdim ) * 9 + 1 ];
                            A_n[ base_index + 2 ] = A[ ( x + ( y - 1 ) * xdim ) * 9 + 2 ];
                            A_n[ base_index + 3 ] = A[ ( ( x + 1 ) + y * xdim ) * 9 + 3 ];
                            A_n[ base_index + 4 ] = A[ ( x + ( y + 1 ) * xdim ) * 9 + 4 ];
                            A_n[ base_index + 5 ] = A[ ( ( x - 1 ) + ( y + 1 ) * xdim ) * 9 + 5 ];
                            A_n[ base_index + 6 ] = A[ ( ( x + 1 ) + ( y + 1 ) * xdim ) * 9 + 6 ];
                            A_n[ base_index + 7 ] = A[ ( ( x + 1 ) + ( y - 1 ) * xdim ) * 9 + 7 ];
                            A_n[ base_index + 8 ] = A[ ( ( x - 1 ) + ( y - 1 ) * xdim ) * 9 + 8 ];
                        }
                    }
                });
        }
        END_TIMER
        for ( size_t k = 0; k < 10; ++k ) 
            std::cout << A_n[ k ] << std::endl;

        for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
            std::cout << A_n[ k ] << std::endl;
    }
);

/*
PROFILE_AVG( gpu_no_ivs, collide_and_stream, f, {

    std::vector<T> A( vec_len * 9 );

    for ( size_t i = 0; i < vec_len * 9; ++i )
        A[ i ] = i * 0.01;
    
    std::optional<sycl::device> gpu = get_gpu();
    START_TIMER
    collide_and_stream_no_ivs( *gpu, A ); 
    END_TIMER

    for ( size_t k = 0; k < 10; ++k ) 
        std::cout << A[ k ] << std::endl;

    for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
        std::cout << A[ k ] << std::endl;
})
*/

PROFILE_AVG( gpu, collide_and_stream, f, {

    std::vector<T> A( vec_len * 9 );

    for ( size_t i = 0; i < vec_len * 9; ++i )
        A[ i ] = i * 0.01;
    
    std::optional<sycl::device> gpu = get_gpu();
    START_TIMER
    collide_and_stream( *gpu, A ); 
    END_TIMER

    for ( size_t k = 0; k < 10; ++k ) 
        std::cout << A[ k ] << std::endl;

    for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
        std::cout << A[ k ] << std::endl;
})

const bool cpu = profiler::profile_avg<f>(
    "cpu",
    "set_equil",
    []( auto& fx, auto& start_time, auto& end_time ) {

        std::cout << "updated s" << std::endl;

        std::vector<T> A_f32( vec_len * 9 );

        for ( size_t i = 0; i < vec_len * 9; ++i ) {

            A_f32[ i ] = i * 0.01f;
        }

        START_TIMER

        const double u_x = 0.1;
        const double u_y = 0.0;

        const double ux_3 = 3 * u_x;
        const double uy_3 = 3 * u_y;
        const double ux_2 = u_x * u_x;
        const double uy_2 = u_y * u_y;
        const double uxuy2 = 2 * u_x * u_y;
        const double u_2 = ux_2 + uy_2;
        const double u_215 = 1.5 * u_2;

        for ( size_t z = 0; z < 20; ++z ) {
            tbb::parallel_for( tbb::blocked_range<size_t>( 0, vec_len ),
                [&]( const tbb::blocked_range<size_t>& r ) {

                    for ( size_t i = r.begin(); i < r.end(); ++i ) {

                        const size_t A_index = i * 9;

                        A_f32[ A_index ] = ( 4.0 / 9.0 ) * ( 1 - u_215 );
                        A_f32[ A_index + 1 ] = ( 1.0 / 9.0 ) * ( 1 + ux_3 + 4.5 * ux_2 - u_215 );
                        A_f32[ A_index + 2 ] = ( 1.0 / 9.0 ) * ( 1 + uy_3 + 4.5 * uy_2 - u_215 );
                        A_f32[ A_index + 3 ] = ( 1.0 / 9.0 ) * ( 1 - ux_3 + 4.5 * ux_2 - u_215 );
                        A_f32[ A_index + 4 ] = ( 1.0 / 9.0 ) * ( 1 - uy_3 + 4.5 * uy_2 - u_215 );
                        A_f32[ A_index + 5 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                        A_f32[ A_index + 6 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
                        A_f32[ A_index + 7 ] = ( 1.0 / 36.0 ) * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy2 ) - u_215 );
                        A_f32[ A_index + 8 ] = ( 1.0 / 36.0 ) * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy2 ) - u_215 );
                    }
                });
        }
        END_TIMER
        for ( size_t k = 0; k < 10; ++k ) 
            std::cout << A_f32[ k ] << std::endl;

        for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
            std::cout << A_f32[ k ] << std::endl;
    }
);
/*
PROFILE_AVG( single_kernel, set_equil, f, {
    std::vector<T> A_f32( vec_len * 9 );

    for ( size_t i = 0; i < vec_len * 9; ++i ) {
        A_f32[ i ] = i * 0.01f;
    }
    START_TIMER
    std::optional<sycl::device> gpu = get_gpu();
    set_equil( *gpu, A_f32 );
    END_TIMER
    for ( size_t k = 0; k < 10; ++k ) 
        std::cout << A_f32[ k ] << std::endl;

    for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
        std::cout << A_f32[ k ] << std::endl;

    std::cout << "zzz" << std::endl;
})

PROFILE_AVG( multi_kernel, set_equil, f, {
    std::vector<T> A_f32( vec_len * 9 );

    for ( size_t i = 0; i < vec_len * 9; ++i ) {
        A_f32[ i ] = i * 0.01f;
    }
    START_TIMER
    std::optional<sycl::device> gpu = get_gpu();
    set_equil_mk( *gpu, A_f32 );
    END_TIMER
    for ( size_t k = 0; k < 10; ++k ) 
        std::cout << A_f32[ k ] << std::endl;

    for ( size_t k = vec_len - 1; k > vec_len - 11; --k )
        std::cout << A_f32[ k ] << std::endl;

    std::cout << "zzz" << std::endl;
})
*/

/*
0 1 3 5 
2 4 6 7 
auto max_elem_n = std::min( ydim, xdim );
auto max_elem_rows_n = std::max( ydim, xdim ) - std::min( ydim, xdim ) + 1;


xdim, ydim
auto elems = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

auto { { a_0, a_1 }, { a_2, a_3 }, { a_4, a_5 },     };
auto { {   0,   1 }, {   1,   3 }, {   3,   5 }, ... };
     {            0 ,           1 ,           2 , ... };

1, 2, 2, 2, 1
0, 1, 2, 3, 4
sum( 1, 2, 3, ... max_elems_n ) index( i )



size_t global_index{};
for ( auto elem_n = 1; elem_n < max_elem_n; ++elem_n ) {
    for ( auto i = global_index; i < global_index + elem_n; ++i ) {
        std::cout << elems[ i ] << std::endl;
    }
    global_index += elem_n;
}

for ( auto i = 0; i < max_elem_rows_n; ++i ) {
    for ( auto j = global_index; j < global_index + max_elem_n; ++j ) {
        std::cout << elems[ j ] << std::endl;
    }
    global_index += max_elem_n;
}

for ( auto elem_n = global_index - 1; elem_n > 0; -- elem_n) {
    for ( auto i = global_index; i > global_index - elen_n; -- i ) {
        std::cout << elems[ i ];
    }
    global_index -= elem_n;
}

max_elem_n
max_elem_rows_n
given ( x, y )
calculate global_index

x = 1, y = 3



global_index = 0
elem_n = 1
i = 0
print: 0
elem_n = 2 end loop

*/

 