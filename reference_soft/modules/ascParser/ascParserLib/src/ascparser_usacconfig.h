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

#ifndef __ASCPARSER_USACCONFIG_H__
#define __ASCPARSER_USACCONFIG_H__

#include "ascparser_machine.h"
#include "ascparser_aots.h"
#include "ascparser_bitstream.h"
#include "ascparser_metadata.h"


#define WD1_FRAMEWORK_CONFIG_3D
#define WD1_ELEMENT_MAPPING_3D
#define WD1_ESCAPED_OBJECTS_MAPPING_3D
#define WD1_REFERENCE_LAYOUT_3D
#define CD_SIGNALS_3D_SIGNAL_GROUPS
#define SUPPORT_HOA

#define ASCPARSER_USAC_MAX_ELEMENTS         (255)
#define ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS (255)

#define ASCPARSER_USAC_MAX_SBR_ELEMENTS (16)
#define ASCPARSER_USAC_MAX_MPS_ELEMENTS (16)

#define ASCPARSER_USAC_MAX_CONFIG_EXTENSIONS 16

#define ASCPARSER_USAC_MAX_DMX_MATRICES_PER_ID           (8)
#define ASCPARSER_USAC_MAX_DMX_MAX_GROUPS_ASSIGNED      (16)
#define ASCPARSER_USAC_MAX_DMX_MATRIX_SIZE             (256)
#define ASCPARSER_USAC_MAX_DMX_MATRIX_ELEMENTS           (5)

#define ASCPARSER_USAC_MAX_LOUDNESS_INFO          64
#define ASCPARSER_USAC_MAX_LOUDNESS_INFO_ALBUM    64
#define ASCPARSER_USAC_MAX_LOUDNESS_INFO_MEASUREMENT  16

#define ASCPARSER_USAC_MAX_HOA_MATRIX_SIZE 512
#define ASCPARSER_USAC_MAX_HOA_MATRIX_ELEMENTS 5

#define ASCPARSER_USAC_MAX_AUDIO_GROUPS 32

#define ASCPARSER_USAC_MAX_COMPATIBLE_SETS 16

#define SUPPORT_SAOC_DMX_LAYOUT

struct ascparserBitStream;

/* Downmix Matrix config extension */

#ifdef PARSE_LOUDNESS
typedef struct {
  UINT32 drcSetId;
  UINT32 downmixId;
  UINT32 samplePeakLevelPresent;
  UINT32 samplePeakLevel;
  UINT32 truePeakLevelPresent;
  UINT32 truePeakLevel;
  UINT32 measurementSystem;
  UINT32 reliability;
  UINT32 measurementCount;

  UINT32 MMethodDefinition[ASCPARSER_USAC_MAX_LOUDNESS_INFO_MEASUREMENT];
  UINT32 MMethodValue[ASCPARSER_USAC_MAX_LOUDNESS_INFO_MEASUREMENT];
  UINT32 MMeasurementSystem[ASCPARSER_USAC_MAX_LOUDNESS_INFO_MEASUREMENT];
  UINT32 MReliability[ASCPARSER_USAC_MAX_LOUDNESS_INFO_MEASUREMENT];

} ASCPARSER_USAC_LOUDNESS_INFO;


typedef struct {
  UINT32 loudnessInfoCount;
  UINT32 loudnessInfoType;
  UINT32 mae_groupID;
  UINT32 mae_groupCollectionID;

  UINT32 loudnessInfoAlbumPresent;
  UINT32 loudnessInfoAlbumCount;
  ASCPARSER_USAC_LOUDNESS_INFO loudnessInfo[ASCPARSER_USAC_MAX_LOUDNESS_INFO];
  ASCPARSER_USAC_LOUDNESS_INFO loudnessInfoAlbum[ASCPARSER_USAC_MAX_LOUDNESS_INFO_ALBUM];
} ASCPARSER_USAC_LOUDNESS_INFO_SET;

