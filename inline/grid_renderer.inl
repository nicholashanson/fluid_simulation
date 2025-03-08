#ifndef GRID_RENDERER_INL
#define GRID_RENDERER_INL

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fs/fs.hpp>

#include <algorithm>
#include <concepts>

#include <opencv2/opencv.hpp>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

namespace app {

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

    template<typename Obstacle>
    concept boolean_indexable = requires( const Obstacle& b, size_t i ) {
        { b[ i ] } -> std::convertible_to<bool>;
    };

    template<size_t ydim, size_t xdim, typename Obstacle>
    requires boolean_indexable<Obstacle>
    std::vector<float> obstacle_to_vertex_data( const Obstacle& obstacle ) {

        // number of vertices per grid square
        const size_t vertex_n = ydim * xdim;

        // number of data points per vertex
        const size_t data_n = 5;

        // 2 floats for position and 3 for rgb = 5
        std::vector<float> vertices;

        const float cell_size = 2.0f / std::max( ydim, xdim );

        for ( size_t y = 0; y < ydim; ++y ) {
            for ( size_t x = 0; x < xdim ; ++x ) {

                if ( obstacle[ x + y * xdim ] ) {

                    float x_ = -1.0f  + x * cell_size;
                    float y_ = 1.0f - y * cell_size;

                    vertices.insert( vertices.end(), {

                        // first triangle
                        x_, y_, 0.0f, 0.0f, 0.0f,
                        x_ + cell_size, y_, 0.0f, 0.0f, 0.0f,
                        x_, y_ - cell_size, 0.0f, 0.0f, 0.0f,

                        // second triangle
                        x_ + cell_size, y_, 0.0f, 0.0f, 0.0f,
                        x_, y_ - cell_size, 0.0f, 0.0f, 0.0f,
                        x_ + cell_size, y_ - cell_size, 0.0f, 0.0f, 0.0f,
                    });
                }
            }
        }

        return vertices;
    }

