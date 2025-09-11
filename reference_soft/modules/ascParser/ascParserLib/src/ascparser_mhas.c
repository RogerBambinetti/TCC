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

#include <string.h>

#include "ascparser_mhas.h"
#include "ascparser_bitbuf.h"

#define MAGIC_MHAS_PREFIX "MHAS\0"

#define MHAS_SYNCWORD 0xA5

int MHAS_HEADER_SIZES[3][3] = {
  { 3, 8, 8}, /* MHASPacketType */
  { 2, 8,32}, /* MHASPacketLabel */
  {11,24,24}  /* MHASPacketLength */
};


typedef enum {
  MHAS_PACTYP_FILLDATA        = 0,
  MHAS_PACTYP_MPEGH3DACFG     = 1,
  MHAS_PACTYP_MPEGH3DAFRAME   = 2,
  MHAS_PACTYP_AUDIOSCENEINFO  = 3,
  MHAS_PACTYP_SYNC            = 6,
  MHAS_PACTYP_SYNCGAP         = 7,
  MHAS_PACTYP_MARKER          = 8,
  MHAS_PACTYP_CRC16           = 9,
  MHAS_PACTYP_CRC32           = 10,
  MHAS_PACTYP_DESCRIPTOR      = 11,
  MHAS_PACTYP_USERINTERACTION = 12,
  MHAS_PACTYP_LOUDNESS_DRC    = 13,
  MHAS_PACTYP_BUFFERINFO      = 14,
  MHAS_PACTYP_AUDIOTRUNCATION = 17,
  MHAS_PACTYP_EARCON          = 19,
  MHAS_PACTYP_PCMCONFIG       = 20,
  MHAS_PACTYP_PCMDATA         = 21,
  MHAS_PACTYP_LOUDNESS        = 22,
  MHAS_PACTYP_UNDEF           = 518
} mhas_packetType;


typedef struct {
  mhas_packetType PacType;
  UINT32 pacLabel;
  UINT32 pacLength;
} MHASPacketInfo;


UINT32 readEscapedValue(ascparserBitStreamPtr bs, int nBits[3]);


UINT32 readEscapedValue(ascparserBitStreamPtr bs, int nBits[3])
{
  const UINT32 dummyId = 0;

  UINT32 nBits1 =  nBits[0];
  UINT32 nBits2 =  nBits[1];
  UINT32 nBits3 =  nBits[2];

  UINT32 valueAdd = 0;
  UINT32 maxValue1 = (1 << nBits1) - 1;
  UINT32 maxValue2 = (1 << nBits2) - 1;
  UINT32 value = __getBits(bs, dummyId, nBits1);

  if (value == maxValue1) {
    valueAdd = __getBits(bs, dummyId, nBits2);
    value += valueAdd;

    if (valueAdd == maxValue2) {
      valueAdd = __getBits(bs, dummyId, nBits3);
      value += valueAdd;
    }
  }

  return value;
}

static int readMagicFilePrefix(ascparserBitStreamPtr bs)
{
  UINT32 dummyId = 0;
  unsigned int  i = 0;
  char mp4Prefix[5] = {'\0'};
  char mhasPrefix[5] = {'\0'};
 
  for (i = 0; i < 5; ++i) {
    /* including \n */
    mp4Prefix[i] = __getBits(bs, dummyId, 8);
  }

  if (strncmp(mp4Prefix, ".mp4\n",5 ) != 0 ) {
    /* including \n */
    return -1;
  }

  for (i = 0; i < 5; ++i) {
    mhasPrefix[i] = __getBits(bs, dummyId, 8);
  }

  /* MHAS file prefix */
  if (strncmp(mhasPrefix, MAGIC_MHAS_PREFIX, 5) != 0) {
    return -1;
  }

  return 0;
}

static int mhasReadLoudnessInfoSet(ascparserBitStreamPtr bs, ascparserUsacConfigExtensionPtr pConfigExt, UINT32 length)
{
  int error = 0;
  int loudnessConfigExtPresent = 0;
  unsigned int i = 0;
  unsigned int k = 0;
  int numConfExt = 0;
  int dummyId = 0;

  for ( i = 0; i < pConfigExt->m_usacConfigExtNumConfExt; ++i)
  {
    if ( pConfigExt->m_usacConfigExtType[i] == ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET)
    {
      for ( k = 0; k < length; k++ ) {
        pConfigExt->m_usacConfigExtPayload[i][k] = (unsigned char)__getBits(bs, dummyId, 8);
      }
      pConfigExt->m_usacConfigExtLength[i] = length;
      loudnessConfigExtPresent = 1;
      break;
    }
  }
  if (loudnessConfigExtPresent == 0) {
    numConfExt = pConfigExt->m_usacConfigExtNumConfExt;
    pConfigExt->m_usacConfigExtType[numConfExt] = ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET;
    for ( k = 0; k < length; k++ ) {
      pConfigExt->m_usacConfigExtPayload[i][k] = (unsigned char)__getBits(bs, dummyId, 8);
    }
    pConfigExt->m_usacConfigExtLength[i] = length;
    pConfigExt->m_usacConfigExtNumConfExt++;
  }

  return error;
}

