#include <cmath>

#ifndef M_PI
#define M_PI ( 4 * std::atan( 1 ) )
#endif

namespace adaptive_arithmetic {

	// ======
    //  Sign
    // ======

	template<typename T>
    int sign( T val ) {
        return ( T( 0 ) < val ) - ( val < T( 0 ) );
    }

    template <typename... Ts, std::size_t... I>
    auto tuple_to_array_impl( const std::tuple<Ts...>& t, std::index_sequence<I...> ) {
        return std::array{ std::get<I>( t )... }; 
    }

    // ================
    //  Tuple To Array
    // ================

    template <typename... Ts>
    auto tuple_to_array( const std::tuple<Ts...>& t ) {
        return tuple_to_array_impl( t, std::index_sequence_for<Ts...>{} );
    }

	// ===========
    //  Err Bound
    // ===========

    template<typename T>
    struct err_bound {
        T A;
        T B;
        T C;

        constexpr T calc( const T& a, const T& b, const T& epsilon ) {
            return ( a + b * epsilon ) * epsilon;
        }

        constexpr err_bound( const T& aa, const T& ab,
                             const T& ba, const T& bb,
                             const T& ca, const T& cb,
                             const T& epsilon )
            : A( calc( aa, ab, epsilon ) ), 
              B( calc( ba, bb, epsilon ) ),
              C( calc( ca, cb, epsilon ) ) {}
    };

    // ================
    //  Init Constants
    // ================

    template<typename T>
    struct init_constants {
        T epsilon;
        T splitter;
        T result_err_bound;
        err_bound<T> ccw;
        err_bound<T> o3d;
        err_bound<T> icc;
        err_bound<T> isperr;

        constexpr init_constants( const T epsilon ) 
            : epsilon( epsilon ),
              splitter( T( 1 ) + epsilon ),
              result_err_bound( ( ( T )3 + T( 8 ) * epsilon ) * epsilon ),
              ccw( 3, 16, 2, 12, 2, 12, epsilon ),
              o3d( 7, 56, 3, 28, 26, 288, epsilon ),
              icc( 10, 96, 4, 48, 44, 576, epsilon ),
              isperr( 16, 224, 5, 72, 71, 1408, epsilon ) {}
    };

    // ==================
    //  Double Constants
    // ==================

    constexpr init_constants<double> double_constants( std::numeric_limits<double>::epsilon() );

    // ===========
    //  Set Index
    // ===========

    template<size_t N,typename T>
    void set_index( std::array<T,N>& e, const T value, const std::size_t index ) {
        if ( index < e.size() ) {
            e[ index ] = value;
        }
    }

    // ================
    //  Safe Get Index
    // ================

    template<size_t N,typename T>
    T safe_get_index( const std::array<T,N> e, const std::size_t e_index ) {
        if ( e_index < N && e_index < e.size() ) {
            return e[ e_index ];
        } else {
            return ( T )0;
        }
    }

    // ===================
    //  Fast Two Sum Tail
    // ===================

    template<typename T>
    T fast_two_sum_tail( const T a, const T b, const T x ) {
        const T b_virt = x - a;
        const T y = b - b_virt;
        return y;
    }

    // ==============
    //  Fast Two Sum
    // ==============

    template<typename T>
    std::tuple<T,T> fast_two_sum( const T a, const T b ) {
        const T x = a + b;
        const T y = fast_two_sum_tail( a, b, x );
        return std::make_tuple( x, y );
    }

    // ==============
    //  Two Sum Tail
    // ==============

    template<typename T>
    T two_sum_tail( const T a, const T b, const T x ) {
        const T b_virt = x - a;
        const T a_virt = x - b_virt;
        const T b_round = b - b_virt;
        const T a_round = a - a_virt;
        const T y = a_round + b_round;
        return y;
    }

    // =========
    //  Two Sum
    // =========

    template<typename T>
    std::pair<T,T> two_sum( const T a, const T b ) {
        const T x = a + b;
        const T y = two_sum_tail( a, b, x );
        return { x, y };
    }

