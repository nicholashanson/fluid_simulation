#ifndef LBM_STREAMING_STEP_HPP
#define LBM_STREAMING_STEP_HPP

#include <lbm_global_data.hpp>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    void stream_inlet( grid<Array, MDSpan>& gd, grid<Array, MDSpan>& new_state ) {

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i )
            for ( size_t q = 0; q < 6; ++q ) {

                    size_t neighbor_i = i + e[ q ].second;
                    size_t neighbor_j = e[ q ].first;

                    new_state.set_cell_state( gd.get_cell_state( i, 0zu, q ), neighbor_i, neighbor_j, q );
            }

    }

    template<typename Array, typename MDSpan>
    void stream_tunnel( grid<Array, MDSpan>& gd, grid<Array, MDSpan>& new_state ) {

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i )
            for ( size_t j = 1; j < gd.get_grid_width() - 1; ++ j )

                for ( size_t q = 0; q < 9; ++q ) {

                    size_t neighbor_i = i + e[ q ].second;
                    size_t neighbor_j = j + e[ q ].first;

                    new_state.set_cell_state( gd.get_cell_state( i, j, q ), neighbor_i, neighbor_j, q );
                }

    }

    template<typename Array, typename MDSpan>
    void stream_outlet( const grid<Array, MDSpan>& gd, grid<Array, MDSpan>& new_state ) {

        const size_t last_column_index = gd.get_grid_width() - 1;

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i ) {

            for ( size_t q = 0; q < 9; ++q ) {

                if ( e[ q ].first < 1 ) {

                    auto neighbor_x = last_column_index + e[ q ].first;
                    auto neighbor_y = i + e[ q ].second;

                    new_state.set_cell_state( gd.get_cell_state( i, last_column_index, q ), neighbor_y, neighbor_x, q );

                }

            }

        }

    }

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan> streaming_step( grid<Array, MDSpan>& gd, Array& cell_states_ ) {

        grid<Array, MDSpan> new_state( cell_states_ );

        stream_inlet( gd, new_state );

        stream_tunnel( gd, new_state );

        stream_outlet( gd, new_state );

        return new_state;

    }

}

}

#endif
