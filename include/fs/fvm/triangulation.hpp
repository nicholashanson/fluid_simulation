#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>
#include <limits>
#include <iterator>
#include <optional>

#include <set>
#include <map>
#include <tuple>
#include <vector>

#include <concepts>
#include <type_traits>

namespace fs {

    namespace fvm {

        template <typename... Ts, std::size_t... I>
        auto tuple_to_array_impl(const std::tuple<Ts...>& t, std::index_sequence<I...>) {
            return std::array{std::get<I>(t)...}; // Use index sequence to access elements
        }

        template <typename... Ts>
        auto tuple_to_array(const std::tuple<Ts...>& t) {
            return tuple_to_array_impl(t, std::index_sequence_for<Ts...>{});
        }

        template<typename T>
        T fast_two_sum_tail( const T a, const T b, const T x ) {

            const T b_virt = x - a;
            const T y = b - b_virt;

            return y;
        }

        template<typename T>
        std::tuple<T,T> fast_two_sum( const T a, const T b ) {

            const T x = a + b;
            const T y = fast_two_sum_tail( a, b, x );
            
            return std::make_tuple( x, y );
        }

        template<typename T>
        T two_sum_tail( const T a, const T b, const T x ) {

            const T b_virt = x - a;
            const T a_virt = x - b_virt;
            const T b_round = b - b_virt;
            const T a_round = a - a_virt;
            const T y = a_round + b_round;

            return y;
        }

        template<typename T>
        std::pair<T,T> two_sum( const T a, const T b ) {

            const T x = a + b;
            const T y = two_sum_tail( a, b, x );
            
            return std::make_tuple( x, y );
        }

        template<typename T>
        T two_diff_tail( const T a, const T b, const T x ) {

            const T b_virt = a - x;
            const T a_virt = x + b_virt;
            const T b_round = b_virt - b;
            const T a_round = a - a_virt;
            const T y = a_round + b_round;

            return y;
        }

        template<typename T>
        std::tuple<T,T> two_diff( const T a, const T b ) {
            
            const T x = a - b;
            const T y = two_diff_tail( a, b, x );
            
            return std::make_tuple( x, y );  
        }

        template<typename T>
        int sign( T val ) {
            return ( T( 0 ) < val ) - ( val < T( 0 ) );
        }

        template<typename T> 
        struct three_d_plane {
            T alpha;
            T beta;
            T gamma;
            T delta;

            three_d_plane( const T alpha, const T beta, const T gamma, const T delta )
                : alpha( alpha ), beta( beta ), gamma( gamma ), delta( delta ) {}
        };

        template<typename T>
        struct three_d_point {
            T x;
            T y;
            T z;
        };

        template<typename T>
        three_d_plane<T> get_plane_through_three_points( 
            const three_d_point<T>& p,
            const three_d_point<T>& q,
            const three_d_point<T>& r
        ) {

            const T alpha = p.y * q.z + p.z * q.y - p.z * r.y + q.y * r.z - q.z * r.y;
            const T beta = p.z * q.x - p.x * q.z + p.x * r.z - p.z * r.x - q.x * r.z + q.z * r.x;
            const T gamma = p.x * q.y - p.y * q.x - p.x * r.y + p.y * r.x + q.x * r.y - q.y * r.x;
            const T delta = p.x * q.z * r.y - p.x * q.y * r.z + p.y * q.x * r.z - p.y * q.z * r.x - p.z * q.x * r.y + p.z * q.y * r.x;

            return three_d_plane( alpha, beta, gamma, delta );
        }

        template<typename T>
        struct err_bound {
            T A;
            T B;
            T C;

            constexpr T calc( const T& a, const T& b, const T& epsilon ) {
                return ( a + b * epsilon ) * epsilon;
            }

            constexpr err_bound( const T& aa, const T& ab,
                                 const T& ba, const T& bb,
                                 const T& ca, const T& cb,
                                 const T& epsilon )
                : A( calc( aa, ab, epsilon ) ), 
                  B( calc( ba, bb, epsilon ) ),
                  C( calc( ca, cb, epsilon ) ) {}
        };

