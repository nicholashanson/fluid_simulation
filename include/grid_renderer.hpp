#ifndef GRID_RENDERER_HPP
#define GRID_RENDERER_HPP

#include <vector>
#include <grid.hpp>
#include <functional>
#include <array>

namespace app {

    void setup_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void render_grid( unsigned int VAO, size_t vertex_count );
}

#include <grid_renderer.inl> // include template implementations

#endif