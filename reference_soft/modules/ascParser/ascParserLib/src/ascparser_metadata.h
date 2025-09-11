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

#ifndef __ASCPARSER_METADATA_H__
#define __ASCPARSER_METADATA_H__

#include "ascparser_bitstream.h"
#include <math.h>

#define MAE_MAX_NUM_GROUPS            32
#define MAE_MAX_NUM_OBJECTS           32
#define MAE_MAX_NUM_ELEMENTS          64
#define MAE_MAX_DESCR_LENGTH          256
#define MAE_MAX_NUM_DESCR_LANGUAGES   16
#define MAE_MAX_NUM_EXCLUDED_SECTORS  15

#define MAE_MAX_NUM_GROUP_PRESETS           32
#define MAE_MAX_NUM_GROUP_PRESET_CONDITIONS 17
#define MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS 32
#define MAE_MAX_NUM_SPEAKERS                32

#define MAE_MAX_NUM_TARGET_LOUDNESS_CONDITIONS 8

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

enum ID_MAE_DATA {
  ID_MAE_GROUP_DESCRIPTION	      =  0,
  ID_MAE_SWITCHGROUP_DESCRIPTION  =  1,
  ID_MAE_GROUP_CONTENT	          =  2,
  ID_MAE_GROUP_COMPOSITE          =  3,
  ID_MAE_SCREEN_SIZE              =  4,
  ID_MAE_GROUPPRESET_DESCRIPTION  =  5,
  ID_MAE_DRC_UI_INFO              =  6,
  ID_MAE_SCREEN_SIZE_EXTENSION    =  7,
  ID_MAE_GROUPPRESET_EXTENSION    =  8,
  ID_MAE_LOUDNESS_COMPENSATION    =  9
};

