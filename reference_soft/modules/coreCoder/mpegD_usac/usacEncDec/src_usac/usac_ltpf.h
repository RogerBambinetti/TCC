/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2015.
 
 ***********************************************************************************/
#ifndef __LTP_POST_H__
#define __LTP_POST_H__

typedef struct ltpf_data_tag {
  int L_frame;
  int L_transition;
  int enc_dec_delay;

  int pit_min;
  int pit_fr2;
  int pit_fr1;
  int pit_max;
  int pit_res;

  int data_present;
  int pitch_index;
  int gain_index;

  int pitch_int_past;
  int pitch_fr_past;
  float gain_past;
  int gain_index_past;
  float *mem_in;
  float *mem_out;
} LTPF_DATA;

void usac_ltpf_bs_duplicate(LTPF_DATA ltpfDataL, LTPF_DATA *ltpfDataR);

void usac_ltpf_init(
                    LTPF_DATA                *ltpfData,
                    int                       samplingRate,
                    int                       block_size_samples
);

void usac_ltpf_process(
                    LTPF_DATA                *ltpfData,
                    float                    *time_sample_vector
);

void usac_ltpf_free(
                    LTPF_DATA                *ltpfData
);

#endif
