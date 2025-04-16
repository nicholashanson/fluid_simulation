#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

#include <ranges>

TEST( FVMTests, GetLatticeTriangles ) {

    fs::fvm::triangle_set expected_triangles = {         
        {  1,  2, 51 }, 
        {  2,  3, 52 }, 
        {  3,  4, 53 }, 
        {  4,  5, 54 },  
        {  5,  6, 55 }, 
        {  6,  7, 56 }, 
        {  7,  8, 57 }, 
        {  8,  9, 58 }, 
        {  9, 10, 59 }, 
        { 10, 11, 60 }
    };

    fs::fvm::triangle_set actual_triangles = fs::fvm::get_lattice_triangles( 50, 50 );

    fs::fvm::triangle_set first_ten;

    for ( auto [ index, tri ] : actual_triangles | std::views::enumerate ) {
        first_ten.insert( tri );
        if ( index >= 9 ) break;
    }

    EXPECT_EQ( expected_triangles, first_ten );
} 