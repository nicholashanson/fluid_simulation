#ifndef GRID_RENDERER_HPP
#define GRID_RENDERER_HPP

#include <vector>
#include <grid.hpp>
#include <functional>
#include <array>

namespace app {

    void setup_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void init_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void init_grid_buffers_3D( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void refresh_grid_buffers( const std::vector<float>& vertices, unsigned int VBO );

    void render_grid( unsigned int VAO, size_t vertex_count );

    void render_wiremesh( unsigned int VAO, size_t vertex_count );
        
    void generate_wiremesh_vertices( std::vector<float>& vertices );

    std::vector<float> axes();

    std::vector<float> boundaries();
}

#include <grid_renderer.inl> // include template implementations

#endif