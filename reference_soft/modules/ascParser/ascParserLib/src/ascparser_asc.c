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

#ifndef WD1_FRAMEWORK_CONFIG_3D
#define WD1_FRAMEWORK_CONFIG_3D
#endif

#ifndef WD1_ELEMENT_MAPPING_3D
#define WD1_ELEMENT_MAPPING_3D
#endif

#ifndef WD1_ESCAPED_OBJECTS_MAPPING_3D
#define WD1_ESCAPED_OBJECTS_MAPPING_3D
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ascparser_asc.h"
#include "ascparser_bitstream.h"
#include "ascparser.h"
#include "ascparser_usacconfig.h"

static const int SamplingRateInfoTable[16] =
{ 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, 0 };

static ASCPARSER_AUDIO_OBJECT_TYPE __getAOT(ascparserBitStreamPtr bs);
static INT32 copy_SpeakerConfig3d_to_ascparser_Layout(ASCPARSER_SPEAKER_CONFIG_3D  *ascparser_Layout, SpeakerConfig3d SpeakerConfig3d_Layout);


INT32 ascparser_ASC_ParseExt(
                                   ascparser_ASCPtr self, 
                                   ascparser_ASCPtr baselayer, 
                                   ascparserBitStreamPtr bs, 
                                   UINT8 streamsPerLayer, 
                                   UINT32 avgBitrate,
                                   UINT8 latm_flag
                                   )
{
  UINT32 dummyId = 0;
  UINT32 ascLen = 0;

  ascLen = __getValidBits( bs );

  self->m_noOfStreams = streamsPerLayer;
  self->m_avgBitRate = avgBitrate;


  self->m_aot = __getAOT(bs);
  self->m_samplingFrequencyIndex = __getBits( bs, dummyId, 4 ) ;

  if( self->m_samplingFrequencyIndex == 0xf ) {
    self->m_samplingFrequency = __getBits( bs, dummyId, 24 ) ;
  } else {
    self->m_samplingFrequency = SamplingRateInfoTable[self->m_samplingFrequencyIndex];
  }

  self->m_channelConfiguration = __getBits(bs, dummyId, 4) ;
  /* end of generic part */


  /* SBR extension ( explicit non-backwards compatible mode ) */
  self->m_sbrPresentFlag = -1;
  self->m_psPresentFlag  = -1;

  if ( self->m_aot == AOT_SBR || self->m_aot == AOT_PS ) {
    /* self->m_extensionAudioObjectType = self->m_aot; */    /* according to w6791, this has to be: self->m_extensionAudioObjectType = AOT_SBR ! */
    self->m_extensionAudioObjectType = AOT_SBR;
    self->m_sbrPresentFlag = 1;

    if ( self->m_aot == AOT_PS ) {
      self->m_psPresentFlag = 1;
    }
    
    self->m_extensionSamplingFrequencyIndex = __getBits( bs, dummyId, 4 );

    if ( self->m_extensionSamplingFrequencyIndex == 0xf ) {
      self->m_extensionSamplingFrequency = __getBits( bs, dummyId, 24 );
    } else {
      self->m_extensionSamplingFrequency = SamplingRateInfoTable[self->m_extensionSamplingFrequencyIndex];
    }

    self->m_aot = __getAOT(bs);
  } else {
    self->m_extensionAudioObjectType = AOT_NULL_OBJECT;
  }

  /* sanity check for unknown AOTs */
  switch (self->m_aot) {
  case AOT_SAAS:
    break;   /* go on, we support parsing these specific configs */

  default:
    return 0;
  }

  /* SAAS */
  if ( self->m_aot == AOT_SAAS ) {
    ascparserUsacConfig_Parse(&self->m_usacConfig, bs);

    self->m_extensionSamplingFrequency = self->m_usacConfig.m_usacSamplingFrequency;
    if(self->m_usacConfig.m_sbrRatioIndex > 0){
      self->m_sbrPresentFlag = 1;
      self->m_samplingFrequency = self->m_extensionSamplingFrequency/2;
    
      if (self->m_usacConfig.m_sbrRatioIndex == 1) {
        self->m_samplingFrequency = self->m_extensionSamplingFrequency/4;
      }
    }
  }

  ascparserUsacConfig_GetProductionMetadataConfig(&self->m_usacConfig,
                                                  &self->m_has_reference_distance,
                                                  &self->m_bs_reference_distance,
                                                  &self->m_has_object_distance[0],
                                                  &self->m_directHeadphone[0],
                                                  &self->m_numObjectGroups,
                                                  &self->m_numChannelGroups);

  /************* derived values *******************/
  /* set nr of channels (derived value) */
  self->m_channels = -1;
  self->m_objects  = 0;
  switch (self->m_channelConfiguration) {
  case 0:
    if(self->m_aot == AOT_SAAS){
      self->m_saocHeaderLength = ascparserUsacConfig_GetSaocHeaderLength(&self->m_usacConfig);
 
      ascparserUsacConfig_GetObjectMetadataConfig(&self->m_usacConfig, &self->m_objectMetadataMode, &self->m_objectMetadataFrameLength, &self->m_objectMetadataHasScreenRelativeObjects, self->m_objectMetadataIsScreenRelativeObject, &self->m_objectMetadataHasDynamicObjectPriority, &self->m_objectMetadataHasUniformSpread);

    }
    else
        return -1;
  
    break;
  case 1:  self->m_channels =  1; break;
  case 2:  self->m_channels =  2; break;
  case 3:  self->m_channels =  3; break;
  case 4:  self->m_channels =  4; break;
  case 5:  self->m_channels =  5; break;
  case 6:  self->m_channels =  6; break;
  case 7:  self->m_channels =  8; break;
  case 13: self->m_channels = 24; break;
  default: 
    return -1;
    break;
  }

#ifdef WD1_FRAMEWORK_CONFIG_3D
  self->m_channels = self->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioChannels;
  self->m_objects = self->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioObjects;
  self->m_SAOCTransportChannels = self->m_usacConfig.m_usacFramework.m_signals3D.m_numSAOCTransportChannels;

#ifdef SUPPORT_HOA
  self->m_HOATransportChannels = self->m_usacConfig.m_usacFramework.m_signals3D.m_numHOATransportChannels;
#endif

#endif

  /* samplesperframe */
  switch (self->m_aot) {

  case AOT_SAAS:
    switch(self->m_usacConfig.m_sbrRatioIndex){
    case 0:
      if(self->m_usacConfig.m_outputFrameLength == 768){
        self->m_samplesPerFrame = 768;
      } else {
        self->m_samplesPerFrame = 1024;
      }
      break;
    case 2:
      self->m_samplesPerFrame = 768;
      break;
    default:
      self->m_samplesPerFrame = 1024;
      break;
    }
    break;

  default:
    return -1;
    break;
  }

  return 0;
}

