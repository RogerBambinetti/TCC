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

Copyright (c) ISO/IEC 2016.

***********************************************************************************/

#include "ascparser.h"
#include "profileLevelHandling.h"
#include "channelPath.h"


#define NUMBER_OF_PROFILES                (5)
#define NUMBER_OF_ALLOWED_SAMPLING_RATES  (12)
#define MAX_ALLOWED_CORE_OAM_RATIO        (4)
#define MAX_STRING_LENGTH                 (128)

extern ASCPARSERINFO  asc;
extern ASCPARSER_AUDIO_SCENE asi;
extern CICP_FORMAT    outputCicp;
extern char           fileOutGeo[FILENAME_MAX];
extern int            hasASI;

int nrCoreChannels = 0;
int nrReferenceLayoutChannels = 0;

typedef enum rateFamilyType
{
  MPEG3DA_44100_TYPE,
  MPEG3DA_48000_TYPE
} RATE_FAMILY_TYPE;

static int allowedSamplingRatesTable[NUMBER_OF_PROFILES][NUMBER_OF_ALLOWED_SAMPLING_RATES] =
{
  {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000,    -1,    -1,    -1,    -1},
  {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000,    -1,    -1,    -1,    -1},
  {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000,    -1,    -1,    -1,    -1},
  {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000,    -1,    -1,    -1,    -1},
  {14700, 16000, 22050, 24000, 29400, 32000, 44100, 48000, 58800, 64000, 88200, 96000},
};

static int samplingRateFamilyTable[2][NUMBER_OF_ALLOWED_SAMPLING_RATES/2] =
{
  {14700, 22050, 29400, 44100, 58800, 88200},   /* 48000 Hz family */
  {16000, 24000, 32000, 48000, 64000, 96000}    /* 44100 Hz family */
};

static int samplingRateFamily(const int samplingFrequency)
{
  int i = 0;
  int j = 0;

  for ( j = 0; j < 2; ++j) {
    for ( i = 0; i < NUMBER_OF_ALLOWED_SAMPLING_RATES/2; ++i ) {
      if ( samplingFrequency == samplingRateFamilyTable[j][i] ){
        return j;
      }
    }
  }
  return -1;
}

static int samplingRateAllowed(const LEVEL level, const int samplingFrequency)
{
  int i          =  0;
  int levelIndex = -1;

  switch (level) {
    case L1:
      levelIndex = 0;
      break;
    case L2:
      levelIndex = 1;
      break;
    case L3:
      levelIndex = 2;
      break;
    case L4:
      levelIndex = 3;
      break;
    case L5:
      levelIndex = 4;
      break;
    default:
      levelIndex = -1;
  }

  if ( levelIndex == -1 ) {
    fprintf( stderr, "\nLevel not defined!\n\n");
    return 0;
  }

  for ( i = 0; i < NUMBER_OF_ALLOWED_SAMPLING_RATES; ++i ) {
    if ( samplingFrequency == allowedSamplingRatesTable[levelIndex][i] ){
      return 1;
    }
  }
  return 0;
}

static int checkObjectOnlyRestrictions(ascparserUsacDecoderConfig *usacDecoderConfig)
{
  /* ISO/IEC 23008-3, 4.8.2.5.1  Complexity restrictions for Level 3 with more than 16 decoder processed core channels */

  int restrictionsAreMet = 1;

  if (restrictionsAreMet) {
    int i;
    for (i = 0; i < asc.numSignalGroups; i++) {
      if (asc.signalGroupType[i] != ASCPARSER_SIGNAL_GROUP_TYPE_OBJECTS) {
        restrictionsAreMet = 0;
        break;
      }
    }
  }

  if (restrictionsAreMet) {
    UINT32 i;

    for (i = 0; i < usacDecoderConfig->m_numElements; i++) {
      if (usacDecoderConfig->m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_SCE) {
        ascparserUsacCoreConfig usacCoreConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacSceConfig.m_usacCoreConfig;

        if (usacCoreConfig.m_noiseFilling) {
          restrictionsAreMet = 0;
          break;
        }
        if (usacCoreConfig.m_enhancedNoiseFilling) {
          restrictionsAreMet = 0;
          break;
        }

      } else if (usacDecoderConfig->m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_EXT) {
        ascparserUsacExtConfig usacExtConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacExtConfig;

        if ((usacExtConfig.m_usacExtElementType == ASCPARSER_USAC_ID_EXT_ELE_MCT)) {
          restrictionsAreMet = 0;
          break;
        }

      } else {
        /* Complexity restrictions for Level 3 with more than 16 decoder processed core channels:
           usacElementType[elemIdx] in mpegh3daDecoderConfig() shall indicate ID_USAC_SCE or ID_USAC_EXT */
        restrictionsAreMet = 0;
        break;
      }
    }
  }

  return restrictionsAreMet;
}

