#include <geometry/triangle.hpp>

namespace geometry {

    using triangle_set = std::set<triangle>;

    // =======================
    //  Representative Coords
    // =======================

    template<typename I,typename T>
    struct representative_coords {
        T x_;
        T y_;
        I n_;

        representative_coords( T x, T y, T n ) : x_( x ), y_( y ), n_( n ) {}
        
        void add_point( const std::pair<T,T>& p ) {
            x_ = 1 / ( n_ + 1 ) * ( n_ * x_ + p.first );
            y_ = 1 / ( n_ + 1 ) * ( n_ * y_ + p.second );
            n_ += 1;
        }
        void delete_point( const std::pair<T,T>& p ) {
            x_ = 1 / ( n_ - 1 ) * ( n_ * x_ - p.first );
            y_ = 1 / ( n_ - 1 ) * ( n_ * y_ - p.second );
            n_ += 1;
        }
        void reset() {
            x_ = ( T )0;
            y_ = ( T )0;
            n_ = ( I )0; 
        }
        void compute_centroid( const std::vector<std::pair<T,T>>& points ) {
            reset();
            for ( auto& p : points ) {
                add_point( p );
            }
        }
    };

    // ===============
    //  Triangulation
    // ===============

    template<typename I,typename T>
    class triangulation {
        public:
            triangulation( const std::vector<std::pair<T,T>>&& points ) 
                : m_points( std::move( points ) ) {}

            triangulation( const triangle_set&& triangles, const std::vector<std::pair<T,T>>&& points, 
                           const std::vector<I>&& boundary ) 
                : m_triangles( std::move( triangles ) ), 
                  m_points( std::move( points ) ), 
                  m_boundary( std::move( boundary ) ) {}

            const std::map<I,representative_coords<I,T>>& get_representative_points() {
                return m_representative_points;
            }
            T get_weight( const std::size_t i ) const {
                return ( T )0;
            }
            std::pair<T,T> get_point( const std::size_t i ) const {
                return m_points[ i ];
            } 
            const std::vector<std::pair<T,T>>& get_points() const {
                return m_points;
            }
            const triangle_set& get_triangles() const {
                return m_triangles;
            }
            const std::size_t number_of_point() const {
                return m_points.size();
            }
        private:
            triangle_set m_triangles;
            std::map<I,representative_coords<I,T>> m_representative_points;
            std::vector<std::pair<T,T>> m_points;
            std::vector<I> m_boundary;
    };

	// ===========
    //  Sub 2 Ind
    // ===========

    inline size_t sub_2_ind( const std::size_t x, const std::size_t y, const std::size_t xdim ) {
        return y * xdim + x;
    }

	// ====================
    //  Get Lattice Points
    // ====================

    template<typename T>
    std::vector<std::pair<T,T>> get_lattice_points( const T a, const T b, const T c, const T d, 
                                                    const std::size_t xdim, const std::size_t ydim ) {
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

   	// ======================
    //  Get Lattice Boundary
    // ======================

    inline std::vector<int> get_lattice_boundary( const std::size_t xdim, const std::size_t ydim ) {
        std::vector<int> boundary( 2 * xdim + 2 * ydim - 4 /* corners are counterd twice */ );
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

    // =======================
    //  Get Lattice Triangles
    // =======================

    inline triangle_set get_lattice_triangles( const std::size_t ydim, const std::size_t xdim ) {
        triangle_set triangles = {};
        for ( size_t y = 0; y < ydim - 1; ++y ) {
            for ( size_t x = 0; x < xdim - 1; ++x ) {
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

    // =======================
    //  Triangulate Rectangle
    // =======================

    template<typename I,typename T>
    triangulation<I,T> triangulate_rectangle( const T a, const T b, const T c, const T d,
                                              const std::size_t xdim, const std::size_t ydim,  
                                              bool single_boundary = false ) {
        auto lattice_triangles = get_lattice_triangles( ydim, xdim );
        auto points = get_lattice_points( a, b, c, d, xdim, ydim );
        auto boundary = get_lattice_boundary( xdim, ydim ); 
        auto tri = triangulation( std::move( lattice_triangles ), std::move( points ), std::move( boundary ) );
        return tri;
    }

    // ===========================
    //  Get Triangle Ortho Center
    // ===========================

    template<typename I,typename T>
    std::pair<T,T> get_triangle_circumcenter( const triangulation<I,T>& tri, const triangle& t ) {
        const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
        const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
        const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );
        return get_triangle_circumcenter( p, q, r );
    }

    // =======================
    //  Get Triangle Vertices
    // =======================

    template<typename T>
    std::vector<float> get_triangle_vertices( const triangle_set& tri_vertices, const std::vector<std::pair<T,T>>& lattice_points ) {
        std::vector<float> vertices;
        for ( auto& triangle: tri_vertices ) {
            auto [ v_1, v_2, v_3 ] = triangle;
            for ( auto v : { v_1, v_2, v_3 } ) {
                const auto& lattice_point = lattice_points[ v ];
                vertices.push_back( lattice_point.first );
                vertices.push_back( lattice_point.second );
            }

        }
        return vertices;
    }

    // ==================================
    //  Get Triangle Orthoradius Squared
    // ==================================
    
    template<typename I,typename T>
    T get_triangle_orthoradius_squared( const triangulation<I,T>& tri, const triangle& t ) {
        const std::pair<T,T> p = tri.get_point( std::get<0>( t ) );
        const std::pair<T,T> q = tri.get_point( std::get<1>( t ) );
        const std::pair<T,T> r = tri.get_point( std::get<2>( t ) );
        const T a = tri.get_weight( std::get<0>( t ) );
        const T b = tri.get_weight( std::get<1>( t ) );
        const T c = tri.get_weight( std::get<2>( t ) );
        return get_triangle_orthoradius_squared( p, q, r, a, b, c );
    }

} // namespace geometry