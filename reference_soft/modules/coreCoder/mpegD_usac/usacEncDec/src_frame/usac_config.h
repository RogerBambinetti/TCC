/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/

#ifndef __INCLUDED_USAC_CONFIG_H
#define __INCLUDED_USAC_CONFIG_H

#include "allHandles.h"
#include "obj_descr.h"           /* structs */
#include "common_m4a.h"
#include "usac_channelconf.h"

#define WD1_FRAMEWORK_CONFIG_3D
#define WD1_REFERENCE_LAYOUT_3D
#define WD1_ESCAPED_OBJECTS_MAPPING_3D
#define WD1_ELEMENT_MAPPING_3D
#define CD_SIGNALS_3D_SIGNAL_GROUPS

#define PDAM3_PHASE2_TCC

#define SPECIFY_INPUT_CONTENTTYPE
#define IAR 1

#define RM6_ICG_SYNTAX

#define SUPPORT_SAOC_DMX_LAYOUT

/* note: restriction of USAC_MAX_ELEMENTS here does not correspond 
   to any profile or max. allowed element number in bitstream      */
#define USAC_MAX_ELEMENTS (64) 

/* note: restriction of USAC_MAX_CONFIG_EXTENSIONS here does not correspond 
   to any profile or max. allowed element number in bitstream      */
#define USAC_MAX_CONFIG_EXTENSIONS (16)

#define USAC_MAX_COMPATIBLE_PROFILE_LEVELS (16)

typedef enum {

  USAC_ELEMENT_TYPE_INVALID = -1, /* use this for initialization only */
  USAC_ELEMENT_TYPE_SCE = 0,
  USAC_ELEMENT_TYPE_CPE = 1,
  USAC_ELEMENT_TYPE_LFE = 2,
  USAC_ELEMENT_TYPE_EXT = 3

} USAC_ELEMENT_TYPE;

typedef enum {

  USAC_SBR_RATIO_INDEX_NO_SBR = 0,
  USAC_SBR_RATIO_INDEX_4_1 = 1,
  USAC_SBR_RATIO_INDEX_8_3 = 2,
  USAC_SBR_RATIO_INDEX_2_1 = 3

} USAC_SBR_RATIO_INDEX;

typedef enum {

  USAC_OUT_FRAMELENGTH_768  =  768,
  USAC_OUT_FRAMELENGTH_1024 = 1024,
  USAC_OUT_FRAMELENGTH_2048 = 2048,
  USAC_OUT_FRAMELENGTH_4096 = 4096

} USAC_OUT_FRAMELENGTH;

typedef enum {
  USAC_ID_EXT_ELE_UNDEFINED                = -1,
  USAC_ID_EXT_ELE_FILL                     = 0,
  USAC_ID_EXT_ELE_MPEGS                    = 1,
  USAC_ID_EXT_ELE_SAOC                     = 2,

  USAC_ID_EXT_ELE_AUDIOPREROLL             = 3,
  USAC_ID_EXT_ELE_UNI_DRC                  = 4,
  USAC_ID_EXT_ELE_OBJ                      = 5,
  USAC_ID_EXT_ELE_SAOC_3D                  = 6,
  USAC_ID_EXT_ELE_HOA                      = 7,
#if IAR
  USAC_ID_EXT_ELE_FMT_CNVTR                = 8,
#endif
  USAC_ID_EXT_ELE_MC                       = 9,
#ifdef PDAM3_PHASE2_TCC
  USAC_ID_EXT_ELE_TCC                      = 10,
#endif
  USAC_ID_EXT_ELE_HREP                     = 12,
  USAC_ID_EXT_ELE_ENHANCED_OBJECT_METADATA = 13,
  USAC_ID_EXT_TYPE_PRODUCTION_METADATA     = 14 

  /* elements 6 - 127 reserved for ISO use                */
  /* elements 128 and higher reserved for outside ISO use */

} USAC_ID_EXT_ELE;


typedef enum {

  USAC_CONFIG_EXT_TYPE_FILL        = 0,
  USAC_CONFIG_EXT_ICG_CONFIG       = 5,
  USAC_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET = 7

  /* elements 8 - 127 reserved for ISO use                */
  /* elements 128 and higher reserved for outside ISO use */

} USAC_CONFIG_EXT_TYPE;

typedef struct {

  DESCR_ELE tw_mdct;
  DESCR_ELE fullbandLPD;
  DESCR_ELE noiseFilling;
  DESCR_ELE enhancedNoiseFilling;
  DESCR_ELE reserved;
  DESCR_ELE igf_UseENF;
  DESCR_ELE igf_UseWhitening;
  DESCR_ELE igf_AfterTnsSynth;
  DESCR_ELE igf_IndependentTiling;
  DESCR_ELE igf_StartIdx;
  DESCR_ELE igf_UseHighRes;
  DESCR_ELE igf_StopIdx;

} USAC_CORE_CONFIG;

