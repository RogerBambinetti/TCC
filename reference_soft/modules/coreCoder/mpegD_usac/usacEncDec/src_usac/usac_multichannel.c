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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "usac_multichannel.h"
#include "buffers.h"
#include "usac_mainStruct.h"

static int usac_multichannel_decodeHuffman(HANDLE_BUFFER hBitBuf, const int *const ctab, const int *const ltab, int limit)
{
  int decValue = 0;
  int nBits = 0, tabIdx = 0;
  int newBit;
  int bsVal = 0;
  int found = 0;

  StoreBufferPointer(hBitBuf);

  while(1) {
    nBits = 0;
    bsVal = 0;
    while(nBits < ltab[tabIdx]) {

      newBit = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
      bsVal = (bsVal<<1) | newBit; /* add new bit to the bit stream value */
      nBits++;
    }

    /* found */
    if(ctab[tabIdx] == bsVal) {
#ifdef HUFFMAN_VERIFICATION
      fprintf(stdout, "%d\t%d\t%d \n",tabIdx, ctab[tabIdx], nBits);
#endif
      break;
    } else {
      RestoreBufferPointer(hBitBuf);
    }

    tabIdx++;
    if(tabIdx > limit) {
      CommonExit(1, "End of angle code book reached!\n");
      return -1;
    }
  }

  return tabIdx;
}

static int escapedValue(HANDLE_BUFFER hBitBuf, unsigned long *value, int nBits[3])
{
  unsigned long escVal[3] =
  {
    (1<<nBits[0])-1,
    (1<<nBits[1])-1,
    (1<<nBits[2])-1
  };
  unsigned long tmpVal;

  /* writeFlag - 0: read, 1:write */
  int cnt=0;
  *value = 0;
  tmpVal = 0;
  do {
    tmpVal = GetBits(nBits[cnt], MAX_ELEMENTS, 0, 0, hBitBuf);
    *value += tmpVal;
    cnt++;
  } while (tmpVal == escVal[cnt-1]);

  return 0;
}

static void apply_multichannel_rotation(float* dmx, float* res, int alphaIdx, int nSamples)
{
  int n;
  float L,R;
  for (n=0;n<nSamples;n++) {

    L =  dmx[n] * tabIndexToCosAlpha[alphaIdx] - res[n] * tabIndexToSinAlpha[alphaIdx];
    R =  dmx[n] * tabIndexToSinAlpha[alphaIdx] + res[n] * tabIndexToCosAlpha[alphaIdx];

    dmx[n] = L;
    res[n] = R;
  }
}

static void apply_multichannel_inverse_rotation(float* L, float* R, float* dmx, int alphaIdx, int nSamples)
{
  int n;
  for (n=0;n<nSamples;n++) {
    dmx[n] = 0.0f;
    if(L) {
      dmx[n] += L[n] * tabIndexToCosAlpha[alphaIdx];
    }
    if(R) {
      dmx[n] += R[n] * tabIndexToSinAlpha[alphaIdx];
    }
    if ((float)fabs(dmx[n]) < 1e-8f) dmx[n] = 0.0f; /* prevent float value underflows */
  }
}

static void apply_multichannel_prediction(float* dmx, float* res, int alphaQ, int nSamples, int predDir)
{
  int n;
  float L,R;
  float alpha = alphaQ * 0.1f;

  for (n=0;n<nSamples;n++) {
    if(predDir == 0) {
      L =   dmx[n] + alpha * dmx[n] + res[n];
      R =   dmx[n] - alpha * dmx[n] - res[n];
    } else {
      L =   dmx[n] + alpha * dmx[n] + res[n];
      R = - dmx[n] + alpha * dmx[n] + res[n];
    }

    dmx[n] = L;
    res[n] = R;
  }
}

static int usac_multichannel_parseChannelPair(int codePairs[2],
                                              int numChannelsToApply,
                                              HANDLE_BUFFER hBitBuf)
{
  int chan0,chan1;
  int nBits=0;

  int pairIdx=0;
  int pairCounter = 0;
  int maxNumPairIdx = numChannelsToApply*(numChannelsToApply-1)/2 - 1;
  int numBits = 1;

  if(codePairs == NULL) return -1;
  if(hBitBuf == NULL) return -1;
  if(maxNumPairIdx < 0) return -1;

  while (maxNumPairIdx >>= 1) { ++numBits; }

  pairIdx = GetBits(numBits, MAX_ELEMENTS, 0, 0, hBitBuf);

  for (chan1=1; chan1 < numChannelsToApply; chan1++) {
    for (chan0=0; chan0 < chan1; chan0++) {
      if (pairCounter == pairIdx) {
        codePairs[0] = chan0;
        codePairs[1] = chan1;
        return 0;
      }
      else
        pairCounter++;
    }
  }

  return -1;
}

