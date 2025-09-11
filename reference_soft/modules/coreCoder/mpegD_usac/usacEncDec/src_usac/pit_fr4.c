/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/


/*-------------------------------------------------------------------*
 * procedure pitch_fr4                                               *
 * ~~~~~~~~~~~~~~~~~~~                                               *
 * Find the closed loop pitch period with 1/4 subsample resolution.  *
 *-------------------------------------------------------------------*/
#include <math.h>
#include "acelp_plus.h"
/* locals functions */
/*-------------------------------------------------------------------*
 * Function  pred_lt4:                                               *
 *           ~~~~~~~~~                                               *
 *-------------------------------------------------------------------*
 * Compute the result of long term prediction with fractionnal       *
 * interpolation of resolution 1/4.                                  *
 *                                                                   *
 * On return exc[0..L_subfr-1] contains the interpolated signal      *
 *   (adaptive codebook excitation)                                  *
 *-------------------------------------------------------------------*/
void pred_lt4(
  float exc[],       /* in/out: excitation buffer */
  int   T0,          /* input : integer pitch lag */
  int   frac,        /* input : fraction of lag   */
  int   L_subfr      /* input : subframe size     */
)
{
  int   i, j;
  float s, *x0, *x1, *x2;
  const float *c1, *c2;

  x0 = &exc[-T0];
  frac = -frac;
  if (frac < 0) {
    frac += PIT_UP_SAMP;
    x0--;
  }
  for (j=0; j<L_subfr; j++)
  {
    x1 = x0++;
    x2 = x1+1;
    c1 = &inter4_2[frac];
    c2 = &inter4_2[PIT_UP_SAMP-frac];
    s = 0.0;
    for(i=0; i<PIT_L_INTERPOL2; i++, c1+=PIT_UP_SAMP, c2+=PIT_UP_SAMP) {
      s += (*x1--) * (*c1) + (*x2++) * (*c2);
    }
    exc[j] = s;
  }
  return;
}
