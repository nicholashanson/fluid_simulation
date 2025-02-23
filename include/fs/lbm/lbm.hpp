#ifndef LBM_HPP
#define LBM_HPP

#include <cstddef>

#include <fs/global_aliases.hpp>

#include <fs/lbm/common.hpp>

#if !defined(DPCPP_COMPILER)
#include <fs/lbm/grid_to_vertex_data_interface.hpp>
#include <fs/lbm/collide_and_stream_dpcxx_interface.hpp>
#include <fs/lbm/collision_step_tbb_thread_pool.hpp>
#include <fs/lbm/collision_step_serial.hpp>
//#include <fs/lbm/streaming_step.hpp>
//#include <fs/lbm/set_inlet.hpp>
#include <fs/lbm/initialize_grid.hpp>
//#include <fs/lbm/handle_boundaries.hpp>
#include <fs/lbm/collision_step.hpp>
//#include <fs/lbm/velocity.hpp>
#include <fs/lbm/property.hpp>
#endif

#endif