int usac_multichannel_init(MULTICHANNEL_DATA **pMcData,
                           unsigned char *bitbuf,
                           unsigned char nBitbufLen,
                           int nChannels,
                           int startChannel,
                           int startElement)
{
  int i,error = 0;
  HANDLE_BUFFER hBitBuf = 0;
  MULTICHANNEL_DATA *mcData = NULL;

  if(bitbuf == NULL) return -1;

  *pMcData = (MULTICHANNEL_DATA*)malloc(sizeof(MULTICHANNEL_DATA));
  mcData = *pMcData;
  memset(mcData, 0, sizeof(MULTICHANNEL_DATA));

  /* write bit stream to buffer */
  hBitBuf = CreateBuffer(nBitbufLen*8);
  if(hBitBuf == 0) return -1;

  PrepareWriting(hBitBuf);
  ResetWriteBitCnt(hBitBuf);
  for(i=0;i<nBitbufLen;i++) {
    PutBits(8, hBitBuf, (unsigned long)bitbuf[i]);
  }

  /* Read bit stream */
  PrepareReading(hBitBuf);
  ResetReadBitCnt(hBitBuf);

  mcData->useTool = 1;
  mcData->startChannel = startChannel;
  mcData->startElement = startElement;
  for(i=0;i<nChannels;i++) {
    mcData->channelMask[i+startChannel] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
    if(mcData->channelMask[i+startChannel] > 0) {
      mcData->channelMap[mcData->numChannelsToApply+startChannel] = i+startChannel;
      mcData->numChannelsToApply++;
    }
  }

  *pMcData = mcData;

  if(hBitBuf) DeleteBuffer(hBitBuf);

  return error;
}

