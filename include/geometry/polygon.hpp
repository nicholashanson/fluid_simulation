#ifndef POLYGON_HPP
#define POLYGON_HPP

#include <fs/fvm/triangulation.hpp>

namespace geometry {

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

        const size_t n_edge = fs::fvm::num_boundary_edges( boundary_nodes );

        // get the global index of the first node in the boundary
        auto q_index = fs::fvm::get_boundary_nodes( boundary_nodes, 0 );

        // get the co-ord ( x, y ) at that global index
        auto q = fs::fvm::get_point( points, q_index );

        // the starting point of the line segment
        T q_x = q.first;
        T q_y = q.second;

        for ( size_t i = 1; i <= n_edge; ++i ) {

            auto r_index = fs::fvm::get_boundary_nodes( boundary_nodes, i );
            auto r = fs::fvm::get_point( points, r_index );

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

            T new_dist = fs::fvm::squared_distance_to_segment( q_x, q_y, r_x, r_y, p_x, p_y );
            dist = new_dist < dist ? new_dist : dist;

            q_x = r_x;
            q_y = r_y;
        }

        dist = return_sqrt ? std::sqrt( dist ) : dist;
        return is_in_outer ? dist : -dist;
    }

    template<typename T,typename Points,typename BoundaryNodes>
    T distance_to_polygon_multiple_segments( const std::pair<T,T>& p, 
                                                const Points& points,
                                                const BoundaryNodes& boundary_nodes,
                                                bool is_in_outer = false, 
                                                const bool return_sqrt = true ) {

        using InnerBoundaryNodess = std::decay_t<decltype(boundary_nodes[0])>;

        T dist = std::numeric_limits<T>::max();
        
        const size_t ns = fs::fvm::num_sections( boundary_nodes );

        for ( size_t i = 0; i < ns; ++i ) {

            InnerBoundaryNodess bn = fs::fvm::get_boundary_nodes( boundary_nodes, i );

            T new_dist = distance_to_polygon_single_segment( p, points, bn, is_in_outer == true, false );

            is_in_outer = fs::fvm::sign( new_dist ) == 1 ? true : false;
            new_dist = std::abs( new_dist );

            dist = new_dist < dist ? new_dist : dist;
        }

        dist = return_sqrt ? std::sqrt( dist ) : dist;
        return is_in_outer ? dist : -dist;
    }

} // namespace geometry

#endif