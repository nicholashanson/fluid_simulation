#include <gtest/gtest.h>

#include <lga.hpp>

using namespace lga;

TEST( LGATests, ConservationOfMomentum ) {

    grid<std::vector<velocity>, grid_view> gd( cell_states );

    random_seed( gd );

    auto initial_momentum = compute_momentum( gd );

    gd = propagate( gd, cell_states );

    auto final_momentum = compute_momentum( gd );

    ASSERT_EQ( initial_momentum, final_momentum ) << "Momentum is not conserved";

}
