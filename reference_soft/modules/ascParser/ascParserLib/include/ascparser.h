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

#ifndef __ASCPARSER_H__
#define __ASCPARSER_H__

#include "cicp2geometry.h"
#include "ascparser_metadata.h"
#include "ascparser_usacconfig.h"

#define SUPPORT_HOA
#define WD1_REFERENCE_LAYOUT_3D

#define ASCPARSER_MAX_DMX_MATRICES_PER_ID           (8)
#define ASCPARSER_MAX_DMX_MAX_GROUPS_ASSIGNED      (16)
#define ASCPARSER_MAX_DMX_MATRIX_SIZE             (256) /* bytes */
#define ASCPARSER_MAX_DMX_MATRIX_ELEMENTS           (6) /* Max number of downmix matrices one can embed */

#define ASCPARSER_MAX_HOA_MATRIX_SIZE 512       /* bytes */
#define ASCPARSER_MAX_HOA_MATRIX_ELEMENTS 8     /* Max number of hoa matrices one can emded */


typedef enum {
  ASCPARSER_CONFIG_EXT_FILL             = 0,
  ASCPARSER_CONFIG_EXT_DOWNMIX_CONFIG   = 1,
  ASCPARSER_CONFIG_EXT_LOUDNESS_INFO    = 2,
  ASCPARSER_CONFIG_EXT_AUDIOSCENE_INFO  = 3,
  ASCPARSER_CONFIG_EXT_HOA_CONFIG       = 4,
  ASCPARSER_CONFIG_EXT_ICG              = 5,
  ASCPARSER_CONFIG_EXT_SIG_GROUP_INFO   = 6
} ASCPARSER_CONFIG_EXTENSION_TYPE;


typedef struct _ASCPARSER* HANDLE_ASCPARSER;

typedef struct {
  unsigned int DMX_ID;
  unsigned int downmixType;
  unsigned int CICPspeakerLayoutIdx;
  unsigned int downmixMatrixCount;
  unsigned int numAssignedGroupIDs[ASCPARSER_MAX_DMX_MATRICES_PER_ID];
  unsigned int signal_groupID[ASCPARSER_MAX_DMX_MATRICES_PER_ID][ASCPARSER_MAX_DMX_MAX_GROUPS_ASSIGNED];
  unsigned int DmxMatrixLenBits[ASCPARSER_MAX_DMX_MATRICES_PER_ID];
  unsigned char DownmixMatrix[ASCPARSER_MAX_DMX_MATRICES_PER_ID][ASCPARSER_MAX_DMX_MATRIX_SIZE];
} ASCPARSER_DOWNMIX_MATRIX;

typedef struct {

  unsigned int downmixConfigType;
  unsigned int passiveDownmixFlag;
  unsigned int phaseAlignStrength;
  unsigned int immersiveDownmixFlag;
  unsigned int downmixIdCount;
  ASCPARSER_DOWNMIX_MATRIX downmixMatrix[ASCPARSER_MAX_DMX_MATRIX_ELEMENTS];

} ASCPARSER_DOWNMIX_CONFIG;


typedef struct {
  AudioSceneInfo    asi;
  INT32             screenRelativeObjects[ASCPARSER_USAC_MAX_ELEMENTS]; 
} ASCPARSER_AUDIO_SCENE;

