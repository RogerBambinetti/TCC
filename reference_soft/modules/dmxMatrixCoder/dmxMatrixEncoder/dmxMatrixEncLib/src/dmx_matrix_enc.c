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

#include "dmx_matrix_enc.h"


/* #define DMXMATRIX_ENCODER_GENERATE_COMPACTTEMPLATE */
/* enable for generating a compact template on stdout when calling EncodeDownmixMatrix */


static int integer_log2(unsigned int n)
{
  int ilog2 = 0;

  while (n > 1) {
    n >>= 1;
    ++ilog2;
  }

  return ilog2;
}

static int WriteRange(wobitbufHandle hBitstream, unsigned int value, unsigned int alphabetSize)
{
  int error = 0;

  int nBits = integer_log2(alphabetSize);
  unsigned int nUnused = (1U << (nBits + 1)) - alphabetSize;
  if (value < nUnused) {
    error = wobitbuf_WriteBits(hBitstream, value, nBits); if (error) return -1;
  } else {
    value += nUnused;
    error = wobitbuf_WriteBits(hBitstream, value >> 1, nBits); if (error) return -1;
    error = wobitbuf_WriteBits(hBitstream, value & 1, 1); if (error) return -1;
  }

  return error;
}


static int WriteEscapedValue(wobitbufHandle hBitStream, unsigned int value, int count1, int count2, int count3)
{
  int error = 0;

  if (value < (1U << count1) - 1) {
    error = wobitbuf_WriteBits(hBitStream, value, count1); if (error) return -1;
  } else {
    unsigned int escape1 = (1U << count1) - 1;
    error = wobitbuf_WriteBits(hBitStream, escape1, count1); if (error) return -1;
    value -= escape1;
    if (value < (1U << count2) - 1) {
      error = wobitbuf_WriteBits(hBitStream, value, count2); if (error) return -1;
    } else {
      unsigned int escape2 = (1U << count2) - 1;
      error = wobitbuf_WriteBits(hBitStream, escape2, count2); if (error) return -1;
      value -= escape2;
      assert(value < (1U << count3));
      error = wobitbuf_WriteBits(hBitStream, value, count3); if (error) return -1;
    }
  }

  return error;
}


static int CoderStateComputeBestParam(CoderState* cs)
{
  int testParam;
  int testLength;
  int bestParam = -1;
  int bestLength = 1 << 30;
  int i, k;

  /* find the best limited Golomb-Rice parameter */
  for (testParam = 0; testParam < (1 << 3); ++testParam) {
    int maxHead = (cs->gainTableSize - 1) >> testParam;
    testLength = 0;

    for (i = 0; i < cs->historyCount; ++i) {
      int gainIndex = -1;
      float gain_dB = cs->history[i];

      for (k = 0; k < cs->gainTableSize; ++k) {
        if (gain_dB == cs->gainTable[k]) {
          gainIndex = k;
          break;
        }
      }
      assert(gainIndex != -1);

      /* add the limited Golomb-Rice code length */
      testLength += min((gainIndex >> testParam) + 1, maxHead) + testParam;
    }

    if (testLength < bestLength) {
      bestParam = testParam;
      bestLength = testLength;
    }
  }

  cs->gainLGRParam = bestParam;

  return bestLength;
}


static int EncodeGainValue(wobitbufHandle hBitStream, CoderState* cs, float gain_dB)
{
  int error = 0;

  if ((cs->gainLGRParam == -1) && !cs->rawCodingNonzeros) {
    /* only collect the gains in order to determine gainLGRparam */
    cs->history[cs->historyCount++] = gain_dB;

    return 0;
  }

  if (cs->rawCodingNonzeros) {
    int numValues = ((cs->maxGain - cs->minGain) << cs->precisionLevel) + 2;
    int gainIndex;

    if (gain_dB == DMX_MATRIX_GAIN_ZERO) {
      gainIndex = numValues - 1; /* the largest gain index indicates DMX_MATRIX_GAIN_ZERO */
    } else {
        gainIndex = (int)((cs->maxGain - gain_dB) * (float) (1 << cs->precisionLevel));
    }

    error = WriteRange(hBitStream, gainIndex, numValues); if (error) return -1;
  } else {
    int gainIndex = -1;
    int maxHead = (cs->gainTableSize - 1) >> cs->gainLGRParam;
    int head, t, i;

    for (i = 0; i < cs->gainTableSize; ++i) {
      if (gain_dB == cs->gainTable[i]) {
        gainIndex = i;
        break;
      }
    }
    assert(gainIndex != -1);

    head = gainIndex >> cs->gainLGRParam;
    /* write head zero bits */
    for (t = 0; t < head; ++t) {
      error = wobitbuf_WriteBits(hBitStream, 0, 1); if (error) return -1;
    }

    if (head < maxHead) {
      /* write the terminating one bit, if needed */
      error = wobitbuf_WriteBits(hBitStream, 1, 1); if (error) return -1;
    }
    /* write the tail bits */
    error = wobitbuf_WriteBits(hBitStream, gainIndex & ((1 << cs->gainLGRParam) - 1), cs->gainLGRParam);
    if (error) return -1;
  }

  return 0;
}


