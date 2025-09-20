#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <fs/fvm/triangulation.hpp>
#include <geometry/geometry.hpp>

namespace geometry {
  
    // =======================
    //  Get Triangle Centroid
    // =======================
    // the centroid of a triangle is the point where the three medians of the traingle intersect
    // a median is a line segment that connects a vertex to the mid-point of the opposite edge
    template<typename T>
    std::pair<T,T> get_triangle_centroid( const std::pair<T,T>& p, 
                                          const std::pair<T,T>& q,
                                          const std::pair<T,T>& r ) {
        auto [ qx_prime, qy_prime ] = fs::fvm::get_difference( q, p );
        auto [ rx_prime, ry_prime ] = fs::fvm::get_difference( r, p );
        auto cx_prime = 2 * fs::fvm::get_mid_point( qx_prime, rx_prime ) / 3;
        auto cy_prime = 2 * fs::fvm::get_mid_point( qy_prime, ry_prime ) / 3;
        auto [ cx, cy ] = fs::fvm::get_sum( { cx_prime, cy_prime }, p );
        return { cx, cy };
    }

    // ===========================
    //  Get Triangle Ortho Center
    // ===========================

    template<typename I,typename T>
    std::pair<T,T> get_triangle_ortho_center( const fs::fvm::triangulation<I,T>& tri, const fs::fvm::triangle& t ) {
        const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
        const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
        const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );

        const T a = tri.get_weight( std::get<0>( t ) );
        const T b = tri.get_weight( std::get<1>( t ) );
        const T c = tri.get_weight( std::get<2>( t ) );

        const T A = fs::fvm::triangle_area( p, q, r );

        const T d_11 = fs::fvm::dist_sqr( p, r ) + c - a;
        const T d_12 = p.second - r.second;
        const T d_21 = fs::fvm::dist_sqr( q, r ) + c - b;
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
    std::optional<fs::fvm::triangle> construct_positively_oriented_triangle( const Points& points,
                                                                             const size_t i, 
                                                                             const size_t j, 
                                                                             const size_t k ) {
        auto [ p, q, r ] = fs::fvm::get_triangle_points( points, i, j, k );
        auto orientation = fs::fvm::get_orient( p, q, r );
        if ( orientation == fs::fvm::orient::DEGENERATE ) {
            return std::nullopt;
        }
        if ( orientation == fs::fvm::orient::POSITIVE ) {
            return fs::fvm::triangle( i, j, k );
        } else {
            // switching first and second vertices switches the orientation of the 
            // triangle, in this case from negative to positive
            return fs::fvm::triangle( j, i, k );
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
        const auto [ l_1_sqr, l_2_sqr, l_3_sqr ] = fs::fvm::squared_triangle_lengths( p, q, r );
        T A = fs::fvm::triangle_area( p, q, r );
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
        T A = fs::fvm::triangle_area( p, q, r );
        T d_11 = fs::fvm::dist_sqr( p, r ) + c - a;
        T d_21 = fs::fvm::dist_sqr( q, r ) + c - b;
        auto [ e_11, d_12 ] = fs::fvm::get_difference( p, r );
        auto [ e_21, d_22 ] = fs::fvm::get_difference( q, r );

        T e_12 = d_11;
        T e_22 = d_21;

        T t_1 = d_11 * d_22 - d_12 * d_21;
        T t_2 = e_11 * e_22 - e_12 * e_21;
        return ( t_1 * t_1 + t_2 * t_2 ) / ( 16 * A * A ) - c;
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename I,typename T>
    T get_triangle_orthoradius_squared( const fs::fvm::triangulation<I,T>& tri, 
                                        const fs::fvm::triangle& t ) {
        const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
        const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
        const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );

        const T a = tri.get_weight( std::get<0>( t ) );
        const T b = tri.get_weight( std::get<1>( t ) );
        const T c = tri.get_weight( std::get<2>( t ) );
        return get_triangle_orthoradius_squared( p, q, r, a, b, c );
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
    T get_triangle_inradius( const std::pair<T,T>& p, 
                         const std::pair<T,T>& q, 
                         const std::pair<T,T>& r ) {
        T A = fs::fvm::triangle_area( p, q, r );
        T perimeter = fs::fvm::triangle_perimeter( p, q, r );
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
        T d_11 = fs::fvm::dist_sqr( p, r );
        T d_12 = p.second - r.second;
        T d_21 = fs::fvm::dist_sqr( q, r );
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
        T A = fs::fvm::triangle_area( p, q, r );
        return get_triangle_circumcenter( p, q, r, A );
    }

    // =====================
    //  Get Triangle Angles
    // =====================

    template<typename T>
    std::tuple<T,T,T> get_triangle_angles( const std::pair<T,T>& p,
                                           const std::pair<T,T>& q,
                                           const std::pair<T,T>& r ) {
        const T A = fs::fvm::triangle_area( p, q, r );
        T theta_1 = calculate_theta( p, q, r, p, A );
        T theta_2 = calculate_theta( q, p, r, q, A ); 
        T theta_3 = calculate_theta( r, p, q, r, A );
        return fs::fvm::min_med_max( theta_1, theta_2, theta_3 );
    }

} // namespace geometry

#endif
