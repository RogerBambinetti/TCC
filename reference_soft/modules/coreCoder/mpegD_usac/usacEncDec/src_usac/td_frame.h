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

#ifndef TD_FRAME_INC
#define TD_FRAME_INC

#include "cnst.h"

typedef struct {
  int core_mode_index;
  int mod[NB_DIV];
  int fac_gain[NB_DIV];
  int fac[NB_DIV*LFAC_1024];
  int fdFac[LFAC_1024+1];
  int mean_energy[NB_DIV];
  int acb_index[NB_SUBFR_SUPERFR_1024];
  int tcx_noise_factor[NB_DIV];
  int tcx_global_gain[NB_DIV]; /* float?*/
  int tcx_arith_reset_flag;
  int tcx_quant[L_FRAME_1024];
  int tcx_lg[NB_DIV];
  int ltp_filtering_flag[NB_SUBFR_SUPERFR_1024];
  int icb_index[NB_SUBFR_SUPERFR_1024][8];
  int gains[NB_SUBFR_SUPERFR_1024];
  int mode_lpc[NB_DIV];
  int lpc[110]; /* 5*22 lpc parameters */
} td_frame_data;
#endif
