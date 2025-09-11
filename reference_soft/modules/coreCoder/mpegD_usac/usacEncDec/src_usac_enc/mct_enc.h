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

 Copyright (c) ISO/IEC 2016.

 ***********************************************************************************/

#ifndef MCT_ENC_H_
#define MCT_ENC_H_ 1

#include "tf_mainStruct.h"

#define MAX_NUM_MCT_PAIRS (32)

typedef struct {
  int mctSignalingType;
  int numChannels;
  int numPairs;
  int extElemIndex;
  int channelPair[MAX_NUM_MCT_PAIRS][2];
  int pairAlpha[MAX_NUM_MCT_PAIRS][MAX_SCFAC_BANDS];
  int pairMctMask[MAX_NUM_MCT_PAIRS][MAX_SCFAC_BANDS];
  int numMaskBands[MAX_NUM_MCT_PAIRS];  
} MCTData;

void mct_encode(double *spectral_line_vector[MAX_TIME_CHANNELS],
                int         blockSizeSamples,
                WINDOW_SEQUENCE         windowSequence[MAX_TIME_CHANNELS],
                int         num_window_groups,
                const int   window_group_length[MAX_SHORT_WINDOWS],
                const int   sfb_offset[MAX_SCFAC_BANDS+1],
                int         endSfb,
                MCTData     *mctData,
                int         debugLevel,
                int         chOffset);

void mct_write_bitstream(MCTData     *mctData,
                         int indepFlag,
                         unsigned char *extensionPayload,
                         int *extensionPayloadSize,
                         int *extensionPayloadPresent);
#endif