#ifndef SET_INLET_HPP
#define SET_INLET_HPP

#include <lbm_global_data.hpp>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    void set_velocity( grid<Array, MDSpan>& gd, const size_t i, const size_t j, const double u_x, const double u_y ) {

        for ( size_t q = 0; q < 9; ++ q ) {

            double f_eq = calculate_f_eq( q, 1.0, u_x, u_y );

            gd.set_cell_state( f_eq, i, j, q );

        }

    }

    template<typename Array, typename MDSpan>
    void set_tunnel_inlet( grid<Array, MDSpan>& gd, const double v_x, const double v_y ) {

        for ( size_t i = 1; i < gd.get_grid_height() - 1; ++i )
            set_velocity( gd, i, 0zu, v_x, v_y );

    }

}
}

#endif
