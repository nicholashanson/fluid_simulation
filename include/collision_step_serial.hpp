#ifndef LBM_COLLISION_STEP_SERIAL_HPP
#define LBM_COLLISION_STEP_SERIAL_HPP

#include <collision_step.hpp>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan> collision_step( const grid<Array, MDSpan>& gd, Array& cell_states_, const double tau ) {

        grid<Array, MDSpan> new_state( cell_states_ );

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i )
            for ( size_t j = 1; j < gd.get_grid_width() - 1; ++j )

                handle_cell_collisions( gd, new_state, i, j, tau );

        return new_state;

    }

}
}

#endif
