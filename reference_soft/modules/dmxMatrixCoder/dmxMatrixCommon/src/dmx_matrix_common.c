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

#include "dmx_matrix_common.h"


/* 22_2 to 5_1 */
const signed char compactTemplate_CICP13_to_CICP6[15 * 4] = {
   1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
   1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1,
   0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0
};

/* 5_2_1 to 5_1 */
const signed char compactTemplate_CICP14_to_CICP6[5 * 4] = {
   1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0
};

/* 7_1 to 5_1 */
const signed char compactTemplate_CICP12_to_CICP6[5 * 4] = {
   1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1
};

/* 7_1_ALT to 5_1 */
const signed char compactTemplate_CICP7_to_CICP6[5 * 4] = {
   1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1
};

/* 22_2 to 5_2_1 */
const signed char compactTemplate_CICP13_to_CICP14[15 * 5] = {
   1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0,
   0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
   0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0,
   0, 0, 0
};

/* 22_2 to 7_1 */
const signed char compactTemplate_CICP13_to_CICP12[15 * 5] = {
   1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
   0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0,
   1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0,
   0, 0, 0
};

/* 22_2 to 7_1_ALT */
const signed char compactTemplate_CICP13_to_CICP7[15 * 5] = {
   0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0,
   0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0,
   1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0,
   0, 0, 0
};

