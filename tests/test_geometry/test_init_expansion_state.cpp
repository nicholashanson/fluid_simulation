#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, InitExpansionState ) {
    std::array<double,2> e = { 1.0, 0.5 };
    std::array<double,4> f = { 2.0, 0.1, 0.0, 0.0 };
    std::array<double,6> h = {};
    adaptive_arithmetic::expansion_state<double,6> state( e, f, h, 2, 2 );
    adaptive_arithmetic::init_expansion_state( state );
    adaptive_arithmetic::expansion_state<double,6> expected_state(
        e, f, h,
        2, 2,       // e_len, f_len
        0.5, 2.0,   // e_now, f_now
        1, 0,       // e_index, f_index
        0,          // h_index
        1.0, 0.0    // Q, hh
    );
    EXPECT_EQ( state, expected_state );
}