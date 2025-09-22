#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <geometry/geometry.hpp>

namespace geometry {

    using triangle = std::tuple<int,int,int>;

    // =====================
    //  Get Triangle Points
    // =====================

    template<typename T>
    std::array<std::pair<T,T>,3> get_triangle_points( const std::pair<std::vector<T>,std::vector<T>>& points, 
                                                      const size_t i, const size_t j, const size_t k ) {
        return { std::pair<T,T>( points.first[ i ], points.second[ i ] ) , 
                 std::pair<T,T>( points.first[ j ], points.second[ j ] ) ,
                 std::pair<T,T>( points.first[ k ], points.second[ k ] ) };
    }

    // ============================================
    //  Get Squared Triangle Lengths Smalled Index
    // ============================================

    template<typename T>
    std::tuple<T,T,T,size_t> get_squared_triangle_lengths_with_smallest_index( const std::pair<T,T>& p,
                                                                               const std::pair<T,T>& q,
                                                                               const std::pair<T,T>& r ) {
        const T l_1 = dist_sqr( p, q );
        const T l_2 = dist_sqr( q, r );
        const T l_3 = dist_sqr( r, p );
        auto [ l_min, l_med, l_max ] = min_med_max( l_1, l_2, l_3 );
        size_t shortest_index{};
        if ( l_min == l_2 ) {
            shortest_index = 1;
        }
        if ( l_min == l_3 ) {
            shortest_index = 2;
        }
        return std::tuple( l_min, l_med, l_max, shortest_index );
    }

    // ==========================
    //  Squared Triangle Lengths
    // ==========================

    template<typename T>
    std::tuple<T,T,T> get_squared_triangle_lengths( const std::pair<T,T>& p, const std::pair<T,T>& q, 
                                                    const std::pair<T,T>& r ) {
        T l_1{};
        T l_2{};
        T l_3{};
        std::tie( l_1, l_2, l_3, std::ignore ) = get_squared_triangle_lengths_with_smallest_index( p, q, r );
        return std::make_tuple( l_1, l_2, l_3 );
    }

    // ======================
    //  Get Triangle Lengths
    // ======================

    template<typename T>
    std::tuple<T,T,T> get_triangle_lengths( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        T l_1{};
        T l_2{};
        T l_3{};
        std::tie( l_1, l_2, l_3, std::ignore ) = get_squared_triangle_lengths_with_smallest_index( p, q, r );
        return std::make_tuple( std::sqrt( l_1 ), std::sqrt( l_2 ), std::sqrt( l_3 ) );
    }