typedef struct {

  DESCR_ELE start_freq;
  DESCR_ELE stop_freq;
  DESCR_ELE header_extra1;
  DESCR_ELE header_extra2;
  DESCR_ELE freq_scale;
  DESCR_ELE alter_scale;
  DESCR_ELE noise_bands;
  DESCR_ELE limiter_bands;
  DESCR_ELE limiter_gains;
  DESCR_ELE interpol_freq;
  DESCR_ELE smoothing_mode;

} USAC_SBR_HEADER;

typedef struct {
  
  DESCR_ELE harmonicSBR;
  DESCR_ELE bs_interTes;
  DESCR_ELE bs_pvc;
  USAC_SBR_HEADER sbrDfltHeader;

} USAC_SBR_CONFIG;

typedef struct {

  DESCR_ELE bsFreqRes;
  DESCR_ELE bsFixedGainDMX;
  DESCR_ELE bsTempShapeConfig;
  DESCR_ELE bsDecorrConfig;
  DESCR_ELE bsHighRateMode;
  DESCR_ELE bsPhaseCoding;
  DESCR_ELE bsOttBandsPhasePresent;
  DESCR_ELE bsOttBandsPhase;
  DESCR_ELE bsResidualBands;
  DESCR_ELE bsPseudoLr;
  DESCR_ELE bsEnvQuantMode;

} USAC_MPS212_CONFIG;

typedef struct {

  USAC_CORE_CONFIG usacCoreConfig;
  USAC_SBR_CONFIG  usacSbrConfig;

} USAC_SCE_CONFIG;

typedef struct {

  USAC_CORE_CONFIG   usacCoreConfig;
  USAC_SBR_CONFIG    usacSbrConfig;
  DESCR_ELE          stereoConfigIndex;
  USAC_MPS212_CONFIG usacMps212Config;
  UINT32             qceFlag;
  DESCR_ELE          lpdStereoIndex;

} USAC_CPE_CONFIG;

typedef struct {

  USAC_CORE_CONFIG usacCoreConfig;

} USAC_LFE_CONFIG;

typedef struct {

  USAC_ID_EXT_ELE  usacExtElementType;
  unsigned int     usacExtElementConfigLength;
  DESCR_ELE        usacExtElementDefaultLengthPresent;
  unsigned int     usacExtElementDefaultLength;
  DESCR_ELE        usacExtElementPayloadFrag;
  unsigned char    usacExtElementConfigPayload[6144/8];

} USAC_EXT_CONFIG;



typedef struct {

  USAC_SCE_CONFIG usacSceConfig;
  USAC_CPE_CONFIG usacCpeConfig;
  USAC_LFE_CONFIG usacLfeConfig;
  USAC_EXT_CONFIG usacExtConfig;

} USAC_ELEMENT_CONFIG;

typedef struct {

  unsigned int        numElements;
  unsigned int        numChannels;
  USAC_ELEMENT_TYPE   usacElementType[USAC_MAX_ELEMENTS];
  USAC_ELEMENT_CONFIG usacElementConfig[USAC_MAX_ELEMENTS];
  UINT32              usacElementLength[USAC_MAX_ELEMENTS];
  UINT32              usacElementLengthPresent;
  int                 usacChannelIndex[USAC_MAX_NUM_OUT_CHANNELS];
  int                 use_fd_extensions;
} USAC_DECODER_CONFIG;

#ifdef WD1_REFERENCE_LAYOUT_3D
typedef struct {

  DESCR_ELE isCICPspeakerIdx;
  DESCR_ELE CICPspeakerIdx;
  DESCR_ELE ElevationClass;
  DESCR_ELE ElevationAngleIdx; 
  DESCR_ELE ElevationDirection; 
  DESCR_ELE AzimuthAngleIdx;
  DESCR_ELE AzimuthDirection; 
  DESCR_ELE isLFE;

} USAC3DSPEAKERDESCRIPTION;

typedef struct  {

  DESCR_ELE angularPrecision;
  USAC3DSPEAKERDESCRIPTION speakerDescription[USAC_MAX_NUM_OUT_CHANNELS];

} USAC3DFLEXIBLESPEAKERCONFIG ;

typedef struct  {

	DESCR_ELE speakerLayoutType;
	DESCR_ELE CICPspeakerLayoutIdx;
  DESCR_ELE numSpeakers;
  
  DESCR_ELE CICPspeakerIdx[USAC_MAX_NUM_OUT_CHANNELS];
  
  USAC3DFLEXIBLESPEAKERCONFIG flexibleSpeakerConfig;

} SPEAKERCONFIG3D;

#endif /* WD1_REFERENCE_LAYOUT_3D */

