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

#ifndef SAC_ENC_H
#define SAC_ENC_H

#ifndef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#define RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#endif

#include "stdio.h"
#include "defines.h"
#include "const.h"
#include "saoc_bitstream.h"
#include "qmflib.h"
#include "qmflib_hybfilter.h"

typedef struct sSpatialEnc {
  int treeConfig;
  int outputChannels;
  int timeSlots;
  int frameSize;

  QMFLIB_POLYPHASE_ANA_FILTERBANK** pAnalysisQmf;
  QMFLIB_HYBRID_FILTER_STATE hybFilterState[MAX_NUM_INPUTCHANNELS];

  BS_INSTANCE *bitstreamFormatter;

} tSpatialEnc;

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
                         unsigned int n3DAdmxObjects);

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
                  int LdMode);


void SaocEncClose(tSpatialEnc *self, int nChannelsIn);

#endif
