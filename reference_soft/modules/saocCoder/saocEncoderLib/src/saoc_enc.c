/*******************************************************************************
This software module was originally developed by

Coding Technologies, Fraunhofer IIS, Philips

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

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:
Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Coding Technologies, Fraunhofer IIS,
Philips own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Coding Technologies, Fraunhofer
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

#include <assert.h>

#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "saoc_enc.h"
#include "saoc_bitstream.h"

static int kernels_20[MAX_HYBRID_BANDS] = {
    1,
    0,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    14,
    15,
    15,
    15,
    16,
    16,
    16,
    16,
    17,
    17,
    17,
    17,
    17,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19
};


static void CalcOldIoc(int slots, 
					   int nChannels, 
					   float mHybridReal[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][MAX_HYBRID_BANDS], 
					   float mHybridImag[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][MAX_HYBRID_BANDS], 
					   float pOld[MAX_OBJECTS][MAX_PARAM_BANDS], 
					   float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS],
					   float pMax[MAX_PARAM_BANDS])
{
  int i,j,k,l;
  int sumHb=0;

  float pPow[MAX_NUM_INPUTCHANNELS][MAX_NUM_INPUTCHANNELS][MAX_HYBRID_BANDS];
  float pPowParBand[MAX_NUM_INPUTCHANNELS][MAX_NUM_INPUTCHANNELS][PARAMETER_BANDS];

  memset(pPow,0,sizeof(pPow));
  memset(pPowParBand,0,sizeof(pPowParBand));
  memset(pMax,0,sizeof(pMax));

  for(l = 0; l < nChannels; l++)
  {
    for(k = 0; k < nChannels; k++)
    {
      for(i = 0; i < slots; i++)
      {
        for(j = 0; j < MAX_HYBRID_BANDS; j++)
        {
          pPow[l][k][j] += mHybridReal[l][i][j]*mHybridReal[k][i][j]+mHybridImag[l][i][j]*mHybridImag[k][i][j];
        }
      }
    }
  }

  for(l = 0; l < nChannels; l++)
  {
    for(k = 0; k < nChannels; k++)
    {
      for(i=0;i<MAX_HYBRID_BANDS;i++)
      {
        pPowParBand[l][k][kernels_20[i]] += pPow[l][k][i];
      }
    }
  }

  /* find max */

  for(i=0;i<PARAMETER_BANDS;i++)
  {
    for(k = 0; k < nChannels; k++)
    {
      if(pPowParBand[k][k][i]>pMax[i])
      {
        pMax[i]=pPowParBand[k][k][i];
      }
    }
  }

  for(i=0;i<PARAMETER_BANDS;i++)
  {
    for(k = 0; k < nChannels; k++)
    {
      pOld[k][i]=(pPowParBand[k][k][i]+EPSILON)/(pMax[i]+EPSILON);
    }
  }

  for(i=0;i<PARAMETER_BANDS;i++)
  {
    for(k = 0; k < nChannels; k++)
    {
      for(l = 0; l < nChannels; l++) 
      {
        pIoc[k][l][i]=(pPowParBand[k][l][i]+EPSILON)/(float)(sqrt(pPowParBand[k][k][i]*pPowParBand[l][l][i]+EPSILON));
      }
    }
  }

  /* absolute energy */
  for (i=0;i<PARAMETER_BANDS;i++)
  {
	  sumHb=0;
	  for (j=0;j<MAX_HYBRID_BANDS;j++)
	  {
		  if (kernels_20[j]==i)
			  sumHb+=1;
	  }
	  pMax[i]/=(slots*sumHb);
  }
}

