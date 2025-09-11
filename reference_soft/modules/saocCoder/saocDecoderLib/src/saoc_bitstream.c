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



#include <stdlib.h>
#include <stdio.h> 
#include <assert.h>
#include <math.h>
#include "saoc_bitstream.h"
#include "saoc_nlc_dec.h"
#include "saoc_const.h"
#include "error.h"

 
#define MAXBANDS 128
 
extern char *errorMessage;

static const int SampleRateTable[] = {
  96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,
  -1,-1,-1,-1
};

static int pbStrideTable[]   = {1, 2, 5, 28};
static int FreqResBinTable[] = {0, 28, 20, 14, 10, 7, 5, 4};
static int pbStrideTable_Ld[]   = {1, 2, 5, 23};
static int FreqResBinTable_Ld[] = {0, 23, 15, 12, 9, 7, 5, 4};

static void coarse2fine( short* data, DATA_TYPE dataType, int numBands ) {

  int i;

  for( i=0; i<numBands; i++ ) {
    data[i] <<= 1;
  }

  if (dataType == t_CLD) {
    for( i=0; i<numBands; i++ ) {
      if(data[i] == -14)
        data[i] = -15;
      else if(data[i] == 14)
        data[i] = 15;
    }
  }

  
  if (dataType == t_OLD) {
    for( i=0; i<numBands; i++ ) {
      if(data[i] > 0)	{
        data[i] += 1;
      }
    }
  }

}
 
 
static void
fine2coarse( short* data, DATA_TYPE dataType, int numBands ) {

  int i;

  for( i=0; i<numBands; i++ ) { 
    if ( dataType == t_CLD )
      data[i] /= 2;
    else
      data[i] >>= 1;
  }

}

 
static int
getStrideMap(int freqResStride, int numBands, int* aStrides, int LdMode) {

  int i, pb, pbStride, dataBands, strOffset;

  if(LdMode==0){
    pbStride = pbStrideTable[freqResStride];
  }else{
    pbStride = pbStrideTable_Ld[freqResStride];
  }
  dataBands = (numBands-1)/pbStride+1;

  aStrides[0] = 0;
  for(pb=1; pb<=dataBands; pb++) {
    aStrides[pb] = aStrides[pb-1]+pbStride;
  }
  strOffset = 0;
  while(aStrides[dataBands] > numBands) {
    if (strOffset < dataBands) strOffset++;
    for(i=strOffset; i<=dataBands; i++) {
      aStrides[i]--;
    }
  }

  return dataBands;
}
 

static int saoc_get_geometry_from_speakerConfig(SAOC_3D_SPEAKER_CONFIG_3D pSpeakerConfig3d,
                                                CICP2GEOMETRY_CHANNEL_GEOMETRY geometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                                int *NumSaocChannels,
                                                int *NumSaocLFEs); 
static int saocConfig_SpeakerConfig3d(SAOC_3D_SPEAKER_CONFIG_3D* pSpeakerConfig3d, HANDLE_S_BITINPUT bs);
static int saocConfig_Saoc3dSpeakerDescription( SAOC_3D_SPEAKER_DESCRIPTION* speakerDescription, int angularPrecision, HANDLE_S_BITINPUT bs);
static int saocConfig_Saoc3dFlexibleSpeakerConfig(SAOC_3D_FLEXIBLE_SPEAKER_CONFIG * pFlexibleSpeakerConfig3d, int numSpeakers, HANDLE_S_BITINPUT bs);


void DestroySAOCDecoder(BSR_INSTANCE **selfPtr) {
  *selfPtr = NULL;
}

 
int CreateSAOCDecoder(BSR_INSTANCE **selfPtr) {

  BSR_INSTANCE *self = NULL;
  
  
  self = calloc(1, sizeof(*self));
  if(self == NULL) {
    errorMessage = "Could not allocate decoder.";
    return ERROR_NO_FREE_MEMORY;
  }

  *selfPtr = self;

  return ERROR_NONE;
}
 

SAOCSPECIFICCONFIG *GetSAOCSpecificConfig(BSR_INSTANCE *selfPtr) {
  return &selfPtr->saocSpecificConfig;
}

 
int SpatialDecParseExtensionConfig(HANDLE_S_BITINPUT bitstream,
                                   BSR_INSTANCE *selfPtr,
                                   int bitsAvailable) 
{
  SAOCSPECIFICCONFIG* config = &selfPtr->saocSpecificConfig;

  unsigned long data;

  int tmp, sacExtLen, bitsRead, nFillBits;
  int ba = bitsAvailable;
  int sacExtCnt = 0;

  config->SaocExtNum = 0;

  while (ba >= 8) {

    if( ERROR_NONE != s_GetBits2(bitstream, 4, &data) ) goto error;
    config->bsSacExtType[sacExtCnt] = (int)data;
    ba -= 4;

    config->SaocExtNum += 1;

    if( ERROR_NONE != s_GetBits2(bitstream, 4, &data) ) goto error;
    sacExtLen = (int)data;
    ba -= 4;

    if (sacExtLen == 15) {
      if( ERROR_NONE != s_GetBits2(bitstream, 8, &data) ) goto error;
      sacExtLen += (int)data;
      ba -= 8;
      if (sacExtLen == 15+255) {
        if( ERROR_NONE != s_GetBits2(bitstream, 16, &data) ) goto error;
        sacExtLen += (int)data;	
        ba -= 16;
      }
    }

    tmp = s_getNumBits(bitstream);

    switch (config->bsSacExtType[sacExtCnt]) {
    case 0:

      bitsRead = s_getNumBits(bitstream) - tmp;
      nFillBits = 8*sacExtLen - bitsRead;

      while(nFillBits > 7) {
        s_GetBits(bitstream, 8);
        nFillBits -= 8;
      }
      if(nFillBits > 0) {
        s_GetBits(bitstream, nFillBits);
      }

      ba -= 8*sacExtLen;
      sacExtCnt++; 
    }
    s_byteAlign( bitstream );
  }

  return ERROR_NONE;

 error:
  errorMessage = "Error parsing SAOCSpecificConfig.";

  return ERROR_PARSING_BITSTREAM;
}


