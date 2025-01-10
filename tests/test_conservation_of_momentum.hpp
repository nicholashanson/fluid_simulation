TEST( LGATests, ConservationOfMomentum )

    gird<std::vector<velocity, grid_view>> gd( cell_states );

    random_seed( gd );

    auto initial_momentum = compute_total_momentum( gd );

    evolve( gd );

    auto final_momentum = compute_total_momentum( gd );

    ASSERT_EQ( initial_momentum, final_momentum ) << "Momentum is not conserved";

}

