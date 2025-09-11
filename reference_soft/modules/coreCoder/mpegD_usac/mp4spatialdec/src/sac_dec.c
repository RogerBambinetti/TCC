/*******************************************************************************
This software module was originally developed by

Agere Systems, Coding Technologies, Fraunhofer IIS, Philips

and edited by

-

in the course of development of ISO/IEC 23003 for reference purposes and its
performance may not have been optimized. This software module is an
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives
You a royalty-free, worldwide, non-exclusive, copyright license to copy,
distribute, and make derivative works of this software module or modifications
thereof for use in implementations of ISO/IEC 23003 in products that satisfy
conformance criteria (if any). Those intending to use this software module in
products are advised that its use may infringe existing patents. ISO/IEC have no
liability for use of this software module or modifications thereof. Copyright is
not released for products that do not conform to audiovisual and image-coding
related ITU Recommendations and/or ISO/IEC International Standards.

Agere Systems, Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2008.
*******************************************************************************/




#include "sac_dec.h"
#include "sac_process.h"
#include "sac_bitdec.h"
#include "sac_decor.h"
#include "sac_hybfilter.h"
#include "sac_types.h"
#include "sac_smoothing.h"
#include "sac_tonality.h"
#include "sac_TPprocess.h"
#include "sac_reshapeBBEnv.h"
#include "sac_nlc_dec.h"
#include "sac_mdct2qmf.h"

#include "sac_blind.h"

#include "sac_calcM1andM2.h"
#include "sac_hrtf.h"
#include "sac_parallelReverb.h"

#include "math.h"
#include "common_m4a.h"             /* common module */

#include <assert.h>
#include <string.h>

#define DECORR_FIX

/* originally declared in ct_sbrdec.h */
extern void
sbr_dec_from_mps(int channel, int el,
  float qmfOutputReal[][MAX_NUM_QMF_BANDS],
  float qmfOutputImag[][MAX_NUM_QMF_BANDS]);




#ifdef PARTIALLY_COMPLEX
void Cos2Exp ( const float prData[][PC_NUM_BANDS], float *piData, int nDim );
void QmfAlias( spatialDec  *self );
#endif


void SpatialDecDisableDecorrelators(spatialDec * self, int bDisable){
 
  if(self){
   self->disableDecorrelators = bDisable;
 }
}

int SpatialDecGetNumOutputChannels(spatialDec *self){

  int nChannels = 0;

  if(self){
    nChannels = self->numOutputChannels;
  }

  return nChannels;
}

int SpatialDecGetNumTimeSlots(spatialDec *self){

  int nTimeSlots = 0;

  if(self){
    nTimeSlots = self->timeSlots;
  }

  return nTimeSlots;
}