INT32 ascparser_ASC_FillEnhancedObjectMetadataConfig(const ascparser_ASCPtr asc, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig)
{
  enhancedObjectMetadataConfig->hasDiffuseness = asc->m_enhObjectHasDiffuseness;
  enhancedObjectMetadataConfig->hasCommonGroupDiffuseness = asc->m_enhObjectHasCommonGroupDiffuseness;
  enhancedObjectMetadataConfig->hasExcludedSectors = asc->m_enhObjectHasExcludedSectors;
  enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors = asc->m_enhObjectHasCommonGroupExcludedSectors;
  enhancedObjectMetadataConfig->hasClosestSpeakerCondition = asc->m_enhObjectHasClosestSpeakerCondition;
  enhancedObjectMetadataConfig->closestSpeakerThresholdAngle = asc->m_enhObjectClosestSpeakerThresholdAngle;

  memcpy(enhancedObjectMetadataConfig->useOnlyPredefinedSectors, asc->m_enhObjectUseOnlyPredefinedSectors, sizeof(UINT32)*ASCPARSER_USAC_MAX_ELEMENTS);
  memcpy(enhancedObjectMetadataConfig->hasDivergence, asc->m_enhObjectHasDivergence, sizeof(float)*ASCPARSER_USAC_MAX_ELEMENTS);
  memcpy(enhancedObjectMetadataConfig->divergenceAzimuthRange, asc->m_enhObjectDivergenceAzimuthRange, sizeof(float)*ASCPARSER_USAC_MAX_ELEMENTS);
  
  return 0;
}

INT32 ascparser_ASC_FillProductionMetadataConfig(const ascparser_ASCPtr asc, ASCPARSER_PRODUCTION_METADATA_CONFIG* productionMetadataConfig)
{
  int i = 0;

  productionMetadataConfig->has_reference_distance = asc->m_has_reference_distance;

  if (productionMetadataConfig->has_reference_distance == 0) {
    productionMetadataConfig->bs_reference_distance = 57;
  } else {
    productionMetadataConfig->bs_reference_distance = asc->m_bs_reference_distance;
  }

  for (i = 0 ; i < asc->m_numObjectGroups ; i++){
    productionMetadataConfig->has_object_distance[i] = asc->m_has_object_distance[i];
  }

  for (i = 0 ; i < asc->m_numChannelGroups ; i++){
    productionMetadataConfig->directHeadphone[i] = asc->m_directHeadphone[i];
  }

  return 0;
}

