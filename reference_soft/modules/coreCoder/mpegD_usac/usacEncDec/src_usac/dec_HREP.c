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

#include "dec_HREP.h"

#define THRESHOLD         78.3837f

const float win[HREP_HALF_BLOCK_SIZE] =
{
  0.01227153f, 0.03680722f, 0.06132073f, 0.08579731f, 0.11022220f, 0.13458070f, 0.15885814f, 0.18303988f,
  0.20711137f, 0.23105810f, 0.25486565f, 0.27851968f, 0.30200594f, 0.32531029f, 0.34841868f, 0.37131719f,
  0.39399204f, 0.41642956f, 0.43861623f, 0.46053871f, 0.48218377f, 0.50353838f, 0.52458968f, 0.54532498f,
  0.56573181f, 0.58579785f, 0.60551104f, 0.62485948f, 0.64383154f, 0.66241577f, 0.68060099f, 0.69837624f,
  0.71573082f, 0.73265427f, 0.74913639f, 0.76516726f, 0.78073722f, 0.79583690f, 0.81045719f, 0.82458930f,
  0.83822470f, 0.85135519f, 0.86397285f, 0.87607009f, 0.88763962f, 0.89867446f, 0.90916798f, 0.91911385f, 
  0.92850608f, 0.93733901f, 0.94560732f, 0.95330604f, 0.96043051f, 0.96697647f, 0.97293995f, 0.97831737f,
  0.98310548f, 0.98730141f, 0.99090263f, 0.99390697f, 0.99631261f, 0.99811811f, 0.99932238f, 0.99992470f
};

const float squ_win[HREP_HALF_BLOCK_SIZE] =
{
  0.00015059f, 0.00135477f, 0.00376023f, 0.00736117f, 0.01214893f, 0.01811196f, 0.02523590f, 0.03350360f,
  0.04289512f, 0.05338784f, 0.06495650f, 0.07757321f, 0.09120759f, 0.10582678f, 0.12139557f, 0.13787645f,
  0.15522972f, 0.17341357f, 0.19238420f, 0.21209590f, 0.23250119f, 0.25355090f, 0.27519433f, 0.29737934f,
  0.32005248f, 0.34315912f, 0.36664362f, 0.39044937f, 0.41451905f, 0.43879466f, 0.46321771f, 0.48772938f,
  0.51227061f, 0.53678228f, 0.56120533f, 0.58548094f, 0.60955062f, 0.63335637f, 0.65684087f, 0.67994751f,
  0.70262065f, 0.72480566f, 0.74644909f, 0.76749880f, 0.78790409f, 0.80761579f, 0.82658642f, 0.84477027f,
  0.86212354f, 0.87860442f, 0.89417321f, 0.90879240f, 0.92242678f, 0.93504349f, 0.94661215f, 0.95710487f,
  0.96649639f, 0.97476409f, 0.98188803f, 0.98785106f, 0.99263882f, 0.99623976f, 0.99864522f, 0.99984940f
};

const float beta_factor_dequant_table[8] =
{
  0.000f, 0.035f, 0.070f, 0.120f, 0.170f, 0.220f, 0.270f, 0.320f
};

const float beta_factor_dequant_table_precise[16] =
{
  0.000f, 0.035f, 0.070f, 0.095f, 0.120f, 0.145f, 0.170f, 0.195f,
  0.220f, 0.245f, 0.270f, 0.295f, 0.320f, 0.345f, 0.370f, 0.395f
};

const float gain_table[8] =
{
  0.50000000f, 0.59460355f, 0.70710678f, 0.84089641f, 1.00000000, 1.18920711f, 1.41421356f, 1.68179283f
};


int arithDecodeProbBit (unsigned char *buf,
                        long *bp,
                        Tastat *s,
                        int   cnt0,
                        int   total
                        )
{
  const int probScale = 1 << 14;
  int res;

  unsigned short tbl[2];
  tbl[0] = probScale - (cnt0*probScale) / total;
  tbl[1] = 0;

  *bp = ari_decode(buf, *bp, &res, s, tbl, 2);

  return res;
}

