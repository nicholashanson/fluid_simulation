#ifndef GLOBAL_ALIASES_HPP
#define GLOBAL_ALIASES_HPP

#include <mdspan-stable/include/mdspan/mdspan.hpp>
#include <settings.hpp>

namespace fs {

    using velocity = std::pair<int, int>;

    using a25 = std::array<std::pair<int, int>, 25>;
    using gv25 = Kokkos::mdspan<velocity, Kokkos::extents<size_t, 5, 5>>;

    using lb_a25 = std::array<double, 25 * 9>;
    using lb_gv25 = Kokkos::mdspan<double, Kokkos::extents<size_t, 5, 5, 9>>;

    using grid_view = Kokkos::mdspan<velocity, Kokkos::extents<size_t, settings::grid_width, settings::grid_height>>;
    using lb_grid = Kokkos::mdspan<double, Kokkos::extents<size_t, settings::grid_width, settings::grid_height, 9>>;
    using vec_mag_view = Kokkos::mdspan<double, Kokkos::extents<size_t, settings::grid_width, settings::grid_height>>;
}

#endif
