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

#ifndef __SAOC_DEC_H__
#define __SAOC_DEC_H__

#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h> 
#include <stdio.h> 

#include "saoc_render.h"
#include "saoc_decor.h"
#include "cicp2geometry.h"

#include "wavIO.h"
#include "saoc_decode.h"
#include "qmflib.h"
#include "qmflib_hybfilter.h"

#include "saoc_spatial_filereader.h"
#include "saoc_bitstream.h"
#include "saoc_decor.h"



typedef struct sSaocDec {
		  
		 int isInQmf;
		 int isOutQmf;
		 int frameSize;
		 int nQmfBands;
		 int nHybridBands;
		 int nTimeSlots;
		 int LdMode;
		 int moduleDelay;
		 int dualMode;
		 int bandsLow;
		 int numParamBands;

		 int nChannelsIn;
		 int nChannelsOut;
		 int numOutLFEs; 
		 int numInLFEs;
		 
		 int nSaocChannels;
		 int nSaocObjects;
		 int nSaocDmxChannels;
		 int nSaocDmxObjects;
		 int nNumPreMixedChannels;
		 int RelatedToMatrix[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS];
		 double sampleFreq ;
		 
		 unsigned int DecorrelationLevel;
		 unsigned int DecorrMethod;
		 unsigned int numDecorrelators;


		 HANDLE_S_BITINPUT bitstreamIn;
		 BSR_INSTANCE*     pBsrInstance;

		 SAOC_HANDLE_DECORR_DEC hDecorrDec[SAOC_MAX_RENDER_CHANNELS];
		 unsigned startHybBand;

		 QMFLIB_POLYPHASE_ANA_FILTERBANK** pAnalysisQmf;
		 QMFLIB_POLYPHASE_SYN_FILTERBANK** pSynthesisQmf;
		 QMFLIB_HYBRID_FILTER_STATE hybFilterState[SAOC_MAX_CHANNELS];
		 QMFLIB_HYBRID_FILTER_MODE hybridMode;
		 int hybridHFAlign;

		 unsigned int qmfSamplesPcDelay;

		 float **qmfInRealBuffer;
		 float **qmfInImagBuffer;
		 float **tmpInRealQmfBuffer;
		 float **tmpInImagQmfBuffer;
		 float **qmfOutRealBuffer;
		 float **qmfOutImagBuffer;
		 float **inBuffer;
		 float **outBuffer;
		 float **tmpInBuffer;
		  
		 float RG_matrix[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
		 float pP_dry[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
		 float pP_wet[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];

		 float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
		 float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];
		 float Hproto[SAOC_MAX_RENDER_CHANNELS][2*SAOC_MAX_RENDER_CHANNELS];

		 int inSaocCICPIndex;
		 CICP2GEOMETRY_CHANNEL_GEOMETRY saocInputGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
		 SAOC_3D_SPEAKER_CONFIG_3D premixChannelLayout;


} tSaocDec;

tSaocDec *saoc_DecoderOpen( int isInQmf,
							int isOutQmf,
							int nChannelsOut,
							int numOutLFEs,
							QMFLIB_HYBRID_FILTER_MODE hybridMode,
							int hybridHFAlign,
							unsigned int *nIn_WAV_IO_Channels,
							unsigned int *nOut_WAV_IO_Channels,
							char* inbsFilename,
							int sacHeaderLen,
							int outCICPIndex,
							CICP2GEOMETRY_CHANNEL_GEOMETRY* saocOutputGeometryInfo,
							int numPremixChannels,
							int coreOutputFrameLength,
							int *error_init);

void saoc_DecoderApply( tSaocDec *self,
						unsigned int      isLastFrame,
						int               numOamPerFrame,
						int               framePossOAM[32],
						float             frameRenderMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] ,
						float             framePreMixMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
						int               coder_offset,
						int               use_ren_mat_files,
						float             ***pRenderMats,
						float             *time_instances,
						int               num_time_instances,
						float             pRenderMat_ch[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
						INVERSION_PARAMETERS inversionParam,
						int               saocInputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
						int               saocOutputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
						int               drcFlag,
						float***          drcMatrix,
						int               *frameLength);

void saoc_DecoderClose( tSaocDec *self, 
						unsigned int nIn_WAV_IO_Channels,
						unsigned int nOut_WAV_IO_Channels
					  );



#endif