int ReadSAOCSpecificConfig(HANDLE_S_BITINPUT bitstream, 
                           BSR_INSTANCE *selfPtr,
                           int sacHeaderLen,
                           int numPremixedChannels)
{

  unsigned long data;

  SAOC_3D_SPEAKER_CONFIG_3D audioChannelLayout;

  int obj1, obj2;

  int tmp = s_getNumBits(bitstream);
  int bitsAvailable, numHeaderBits;

  SAOCSPECIFICCONFIG* config = &selfPtr->saocSpecificConfig;
   
  bitsAvailable = 8*sacHeaderLen;
    
  tmp = s_getNumBits(bitstream);

  if( ERROR_NONE != s_GetBits2(bitstream, 4, &data) ) goto error;
  config->bsSamplingFreqIdx = (int)data;

  if( config->bsSamplingFreqIdx == 15) 
	{
    if( ERROR_NONE != s_GetBits2(bitstream, 24, &data) ) goto error;
    config->bsSamplingFrequency = (int)data;
  }
	else
	{
    config->bsSamplingFrequency = SampleRateTable[config->bsSamplingFreqIdx];  
  }

  /*if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
  config->bsLowDelayMode = (int)data; */
  config->bsLowDelayMode           = 0;

  if( ERROR_NONE != s_GetBits2(bitstream, 3, &data) ) goto error;
  config->bsFreqRes = (int)data;

  if(config->bsLowDelayMode==0)
	{
    config->bsNumBins = FreqResBinTable[config->bsFreqRes];
  }
	else
	{
    config->bsNumBins = FreqResBinTable_Ld[config->bsFreqRes];
  }

  /*if(config->bsLowDelayMode==0){
    if( ERROR_NONE != s_GetBits2(bitstream, 7, &data) ) goto error;
  }else{
    if( ERROR_NONE != s_GetBits2(bitstream, 5, &data) ) goto error;
  }*/

  /*config->bsFrameLength = (int)data;*/
  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
  config->bsDoubleFrameLengthFlag = (int)data;

  if( ERROR_NONE != s_GetBits2(bitstream, 5, &data) ) goto error;
  config->bsNumSaocDmxChannels  = (int)data;

  if( ERROR_NONE != s_GetBits2(bitstream, 5, &data) ) goto error;
  config->bsNumSaocDmxObjects  = (int)data;

  config->NumInputSignals = 0;

  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
  config->bsDecorrelationMethod = (int)data; 

  if (config->bsNumSaocDmxChannels > 0) 
	{
    saocConfig_SpeakerConfig3d(&audioChannelLayout, bitstream);
    if (audioChannelLayout.speakerLayoutID == 0) 
		{
      config->inSaocCICPIndex = audioChannelLayout.CICPspeakerLayoutIdx;
    } 
		else 
		{
       config->inSaocCICPIndex = -1;
    }
    saoc_get_geometry_from_speakerConfig(audioChannelLayout, config->saocInputGeometryInfo, &config->bsNumSaocChannels, &config->bsNumSaocLFEs); 

		config->NumInputSignals += config->bsNumSaocChannels;
	}

  if( ERROR_NONE != s_GetBits2(bitstream, 8, &data) ) goto error;
  config->bsNumSaocObjects  = (int)data; 

  config->NumInputSignals += config->bsNumSaocObjects;

  config->bsDecorrelationLevel = 0;

  for( obj1=0; obj1<config->bsNumSaocChannels; obj1++ ) {
    config->bsRelatedTo[obj1][obj1] = 1;
    for( obj2=obj1+1; obj2<config->bsNumSaocChannels; obj2++ ) {
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
      config->bsRelatedTo[obj1][obj2] = (int)data;
      config->bsRelatedTo[obj2][obj1] = (int)data;
    }
  }
    
  for( obj1=config->bsNumSaocChannels; obj1<config->NumInputSignals; obj1++ ) {
    for( obj2=0; obj2<config->bsNumSaocChannels; obj2++ ) {
        config->bsRelatedTo[obj1][obj2] = 0;
        config->bsRelatedTo[obj2][obj1] = 0;
    }
  }

  for( obj1=config->bsNumSaocChannels; obj1<config->NumInputSignals; obj1++ ) {
    config->bsRelatedTo[obj1][obj1] = 1;
    for( obj2=obj1+1; obj2<config->NumInputSignals; obj2++ ) {
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
      config->bsRelatedTo[obj1][obj2] = (int)data;
      config->bsRelatedTo[obj2][obj1] = (int)data;
    }
  }

  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
  config->bsOneIOC = (int)data;


  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
  config->bsSaocDmxMethod = (int)data;

  if (config->bsSaocDmxMethod == 1) 
	{
    config->bsNumPreMixedChannels = numPremixedChannels;
  }
	else 
	{ 
    config->bsNumPreMixedChannels = 0;
  }

  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
  config->bsDualMode = (int)data;

  if (config->bsDualMode) 
	{
	  if( ERROR_NONE != s_GetBits2(bitstream, 5, &data) ) goto error;
    config->bsBandsLow = (int)data;
    config->bsBandsHigh = config->bsNumBins;
  } 
	else
	{
    config->bsBandsLow = config->bsNumBins;
  }

  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
  config->bsDcuFlag = (int)data; 

  if (config->bsDcuFlag == 1){
    if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
    config->bsDcuMandatory = (int)data;
    if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
    config->bsDcuDynamic = (int)data;
    if (config->bsDcuDynamic == 0){	  
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
      config->bsDcuMode = (int)data;
      if( ERROR_NONE != s_GetBits2(bitstream, 4, &data) ) goto error; 
      config->bsDcuParam = (int)data;		  
    }
  } 
  else {
    config->bsDcuMandatory = 0;
    config->bsDcuDynamic = 0;
    config->bsDcuMode = 0;
    config->bsDcuParam = 0;	
  }

  /**/

  s_byteAlign( bitstream );

  numHeaderBits = s_getNumBits(bitstream) - tmp;
  bitsAvailable -= numHeaderBits;
  
  SpatialDecParseExtensionConfig(bitstream,
                                 selfPtr,
                                 bitsAvailable); 


  return ERROR_NONE;

 error:
  errorMessage = "Error parsing SAOCSpecificConfig.";
  return ERROR_PARSING_BITSTREAM;

}

