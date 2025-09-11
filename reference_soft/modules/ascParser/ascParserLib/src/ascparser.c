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


#include <stdlib.h>
#include <string.h>

#include "ascparser.h"
#include "ascparser_stream_wrap.h"
#include "ascparser_asc.h"

#define ASCPARSER_STRING_LEN        (1024)

typedef struct _ASCPARSER
{
  ASCPARSER_BITSREAM*    fBS;
  struct ascparser_ASC * p_asc;


} ASCPARSER;



int ASCPARSER_Init(HANDLE_ASCPARSER* phAscParser, const char* filename)
{
  HANDLE_ASCPARSER tmp;
  tmp = (HANDLE_ASCPARSER) calloc(1, sizeof(ASCPARSER));

  tmp->p_asc = (struct ascparser_ASC *) calloc(sizeof(ascparser_ASC), 1);
  tmp->fBS = ascparserOpenInputStream( filename );

  *phAscParser = tmp;

  if ( tmp->fBS == NULL ) {
    printf("error: opening the input file\n");
    return -1;
  }
  else {
    return 0;
  }
}

int ASCPARSER_GetASC(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc)
{
  asc->asc_isValid = 0;

  if (ascparserReadConfig(hAscParser->fBS, hAscParser->p_asc)) {
    printf("error: reading config (ASC) from input\n");

    return SIW_COULDNOTREAD_CFG;
  }

  ascparser_ASC_FillStruct(hAscParser->p_asc, asc);

  asc->asc_isValid = 1;

  return SIW_SUCCESS;
}

int ASCPARSER_GetDecoderConfig(HANDLE_ASCPARSER hAscParser, ascparserUsacDecoderConfig *usacDecoderConfig)
{
  if (&(hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig) == NULL){
    return SIW_INVALID_HANDLE;
  }
  memcpy(usacDecoderConfig, &(hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig), sizeof(ascparserUsacDecoderConfig));
  if (usacDecoderConfig == NULL){
    return SIW_INVALIDDATA;
  }
  return SIW_SUCCESS;
}

int  ASCPARSER_GetMaxStereoConfigIndex(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc)
{
  unsigned int i;
  int tmpStereoConfigIndex = -1;

  for (i = 0; i < hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig.m_numElements; ++i) {
    if (hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig.m_usacElementType[i] == ASCPARSER_USAC_ELEMENT_TYPE_CPE) {
      if (tmpStereoConfigIndex < (int)hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig.m_usacElementConfig->m_usacCpeConfig.m_stereoConfigIndex) {
        tmpStereoConfigIndex = (int)hAscParser->p_asc->m_usacConfig.m_usacDecoderConfig.m_usacElementConfig->m_usacCpeConfig.m_stereoConfigIndex;
      }
    }
  }

  asc->maxStereoConfigIndex = tmpStereoConfigIndex;
  return 0;
}

int ASCPARSER_GetASI(HANDLE_ASCPARSER hAscParser, ASCPARSERINFO* asc, ASCPARSER_AUDIO_SCENE* audioScene, int* asi_valid)
{
  int error = 0;
  int asi_present = 0;
  int asi_mhas_present = 0;
  int asi_config_present = 0;

  if (NULL != asi_valid) {
    *asi_valid = 0;
  }

  /* check if ASI is present as ConfigExtension */
  error = ascParserCheckAudioSceneConfigExtension(hAscParser->p_asc, &asi_config_present);
  if (SIW_SUCCESS != error) {
    return SIW_COULDNOTREAD_CFG;
  }

  /* needed if AUDIOSCENE as MHAS package comes after CONFIG MHAS package*/
  error = ascParserReadAudioScenePacket(hAscParser->fBS, hAscParser->p_asc, &asi_mhas_present);
  if (SIW_SUCCESS != error) {
    return SIW_COULDNOTREAD_ASI;
  }

  /* check if ASI occurs only once */
  if (1 == asi_config_present && 1 == asi_mhas_present) {
    return SIW_ASIEXISTSTWICE;
  }
  if (1 == asi_config_present || 1 == asi_mhas_present) {
    asi_present = 1;
  }

  error = ascparser_ASI_FillStruct(hAscParser->p_asc, audioScene, asi_present, asi_valid);
  if (SIW_SUCCESS != error) {
    return SIW_COULDNOTREAD_ASI;
  }

  return SIW_SUCCESS;
}