static int EncodeFlatCompactMatrix(wobitbufHandle hBitStream, signed char* flatCompactMatrix, int totalCount)
{
  int error = 0;
  int maxHead;
  int n = 3;
  int testParam;
  int testLength;
  int bestParam = -1;
  int bestLength = 1 << 30;
  int idx, run;
  int bitCount = wobitbuf_GetBitsWritten(hBitStream);

  if (flatCompactMatrix[totalCount - 1] == 0) {
    /* ensure the last run of zeros ends with an one */
    flatCompactMatrix[totalCount] = 1;
  }
  if (totalCount >= 256) n = 4;

  /* find the best Limited Golomb-Rice parameter */
  for (testParam = 0; testParam < (1 << n); ++testParam) {
    maxHead = totalCount >> testParam;
    idx = 0;
    testLength = n;
    do {
      run = 0;
      while (flatCompactMatrix[idx++] == 0) {
        ++run;
      }
      /* add the limited Golomb-Rice code length */
      testLength += min((run >> testParam) + 1, maxHead) + testParam;
    } while (idx < totalCount);

    if (testLength < bestLength) {
      bestParam = testParam;
      bestLength = testLength;
    }
  }

  /* do the actual encoding */
  error = wobitbuf_WriteBits(hBitStream, bestParam, n); if (error) return -1; /* runLGRParam */
  maxHead = totalCount >> bestParam;
  idx = 0;
  do {
    int head;
    int t;

    run = 0;
    while (flatCompactMatrix[idx++] == 0) {
      ++run;
    }
    head = run >> bestParam;
    /* write head zero bits */
    for (t = 0; t < head; ++t) {
      error = wobitbuf_WriteBits(hBitStream, 0, 1); if (error) return -1;
    }
    if (head < maxHead) {
      /* write the terminating one bit, if needed */
      error = wobitbuf_WriteBits(hBitStream, 1, 1); if (error) return -1;
    }
    /* write the tail bits */
    error = wobitbuf_WriteBits(hBitStream, run & ((1 << bestParam) - 1), bestParam);
    if (error) return -1;
  } while (idx < totalCount);

  bitCount = wobitbuf_GetBitsWritten(hBitStream) - bitCount;
  assert(bitCount == bestLength);

  return 0;
}