#ifdef WD1_FRAMEWORK_CONFIG_3D
typedef struct {

  unsigned int           m_numSignalGroups;
  unsigned int           m_signalGroupType[USAC_MAX_AUDIO_GROUPS];
  unsigned int           m_numberOfSignals[USAC_MAX_AUDIO_GROUPS];

  unsigned int           numAudioChannels;
  unsigned int           numAudioObjects;
  unsigned int           numSAOCTransportChannels;
  unsigned int           numHOATransportChannels;

  SPEAKERCONFIG3D        saocDmxChannelLayout;

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  unsigned int           saocDmxLayoutPresent;
  SPEAKERCONFIG3D        audioChannelLayout[USAC_MAX_AUDIO_CHANNEL_LAYOUTS];
#endif

} SIGNALS_3D;

typedef struct {

  SIGNALS_3D      signals3D;

} FRAMEWORK_CONFIG_3D;

#ifdef SPECIFY_INPUT_CONTENTTYPE
typedef enum {

  CONTENT_3D_CHANNELS     = 0,
  CONTENT_3D_OBJECTS      = 1,
  CONTENT_3D_SAOC_OBJECTS = 2,
  CONTENT_3D_HOA          = 3
#if IAR
 ,CONTENT_3D_IAR          = 4
#endif
} INPUT_CONTENT_TYPE_3D;
#endif
#endif

typedef struct {
  DESCR_ELE            numCompatibleSets;
  DESCR_ELE            reserved;
  DESCR_ELE            CompatibleSetIndication[USAC_MAX_COMPATIBLE_PROFILE_LEVELS];
} COMPATIBLE_PROFILE_LEVEL_SET;

typedef struct {

  unsigned int                 numConfigExtensions;
  USAC_CONFIG_EXT_TYPE         usacConfigExtType[USAC_MAX_CONFIG_EXTENSIONS];
  unsigned int                 usacConfigExtLength[USAC_MAX_CONFIG_EXTENSIONS];
  COMPATIBLE_PROFILE_LEVEL_SET usacCompatibleProfileLevelSet;
} USAC_CONFIG_EXTENSION;

typedef struct {

  DESCR_ELE             mpegh3daProfileLevelIndication;
  DESCR_ELE             usacSamplingFrequencyIndex;
  DESCR_ELE             usacSamplingFrequency;
  DESCR_ELE             coreSbrFrameLengthIndex;
  DESCR_ELE             reservedCoreQmfDelayCompensation;
  DESCR_ELE             receiverDelayCompensation;

  DESCR_ELE             frameLength;

#ifdef WD1_REFERENCE_LAYOUT_3D
  SPEAKERCONFIG3D       referenceLayout;
#endif /* WD1_REFERENCE_LAYOUT_3D */

#ifndef WD1_FRAMEWORK_CONFIG_3D
  USAC_CHANNEL_CONFIG   usacChannelConfig;
#endif

#ifdef WD1_FRAMEWORK_CONFIG_3D
  FRAMEWORK_CONFIG_3D   frameworkConfig3d;
#endif

  USAC_DECODER_CONFIG   usacDecoderConfig;
  
  DESCR_ELE             usacConfigExtensionPresent;
  USAC_CONFIG_EXTENSION usacConfigExtension;

} USAC_CONFIG;



void UsacConfig_Init( USAC_CONFIG *usacConf, int WriteFlag );
int UsacConfig_ReadEscapedValue(HANDLE_BSBITSTREAM bitStream, unsigned int *pValue, unsigned int nBits1, unsigned int nBits2, unsigned int nBits3);

int UsacConfig_Advance(HANDLE_BSBITSTREAM bitStream, USAC_CONFIG *pUsacConfig, int WriteFlag);

USAC_SBR_RATIO_INDEX UsacConfig_GetSbrRatioIndex(unsigned int coreSbrFrameLengthIndex); 

USAC_OUT_FRAMELENGTH UsacConfig_GetOutputFrameLength(unsigned int coreSbrFrameLengthIndex);

int UsacConfig_GetMps212NumSlots(unsigned int coreSbrFrameLengthIndex);

USAC_ELEMENT_TYPE UsacConfig_GetUsacElementType(USAC_CONFIG *pUsacConfig, unsigned int elemIdx);

USAC_CORE_CONFIG * UsacConfig_GetUsacCoreConfig(USAC_CONFIG *pUsacConfig, unsigned int elemIdx);

USAC_SBR_CONFIG * UsacConfig_GetUsacSbrConfig(USAC_CONFIG *pUsacConfig, unsigned int elemIdx);

USAC_MPS212_CONFIG * UsacConfig_GetUsacMps212Config(USAC_CONFIG *pUsacConfig, unsigned int elemIdx);

int UsacConfig_GetStereoConfigIndex(USAC_CONFIG *pUsacConfig, unsigned int elemIdx);

int UsacConfig_GetNumElements(int numChannels);

int UsacConfig_GetCICPIndex(int numChannels);

USAC_ELEMENT_TYPE UsacConfig_GetElementType(int elemIdx, int numChan);

int UsacConfig_GetIsQceElement(int elemIdx, int numChannels, int allowQce);

void UsacConfig_GetChannelIdx(USAC_CONFIG * pUsacConfig, unsigned int const elemIdx, int * offsets);

#endif /* __INCLUDED_USAC_CONFIG_H */