int PrintSAOCSpecificConfig(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE *selfPtr) {
	
  int  obj1, obj2;
	
  SAOCSPECIFICCONFIG* config = &selfPtr->saocSpecificConfig;
	
  FILE *fSAOCSpecificConfig;
  fSAOCSpecificConfig = fopen("SAOCSpecificConfig.txt","wt"); 
	
  fprintf(fSAOCSpecificConfig,"\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"* SAOC Specific Config Info                 *\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"\n");
	
  fprintf(fSAOCSpecificConfig,"Sampling Freq Idx: %d\n",config->bsSamplingFreqIdx);
  fprintf(fSAOCSpecificConfig,"Num Bins: %d\n",config->bsNumBins);
  fprintf(fSAOCSpecificConfig,"Num Input Signals: %d\n",config->NumInputSignals  );

	fprintf(fSAOCSpecificConfig,"RelatedTo: \n");
	for( obj1=0; obj1<config->NumInputSignals ; obj1++ ) {
		for( obj2=0; obj2<config->NumInputSignals ; obj2++ ) {
			fprintf(fSAOCSpecificConfig," %d ",config->bsRelatedTo[obj1][obj2]);
		}
		fprintf(fSAOCSpecificConfig,"\n");
	}
	
	/*fprintf(fSAOCSpecificConfig,"Transmit Abs Nrg: %d\n",config->bsTransmitAbsNrg);*/
	/*fprintf(fSAOCSpecificConfig,"Num Dmx Channels: %d\n",config->bsNumSaocDmxChannels + config->bsNumSaocDmxObjects);*/


  if ((config->bsDcuFlag == 1)&&(config->bsDcuMandatory==1)&&(config->bsDcuDynamic==1)){
    fprintf(fSAOCSpecificConfig,"Mandatory dynamic DCU data\n");
  } 
  if ((config->bsDcuFlag == 1)&&(config->bsDcuMandatory==1)&&(config->bsDcuDynamic==0)){
    fprintf(fSAOCSpecificConfig,"Mandatory static DCU data\n");
    fprintf(fSAOCSpecificConfig,"DCU mode: %d\n",config->bsDcuMode);
    fprintf(fSAOCSpecificConfig,"DCU value: %d\n",config->bsDcuParam);
  } 
  if ((config->bsDcuFlag == 1)&&(config->bsDcuMandatory==0)&&(config->bsDcuDynamic==1)){
    fprintf(fSAOCSpecificConfig,"Optional dynamic DCU data\n");
  } 
  if ((config->bsDcuFlag == 1)&&(config->bsDcuMandatory==0)&&(config->bsDcuDynamic==0)){
    fprintf(fSAOCSpecificConfig,"Optional static DCU data\n");
    fprintf(fSAOCSpecificConfig,"DCU mode: %d\n",config->bsDcuMode);
    fprintf(fSAOCSpecificConfig,"DCU value: %d\n",config->bsDcuParam);
  } 
	
  fprintf(fSAOCSpecificConfig,"\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"* SAOC Extension Config Info                *\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"\n");
		
  fprintf(fSAOCSpecificConfig,"\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"* SAOC Frame Info		     *\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"\n");

  fclose(fSAOCSpecificConfig);
  return ERROR_NONE;
}


