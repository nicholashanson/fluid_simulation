#include <lbm.hpp>

namespace lga {

    double calculate_feq( size_t q, double rho, double u_x, double u_y ) {

        double e_dot_u = e[ q ].first * u_x + e[ q ].second * u_y;

        // |u|^2
        double u_dot_u = u_x * u_x + u_y * u_y;

        // equilibrium distribution function
        double f_eq = w[ q ] * rho * ( 1.0 + e_dot_u / c_s2 + ( e_dot_u * e_dot_u ) / ( 2 * c_s2 * c_s2 ) - u_dot_u / ( 2 * c_s2 ) );

        return f_eq;

    }

    std::vector<std::pair<size_t, size_t>> obstacle_coords = {

            { 10, 16 },
            { 10, 17 },
            { 10, 18 },
            { 10, 19 },
            { 10, 20 },
            { 10, 21 },
            { 10, 22 },
            { 10, 23 },
            { 10, 24 },
            { 10, 25 },
            { 10, 26 },
            { 10, 27 },
            { 10, 28 },
            { 10, 29 },
            { 10, 30 },
            { 10, 31 },
            { 10, 32 },
            { 10, 33 },
            { 10, 34 }

    };

}

