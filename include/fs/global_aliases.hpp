#ifndef GLOBAL_ALIASES_HPP
#define GLOBAL_ALIASES_HPP

#ifdef DPCPP_COMPILER
#include <sycl/sycl.hpp>
#endif

#include <mdspan-stable/include/mdspan/mdspan.hpp>
#include <settings.hpp>

namespace fs {

    namespace lbm {

        using T = double;

        using D2Q9_view = Kokkos::mdspan<double, Kokkos::extents<size_t, settings::ydim, settings::xdim, 9>>;

    } // lbm

    using property_view = Kokkos::mdspan<double, Kokkos::extents<size_t, settings::ydim, settings::xdim>>;
    using vertex_view = Kokkos::mdspan<float, Kokkos::extents<size_t, settings::ydim, settings::xdim, 5*6>>;

} // fs

#endif
