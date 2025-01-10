#ifndef COMPUTE_MASS_HPP
#define COMPUTE_MASS_HPP

namespace lga {

    template<typename array, typename mdspan>
    size_t compute_mass( const grid<array, mdspan>& gd ) {

        size_t total_mass{};

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                auto [ vx, vy ] = gd.get_cell_state( i, j );

                if ( vx != 0 || vy != 0 )  // a non-zero velocity indicates a particle exists

                    total_mass++;

            }

        return total_mass;

    }

}

#endif