int ASCPARSER_GetSignalGroupConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_SIGNAL_GROUP_CONFIG * signalGroupConfig)
{
  int error = 0;
  error = ascparser_ASC_FillAudioGroupConfig(hAscParser->p_asc, signalGroupConfig);
  return 0;
}

int ASCPARSER_GetSignalGroupInfo(HANDLE_ASCPARSER hAscParser, ASCPARSER_SIGNAL_GROUP_INFO * signalGroupInfo, int* signalGroupInfoAvailable)
{
  int error = 0;
  int temp = 0;
  error = ascparser_ASC_FillAudioGroupInfo(hAscParser->p_asc, signalGroupInfo, &temp);
  *signalGroupInfoAvailable = temp;
  return 0;
}

int ASCPARSER_GetCompatibleProfileLevelSet(HANDLE_ASCPARSER hAscParser, ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET * compatibleProfileLevelSet, int* compatibleProfileLevelSetAvailable)
{
  int error = 0;
  error = ascparser_ASC_FillCompatibleProfileLevelSet(hAscParser->p_asc, compatibleProfileLevelSet, compatibleProfileLevelSetAvailable);
  return 0;
}

int ASCPARSER_GetEnhancedObjectConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig)
{
  int error = 0;
  error = ascparser_ASC_FillEnhancedObjectMetadataConfig(hAscParser->p_asc, enhancedObjectMetadataConfig);
  return 0;
}

int ASCPARSER_GetProductionMetadataConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_PRODUCTION_METADATA_CONFIG* productionMetadataConfig)
{
  int error = 0;
  error = ascparser_ASC_FillProductionMetadataConfig(hAscParser->p_asc, productionMetadataConfig);
  return 0;
}

int  ASCPARSER_GetDownmixConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_DOWNMIX_CONFIG* downmixConfig, int* downmixConfigAvailable)
{
  return (ascparser_ASC_GetDownmixConfig( hAscParser->p_asc, downmixConfig, downmixConfigAvailable) );
}

int ASCPARSER_GetLoudnessInfoSet(HANDLE_ASCPARSER hAscParser, unsigned char* loudnessInfoSet, int* loudnessPresent, int* bitSize)
{
  int error = 0;
  int loudnessMhasPresent = 0;
  int loudnessConfigPresent = 0;
  unsigned int i = 0;

  /* check if LOUDNESS INFO SET is present as ConfigExtension */
  error = ascparser_ASC_CheckLoudnessInfoSetConfigExtension( hAscParser->p_asc, &loudnessConfigPresent);
  if (SIW_SUCCESS != error) {
    return SIW_COULDNOTREAD_CFG;
  }

  /* check if LOUDNESS INFO SET is present as MHAS package */
  error = ascParserReadLoudnessPacket(hAscParser->fBS, hAscParser->p_asc, &loudnessMhasPresent);
  if (SIW_SUCCESS != error) {
    return SIW_COULDNOTREAD_LOUDNESS;
  }
  if (loudnessMhasPresent == 1) {
    printf("MHAS Packet of type PACTYP_LOUDNESS is present and takes precedence over potential loudness payload in config extension.\n");
  }

  for ( i = 0; i < hAscParser->p_asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i)
  {
    if ( hAscParser->p_asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET)
    {
      memcpy(loudnessInfoSet, 
           hAscParser->p_asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtPayload[i], 
           hAscParser->p_asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtLength[i] );
      *bitSize = hAscParser->p_asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtLength[i]<<3;
      break;
    }
  }

  if (1 == loudnessConfigPresent || 1 == loudnessMhasPresent) {
    *loudnessPresent = 1;
  }

  return SIW_SUCCESS;
}

int  ASCPARSER_GetHoaConfig(HANDLE_ASCPARSER hAscParser, ASCPARSER_HOA_CONFIG* hoaConfig, int* hoaConfigAvailable)
{
  return (ascparser_ASC_GetHoaConfig( hAscParser->p_asc, hoaConfig, hoaConfigAvailable) );
}

void ASCPARSER_Close(HANDLE_ASCPARSER hAscParser)
{
  if (hAscParser)
  {
    if (hAscParser->fBS)
      ascparserCloseInputStream(hAscParser->fBS);
    if (hAscParser->p_asc)
      free(hAscParser->p_asc);

    free(hAscParser);
  }
}
