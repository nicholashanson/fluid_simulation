#ifndef GRID_HPP
#define GRID_HPP

#include <variant>
#include <iostream>
#include <cmath>

namespace fs {

    template<typename Array, typename MDSpan>
    class grid {
        public:

            using value_type_ = typename MDSpan::element_type;

            grid();

            // constructor for owning grid
            grid( Array );
            // constructor for non-owning grid
            grid( typename Array::pointer );

            grid( const grid& other );

            grid( grid&& other ) noexcept;

            grid& operator=( grid&& other ) noexcept;

#ifndef NDEBUG
            // equality operator only used for testing
            bool operator==( const grid& ) const;
#endif

            size_t get_grid_width() const {
                return grid_.static_extent( 1 );
            }
            size_t get_grid_height() const {
                return grid_.static_extent( 0 );
            }

            template<typename... Indices>
            decltype( auto ) get_cell_state( Indices... ) const;

            template<typename... Indices>
            void set_cell_state( const value_type_, Indices... );

            typename Array::pointer get_data_handle();

            const typename Array::pointer get_data_handle() const;

#ifndef NDEBUG
            void print() {
                for ( size_t i = 0; i < get_grid_height(); ++i ) {
                    for ( size_t j = 0; j < get_grid_width(); ++j ) {
                        for ( size_t q = 0; q < 9; ++q )
                            std::cout << grid_[ i, j, q ] << ", " ;
                        std::cout << std::endl;
                    }
                }
            }
#endif

        private:
            // Array is either owning or non-owning
            std::variant<Array, typename Array::pointer> cell_states_;
            MDSpan grid_;
    };

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>::grid()
        : cell_states_( nullptr ), grid_( nullptr ) {
    }

    // constructor for an owning grid
    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>::grid( Array cell_states ) :
        cell_states_( std::move( cell_states ) ), grid_( std::get<Array>( cell_states_ ).data() ) {
    }

    // constructor for a non-owning grid
    // cell_states = Array.data()
    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>::grid( typename Array::pointer cell_states ) :
        cell_states_( cell_states ), grid_( std::get<typename Array::pointer>( cell_states_ ) ) {
    }

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>::grid( const grid& other )
        : cell_states_( other.cell_states_ ), grid_( std::get<Array>( cell_states_ ).data() ) {
    }

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>::grid( grid&& other ) noexcept
        : cell_states_( std::move( other.cell_states_)), grid_( std::move( other.grid_ ) ) {
    }

    template<typename Array, typename MDSpan>
    grid<Array, MDSpan>& grid<Array, MDSpan>::operator=( grid&& other ) noexcept {
        if ( this != &other ) {
            cell_states_ = std::move( other.cell_states_ );

            if ( std::holds_alternative<Array>( cell_states_ ) )
                grid_ = MDSpan( std::get<Array>( cell_states_ ).data() );
            else
                grid_ = MDSpan( std::get<typename Array::pointer>( cell_states_ ) );
        }
        return *this;
    }

#ifndef NDEBUG
    // test for equality between two grids
    template<typename Array, typename MDSpan>
    bool grid<Array, MDSpan>::operator==( const grid& rhs ) const {
        for ( size_t i = 0; i < grid_.static_extent( 0 ); ++i )
            for ( size_t j = 0; j < grid_.static_extent( 1 ); ++j )
                for ( size_t q = 0; q < grid_.static_extent( 2 ); ++q )

                    if constexpr ( std::is_floating_point_v<value_type_> ) {
                        if ( std::fabs( grid_[ i, j, q ] - rhs.grid_[ i, j, q ] ) > 1e-6 ) {
                            return false;
                        }
                    } else {
                        if ( grid_[ i, j, q ] != rhs.grid_[ i, j, q ] ) {
                            return false;
                        }
                    }
        return true;
    }
#endif

    template<typename Array, typename MDSpan>
    template<typename... Indices>
    decltype( auto ) grid<Array, MDSpan>::get_cell_state( Indices... indices ) const {
        return grid_[ indices... ];
    }

    template<typename Array, typename MDSpan>
    template<typename... Indices>
    void grid<Array, MDSpan>::set_cell_state( const value_type_ state, Indices... indices ) {
        grid_[ indices... ] = state;
    }

    template<typename Array, typename MDSpan>
    typename Array::pointer grid<Array, MDSpan>::get_data_handle() {
        if ( std::holds_alternative<Array>( cell_states_ ) )
            return std::get<Array>( cell_states_ ).data();
        else
            return std::get<typename Array::pointer>( cell_states_ );
    }

    template<typename Array, typename MDSpan>
    const typename Array::pointer grid<Array, MDSpan>::get_data_handle() const {
        if ( std::holds_alternative<Array>( cell_states_ ) )
            return std::get<Array>( cell_states_ ).data();
        else
            return std::get<typename Array::pointer>( cell_states_ );
    }

}

#endif