spatialDec* SpatialDecOpen(HANDLE_BYTE_READER bitstream,
                           char* spatialPcmOutFileName,
                           int samplingFreq,
                           int nInChannels,
                           int* pTimeSlots,
                           int nQmfBands,
                           int bsFrameLength,
                           int bsResidualCoding,
                           int nDecType,
                           int nUpmixType,
                           int nBinauralQuality, 
                           int bStandaloneSsc, 
                           int nBitsAvailable, 
                           int *pnBitsRead, 
                           SAC_DEC_USAC_MPS212_CONFIG *pUsacMps212Config
                           ) {


  int nCh, i, j, k, sacHeaderLen;

  spatialDec* self =0;
  self = (spatialDec*) calloc (sizeof(spatialDec),1);

  if (self==0) return 0;
  self->bitstream =0;
#ifdef USE_AFSP
  self->outputFile =0;
#endif

  self->samplingFreq = samplingFreq;

  self->numParameterSets = 1;

  self->decType = nDecType;

  self->upmixType = nUpmixType;

  self->qmfBands = nQmfBands;

#ifdef HRTF_DYNAMIC_UPDATE
  self->hrtfSource = 0;
#endif 

  self->binauralQuality = nBinauralQuality;

  if ( (self->upmixType != 1) && (pUsacMps212Config == NULL) ) {
    self->bitstream = s_bitinput_open(bitstream);
    if (self->bitstream ==0) {
        SpatialDecClose(self);
        return 0;
    }
  }

  

  if (self->upmixType != 1) {
    if(!bStandaloneSsc){
      self->sacTimeAlignFlag = s_GetBits(self->bitstream, 1);

      sacHeaderLen = s_GetBits(self->bitstream, 7);
      if (sacHeaderLen == 127) {
        sacHeaderLen += s_GetBits(self->bitstream, 16);
      }
    } else {
      sacHeaderLen = (nBitsAvailable + 7)/8;
    }

    if(pUsacMps212Config){
      GetUsacMps212SpecificConfig(self, bsFrameLength, bsResidualCoding, pUsacMps212Config);
    } else {
      SpatialDecParseSpecificConfig(self, sacHeaderLen, pnBitsRead, bsFrameLength, bsResidualCoding);
    }

    /*  s_byteAlign(self->bitstream); */
    
    if (self->sacTimeAlignFlag) {
      self->sacTimeAlign = s_GetBits(self->bitstream, 16);
      if (self->sacTimeAlign != 0) {
        CommonExit(1,"ERROR: sacTimeAlign != 0 not implemented\n");
      }
    }
  }
  else {
    SpatialDecDefaultSpecificConfig(self);
  }

  
  for(i=0; i<MAX_OUTPUT_CHANNELS_AT; i++)
    for(j=0; j<MAX_PARAMETER_BANDS; j++)
      self->prevGainAT[i][j] = 1.0f;

  SpatialDecDecodeHeader(self);

  if (self->residualCoding) {
    for (nCh = 0; nCh < self->numOttBoxes + self->numTttBoxes; nCh++) {
      if (self->resBands[nCh] > 0) {
        self->numResChannels++;
      }
    }
    if (nInChannels == (self->numInputChannels + self->numResChannels)) {
      self->coreCodedResidual = 1;
    }
  }
  else {
    assert(nInChannels == self->numInputChannels);
  }

  assert((nQmfBands == 32)||(nQmfBands == 64)||(nQmfBands == 128));


  *pTimeSlots = self->timeSlots;

  

  SpatialDecInitResidual(self);
  SPACE_MDCT2QMF_Create(self);

  
  initSpatialTonality(self);
  initParameterSmoothing(self);

  

  SacInitBBEnv(self);

 
  SacInitSynFilterbank(NULL, self->qmfBands);
  
  for (nCh = 0; nCh < self->numOutputChannelsAT; nCh++) {
    SacOpenSynFilterbank(&self->qmfFilterState[nCh]);
  }

  
  for (nCh = 0; nCh < self->numInputChannels; nCh++) {
    SacInitAnaHybFilterbank(&self->hybFilterState[nCh]);
  }

  if (self->residualCoding) {
    int offset = self->numInputChannels;
    for (nCh = 0; nCh < self->numOttBoxes + self->numTttBoxes; nCh++) {
      if (self->resBands[nCh] > 0) {
        SacInitAnaHybFilterbank(&self->hybFilterState[offset + nCh]);
      }
    }
  }

  if (self->arbitraryDownmix == 2) {
    int offset = self->numInputChannels + self->numOttBoxes + self->numTttBoxes;
    for (nCh = 0; nCh < self->numInputChannels; nCh++) {
      SacInitAnaHybFilterbank(&self->hybFilterState[offset + nCh]);
    }
  }

  for (nCh=0; nCh<self->numOutputChannelsAT; nCh++)
  {
    SacInitSynHybFilterbank();
  }

#ifdef PARTIALLY_COMPLEX
  {
    int seedVec[MAX_NO_DECORR_CHANNELS];
    int psIdx = 0;

    for (k=0; k<MAX_NO_DECORR_CHANNELS; k++){
      seedVec[k] = k;
    }

	if (self->treeConfig == TREE_212){
      seedVec[0] = 0; 
      psIdx = 0;
    }
    if (self->treeConfig == TREE_5151){
      seedVec[0] = 0;
      seedVec[1] = 1;
      seedVec[2] = 2;
      seedVec[3] = 2;
      psIdx = 2;
#ifdef DECORR_FIX
      if (self->upmixType == 2) { 
        psIdx = 0;
      }
#endif
    }
    if (self->treeConfig == TREE_5152){
      seedVec[0] = 0;
      seedVec[1] = 2;
      seedVec[2] = 1;
      seedVec[3] = 1;
      psIdx = 2;
#ifdef DECORR_FIX
      if (self->upmixType == 2) {
        psIdx = 0;
      }
#endif
    }
    if (self->treeConfig == TREE_525){
      seedVec[0] = 0;
      seedVec[1] = 0;
      psIdx = 0;
    }
    if (self->treeConfig == TREE_7271){
      seedVec[0] = 1;
      seedVec[1] = 1;
      seedVec[2] = 2;
      seedVec[3] = 0;
      seedVec[4] = 0;
      psIdx = 1;
    }
    if (self->treeConfig == TREE_7272){
      seedVec[0] = 1;
      seedVec[1] = 1;
      seedVec[2] = 2;
      seedVec[3] = 0;
      seedVec[4] = 0;
      psIdx = 1;
    }
    if (self->treeConfig == TREE_7571){
      seedVec[0] = 0;
      seedVec[1] = 0;
      psIdx = 0;
    }
    if (self->treeConfig == TREE_7572){
      seedVec[0] = 0;
      seedVec[1] = 0;
      psIdx = 0;
    }


    for (k=0; k<MAX_NO_DECORR_CHANNELS; k++)
    {
      int errCode;

      errCode = SpatialDecDecorrelateCreate(  &(self->apDecor[k]),
                                                  self->hybridBands,
                                                  seedVec[k],
                                                  self->decType,
                                                  seedVec[k] == psIdx,
                                                  self->decorrConfig,
                                                  self->treeConfig
                                               );
      if (errCode != ERR_NONE) break;
    }
  }

#else

  for (k=0; k<MAX_NO_DECORR_CHANNELS; k++)
  {
    int errCode, idec;

#ifdef DECORR_FIX
    if (self->upmixType == 3) {
      idec = 0;
    }
    else {
      idec = k;
    }
#else
    idec = k;
#endif

    errCode = SpatialDecDecorrelateCreate(  &(self->apDecor[k]),
                                                self->hybridBands,
                                                idec,
                                                self->decType,
                                                0,
                                                self->decorrConfig,
                                                self->treeConfig
                                             );
    if (errCode != ERR_NONE) break;
  }
#endif

  if (self->upmixType == 1) {
    SpatialDecInitBlind(self);
  }

#ifdef USE_AFSP
  if (strlen(spatialPcmOutFileName) > 0) {
    self->outputFile = AFopnWrite(spatialPcmOutFileName,
                                  2, 
                                  5, 
                                  self->numOutputChannelsAT,
                                  (double)self->samplingFreq,
                                  0);
  }
#endif



  SpatialDecInitM1andM2(self, 0);


  
  BinauralInitParallelReverb(self);


  
  /*self->parseNextBitstreamFrame=1;*/


  
  for(k = 0; k < 2; k++) {

    SPATIAL_BS_FRAME bsFrame = self->bsFrame[k];

    for(i = 0; i < MAX_NUM_OTT; i++) {
      for(j = 0; j < MAX_PARAMETER_BANDS; j++) {
        bsFrame.ottCLDidxPrev       [i][j] = 0;
        bsFrame.ottICCidxPrev       [i][j] = 0;
        bsFrame.cmpOttCLDidxPrev    [i][j] = 0;
        bsFrame.cmpOttICCidxPrev    [i][j] = 0;
#ifdef VERIFICATION_TEST_COMPATIBILITY
        bsFrame.cmpOttICCtempidxPrev[i][j] = 0;
#endif
      }
    }
    for(i = 0; i < MAX_NUM_TTT; i++) {
      for(j = 0; j < MAX_PARAMETER_BANDS; j++) {
        bsFrame.tttCPC1idxPrev   [i][j] = 0;
        bsFrame.tttCPC2idxPrev   [i][j] = 0;
        bsFrame.tttCLD1idxPrev   [i][j] = 0;
        bsFrame.tttCLD2idxPrev   [i][j] = 0;
        bsFrame.tttICCidxPrev    [i][j] = 0;
        bsFrame.cmpTttCPC1idxPrev[i][j] = 0;
        bsFrame.cmpTttCPC2idxPrev[i][j] = 0;
        bsFrame.cmpTttCLD1idxPrev[i][j] = 0;
        bsFrame.cmpTttCLD2idxPrev[i][j] = 0;
        bsFrame.cmpTttICCidxPrev [i][j] = 0;
      }
    }
    for(i = 0; i < MAX_INPUT_CHANNELS; i++) {
      for(j = 0; j < MAX_PARAMETER_BANDS; j++) {
        bsFrame.arbdmxGainIdxPrev[i][j]    = 0;
        bsFrame.cmpArbdmxGainIdxPrev[i][j] = 0;
      }
    }

  }

  
  return self;
}



