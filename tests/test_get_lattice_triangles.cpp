#include <gtest/gtest.h>

#include <fs/fvm/triangulation.hpp>

#include <ranges>

#include <iomanip>

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

    std::vector<std::pair<double,double>> lattice_points = fs::fvm::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );

    for ( auto [ index, tri ] : actual_triangles | std::views::enumerate ) {
        first_ten.insert( tri );
        if ( index >= 9 ) break;
    }

    EXPECT_EQ( expected_triangles, first_ten );
} 

TEST( FVMTests, GetLatticePoints ) {

    std::vector<std::pair<double,double>> expected_lattice_points = {         
        {                 0.0, 0.0 }, 
        { 0.04081632653061224, 0.0 }, 
        { 0.08163265306122448, 0.0 }, 
        { 0.12244897959183673, 0.0 },  
        { 0.16326530612244897, 0.0 }, 
        {  0.2040816326530612, 0.0 }, 
        { 0.24489795918367346, 0.0 }, 
        {  0.2857142857142857, 0.0 }, 
        { 0.32653061224489793, 0.0 }, 
        { 0.36734693877551017, 0.0 },
    };

    std::vector<std::pair<double,double>> actual_lattice_points = fs::fvm::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );
    
    /*
    for ( size_t k = 0; k < 10; ++k ) {
        std::cout << std::setprecision( 16 ) << actual_lattice_points[ k ].first << ", " 
                  << std::setprecision( 16 ) << actual_lattice_points[ k ].second << std::endl;
    }
    */

    std::vector<std::pair<double,double>> first_ten( actual_lattice_points.begin(), actual_lattice_points.begin() + 10 );

    EXPECT_EQ( expected_lattice_points, first_ten );
} 

TEST( FVMTests, NumBoundaryEdges ) {

    auto lattice_boundary = fs::fvm::get_lattice_boundary( 50, 50 );

    EXPECT_EQ( lattice_boundary.size(), 196 );
}

TEST( FVMTests, MultipleCurves ) {

    auto lattice_boundary = fs::fvm::get_lattice_boundary( 50, 50 );

    auto tri = fs::fvm::triangulate_rectangle<int,double>( 0.0, 2.0, 0.0, 2.0, 50, 50 );

    EXPECT_FALSE( fs::fvm::has_multiple_sections( lattice_boundary ) );
}