static int isNumberCoreChannelsTooHighForBaseline(const LEVEL level, ascparserUsacDecoderConfig *usacDecoderConfig)
{
  nrCoreChannels = asc.channels + asc.HOATransportChannels + asc.objects;

  switch (level) {
    case L1:
      return nrCoreChannels > 5;
    case L2:
      return nrCoreChannels > 9;
    case L3:
      if (checkObjectOnlyRestrictions(usacDecoderConfig)){
        return nrCoreChannels > 24;
      }
      else{
        return nrCoreChannels > 16;
      }
      return nrCoreChannels > 16;
    case L4:
      return nrCoreChannels > 28;
    case L5:
      return nrCoreChannels > 28;
    default:
      return -1;
  }
}

static int isNumberCoreChannelsTooHighForLc(const LEVEL level)
{
  nrCoreChannels = asc.channels + asc.HOATransportChannels + asc.objects;

  switch (level) {
    case L1:
      return nrCoreChannels > 5;
    case L2:
      return nrCoreChannels > 9;
    case L3:
      return nrCoreChannels > 16;
    case L4:
      return nrCoreChannels > 28;
    case L5:
      return nrCoreChannels > 28;
    default:
      return -1;
  }
}

static int numberOutputChannelsTooGreat(const LEVEL level, const int numChannelsAsc)
{
  switch (level) {
    case L1:
      return getNumberOfOutputChannels((int)outputCicp, fileOutGeo, numChannelsAsc) > 2;
    case L2:
      return getNumberOfOutputChannels((int)outputCicp, fileOutGeo, numChannelsAsc) > 8;
    case L3:
      return getNumberOfOutputChannels((int)outputCicp, fileOutGeo, numChannelsAsc) > 12;
    case L4:
      return getNumberOfOutputChannels((int)outputCicp, fileOutGeo, numChannelsAsc) > 24;
    case L5:
      return getNumberOfOutputChannels((int)outputCicp, fileOutGeo, numChannelsAsc) > 24;
    default:
      return -1;
  }
}

static int isNumberChannelsInReferenceLayoutTooHigh(const LEVEL level, ascparserUsacDecoderConfig* usacDecoderConfig)
{
  nrReferenceLayoutChannels = asc.referenceLayout.numLoudspeakers;

  switch (level) {
  case L1:
    return nrReferenceLayoutChannels > 5;
  case L2:
    return nrReferenceLayoutChannels > 9;
  case L3:
    if (checkObjectOnlyRestrictions(usacDecoderConfig)) {
      return nrReferenceLayoutChannels > 24;
    }
    else {
      return nrReferenceLayoutChannels > 16;
    }
    return nrReferenceLayoutChannels > 16;
  case L4:
    return nrReferenceLayoutChannels > 24;
  case L5:
    return nrReferenceLayoutChannels > 24;
  default:
    return -1;
  }
}

