#ifndef LBM_HPP
#define LBM_HPP

#include <cstddef>

#include <fs/global_aliases.hpp>

#include <fs/lbm/common.hpp>

#if !defined(DPCPP_COMPILER)
#include <fs/lbm/collide_and_stream_dpcxx_interface.hpp>
#include <fs/lbm/initialize_grid.hpp>
#include <fs/lbm/property.hpp>
#endif

#endif