#endif



typedef struct {
  UINT32 DMX_ID;
  UINT32 downmixType;
  UINT32 CICPspeakerLayoutIdx;
  UINT32 downmixMatrixCount;
  UINT32 numAssignedGroupIDs[ASCPARSER_USAC_MAX_DMX_MATRICES_PER_ID];
  UINT32 signal_groupID[ASCPARSER_USAC_MAX_DMX_MATRICES_PER_ID][ASCPARSER_USAC_MAX_DMX_MAX_GROUPS_ASSIGNED];

  UINT32 DmxMatrixLenBits[ASCPARSER_USAC_MAX_DMX_MATRICES_PER_ID];
  
  unsigned char DownmixMatrix[ASCPARSER_USAC_MAX_DMX_MATRICES_PER_ID][ASCPARSER_USAC_MAX_DMX_MATRIX_SIZE];
} ASCPARSER_USAC_DOWNMIX_MATRIX;

typedef struct {
  UINT32 downmixConfigType;
  UINT32 passiveDownmixFlag;
  UINT32 phaseAlignStrength;
  UINT32 immersiveDownmixFlag;
  UINT32 downmixIdCount;
  ASCPARSER_USAC_DOWNMIX_MATRIX downmixMatrix[ASCPARSER_USAC_MAX_DMX_MATRIX_ELEMENTS];

} ASCPARSER_USAC_DOWNMIX_CONFIG;

/* END: Downmix Matrix config extension */

/* HOA Rendering Matrix config extension */

typedef struct {
  UINT32 HOA_ID;
  UINT32 CICPspeakerLayoutIdx;
  UINT32 HoaMatrixLenBits;
  unsigned char HoaMatrix[ASCPARSER_USAC_MAX_HOA_MATRIX_SIZE];
} ASCPARSER_USAC_HOA_MATRIX;

typedef struct {
  UINT32 numHoaMatrices;
  ASCPARSER_USAC_HOA_MATRIX hoaMatrix[ASCPARSER_USAC_MAX_HOA_MATRIX_ELEMENTS];

} ASCPARSER_USAC_HOA_CONFIG;

/* END: Hoa Rendering Matrix config extension */

/* Signal Group Info Extension */