    // ===============
    //  Two Diff Tail
    // ===============

    template<typename T>
    T two_diff_tail( const T a, const T b, const T x ) {
        const T b_virt = a - x;
        const T a_virt = x + b_virt;
        const T b_round = b_virt - b;
        const T a_round = a - a_virt;
        const T y = a_round + b_round;
        return y;
    }

    // ==========
    //  Two Diff
    // ==========

    template<typename T>
    std::tuple<T,T> two_diff( const T a, const T b ) {
        const T x = a - b;
        const T y = two_diff_tail( a, b, x );
        return std::make_tuple( x, y );  
    }

    // ==============
    //  Two One Diff
    // ==============

    template<typename T>
    std::tuple<T,T,T> two_one_diff( const T a_1, const T a_0, const T b ) {
        const auto [ _i, x_0 ] = two_diff( a_0, b );
        const auto [ x_2, x_1 ] = two_sum( a_1, _i );
        return std::make_tuple( x_2, x_1, x_0 );
    }

    // ==============
    //  Two Two Diff
    // ==============

    template<typename T>
    std::tuple<T,T,T,T> two_two_diff( const T a_1, const T a_0, const T b_1, const T b_0 ) {
        const auto [ _j, _0, x_0 ] = two_one_diff( a_1, a_0, b_0 );
        const auto [ x_3, x_2, x_1 ] = two_one_diff( _j, _0, b_1 );
        return std::make_tuple( x_3, x_2, x_1, x_0 );
    }

    // =======
    //  Split
    // =======

    template<typename T>
    std::tuple<T,T> split( const T a ) {
        const T c = double_constants.splitter * a;
        const T a_big = c - a;
        const T a_hi = c - a_big;
        const T a_lo = a - a_hi;
        return std::make_tuple( a_hi, a_lo );
    }

    // ==================
    //  Two Product Tail
    // ==================

    template<typename T> 
    T two_product_tail( const T a, const T b, const T x ) {
        const auto [ a_hi, a_lo ] = split( a );
        const auto [ b_hi, b_lo ] = split( b );
        const T err_1 = x - ( a_hi * b_hi );
        const T err_2 = err_1 - ( a_lo * b_hi );
        const T err_3 = err_2 - ( a_hi * b_lo );
        const T y = ( a_lo * b_lo ) - err_3;
        
        return y;
    }

    // =============
    //  Two Product
    // =============

    template<typename T> 
    std::tuple<T,T> two_product( const T a, const T b ) {
        T x = a * b;
        T y = two_product_tail( a, b, x );
        return std::make_tuple( x, y );
    }

    // ==========
    //  Estimate
    // ==========

    template<typename T>
    T estimate( const size_t e_len, std::tuple<T,T,T,T> e ) {
        T Q{};
        auto e_arr = tuple_to_array( e );
        for ( size_t i = 0; i < e_len; ++i ) {
            Q += e_arr[ i ];
        }
        return Q;
    }

    // =================
    //  Accumulate Term
    // =================

    template <size_t N,typename T>
	void accumulate_term( T& Q, T term, std::array<T,N>& h, std::size_t& h_index ) {
	    auto [ new_Q, hh ] = two_sum( Q, term );
	    Q = new_Q;
	    if ( hh != 0 ) {
	        set_index<N>( h, hh, h_index );
	        h_index++;
	    }
	}

	// ============================
    //  Has Greater Absolute Value
    // ============================

    template<typename T>
    bool has_greater_absolute_value( T a, T x ) {
    	return ( x > a ) == ( x > -a );
    }

    // =================
    //  Expansion State
    // =================

    template<typename T,size_t N>
	struct expansion_state {
	    const std::array<T,N-4>& e;
	    const std::array<T,4>& f;
	    std::array<T, N>& h;
	    T e_now;
	    T f_now;
	    std::size_t e_index;
	    std::size_t f_index;
	    std::size_t h_index;
	    T Q;
	    T hh;
	    std::size_t e_len;
	    std::size_t f_len;