INT32 ascparser_ASC_FillAudioGroupInfo(const ascparser_ASCPtr asc, ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, int* signalGroupInfoAvailable)
{
  unsigned int i, j;
  *signalGroupInfoAvailable = 0;

  signalGroupInfo->numSignalGroups = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSignalGroups;

  for (i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i) {
    if (asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_SIG_GROUP_INFO) {
      ASCPARSER_SIGNAL_GROUP_INFO* pSigInfo = (ASCPARSER_SIGNAL_GROUP_INFO*) asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i];
      *signalGroupInfoAvailable = 1;
      for (j = 0; j < signalGroupInfo->numSignalGroups; j++) {
        signalGroupInfo->fixedPosition[j] = pSigInfo->fixedPosition[j];
        signalGroupInfo->groupPriority[j] = pSigInfo->groupPriority[j];
      }
    }
  }

  if (*signalGroupInfoAvailable == 0) {
    for (j = 0; j < signalGroupInfo->numSignalGroups; j++) {
      signalGroupInfo->fixedPosition[j] = 1;
      signalGroupInfo->groupPriority[j] = 7;
    }
  }

  return 0;
}

INT32 ascparser_ASC_FillCompatibleProfileLevelSet(const ascparser_ASCPtr asc, ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet, int* compatibleProfileLevelSetAvailable)
{
  unsigned int i,j;

  for (i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i) {
    if (asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET) {
      ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* pCompatibleProfileLevelSet = (ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET*) asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i];
      *compatibleProfileLevelSetAvailable = 1;
      compatibleProfileLevelSet->numCompatibleSets = pCompatibleProfileLevelSet->numCompatibleSets;
      compatibleProfileLevelSet->reserved = pCompatibleProfileLevelSet->reserved;

      for (j = 0; j < compatibleProfileLevelSet->numCompatibleSets; j++) {
        compatibleProfileLevelSet->CompatibleSetIndication[j] = pCompatibleProfileLevelSet->CompatibleSetIndication[j];
      }
    }
  }

  return 0;
}

INT32 ascparser_ASC_FillAudioGroupConfig(const ascparser_ASCPtr asc, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig)
{
  unsigned int i = 0;

  /*
  signalGroupConfig->numSignalGroups = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSignalGroups;
  for (i = 0; i < signalGroupConfig->numSignalGroups; i++)
  {
    signalGroupConfig->signalGroupType[i] = asc->m_usacConfig.m_usacFramework.m_signals3D.m_signalGroupType[i];
    signalGroupConfig->numberOfSignals[i] = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numberOfSignals[i];
    memcpy(&signalGroupConfig->signalGroupChannelLayout[i], &asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[i], sizeof(asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[i]));
  }
  */

  signalGroupConfig->numSignalGroups = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSignalGroups;

  memcpy(&signalGroupConfig->signalGroupType,
         &asc->m_usacConfig.m_usacFramework.m_signals3D.m_signalGroupType,
         sizeof(asc->m_usacConfig.m_usacFramework.m_signals3D.m_signalGroupType));

  memcpy(&signalGroupConfig->numberOfSignals,
         &asc->m_usacConfig.m_usacFramework.m_signals3D.m_numberOfSignals,
         sizeof(asc->m_usacConfig.m_usacFramework.m_signals3D.m_numberOfSignals));

  memcpy(&signalGroupConfig->signalGroupChannelLayout,
         &asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout,
        sizeof(asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout));

  for (i = 0; i < signalGroupConfig->numSignalGroups; i++) 
  {
    signalGroupConfig->numberOfSignals[i]++;
    if (asc->m_usacConfig.m_usacFramework.m_signals3D.m_differsFromReferenceLayout[i] == 0) 
    {
      /* copy reference layout to signalGroupConfig */
      int nChannels = 0;
      int nLFE = 0;
      signalGroupConfig->signalGroupChannelLayout[i].speakerLayoutID = asc->m_usacConfig.m_referenceLayout.speakerLayoutID;
      if (signalGroupConfig->signalGroupChannelLayout[i].speakerLayoutID == 0)
      {
        CICP2GEOMETRY_CHANNEL_GEOMETRY temp[32];
        signalGroupConfig->signalGroupChannelLayout[i].CICPspeakerLayoutIdx = asc->m_usacConfig.m_referenceLayout.CICPspeakerLayoutIdx;
        cicp2geometry_get_geometry_from_cicp( signalGroupConfig->signalGroupChannelLayout[i].CICPspeakerLayoutIdx, 
                                          temp, &nChannels, &nLFE);

        signalGroupConfig->signalGroupChannelLayout[i].numSpeakers = nChannels + nLFE;
      }
      else if (signalGroupConfig->signalGroupChannelLayout[i].speakerLayoutID == 1)
      {
        unsigned int j = 0;
        signalGroupConfig->signalGroupChannelLayout[i].numSpeakers = asc->m_usacConfig.m_referenceLayout.numSpeakers;
        for ( j = 0; j < signalGroupConfig->signalGroupChannelLayout[i].numSpeakers; ++j )
        {
          signalGroupConfig->signalGroupChannelLayout[i].CICPspeakerIdx[j] = asc->m_usacConfig.m_referenceLayout.CICPspeakerIdx[j];
        }
      }
      else if ( signalGroupConfig->signalGroupChannelLayout[i].speakerLayoutID == 2 )
      {   
        unsigned int j = 0;
        signalGroupConfig->signalGroupChannelLayout[i].numSpeakers = asc->m_usacConfig.m_referenceLayout.numSpeakers;

        for ( j = 0; j < signalGroupConfig->signalGroupChannelLayout[i].numSpeakers; ++j )
        {
          signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].isCICPspeakerIdx = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].isCICPspeakerIdx;
          if(signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].isCICPspeakerIdx) 
          {
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].CICPspeakerIdx = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].CICPspeakerIdx;
          }
          else 
          {
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.angularPrecision = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.angularPrecision;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].isCICPspeakerIdx = -1;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].AzimuthAngleIdx = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].AzimuthAngleIdx;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].AzimuthDirection = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].AzimuthDirection;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].ElevationAngleIdx = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].ElevationAngleIdx;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].ElevationClass = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].ElevationClass;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].ElevationDirection = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].ElevationDirection;
            signalGroupConfig->signalGroupChannelLayout[i].flexibleSpeakerConfig.speakerDescription[j].isLFE = asc->m_usacConfig.m_referenceLayout.flexibleSpeakerConfig.speakerDescription[j].isLFE;
          }
        }
      }
    }
  }
  return 0;
}

