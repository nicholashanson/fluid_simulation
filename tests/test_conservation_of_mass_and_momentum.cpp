#include <gtest/gtest.h>

TEST( LGATests, ConservationOfMass ) {

    grid<std::vector<velocity>, grid_view> gd( cell_states );

    random_seed( gd );

    auto initial_mass = compute_total_mass( gd );

    evolve( gd );

    auto final_mass = compute_total_mass( gd );

    ASSERT_EQ( initial_mass, final_mass ) << "Mass is not conserved";

}

