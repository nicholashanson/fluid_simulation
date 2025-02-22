#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstddef>

namespace fs {

    namespace settings {

        constexpr size_t xdim = 32 * 20;
        constexpr size_t ydim = 32 * 8;

        constexpr size_t pixels_per_cell = 2;

        constexpr int window_width = pixels_per_cell * xdim;
        constexpr int window_height = pixels_per_cell * ydim;

        constexpr size_t vertex_n = 6;
        constexpr size_t vertex_data_n = 5;

    }

}

#endif
