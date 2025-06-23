#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>

#include <algorithm>
#include <limits>
#include <iterator>
#include <optional>
#include <numeric>
#include <random>

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <ranges>

#include <concepts>
#include <type_traits>

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

namespace fs {

    namespace fvm {

        template<typename T>
        std::pair<T,T> get_difference( const std::pair<T,T>& p,
                                       const std::pair<T,T>& q ) {

            return { p.first - q.first, p.second - q.second };
        }

        enum class relative_position {
            LEFT,
            ON,
            RIGHT
        };

        enum class orient {
            POSITIVE,
            DEGENERATE,
            NEGATIVE
        };

        enum class in_circle {
            INSIDE,
            ON,
            OUTSIDE
        };

        template<typename T>
        T inner_product(
            const std::pair<T,T>& p,
            const std::pair<T,T>& q
        ) {
            return p.first * q.first + p.second * q.second;
        }

        template<typename T>
        using VectorOfPairs = std::vector<std::pair<T,T>>;

        template<typename T>
        using PairOfVectors = std::pair<std::vector<T>,std::vector<T>>; 

        template<typename T>
        T exterior_product( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
            return p.first * q.second - p.second * q.first;   
        }

        template<typename I,typename Edge>
        struct adjacent_2_vertex { 
            std::map<I,std::set<Edge>> adjacent_2_vertex_map;

            std::map<I,std::set<Edge>>& get_adjacent_2_vertex() {
                return adjacent_2_vertex_map;
            }

            void add( const I u, const I v, const I w ) {
                adjacent_2_vertex_map[ u ] = construct_edge( v, w );
            }

            void remove( const I u, const I v, const I w ) {
                Edge e = construct_edge( v,  w );
                adjacent_2_vertex_map[ u ].erase( e );
            }
        };

        template<typename I,typename Edge>
        struct adjacent {
            std::map<Edge,I> adjacent_map;

            std::map<Edge,I>& get_adjacent() {
                return adjacent_map;
            }

            void add( const I u, const I v, const I w ) {
                adjacent_map[ construct_edge( u, v ) ] = w;
            }

            void remove( const I u, const I v ) {
                adjacent_map.remove( construct_edge( u, v ) );
            }
        };

        template<typename T,size_t rows,size_t cols>
        struct matrix_ {

            std::array<T,rows*cols> elements;

            matrix_() : elements{} {}

            matrix_( const std::array<T,rows*cols>& elements )
                : elements( elements ) {} 

            T& operator[]( size_t row, size_t col ) {
                return elements[ col + row * cols ];
            }

            const T& operator[]( size_t row, size_t col ) const {
                return elements[ col + row * cols ];
            }

            void swap_rows( size_t row_a, size_t row_b ) {

                if ( row_a == row_b ) return; 

                std::swap_ranges( elements.begin() + row_a * cols, 
                                  elements.begin() + row_a * cols + cols, 
                                  elements.begin() + row_b * cols );
            }
        };

        using triangle = std::tuple<int,int,int>;

        struct compare_triangles {
            bool operator()( const triangle& a, const triangle& b ) const {
                if ( std::get<0>( a ) != std::get<0>( b ) ) return std::get<0>( a ) < std::get<0>( b );
                if ( std::get<1>( a ) != std::get<1>( b ) ) return std::get<1>( a ) < std::get<1>( b );
                return std::get<2>( a ) < std::get<2>( b );
            }
        };

        using triangle_set = std::set<triangle, compare_triangles>;

        template <typename... Ts, std::size_t... I>
        auto tuple_to_array_impl( const std::tuple<Ts...>& t, std::index_sequence<I...> ) {
            return std::array{ std::get<I>( t )... }; 
        }

        template <typename... Ts>
        auto tuple_to_array( const std::tuple<Ts...>& t ) {
            return tuple_to_array_impl( t, std::index_sequence_for<Ts...>{} );
        }