static int EncodeEqConfig(wobitbufHandle hBitStream,
                   int inputCount,
                   int eqPrecisionLevel,
                   eqConfigStruct* eqConfig)
{
  int error = 0;
  int i, j, k;
  int eqExtendedRange = 0;
  int sgPrecisionLevel = 0;

  error = WriteEscapedValue(hBitStream, eqConfig->numEQs - 1, 3, 5, 0); if (error) return -1;
  error = wobitbuf_WriteBits(hBitStream, eqPrecisionLevel, 2); if (error) return -1;

  /* determine if an extended value range is needed */
  sgPrecisionLevel = min(eqPrecisionLevel + 1, 3);
  for (i = 0; i < eqConfig->numEQs; ++i) {
    if ((eqConfig->eqParams[i].G < eqMinRanges[0][sgPrecisionLevel]) ||
        (eqConfig->eqParams[i].G > eqMaxRanges[0][sgPrecisionLevel])) {
      eqExtendedRange = 1;
      break;
    }
    for (j = 0; j < eqConfig->eqParams[i].nPkFilter; ++j) {
      if ((eqConfig->eqParams[i].pkFilterParams[j].g < eqMinRanges[0][eqPrecisionLevel]) ||
          (eqConfig->eqParams[i].pkFilterParams[j].g > eqMaxRanges[0][eqPrecisionLevel])) {
        eqExtendedRange = 1;
        break;
      }
    }
  }
  error = wobitbuf_WriteBits(hBitStream, eqExtendedRange, 1); if (error) return -1;

  for (i = 0; i < eqConfig->numEQs; ++i) {
    int lastCenterFreqP10 = 0;
    int lastCenterFreqLd2 = 10;
    int maxCenterFreqLd2 = 99;
    float scalingGain;
    int scalingGainIndex;

    error = WriteEscapedValue(hBitStream, eqConfig->eqParams[i].nPkFilter - 1, 2, 4, 0); if (error) return -1;

    /* sort the entries of vector eqConfig->eqParams[i].pkFilterParams according to struct member f */
    for (j = 0; j < eqConfig->eqParams[i].nPkFilter - 1; ++j) {
      pkFilterParamsStruct* param_j = &(eqConfig->eqParams[i].pkFilterParams[j]);
      for (k = j + 1; k < eqConfig->eqParams[i].nPkFilter; ++k) {
        pkFilterParamsStruct* param_k = &(eqConfig->eqParams[i].pkFilterParams[k]);
        if (param_j->f > param_k->f) {
          float tmp;
          /* swap the values of param_j and param_k */
          tmp = param_j->f; param_j->f = param_k->f; param_k->f = tmp;
          tmp = param_j->q; param_j->q = param_k->q; param_k->q = tmp;
          tmp = param_j->g; param_j->g = param_k->g; param_k->g = tmp;
        }
      }
    }

    for (j = 0; j < eqConfig->eqParams[i].nPkFilter; ++j) {
      int centerFreq;
      int centerFreqP10;
      int centerFreqLd2;
      float qualityFactor;
      int qFactorIndex;
      int qFactorExtra;
      float centerGain;
      int centerGainIndex;

      /* encode centerFreq */
      centerFreq = (int) (eqConfig->eqParams[i].pkFilterParams[j].f + 0.5f);
      if ((centerFreq < 10) || (centerFreq > 24000)) {
        fprintf(stderr, "Warning: centerFreq %d outside range, value clipped\n", centerFreq);
        centerFreq = max(10, min(centerFreq, 24000));
      }
      /* convert centerFreq to Ld2 and P10 representation */
      if (centerFreq < 100) {
        centerFreqLd2 = centerFreq;
        centerFreqP10 = 0;
      } else if (centerFreq < 995) {
        centerFreqLd2 = (centerFreq + 5) / 10;
        centerFreqP10 = 1;
      } else if (centerFreq < 9950) {
        centerFreqLd2 = (centerFreq + 50) / 100;
        centerFreqP10 = 2;
      } else {
        centerFreqLd2 = (centerFreq + 500) / 1000;
        centerFreqLd2 = min(centerFreqLd2, 24);
        centerFreqP10 = 3;
      }
      assert(centerFreqP10 >= lastCenterFreqP10);
      error = WriteRange(hBitStream, centerFreqP10 - lastCenterFreqP10, 4 - lastCenterFreqP10); if (error) return -1;
      if (centerFreqP10 > lastCenterFreqP10) lastCenterFreqLd2 = 10;
      if (centerFreqP10 == 3) maxCenterFreqLd2 = 24;
      assert(centerFreqLd2 >= lastCenterFreqLd2);
      error = WriteRange(hBitStream, centerFreqLd2 - lastCenterFreqLd2, 1 + maxCenterFreqLd2 - lastCenterFreqLd2); if (error) return -1;
      lastCenterFreqP10 = centerFreqP10;
      lastCenterFreqLd2 = centerFreqLd2;

      /* encode qualityFactor */
      qualityFactor = eqConfig->eqParams[i].pkFilterParams[j].q;
      if ((qualityFactor < 0.05f) || (qualityFactor > 10.6f)) {
        fprintf(stderr, "Warning: qualityFactor %4.2f outside range, value clipped\n", qualityFactor);
        qualityFactor = max(0.05f, min(qualityFactor, 10.6f));
      }
      if (qualityFactor < 1.05f) {
        qFactorIndex = (int) ((qualityFactor - 0.05f) * 20.0f + 0.5f);
        qFactorIndex = min(qFactorIndex, 19); /* for the interval [1.025, 1.05) */
        error = wobitbuf_WriteBits(hBitStream, qFactorIndex, 5); if (error) return -1;
      } else {
        int qFactor = (int) ((qualityFactor - 1.0f) * 10.0f + 0.5f) - 1;
        qFactor = min(qFactor, (31 - 20) * 8 + 7);
        qFactorIndex = (qFactor >> 3) + 20;
        qFactorExtra = qFactor & 7;
        error = wobitbuf_WriteBits(hBitStream, qFactorIndex, 5); if (error) return -1;
        error = wobitbuf_WriteBits(hBitStream, qFactorExtra, 3); if (error) return -1;
      }

      /* encode centerGain */
      centerGain = eqConfig->eqParams[i].pkFilterParams[j].g;
      if ((centerGain < eqMinRanges[eqExtendedRange][eqPrecisionLevel]) ||
          (centerGain > eqMaxRanges[eqExtendedRange][eqPrecisionLevel])) {
        assert(eqExtendedRange != 0);
        fprintf(stderr, "Warning: centerGain %4.2f outside range, value clipped\n", centerGain);
        centerGain = max(eqMinRanges[eqExtendedRange][eqPrecisionLevel],
          min(centerGain, eqMaxRanges[eqExtendedRange][eqPrecisionLevel]));
      }
      centerGainIndex = (int)
          ((centerGain - eqMinRanges[eqExtendedRange][eqPrecisionLevel]) / eqPrecisions[eqPrecisionLevel] + 0.5f);
      assert(centerGainIndex < (1 << (4 + eqExtendedRange + eqPrecisionLevel)));
      error = wobitbuf_WriteBits(hBitStream, centerGainIndex, 4 + eqExtendedRange + eqPrecisionLevel); if (error) return -1;
    }

    /* encode scalingGain */
    scalingGain = eqConfig->eqParams[i].G;
    if ((scalingGain < eqMinRanges[eqExtendedRange][sgPrecisionLevel]) ||
        (scalingGain > eqMaxRanges[eqExtendedRange][sgPrecisionLevel])) {
        assert(eqExtendedRange != 0);
        fprintf(stderr, "Warning: scalingGain %4.2f outside range, value clipped\n", scalingGain);
        scalingGain = max(eqMinRanges[eqExtendedRange][sgPrecisionLevel],
          min(scalingGain, eqMaxRanges[eqExtendedRange][sgPrecisionLevel]));
    }
    scalingGainIndex = (int)
        ((scalingGain - eqMinRanges[eqExtendedRange][sgPrecisionLevel]) / eqPrecisions[sgPrecisionLevel] + 0.5f);
    assert(scalingGainIndex < (1 << (4 + eqExtendedRange + sgPrecisionLevel)));
    error = wobitbuf_WriteBits(hBitStream, scalingGainIndex, 4 + eqExtendedRange + sgPrecisionLevel); if (error) return -1;
  }

  for (i = 0; i < inputCount; ++i) {
    if (eqConfig->eqMap[i] != 0) {
      assert(eqConfig->eqMap[i] <= eqConfig->numEQs);
      error = wobitbuf_WriteBits(hBitStream, 1, 1); if (error) return -1;
      error = WriteRange(hBitStream, eqConfig->eqMap[i] - 1, eqConfig->numEQs); if (error) return -1;
    } else {
      error = wobitbuf_WriteBits(hBitStream, 0, 1); if (error) return -1;
    }
  }

  return error;
}