int PrintSAOCSpecificFrame(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE *selfPtr) {
	
	int  obj1, obj2;

	SAOCSPECIFICCONFIG* config = &selfPtr->saocSpecificConfig;
  SAOCFRAME*          frame  = &selfPtr->currentSAOCFrame;
	
  FILE *fSAOCSpecificConfig;
  fSAOCSpecificConfig = fopen("SAOCSpecificConfig.txt","a"); 
	
  if (frame->bsDcuDynamicUpdate == 1 ){
    fprintf(fSAOCSpecificConfig,"DCU mode: %d\n",frame->bsDcuMode);
    fprintf(fSAOCSpecificConfig,"DCU value: %d\n",frame->bsDcuParam);
  }
	
  if (frame->bsDcuDynamicUpdate2 == 1 ){
    fprintf(fSAOCSpecificConfig,"DCU mode for EAO: %d\n",frame->bsDcuMode2);
    fprintf(fSAOCSpecificConfig,"DCU value for EAO: %d\n",frame->bsDcuParam2);
  }
	
  fprintf(fSAOCSpecificConfig,"\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"* SAOC Downmix Matrix Info                 *\n");
  fprintf(fSAOCSpecificConfig,"*-------------------------------------------*\n");
  fprintf(fSAOCSpecificConfig,"\n");

	for( obj1=0; obj1<config->bsNumSaocDmxChannels +config->bsNumSaocDmxObjects; obj1++ ) {
		for( obj2=0; obj2<config->NumInputSignals ; obj2++ ) {
			fprintf(fSAOCSpecificConfig," %d ",frame->dmxMat[obj1].cur_data[0].qval[obj2]);
		}
		fprintf(fSAOCSpecificConfig,"\n");
	}

  fprintf(fSAOCSpecificConfig,"\n");
  fprintf(fSAOCSpecificConfig,"--- Next Frame ---\n");
  fprintf(fSAOCSpecificConfig,"\n");
	
  fclose(fSAOCSpecificConfig);
  return ERROR_NONE;
}

static int ecDataSAOC( HANDLE_S_BITINPUT      bitstream,
                       PREV_NEW_LOSSLESSDATA* lldata,
                       DATA_TYPE              dataType,
                       int                    numParamSets,
                       int*                   paramSlot,
                       int                    independencyFlag,
                       int                    startBand,
                       int                    numBands,
                       int					          LdMode)
{ 
  int error;
  unsigned long data;

  int ps, ps2, numDataSetsLeft, dataPairFlag;
  int pb, numStrides, i;

  int aStrides[max(SAOC_MAX_PARAMETER_BANDS,SAOC_MAX_OBJECTS)+1] = {0};
  
  short aDataPair[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)];
  short aPrevDataSet[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)];

  SAOCLOSSLESSDATA (*curData)[SAOC_MAX_PARAM_SETS] = &lldata->cur_data;
  SAOCLOSSLESSDATA *prevData = &lldata->prev_data;
  SAOCLOSSLESSDATA *lastData;

  int aInterpolate[SAOC_MAX_PARAMETER_SETS];

  int i1, i2, x1, xi, x2;

  numDataSetsLeft = 0;
  for( ps=0; ps<numParamSets; ps++ ) {
    if( ERROR_NONE != s_GetBits2(bitstream, 2, &data) ) goto error;
    (*curData)[ps].bsDataMode = (int)data;

    if ((independencyFlag == 1 && ps == 0 && ((*curData)[ps].bsDataMode == KEEP || (*curData)[ps].bsDataMode == INTERPOLATE)))
      {
        fprintf(stderr, "invalid bsDataMode because bsIndependencyFlag is equal 1");
      }

    if( (*curData)[ps].bsDataMode == FINECOARSE ) {
      numDataSetsLeft++;
    }
  }
  
  lastData = prevData;

  dataPairFlag = 0;
  ps = 0;
  while( ps < numParamSets ) {

    aInterpolate[ps] = 0;

    switch( (*curData)[ps].bsDataMode ) {
    case DEFAULT:
      (*curData)[ps].bsFreqResStride = 0;
      (*curData)[ps].bsQuantCoarse   = 0;
      for( pb=startBand; pb<startBand+numBands; pb++ ) {
        (*curData)[ps].qval[pb] = 0;
        lastData->qval[pb] = 0;
      }
      ps++;
      continue;
  	
    case INTERPOLATE:
      aInterpolate[ps] = 1;
      /* fall through */
    case KEEP:
      (*curData)[ps].bsFreqResStride = lastData->bsFreqResStride;
      (*curData)[ps].bsQuantCoarse   = lastData->bsQuantCoarse;
      for( pb=startBand; pb<startBand+numBands; pb++ ) {
        (*curData)[ps].qval[pb] = lastData->qval[pb];
      }
      ps++;
      continue;
    default:
      break;
    }

    if( numDataSetsLeft > 1 ) {
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
      dataPairFlag = (int)data;
    }
    else {
      dataPairFlag = 0;
    }

    if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
    (*curData)[ps].bsQuantCoarse = (int)data;

    if( ERROR_NONE != s_GetBits2(bitstream, 2, &data) ) goto error;
    (*curData)[ps].bsFreqResStride = (int)data;

    if( dataPairFlag ) {
      ps2 = ps+1;
      while( (*curData)[ps2].bsDataMode != FINECOARSE ) ps2++;
      (*curData)[ps2].bsQuantCoarse   = (*curData)[ps].bsQuantCoarse;
      (*curData)[ps2].bsFreqResStride = (*curData)[ps].bsFreqResStride;
    }

    for( pb=0; pb<numBands; pb++ ) {
      aPrevDataSet[pb] = lastData->qval[startBand+pb];
    }

    if( (*curData)[ps].bsQuantCoarse) {
      fine2coarse( aPrevDataSet, dataType, numBands );
    }

    numStrides = getStrideMap( (*curData)[ps].bsFreqResStride, numBands, aStrides, LdMode );
    for( pb=0; pb<numStrides; pb++ ) {
      aPrevDataSet[pb] = aPrevDataSet[aStrides[pb]];
    }

    error = EcDataPairDec( bitstream,
                           aDataPair,
                           aPrevDataSet,
                           dataType,
                           ps,
                           numStrides,
                           dataPairFlag,
                           (*curData)[ps].bsQuantCoarse,
                           independencyFlag && ps == 0);

    if( ERROR_NONE != error ) return error;

    if( (*curData)[ps].bsQuantCoarse ) {

      coarse2fine( aDataPair[0], dataType, numBands );

      if(dataPairFlag) {
        coarse2fine( aDataPair[1], dataType, numBands );
      }
    }




    for( pb=0; pb<numStrides; pb++ ) {
      for( i=aStrides[pb]; i<aStrides[pb+1]; i++ ) {
        (*curData)[ps].qval[startBand+i] = aDataPair[0][pb];
        if( dataPairFlag ) {
          (*curData)[ps2].qval[startBand+i] = aDataPair[1][pb];
        }
      }
    }

    if( dataPairFlag ) {
      lastData = &(*curData)[ps2];
      numDataSetsLeft -= 2;
      ps += 2;
    } else {
      lastData = &(*curData)[ps];
      numDataSetsLeft--;
      ps++;
    }
  }

  for( pb=startBand; pb<numBands; pb++ ) {
    prevData->qval[pb] = lastData->qval[pb];
  }
  prevData->bsDataMode      = lastData->bsDataMode;
  prevData->bsFreqResStride = lastData->bsFreqResStride;
  prevData->bsQuantCoarse   = lastData->bsQuantCoarse;

  /* interpolate */

  i1 = -1;
  x1 =  0;
  i2 =  0;
  for( i = 0 ; i < numParamSets; i++ ) {

    if(aInterpolate[i] != 1) {
      i1 = i;
    }
    i2 = i;
    while(aInterpolate[i2] == 1){
      i2++;
    }

    if(i1==-1){
      x1=0;
      i1=0;
    }else{
      x1 = paramSlot[i1];
    }

    xi = paramSlot[i];
    x2 = paramSlot[i2];

    if( aInterpolate[i] == 1 ) {
      assert(i2 < numParamSets);
      for(pb = startBand ; pb < numBands; pb++) {
        int yi,y1,y2;
        y1 = (*curData)[i1].qval[pb];
        y2 = (*curData)[i2].qval[pb];
        yi = y1 + (xi-x1)*(y2-y1)/(x2-x1);
        (*curData)[i].qval[pb] = yi;
      }
    }
  } 

  return ERROR_NONE;

 error:
  errorMessage = "Error parsing entropy coded data.";
  return ERROR_PARSING_BITSTREAM;

}


 
SAOCFRAME* GetSAOCFrame(BSR_INSTANCE *selfPtr) {
  return &selfPtr->currentSAOCFrame;
}


