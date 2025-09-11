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

/****************************************************************************/
/* SAOC Bitstream Reader                                                    */
/****************************************************************************/

#ifndef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#define RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#endif

#ifndef RM2_3D_BUGFIX_SAOC_3
#define RM2_3D_BUGFIX_SAOC_3
#endif

#ifndef WD2_SAOC_FLEXIBLE_LAYOUT_3D
#define WD2_SAOC_FLEXIBLE_LAYOUT_3D
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "saoc_bitstream.h"
#include "saoc_types.h"
#include "const.h"
#include "error.h"
#include "saoc_nlc_enc.h"

/****************************************************************************/
/* Globals                                                                  */
/****************************************************************************/

/* #pragma warning(disable : 1418) */
char *errorMessage;


static int FreqResStrideTable[] = {1, 2, 5, 28};
static int pbStrideTable[]   = {1, 2, 5, 28};
static int FreqResBinTable[] = {0, 28, 20, 14, 10, 7, 5, 4};


/*******************************************************************************
 Functionname: coarse2fine
 *******************************************************************************

 Description:
   Parameter index mapping from coarse to fine quantization

 Arguments:

Input:

Output:

*******************************************************************************/
static void
coarse2fine( int* data, DATA_TYPE dataType, int numBands ) {

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

}


/*******************************************************************************
 Functionname: fine2coarse
 *******************************************************************************

 Description:
   Parameter index mapping from fine to coarse quantization

 Arguments:

Input:

Output:

*******************************************************************************/
static void
fine2coarse( int* data, int numBands ) {

  int i;

  for( i=0; i<numBands; i++ ) {
    data[i] >>= 1;
  }

}


