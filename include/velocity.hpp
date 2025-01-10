#ifndef VELOCITY_HPP
#define VELOCITY_HPP

#include <lbm.hpp>
#include <grid.hpp>

namespace fs {
namespace lbm {

    template<typename Array, typename MDSpan>
    double velocity_magnitude( const grid<Array, MDSpan>& gd, const size_t i, const size_t j ) {

        double u_x{};
        double u_y{};

        for ( size_t q = 0; q < 9; ++q ) {

            double f = gd.get_cell_state( i, j, q );

            u_x += e[ q ].first * f;
            u_y += e[ q ].second * f;

        }

        return std::sqrt( u_x * u_x + u_y * u_y );

    }

    template<typename Array, typename MDSpan>
    grid<std::vector<double>, vec_mag_view> calculate_velocities( const grid<Array, MDSpan>& gd, std::vector<double>& vec_mag_states_ ) {

        grid<std::vector<double>, vec_mag_view> v_grid( vec_mag_states_ );

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                double velocity = velocity_magnitude( gd, i, j );

                v_grid.set_cell_state( velocity, i, j );

            }

        return v_grid;

    }

    template<typename Array, typename MDSpan>
    std::tuple<double, grid<std::vector<double>, vec_mag_view>>
    calculate_velocities_with_max( const grid<Array, MDSpan>& gd, std::vector<double>& vec_mag_states_ ) {

        grid<std::vector<double>, vec_mag_view> v_grid( vec_mag_states_ );

        double max_velocity{};

        for ( size_t i = 0; i < gd.get_grid_height(); ++i )
            for ( size_t j = 0; j < gd.get_grid_width(); ++j ) {

                double velocity = velocity_magnitude( gd, i, j );

                if ( velocity > max_velocity )
                    max_velocity = velocity;

                v_grid.set_cell_state( velocity, i, j );

            }

        return std::make_tuple( max_velocity, std::move( v_grid ) );

    }

}
}

#endif
