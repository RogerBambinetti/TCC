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

#ifndef __SAOC_DRC_LFE_H__
#define __SAOC_DRC_LFE_H__

#include "saocdeclib.h"
#include "error.h"
#include "saoc_const.h"
#include "saoc_interpolate.h"
#include "saoc_kernels.h"
#include "qmflib.h"
#include "qmflib_hybfilter.h"
#include "qmflib_const.h"

typedef struct sSaocDrcLfe {
          
         float RenMat[SAOC_MAX_PARAM_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS];

         QMFLIB_POLYPHASE_ANA_FILTERBANK** pAnalysisQmf;
         QMFLIB_POLYPHASE_SYN_FILTERBANK** pSynthesisQmf;
         QMFLIB_HYBRID_FILTER_STATE hybFilterState[SAOC_MAX_LFE_CHANNELS];
         QMFLIB_HYBRID_FILTER_MODE hybridMode;

} tSaocDrcLfe;

tSaocDrcLfe *saoc_DrcLfeInterfaceOpen( int isInQmf,
                                       int isOutQmf,
                                       int numInLFEs,
                                       int numOutLFEs,
                                       int nQmfBands);

void saoc_DrcLfeInterfaceClose(tSaocDrcLfe *self,
                               int isInQmf,
                               int isOutQmf,
                               int numInLFEs,
                               int numOutLFEs);

void saoc_DrcLfeInterfaceApply(tSaocDec          *dec, 
                               tSaocDrcLfe       *self,
                               int               saocInputLFE_Mapping[SAOC_MAX_LFE_CHANNELS ],
                               int               saocOutputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                               float             pRenderMat_lfe[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_LFE_CHANNELS], 
                               float***          drcMatrix
                               );
#endif 