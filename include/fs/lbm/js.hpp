#ifndef LBM_FLAT_HPP
#define LBM_FLAT_HPP

#include <array>
#include <cmath>

#include <settings.hpp>

namespace js {

    inline const size_t xdim = fs::settings::xdim;	
    inline const size_t ydim = fs::settings::ydim;
    inline const double four9ths = 4.0 / 9.0;					
    inline const double one9th = 1.0 / 9.0;
    inline const double one36th = 1.0 / 36.0;
    inline const size_t barrierSize = 1;

    using double_arr = std::array<double, xdim*ydim>;
    using bool_arr = std::array<bool, xdim*ydim>;	

    inline double_arr n0;		
    inline double_arr nN;
    inline double_arr nS;
    inline double_arr nE;
    inline double_arr nW;
    inline double_arr nNE;
    inline double_arr nSE;
    inline double_arr nNW;
    inline double_arr nSW;
    inline double_arr rho;			
    inline double_arr ux;			
    inline double_arr uy;
    inline double_arr u;
    inline double_arr curl;
    inline bool_arr barrier;

    inline void setEquil( size_t x, size_t y, double newux, double newuy, double newrho ) {
        const size_t i = x + y*xdim;
        const double ux3 = 3 * newux;
        const double uy3 = 3 * newuy;
        const double ux2 = newux * newux;
        const double uy2 = newuy * newuy;
        const double uxuy2 = 2 * newux * newuy;
        const double u2 = ux2 + uy2;
        const double u215 = 1.5 * u2;
        n0[i]  = four9ths * newrho * (1                              - u215);
        nE[i]  =   one9th * newrho * (1 + ux3       + 4.5*ux2        - u215);
        nW[i]  =   one9th * newrho * (1 - ux3       + 4.5*ux2        - u215);
        nN[i]  =   one9th * newrho * (1 + uy3       + 4.5*uy2        - u215);
        nS[i]  =   one9th * newrho * (1 - uy3       + 4.5*uy2        - u215);
        nNE[i] =  one36th * newrho * (1 + ux3 + uy3 + 4.5*(u2+uxuy2) - u215);
        nSE[i] =  one36th * newrho * (1 + ux3 - uy3 + 4.5*(u2-uxuy2) - u215);
        nNW[i] =  one36th * newrho * (1 - ux3 + uy3 + 4.5*(u2-uxuy2) - u215);
        nSW[i] =  one36th * newrho * (1 - ux3 - uy3 + 4.5*(u2+uxuy2) - u215);
        rho[i] = newrho;
        ux[i] = newux;
        uy[i] = newuy;
    }

    inline void setBoundaries() {
        const double u0 = 0.1;
        for ( size_t x = 0;  x < xdim; x++ ) {
            setEquil( x, 0, u0, 0, 1 );
            setEquil( x, ydim-1, u0, 0, 1 );
        }
        for (size_t y = 1; y < ydim - 1; y++ ) {
            setEquil( 0, y, u0, 0, 1 );
            setEquil( xdim-1, y, u0, 0, 1 );
        }
    }

    inline void collide() {
        const double viscosity = 0.005;	
        const double omega = 1 / ( 3 * viscosity + 0.5 );
        for ( size_t y = 0; y < ydim - 1; y++ ) {
            for ( size_t x = 0; x < xdim - 1; x++ ) {
                const size_t i = x + y * xdim;	
             
                const double thisrho = n0[i] + nN[i] + nS[i] + nE[i] + nW[i] + nNW[i] + nNE[i] + nSW[i] + nSE[i];
                rho[i] = thisrho;
                const double thisux = (nE[i] + nNE[i] + nSE[i] - nW[i] - nNW[i] - nSW[i]) / thisrho;
                ux[i] = thisux;
                const double thisuy = (nN[i] + nNE[i] + nNW[i] - nS[i] - nSE[i] - nSW[i]) / thisrho;
                uy[i] = thisuy;

                const double one9thrho = one9th * thisrho;		
                const double one36thrho = one36th * thisrho;
                const double ux3 = 3 * thisux;
                const double uy3 = 3 * thisuy;
                const double ux2 = thisux * thisux;
                const double uy2 = thisuy * thisuy;
                const double uxuy2 = 2 * thisux * thisuy;
                const double u2 = ux2 + uy2;

                u[i] = std::sqrt( u2 );
                const double u215 = 1.5 * u2;
                n0[i]  += omega * (four9ths*thisrho * (1                          - u215) - n0[i]);
                nE[i]  += omega * (   one9thrho * (1 + ux3       + 4.5*ux2        - u215) - nE[i]);
                nW[i]  += omega * (   one9thrho * (1 - ux3       + 4.5*ux2        - u215) - nW[i]);
                nN[i]  += omega * (   one9thrho * (1 + uy3       + 4.5*uy2        - u215) - nN[i]);
                nS[i]  += omega * (   one9thrho * (1 - uy3       + 4.5*uy2        - u215) - nS[i]);
                nNE[i] += omega * (  one36thrho * (1 + ux3 + uy3 + 4.5*(u2+uxuy2) - u215) - nNE[i]);
                nSE[i] += omega * (  one36thrho * (1 + ux3 - uy3 + 4.5*(u2-uxuy2) - u215) - nSE[i]);
                nNW[i] += omega * (  one36thrho * (1 - ux3 + uy3 + 4.5*(u2-uxuy2) - u215) - nNW[i]);
                nSW[i] += omega * (  one36thrho * (1 - ux3 - uy3 + 4.5*(u2+uxuy2) - u215) - nSW[i]);
            }
        }
        for ( size_t y = 1; y < ydim - 2; y++ ) {
            nW[ xdim - 1 + y * xdim ] = nW[ xdim - 2 + y * xdim ];		
            nNW[ xdim - 1 + y * xdim ] = nNW[ xdim - 2 + y * xdim ];
            nSW[ xdim - 1 + y * xdim ] = nSW[ xdim - 2 + y * xdim ];
        }
    }