	    expansion_state( const std::array<T, N-4>& e_, 
	    				 const std::array<T, 4>& f_, 
	    				 std::array<T, N>& h_, 
	    				 std::size_t e_len_, 
	    				 std::size_t f_len_ )
	        : e( e_ ), f( f_ ), h( h_ ), e_now( e_[0] ), f_now( f_[0] ),
	          e_index( 0 ), f_index( 0 ), h_index( 0 ), Q( 0 ), hh( 0 ), e_len( e_len_ ), f_len( f_len_ ) {}

	    expansion_state( const std::array<T,N-4>& e_, 
	    	             const std::array<T,4>& f_, 
	    	             std::array<T,N>& h_,
                    	 std::size_t e_len_, 
                    	 std::size_t f_len_,
                    	 T e_now_, 
                    	 T f_now_, 
                    	 std::size_t e_index_, 
                    	 std::size_t f_index_,
                    	 std::size_t h_index_, 
                    	 T Q_, 
                    	 T hh_ )
        	: e( e_ ), f( f_ ), h( h_ ), e_len( e_len_ ), f_len( f_len_ ),
          	  e_now( e_now_ ), f_now( f_now_ ), e_index( e_index_ ), f_index( f_index_ ),
          	  h_index( h_index_ ), Q( Q_ ), hh( hh_ ) {}

		bool operator==( const expansion_state<T,N>& rhs ) const {
		    return e_now == rhs.e_now &&
		           f_now == rhs.f_now &&
		           e_index == rhs.e_index &&
		           f_index == rhs.f_index &&
		           h_index == rhs.h_index &&
		           Q == rhs.Q &&
		           hh == rhs.hh &&
		           e_len == rhs.e_len &&
		           f_len == rhs.f_len &&
		           std::equal( h.begin(), h.end(), rhs.h.begin() ) &&
		           std::equal( e.begin(), e.end(), rhs.e.begin() ) &&
		           std::equal( f.begin(), f.end(), rhs.f.begin() );
		}

	    void advance_e() {
	        e_index++;
	        e_now = safe_get_index<N-4>( e, e_index );
	    }

	    void advance_f() {
	        f_index++;
	        f_now = safe_get_index<4>( f, f_index );
	    }
	};

	template<typename T, std::size_t N>
	std::ostream& operator<<( std::ostream& os, const expansion_state<T,N>& state ) {
    	os 	<< "{ e_now=" << state.e_now
       		<< ", f_now=" << state.f_now
       		<< ", e_index=" << state.e_index
       		<< ", f_index=" << state.f_index
       		<< ", h_index=" << state.h_index
       		<< ", Q=" << state.Q
       		<< ", hh=" << state.hh
       		<< ", e=[";
    		for ( auto v : state.e ) os << v << " ";
    		os << "], f=[";
    		for ( auto v : state.f ) os << v << " ";
    		os << "], h=[";
    		for ( auto v : state.h ) os << v << " ";
    		os << "]}";
    		return os;
	}

	// =======================
    //  Merge Expansion Terms
    // =======================

    template<typename T,std::size_t N>
    void merge_expansion_terms( expansion_state<T,N>& state ) {
       while ( ( state.e_index <= state.e_len ) && ( state.f_index <= state.f_len ) ) {
            if ( has_greater_absolute_value( state.e_now, state.f_now ) ) {
                std::tie( state.Q, state.hh ) = two_sum( state.Q, state.e_now );
                state.advance_e();
            } else {
                std::tie( state.Q, state.hh ) = fast_two_sum( state.Q, state.f_now );
                state.advance_f();
            }
            if ( state.hh != 0 ) { 
                set_index<N>( state.h, state.hh, state.h_index );
                state.h_index++;
            }
        }
    }

    // ====================
    //  Finalize Expansion
    // ====================