static int ReadSAOCFramingInfo(HANDLE_S_BITINPUT bitstream, FRAMINGINFO* frInfo, SAOCSPECIFICCONFIG* config, int *numParamSets) {

	unsigned long data;
	int bitsParamSlot = 0, i;

	if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
  frInfo->bsFramingType = (int)data;

  if(config->bsLowDelayMode==0){
    if( ERROR_NONE != s_GetBits2(bitstream, 3, &data) ) goto error;
  }else{
    if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
  }

  frInfo->bsNumParamSets = (int)data;
  *numParamSets = frInfo->bsNumParamSets+1;

  for( i=0; i<*numParamSets; i++ ) {
    if( frInfo->bsFramingType ) {
      while( (1<<bitsParamSlot) < (config->bsFrameLength+1) ) bitsParamSlot++;
      
      if( ERROR_NONE != s_GetBits2(bitstream, bitsParamSlot, &data) ) goto error;
      frInfo->bsParamSlots[i] = (int)data;
    } else {
      frInfo->bsParamSlots[i] = (int)ceil((float)(config->bsFrameLength+1) * (float)(i+1) / (float) *numParamSets) - 1;
    }
  }
  return ERROR_NONE;

error:
  errorMessage = "Error parsing Framing Info.";
  return ERROR_PARSING_BITSTREAM;
}

