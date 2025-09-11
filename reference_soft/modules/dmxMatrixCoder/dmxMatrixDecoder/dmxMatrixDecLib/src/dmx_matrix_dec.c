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

#include "dmx_matrix_dec.h"


static int integer_log2(unsigned int n)
{
  int ilog2 = 0;

  while (n > 1) {
    n >>= 1;
    ++ilog2;
  }

  return ilog2;
}

static unsigned int ReadRange(robitbufHandle hBitstream, unsigned int alphabetSize)
{
  int nBits = integer_log2(alphabetSize);
  unsigned int nUnused = (1U << (nBits + 1)) - alphabetSize;
  unsigned int value = robitbuf_ReadBits(hBitstream, nBits);

  if (value >= nUnused) {
    value = (value << 1) - nUnused + robitbuf_ReadBits(hBitstream, 1);
  }

  return value;
}


static int ReadEscapedValue(robitbufHandle hBitStream, int count1, int count2, int count3)
{
  unsigned int value = 0;

  unsigned int value1 = robitbuf_ReadBits(hBitStream, count1);
  unsigned int escape1 = (1U << count1) - 1;

  value = value1;
  if (value1 == escape1) {
    unsigned int value2 = robitbuf_ReadBits(hBitStream, count2);
    unsigned int escape2 = (1U << count2) - 1;

    value += value2;
    if (value2 == escape2) {
      unsigned int value3 = robitbuf_ReadBits(hBitStream, count3);

      value += value3;
    }
  }

  return value;
}


static void DecodeFlatCompactMatrix(robitbufHandle hBitStream, signed char* flatCompactMatrix, int totalCount)
{
  int maxHead;
  int nBits = 3;
  int runLGRParam = -1;
  int idx, run;

  if (totalCount >= 256) nBits = 4;
  runLGRParam = robitbuf_ReadBits(hBitStream, nBits);
  maxHead = totalCount >> runLGRParam;
  idx = 0;
  do {
    int head = 0;
    int t;

    while (head < maxHead) {
      if (0 != robitbuf_ReadBits(hBitStream, 1)) break;
      ++head;
    }
    run = (head << runLGRParam) + robitbuf_ReadBits(hBitStream, runLGRParam);
    for (t = 0; t < run; ++t) {
      flatCompactMatrix[idx++] = 0;
    }
    if (idx < totalCount) flatCompactMatrix[idx++] = 1;
  } while (idx < totalCount);
}


static float DecodeGainValue(robitbufHandle hBitStream, CoderState* cs)
{
  float gainValue = DMX_MATRIX_GAIN_ZERO;

  if (cs->rawCodingNonzeros) {
    int nAlphabet = ((cs->maxGain - cs->minGain) << cs->precisionLevel) + 2;
    int gainValueIndex = ReadRange(hBitStream, nAlphabet);

    gainValue = cs->maxGain - gainValueIndex / (float) (1 << cs->precisionLevel);
  } else {
    int gainValueIndex = -1;
    int maxHead = (cs->gainTableSize - 1) >> cs->gainLGRParam;
    int head = 0;

    while (head < maxHead) {
      if (0 != robitbuf_ReadBits(hBitStream, 1)) break;
      ++head;
    }
    gainValueIndex = (head << cs->gainLGRParam) + robitbuf_ReadBits(hBitStream, cs->gainLGRParam);
    gainValue = cs->gainTable[gainValueIndex];
  }

  if (gainValue < cs->minGain) gainValue = DMX_MATRIX_GAIN_ZERO;
  return gainValue;
}


