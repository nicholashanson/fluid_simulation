#include <gtest/gtest.h>

#include <cmath>

#include <fs/fvm/triangulation.hpp>

#include "test_constants.hpp"

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

TEST( GeometryTests, TriangleArea ) {

    std::pair<double,double> p = { 0.0, 0.0 };  
    std::pair<double,double> q = { 1.0, 0.0 };  
    std::pair<double,double> r = { 1.0, 1.0 };  

    auto area = fs::fvm::triangle_area( p, q, r );

    EXPECT_DOUBLE_EQ( area, 0.5 );
}

TEST( GeometryTests, RightTriangleAngleTest ) {

    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 1.0, 0.0 };
    std::pair<double, double> r = { 0.0, 1.0 };

    auto angles = fs::fvm::triangle_angles(p, q, r);

    double angle_1{};
    double angle_2{}; 
    double angle_3{};

    std::tie( angle_1, angle_2, angle_3 ) = angles;

    EXPECT_NEAR( angle_1, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_2, M_PI / 4.0, 0.01 );
    EXPECT_NEAR( angle_3, M_PI / 2.0, 0.01 );
}

TEST( GeometryTests, SquaredTriangleLengthsWithSmallestIndex ) {

    std::pair<double, double> p = { 0.0, 0.0 };
    std::pair<double, double> q = { 3.0, 0.0 };
    std::pair<double, double> r = { 0.0, 4.0 };  

    double l_min{}; 
    double l_med{}; 
    double l_max{};
    size_t index{};

    auto result = fs::fvm::squared_triangle_lengths_with_smallest_index( p, q, r );
    
    std::tie( l_min, l_med, l_max, index ) = result;

    EXPECT_NEAR( l_min, 9.0, 0.01 ); 
    EXPECT_NEAR( l_med, 16.0, 0.01 ); 
    EXPECT_NEAR( l_max, 25.0, 0.01 ); 
    EXPECT_EQ( index, 0 ); 
    
    result = fs::fvm::squared_triangle_lengths_with_smallest_index( r, p, q );

    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;

    EXPECT_EQ( index, 1 ); 

    result = fs::fvm::squared_triangle_lengths_with_smallest_index( q, r, p );

    std::tie( std::ignore, std::ignore, std::ignore, index ) = result;

    EXPECT_EQ( index, 2 ); 
}

TEST( GeometryTests, TriangleCircumcenterTest ) {

    std::pair<double, double> p = { 0.0, 0.0 }; 
    std::pair<double, double> q = { 4.0, 0.0 }; 
    std::pair<double, double> r = { 2.0, 4.0 }; 

    std::pair<double, double> expected_circumcenter = { 2.0, 1.5 };

    auto result = fs::fvm::triangle_circumcenter( p, q, r );

    EXPECT_NEAR( result.first, expected_circumcenter.first, 1e-5 );
    EXPECT_NEAR( result.second, expected_circumcenter.second, 1e-5 );
}

TEST( GeometryTests, InradiusOfEquilateralTriangle ) {

    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 2.0, 0.0 );
    std::pair<double,double> r( 1.0, std::sqrt( 3.0 ) );

    double expected_area = std::sqrt( 3.0 );
    double expected_perimeter = 6.0;
    double expected_inradius = expected_area / ( 0.5 * expected_perimeter );

    double actual_inradius = fs::fvm::triangle_inradius( p, q, r );

    EXPECT_NEAR( actual_inradius, expected_inradius, 1e-6 );
}

TEST( GeometryTests, InradiusOfRightAngleTriangle ) {

    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 3.0, 0.0 );
    std::pair<double,double> r( 0.0, 4.0 );

    double expected_area = 6.0;
    double expected_perimeter = 12.0;
    double expected_inradius = expected_area / ( 0.5 * expected_perimeter );

    double actual_inradius = fs::fvm::triangle_inradius( p, q, r );

    EXPECT_NEAR( actual_inradius, expected_inradius, 1e-6 );
}

TEST( GeometryTests, CircumRadius ) {

    std::pair<double,double> p{ 0.0, 0.0 };
    std::pair<double,double> q{ 4.0, 0.0 };
    std::pair<double,double> r{ 0.0, 3.0 };

    double expected_radius = 2.5; 

    double actual_radius = fs::fvm::triangle_circumradius( p, q, r );

    EXPECT_NEAR( actual_radius, expected_radius, 1e-8 );
}