int ReadSAOCFrame(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE* selfPtr) {

  unsigned long data;
  int error;

  int numParamSets, i, j, pb, ps;
  int firstObj, numObj, startBand, numBands;
  int dmgIdx, numDmg;
  int isFirstIOC = 1;
  int firstIOCi, firstIOCj;

  SAOCFRAME*          frame  = &selfPtr->currentSAOCFrame;
  SAOCSPECIFICCONFIG* config = &selfPtr->saocSpecificConfig;
  FRAMINGINFO* frInfo = &frame->framingInfo;

  error = ReadSAOCFramingInfo(bitstream, frInfo, config, &numParamSets);

  if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error;
  frame->bsIndependencyFlag = (int)data;

  firstObj = 0;
  numObj   = config->NumInputSignals;

  startBand = 0;
  numBands  = config->bsNumBins;

  for( i=firstObj; i<firstObj+numObj; i++ ) {
    error = ecDataSAOC( bitstream,
                        &frame->old[i],
                        t_OLD,
                        numParamSets,
                        frInfo->bsParamSlots,
                        frame->bsIndependencyFlag,
                        startBand,
                        numBands,
                        config->bsLowDelayMode);

    if( ERROR_NONE != error ) return error;
  }
     
  for( i=0; i<numObj; i++ ) {
    for( ps=0; ps<numParamSets; ps++ ) {
      for( pb=startBand; pb<startBand+numBands; pb++ ) {
        frame->ioc[i][i].cur_data[ps].qval[pb] = 0;  
      }
      frame->ioc[i][i].cur_data[ps].bsDataMode      = DEFAULT;
      frame->ioc[i][i].cur_data[ps].bsFreqResStride = 0;
      frame->ioc[i][i].cur_data[ps].bsQuantCoarse   = 0;
    }
    for( j=i+1; j<numObj; j++ ) {
      if( config->bsRelatedTo[i][j] ) {
        if( config->bsOneIOC == 0 ) {
          error = ecDataSAOC( bitstream,
                              &frame->ioc[i][j],
                              t_ICC,
                              numParamSets,
                              frInfo->bsParamSlots,
                              frame->bsIndependencyFlag,
                              startBand,
                              numBands,
                              config->bsLowDelayMode);
          
          if( ERROR_NONE != error ) return error;
        } else {
          if(isFirstIOC==1) {		  
            error = ecDataSAOC( bitstream,
                                &frame->ioc[i][j],
                                t_ICC,
                                numParamSets,
                                frInfo->bsParamSlots,
                                frame->bsIndependencyFlag,
                                startBand,
                                numBands,
                                config->bsLowDelayMode);
            
            if( ERROR_NONE != error ) return error;
                        
            firstIOCi = i;
            firstIOCj = j;
            isFirstIOC=0;   
          } else {
            for( ps=0; ps<numParamSets; ps++ ) {
              for( pb=startBand; pb<startBand+numBands; pb++ ) {
                frame->ioc[i][j].cur_data[ps].qval[pb] = frame->ioc[firstIOCi][firstIOCj].cur_data[ps].qval[pb];
              }
              frame->ioc[i][j].cur_data[ps].bsDataMode      = frame->ioc[firstIOCi][firstIOCj].cur_data[ps].bsDataMode;
              frame->ioc[i][j].cur_data[ps].bsFreqResStride = frame->ioc[firstIOCi][firstIOCj].cur_data[ps].bsFreqResStride;
              frame->ioc[i][j].cur_data[ps].bsQuantCoarse   = frame->ioc[firstIOCi][firstIOCj].cur_data[ps].bsQuantCoarse;
            }
          }       
        }
      } else {
        for( ps=0; ps<numParamSets; ps++ ) {
          for( pb=startBand; pb<startBand+numBands; pb++ ) {
            frame->ioc[i][j].cur_data[ps].qval[pb] = 5;  
          }
          frame->ioc[i][j].cur_data[ps].bsDataMode      = DEFAULT;
          frame->ioc[i][j].cur_data[ps].bsFreqResStride = 0;
          frame->ioc[i][j].cur_data[ps].bsQuantCoarse   = 0;
        }
      }

      for( ps=0; ps<numParamSets; ps++ ) {
        for( pb=startBand; pb<startBand+numBands; pb++ ) {
          frame->ioc[j][i].cur_data[ps].qval[pb] = frame->ioc[i][j].cur_data[ps].qval[pb];
        }
        frame->ioc[j][i].cur_data[ps].bsDataMode      = frame->ioc[i][j].cur_data[ps].bsDataMode;
        frame->ioc[j][i].cur_data[ps].bsFreqResStride = frame->ioc[i][j].cur_data[ps].bsFreqResStride;
        frame->ioc[j][i].cur_data[ps].bsQuantCoarse   = frame->ioc[i][j].cur_data[ps].bsQuantCoarse;
      }
    }
  }

  if (config->bsNumSaocDmxObjects == 0) {
    for( i=0; i<config->bsNumSaocDmxChannels; i++ ) {
      error = ecDataSAOC(  bitstream,
                           &frame->dmxMat[i],
                           t_CLD,
                           numParamSets,
                           frInfo->bsParamSlots,
                           frame->bsIndependencyFlag,
                           firstObj,
                           config->NumInputSignals,
                           config->bsLowDelayMode);
        if( ERROR_NONE != error ) return error;
    }
  } else {
    dmgIdx = 0;
    for( i=0; i<config->bsNumSaocDmxChannels; i++ ) {
      error = ecDataSAOC(  bitstream,
                           &frame->dmxMat[i],
                           t_CLD,
                           numParamSets,
                           frInfo->bsParamSlots,
                           frame->bsIndependencyFlag,
                           firstObj,
                           config->bsNumSaocChannels,
                           config->bsLowDelayMode);
        if( ERROR_NONE != error ) return error;
    }
    dmgIdx = config->bsNumSaocDmxChannels;

    if (config->bsSaocDmxMethod > 0) {
      numDmg = config->bsNumPreMixedChannels;
    } else {
      numDmg = config->bsNumSaocObjects;
    }

    for( i=dmgIdx ; i<dmgIdx + config->bsNumSaocDmxObjects; i++ ) {
      error = ecDataSAOC(  bitstream,
                           &frame->dmxMat[i],
                           t_CLD,
                           numParamSets,
                           frInfo->bsParamSlots,
                           frame->bsIndependencyFlag,
                           firstObj,
                           numDmg,
                           config->bsLowDelayMode);

      if( ERROR_NONE != error ) return error;
    }
  }


  if ((config->bsDcuFlag == 1) && (config->bsDcuDynamic == 1)){
    if (frame->bsIndependencyFlag == 1) {
      frame->bsDcuDynamicUpdate = 1;
    }
    else {
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
      frame->bsDcuDynamicUpdate = (int)data;
    }	  
    if (frame->bsDcuDynamicUpdate == 1) {
      if( ERROR_NONE != s_GetBits2(bitstream, 1, &data) ) goto error; 
      frame->bsDcuMode = (int)data;
      if( ERROR_NONE != s_GetBits2(bitstream, 4, &data) ) goto error; 
      frame->bsDcuParam = (int)data;		  
    }
  } 

  s_byteAlign( bitstream );
  return ERROR_NONE;

 error:
  errorMessage = "Error parsing SAOCFrame.";
  return ERROR_PARSING_BITSTREAM;
}


static float dequantOLD[] = {-150.0f,    -45.0f,     -40.0f,     -35.0f,     -30.0f,     -25.0f,     -22.0f,     -19.0f,     -16.0f,     -13.0f,    -10.0f,    -8.0f,     -6.0f,     -4.0f,     -2.0f,     0.0f};
static float dequantIOC[] = {1.f, 0.937f, 0.84118f, 0.60092f, 0.36764f, 0.f, -0.589f, -0.99f};
static float dequantCLD[] = {-150, -45, -40, -35, -30, -25, -22, -19, -16, -13, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 13, 16, 19, 22, 25, 30, 35, 40, 45, 150};

 
static float deq( int value, int paramType)
{
  switch( paramType )
    {
    case t_IOC:
      return dequantIOC[value];

    case t_CLD:
      return dequantCLD[value+15]; 
    
    case t_OLD:

      return (float)pow(10.f,dequantOLD[value]/10.f);
   
    default:
      assert(0);
      return 0.0;
    }
}

