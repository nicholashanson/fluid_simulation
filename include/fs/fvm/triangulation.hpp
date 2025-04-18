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

        /*
        template<typename T> 
        std::tuple<T,T> two_product( const T a, const T b ) {

            T x = a * b
            T y = two_product_tail( a, b, x )
            
            return std::make_tuple( x, y );
        }
        */

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
        T distance_to_polygon_single_segment( std::pair<T,T> p, 
                                              Points points,
                                              BoundaryNodes boundary_nodes,
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

    } // namespace fvm

} // namespace fs 

#endif 