typedef struct {
  UINT32                       numSignalGroups;
  ASCPARSER_SIGNAL_GROUP_TYPE  signalGroupType[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  SpeakerConfig3d              signalGroupChannelLayout[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  UINT32                       numberOfSignals[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
} ASCPARSER_SIGNAL_GROUP_CONFIG;

typedef struct {
  UINT32      numSignalGroups;
  UINT32      groupPriority[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  UINT32      fixedPosition[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

} ASCPARSER_SIGNAL_GROUP_INFO;

typedef struct {
  UINT32      numCompatibleSets;
  UINT32      reserved;
  UINT32      CompatibleSetIndication[ASCPARSER_USAC_MAX_COMPATIBLE_SETS];

} ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET;

typedef struct {
  UINT32  hasDiffuseness;
  UINT32  hasCommonGroupDiffuseness;
  UINT32  hasExcludedSectors;
  UINT32  hasCommonGroupExcludedSectors;
  UINT32  useOnlyPredefinedSectors[ASCPARSER_USAC_MAX_ELEMENTS];
  UINT32  hasClosestSpeakerCondition;
  float   closestSpeakerThresholdAngle;
  UINT32  hasDivergence[ASCPARSER_USAC_MAX_ELEMENTS];
  float   divergenceAzimuthRange[ASCPARSER_USAC_MAX_ELEMENTS];

} ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG;

typedef struct {
  UINT32  has_reference_distance;
  UINT32  bs_reference_distance;
  UINT32  has_object_distance[ASCPARSER_USAC_MAX_ELEMENTS];
  UINT32  directHeadphone[ASCPARSER_USAC_MAX_ELEMENTS];
} ASCPARSER_PRODUCTION_METADATA_CONFIG;

typedef struct {
  unsigned char data[2048];
} ASCPARSER_LOUDNESS_INFO_SET;

typedef struct {
  unsigned int HOA_ID;
  unsigned int CICPspeakerLayoutIdx;
  unsigned int HoaMatrixLenBits;
  unsigned char HoaMatrix[ASCPARSER_MAX_HOA_MATRIX_SIZE];
} ASCPARSER_HOA_MATRIX;

typedef struct {
  unsigned int numHoaMatrices;
  ASCPARSER_HOA_MATRIX hoaMatrix[ASCPARSER_MAX_HOA_MATRIX_ELEMENTS];
} ASCPARSER_HOA_CONFIG;

typedef struct
{
  int  speakerLayoutID;
  int  CICPspeakerLayoutIdx;
  int  numLoudspeakers;
  CICP2GEOMETRY_CHANNEL_GEOMETRY geometryInfo[32];
} ASCPARSER_SPEAKER_CONFIG_3D;


typedef struct 
{
  int aot;
  int fs;
  int ext_fs;
  int chConfig;
  int channels;
  int objects;
  int earcon;
  int asc_isValid;
  int SAOCTransportChannels;

#ifdef SUPPORT_HOA
  int HOATransportChannels;
#endif

  int oam_mode;
  int oam_length;
  int oam_hasDynamicObjectPriority;
  int oam_hasUniformSpread;
  int saocHeaderLength;
  int usacSamplingFreqIndx;
  int usacSamplingFreq;
  int outputFrameLength;
  int sbrRatioIndex;
  int receiverDelayCompensation;
  int profileLevelIndication;
  int elementLengthPresent;
  int maxStereoConfigIndex;
  int has_reference_distance;
  int bs_reference_distance;
  int has_object_distance[ASCPARSER_USAC_MAX_AUDIO_GROUPS];
  int directHeadphone[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

#ifndef WD1_REFERENCE_LAYOUT_3D
  int channelConfigurationIndex;
#else
  ASCPARSER_SPEAKER_CONFIG_3D  referenceLayout;
  ASCPARSER_SPEAKER_CONFIG_3D  audioChannelLayout;
  int differsFromReferenceLayout;
#endif

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  int  saocDmxLayoutPresent;
  ASCPARSER_SPEAKER_CONFIG_3D saocDmxChannelLayout;
#endif

  int numSignalGroups;
  ASCPARSER_SIGNAL_GROUP_TYPE signalGroupType[ASCPARSER_USAC_MAX_AUDIO_GROUPS];

  int numConfigExtensions;
  ASCPARSER_CONFIG_EXTENSION_TYPE  configExtensionType[10];

  ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET compatibleProfileLevelSet;
} ASCPARSERINFO;


int  ASCPARSER_Init(HANDLE_ASCPARSER* phAscParser, const char* filename);
int  ASCPARSER_GetASC(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc);
int  ASCPARSER_GetMaxStereoConfigIndex(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc);
int  ASCPARSER_GetDownmixConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_DOWNMIX_CONFIG* downmixConfig, int* downmixConfigAvailable);
int  ASCPARSER_GetLoudnessInfoSet(HANDLE_ASCPARSER hAscParser, unsigned char* loudnessInfoSet, int* loudnessInfoAvailable, int* bitSize);
int  ASCPARSER_GetHoaConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_HOA_CONFIG* hoaConfig, int* hoaConfigAvailable);
void ASCPARSER_Close(HANDLE_ASCPARSER hAscParser);
int ASCPARSER_GetASI(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc, ASCPARSER_AUDIO_SCENE* audioScene, int* asi_valid);
int ASCPARSER_GetSignalGroupConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig);
int ASCPARSER_GetSignalGroupInfo(HANDLE_ASCPARSER hAscParser, ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, int *signalGroupInfoAvailable);
int ASCPARSER_GetEnhancedObjectConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig);
int ASCPARSER_GetProductionMetadataConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_PRODUCTION_METADATA_CONFIG* productionMetadataConfig);
int ASCPARSER_GetCompatibleProfileLevelSet(HANDLE_ASCPARSER hAscParser, ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet, int* compatibleProfileLevelSetAvailable);
int ASCPARSER_GetDecoderConfig(HANDLE_ASCPARSER hAscParser, ascparserUsacDecoderConfig *usacDecoderConfig);

#endif
