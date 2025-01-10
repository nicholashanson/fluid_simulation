#include <grid_renderer.hpp>
#include <glad/glad.h>
#include <vector>

namespace fs {

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

    void render_grid( unsigned int VAO, size_t vertex_count ) {

        glBindVertexArray( VAO );

        size_t triangle_count = vertex_count / 3;

        glDrawArrays( GL_TRIANGLES, 0, triangle_count );

        glBindVertexArray( 0 );
    }

}
