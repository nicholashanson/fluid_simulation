#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "geometry.hpp"

namespace geometry {

    // ======================
    //  Is A Between B And C
    // ======================

    template<typename T>    
    bool is_a_between_b_and_c( const T a, const T b, const T c ) {
        return ( c > a ) != ( b > a ); 
    }

    // =================
    //  Get X Intersect
    // =================

    template<typename T>
    T get_x_intersect( const std::pair<T,T>& a, const std::pair<T,T>& b, const std::pair<T,T>& point ) {
        return a.first + ( point.second - a.second) * ( b.first - a.first ) / ( b.second - a.second );
    }

    // =====================
    //  Get Boundary Points
    // =====================

    template<typename I,typename T>
    std::vector<std::pair<T,T>> get_boundary_points( const std::vector<std::pair<T,T>>& points, const std::vector<I> boundary ) {
        std::vector<std::pair<T,T>> boundary_points;
        for ( auto& point_index : boundary ) {
            boundary_points.push_back( points[ point_index ] );
        }
        return boundary_points;
    }

    // ==============
    //  Bounding Box
    // ==============

    template<typename T>
    struct bounding_box {
        T xmin;
        T xmax;
        T ymin;
        T ymax;

        bounding_box() = default;

        bounding_box( const T& xmin, const T& xmax, const T& ymin, const T& ymax ) 
            : xmin( xmin ), xmax( xmax ), ymin( ymin ), ymax( ymax ) {}

        bool operator==( const bounding_box& other ) const {
            return xmin == other.xmin &&
                   xmax == other.xmax &&
                   ymin == other.ymin &&
                   ymax == other.ymax;
        }
    };

    template<typename T>
    concept HasInfinity = requires {
        { std::numeric_limits<T>::infinity() } -> std::same_as<T>;
    };

    // ==================
    //  Get Bounding Box
    // ==================

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

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename I,typename T>
    bounding_box<T> get_bounding_box( const std::vector<std::pair<T,T>>& points, const std::vector<I> boundary ) {
        auto boundary_points = get_boundary_points( points, boundary );
        return get_bounding_box( boundary_points );
    } 

    // =================================
    //  Polygon Features Single Segment
    // =================================

    template<typename I,typename T>
    std::pair<T,std::pair<T,T>> get_polygon_featues_single_segment( const std::vector<std::pair<T,T>>& points, 
                                                                    const std::vector<I>& boundary ) {
        std::pair<T,T> centroid{};
        T area{};
        auto a = points[ boundary.front() ];
        for ( std::size_t i = 1; i < boundary.size(); ++i ) {
            auto b = points[ boundary[ i % boundary.size() ] ];
            T area_contribution = get_cross_product( a, b );
            centroid = get_sum( centroid, scale( area_contribution, get_sum( a, b ) ) );
            area += area_contribution;
            a = b;
        }
        T signed_area = area / 2.0;
        T factor = ( 3.0 * area != 0 ) ? ( 1.0 / ( 3.0 * area ) ) : 0;
        return { signed_area, scale( factor, centroid ) };
    }

    // ==============
    //  Polygon Tree
    // ==============

    template<typename I>
    struct polygon_tree {
        std::optional<polygon_tree*> parent;
        std::set<polygon_tree*> children;
        I index;
        int height;
    };

    // ===================
    //  Polygon Heirarchy
    // ===================

    template<typename I,typename T>
    struct polygon_heirarchy {
        std::vector<bool> polygon_orientations;
        std::vector<bounding_box<T>> bounding_boxes;
        std::map<I,polygon_tree<I>*> trees;
        std::vector<polygon_tree<I>*> reoder_cache;

        polygon_tree<I>* get_polygon_tree( const I id ) const {
            auto it = trees.find( id );
            if ( it != trees.end() ) {
                return it->second;
            }
            return nullptr;
        }

    };

    // ===========================
    //  Polygon Hierarchy Context
    // ===========================

    template<typename I,typename T>
    struct polygon_heirarchy_context {
        polygon_heirarchy<I,T>& hierarchy;
        const std::vector<std::pair<T,T>>& points;
        const std::vector<std::vector<I>>& boundaries;
    };

    // =====================
    //  Point Is In Polygon
    // =====================

