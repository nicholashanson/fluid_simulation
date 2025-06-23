#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include <fs/fvm/triangulation.hpp>

namespace geometry {

    enum class in_circle {
        INSIDE,
        ON,
        OUTSIDE
    };

    /*
        point_in_circle()

        determines if the the point a is inside, outside or on the circle formed by the points p, q, r
    */
    template<typename T>
    in_circle point_in_circle(
        const std::pair<T,T>& p,
        const std::pair<T,T>& q,
        const std::pair<T,T>& r, 
        const std::pair<T,T>& a 
    ) {

        auto q_p = fs::fvm::get_difference( q, p );
        auto r_p = fs::fvm::get_difference( r, p );
        auto a_p = fs::fvm::get_difference( a, p );
        auto a_q = fs::fvm::get_difference( a, q );
        auto r_q = fs::fvm::get_difference( r, q );

        auto result = fs::fvm::exterior_product( q_p, a_p ) * fs::fvm::inner_product( r_p, r_q ) -
                      fs::fvm::exterior_product( q_p, r_p ) * fs::fvm::inner_product( a_p, a_q );

        if ( result == 0 )
            return in_circle::ON;

        return result > 0 ? in_circle::INSIDE : in_circle::OUTSIDE;
    }

} // namespace geometry

#endif