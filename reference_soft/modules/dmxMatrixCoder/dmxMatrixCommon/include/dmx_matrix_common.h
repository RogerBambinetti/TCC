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
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef __DMX_MATRIX_COMMON_H__
#define __DMX_MATRIX_COMMON_H__


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
  SP_PAIR_NONE /* the right speaker of a symmetric L/R speaker pair */
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


extern const short compactTemplates_InputIndex[];
extern const short compactTemplates_OutputIndex[];
extern const signed char* compactTemplates_Data[];


#define DMX_MATRIX_MAX_SPEAKER_COUNT 32
#define DMX_MATRIX_GAIN_ZERO -256.0f
#define DMX_MATRIX_CODER_STATE_COUNT_MAX 512
#define DMX_MATRIX_GAIN_TABLE_SIZE_MAX ((22 - (-46)) * (1 << 2) + 2)
#define DMX_MATRIX_BITBUFSIZE 4096

typedef struct {
  int minGain;
  int maxGain;
  int precisionLevel;
  int rawCodingNonzeros;
  int gainLGRParam;
  float history[DMX_MATRIX_CODER_STATE_COUNT_MAX];
  int historyCount;
  float gainTable[DMX_MATRIX_GAIN_TABLE_SIZE_MAX];
  int gainTableSize;
} CoderState;

void CoderStateInit(CoderState* cs);
void CoderStateGenerateGainTable(CoderState* cs);


void ConvertToCompactConfig(int inputCount, SpeakerInformation* inputConfig,
    int* compactInputCount, SpeakerInformation* compactInputConfig[]);

signed char* FindCompactTemplate(int inputIndex, int outputIndex);


/* parameters struct for peak filter definition */
typedef struct {
  float f; /* peak frequency [Hz] */
  float q; /* peak Q factor */
  float g; /* peak gain [dB] */
} pkFilterParamsStruct;

/* parameters struct for EQ definition */
typedef struct {
  int nPkFilter; /* number of cascaded peak filters */
  float G; /* global gain [dB] */
  pkFilterParamsStruct* pkFilterParams; /* array of peak filters of size nPkFilter */
} eqParamsStruct;

/* parameters struct for the entire EQ configuration */
typedef struct {
  int numEQs; /* number of EQs */
  eqParamsStruct* eqParams; /* array of EQs of size numEQs */
  int* eqMap; /* index map for the EQs of size inputCount */
} eqConfigStruct;


extern const float eqPrecisions[4];
extern const float eqMinRanges[2][4];
extern const float eqMaxRanges[2][4];


#endif /* __DMX_MATRIX_COMMON_H__ */
