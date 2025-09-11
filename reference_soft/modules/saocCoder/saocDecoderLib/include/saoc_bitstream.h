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




 
#ifndef _SAOC_BITSTREAM_H
#define _SAOC_BITSTREAM_H

#include "saoc_const.h"
#include "saoc_bitinput.h"

#include "saoc_machine.h"
#include "cicp2geometry.h"

#define SAOC_MAX_PARAM_SETS        10
#define MAX_NUM_SEPARATION_DATA	   16
#define MAX_NUM_PRESET_USER_DATA_IDENTIFIER 6


typedef struct {
  int                     qval[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)];
  int                     bsDataMode;
  int                     bsQuantCoarse;
  int                     bsFreqResStride;
} SAOCLOSSLESSDATA;

typedef struct {
  SAOCLOSSLESSDATA        prev_data;
  SAOCLOSSLESSDATA        cur_data[SAOC_MAX_PARAM_SETS];
} PREV_NEW_LOSSLESSDATA;

typedef enum {
  ONLYLONG_WINDOW	= 0,
  LONGSTART_WINDOW,
  EIGHTSHORT_WINDOW,
  LONGSTOP_WINDOW,
  NUMWIN_SEQ
} BLOCKTYPE;

typedef struct {
  int                     bsIndependencyFlag;
  FRAMINGINFO             framingInfo;
  PREV_NEW_LOSSLESSDATA   old[SAOC_MAX_OBJECTS];
  PREV_NEW_LOSSLESSDATA   ioc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS];
  PREV_NEW_LOSSLESSDATA   dmxMat[SAOC_MAX_DMX_CHANNELS];

  int						bsDcuDynamicUpdate;
  int						bsDcuMode;  
  int						bsDcuParam;
  int						bsDcuDynamicUpdate2;           
  int						bsDcuMode2; 
  int						bsDcuParam2; 
} SAOCFRAME;


typedef struct {
  UINT32 isCICPspeakerIdx;
  UINT32 CICPspeakerIdx;
  UINT32 ElevationClass;
  UINT32 ElevationAngleIdx; 
  UINT32 ElevationDirection; 
  UINT32 AzimuthAngleIdx;
  UINT32 AzimuthDirection; 
  UINT32 isLFE;

} SAOC_3D_SPEAKER_DESCRIPTION;

typedef struct {

  UINT32 angularPrecision;
  SAOC_3D_SPEAKER_DESCRIPTION speakerDescription[SAOC_MAX_CHANNELS];

} SAOC_3D_FLEXIBLE_SPEAKER_CONFIG;


typedef struct {
	UINT32 speakerLayoutID;
	UINT32 CICPspeakerLayoutIdx;
  UINT32 numSpeakers;
  
  UINT32 CICPspeakerIdx[SAOC_MAX_CHANNELS];

  SAOC_3D_FLEXIBLE_SPEAKER_CONFIG flexibleSpeakerConfig;

} SAOC_3D_SPEAKER_CONFIG_3D;

typedef struct {
  int           bsSamplingFreqIdx;
  int           bsSamplingFrequency;
  int					  bsLowDelayMode;
  int					  bsBandsHigh;
  int					  bsBandsLow;

  int           bsFreqRes;
  int           bsNumBins;
  int           bsFrameLength;
  int           bsDoubleFrameLengthFlag;

  int           bsNumSaocChannels;
  int           bsNumSaocObjects;
  int           bsNumSaocLFEs;
  int           bsNumSaocDmxChannels;
  int           bsNumSaocDmxObjects;
  int           bsNumPreMixedChannels;

  int           NumInputSignals;

  int           bsDecorrelationMethod;
  int           bsDecorrelationLevel;
  int           bsDualMode;

  int           bsReserved;

  int					  bsOneIOC;
  int           bsRelatedTo[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS];
  int           bsSaocDmxMethod; 
  int           Reserved;

  int           bsSacExtType[SAOC_MAX_EXT_CNT]; 
  int					  SaocExtNum; 

  int						bsDcuFlag;
	int						bsDcuMandatory; 
	int						bsDcuDynamic;
	int						bsDcuMode;
	int						bsDcuParam; 
	int						bsDcuFlag2; 
	int						bsDcuMode2; 
	int						bsDcuParam2; 
  int           inSaocCICPIndex;
  CICP2GEOMETRY_CHANNEL_GEOMETRY saocInputGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS]; /*[AZ EL isLFE] */
  CICP2GEOMETRY_CHANNEL_GEOMETRY saocPremixGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS]; /*[AZ EL isLFE] */

} SAOCSPECIFICCONFIG;

typedef struct {

  SAOCSPECIFICCONFIG      saocSpecificConfig;
  SAOCFRAME               currentSAOCFrame;
    
  int                     numBins;
  int                     totalBits;
  int                     frameCounter;
} BSR_INSTANCE;


void DestroySAOCDecoder(BSR_INSTANCE **selfPtr);
int  CreateSAOCDecoder(BSR_INSTANCE **selfPtr);

int ReadSAOCSpecificConfig(HANDLE_S_BITINPUT bitstream, 
                           BSR_INSTANCE *selfPtr,
													 int sacHeaderLen,
													 int numPremixedChannels);

int  PrintSAOCSpecificConfig(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE *selfPtr);
int  PrintSAOCSpecificFrame(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE *selfPtr);

SAOCSPECIFICCONFIG *GetSAOCSpecificConfig(BSR_INSTANCE *selfPtr);
SAOCFRAME* GetSAOCFrame(BSR_INSTANCE *selfPtr);
int ReadSAOCFrame(HANDLE_S_BITINPUT bitstream, BSR_INSTANCE* selfPtr);

void GetSaocOld(float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void GetSaocIoc(float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS],SAOCFRAME* pFrameDataSaoc, int nParamSet, int nNumObjects, int nNumBins);
void SAOC_GetSaocDmxMat(float pDmxMat[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS], SAOCFRAME* pFrameDataSaoc, int nParamSet, 
                                int nSaocChannels,
                                int nSaocObjects,
                                int nSaocDmxChannels,
                                int nSaocDmxObjects,
                                int nNumPremixedChannels);

#endif