static int arithDecodeBit (unsigned char *buf,
                            long *bp,
                            Tastat *s
                            )
{
  unsigned short tbl[2] = {8192, 0};
  int res;

  *bp = ari_decode(buf, *bp, &res, s, tbl, 2);

  return res;
}

int readSingleBit(unsigned char *buf, long *bp) {
  int bit;

  bit = (buf[*bp>>3]>>(7-(*bp&7)))&1;
  (*bp)++;

  return bit;
}

int readMultipleBits(unsigned char *buf, long *bp, int nBits) {
  int i;
  int val = 0;

  for (i = 0; i < nBits; i++) {
    val = (val << 1) | readSingleBit(buf, bp);
  }

  return val;
}

void hrepConfigInit(unsigned char*   buf,
                    unsigned int     bufLen,
                    HREP_GROUP_DATA* hrepGroupData,
                    int              startChannel,
                    int              stopChannel,
                    int*             lfe_channels
                    )
{
  int i;
  long bp = 0;
  unsigned int useCommonSettings;
  int index = -1;

  hrepGroupData->extGainRange = readSingleBit(buf, &bp); /*extended gain range*/

  hrepGroupData->extBetaPrec = readSingleBit(buf, &bp); /*extended beta factor precision*/

  for (i=startChannel; i<stopChannel; i++) {
    if (i == lfe_channels[0] || i == lfe_channels[1]){
      /*HREP inactive for LFE*/
      hrepGroupData->isHREPActive[i] = 0;
    } else {
      hrepGroupData->isHREPActive[i] = readSingleBit(buf, &bp); /*HREP active*/
      if (index == -1) index = i; /* get first active channel */
    }
    if (hrepGroupData->isHREPActive[i]) {
      if (i == startChannel) {
        unsigned int lastFFTLine = readMultipleBits(buf, &bp, 4);
        hrepGroupData->transWidth[startChannel] = readMultipleBits(buf, &bp, 4);
        hrepGroupData->idxMinAdj[startChannel] = lastFFTLine +1 - hrepGroupData->transWidth[startChannel];
        hrepGroupData->idxMinAdj[startChannel]++; /* convert back to Matlab style indexing used by the processing in the HREP code */
        hrepGroupData->defaultBeta[startChannel] = readMultipleBits(buf, &bp, 3+(hrepGroupData->extBetaPrec));
      } else {
        useCommonSettings = readSingleBit(buf, &bp); /*use common settings*/
        if (useCommonSettings) {
          assert(hrepGroupData->isHREPActive[startChannel]); /* only use common settings if first channel of signal group was active, otherwise may indicate broken encoder or corrupted bitstream */
          hrepGroupData->transWidth[i] = hrepGroupData->transWidth[startChannel];
          hrepGroupData->idxMinAdj[i] = hrepGroupData->idxMinAdj[startChannel];
          hrepGroupData->defaultBeta[i] = hrepGroupData->defaultBeta[startChannel];
        } else {
          unsigned int lastFFTLine = readMultipleBits(buf, &bp, 4);
          hrepGroupData->transWidth[i] = readMultipleBits(buf, &bp, 4);
          hrepGroupData->idxMinAdj[i] = lastFFTLine +1 - hrepGroupData->transWidth[i];
          hrepGroupData->idxMinAdj[i]++; /* convert back to Matlab style indexing used by the processing in the HREP code */
          hrepGroupData->defaultBeta[i] = readMultipleBits(buf, &bp, 3+(hrepGroupData->extBetaPrec));
        }
      }
    }
  }

  for (i=startChannel; i<stopChannel; i++) {
    /* initialize variables for inactive channels */
    if (!hrepGroupData->isHREPActive[i]) {
      if (index == -1) {
        hrepGroupData->transWidth[i] = 2;
        hrepGroupData->idxMinAdj[i] = 10;
        hrepGroupData->defaultBeta[i] = 6;
      } else {
        hrepGroupData->transWidth[i] = hrepGroupData->transWidth[index];
        hrepGroupData->idxMinAdj[i] = hrepGroupData->idxMinAdj[index];
        hrepGroupData->defaultBeta[i] = hrepGroupData->defaultBeta[index];
      }
    }
  }

  assert(8*bufLen - bp <= 7);
  while (bp % 8 != 0) {
    assert(readSingleBit(buf, &bp) == 0); /*check that padding is zero*/
  }
  assert(8*bufLen == (unsigned int) bp);

}

