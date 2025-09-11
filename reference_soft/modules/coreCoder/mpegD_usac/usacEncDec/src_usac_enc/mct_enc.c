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

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mct_enc.h"
#include "usac_multichannel.h"
#include "allHandles.h"
#include "bitstream.h"

static int channelPairToIndex(int          chIdx1,
                              int          chIdx2,
                              unsigned int nChannels)
{
  unsigned int ch1,ch2;
  int pairIdx = 0;


  for (ch2=1; ch2 < nChannels; ch2++) {
    for (ch1=0; ch1 < ch2; ch1++) {
      if ((ch1 == chIdx1) && (ch2 == chIdx2))
        return pairIdx;
      else
        pairIdx++;
    }
  }
  return -1;
}

void mct_encode(double *spectral_line_vector[MAX_TIME_CHANNELS],
                int         blockSizeSamples,
                WINDOW_SEQUENCE   windowSequence[MAX_TIME_CHANNELS],
                int         num_window_groups,
                const int   window_group_length[MAX_SHORT_WINDOWS],
                const int   sfb_offset[MAX_SCFAC_BANDS+1],                
                int         endSfb,
                MCTData     *mctData,
                int         debugLevel,
                int         chOffset)
{
  int ch1,ch2, pair, i, sfb;
  
  mctData->numPairs = mctData->numChannels/2;

  for (pair = 0; pair < mctData->numPairs; pair++) {

    /* find highest correlation */
    double maxcorr = 0.0;
    int tmpCh1, tmpCh2;
    for (tmpCh1 = 0; tmpCh1 < mctData->numChannels-1; tmpCh1++){
      for (tmpCh2 = tmpCh1+1; tmpCh2<mctData->numChannels;tmpCh2++) {
        double corr = 0.0;
        int pairWasAlreadyCoded = 0;
        for (i=0;i<pair;i++) {
          if ((mctData->channelPair[i][0] == tmpCh1) && (mctData->channelPair[i][1] == tmpCh2)) 
            pairWasAlreadyCoded = 1;
        }
        if (pairWasAlreadyCoded) continue;
        if (windowSequence[tmpCh1] == EIGHT_SHORT_SEQUENCE) continue;
        if (windowSequence[tmpCh2] == EIGHT_SHORT_SEQUENCE) continue;
        for (i=0; i<sfb_offset[endSfb]; i++) 
          corr += spectral_line_vector[tmpCh1][i]*spectral_line_vector[tmpCh1][i];
        if (fabs(corr) > maxcorr){          
          maxcorr = corr;
          ch1=tmpCh1;
          ch2=tmpCh2;
        }
      }
    }
    if (maxcorr == 0.0f) break;

    mctData->channelPair[pair][0] = ch1;
    mctData->channelPair[pair][1] = ch2;

    mctData->numMaskBands[pair] = 0;
    for (sfb = 0; sfb < endSfb; sfb +=2 ) {
      int alphaQ = 0;
      double alpha = 0.0;


      /* prediction */
      if ((mctData->mctSignalingType== 0) || (mctData->mctSignalingType== 2)) {
        double emid = 0.0, cov = 0.0;
        for (i = sfb_offset[sfb]; i < sfb_offset[sfb+2]; i++) {
          double M = 0.5*spectral_line_vector[ch1][i] + 0.5*spectral_line_vector[ch2][i];
          double S = 0.5*spectral_line_vector[ch1][i] - 0.5*spectral_line_vector[ch2][i];
          emid += M*M;
          cov  += M*S;
        }

        alpha = cov/(emid +  1.0e-6f);
        alphaQ =  (alpha<0.0?-1:1) * min((int)(fabs(alpha)*10.0+0.5),30);

        for (i = sfb_offset[sfb]; i < sfb_offset[sfb+2]; i++) {
          double M = 0.5*spectral_line_vector[ch1][i] + 0.5*spectral_line_vector[ch2][i];
          double S = 0.5*spectral_line_vector[ch1][i] - 0.5*spectral_line_vector[ch2][i];
          double res = S - (double)alphaQ *0.1 * M;
          spectral_line_vector[ch1][i] = M;
          spectral_line_vector[ch2][i] = res;
        }
      }
      /* rotation */
      else if ((mctData->mctSignalingType== 1) || (mctData->mctSignalingType== 3)) {
        double energy1 = 0.0, energy2 = 0.0, corr = 0.0;

        for (i = sfb_offset[sfb]; i < sfb_offset[sfb+2]; i++) {
          energy1 += spectral_line_vector[ch1][i] * spectral_line_vector[ch1][i];
          energy2 += spectral_line_vector[ch2][i] * spectral_line_vector[ch2][i];
        }

        /* calculate angle from panning law and quantize */
        alpha = atan2(sqrt(energy2), sqrt(energy1));
        alphaQ =  32 + (int)((alpha<0?-1:1) * (int)(fabs((alpha/(1.5707963f)*32)+0.5f)) ); /* pi/2 = 1.5707963f */      

        for (i = sfb_offset[sfb]; i < sfb_offset[sfb+2]; i++) {
          double dmx =   spectral_line_vector[ch1][i] * tabIndexToCosAlpha[alphaQ] + spectral_line_vector[ch2][i] * tabIndexToSinAlpha[alphaQ];
          double res = - spectral_line_vector[ch1][i] * tabIndexToSinAlpha[alphaQ] + spectral_line_vector[ch2][i] * tabIndexToCosAlpha[alphaQ];
          spectral_line_vector[ch1][i] = dmx;
          spectral_line_vector[ch2][i] = res;
        }
      }
      mctData->pairAlpha[pair][mctData->numMaskBands[pair]] = alphaQ; 
      mctData->pairMctMask[pair][mctData->numMaskBands[pair]++] = 1;           
    }

  }
  mctData->numPairs = pair;
}