static int mhasReadPacketHeader(ascparserBitStreamPtr bs, MHASPacketInfo *pacInfo)
{
  if ( __getValidBits(bs) < 16)
    return -2;

  pacInfo->PacType   = (mhas_packetType)readEscapedValue(bs, MHAS_HEADER_SIZES[0] );
  pacInfo->pacLabel  = readEscapedValue(bs, MHAS_HEADER_SIZES[1] );
  pacInfo->pacLength = readEscapedValue(bs, MHAS_HEADER_SIZES[2] );

  return 0;
}


static int mhasReadPacket(ascparserBitStreamPtr bs, MHASPacketInfo pacInfo, ascparser_ASC* asc)
{
  UINT32 k = 0;
  UINT32 dummy = 0;
  UINT32 dummyId = 0;

  switch (pacInfo.PacType) 
  {
  case MHAS_PACTYP_MPEGH3DACFG:
    ascparserUsacConfig_Parse(&asc->m_usacConfig, bs);

    /* Patch some ASC values, this can later be removed if ASC is not used anymore */
    {
      asc->m_channels = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioChannels;
      asc->m_objects  = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numAudioObjects;
      asc->m_SAOCTransportChannels = asc->m_usacConfig.m_usacFramework.m_signals3D.m_numSAOCTransportChannels;
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
    break;

  case MHAS_PACTYP_MPEGH3DAFRAME:  /* Read away AU */
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy  = __getBits(bs, dummyId, 8);

    break;

  case MHAS_PACTYP_AUDIOSCENEINFO:  /* Read ASI */
    if ( pacInfo.pacLength != 0 ) {
      Read_AudioSceneInfo(bs, &asc->m_usacConfig.m_usacConfigExtension.m_audioSceneInfo);
    }

    break;

  case MHAS_PACTYP_SYNC:
    if ( pacInfo.pacLength !=1 ) return -1;
    
    dummy  = __getBits(bs, dummyId, 8);
    if ( dummy != MHAS_SYNCWORD) return -1;

    break;

  case MHAS_PACTYP_LOUDNESS:  /* Read LoudnessInfoSet */
    if ( pacInfo.pacLength != 0 ) {
      mhasReadLoudnessInfoSet(bs, &asc->m_usacConfig.m_usacConfigExtension, pacInfo.pacLength);
    }
    break;

  /* default behavior for not implemented pactypes */
  case MHAS_PACTYP_FILLDATA:
    fprintf(stderr, "Skipping MHAS_PACTYP_FILLDATA \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_SYNCGAP:
    fprintf(stderr, "Skipping MHAS_PACTYP_SYNCGAP \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_MARKER:
    fprintf(stderr, "Skipping MHAS_PACTYP_MARKER \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_CRC16:
    fprintf(stderr, "Skipping MHAS_PACTYP_CRC16 \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_CRC32:
    fprintf(stderr, "Skipping MHAS_PACTYP_CRC32 \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_DESCRIPTOR:
    fprintf(stderr, "Skipping MHAS_PACTYP_DESCRIPTOR \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_USERINTERACTION:
    fprintf(stderr, "Skipping MHAS_PACTYP_USERINTERACTION \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_LOUDNESS_DRC:
    fprintf(stderr, "Skipping MHAS_PACTYP_LOUDNESS_DRC \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_BUFFERINFO:
    fprintf(stderr, "Skipping MHAS_PACTYP_BUFFERINFO \n");
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_EARCON:
    /*fprintf(stderr, "Skipping MHAS_PACTYP_EARCON \n");*/
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_PCMCONFIG:
    /*fprintf(stderr, "Skipping MHAS_PACTYP_EARCON \n");*/
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  case MHAS_PACTYP_PCMDATA:
    /*fprintf(stderr, "Skipping MHAS_PACTYP_EARCON \n");*/
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  default: /* Read away Payload */
    fprintf(stderr, "Skipping payload. MHAS_PACTYPE %i not recognized. \n", pacInfo.PacType);
    for ( k = 0; k < pacInfo.pacLength; k++ )
      dummy = __getBits(bs, dummyId, 8);
    break;
  }

  return 0;
}

int ascparser_mhas_searchSync(unsigned char* buf, unsigned int len, const int siw_mhas_type)
{
  int err = 0;
  ascparserBitBuffer bitbuf;
  ascparserBitStreamPtr bs; 
  UINT32 bitbuflen = len;
  UINT32 tmp = bitbuflen;

  MHASPacketInfo tmpPacInfo;

  ascparserBitBuffer_Initialize(&bitbuf);
  ascparserBitBuffer_Feed(&bitbuf, buf, bitbuflen, &tmp);

  if (tmp != 0) {
    return -1;
  }
  
  bs = &(bitbuf.base);

  if (1 == siw_mhas_type) {
    err = readMagicFilePrefix(bs);
    if (err) {
      return err;
    }
  }

  do {
    err = mhasReadPacketHeader(bs, &tmpPacInfo);

    if (err) {
      return err;
    }
  } while ( tmpPacInfo.PacType != MHAS_PACTYP_SYNC );

  return 0;
}

/* read config and asi from mhas stream */
int ascparser_mhas_getConfig(unsigned char* buf, unsigned int len, ascparser_ASC* p_asc, const int siw_mhas_type)
{
  int err = 0;
  ascparserBitBuffer bitbuf;
  ascparserBitStreamPtr bs; 
  UINT32 bitbuflen = len;
  UINT32 tmp = bitbuflen;

  MHASPacketInfo tmpPacInfo;

  ascparserBitBuffer_Initialize(&bitbuf);
  ascparserBitBuffer_Feed(&bitbuf, buf, bitbuflen, &tmp);

  if (tmp != 0) {
    return -1;
  }

  bs  = &(bitbuf.base);

  if (1 == siw_mhas_type) {
    err = readMagicFilePrefix(bs);

    if (err) {
      return err;
    }
  }

  /* Search for Config Packet */
  do {
    err = mhasReadPacketHeader(bs, &tmpPacInfo); 

    if (err) {
      return err;
    }

    err = mhasReadPacket(bs, tmpPacInfo, p_asc);
  } while (tmpPacInfo.PacType != MHAS_PACTYP_MPEGH3DACFG);

  return 0;
}

int ascparser_mhas_getAudioSceneInfo(unsigned char* buf, unsigned int len, ascparser_ASC* p_asc, const int siw_mhas_type, int* asi_mhas_present)
{
  INT32 err = 0;
  ascparserBitBuffer bitbuf;
  ascparserBitStreamPtr bs; 
  UINT32 bitbuflen = len;
  UINT32 tmp = bitbuflen;
  UINT32 dummy = 0;
  UINT32 dummyId = 0;
  UINT32 k = 0;

  MHASPacketInfo tmpPacInfo;

  if (NULL != asi_mhas_present) {
    *asi_mhas_present = 0;
  }

  ascparserBitBuffer_Initialize(&bitbuf);
  ascparserBitBuffer_Feed(&bitbuf, buf, bitbuflen, &tmp);

  if (tmp != 0) {
    return -1;
  }

  bs = &(bitbuf.base);

  if (1 == siw_mhas_type) {
    err = readMagicFilePrefix(bs);

    if (err) {
      return err;
    }
  }

  /* Search for AudioScene Packet */
  do {
    err = mhasReadPacketHeader(bs, &tmpPacInfo); 

    if (err) {
      return err;
    }
    if (tmpPacInfo.PacType == MHAS_PACTYP_AUDIOSCENEINFO) {
      err = mhasReadPacket(bs, tmpPacInfo, p_asc);
    } else {
      for ( k = 0; k < tmpPacInfo.pacLength; k++ ){
        dummy = __getBits(bs, dummyId, 8);
      }
    }
    if (tmpPacInfo.PacType == MHAS_PACTYP_AUDIOSCENEINFO && NULL != asi_mhas_present) {
      *asi_mhas_present = 1;
    }
  } while ((tmpPacInfo.PacType != MHAS_PACTYP_MPEGH3DAFRAME) && (tmpPacInfo.PacType != MHAS_PACTYP_AUDIOSCENEINFO));

  return 0;
}

int ascparser_mhas_getLoudnessInfoSet(unsigned char* buf, unsigned int len, ascparser_ASC* p_asc, const int siw_mhas_type, int* loudness_mhas_present)
{
  INT32 err = 0;
  ascparserBitBuffer bitbuf;
  ascparserBitStreamPtr bs; 
  UINT32 bitbuflen = len;
  UINT32 tmp = bitbuflen;
  UINT32 dummy = 0;
  UINT32 dummyId = 0;
  UINT32 k = 0;

  MHASPacketInfo tmpPacInfo;

  if (NULL != loudness_mhas_present) {
    *loudness_mhas_present = 0;
  }

  ascparserBitBuffer_Initialize(&bitbuf);
  ascparserBitBuffer_Feed(&bitbuf, buf, bitbuflen, &tmp);

  if (tmp != 0) {
    return -1;
  }

  bs = &(bitbuf.base);

  if (1 == siw_mhas_type) {
    err = readMagicFilePrefix(bs);

    if (err) {
      return err;
    }
  }

  /* Search for Loudness Info Set Packet */
  do {
    err = mhasReadPacketHeader(bs, &tmpPacInfo); 

    if (err) {
      return err;
    }
    if (tmpPacInfo.PacType == MHAS_PACTYP_LOUDNESS) {
      err = mhasReadPacket(bs, tmpPacInfo, p_asc);
    } else {
      for ( k = 0; k < tmpPacInfo.pacLength; k++ ){
        dummy = __getBits(bs, dummyId, 8);
      }
    }
    if (tmpPacInfo.PacType == MHAS_PACTYP_LOUDNESS && NULL != loudness_mhas_present) {
      *loudness_mhas_present = 1;
    }
  } while ((tmpPacInfo.PacType != MHAS_PACTYP_MPEGH3DAFRAME) && (tmpPacInfo.PacType != MHAS_PACTYP_LOUDNESS));

  return 0;
}
