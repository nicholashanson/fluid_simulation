#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <fs/fvm/triangulation.hpp>

namespace geometry {
  
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

        auto [ qx_prime, qy_prime ] = fs::fvm::get_difference( q, p );
        auto [ rx_prime, ry_prime ] = fs::fvm::get_difference( r, p );

        auto cx_prime = 2 * fs::fvm::get_mid_point( qx_prime, rx_prime ) / 3;
        auto cy_prime = 2 * fs::fvm::get_mid_point( qy_prime, ry_prime ) / 3;

        auto [ cx, cy ] = fs::fvm::get_sum( { cx_prime, cy_prime }, p );

        return { cx, cy };
    }

} // namespace geometry

#endif
