#ifndef LBM_COLLISION_STEP_HPP
#define LBM_COLLISION_STEP_HPP

#include <lbm_global_data.hpp>
#include <grid.hpp>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    void calculate_ux_uy_rho( const grid<Array, MDSpan>& gd, const size_t i, const size_t j,
                               double& rho, double& u_x, double& u_y ) {

        for ( size_t q = 0; q < 9; ++q ) {

            double f_q = gd.get_cell_state( i, j, q );

            rho += f_q;

            u_x += e[ q ].first * f_q;
            u_y += e[ q ].second * f_q;
        }

    }

    template<typename Array, typename MDSpan>
    double calculate_f_current( const grid<Array, MDSpan>& gd, const size_t i, const size_t j, const size_t q,
                                 double rho, double u_x, double u_y, const double tau ) {

        double f_eq = calculate_f_eq( q, rho, u_x, u_y );

        double f_prev = gd.get_cell_state( i, j, q );

        double f_current = f_prev + ( 1.0 / tau ) * ( f_eq - f_prev );

        if ( f_current < 0.0 )
            f_current = 0.0;

        return f_current;

    }

    template<typename Array, typename MDSpan>
    void update_f( const grid<Array, MDSpan>& gd, grid<Array, MDSpan>& new_state, const size_t i, const size_t j,
                   double rho, double u_x, double u_y, const double tau ) {

        for ( size_t q = 0; q < 9; ++q ) {

            double f_current = calculate_f_current( gd, i, j, q, rho, u_x, u_y, tau );

            new_state.set_cell_state( f_current, i, j, q );
        }

    }

    template<typename Array, typename MDSpan>
    void handle_cell_collisions( const grid<Array, MDSpan>& gd, grid<Array, MDSpan>& new_state, const size_t i, const size_t j, const double tau ) {

        double rho{};
        double u_x{};
        double u_y{};

        calculate_ux_uy_rho( gd, i, j, rho, u_x, u_y );

        if ( rho < 1e-6 )
            rho = 1e-6;

        u_x /= rho;
        u_y /= rho;

        update_f( gd, new_state, i, j, rho, u_x, u_y, tau );

    }

}
}

#endif
