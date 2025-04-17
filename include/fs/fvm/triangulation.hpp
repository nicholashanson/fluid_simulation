#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>
#include <limits>
#include <iterator>

#include <set>
#include <map>
#include <tuple>
#include <vector>

#include <type_traits>

namespace fs {

    namespace fvm {

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

            std::pair<T,T> vec = { q.first - p.frst, q.second - p.second };
            return vec.first * vec.first + vec.second * vec.second;
        }

        template<typename T>
        T squared_distance_to_segment( const T x_1, const T y_1,
                                       const T x_2, const T y_2, 
                                       const T x, const T y ) {
            T q_p_1_x = x - x_1;
            T q_p_1_y = y - y_1;
            T p_1_p_2_x = x_2 - x_1;
            T p_1_p_2_y = y_2 - y_1;

            T denom = dist_sqr( { x_1, y_1 }, { x_2, y_2 } );

            T t = ( q_p_1_x * p_1_p_2_x + q_p_1_y * p_1_p_2_y ) / denom;
            T ts = std::min( std::max( t, ( T )0), ( T )1 );

            T intersect_x = x_1 + ts * p_1_p_2_x;
            T intersect_y = y_1 + ts * p_1_p_2_y;

            T delta_2 = dist_sqr( { x, y }, { intersect_x, intersect_y } );

            return delta_2;
        } 

        template<typename T,typename Points,typename BoundaryNodes>
        T distance_to_polygon_single_segment( std::pair<T,T> q, 
                                              Points points,
                                              BoundaryNodes boundary_nodes,
                                              bool is_in_outer = false, 
                                              bool return_sqrt = true ) {
            T x = q.first;
            T y = q.second;

            using F = number_type<Points>::type;

            F dist = std::numeric_limits<F>::max;

            const size_t n_edge = num_boundary_edges( boundary_nodes );

            auto v_i = get_boundary_nodes( boundary_nodes, 1 );
            auto p_i = get_point( points, v_i );

            T x_i = p_i.first;
            T y_i = p_i.second;

            for ( size_t j = 2; j < n_edge; ++j ) {

                auto v_i_1 = get_boundary_nodes( boundary_nodes, j );
                auto p_i_1 = get_point( points, v_i_1 );

                T x_i_1 = p_i_1.first; 
                T y_i_1 = p_i_1.second;

                if ( ( y_i_1 > y ) != ( y_i > y ) ) {

                    T x_intersect = ( x_i - x_i_1 ) * ( y - y_i_1 ) / ( y_i - y_i_1 ) + x_i_1;
                    if ( x < x_intersect ) {
                        is_in_outer = !is_in_outer;
                    }
                }

                F new_dist = squared_distance_to_segment( x_i, y_i, x_i_1, y_i_1, x, y );
                dist = new_dist < dist ? new_dist : dist;

                v_i = v_i_1;
                p_i = p_i_1;
                x_i = x_i_1;
                y_i = y_i_1;
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
        bool has_multiple_sections( const T& ) {
            return false;
        }

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

            T xmin = std::numeric_limits<T>::max();
            T xmax = std::numeric_limits<T>::min();
            T ymin = std::numeric_limits<T>::max();
            T ymax = std::numeric_limits<T>::min();
        
        }

        template<typename T>
        void polygon_bounds( bool check_all_curves = false ) {
            polygon_bounds_multiple_segments<T>();
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

            // Rule-of-Zero
            BPL<I,T> get_representative_points() {
                return representative_points;
            }
        private:
            BPL<I,T> representative_points;
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

        inline std::vector<std::pair<double,double>> get_lattice_points( const double a, const double b, 
                                                                         const double c, const double d, 
                                                                         const size_t xdim, const size_t ydim ) {
            
            std::vector<std::pair<double,double>> points( ydim * xdim, { 0.0, 0.0 } );

            double delta_x = ( b - a ) / ( static_cast<double>( xdim - 1 ) );
            double delta_y = ( d - c ) / ( static_cast<double>( ydim - 1 ) );
        
            for ( size_t y = 0; y < ydim; ++y ) {

                double y_ = c + static_cast<double>( y ) * delta_y;

                for ( size_t x = 0; x < xdim; ++x ) {

                    double x_ = a + static_cast<double>( x ) * delta_x; 

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
                boundary[ ( xdim - 1 ) + x ] = sub_2_ind( x, ydim - 1, xdim );
            }

            for ( size_t y = 1; y < ydim - 1; ++y ) {

                boundary[ 2 * ( xdim - 1 ) + y - 1 ] = sub_2_ind( 0, y, xdim );
                boundary[ 2 * ( xdim - 1 ) + ( ydim - 2 ) + y ] = sub_2_ind( xdim, y, xdim );
            }

            return boundary;
        }

    } // namespace fvm

} // namespace fs 

#endif 