    template<size_t ydim, size_t xdim>
    std::vector<float> property_to_vertex_data_tbb( const std::array<double, xdim*ydim>& property_v ) {

        // number of vertices per grid square
        const size_t vertex_n = 6;

        // number of data points per vertex
        const size_t data_n = 5;

        // 2 floats for position and 3 for rgb = 5
        std::vector<float> vertices( xdim * ydim * vertex_n * data_n );

        sim::grid<std::vector<float>, fs::vertex_view> vertex_grid( vertices.data() );
        
        const float cell_size = 2.0f / std::max( xdim, ydim );

        const double max_property = *std::max_element( property_v.begin(), property_v.end() ); 

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 0, ydim ),
            [&]( const tbb::blocked_range<size_t>& range ) {

                for ( size_t y = range.begin(); y < range.end(); ++y )
                    for ( size_t x = 0; x < xdim; ++x ) {

                        struct Color cell_color = property_to_color( property_v[ x + y*xdim ], max_property );

                        float x_ = -1.0f  + x * cell_size;
                        float y_ = 1.0f - y * cell_size;

                        std::array<float, vertex_n * data_n> vertex_data = {
                            // first triangle
                            x_, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,

                            // second triangle
                            x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                            x_ + cell_size, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                        };

                        vertex_grid.set_cell_state_array( vertex_data, y, x );
                    }
            }
        );

        return vertices;
    }

    template<size_t ydim, size_t xdim>
    std::vector<float> property_to_vertex_data_tbb_grid( const std::array<double, xdim*ydim>& property_v ) {

        // number of vertices per grid square
        const size_t vertex_n = 6;

        // number of data points per vertex
        const size_t vertex_data_n = 5;

        const size_t data_n = vertex_n * vertex_data_n;

        // 2 floats for position and 3 for rgb = 5
        std::vector<float> vertices( ydim * xdim * data_n );
        
        const float cell_size = 2.0f / std::max( ydim, xdim );

        const double max_property = *std::max_element( property_v.begin(), property_v.end() ); 

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 0, ydim ),
            [&]( const tbb::blocked_range<size_t>& range ) {

                for ( size_t y = range.begin(); y < range.end(); ++y ) {
                    for ( size_t x = 0; x < xdim; ++x ) {

                        struct Color cell_color = property_to_color( property_v[ x + y * xdim ], max_property );

                        float x_ = -1.0f + x * cell_size;
                        float y_ = 1.0f - y * cell_size;

                        std::array<float, data_n> vertex_data = {
                            // first triangle
                            x_, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,

                            // second triangle
                            x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                            x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                            x_ + cell_size, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                        };

                        const size_t starting_index = ( x + y * xdim ) * data_n;

                        for ( size_t z = starting_index; auto vertex : vertex_data ) {

                            vertices[ z++ ] = vertex;
                        }
                    }
                }
            }
        );

        return vertices;
    }

    template<size_t ydim, size_t xdim>
    std::vector<float> property_array_to_vertex_data( const std::array<double, xdim*ydim>& u_x ) {

        std::vector<float> vertices;
        
        const float cell_size = 2.0f / xdim;

        const double max_velocity = *std::max_element( u_x.begin(), u_x.end() ); 

        for ( size_t y = 0; y < ydim; ++y ) {
            for ( size_t x = 0; x < xdim ; ++x ) {

                struct Color cell_color = property_to_color( u_x[ x + y*xdim ], max_velocity );  

                float x_ = -1.0f  + x * cell_size;
                float y_ = 1.0f - y * cell_size;

                vertices.insert( vertices.end(), {

                    // first triangle
                    x_, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,

                    // second triangle
                    x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                    x_ + cell_size, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                } );
            }
        }  

        return vertices; 
    }

    template<typename Array, typename MDSpan>
    std::vector<float> property_grid_to_vertex_data( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states,
                                                     std::function<double( std::array<double, 9>& )> calculate_property ) {

        std::vector<float> vertices;

        auto [ max_property, properties ] = fs::lbm::calculate_property_v_with_max( gd, property_states, calculate_property );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        for ( size_t y = 0; y < ydim; ++y ) {
            for ( size_t x = 0; x < xdim; ++x ) {

                struct Color cell_color = property_to_color( properties.get_cell_state( y, x ), max_property );

                float x_ = -1.0f  + x * cell_size;
                float y_ = 1.0f - y * cell_size;

                vertices.insert( vertices.end(), {

                    // first triangle
                    x_, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,

                    // second triangle
                    x_ + cell_size, y_, cell_color.r, cell_color.g, cell_color.b,
                    x_, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                    x_ + cell_size, y_ - cell_size, cell_color.r, cell_color.g, cell_color.b,
                });
            }
        }

        return vertices;
    }

    template<typename Array, typename MDSpan>
    std::vector<float> property_grid_to_vertex_data_cv( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states,
                                                        std::function<double( std::array<double, 9>& )> calculate_property ) {

        std::vector<float> vertices;

        auto [ max_property, properties ] = fs::lbm::calculate_property_v_with_max( gd, property_states, calculate_property );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        cv::Mat colors( ydim, xdim, CV_64F, ( void* )properties.get_data_handle() );

        double scale = 255.0 / max_property;

        colors.convertTo( colors, CV_8U, scale );

        cv::applyColorMap( colors, colors, cv::COLORMAP_RAINBOW );

        for ( size_t y = 0; y < ydim; ++y ) {
            for ( size_t x = 0; x < xdim; ++x ) {

                cv::Vec3b color = colors.at<cv::Vec3b>( y, x );

                float x_ = -1.0f + x * cell_size;
                float y_ = 1.0f - y * cell_size;
    
                float r = color[ 2 ] / 255.0f; 
                float g = color[ 1 ] / 255.0f;
                float b = color[ 0 ] / 255.0f;

                vertices.insert( vertices.end(), {

                    // first triangle
                    x_, y_, r, g, b,
                    x_ + cell_size, y_, r, g, b,
                    x_, y_ - cell_size, r, g, b,

                    // second triangle
                    x_ + cell_size, y_, r, g, b,
                    x_, y_ - cell_size, r, g, b,
                    x_ + cell_size, y_ - cell_size, r, g, b,
                });
            }
        }

        return vertices;
    }

    template<typename Array, typename MDSpan>
    std::vector<float> property_grid_to_vertex_data_cv_tbb( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states,
                                                            std::function<double( std::array<double, 9>& )> calculate_property ) {

        // number of vertices per grid square
        const size_t vertex_n = 6;

        // number of data points per vertex
        const size_t vertex_data_n = 5;

        const size_t data_n = vertex_n * vertex_data_n;

        auto [ max_property, properties ] = fs::lbm::calculate_property_v_with_max( gd, property_states, calculate_property );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        std::vector<float> vertices( ydim * xdim * data_n );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        cv::Mat colors( ydim, xdim, CV_64F, ( void* )properties.get_data_handle() );

        double scale = 255.0 / max_property;

        colors.convertTo( colors, CV_8U, scale );

        cv::applyColorMap( colors, colors, cv::COLORMAP_RAINBOW );

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 0, ydim ),
            [&]( const tbb::blocked_range<size_t>& range ) {
        
                for ( size_t y = range.begin(); y < range.end(); ++y ) {
                    for ( size_t x = 0; x < xdim; ++x ) {
        
                        cv::Vec3b color = colors.at<cv::Vec3b>( y, x );
        
                        float x_ = -1.0f + x * cell_size;
                        float y_ = 1.0f - y * cell_size;
    
                        float r = color[ 2 ] / 255.0f; 
                        float g = color[ 1 ] / 255.0f;
                        float b = color[ 0 ] / 255.0f;
        
                        std::array<float, data_n> vertex_data = {
                            // first triangle
                            x_, y_, r, g, b, 
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,

                            // second triangle
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,
                            x_ + cell_size, y_ - cell_size, r, g, b,
                        };

                        const size_t starting_index = ( x + y * xdim ) * data_n;

                        for ( size_t z = starting_index; auto vertex : vertex_data ) {

                            vertices[ z++ ] = vertex;
                        }
                    }
                }
            }
        );

        return vertices;
    }

    template<typename Array, typename MDSpan>
    std::vector<float> property_grid_to_vertex_data_cv_tbb_copy( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states,
                                                                 std::function<double( std::array<double, 9>& )> calculate_property ) {

        // number of vertices per grid square
        const size_t vertex_n = 6;

        // number of data points per vertex
        const size_t vertex_data_n = 5;

        const size_t data_n = vertex_n * vertex_data_n;

        auto [ max_property, properties ] = fs::lbm::calculate_property_v_with_max( gd, property_states, calculate_property );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        std::vector<float> vertices( ydim * xdim * data_n );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        cv::Mat colors( ydim, xdim, CV_64F, ( void* )properties.get_data_handle() );

        double scale = 255.0 / max_property;

        colors.convertTo( colors, CV_8U, scale );

        cv::applyColorMap( colors, colors, cv::COLORMAP_RAINBOW );

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 0, ydim ),
            [&]( const tbb::blocked_range<size_t>& range ) {
        
                for ( size_t y = range.begin(); y < range.end(); ++y ) {
                    for ( size_t x = 0; x < xdim; ++x ) {
        
                        cv::Vec3b color = colors.at<cv::Vec3b>( y, x );
        
                        float x_ = -1.0f + x * cell_size;
                        float y_ = 1.0f - y * cell_size;
    
                        float r = color[ 2 ] / 255.0f; 
                        float g = color[ 1 ] / 255.0f;
                        float b = color[ 0 ] / 255.0f;
        
                        std::array<float, data_n> vertex_data = {
                            // first triangle
                            x_, y_, r, g, b, 
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,

                            // second triangle
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,
                            x_ + cell_size, y_ - cell_size, r, g, b,
                        };

                        const size_t starting_index = ( x + y * xdim ) * data_n;

                        std::copy( vertex_data.begin(), vertex_data.end(), vertices.begin() + starting_index );
                    }
                }
            }
        );

        return vertices;
    }

    template<typename Array, typename MDSpan>
    std::vector<float> curl_to_vertex_data_cv( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states ) {

        std::vector<float> vertices;

        auto [ max_curl, curl ] = fs::lbm::calculate_curl_with_max( gd, property_states );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        // construct a non-owning OpenCV matrix with curl data
        cv::Mat colors( ydim, xdim, CV_64F, ( void* )curl.get_data_handle() );

        // scale used to normalize the curl data 
        double scale = 255.0 / max_curl;

        // convert the matrix to a gray-scale image
        colors.convertTo( colors, CV_8U, scale );

        // apply the color-map to the gray-scale image
        cv::applyColorMap( colors, colors, cv::COLORMAP_VIRIDIS );

        for ( size_t y = 0; y < ydim; ++y ) {
            for ( size_t x = 0; x < xdim; ++x ) {

                // access RGB values for this cell
                cv::Vec3b color = colors.at<cv::Vec3b>( y, x );

                float x_ = -1.0f + x * cell_size;
                float y_ = 1.0f - y * cell_size;
    
                // OpenCV uses BGR
                float r = color[ 2 ] / 255.0f; 
                float g = color[ 1 ] / 255.0f;
                float b = color[ 0 ] / 255.0f;

                vertices.insert( vertices.end(), {

                    // first triangle
                    x_, y_, r, g, b,
                    x_ + cell_size, y_, r, g, b,
                    x_, y_ - cell_size, r, g, b,

                    // second triangle
                    x_ + cell_size, y_, r, g, b,
                    x_, y_ - cell_size, r, g, b,
                    x_ + cell_size, y_ - cell_size, r, g, b,
                });
            }
        }

        return vertices;
    }

    template<typename Array, typename MDSpan>
    std::vector<float> curl_to_vertex_data_cv_tbb( const sim::grid<Array, MDSpan>& gd, const std::vector<double>& property_states ) {

        // number of vertices per grid square
        const size_t vertex_n = 6;

        // number of data points per vertex
        const size_t vertex_data_n = 5;

        const size_t data_n = vertex_n * vertex_data_n;

        auto [ max_curl, curl ] = fs::lbm::calculate_curl_with_max( gd, property_states );
        
        const size_t ydim = gd.get_dim( 0 );
        const size_t xdim = gd.get_dim( 1 );

        std::vector<float> vertices( ydim * xdim * data_n );

        const float cell_size = 2.0f / std::max( ydim, xdim );

        // construct a non-owning OpenCV matrix with curl data
        cv::Mat colors( ydim, xdim, CV_64F, ( void* )curl.get_data_handle() );

        // scale used to normalize the curl data 
        double scale = 255.0 / max_curl;

        // convert the matrix to a gray-scale image
        colors.convertTo( colors, CV_8U, scale );

        // apply the color-map to the gray-scale image
        cv::applyColorMap( colors, colors, cv::COLORMAP_VIRIDIS );

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 0, ydim ),
            [&]( const tbb::blocked_range<size_t>& range ) {
        
                for ( size_t y = range.begin(); y < range.end(); ++y ) {
                    for ( size_t x = 0; x < xdim; ++x ) {
        
                        cv::Vec3b color = colors.at<cv::Vec3b>( y, x );
        
                        float x_ = -1.0f + x * cell_size;
                        float y_ = 1.0f - y * cell_size;
    
                        float r = color[ 2 ] / 255.0f; 
                        float g = color[ 1 ] / 255.0f;
                        float b = color[ 0 ] / 255.0f;
        
                        std::array<float, data_n> vertex_data = {
                            // first triangle
                            x_, y_, r, g, b, 
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,

                            // second triangle
                            x_ + cell_size, y_, r, g, b,
                            x_, y_ - cell_size, r, g, b,
                            x_ + cell_size, y_ - cell_size, r, g, b,
                        };

                        const size_t starting_index = ( x + y * xdim ) * data_n;

                        for ( size_t z = starting_index; auto vertex : vertex_data ) {

                            vertices[ z++ ] = vertex;
                        }
                    }
                }
            }
        );

        return vertices;
    }

}

#endif