void decodeHrepSideInfo(unsigned char *bitbuf,
                        int bufLen,
                        HREP_DATA *hrepData,
                        int sigGroup,
                        int lfe_channels[2]
                        ) 
{
  int i,j;
  int rawCoding;
  long bp=0;

  Tastat arithDec = {0, 0, 0};

  int cntMask[2] = {1, 1};
  int cntSign[2] = {1, 1};
  int cntNeg[2] = {1, 1};
  int cntPos[2] = {1, 1};

  rawCoding = readSingleBit(bitbuf, &bp);

  if (rawCoding) {
    for (i = 0; i < hrepData->numHrepBlocks; i++) {
      for (j = hrepData->startChannels[sigGroup]; j < hrepData->startChannels[sigGroup+1]; j++) {
        if (lfe_channels[0] == j || lfe_channels[1] == j || hrepData->hrepGroupData[sigGroup].isHREPActive[j] == 0) continue;
        hrepData->gain_idx[i][j] = readMultipleBits(bitbuf,&bp,3 + hrepData->hrepGroupData[sigGroup].extGainRange);
        hrepData->gain_idx[i][j] -= 4 + 4*hrepData->hrepGroupData[sigGroup].extGainRange;
      }
    }
  } else {

    bp = ari_start_decoding(bitbuf, bp, &arithDec);

    for (i = 0; i < hrepData->numHrepBlocks; i++) {
      for (j = hrepData->startChannels[sigGroup]; j < hrepData->startChannels[sigGroup+1]; j++) {
        int maskBit;
        if (lfe_channels[0] == j || lfe_channels[1] == j || hrepData->hrepGroupData[sigGroup].isHREPActive[j] == 0) continue;
        maskBit = arithDecodeProbBit(bitbuf, &bp, &arithDec, cntMask[0], cntMask[0]+cntMask[1]);
        cntMask[maskBit]++;

        if (maskBit) {
          int signBit = arithDecodeProbBit(bitbuf, &bp, &arithDec, cntSign[0], cntSign[0]+cntSign[1]);
          cntSign[signBit] += 2;

          if (signBit) {
            int largeBit = arithDecodeProbBit(bitbuf, &bp, &arithDec, cntNeg[0], cntNeg[0]+cntNeg[1]);
            int lastBit = arithDecodeBit(bitbuf, &bp, &arithDec);
            cntNeg[largeBit] += 2;
            hrepData->gain_idx[i][j] = -2 * largeBit - 2 + lastBit;
          } else {
            int largeBit = arithDecodeProbBit(bitbuf, &bp, &arithDec, cntPos[0], cntPos[0]+cntPos[1]);
            cntPos[largeBit] += 2;
            if (largeBit == 0) {
              int lastBit = arithDecodeBit(bitbuf, &bp, &arithDec);
              hrepData->gain_idx[i][j] = 2 - lastBit;
            } else {
              hrepData->gain_idx[i][j] = 3;
            }
          }
        }else {
          hrepData->gain_idx[i][j] = 0;
        }

        if (hrepData->hrepGroupData[sigGroup].extGainRange) {
          const int prob_scale = 1 << 14;
          const int esc_cnt = prob_scale / 5;
          unsigned short tbl_esc[5] = {prob_scale-esc_cnt, prob_scale-2*esc_cnt, prob_scale-3*esc_cnt, prob_scale-4*esc_cnt, 0};
          int sym = hrepData->gain_idx[i][j];
          int esc;
          if (sym <= -4) {
            bp = ari_decode(bitbuf, bp, &esc, &arithDec, tbl_esc, 5);
            sym = -4 - esc;
          } else if (sym >= 3) {
            bp = ari_decode(bitbuf, bp, &esc, &arithDec, tbl_esc, 5);
            sym = 3 + esc;
          }
          hrepData->gain_idx[i][j] = sym;
        }
      } 
    }
    bp = ari_done_decoding(bp);
  }

  for (j = hrepData->startChannels[sigGroup]; j < hrepData->startChannels[sigGroup+1]; j++) {
    for (i = 0; i < hrepData->numHrepBlocks; i++) {
      if (hrepData->gain_idx[i][j]) {
        int useDefaultBeta = readSingleBit(bitbuf,&bp);
        if (useDefaultBeta) {
          hrepData->betaFactorIdx[j] = hrepData->hrepGroupData[sigGroup].defaultBeta[j];
        } else {
          hrepData->betaFactorIdx[j] = readMultipleBits(bitbuf,&bp,3);
        }
        break;
      }
    }
  }

  assert(8*bufLen - bp <= 7);
  while (bp % 8 != 0) {
    assert(readSingleBit(bitbuf, &bp) == 0); /*check that padding is zero*/
  }
  assert(8*bufLen == bp);
}