void SpatialDecClose(spatialDec* self) {
  int k;

  if (self) {
    for (k = 0; k < self->numOutputChannelsAT; k++) {
      SacCloseSynFilterbank(self->qmfFilterState[k]);
    }

    SacCloseAnaHybFilterbank(); 
    SacCloseSynHybFilterbank();

    for (k = 0; k < MAX_NO_DECORR_CHANNELS; k++) {
      SpatialDecDecorrelateDestroy(&self->apDecor[k]);
    }

    if (self->upmixType == 1) {
      SpatialDecCloseBlind(self);
    }

    if (self->upmixType == 2) {
      SpatialDecCloseHrtfFilterbank(self->hrtfFilter);

#ifdef HRTF_DYNAMIC_UPDATE
      if (self->hrtfSource != NULL) {
        free((HRTF_DATA*) self->hrtfData);
      }
#endif 
    }

  BinauralDestroyParallelReverb(self);

#ifdef USE_AFSP
    if (self->outputFile) {
      AFclose(self->outputFile);
    }
#endif
    if (self->bitstream) {
      s_bitinput_close(self->bitstream);
    }
    free(self);
  }
  return;
}




void SpatialDecApplyFrame(spatialDec* self,
                          int inTimeSlots,
                          float** pointers[4],
                          float** ppTimeOut, 
                          float   scalingFactor,
                          int     el,
                          int     qceOn,
                          int     receiverDelayCompensation
                          )
{
  float *pcmOutBuf = malloc(MAX_OUTPUT_CHANNELS*MAX_TIME_SLOTS*MAX_NUM_QMF_BANDS*sizeof(float));
  int ch,ts,sam,qs,n;

  assert(pcmOutBuf != NULL);
  assert(self->curTimeSlot+inTimeSlots <= self->timeSlots);


#ifdef PARTIALLY_COMPLEX
  
  for (ts = 0; ts < inTimeSlots; ts++) {
    const float invSqrt2 = (float) (1.0 / sqrt(2.0));

    for (ch = 0; ch < self->numInputChannels; ch++) {
      for (qs = 0; qs < PC_NUM_BANDS; qs++) {
        for (n = 0; n < (PC_FILTERLENGTH - 1); n++) {
          self->qmfCos2ExpInput[ch][n][qs] = self->qmfCos2ExpInput[ch][n + 1][qs];
        }
        self->qmfCos2ExpInput[ch][PC_FILTERLENGTH - 1][qs] = pointers[ch][ts][qs];
      }

      Cos2Exp(self->qmfCos2ExpInput[ch], self->qmfInputImag[ch][self->curTimeSlot+ts], PC_NUM_BANDS);

      for (qs = 0; qs < PC_NUM_BANDS; qs++) {
        self->qmfInputReal[ch][self->curTimeSlot+ts][qs]  = invSqrt2 * self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs];
        self->qmfInputImag[ch][self->curTimeSlot+ts][qs] *= invSqrt2;

        self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs] = pointers[ch][ts][qs];
      }

      for (qs = PC_NUM_BANDS; qs < self->qmfBands; qs++) {
        self->qmfInputReal[ch][self->curTimeSlot+ts][qs] = self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs];
        self->qmfInputImag[ch][self->curTimeSlot+ts][qs] = 0;

        self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs] = pointers[ch][ts][qs];
      }
    }

    self->qmfInputDelayIndex++;

    if (self->qmfInputDelayIndex == PC_FILTERDELAY) {
      self->qmfInputDelayIndex = 0;
    }
  }