INT32 ascparser_ASI_FillStruct(const ascparser_ASCPtr asc, ASCPARSER_AUDIO_SCENE* audioScene, int asi_present, int* asi_valid)
{
  if (asi_present) {
    audioScene->asi.bsMetaDataElementIDoffset = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.bsMetaDataElementIDoffset;
    audioScene->asi.isMainStream              = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.isMainStream;
    audioScene->asi.audioSceneInfoIDPresent   = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.audioSceneInfoIDPresent;
    audioScene->asi.audioSceneInfoID          = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.audioSceneInfoID;
    audioScene->asi.numDataSets               = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.numDataSets;
    audioScene->asi.numGroups                 = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.numGroups;
    audioScene->asi.numSwitchGroups           = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.numSwitchGroups; 
    audioScene->asi.numGroupPresets           = asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.numGroupPresets; 

    memcpy(&audioScene->asi.groups,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groups,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groups));

    memcpy(&audioScene->asi.switchGroups,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.switchGroups,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.switchGroups));

    memcpy(&audioScene->asi.groupPresets,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresets,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresets));

    memcpy(&audioScene->asi.compositeObjectPair,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.compositeObjectPair,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.compositeObjectPair));
    
    memcpy(&audioScene->asi.content,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.content,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.content));
    
    memcpy(&audioScene->asi.groupDescription,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupDescription,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupDescription));

    memcpy(&audioScene->asi.switchGroupDescription,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.switchGroupDescription,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.switchGroupDescription));

    memcpy(&audioScene->asi.groupPresetDescription,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresetDescription,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresetDescription));

    memcpy(&audioScene->asi.productionScreen,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.productionScreen,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.productionScreen));

    if (asc->m_objectMetadataHasScreenRelativeObjects == 1) {
      memcpy(audioScene->screenRelativeObjects,
             asc->m_objectMetadataIsScreenRelativeObject,
             sizeof(asc->m_objectMetadataIsScreenRelativeObject));
    }

    /* copy extensions */
    memcpy(&audioScene->asi.groupPresetExtensions,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresetExtensions,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.groupPresetExtensions));

    memcpy(&audioScene->asi.productionScreenExtensions,
           &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.productionScreenExtensions,
           sizeof(asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo.productionScreenExtensions));
  }

  if (NULL != asi_valid) {
    if (audioScene->asi.numGroups > 0) {
      /* indication for available ASI -> used to trigger metadata preprocessing */
      *asi_valid = asi_present;
    } else {
      *asi_valid = 0;
    }
  }

  return 0;
}