        template<typename T>
        struct init_constants {
            T epsilon;
            T splitter;
            T result_err_bound;
            err_bound<T> ccw;
            err_bound<T> o3d;
            err_bound<T> icc;
            err_bound<T> isperr;

            constexpr init_constants( const T epsilon ) 
                : epsilon( epsilon ),
                  splitter( T( 1 ) + epsilon ),
                  result_err_bound( ( ( T )3 + T( 8 ) * epsilon ) * epsilon ),
                  ccw( 3, 16, 2, 12, 2, 12, epsilon ),
                  o3d( 7, 56, 3, 28, 26, 288, epsilon ),
                  icc( 10, 96, 4, 48, 44, 576, epsilon ),
                  isperr( 16, 224, 5, 72, 71, 1408, epsilon ) {}
        };

        constexpr init_constants<double> double_constants( std::numeric_limits<double>::epsilon() );

        template<typename T>
        std::tuple<T,T,T> two_one_diff( const T a_1, const T a_0, const T b ) {
            
            const auto [ _i, x_0 ] = two_diff( a_0, b );
            const auto [ x_2, x_1 ] = two_sum( a_1, _i );
            
            return std::make_tuple( x_2, x_1, x_0 );
        }

        template<typename T>
        std::tuple<T,T,T,T>
        two_two_diff( const T a_1, const T a_0, const T b_1, const T b_0 ) {

            const auto [ _j, _0, x_0 ] = two_one_diff( a_1, a_0, b_0 );
            const auto [ x_3, x_2, x_1 ] = two_one_diff( _j, _0, b_1 );
            
            return std::make_tuple( x_3, x_2, x_1, x_0 );
        }

        template<typename T>
        std::tuple<T,T> split( const T a ) {
            
            const T c = double_constants.splitter * a;
            const T a_big = c - a;
            const T a_hi = c - a_big;
            const T a_lo = a - a_hi;
            
            return std::make_tuple( a_hi, a_lo );
        }

        template<typename T> 
        T two_product_tail( const T a, const T b, const T x ) {

            const auto [ a_hi, a_lo ] = split( a );
            const auto [ b_hi, b_lo ] = split( b );

            const T err_1 = x - ( a_hi * b_hi );
            const T err_2 = err_1 - ( a_lo * b_hi );
            const T err_3 = err_2 - ( a_hi * b_lo );
            const T y = ( a_lo * b_lo ) - err_3;
            
            return y;
        }

        template<typename T> 
        std::tuple<T,T> two_product( const T a, const T b ) {

            T x = a * b;
            T y = two_product_tail( a, b, x );
            
            return std::make_tuple( x, y );
        }

        template<typename T>
        T estimate( const size_t e_len, std::tuple<T,T,T,T> e ) {
            T Q{};
            auto e_arr = tuple_to_array( e );
            for ( size_t i = 0; i < e_len; ++i ) {
                Q += e_arr[ i ];
            }
            return Q;
        }

        template<size_t N,typename T>
        void set_index( std::array<T,N>& e, const T value, const size_t index ) {

            if ( index < e.size() ) {
                e[ index ] = value;
            }
        }

        template<size_t N,typename T>
        T safe_get_index( const std::array<T,N> e, const size_t e_index ) {

            if ( e_index <= N && e_index <= e.size() ) {
                return e[ e_index ];
            } else {
                return ( T )0;
            }
        }