/*******************************************************************************
 Functionname: getStrideMap
 *******************************************************************************

 Description:
   Index Mapping accroding to pbStrides

 Arguments:

Input:

Output:

*******************************************************************************/
static int
getStrideMap(int freqResStride, int numBands, int* aStrides) {

  int i, pb, pbStride, dataBands, strOffset;

  pbStride = pbStrideTable[freqResStride];
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


/* destroy decoder instance */
void DestroySAOCDecoder(BS_INSTANCE **selfPtr) {
  *selfPtr = NULL;
}

/* create decoder instance */
int CreateSAOCDecoder(BS_INSTANCE **selfPtr) {

  BS_INSTANCE *self = NULL;

  /* allocate decoder struct */
  self = calloc(1, sizeof(*self));
  if(self == NULL) {
    errorMessage = "Could not allocate decoder.";
    return ERROR_NO_FREE_MEMORY;
  }

  *selfPtr = self;

  return ERROR_NONE;
}

/* get SAOCSpecificConfig struct */
SAOCSPECIFICCONFIG *GetSAOCSpecificConfig(BS_INSTANCE *selfPtr) {
  return &selfPtr->saocSpecificConfig;
}

/*  write SAOC data to bitstream  */
static void
ecDataSAOC_write(Stream                *bitstream,
		             PREV_NEW_LOSSLESSDATA *lldata,
                 DATA_TYPE             dataType,
			           int                   numParamSets,
                 int                   independencyFlag,
			           int                   startBand,
			           int                   numBands )
{
  int i, setIdx, ps, ps2, pb, pbStride, numDataSets, numDataBands, strOffset;
  int si2ps[MAX_NUM_PARAMS];
  int bsDataPair[MAX_NUM_PARAMS];
  int aStrides[max(MAX_NUM_BINS,MAX_OBJECTS)+1];
  int cmpData[2][max(MAX_NUM_BINS,MAX_OBJECTS)];
  int cmpPrevData[max(MAX_NUM_BINS,MAX_OBJECTS)];

  SAOCLOSSLESSDATA (*data)[MAX_NUM_PARAMS] = &lldata->cur_data;
  SAOCLOSSLESSDATA *prevData = &lldata->prev_data;


  /*  determine bsDataMode */
  if( ((*data)[0].bsQuantCoarse != prevData->bsQuantCoarse) ||
	  ((numParamSets == 1) && independencyFlag)
	)
  {
	(*data)[0].bsDataMode = FINECOARSE;
  }
  else {
    (*data)[0].bsDataMode = KEEP;
    for( i=startBand; i<startBand+numBands; i++ ) {
	  if( (*data)[0].qval[i] != prevData->qval[i] ) {
        (*data)[0].bsDataMode = FINECOARSE;
        break;
      }
    }
  }
  writeBits(bitstream, (*data)[0].bsDataMode, 2);

  for( ps=1; ps<numParamSets; ps++ ) {
    if ( ((ps == 1) && independencyFlag && ((*data)[0].bsDataMode == KEEP)) ||
		 ((*data)[ps].bsQuantCoarse != (*data)[ps-1].bsQuantCoarse)
	   )
    {
	  (*data)[ps].bsDataMode = FINECOARSE;
    }
    else{
      (*data)[ps].bsDataMode = KEEP;
      for( i=startBand; i<startBand+numBands; i++ ) {
		if( (*data)[ps].qval[i] != (*data)[ps-1].qval[i] ) {
		  (*data)[ps].bsDataMode = FINECOARSE;
		  break;
		}
      }
    }
    writeBits(bitstream, (*data)[ps].bsDataMode, 2);
  }

  /*  bsDataPair */
  setIdx = 0;
  for( ps=0; ps<numParamSets; ps++ ) {
    if( (*data)[ps].bsDataMode == FINECOARSE ) {
      si2ps[setIdx] = ps;
	  bsDataPair[setIdx] = ( (ps < numParamSets-1) && ((*data)[ps+1].bsDataMode == FINECOARSE) );
      if( bsDataPair[setIdx] ) {
        bsDataPair[setIdx+1] = 1;
		si2ps[++setIdx] = ++ps;
      }
	  ++setIdx;
    }
  }
  numDataSets = setIdx;

  setIdx = 0;
  while( setIdx < numDataSets ) {

    ps = si2ps[setIdx];
    if( bsDataPair[setIdx] ) {
	  ps2 = si2ps[setIdx+1];
	}

	if( numDataSets-setIdx > 1 ) {
      writeBits( bitstream, bsDataPair[setIdx], 1 );
      if( bsDataPair[setIdx] ) {
        assert( (*data)[ps2].bsQuantCoarse   == (*data)[ps].bsQuantCoarse   );
        assert( (*data)[ps2].bsFreqResStride == (*data)[ps].bsFreqResStride );
	  }
	}
	else {
	  assert( bsDataPair[setIdx] == 0 );
	}

    writeBits(bitstream, (*data)[ps].bsQuantCoarse  , 1);
    writeBits(bitstream, (*data)[ps].bsFreqResStride, 2);

	/*  coarse <=> fine conversion */
    if( (*data)[ps].bsQuantCoarse != prevData->bsQuantCoarse ) {
      if( prevData->bsQuantCoarse ) {
        coarse2fine( prevData->qval, dataType,  numBands );
      }
      else {
        fine2coarse( prevData->qval, numBands );
      }
    }

    pbStride = FreqResStrideTable[(*data)[ps].bsFreqResStride];
    numDataBands = (numBands-1)/pbStride+1;

    aStrides[0] = startBand;
    for( pb=1; pb<=numDataBands; pb++ ) {
      aStrides[pb] = aStrides[pb-1] + pbStride;
    }
    strOffset = 0;
    while( aStrides[numDataBands] > startBand+numBands ) {
	  if( strOffset < numDataBands ) {
		strOffset++;
	  }
      for( i=strOffset; i<=numDataBands; i++ ) {
        aStrides[i]--;
      }
    }

    for( pb=0; pb<numDataBands; pb++ ) {
	  cmpPrevData[pb] = prevData->qval[aStrides[pb]];
      cmpData[0][pb]  = (*data)[ps].qval[aStrides[pb]];
      for( i=aStrides[pb]+1; i<aStrides[pb+1]; i++ ) {
        assert( (*data)[ps].qval[i] == cmpData[0][pb] );
      }
      if( bsDataPair[setIdx] ) {
        cmpData[1][pb] = (*data)[ps2].qval[aStrides[pb]];
        for( i=aStrides[pb]+1; i<aStrides[pb+1]; i++ ) {
          assert( (*data)[ps2].qval[i] == cmpData[1][pb] );
        }
      }
    }


    EcDataPairEnc_SAOC( bitstream,
		       cmpData,
					 cmpPrevData,
					 dataType, 
					 setIdx, 0,
					 numDataBands, 
           bsDataPair[setIdx],
					 (*data)[ps].bsQuantCoarse, 
					 independencyFlag );

    if( bsDataPair[setIdx] ) {
      memcpy( prevData->qval+startBand, (*data)[ps2].qval+startBand, numBands*sizeof(int) );
	}
    else {
      memcpy( prevData->qval+startBand, (*data)[ps].qval+startBand, numBands*sizeof(int) );
    }

    prevData->bsDataMode      = (*data)[ps].bsDataMode;
	prevData->bsQuantCoarse   = (*data)[ps].bsQuantCoarse;
	prevData->bsFreqResStride = (*data)[ps].bsFreqResStride;

    if( bsDataPair[setIdx] ) {
	  setIdx += 2;
	}
	else {
	  setIdx += 1;
	}
  }
}

 



















/* get SAOCFrame struct */
SAOCFRAME* GetSAOCFrame(BS_INSTANCE *selfPtr) {
  return &selfPtr->currentSAOCFrame;
}


static float dequantOLD[] = {-150.0f,    -45.0f,     -40.0f,     -35.0f,     -30.0f,     -25.0f,     -22.0f,     -19.0f,     -16.0f,     -13.0f,    -10.0f,    -8.0f,     -6.0f,     -4.0f,     -2.0f,     0.0f};
static float dequantIOC[] = {1.f, 0.937f, 0.84118f, 0.60092f, 0.36764f, 0.f, -0.589f, -0.99f};
static float dequantCLD[] = {-150, -45, -40, -35, -30, -25, -22, -19, -16, -13, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 13, 16, 19, 22, 25, 30, 35, 40, 45, 150};
static float dequantNRG[] = {-94.5f, -93.0f, -91.5f, -90.0f, -88.5f, -87.0f, -85.5f, -84.0f, -82.5f, -81.0f, -79.5f, -78.0f, -76.5f, -75.0f, -73.5f, -72.0f, -70.5f, -69.0f, -67.5f, -66.0f, -64.5f, -63.0f, -61.5f, -60.0f, -58.5f, -57.0f, -55.5f, -54.0f, -52.5f, -51.0f, -49.5f, -48.0f, -46.5f, -45.0f, -43.5f, -42.0f, -40.5f, -39.0f, -37.5f, -36.0f, -34.5f, -33.0f, -31.5f, -30.0f, -28.5f, -27.0f, -25.5f, -24.0f, -22.5f, -21.0f, -19.5f, -18.0f, -16.5f, -15.0f, -13.5f, -12.0f, -10.5f, -9.0f, -7.5f, -6.0f, -4.5f, -3.0f, -1.5f, 0.0f};


static float
deq( int value,
    int paramType)
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


void GetSaocOld(float pOld[MAX_OBJECTS][MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{

  int j,k;
  for(j=0;j<nNumObjects;j++) {
    for(k=0;k<nNumBins;k++) {
      
      pOld[j][k]=deq(pFrameDataSaoc->old[j].cur_data[nParamSet].qval[k],t_OLD);
      
    }
  }
}

void GetSaocIoc(float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{
  int j,k,l;
  
  for(j=0;j<nNumObjects;j++) {
    for(k=0;k<nNumObjects;k++) {
      for(l=0;l<nNumBins;l++) {
        
        pIoc[j][k][l]=deq(pFrameDataSaoc->ioc[j][k].cur_data[nParamSet].qval[l],t_IOC);
        
      }
    }
  }
}

void GetSaocDmxMat(float pDmxMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumDmxChannels, int nNumBins)
{
  int l,j;
  
  for(l=0;l<nNumObjects;l++) {
    for(j=0;j<nNumDmxChannels;j++) {
      pDmxMat[j][l] = deq(pFrameDataSaoc->dmxMat[j].cur_data[nParamSet].qval[l],t_CLD);
    }
  }
}




/* destroy encoder instance */
void DestroySAOCEncoder(BS_INSTANCE **selfPtr) {
  *selfPtr = NULL;
}



/* create encoder instance */
int CreateSAOCEncoder(BS_INSTANCE **selfPtr) {
  BS_INSTANCE *self      = NULL;

  /* allocate encoder struct */
  self = calloc(1, sizeof(*self));

  if(self == NULL) {
    errorMessage = "Could not allocate encoder";
    return ERROR_NO_FREE_MEMORY;
  }

  *selfPtr = self;

  return ERROR_NONE;
}



/* write spatialSpecificConfig to stream */
int WriteSAOCSpecificConfig(Stream *bitstream, BS_INSTANCE *selfPtr) {

  int numObj, obj1, obj2, rt12;
  int objIsGrouped[MAX_OBJECTS] = {0};

  /* SAOCFRAME          *frame  = &(selfPtr->currentSAOCFrame); */
  SAOCSPECIFICCONFIG *config = &(selfPtr->saocSpecificConfig);

/*   selfPtr->numBins = FreqResBinTable[config->bsFreqRes]; */

  /* write to bitstream */
  writeBits(bitstream, config->bsSamplingFreqIdx, 4);
  if(config->bsSamplingFreqIdx == 15) {
    writeBits(bitstream, config->bsSamplingFrequency, 24);
  }


#ifdef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
  /*writeBits(bitstream, config->bsLowDelayMode        , 1);*/
#endif
  
  writeBits(bitstream, config->bsFreqRes        , 3);

  writeBits(bitstream, config->bsDoubleFrameLengthFlag    , 1);
  
  /*if (config->bsLowDelayMode == 0) {
    writeBits(bitstream, config->bsFrameLength    , 7);
  } else {
    writeBits(bitstream, config->bsFrameLength    , 5);
  }*/

#ifdef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
  writeBits(bitstream, config->bsNumSaocDmxChannels , 5);
  writeBits(bitstream, config->bsNumSaocDmxObjects  , 5);
#endif

#ifdef RM2_3D_BUGFIX_SAOC_3 
  writeBits(bitstream, config->bsDecorrelationMethod  , 1);
#else
  writeBits(bitstream, config->bsDecorrelationMethod  , 2);
#endif

  if (config->bsNumSaocDmxChannels > 0) {
#ifdef WD2_SAOC_FLEXIBLE_LAYOUT_3D
    /* Channel path not active in Reference Software SAOC 3D encoder */
#else
    writeBits(bitstream, config->bsNumSaocChannels    , 6);
    writeBits(bitstream, config->bsNumSaocLFEs        , 2);
#endif
  }
  writeBits(bitstream, config->bsNumSaocObjects     , 8);

#ifndef RM2_3D_BUGFIX_SAOC_3 
  writeBits(bitstream, config->bsDecorrelationLevel   , 2);
#endif

  /* relatedTo */
  numObj = config->bsNumSaocChannels;
  for( obj1=0; obj1<numObj; obj1++ ) {
	  for( obj2=obj1+1; obj2<numObj; obj2++ ) {
	    if( objIsGrouped[obj2] ) continue;
	    rt12 = config->bsRelatedTo[obj1][obj2];
	    writeBits(bitstream, rt12, 1);
	    if( rt12 ) {
        objIsGrouped[obj1] = 1;
        objIsGrouped[obj2] = 1;
	    }
	  }
  }

  /* relatedTo */
  numObj = config->bsNumSaocObjects;
  for( obj1=0; obj1<numObj; obj1++ ) {
	  for( obj2=obj1+1; obj2<numObj; obj2++ ) {
	    if( objIsGrouped[obj2] ) continue;
	    rt12 = config->bsRelatedTo[obj1][obj2];
	    writeBits(bitstream, rt12, 1);
	    if( rt12 ) {
        objIsGrouped[obj1] = 1;
        objIsGrouped[obj2] = 1;
	    }
	  }
  }

  writeBits(bitstream, config->bsOneIOC, 1);

#ifdef WD2_SAOC_FLEXIBLE_LAYOUT_3D
  writeBits(bitstream, config->bsSaocDmxMethod , 1);
#else
  writeBits(bitstream, config->bsSaocDmxMethod , 4);
  if (config->bsSaocDmxMethod == 15) {
    writeBits(bitstream, config->bsNumPreMixedObjects , 5);
  }
#endif

#ifndef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
  writeBits(bitstream, config->Reserved, 3);
#endif

  writeBits(bitstream, config->bsDualMode, 1);
  if (config->bsDualMode > 0) {
    writeBits(bitstream, config->bsBandsLow, 5);
  }

  writeBits(bitstream, config->bsDcuFlag, 1);

  /* Byte alignment */  
  ByteAlignWrite(bitstream);

  return ERROR_NONE;
}

int WriteSAOCFrame(Stream *bitstream, BS_INSTANCE *selfPtr) {

  int i, j;

  SAOCFRAME *frame = &(selfPtr->currentSAOCFrame);
  SAOCSPECIFICCONFIG *config = &(selfPtr->saocSpecificConfig);

  int bsIndependencyFlag = frame->bsIndependencyFlag;
  int numParamSets = frame->framingInfo.bsNumParamSets;
  int numObj = config->bsNumObjects+1;

  int freqRes = FreqResBinTable[config->bsFreqRes];
  int dmgIdx, numDmg;
  int bitsParamSlot;

  /* DEBUG */
#define __DEBUG_PRINT
#ifdef DEBUG_PRINT
  FILE *fSAOCData = fopen("SAOCFrameDataEncoder.txt","at");
#endif
  /* DEBUG */

  /* selfPtr->frameCounter++; */

  writeBits(bitstream, frame->framingInfo.bsFramingType, 1);
  writeBits(bitstream, numParamSets-1, 3);

  /* DEBUG */
#ifdef DEBUG_PRINT
  fprintf(fSpatialData,"bsFramingType %d\n",frame->framingInfo.bsFramingType);
  fprintf(fSpatialData,"self->numParameterSets %d\n",numParamSets);
#endif
  /* DEBUG */

  if(frame->framingInfo.bsFramingType) {
    for(i = 0; i < numParamSets; i++) {
      bitsParamSlot = 0;
      while( (1<<bitsParamSlot) < (config->bsFrameLength+1) ) {
		 bitsParamSlot++;
	  }
      if(bitsParamSlot > 0) {
		writeBits(bitstream, frame->framingInfo.bsParamSlots[i], bitsParamSlot);
	  }
      
#ifdef DEBUG_PRINT
      fprintf(fSpatialData,"self->paramSlot[%d] %d\n",i,frame->framingInfo.bsParamSlots[i]);
#endif
      
    }
  }

  writeBits(bitstream, frame->bsIndependencyFlag, 1);

  /* DEBUG */
#ifdef DEBUG_PRINT
  fprintf(fSpatialData,"frame->bsIndependencyFlag %d\n",frame->bsIndependencyFlag);
#endif
  /* DEBUG */


  /* OLD data */
  for( i=0; i<numObj; i++ ) {
     ecDataSAOC_write( bitstream,
                 &frame->old[i],
                 t_OLD,
			     numParamSets,
                 bsIndependencyFlag,
				 0,
				 freqRes );
     if (frame->old[i].cur_data[0].bsDataMode==2 || frame->old[i].cur_data[1].bsDataMode==2) {
      j = 0;
     }
  }
 
  /* IOC data */
  for( i=0; i<numObj; i++ ) {
    for( j=i+1; j<numObj; j++ ) {
      if( config->bsRelatedTo[i][j] ) {
        ecDataSAOC_write( bitstream,
          &frame->ioc[i][j],
          t_ICC,
          numParamSets,
          bsIndependencyFlag,
          0,
          freqRes );
      }
    }
  }

 
    for( i=0; i<config->bsNumSaocDmxChannels; i++ ) {
      ecDataSAOC_write( bitstream,
                        &frame->dmxMat[i],
                        t_CLD,
                        numParamSets,
                        bsIndependencyFlag,
		                    0,
			                  config->bsNumSaocChannels);
    }

    dmgIdx = config->bsNumSaocDmxChannels;
    if (config->bsSaocDmxMethod == 1) {
      numDmg = config->bsNumPreMixedObjects;
    } else {
      numDmg = config->bsNumSaocObjects;
    }

    for( i=dmgIdx ; i<dmgIdx + config->bsNumSaocDmxObjects; i++ ) {
      ecDataSAOC_write( bitstream,
                        &frame->dmxMat[i],
                        t_CLD,
                        numParamSets,
                        bsIndependencyFlag,
		                    0,
			                  numDmg);
    }

  /* Byte alignment */
  ByteAlignWrite(bitstream);  

#ifdef DEBUG_PRINT
  fclose(fSpatialData);
#endif

  return ERROR_NONE;
}


static int IOCQuant(float val)
{
  float pQSteps[7]= {0.9685f, 0.88909f, 0.72105f, 0.48428f, 0.18382f, -0.2945f, -0.7895f};
  int i;

  if(val>=pQSteps[0]) {
    return 0;
  }
  for(i=1;i<6;i++){
    if ((val>=pQSteps[i]) && (val<=pQSteps[i-1])) {
      return i;
    }
  }
  return 7;
}


static int quant( float value, int paramType)
{
	int i;
	double value_dB;
    float pQSteps[30]= {-47.5, -42.5, -37.5, -32.5, -27.5, -23.5, -20.5, -17.5, -14.5, -11.5, -9.0, -7.0, -5.0, -3.0, -1.0, 1.0, 3.0, 5.0, 7.0, 9.0, 11.5, 14.5, 17.5, 20.5, 23.5, 27.5, 32.5, 37.5, 42.5, 47.5};

	switch( paramType )
	{
  case t_IOC:
	
	  return IOCQuant(value);

  case t_CLD:
  
	  if(value<pQSteps[0]) {
		  return 0-15;
	  }
	  for(i=1;i<30;i++){
		  if ((value<=pQSteps[i]) && (value>=pQSteps[i-1])) {
			  return i-15;
		  }
	  }
	  return 30-15;
	  
  case t_OLD:
	  
	  value_dB = 10.f*(double)log10(value+EPSILON);
	 
	  if(value_dB<pQSteps[0]) {
		  return 0;
	  }
	  for(i=1;i<15;i++){
		  if ((value_dB<=pQSteps[i]) && (value_dB>=pQSteps[i-1])) {
			  return i;
		  }
	  }
	  return 15;

	case t_NRG:
		
	value_dB = 10.f*(float)log10(value/50.f);

	if((value_dB - 0.75)>=dequantNRG[63]) 
	{
		return 63;
	}
	else
	{
		for(i=1;i<64;i++)
		{
			if (((value_dB - 0.75)>=dequantNRG[i-1]) && ((value_dB - 0.75)<=dequantNRG[i])) 
			{
				return i;
			}
		}
		return 0;
	}

	case t_DMG:
		return (int) value;

	default:
		assert(0);
		return 0;
  }
}



void SetSaocOld(float pOld[MAX_OBJECTS][MAX_PARAM_BANDS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{
  int j,k;
	for(j=0;j<nNumObjects;j++) 
	{
		for(k=0;k<nNumBins;k++) 
		{
			pFrameDataSaoc->old[j].cur_data[nParamSet].qval[k]=quant(pOld[j][k],t_OLD);
    }
	}
}

void SetSaocIoc(float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins)
{
	int j,k,n;
		for(n=0;n<nNumObjects;n++)
		{
			for(j=0;j<nNumObjects;j++)
			{
				for(k=0;k<nNumBins;k++)
				{
					pFrameDataSaoc->ioc[j][n].cur_data[nParamSet].qval[k]=quant(pIoc[j][n][k],t_IOC);
				}
			}
		}
}


void SetSaocDmxMat(float pDmxMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumDownmixChannels, int nNumBins)
{
	int i,j;
	for(i=0;i<nNumObjects;i++) {
    for(j=0;j<nNumDownmixChannels;j++) {
		  pFrameDataSaoc->dmxMat[j].cur_data[nParamSet].qval[i]=quant(pDmxMat[j][i],t_CLD);
    }
	}
}