int usac_multichannel_parse(MULTICHANNEL_DATA *mcData,
                            unsigned char *bitbuf,
                            unsigned int nBitBufLen,
                            const int indepFlag,
                            Info *sfbInfo)
{
  int i,j,error = 0;
  int defaultVal;
  HANDLE_BUFFER hBitBuf = 0;

  if(bitbuf == 0) return -1;
  if(mcData == 0) return -1;
  if(!mcData->useTool) return 0;

  hBitBuf = CreateBuffer(nBitBufLen*8);
  if(hBitBuf == 0) return -1;

  /* write bit stream to buffer */
  PrepareWriting(hBitBuf);
  ResetWriteBitCnt(hBitBuf);
  for(i=0;i<(int)nBitBufLen;i++) {
    PutBits(8, hBitBuf, (unsigned long)bitbuf[i]);
  }

  /* Read bit stream */
  PrepareReading(hBitBuf);
  ResetReadBitCnt(hBitBuf);
  mcData->stereoFilling = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
  mcData->signalingType = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
  
  if(mcData->signalingType == 0) {
    defaultVal = DEFAULT_ALPHA;
  } else {
    defaultVal = DEFAULT_BETA;
  }

  if(indepFlag > 0) {
    mcData->keepTree = 0;
  } else {
    mcData->keepTree = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
  }

  if(indepFlag > 0) {

    /* reset coefficient memory on indepFlag */
    for(i=0;i<MAX_NUM_MC_BOXES;i++) {
      mcData->pairCoeffQFbPrev[i] = defaultVal;
      for(j=0;j<MAX_NUM_MC_BANDS;j++) {
        mcData->pairCoeffQSfbPrev[i][j] = defaultVal;
      }
    }
  }

  if(mcData->keepTree == 0) {
    int nBits[] = {5,8,16};
    escapedValue(hBitBuf, &mcData->numPairs, nBits);
  }

  /* reset coefficient memory on less pairs */
  for (i=mcData->numPairs; i<MAX_NUM_MC_BOXES; i++) {
    mcData->pairCoeffQFbPrev[i] = defaultVal;
    for(j=0;j<MAX_NUM_MC_BANDS;j++) {
      mcData->pairCoeffQSfbPrev[i][j] = defaultVal;
    }
  }
  
  for(i=0; i<mcData->numPairs; i++) {
    if (mcData->stereoFilling) {
      mcData->bHasStereoFilling[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
    } else {
      mcData->bHasStereoFilling[i] = 0;
    }

    if(mcData->signalingType == 0) {
      int lastVal;
      int newCoeff;
      int bandsPerWindow;

      if(mcData->keepTree == 0) {
        usac_multichannel_parseChannelPair(mcData->codePairs[i], mcData->numChannelsToApply, hBitBuf);
      }

      /* reset coefficient memory on transform length change */
      if ((mcData->windowSequenceIsLongPrev[i] == 0 && sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong != 0)
        || (mcData->windowSequenceIsLongPrev[i] != 0 && sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong == 0)) {

        mcData->pairCoeffQFbPrev[i] = DEFAULT_ALPHA;
        for(j=0;j<MAX_NUM_MC_BANDS;j++) {
          mcData->pairCoeffQSfbPrev[i][j] = DEFAULT_ALPHA;
        }
      }
      mcData->windowSequenceIsLongPrev[i] = sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong;

      mcData->bHasMask[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
      mcData->bHasBandwiseCoeffs[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);

      /* read number of bands for non-fullband boxes */
      if (mcData->bHasMask[i] || mcData->bHasBandwiseCoeffs[i]) {
        int isShort = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
        mcData->numMaskBands[i] = GetBits(5, MAX_ELEMENTS, 0, 0, hBitBuf);
        if (isShort) {
          assert(mcData->numMaskBands[i]<9);
          mcData->numMaskBands[i] *= 8;
        }
      }
      else {
        mcData->numMaskBands[i] = MAX_NUM_MC_BANDS;
      }

      if (mcData->bHasMask[i]) {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->mask[i][j] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
        }
      }
      else {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->mask[i][j] = 1;
        }
      }

      mcData->predDir[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);

      /* time differential coding */
      if (indepFlag > 0) {
        mcData->bDeltaTime[i] = 0;
      }
      else {
        mcData->bDeltaTime[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
      }

      /* bandsPerWindow */
      if (sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong) {
        bandsPerWindow = mcData->numMaskBands[i];
      }
      else {
        bandsPerWindow = mcData->numMaskBands[i]/8;
      }

      if (mcData->bHasBandwiseCoeffs[i] == 0) {
        if (mcData->bDeltaTime[i] > 0) {
          lastVal = mcData->pairCoeffQFbPrev[i];
        }
        else {
          lastVal = DEFAULT_ALPHA;
        }

        newCoeff = lastVal - usac_multichannel_decodeHuffman(hBitBuf, huff_ctabscf, huff_ltabscf, CODE_BOOK_ALPHA_LAV) + 60;

        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->pairCoeffQSfb[i][j] = newCoeff;

          if (mcData->mask[i][j] > 0) {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = newCoeff;
          }
          else {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = DEFAULT_ALPHA;
          }
        }
        mcData->pairCoeffQFbPrev[i] = newCoeff;
      }
      else {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          if (mcData->bDeltaTime[i] > 0) {
            lastVal = mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow];
          }
          else {
            if ((j%bandsPerWindow) == 0) {
              lastVal = DEFAULT_ALPHA;
            }
          }

          if (mcData->mask[i][j] > 0) {
            newCoeff = lastVal - usac_multichannel_decodeHuffman(hBitBuf, huff_ctabscf, huff_ltabscf, CODE_BOOK_ALPHA_LAV) + 60;
            mcData->pairCoeffQSfb[i][j] = newCoeff;
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = newCoeff;
            lastVal = newCoeff;
          }
          else {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = DEFAULT_ALPHA;
          }
        }

        /* reset fullband angle */
        mcData->pairCoeffQFbPrev[i] = DEFAULT_ALPHA;
      }

      /* reset previous angles for bands above mct mask */
      for (j=bandsPerWindow ; j<MAX_NUM_MC_BANDS; j++) {
        mcData->pairCoeffQSfbPrev[i][j] = DEFAULT_ALPHA;
      }
    }
    else if (mcData->signalingType == 1) {
      int lastVal;
      int newCoeff;
      int bandsPerWindow;

      if(mcData->keepTree == 0) {
        usac_multichannel_parseChannelPair(mcData->codePairs[i], mcData->numChannelsToApply, hBitBuf);
      }

      /* reset coefficient memory on transform length change */
      if ((mcData->windowSequenceIsLongPrev[i] == 0 && sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong != 0)
        || (mcData->windowSequenceIsLongPrev[i] != 0 && sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong == 0)) {

        mcData->pairCoeffQFbPrev[i] = DEFAULT_BETA;
        for(j=0;j<MAX_NUM_MC_BANDS;j++) {
          mcData->pairCoeffQSfbPrev[i][j] = DEFAULT_BETA;
        }
      }
      mcData->windowSequenceIsLongPrev[i] = sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong;

      mcData->bHasMask[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
      mcData->bHasBandwiseCoeffs[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);

      /* read number of bands for non-fullband boxes */
      if (mcData->bHasMask[i] || mcData->bHasBandwiseCoeffs[i]) {
        int isShort = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
        mcData->numMaskBands[i] = GetBits(5, MAX_ELEMENTS, 0, 0, hBitBuf);
        if (isShort) {
          mcData->numMaskBands[i] *= 8;
        }
      }
      else {
        mcData->numMaskBands[i] = MAX_NUM_MC_BANDS;
      }

      if (mcData->bHasMask[i]) {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->mask[i][j] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
        }
      }
      else {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->mask[i][j] = 1;
        }
      }

      mcData->predDir[i] = 0; /* only needed for prediction based stereo processing */

      /* time differential coding */
      if (indepFlag > 0) {
        mcData->bDeltaTime[i] = 0;
      }
      else {
        mcData->bDeltaTime[i] = GetBits(1, MAX_ELEMENTS, 0, 0, hBitBuf);
      }

      /* bandsPerWindow */
      if (sfbInfo[mcData->channelMap[mcData->startChannel+mcData->codePairs[i][0]]].islong) {
        bandsPerWindow = mcData->numMaskBands[i];
      }
      else {
        bandsPerWindow = mcData->numMaskBands[i]/8;
      }

      if (mcData->bHasBandwiseCoeffs[i] == 0) {
        if (mcData->bDeltaTime[i] > 0) {
          lastVal = mcData->pairCoeffQFbPrev[i];
        }
        else {
          lastVal = DEFAULT_BETA;
        }

        newCoeff = lastVal + usac_multichannel_decodeHuffman(hBitBuf, huff_ctabAngle, huff_ltabAngle, CODE_BOOK_BETA_LAV);
        if (newCoeff >= CODE_BOOK_BETA_LAV) {
          newCoeff -= CODE_BOOK_BETA_LAV;
        }

        for (j=0; j<mcData->numMaskBands[i]; j++) {
          mcData->pairCoeffQSfb[i][j] = newCoeff;

          if (mcData->mask[i][j] > 0) {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = newCoeff;
          }
          else {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = DEFAULT_BETA;
          }
        }
        mcData->pairCoeffQFbPrev[i] = newCoeff;
      }
      else {
        for (j=0; j<mcData->numMaskBands[i]; j++) {
          if (mcData->bDeltaTime[i] > 0) {
            lastVal = mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow];
          }
          else {
            if ((j%bandsPerWindow) == 0) {
              lastVal = DEFAULT_BETA;
            }
          }

          if (mcData->mask[i][j] > 0) {
            newCoeff = lastVal + usac_multichannel_decodeHuffman(hBitBuf, huff_ctabAngle, huff_ltabAngle, CODE_BOOK_BETA_LAV);
            if (newCoeff >= CODE_BOOK_BETA_LAV) {
              newCoeff -= CODE_BOOK_BETA_LAV;
            }
            mcData->pairCoeffQSfb[i][j] = newCoeff;
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = newCoeff;
            lastVal = newCoeff;
          }
          else {
            mcData->pairCoeffQSfbPrev[i][j%bandsPerWindow] = DEFAULT_BETA;
          }
        }

        /* reset fullband angle */
        mcData->pairCoeffQFbPrev[i] = DEFAULT_BETA;
      }

      /* reset previous angles for bands above mct mask */
      for (j=bandsPerWindow ; j<MAX_NUM_MC_BANDS; j++) {
        mcData->pairCoeffQSfbPrev[i][j] = DEFAULT_BETA;
      }
    }
  }
  for (/*i*/; i<MAX_NUM_MC_BOXES; i++) {
    mcData->codePairs[i][0] = 0;
    mcData->codePairs[i][1] = 0;
  }

  /* validate that the MCT payload was parsed correctly */
  assert(nBitBufLen*8 - GetReadBitCnt(hBitBuf) < 8);
  assert(nBitBufLen*8 - GetReadBitCnt(hBitBuf) >= 0);
  if(hBitBuf) DeleteBuffer(hBitBuf);

  return error;
}