    // ==================
    //  Get Triangle Area
    // ===================
    // l_1, l_2, l_3: the squared lengths of the sides of a the triangle
    template<typename T>
    T get_triangle_area( const T l_1, const T l_2, const T l_3 ) {
        // area squared
        T A_2 = ( 4 * l_1 * l_2 - ( l_1 + l_2 - l_3) * ( l_1 + l_2 - l_3 ) ) / 16;
        if ( A_2 < 0 ) {
            T a = std::sqrt( l_1 );
            T b = std::sqrt( l_2 );
            T c = std::sqrt( l_3 );
            A_2 = ( a + ( b + c ) ) * ( c - ( a - b) ) * ( c + ( a - b ) ) * ( a + ( b - c ) ) / 16;
        }
        if ( A_2 < 0 ) {
            return 0;
        } else {
            return std::sqrt( A_2 );
        }
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_triangle_area( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        const T A = adaptive_arithmetic::orient_2( p, q, r ) / ( T )2;
        return A;
    }

    // ========================
    //  Get Triangle Perimeter
    // ========================

    template<typename T>
    T get_triangle_perimeter( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        T l_1{};
        T l_2{};
        T l_3{};
        std::tie( l_1, l_2, l_3 ) = get_triangle_lengths( p, q, r );
        return l_1 + l_2 + l_3;
    }
  
    // =======================
    //  Get Triangle Centroid
    // =======================
    // the centroid of a triangle is the point where the three medians of the traingle intersect
    // a median is a line segment that connects a vertex to the mid-point of the opposite edge
    template<typename T>
    std::pair<T,T> get_triangle_centroid( const std::pair<T,T>& p, 
                                          const std::pair<T,T>& q,
                                          const std::pair<T,T>& r ) {
        auto [ qx_prime, qy_prime ] = get_difference( q, p );
        auto [ rx_prime, ry_prime ] = get_difference( r, p );
        auto cx_prime = 2 * get_mid_point( qx_prime, rx_prime ) / 3;
        auto cy_prime = 2 * get_mid_point( qy_prime, ry_prime ) / 3;
        auto [ cx, cy ] = get_sum( { cx_prime, cy_prime }, p );
        return { cx, cy };
    }

    // ===========================
    //  Get Triangle Ortho Center
    // ===========================

    template<typename T>
    std::pair<T,T> get_triangle_ortho_center( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r,
                                              T a = 0, T b = 0, T c = 0 ) {
        const T A = get_triangle_area( p, q, r );
        const T d_11 = dist_sqr( p, r ) + c - a;
        const T d_12 = p.second - r.second;
        const T d_21 = dist_sqr( q, r ) + c - b;
        const T d_22 = q.second - r.second;
        
        const T o_x = r.first + ( d_11 * d_22 - d_12 * d_21 ) / ( 4 * A );
        const T e_11 = p.first - r.first;
        const T e_12 = d_11;
        const T e_21 = q.first - r.first;
        const T e_22 = d_21;
        const T o_y = r.second + ( e_11 * e_22 - e_12 * e_21 ) / ( 4 * A );
        return { o_x, o_y };
    }

    // ========================================
    //  Construct Positively Oriented Triangle
    // ========================================

    template<typename T,typename Points>
    std::optional<triangle> construct_positively_oriented_triangle( const Points& points,
                                                                    const std::size_t i, 
                                                                    const std::size_t j, 
                                                                    const std::size_t k ) {
        auto [ p, q, r ] = get_triangle_points( points, i, j, k );
        auto orientation = get_orient( p, q, r );
        if ( orientation == orient::degenerate ) {
            return std::nullopt;
        }
        if ( orientation == orient::positive ) {
            return triangle( i, j, k );
        } else {
            // switching first and second vertices switches the orientation of the 
            // triangle, in this case from negative to positive
            return triangle( j, i, k );
        }
    }

    // ===========================
    //  Get Triangle Circumradius
    // ===========================

    template<typename T>
    T get_triangle_circumradius( const T A, const T l_min_sqr, const T l_med_sqr, const T l_max_sqr ) {
        return std::sqrt( l_min_sqr * l_med_sqr * l_max_sqr ) / ( 4 * A );
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_triangle_circumradius( const std::pair<T,T>& p, 
                                 const std::pair<T,T>& q, 
                                 const std::pair<T,T>& r ) {
        const auto [ l_1_sqr, l_2_sqr, l_3_sqr ] = get_squared_triangle_lengths( p, q, r );
        T A = get_triangle_area( p, q, r );
        return get_triangle_circumradius( A, l_1_sqr, l_2_sqr, l_3_sqr );
    }

    // ==================================
    //  Get Triangle Orthoradius Squared
    // ==================================

    template<typename I,typename T>
    T get_triangle_orthoradius_squared( const std::pair<T,T>& p, 
                                        const std::pair<T,T>& q,
                                        const std::pair<T,T>& r,
                                        const T a, const T b, const T c ) {
        T A = get_triangle_area( p, q, r );
        T d_11 = dist_sqr( p, r ) + c - a;
        T d_21 = dist_sqr( q, r ) + c - b;
        auto [ e_11, d_12 ] = get_difference( p, r );
        auto [ e_21, d_22 ] = get_difference( q, r );

        T e_12 = d_11;
        T e_22 = d_21;

        T t_1 = d_11 * d_22 - d_12 * d_21;
        T t_2 = e_11 * e_22 - e_12 * e_21;
        return ( t_1 * t_1 + t_2 * t_2 ) / ( 16 * A * A ) - c;
    }

    // =======================
    //  Get Triangle Inradius
    // =======================

    template<typename T>
    T get_triangle_inradius( const T A, const T perimeter ) {
        return 2 * A / perimeter;
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_triangle_inradius( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        T A = get_triangle_area( p, q, r );
        T perimeter = get_triangle_perimeter( p, q, r );
        return get_triangle_inradius( A, perimeter );
    }

    // ===========================
    //  Get Triangle Circumcenter
    // ===========================

    template<typename T>
    std::pair<T,T> get_triangle_circumcenter( const std::pair<T,T>& p, 
                                              const std::pair<T,T>& q, 
                                              const std::pair<T,T>& r,
                                              const T A ) {
        T d_11 = dist_sqr( p, r );
        T d_12 = p.second - r.second;
        T d_21 = dist_sqr( q, r );
        T d_22 = q.second - r.second;
        T o_x = r.first + ( d_11 * d_22 - d_12 * d_21 ) / ( 4 * A );
        T e_11 = p.first - r.first;
        T e_12 = d_11;
        T e_21 = q.first - r.first;
        T e_22 = d_21;
        T o_y = r.second + ( e_11 * e_22 - e_12 * e_21 ) / ( 4 * A );
        return { o_x, o_y };
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    std::pair<T,T> get_triangle_circumcenter( const std::pair<T,T>& p, 
                                              const std::pair<T,T>& q, 
                                              const std::pair<T,T>& r ) {
        T A = get_triangle_area( p, q, r );
        return get_triangle_circumcenter( p, q, r, A );
    }

    // =====================
    //  Get Triangle Angles
    // =====================

    template<typename T>
    std::tuple<T,T,T> get_triangle_angles( const std::pair<T,T>& p,
                                           const std::pair<T,T>& q,
                                           const std::pair<T,T>& r ) {
        const T A = get_triangle_area( p, q, r );
        T theta_1 = calculate_theta( p, q, r, p, A );
        T theta_2 = calculate_theta( q, p, r, q, A ); 
        T theta_3 = calculate_theta( r, p, q, r, A );
        return min_med_max( theta_1, theta_2, theta_3 );
    }

} // namespace geometry

#endif
