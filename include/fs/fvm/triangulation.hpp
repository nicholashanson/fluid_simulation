#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>

#include <set>
#include <tuple>
#include <vector>

namespace fs {

    namespace fvm {

        using triangle = std::tuple<int,int,int>;

        struct compare_triangles {
            bool operator()( const triangle& a, const triangle& b ) const {
                return std::get<0>( a ) < std::get<0>( b );
            }
        };

        using triangle_set = std::set<triangle, compare_triangles>;

        inline size_t sub_2_ind( const size_t x, const size_t y, const size_t xdim ) {

            return std::fma( y - 1, xdim, x );
        }

        inline triangle_set get_lattice_triangles( const size_t ydim, const size_t xdim ) {

            triangle_set triangles = {};

            for ( size_t y = 1; y < ydim; ++y ) {
                for ( size_t x = 1; x < xdim; ++x ) {

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

    } // namespace fvm

} // namespace fs 

#endif 