void GetSaocOld(float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{
  int j,k;
  for(j=0;j<nNumObjects;j++) {
    for(k=0;k<nNumBins;k++) {
      
      pOld[j][k]=deq(pFrameDataSaoc->old[j].cur_data[nParamSet].qval[k],t_OLD);
      
    }
  }
}

void GetSaocIoc(float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{
  int j,k,l;
  
  for(j=0;j<nNumObjects;j++) {
    for(k=0;k<nNumObjects;k++) {
      for(l=0;l<nNumBins;l++) {
        pIoc[j][k][l]=deq(pFrameDataSaoc->ioc[j][k].cur_data[nParamSet].qval[l],t_IOC);
#ifdef SAOC_AVOID_NEGATIVE_IOCS
        if (pIoc[j][k][l]<0.0f) {
          pIoc[j][k][l] = 0.0f;
        }
#endif
      }
    }
  }
}

void SAOC_GetSaocDmxMat(float pDmxMat[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS], SAOCFRAME* pFrameDataSaoc, int nParamSet, 
                                int nSaocChannels,
                                int nSaocObjects,
                                int nSaocDmxChannels,
                                int nSaocDmxObjects,
                                int nNumPremixedChannels)
{
  int l, j, numDmxObj;
  if (nSaocDmxObjects == 0) {
    for(l=0;l<nSaocChannels + nSaocObjects;l++) {
      for(j=0;j<nSaocDmxChannels;j++) {
        pDmxMat[j][l]=(float)pow(10.0f,deq(pFrameDataSaoc->dmxMat[j].cur_data[nParamSet].qval[l],t_CLD)/20.0f);
      }
    }
  } else {
    if (nNumPremixedChannels == 0) {
      numDmxObj = nSaocDmxObjects;
    } else {
      numDmxObj = nNumPremixedChannels;
    }
    for(l=0;l<nSaocChannels + nSaocObjects;l++) {
      for(j=0;j<nSaocDmxChannels + numDmxObj;j++) {
        pDmxMat[j][l]=0.0f;
      }
    }

    for(l=0;l<nSaocChannels;l++) {
      for(j=0;j<nSaocDmxChannels;j++) {
        pDmxMat[j][l]=(float)pow(10.0f,deq(pFrameDataSaoc->dmxMat[j].cur_data[nParamSet].qval[l],t_CLD)/20.0f);
      }
    }
    for(l=0;l< nSaocObjects;l++) {
      for(j=nSaocDmxChannels;j<nSaocDmxChannels + numDmxObj;j++) {
        pDmxMat[j][nSaocChannels + l]=(float)pow(10.0f,deq(pFrameDataSaoc->dmxMat[j].cur_data[nParamSet].qval[l],t_CLD)/20.0f);
      }
    }
  }
}

UINT32 saocConfig_ParseEscapedValue(HANDLE_S_BITINPUT bs, UINT32 nBits1, UINT32 nBits2, UINT32 nBits3){

  UINT32 value;
  UINT32 valueAdd = 0;
  UINT32 maxValue1 = (1 << nBits1) - 1;
  UINT32 maxValue2 = (1 << nBits2) - 1;
  unsigned long data;

  s_GetBits2(bs, nBits1, &data);
  value = (UINT32)data;

  if(value == maxValue1){
    s_GetBits2(bs, nBits2, &data);
    valueAdd = (UINT32)data;
    value += valueAdd;

    if(valueAdd == maxValue2){
      s_GetBits2(bs, nBits3, &data);
      valueAdd = (UINT32)data;
      value += valueAdd;
    }
  }

  return value;
}

static int saoc_AzimuthAngleIdxToDegrees ( int idx, int direction, int precision ) {
  
  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

static int saoc_ElevationAngleIdxToDegrees ( int idx, int direction, int precision ) {

  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

static int saoc_get_geometry_from_speakerConfig(SAOC_3D_SPEAKER_CONFIG_3D pSpeakerConfig3d,
                                                CICP2GEOMETRY_CHANNEL_GEOMETRY geometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS], 
                                                int *NumSaocChannels,
                                                int *NumSaocLFEs) 
{

  unsigned int i;

  if (pSpeakerConfig3d.speakerLayoutID == 0 ) {
    cicp2geometry_get_geometry_from_cicp(pSpeakerConfig3d.CICPspeakerLayoutIdx, geometryInfo, NumSaocChannels, NumSaocLFEs); 
  } else {
    if (pSpeakerConfig3d.speakerLayoutID == 1 ) {
      *NumSaocChannels = pSpeakerConfig3d.numSpeakers;
      *NumSaocLFEs = 0;
      for (i = 0; i<pSpeakerConfig3d.numSpeakers; i++) {
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(pSpeakerConfig3d.CICPspeakerIdx[i], 
                                                               &(geometryInfo[i])); 
        if ( (pSpeakerConfig3d.CICPspeakerIdx[i]  == 3 ) || (pSpeakerConfig3d.CICPspeakerIdx[i]  == 26) ){
          (*NumSaocChannels)--;
          (*NumSaocLFEs)++;
        }
      } 
    } else if (pSpeakerConfig3d.speakerLayoutID == 2 ) {
      *NumSaocChannels = pSpeakerConfig3d.numSpeakers;
      *NumSaocLFEs = 0;
      for (i = 0; i<pSpeakerConfig3d.numSpeakers; i++) {
        if(pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx) {
          cicp2geometry_get_geometry_from_cicp_loudspeaker_index(pSpeakerConfig3d.CICPspeakerIdx[i], 
                                                                 &(geometryInfo[i])); 
        }
        else {
          geometryInfo[i].cicpLoudspeakerIndex = -1;
          geometryInfo[i].Az = saoc_AzimuthAngleIdxToDegrees(pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx, 
                                                          pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection,  
                                                          pSpeakerConfig3d.flexibleSpeakerConfig.angularPrecision);
          geometryInfo[i].El = saoc_ElevationAngleIdxToDegrees(pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx, 
                                                            pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationDirection,  
                                                            pSpeakerConfig3d.flexibleSpeakerConfig.angularPrecision);

          geometryInfo[i].LFE = pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isLFE;

        }
        if ( pSpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isLFE  !=0 ) {
          (*NumSaocChannels)--;
          (*NumSaocLFEs)++;
        }
      } 
    }
  }
  return 0;
} 

static int saocConfig_SpeakerConfig3d(SAOC_3D_SPEAKER_CONFIG_3D* pSpeakerConfig3d, HANDLE_S_BITINPUT bs) {

  unsigned int i;
  unsigned long data;

  if( ERROR_NONE != s_GetBits2(bs, 2, &data) ) goto error;
  pSpeakerConfig3d->speakerLayoutID = (int)data;

  if ( pSpeakerConfig3d->speakerLayoutID == 0 ) {
    if( ERROR_NONE != s_GetBits2(bs, 6, &data) ) goto error;
    pSpeakerConfig3d->CICPspeakerLayoutIdx = (int)data;
  } else {
    pSpeakerConfig3d->numSpeakers = saocConfig_ParseEscapedValue ( bs, 5, 8, 16 ) + 1;

    if ( pSpeakerConfig3d->speakerLayoutID == 1 ) {
      for ( i = 0; i < pSpeakerConfig3d->numSpeakers; i++ ) {
        if( ERROR_NONE != s_GetBits2(bs, 7, &data) ) goto error;
        pSpeakerConfig3d->CICPspeakerIdx[i] = (int)data;
      }
    }

    if ( pSpeakerConfig3d->speakerLayoutID == 2 ) {
      saocConfig_Saoc3dFlexibleSpeakerConfig( & ( pSpeakerConfig3d->flexibleSpeakerConfig ), pSpeakerConfig3d->numSpeakers, bs );
    }
  }

  return ERROR_NONE;

error:
  errorMessage = "Error parsing SAOCFrame.";
  return ERROR_PARSING_BITSTREAM;
}

static int saocConfig_Saoc3dFlexibleSpeakerConfig(SAOC_3D_FLEXIBLE_SPEAKER_CONFIG * pFlexibleSpeakerConfig3d, int numSpeakers, HANDLE_S_BITINPUT bs) {

  int i = 0;
  int alsoAddSymmetricPair;
  unsigned long data;

  if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
  pFlexibleSpeakerConfig3d->angularPrecision = (int)data;

  while ( i < numSpeakers ) {

    int azimuth;

    saocConfig_Saoc3dSpeakerDescription ( &pFlexibleSpeakerConfig3d->speakerDescription[i], pFlexibleSpeakerConfig3d->angularPrecision, bs );

    if(pFlexibleSpeakerConfig3d->speakerDescription[i].isCICPspeakerIdx) {
      CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
      cicp2geometry_get_geometry_from_cicp_loudspeaker_index( pFlexibleSpeakerConfig3d->speakerDescription[i].CICPspeakerIdx, &AzElLfe);
      azimuth = AzElLfe.Az;
    }
    else {
      azimuth = saoc_AzimuthAngleIdxToDegrees ( pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthAngleIdx, pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection, pFlexibleSpeakerConfig3d->angularPrecision );
    }
 
    if ( (azimuth != 0 ) && (azimuth != 180 ) /* && (the symmetric speaker is not yet in the list) */ ) {
      if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
      alsoAddSymmetricPair = (int)data;

      if ( alsoAddSymmetricPair ) {
        /* (also add the speaker with the opposite AzimuthDirection); */
        i++;
        pFlexibleSpeakerConfig3d->speakerDescription[i] = pFlexibleSpeakerConfig3d->speakerDescription[i - 1];
        pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection = 1 - pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection;
      }
    }

    i++;
  }

  return ERROR_NONE;

error:
  errorMessage = "Error parsing SAOCFrame.";
  return ERROR_PARSING_BITSTREAM;
}

static int saocConfig_Saoc3dSpeakerDescription( SAOC_3D_SPEAKER_DESCRIPTION* speakerDescription, int angularPrecision, HANDLE_S_BITINPUT bs) {

  unsigned long data;

  if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
  speakerDescription->isCICPspeakerIdx = (int)data;

  if ( speakerDescription->isCICPspeakerIdx ) {
    if( ERROR_NONE != s_GetBits2(bs, 7, &data) ) goto error;
    speakerDescription->CICPspeakerIdx  = (int)data;
  } else {
    if( ERROR_NONE != s_GetBits2(bs, 2, &data) ) goto error;
    speakerDescription->ElevationClass  = (int)data;

    if ( speakerDescription->ElevationClass == 3 ) {
      if ( angularPrecision == 0 ) {
        if( ERROR_NONE != s_GetBits2(bs, 5, &data) ) goto error;
        speakerDescription->ElevationAngleIdx = (int)data;
      } else {
        if( ERROR_NONE != s_GetBits2(bs, 7, &data) ) goto error;
        speakerDescription->ElevationAngleIdx = (int)data;
      }

      if ( saoc_ElevationAngleIdxToDegrees ( speakerDescription->ElevationAngleIdx, speakerDescription->ElevationDirection,  angularPrecision ) != 0 ) {
        if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
        speakerDescription->ElevationDirection = (int)data;
      }
    }

    if ( angularPrecision == 0 ) {
      if( ERROR_NONE != s_GetBits2(bs, 6, &data) ) goto error;
      speakerDescription->AzimuthAngleIdx = (int)data;
    } else {
      if( ERROR_NONE != s_GetBits2(bs, 8, &data) ) goto error;
      speakerDescription->AzimuthAngleIdx = (int)data;
    }

    if ( ( saoc_AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx, speakerDescription->AzimuthDirection, angularPrecision ) != 0 ) &&
         ( saoc_AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx, speakerDescription->AzimuthDirection, angularPrecision ) != 180 ) ) {
      if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
      speakerDescription->AzimuthDirection = (int)data;
    }
    if( ERROR_NONE != s_GetBits2(bs, 1, &data) ) goto error;
    speakerDescription->isLFE = (int)data;
    
  }

  return ERROR_NONE;

error:
  errorMessage = "Error parsing SAOCFrame.";
  return ERROR_PARSING_BITSTREAM;
}