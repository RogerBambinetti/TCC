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

#include "saoc_drc_lfe.h"

static float pQmfReal[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_NUM_QMF_BANDS] = {{{0}}};
static float pQmfImag[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_NUM_QMF_BANDS] = {{{0}}};

static float pInHybReal[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS] = {{{0}}};
static float pInHybImag[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS] = {{{0}}};

static float pOutHybReal[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS] = {{{0}}};
static float pOutHybImag[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS] = {{{0}}};

float RenMat_Interp[SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];


tSaocDrcLfe *saoc_DrcLfeInterfaceOpen( int isInQmf,
                                       int isOutQmf,
                                       int numInLFEs,
                                       int numOutLFEs,
                                       int nQmfBands)
{
  int ch;

  tSaocDrcLfe *dec = (tSaocDrcLfe*)calloc(1, sizeof(tSaocDrcLfe));

  if (!isInQmf) {
    /* Init QMFlib Analysis */
    QMFlib_InitAnaFilterbank(nQmfBands, 0);
    dec->pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK**) calloc(numInLFEs, sizeof(QMFLIB_POLYPHASE_ANA_FILTERBANK*));
    for (ch = 0; ch <  numInLFEs; ch++) {
      QMFlib_OpenAnaFilterbank(&(dec->pAnalysisQmf[ch]));
    }
  }

  if (!isOutQmf) {
    /* Init QMFLib Synthesis */
    QMFlib_InitSynFilterbank(nQmfBands, 0);
    dec->pSynthesisQmf = (QMFLIB_POLYPHASE_SYN_FILTERBANK**) calloc(numOutLFEs, sizeof(QMFLIB_POLYPHASE_SYN_FILTERBANK*));
    for (ch = 0; ch < numOutLFEs; ch++) {
      QMFlib_OpenSynFilterbank(&(dec->pSynthesisQmf[ch]));
    }
  }

  /* Hybrid filtering init */
  for (ch = 0; ch < numInLFEs; ch++) {
    QMFlib_InitAnaHybFilterbank(&(dec->hybFilterState[ch]));
  }

  return dec;

}

void saoc_DrcLfeInterfaceClose(tSaocDrcLfe *self,
                               int isInQmf,
                               int isOutQmf,
                               int numInLFEs,
                               int numOutLFEs)
{
  int ch;

  if (!isInQmf) {
    for (ch = 0; ch < numInLFEs; ch++) {
      QMFlib_CloseAnaFilterbank(self->pAnalysisQmf[ch]);
    } 
  }
  if (!isOutQmf) {
    for (ch = 0; ch < numOutLFEs; ch++)  {
      QMFlib_CloseSynFilterbank(self->pSynthesisQmf[ch]);
    }
  }
  free(self);
}

void saoc_DrcLfeInterfaceApply(tSaocDec          *dec, 
                               tSaocDrcLfe       *self,
                               int               saocInputLFE_Mapping[SAOC_MAX_LFE_CHANNELS ],
                               int               saocOutputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                               float             pRenderMat_lfe[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_LFE_CHANNELS], 
                               float***          drcMatrix
                               )