tSpatialEnc *SaocEncOpen(int nObjects,
                         int bsRelatedTo[MAX_OBJECTS][MAX_OBJECTS],
                         int nDmxCh,
                         int timeSlots,
                         int coreTimeSlots,
                         int transAbsNrg,
                         int *bufferSize,
                         Stream *bitstream,
                         QMFLIB_HYBRID_FILTER_MODE hybridMode,
                         int LdMode,
                         int numQMFBands,
                         int bQmfInput,
                         unsigned int n3DAChannels,
                         unsigned int n3DAObjects,
                         unsigned int n3DALFEs,
                         unsigned int n3DAdmxChannels,
                         unsigned int n3DAdmxObjects)
{
  int i;
  SAOCSPECIFICCONFIG *pConfig;
  tSpatialEnc *enc = calloc(1, sizeof(tSpatialEnc));

  enc->timeSlots = timeSlots;

  enc->frameSize = numQMFBands * enc->timeSlots;
  *bufferSize = enc->frameSize;

  enc->pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK**) calloc(nObjects, sizeof(QMFLIB_POLYPHASE_ANA_FILTERBANK*));
  if ( QMFLIB_HYBRID_OFF != hybridMode ) {
    for ( i = 0; i < nObjects; ++i ) {
      QMFlib_InitAnaHybFilterbank(&(enc->hybFilterState[i]));
    }
  }

  if( !bQmfInput ) {
    /* Init QMFlib Analysis */
    QMFlib_InitAnaFilterbank(numQMFBands, LdMode);
    for (i = 0; i < nObjects; ++i ) {
      QMFlib_OpenAnaFilterbank( &(enc->pAnalysisQmf[i]) ); 
    }
  }

  CreateSAOCEncoder(&enc->bitstreamFormatter);
  pConfig=GetSAOCSpecificConfig(enc->bitstreamFormatter);

  memset(pConfig,0,sizeof(SAOCSPECIFICCONFIG));

  pConfig->bsSamplingFrequency=48000;
  pConfig->bsSamplingFreqIdx = 3;

  if (timeSlots == coreTimeSlots) {
    pConfig->bsDoubleFrameLengthFlag = 0;
  } else if (timeSlots == 2 * coreTimeSlots) {
    pConfig->bsDoubleFrameLengthFlag = 1;
  }

  pConfig->bsFrameLength = enc->timeSlots-1;
  pConfig->bsFreqRes = 1;


  pConfig->bsNumObjects = nObjects-1;
  pConfig->bsNumDmxChannels = nDmxCh-1;

  pConfig->bsNumSaocChannels = n3DAChannels;
  pConfig->bsNumSaocObjects = n3DAObjects;
  pConfig->bsNumSaocLFEs = n3DALFEs;
  pConfig->bsNumSaocDmxChannels = n3DAdmxChannels;
  pConfig->bsNumSaocDmxObjects = n3DAdmxObjects;
  pConfig->bsNumSaocChannels = 0;
  pConfig->bsLowDelayMode  = 0;
  pConfig->bsSaocDmxMethod       = 0;
  pConfig->bsDcuFlag  = 0;
  pConfig->bsOneIOC  = 0;
  pConfig->Reserved  = 0;

  memcpy(pConfig->bsRelatedTo, bsRelatedTo, sizeof(float)*MAX_OBJECTS*MAX_OBJECTS);

#ifndef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
  /* count saocConfig bits  */
   {
	int tmp;
	FILE* fptr=bitstream->p_file;
	bitstream->p_file = 0;
    bitstream->bufferOffset = 0;
    bitstream->markers      = 0;

	WriteSAOCSpecificConfig(bitstream, enc->bitstreamFormatter);
  

	tmp=bitstream->bitsInBuffer / 8;

	bitstream->p_file = fptr;
	bitstream->bitsInBuffer = 0;

    writeBits(bitstream, tmp , 8); 
  }
#endif

  WriteSAOCSpecificConfig(bitstream, enc->bitstreamFormatter);

  return enc;
}

