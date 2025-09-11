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

#ifndef _SAOC_BITSTREAM_H
#define _SAOC_BITSTREAM_H

#ifndef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#define RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
#endif

#include "saoc_stream.h"
#include "const.h"
#include "saoc_bitinput.h"

#define MAX_OBJECTS    32
#define MAX_PARAM_SETS  8
#define MAX_NUM_PARAMS  8

typedef struct {
  int                     qval[max(MAX_NUM_BINS,MAX_OBJECTS)];
  int                     bsDataMode;
  int                     bsQuantCoarse;
  int                     bsFreqResStride;
} SAOCLOSSLESSDATA;

typedef struct {
  SAOCLOSSLESSDATA        prev_data;
  SAOCLOSSLESSDATA        cur_data[MAX_PARAM_SETS];
} PREV_NEW_LOSSLESSDATA;

typedef struct {
  int                     bsIndependencyFlag;
  FRAMINGINFO             framingInfo;
  PREV_NEW_LOSSLESSDATA   old[MAX_OBJECTS];
  PREV_NEW_LOSSLESSDATA   ioc[MAX_OBJECTS][MAX_OBJECTS];
  PREV_NEW_LOSSLESSDATA   dmxMat[MAX_NUM_OUTPUTCHANNELS];
} SAOCFRAME;

typedef struct {
  int                     bsSamplingFreqIdx;
  int                     bsSamplingFrequency;

  int                     bsNumObjects;
  int                     bsNumDmxChannels;

  int                     bsNumSaocChannels;
  int                     bsNumSaocObjects;
  int                     bsNumSaocLFEs;
  int                     bsNumSaocDmxChannels;
  int                     bsNumSaocDmxObjects;
  int                     bsNumPreMixedObjects;

  int                     bsDecorrelationMethod;
  int                     bsDecorrelationLevel;

  int                     bsFreqRes;
  int                     bsNumBins;
  int                     bsFrameLength;
  int                     bsDoubleFrameLengthFlag;

  int                     bsRelatedTo[MAX_OBJECTS][MAX_OBJECTS];

  int					            bsOneIOC;
  int                     bsSaocDmxMethod; 
  int                     Reserved;
  int                     bsDcuFlag;

  int                     bsLowDelayMode;
  int                     bsDualMode;
  int                     bsBandsLow;


} SAOCSPECIFICCONFIG;

typedef struct {

  SAOCSPECIFICCONFIG      saocSpecificConfig;
  SAOCFRAME               currentSAOCFrame;

} BS_INSTANCE;

/*
typedef struct {

  SAOCSPECIFICCONFIG      saocSpecificConfig;
  SAOCFRAME               currentSAOCFrame;
  int                     numBins;
  int                     totalBits;
  int                     frameCounter;

} BSF_INSTANCE_SAOC;
*/

void DestroySAOCDecoder(BS_INSTANCE **selfPtr);
int  CreateSAOCDecoder(BS_INSTANCE **selfPtr);

void DestroySAOCEncoder(BS_INSTANCE **selfPtr);
int CreateSAOCEncoder(BS_INSTANCE **selfPtr);
int WriteSAOCFrame(Stream *bitstream, BS_INSTANCE *selfPtr);
int WriteSAOCSpecificConfig(Stream *bitstream, BS_INSTANCE *selfPtr);

SAOCSPECIFICCONFIG *GetSAOCSpecificConfig(BS_INSTANCE *selfPtr);
SAOCFRAME* GetSAOCFrame(BS_INSTANCE *selfPtr);

void GetSaocOld(float pOld[MAX_OBJECTS][MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void GetSaocIoc(float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void GetSaocDmxMat(float pDmxMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumDmxChannels, int nNumBins);

void SetSaocOld(float pOld[MAX_OBJECTS][MAX_PARAM_BANDS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void SetSaocIoc(float pIoc[MAX_OBJECTS][MAX_OBJECTS][MAX_PARAM_BANDS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void SetSaocDmxMat(float pDmxMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS], SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumDownmixChannels, int nNumBins);

#endif