static int checkTargetSamplingRate(const LEVEL level, const int usacSamplingFreq, const int targetSamplingRate)
{
  int err                  = 0;
  int coreRateFamilyIdx    = 0;
  int targetRateFamiliyIdx = 0;

  coreRateFamilyIdx    = samplingRateFamily(usacSamplingFreq);
  targetRateFamiliyIdx = samplingRateFamily(targetSamplingRate);

  if (coreRateFamilyIdx != targetRateFamiliyIdx) {
    return -1;
  }

  switch (level) {
    case L1:
    case L2:
    case L3:
    case L4:
      if ((MPEG3DA_44100 != targetSamplingRate) && (MPEG3DA_48000 != targetSamplingRate)) {
        err = -1;
      }
      break;
    case L5:
      if (targetRateFamiliyIdx == MPEG3DA_44100_TYPE) {
        if (MPEG3DA_44100 > targetSamplingRate) {
          err = -1;
        }
      } else {
        if (MPEG3DA_48000 > targetSamplingRate) {
          err = -1;
        }
      }
      break;
    default:
      fprintf( stderr, "\nLevel not defined!\n\n");
      return -2;
  }

  return err;
}

static int checkMaxNumNumMaeGroupsBaseline(const LEVEL level, const int groups, ascparserUsacDecoderConfig *usacDecoderConfig)
{
  int maxNumGroups[5] = {5, 9, 16, 28, 28};
    if (checkObjectOnlyRestrictions(usacDecoderConfig)){
      maxNumGroups[2] = 24;
    }
  return !(groups > maxNumGroups[level-1]);
}
static int checkMaxNumNumMaeGroupsLc(const LEVEL level, const int groups)
{
  int maxNumGroups[5] = {5, 9, 16, 28, 28};
  return !(groups > maxNumGroups[level-1]);
}


static int checkMaxNumNumSwitchGroups(const LEVEL level, const int switchGroups)
{
  int maxNumSwitchGroups[5] = {2, 4, 8, 14, 14};
  return !(switchGroups > maxNumSwitchGroups[level-1]);
}

static int checkMaxNumNumPresets(const LEVEL level, const int presets)
{
  int maxNumPresets[5] = {4, 4, 8, 16, 31};
  return !(presets > maxNumPresets[level-1]);
}

static int checkMaxNumNumPresetConditions(const LEVEL level)
{
  int maxNumPresetConditions[5] = {5, 9, 16, 16, 16};
  int i;
  int ct = 0;
  for (i = 0; i < asi.asi.numGroupPresets; i++) {
    ct = asi.asi.groupPresets[i].presetNumConditions;

    if (ct > maxNumPresetConditions[level - 1]) {
      return ct;
    }
  }
  return -1;
}

static int checkMaxNumDmxIdExtensions(const LEVEL level)
{
  int maxNumDownmixIdExtensions[5] = {4, 4, 8, 16, 31};
  int i;
  int ct = 0;
  for (i = 0; i < asi.asi.numGroupPresets; i++)
  {
    ct = asi.asi.groupPresetExtensions[i].numDownmixIdPresetExtensions;
    if (ct >  maxNumDownmixIdExtensions[level-1])
    {
      return ct;
    }
  }
  return -1;
}

static int getSignalGroupType(int groupIndex)
{
  /* assuming all mae_group members are originating from one signal group (and are of equal type) */
  int startID = asi.asi.groups[groupIndex].startID;
  int i, j;
  int sGroupIndex = -1;
  int groupType = -1;
  int maeID = -1;
  int numSignals = 0;

  for (i = 0; i < asc.numSignalGroups; i++)
  {
    int type = asc.signalGroupType[i];
    
    if (type == 0) /* assume first signal group as channel group */
    {
      numSignals = asc.channels;
    }
    if (type == 1) /* assume second signal group as objects group */
    {
      numSignals = asc.objects;
    }
    else
    {
      return -1;
    }
    for (j = 0; j < numSignals; j++)
    {
      maeID++;
      if (maeID == startID)
      {
        sGroupIndex = i;
        break;
      }
    }
  }

  if (sGroupIndex == -1)
  {
    /* fallback*/
     groupType = asc.signalGroupType[groupIndex];
  }
  else
  {
    groupType = asc.signalGroupType[sGroupIndex];
  }

  return groupType;
}