static INT32 copy_SpeakerConfig3d_to_ascparser_Layout(ASCPARSER_SPEAKER_CONFIG_3D  *ascparser_Layout, SpeakerConfig3d SpeakerConfig3d_Layout)
{
  int i = 0;
  int nChannels = 0;
  int nLFE      = 0;

  ascparser_Layout->speakerLayoutID         = SpeakerConfig3d_Layout.speakerLayoutID;

  if ( ascparser_Layout->speakerLayoutID == 0 )
  {
    ascparser_Layout->CICPspeakerLayoutIdx    = SpeakerConfig3d_Layout.CICPspeakerLayoutIdx;
    cicp2geometry_get_geometry_from_cicp( ascparser_Layout->CICPspeakerLayoutIdx, 
                                          ascparser_Layout->geometryInfo, &nChannels, &nLFE);

    ascparser_Layout->numLoudspeakers = nChannels + nLFE;
  }
  else if ( ascparser_Layout->speakerLayoutID == 1 )
  {
    int i = 0;
    ascparser_Layout->numLoudspeakers = SpeakerConfig3d_Layout.numSpeakers;

    for ( i = 0; i < ascparser_Layout->numLoudspeakers; ++i )
    {
      cicp2geometry_get_geometry_from_cicp_loudspeaker_index(SpeakerConfig3d_Layout.CICPspeakerIdx[i], 
                                                             &(ascparser_Layout->geometryInfo[i])); 
    }
  }
  else if ( ascparser_Layout->speakerLayoutID == 2 )
  {   
    int i = 0;
    ascparser_Layout->numLoudspeakers = SpeakerConfig3d_Layout.numSpeakers;

    for ( i = 0; i < ascparser_Layout->numLoudspeakers; ++i )
    {
      if(SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx) {
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx, 
                                                               &(ascparser_Layout->geometryInfo[i]));
      }
      else {
        ascparser_Layout->geometryInfo[i].cicpLoudspeakerIndex = -1;
        ascparser_Layout->geometryInfo[i].Az = ascparser_AzimuthAngleIdxToDegrees(SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx, 
                                                                                   SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection,  
                                                                                   SpeakerConfig3d_Layout.flexibleSpeakerConfig.angularPrecision);
        ascparser_Layout->geometryInfo[i].El = ascparser_ElevationAngleIdxToDegrees(SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx, 
                                                                                     SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].ElevationDirection,  
                                                                                     SpeakerConfig3d_Layout.flexibleSpeakerConfig.angularPrecision);

        ascparser_Layout->geometryInfo[i].LFE = SpeakerConfig3d_Layout.flexibleSpeakerConfig.speakerDescription[i].isLFE;

      }
    }

  }
    
  return 0;
}


