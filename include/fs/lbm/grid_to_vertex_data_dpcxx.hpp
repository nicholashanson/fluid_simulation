#ifndef GRID_TO_VERTEX_DATA_DPCPP
#define GRID_TO_VERTEX_DATA_DPCPP

#include <sycl/sycl.hpp>

#include <optional>

#include <settings.hpp>
#include <fs/lbm/dpcxx_common.hpp>

#include <opencv.hpp>

namespace fs { namespace dpcxx { namespace lbm {

    struct Color {
        float r;
        float g;
        float b;
    };

    inline Color property_to_color( double property, double max_property ) {

        double normalized_property = property / max_property;

        float r = static_cast<float>( normalized_property );
        float b = static_cast<float>( 1.0 - normalized_property );

        return Color{ r, 0, b };
    }

    template<size_t ydim, size_t xdim>
    void grid_to_vertex_data( float* vertex_data, double* property_data ) {

        const size_t vertices_n = 6;
        
        const size_t vertex_data_n = 5;

        constexpr size_t data_n = vertices_n * vertex_data_n;

        const float cell_size = 2.0f / std::max( ydim, xdim );

        const double max_property = *std::max_element( property_data, property_data + ydim * xdim ); 

        std::optional<sycl::device> gpu = get_gpu();

        if ( !gpu ) {
            throw std::runtime_error( "Error: No suitable GPU device found." );
        } 

        sycl::queue gpu_queue( *gpu );

        float* vertex_data_device = sycl::malloc_device<float>( ydim * xdim * data_n, gpu_queue );
        double* property_device = sycl::malloc_device<double>( ydim * xdim, gpu_queue );

        gpu_queue.memcpy( property_device, property_data, ydim * xdim * sizeof( double ) ).wait();

        gpu_queue.submit( [&]( sycl::handler& h ) {
            h.parallel_for( sycl::nd_range<1>{ ydim * xdim, 32 }, [=]( sycl::nd_item<1> item ) {
                
                    size_t global_index = item.get_global_id(0);

                    size_t y = global_index / xdim;
                    size_t x = global_index % xdim; 

                    struct Color cell_color = property_to_color( property_device[ x + y * xdim ], max_property );

                    float x_ = -1.0f + x * cell_size;
                    float y_ = 1.0f - y * cell_size;

                    const float vertex_data_[ vertices_n * 2 ] = {
                        // first_triange
                        x_, y_, 
                        x_ + cell_size, y_, 
                        x_, y_ - cell_size, 

                        // second triangle
                        x_ + cell_size, y_, 
                        x_, y_ - cell_size, 
                        x_ + cell_size, y_ - cell_size, 
                    };

                    const size_t index_offset = ( x + y * xdim ) * data_n;

                    for ( size_t k = 0; k < vertices_n; ++k ) {
                        
                        const size_t local_offset = k * vertex_data_n;

                        const size_t total_offset = index_offset + local_offset;

                        vertex_data_device[ total_offset ] = vertex_data_[ k * 2 ];
                        vertex_data_device[ total_offset + 1 ] = vertex_data_[ k * 2 + 1 ]; 
                        vertex_data_device[ total_offset + 2 ] = cell_color.r;
                        vertex_data_device[ total_offset + 3 ] = cell_color.g;
                        vertex_data_device[ total_offset + 4 ] = cell_color.b;
                    }
            });
        });

        gpu_queue.wait();

        gpu_queue.memcpy( vertex_data, vertex_data_device, ydim * xdim * data_n * sizeof( float ) ).wait();

        sycl::free( vertex_data_device, gpu_queue );
        sycl::free( property_device, gpu_queue );
    }

    template<size_t ydim, size_t xdim>
    void grid_to_vertex_data_cv( float* vertex_data, double* property_data, int colormap ) {

        const size_t vertices_n = 6;
        
        const size_t vertex_data_n = 5;

        constexpr size_t data_n = vertices_n * vertex_data_n;

        const float cell_size = 2.0f / std::max( ydim, xdim );

        const double max_property = *std::max_element( property_data, property_data + ydim * xdim ); 

        std::optional<sycl::device> gpu = get_gpu();

        if ( !gpu ) {

            throw std::runtime_error( "Error: No suitable GPU device found." );
        } 

        sycl::queue gpu_queue( *gpu );

        float* vertex_data_device = sycl::malloc_device<float>( ydim * xdim * data_n, gpu_queue );
        double* property_device = sycl::malloc_device<double>( ydim * xdim, gpu_queue );

        cv::Mat colormap_mat( 1, 256, CV_8UC1 ); 
-
        for ( int i = 0; i < 256; ++i ) {

            colormap_mat.at<uchar>( 0, i ) = i;
        }

        cv::applyColorMap( colormap_mat, colormap_mat, colormap );

        uint8_t* colormap_device = sycl::malloc_device<uint8_t>( 256 * 3, gpu_queue );

        gpu_queue.memcpy( colormap_device, colormap_mat.data, 256 * 3 * sizeof( uint8_t ) ).wait();

        gpu_queue.memcpy( property_device, property_data, ydim * xdim * sizeof( double ) ).wait();

        gpu_queue.submit( [&]( sycl::handler& h ) {

            h.parallel_for( sycl::nd_range<1>{ ydim * xdim, 32 }, [=]( sycl::nd_item<1> item ) {
                
                    size_t global_index = item.get_global_id(0);

                    size_t y = global_index / xdim;
                    size_t x = global_index % xdim; 

                    double normalized_property = property_device[ x + y * xdim ] / max_property;
                    int color_index = static_cast<int>( normalized_property * 255.0 );

                    float b = static_cast<float>( colormap_device[ color_index * 3 ] ) / 255.0f;
                    float g = static_cast<float>( colormap_device[ color_index * 3 + 1 ] ) / 255.0f;
                    float r = static_cast<float>( colormap_device[ color_index * 3 + 2 ] ) / 255.0f;

                    float x_ = -1.0f + x * cell_size;
                    float y_ = 1.0f - y * cell_size;

                    const float vertex_data_[ vertices_n * 2 ] = {
                        // first_triange
                        x_, y_, 
                        x_ + cell_size, y_, 
                        x_, y_ - cell_size, 

                        // second triangle
                        x_ + cell_size, y_, 
                        x_, y_ - cell_size, 
                        x_ + cell_size, y_ - cell_size, 
                    };

                    const size_t index_offset = ( x + y * xdim ) * data_n;

                    for ( size_t k = 0; k < vertices_n; ++k ) {
                        
                        const size_t local_offset = k * vertex_data_n;

                        const size_t total_offset = index_offset + local_offset;

                        vertex_data_device[ total_offset ] = vertex_data_[ k * 2 ];
                        vertex_data_device[ total_offset + 1 ] = vertex_data_[ k * 2 + 1 ]; 
                        vertex_data_device[ total_offset + 2 ] = r;
                        vertex_data_device[ total_offset + 3 ] = g;
                        vertex_data_device[ total_offset + 4 ] = b;
                    }
            });
        });

        gpu_queue.wait();

        gpu_queue.memcpy( vertex_data, vertex_data_device, ydim * xdim * data_n * sizeof( float ) ).wait();

        sycl::free( vertex_data_device, gpu_queue );
        sycl::free( property_device, gpu_queue );
        sycl::free( colormap_device, gpu_queue );
    }

}}} // } namespace lbm } namespace dpcxx } namespace fs }

#endif
