#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include <geometry/geometry.hpp>

namespace geometry {

    // ===========
    //  In Circle
    // ===========

    enum class in_circle : uint8_t {
        inside,
        on,
        outside
    };

    // ====================
    //  Is Point In Circle
    // ====================
    // is point a inside, outside or on the circle formed by the points p, q, r
    template<typename T>
    in_circle is_point_in_circle( const std::pair<T,T>& p,
                                  const std::pair<T,T>& q,
                                  const std::pair<T,T>& r, 
                                  const std::pair<T,T>& a ) {
        auto q_p = get_difference( q, p );
        auto r_p = get_difference( r, p );
        auto a_p = get_difference( a, p );
        auto a_q = get_difference( a, q );
        auto r_q = get_difference( r, q );
        auto result = get_exterior_product( q_p, a_p ) * get_inner_product( r_p, r_q ) -
                      get_exterior_product( q_p, r_p ) * get_inner_product( a_p, a_q );
        if ( result == 0 ) {
            return in_circle::on;
        }
        return result > 0 ? in_circle::inside : in_circle::outside;
    }

} // namespace geometry

#endif