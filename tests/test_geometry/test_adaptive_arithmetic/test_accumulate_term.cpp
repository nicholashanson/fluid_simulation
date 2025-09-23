#include <gtest/gtest.h>

#include <geometry/adaptive_arithmetic.hpp>

TEST( GeometryTest, AccumulateTerm_SmallTerm ) {
    constexpr std::size_t N = 4;
    std::array<double,N> h{};
    std::size_t h_index = 0;
    double Q = 1e16;  
    double term = 1.0;
    adaptive_arithmetic::accumulate_term( Q, term, h, h_index );
    EXPECT_EQ( Q, 1e16 );
    ASSERT_EQ( h_index, 1u );
    EXPECT_EQ( h[ 0 ], 1.0 );
}

TEST( GeometryTest, AccumulateTerm_AddZero ) {
    constexpr std::size_t N = 4;
    std::array<double,N> h{};
    std::size_t h_index = 0;
    double Q = 1.0;  
    double term = 0.0;
    adaptive_arithmetic::accumulate_term( Q, term, h, h_index );
    EXPECT_EQ( Q, 1.0 );
    ASSERT_EQ( h_index, 0u );
}

TEST( GeometryTest, AccumulateTerm_NoResidual ) {
    constexpr std::size_t N = 4;
    std::array<double,N> h{};
    std::size_t h_index = 0;
    double Q = 1.0;  
    double term = 2.0;
    adaptive_arithmetic::accumulate_term( Q, term, h, h_index );
    EXPECT_EQ( Q, 3.0 );
    ASSERT_EQ( h_index, 0u );
}