void decode_HREP(float* time_samples, 
                int    blockSize, 
                int    currCh, 
                HANDLE_MODULO_BUF_VM* hrepInBuffer,
                HREP_DATA *hrepData,
                int    sigGroup
                ) 
{
  int i, j;
  int idxMin;
  int numBlocks = blockSize/HREP_HALF_BLOCK_SIZE;
  float fmult, fadd, betaFactor;

  float lastGain=1, nextGain=1;
  float currGain=1, compGain=1;

  float processing_shape[HREP_HALF_BLOCK_SIZE+1] = {0};
  float reconstruction_shape[HREP_HALF_BLOCK_SIZE+1] = {0};

  float in_data[4096+2*HREP_HALF_BLOCK_SIZE] = {0};
  float fft_in_re[2*HREP_HALF_BLOCK_SIZE] = {0};
  float fft_in_im[2*HREP_HALF_BLOCK_SIZE] = {0};
  float fft_out_re[2*HREP_HALF_BLOCK_SIZE] = {0};
  float fft_out_im[2*HREP_HALF_BLOCK_SIZE] = {0};
  float lp_block[2*HREP_HALF_BLOCK_SIZE] = {0};
  float lp_block_im[2*HREP_HALF_BLOCK_SIZE] = {0};
  float hp_block[2*HREP_HALF_BLOCK_SIZE] = {0};

  float interp_corr[2*HREP_HALF_BLOCK_SIZE] = {0};
  float interp_factor;

  float proc_data[4096+4*HREP_HALF_BLOCK_SIZE] = {0};

  idxMin = hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh] + hrepData->hrepGroupData[sigGroup].transWidth[currCh];

  for (i = hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh]-1; i < idxMin-1; i++) {
    processing_shape[i] = (float)(i - hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh] +2) / (float)(idxMin - hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh] + 1);
  }
  for (i = idxMin-1; i <= HREP_HALF_BLOCK_SIZE; i++) {
    processing_shape[i] = 1;
    reconstruction_shape[i] = 1;
  }

  if (hrepData->hrepGroupData[sigGroup].extBetaPrec) {
    betaFactor = beta_factor_dequant_table_precise[hrepData->betaFactorIdx[currCh]];
  } else {
    betaFactor = beta_factor_dequant_table[hrepData->betaFactorIdx[currCh]];
  }
  fmult = 1+ betaFactor;
  fadd = -betaFactor;

  AddFloatModuloBufferValues(hrepInBuffer[currCh], time_samples, blockSize);
  ReadFloatModuloBufferValues(hrepInBuffer[currCh], in_data, blockSize);
  GetFloatModuloBufferValues(hrepInBuffer[currCh], in_data+blockSize, HREP_HALF_BLOCK_SIZE, HREP_HALF_BLOCK_SIZE);

  /* put delayed samples at the beginning of processing array */
  for (i = 0; i < HREP_HALF_BLOCK_SIZE; i++){
    proc_data[i] = hrepData->delay_buf[i][currCh];
  }

  for (i = 0; i < numBlocks; i++) {
    /*copy time samples for FFT*/

    for (j = 0; j < 2*HREP_HALF_BLOCK_SIZE; j++) {
      fft_in_re[j] = in_data[i*HREP_HALF_BLOCK_SIZE+j];
    }

    /*apply sine window*/
    for (j = 0; j < HREP_HALF_BLOCK_SIZE; j++) {
      fft_in_re[j] *= win[j]; 
      fft_in_re[2*HREP_HALF_BLOCK_SIZE-1-j] *= win[j];
    }

    /*calculate FFT*/
    CFFTN_NI(fft_in_re, fft_in_im, fft_out_re, fft_out_im, 2*HREP_HALF_BLOCK_SIZE, -1);

    if (i == 0) {
      lastGain = hrepData->delayedGains[0][currCh];
      currGain = hrepData->delayedGains[1][currCh];
    } else {
      lastGain = currGain;
      currGain = nextGain;
    }
    nextGain = gain_table[hrepData->gain_idx[i][currCh]+4];
    compGain = fmult * currGain + fadd;

    /*compute the adaptive reconstruction shape in the transition region*/
    for (j = hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh]-1; j < idxMin-1; j++) {
      reconstruction_shape[j] = processing_shape[j] * currGain / (1 + (currGain - 1) * processing_shape[j]);
    }

