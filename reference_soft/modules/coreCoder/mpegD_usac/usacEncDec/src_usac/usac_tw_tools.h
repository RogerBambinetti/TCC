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


#ifndef USAC_TW_TOOLS_H_
#define USAC_TW_TOOLS_H_


int tw_resamp(const float *samplesIn,
              const int    startPos,
              int numSamplesIn,
              int numSamplesOut,
              const float *samplePos,
              float offsetPos,
              float *samplesOut);

int tw_calc_tw (int        tw_data_present,
                int        frameWasTd,
                int        *tw_ratio,
                WINDOW_SEQUENCE windowSequence,
                float      *warp_cont_mem,
                float      *sample_pos,
                float      *tw_trans_len,
                int        *tw_start_stop,
                float      *pitch_sum,
                int const mdctLen);

void tw_adjust_past ( WINDOW_SEQUENCE windowSequence,
                      WINDOW_SEQUENCE lastWindowSequence,
                      int frameWasTD,
                      int start_stop[],
                      float trans_len[],
                      int const mdctLen
                     );

void tw_init(void);

void tw_reset(const int  frame_len,
              float     *tw_cont_mem,
              float     *pitch_sum );

void tw_windowing_short(const float *wfIn,
                                 float *wfOut,
                                 int wStart,
                                 int wEnd,
                                 float warpedTransLenLeft,
                                 float warpedTransLenRight,
                                 const float *mdctWinTransLeft,
                                 const float *mdctWinTransRight,
                                 int const mdctLenShort);

void tw_windowing_long(const float *wfIn,
                                 float *wfOut,
                                 int wStart,
                                 int wEnd,
                                 int nlong,
                                 float warpedTransLenLeft,
                                 float warpedTransLenRight,
                                 const float *mdctWinTransLeft,
                                 const float *mdctWinTransRight);

void tw_windowing_past(const float *wfIn,
                       float *wfOut,
                       int wEnd,
                       int nLong,
                       float warpedTransLenRight,
                       const float *mdctWinTransRight);

#endif /* USAC_TW_TOOLS_H_ */
