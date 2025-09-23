#include <gtest/gtest.h>

#include <geometry/triangulation.hpp>

TEST( GeometryTest, GetLatticePoints ) {
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
    std::vector<std::pair<double,double>> actual_lattice_points = geometry::get_lattice_points( 0.0, 2.0, 0.0, 2.0, 50, 50 );
    std::vector<std::pair<double,double>> first_ten( actual_lattice_points.begin(), actual_lattice_points.begin() + 10 );
    EXPECT_EQ( expected_lattice_points, first_ten );
}