void usac_multichannel_process(MULTICHANNEL_DATA *mcData,
                               float *dmx,
                               float *res,
                               int *coeffSfbIdx,
                               int *mask,
                               int bandsPerWindow,
                               int totalSfb,
                               int pair,
                               const short *sfbOffset,
                               int nSamples)
{
  int sfb = -1; /* note: sfbOffset does not start at zero */
  int i, startLine, stopLine;

  if (!mcData->useTool) return;

  if (mcData->signalingType == 0) {
    /* apply fullband prediction box */
    if (!mcData->bHasBandwiseCoeffs[pair] && !mcData->bHasMask[pair]) {
      apply_multichannel_prediction(dmx, res, coeffSfbIdx[0], nSamples, mcData->predDir[pair]);
    }
    else {
      /* apply bandwise prediction box */
      for (i = 0; i < bandsPerWindow; i++) {
        if (mask[i] == 1) {
          startLine = (sfb < 0) ? 0 : sfbOffset[sfb];
          stopLine = (sfb + 2 < totalSfb) ? sfbOffset[sfb+2] : sfbOffset[sfb+1];

          apply_multichannel_prediction(&dmx[startLine], &res[startLine], coeffSfbIdx[i], stopLine - startLine, mcData->predDir[pair]);
        }
        sfb += 2;

        /* break condition */
        if (sfb >= totalSfb) {
          break;
        }
      }
    }
  }
  else
  {
    /* apply fullband rotation box */
    if (!mcData->bHasBandwiseCoeffs[pair] && !mcData->bHasMask[pair]) {
      apply_multichannel_rotation(dmx, res, coeffSfbIdx[0], nSamples);
    }
    else {
    /* apply bandwise rotation box */
      for (i=0; i<bandsPerWindow; i++) {
        if (mask[i] == 1) {
          startLine = (sfb < 0) ? 0 : sfbOffset[sfb];
          stopLine = (sfb + 2 < totalSfb) ? sfbOffset[sfb+2] : sfbOffset[sfb+1];

          apply_multichannel_rotation(&dmx[startLine], &res[startLine], coeffSfbIdx[i], stopLine - startLine);
        }
        sfb += 2;

        /* break condition */
        if (sfb >= totalSfb) {
          break;
        }
      }
    }
  }
}

