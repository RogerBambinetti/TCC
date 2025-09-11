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
#include <stdio.h>
#include <string.h>

#include "ascparser_parse.h"
#include "ascparser_asc.h"
#include "ascparser_bitbuf.h"

#if defined _WIN32 || defined _WINDOWS
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif


#define MAX_ASC_LEN 512

static int ascparser_ascParseInternal(
                                                                  const unsigned char* decSpecificInfoBuf[], 
                                                                  const unsigned int decSpecificInfoBuf_len[], 
                                                                  const unsigned int streamsPerLayer[], 
                                                                  const unsigned int avgBitrate[],
                                                                  const unsigned int layers, 
                                                                  struct ascparser_ASC* asc[]
                                                                  )
{
  UINT8 lay;
  int err = 0;

  /* chk handles */
  if (!decSpecificInfoBuf) return -1;
  for (lay=0; lay<layers; lay++) {
    if (!(decSpecificInfoBuf[lay])) return -1;
  }
  if (!decSpecificInfoBuf_len) return -1;
  if (!streamsPerLayer) return -1;
  if (!avgBitrate) return -1;
  if (!asc) return -1;
  for (lay=0; lay<layers; lay++) {
    if (!(asc[lay])) return -1;
  }


  if (layers > 1)
    return -1;

  for (lay = 0; lay < layers; lay ++) {
    ascparserBitBuffer bitbuf;
    UINT32 bitbuflen = decSpecificInfoBuf_len[lay];
    UINT32 tmp = bitbuflen;

    ascparserBitBuffer_Initialize(&bitbuf);
    ascparserBitBuffer_Feed(&bitbuf, decSpecificInfoBuf[lay], bitbuflen, &tmp);
    if (tmp != 0)
      return -1;

    err = ascparser_ASC_ParseExt(
                                       asc[lay],
                                       asc[0],
                                       &bitbuf.base,
                                       streamsPerLayer[lay],
                                       avgBitrate[lay],
                                       0 /*latm_flag*/
                                       );
    if (err != 0)
      break;

    /* CSBitbufferFree() not necessary */
    asc[lay]->m_layer = lay;
  }

  return err;
}


int ascparser_ascParse(
                                                   const unsigned char* decSpecificInfoBuf, 
                                                   const unsigned int decSpecificInfoBuf_len, 
                                                   struct ascparser_ASC* asc
                                                   )
{
  UINT32 tmp_spl[1] = {1};
  UINT32 tmp_avBR[1] = {0};

  /* chk handles */
  if (!decSpecificInfoBuf) return -1;
  if (!asc) return -1;

  /* minimal version: no scalable, no ep1, no tvq ... */
  return ascparser_ascParseInternal(
                                          &decSpecificInfoBuf,
                                          &decSpecificInfoBuf_len,
                                          tmp_spl,
                                          tmp_avBR,
                                          1,
                                          &asc
                                          );
}


 int ascparser_mpeghConfigParse( const unsigned char* decSpecificInfoBuf,
                                 const unsigned int decSpecificInfoBuf_len,
                                 struct ascparser_ASC* asc
                                )
{
  ascparserBitBuffer bitbuf;
  ascparserBitStreamPtr bs; 
  UINT32 bitbuflen = 0;
  UINT32 tmp = 0;


  if (!decSpecificInfoBuf) return -1;
  if (!decSpecificInfoBuf_len) return -1;
  if (!asc) return -1;

  bitbuflen = decSpecificInfoBuf_len;
  tmp = bitbuflen;

  ascparserBitBuffer_Initialize(&bitbuf);
  ascparserBitBuffer_Feed(&bitbuf, decSpecificInfoBuf, bitbuflen, &tmp);
  if (tmp != 0)
    return -1;

  bs = &(bitbuf.base);

  ascparserUsacConfig_Parse(&asc->m_usacConfig, bs);

  /* Patch some ASC values, this can later be removed if ASC is not used anymore */
  {
    asc->m_channels = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioChannels;
    asc->m_objects  = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioObjects;
    asc->m_SAOCTransportChannels = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSAOCTransportChannels;

#ifdef SUPPORT_SAOC_DMX_LAYOUT
    asc->m_saocDmxLayoutPresent = asc->m_usacConfig.m_usacFramework.m_signals3D.m_saocDmxLayoutPresent;
    asc->m_saocDmxChannelLayout = asc->m_usacConfig.m_usacFramework.m_signals3D.m_saocDmxChannelLayout;
#endif

    asc->m_HOATransportChannels = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numHOATransportChannels;

    asc->m_saocHeaderLength = ascparserUsacConfig_GetSaocHeaderLength(&asc->m_usacConfig);

    ascparserUsacConfig_GetObjectMetadataConfig(&asc->m_usacConfig,
                                                &asc->m_objectMetadataMode,
                                                &asc->m_objectMetadataFrameLength,
                                                &asc->m_objectMetadataHasScreenRelativeObjects,
                                                asc->m_objectMetadataIsScreenRelativeObject,
                                                &asc->m_objectMetadataHasDynamicObjectPriority,
                                                &asc->m_objectMetadataHasUniformSpread);

    ascparserUsacConfig_GetEnhancedObjectMetadataConfig(&asc->m_usacConfig,
                                                        &asc->m_enhObjectHasDiffuseness,
                                                        &asc->m_enhObjectHasCommonGroupDiffuseness,
                                                        &asc->m_enhObjectHasExcludedSectors,
                                                        &asc->m_enhObjectHasCommonGroupExcludedSectors,
                                                        &asc->m_enhObjectUseOnlyPredefinedSectors[0],
                                                        &asc->m_enhObjectHasClosestSpeakerCondition,
                                                        &asc->m_enhObjectClosestSpeakerThresholdAngle,
                                                        &asc->m_enhObjectHasDivergence[0],
                                                        &asc->m_enhObjectDivergenceAzimuthRange[0]);
  
    ascparserUsacConfig_GetProductionMetadataConfig(&asc->m_usacConfig,
                                                    &asc->m_has_reference_distance,
                                                    &asc->m_bs_reference_distance,
                                                    &asc->m_has_object_distance[0],
                                                    &asc->m_directHeadphone[0],
                                                    &asc->m_numObjectGroups,
                                                    &asc->m_numChannelGroups);

  }


  return 0;
}
