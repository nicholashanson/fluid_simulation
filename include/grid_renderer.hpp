#ifndef GRID_RENDERER_HPP
#define GRID_RENDERER_HPP

#include <vector>
#include <grid.hpp>

namespace fs {

    unsigned int setup_shaders( const char * vertex_path, const char * fragment_path );

    template<typename Array, typename MDSpan>
    std::vector<float> grid_to_vertex_data( const grid<Array, MDSpan>& gd );

    template<typename Array, typename MDSpan>
    std::vector<float> lb_to_vertex_data( const grid<Array, MDSpan>& gd, std::vector<double>& vec_mag_states );

    void setup_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void render_grid( unsigned int VAO, size_t vertex_count );

}

#include <grid_renderer.inl> // include template implementations

#endif