INT32 ascparser_ASC_FillStruct(const ascparser_ASCPtr asc, ASCPARSERINFO* ascStruct)
{
  int i = 0;
  int nChannels = 0;
  int nLFE      = 0;

  memset(ascStruct, 0, sizeof(ASCPARSERINFO) );

  ascStruct->aot                          = asc->m_aot;
  ascStruct->fs                           = asc->m_samplingFrequency;

  if (asc->m_extensionSamplingFrequency != 0) {
    ascStruct->ext_fs                     = asc->m_extensionSamplingFrequency;
  }

  ascStruct->chConfig                     = asc->m_channelConfiguration;
  ascStruct->channels                     = asc->m_channels;
  ascStruct->objects                      = asc->m_objects;
  ascStruct->SAOCTransportChannels        = asc->m_SAOCTransportChannels;

#ifdef SUPPORT_HOA
  ascStruct->HOATransportChannels         = asc->m_HOATransportChannels;
#endif

  ascStruct->has_reference_distance       = asc->m_has_reference_distance;
  ascStruct->bs_reference_distance        = asc->m_bs_reference_distance;
  for (i = 0; i < asc->m_numObjectGroups; ++i) {
    ascStruct->has_object_distance[i]     = asc->m_has_object_distance[i];
  }
  for (i = 0; i < asc->m_numChannelGroups; ++i) {
    ascStruct->directHeadphone[i]         = asc->m_directHeadphone[i];
  }
  ascStruct->receiverDelayCompensation    = asc->m_usacConfig.m_receiverDelayCompensation;
  ascStruct->profileLevelIndication       = asc->m_usacConfig.m_mpegh3daProfileLevelIndication;
  ascStruct->elementLengthPresent         = asc->m_usacConfig.m_usacDecoderConfig.m_elementLengthPresent;
  ascStruct->oam_mode                     = asc->m_objectMetadataMode;
  ascStruct->oam_length                   = asc->m_objectMetadataFrameLength;
  ascStruct->oam_hasDynamicObjectPriority = asc->m_objectMetadataHasDynamicObjectPriority;
  ascStruct->oam_hasUniformSpread         = asc->m_objectMetadataHasUniformSpread;
  ascStruct->saocHeaderLength             = asc->m_saocHeaderLength;
  ascStruct->usacSamplingFreqIndx         = asc->m_usacConfig.m_usacSamplingFrequencyIndex;
  ascStruct->usacSamplingFreq             = asc->m_usacConfig.m_usacSamplingFrequency;
  ascStruct->outputFrameLength            = asc->m_usacConfig.m_outputFrameLength;
  ascStruct->sbrRatioIndex                = asc->m_usacConfig.m_sbrRatioIndex;
#ifndef WD1_REFERENCE_LAYOUT_3D
  ascStruct->channelConfigurationIndex    = asc->m_usacConfig.m_channelConfigurationIndex;
#else
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
  ascStruct->differsFromReferenceLayout   = asc->m_usacConfig.m_differsFromReferenceLayout;
#else
  ascStruct->differsFromReferenceLayout   = asc->m_usacConfig.m_usacFramework.m_signals3D.m_differsFromReferenceLayout[0];
#endif
  ascStruct->numSignalGroups              = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSignalGroups;
  
  for ( i = 0; i < ascStruct->numSignalGroups; ++i ) {
    ascStruct->signalGroupType[i] = asc->m_usacConfig.m_usacFramework.m_signals3D.m_signalGroupType[i];
  }

  /* Reference Layout */
  copy_SpeakerConfig3d_to_ascparser_Layout(&(ascStruct->referenceLayout), asc->m_usacConfig.m_referenceLayout);

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  /* SAOC Dmx Layout */
  if (asc->m_saocDmxLayoutPresent) {
    ascStruct->saocDmxLayoutPresent = asc->m_saocDmxLayoutPresent;
    copy_SpeakerConfig3d_to_ascparser_Layout(&(ascStruct->saocDmxChannelLayout), asc->m_saocDmxChannelLayout);
  }
#endif

  /* Audio Channel Layout */
  if ( ascStruct->differsFromReferenceLayout )
  {
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
    ascStruct->audioChannelLayout.speakerLayoutID      = asc->m_usacConfig.m_audioChannelLayout.speakerLayoutID;
#else
    ascStruct->audioChannelLayout.speakerLayoutID      = asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[0].speakerLayoutID;
#endif
    if ( ascStruct->audioChannelLayout.speakerLayoutID == 0 )
    {
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
      ascStruct->audioChannelLayout.CICPspeakerLayoutIdx = asc->m_usacConfig.m_audioChannelLayout.CICPspeakerLayoutIdx;
#else
      ascStruct->audioChannelLayout.CICPspeakerLayoutIdx = asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[0].CICPspeakerLayoutIdx;
#endif
      cicp2geometry_get_geometry_from_cicp( ascStruct->audioChannelLayout.CICPspeakerLayoutIdx, 
                                          ascStruct->audioChannelLayout.geometryInfo, &nChannels, &nLFE);

      ascStruct->audioChannelLayout.numLoudspeakers = nChannels + nLFE;
    }
    else if ( ascStruct->audioChannelLayout.speakerLayoutID == 1 )
    {
      int i = 0;
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
      ascStruct->audioChannelLayout.numLoudspeakers = asc->m_usacConfig.m_audioChannelLayout.numSpeakers;
#else
      ascStruct->audioChannelLayout.numLoudspeakers = asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[0].numSpeakers;
#endif

      for (i = 0; i < ascStruct->audioChannelLayout.numLoudspeakers; ++i) {
#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(asc->m_usacConfig.m_audioChannelLayout.CICPspeakerIdx[i], 
                                                             &(ascStruct->audioChannelLayout.geometryInfo[i])); 
#else
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(asc->m_usacConfig.m_usacFramework.m_signals3D.m_audioChannelLayout[0].CICPspeakerIdx[i], 
                                                             &(ascStruct->audioChannelLayout.geometryInfo[i])); 
#endif
      }
    }
    else if ( ascStruct->audioChannelLayout.speakerLayoutID == 2 ) {
      
    }
  }

#endif

  ascStruct->numConfigExtensions         = asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt;

  for (i = 0; i < ascStruct->numConfigExtensions; ++i) {
    ascStruct->configExtensionType[i] =(ASCPARSER_CONFIG_EXTENSION_TYPE)asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i];
  }
  
  ascStruct->compatibleProfileLevelSet = asc->m_compatibleProfileLevelSet;
  
  return 0;
}