    template<typename I,typename T>
    bool point_is_in_polygon( const std::vector<std::pair<T,T>>& points, const std::vector<I>& boundary, const std::pair<T,T>& point ) {
        bool inside = false;
        for ( std::size_t i = 0; i < boundary.size(); ++i ) {
            auto& a = points[ boundary[ i ] ];
            auto& b = points[ boundary[ ( i + 1 ) % boundary.size() ] ];
            if ( a.second == b.second ) {
                continue;
            }
            if ( is_a_between_b_and_c( point.second, a.second, b.second ) ) {
                T x_intersect = get_x_intersect( a, b, point );
                if ( point.first < x_intersect ) {
                    inside = !inside;
                }
            }
        }
        return inside;
    }

    // ==========================
    //  Point Is In Polygon Tree
    // ==========================

    template<typename I,typename T>
    bool point_is_in_polygon_tree( const polygon_heirarchy_context<I,T>& context, const polygon_tree<I>* tree,
                                   const std::pair<T,T>& representative_point ) {
        I index = tree->index;
        if ( index < context.hierarchy.bounding_boxes.size() ) {
            auto& bb = context.hierarchy.bounding_boxes[ index ];
            if ( representative_point.first < bb.xmin  || representative_point.first > bb.xmax   || 
                 representative_point.second < bb.ymin || representative_point.second > bb.ymax ) {
                return false;
            }
        }
        const std::vector<I>& section = context.boundaries[ index ];
        if ( point_is_in_polygon( context.points, section, representative_point ) ) {
            return true;
        }
        return false;
    }

    // =========================
    //  Find Deepest Containing
    // =========================

    template<typename I,typename T>
    polygon_tree<I>* find_deepest_containing_polygon_tree( const polygon_heirarchy_context<I,T>& context, polygon_tree<I>* tree,
                                                           const std::pair<T,T>& representative_point ) {
        polygon_tree<I>* deepest = nullptr;
        for ( polygon_tree<I>* child : tree->children ) {
            if ( point_is_in_polygon_tree( context, child, representative_point ) ) {
                auto result = find_deepest_containing_polygon_tree( context, child, representative_point );
                if ( result ) {
                    deepest = result;
                }
            }
        }
        if ( point_is_in_polygon_tree( context, tree, representative_point ) && !deepest ) {
            deepest = tree;
        }
        return deepest;
    }

    // ===========
    //  Find Tree
    // ===========

    template<typename I,typename T>
    polygon_tree<I>* find_tree( const polygon_heirarchy_context<I,T>& context, const std::pair<T,T>& representative_point ) {
        for ( auto& key_value : context.hierarchy.trees ) {
            polygon_tree<I>* t = key_value.second;
            if ( !t->parent && point_is_in_polygon_tree( context, t, representative_point ) ) {
                return find_deepest_containing_polygon_tree( context, t, representative_point );
            }
        }
        return nullptr;        
    }

    // ================
    //  Increase Depth
    // ================

    template<typename I>
    void increase_polygon_tree_depth( polygon_tree<I>* tree, int increase_by = 1 ) {
        tree->height += increase_by;
        for ( polygon_tree<I>* child : tree->children ) {
            increase_polygon_tree_depth( child, increase_by );
        }
    }

    // ===================
    //  Reorder Hierarchy
    // ===================

    template<typename I,typename T>
    void reorder_polygon_hierarchy( polygon_heirarchy_context<I,T>& context, polygon_tree<I>* new_tree ) {
        context.hierarchy.trees[ new_tree->index ] = new_tree;
        context.hierarchy.reoder_cache.clear();
        for ( auto& key_value : context.hierarchy.trees ) {
            polygon_tree<I>* t = key_value.second;
            if ( t == new_tree ) {
                continue;
            }
            if ( t->parent ) {
                const std::vector<I>& section = context.boundaries[ t->index ];
                std::pair<T,T> representative_point = context.points[ section.front() ];
                if ( point_is_in_polygon_tree( context, new_tree, representative_point ) ) {
                    t->parent = new_tree;
                    new_tree->children.insert( t );
                    increase_polygon_tree_depth( t, 1 );
                    context.hierarchy.reoder_cache.push_back( t );
                }
            }
        }
    }

    // ==========================================
    //  Construct Polygon Hierarchy Single Curve
    // ==========================================

    template<typename I,typename T>
    polygon_heirarchy<I,T> construct_polygon_hierarchy_single_curve( const std::vector<std::pair<T,T>>& points, 
                                                                     const std::vector<I>& boundary ) {
        polygon_heirarchy<I,T> hierarchy;
        bounding_box<T> bb;
        if ( points.size() == boundary.size() ) {
            bb = get_bounding_box( points );
        }
        else {
            auto boundary_points = get_boundary_points( points, boundary );
            bb = get_bounding_box( boundary_points );
        }
        hierarchy.bounding_boxes.push_back( bb );
        //T area = get_polygon_area(points, boundary_nodes); 
        //hierarchy.polygon_orientations.push_back( area > 0 );
        auto* root = new polygon_tree<I>{ std::nullopt, {}, 0, 0 };
        hierarchy.trees[ root->index ] = root;
        return hierarchy;
    }