/* ghd: fix reconstruction_shape application for the transition region and for the last line */
    for (j = hrepData->hrepGroupData[sigGroup].idxMinAdj[currCh]-2; j < HREP_HALF_BLOCK_SIZE; j++) {
      fft_out_re[j] *= (1 - reconstruction_shape[j]); /* real */
      fft_out_re[2*HREP_HALF_BLOCK_SIZE-1-j] *= (1 - reconstruction_shape[j+1]); /* real (mirrored spectrum) */
      fft_out_im[j] *= (1 - reconstruction_shape[j]); /* imag */
      fft_out_im[2*HREP_HALF_BLOCK_SIZE-1-j] *= (1 - reconstruction_shape[j+1]); /* imag (mirrored spectrum) */
    }
    
    /* calculate IFFT */
    CFFTN_NI(fft_out_re, fft_out_im, lp_block, lp_block_im, 2*HREP_HALF_BLOCK_SIZE, 1);

    for (j = 0; j < HREP_HALF_BLOCK_SIZE; j++) {
      lp_block[j] *= win[j]; 
      lp_block[2*HREP_HALF_BLOCK_SIZE-1-j] *= win[j];
    }

    /*apply sine window*/
    for (j = 0; j < HREP_HALF_BLOCK_SIZE; j++) {
      fft_in_re[j] *= win[j]; 
      fft_in_re[2*HREP_HALF_BLOCK_SIZE-1-j] *= win[j];
    }

    for (j = 0; j < 2*HREP_HALF_BLOCK_SIZE; j++) {
      lp_block[j] /= 2*HREP_HALF_BLOCK_SIZE;
      hp_block[j] = fft_in_re[j] - lp_block[j];
    }

    interp_factor = lastGain / currGain + currGain / lastGain - 2;
    for (j = 0; j < HREP_HALF_BLOCK_SIZE; j++) {
      interp_corr[j] = 1 + interp_factor * squ_win[j] * (1 - squ_win[j]);
    }

    interp_factor = nextGain / currGain + currGain / nextGain - 2;
    for (j = HREP_HALF_BLOCK_SIZE; j < 2*HREP_HALF_BLOCK_SIZE; j++) {
      interp_corr[j] = 1 + interp_factor * squ_win[2*HREP_HALF_BLOCK_SIZE-1-j] * (1 - squ_win[2*HREP_HALF_BLOCK_SIZE-1-j]);
    }
    
    for (j = 0; j < 2*HREP_HALF_BLOCK_SIZE; j++) {
      proc_data[i*HREP_HALF_BLOCK_SIZE + j] += lp_block[j] + (1.f / compGain) * hp_block[j] / interp_corr[j];
    }
  }

  /* save last gain for processing in next frame */
  hrepData->delayedGains[0][currCh] = currGain;
  hrepData->delayedGains[1][currCh] = nextGain;

  /* give back fully processed samples */
  for (i = 0; i < blockSize; i++) {
    time_samples[i] = proc_data[i];
  }

  /* fill delay buffer */
  for (i = 0; i < HREP_HALF_BLOCK_SIZE; i++) {
    hrepData->delay_buf[i][currCh] = proc_data[blockSize+i];
  }
}