/* 22_2 to 2_0 */
const signed char compactTemplate_CICP13_to_CICP2[15 * 1] = {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const short compactTemplates_InputIndex[] = {
 13, 13, 13, 13, 14, 12, 7, 13
};

const short compactTemplates_OutputIndex[] = {
  6, 14, 12, 7, 6, 6, 6, 2
};

const signed char* compactTemplates_Data[] = {
  (const signed char*) &compactTemplate_CICP13_to_CICP6, (const signed char*) &compactTemplate_CICP13_to_CICP14,
  (const signed char*) &compactTemplate_CICP13_to_CICP12, (const signed char*) &compactTemplate_CICP13_to_CICP7,
  (const signed char*) &compactTemplate_CICP14_to_CICP6, (const signed char*) &compactTemplate_CICP12_to_CICP6,
  (const signed char*) &compactTemplate_CICP7_to_CICP6, (const signed char*) &compactTemplate_CICP13_to_CICP2
};


void CoderStateInit(CoderState* cs)
{
  cs->minGain = -1;
  cs->maxGain = 0;
  cs->precisionLevel = 0;
  cs->rawCodingNonzeros = 1;
  cs->gainLGRParam = -1;
  cs->historyCount = 0;
  cs->gainTableSize = 0;
}


void CoderStateGenerateGainTable(CoderState* cs)
{
  int i, p;
  int index = 0;
  double f;
  float last_step, step;

  assert(cs->precisionLevel <= 2); /* 2 fractional bits give 0.25 dB resolution */

  /* add multiples of 3 to the table, first values <= 0 and then values > 0 */
  for (i = 0; i >= cs->minGain; i -= 3) {
    cs->gainTable[index++] = (float) i;
  }
  for (i = 3; i <= cs->maxGain; i += 3) {
    cs->gainTable[index++] = (float) i;
  }

  /* add the rest of the integers which are not multiples of 3 to the table */
  for (i = 0; i >= cs->minGain; --i) {
    if ((i % 3) != 0) cs->gainTable[index++] = (float) i;
  }

  for (i = 1; i <= cs->maxGain; ++i) {
    if ((i % 3) != 0) cs->gainTable[index++] = (float) i;
  }

  /* add values which are multiples of 0.5, then multiples of 0.25,
     up to and finally multiples of 2 ^ (-precisionLevel) to the table,
     but do not add values which are already in the table */
  last_step = 1.0f;
  for (p = 1; p <= cs->precisionLevel; ++p) {
    step = 1.0f / (float) (1 << p);

    for (f = 0; f >= cs->minGain; f -= step) {
      if (fmod(f, last_step) != 0.0f) cs->gainTable[index++] = (float) f;
    }
    for (f = step; f <= cs->maxGain; f += step) {
      if (fmod(f, last_step) != 0.0f) cs->gainTable[index++] = (float) f;
    }

    last_step = step;
  }

  cs->gainTable[index++] = DMX_MATRIX_GAIN_ZERO;
  cs->gainTableSize = index;
  assert(cs->gainTableSize == ((cs->maxGain - cs->minGain) << cs->precisionLevel) + 2);
}


void ConvertToCompactConfig(int inputCount,
                    SpeakerInformation* inputConfig,
                    int* compactInputCount,
                    SpeakerInformation* compactInputConfig[])
{
  int i, j;

  for (i = 0; i < inputCount; ++i) {
    inputConfig[i].originalPosition = (short) i;
    inputConfig[i].isAlreadyUsed = 0;
    inputConfig[i].symmetricPair = NULL;
  }

  *compactInputCount = 0;
  for (i = 0; i < inputCount; ++i) {
    if (inputConfig[i].isAlreadyUsed) continue;

    if ((inputConfig[i].azimuth == 0) || (abs(inputConfig[i].azimuth) == 180)) {
      /* the speaker is in the center, it has no pair */
      compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
      inputConfig[i].symmetricPair = NULL;
      inputConfig[i].pairType = SP_PAIR_CENTER;

      inputConfig[i].isAlreadyUsed = 1;
    } else {
      for (j = i + 1; j < inputCount; ++j) {
        if (inputConfig[j].isAlreadyUsed) continue;

        if ((inputConfig[i].isLFE == inputConfig[j].isLFE) &&
            (inputConfig[i].elevation == inputConfig[j].elevation) &&
            (inputConfig[i].azimuth == -inputConfig[j].azimuth)) {

          /* we found a symmetric L/R pair for speaker on position i */
          if (inputConfig[i].azimuth > 0) {
            compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
            inputConfig[i].symmetricPair = &(inputConfig[j]);
            inputConfig[i].pairType = SP_PAIR_SYMMETRIC;
            inputConfig[j].symmetricPair = NULL;
            inputConfig[j].pairType = SP_PAIR_NONE;
          } else {
            compactInputConfig[(*compactInputCount)++] = &(inputConfig[j]);
            inputConfig[j].symmetricPair = &(inputConfig[i]);
            inputConfig[j].pairType = SP_PAIR_SYMMETRIC;
            inputConfig[i].symmetricPair = NULL;
            inputConfig[i].pairType = SP_PAIR_NONE;
          }

          inputConfig[i].isAlreadyUsed = 1;
          inputConfig[j].isAlreadyUsed = 1;
          break;
        }
      }

      if (!inputConfig[i].isAlreadyUsed) {
        /* we did not found a symmetric L/R pair for speaker on position i */
        compactInputConfig[(*compactInputCount)++] = &(inputConfig[i]);
        inputConfig[i].symmetricPair = NULL;
        inputConfig[i].pairType = SP_PAIR_SINGLE;

        inputConfig[i].isAlreadyUsed = 1;
      }
    }
  }
}


signed char* FindCompactTemplate(int inputIndex,
                                 int outputIndex)
{
  int i;

  /* a compact template should be used only for CICP speaker layouts */
  assert((inputIndex != -1) && (outputIndex != -1));

  for (i = 0; i < (int) array_length(compactTemplates_Data); ++i) {
    if (compactTemplates_InputIndex[i] != inputIndex) continue;
    if (compactTemplates_OutputIndex[i] != outputIndex) continue;

    /* a matching template was found */
    return (signed char*) compactTemplates_Data[i];
  }

  /* a matching template was not found */
  return NULL;
}


const float eqPrecisions[4] = {
  1.0f, 0.5f, 0.25f, 0.1f
};

const float eqMinRanges[2][4] = {
  {-8.0f, -8.0f, -8.0f, -6.4f},
  {-16.0f, -16.0f, -16.0f, -12.8f}
};

const float eqMaxRanges[2][4] = {
  {7.0f, 7.5f, 7.75f, 6.3f},
  {15.0f, 15.5f, 15.75f, 12.7f}
};
