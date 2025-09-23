#include <gtest/gtest.h>

#include <geometry/geometry.hpp>

TEST( GeometryTest, GetOrient ) {
    using dp = std::pair<double,double>;
    // the triangle ( p, q, r ) is positively oriented
    // the triangle ( p, r, q ) is negatively oriented
    dp p( 0.0, 0.0 );
    dp q( 1.0, 0.0 );
    dp r( 0.0, 1.0 );
    auto expected_positive = geometry::get_orient( p, q, r );       
    auto expected_negative = geometry::get_orient( p, r, q );        
    ASSERT_EQ( expected_positive, geometry::orient::positive );
    ASSERT_EQ( expected_negative, geometry::orient::negative );  
    p = { 0.0, 0.0 }; // make the vertices of the triangle co-linear
    q = { 0.0, 1.0 }; // so that get_orient returns orient::DEGENERATE
    r = { 0.0, 2.0 };
    auto expected_degenerate = geometry::get_orient( p, q, r );
    ASSERT_EQ( expected_degenerate, geometry::orient::degenerate );  
}