static int DecodeEqConfig(robitbufHandle hBitStream,
                   int inputCount,
                   eqConfigStruct* eqConfig)
{
  int i, j, k;
  int eqPrecisionLevel = 0;
  int sgPrecisionLevel = 0;
  int eqExtendedRange = 0;

  eqConfig->numEQs = ReadEscapedValue(hBitStream, 3, 5, 0) + 1;
  eqConfig->eqParams = (eqParamsStruct*) calloc(eqConfig->numEQs, sizeof(eqParamsStruct));
  assert(eqConfig->eqParams != NULL);

  eqPrecisionLevel = robitbuf_ReadBits(hBitStream, 2);
  sgPrecisionLevel = min(eqPrecisionLevel + 1, 3);
  eqExtendedRange = robitbuf_ReadBits(hBitStream, 1);

  for (i = 0; i < eqConfig->numEQs; ++i) {
    int lastCenterFreqP10 = 0;
    int lastCenterFreqLd2 = 10;
    int maxCenterFreqLd2 = 99;
    float scalingGain;
    int scalingGainIndex;

    eqConfig->eqParams[i].nPkFilter = ReadEscapedValue(hBitStream, 2, 4, 0) + 1;
    eqConfig->eqParams[i].pkFilterParams =
        (pkFilterParamsStruct*) calloc(eqConfig->eqParams[i].nPkFilter, sizeof(pkFilterParamsStruct));
    assert(eqConfig->eqParams[i].pkFilterParams != NULL);

    for (j = 0; j < eqConfig->eqParams[i].nPkFilter; ++j) {
      int centerFreq;
      int centerFreqP10;
      int centerFreqLd2;
      float qualityFactor;
      int qFactorIndex;
      float centerGain;
      int centerGainIndex;

      /* decode centerFreq */
      centerFreqP10 = ReadRange(hBitStream, 4 - lastCenterFreqP10) + lastCenterFreqP10;
      if (centerFreqP10 > lastCenterFreqP10) lastCenterFreqLd2 = 10;
      if (centerFreqP10 == 3) maxCenterFreqLd2 = 24;
      centerFreqLd2 = ReadRange(hBitStream, 1 + maxCenterFreqLd2 - lastCenterFreqLd2) + lastCenterFreqLd2;
      centerFreq = centerFreqLd2;
      for (k = 0; k < centerFreqP10; ++k) centerFreq *= 10;
      lastCenterFreqP10 = centerFreqP10;
      lastCenterFreqLd2 = centerFreqLd2;
      eqConfig->eqParams[i].pkFilterParams[j].f = (float) centerFreq;

      /* decode qualityFactor */
      qFactorIndex = robitbuf_ReadBits(hBitStream, 5);
      if (qFactorIndex <= 19) {
        qualityFactor = 0.05f * (qFactorIndex + 1);
      } else {
        int qFactorExtra = robitbuf_ReadBits(hBitStream, 3);
        qualityFactor = 1.0f + 0.1f * ((qFactorIndex - 20) * 8 + qFactorExtra + 1);
      }
      eqConfig->eqParams[i].pkFilterParams[j].q = qualityFactor;

      /* decode centerGain */
      centerGainIndex = robitbuf_ReadBits(hBitStream, 4 + eqExtendedRange + eqPrecisionLevel);
      centerGain = eqMinRanges[eqExtendedRange][eqPrecisionLevel] +
          eqPrecisions[eqPrecisionLevel] * centerGainIndex;
      eqConfig->eqParams[i].pkFilterParams[j].g = centerGain;
    }

    /* decode scalingGain */
    scalingGainIndex = robitbuf_ReadBits(hBitStream, 4 + eqExtendedRange + sgPrecisionLevel);
    scalingGain = eqMinRanges[eqExtendedRange][sgPrecisionLevel] +
        eqPrecisions[sgPrecisionLevel] * scalingGainIndex;
    eqConfig->eqParams[i].G = scalingGain;
  }

  eqConfig->eqMap = (int*) calloc(inputCount, sizeof(int));
  assert(eqConfig->eqMap != NULL);
  for (i = 0; i < inputCount; ++i) {
    int hasEqualizer = robitbuf_ReadBits(hBitStream, 1);
    if (hasEqualizer) {
      eqConfig->eqMap[i] = ReadRange(hBitStream, eqConfig->numEQs) + 1;
    } else {
      eqConfig->eqMap[i] = 0;
    }
  }

  return 0;
}


