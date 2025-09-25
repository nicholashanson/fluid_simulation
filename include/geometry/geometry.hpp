#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "adaptive_arithmetic.hpp"
#include "gl_quadrature.hpp"

namespace geometry {

    // ==========
    //  B Spline
    // ==========

    template<typename T>
    struct b_spline;

    template<typename T>
    std::pair<T,T> de_boor( const b_spline<T>&, const T t );

    template<typename T>
    struct b_spline {
        b_spline( const std::vector<std::pair<T,T>>& control_points_, const int degree_ ) 
        : control_points( control_points_ ), 
          knots( degree_ + control_points_.size() + 1 ),
          degree( degree_ ) {
            std::size_t order = degree_ + 1;
            std::fill( knots.begin(), knots.begin() + order, 0 );
            for ( std::size_t i = order; i <= control_points_.size(); ++i ) {
                knots[ i ] = knots[ i - 1 ] + 1;
            }
            std::fill( knots.begin() + control_points_.size(), knots.end(), 
                knots[ control_points_.size() ] );
        };

        b_spline( const std::vector<std::pair<T,T>>& control_points_, const std::vector<int> knots_, const int degree_ ) 
        : control_points( control_points_ ), 
          knots( knots_ ),
          degree( degree_ ) {}

        std::pair<T,T> evaluate( const T t ) const {
            return de_boor( *this, t );
        }
        
        int degree;
        std::vector<std::pair<T,T>> control_points;
        std::vector<int> knots;
    };

    // ================
    //  Find Knot Span
    // ================

    template<typename T>
    std::size_t find_knot_span( const b_spline<T>& spline, const T t ) {
        std::size_t n = spline.knots.size() - spline.degree - 1;
        if ( t == spline.knots[ n + 1 ] ) {
            return n;
        }
        if (t == spline.knots[ spline.degree ] ) {
            return spline.degree;
        }
        auto it = std::upper_bound( spline.knots.begin() + spline.degree, spline.knots.begin() + n + 1, t );
        return std::distance( spline.knots.begin(), it ) - 1;
    }

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

        bool operator==( const three_d_point& rhs ) const {
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

        bool operator==( const three_d_plane& rhs ) const {
            return alpha == rhs.alpha &&
                   beta == rhs.beta &&
                   gamma == rhs.gamma &&
                   delta == rhs.delta;
        }
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
    std::pair<T,T> get_sum( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return { p.first + q.first, p.second + q.second };
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    three_d_point<T> get_sum( const three_d_point<T>& p, const three_d_point<T>& q ) {
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

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T get_distance( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return std::sqrt( ( p.first - q.first ) * ( p.first - q.first ) + 
                          ( p.second - q.second ) * ( p.second - q.second ) );
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

    // =========
    //  De Boor
    // =========

    template<typename T>
    std::pair<T,T> de_boor( const b_spline<T>& spline, const T t_normalized ) {
        T a = spline.knots[ spline.degree ]; 
        T b = spline.knots[ spline.control_points.size() ];
        T t = a + t_normalized * ( b - a );
        if ( t_normalized == 0 ) {
            return spline.control_points.front();
        }
        if ( t_normalized == 1 ) {
            return spline.control_points.back();
        }
        auto k = find_knot_span( spline, t );
        std::vector<std::pair<T,T>> d( spline.degree + 1 );
        for ( std::size_t i = 0; i <= spline.degree; ++i ) {
            d[ i ] = spline.control_points[ k - spline.degree + i ];
        }
        for ( std::size_t r = 1; r <= spline.degree; ++r ) {
            for ( std::size_t j = spline.degree; j >= r ; --j ) {
                std::size_t i = k - spline.degree + j;
                T alpha = ( t - spline.knots[ i ] ) / ( spline.knots[ i + spline.degree - r + 1 ] - spline.knots[ i ] );
                d[ j ] = get_sum( scale( 1 - alpha, d[ j - 1 ] ), scale( alpha, d[ j ] ) );
            } 
        }
        return d[ spline.degree ];
    }

    // ===============
    //  Differentiate
    // ===============

    template<typename T>
    std::pair<T,T> get_first_derivative( const b_spline<T>& spline, const T t ) {
        std::vector<std::pair<T,T>> derivative_control_points( spline.control_points.size() - 1 );
        for ( std::size_t i = 0; i < spline.control_points.size() - 1; ++i ) {
            T denom = spline.knots[ i + spline.degree + 1 ] - spline.knots[ i + 1 ];
            auto factor = static_cast<T>( spline.degree ) / denom;
            derivative_control_points[ i ] = std::make_pair( spline.control_points[ i + 1 ].first - spline.control_points[ i ].first,
                                                             spline.control_points[ i + 1 ].second - spline.control_points[ i ].second );
            derivative_control_points[ i ] = scale( factor, derivative_control_points[ i ] );
        }
        std::vector<int> derivative_knots( spline.knots.begin() + 1, spline.knots.end() - 1 );
        auto derivative_spline = b_spline( derivative_control_points, derivative_knots, spline.degree - 1 );
        std::pair<T,T> derivative = de_boor( derivative_spline, t );
        T range = spline.knots.back() - spline.knots.front();
        return scale( range, derivative );
    }

    // ================
    //  Get Arc Legnth
    // ================

    template<typename T>
    T get_arc_length( const b_spline<T>& spline, const T t_1, const T t_2 ) {
        T factor = ( t_2 - t_1 ) / 2.0;
        T shift  = ( t_2 + t_1 ) / 2.0;
        T s{};
        for ( auto [ x, w ] : gl ) {
            T t = factor * x + shift;
            auto [ dx, dy ] = get_first_derivative( spline, t );
            s += w * std::sqrt( dx * dx + dy * dy );
        }
        return factor * s;
    }

    // =======================
    //  Get Second Derivative
    // =======================

    template<typename T>
    std::pair<T,T> get_second_derivative( const b_spline<T>& spline, const T t ) {
        int degree = spline.degree;
        std::vector<std::pair<T,T>> deriv_points( spline.control_points.size() - 2 );
        for ( std::size_t i = 0; i < spline.control_points.size() - 2; ++i ) {
            auto x0 = spline.control_points[ i ];
            auto x1 = spline.control_points[ i + 1 ];
            auto x2 = spline.control_points[ i + 2 ];
            auto scale1 = static_cast<T>( degree     ) / ( spline.knots[ i + degree + 1 ] - spline.knots[ i + 1 ] );
            auto scale2 = static_cast<T>( degree     ) / ( spline.knots[ i + degree + 2 ] - spline.knots[ i + 2 ] );
            auto scale3 = static_cast<T>( degree - 1 ) / ( spline.knots[ i + degree + 1 ] - spline.knots[ i + 2 ] );
            auto q0 = scale( scale1, get_difference( x1, x0 ) );
            auto q1 = scale( scale2, get_difference( x2, x1 ) );
            deriv_points[ i ] = scale( scale3, get_difference( q1, q0 ) );
        }
        std::vector<int> deriv_knots( spline.knots.begin() + 2, spline.knots.end() - 2);
        b_spline<T> derivative_spline( deriv_points, deriv_knots, degree - 2 );
        auto deriv = derivative_spline.evaluate( t );
        T range = spline.knots.back() - spline.knots.front();
        range *= range;
        return scale( range, deriv );
    }

} // namespace geometry

#endif