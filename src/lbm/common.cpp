#include <fs/lbm/common.hpp>

namespace fs {

    namespace lbm {

        std::set<std::pair<size_t, size_t>> obstacle_coords;

        std::vector<std::pair<size_t, size_t>> obstacle_coords_line = {
            { 3, 3 }
        };

        const std::vector<double> D2Q9_states( settings::ydim * settings::xdim * 9, 0.0 );

        const std::vector<double> property_states( settings::ydim * settings::xdim, 0.0 );

        double calculate_f_eq( size_t q, double rho, double u_x, double u_y ) {

            double e_dot_u = e[ q ].first * u_x + e[ q ].second * u_y;

            // |u|^2
            double u_dot_u = u_x * u_x + u_y * u_y;

            // equilibrium distribution function
            double f_eq = w[ q ] * rho * ( 1.0 + e_dot_u / c_s2 + ( e_dot_u * e_dot_u ) / ( 2 * c_s2 * c_s2 ) - u_dot_u / ( 2 * c_s2 ) );

            return f_eq;
        }

    } // lbm

} // fs