    // =================
    //  Reorder Subtree
    // =================

    template<typename I,typename T>
    void reorder_subtree( polygon_heirarchy_context<I,T>& context, polygon_tree<I>* parent, polygon_tree<I>* tree ) {
        context.hierarchy.trees[ tree->index ] = tree;
        parent->children.insert( tree );
        return;
    }

    // =============================================
    //  Construct Polygon Hierarchy Multiple Curves
    // =============================================

    template<typename I,typename T>
    polygon_heirarchy<I,T> construct_polygon_hierarchy_multiple_curves( const std::vector<std::pair<T,T>>& points,
                                                                        const std::vector<std::vector<I>>& boundaries ) {
        auto& curve_nodes = boundaries.front();
        polygon_heirarchy<I,T> hierarchy = construct_polygon_hierarchy_single_curve( points, curve_nodes );
        hierarchy.bounding_boxes.resize( boundaries.size() );
        for ( std::size_t curve_index = 1; curve_index < boundaries.size(); ++curve_index ) {
            auto curve_nodes = boundaries[ curve_index ];
            //T area = get_polygon_area( points, curve_nodes );
            //bool is_positvie = ( area > 0 );
            //auto box = get_polygon_bounds( points, curve_nodes );
            //hierarchy.orientations[ curve_index ] = is_positvie;
            hierarchy.bounding_boxes[ curve_index ] = get_bounding_box( points, curve_nodes );

            I representative_point_index = curve_nodes.front();
            std::pair<T,T> representative_point = points[ representative_point_index ];
            polygon_heirarchy_context<I,T> context( hierarchy, points, boundaries );
            auto* tree = find_tree( context, representative_point );
            polygon_tree<I>* new_tree;

            if ( tree ) {
                new_tree = new polygon_tree<I>( tree, {}, curve_index, tree->height + 1 );
                reorder_subtree( context, tree, new_tree );
            } else {
                new_tree = new polygon_tree<I>( nullptr, {}, curve_index, 0 );
                reorder_polygon_hierarchy( context, new_tree );
            }
        }
        return hierarchy;
    }

    // =============================================
    //  Get Distance To Polygon With Single Segment
    // =============================================
    // does a horizontal line that extends from the point p intersect a line segment extending between
    // the points q and r?
    // If so, at what x co-ordinate does that horizontal line intersect with that line segment?
    template<typename I,typename T>
    T get_distance_to_polygon_with_single_segment( const std::pair<T,T>& p, const std::vector<std::pair<T,T>>& points,
                                                   const std::vector<I>& boundary, bool is_in_outer = false, bool return_sqrt = true ) {
        T dist = std::numeric_limits<T>::max();
        auto a = points[ boundary.front() ];
        for ( std::size_t i = 1; i < boundary.size(); ++i ) {
            auto b = points[ boundary[ i ] ];
            if ( is_a_between_b_and_c( p.second, a.second, b.second ) ) {
                T x_intersect = get_x_intersect( a, b, p );
                if ( p.first < x_intersect ) {
                    is_in_outer = !is_in_outer;
                }
            }
            T new_dist = get_squared_distance_to_segment( a, b, p );
            dist = new_dist < dist ? new_dist : dist;
            a = b;
        }
        dist = return_sqrt ? std::sqrt( dist ) : dist;
        return is_in_outer ? dist : -dist;
    }

    // ==============================================
    //  Get Distance To Polygon With Single Segments
    // ==============================================

    template<typename I,typename T>
    T get_distance_to_polygon_with_multiple_segments( const std::pair<T,T>& p, const std::vector<std::pair<T,T>>& points,
                                                      const std::vector<std::vector<I>>& boundaries, bool is_in_outer = false, 
                                                      const bool return_sqrt = true ) {
        T dist = std::numeric_limits<T>::max();
        for ( auto& boundary : boundaries ) {
            T new_dist = get_distance_to_polygon_with_single_segment( p, points, boundary, is_in_outer == true, false );
            is_in_outer = adaptive_arithmetic::sign( new_dist ) == 1 ? true : false;
            new_dist = std::abs( new_dist );
            dist = new_dist < dist ? new_dist : dist;
        }
        dist = return_sqrt ? std::sqrt( dist ) : dist;
        return is_in_outer ? dist : -dist;
    }

} // namespace geometry

#endif