    template<typename T,std::size_t N>
    void finalize_expansion( expansion_state<T,N>& state ) {
        while ( state.e_index <= state.e_len ) {
            accumulate_term( state.Q, state.e_now, state.h, state.h_index );
			state.advance_e();
        }
        while ( state.f_index <= state.f_len ) {
			accumulate_term( state.Q, state.f_now, state.h, state.h_index );
            state.advance_f();
        }
        if ( state.Q != 0 || state.h_index == 1 ) {
            set_index<N>( state.h, state.Q, state.h_index );
            state.h_index++;
        }
    }

    // =======================
    //  Accumulate First Time
    // =======================

    template<typename T,std::size_t N>
    void accumlate_first_term( expansion_state<T,N>& state ) {
        if ( has_greater_absolute_value( state.f_now, state.e_now ) ) {
            std::tie( state.Q, state.hh ) = fast_two_sum( state.e_now, state.Q );
            state.advance_e();
        } else {
            std::tie( state.Q, state.hh ) = fast_two_sum( state.f_now, state.Q );
            state.advance_f();
        }
        if ( state.hh != 0 ) {
            set_index<N>( state.h, state.hh, state.h_index );
            state.h_index++;
        }
    }

    // ======================
    //  Init Expansion State
    // ======================

    template<typename T,std::size_t N>
    void init_expansion_state( expansion_state<T,N>& state ) {
        if ( has_greater_absolute_value( state.e_now, state.f_now ) ) {
            state.Q = state.e_now;
            state.advance_e();
        } else {
            state.Q = state.f_now;
            state.advance_f();
        }
    }

    // ==============================
    //  Fast Expansion Sum Zero Elim
    // ==============================

    template<size_t N,typename T>
    std::tuple<size_t,std::array<T,N>> fast_expansion_sum_zero_elim( const size_t e_len, const std::array<T,N-4>& e, 
                                                                     const size_t f_len, const std::array<T,4>& f,
                                                                     std::array<T,N>& h ) {
        expansion_state<T,N> state( e, f, h, e_len, f_len );
		init_expansion_state( state );
	    if ( ( state.e_index <= state.e_len ) && ( state.f_index <= state.f_len ) ) {
	        accumlate_first_term( state );
	        merge_expansion_terms( state );
	    }
	    finalize_expansion( state );
	    return std::make_tuple( state.h_index, state.h );
    }

    // =================
    //  Oreient 2 Adapt
    // =================

	template<typename T>
    T orient_2_adapt( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r,
     				  const std::pair<T,T>& ac, const std::pair<T,T>& bc, const T detsum ) {
        auto [ ac_x, ac_y ] = ac;
        auto [ bc_x, bc_y ] = bc;
        auto [ det_left, det_left_tail ] = two_product( ac_x, bc_y );
        auto [ det_right, det_right_tail ] = two_product( ac_y, bc_x );

        const auto B = two_two_diff( det_left, det_left_tail, det_right, det_right_tail );
        T det = estimate( 4, B ); 
        T err_bound = double_constants.ccw.B * detsum;

        if ( ( det >= err_bound ) || ( -det >= err_bound ) ) {
            return det;
        }

        T ac_x_tail = two_diff_tail( p.first, r.first, ac_x );
        T bc_x_tail = two_diff_tail( q.first, r.first, bc_x );
        T ac_y_tail = two_diff_tail( p.second, r.second, ac_y );
        T bc_y_tail = two_diff_tail( q.second, r.second, bc_y );

        if ( ac_x_tail == 0 && ac_y_tail == 0 && bc_x_tail == 0 && bc_y_tail == 0 ) {
            return det;
        }

        err_bound = double_constants.ccw.C * detsum + double_constants.result_err_bound * std::abs( det );
        det += ( ac_x * bc_y_tail + bc_y * ac_x_tail ) - ( ac_y * bc_x_tail + bc_x * ac_y_tail );
    
        if ( ( det >= err_bound ) || ( -det >= err_bound ) ) {
            return det;
        }

        auto [ s_1, s_0 ] = two_product( ac_x_tail, bc_y );
        auto [ t_1, t_0 ] = two_product( ac_y_tail, bc_x );
        auto u = two_two_diff( s_1, s_0, t_1, t_0 );

        std::array<T, 8> h_8 = { T( 0 ) };

        auto B_arr = tuple_to_array( B );
        auto u_arr = tuple_to_array( u );

        const auto [ c_1_len, c_1 ] = fast_expansion_sum_zero_elim<8>( 4, B_arr, 4, u_arr, h_8 );

        std::tie( s_1, s_0 ) = two_product( ac_x, bc_y_tail );
        std::tie( t_1, t_0 ) = two_product( ac_y, bc_x_tail );
        u = two_two_diff( s_1, s_0, t_1, t_0 );
        u_arr = tuple_to_array( u ); 

        std::array<T,12> h_12 = { ( T )0 };
        const auto [ c_2_len, c_2 ] = fast_expansion_sum_zero_elim<12>( c_1_len, c_1, 4, u_arr, h_12 );

        std::tie( s_1, s_0 ) = two_product( ac_x_tail, bc_y_tail );
        std::tie( t_1, t_0 ) = two_product( ac_y_tail, bc_x_tail );
        u = two_two_diff( s_1, s_0, t_1, t_0 );
        u_arr = tuple_to_array( u );
    
        std::array<T,16> h_16 = { ( T )0 };
        const auto [ d_len, d ] = fast_expansion_sum_zero_elim<16>( c_2_len, c_2, 4, u_arr, h_16 );

        return d[ d_len ];
    }

