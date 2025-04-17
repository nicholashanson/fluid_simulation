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

            T denom = dist_sqr( std::pair<T,T>{ q_x, q_y }, std::pair<T,T>{ r_x, r_y } );

            // degenerate case: Q = R
            if ( denom == T( 0 ) )
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