#else
  
  if (self->upmixType == 1) {
    for (ts = 0; ts < inTimeSlots; ts++) {
      for (ch = 0; ch < self->numInputChannels; ch++) {
        for (qs = 0; qs < self->qmfBands; qs++) {
          self->qmfInputReal[ch][self->curTimeSlot+ts][qs] = pointers[2*ch][ts][qs];
          self->qmfInputImag[ch][self->curTimeSlot+ts][qs] = pointers[2*ch+1][ts][qs];
        }
      }
    }
  }
  else {
    int numInputChannels = self->numInputChannels;
    if (self->coreCodedResidual) {
      numInputChannels += self->numResChannels;
    }

#ifdef MPS212_DECODING_DELAY_NOFIX
    for (ts = 0; ts < inTimeSlots; ts++) {
      for (ch = 0; ch < numInputChannels; ch++) {
        for (qs = 0; qs < self->qmfBands; qs++) {
          self->qmfInputReal[ch][self->curTimeSlot+ts][qs] = self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs];
          self->qmfInputImag[ch][self->curTimeSlot+ts][qs] = self->qmfInputDelayImag[ch][self->qmfInputDelayIndex][qs];

          self->qmfInputDelayReal[ch][self->qmfInputDelayIndex][qs] = self->clipProtectGain * pointers[2*ch][ts][qs];
          self->qmfInputDelayImag[ch][self->qmfInputDelayIndex][qs] = self->clipProtectGain * pointers[2*ch+1][ts][qs];
        }
      }

      self->qmfInputDelayIndex++;

      if (self->qmfInputDelayIndex == PC_FILTERDELAY) {
        self->qmfInputDelayIndex = 0;
      }
    }
