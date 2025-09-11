/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria.
Those intending to use this software module in products are advised that
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2003.

*************************************************************************/

#ifndef _INCLUDED_MS_H_
#define _INCLUDED_MS_H_

#include "tf_mainStruct.h"

typedef struct {
  int ms_mask;
  int ms_used[MAX_SHORT_WINDOWS][SFB_NUM_MAX];
} MSInfo;
  
void MSInverse(int num_sfb,
  int *sfb_offset,
  int ms_used[MAX_SHORT_WINDOWS][SFB_NUM_MAX],
  int num_windows,
  int blockSizeSamples,
  double *left,
  double *right);

void MSApply(int num_sfb,
  int *sfb_offset,
  int ms_used[MAX_SHORT_WINDOWS][SFB_NUM_MAX],
  int num_windows,
  int blockSizeSamples,
  double *left,
  double *right);

void select_ms(
  double *spectral_line_vector[MAX_TIME_CHANNELS],
  int         blockSizeSamples,
  int         num_window_groups,
  const int   window_group_length[MAX_SHORT_WINDOWS],
  const int   sfb_offset[MAX_SCFAC_BANDS+1],
  int         start_sfb,
  int         end_sfb,
  MSInfo      *msInfo,
  int         ms_select,
  int         debugLevel,
  int         chOffset);

#endif
