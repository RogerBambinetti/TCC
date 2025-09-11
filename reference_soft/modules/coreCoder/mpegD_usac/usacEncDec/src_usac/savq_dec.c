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

/*---------------------------------------------------------------------------*
 *         SPLIT ALGEBRAIC VECTOR QUANTIZER BASED ON RE8 LATTICE             *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "re8.h"
#include "proto_func.h"


void SAVQ_dec(
  int *indx,    /* input:  index[] (4 bits per words)      */
  int *nvecq,   /* output: vector quantized                */
  int Nsv)      /* input:  number of subvectors (lg=Nsv*8) */  
{			  
  int    i, l, n, nq, nk, pos, c[8], kv[8];
  long   I;
  
  pos = Nsv;

  /* decode all subvectors */
  for (l=0; l<Nsv; l++) {
    nq = indx[l];        /* quantizer number (0,2,3..n) */

    nk = 0;
    n = nq;
    if (nq > 4)
    {
      nk = (nq-3)>>1;
      n = nq - nk*2;
    }

    if (nq > 0) {
      /* read base codebook index (I) */
      I = indx[pos++];
      
      /* read Voronoi indices (k[]) */
      for (i=0; i<8; i++) {
        kv[i] = indx[pos++];
      }
    } else {
      I = 0;
      for (i=0;i<8;i++) {
        kv[i] = 0;
      }
    }

    /* multi-rate RE8 decoder */
    RE8_dec(nq, I, kv, c);

    /* write decoded RE8 vector */
    for (i=0; i<8; i++) {
      nvecq[(l*8)+i] = c[i];
    }
  }

  return;
}
