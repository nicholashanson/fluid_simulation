#include <gtest/gtest.h>

#include <lga.hpp>

using namespace lga;

TEST( LGATests, TunnelInitialState ) {

    grid<lb_a25, lb_gv25> gd( blank_lb_a25 );

    set_tunnel_inlet( gd, 0.01, 0.0 );

    gd.print();

    grid<lb_a25, lb_gv25> expected_gd( tunnel_initial_state );

    expected_gd.print();

    ASSERT_EQ( gd, expected_gd ) << "Tunnel inlet initialized incorrectly";

}
