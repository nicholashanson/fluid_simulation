#ifndef COMPUTE_MOMENTUM_HPP
#define COMPUTE_MOMENTUM_HPP

namespace lga {

    template<typename array, typename mdspan>
    std::pair<int, int> compute_momentum( const grid<array, mdspan>& gd ) {

        int p_x{};
        int p_y{};

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                auto [ v_x, v_y ] = gd.get_cell_state( i, j );

                p_x += v_x;
                p_y += v_y;

            }

        return { p_x, p_y };

    }

}

#endif
