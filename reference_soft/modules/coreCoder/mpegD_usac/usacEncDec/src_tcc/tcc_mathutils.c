/*
 * tcc_mathutils.c
 *
 *  This file contains math functions which are used by TCC decoder.
 *
 *
 *	 This software module was originally developed by
 *
 *		Zylia Sp. z o.o.
 *
 *		Authors:
 *			Andrzej Ruminski ( andrzej.ruminski@zylia.pl )
 *			Lukasz Januszkiewicz ( lukasz.januszkiewicz@zylia.pl )
 *			Marzena Malczewska ( marzena.malczewska@zylia.pl )
 *
 *	 in the course of development of the ISO/IEC 23003-3 for reference purposes and its
 * 	 performance may not have been optimized. This software module is an implementation
 * 	 of one or more tools as specified by the ISO/IEC 23003-3 standard. ISO/IEC gives
 *	 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 *	 and make derivative works of this software module or modifications  thereof for use
 *	 in implementations or products claiming conformance to the ISO/IEC 23003-3 standard
 *	 and which satisfy any specified conformance criteria. Those intending to use this
 *	 software module in products are advised that its use may infringe existing patents.
 *	 ISO/IEC have no liability for use of this software module or modifications thereof.
 *	 Copyright is not released for products that do not conform to the ISO/IEC 23003-3
 *	 standard.
 *
 *	 Zylia Sp. z o.o. retains full right to modify and use the code for its
 *	 own purpose, assign or donate the code to a third party and to inhibit third parties
 *	 from using the code for products that do not conform to MPEG-related ITU Recommenda-
 *	 tions and/or ISO/IEC International Standards.
 *
 *	 This copyright notice must be included in all copies or derivative works.
 *
 *	 Copyright (c) ISO/IEC 2016.
 */

#include "tcc_mathutils.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double interpolateValue( int i, int n, double x1, double x2 );
static float princargValue (float inVal);


#ifdef COS_TAYLOR
static int factorial[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600};

static double absF ( double x ) {
	return ( x < 0 ? -x : x );
}

static double cosT (double x, int n) {

	int negative = 1;
	double ret = 0.0;
	double x2 = 0.0;
	int sig = 1;
	double x_pow = 1;
	int i;

	/* convert x if x > pi/2 */
	x = absF(x);
	if (x > (M_PI/2)) {
		negative = -1;
		x = M_PI - x;
	}

	ret   = 0.0;
	x2    = x * x;
	sig   = 1;
	x_pow = 1;

	for (i = 0; i<=n; i++) {
		ret += (x_pow / factorial[2*i]) * sig ;
		x_pow *= x2;
		sig *= -1;
	}

	return negative*ret;
}
#endif

/* conver angle to angle in [ -PI, PI ] */
static float princargValue (float inVal)
{
  return (float)(((inVal + M_PI) - (-2.0* M_PI * floor((inVal + M_PI)/(-2.0*M_PI)))) + M_PI);
}


static int sign( double x ) {
	if ( x > 0.0 ) return 1;
	return ( x < 0.0 ? -1 : 0 );
}

/* dequant function */
double iquant (int dqCoeff, double del, double thr)
{
  int s =  sign(dqCoeff);
  dqCoeff = abs(dqCoeff);
  return ( ((double)dqCoeff * del) + (thr - (del/2.0)) )*s;
}

/* invert DCT transform */
void idct (double* buffer, int n)
{
	double tempBuffer[64];
	int i,j;

	for( i = 0; i < n; i++ ) {
		tempBuffer[i] = 0.0;
		for( j = 0; j < n; j++ ) {
			tempBuffer[i] += ( j == 0 ) ?
						     sqrt( 1.0/n ) * buffer[j] * cos( M_PI / ( 2.0 * n ) * ( 2.0 * (i + 1.0) - 1.0 ) * j ) :
							 sqrt( 2.0/n ) * buffer[j] * cos( M_PI / ( 2.0 * n ) * ( 2.0 * (i + 1.0) - 1.0 ) * j );
		}
	}

	memcpy( buffer, tempBuffer, n * sizeof(double) );

}


static double interpolateValue( int i, int n, double x1, double x2 ){

	if( i == 0 ) return x1;

	return x1 + (double)i/(double)(n) * (x2 - x1);

}


double sinSynth( double f1, double f2,
				 double a1, double a2,
				 int hopSize,
				 double lastPhaseValue,
				 float* resultInternal) {
	int i;
	double phase = lastPhaseValue;
	short orderT = 5;

	resultInternal[0] += (float)
					(interpolateValue( 0, hopSize, a1, a2 )
#ifdef COS_TAYLOR
					* cosT( phase, orderT ) ) ;
#else
					* cos( phase ) ) ;
#endif

	for ( i = 1; i < hopSize; i++ ) {
		lastPhaseValue += 2.0 * M_PI * interpolateValue( i, hopSize, f1, f2 );
		phase = princargValue( (float) lastPhaseValue );

		resultInternal[i] += (float)
				(interpolateValue( i, hopSize, a1, a2 )
#ifdef COS_TAYLOR
					* cosT( phase, orderT ) ) ;
#else
					* cos( phase ) ) ;
#endif

	}

	lastPhaseValue = lastPhaseValue + 2.0 * M_PI * f2;

	return princargValue((float)lastPhaseValue);
}




