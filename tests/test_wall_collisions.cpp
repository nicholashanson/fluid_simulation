#include <gtest/gtest.h>

#include <lga.hpp>

using namespace lga;

TEST( LGATests, WallCollisions ) {

    grid<a25, gv25> gd( state_before_flushing );

    gd.print();

    flush_walls( gd );

    gd.print();

    grid<a25, gv25> expected_gd( state_after_flushing );

    ASSERT_EQ( gd, expected_gd ) << "Wall collisions handled incorrectly";

}