void usac_multichannel_get_prev_dmx(MULTICHANNEL_DATA *mcData,
                                    float *prevSpec1,
                                    float *prevSpec2,
                                    float *prevDmx,
                                    const int bandsPerWindow,
                                    const int *mask,
                                    const int *coeffSfbIdx,
                                    const int nSamples,
                                    const int pair,
                                    const int totalSfb,
                                    const short *sfbOffset)
{
  int sfb = -1; /* note: sfbOffset does not start at zero */
  int i, startLine, stopLine;

  if (mcData->signalingType == 0) {
    /* apply fullband prediction box */
    if ((prevSpec1 != NULL) && (prevSpec2 != NULL)) {
      if (mcData->predDir[pair]) {
        for (i = 0; i < nSamples; i++) {
          prevDmx[i] = 0.5f * (prevSpec1[i] - prevSpec2[i]);
        }
      } else {
        for (i = 0; i < nSamples; i++) {
          prevDmx[i] = 0.5f * (prevSpec1[i] + prevSpec2[i]);
        }
      }
    }
  }
  else if (!mcData->bHasBandwiseCoeffs[pair] && !mcData->bHasMask[pair]) {
    /* apply fullband rotation box */
    apply_multichannel_inverse_rotation(prevSpec1, prevSpec2, prevDmx, coeffSfbIdx[0], nSamples);
  }
  else {
    /* apply bandwise rotation box */
    for (i = 0; i < MAX_NUM_MC_BANDS; i++) {
      const int rotAngle = (mask[i] && i < bandsPerWindow) ? coeffSfbIdx[i] : (CODE_BOOK_BETA_LAV >> 1);
      float *prevSpec1Start, *prevSpec2Start;

      startLine = (sfb < 0) ? 0 : sfbOffset[sfb];
      stopLine = (sfb + 2 < totalSfb) ? sfbOffset[sfb+2] : sfbOffset[sfb+1];

      prevSpec1Start = (prevSpec1) ? &prevSpec1[startLine] : NULL;
      prevSpec2Start = (prevSpec2) ? &prevSpec2[startLine] : NULL;

      apply_multichannel_inverse_rotation(prevSpec1Start, prevSpec2Start, &prevDmx[startLine], rotAngle, stopLine - startLine);
      sfb += 2;

      /* break condition */
      if (sfb >= totalSfb) {
        break;
      }
    }
  }
}