        template<size_t N,typename T>
        std::tuple<size_t,std::array<T,N>> fast_expansion_sum_zero_elim( const size_t e_len, const std::array<T,N-4>& e, 
                                                                         const size_t f_len, const std::array<T,4>& f,
                                                                         std::array<T,N>& h ) {
        
            T e_now = e[ 0 ];
            T f_now = f[ 0 ];

            size_t e_index = 1;
            size_t f_index = 1;

            T Q{};

            if ( ( f_now > e_now ) == ( f_now > -e_now ) ) {
                Q = e_now;
                e_index += 1;
                e_now = safe_get_index<N-4>( e, e_index );
            } else {
                Q = f_now;
                f_index += 1;
                f_now = safe_get_index<4>( f, f_index );
            }
         
            size_t h_index = 1;
            T hh{};

            if ( ( e_index <= e_len ) && ( f_index <= f_len ) ) {

                if ( ( f_now > e_now ) == ( f_now > -e_now ) ) {

                    std::tuple<T,T> sum = fast_two_sum( e_now, Q );
                    Q = std::get<0>( sum );
                    hh = std::get<1>( sum );
                    e_index += 1;
                    e_now = safe_get_index<N-4>( e, e_index );
                } else {
                    std::tuple<T,T> sum = fast_two_sum( f_now, Q );
                    Q = std::get<0>( sum );
                    hh = std::get<1>( sum );
                    f_index += 1;
                    f_now = safe_get_index<4>( f, f_index );
                }
            }

            if ( hh != 0 ) {
                set_index<N>( h, hh, h_index );
                h_index += 1;
            }

            while ( ( e_index <= e_len ) && ( f_index <= f_len ) ) {
                if ( ( f_now > e_now ) == ( f_now > -e_now ) ) {
                    std::tuple<T,T> sum = two_sum( Q, e_now );
                    Q = std::get<0>( sum );
                    hh = std::get<1>( sum );
                    e_index += 1;
                    e_now = safe_get_index<N-4>( e, e_index );
                } else {
                    std::tuple<T,T> sum = fast_two_sum( Q, f_now );
                    Q = std::get<0>( sum );
                    hh = std::get<1>( sum );
                    f_index += 1;
                    f_now = safe_get_index<4>( f, f_index );
                }

                if ( hh != 0 ) { 
                    set_index<N>( h, hh, h_index );
                    h_index += 1;
                }
            }
        
            while ( e_index <= e_len ) {
                
                std::tuple<T,T> sum = two_sum( Q, e_now );
                Q = std::get<0>( sum );
                hh = std::get<1>( sum );
                e_index += 1;
                e_now = safe_get_index<N-4>( e, e_index );
                
                if ( hh != 0 ) {
                    set_index<N>( h, hh, h_index );
                    h_index += 1;
                }
            }

            while ( f_index <= f_len ) {

                std::tuple<T,T> sum = two_sum( Q, f_now );
                Q = std::get<0>( sum );
                hh = std::get<1>( sum );
                f_index += 1;
                f_now = safe_get_index<4>( f, f_index );

                if ( hh != 0 ) {
                    set_index<N>( h, hh, h_index );
                    h_index += 1;
                }
            }

            if ( Q != 0 || h_index == 1 ) {
                set_index<N>( h, Q, h_index );
                h_index += 1;
            }

            return std::make_tuple( h_index - 1, h );
        }

        template<typename T>
        T orient_2_adapt( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r, const T detsum ) {

            std::cout << "called backup" << std::endl;

            const T ac_x = p.first - r.first;
            const T bc_x = q.first - r.first;
            const T ac_y = p.second - r.second;
            const T bc_y = q.second - r.second;

            auto [ det_left, det_left_tail ] = two_product( ac_x, bc_y );
            auto [ det_right, det_right_tail ] = two_product( ac_y, bc_x );

            const auto B = two_two_diff( det_left, det_left_tail, det_right, det_right_tail );

            T det = estimate( 4, B ); 
            T err_bound = double_constants.ccw.B * detsum;

            if ( ( det >= err_bound ) || ( -det >= err_bound ) ) {
                return det;
            }

            T ac_x_tail = two_diff_tail( p.first, r.first, ac_x );
            T bc_x_tail = two_diff_tail( q.first, r.first, bc_x );
            T ac_y_tail = two_diff_tail( p.second, r.second, ac_y );
            T bc_y_tail = two_diff_tail( q.second, r.second, bc_y );

            if ( ac_x_tail == 0 && ac_y_tail == 0 && bc_x_tail == 0 && bc_y_tail == 0 ) {
                return det;
            }

            err_bound = double_constants.ccw.C * detsum + double_constants.result_err_bound * std::abs( det );
            det += ( ac_x * bc_y_tail + bc_y * ac_x_tail ) - ( ac_y * bc_x_tail + bc_x * ac_y_tail );
        
            if ( ( det >= err_bound ) || ( -det >= err_bound ) ) {
                return det;
            }

            auto [ s_1, s_0 ] = two_product( ac_x_tail, bc_y );
            auto [ t_1, t_0 ] = two_product( ac_y_tail, bc_x );
            auto u = two_two_diff( s_1, s_0, t_1, t_0 );

            std::array<T, 8> h_8 = { T( 0 ) };

            auto B_arr = tuple_to_array( B );
            auto u_arr = tuple_to_array( u );

            const auto [ c_1_len, c_1 ] = fast_expansion_sum_zero_elim<8>( 4, B_arr, 4, u_arr, h_8 );

            std::tie( s_1, s_0 ) = two_product( ac_x, bc_y_tail );
            std::tie( t_1, t_0 ) = two_product( ac_y, bc_x_tail );
            u = two_two_diff( s_1, s_0, t_1, t_0 );
            u_arr = tuple_to_array( u ); 

            std::array<T,12> h_12 = { ( T )0 };
            const auto [ c_2_len, c_2 ] = fast_expansion_sum_zero_elim<12>( c_1_len, c_1, 4, u_arr, h_12 );

            std::tie( s_1, s_0 ) = two_product( ac_x_tail, bc_y_tail );
            std::tie( t_1, t_0 ) = two_product( ac_y_tail, bc_x_tail );
            u = two_two_diff( s_1, s_0, t_1, t_0 );
            u_arr = tuple_to_array( u );
        
            std::array<T,16> h_16 = { ( T )0 };
            const auto [ d_len, d ] = fast_expansion_sum_zero_elim<16>( c_2_len, c_2, 4, u_arr, h_16 );

            return d[ d_len ];
        }

