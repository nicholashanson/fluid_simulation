#ifndef LBM_COLLIDE_AND_STREAM_TBB_HPP
#define LBM_COLLIDE_AND_STREAM_TBB_HPP

#include <cstdlib>
#include <cstring>

#ifndef DPCPP_COMPILER
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#endif

using T = double;

namespace fs {

    namespace lbm {

        inline void collide_and_stream_tbb( double* D2Q9, unsigned char* obstacle, size_t steps ) {

            const T viscosity = 0.005;

            const T omega = 1 / ( 3 * viscosity + 0.5 );

            const size_t ydim = fs::settings::ydim;
            const size_t xdim = fs::settings::xdim;
            const size_t vec_len = ydim * xdim;

            T* D2Q9_n = ( T* )std::malloc( ydim * xdim * 9 * sizeof( T ) );

            T* D2Q9_n_initial = D2Q9_n;

            std::memcpy( D2Q9_n, D2Q9, ydim * xdim * 9 * sizeof( T ) );

            for ( size_t z = 0; z < steps; ++z ) {

                // collide

                tbb::parallel_for( tbb::blocked_range<size_t>( 0, vec_len ),
                    [&]( const tbb::blocked_range<size_t>& r ) {

                        for ( size_t i = r.begin(); i < r.end(); ++i ) {
                
                            const size_t index_offset = i * 9;

                            T rho_{};
                                
                            for ( size_t k = 0; k < 9; ++k )
                                rho_ += D2Q9[ index_offset + k ];

                            T ux_{};

                            ux_ = ( D2Q9[ index_offset + 1 ] + D2Q9[ index_offset + 5 ] +
                                    D2Q9[ index_offset + 8 ] - D2Q9[ index_offset + 3 ] -
                                    D2Q9[ index_offset + 6 ] - D2Q9[ index_offset + 7 ] ) / rho_;

                            T uy_{};

                            uy_ = ( D2Q9[ index_offset + 2 ] + D2Q9[ index_offset + 5 ] +
                                    D2Q9[ index_offset + 6 ] - D2Q9[ index_offset + 4 ] -
                                    D2Q9[ index_offset + 7 ] - D2Q9[ index_offset + 8 ] ) / rho_;

                            const T ux_2 = ux_ * ux_;
                            const T uy_2 = uy_ * uy_;
                            const T u_215 = 1.5 * ( ( ux_2 ) + ( uy_2 ) );
                            const T ux_3 = ux_ * 3;
                            const T uy_3 = uy_ * 3;

                            D2Q9[ index_offset ] += omega * ( ( 4.0 / 9.0 ) * rho_ * ( 1 - u_215 ) - D2Q9[ index_offset ] );

                            D2Q9[ index_offset + 1 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 + ux_3 + 4.5 * ux_2 - u_215 ) - D2Q9[ index_offset + 1 ] );

                            D2Q9[ index_offset + 2 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 + uy_3 + 4.5 * uy_2 - u_215 ) - D2Q9[ index_offset + 2 ] );

                            D2Q9[ index_offset + 3 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 - ux_3 + 4.5 * ux_2 - u_215 ) - D2Q9[ index_offset + 3 ] );    
                        
                            D2Q9[ index_offset + 4 ] += omega * ( ( 1.0f / 9.0f ) * rho_ * ( 1 - uy_3 + 4.5 * uy_2 - u_215 )  - D2Q9[ index_offset + 4 ] ); 
                        
                            const T uxuy_2 = 2 * ux_ * uy_;

                            const T u_2 = ux_2 + uy_2;

                            D2Q9[ index_offset + 5 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 + ux_3 + uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - D2Q9[ index_offset + 5 ] ); 

                            D2Q9[ index_offset + 6 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 - ux_3 + uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - D2Q9[ index_offset + 6 ] );

                            D2Q9[ index_offset + 7 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 - ux_3 - uy_3 + 4.5 * ( u_2 + uxuy_2 ) - u_215 ) - D2Q9[ index_offset + 7 ] ); 
                    
                            D2Q9[ index_offset + 8 ] += omega * ( ( 1.0f / 36.0f ) * rho_ * ( 1 + ux_3 - uy_3 + 4.5 * ( u_2 - uxuy_2 ) - u_215 ) - D2Q9[ index_offset + 8 ] ); 
                        }
                    }
                );

                // stream

                tbb::parallel_for( tbb::blocked_range<size_t>( 1, ydim - 1 ),
                    [&]( const tbb::blocked_range<size_t>& r ) {

                        for ( size_t y = r.begin(); y < r.end(); ++y ) {

                            for ( size_t x = 1; x < xdim - 1; ++x ) {

                                const size_t base_index = ( x + y * xdim ) * 9;

                                D2Q9_n[ base_index ] = D2Q9[ base_index ];

                                D2Q9_n[ base_index + 1 ] = D2Q9[ ( ( x - 1 ) + y * xdim ) * 9 + 1 ];

                                D2Q9_n[ base_index + 4 ] = D2Q9[ ( x + ( y + 1 ) * xdim ) * 9 + 4 ];

                                D2Q9_n[ base_index + 3 ] = D2Q9[ ( ( x + 1 ) + y * xdim ) * 9 + 3 ];

                                D2Q9_n[ base_index + 2 ] = D2Q9[ ( x + ( y - 1 ) * xdim ) * 9 + 2 ];

                                D2Q9_n[ base_index + 8 ] = D2Q9[ ( ( x - 1 ) + ( y + 1 ) * xdim ) * 9 + 8 ];

                                D2Q9_n[ base_index + 7 ] = D2Q9[ ( ( x + 1 ) + ( y + 1 ) * xdim ) * 9 + 7 ];

                                D2Q9_n[ base_index + 6 ] = D2Q9[ ( ( x + 1 ) + ( y - 1 ) * xdim ) * 9 + 6 ];

                                D2Q9_n[ base_index + 5 ] = D2Q9[ ( ( x - 1 ) + ( y - 1 ) * xdim ) * 9 + 5 ];
                            }
                        }
                    }
                );

                for ( size_t y = 1; y < ydim; ++y ) {

                    for ( size_t x = 1; x < xdim - 1; ++x ) {

                        if ( obstacle[ x + y * xdim ] ) {

                            size_t index = ( x + y * xdim ) * 9;
                
                            D2Q9_n[ ( x + 1 + y * xdim ) * 9 + 1 ] = D2Q9_n[ index + 3 ];

                            D2Q9_n[ ( x - 1 + y * xdim ) * 9 + 3 ] = D2Q9_n[ index + 1 ];

                            D2Q9_n[ ( x + ( y + 1 ) * xdim ) * 9 + 2 ] = D2Q9_n[ index + 4 ];

                            D2Q9_n[ ( x + ( y - 1 ) * xdim ) * 9 + 4 ] = D2Q9_n[ index + 2 ];

                            D2Q9_n[ ( x + 1 + ( y + 1 ) * xdim ) * 9 + 5 ] = D2Q9_n[ index + 7 ];

                            D2Q9_n[ ( x - 1 + ( y + 1 ) * xdim ) * 9 + 6 ] = D2Q9_n[ index + 8 ];

                            D2Q9_n[ ( x + 1 + ( y - 1 ) * xdim ) * 9 + 8 ] = D2Q9_n[ index + 6 ];

                            D2Q9_n[ ( x - 1 + ( y - 1 ) * xdim ) * 9 + 7 ] = D2Q9_n[ index + 5 ];
                        }
                    }  
                }

                std::swap( D2Q9, D2Q9_n );
            }

            if ( D2Q9_n != D2Q9_n_initial ) {

                std::memcpy( D2Q9_n, D2Q9, ydim * xdim * 9 * sizeof( T ) );

                std::swap( D2Q9, D2Q9_n );
            }

            std::free( D2Q9_n );
        } 
    
    } // lbm

} // fs

#endif