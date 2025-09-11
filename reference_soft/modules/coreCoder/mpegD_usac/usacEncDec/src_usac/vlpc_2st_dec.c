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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "proto_func.h"


#define ORDER    16
#define LSF_GAP  50.0f
#define FREQ_MAX 6400.0f
#define FREQ_DIV 400.0f

void lsf_weight_2st(float *lsfq, float *w, int mode);

void reorder_lsf(float *lsf, float min_dist, int n);

void SAVQ_dec(
  int *indx,    /* input:  index[] (4 bits per words)      */
  int *nvecq,   /* output: vector quantized                */
  int Nsv);      /* input:  number of subvectors (lg=Nsv*8) */

void vlpc_2st_dec( 
  float *lsfq,    /* i/o:    i:1st stage   o:1st+2nd stage   */
  int *indx,      /* input:  index[] (4 bits per words)      */
  int mode)       /* input:  0=abs, >0=rel                   */
{
  int i;
  float w[ORDER];
  int xq[ORDER];

  /* weighting from the 1st stage */
  lsf_weight_2st(lsfq, w, mode);

  /* quantize */
  SAVQ_dec(indx, xq, 2);

  /* quantized lsf */
  for (i=0; i<ORDER; i++) lsfq[i] += (w[i]*(float)xq[i]);

  /* reorder */
  reorder_lsf(lsfq, LSF_GAP, ORDER);

  return;
}