static int numDecodedObjectsCheck(const LEVEL level)
{
  int maxNumDecodedObjects[5] = {5, 9, 16, 28, 28};
  int numSG = asi.asi.numSwitchGroups;
  int i, j;
  int ct = 0;

  for (i = 0; i < numSG; i++)
  {
    int onOff = asi.asi.switchGroups[i].defaultOnOff;
    if (onOff == 1)
    {
      int defaultID = asi.asi.switchGroups[i].switchGroupDefaultGroupID;
      for (j = 0; j < asi.asi.numGroups; j++)
      {
        int signalGroupType = getSignalGroupType(j);
        if (signalGroupType > -1)
        {
          if ((asi.asi.groups[j].groupID == defaultID) && (signalGroupType == 1))
          {
            ct += asi.asi.groups[j].groupNumMembers; /* count objects only */
          }
          break;
        }
      }
    }
    if (ct > maxNumDecodedObjects[level-1])
    {
      return ct;
    }
  }
  return -1;
}

static int numDecodedChannelsCheckBaseline(const LEVEL level, ascparserUsacDecoderConfig *usacDecoderConfig)
{
  int numSG = asi.asi.numSwitchGroups;
  int i, j;
  int ct = 0;
  int maxNumDecodedChannels[5] = {5, 9, 16, 28, 28};

    if (checkObjectOnlyRestrictions(usacDecoderConfig)){
      maxNumDecodedChannels[2] = 24;
    }

    for (i = 0; i < numSG; i++)
  {
    int onOff = asi.asi.switchGroups[i].defaultOnOff;
    if (onOff == 1)
    {
      int defaultID = asi.asi.switchGroups[i].switchGroupDefaultGroupID;
      for (j = 0; j < asi.asi.numGroups; j++)
      {
        int signalGroupType = getSignalGroupType(j);
        if (signalGroupType > -1)
        {
          if ((asi.asi.groups[j].groupID == defaultID) && (signalGroupType == 0))
          {
            ct += asi.asi.groups[j].groupNumMembers; /* count channels only */
          }
          break;
        }
      }
    }
    if (ct > maxNumDecodedChannels[level-1])
    {
      return ct;
    }
  }
  return -1;
}

static int numDecodedChannelsCheckLc(const LEVEL level)
{
  int numSG = asi.asi.numSwitchGroups;
  int i, j;
  int ct = 0;
  int maxNumDecodedChannels[5] = {5, 9, 16, 28, 28};
    for (i = 0; i < numSG; i++)
  {
    int onOff = asi.asi.switchGroups[i].defaultOnOff;
    if (onOff == 1)
    {
      int defaultID = asi.asi.switchGroups[i].switchGroupDefaultGroupID;
      for (j = 0; j < asi.asi.numGroups; j++)
      {
        int signalGroupType = getSignalGroupType(j);
        if (signalGroupType > -1)
        {
          if ((asi.asi.groups[j].groupID == defaultID) && (signalGroupType == 0))
          {
            ct += asi.asi.groups[j].groupNumMembers; /* count channels only */
          }
          break;
        }
      }
    }
    if (ct > maxNumDecodedChannels[level-1])
    {
      return ct;
    }
  }
  return -1;
}

