#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>

#include <set>
#include <map>
#include <tuple>
#include <vector>

namespace fs {

    namespace fvm {

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
                rerpesentative_point.y = ( T )0;
                representative_point.n = ( I )0; 
            } 
        }

        template<typename I,typename T>
        class triangulation {
        public:

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

            double delta_X = ( b - a ) / ( static_cast<double>( xdim - 1 ) );
            double delta_y = ( d - c ) / ( static_cast<double>( ydim - 1 ) );
        
            for ( size_t y = 1; y < ydim; ++y ) {

                double y_ = c + static_cast<double>( y - 1 ) * delta_y;

                for ( size_t x = 1; x < xdim; ++x ) {

                    double x_ = a + static_cast<double>( x - 1 ) * delta_X; 

                    const size_t index = sub_2_ind( x - 1, y, xdim );

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