INT32 ascparser_ASC_GetDownmixConfig( const ascparser_ASCPtr asc, ASCPARSER_DOWNMIX_CONFIG* downmixConfig, int* downmixConfigAvailable)
{
  unsigned int i = 0;
  for ( i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i)
  {
    if ( asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_DOWNMIX_CONFIG)
    {
      unsigned int j = 0;
      ASCPARSER_USAC_DOWNMIX_CONFIG* pDmxSet = (ASCPARSER_USAC_DOWNMIX_CONFIG*) asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i];

      downmixConfig->downmixConfigType = pDmxSet->downmixConfigType;
      if( downmixConfig->downmixConfigType == 0 || downmixConfig->downmixConfigType == 2 )
      {
        downmixConfig->passiveDownmixFlag = pDmxSet->passiveDownmixFlag;
        if( downmixConfig->passiveDownmixFlag == 0 )
        {
          downmixConfig->phaseAlignStrength= pDmxSet->phaseAlignStrength;
        }
        downmixConfig->immersiveDownmixFlag = pDmxSet->immersiveDownmixFlag;
      }

      downmixConfig->downmixIdCount = pDmxSet->downmixIdCount;

      if (  pDmxSet->downmixIdCount > ASCPARSER_MAX_DMX_MATRIX_ELEMENTS)
      {
        fprintf(stderr, "Too many downmix matrices in bitstream\n");
        return -1;
      }
      
      for ( j = 0; j < pDmxSet->downmixIdCount; ++j)
      {
        int spaceNeeded = 0;
        unsigned int l, m;
        if ( spaceNeeded > ASCPARSER_MAX_DMX_MATRIX_SIZE )
        {
          fprintf(stderr, "Downmix matrix %d is too large ( %d bytes)\n", j, spaceNeeded);
        }
        downmixConfig->downmixMatrix[j].DMX_ID                = pDmxSet->downmixMatrix[j].DMX_ID;
        downmixConfig->downmixMatrix[j].downmixType           = pDmxSet->downmixMatrix[j].downmixType;

        if( downmixConfig->downmixMatrix[j].downmixType == 0 )
        {
          downmixConfig->downmixMatrix[j].CICPspeakerLayoutIdx  = pDmxSet->downmixMatrix[j].CICPspeakerLayoutIdx;
        }
        else if ( downmixConfig->downmixMatrix[j].downmixType == 1 )
        {
          downmixConfig->downmixMatrix[j].CICPspeakerLayoutIdx  = pDmxSet->downmixMatrix[j].CICPspeakerLayoutIdx;
          downmixConfig->downmixMatrix[j].downmixMatrixCount    = pDmxSet->downmixMatrix[j].downmixMatrixCount;
          for( l = 0; l < downmixConfig->downmixMatrix[j].downmixMatrixCount; l++ )
          {
            downmixConfig->downmixMatrix[j].numAssignedGroupIDs[l] = pDmxSet->downmixMatrix[j].numAssignedGroupIDs[l];
            for( m = 0; m < downmixConfig->downmixMatrix[j].numAssignedGroupIDs[l]; m++ )
            {
              downmixConfig->downmixMatrix[j].signal_groupID[l][m] = pDmxSet->downmixMatrix[j].signal_groupID[l][m];
            }

            spaceNeeded = (pDmxSet->downmixMatrix[j].DmxMatrixLenBits[l] + 7) / 8;
            downmixConfig->downmixMatrix[j].DmxMatrixLenBits[l]      = pDmxSet->downmixMatrix[j].DmxMatrixLenBits[l];
            memcpy(downmixConfig->downmixMatrix[j].DownmixMatrix[l], pDmxSet->downmixMatrix[j].DownmixMatrix[l], spaceNeeded);
          }

        }
      }
      /* Do not search further for more downmix config extensions. Just return the first. */
      *downmixConfigAvailable = 1;
      return 0;
    }
  }
  *downmixConfigAvailable = 0;
  return 0;
}

INT32 ascparser_ASC_GetLoudnessInfoSet( const ascparser_ASCPtr asc, unsigned char* loudnessInfoSet, int* loudnessInfoAvailable, int* bitSize)
{
  unsigned int i = 0;
  *loudnessInfoAvailable = 0;

  for ( i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i)
  {
    if ( asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET)
    {
      memcpy(loudnessInfoSet, 
             asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i], 
             asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtLength[i] );
      *loudnessInfoAvailable = 1;
      *bitSize = asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtLength[i] * 8;
      break;
    }
  }
  
  return 0;
}

INT32 ascparser_ASC_CheckLoudnessInfoSetConfigExtension( const ascparser_ASCPtr asc, int* loudnessInfoAvailable)
{
  unsigned int i = 0;
  *loudnessInfoAvailable = 0;

  for ( i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i)
  {
    if ( asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET)
    {
      *loudnessInfoAvailable = 1;
      break;
    }
  }
  
  return 0;
}

