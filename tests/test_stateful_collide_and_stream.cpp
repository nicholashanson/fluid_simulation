#include <gtest/gtest.h>

#include <fs/fs.hpp>

#include <settings.hpp>
#include <grid.hpp>

#include <cmath>

#include <vector>

const double epsilon_ = 1e-6; 

bool approx_equal_cells_( const size_t y, const size_t x, 
                          const std::array<fs::lbm::T, 9>& stateless, 
                          const std::array<fs::lbm::T, 9>& stateful ) {

    bool approx_equal_ = true;

    auto approx_equal = [=]( fs::lbm::T a, fs::lbm::T b ) {  
        return std::fabs( a - b ) < epsilon_;
    };

    for ( size_t k = 0; k < 9; ++k ) {

        if ( !approx_equal( stateless[ k ], stateful[ k ] ) ) {

            std::cout << "y: " << y << ", x: " << x << std::endl;

            std::cout << "stateless: " << stateless[ k ] << ", stateful: " << stateful[ k ] << std::endl;

            approx_equal_ = false;
        }
    }

    return approx_equal_;
}

TEST( LBMTests, StatefulCollideAndStream ) {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> stateful_grid( fs::lbm::D2Q9_states );
    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> stateless_grid( fs::lbm::D2Q9_states );

    fs::lbm::initialize_grid( stateful_grid );
    fs::lbm::initialize_grid( stateless_grid );

    bool equivalent_values = true;

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const auto stateful = stateful_grid.get_cell_state_array( y, x );
            const auto stateless = stateless_grid.get_cell_state_array( y, x );

            if ( !approx_equal_cells_( y, x, stateful, stateless ) ) { 

                equivalent_values = false;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values equivalent after initialization";

    /*
    fs::lbm::obstacle_coords = fs::lbm::get_airfoil_coords_aoa( 0.04, 0.4, 0.12, 0.0 );

    std::vector<unsigned char> barrier( fs::settings::ydim * fs::settings::xdim, 0 ); 

	for ( auto xy : fs::lbm::obstacle_coords ) {

	    barrier[ xy.second + xy.first * fs::settings::xdim ] = 1;
    }
    */

    unsigned char barrier[ fs::settings::xdim * fs::settings::ydim ] = { 0 };

    barrier[ 100 + 100 * fs::settings::xdim ] = 1;

    std::vector<unsigned char> barrier_v( barrier, barrier + fs::settings::xdim * fs::settings::ydim );

    double viscosity = 0.005;

    // auto state = fs::dpcxx::lbm::init_cs( stateful_grid, barrier, viscosity );

    auto state = fs::dpcxx::lbm::init_cs( stateful_grid, barrier_v, viscosity );

    ASSERT_NE( state, nullptr ) << "Initialization failed: 'state' is nullptr.";

    // get grid dimensions
    const size_t ydim = stateful_grid.get_dim( 0 );
    const size_t xdim = stateful_grid.get_dim( 1 );

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const auto stateful = stateful_grid.get_cell_state_array( y, x );
            const auto stateless = stateless_grid.get_cell_state_array( y, x );

            if ( !approx_equal_cells_( y, x, stateful, stateless ) ) { 

                equivalent_values = false;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values equivalent after state set-up";

    for ( size_t z = 0; z < 1; ++z ) {

        fs::lbm::set_boundaries( stateless_grid );

        fs::dpcxx::lbm::stateful_collide_and_stream_c( state, 20 );

        fs::dpcxx::lbm::collide_and_stream( stateless_grid, barrier, 20 );
        
        // fs::dpcxx::lbm::collide_and_stream( stateless_grid, barrier.data(), 20 );

        for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
            for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

                const auto stateful = stateful_grid.get_cell_state_array( y, x );
                const auto stateless = stateless_grid.get_cell_state_array( y, x );

                if ( !approx_equal_cells_( y, x, stateful, stateless ) ) { 

                    equivalent_values = false;
                }
            }
        }

        ASSERT_TRUE( equivalent_values ) << "values are not equivalent on the iteration " << z;
    }

    fs::dpcxx::lbm::terminate_cs_c( state );
} 
