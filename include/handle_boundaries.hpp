#ifndef LBM_HANDLE_BOUNDARIES_HPP
#define LBM_HANDLE_BOUNDARIES_HPP

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    void bounce_back( grid<Array, MDSpan>& gd, const size_t i, const size_t j ) {

        for ( size_t q = 0; q < 9; ++q )
            if ( gd.get_cell_state( i, j, q ) != 0.0 ) {

                auto neighbor_x = j - e[ q ].first;
                auto neighbor_y = i - e[ q ].second;

                size_t opposite_q_index = opposite_q[ q ];

                gd.set_cell_state( gd.get_cell_state( i, j, q ), neighbor_y, neighbor_x, opposite_q_index );

                gd.set_cell_state( 0zu, i, j, q );

            }

    }

    template<typename Array, typename MDSpan>
    void apply_obstacle_boundary( grid<Array, MDSpan>& gd ) {

        for ( auto xy : obstacle_coords ) {

            auto [ x, y ] = xy;

            bounce_back( gd, y, x );

        }

    }

    template<typename Array, typename MDSpan>
    void flush_tunnel_walls( grid<Array, MDSpan>& gd ) {

        const size_t bottom_wall_index = gd.get_grid_height() - 1;

        for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

            bounce_back( gd, 0zu, j );
            bounce_back( gd, bottom_wall_index, j );

        }

    }

}
}

#endif
