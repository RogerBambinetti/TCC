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

#ifndef table_decl_h
#define table_decl_h

#include "cnst.h"

/* ISF initialization vector */
extern const float lsf_init[M];

/* MDCT tables */
extern const float sineWindow64[64];
extern const float sineWindow96[96];
extern const float sineWindow128[128];
extern const float sineWindow192[192];
extern const float sineWindow256[256];
extern const float sineWindow384[384];
extern const float sineWindow512[512];
extern const float kbdWindow64[64];
extern const float kbdWindow96[96];
extern const float kbdWindow128[128];
extern const float kbdWindow192[192];
extern const float kbdWindow256[256];
extern const float kbdWindow384[384];
extern const float kbdWindow512[512];

#ifdef WINDOW_SHAPE_FOR_TCX
extern const float kbdWindow64[64];
extern const float kbdWindow96[96];
extern const float kbdWindow128[128];
extern const float kbdWindow192[192];
extern const float kbdWindow256[256];
extern const float kbdWindow384[384];
extern const float kbdWindow512[512];
#endif

/* For pitch predictor */
extern const float inter4_2[PIT_FIR_SIZE2];

/* For bass postfilter */
extern const float filt_lp[1 + L_FILT];
extern const float filt_lp2[1+2*(L_FILT)+1];

/* For LPC analyisi*/
extern const float lag_window[17];

#endif
