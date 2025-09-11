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

/*---------------------------------------------------------------------*
 * routine int_lpc()                                                   *
 * ~~~~~~~~~~~~~~~~~                                                   *
 * Find the interpolated LPC parameters in every subframes.            *
 *---------------------------------------------------------------------*/
#include "acelp_plus.h"

void int_lpc_np1(float lsp_old[],  /* input : LSPs from past frame              */
                 float lsp_new[],  /* input : LSPs from present frame           */
                 float a[],        /* output: LP coefficients in both subframes */
                 int   nb_subfr,   /* input: number of subframe                 */
                 int   m           /* input : order of LP filter                */
)
{
  float lsp[M], *p_a, inc, fnew, fold; 
  int i, k;

  inc = 1.0f / (float)nb_subfr;
  p_a = a;
  fnew = 0.0f;

  for (k = 0; k < nb_subfr; k++){
    fold = 1.0f - fnew;
    for (i = 0; i < m; i++){
      lsp[i] = (float)(lsp_old[i]*fold + lsp_new[i]*fnew);
    }
    fnew += inc;
    E_LPC_f_lsp_a_conversion(lsp, p_a);
    p_a += (m+1);
  }  
  /* estimated coef for next subframe: use isf_new */
  E_LPC_f_lsp_a_conversion(lsp_new, p_a);

  return;
}
