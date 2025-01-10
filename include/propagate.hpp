#ifndef PROPAGATE_HPP
#define PROPAGATE_HPP

#include <array>
#include <vector>
#include <cmath>
#include <grid.hpp>
#include <global_aliases.hpp>

namespace fs {
namespace lga {

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan> propagate( grid<Array, MDSpan>& gd, Array& cell_states_ ) {

        grid<Array, MDSpan> new_state( cell_states_ );

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                auto [ v_x, v_y ] = gd.get_cell_state( i, j );

                if ( new_state.get_cell_state( i + v_y, j + v_x ) != std::pair<int,int>{ 0, 0 } )
                    handle_collision( gd, i, j, v_x, v_y );
                else
                    new_state.set_cell_state( { v_x, v_y }, i + v_y, j + v_x );

            }

        return new_state;

    }

    template<typename Array, typename MDSpan>
    void handle_collision( grid<Array, MDSpan>& gd, size_t i, size_t j, int vx, int vy ) {

        auto [ vx_0, vy_0 ] = gd.get_cell_state( i + vy, j + vx );

        size_t x = j + vx;
        size_t y = i + vy;

        if ( vx_0 == -vx && vy_0 == -vy ) {

            gd.set_cell_state( { vy_0, vx_0 }, y + vx_0, x + vy_0 );
            gd.set_cell_state( { vy, vx }, y + vx, x + vy );
            gd.set_cell_state( { 0, 0 }, y, x );

        }

    }

    template<typename Array, typename MDSpan>
    void flush_walls( grid<Array, MDSpan>& gd ) {

        for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {
            if ( gd.get_cell_state( 0, j ) != std::pair<int,int>{ 0, 0 } ) {

                auto [ v_x, v_y ] = gd.get_cell_state( 0, j );


                if ( gd.get_cell_state( 1, j ) == std::pair<int,int>{ 0, 0 } ) {
                    gd.set_cell_state( { v_x, -v_y }, 1, j );
                    gd.set_cell_state( { 0, 0 }, 0, j );
                } else
                    handle_collision( gd, 0, j, v_x, -v_y );

            }

            const size_t last_row_index = gd.get_grid_height() - 1;

            if ( gd.get_cell_state( last_row_index, j ) != std::pair<int,int>{ 0, 0 } ) {

                auto [ v_x, v_y ] = gd.get_cell_state( last_row_index, j );

                if ( gd.get_cell_state( last_row_index - 1, j ) == std::pair<int,int>{ 0, 0 } ) {
                    gd.set_cell_state( { v_x, -v_y }, last_row_index - 1, j );
                    gd.set_cell_state( { 0, 0 }, last_row_index, j );
                } else
                    handle_collision( gd, last_row_index, j, v_x, -v_y );

            }

        }

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i ) {
            if ( gd.get_cell_state( i, 0 ) != std::pair<int,int>{ 0, 0 } ) {

                auto [ v_x, v_y ] = gd.get_cell_state( i, 0 );


                if ( gd.get_cell_state( i, 1 ) == std::pair<int,int>{ 0, 0 } ) {
                    gd.set_cell_state( { -v_x, v_y }, i, 1 );
                    gd.set_cell_state( { 0, 0 }, i, 0 );
                } else
                    handle_collision( gd, i, 1, -v_x, v_y );

            }

            const size_t last_column_index = gd.get_grid_width() - 1;

            if ( gd.get_cell_state( i, last_column_index ) != std::pair<int,int>{ 0, 0 } ) {

                auto [ v_x, v_y ] = gd.get_cell_state( i, last_column_index );

                if ( gd.get_cell_state( i, last_column_index - 1 ) == std::pair<int,int>{ 0, 0 } ) {
                    gd.set_cell_state( { -v_x, v_y }, i, last_column_index - 1 );
                    gd.set_cell_state( { 0, 0 }, i, last_column_index );
                } else
                    handle_collision( gd, i, last_column_index, -v_x, v_y );

            }

        }

    }

}
}

#endif
