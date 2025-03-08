#include <gtest/gtest.h>

#include <fs/fs.hpp>
#include <fs/lbm/js.hpp>
#include <settings.hpp>
#include <grid.hpp>
#include <iostream>
#include <functional>

#include <vector>
#include <array>

const double epsilon = 1e-6; 

bool approx_equal_cells( const size_t y, const size_t x, const std::array<double, 9>& dpcxx_state, const std::array<double, 9>& tbb_state ) {

    bool approx_equal_ = true;

    auto approx_equal = [=]( double a, double b ) {
        return std::fabs( a - b ) < epsilon;
    };

    // an array of references to all the flat FD arrays for cleaner code
    std::array<std::reference_wrapper<std::array<double, fs::settings::ydim * fs::settings::xdim>>, 9> ni = {
        js::n0, js::nE, js::nN, js::nW, js::nS, js::nNE, js::nNW, js::nSW, js::nSE
    };

    for ( size_t k = 0; k < 9; ++k ) {
        if ( !approx_equal( ni[ k ].get()[ x + y * fs::settings::xdim ], tbb_state[ k ] ) ||
             !approx_equal( ni[ k ].get()[ x + y * fs::settings::xdim ], dpcxx_state[ k ] ) ) {

            approx_equal_ = false;
        }
    }

    return approx_equal_;
}

TEST( LBMTests, CollideAndStreamEquivalence ) {

    js::initFluid();

    const size_t barrier_size = 1;

    unsigned char obstacle[ fs::settings::ydim * fs::settings::xdim ] = { 0 };

    obstacle[ 4 + 4 * fs::settings::xdim ] = 1;

    /*
    for ( size_t y = ( fs::settings::ydim / 2 ) - barrier_size; y <= ( fs::settings::ydim / 2 ) + barrier_size; y++ ) {
		size_t x = fs::settings::ydim / 3;
		obstacle[ x + y * fs::settings::xdim ] = 1;
	}
    */

    const size_t steps = 20;

    auto approx_equal = [=]( double a, double b ) {
        return std::fabs( a - b ) < epsilon;
    };

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> grid_tbb( fs::lbm::D2Q9_states );
    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> grid_dpcxx( fs::lbm::D2Q9_states );

    fs::lbm::initialize_grid( grid_tbb );
    fs::lbm::initialize_grid( grid_dpcxx );

    bool equivalent_values = true;

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const std::array<double, 9> tbb_state = grid_tbb.get_cell_state_array( y, x );
            const std::array<double, 9> dpcxx_state = grid_dpcxx.get_cell_state_array( y, x );

            if ( !approx_equal_cells( y, x, dpcxx_state, tbb_state ) ) { 

                equivalent_values = false;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values equivalent after initialization";

    js::initObstacle();
    js::setBoundaries();
    
    for ( size_t z = 0; z < steps; ++z ) {

        js::collide();
        js::stream();
    }

    js::setBoundaries();

    fs::lbm::collide_and_stream_tbb( grid_tbb.get_data_handle(), obstacle, steps );
    fs::dpcxx::lbm::collide_and_stream( grid_dpcxx, obstacle, steps );

    fs::lbm::set_boundaries( grid_tbb );
    fs::lbm::set_boundaries( grid_dpcxx );

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const std::array<double, 9> tbb_state = grid_tbb.get_cell_state_array( y, x );
            const std::array<double, 9> dpcxx_state = grid_dpcxx.get_cell_state_array( y, x );

            if ( !approx_equal_cells( y, x, dpcxx_state, tbb_state ) ) { 

                equivalent_values = false;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values are not equivalent";
    ASSERT_EQ( grid_tbb, grid_dpcxx ) << "collide-and-stream results are inconsistent";
} 