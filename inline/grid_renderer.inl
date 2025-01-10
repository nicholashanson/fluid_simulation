#ifndef GRID_RENDERER_INL
#define GRID_RENDERER_INL

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <lbm.hpp>

#include <colors.hpp>

namespace fs {

    template<typename Array, typename MDSpan>
    std::vector<float> lb_to_vertex_data( const grid<Array, MDSpan>& gd, std::vector<double>& vec_mag_states_ ) {

        std::vector<float> vertices;

        auto [ max_velocity, velocities ] = lbm::calculate_velocities_with_max( gd, vec_mag_states_ );

        const float cell_size = 2.0f / gd.get_grid_width();

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                struct Color cell_color = velocity_to_color( velocities.get_cell_state( i, j ), max_velocity );

                float x = -1.0f  + j * cell_size;
                float y = 1.0f - i * cell_size;

                vertices.insert( vertices.end(), {

                    // first triangle
                    x, y, cell_color.r, cell_color.g, cell_color.b,
                    x + cell_size, y, cell_color.r, cell_color.g, cell_color.b,
                    x, y - cell_size, cell_color.r, cell_color.g, cell_color.b,

                    // second triangle
                    x + cell_size, y, cell_color.r, cell_color.g, cell_color.b,
                    x, y - cell_size, cell_color.r, cell_color.g, cell_color.b,
                    x + cell_size, y - cell_size, cell_color.r, cell_color.g, cell_color.b,

                } );

            }

        return vertices;

    }

    template<typename array, typename mdspan>
    std::vector<float> grid_to_vertex_data( const grid<array, mdspan>& gd ) {

        std::vector<float> vertices;

        const float cell_size = 1.0f / gd.get_grid_height();

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                auto [ v_x, v_y ] = gd.get_cell_state( i, j );

                float x = -1.0f  + j * cell_size;
                float y = 1.0f - i * cell_size;

                struct Color cell_color = color_map.at( { v_x, v_y } );

                vertices.insert( vertices.end(), {

                    // first triangle
                    x, y, cell_color.r, cell_color.g, cell_color.b,
                    x + cell_size, y, cell_color.r, cell_color.g, cell_color.b,
                    x, y - cell_size, cell_color.r, cell_color.g, cell_color.b,

                    // second triangle
                    x + cell_size, y, cell_color.r, cell_color.g, cell_color.b,
                    x, y - cell_size, cell_color.r, cell_color.g, cell_color.b,
                    x + cell_size, y - cell_size, cell_color.r, cell_color.g, cell_color.b,
                } );

            }

        return vertices;

    }

}

#endif

