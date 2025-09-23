#include <gtest/gtest.h>

#include <ranges>

#include <geometry/triangulation.hpp>

TEST( GeometryTest, GetLatticeTriangles ) {
    geometry::triangle_set expected_triangles = {         
        {  0,  1, 50 }, 
        {  1,  2, 51 }, 
        {  2,  3, 52 }, 
        {  3,  4, 53 },  
        {  4,  5, 54 }, 
        {  5,  6, 55 }, 
        {  6,  7, 56 }, 
        {  7,  8, 57 }, 
        {  8,  9, 58 }, 
        {  9, 10, 59 }
    };
    geometry::triangle_set actual_triangles = geometry::get_lattice_triangles( 50, 50 );
    geometry::triangle_set first_ten;
    for ( auto [ index, tri ] : actual_triangles | std::views::enumerate ) {
        first_ten.insert( tri );
        if ( index >= 9 ) break;
    }
    EXPECT_EQ( expected_triangles, first_ten );
} 


