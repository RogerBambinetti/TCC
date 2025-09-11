/***********************************************************************************
 
 This software module was originally developed by 
 
 Qualcomm Techonologies, Inc. (QTI)
 
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
 
 Qualcomm Techonologies, Inc. (QTI) retains full right to modify and use the code 
 for its own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#ifndef __HOA_MATRIX_COMMON_H__
#define __HOA_MATRIX_COMMON_H__


#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


#ifndef _MSC_VER
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))
#endif
#define array_length(a) (sizeof(a) / sizeof(a[0]))
#define static_assert(c) extern char __static_assert_failed__[2 * (c) - 1]


typedef enum {
  SP_PAIR_CENTER, /* one center speaker */
  SP_PAIR_SYMMETRIC, /* a symmetric L/R speaker pair */
  SP_PAIR_SINGLE, /* an asymmetric single speaker */
  SP_PAIR_NONE /* none */
} SP_PAIR_TYPE;

typedef struct SpeakerInformationStruct {
  short elevation; /* speaker elevation in degrees, positive angles upwards */
  short azimuth; /* speaker azimuth in degrees, positive angles to the left */
  short isLFE; /* whether the speaker type is LFE */
  short originalPosition; /* original speaker position in the channel list */
  short isAlreadyUsed; /* channel is already used in the compact channel list */
  struct SpeakerInformationStruct* symmetricPair; /* the right speaker of a symmetric L/R speaker pair */
  SP_PAIR_TYPE pairType; /* the type of pair for compact speaker configurations */
} SpeakerInformation;

#define HOA_MATRIX_MAX_HOA_ORDER 29
#define HOA_MATRIX_MAX_HOA_COEF_COUNT (HOA_MATRIX_MAX_HOA_ORDER+1)*(HOA_MATRIX_MAX_HOA_ORDER+1)
#define HOA_MATRIX_MAX_SPEAKER_COUNT 32
#define HOA_MATRIX_GAIN_ZERO -256.0f
#define HOA_MATRIX_BITBUFSIZE 4096*2
#define HOA_MATRIX_LINEAR_GAIN_ZERO 0.0f
typedef struct {
	int minGain[HOA_MATRIX_MAX_HOA_ORDER+1];
	int maxGain[HOA_MATRIX_MAX_HOA_ORDER+1]; 
  int precisionLevel;
  int rawCodingNonzeros;
} CoderState;

void CoderStateInit(CoderState* cs, float maxGain);

int findSymmetricSpeakers(int outputCount, SpeakerInformation* outputConfig, int hasLfeRendering);

void createSymSigns(int* symSigns, int hoaOrder);
void create2dBitmask(int* bitmask, int hoaOrder);

#endif /* __HOA_MATRIX_COMMON_H__ */