void SaocEncApply(tSpatialEnc *self, 
                  float **audioInput, 
                  float **qmfInRealBuffer,
                  float **qmfInImagBuffer,
                  int nObjects, 
                  Stream *bitstream, 
                  int nDmxCh, 
                  float pDmxMatrix[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS],
                  float deqDmxMatrix[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS],
                  int bQmfInput,
                  int numQmfBands,
                  QMFLIB_HYBRID_FILTER_MODE hybridMode,
                  int LdMode)
{

  static int independencyCounter = 0;

  float mQmfReal[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][NUM_QMF_BANDS];
  float mQmfImag[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][NUM_QMF_BANDS];
  float mHybridReal[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][MAX_HYBRID_BANDS];
  float mHybridImag[MAX_NUM_INPUTCHANNELS][MAX_TIME_SLOTS][MAX_HYBRID_BANDS];

  float pOld[MAX_OBJECTS][MAX_PARAM_BANDS];
  float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS];
  float pNrg[MAX_PARAM_BANDS];

  float pDmxMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS] = {0.0f};


  SAOCFRAME *pFrameData;

  int i, j, k, ts;

  assert(nObjects<=MAX_NUM_INPUTCHANNELS);

  if ( !bQmfInput ) {
    for (i = 0; i < nObjects; i++) {
      for (ts = 0; ts < self->timeSlots; ts++) {
        float tmp2QmfBuf[QMFLIB_MAX_NUM_QMF_BANDS] = {0}; 
            
        for (k=0; k < numQmfBands; k++) {
          tmp2QmfBuf[k] = audioInput[i][numQmfBands*ts+k];
        }

        QMFlib_CalculateAnaFilterbank(self->pAnalysisQmf[i],
                                      tmp2QmfBuf,
                                      mQmfReal[i][ts],
                                      mQmfImag[i][ts],
                                      LdMode);
        }
    }
  } else {
    for (i = 0; i < nObjects; i++) {
      for (ts = 0; ts < self->timeSlots; ts++) {          
        for (k=0; k < numQmfBands; k++) {
          mQmfReal[i][ts][k] = qmfInRealBuffer[i][numQmfBands*ts+k];
          mQmfImag[i][ts][k] = qmfInImagBuffer[i][numQmfBands*ts+k];
        }
      }
    }  
  }

  if (!LdMode) {
    for (i = 0; i < nObjects; i++) {
      for (ts = 0; ts < self->timeSlots; ts++) {        
        QMFlib_ApplyAnaHybFilterbank(&self->hybFilterState[i], 
                                     hybridMode,
                                     numQmfBands,
                                     1, /* hybridHFAlign*/
                                     mQmfReal[i][ts],
                                     mQmfImag[i][ts],
                                     mHybridReal[i][ts], 
                                     mHybridImag[i][ts]); 
      }
    }
  } else {
    for (i = 0; i < nObjects; i++) {
      for (ts = 0; ts < self->timeSlots; ts++) { 
        for (k=0;k< numQmfBands; k++){
          mHybridReal[i][ts][k] = mQmfReal[i][ts][k];
			    mHybridImag[i][ts][k] = mQmfImag[i][ts][k];
        }
      }
    }    
  }

  pFrameData=GetSAOCFrame(self->bitstreamFormatter);

  memset(pFrameData,0,sizeof(SAOCFRAME));

  pFrameData->framingInfo.bsFramingType=1;
  pFrameData->framingInfo.bsNumParamSets=1;
  pFrameData->framingInfo.bsParamSlots[0]=31;

  pFrameData->bsIndependencyFlag=0;

  if(independencyCounter-- == 0) {
    independencyCounter = 24;
    pFrameData->bsIndependencyFlag=1;
  }
  
  CalcOldIoc(self->timeSlots, nObjects, mHybridReal, mHybridImag, pOld, pIoc, pNrg);
  
  SetSaocOld(pOld,pFrameData,0, nObjects, PARAMETER_BANDS);
  SetSaocIoc(pIoc,pFrameData,0, nObjects, PARAMETER_BANDS);

  for(i=0;i<nObjects;i++)  {
    for (j=0; j<nDmxCh; j++) {
      pDmxMat[j][i] = 20.f*(float)log10(pDmxMatrix[j][i]+EPSILON);
    }
  }   
  SetSaocDmxMat(pDmxMat,pFrameData,0, nObjects, nDmxCh, PARAMETER_BANDS);

  WriteSAOCFrame(bitstream, self->bitstreamFormatter);

  GetSaocDmxMat(pDmxMat,pFrameData,0, nObjects, nDmxCh, PARAMETER_BANDS);
  for(i=0;i<nObjects;i++)  {
    for (j=0; j<nDmxCh; j++) {
      deqDmxMatrix[j][i] = (float)pow( 10, (0.05f * pDmxMat[j][i]));
    }
  }   

}

void SaocEncClose(tSpatialEnc *self, int nChannelsIn)
{
  int i;
  if(self != NULL) {

    for (i = 0; i < nChannelsIn; i++) {
      QMFlib_CloseAnaFilterbank(self->pAnalysisQmf[i]);
    } 

    DestroySAOCEncoder(&self->bitstreamFormatter);
    free(self);
  }
}
