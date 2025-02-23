#ifndef GRID_RENDERER_HPP
#define GRID_RENDERER_HPP

#include <vector>
#include <grid.hpp>
#include <functional>
#include <array>

namespace app {

    void setup_grid_buffers( const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO );

    void render_grid( unsigned int VAO, size_t vertex_count );

    template<size_t ydim, size_t xdim>
    std::vector<float> obstacle_to_vertex_data( const std::array<bool, xdim*ydim>& barrier );

    template<size_t ydim, size_t xdim>
    std::vector<float> js( const std::array<double, xdim*ydim>& u_x );

    template<size_t ydim, size_t xdim>
    std::vector<float> js_tbb( const std::array<double, xdim*ydim>& u_x );

    /*
    template<typename Array, typename MDSpan>
    std::vector<float> property_to_vertex_data( const sim::grid<Array, MDSpan>& gd, std::vector<double>& property_states,
                                                std::function<double(std::array<double, 9> calculate_property )> );
    */
}

#include <grid_renderer.inl> // include template implementations

#endif