#else /* MPS212_DECODING_DELAY_NOFIX */
    for (ts = 0; ts < inTimeSlots; ts++) {
      for (ch = 0; ch < numInputChannels; ch++) {
        for (qs = 0; qs < self->qmfBands; qs++) {
          self->qmfInputReal[ch][self->curTimeSlot+ts][qs] = self->clipProtectGain * pointers[2*ch][ts][qs]; 
          self->qmfInputImag[ch][self->curTimeSlot+ts][qs] = self->clipProtectGain * pointers[2*ch+1][ts][qs];      
        }    
      }  
    }
#endif /* MPS212_DECODING_DELAY_NOFIX */
  }

#endif

  self->curTimeSlot += inTimeSlots;

  
  if (self->curTimeSlot < self->timeSlots)
    return;

  self->curTimeSlot = 0;

  SpatialDecDecodeFrame(self);

  if (self->coreCodedResidual) {
    SpatialDecCopyPcmResidual(self);
  }
  else {
    SpatialDecMDCT2QMF(self);
  }

  SpatialDecHybridQMFAnalysis(self);

  if (self->upmixType == 1) {
    SpatialDecApplyBlind(self);
  }

#ifdef PARTIALLY_COMPLEX
  QmfAlias(self);
#endif

#ifdef   RM6_INTERNAL_CHANNEL
  if    ( ICConfig.isICON == IC_PRE_STR )
  {
    /* stereo using pre-applied internal channel gain */
    SpatialDecCopyXtoY(self);          /* NO CHANGE */
  }
  else if  ( ICConfig.isICON == IC_POST_STR )
  {
    /* stereo using post-applied internal channel gain */
    SpatialDecCreateX(self);           /* NO CHANGE */
    SpatialDecCalculateM1andM2(self);  /* NO CHANGE  - changed for PRE_STR */
    SpatialDecSmoothM1andM2(self);     /* NO CHANGE */
    SpatialDecApplyM1(self);           /* NO CHANGE */
    SpatialDecCreateW(self);           /* NO CHANGE */
    SpatialDecApplyM2_IC(self);        /* CHANGED */
  }
  else if  ( ICConfig.isICON == IC_PRE_MUL )
  {
    /* stereo using post-applied internal channel gain */
    SpatialDecCreateX(self);              /* NO CHANGE */
    SpatialDecCalculateM1andM2(self);     /* NO CHANGE  - changed for PRE_STR */
    SpatialDecSmoothM1andM2(self);        /* NO CHANGE */
    SpatialDecApplyM1(self);              /* NO CHANGE */
    SpatialDecCreateW(self);              /* NO CHANGE */
    SpatialDecApplyM2_InvIC(self);        /* CHANGED */
  }
  else