static PROFILE getProfile(int ascProfileLevelIndication)
{
  PROFILE tmpProfile = PROFILE_LOW_COMPLEXITY;

  switch ( ascProfileLevelIndication ) {
    case 0x0:
      tmpProfile = PROFILE_RESERVED_ISO;
      break;
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
      tmpProfile = PROFILE_MAIN;
      break;
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
      tmpProfile = PROFILE_HIGH;
      break;
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      tmpProfile = PROFILE_LOW_COMPLEXITY;
      break;
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
      tmpProfile = PROFILE_BASELINE;
      break;
    default:
      tmpProfile = PROFILE_UNDEFINED;
  }

  fprintf ( stdout, "\n" );
  fprintf ( stdout, "::-----------------------------------------------------------------------------\n::    " );
  switch ( tmpProfile ) {
    case PROFILE_MAIN:
      fprintf ( stdout, "Main Profile no longer supported!\n" );
      break;
    case PROFILE_RESERVED_ISO:
      fprintf ( stdout, "Profile is reserved for ISO!\n" );
      break;
    case PROFILE_HIGH:
      fprintf ( stdout, "Decoding High Profile bit stream...\n" );
      break;
    case PROFILE_LOW_COMPLEXITY:
      fprintf ( stdout, "Decoding Low Complexity Profile bit stream...\n" );
      break;
    case PROFILE_BASELINE:
      fprintf ( stdout, "Decoding Baseline Profile bit stream...\n" );
      break;
    default:
      fprintf ( stdout, "Undefined Profile!\n" );
      break;
  }
  fprintf ( stdout, "::-----------------------------------------------------------------------------\n" );

  return tmpProfile;
}

static LEVEL getLevel(int ascProfileLevelIndication)
{
  LEVEL tmpLevel = L_RESERVED_ISO;

  switch ( ascProfileLevelIndication ) {

    case 0x0:
      tmpLevel = L_RESERVED_ISO;
      break;
    case 0x01:
    case 0x06:
    case 0x0B:
    case 0x10:
      tmpLevel = L1;
      break;
    case 0x02:
    case 0x07:
    case 0x0C:
    case 0x11:
      tmpLevel = L2;
      break;
    case 0x03:
    case 0x08:
    case 0x0D:
    case 0x12:
      tmpLevel = L3;
      break;
    case 0x04:
    case 0x09:
    case 0x0E:
    case 0x13:
      tmpLevel = L4;
      break;
    case 0x05:
    case 0x0A:
    case 0x14:
    case 0x0F:
      tmpLevel = L5;
      break;
    default:
      tmpLevel = L_UNDEFINED;
      break;
  }

  return tmpLevel;
}

static void getProfilesAndLevelsOfAllCompatibleSets(ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet, PROFILE* compatibleProfiles, LEVEL* compatibleProfileLevels)
{
  unsigned int i;

  for (i = 0; i < compatibleProfileLevelSet->numCompatibleSets; i++) {
    compatibleProfiles[i] = getProfile(compatibleProfileLevelSet->CompatibleSetIndication[i]);
    compatibleProfileLevels[i] = getLevel(compatibleProfileLevelSet->CompatibleSetIndication[i]);
  }
}