{
  int j, ch, ts, qs, k, islot, ihyb, iUpmx, jDmx, ipar;

  int nNumBins;

  int kernels[SAOC_MAX_HYBRID_BANDS][2];

  int paramSet, nParamSets, nParamSetsMod;
  int pParamSlot[SAOC_MAX_PARAM_SETS];
  int repeatLastParamSet = 0;
  int frameLength;

  SAOCFRAME*        pFrameDataSaoc;

  if(!dec->isInQmf) {
    for (ch = 0; ch < dec->numInLFEs; ch++) {
      for (ts = 0; ts < dec->nTimeSlots; ts++) {
        float tmp2QmfBuf[QMFLIB_MAX_NUM_QMF_BANDS] = {0}; 
        for (k=0; k < dec->nQmfBands; k++) {
          tmp2QmfBuf[k] = dec->inBuffer[ch][dec->nQmfBands*ts+k];
        }
        QMFlib_CalculateAnaFilterbank(self->pAnalysisQmf[ch],
                                      tmp2QmfBuf, 
                                      pQmfReal[ch][ts],
                                      pQmfImag[ch][ts],
                                      0);
      }       
    }
  } else { /*if(!dec->isInQmf) { */
    for (ch=0; ch<dec->numInLFEs; ch++) {
      for (ts=0; ts< dec->nTimeSlots; ts++) {
        for (k=0; k< dec->nQmfBands; k++) {
          pQmfReal[ch][ts][k] = dec->qmfInRealBuffer[ch][dec->nQmfBands*ts+k]; 
          pQmfImag[ch][ts][k] = dec->qmfInImagBuffer[ch][dec->nQmfBands*ts+k]; 
        }
      } 
    }
  }/* end of if(!isInQmf) { */


    for (ch = 0; ch < dec->numInLFEs; ch++) {
      for (ts = 0; ts < dec->nTimeSlots; ts++) {        
        QMFlib_ApplyAnaHybFilterbank(&self->hybFilterState[ch], 
                                     dec->hybridMode,
                                     dec->nQmfBands,
                                     dec->hybridHFAlign,
                                     pQmfReal[ch][ts],
                                     pQmfImag[ch][ts],
                                     pInHybReal[ch][ts], 
                                     pInHybImag[ch][ts]); 
      }
    }


    pFrameDataSaoc = GetSAOCFrame(dec->pBsrInstance);
    nParamSets  = pFrameDataSaoc->framingInfo.bsNumParamSets + 1;
    
    pParamSlot[0] = -1;
    for(paramSet = 0; paramSet < nParamSets; paramSet++) {
      pParamSlot[paramSet+1] = pFrameDataSaoc->framingInfo.bsParamSlots[paramSet];  
    }


  nNumBins     = dec->pBsrInstance->saocSpecificConfig.bsNumBins; 
  frameLength = dec->pBsrInstance->saocSpecificConfig.bsFrameLength+1;

  repeatLastParamSet = 0;
  if (pParamSlot[nParamSets] != frameLength - 1) {
    repeatLastParamSet = 1;
  }

  /*  Make Ren Mat frequency dependent and apply DRC sequence */
  for(paramSet=0; paramSet<nParamSets; paramSet++) {
	  for(ch=0; ch<dec->numOutLFEs; ch++) {
		  for(j=0; j<dec->numInLFEs; j++) {
				for(k=0; k<dec->numParamBands; k++) {
					self->RenMat[paramSet+1][k][ch][j] = pRenderMat_lfe[ch][j] * drcMatrix[pParamSlot[paramSet+1]][saocInputLFE_Mapping[j]][k];
				}
			}
		}   
  } 

  /* Interpolate Rendering Matrix */
  SAOC_GetKernels(nNumBins, dec->nHybridBands, 0,  kernels);

  if (pParamSlot[nParamSets] != frameLength - 1){
    nParamSetsMod= nParamSets + 1;
    pParamSlot[nParamSetsMod] = frameLength - 1;
  } else{
    nParamSetsMod = nParamSets;
  }

  saoc_interpolate(dec->numOutLFEs, dec->numInLFEs, nNumBins, nParamSets, nParamSetsMod, dec->nHybridBands, 0, pParamSlot, self->RenMat, RenMat_Interp);
  
  /* Apply Rendering Matrix */
  for (islot = pParamSlot[0]+1; islot <= pParamSlot[nParamSetsMod]; islot++) {
    for (ihyb = 0; ihyb < dec->nHybridBands; ihyb++){
      for (iUpmx=0; iUpmx < dec->numOutLFEs; iUpmx++){
        pOutHybReal[iUpmx][islot][ihyb] = 0.0f;
        pOutHybImag[iUpmx][islot][ihyb] = 0.0f;
        for (jDmx=0; jDmx < dec->numInLFEs; jDmx++){
          pOutHybReal[iUpmx][islot][ihyb] += RenMat_Interp[islot][ihyb][iUpmx][jDmx] * pInHybReal[jDmx][islot][ihyb];
          pOutHybImag[iUpmx][islot][ihyb] += RenMat_Interp[islot][ihyb][iUpmx][jDmx] * pInHybImag[jDmx][islot][ihyb];
        }
      }
    }
  }

  /* hold last parameter position to end of frame: */
  for (ipar = 0; ipar < nNumBins; ipar++) {
    for (ch=0; ch<dec->numOutLFEs; ch++){
      for (j=0; j<dec->numInLFEs; j++){
        self->RenMat[0][ipar][ch][j]  = self->RenMat[nParamSets][ipar][ch][j];
      }
    }
  }

  for (ch = 0; ch < dec->numOutLFEs; ch++) {
    for (ts = 0; ts < dec->nTimeSlots; ts++) {         
      QMFlib_ApplySynHybFilterbank(dec->nQmfBands,
                                   dec->hybridMode,
                                   pOutHybReal[ch][ts],
                                   pOutHybImag[ch][ts],
                                   pQmfReal[ch][ts], 
                                   pQmfImag[ch][ts]); 
    }
  }


  if(!dec->isOutQmf){
    for (ch = 0; ch < dec->numOutLFEs; ch++) {
      for (ts = 0; ts < dec->nTimeSlots; ts++) {
        float tmpQmfBuf[QMFLIB_MAX_NUM_QMF_BANDS] = {0.0f}; 
        QMFlib_CalculateSynFilterbank(self->pSynthesisQmf[ch],
                                      pQmfReal[ch][ts],
                                      pQmfImag[ch][ts],
                                      tmpQmfBuf, /* &outBuffer[ch][nQmfBands*ts], */
                                      0);
        for (qs=0; qs < dec->nQmfBands; qs++) {
          dec->outBuffer[ saocOutputLFE_Mapping[ch] ][dec->nQmfBands*ts+qs] = tmpQmfBuf[qs];
        }
      }
    }
  }
  else {
    for (ch = 0; ch < dec->numOutLFEs; ch++) {
      for (ts = 0; ts < dec->nTimeSlots; ts++) {
        for (qs=0; qs < dec->nQmfBands; qs++) {
          dec->qmfOutRealBuffer[ saocOutputLFE_Mapping[ch] ][dec->nQmfBands*ts+qs] = pQmfReal[ch][ts][qs];
          dec->qmfOutImagBuffer[ saocOutputLFE_Mapping[ch] ][dec->nQmfBands*ts+qs] = pQmfImag[ch][ts][qs];
        }
      }
    }
  }
}