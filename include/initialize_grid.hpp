#ifndef LBM_INITIALIZE_GRID_HPP
#define LBM_INITIALIZE_GRID_HPP

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    void initialize_grid( grid<Array, MDSpan>& gd ) {

        constexpr double initial_density = 0.1;

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j )
                for ( size_t q = 0; q < 9; ++q ) {
                    double f_eq = w[q] * initial_density;
                    gd.set_cell_state( f_eq, i, j, q );
                }

    }

}
}

#endif