TEST( GeometryTests, CircumSphere ) {

    fs::fvm::three_d_point<double> p( 1.0, 0.0, 0.0 );
    fs::fvm::three_d_point<double> q( 0.0, 1.0, 0.0 );
    fs::fvm::three_d_point<double> r( 0.0, 0.0, 1.0 );

    auto [ center, radius ]  = fs::fvm::get_circumsphere( p, q, r );

    double expected_radius = 0.816496580927726;
    double pos = 0.333333333333333;

    EXPECT_NEAR( radius, expected_radius, 1e-8 );
    EXPECT_NEAR( center.x, pos, 1e-8 );
    EXPECT_NEAR( center.y, pos, 1e-8 );
    EXPECT_NEAR( center.z, pos, 1e-8 );
}

TEST( LinAlgTests, CramersRule ) {

    fs::fvm::three_d_point<double> p( 1.0, 0.0, 0.0 );
    fs::fvm::three_d_point<double> q( 0.0, 1.0, 0.0 );
    fs::fvm::three_d_point<double> r( 0.0, 0.0, 1.0 );

    fs::fvm::three_d_point<double> D( 1.0, 1.0, 1.0 );

    fs::fvm::matrix<double> A( p, q, r );
         
    fs::fvm::three_d_point<double> solution = fs::fvm::solve_cramer( A, D );

    EXPECT_NEAR( solution.x, 1.0, 1e-9 );
    EXPECT_NEAR( solution.y, 1.0, 1e-9 );
    EXPECT_NEAR( solution.z, 1.0, 1e-9 );
}

TEST( LinAlgTests, Pivot ) {

    const size_t rows = 3;
    const size_t columns = 3;

    const std::array<double,rows*columns> elemenents_before_pivot = {
        0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0,
    };

    fs::fvm::matrix_<double,rows,columns> m( elemenents_before_pivot );

    auto permutations = fs::fvm::pivot<double,rows,columns>( m );

    const std::array<double,rows*columns> elemenents_after_pivot = {
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
    };

    EXPECT_EQ( m.elements, elemenents_after_pivot );
    EXPECT_EQ( permutations, ( std::array<size_t,rows>{ 1, 2, 0 } ) );
}

TEST( LinAlgTests, LUSolve ) {

    const size_t rows = 3;
    const size_t columns = 3;

    /*
        A = [ 1.0, 1.0, 1.0       x +  y + z = 6.0
              2.0, 3.0, 1.0      2x + 3y + z = 11.0
              3.0, 2.0, 1.0 ]    3x + 2y + z = 10.0

        solution:
            x = 1.0
            y = 2.0
            z = 3.0
    */
    const std::array<double,rows*columns> coefficients = {
        1.0, 1.0, 1.0, 
        2.0, 3.0, 1.0, 
        3.0, 2.0, 1.0,
    };

    const std::array<double,rows> b = {
        6.0,    //  x +  y + z = 6.0
        11.0,   // 2x + 3y + z = 11.0
        10.0,   // 3x + 2y + z = 10.0 
    };

    fs::fvm::matrix_<double,rows,columns> A( coefficients );

    auto LU_decomp = fs::fvm::LU_decomposition<double,rows,columns>( A );

    auto [ L, U, ps ] = LU_decomp.value();

    auto solution = fs::fvm::LU_solve<double,rows>( L, U, b, ps );

    std::array<double,rows> expected_solution{ 1.0, 2.0, 3.0 };
    
    for ( size_t i = 0; i < rows; ++i ) {
        EXPECT_NEAR( solution[ i ], expected_solution[ i ], 1e-9 ); 
    }
}