typedef struct {
  UINT32 numSignalGroups;
  UINT32 groupPriority[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  UINT32 fixedPosition[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

}  ASCPARSER_USAC_SIG_GROUP_INFO ;

/* END: Signal Group Info Extension */

/* Compatible Profile Level Set Extension */

typedef struct {
  UINT32 numCompatibleSets;
  UINT32 reserved;
  UINT32 CompatibleSetIndication[ASCPARSER_USAC_MAX_COMPATIBLE_SETS];

}  ASCPARSER_USAC_COMPATIBLE_PROFILE_LEVEL_SET ;

/* END: Compatible Profile Level Set Extension */


typedef enum {

  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_NA   = -1, /* n/a                                */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_L    =  0, /* Left Front                          */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_R    =  1, /* Right Front                         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_C    =  2, /* Center Front                        */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LFE  =  3, /* Low Frequency Enhancement           */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LS   =  4, /* Left Surround                       */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RS   =  5, /* Right Surround                      */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LC   =  6, /* Left Front Center                   */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RC   =  7, /* Right Front Center                  */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LSR  =  8, /* Rear Surround Left                  */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RSR  =  9, /* Rear Surround Right                 */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_CS   = 10, /* Rear Center                         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LSD  = 11, /* Left Surround Direct                */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RSD  = 12, /* Right Surround Direct               */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LSS  = 13, /* Left Side Surround                  */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RSS  = 14, /* Right Side Surround                 */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LW   = 15, /* Left Wide Front                     */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RW   = 16, /* Right Wide Front                    */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LV   = 17, /* Left Front Vertical Height          */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RV   = 18, /* Right Front Vertical Height         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_CV   = 19, /* Center Front Vertical Height        */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LVR  = 20, /* Left Surround Vertical Height Rear  */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RVR  = 21, /* Right Surround Vertical Height Rear */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_CVR  = 22, /* Center Vertical Height Rear         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LVSS = 23, /* Left Vertical Height Side Surround  */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RVSS = 24, /* Right Vertical Height Side Surround */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_TS   = 25, /* Top Center Surround                 */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LFE2 = 26, /* Low Frequency Enhancement 2         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LB   = 27, /* Left Front Vertical Bottom          */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RB   = 28, /* Right Front Vertical Bottom         */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_CB   = 29, /* Center Front Vertical Bottom        */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_LVS  = 30, /* Left Vertical Height Surround       */
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS_RVS  = 31  /* Right Vertical Height Surround      */

} ASCPARSER_USAC_OUTPUT_CHANNEL_POS; 

typedef struct {

  UINT32                  m_numOutChannels;
  INT32                   m_outputChannelsIndex[ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS];
  ASCPARSER_USAC_OUTPUT_CHANNEL_POS m_outputChannelPos[ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS];

} ascparserUsacChannelConfig;

typedef enum {

  ASCPARSER_USAC_ELEMENT_TYPE_INVALID = -1, /* use for initialization only */
  ASCPARSER_USAC_ELEMENT_TYPE_SCE = 0,
  ASCPARSER_USAC_ELEMENT_TYPE_CPE = 1,
  ASCPARSER_USAC_ELEMENT_TYPE_LFE = 2,
  ASCPARSER_USAC_ELEMENT_TYPE_EXT = 3

} ASCPARSER_USAC_ELEMENT_TYPE;

typedef enum {

  ASCPARSER_USAC_ID_EXT_ELE_FILL                  = 0,
  ASCPARSER_USAC_ID_EXT_ELE_MPEGS                 = 1,
  ASCPARSER_USAC_ID_EXT_ELE_SAOC                  = 2,
  ASCPARSER_USAC_ID_EXT_ELE_AUDIOPREROLL          = 3,
  ASCPARSER_USAC_ID_EXT_ELE_UNI_DRC               = 4,
  ASCPARSER_USAC_ID_EXT_ELE_OBJ                   = 5,
  ASCPARSER_USAC_ID_EXT_ELE_SAOC_3D               = 6,
  ASCPARSER_USAC_ID_EXT_ELE_HOA                   = 7,
  ASCPARSER_USAC_ID_EXT_ELE_FMT_CNVRTR            = 8,
  ASCPARSER_USAC_ID_EXT_ELE_MCT                   = 9,
  ASCPARSER_USAC_ID_EXT_ELE_TCC                   = 10,
  ASCPARSER_USAC_ID_EXT_ELE_HOA_ENH_LAYER         = 11,
  ASCPARSER_USAC_ID_EXT_ELE_HREP                  = 12,
  ASCPARSER_USAC_ID_EXT_ELE_ENHANCED_OBJ_METADATA = 13,
  ASCPARSER_USAC_ID_EXT_ELE_PRODUCTION_METADATA   = 14

} ASCPARSER_USAC_EXT_ELEMENT_TYPE;


typedef enum {

  ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS = 0,
  ASCPARSER_SIGNAL_GROUP_TYPE_OBJECTS  = 1,
  ASCPARSER_SIGNAL_GROUP_TYPE_SAOC     = 2,
  ASCPARSER_SIGNAL_GROUP_TYPE_HOA      = 3,
  ASCPARSER_SIGNAL_GROUP_TYPE_RSRVD1   = 4,
  ASCPARSER_SIGNAL_GROUP_TYPE_RSRVD2   = 5,
  ASCPARSER_SIGNAL_GROUP_TYPE_RSRVD3   = 6,
  ASCPARSER_SIGNAL_GROUP_TYPE_RSRVD4   = 7
  
} ASCPARSER_SIGNAL_GROUP_TYPE;

typedef struct {

  UINT32 m_tw_mdct;
  UINT32 m_fullbandLPD;
  UINT32 m_noiseFilling;
  UINT32 m_enhancedNoiseFilling;
  UINT32 m_igfUseEnf;
  UINT32 m_igfUseWhitening;
  UINT32 m_igfAfterTnsSynth;
  UINT32 m_igfStartIndex;
  UINT32 m_igfUseHighRes;
  UINT32 m_igfStopIndex;

} ascparserUsacCoreConfig, *ascparserUsacCoreConfigPtr;

typedef struct {

  UINT32 m_start_freq;
  UINT32 m_stop_freq;
  UINT32 m_header_extra1;
  UINT32 m_header_extra2;
  UINT32 m_freq_scale;
  UINT32 m_alter_scale;
  UINT32 m_noise_bands;
  UINT32 m_limiter_bands;
  UINT32 m_limiter_gains;
  UINT32 m_interpol_freq;
  UINT32 m_smoothing_mode;

} ascparserUsacSbrHeader, *ascparserUsacSbrHeaderPtr;

typedef struct {
  
  UINT32 m_harmonicSBR;
  UINT32 m_bs_interTes;
  UINT32 m_bs_pvc;
  ascparserUsacSbrHeader m_sbrDfltHeader;

} ascparserUsacSbrConfig, *ascparserUsacSbrConfigPtr;

typedef struct {

  ascparserUsacCoreConfig m_usacCoreConfig;
  ascparserUsacSbrConfig  m_usacSbrConfig;

} ascparserUsacSceConfig, *ascparserUsacSceConfigPtr;

typedef struct {

  UINT32 m_bsFreqRes;
  UINT32 m_bsFixedGainDMX;
  UINT32 m_bsTempShapeConfig;
  UINT32 m_bsDecorrConfig;
  UINT32 m_bsHighRateMode;
  UINT32 m_bsPhaseCoding;
  UINT32 m_bsOttBandsPhasePresent;
  UINT32 m_bsOttBandsPhase;
  UINT32 m_bsResidualBands;
  UINT32 m_bsPseudoLr;
  UINT32 m_bsEnvQuantMode;

} ascparserUsacMps212Config, *ascparserUsacMps212ConfigPtr;

typedef struct {

  ascparserUsacCoreConfig   m_usacCoreConfig;
  ascparserUsacSbrConfig    m_usacSbrConfig;
  UINT32                    m_stereoConfigIndex;
  UINT32                    m_qceIndex;
  UINT32                    m_lpdStereoIndex;
  ascparserUsacMps212Config m_usacMps212Config;

} ascparserUsacCpeConfig, *ascparserUsacCpeConfigPtr;

typedef struct {

  ascparserUsacCoreConfig m_usacCoreConfig;

} ascparserUsacLfeConfig, *ascparserUsacLfeConfigPtr;

typedef enum {

  ASCPARSER_USAC_ID_CONFIG_EXT_FILL               = 0,
  ASCPARSER_USAC_ID_CONFIG_EXT_DOWNMIX_CONFIG     = 1,
  ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET  = 2,
  ASCPARSER_USAC_ID_CONFIG_EXT_AUDIOSCENE_INFO    = 3,
  ASCPARSER_USAC_ID_CONFIG_EXT_HOA_CONFIG         = 4,
  ASCPARSER_USAC_ID_CONFIG_EXT_ICG                = 5,
  ASCPARSER_USAC_ID_CONFIG_EXT_SIG_GROUP_INFO     = 6,
  ASCPARSER_USAC_ID_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET = 7

} ASCPARSER_USAC_CONFIG_EXTENSION_TYPE;

typedef struct {
  UINT32                     m_usacConfigExtNumConfExt;
  ASCPARSER_USAC_CONFIG_EXTENSION_TYPE m_usacConfigExtType[ASCPARSER_USAC_MAX_CONFIG_EXTENSIONS];
  UINT32                     m_usacConfigExtLength[ASCPARSER_USAC_MAX_CONFIG_EXTENSIONS];
  unsigned char              m_usacConfigExtPayload[ASCPARSER_USAC_MAX_CONFIG_EXTENSIONS][2048];

  AudioSceneInfo             m_audioSceneInfo;
} ascparserUsacConfigExtension, *ascparserUsacConfigExtensionPtr;

typedef struct {
  ASCPARSER_USAC_EXT_ELEMENT_TYPE     m_usacExtElementType;
  UINT32                    m_usacExtElementConfigLength;
  UINT32                    m_usacExtElementDefaultLengthPresent;
  UINT32                    m_usacExtElementDefaultLength;
  UINT32                    m_usacExtElementPayloadFrag;
  unsigned char             m_usacExtElementConfigPayload[6144/8];

} ascparserUsacExtConfig, *ascparserUsacExtConfigPtr;

typedef union {

  ascparserUsacSceConfig m_usacSceConfig;
  ascparserUsacCpeConfig m_usacCpeConfig;
  ascparserUsacLfeConfig m_usacLfeConfig;
  ascparserUsacExtConfig m_usacExtConfig;

} ascparserUsacElementConfig, *ascparserUsacElementConfigPtr;


#ifdef WD1_REFERENCE_LAYOUT_3D

typedef struct {

  UINT32 isCICPspeakerIdx;
  UINT32 CICPspeakerIdx;
  UINT32 ElevationClass;
  UINT32 ElevationAngleIdx; 
  UINT32 ElevationDirection; 
  UINT32 AzimuthAngleIdx;
  UINT32 AzimuthDirection; 
  UINT32 isLFE;

} Usac3dSpeakerDescription;

typedef struct {

  UINT32 angularPrecision;
  Usac3dSpeakerDescription speakerDescription[ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS];

} Usac3dFlexibleSpeakerConfig ;

typedef struct {
  UINT32 speakerLayoutID;
  UINT32 CICPspeakerLayoutIdx;
  UINT32 numSpeakers;
  UINT32 CICPspeakerIdx[ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS];
  Usac3dFlexibleSpeakerConfig flexibleSpeakerConfig;
} SpeakerConfig3d;

#endif /* WD1_REFERENCE_LAYOUT_3D */

#ifdef WD1_FRAMEWORK_CONFIG_3D
typedef struct {

  UINT32                       m_numSignalGroups;
  ASCPARSER_SIGNAL_GROUP_TYPE  m_signalGroupType[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  UINT32                       m_numberOfSignals[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

  UINT32           m_numAudioChannels;
  UINT32           m_numAudioObjects;
  UINT32           m_numSAOCTransportChannels;

#ifdef SUPPORT_HOA
  UINT32           m_numHOATransportChannels;
#endif

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  UINT32           m_saocDmxLayoutPresent; 
  SpeakerConfig3d  m_saocDmxChannelLayout;
#endif

  UINT32           m_differsFromReferenceLayout[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  SpeakerConfig3d  m_audioChannelLayout[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

} ascparserUsacSignals, *ascparserUsacSignalsPtr;

typedef struct {

  ascparserUsacSignals      m_signals3D;       

} ascparserUsacFramework, *ascparserUsacFrameworkPtr;
#endif

typedef struct {

  UINT32                        m_numElements;
  UINT32                        m_elementLengthPresent;

  ASCPARSER_USAC_ELEMENT_TYPE   m_usacElementType[ASCPARSER_USAC_MAX_ELEMENTS];
  ascparserUsacElementConfig    m_usacElementConfig[ASCPARSER_USAC_MAX_ELEMENTS];

} ascparserUsacDecoderConfig, *ascparserUsacDecoderConfigPtr;

typedef struct ascparserUsacConfig {

  UINT32 m_mpegh3daProfileLevelIndication;

  UINT32 m_usacSamplingFrequencyIndex;
  UINT32 m_usacSamplingFrequency;

  UINT32 m_outputFrameLength;
  UINT32 m_sbrRatioIndex;

  UINT32 m_coreQmfDelayCompensation;
  UINT32 m_receiverDelayCompensation;

#ifdef WD1_REFERENCE_LAYOUT_3D
  SpeakerConfig3d m_referenceLayout;
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
  UINT32 m_differsFromReferenceLayout;
  SpeakerConfig3d m_audioChannelLayout;
#endif
#else /* WD1_REFERENCE_LAYOUT_3D */
  UINT32 m_channelConfigurationIndex;
#endif /* WD1_REFERENCE_LAYOUT_3D */

  ascparserUsacChannelConfig m_usacChannelConfig;


  ascparserUsacDecoderConfig m_usacDecoderConfig;

#ifdef WD1_FRAMEWORK_CONFIG_3D
  ascparserUsacFramework m_usacFramework;
#endif
  
  ascparserUsacConfigExtension m_usacConfigExtension;
  UINT32 m_usacDecMode;

  AudioSceneInfo m_audioSceneInfo; 

} ascparserUsacConfig, *ascparserUsacConfigPtr;

int ascparser_AzimuthAngleIdxToDegrees ( int idx, int direction, int precision );
int ascparser_ElevationAngleIdxToDegrees ( int idx, int direction, int precision );

void ascparserUsacConfig_Parse(ascparserUsacConfigPtr self, struct ascparserBitStream *bs);
void ascparserUsacConfig_Copy(ascparserUsacConfigPtr self, const ascparserUsacConfigPtr gasc);
UINT32 ascparserUsacConfig_Compare(const ascparserUsacConfigPtr self, const ascparserUsacConfigPtr gasc, const ASCPARSER_AUDIO_OBJECT_TYPE aot);
INT32 ascparserUsacConfig_Print(const ascparserUsacConfigPtr gasc, char string[], INT32 channelConfiguration, const ASCPARSER_AUDIO_OBJECT_TYPE aot);


UINT32 ascparserUsacConfig_GetSaocHeaderLength(const ascparserUsacConfig* self);
UINT32 ascparserUsacConfig_GetObjectMetadataConfig(const ascparserUsacConfig* self, int* mode, int* frameLength, int* hasScreenRelativeObjects, int* isScreenRelativeObject, int* hasDynamicObjectPriority, int* hasUniformSpread);
UINT32 ascparserUsacConfig_GetProductionMetadataConfig(const ascparserUsacConfig* self, int* has_reference_distance, int* bs_reference_distance, int* has_object_distance, int* directHeadphone, int* numObjectGroups, int* numChannelGroups);
UINT32 ascparserUsacConfig_GetEnhancedObjectMetadataConfig(ascparserUsacConfig* self, int *hasDiffuseness, int *hasCommonGroupDiffuseness, int *hasExcludedSectors, int *hasCommonGroupExcludedSectors, int *useOnlyPredefinedSectors, int *hasClosestSpeakerCondition, float *closestSpeakerThresholdAngle, int *hasDivergence, float *divergenceAzimuthRange);

ASCPARSER_USAC_ELEMENT_TYPE ascparserUsacConfig_GetUsacElementType(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

const ascparserUsacCoreConfig * ascparserUsacConfig_GetUsacCoreConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

const ascparserUsacExtConfig * ascparserUsacConfig_GetUsacExtConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

const ascparserUsacSbrConfig * ascparserUsacConfig_GetUsacSbrConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

const ascparserUsacMps212Config * ascparserUsacConfig_GetUsacMps212Config(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

int ascparserUsacConfig_GetStereoConfigIndex(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);
int ascparserUsacConfig_GetQCEIndex(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx);

UINT32 ascparserUsacConfig_ParseEscapedValue(ascparserBitStreamPtr bs, UINT32 nBits1, UINT32 nBits2, UINT32 nBits3);

#endif
