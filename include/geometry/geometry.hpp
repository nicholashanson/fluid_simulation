#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "adaptive_arithmetic.hpp"

namespace geometry {

    // =========
    //  Reverse
    // =========

    template<typename T>
    std::pair<T,T> reverse( const std::pair<T,T>& p ) {
        return { p.second, p.first };
    }

    // ============
    //  Matrix 2x2
    // ============

    template<typename T>
    struct matrix2x2 {
        T a11;
        T a12;
        T a21;
        T a22;

        matrix2x2( T a11_, T a12_, T a21_, T a22_ )
            : a11( a11_ ), a12( a12_ ), a21( a21_ ), a22( a22_ ) {}

        matrix2x2( const std::pair<T,T>& col1, const std::pair<T,T>& col2 ) 
            : a11( col1.first ), a12( col2.first ), a21( col1.second ), a22( col2.second ) {}

        std::pair<T,T> c1() const {
            return { a11, a21 };
        } 

        std::pair<T,T> c2() const {
            return { a12, a22 };
        }

        std::pair<T,T> r1() const {
            return { a11, a12 };
        } 

        std::pair<T,T> r2() const {
            return { a21, a22 };
        }
    };

    // ===============
    //  Three D Point
    // ===============

    template<typename T>
    struct three_d_point {
        T x;
        T y;
        T z;

        three_d_point() = default;

        three_d_point( T x, T y, T z ) 
            : x( x ), y( y ), z( z ) {}

        constexpr operator std::tuple<T,T,T>() const {
            return { x, y, z };
        }

        bool operator==( const three_d_point& rhs ) {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }
    };

    // ===============
    //  Three D Plane
    // ===============

    template<typename T> 
    struct three_d_plane {
        T alpha;
        T beta;
        T gamma;
        T delta;

        three_d_plane( const T alpha, const T beta, const T gamma, const T delta )
            : alpha( alpha ), beta( beta ), gamma( gamma ), delta( delta ) {}
    };

    // ==========
    //  Dist Sqr
    // ==========

    template<typename T>
    T dist_sqr( const std::pair<T, T>& p, const std::pair<T,T>& q ) {
        std::pair<T,T> vec = { q.first - p.first, q.second - p.second };
        return vec.first * vec.first + vec.second * vec.second;
    }

    // =========
    //  Get Sum
    // =========