int EncodeDownmixMatrix(int inputIndex,
                    int inputCount,
                    SpeakerInformation* inputConfig,
                    int outputIndex,
                    int outputCount,
                    SpeakerInformation* outputConfig,
                    int precisionLevel,
                    wobitbufHandle hBitStream,
                    float* downmixMatrix,
                    int eqPrecisionLevel,
                    eqConfigStruct* eqConfig)
{
  int i, j;

  SpeakerInformation* compactInputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int compactInputCount = 0;
  SpeakerInformation* compactOutputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int compactOutputCount = 0;
  int isSeparable[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int isSymmetric[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int isAllSeparable = 1;
  int isAllSymmetric = 1;
  signed char compactDownmixMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT][DMX_MATRIX_MAX_SPEAKER_COUNT];

  int configFullForAsymmetricInputs = 0;
  int configMixLFEOnlyToLFE = 1;
  int configRawCodingCompactMatrix = 0;
  int configUseCompactTemplate = 1;
  int configRawCodingNonzeros = 0;

  int useCompactTemplate = configUseCompactTemplate;
  int mixLFEOnlyToLFE = configMixLFEOnlyToLFE;
  int rawCodingCompactMatrix = configRawCodingCompactMatrix;
  CoderState cs;
  int error = 0;
  int gainsBestLength = 0;
  int step;
  float multiplier = (float) (1 << precisionLevel);
  float invMultiplier = (float) (1.0f / multiplier);
  float minGain = 96.0f;
  float maxGain = -96.0f;
  signed char* compactTemplate = NULL; /* compactTemplate[compactInputCount][compactOutputCount] stored in the C array layout */
  signed char compactTemplateBuffer[DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT];

  int initialBitCount = wobitbuf_GetBitsWritten(hBitStream);
  int bitCountSave = 0;
  int bitCount = 0;

  /* encode the EQ config */
  if (eqConfig->numEQs != 0) {
    error = wobitbuf_WriteBits(hBitStream, 1, 1); if (error) return -1;
    error = EncodeEqConfig(hBitStream, inputCount, eqPrecisionLevel, eqConfig); if (error) return -1;
  } else {
    error = wobitbuf_WriteBits(hBitStream, 0, 1); if (error) return -1;
  }

  printf("\n");
  printf("configFullForAsymmetricInputs %1d\n", configFullForAsymmetricInputs);
  printf("configMixLFEOnlyToLFE %1d\n", configMixLFEOnlyToLFE);
  printf("configRawCodingCompactMatrix %1d\n", configRawCodingCompactMatrix);
  printf("configUseCompactTemplate %1d\n", configUseCompactTemplate);
  printf("configRawCodingNonzeros %1d\n", configRawCodingNonzeros);
  printf("\n");

  ConvertToCompactConfig(inputCount, inputConfig, &compactInputCount, compactInputConfig);
  ConvertToCompactConfig(outputCount, outputConfig, &compactOutputCount, compactOutputConfig);

  /* convert the downmix matrix to dB domain and quantize it to the requested precisionLevel */
  assert(precisionLevel <= 2);
  for (i = 0; i < inputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      float value = downmixMatrix[i * outputCount + j];
      if (value != 0.0f) {
        float gain_dB = (float) (20.0f * log10(value));
        gain_dB = (float) (floor(gain_dB * multiplier + 0.5) * invMultiplier);
        gain_dB = (float) min(gain_dB, 22.0f);
        gain_dB = (float) max(gain_dB, -47.0f);
        downmixMatrix[i * outputCount + j] = gain_dB;
        minGain = (float) min(minGain, floor(gain_dB));
        maxGain = (float) max(maxGain, ceil(gain_dB));
      } else {
        downmixMatrix[i * outputCount + j] = DMX_MATRIX_GAIN_ZERO;
      }
    }
  }
  if (minGain == 96.0f) { /* the entire matrix is zero */
    minGain = -1.0f;
    maxGain = 0.0f;
  }
  if (minGain >= 0.0f) { /* minGain must be <= -1 */
    minGain = -1.0f;
  }
  if (maxGain <= -1.0f) { /* maxGain must be >= 0 */
    maxGain = 0.0f;
  }
  printf("precisionLevel %1d, precision %4.2f dB\n", precisionLevel, 1.0f / (1 << precisionLevel));
  printf("minGain %3.0f maxGain %3.0f\n", minGain, maxGain);
  printf("\n");
  error = wobitbuf_WriteBits(hBitStream, precisionLevel, 2); if (error) return -1;
  error = WriteEscapedValue(hBitStream, (unsigned int) maxGain, 3, 4, 0); if (error) return -1;
  error = WriteEscapedValue(hBitStream, (unsigned int) (-minGain - 1), 4, 5, 0); if (error) return -1;

  for (j = 0; j < compactOutputCount; ++j) {
    isSeparable[j] = 1;
    isSymmetric[j] = 1;
  }

  for (i = 0; i < compactInputCount; ++i) {
    assert(compactInputConfig[i]->pairType != SP_PAIR_NONE);

    for (j = 0; j < compactOutputCount; ++j) {
      float mat[2][2];
      int li = 1 + (compactInputConfig[i]->symmetricPair != NULL);
      int co = 1 + (compactOutputConfig[j]->symmetricPair != NULL);

      assert(compactOutputConfig[j]->pairType != SP_PAIR_NONE);

      mat[0][0] = downmixMatrix[compactInputConfig[i]->originalPosition * outputCount + compactOutputConfig[j]->originalPosition];
      if (li == 2) {
        mat[1][0] = downmixMatrix[compactInputConfig[i]->symmetricPair->originalPosition * outputCount + compactOutputConfig[j]->originalPosition];
      }
      if (co == 2) {
        mat[0][1] = downmixMatrix[compactInputConfig[i]->originalPosition * outputCount + compactOutputConfig[j]->symmetricPair->originalPosition];
        if (li == 2) {
          mat[1][1] = downmixMatrix[compactInputConfig[i]->symmetricPair->originalPosition * outputCount + compactOutputConfig[j]->symmetricPair->originalPosition];
        }
      }

      if ((li == 1) && (co == 1)) {
        /* nothing to check */
        compactDownmixMatrix[i][j] = (mat[0][0] != DMX_MATRIX_GAIN_ZERO);
      } else if ((li == 1) && (co == 2)) {
        if (!configFullForAsymmetricInputs) {
          if (mat[0][0] != mat[0][1]) isSymmetric[j] = 0;
        } else {
          if ((compactInputConfig[i]->pairType != SP_PAIR_SINGLE) && (mat[0][0] != mat[0][1])) isSymmetric[j] = 0;
        }
        compactDownmixMatrix[i][j] = (mat[0][0] != DMX_MATRIX_GAIN_ZERO) || (mat[0][1] != DMX_MATRIX_GAIN_ZERO);
      } else if ((li == 2) && (co == 1)) {
        if (mat[0][0] != mat[1][0]) isSymmetric[j] = 0;
        compactDownmixMatrix[i][j] = (mat[0][0] != DMX_MATRIX_GAIN_ZERO) || (mat[1][0] != DMX_MATRIX_GAIN_ZERO);
      } else { /* (li == 2) && (co == 2) */
        if ((mat[1][0] != DMX_MATRIX_GAIN_ZERO) || (mat[0][1] != DMX_MATRIX_GAIN_ZERO)) isSeparable[j] = 0;
        if ((mat[0][0] != mat[1][1]) || (mat[0][1] != mat[1][0])) isSymmetric[j] = 0;
        compactDownmixMatrix[i][j] = (mat[0][0] != DMX_MATRIX_GAIN_ZERO) || (mat[0][1] != DMX_MATRIX_GAIN_ZERO)
            || (mat[1][0] != DMX_MATRIX_GAIN_ZERO) || (mat[1][1] != DMX_MATRIX_GAIN_ZERO);
      }
      isAllSeparable &= isSeparable[j];
      isAllSymmetric &= isSymmetric[j];
      if (compactDownmixMatrix[i][j]) {
        mixLFEOnlyToLFE &= (compactInputConfig[i]->isLFE == compactOutputConfig[j]->isLFE);
      }

    }
  }


  CoderStateInit(&cs);
  cs.historyCount = 0;
  cs.rawCodingNonzeros = configRawCodingNonzeros;
  cs.gainLGRParam = -1;
  cs.minGain = (int) minGain;
  cs.maxGain = (int) maxGain;
  cs.precisionLevel = precisionLevel;
  cs.gainTableSize = 0;

  printf("separable:");
  error = wobitbuf_WriteBits(hBitStream, isAllSeparable, 1); if (error) return -1;
  for (j = 0; j < compactOutputCount; ++j) {
    printf(" %1d", isSeparable[j]);
    if (!isAllSeparable && (compactOutputConfig[j]->pairType == SP_PAIR_SYMMETRIC)) {
      error = wobitbuf_WriteBits(hBitStream, isSeparable[j], 1); if (error) return -1;
    }
  }
  printf("\n");

  printf("symmetric:");
  error = wobitbuf_WriteBits(hBitStream, isAllSymmetric, 1); if (error) return -1;
  for (j = 0; j < compactOutputCount; ++j) {
    printf(" %1d", isSymmetric[j]);
    if (!isAllSymmetric) {
      error = wobitbuf_WriteBits(hBitStream, isSymmetric[j], 1); if (error) return -1;
    }
  }
  printf("\n");

  if (mixLFEOnlyToLFE != configMixLFEOnlyToLFE) {
    printf("mixLFEOnlyToLFE updated to %1d\n", mixLFEOnlyToLFE);
  }

  if ((inputIndex == -1) || (outputIndex == -1)) {
    /* a compact template should be used only for CICP speaker layouts */
    useCompactTemplate = 0;
    if (useCompactTemplate != configUseCompactTemplate) {
      printf("useCompactTemplate updated to 0, due to non-CICP layout\n");
    }
  }

  if (compactInputCount * compactOutputCount <= 8) {
    rawCodingCompactMatrix = 1;
    /* entropy coding of the compact matrix uses at least 1 + 3 + ceil(log2(totalCount + 1)) bits,
       and raw coding uses totalCount bits, both ignoring the bits corresponding for LFE to LFE;
       therefore, for totalCount <= compactInputCount * compactOutputCount <= 8,
       raw coding of the compact matrix is always smaller or equal than entropy coding */
    if (rawCodingCompactMatrix != configRawCodingCompactMatrix) {
      printf("rawCodingCompactMatrix updated to %1d, for coding efficiency\n", rawCodingCompactMatrix);
    }
  }

  /* encode the compact template */
  error = wobitbuf_WriteBits(hBitStream, mixLFEOnlyToLFE, 1); if (error) return -1;
  error = wobitbuf_WriteBits(hBitStream, rawCodingCompactMatrix, 1); if (error) return -1;

  if (rawCodingCompactMatrix) {
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (!mixLFEOnlyToLFE || (compactInputConfig[i]->isLFE == compactOutputConfig[j]->isLFE)) {
          error = wobitbuf_WriteBits(hBitStream, compactDownmixMatrix[i][j], 1); if (error) return -1;
        } else {
          assert(compactDownmixMatrix[i][j] == 0);
        }
      }
    }
  } else {
    signed char flatCompactMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT + 1];
    int count = 0;
    int totalCount = 0;

    if (useCompactTemplate) {
      compactTemplate = FindCompactTemplate(inputIndex, outputIndex);
      if (compactTemplate == NULL) { /* a template was not found */
        useCompactTemplate = 0;
        printf("useCompactTemplate updated to 0, due to compactTemplate not found\n");
        /* use a template filled with value 0, which has no effect */
        for (i = 0; i < compactInputCount * compactOutputCount; ++i) compactTemplateBuffer[i] = 0;
        compactTemplate = (signed char *) compactTemplateBuffer;
      }
    } else {
      /* use a template filled with value 0, which has no effect */
      for (i = 0; i < compactInputCount * compactOutputCount; ++i) compactTemplateBuffer[i] = 0;
      compactTemplate = (signed char *) compactTemplateBuffer;
    }
    error = wobitbuf_WriteBits(hBitStream, useCompactTemplate, 1); if (error) return -1;

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

    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (!mixLFEOnlyToLFE || (!compactInputConfig[i]->isLFE && !compactOutputConfig[j]->isLFE)) {
          flatCompactMatrix[count++] = compactDownmixMatrix[i][j] ^ compactTemplate[i * compactOutputCount + j];
        }
      }
    }
    assert(count == totalCount);

    error = EncodeFlatCompactMatrix(hBitStream, flatCompactMatrix, totalCount); if (error) return -1;
    count = 0;
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if (mixLFEOnlyToLFE && compactInputConfig[i]->isLFE && compactOutputConfig[j]->isLFE) {
          error = wobitbuf_WriteBits(hBitStream, compactDownmixMatrix[i][j], 1); if (error) return -1;
        } else if (mixLFEOnlyToLFE && (compactInputConfig[i]->isLFE ^ compactOutputConfig[j]->isLFE)) {
          assert(compactDownmixMatrix[i][j] == 0);
        } else {
          count++; /* all the values that were copied above to flatCompactMatrix */
        }
      }
    }
    assert(count == totalCount);
  }


  /* encode values for the nonzero entries in compactDownmixMatrix */
  error = wobitbuf_WriteBits(hBitStream, configFullForAsymmetricInputs, 1); if (error) return -1;
  error = wobitbuf_WriteBits(hBitStream, configRawCodingNonzeros, 1); if (error) return -1;

  for (step = 0; step <= !configRawCodingNonzeros; ++step) {
    if ((step == 0) && !configRawCodingNonzeros) {
      CoderStateGenerateGainTable(&cs);
    }
    if ((step == 1) && !configRawCodingNonzeros) {
      /* compute the optimal gainLGRParam */
      gainsBestLength = CoderStateComputeBestParam(&cs);
      error = wobitbuf_WriteBits(hBitStream, cs.gainLGRParam, 3); if (error) return -1;

      bitCountSave = wobitbuf_GetBitsWritten(hBitStream) - initialBitCount;
    }

    for (i = 0; i < compactInputCount; ++i) {
      assert(compactInputConfig[i]->pairType != SP_PAIR_NONE);

      for (j = 0; j < compactOutputCount; ++j) {
        float mat[2][2];
        int li = 1 + (compactInputConfig[i]->symmetricPair != NULL);
        int co = 1 + (compactOutputConfig[j]->symmetricPair != NULL);

        assert(compactOutputConfig[j]->pairType != SP_PAIR_NONE);

        mat[0][0] = downmixMatrix[compactInputConfig[i]->originalPosition * outputCount + compactOutputConfig[j]->originalPosition];
        if (li == 2) {
          mat[1][0] = downmixMatrix[compactInputConfig[i]->symmetricPair->originalPosition * outputCount + compactOutputConfig[j]->originalPosition];
        }
        if (co == 2) {
          mat[0][1] = downmixMatrix[compactInputConfig[i]->originalPosition * outputCount + compactOutputConfig[j]->symmetricPair->originalPosition];
          if (li == 2) {
            mat[1][1] = downmixMatrix[compactInputConfig[i]->symmetricPair->originalPosition * outputCount + compactOutputConfig[j]->symmetricPair->originalPosition];
          }
        }

        if (compactDownmixMatrix[i][j] != 0) {
          if ((li == 1) && (co == 1)) {
            error = EncodeGainValue(hBitStream, &cs, mat[0][0]); if (error) return -1;

          } else if ((li == 1) && (co == 2)) {
            int useFull = (compactInputConfig[i]->pairType == SP_PAIR_SINGLE) && configFullForAsymmetricInputs;
            error = EncodeGainValue(hBitStream, &cs, mat[0][0]); if (error) return -1;

            if (!isSymmetric[j] || useFull) {
              error = EncodeGainValue(hBitStream, &cs, mat[0][1]); if (error) return -1;
            } else {
              assert(mat[0][0] == mat[0][1]);
            }
          } else if ((li == 2) && (co == 1)) {
            error = EncodeGainValue(hBitStream, &cs, mat[0][0]); if (error) return -1;

            if (!isSymmetric[j]) {
              error = EncodeGainValue(hBitStream, &cs, mat[1][0]); if (error) return -1;
            } else {
              assert(mat[0][0] == mat[1][0]);
            }
          } else { /* (li == 2) && (co == 2) */
            error = EncodeGainValue(hBitStream, &cs, mat[0][0]); if (error) return -1;

            if (isSeparable[j] && isSymmetric[j]) {
              assert((mat[1][0] == DMX_MATRIX_GAIN_ZERO) && (mat[0][1] == DMX_MATRIX_GAIN_ZERO));
              assert((mat[0][0] == mat[1][1]) && (mat[0][1] == mat[1][0]));
            } else if (!isSeparable[j] && isSymmetric[j]) {
              assert((mat[0][0] == mat[1][1]) && (mat[0][1] == mat[1][0]));
              error = EncodeGainValue(hBitStream, &cs, mat[0][1]); if (error) return -1;
            } else if (isSeparable[j] && !isSymmetric[j]) {
              assert((mat[1][0] == DMX_MATRIX_GAIN_ZERO) && (mat[0][1] == DMX_MATRIX_GAIN_ZERO));
              error = EncodeGainValue(hBitStream, &cs, mat[1][1]); if (error) return -1;
            } else { /* !isSeparable[j] && !isSymmetric[j] */
              error = EncodeGainValue(hBitStream, &cs, mat[0][1]); if (error) return -1;
              error = EncodeGainValue(hBitStream, &cs, mat[1][0]); if (error) return -1;
              error = EncodeGainValue(hBitStream, &cs, mat[1][1]); if (error) return -1;
            }
          }
        }

      }
    }

    if ((step == 1) && !configRawCodingNonzeros) {
      bitCount = wobitbuf_GetBitsWritten(hBitStream) - initialBitCount;
      assert(bitCount - bitCountSave == gainsBestLength);
    }
  }

  printf("\n");
  printf("bitCount %4d\n", bitCount);
  printf("compressed to %4.2f bits/gain\n", (float) bitCount / (inputCount * outputCount));
  printf("raw bitCount %4d\n", inputCount * outputCount * 4);
  printf("\n");


#ifdef DMXMATRIX_ENCODER_GENERATE_COMPACTTEMPLATE
  printf("compactTemplate:\n");
  for (i = 0; i < compactInputCount; ++i) {
    for (j = 0; j < compactOutputCount; ++j) {
      printf(" %d", compactDownmixMatrix[i][j]);
    }
    printf("\n");
  }

  {
    int count = 0;
    int perLine = 24;
    printf("\nconst signed char compactTemplate_CICP%d_to_CICP%d[%d * %d] = {", inputIndex, outputIndex, compactInputCount, compactOutputCount);
    for (i = 0; i < compactInputCount; ++i) {
      for (j = 0; j < compactOutputCount; ++j) {
        if ((count % perLine) == 0) printf("\n  ");
        printf(" %d", compactDownmixMatrix[i][j]);
        if ((i != compactInputCount - 1) || (j != compactOutputCount - 1)) printf(",");
        count++;
      }
    }
    printf("\n}\n\n");
  }
#endif

  return error;
}
