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


#ifndef __SAOC_DECODE_H__
#define __SAOC_DECODE_H__

#include "saoc_const.h"
#include "saoc_decor.h"
#include "saoc_interpolate.h"

typedef struct {
  float inversion_reltol;
  float inversion_abstol;
  int omiteigvecs;
} INVERSION_PARAMETERS;

typedef struct saocCoreGroupList {
	int numDmx;
	int numObj;
	int dmxList[SAOC_MAX_DMX_CHANNELS];
	int objList[SAOC_MAX_OBJECTS];
} saocCoreGroupList;

typedef struct saocCoreGroup {
  int numGroups;
  saocCoreGroupList saocGroup[SAOC_MAX_DMX_CHANNELS];
} saocCoreGroup;

void saoc_ParametersEstimation(float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
				float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],
				float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
				float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
				float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
				float Hproto[SAOC_MAX_RENDER_CHANNELS][2*SAOC_MAX_RENDER_CHANNELS],
				float pRenderMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAMETER_BANDS],
				int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS],
				int DecorrMethod,
				int numDecorrelators,
				int numOutCh,
				int nSaocChannels,
				int nSaocObjects,
				int nSaocDmxChannels,
				int nSaocDmxObjects,  
				int nNumBins,
				int iset,
				INVERSION_PARAMETERS inversionParam,
				int dualMode,
				int bandsLow,
				float RG[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS], 
				float pP_dry[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
				float pP_wet[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS]
				);


void saoc_Processing(float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS],
			   float RG[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],      
			   float pP_dry[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
			   float pP_wet[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
			   float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
			   float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
			   SAOC_HANDLE_DECORR_DEC hDecorrDec[SAOC_MAX_RENDER_CHANNELS],
			   int DecorrMethod,
			   int numDecorrelators,
			   int startHybBand,
			   int numUpmx,
			   int lowDelay,
			   float pHybReal[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS],
			   float pHybImag[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS],
			   int numDmx,
			   int numObj,
			   int nNumBins,
			   int nHybridBands,
			   int nParamSets,
			   int pParamSlot[SAOC_MAX_PARAM_SETS],
			   int frameLength);

#endif
