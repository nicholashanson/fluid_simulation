#include <gtest/gtest.h>

#include <sycl/sycl.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <fs/fs.hpp>
#include <fs/lbm/collide_and_stream_dpcxx.hpp>
#include <settings.hpp>
#include <grid.hpp>

#include <vector>

TEST( LBMTests, CollideAndStreamState ) {

    sim::grid<std::vector<double>, fs::lbm::D2Q9_view> lbm_grid( fs::lbm::D2Q9_states );

    unsigned char barrier[ fs::settings::ydim * fs::settings::xdim ] = { 0 };

    barrier[ 4 + 4 * fs::settings::xdim ] = 1;

    double viscosity = 0.05;

    fs::dpcxx::lbm::cs_state* state = ( fs::dpcxx::lbm::cs_state* )fs::dpcxx::lbm::init_cs( lbm_grid.get_data_handle(), barrier, fs::settings::ydim, fs::settings::xdim, viscosity );

    // get grid dimenstions
    const size_t ydim = lbm_grid.get_dim( 0 );
    const size_t xdim = lbm_grid.get_dim( 1 );

    EXPECT_EQ( state->ydim, ydim );
    EXPECT_EQ( state->xdim, xdim );
    EXPECT_EQ( state->vec_len, ydim * xdim );
    EXPECT_FLOAT_EQ( state->omega, 1.0 / ( 3.0 * viscosity + 0.5 ) );

    ASSERT_NE( state->d_D2Q9, nullptr );
    ASSERT_NE( state->d_D2Q9_n, nullptr );
    ASSERT_NE( state->d_obstacle, nullptr );
} 

// for some reason, linking gtest_main with icpx doesn't define main
int main( int argc, char** argv ) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}