    inline void stream( bool no_bounce = false ) {
        for ( size_t y=ydim-2; y>0; y--) {			
            for ( size_t x=1; x<xdim-1; x++) {
                nN[x+y*xdim] = nN[x+(y-1)*xdim];			
                nNW[x+y*xdim] = nNW[x+1+(y-1)*xdim];		
            }
        }
        for ( size_t y=ydim-2; y>0; y--) {			
            for ( size_t x=xdim-2; x>0; x--) {
                nE[x+y*xdim] = nE[x-1+y*xdim];			 
                nNE[x+y*xdim] = nNE[x-1+(y-1)*xdim];		
            }
        }
        for ( size_t y=1; y<ydim-1; y++) {			
            for ( size_t x=xdim-2; x>0; x--) {
                nS[x+y*xdim] = nS[x+(y+1)*xdim];			
                nSE[x+y*xdim] = nSE[x-1+(y+1)*xdim];	
            }
        }
        for ( size_t y=1; y<ydim-1; y++) {				
            for ( size_t x=1; x<xdim-1; x++) {
                nW[x+y*xdim] = nW[x+1+y*xdim];		
                nSW[x+y*xdim] = nSW[x+1+(y+1)*xdim];		
            }
        }
        if ( no_bounce == true ) return;

        for ( size_t y=1; y<ydim-1; y++) {				
            for ( size_t x=1; x<xdim-1; x++) {
                if ( barrier[x+y*xdim]) {
                    size_t index = x + y*xdim;
                    nE[x+1+y*xdim] = nW[index];
                    nW[x-1+y*xdim] = nE[index];
                    nN[x+(y+1)*xdim] = nS[index];
                    nS[x+(y-1)*xdim] = nN[index];
                    nNE[x+1+(y+1)*xdim] = nSW[index];
                    nNW[x-1+(y+1)*xdim] = nSE[index];
                    nSE[x+1+(y-1)*xdim] = nNW[index];
                    nSW[x-1+(y-1)*xdim] = nNE[index];
                }
            }
        }
    }

    inline void computeCurl() {
        for (size_t y=1; y<ydim-1; y++) {
            for (size_t x=1; x<xdim-1; x++) {
                curl[x+y*xdim] = uy[x+1+y*xdim] - uy[x-1+y*xdim] - ux[x+(y+1)*xdim] + ux[x+(y-1)*xdim];
            }
        }
    }
    
    inline void initFluid() {
        double u0 = 0.1;
        for (size_t y=0; y<ydim; y++) {
            for (size_t x=0; x<xdim; x++) {
                setEquil(x, y, u0, 0, 1);
                curl[x+y*xdim] = 0.0;
            }
        }
        for ( size_t y = 0; y < ydim; y++ ) {
            for ( size_t x = 0; x < xdim; x++ ) {
                barrier[ x+y*xdim ] = false;
            }
        }
    }

    inline void initObstacle() {

        barrier[ 4 + 4 * xdim ] = true;
        /*
        for ( size_t y = ( ydim / 2 ) - barrierSize; y <= ( ydim / 2 ) + barrierSize; ++y ) {
		    size_t x = ydim / 3;
		    barrier[ x + y * xdim ] = true;
	    }
        */
    }

    inline void simulate() {
        setBoundaries();
        collide();
        stream(); 
    }	

}

#endif







