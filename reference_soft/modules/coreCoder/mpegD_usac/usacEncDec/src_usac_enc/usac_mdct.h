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


#ifndef USAC_MDCT_H_
#define USAC_MDCT_H_

void calc_window_db( double window[], int len, WINDOW_SHAPE wfun_select);

void buffer2fd(
  double           p_in_data[],
  double           p_out_mdct[],
  double           p_overlap[],
  WINDOW_SEQUENCE  windowSequence,
  WINDOW_SHAPE     wfun_select,      /* offers the possibility to select different window functions */
  WINDOW_SHAPE     wfun_select_prev,
  int              nlong,            /* shift length for long windows   */
  int              nshort,           /* shift length for short windows  */
  Mdct_in          overlap_select,   /* select mdct input *TK*          */
                                     /* switch (overlap_select) {       */
                                     /* case OVERLAPPED_MODE:                */
                                     /*   p_in_data[]                   */
                                     /*   = overlapped signal           */
                                     /*         (bufferlength: nlong)   */
                                     /* case NON_OVERLAPPED_MODE:            */
                                     /*   p_in_data[]                   */
                                     /*   = non overlapped signal       */
                                     /*         (bufferlength: 2*nlong) */
  int              previousMode,
  int              nextMode,
  int              num_short_win,     /* number of short windows per frame */
  int              twMdct,            /* TW-MDCT flag */
  float            sample_pos[],      /* TW sample positions */
  float            tw_trans_len[],    /* TW transition lengths */
  int              tw_start_stop[]    /* TW window borders */
);
#endif /* USAC_MDCT_H_ */