#endif
  {
  SpatialDecCreateX(self);

  SpatialDecCalculateM1andM2(self);
  SpatialDecSmoothM1andM2(self);

  SpatialDecApplyM1(self);

  
  SpatialDecCreateW(self);

  SpatialDecApplyM2(self);
  }

  if (self->upmixType != 2) {
    
    if (self->tempShapeConfig == 2) {
      spatialDecReshapeBBEnv(self);
    }
  }

  if(qceOn) {

    tpProcess1(self);

  }
  else{


    tpProcess1(self);


    for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
      sbr_dec_from_mps(ch, el, self->qmfOutputRealDry[ch], self->qmfOutputImagDry[ch]);
    }

#ifdef RM6_CONSTANT_DELAY_ELEMENTALIGNMENT

    if ( receiverDelayCompensation ) {
      /* qmf delay line for SBR w/o MPS */
      int outputQmfDelay = HYBRID_FILTER_DELAY;
      int ts = 0, k = 0;
      int numQmfBands = 64;
      int startPointQmfToBuffer = self->timeSlots - HYBRID_FILTER_DELAY;
      float tmpQmfBufferReal[MAX_OUTPUT_CHANNELS][MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS] = {{{ 0.f }}};
      float tmpQmfBufferImag[MAX_OUTPUT_CHANNELS][MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS] = {{{ 0.f }}};

      /* store the complete current qmf buffer */
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for (ts = 0; ts < self->timeSlots; ts++) {
          for (k = 0; k < self->qmfBands; k++) {
            tmpQmfBufferReal[ch][ts][k] = self->qmfOutputRealDry[ch][ts][k];
            tmpQmfBufferImag[ch][ts][k] = self->qmfOutputImagDry[ch][ts][k];
          }
        }
      }

      /* use the qmfoutDelay buffers to get the buffered values */
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for ( ts = 0; ts < HYBRID_FILTER_DELAY; ++ts ) {
          for ( k = 0; k < self->qmfBands; k++ ) { 
            self->qmfOutputRealDry[ch][ts][k] = self->qmfoutDelayReal[ch][ts][k];
            self->qmfOutputImagDry[ch][ts][k] = self->qmfoutDelayImag[ch][ts][k];
          }
        }
      }

      /* use the tmpQmf buffers to get the actual frame content */
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for (ts = HYBRID_FILTER_DELAY; ts < self->timeSlots; ts++) {
          for ( k = 0; k < self->qmfBands; k++ ) { 
            self->qmfOutputRealDry[ch][ts][k] = tmpQmfBufferReal[ch][ts-HYBRID_FILTER_DELAY][k];
            self->qmfOutputImagDry[ch][ts][k] = tmpQmfBufferImag[ch][ts-HYBRID_FILTER_DELAY][k];
          }
        }
      }

      /* write the remaining time slots in the qmfoutDelay buffers */
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for ( ts = 0; ts < HYBRID_FILTER_DELAY; ++ts ) {
          for ( k = 0; k <  self->qmfBands; k++ ) {
            self->qmfoutDelayReal[ch][ts][k] = tmpQmfBufferReal[ch][startPointQmfToBuffer+ts][k];
            self->qmfoutDelayImag[ch][ts][k] = tmpQmfBufferImag[ch][startPointQmfToBuffer+ts][k];
          }
        }
      }
    }
#endif /* RM6_CONSTANT_DELAY_ELEMENTALIGNMENT */
  