        template<typename T>
        T orient_2( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {

            T det_left = ( p.first - r.first ) * ( q.second - r.second );
            T det_right = ( p.second - r.second ) * ( q.first - r.first );

            T det = det_left - det_right;

            T detsum{};

            if ( det_left > 0 ) {
                if ( det_right <= 0 ) {
                    return det;
                } else {
                    detsum = det_left + det_right;
                }
            } else if ( det_left < 0 ) {
                if ( det_right ) {
                    return det;
                } else {
                    detsum = -det_left - det_right;
                }
            } else {
                return det;
            }

            const T err_bound = double_constants.ccw.A * detsum;
            if ( ( det >= err_bound ) || ( -det >= err_bound ) ) {
                return det;
            }

            return orient_2_adapt( p, q, r, detsum );
        }

        struct custom_point;
        struct custom_points;
        struct custom_triangle;

        template<typename T>
        struct number_type;

        template<>
        struct number_type<custom_point> {
            using type = double;
        };

        template<>
        struct number_type<custom_points> {
            using type = double;
        };

        template<>
        struct number_type<custom_triangle> {
            using type = int32_t;
        };

        template<typename T>
        std::pair<T,T> get_point( const std::vector<std::pair<T,T>>& points, const size_t i ) {
            return points[ i ];
        }

        template<typename T>
        std::pair<T,T> get_point( const std::vector<std::vector<T>>& points, const size_t i ) {
            return { points[ 0 ][ i ], points[ 1 ][ i ] };
        }

        template<typename T>
        T get_boundary_nodes( const std::vector<T>& boundary_nodes, const size_t m ) {
            return boundary_nodes[ m ];
        }

        template<typename T>
        T get_boundary_nodes( const std::vector<std::vector<T>>& boundary_nodes, const size_t m, const size_t n ) {
            return get_boundary_nodes( boundary_nodes[ m ], n );
        }

        template<typename T>
        T get_boundary_nodes( const std::vector<std::vector<T>>& boundary_nodes, const std::pair<size_t,size_t> indices ) {
            return get_boundary_nodes( boundary_nodes, indices.first, indices.second );
        }

        template<typename Container>
        size_t num_boundary_edges( const Container& boundary_nodes ) {
            auto size = std::ssize( boundary_nodes );
            return size > 0 ? static_cast<size_t>( size - 1 ) : 0;
        }

        template<typename... Ts>
        size_t num_boundary_edges( const std::tuple<Ts...>& boundary_nodes ) {
            return sizeof...( Ts ) > 0 ? sizeof...( Ts ) - 1 : 0;
        }

        template<typename T>
        T dist_sqr( const std::pair<T, T>& p, const std::pair<T,T>& q ) {

            std::pair<T,T> vec = { q.first - p.first, q.second - p.second };
            return vec.first * vec.first + vec.second * vec.second;
        }

        /*
            calculate the distance between a point P and the
            line segment Q-R 
        */
        template<typename T>
        T squared_distance_to_segment( const T q_x, const T q_y,
                                       const T r_x, const T r_y, 
                                       const T p_x, const T p_y ) {

            // the x distance between P and Q
            T p_q_x = p_x - q_x;
            
            // the y distance between P and Q
            T p_q_y = p_y - q_y;

            // the x distance between Q and R
            T r_q_x = r_x - q_x;

            // the y distance between Q and R
            T r_q_y = r_y - q_y;

            // dot product of (P - Q) and (R - Q)
            T dot_product = p_q_x * r_q_x + p_q_y * r_q_y;

            // squared distance between Q and R
            T denom = dist_sqr( std::pair<T,T>{ q_x, q_y }, std::pair<T,T>{ r_x, r_y } );

            // degenerate case: Q = R
            if ( denom == T( 0 ) )
                // squared distance between P and Q
                return dist_sqr( std::pair<T,T>{ p_x, p_y }, std::pair<T,T>{ q_x, q_y } );

            /*
                if 0 < t < 1 then the closest point to P
                is somewhere between Q and R:
                t = 0: closest to Q
                t = 1: closest to R
            */
            T t = dot_product / denom;

            /*
                it's possible that t < 0 or t > 1

                in these cases we need to clamp t 
                so that t = 0 or t = 1
            */
            T clamped_t = std::min( std::max( t, ( T )0 ), ( T )1 );

            T intersect_x = q_x + clamped_t * r_q_x;
            T intersect_y = q_y + clamped_t * r_q_y;

            T delta_2 = dist_sqr( std::pair<T,T>{ p_x, p_y }, std::pair<T,T>{ intersect_x, intersect_y } );

            return delta_2;
        }

        /*
            does a horizontal line that extends from the point p
            intersect a line segment extending between the points
            q and r?

            If so, at what x co-ordinate does that horizontal
            line intersect with that line segment?

        */
        template<typename T,typename Points,typename BoundaryNodes>
        T distance_to_polygon_single_segment( const std::pair<T,T>& p, 
                                              const Points& points,
                                              const BoundaryNodes& boundary_nodes,
                                              bool is_in_outer = false, 
                                              bool return_sqrt = true ) {

            T p_x = p.first;
            T p_y = p.second;

            //using F = number_type<Points>::type;

            T dist = std::numeric_limits<T>::max();

            const size_t n_edge = num_boundary_edges( boundary_nodes );

            // get the global index of the first node in the boundary
            auto q_index = get_boundary_nodes( boundary_nodes, 0 );

            // get the co-ord ( x, y ) at that global index
            auto q = get_point( points, q_index );

            // the starting point of the line segment
            T q_x = q.first;
            T q_y = q.second;

            for ( size_t i = 1; i <= n_edge; ++i ) {

                auto r_index = get_boundary_nodes( boundary_nodes, i );
                auto r = get_point( points, r_index );

                // the end point of the line segment
                T r_x = r.first; 
                T r_y = r.second;

                // p_y lies between q_y and r_y
                if ( ( r_y > p_y ) != ( q_y > p_y ) ) {

                    /*
                        the point where the hypothetical hotizontal line 
                        intersects with the line segment
                    */
                    T x_intersect = ( q_x - r_x ) * ( p_y - r_y ) / ( q_y - r_y ) + r_x;

                    // ( p_x, p_y ) lies on the inside of the line segment
                    if ( p_x < x_intersect ) {
                        is_in_outer = !is_in_outer;
                    }
                }

                T new_dist = squared_distance_to_segment( q_x, q_y, r_x, r_y, p_x, p_y );
                dist = new_dist < dist ? new_dist : dist;

                q_x = r_x;
                q_y = r_y;
            }

            dist = return_sqrt ? std::sqrt( dist ) : dist;
            return is_in_outer ? dist : -dist;
        }


        template<typename BoundaryNodes>
        size_t num_sections( const BoundaryNodes& boundary_nodes ) {

            return boundary_nodes.size();
        }

        template<typename T,typename Points,typename BoundaryNodes>
        T distance_to_polygon_multiple_segments( const std::pair<T,T>& p, 
                                                 const Points& points,
                                                 const BoundaryNodes& boundary_nodes,
                                                 bool is_in_outer = false, 
                                                 bool return_sqrt = true ) {

            using InnerBoundaryNodess = std::decay_t<decltype(boundary_nodes[0])>;

            T dist = std::numeric_limits<T>::max();
            
            const size_t ns = num_sections( boundary_nodes );

            for ( size_t i = 0; i < ns; ++i ) {

                InnerBoundaryNodess bn = get_boundary_nodes( boundary_nodes, i );

                T new_dist = distance_to_polygon_single_segment( p, points, bn, is_in_outer == true, false );

                is_in_outer = sign( new_dist ) == 1 ? true : false;
                new_dist = std::abs( new_dist );

                dist = new_dist < dist ? new_dist : dist;
            }

            dist = return_sqrt ? std::sqrt( dist ) : dist;
            return is_in_outer ? dist : -dist;
        }

        template<typename T,typename Points,typename BondaryNodes>
        struct cell {
            T x;
            T y;
            T half_width;
            T dist;
            T max_dist;

            cell( T x, T y, T half_width, Points points, BondaryNodes boundary_nodes )
                : x( x ), y( y ), half_width( half_width ) {

                std::pair<T,T> q = { x, y };

                dist = distance_to_polygon( q, points, boundary_nodes );

                max_dist = dist + half_width * std::sqrt( 2 );
            }
        };

        template<typename T>
        typename std::enable_if<
            std::is_arithmetic<typename T::value_type>::value,
            bool
        >::type
        has_multiple_sections( const T& ) {
            return false;
        }

        template<typename T>
        typename std::enable_if<
            std::is_same<T, std::vector<std::vector<typename T::value_type::value_type>>>::value, 
            bool
        >::type
        has_multiple_sections(const T&) {
            return true;
        }

        template<typename T>
        typename std::enable_if<
            std::is_same<T, std::vector<std::vector<std::vector<typename T::value_type::value_type>>>>::value,
            bool
        >::type
        has_multiple_sections( const T& ) {
            return true;
        }

        template<typename T>
        T mid_point( T a, T b ) {

            static_assert( a != b && "Can't get mid-point of equal numbers" );
      
            if ( a < b ) {
                return a + ( b - a ) / 2;
            } else {
                return b + ( a - b ) / 2;
            }
        }

        template<typename T>
        void polygon_bounds_multiple_segments() {
        }

        template<typename T>
        struct polygon_bounds {
            T xmin;
            T xmax;
            T ymin;
            T ymax;

            polygon_bounds() 
                : xmin( std::numeric_limits<T>::max() ),
                  xmax( std::numeric_limits<T>::min() ),
                  ymin( std::numeric_limits<T>::max() ),
                  ymax( std::numeric_limits<T>::min() ) {}

            polygon_bounds( const T& xmin, const T& xmax, const T& ymin, const T& ymax ) 
                : xmin( xmin ), xmax( xmax ), ymin( ymin ), ymax( ymax ) {}

            void update( const std::pair<T,T>& p ) {
                xmin = std::min( p.first, xmin );
                xmax = std::max( p.first, xmax );
                ymin = std::min( p.second, ymin );
                ymax = std::max( p.second, ymax );
            }
        };

        template<typename T,typename Points,typename BoundaryNodes>
        polygon_bounds<T> get_polygon_bounds_single_segment( const Points points, 
                                                             const BoundaryNodes& boundary_nodes, 
                                                             bool check_all_curves = false ) {

            polygon_bounds<T> pb;
            
            const size_t n_edge = num_boundary_edges( boundary_nodes );

            for ( size_t i = 0; i < n_edge; ++ i ) {
                
                const auto p_index = get_boundary_nodes( boundary_nodes, i );
                const auto p = get_point( points, p_index );
                pb.update( p );
            }

            return pb;
        }

        template<typename T,typename Points,typename BoundaryNodes>
        polygon_bounds<T> get_polygon_bounds( const Points points, 
                                              const BoundaryNodes& boundary_nodes, 
                                              bool check_all_curves = false ) {

            polygon_bounds_multiple_segments<T>();

            auto polygon_bounds = get_polygon_bounds_single_segment( points, boundary_nodes );

            return polygon_bounds;
        }

        template<typename I,typename T>
        struct representative_coords {
            T x;
            T y;
            I n;

            representative_coords( T x, T y, T n ) : x( x ), y( y ), n( n ) {}
        };

        template<typename I,typename T>
        using BPL = std::map<I, representative_coords<I,T>>;

        template<typename I,typename T>
        class triangulation;
        
        template<typename I,typename T>
        void reset_representative_coordinates( triangulation<I,T>& tri ) {
            for ( auto& representative_point : tri.get_representative_points() ) {
                representative_point.x = ( T )0;
                representative_point.y = ( T )0;
                representative_point.n = ( I )0; 
            } 
        }

        template<typename I,typename T>
        class triangulation {
        public:

            using integer_type = I;

            triangulation( const std::vector<std::pair<T,T>>&& points, const std::vector<I>&& boundary ) 
                : points( std::move( points ) ), boundary( std::move( boundary ) ) {}

            // Rule-of-Zero
            BPL<I,T> get_representative_points() {
                return representative_points;
            }
        private:
            BPL<I,T> representative_points;
            std::vector<std::pair<T,T>> points;
            std::vector<I> boundary;
        };

        using triangle = std::tuple<int,int,int>;

        struct compare_triangles {
            bool operator()( const triangle& a, const triangle& b ) const {
                return std::get<0>( a ) < std::get<0>( b );
            }
        };

        using triangle_set = std::set<triangle, compare_triangles>;

        inline size_t sub_2_ind( const size_t x, const size_t y, const size_t xdim ) {

            return std::fma( y, xdim, x );
        }

        inline triangle_set get_lattice_triangles( const size_t ydim, const size_t xdim ) {

            triangle_set triangles = {};

            for ( size_t y = 1; y < ydim; ++y ) {
                for ( size_t x = 1; x < xdim; ++x ) {

                    int u = sub_2_ind( x, y - 1, xdim );
                    int v = sub_2_ind( x + 1, y - 1, xdim ); 
                    int w = sub_2_ind( x, y, xdim );

                    triangles.insert( std::make_tuple( u, v, w ) );

                    u = sub_2_ind( x, y, xdim );
                    v = sub_2_ind( x + 1, y - 1, xdim ); 
                    w = sub_2_ind( x + 1, y, xdim );

                    triangles.insert( std::make_tuple( u, v, w ) );
                }
            }

            return triangles;
        }

        template<typename T>
        std::vector<std::pair<T,T>> get_lattice_points( const T a, const T b, 
                                                        const T c, const T d, 
                                                        const size_t xdim, const size_t ydim ) {
            
            std::vector<std::pair<T,T>> points( ydim * xdim, { ( T )0, ( T )0 } );

            T delta_x = ( b - a ) / ( static_cast<T>( xdim - 1 ) );
            T delta_y = ( d - c ) / ( static_cast<T>( ydim - 1 ) );
        
            for ( size_t y = 0; y < ydim; ++y ) {

                T y_ = c + static_cast<T>( y ) * delta_y;

                for ( size_t x = 0; x < xdim; ++x ) {

                    T x_ = a + static_cast<T>( x ) * delta_x; 

                    const size_t index = sub_2_ind( x, y, xdim );
                    points[ index ] = { x_, y_ }; 
                }
            }

            return points;
        }

        inline std::vector<int> get_lattice_boundary( const size_t xdim, const size_t ydim ) {
   
            std::vector<int> boundary( 2 * xdim + 2 * ydim - 4 );
    
            for ( size_t x = 0; x < xdim; ++x ) {

                boundary[ x ] = sub_2_ind( x, 0, xdim );
                boundary[ xdim + x ] = sub_2_ind( x, ydim - 1, xdim );
            }

            for ( size_t y = 1; y < ydim - 1; ++y ) {

                boundary[ 2 * xdim + y - 1 ] = sub_2_ind( 0, y, xdim );
                boundary[ 2 * xdim + ( ydim - 2 ) + y - 1 ] = sub_2_ind( xdim - 1, y, xdim );
            }

            return boundary;
        }

        template<typename I,typename T>
        inline triangulation<I,T> triangulate_rectangle( const T a, const T b, 
                                                         const T c, const T d,
                                                         const size_t xdim, const size_t ydim,  
                                                         bool single_boundary = false ) {

            auto lattice_triangles = get_lattice_triangles( ydim, xdim );
            auto points = get_lattice_points( a, b, c, d, xdim, ydim );
            auto boundary = get_lattice_boundary( xdim, ydim );
            
            auto tri = triangulation( std::move( points ), std::move( boundary ) );

            return tri;
        }

        template<typename I>
        std::map<std::pair<I,I>, std::pair<I,I>> construct_boundary_edge_contiguous( std::vector<I> boundary_nodes ) {

            std::map<std::pair<I,I>,std::pair<I,I>> edges;

            const size_t num_edges = num_boundary_edges( boundary_nodes );

            for ( size_t i = 0; i < num_edges; ++ i ) {
                const I u = get_boundary_nodes( boundary_nodes, i );
                const I v = get_boundary_nodes( boundary_nodes, i + 1 );
                edges[ { u, v } ] = { u, v };
            }

            return edges;
        }

        template<typename T>
        struct bounding_box {

            T xmin;
            T xmax;
            T ymin;
            T ymax;

            bounding_box( const T& xmin, const T& xmax, const T& ymin, const T& ymax ) 
                : xmin( xmin ), xmax( xmax ), ymin( ymin ), ymax( ymax ) {}
        };

        template<typename I>
        struct polygon_tree {

            std::optional<polygon_tree> parent;
            std::set<polygon_tree> children;
            I index;
            int height;
        };

        template<typename T>
        concept HasInfinity = requires {
            { std::numeric_limits<T>::infinity() } -> std::same_as<T>;
        };

        template<typename T>
        requires HasInfinity<T>
        bounding_box<T> get_bounding_box( const std::vector<std::pair<T,T>>& points ) {

            T xmin = std::numeric_limits<T>::infinity();
            T xmax = -std::numeric_limits<T>::infinity();

            T ymin = std::numeric_limits<T>::infinity();
            T ymax = -std::numeric_limits<T>::infinity();

            for ( const auto& point : points ) {

                xmin = std::min( xmin, point.first );
                xmax = std::max( xmax, point.first );
                ymin = std::min( ymin, point.second );
                ymax = std::max( ymax, point.second );
            }

            return bounding_box( xmin, xmax, ymin, ymax );
        }

        template<typename I,typename T>
        struct polygon_heirarchy {

            std::vector<bool> polygon_orientations;
            std::vector<bounding_box<T>> bounding_boxes;
            std::map<I,polygon_tree<I>> trees;

            polygon_heirarchy() {
                polygon_orientations.push_back( true );
            }
        };

        template<typename T>
        T triangle_area( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {

            const T A = orient_2( p, q, r ) / ( T )2;
            return A;
        }

        template<typename I,typename T>
        std::pair<T,T> triangle_orthocenter( const triangulation<I,T>& tri, const triangle& t ) {

            const std::pair<T,T> p = get_point( tri, std::get<0>( t ) );
            const std::pair<T,T> q = get_point( tri, std::get<1>( t ) );
            const std::pair<T,T> r = get_point( tri, std::get<2>( t ) );

            const T a = get_weight( tri, std::get<0>( t ) );
            const T b = get_weight( tri, std::get<1>( t ) );
            const T c = get_weight( tri, std::get<2>( t ) );

            const T A = triangle_area( p, q, r );

            const T d_11 = dist_Sqr( p, r ) + c - a;
            const T d_12 = p.second - r.second;
            const T d_21 = dist_sqr( q, r ) + c - b;
            const T d_22 = q.second - r.second;
            
            const T o_x = r.first + ( d_11 * d_22 - d_12 * d_21 ) / ( 4 * A );
            const T e_11 = p.fist - r.first;
            const T e_12 = d_11;
            const T e_21 = q.first - r.first;
            const T e_22 = d_21;
            const T o_y = r.second + ( e_11 * e_22 - e_12 * e_21 ) / ( 4 * A );

            return { o_x, o_y };
        }

    } // namespace fvm

} // namespace fs 

#endif 