    template<typename T>
    std::pair<T,T> get_sum( const std::pair<T,T>& p, 
                            const std::pair<T,T>& q ) {
        return { p.first + q.first, p.second + q.second };
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    three_d_point<T> get_sum( const three_d_point<T>& p, 
                              const three_d_point<T>& q ) {
        three_d_point<T> sum;
        sum.x = p.x + q.x;
        sum.y = p.y + q.y;
        sum.z = p.z + q.z;
        return sum;
    }

    // ================
    //  Is Zero Vector
    // ================

    template<typename T> 
    bool is_zero_vector( const three_d_point<T>& v ) {
        return ( v.x == 0 && v.y == 0 && v.z == 0 );
    }

    // =================
    //  Get Dot Product
    // =================

    template<typename T>
    T get_dot_product( const three_d_point<T>& p, const three_d_point<T>& q ) {
        return p.x * q.x + p.y * q.y + p.z * q.z;
    }

    // ===================
    //  Get Inner Product
    // ===================

    template<typename T>
    T get_inner_product( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return p.first * q.first + p.second * q.second;
    }

    // ======================
    //  Get Exterior Product
    // ======================

    template<typename T>
    T get_exterior_product( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return p.first * q.second - p.second * q.first;   
    }

    // =======
    //  Scale
    // =======

    template<typename T>
    three_d_point<T> scale( const T c, const three_d_point<T>& v ) {
        return three_d_point( c * v.x, c * v.y, c * v.z );
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    std::pair<T,T> scale( const T c, const std::pair<T,T>& v ) {
        return std::make_pair( c * v.first, c * v.second );
    } 

    // ========
    //  Matrix
    // ========

    template<typename T>
    struct matrix {
        three_d_point<T> a;
        three_d_point<T> b;
        three_d_point<T> c;

        matrix( const three_d_point<T>& p, const three_d_point<T>& q, const three_d_point<T>& r )
            : a( p ), b( q ), c( r ) {}
    };

    // =================
    //  Get Determinant
    // =================

    template<typename T>
    T get_determinant( const matrix<T>& m ) {
        T a = m.b.y * m.c.z - m.b.z * m.c.y;
        T b = m.b.x * m.c.z - m.b.z * m.c.x;
        T c = m.b.x * m.c.y - m.b.y * m.c.x;
        return m.a.x * a - m.a.y * b + m.a.z * c;
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_determinant( matrix2x2<T> m ) {
        return m.a11 * m.a22 - m.a12 * m.a21;
    }

    // ==========
    //  Distance
    // ==========

    template<typename T>
    T get_distance( const three_d_point<T>& p, const three_d_point<T>& q ) {
        return std::sqrt( ( p.x - q.x ) * ( p.x - q.x ) + 
                          ( p.y - q.y ) * ( p.y - q.y ) +
                          ( p.z - q.z ) * ( p.z - q.z ) );
    }

    // ===================
    //  Get Cross Product
    // ===================

    template<typename T>
    three_d_point<T> get_cross_product( const three_d_point<T>& p, const three_d_point<T>& q ) {
        three_d_point<T> cross_product;
        cross_product.x = p.y * q.z - p.z * q.y;
        cross_product.y = p.z * q.x - p.x * q.z;
        cross_product.z = p.x * q.y - p.y * q.x;
        return cross_product;
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_cross_product( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return p.first * q.second - p.second * q.first;
    }

    // ================
    //  Get Difference
    // ================

    template<typename T>
    std::pair<T,T> get_difference( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return { p.first - q.first, p.second - q.second };
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    three_d_point<T> get_difference( const three_d_point<T>& p, const three_d_point<T>& q ) {
        three_d_point<T> difference;
        difference.x = p.x - q.x;
        difference.y = p.y - q.y;
        difference.z = p.z - q.z;
        return difference;
    }

    // =========
    //  Min Max
    // =========

    template<typename T>
    std::tuple<T,T> min_max( const T& a, const T& b ) {
        return std::make_tuple( std::min( a, b ), std::max( a, b ) );
    }

    // =============
    //  Min Med Max
    // =============

    template<typename T>
    std::tuple<T,T,T> min_med_max( T a, T b, T c ) {
        std::tie( b, c ) = min_max( b, c );
        std::tie( a, c ) = min_max( a, c );
        std::tie( a, b ) = min_max( a, b );
        return std::make_tuple( a, b, c );
    }

    // ================
    //  Orient 2 Adapt
    // ================

    template<typename T>
    T orient_2_adapt( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r, const T det_sum ) {
        auto ac = get_difference( p, r );
        auto bc = get_difference( q, r );
        return adaptive_arithmetic::orient_2_adapt( p, q, r, ac, bc, det_sum );  
    }

    // ===============
    //  Get Mid Point
    // ===============

    template<typename T>
    T get_mid_point( T a, T b ) {
        if ( a < b ) {
            return a + ( b - a ) / 2;
        } 
        return b + ( a - b ) / 2;
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    three_d_point<T> get_mid_point( const three_d_point<T>& p, const three_d_point<T>& q ) {
        three_d_point<T> mid_point;
        mid_point.x = get_mid_point( p.x, q.x );
        mid_point.y = get_mid_point( p.y, q.y );
        mid_point.z = get_mid_point( p.z, q.z );
        return mid_point;
    }

    // ========
    //  Orient
    // ========

    enum class orient : uint8_t {
        positive,
        degenerate,
        negative
    };

    // ============
    //  Get Orient
    // ============
    // - (p,q,r) forms a  positively-oriented if, when traversing in the order p -> q -> r, 
    // the interior of the triangle is on the right-hand side
    template<typename T>
    orient get_orient( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {  
        auto delta_pr = get_difference( p, r );
        auto delta_qr = get_difference( q, r );
        auto exterior_product = get_exterior_product( delta_pr, delta_qr );
        const T epsilon = static_cast<T>( 1e-9 );
        if ( std::abs( exterior_product ) < epsilon /* colinear */ ) {
            return orient::degenerate;
        }
        return adaptive_arithmetic::sign( exterior_product ) > 0 ? orient::positive : orient::negative; 
    }

    // =================================
    //  Get Squared Distance To Segment
    // =================================
    // - calculate the distance between point p and line segment a-b 
    template<typename T>
    T get_squared_distance_to_segment( const std::pair<T,T>& a, const std::pair<T,T>& b, const std::pair<T,T>& p ) {
        auto delta_pa = get_difference( p, a );
        auto delta_ba = get_difference( b, a );
        auto dot_product = get_inner_product( delta_pa, delta_ba );
        auto ab_dist_sqr = dist_sqr( a, b );
        if ( ab_dist_sqr == 0 /* a == b */ ) {
            return dist_sqr( p, a );
        }
        T t = dot_product / ab_dist_sqr /* (1) */;
        T clamped_t = std::min( std::max( t, ( T )0 ), ( T )1 );
        auto insersect = get_sum( a, scale( clamped_t, delta_ba ) ); 
        T pintersect_dist_sqr = dist_sqr( p, insersect );
        return pintersect_dist_sqr;
    }
    // (1) t <= 0: p is closest to a, t >= 1: p is closest to b, 0 < t < 1: p is closest to an intermediate point

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_squared_distance_to_segment( const T q_x, const T q_y, const T r_x, const T r_y, const T p_x, const T p_y ) {
        return get_squared_distance_to_segment( std::make_pair( q_x, q_y ), std::make_pair( r_x, r_y ), std::make_pair( p_x, p_y ) );
    }

    // ================================
    //  Get Plane Through Three Points
    // ================================

    template<typename T>
    three_d_plane<T> get_plane_from_three_points( const three_d_point<T>& p, const three_d_point<T>& q, const three_d_point<T>& r ) {
        const T alpha = p.y * q.z + p.z * q.y - p.z * r.y + q.y * r.z - q.z * r.y;
        const T beta = p.z * q.x - p.x * q.z + p.x * r.z - p.z * r.x - q.x * r.z + q.z * r.x;
        const T gamma = p.x * q.y - p.y * q.x - p.x * r.y + p.y * r.x + q.x * r.y - q.y * r.x;
        const T delta = p.x * q.z * r.y - p.x * q.y * r.z + p.y * q.x * r.z - p.y * q.z * r.x - p.z * q.x * r.y + p.z * q.y * r.x;
        return three_d_plane( alpha, beta, gamma, delta );
    }

    // ===================
    //  Relative Position 
    // ===================

    enum class relative_position : uint8_t {
        left,
        on,
        right
    };

    // ========================================
    //  Get Relative Position Of Point To Line
    // ========================================

    template<typename T>
    relative_position get_relaive_position_of_point_to_line( const std::pair<T,T>& a, const std::pair<T,T>& b, const std::pair<T,T>& p ) {
        auto orientation = get_orient( a, b, p );
        if ( orientation == orient::degenerate /* colinear */ ) 
            return relative_position::on;
        else if ( orientation == orient::positive ) {
            return relative_position::left /* (1) */;
        } else {
            return relative_position::right;
        }
    }
    // if (a,b,p) forms a positively-oriented triangle, the p lies to the left of a-b

    // =================
    //  Calculate Theta
    // =================

    template <typename T>
    T calculate_theta( const std::pair<T, T>& p, const std::pair<T, T>& q, const std::pair<T, T>& r, const std::pair<T, T>& s, T A ) {
        auto delta_qp = get_difference( q, p );
        auto delta_rs = get_difference( r, s );
        T dot_product = get_inner_product( delta_qp, delta_rs );
        T theta = dot_product == 0 ? ( M_PI / 2.0 ) : std::atan( 2 * A / dot_product );
        if ( theta < 0 ) { 
            theta += M_PI;
        }
        return theta;
    }

} // namespace geometry

#endif