TEST( GeometryTests, Orient ) {

    using dp = std::pair<double,double>;

    /*
        the triangle ( p, q, r ) is positively oriented
        the triangle ( p, r, q ) is negatively oriented
    */
    dp p( 0.0, 0.0 );
    dp q( 1.0, 0.0 );
    dp r( 0.0, 1.0 );

    auto expected_positive = fs::fvm::get_orient( p, q, r );       
    auto expected_negative = fs::fvm::get_orient( p, r, q );        

    ASSERT_EQ( expected_positive, fs::fvm::orient::POSITIVE );
    ASSERT_EQ( expected_negative, fs::fvm::orient::NEGATIVE );  

    p = { 0.0, 0.0 }; // make the vertices of the triangle co-linear
    q = { 0.0, 1.0 }; // so that get_orient returns orient::DEGENERATE
    r = { 0.0, 2.0 };

    auto expected_degenerate = fs::fvm::get_orient( p, q, r );
    
    ASSERT_EQ( expected_degenerate, fs::fvm::orient::DEGENERATE );  
}

/*
    TEST: ConstructPositivelyOrientedTriangle

    "points" is a pair of vectors:
    - the first vector is x
    - the second vector is y    

*/
TEST( GeometryTests, ConstructPositivelyOrientedTriangle ) {

    /*
        ( p, q, r ) is a right-angle triangle
        p: x = 0.0, y = 0.0
        q: x = 1.0, y = 0.0
        q: x = 0.0, y = 1.0
    */
    std::pair<double,double> p( 0.0, 0.0 );
    std::pair<double,double> q( 1.0, 0.0 );
    std::pair<double,double> r( 0.0, 1.0 );

    std::pair<std::vector<double>,std::vector<double>> points( 
        { p.first, r.first, q.first }, { p.second, r.second, q.second }
    );

    /*
        0, 1, 2 are the indices of p, q, r in "points"
    */
    auto tri = fs::fvm::construct_positively_oriented_triangle<double>( points, 0, 1, 2 );

    /*
        v, u, w are the indices of the positively-oriented triangle, which should be:

            0, 1, 2

        because p, q, r are already positively-oriented
    */
    auto [ v, u, w ] = tri.value();

    std::tie( p, q, r ) = fs::fvm::get_triangle_points( points, v, u, w );

    auto expected_positive = fs::fvm::get_orient( p, q, r );

    ASSERT_EQ( expected_positive, fs::fvm::orient::POSITIVE );
}

/*
    TEST: TwoSum

    1e16 and 1.0 are perfectly representable in binary ( both for float and double )

    1e16 + 1.0 = 1e16

    so the error term should be 1.0
*/
TEST( AdaptivePredicates, TwoSum ) {

    double a = 1e16;
    double b = 1.0; 

    auto [ sum, err ] = fs::fvm::two_sum( a, b );

    ASSERT_EQ( sum, a );
    ASSERT_EQ( err, b );
}

/*
    TEST: InCirle

    test if three points ( 5.0, 5.0 ), ( 0.0, 0.0 ), ( 0.0, -1.0 ) are inside, outside or on the unit circle
*/
TEST( GeometryTests, InCircle ) {

    std::pair<double,double> p( -1.0, 0.0 );
    std::pair<double,double> q(  1.0, 0.0 );
    std::pair<double,double> r(  0.0, 1.0 );
    std::pair<double,double> a(  5.0, 5.0 );

    auto expected_outside = fs::fvm::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_outside, fs::fvm::in_circle::OUTSIDE );

    a = { 0.0, 0.0 };

    auto expected_inside = fs::fvm::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_inside, fs::fvm::in_circle::INSIDE );

    a = { 0.0, -1.0 };

    auto expected_on = fs::fvm::point_in_circle( p, q, r, a );

    ASSERT_EQ( expected_on, fs::fvm::in_circle::ON );
}

TEST( GeometryTests, PointRelativeToLine ) {

    std::pair<double,double> a( 0.0, -1.0 );
    std::pair<double,double> b( 0.0, 1.0 );
    std::pair<double,double> p( -1.0, 0.0 );

    auto expected_left = fs::fvm::get_relaive_position_of_point_to_line( a, b, p );

    ASSERT_EQ( expected_left, fs::fvm::relative_position::LEFT );

    p = { 1.0, 0.0 };

    auto expected_right = fs::fvm::get_relaive_position_of_point_to_line( a, b, p );

    ASSERT_EQ( expected_right, fs::fvm::relative_position::RIGHT );

    p = { 0.0, 0.0 };

    auto expected_on = fs::fvm::get_relaive_position_of_point_to_line( a, b, p );

    ASSERT_EQ( expected_on, fs::fvm::relative_position::ON );
}