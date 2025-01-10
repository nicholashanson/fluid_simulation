#ifndef FS_HPP
#define FS_HPP

#include <grid.hpp>
#include <lga.hpp>
#include <lbm.hpp>

#include <colors.hpp>
#include <shader.hpp>
#include <grid_renderer.hpp>

namespace fs {

    extern std::vector<velocity> cell_states;
    extern std::vector<double> cell_states_;
    extern std::vector<double> vec_mag_states;
    extern a25 blank_a25;
    extern a25 state_before_collision;
    extern a25 state_after_collision;
    extern a25 state_before_flushing;
    extern a25 state_after_flushing;
    extern lb_a25 blank_lb_a25;
    extern lb_a25 tunnel_initial_state;

}

#endif
