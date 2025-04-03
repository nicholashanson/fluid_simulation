#include <grid_renderer.hpp>
#include <glad/glad.h>
#include <vector>

namespace app {

    void setup_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO ) {

        glGenVertexArrays( 1, &VAO );
        glGenBuffers( 1, &VBO );

        glBindVertexArray( VAO );

        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_STATIC_DRAW );

        // vertex position ( x, y ) attribute
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )0 );
        glEnableVertexAttribArray( 0 );

        // vertex color ( r, g, b ) attribute
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )( 2 * sizeof( float ) ) );
        glEnableVertexAttribArray( 1 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindVertexArray( 0 );
    }

    void init_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO ) {

        glGenVertexArrays( 1, &VAO );
        glGenBuffers( 1, &VBO );

        glBindVertexArray( VAO );

        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_DYNAMIC_DRAW );

        // vertex position ( x, y ) attribute
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )0 );
        glEnableVertexAttribArray( 0 );

        // vertex color ( r, g, b ) attribute
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void * )( 2 * sizeof( float ) ) );
        glEnableVertexAttribArray( 1 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindVertexArray( 0 );
    }

    void refresh_grid_buffers( const std::vector<float>& vertices, unsigned int VBO) {

        glBindBuffer( GL_ARRAY_BUFFER, VBO );

        glBufferSubData( GL_ARRAY_BUFFER, 0, vertices.size() * sizeof( float ), vertices.data() );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void render_grid( unsigned int VAO, size_t vertex_count ) {

        glBindVertexArray( VAO );

        size_t triangle_count = vertex_count / 3;

        glDrawArrays( GL_TRIANGLES, 0, triangle_count );

        glBindVertexArray( 0 );
    }

    void setup_grid_buffers_3D( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO ) {
        
        glGenVertexArrays( 1, &VAO );
        glGenBuffers( 1, &VBO );

        glBindVertexArray( VAO );

        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_STATIC_DRAW );

        // vertex position ( x, y, z ) attribute
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void* )0 );
        glEnableVertexAttribArray( 0 );

        // vertex color ( r, g, b ) attribute
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
        glEnableVertexAttribArray( 1 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindVertexArray( 0 );
    }

    void init_grid_buffers_3D( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO ) {
        
        glGenVertexArrays( 1, &VAO );
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( float ), vertices.data(), GL_DYNAMIC_DRAW );

        // vertex position ( x, y, z ) attribute
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void* )0 );
        glEnableVertexAttribArray( 0 );

        // vertex color ( r, g, b ) attribute
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
        glEnableVertexAttribArray( 1 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindVertexArray( 0 );
    } 

    void render_wiremesh( unsigned int VAO, size_t vertex_count ) {
        
        glBindVertexArray( VAO );

        size_t line_count = vertex_count / 2;  
        glDrawArrays( GL_LINES, 0, line_count );

        glBindVertexArray( 0 );
    }

    void generate_wiremesh_vertices( std::vector<float>& vertices ) {
        
        float step = 0.5f;

        float xdim = static_cast<float>( fs::settings::xdim );
        float ydim = static_cast<float>( fs::settings::ydim );
        float zdim = static_cast<float>( fs::settings::zdim );

        float x_start = -( xdim / 2.0 ) * step;
        float y_start = -( ydim / 2.0 ) * step;
        float z_start = -( zdim / 2.0 ) * step;

        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {
            for ( size_t y = 0; y < fs::settings::ydim; ++y ) {

                vertices.insert( vertices.end(), {
                    x_start + static_cast<float>( x ) * step, y_start + static_cast<float>( y ) * step, z_start,
                    0.5f, 0.5f, 0.5f,
                    x_start + static_cast<float>( x ) * step, y_start + static_cast<float>( y ) * step, z_start + zdim * step,
                    0.5f, 0.5f, 0.5f,
                });
            }
        }

        for ( size_t z = 0; z < fs::settings::zdim; ++z ) {
            for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                vertices.insert( vertices.end(), {
                    x_start + static_cast<float>( x ) * step, y_start, z_start + static_cast<float>( z ) * step,
                    0.5f, 0.5f, 0.5f,
                    x_start + static_cast<float>( x ) * step, y_start + ydim * step, z_start + static_cast<float>( z ) * step,
                    0.5f, 0.5f, 0.5f,
                });
            }
        }

        for ( size_t z = 0; z < fs::settings::zdim; ++z ) {
            for ( size_t y = 0; y < fs::settings::ydim; ++y ) {

                vertices.insert( vertices.end(), {
                    x_start, y_start + static_cast<float>( y ) * step, z_start + static_cast<float>( z ) * step,
                    0.5f, 0.5f, 0.5f,
                    x_start + xdim * step, y_start + static_cast<float>( y ) * step, z_start + static_cast<float>( z ) * step,
                    0.5f, 0.5f, 0.5f,
                });
            }
        }
    }

    std::vector<float> boundaries() {

        std::vector<float> boundaries;

        float step = 0.5f;

        float xdim_f = static_cast<float>( fs::settings::xdim );
        float ydim_f = static_cast<float>( fs::settings::ydim );
        float zdim_f = static_cast<float>( fs::settings::zdim );

        float x_start = -( xdim_f / 2.0 ) * step;
        float y_start = -( ydim_f / 2.0 ) * step;
        float z_start = -( zdim_f / 2.0 ) * step;

        float x_end = -x_start;
        float y_end = -y_start;
        float z_end = -z_start;

        boundaries.insert( boundaries.end(), {
            -100.0f, y_start, z_start,
               1.0f,    0.0f,    0.0f,
            
             100.0f, y_start, z_start,
               1.0f,    0.0f,    0.0f,

            x_start, -100.0f, z_start,
               0.0f,    1.0f,    0.0f,

            x_start,  100.0f, z_start,
               0.0f,    1.0f,    0.0f,

            x_start, y_start, -100.0f,
               0.0f,    0.0f,    1.0f,

            x_start, y_start,  100.0f,
               0.0f,    0.0f,    1.0f,

            -100.0f,   y_end,   z_end,
               1.0f,    0.0f,    0.0f,
            
             100.0f,   y_end,   z_end,
               1.0f,    0.0f,    0.0f,

              x_end, -100.0f,   z_end,
               0.0f,    1.0f,    0.0f,

              x_end,  100.0f,   z_end,
               0.0f,    1.0f,    0.0f,

              x_end,   y_end, -100.0f,
               0.0f,    0.0f,    1.0f,

              x_end,   y_end,  100.0f,
               0.0f,    0.0f,    1.0f,
        });

        return boundaries;
    }

    std::vector<float> axes() {

        std::vector<float> axes;

        axes.insert( axes.end(), {
            -100.0f,    0.0f,    0.0f,
               1.0f,    0.0f,    0.0f,
            
             100.0f,    0.0f,    0.0f,
               1.0f,    0.0f,    0.0f,

               0.0f, -100.0f,    0.0f,
               0.0f,    1.0f,    0.0f,

               0.0f,  100.0f,    0.0f,
               0.0f,    1.0f,    0.0f,

               0.0f,    0.0f, -100.0f,
               0.0f,    0.0f,    1.0f,

               0.0f,    0.0f,  100.0f,
               0.0f,    0.0f,    1.0f,
        });

        return axes;
    }

}
