#include <fs/lbm/common.hpp>
#include <settings.hpp>

namespace fs {

    namespace lbm {

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