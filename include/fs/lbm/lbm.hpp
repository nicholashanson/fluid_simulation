#ifndef LBM_HPP
#define LBM_HPP

#include <cstddef>

#include <fs/global_aliases.hpp>
#include <fs/lbm/common.hpp>

#include <fs/lbm/collide_and_stream_tbb.hpp>

#if !defined(DPCPP_COMPILER)

#ifdef GPU
#include <fs/lbm/collide_and_stream_dpcxx_interface.hpp>
#endif // GPU

#include <fs/lbm/initialize_grid.hpp>
#include <fs/lbm/property.hpp>

#endif // !DPCPP_COMPILER

#endif // LBM_HPP