#ifdef RM6_INTERNAL_CHANNEL
    if ( ICConfig.isICON == IC_OFF || ICConfig.isICON == IC_POST_MUL)
#endif
    {
      tpProcess2(self);
    } 
#ifdef RM6_INTERNAL_CHANNEL
    else 
    {
      if (!ICConfig.isQMFOut)
      {
  tpProcess2(self);
      }
    }
#endif

  
  /* return qmf buffers to higher layer */
    for (ts = 0; ts < self->timeSlots; ts++) {
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for (qs = 0; qs < self->qmfBands; qs++) {
          pointers[2*ch][ts][qs] = self->qmfOutputRealDry[ch][ts][qs];
          pointers[2*ch+1][ts][qs] = self->qmfOutputImagDry[ch][ts][qs];
        }    
      }  
    }

  if(!self->bsConfig.arbitraryTree && self->upmixType != 2 && self->upmixType != 3) {
    
    for (n=0; n< self->frameLength; n++) {
      self->timeOut[3][n] *= self->lfeGain;       
      self->timeOut[4][n] *= self->surroundGain;  
      self->timeOut[5][n] *= self->surroundGain;  
    
      if(self->treeConfig == 4 || self->treeConfig == 6){
        self->timeOut[6][n] *= self->surroundGain;  
        self->timeOut[7][n] *= self->surroundGain;  
      }
    }
  }

  BinauralApplyParallelReverb(self);

  

  for (ch=0; ch< self->numOutputChannelsAT; ch++) {
    for (sam=0; sam<self->frameLength; sam++) {
      pcmOutBuf[sam*self->numOutputChannelsAT + ch] =
        (float)(self->timeOut[ch][sam] / scalingFactor); 
    }
  }

  SpatialDecUpdateBuffers(self);

#ifdef USE_AFSP
  if(self->outputFile){
    AFfWriteData(self->outputFile, pcmOutBuf,self->frameLength*self->numOutputChannelsAT);
  } 
#endif
  if (ppTimeOut){
    for (ch=0; ch< self->numOutputChannelsAT; ch++) {
      for (sam=0; sam<self->frameLength; sam++) {
        ppTimeOut[ch][sam] = pcmOutBuf[sam*self->numOutputChannelsAT + ch];
      }
    }
  }

  /*self->parseNextBitstreamFrame=1;*/
  }

  free(pcmOutBuf);
}

void SpatialDecPostProcessFrame(spatialDec* self,
                                float** ppTimeOut,
                                float** pointers[4],
                                float   scalingFactor) 
{
  int n = 0, ch, sam, qs, ts;

  float pcmOutBuf[MAX_OUTPUT_CHANNELS*MAX_TIME_SLOTS*MAX_NUM_QMF_BANDS];


  tpProcess2(self);

    /* return qmf buffers to higher layer */
    for (ts = 0; ts < self->timeSlots; ts++) {
      for (ch = 0; ch < self->numOutputChannelsAT; ch++) {
        for (qs = 0; qs < self->qmfBands; qs++) {
          pointers[2*ch][ts][qs] = self->qmfOutputRealDry[ch][ts][qs];
          pointers[2*ch+1][ts][qs] = self->qmfOutputImagDry[ch][ts][qs];
        }    
      }  
    }

  if(self->treeConfig != 7 && !self->bsConfig.arbitraryTree && self->upmixType != 2 && self->upmixType != 3) {
    
    for (n=0; n< self->frameLength; n++) {
      self->timeOut[3][n] *= self->lfeGain;       
      self->timeOut[4][n] *= self->surroundGain;  
      self->timeOut[5][n] *= self->surroundGain;  
    
      if(self->treeConfig == 4 || self->treeConfig == 6){
        self->timeOut[6][n] *= self->surroundGain;  
        self->timeOut[7][n] *= self->surroundGain;  
      }
    }
  }

  BinauralApplyParallelReverb(self);

  

  for (ch=0; ch< self->numOutputChannelsAT; ch++) {
    for (sam=0; sam<self->frameLength; sam++) {
      pcmOutBuf[sam*self->numOutputChannelsAT + ch] =
        (float)(self->timeOut[ch][sam] / scalingFactor); 
    }
  }

  SpatialDecUpdateBuffers(self);

#ifdef USE_AFSP
  /* ndr: removed. to use this, you have to link libtsp */
  if(self->outputFile){
    AFfWriteData(self->outputFile, pcmOutBuf,self->frameLength*self->numOutputChannelsAT);
  } 
#endif

  if (ppTimeOut){
    for (ch=0; ch< self->numOutputChannelsAT; ch++) {
      for (sam=0; sam<self->frameLength; sam++) {
        ppTimeOut[ch][sam] = pcmOutBuf[sam*self->numOutputChannelsAT + ch];
      }
    }
  }

  /*self->parseNextBitstreamFrame=1;*/
}