static MPEG3DA_RETURN_CODE checkBaselineProfileConstraints(
  int* error_depth,
  VERBOSE_LEVEL verboseLvl,
  LEVEL tmpLevel,
  const int targetSamplingRate,
  ASCPARSER_DOWNMIX_CONFIG *downmixConfig,
  ascparserUsacDecoderConfig *usacDecoderConfig
) {
  if ( asc.sbrRatioIndex > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", "SBR not allowed in Baseline Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.maxStereoConfigIndex > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", "MPS212 not allowed in Baseline Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.SAOCTransportChannels > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", "SAOC not allowed in Baseline Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.oam_mode == 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", "Efficient Object Metadata Decoding is not permitted,\n\ti.e. lowDelayMetadataCoding shall be 1 for Baseline Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (downmixConfig->phaseAlignStrength != 0){
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "phaseAlignStrength shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
  {
    unsigned int i;
    for (i = 0; i < usacDecoderConfig->m_numElements; i++) {
      if (usacDecoderConfig->m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_EXT) {
      ascparserUsacExtConfig usacExtConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacExtConfig;

        if ((usacExtConfig.m_usacExtElementType == ASCPARSER_USAC_ID_EXT_ELE_HOA)) {
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", "HOA not allowed in Baseline Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      else if (usacDecoderConfig->m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_CPE) {
        ascparserUsacCpeConfig usacCpeConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacCpeConfig;
        ascparserUsacCoreConfig usacCoreConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacCpeConfig.m_usacCoreConfig;
        
        if (usacCoreConfig.m_tw_mdct != 0){
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "Time warped MDCT shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (usacCoreConfig.m_fullbandLPD != 0){
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "fullbandLPD shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (usacCpeConfig.m_stereoConfigIndex) {
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "stereoConfigIndex shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (usacCpeConfig.m_qceIndex) {
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "qceIndex shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }

      } else if(usacDecoderConfig->m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_SCE) {
        ascparserUsacCoreConfig usacCoreConfig = usacDecoderConfig->m_usacElementConfig[i].m_usacSceConfig.m_usacCoreConfig;
        
        if (usacCoreConfig.m_tw_mdct != 0){
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "Time warped MDCT shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (usacCoreConfig.m_fullbandLPD != 0){
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "fullbandLPD shall have the value 0 in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
    }
  }

  {
    int i;

    for (i = 0; i < asc.numSignalGroups; i++) {
      if (!((asc.signalGroupType[i] == ASCPARSER_SIGNAL_GROUP_TYPE_OBJECTS) || (asc.signalGroupType[i] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS))){
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.",  "Signal Group Type shall not be different from Objects or Channels in Baseline profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  if ( asc.outputFrameLength / asc.oam_length > MAX_ALLOWED_CORE_OAM_RATIO ) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The ratio between coreCoderFrameLength and oamFramesize\n\tshall not be greater than %d in Baseline Profile.", MAX_ALLOWED_CORE_OAM_RATIO);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
    
  if ( samplingRateAllowed(tmpLevel, asc.usacSamplingFreq) == 0 ) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The sampling rate %d of the core coder is not allowed in Baseline Profile Level %d.", asc.usacSamplingFreq, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
  if ( !asc.elementLengthPresent && isNumberCoreChannelsTooHighForBaseline(tmpLevel, usacDecoderConfig) ){
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The number of processed core channels (%i signals)\n\tis too great for Baseline Profile Level %d.", nrCoreChannels, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  } else {
    if ( hasASI == 1 ) {
      /* check max number of ... */

      /* ...mae_groups */
      {
        int maeGroups = asi.asi.numGroups;
        if ( checkMaxNumNumMaeGroupsBaseline(tmpLevel, maeGroups, usacDecoderConfig) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of mae_groups (%d) is too high for Baseline Profile Level %d.", maeGroups, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...switch groups */
      {
        int switchGroups = asi.asi.numSwitchGroups;
        if ( checkMaxNumNumSwitchGroups(tmpLevel, switchGroups) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of switch groups (%d) is too high for Baseline Profile Level %d.", switchGroups, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...group presets */
      {
        int presets = asi.asi.numGroupPresets;
        if ( checkMaxNumNumPresets(tmpLevel, presets) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of groupPresets (%d) is too high for Baseline Profile Level %d.", presets, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...group preset conditions */
      if ( checkMaxNumNumPresetConditions(tmpLevel) > 0 )
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of preset conditions (%d) is too high for Baseline Profile Level %d.", checkMaxNumNumPresetConditions(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
        
      /* ...downmixID-dependent group preset extensions per group preset */
      if ( checkMaxNumDmxIdExtensions(tmpLevel) > 0 )
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of downmixID-dependent group preset extensions (%d)\n\tis too high for Baseline Profile Level %d.", checkMaxNumDmxIdExtensions(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }

      /* ASI checked for simultaneous playout */
      /* number of decoded objects */
      if ( numDecodedObjectsCheck(tmpLevel) > 0)
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of simultaneous decoded objects (%d)\n\tis too high for Baseline Profile Level %d.", numDecodedObjectsCheck(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
      /* number of decoded channels */
      if ( numDecodedChannelsCheckBaseline(tmpLevel, usacDecoderConfig) > 0)
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of simultaneous decoded channels (%i)\n\tis too high for Baseline Profile Level %d.", numDecodedChannelsCheckBaseline(tmpLevel, usacDecoderConfig), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  if (isNumberChannelsInReferenceLayoutTooHigh(tmpLevel, usacDecoderConfig)) {
    char tmp[MAX_STRING_LENGTH] = { 0 };
    sprintf(tmp, "The number of channels in referenceLayout (%i channels)\n\tis too great for Baseline Profile Level %d.", nrReferenceLayoutChannels, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (checkTargetSamplingRate(tmpLevel, asc.usacSamplingFreq, targetSamplingRate) != 0) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The given target sampling rate (targetSamplingRate %i) is not allowed for Baseline profile Level %d.", targetSamplingRate, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
  return MPEG3DA_OK;
}

static MPEG3DA_RETURN_CODE checkLcProfileConstraints(
  int* error_depth,
  VERBOSE_LEVEL verboseLvl,
  LEVEL tmpLevel,
  const int targetSamplingRate
) {
  if ( asc.sbrRatioIndex > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", "SBR not allowed in LC Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.maxStereoConfigIndex > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", "MPS212 not allowed in LC Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.SAOCTransportChannels > 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", "SAOC not allowed in LC Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.oam_mode == 0 ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", "Efficient Object Metadata Decoding is not permitted,\n\ti.e. lowDelayMetadataCoding shall be 1 for Low Complexity Profile.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.outputFrameLength / asc.oam_length > MAX_ALLOWED_CORE_OAM_RATIO ) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The ratio between coreCoderFrameLength and oamFramesize\n\tshall not be greater than %d in LC Profile.", MAX_ALLOWED_CORE_OAM_RATIO);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
    
  if ( samplingRateAllowed(tmpLevel, asc.usacSamplingFreq) == 0 ) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The sampling rate %d of the core coder is not allowed in LC Profile Level %d.", asc.usacSamplingFreq, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( !asc.elementLengthPresent && isNumberCoreChannelsTooHighForLc(tmpLevel) ){
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The number of processed core channels (%i signals)\n\tis too great for LC Profile Level %d.", nrCoreChannels, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  } else {
    if ( hasASI == 1 ) {
      /* check max number of ... */

      /* ...mae_groups */
      {
        int maeGroups = asi.asi.numGroups;
        if ( checkMaxNumNumMaeGroupsLc(tmpLevel, maeGroups) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of mae_groups (%d) is too high for LC Profile Level %d.", maeGroups, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...switch groups */
      {
        int switchGroups = asi.asi.numSwitchGroups;
        if ( checkMaxNumNumSwitchGroups(tmpLevel, switchGroups) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of switch groups (%d) is too high for LC Profile Level %d.", switchGroups, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...group presets */
      {
        int presets = asi.asi.numGroupPresets;
        if ( checkMaxNumNumPresets(tmpLevel, presets) == 0 )
        {
          char tmp[MAX_STRING_LENGTH] = {0};
          sprintf(tmp, "The number of groupPresets (%d) is too high for LC Profile Level %d.", presets, tmpLevel);
          return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
      /* ...group preset conditions */
      if ( checkMaxNumNumPresetConditions(tmpLevel) > 0 )
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of preset conditions (%d) is too high for LC Profile Level %d.", checkMaxNumNumPresetConditions(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
        
      /* ...downmixID-dependent group preset extensions per group preset */
      if ( checkMaxNumDmxIdExtensions(tmpLevel) > 0 )
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of downmixID-dependent group preset extensions (%d)\n\tis too high for LC Profile Level %d.", checkMaxNumDmxIdExtensions(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }

      /* ASI checked for simultaneous playout */
      /* number of decoded objects */
      if ( numDecodedObjectsCheck(tmpLevel) > 0)
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of simultaneous decoded objects (%d)\n\tis too high for LC Profile Level %d.", numDecodedObjectsCheck(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
      /* number of decoded channels */
      if ( numDecodedChannelsCheckLc(tmpLevel) > 0)
      {
        char tmp[MAX_STRING_LENGTH] = {0};
        sprintf(tmp, "The number of simultaneous decoded channels (%i)\n\tis too high for LC Profile Level %d.", numDecodedChannelsCheckLc(tmpLevel), tmpLevel);
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  if ( numberOutputChannelsTooGreat(tmpLevel, asc.channels) ) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The number of output channels (cicpOut %i) is too great for LC Profile Level %d.", outputCicp, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_STREAM, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (checkTargetSamplingRate(tmpLevel, asc.usacSamplingFreq, targetSamplingRate) != 0) {
    char tmp[MAX_STRING_LENGTH] = {0};
    sprintf(tmp, "The given target sampling rate (targetSamplingRate %i) is not allowed for LC profile Level %d.", targetSamplingRate, tmpLevel);
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_LC_CONFIG, error_depth, "Error checking profile constraints.", tmp, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  return MPEG3DA_OK;
}

MPEG3DA_RETURN_CODE profileConstraintsTerminator(const int targetSamplingRate,
                                                 PROFILE* profile,
                                                 LEVEL* level,
                                                 ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet,
                                                 int compatibleProfileLevelSetAvailable,
                                                 ASCPARSER_DOWNMIX_CONFIG *downmixConfig,
                                                 ascparserUsacDecoderConfig  *usacDecoderConfig,
                                                 int* error_depth,
                                                 VERBOSE_LEVEL verboseLvl)
{
  PROFILE tmpProfiles[1 + ASCPARSER_USAC_MAX_COMPATIBLE_SETS];  /* Profile Level Indication + Maximum Number of Compatible Sets */
  LEVEL   tmpLevels[1 + ASCPARSER_USAC_MAX_COMPATIBLE_SETS];  /* Profile Level Indication + Maximum Number of Compatible Sets */

  tmpProfiles[0] = getProfile(asc.profileLevelIndication);
  tmpLevels[0]   = getLevel(asc.profileLevelIndication);
  *profile = tmpProfiles[0];
  *level   = tmpLevels[0];

  if (compatibleProfileLevelSetAvailable) {
    getProfilesAndLevelsOfAllCompatibleSets(compatibleProfileLevelSet, &tmpProfiles[1], &tmpLevels[1]);
  }

  if ( asc.oam_length > asc.outputFrameLength ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_STREAM, error_depth, "Error checking profile constraints.", "OAM FrameLength shall not be greater than the coreCoderFrameLength.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.outputFrameLength % asc.oam_length ) {
    return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_STREAM, error_depth, "Error checking profile constraints.", "coreCoderFrameLength shall be an integer multiple of the OAMFrameLength.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( asc.receiverDelayCompensation == 0 && asc.numSignalGroups > 1 ) {
    int i = 1, j = 0;
    for ( i = 1; i < asc.numSignalGroups; ++i ) {
      if ( asc.signalGroupType[i-1] != asc.signalGroupType[i] ) {
        for ( j = 0; j < asc.numSignalGroups; ++j ) {
          fprintf(stderr, "\nInfo: signalGroupType of signalGroup %i: %i \n", j, asc.signalGroupType[j] );
        }
        return PrintErrorCode(MPEG3DA_ERROR_PCT_INVALID_STREAM, error_depth, "Error checking profile constraints.", "receiverDelayCompensation shall be set to 1 when containing signal groups with different signalGroupTypes.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  {
    unsigned int i;
    for (i = 0; i < (1 + compatibleProfileLevelSet->numCompatibleSets); i++) {
      PROFILE tmpProfile = tmpProfiles[i];
      LEVEL tmpLevel     = tmpLevels[i];

      if (tmpProfile == PROFILE_LOW_COMPLEXITY) {
        MPEG3DA_RETURN_CODE error = checkLcProfileConstraints(
          error_depth,
          verboseLvl,
          tmpLevel,
          targetSamplingRate
        );
        if (error != MPEG3DA_OK) return error;

      } else if(tmpProfile == PROFILE_BASELINE) {
        MPEG3DA_RETURN_CODE error = checkBaselineProfileConstraints(
          error_depth,
          verboseLvl,
          tmpLevel,
          targetSamplingRate,
          downmixConfig,
          usacDecoderConfig
        );
        if (error != MPEG3DA_OK) return error;
      }
    }
  }

  return MPEG3DA_OK;
}
