#ifndef FVM_TRIANGULATION_HPP
#define FVM_TRIANGULATION_HPP

#include <cmath>

#include <set>
#include <tuple>

#include <grid.hpp>

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

    } // namespace fvm

} // namespace fs 

#endif 