float SpatialGetClipProtectGain(spatialDec* self) {
  return self->clipProtectGain;
}

int SpatialGetQmfBands(spatialDec* self) {
  int qmfBands = 0;

  if(self){
    qmfBands = self->qmfBands;
  }
  
  return qmfBands;
}


void SpatialDecResortDataSbr(spatialDec* self1, spatialDec* self2)
{
  int i = 0;
  float qmfOutputRealDry_tmp[MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS];
  float qmfOutputImagDry_tmp[MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS];

  assert (2 == self1->numOutputChannelsAT ); 
  assert (2 == self2->numOutputChannelsAT );


  for ( i = 0; i < MAX_TIME_SLOTS; ++i )
    {

      /* Save second channel of first element */
      memcpy(qmfOutputRealDry_tmp[i], self1->qmfOutputRealDry[1][i] , sizeof(float) * MAX_NUM_QMF_BANDS);
      memcpy(qmfOutputImagDry_tmp[i], self1->qmfOutputImagDry[1][i] , sizeof(float) * MAX_NUM_QMF_BANDS);


      /* Copy first channel of second element into second channel of first element */ 
      memcpy(self1->qmfOutputRealDry[1][i], self2->qmfOutputRealDry[0][i] ,  sizeof(float) * MAX_NUM_QMF_BANDS);
      memcpy(self1->qmfOutputImagDry[1][i], self2->qmfOutputImagDry[0][i] ,  sizeof(float) * MAX_NUM_QMF_BANDS);
  
      /* Copy saved second channel of first element into first channel of second element */

      memcpy(self2->qmfOutputRealDry[0][i], qmfOutputRealDry_tmp[i],  sizeof(float) * MAX_NUM_QMF_BANDS);
      memcpy(self2->qmfOutputImagDry[0][i], qmfOutputImagDry_tmp[i],  sizeof(float) * MAX_NUM_QMF_BANDS);

    }
}

void SpatialDecResortQmfOutData(float *** mpsQmfBuffer, spatialDec* self)
{
  int i = 0;
  float qmfOutputRealDry_tmp[MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS];
  float qmfOutputImagDry_tmp[MAX_TIME_SLOTS][MAX_NUM_QMF_BANDS];
  const int numQmfBands = self->qmfBands;
  const int numTimeslots = self->timeSlots;

  for ( i = 0; i < numTimeslots; ++i )
    {

      /* Save second channel of first element */
      memcpy(qmfOutputRealDry_tmp[i], mpsQmfBuffer[2][i] , sizeof(float) * numQmfBands);
      memcpy(qmfOutputImagDry_tmp[i], mpsQmfBuffer[3][i] , sizeof(float) * numQmfBands);


      /* Copy first channel of second element into second channel of first element */ 
      memcpy(mpsQmfBuffer[2][i], mpsQmfBuffer[4][i] ,  sizeof(float) * numQmfBands);
      memcpy(mpsQmfBuffer[3][i], mpsQmfBuffer[5][i] ,  sizeof(float) * numQmfBands);
  
      /* Copy saved second channel of first element into first channel of second element */

      memcpy(mpsQmfBuffer[4][i], qmfOutputRealDry_tmp[i],  sizeof(float) * numQmfBands);
      memcpy(mpsQmfBuffer[5][i], qmfOutputImagDry_tmp[i],  sizeof(float) * numQmfBands);

    }
}
