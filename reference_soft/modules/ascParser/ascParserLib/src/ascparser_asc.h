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


#ifndef __ASCPARSER_ASC_H__
#define __ASCPARSER_ASC_H__

#include "ascparser_machine.h"
#include "ascparser_usacconfig.h"
#include "ascparser.h"

struct ascparserBitStream;

typedef struct ascparser_ASC {

  ASCPARSER_AUDIO_OBJECT_TYPE m_aot;
  UINT32 m_samplingFrequencyIndex;
  UINT32 m_samplingFrequency;
  INT32  m_channelConfiguration;
  UINT32 m_samplesPerFrame;
  UINT32 m_epConfig;
  UINT32 m_directMapping;

  /* SBR/PS extension */
  INT32  m_sbrPresentFlag;
  INT32  m_psPresentFlag;
  ASCPARSER_AUDIO_OBJECT_TYPE m_extensionAudioObjectType;
  UINT32 m_extensionSamplingFrequencyIndex;
  UINT32 m_extensionSamplingFrequency;

  /* */
  UINT32 m_noOfStreams;
  UINT32 m_avgBitRate;
  UINT32 m_layer;

  /* derived values */
  INT32  m_channels;
  INT32  m_objects;
  INT32  m_SAOCTransportChannels;

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  UINT32           m_saocDmxLayoutPresent; 
  SpeakerConfig3d  m_saocDmxChannelLayout;
#endif

#ifdef SUPPORT_HOA
  INT32  m_HOATransportChannels;
#endif

  INT32  m_saocHeaderLength;
  INT32  m_objectMetadataMode;          /* 0: Default 1: Low Delay */
  INT32  m_objectMetadataFrameLength;   /* 1: Uses core coder frame length, 0: frame length explictly transmitted */
  INT32  m_objectMetadataHasScreenRelativeObjects;
  INT32  m_objectMetadataIsScreenRelativeObject[ASCPARSER_USAC_MAX_ELEMENTS]; 
  INT32  m_objectMetadataHasDynamicObjectPriority;
  INT32  m_objectMetadataHasUniformSpread;

  INT32 m_enhObjectHasDiffuseness;
  INT32 m_enhObjectHasCommonGroupDiffuseness;
  INT32 m_enhObjectHasExcludedSectors;
  INT32 m_enhObjectHasCommonGroupExcludedSectors;
  INT32 m_enhObjectUseOnlyPredefinedSectors[ASCPARSER_USAC_MAX_ELEMENTS];
  INT32 m_enhObjectHasClosestSpeakerCondition;
  float m_enhObjectClosestSpeakerThresholdAngle;
  INT32 m_enhObjectHasDivergence[ASCPARSER_USAC_MAX_ELEMENTS];
  float m_enhObjectDivergenceAzimuthRange[ASCPARSER_USAC_MAX_ELEMENTS];

  INT32  m_has_reference_distance;
  INT32  m_bs_reference_distance;
  INT32  m_numObjectGroups;
  INT32  m_numChannelGroups;
  INT32  m_has_object_distance[ASCPARSER_USAC_MAX_ELEMENTS];
  INT32  m_directHeadphone[ASCPARSER_USAC_MAX_ELEMENTS];

  ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET m_compatibleProfileLevelSet;

  ascparserUsacConfig m_usacConfig;

} ascparser_ASC, *ascparser_ASCPtr;


INT32 ascparser_ASC_ParseExt(
                                   ascparser_ASCPtr self, 
                                   ascparser_ASCPtr baselayer, 
                                   struct ascparserBitStream *bs, 
                                   UINT8 streamsPerLayer, 
                                   UINT32 avgBitrate,
                                   UINT8 latm_flag
                                   );

INT32 ascparser_ASC_Print(
                                const ascparser_ASCPtr asc,
                                char string[]
                                );

INT32 ascparser_ASC_FillStruct(const ascparser_ASCPtr asc, ASCPARSERINFO* ascStruct);
INT32 ascparser_ASI_FillStruct(const ascparser_ASCPtr asc, ASCPARSER_AUDIO_SCENE* audioScene, int asi_present, int* asi_valid);
INT32 ascparser_ASC_FillAudioGroupConfig(const ascparser_ASCPtr asc, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig);
int ASCPARSER_GetSignalGroupConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig);
INT32 ascparser_ASC_FillAudioGroupInfo(const ascparser_ASCPtr asc, ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, int* signalGroupInfoAvailable);
INT32 ascparser_ASC_FillCompatibleProfileLevelSet(const ascparser_ASCPtr asc, ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet, int* compatibleProfileLevelSetAvailable);
INT32 ascparser_ASC_FillEnhancedObjectMetadataConfig(const ascparser_ASCPtr asc, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig);
INT32 ascparser_ASC_FillProductionMetadataConfig(const ascparser_ASCPtr asc, ASCPARSER_PRODUCTION_METADATA_CONFIG* productionMetadataConfig);

ASCPARSER_AUDIO_OBJECT_TYPE ascparser_ASC_GetAotFromAsc(ascparser_ASCPtr asc);

INT32 ascparser_ASC_GetLayerFromAsc(
                        ascparser_ASCPtr asc, 
                        ascparser_ASCPtr asc_base
                        );

/**
   Returns the first downmixConfig found in the config extensions.
 */
INT32 ascparser_ASC_GetDownmixConfig( const ascparser_ASCPtr asc, ASCPARSER_DOWNMIX_CONFIG* downmixConfig, int* downmixConfigAvailable);
INT32 ascparser_ASC_GetLoudnessInfoSet ( const ascparser_ASCPtr asc, unsigned char* loudnessInfoSet, int* loudnessInfoAvailable, int* bitSize);
INT32 ascparser_ASC_CheckLoudnessInfoSetConfigExtension( const ascparser_ASCPtr asc, int* loudnessInfoAvailable);
INT32 ascparser_ASC_GetHoaConfig( const ascparser_ASCPtr asc, ASCPARSER_HOA_CONFIG* hoaConfig, int* hoaConfigAvailable);


#endif