        inline bool is_ghost( const triangle& t ) {
            auto arr = tuple_to_array( t );
            return std::any_of( arr.begin(), arr.end(), []( int value ) { return value < 0; } );
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
            
            return { x, y };
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

            auto [ ac_x, ac_y ] = get_difference( p, r );
            auto [ bc_x, bc_y ] = get_difference( q, r );

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

        /*
            get_point()
            get the ( x, y ) co-ordinates of a vertex

            i: index of the point in the "points" data-structure

            return the ( x, y ) co-ordinates of the vertices as a pair
        */
        template<typename T>
        std::pair<T,T> get_point( const std::vector<std::pair<T,T>>& points, const size_t i ) {
            return points[ i ];
        }

        /*
            get_point()
            get the ( x, y ) co-ordinates of a vertex

            i: index of the point in the "points" data-structure

            return the ( x, y ) co-ordinates of the vertices as a pair
        */
        template<typename T>
        std::pair<T,T> get_point( const std::vector<std::vector<T>>& points, const size_t i ) {
            return { points[ 0 ][ i ], points[ 1 ][ i ] };
        }

        /*
            get_triangle_points()
            get the ( x, y ) co-ordinates of each vertex in the triangle

            i, j, k: indices of the points in the "points" data-structure

            return the ( x, y ) co-ordinates of the vertices and array of 3 elements
        */
        template<typename T>
        std::array<std::pair<T,T>,3> get_triangle_points( 
            const std::pair<std::vector<T>,std::vector<T>>& points, 
            const size_t i, const size_t j, const size_t k 
        ) {
            return { std::pair<T,T>( points.first[ i ], points.second[ i ] ) , 
                     std::pair<T,T>( points.first[ j ], points.second[ j ] ) ,
                     std::pair<T,T>( points.first[ k ], points.second[ k ] ) };
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
            does a horizontal line that extends from the point p intersect a line segment extending between
            the points q and r?

            If so, at what x co-ordinate does that horizontal line intersect with that line segment?

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
                                                 const bool return_sqrt = true ) {

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
        T get_mid_point( T a, T b ) {
      
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

            representative_coords( T x, T y, T n ) : x_( x ), y_( y ), n_( n ) {}

            void add_point( const std::pair<T,T>& p ) {
                x_ = 1 / ( n_ + 1 ) * ( n_ * x_ + p.first );
                y_ = 1 / ( n_ + 1 ) * ( n_ * y_ + p.second );
                n_ += 1;
            }

            void delete_point( const std::pair<T,T>& p ) {
                x_ = 1 / ( n_ - 1 ) * ( n_ * x_ - p.first );
                y_ = 1 / ( n_ - 1 ) * ( n_ * y_ - p.second );
                n_ += 1;
            }

            void reset() {
                x_ = ( T )0;
                y_ = ( T )0;
                n_ = ( I )0; 
            }

            void compute_centroid( const VectorOfPairs<T>& points ) {
                reset();

                for ( auto& p : points ) {
                    add_point( p );
                }
            }

            T x_;
            T y_;
            I n_;
        };

        template<typename I,typename T>
        using BPL = std::map<I, representative_coords<I,T>>;

        template<typename I,typename Edge>
        struct graph {

            bool has_vertex( const I val ) {

                if ( vertices.find( val ) != vertices.end() ) {
                    return true;
                } else {
                    return false;
                }
            }

            bool has_ghost_vertices() {
                return std::any_of( vertices.begin(), vertices.end(), []( const auto& vertex ) {
                    vertex == -1;
                });
            }

            void add_neighbor( I u, I v ) {
                neighbors[ u ].insert( v );
            }

            template<typename... Args>
            void add_neighbor( I u, Args... v) {
                ( add_neighbor( u, v ), ... );  
            }

            void add_vertex( I u, I v ) {
                if ( has_vertex( v ) ) return;

                vertices.insert( v);
                if ( neighbors.find( u ) == neighbors.end() ) {
                    neighbors[ v ] = std::set<I>();
                }
            }

            template<typename... Args>
            void add_vertex( I u, Args... v) {
                ( add_vertex( u, v ), ... );  
            }

            void add_edge( const I u, const I v ) {
                edges.insert( construct_edge( v, u ) );
            }

            void delete_edge( const I u, const I v ) {
                edges.erase( construct_edge( u, v ) );
                edges.erase( construct_edge( v, u ) );
            }

            void delete_neighbor( const I u, const I v ) {
                _delete( u, v );
                _delete( v, u );
                delete_edge( u, v );

            } 

            template<typename... Args>
            void delete_neighbor( I u, Args... v) {
                ( delete_neighbor( u, v ), ... );  
            }

            void add_triangle( const I u, const I v, const I w ) {

                add_vertex( u, v, w );
                add_neighbor( u, v, w );
                add_neighbor( v, w );
            }

            void delete_triangle( const I u, const I v, const I w ) {

                delete_neighbor( u, v, w );
                delete_neighbor( v, w );
            }

            void clear_empty_vertices() {
                for ( auto v : vertices ) {
                    const size_t n = neighbors[ v ].size();
                    if ( n == 0 ) {
                        vertices.remove( v );
                    }
                }
            }

            void delete_( const I u, const I v ) {

                if ( neighbors.find( u ) == neighbors.end() ) {
                    neighbors[ u ] = std::set<I>();
                }

                neighbors[ u ].erase( v );

                if ( neighbors[ u ].empty() ) {
                    vertices.erase( u );
                }
            }

            std::set<I> vertices;
            std::set<Edge> edges;
            std::map<I,std::set<I>> neighbors;
        };

        template<typename I,typename T,typename Points = VectorOfPairs<T>>
        class triangulation {
        public:

            using integer_type = I;

            triangulation( const std::vector<std::pair<T,T>>&& points ) 
                : points( std::move( points ) ) {}

            triangulation( const triangle_set&& triangles,
                           const std::vector<std::pair<T,T>>&& points, 
                           const std::vector<I>&& boundary ) 
                : triangles( std::move( triangles ) ), 
                  points( std::move( points ) ), 
                  boundary( std::move( boundary ) ) {}

            // Rule-of-Zero
            BPL<I,T>& get_representative_points() {
                return representative_points;
            }

            T get_weight( const size_t i ) const {
                return ( T )0;
            }

            std::pair<T,T> get_point( const size_t i ) const {
                return points[ i ];
            } 

            const Points& get_points() const {
                return points;
            }

            const triangle_set& get_triangles() const {
                return triangles;
            }

            const size_t num_points() const {
                if constexpr ( std::is_same_v<Points,std::vector<std::pair<T,T>>> ) {
                    return points.size();  
                } else if constexpr ( std::is_same_v<Points,std::pair<std::vector<T>,std::vector<T>>> ) {
                    return points.first.size();  
                }
            }
        private:
            triangle_set triangles;
            BPL<I,T> representative_points;
            Points points;
            std::vector<I> boundary;
        };

        template<typename I,typename T>
        void update_centroid_after_addition
        (
            triangulation<I,T>& tri,  
            const size_t curve_index, 
            const std::pair<T,T>& p
        ) {
            auto representative_points = tri.get_representative_points();
            auto centroid = representative_points[ curve_index ];
            centroid.add_point( p );
        }

        template<typename I,typename T>
        void update_centroid_after_deletion
        (
            triangulation<I,T>& tri,  
            const size_t curve_index, 
            const std::pair<T,T>& p
        ) {
            auto representative_points = tri.get_representative_points();
            auto centroid = representative_points[ curve_index ];
            centroid.delete_point( p );
        }

        template<typename I,typename T>
        std::pair<T,T> get_representative_point_coordinates
        (
            triangulation<I,T>& tri,  
            const size_t curve_index
        ) {
            auto representative_points = tri.get_representative_points();
            auto centroid = representative_points[ curve_index ];
            return { centroid.x_, centroid.y_ };
        }

        template<typename I,typename T>
        void reset_representative_coordinates( triangulation<I,T>& tri ) {
            for ( auto& representative_point : tri.get_representative_points() ) {
                representative_point.x = ( T )0;
                representative_point.y = ( T )0;
                representative_point.n = ( I )0; 
            } 
        }

        inline size_t sub_2_ind( const size_t x, const size_t y, const size_t xdim ) {

            return y * xdim + x;
        }

        inline triangle_set get_lattice_triangles( const size_t ydim, const size_t xdim ) {

            triangle_set triangles = {};

            for ( size_t y = 0; y < ydim - 1; ++y ) {
                for ( size_t x = 0; x < xdim - 1; ++x ) {

                    int u = sub_2_ind( x, y, xdim );
                    int v = sub_2_ind( x + 1, y, xdim ); 
                    int w = sub_2_ind( x, y + 1, xdim );

                    triangles.insert( std::make_tuple( u, v, w ) );
  
                    u = sub_2_ind( x, y + 1, xdim );
                    v = sub_2_ind( x + 1, y, xdim ); 
                    w = sub_2_ind( x + 1, y + 1, xdim ); 
                    
                    triangles.insert( std::make_tuple( u, v, w ) );
                }
            }

            return triangles;
        }

        /*
            get_lattice_points()
        */
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
        triangulation<I,T> triangulate_rectangle( const T a, const T b, 
                                                  const T c, const T d,
                                                  const size_t xdim, const size_t ydim,  
                                                  bool single_boundary = false ) {

            auto lattice_triangles = get_lattice_triangles( ydim, xdim );
            auto points = get_lattice_points( a, b, c, d, xdim, ydim );
            auto boundary = get_lattice_boundary( xdim, ydim );
            
            auto tri = triangulation( std::move( lattice_triangles ), std::move( points ), std::move( boundary ) );

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

        template<typename T>
        struct zero_weight {};

        inline const zero_weight<double> zw;

        template<typename T>
        std::tuple<T,T> min_max( const T& a, const T& b ) {
            return std::make_tuple( std::min( a, b ), std::max( a, b ) );
        }

        template<typename T>
        std::tuple<T,T,T> min_med_max( T a, T b, T c ) {

            std::tie( b, c ) = min_max( b, c );
            std::tie( a, c ) = min_max( a, c );
            std::tie( a, b ) = min_max( a, b );

            return std::make_tuple( a, b, c );
        }

        template<typename I,typename T>
        std::pair<T,T> triangle_orthocenter( const triangulation<I,T>& tri, const triangle& t ) {

            const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
            const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
            const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );

            const T a = tri.get_weight( std::get<0>( t ) );
            const T b = tri.get_weight( std::get<1>( t ) );
            const T c = tri.get_weight( std::get<2>( t ) );

            const T A = triangle_area( p, q, r );

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

        /*
            l_1, l_2, l_3: the squared lengths of the sides of a the triangle
        */
        template<typename T>
        T triangle_area( const T l_1, const T l_2, const T l_3 ) {

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

        template<typename T>
        std::tuple<T,T,T,size_t> squared_triangle_lengths_with_smallest_index(
            const std::pair<T,T>& p,
            const std::pair<T,T>& q,
            const std::pair<T,T>& r
        ) {

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

        template<typename T>
        std::tuple<T,T,T> triangle_angles(
            const std::pair<T,T>& p,
            const std::pair<T,T>& q,
            const std::pair<T,T>& r
        ) {

            const T A = triangle_area( p, q, r );

            T a_x = p.first - q.first;
            T b_y = p.second - q.second;
            T b_x = p.first - r.first;
            T a_y = p.second - r.second;

            T dot_ab = a_x * b_x + a_y * b_y;
            T theta_1 = dot_ab == 0 ? ( M_PI / 2.0 ) : std::atan( 2 * A / dot_ab );

            if ( theta_1 < 0 ) {
                theta_1 += M_PI;
            }

            a_x = q.first - p.first;
            b_y = q.second - p.second;
            b_x = q.first - r.first;
            a_y = q.second - r.second;
            
            dot_ab = a_x * b_x + a_y * b_y;

            T theta_2 = dot_ab == 0 ? ( M_PI / 2.0 ) : std::atan( 2 * A / dot_ab );

            if ( theta_2 < 0 ) {
                theta_2 += M_PI;
            }

            a_x = r.first - p.first;
            b_y = r.second - p.second;
            b_x = r.first - q.first;
            a_y = r.second - q.second;

            T theta_3 = dot_ab == 0 ? ( M_PI / 2.0 ) : std::atan( 2 * A / dot_ab );

            if ( theta_3 < 0 ) {
                theta_3 += M_PI;
            }

            return min_med_max( theta_1, theta_2, theta_3 );
        }

        template<typename T>
        std::tuple<T,T,T> squared_triangle_lengths(
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r ) {

            T l_1{};
            T l_2{};
            T l_3{};
    
            std::tie( l_1, l_2, l_3, std::ignore ) = squared_triangle_lengths_with_smallest_index( p, q, r );
            
            return std::make_tuple( l_1, l_2, l_3 );
        }

        template<typename T>
        std::tuple<T,T,T> triangle_lengths(
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r ) {

            T l_1{};
            T l_2{};
            T l_3{};
    
            std::tie( l_1, l_2, l_3, std::ignore ) = squared_triangle_lengths_with_smallest_index( p, q, r );
            
            return std::make_tuple( std::sqrt( l_1 ), std::sqrt( l_2 ), std::sqrt( l_3 ) );
        }

        template<typename T>
        T triangle_perimeter(
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r ) {

            T l_1{};
            T l_2{};
            T l_3{};
    
            std::tie( l_1, l_2, l_3 ) = triangle_lengths( p, q, r );
            
            return l_1 + l_2 + l_3;
        }

        template<typename T>
        std::pair<T,T> get_triangle_circumcenter( 
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r,
            const T A
        ) {

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

        template<typename T>
        std::pair<T,T> get_triangle_circumcenter( 
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r
        ) {

            T A = triangle_area( p, q, r );
            return get_triangle_circumcenter( p, q, r, A );
        }

        template<typename T>
        T triangle_inradius( const T A, const T perimeter ) {
            return 2 * A / perimeter;
        }

        template<typename T>
        T triangle_inradius( 
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q, 
            const std::pair<T,T>& r
        ) {

            T A = triangle_area( p, q, r );
            T perimeter = triangle_perimeter( p, q, r );
            return triangle_inradius( A, perimeter );
        }

        template<typename T>
        T triangle_circumradius( const T A, const T l_min_sqr, const T l_med_sqr, const T l_max_sqr ) {
            return std::sqrt( l_min_sqr * l_med_sqr * l_max_sqr ) / ( 4 * A );
        }

        template<typename T>
        T triangle_circumradius( const std::pair<T,T>& p, 
                                 const std::pair<T,T>& q, 
                                 const std::pair<T,T>& r ) {

            const auto [ l_1_sqr, l_2_sqr, l_3_sqr ] = squared_triangle_lengths( p, q, r );
            T A = triangle_area( p, q, r );
            return triangle_circumradius( A, l_1_sqr, l_2_sqr, l_3_sqr );
        }

        inline auto get_solid_triangles( const triangle_set& triangles ) {
            return triangles | std::views::filter([]( const triangle& t ) { return !is_ghost( t ); });
        }

        template<typename T>
        three_d_point<T> get_mid_point( const three_d_point<T>& p, 
                                        const three_d_point<T>& q ) {

            three_d_point<T> mid_point;

            mid_point.x = get_mid_point( p.x, q.x );
            mid_point.y = get_mid_point( p.y, q.y );
            mid_point.z = get_mid_point( p.z, q.z );

            return mid_point;
        }

        template<typename T>
        three_d_point<T> get_cross_product( const three_d_point<T>& p, 
                                            const three_d_point<T>& q ) {

            three_d_point<T> cross_product;

            cross_product.x = p.y * q.z - p.z * q.y;
            cross_product.y = p.z * q.x - p.x * q.z;
            cross_product.z = p.x * q.y - p.y * q.x;

            return cross_product;
        }

        template<typename T>
        three_d_point<T> sum( const three_d_point<T>& p, 
                              const three_d_point<T>& q ) {
            
            three_d_point<T> sum;

            sum.x = p.x + q.x;
            sum.y = p.y + q.y;
            sum.z = p.z + q.z;

            return sum;
        }

        template<typename T>
        three_d_point<T> get_difference( const three_d_point<T>& p, 
                                         const three_d_point<T>& q ) {
            
            three_d_point<T> difference;

            difference.x = p.x - q.x;
            difference.y = p.y - q.y;
            difference.z = p.z - q.z;

            return difference;
        }

        template<typename T>
        std::pair<T,T> get_sum( const std::pair<T,T>& p, 
                                const std::pair<T,T>& q ) {
            return { p.first + q.first, p.second + q.second };
        }

        template<typename T> 
        bool is_zero_vector( const three_d_point<T>& v ) {
            return ( v.x == 0 && v.y == 0 && v.z == 0 );
        }

        template<typename T>
        T get_dot_product( const three_d_point<T>& p, const three_d_point<T>& q ) {
            return p.x * q.x + p.y * q.y + p.z * q.z;
        }

        template<typename T>
        three_d_point<T> scale( const T c, const three_d_point<T>& v ) {
            return three_d_point( c * v.x, c * v.y, c * v.z );
        } 

        template<typename T>
        struct matrix {
            three_d_point<T> a;
            three_d_point<T> b;
            three_d_point<T> c;

            matrix( const three_d_point<T>& p, const three_d_point<T>& q, const three_d_point<T>& r )
                : a( p ), b( q ), c( r ) {}
        };

        template<typename T>
        T get_determinant( const matrix<T>& m ) {
            T a = m.b.y * m.c.z - m.b.z * m.c.y;
            T b = m.b.x * m.c.z - m.b.z * m.c.x;
            T c = m.b.x * m.c.y - m.b.y * m.c.x;

            return m.a.x * a - m.a.y * b + m.a.z * c;
        }

        template<typename T>
        three_d_point<T> solve_cramer( const matrix<T>& A, const three_d_point<T>& D ) {
            
            T A_det = get_determinant( A ); 

            matrix<T> det_x = A;
            det_x.a.x = D.x;
            det_x.b.x = D.y;
            det_x.c.x = D.z;

            T x_num = get_determinant( det_x );
            T x = x_num / A_det;

            matrix<T> det_y = A;
            det_x.a.y = D.x;
            det_x.b.y = D.y;
            det_x.c.y = D.z;

            T y_num = get_determinant( det_y );
            T y = y_num / A_det;

            matrix<T> det_z = A;
            det_x.a.z = D.x;
            det_x.b.z = D.y;
            det_x.c.z = D.z;

            T z_num = get_determinant( det_z );
            T z = z_num / A_det;

            return three_d_point( x, y, z );
        }

        template<typename T>
        T get_distance( const three_d_point<T>& p, const three_d_point<T>& q ) {

            return std::sqrt( ( p.x - q.x ) * ( p.x - q.x ) + 
                              ( p.y - q.y ) * ( p.y - q.y ) +
                              ( p.z - q.z ) * ( p.z - q.z ) );
        }

        /*
            get_circumsphere()
        */
        template<typename T>
        std::tuple<three_d_point<T>,T> get_circumsphere( const three_d_point<T>& p, 
                                                         const three_d_point<T>& q, 
                                                         const three_d_point<T>& r ) {

            const size_t rows = 3;
            const size_t cols = 3;

            auto q_p = get_difference( q, p );              // ( q - p )
            auto r_p = get_difference( r, p );              // ( r - p )
            auto norm = get_cross_product( q_p, r_p );      // ( q - p ) X ( r - p )

            auto [ a_00, a_01, a_02 ] = q_p;    // row 1 of coefficient matrix ( q - p ) 
            auto [ a_10, a_11, a_12 ] = r_p;    // row 2 of coefficient matrix ( r - p )
            auto [ a_20, a_21, a_22 ] = norm;   // row 3 of coefficient matrix ( q - p ) X ( r - p )

            // matrix of coefficients
            std::array<T,rows*cols> elements = {
                a_00, a_01, a_02,
                a_10, a_11, a_12,
                a_20, a_21, a_22,
            };

            matrix_<T,rows,cols> A( elements );             // construct matrix of coefficients

            std::array<T,3> b = {                           
                0.5 * get_dot_product( q_p, q_p ),          
                0.5 * get_dot_product( r_p, r_p ),          
                ( T )0,                                     
            };                                              

            auto LU_decomp = LU_decomposition( A );         // A = LU
            auto [ L, U, ps ] = LU_decomp.value();

            auto [ x, y, z ] = LU_solve( L, U, b, ps );     // solve Ax = b

            three_d_point<T> offset( x, y, z );             // displacement between point p and circumcenter

            three_d_point<T> center = sum( p, offset );     // add the offset vector to p to get the circumcenter  

            auto radius = std::sqrt( get_dot_product( offset, offset ) );   // radius of the circumsphere

            return std::make_tuple( center, radius );
        }

        /*
            pivot()

            re-arranges the rows of matrix m so that the abosolute value of each element on the diagonal of
            the matrix is the largest value in that column

            re-arranging the rows of the matrix in this way helps to improve the stability of LU_solve

            the permutations vector represents the order of the rows in the new permutated matrix. It starts
            as [ 0, 1, 2, ... n ], representing the rows in their original order. Each time rows in the 
            matrix are swapped, the corresponding elements in the permutations vector are aslo swapped. For
            example, if row 0 is swapped with row 2, then the 0th element in the permuations vector is 
            swapped with the second.

            the matrix is modified in place. 
            
            returns the permuation vector
        */
        template<typename T,size_t R,size_t C>
        std::array<size_t,R> pivot( matrix_<T,R,C>& m ) {

            std::array<size_t,R> permutations;
            std::iota( permutations.begin(), permutations.end(), 0 );

            for ( size_t k = 0; k < R; ++k ) {
                T max = std::abs( m[ k, k ] );
                size_t max_index = k;
                for ( size_t i = k; i < R; ++i ) {
                    if ( std::abs( m[ i, k ] ) > max ) {
                        max = std::abs( m[ i, k ] );
                        max_index = i;
                    }
                }
                if ( max_index != k ) {
                    m.swap_rows( max_index, k );
                    std::swap( permutations[ max_index ], permutations[ k ] );
                }
            }
            return permutations;
        }

        /*
            LU_decomposition()

            decompose matrix A into the upper-triangular matrix U and the lower-triangular matrix L using 
            the Doolittle Algorithm:

                A = LU

            use a permutation vector to keep track of any row-swaps that take place during pivoting so that:

                PA = LU

            P is the permutation matrix that corresponds to the permutation vector
        */
        template<typename T,size_t R,size_t C>
        std::optional<
            std::tuple<
                matrix_<T,R,C>,         // lower-triangular matrix 
                matrix_<T,R,C>,         // upper-triangular matrix
                std::array<size_t,R>>>  // permutation vector
        LU_decomposition( matrix_<T,R,C>& A, bool pivot_ = true ) {

            if ( R != C ) {
                return std::nullopt;
            }

            matrix_<T,R,C> L;   // lower-triangular matrix 
            matrix_<T,R,C> U;   // upper-triangular matrix

            const size_t n = R;

            std::array<size_t,n> permutations;  // permutation vector
            if ( pivot_ == false ) {
                // [ 0, 1, 2, ... , n - 1 ]:
                // pivoting is switched-off so we return this dummy-vector
                // as the permutation vector
                std::iota( permutations.begin(), permutations.end(), 0 );
            } else {
                // pivot the matrix A
                permutations = pivot( A );
            }

            
            // construct U and L in an inter-leaved way
            // using Doolittle's algorithm
            for ( size_t i = 0; i < n; ++i ) {

                // construct U
                for ( size_t j = i; j < n; ++j ) {
                    T sum{};
                    for ( size_t k = 0; k < i; ++k ) {
                        sum += L[ i, k] * U[ k, j ];
                    }
                    U[ i, j ] = A[ i, j ] - sum;
                }

                // construct L
                for ( size_t j = i; j < n; ++j ) {
                    if ( i == j ) {
                        L[ i, j ] = ( T )1;
                    } else {
                        T sum{};
                        for ( size_t k = 0; k < i; ++k ) {
                            sum += L[ j, k ] * U[ k, i ]; 
                        }
                        L[ j, i ] = ( A[ j, i ] - sum ) / U[ i, i ];
                    }
                }
            }

            return std::make_tuple( L, U, permutations );
        }

        /*
            LU_solve()
            take an LU decomposition of A and use forward and back-substitution to solve Ax = b
            
            before substitution, take the RHS vector b and re-order it according to the permutation vector 
            derived from the LU decomposition

            first we use forward substitution to solve for y in the equality:

                    Ly = b
            
            then we take the intermediate result y and use back substitution to solve for x in the equality:

                    Ux = y

            x is the solution vector in Ax = b
        */
        template<typename T,size_t N>
        std::array<T,N> LU_solve( 
            const matrix_<T,N,N>& L,        // lower-triangular matrix
            const matrix_<T,N,N>& U,        // upper-triangular matrix 
            const std::array<T,N>& b,       // RHS vector
            const std::array<size_t,N>& ps  // permutation vector
        ) {

            std::array<T,N> x;      // final solution vector
            std::array<T,N> y;      // intermediate solution vector

            std::array<T,N> bp;     // permutated RHS

            for ( size_t k = 0; k < N; ++k ) {  // re-odrer RHS according to the permutation vector
                bp[ k ] = b[ ps[ k ] ];
            } 

            // foward substitution with L:
            // solve for y in Ly = b
            for ( size_t i = 0; i < N; ++i ) {
                T sum{};
                for ( size_t j = 0; j < i; ++j ) {
                    sum += L[ i, j ] * y[ j ];
                }
                y[ i ] = ( ( T )1 / L[ i, i ] ) * ( bp[ i ] - sum ); 
            }

            // backward substitution with U:
            // solve for x in Ux = y
            for ( size_t i = N - 1; i != size_t( -1 ); --i ) {
                T sum{};
                for ( size_t j = i + 1; j < N; ++j ) {
                    sum += U[ i, j ] * x[ j ];
                }
                x[ i ] = ( y[ i ] - sum ) / U[ i, i ];
            }

            return x;
        }

        template<typename I,typename T>
        T triangle_orthoradius_squared( 
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q,
            const std::pair<T,T>& r,
            const T a, const T b, const T c
         ) {

            T A = triangle_area( p, q, r );

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

        template<typename I,typename T>
        T triangle_orthoradius_squared( 
            const triangulation<I,T>& tri, 
            const triangle& t ) {

            const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
            const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
            const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );

            const T a = tri.get_weight( std::get<0>( t ) );
            const T b = tri.get_weight( std::get<1>( t ) );
            const T c = tri.get_weight( std::get<2>( t ) );

            return triangle_orthoradius_squared( p, q, r, a, b, c );
        }

        /*
            get_triangle_centroid()

            the centroid of a triangle is the point where the three medians of the traingle intersect

            a median is a line segment that connects a vertex to the mid-point of the opposite edge

            return the ( x, y ) co-ordinates of the centroid
        */
        template<typename T>
        std::pair<T,T> get_triangle_centroid( 
            const std::pair<T,T>& p, 
            const std::pair<T,T>& q,
            const std::pair<T,T>& r
        ) {

            auto [ qx_prime, qy_prime ] = get_difference( q, p );
            auto [ rx_prime, ry_prime ] = get_difference( r, p );

            auto cx_prime = 2 * get_mid_point( qx_prime, rx_prime ) / 3;
            auto cy_prime = 2 * get_mid_point( qy_prime, ry_prime ) / 3;

            auto [ cx, cy ] = get_sum( { cx_prime, cy_prime }, p );

            return { cx, cy };
        }

        /*
            get_orient()

            takes three points representing a triangle and uses those points to determine if the triangle is
            positively-oriented or not

            a triangle represented by the points ( p, q, r ) is positively-oriented if, when traversing in
            the order p -> q -> r, the interior of the triangle is on the right-hand side

            in the case that the three points are co-linear, the triangle is determined to be degenerate
        
            return an enum value that indicates if the triangle is poistively-oriented, negatively-oriented 
            or degenerate
        */
        template<typename T>
        orient get_orient( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
            
            auto p_r = get_difference( p, r );
            auto q_r = get_difference( q, r );

            auto ext = exterior_product( p_r, q_r );

            const T epsilon = static_cast<T>( 1e-9 );

            if ( std::abs( ext ) < epsilon ) {
                return orient::DEGENERATE;
            }

            return sign( ext ) > 0 ? orient::POSITIVE : orient::NEGATIVE; 
        }

        template<typename T,typename Points>
        std::optional<triangle>
        construct_positively_oriented_triangle(
            const Points& points,
            const size_t i, const size_t j, const size_t k
        ) {

            auto [ p, q, r ] = get_triangle_points( points, i, j, k );

            /*
                if orientation = orient::POSITIVE then the triangle is positively oreinted
            */
            auto orientation = get_orient( p, q, r );

            if ( orientation == orient::DEGENERATE ) {
                return std::nullopt;
            }

            if ( orientation == orient::POSITIVE ) {
                return triangle( i, j, k );
            } else {
                /*
                    switching the first and second vertex switches the orientation of the triangle, in this 
                    case from negative to positive
                */
                return triangle( j, i, k );
            }
        }

        inline std::vector<size_t> get_insertion_order( 
            const size_t num_points,
            const bool randomize
        ) {
            
            std::vector<size_t> indices( num_points );

            std::iota( indices.begin(), indices.end(), 0 );

            if ( !randomize ) return indices;

            std::random_device rand_device;
            std::mt19937 generator( rand_device() );

            std::shuffle( indices.begin(), indices.end(), generator );

            return indices;
        }

        template<typename I,typename T>
        std::vector<size_t> get_insertion_order( 
            const triangulation<I,T>& tri,
            const bool randomize
        ) {    

            return get_insertion_order( tri.num_points, randomize );
        }

        template<typename T>
        relative_position
        get_relaive_position_of_point_to_line(
            const std::pair<T,T>& a,
            const std::pair<T,T>& b,
            const std::pair<T,T>& p
        ) {
            /*
                get the orientation of the triangle formed by a, b, p
            */
            auto orientation = get_orient( a, b, p );

            /* 
                a, b, p form a degnerate triangle, therefore they are colinear, therefore p lies on the 
                segment between a and b
            */
            if ( orientation == orient::DEGENERATE ) 
                return relative_position::ON;
            else if ( orientation == orient::POSITIVE ) {
                /*
                    a, b, p form a positively-oriented triangle, therefore p lies to the left of the segment
                    between a and b
                */
                return relative_position::LEFT;
            } else {
                 /*
                    a, b, p form a negatively-oriented triangle, therefore p lies to the right of the segment
                    between a and b
                */
                return relative_position::RIGHT;
            }
        }

        template<typename T>
        std::vector<float> get_triangle_vertices( const triangle_set& tri_vertices, const std::vector<std::pair<T,T>>& lattice_points ) {

            std::vector<float> vertices;

            for ( auto& triangle: tri_vertices ) {

                auto [ v_1, v_2, v_3 ] = triangle;

                for ( auto v : { v_1, v_2, v_3 } ) {
                    const auto& lattice_point = lattice_points[ v ];
                    vertices.push_back( lattice_point.first );
                    vertices.push_back( lattice_point.second );
                }

            }

            return vertices;
        }

    } // namespace fvm

} // namespace fs 

#endif 