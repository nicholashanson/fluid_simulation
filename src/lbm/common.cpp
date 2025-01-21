#include <lbm/common.hpp>

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

        std::vector<std::pair<size_t, size_t>> obstacle_coords = {
                { 20, 15 },
                { 20, 16 },
                { 20, 17 },
                { 20, 18 },
                { 20, 19 },
                { 20, 20 },
                { 20, 21 },
                { 20, 22 },
                { 20, 23 },
                { 20, 24 },
                { 20, 25 },
                { 20, 26 },
                { 20, 27 },
                { 20, 28 },
                { 20, 29 },
                { 20, 30 },
                { 20, 31 },
                { 20, 32 },
                { 20, 33 },
                { 20, 34 }
        };

    } // lbm

} // fs