INT32 ascparser_ASC_GetHoaConfig( const ascparser_ASCPtr asc, ASCPARSER_HOA_CONFIG* hoaConfig, int* hoaConfigAvailable)
{
  unsigned int i = 0;
  for ( i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i)
  {
    if ( asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_HOA_CONFIG)
    {
      unsigned int j = 0;
      ASCPARSER_USAC_HOA_CONFIG* pHoaSet = (ASCPARSER_USAC_HOA_CONFIG*) asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i];

      hoaConfig->numHoaMatrices = pHoaSet->numHoaMatrices;

      if (  pHoaSet->numHoaMatrices > ASCPARSER_MAX_HOA_MATRIX_ELEMENTS)
      {
        fprintf(stderr, "Too many HOA rendering matrices in bitstream\n");
        return -1;
      }
      
      for ( j = 0; j < pHoaSet->numHoaMatrices; ++j)
      {
        int spaceNeeded = (pHoaSet->hoaMatrix[j].HoaMatrixLenBits + 7) / 8;
        if ( spaceNeeded > ASCPARSER_MAX_HOA_MATRIX_SIZE )
        {
          fprintf(stderr, "HOA rendering matrix %d is too large ( %d bytes)\n", j, spaceNeeded);
        }

        hoaConfig->hoaMatrix[j].CICPspeakerLayoutIdx  = pHoaSet->hoaMatrix[j].CICPspeakerLayoutIdx;
        hoaConfig->hoaMatrix[j].HOA_ID                = pHoaSet->hoaMatrix[j].HOA_ID;
        hoaConfig->hoaMatrix[j].HoaMatrixLenBits      = pHoaSet->hoaMatrix[j].HoaMatrixLenBits;        

        memcpy(hoaConfig->hoaMatrix[j].HoaMatrix, pHoaSet->hoaMatrix[j].HoaMatrix, spaceNeeded);
      }
      /* Do not search further for more HOA config extensions. Just return the first. */
      *hoaConfigAvailable = 1;
      return 0;
    }
  }
  *hoaConfigAvailable = 0;
  return 0;
}

/* this function assumes an input string buffer long enough */
INT32 ascparser_ASC_Print(const ascparser_ASCPtr asc, char string[] )
{
  int pos = 0;
  INT32 err = 0;

  sprintf(&string[pos], "%16s %d\n", "AOT:", asc->m_aot);
  pos = strlen(string);

  if (asc->m_extensionAudioObjectType != 0) {
    sprintf(&string[pos], "%16s %d\n", "extAOT:", asc->m_extensionAudioObjectType);
    pos = strlen(string);
    sprintf(&string[pos], "%16s %d\n", "sbrPresent:", asc->m_sbrPresentFlag);
    pos = strlen(string);
    sprintf(&string[pos], "%16s %d\n", "psPresent:", asc->m_psPresentFlag);
    pos = strlen(string);
  }

  sprintf(&string[pos], "%16s %d\n", "fs:", asc->m_samplingFrequency);
  pos = strlen(string);

  if (asc->m_extensionSamplingFrequency != 0) {
    sprintf(&string[pos], "%16s %d\n", "extension_fs:", asc->m_extensionSamplingFrequency);
    pos = strlen(string);
  }

  sprintf(&string[pos], "%16s %d\n", "channelConfig:", asc->m_channelConfiguration);
  pos = strlen(string);

  sprintf(&string[pos], "%16s %d\n", "channels:", asc->m_channels);
  pos = strlen(string);

  switch (asc->m_aot) {
 
  case AOT_SAAS:
    sprintf(&string[pos], "%16s %d\n", "objects:", asc->m_objects);
    pos = strlen(string);

    sprintf(&string[pos], "%16s %d\n", "OAM mode:", asc->m_objectMetadataMode);
    pos = strlen(string);


    sprintf(&string[pos], "%16s %d\n", "OAM length:", asc->m_objectMetadataFrameLength);
    pos = strlen(string);



    sprintf(&string[pos], "%16s %d\n", "SAOC Header Length:", asc->m_saocHeaderLength);
    pos = strlen(string);

    err = ascparserUsacConfig_Print(&asc->m_usacConfig, &string[pos], asc->m_channelConfiguration, asc->m_aot);
    pos = strlen(string);
    break;

  default:
    sprintf(&string[pos], "%16s %d\n", "unknown AOT:", asc->m_aot);
    err = -1;
    break;
  }

  if (err != 0)
    return err;
  pos = strlen(string);


  return err;
}


ASCPARSER_AUDIO_OBJECT_TYPE ascparser_ASC_GetAotFromAsc(ascparser_ASCPtr asc)
{
  return asc->m_aot;
}


INT32 ascparser_ASC_GetLayerFromAsc(ascparser_ASCPtr asc, ascparser_ASCPtr asc_base)
{
  UINT8 layer = 0;


  return layer;
}


static ASCPARSER_AUDIO_OBJECT_TYPE __getAOT(ascparserBitStreamPtr bs)
{
  UINT32 dummyId = 0;
  INT32 tmp = __getBits(bs, dummyId, 5);
  ASCPARSER_AUDIO_OBJECT_TYPE aot_return = AOT_NULL_OBJECT;

  if (tmp == (INT32)AOT_ESCAPE) {
    INT32 tmp2 = __getBits( bs, dummyId, 6);
    tmp = 32 + tmp2;
  }

  aot_return = (ASCPARSER_AUDIO_OBJECT_TYPE)tmp;
  return aot_return;
}
