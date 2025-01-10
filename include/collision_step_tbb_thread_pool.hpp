#ifndef LBM_COLLISION_STEP_TBB_THREAD_POOL
#define LBM_COLLISION_STEP_TBB_THREAD_POOL

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan> collision_step_tbb_thread_pool( const grid<Array, MDSpan>& gd, Array& cell_states_, const double tau ) {

        grid<Array, MDSpan> new_state( cell_states_ );

        tbb::parallel_for(
            tbb::blocked_range<size_t>( 1, gd.get_grid_height() - 1 ),
            [&]( const tbb::blocked_range<size_t>& range ) {

                for ( size_t i = range.begin(); i < range.end(); ++i )
                    for ( size_t j = 1; j < gd.get_grid_width() - 1; ++j )

                        handle_cell_collisions( gd, new_state, i, j, tau );

        });

        return new_state;

    }

}
}

#endif