int DecodeDownmixMatrix(int inputIndex,
                    int inputCount,
                    SpeakerInformation* inputConfig,
                    int outputIndex,
                    int outputCount,
                    SpeakerInformation* outputConfig,
                    robitbufHandle hBitStream,
                    float* downmixMatrix,
                    eqConfigStruct* eqConfig)
{
  int i, j;

  SpeakerInformation* compactInputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int compactInputCount = 0;
  SpeakerInformation* compactOutputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int compactOutputCount = 0;
  int precisionLevel = 0;
  int isSeparable[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int isSymmetric[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int isAllSeparable = 1;
  int isAllSymmetric = 1;
  signed char compactDownmixMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT][DMX_MATRIX_MAX_SPEAKER_COUNT];

  int useCompactTemplate = 1;
  int fullForAsymmetricInputs = 1;
  int mixLFEOnlyToLFE = 1;
  int rawCodingCompactMatrix = 0;
  int rawCodingNonzeros = 0;

  CoderState cs;
  int minGain = 96;
  int maxGain = -96;
  signed char* compactTemplate = NULL; /* compactTemplate[compactInputCount][compactOutputCount] stored in the C array layout */
  int equalizerPresent = 0;

  eqConfig->numEQs = 0; eqConfig->eqParams = NULL; eqConfig->eqMap = NULL;
  equalizerPresent = robitbuf_ReadBits(hBitStream, 1);
  if (equalizerPresent) {
    int error = DecodeEqConfig(hBitStream, inputCount, eqConfig); if (error) return -1;
  }

  precisionLevel = robitbuf_ReadBits(hBitStream, 2);
  maxGain = ReadEscapedValue(hBitStream, 3, 4, 0);
  minGain = -(ReadEscapedValue(hBitStream, 4, 5, 0) + 1);

  ConvertToCompactConfig(inputCount, inputConfig, &compactInputCount, compactInputConfig);
  ConvertToCompactConfig(outputCount, outputConfig, &compactOutputCount, compactOutputConfig);


  isAllSeparable = robitbuf_ReadBits(hBitStream, 1);
  if (!isAllSeparable) {
    for (j = 0; j < compactOutputCount; ++j) {
      if (compactOutputConfig[j]->pairType == SP_PAIR_SYMMETRIC) {
        isSeparable[j] = robitbuf_ReadBits(hBitStream, 1);
      }
    }
  } else {
    for (j = 0; j < compactOutputCount; ++j) {
      if (compactOutputConfig[j]->pairType == SP_PAIR_SYMMETRIC) {
        isSeparable[j] = 1;
      }
    }
  }

  isAllSymmetric = robitbuf_ReadBits(hBitStream, 1);
  if (!isAllSymmetric) {
    for (j = 0; j < compactOutputCount; ++j) {
      isSymmetric[j] = robitbuf_ReadBits(hBitStream, 1);
    }
  } else {
    for (j = 0; j < compactOutputCount; ++j) {
      isSymmetric[j] = 1;
    }
  }


  /* decode the compact template */
  mixLFEOnlyToLFE = robitbuf_ReadBits(hBitStream, 1);
  rawCodingCompactMatrix = robitbuf_ReadBits(hBitStream, 1);
  if (rawCodingCompactMatrix) {
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (!mixLFEOnlyToLFE || (compactInputConfig[i]->isLFE == compactOutputConfig[j]->isLFE)) {
          compactDownmixMatrix[i][j] = (signed char) robitbuf_ReadBits(hBitStream, 1);
        } else {
          compactDownmixMatrix[i][j] = 0;
        }
      }
    }
  } else {
    signed char flatCompactMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT];
    int count = 0;
    int totalCount = 0;

    useCompactTemplate = robitbuf_ReadBits(hBitStream, 1);

    /* compute totalCount as described in the ComputeTotalCount() function */
    if (mixLFEOnlyToLFE) {
      int compactInputLFECount = 0;
      int compactOutputLFECount = 0;

      for (i = 0; i < compactInputCount; ++i) {
        if (compactInputConfig[i]->isLFE) compactInputLFECount++;
      }
      for (i = 0; i < compactOutputCount; ++i) {
        if (compactOutputConfig[i]->isLFE) compactOutputLFECount++;
      }
      totalCount = (compactInputCount - compactInputLFECount) * (compactOutputCount - compactOutputLFECount);
    } else {
      totalCount = compactInputCount * compactOutputCount;
    }

    if (useCompactTemplate) {
      /* a compact template should be used only for CICP speaker layouts */
      if ((inputIndex == -1) || (outputIndex == -1)) {
        /* when useCompactTemplate is enabled and one of the CICP indexes are -1, it means
          that the decoder version is too old and it does not have the required CICP tables */
        return -1;
      }

      compactTemplate = FindCompactTemplate(inputIndex, outputIndex);
      if (compactTemplate == NULL) { /* a template was not found */
        /* when useCompactTemplate is enabled and a template was not found, it means
          that the decoder version is too old and it does not have the required tables */
        return -1;
      }
    }

    DecodeFlatCompactMatrix(hBitStream, flatCompactMatrix, totalCount);
    count = 0;
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (mixLFEOnlyToLFE && compactInputConfig[i]->isLFE && compactOutputConfig[j]->isLFE) {
          compactDownmixMatrix[i][j] = (signed char) robitbuf_ReadBits(hBitStream, 1);
        } else if (mixLFEOnlyToLFE && (compactInputConfig[i]->isLFE ^ compactOutputConfig[j]->isLFE)) {
          compactDownmixMatrix[i][j] = 0;
        } else { /* !mixLFEOnlyToLFE || (!compactInputConfig[i]->isLFE && !compactOutputConfig[j]->isLFE) */
          compactDownmixMatrix[i][j] = flatCompactMatrix[count++];
          if (useCompactTemplate) {
            compactDownmixMatrix[i][j] ^= compactTemplate[i * compactOutputCount + j];
          }
        }
      }
    }
    assert(count == totalCount);
  }


  /* decode values for the nonzero entries in compactDownmixMatrix */
  fullForAsymmetricInputs = robitbuf_ReadBits(hBitStream, 1);
  rawCodingNonzeros = robitbuf_ReadBits(hBitStream, 1);

  CoderStateInit(&cs);
  cs.historyCount = 0;
  cs.rawCodingNonzeros = rawCodingNonzeros;
  cs.gainLGRParam = -1;
  cs.minGain = minGain;
  cs.maxGain = maxGain;
  cs.precisionLevel = precisionLevel;
  cs.gainTableSize = 0;

  if (!rawCodingNonzeros) {
    cs.gainLGRParam = robitbuf_ReadBits(hBitStream, 3);
    CoderStateGenerateGainTable(&cs);
  }

  for (i = 0; i < compactInputCount; ++i) {
    int iType = compactInputConfig[i]->pairType;
    assert(iType != SP_PAIR_NONE);

    for (j = 0; j < compactOutputCount; ++j) {
      int oType = compactOutputConfig[j]->pairType;
      int i1 = compactInputConfig[i]->originalPosition;
      int o1 = compactOutputConfig[j]->originalPosition;
      assert(oType != SP_PAIR_NONE);

      if ((iType != SP_PAIR_SYMMETRIC) && (oType != SP_PAIR_SYMMETRIC)) {
        downmixMatrix[i1 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        downmixMatrix[i1 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
      } else if (iType != SP_PAIR_SYMMETRIC) {
        int o2 = compactOutputConfig[j]->symmetricPair->originalPosition;
        int useFull = (iType == SP_PAIR_SINGLE) && fullForAsymmetricInputs;
        downmixMatrix[i1 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        downmixMatrix[i1 * outputCount + o2] = DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        downmixMatrix[i1 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
        if (isSymmetric[j] && !useFull) {
          downmixMatrix[i1 * outputCount + o2] = downmixMatrix[i1 * outputCount + o1];
        } else {
          downmixMatrix[i1 * outputCount + o2] = DecodeGainValue(hBitStream, &cs);
        }
      } else if (oType != SP_PAIR_SYMMETRIC) {
        int i2 = compactInputConfig[i]->symmetricPair->originalPosition;
        downmixMatrix[i1 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        downmixMatrix[i2 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        downmixMatrix[i1 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
        if (isSymmetric[j]) {
          downmixMatrix[i2 * outputCount + o1] = downmixMatrix[i1 * outputCount + o1];
        } else {
          downmixMatrix[i2 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
        }
      } else {
        int i2 = compactInputConfig[i]->symmetricPair->originalPosition;
        int o2 = compactOutputConfig[j]->symmetricPair->originalPosition;
        downmixMatrix[i1 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        downmixMatrix[i1 * outputCount + o2] = DMX_MATRIX_GAIN_ZERO;
        downmixMatrix[i2 * outputCount + o1] = DMX_MATRIX_GAIN_ZERO;
        downmixMatrix[i2 * outputCount + o2] = DMX_MATRIX_GAIN_ZERO;
        if (!compactDownmixMatrix[i][j]) continue;

        downmixMatrix[i1 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
        if (isSeparable[j] && isSymmetric[j]) {
          downmixMatrix[i2 * outputCount + o2] = downmixMatrix[i1 * outputCount + o1];
        } else if (!isSeparable[j] && isSymmetric[j]) {
          downmixMatrix[i1 * outputCount + o2] = DecodeGainValue(hBitStream, &cs);
          downmixMatrix[i2 * outputCount + o1] = downmixMatrix[i1 * outputCount + o2];
          downmixMatrix[i2 * outputCount + o2] = downmixMatrix[i1 * outputCount + o1];
        } else if (isSeparable[j] && !isSymmetric[j]) {
          downmixMatrix[i2 * outputCount + o2] = DecodeGainValue(hBitStream, &cs);
        } else { /* !isSeparable[j] && !isSymmetric[j] */
          downmixMatrix[i1 * outputCount + o2] = DecodeGainValue(hBitStream, &cs);
          downmixMatrix[i2 * outputCount + o1] = DecodeGainValue(hBitStream, &cs);
          downmixMatrix[i2 * outputCount + o2] = DecodeGainValue(hBitStream, &cs);
        }
      }
    }
  }

  /* convert the downmix matrix to linear domain */
  assert(precisionLevel <= 2);
  for (i = 0; i < inputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      float value = downmixMatrix[i * outputCount + j];
      if (value != DMX_MATRIX_GAIN_ZERO) {
        float gain_linear = (float) pow(10.0f, value / 20.0f);
        downmixMatrix[i * outputCount + j] = gain_linear;
      } else {
        downmixMatrix[i * outputCount + j] = 0.0f;
      }
    }
  }

  return 0;
}
