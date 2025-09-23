#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

TEST( GeometryTest, GetBoundingBox_SinglePoint ) {
    std::vector<std::pair<double,double>> points = { { 1.0, 2.0 } };
    auto box = geometry::get_bounding_box( points );
    EXPECT_EQ( box, geometry::bounding_box<double>( 1.0, 1.0, 2.0, 2.0 ) );
}

TEST( GeometryTest, GetBoundingBox_MultiplePoints ) {
    std::vector<std::pair<double,double>> points = {
        { 1.0, 2.0 }, { 3.5, -1.0 }, { -2.0, 4.0 }
    };
    auto box = geometry::get_bounding_box( points );
    EXPECT_EQ( box, geometry::bounding_box<double>( -2.0, 3.5, -1.0, 4.0 ) );
}

TEST( GeometryTest, GetBoundingBox_EmptyPoints ) {
    std::vector<std::pair<double,double>> points;
    auto box = geometry::get_bounding_box(points);
    EXPECT_EQ( box.xmin, std::numeric_limits<double>::infinity() );
    EXPECT_EQ( box.xmax, -std::numeric_limits<double>::infinity() );
    EXPECT_EQ( box.ymin, std::numeric_limits<double>::infinity() );
    EXPECT_EQ( box.ymax, -std::numeric_limits<double>::infinity() );
}