typedef struct {
  int   presetID;
  int   presetKind;
  int   groupPresetDisableGain[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetDisablePosition[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetGainFlag[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetGain[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetPositionFlag[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetAzOffset[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetElOffset[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   groupPresetDist[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   presetNumConditions;
  int   presetGroupID[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   presetConditionOnOff[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_groupPresetGain[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_groupPresetAzOffset[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_groupPresetElOffset[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_groupPresetDist[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
} GroupPresetDefinition;

typedef struct {
  int   hasSwitchGroupConditions;
  int   isSwitchGroupCondition[MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   numDownmixIdPresetExtensions;
  int   downmixId[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS];
  int   downmixIdPresetNumConditions[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS];
  int   downmixIdPresetIsSwitchGroupCondition[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetReferenceID[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS]; /* either groupID or switchGroupID */
  int   downmixIdPresetConditionOnOff[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetDisableGain[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetDisablePosition[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetGainFlag[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetGain[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetPositionFlag[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetAz[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetEl[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  int   downmixIdPresetDist[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_downmixIdPresetGain[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_downmixIdPresetAz[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_downmixIdPresetEl[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
  float f_downmixIdPresetDist[MAE_MAX_NUM_DOWNMIXID_GROUP_PRESETS][MAE_MAX_NUM_GROUP_PRESET_CONDITIONS];
} GroupPresetDefinitionExtensionData;

typedef struct {
  int	  numContentDataBlocks;
  int   contentDataGroupID[MAE_MAX_NUM_GROUPS];
  int   contentKind[MAE_MAX_NUM_GROUPS];
  int   hasContentLanguage[MAE_MAX_NUM_GROUPS];
  int   contentLanguage[MAE_MAX_NUM_GROUPS];
} ContentData;

typedef struct {
  int	 numDescriptionBlocks;
  int  descriptionGroupID[MAE_MAX_NUM_GROUPS];
  int  numDescLanguages[MAE_MAX_NUM_GROUPS];
  int  descriptionLanguage[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_DESCR_LANGUAGES];
  int  descriptionDataLength[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_DESCR_LANGUAGES];
  char descriptionData[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_DESCR_LANGUAGES][MAE_MAX_DESCR_LENGTH];
} DescriptionData;

typedef struct {
  int	 numCompositePairs;
  int  compositeElementID0[MAE_MAX_NUM_GROUPS];
  int  compositeElementID1[MAE_MAX_NUM_GROUPS];
} CompositeObjectPairData;

typedef struct {
  int hasNonStandardScreenSize;
  int elevation_top;
  int elevation_bottom;
  int azimuth;
  float f_elevation_top;
  float f_elevation_bottom;
  float f_azimuth_left;
  float f_azimuth_right;
  float f_azimuth_left_temp;
  float f_azimuth_right_temp;
  float offset;
} ProductionScreenData;

typedef struct {
  int   overwriteDefaultProductionScreenAzimuth;
  int   defaultScreenSizeLeftAz;
  int   defaultScreenSizeRightAz;
  float f_defaultScreenSizeLeftAz;
  float f_defaultScreenSizeRightAz;
  int   numPresetProductionScreens;
  int   productionScreenPresetID[MAE_MAX_NUM_GROUP_PRESETS];
  int   hasNonStandardScreenSize[MAE_MAX_NUM_GROUP_PRESETS];
  int   isCenteredInAzimuth[MAE_MAX_NUM_GROUP_PRESETS];
  int   azimuth[MAE_MAX_NUM_GROUP_PRESETS];
  int   azimuth_left[MAE_MAX_NUM_GROUP_PRESETS];
  int   azimuth_right[MAE_MAX_NUM_GROUP_PRESETS];
  float f_azimuth[MAE_MAX_NUM_GROUP_PRESETS];
  float f_azimuth_left[MAE_MAX_NUM_GROUP_PRESETS];
  float f_azimuth_right[MAE_MAX_NUM_GROUP_PRESETS];
  int   elevation_top[MAE_MAX_NUM_GROUP_PRESETS];
  int   elevation_bottom[MAE_MAX_NUM_GROUP_PRESETS];
  float f_elevation_top[MAE_MAX_NUM_GROUP_PRESETS];
  float f_elevation_bottom[MAE_MAX_NUM_GROUP_PRESETS];
} ProductionScreenExtensionData;

typedef struct {
  int     groupID;
  int     allowOnOff;
  int     defaultOnOff;
  int     allowPositionInteractivity;
  int     interactivityMinAzOffset;
  int     interactivityMaxAzOffset;
  int     interactivityMinElOffset;
  int     interactivityMaxElOffset;
  int     interactivityMinDistFactor;
  int     interactivityMaxDistFactor;
  float   f_interactivityMinAzOffset;
  float   f_interactivityMaxAzOffset;
  float   f_interactivityMinElOffset;
  float   f_interactivityMaxElOffset;
  float   f_interactivityMinDistFactor;
  float   f_interactivityMaxDistFactor;
  int     allowGainInteractivity;
  int     interactivityMinGain;
  int     interactivityMaxGain;
  float   f_interactivityMinGain;
  float   f_interactivityMaxGain;
  int     groupPriority;
  int     closestSpeakerPlayout;
  int     groupNumMembers;
  int     hasConjunctMembers;
  int     startID;
  int     metaDataElementID[MAE_MAX_NUM_ELEMENTS];
} GroupDefinition;

typedef struct {
  int  switchGroupID;
  int  allowOnOff;
  int  defaultOnOff;
  int  switchGroupNumMembers;
  int  switchGroupMemberID[MAE_MAX_NUM_GROUPS];
  int  switchGroupDefaultGroupID;
} SwitchGroupDefinition;

typedef struct {
  int   loudnessCompensationDataPresent;
  int   groupLoudnessPresent;
  float groupLoudness[MAE_MAX_NUM_GROUPS];
  int   defaultParamsPresent;
  int   defaultIncludeGroup[MAE_MAX_NUM_GROUPS];
  int   defaultMinMaxGainPresent;
  float defaultMinGain;
  float defaultMaxGain;
  int   presetParamsPresent[MAE_MAX_NUM_GROUP_PRESETS];
  int   presetIncludeGroup[MAE_MAX_NUM_GROUP_PRESETS][MAE_MAX_NUM_GROUPS];
  int   presetMinMaxGainPresent[MAE_MAX_NUM_GROUP_PRESETS];
  float presetMinGain[MAE_MAX_NUM_GROUP_PRESETS];
  float presetMaxGain[MAE_MAX_NUM_GROUP_PRESETS];
} LoudnessCompensationData;

typedef struct {
    int   version;
    int   numTargetLoudnessConditions;
    float targetLoudnessValueUpper[MAE_MAX_NUM_TARGET_LOUDNESS_CONDITIONS];
    float targetLoudnessValueLower[MAE_MAX_NUM_TARGET_LOUDNESS_CONDITIONS];
    int   drcSetEffectAvailable[MAE_MAX_NUM_TARGET_LOUDNESS_CONDITIONS];
} DrcUserInterfaceInfoData;

typedef struct {
  int                                 isMainStream;
  int                                 audioSceneInfoIDPresent;
  int                                 audioSceneInfoID;

  int                                 numGroups;
  GroupDefinition                     groups[MAE_MAX_NUM_GROUPS];

  int                                 numSwitchGroups;
  SwitchGroupDefinition               switchGroups[MAE_MAX_NUM_GROUPS];

  int                                 numGroupPresets;
  GroupPresetDefinition               groupPresets[MAE_MAX_NUM_GROUP_PRESETS];
  GroupPresetDefinitionExtensionData  groupPresetExtensions[MAE_MAX_NUM_GROUP_PRESETS];

  int                                 bsMetaDataElementIDoffset;
  int                                 mae_metaDataElementIDmaxAvail;
  
  /* mae_data */
  int                                 numDataSets;
  ContentData                         content;
  DescriptionData                     groupDescription;
  DescriptionData                     switchGroupDescription;
  DescriptionData                     groupPresetDescription;
  CompositeObjectPairData             compositeObjectPair;
  ProductionScreenData                productionScreen;
  ProductionScreenExtensionData       productionScreenExtensions;
  LoudnessCompensationData            loudnessCompensation;
  DrcUserInterfaceInfoData            drcUserInterfaceInfo;

} AudioSceneInfo;

int Read_AudioSceneInfo ( ascparserBitStreamPtr hbitbuf, AudioSceneInfo* asi );

#endif