    // ==============
    //  Determinants
    // ==============

    template<typename T>
    struct determinents {
        T determinent;
        T left_determinet;
        T right_detminent;
    };

    // ==================
    //  Get Determinents
    // ==================

    template<typename T>
    determinents<T> get_determinents( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        T det_left = ( p.first - r.first ) * ( q.second - r.second ); 
        T det_right = ( p.second - r.second ) * ( q.first - r.first ); 
        return determinents{ det_left - det_right, det_left, det_right }; 
    }

    // =========================
    //  Is Orientation Reliable
    // =========================

    template<typename T>
    bool is_orientation_reliable( determinents<T> dets, T det_sum ) {
        if ( dets.determinent == 0 ) { 
            return true;
        }
        if ( dets.left_determinet > 0 && dets.right_detminent <= 0 || 
             dets.left_determinet < 0 && dets.right_detminent >= 0 ) { 
            return true; 
        } 
        const T err_bound = double_constants.ccw.A * det_sum; 
        if ( ( dets.determinent >= err_bound ) || ( -dets.determinent >= err_bound ) ) { 
            return true; 
        } 
        return false;
    }

    // ===================
    //  Calculate Det Sum
    // ===================

    template<typename T>
    T calculate_det_sum( determinents<T> dets ) {
        if ( dets.left_determinet > 0 ) {
            return dets.left_determinet + dets.right_detminent; 
        }
        return -dets.left_determinet - dets.right_detminent; 
    }

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    T calculate_det_sum( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) {
        auto dets = get_determinents( p, q, r );
        if ( dets.left_determinet > 0 ) {
            return dets.left_determinet + dets.right_detminent; 
        }
        return -dets.left_determinet - dets.right_detminent; 
    }

    // ================
    //  Get Difference
    // ================

    template<typename T>
    std::pair<T,T> get_difference( const std::pair<T,T>& p, const std::pair<T,T>& q ) {
        return { p.first - q.first, p.second - q.second };
    }

    // ==========
    //  Orient 2
    // ==========

    template<typename T> 
    T orient_2( const std::pair<T,T>& p, const std::pair<T,T>& q, const std::pair<T,T>& r ) { 
        auto dets = get_determinents( p, q, r );
        T det_sum = calculate_det_sum( dets );
        auto is_reliabe= is_orientation_reliable( dets, det_sum );
        if ( is_reliabe ) {
            return dets.determinent;
        }
        auto ac = get_difference( p, r );
        auto bc = get_difference( q, r );
        return orient_2_adapt( p, q, r, ac, bc, det_sum );   
    }

} // namespace adaptive_arithmetic