void mct_write_bitstream(MCTData     *mctData,
                         int indepFlag,
                         unsigned char *extensionPayload,
                         int *extensionPayloadSize,
                         int *extensionPayloadPresent
) {
  int pair, band;
  unsigned char *tmp;
  int keepTree        = 0;
  int hasMctMask      = 1;
  int hasBandwiseCoef = 1;
  int isMctShort      = 0;
  int mct_delta_time  = 0;
  HANDLE_BSBITSTREAM bs = NULL;
  HANDLE_BSBITBUFFER bsBuffer = BsAllocBuffer(6144);
  int numBits = 0;

  int maxNumPairIdx = mctData->numChannels*(mctData->numChannels-1)/2 - 1;
  int numPairIdxBits = 1;
  while (maxNumPairIdx >>= 1) { ++numPairIdxBits; }

  bs = BsOpenBufferWrite(bsBuffer);

  *extensionPayloadSize    = 0;
  *extensionPayloadPresent = 0;

  BsPutBit(bs, mctData->mctSignalingType, 2); numBits += 2;

  if (!indepFlag) {
    BsPutBit(bs, keepTree , 1); numBits += 1;
  }
  if (keepTree == 0) {
    BsPutBit(bs, mctData->numPairs, 5); numBits += 5;
  }

  for (pair = 0; pair < mctData->numPairs; pair++) {
    if (mctData->mctSignalingType>1) {
      BsPutBit(bs, 1 /* hasStereoFilling[pair]*/ , 1); numBits += 1;
    }

    if (keepTree == 0) {
      int pairIdx = channelPairToIndex(mctData->channelPair[pair][0], mctData->channelPair[pair][1], mctData->numChannels);
      BsPutBit(bs, pairIdx ,numPairIdxBits); numBits += numPairIdxBits;
    }

    BsPutBit(bs, hasMctMask                 , 1); numBits += 1;
    BsPutBit(bs, hasBandwiseCoef            , 1); numBits += 1;
    BsPutBit(bs, isMctShort                 , 1); numBits += 1;
    BsPutBit(bs, mctData->numMaskBands[pair], 5); numBits += 5;

    if (hasMctMask) {
      for (band = 0; band < mctData->numMaskBands[pair]; band++) {
        BsPutBit(bs, mctData->pairMctMask[pair][band] , 1); numBits += 1;
      }
    }
    if (!indepFlag) {
      BsPutBit(bs, 0 /* mct_delta_time */ , 1); numBits += 1;
    }

    /* prediction */
    if ((mctData->mctSignalingType == 0) || (mctData->mctSignalingType == 2)) {
      int lastVal = DEFAULT_ALPHA;
      BsPutBit(bs, 0 /* pred_dir */ , 1); numBits += 1;
      if (hasBandwiseCoef) {
        for (band = 0; band < mctData->numMaskBands[pair]; band++) {
          if (mctData->pairMctMask[pair][band] == 1) {
            int angleBs = -(mctData->pairAlpha[pair][band]-lastVal);
            angleBs += CODE_BOOK_ALPHA_LAV/2;
            lastVal = mctData->pairAlpha[pair][band];
            BsPutBit(bs,huff_ctabscf[angleBs] , huff_ltabscf[angleBs]); numBits += huff_ltabscf[angleBs];
          }
        }
      }
      else {
        int angleBs = -(mctData->pairAlpha[pair][0]-lastVal);
        angleBs += CODE_BOOK_ALPHA_LAV/2;
        BsPutBit(bs,huff_ctabAngle[angleBs] , huff_ltabAngle[angleBs]); numBits += huff_ltabAngle[angleBs];
      }
    }
    /* rotation */
    else if ((mctData->mctSignalingType == 1) || (mctData->mctSignalingType == 3)) {
      int lastVal = DEFAULT_BETA;
      if (hasBandwiseCoef) {
        for (band = 0; band < mctData->numMaskBands[pair]; band++) {
          if (mctData->pairMctMask[pair][band] == 1) {
            int angleBs = mctData->pairAlpha[pair][band]-lastVal;
            if (angleBs < 0) angleBs += CODE_BOOK_BETA_LAV; /* wraparound */
            lastVal = mctData->pairAlpha[pair][band];
            BsPutBit(bs,huff_ctabAngle[angleBs] , huff_ltabAngle[angleBs]); numBits += huff_ltabAngle[angleBs];
          }
        }
      }
      else {
        int angleBs = mctData->pairAlpha[pair][0]-lastVal;
        if (angleBs < 0) angleBs += CODE_BOOK_BETA_LAV; /* wraparound */
        BsPutBit(bs,huff_ctabAngle[angleBs] , huff_ltabAngle[angleBs]); numBits += huff_ltabAngle[angleBs];
      }
    }
  }

  if (numBits%8) {
    BsPutBit(bs, 0 , 8-numBits%8); numBits += 8-numBits%8;
  }
  
  /* copy payload to extension buffer */  
  tmp = BsBufferGetDataBegin(bsBuffer);
  *extensionPayloadSize  = numBits/8;

  if ((*extensionPayloadSize) > 0) {
    memcpy(extensionPayload, tmp, *extensionPayloadSize);
    *extensionPayloadPresent = 1;
  }

  BsClose(bs);
  BsFreeBuffer(bsBuffer);
}