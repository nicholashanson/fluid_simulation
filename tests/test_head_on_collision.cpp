#include <gtest/gtest.h>

#include <lga.hpp>

using namespace lga;

TEST( LGATests, HeadOnCollision ) {

    grid<a25, gv25> gd( state_before_collision );

    gd = propagate( gd, blank_a25 );

    grid<a25, gv25> expected_gd( state_after_collision );

    ASSERT_EQ( gd, expected_gd ) << "Head-on collision handled incorrectly";

}

