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

// init same obstacle in test target

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

    std::cout << "yxxy" << std::endl;

    const size_t steps = 20;

    auto approx_equal = [=]( double a, double b ) {
        return std::fabs( a - b ) < epsilon;
    };

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> grid_tbb( fs::lbm::D2Q9_states );
    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> grid_dpcxx( fs::lbm::D2Q9_states );

    fs::lbm::initialize_grid( grid_tbb );
    fs::lbm::initialize_grid( grid_dpcxx );

    std::array<double, 9> cell_state_0 = grid_dpcxx.get_cell_state_array( 0, 0 );

    std::cout << "dpcxx: ";
    for ( auto x: cell_state_0 ) {
        std::cout << x << ", ";
    }
    std::cout << std::endl;

    bool equivalent_values = true;

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const std::array<double, 9> tbb_state = grid_tbb.get_cell_state_array( y, x );
            const std::array<double, 9> dpcxx_state = grid_dpcxx.get_cell_state_array( y, x );

            if ( !approx_equal_cells( y, x, dpcxx_state, tbb_state ) ) { 

                equivalent_values = false;

                std::cout << "x: " << x << std::endl;
                std::cout << "y: " << y << std::endl;

                std::cout << js::n0[ x + y * fs::settings::xdim ] << ", " << js::nE[ x + y * fs::settings::xdim ] << ", " 
                    << js::nN[ x + y * fs::settings::xdim ] << ", "<< js::nW[ x + y * fs::settings::xdim ] << ", " 
                    << js::nS[ x + y * fs::settings::xdim ] << ", " << js::nNE[ x + y * fs::settings::xdim ] << ", " 
                    << js::nNW[ x + y * fs::settings::xdim ] << ", " << js::nSW[ x + y * fs::settings::xdim ] << ", " 
                    << js::nSE[ x + y * fs::settings::xdim ] << ", " << std::endl;

                std::cout << "tbb: ";
                for ( auto x : tbb_state ) {
                    std::cout << x << ", ";
                }
                std::cout << std::endl;

                std::cout << "dpcxx: ";
                for ( auto x : dpcxx_state ) {
                    std::cout << x << ", ";
                }
                std::cout << std::endl;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values equivalent after initialization";

    js::initObstacle();
    js::setBoundaries();
    
    for ( size_t z = 0; z < steps; ++z ) {

        js::collide();

        js::stream();

        /*
        if ( z == 1 ) 
            js::stream( true );
        else
            js::stream();
        */
    }

    js::setBoundaries();

    std::cout << js::n0[ 0 ] << ", " << js::nE[ 0 ] << ", " 
        << js::nN[ 0 ] << ", "<< js::nW[ 0 ] << ", " 
        << js::nS[ 0 ] << ", " << js::nNE[ 0 ] << ", " 
        << js::nNW[ 0 ] << ", " << js::nSW[ 0 ] << ", " 
        << js::nSE[ 0 ] << ", " << std::endl;

    std::cout << "getting ready" << std::endl;

    fs::dpcxx::lbm::collide_and_stream_tbb( grid_tbb, obstacle, steps );

    std::cout << "finished dpcxx" << std::endl;

    fs::dpcxx::lbm::collide_and_stream( grid_dpcxx, obstacle, steps );

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {

        fs::lbm::set_velocity( grid_tbb, y, 0, 0.1, 0.0 );
        fs::lbm::set_velocity( grid_tbb, y, fs::settings::xdim - 1, 0.1, 0.0 );

        fs::lbm::set_velocity( grid_dpcxx, y, 0, 0.1, 0.0 );
        fs::lbm::set_velocity( grid_dpcxx, y, fs::settings::xdim - 1, 0.1, 0.0 );
    } 

    for ( size_t x = 0; x < fs::settings::xdim; ++x ) {
        
        fs::lbm::set_velocity( grid_tbb, 0, x, 0.1, 0.0 );
        fs::lbm::set_velocity( grid_tbb, fs::settings::ydim - 1, x, 0.1, 0.0 );

        fs::lbm::set_velocity( grid_dpcxx, 0, x, 0.1, 0.0 );
        fs::lbm::set_velocity( grid_dpcxx, fs::settings::ydim - 1, x, 0.1, 0.0 );
    }

    // grid_tbb.reverse_rows();
    // grid_dpcxx.reverse_rows();

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const std::array<double, 9> tbb_state = grid_tbb.get_cell_state_array( y, x );
            const std::array<double, 9> dpcxx_state = grid_dpcxx.get_cell_state_array( y, x );

            // std::cout << "n0: " << js::n0[ x + y * fs::settings::xdim ] << std::endl;
            // std::cout << "tbb: " << tbb_state[ 0 ] << std::endl; 
            // std::cout << "dpcxx: " << dpcxx_state[ 0 ] << std::endl;

            if ( !approx_equal_cells( y, x, dpcxx_state, tbb_state ) ) { 

                equivalent_values = false;

                std::cout << "x: " << x << std::endl;
                std::cout << "y: " << y << std::endl;

                std::cout << js::n0[ x + y * fs::settings::xdim ] << ", " << js::nE[ x + y * fs::settings::xdim ] << ", " 
                    << js::nN[ x + y * fs::settings::xdim ] << ", "<< js::nW[ x + y * fs::settings::xdim ] << ", " 
                    << js::nS[ x + y * fs::settings::xdim ] << ", " << js::nNE[ x + y * fs::settings::xdim ] << ", " 
                    << js::nNW[ x + y * fs::settings::xdim ] << ", " << js::nSW[ x + y * fs::settings::xdim ] << ", " 
                    << js::nSE[ x + y * fs::settings::xdim ] << ", " << std::endl;

                std::cout << "tbb: ";
                for ( auto x : tbb_state ) {
                    std::cout << x << ", ";
                }
                std::cout << std::endl;

                std::cout << "dpcxx: ";
                for ( auto x : dpcxx_state ) {
                    std::cout << x << ", ";
                }
                std::cout << std::endl;
            }
        }
    }

    cell_state_0 = grid_dpcxx.get_cell_state_array( 0, 0 );

    std::cout << "dpcxx: ";
    for ( auto x: cell_state_0 ) {
        std::cout << x << ", ";
    }
    std::cout << std::endl;

    cell_state_0 = grid_tbb.get_cell_state_array( 0, 0 );

    std::cout << "tbb: ";
    for ( auto x: cell_state_0 ) {
        std::cout << x << ", ";
    }
    std::cout << std::endl;

    std::cout << "setps: " << steps << std::endl;

    ASSERT_TRUE( equivalent_values ) << "values are not equivalent";
    ASSERT_EQ( grid_tbb, grid_dpcxx ) << "collide-and-stream results are inconsistent";
} 