void usac_multichannel_stereofilling_add(float *coef, float *dmx_prev, float *scaleFactors, const int total_sfb,
                                         const int group_len, const int bins_per_sbk, const short *sbk_sfb_top)
{
  int groupwin, idx, sfb;

  for (sfb = 1; sfb < total_sfb; sfb++) {   /* start at 1 to avoid need for special handling of band 0 */
    if (scaleFactors[sfb] > 1e-8f) {
      const float scf4 = 4.0f / scaleFactors[sfb];
      float factor = 1e-8f;
      float enTarget = 0.0f;
      float enRes = 0.0f;

      for (groupwin = 0; groupwin < group_len; groupwin++) {
        const int offset = groupwin * bins_per_sbk;

        /* amplify empty band to a maximum of target energy per bin, measure energy of delayed downmix */
        for (idx = sbk_sfb_top[sfb-1]; idx < sbk_sfb_top[sfb]; idx++) {
          factor += dmx_prev[offset+idx] * dmx_prev[offset+idx];
          coef[offset+idx] *= scf4;
          enRes += coef[offset+idx] * coef[offset+idx];
        }
        enTarget += (float)(sbk_sfb_top[sfb] - sbk_sfb_top[sfb-1]);
      }

      if (enRes < enTarget) {
        factor = (float)sqrt((enTarget - enRes) / factor);
        factor = min(factor, 10.0f);
        enRes = 0.0f;

        /* if delayed downmix isn't empty, add scaled downmix such that the band reaches target energy */
        for (groupwin = 0; groupwin < group_len; groupwin++) {
          const int offset = groupwin * bins_per_sbk;

          for (idx = sbk_sfb_top[sfb-1]; idx < sbk_sfb_top[sfb]; idx++) {
            coef[offset+idx] += dmx_prev[offset+idx] * factor;
            enRes += coef[offset+idx] * coef[offset+idx];
          }
        }
        if ((enRes != enTarget) && (enRes > 0.0f)) {  /* target energy isn't reached, correct the band */
          factor = (float)sqrt(enTarget / (enRes + 1e-8f));
          factor = min(factor, 10.0f);

          for (groupwin = 0; groupwin < group_len; groupwin++) {
            const int offset = groupwin * bins_per_sbk;

            for (idx = sbk_sfb_top[sfb-1]; idx < sbk_sfb_top[sfb]; idx++) {
              coef[offset+idx] *= factor;
            }
          }
        }
      } /* if (enRes < enTarget) */

      for (groupwin = 0; groupwin < group_len; groupwin++) { /* apply scale factor to the current band */
        const int offset = groupwin * bins_per_sbk;

        for (idx = sbk_sfb_top[sfb-1]; idx < sbk_sfb_top[sfb]; idx++) {
          coef[offset+idx] *= scaleFactors[sfb];
        }
      }
    }
  } /* loop over bands */
}

void usac_multichannel_save_prev(MULTICHANNEL_DATA *mcData,
                                 float *spec,
                                 const int ch,
                                 const int num_windows,
                                 const int *bins_per_sbk,
                                 const int zeroSpecSaves)
{
  int offs, i, w;

  offs = 0;
  if (zeroSpecSaves) {
    for (w = 0; w < num_windows; w++) {
      for (i = 0; i < bins_per_sbk[w]; i++) {
        mcData->prevOutSpec[ch][i + offs] = 0.0f;
      }
      offs += bins_per_sbk[w];
    }
  } else {
    for (w = 0; w < num_windows; w++) {
      for (i = 0; i < bins_per_sbk[w]; i++) {
        mcData->prevOutSpec[ch][i + offs] = spec[i + offs];
      }
      offs += bins_per_sbk[w];
    }
  }
}
