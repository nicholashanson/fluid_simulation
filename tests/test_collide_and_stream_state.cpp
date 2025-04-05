#include <gtest/gtest.h>

#include <sycl/sycl.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <fs/fs.hpp>
#include <fs/lbm/collide_and_stream_dpcxx.hpp>

#include <settings.hpp>
#include <grid.hpp>

#include <vector>

const double epsilon_ = 1e-6; 

bool approx_equal_cells_( const size_t y, const size_t x, const std::array<double, 9>& stateless, const std::array<double, 9>& stateful ) {

    bool approx_equal_ = true;

    auto approx_equal = [=]( double a, double b ) {  
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

bool approx_equal_arrays( const std::vector<fs::lbm::T>& arr_1, const std::vector<fs::lbm::T>& arr_2 ) {
    
    if ( arr_1.size() != arr_2.size() ) {
        
        return false;
    }

    for ( size_t i = 0; i < arr_1.size(); ++i ) {

        if ( std::fabs( arr_1[ i ] - arr_2[ i ] ) > epsilon_ ) {
            
            return false;
        }
    }

    return true;
}

TEST( LBMTests, CollideAndStreamState ) {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> lbm_grid( fs::lbm::D2Q9_states );
    
    // get grid dimenstions
    const size_t ydim = lbm_grid.get_dim( 0 );
    const size_t xdim = lbm_grid.get_dim( 1 );

    fs::lbm::initialize_grid( lbm_grid );

    std::vector<fs::lbm::T> expected_state( lbm_grid.get_data_handle(), lbm_grid.get_data_handle() + ydim * xdim * 9 );

    unsigned char barrier[ fs::settings::ydim * fs::settings::xdim ] = { 0 };

    barrier[ 4 + 4 * fs::settings::xdim ] = 1;

    double viscosity = 0.005;

    auto state_ = ( fs::dpcxx::lbm::cs_state* )fs::dpcxx::lbm::init_cs( lbm_grid.get_data_handle(), barrier, fs::settings::ydim, fs::settings::xdim, viscosity );

    ASSERT_NE( state_, nullptr ) << "Initialization failed: 'state' is nullptr.";

    fs::dpcxx::lbm::cs_state* state = static_cast<fs::dpcxx::lbm::cs_state*>( state_ );

    EXPECT_EQ( state->ydim, ydim );
    EXPECT_EQ( state->xdim, xdim );
    EXPECT_EQ( state->vec_len, ydim * xdim );
    EXPECT_FLOAT_EQ( state->omega, 1.0 / ( 3.0 * viscosity + 0.5 ) );

    ASSERT_NE( state->d_D2Q9, nullptr );
    ASSERT_NE( state->d_D2Q9_n, nullptr );
    ASSERT_NE( state->d_obstacle, nullptr );
    ASSERT_NE( state->gpu_queue, nullptr );

    ASSERT_EQ( state->D2Q9, lbm_grid.get_data_handle() );

    fs::lbm::T* actual_state = sycl::malloc_host<fs::lbm::T>( state->vec_len * 9, *( state->gpu_queue ) ); 

    state->gpu_queue->memcpy( actual_state, state->d_D2Q9, state->vec_len * 9 * sizeof( fs::lbm::T ) );

    state->gpu_queue->wait();

    std::vector<fs::lbm::T> actual_state_v( actual_state, actual_state + state->vec_len * 9 );

    EXPECT_TRUE( approx_equal_arrays( expected_state, actual_state_v ) ) << "States are not equivalent.";

    fs::dpcxx::lbm::stateful_collide_and_stream( ( void* )state, 20 );

    fs::dpcxx::lbm::terminate_cs( state );
} 

TEST( LBMTests, CollideAndStreamStateSB ) {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> lbm_grid_0( fs::lbm::D2Q9_states );
    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> lbm_grid_1( fs::lbm::D2Q9_states );
    
    // get grid dimenstions
    const size_t ydim = lbm_grid_0.get_dim( 0 );
    const size_t xdim = lbm_grid_0.get_dim( 1 );

    fs::lbm::initialize_grid( lbm_grid_0 );
    fs::lbm::initialize_grid( lbm_grid_1 );

    unsigned char barrier[ fs::settings::ydim * fs::settings::xdim ] = { 0 };

    barrier[ 4 + 4 * fs::settings::xdim ] = 1;

    fs::lbm::T viscosity = 0.005;

    auto state_0 = ( fs::dpcxx::lbm::cs_state* )fs::dpcxx::lbm::init_cs( lbm_grid_0.get_data_handle(), barrier, fs::settings::ydim, fs::settings::xdim, viscosity );
    auto state_1 = ( fs::dpcxx::lbm::cs_state* )fs::dpcxx::lbm::init_cs( lbm_grid_1.get_data_handle(), barrier, fs::settings::ydim, fs::settings::xdim, viscosity );

    fs::dpcxx::lbm::set_grid_boundaries( ( void* )state_0 );

    bool equivalent_values = true;

    for ( size_t y = 0; y < fs::settings::ydim; ++y ) {
        for ( size_t x = 0; x < fs::settings::xdim; ++x ) {

            const auto stateful = lbm_grid_0.get_cell_state_array( y, x );
            const auto stateless = lbm_grid_1.get_cell_state_array( y, x );

            if ( !approx_equal_cells_( y, x, stateful, stateless ) ) { 

                equivalent_values = false;
            }
        }
    }

    ASSERT_TRUE( equivalent_values ) << "values are not equivalent";

    fs::dpcxx::lbm::terminate_cs( state_0 );
    fs::dpcxx::lbm::terminate_cs( state_1 );
} 

// for some reason, linking gtest_main with icpx doesn't define main
int main( int argc, char** argv ) {

    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}