#include <gtest/gtest.h>

#include <lga.hpp>

using namespace lga;

TEST( LGATests, ConservationOfMass ) {

    grid<std::vector<velocity>, grid_view> gd( cell_states );

    random_seed( gd );

    auto initial_mass = compute_mass( gd );

    propagate( gd, cell_states );

    auto final_mass = compute_mass( gd );

    ASSERT_EQ( initial_mass, final_mass ) << "Mass is not conserved";

}


