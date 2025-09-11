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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cicp2geometry.h"
#include "ascparser_usacconfig.h"
#include "ascparser_bitstream.h"
#include "ascparser_metadata.h"

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

typedef struct oamByteParser {
  int            bitsRead;   /* bits already read of byte */
  unsigned char  byte;       /* byte to read */
} OAM_BYTE_PARSER, * OAM_BYTE_PARSER_HANDLE;


static int _oamBitwiseByteParser(OAM_BYTE_PARSER_HANDLE hOamByteParser, int* value, int numberOfBitsToRead) {

  int bitCnt     = 0;
  int valueIndex = 0;
  int bitmask    = 0x0;

  if ( hOamByteParser->bitsRead >= 8                     ||
       hOamByteParser->bitsRead + numberOfBitsToRead > 8 ) {
    return 2;
  }

  for ( bitCnt = hOamByteParser->bitsRead; bitCnt < hOamByteParser->bitsRead + numberOfBitsToRead; ++bitCnt ) {

    /* get the right bitmask by shifting according to already read bits*/
    bitmask = 1 << (7-bitCnt);

    value[valueIndex++] = ( (hOamByteParser->byte&bitmask)  >> (7-bitCnt) );
  
  }

  hOamByteParser->bitsRead += numberOfBitsToRead;
  
  if ( hOamByteParser->bitsRead >= 8 ) {
    /* need new byte */
    return 1;
  }

  return 0;
}

static int _oamBitclusterByteParser(OAM_BYTE_PARSER_HANDLE hOamByteParser, unsigned char* bitCluster, int numberOfBitsToRead) {

  unsigned char ones     = 0xff;
  unsigned char bitmask  = 0x0;
  int bitsLeftInByte     = 8 - ( hOamByteParser->bitsRead + numberOfBitsToRead );

  if ( bitsLeftInByte < 0 ) {
    /* too many bits required */
    return 2;
  }

  /* shift ones to get the the desired number */
  ones = ones >> (8 - numberOfBitsToRead);

  /* shift to get the right bitmask */
  bitmask = ones << bitsLeftInByte;

  /* shift again to remove bit-offset of none read bits */
  *bitCluster = ( hOamByteParser->byte & bitmask ) >> bitsLeftInByte;

  hOamByteParser->bitsRead += numberOfBitsToRead;

  if ( hOamByteParser->bitsRead >= 8 ) {
    /* need new byte */
    return 1;
  }

  return 0;
}

static int _oamFeedByteParser(OAM_BYTE_PARSER_HANDLE hOamByteParser, unsigned char newByte) {

  if ( hOamByteParser == NULL ) {
    return 1;
  }

  hOamByteParser->bitsRead = 0;



  memcpy( &(hOamByteParser->byte), &(newByte), sizeof(unsigned char));

  return 0;

}

static const INT32 UsacSamplingFrequencyTable[32] =
  {
    96000,                 /* 0x00 */
    88200,                 /* 0x01 */
    64000,                 /* 0x02 */
    48000,                 /* 0x03 */
    44100,                 /* 0x04 */
    32000,                 /* 0x05 */
    24000,                 /* 0x06 */
    22050,                 /* 0x07 */
    16000,                 /* 0x08 */
    12000,                 /* 0x09 */
    11025,                 /* 0x0a */
    8000,                  /* 0x0b */
    7350,                  /* 0x0c */
    -1 /* reserved */,     /* 0x0d */
    -1 /* reserved */,     /* 0x0e */
    57600,                 /* 0x0f */
    51200,                 /* 0x10 */
    40000,                 /* 0x11 */
    38400,                 /* 0x12 */
    34150,                 /* 0x13 */
    28800,                 /* 0x14 */
    25600,                 /* 0x15 */
    20000,                 /* 0x16 */
    19200,                 /* 0x17 */
    17075,                 /* 0x18 */
    14400,                 /* 0x19 */
    12800,                 /* 0x1a */
    9600,                  /* 0x1b */
    -1 /* reserved */,     /* 0x1c */
    -1 /* reserved */,     /* 0x1d */
    -1 /* reserved */,     /* 0x1e */
     0 /* escape value */, /* 0x1f */
  };


#ifdef WD1_REFERENCE_LAYOUT_3D
static int __ascparserUsacConfig_SpeakerConfig3d(SpeakerConfig3d * pSpeakerConfig3d, ascparserBitStreamPtr bs);
static int __ascparserUsacConfig_Usac3dSpeakerDescription( Usac3dSpeakerDescription* speakerDescription, int angularPrecision, ascparserBitStreamPtr bs);
static int __ascparserUsacConfig_Usac3dFlexibleSpeakerConfig(Usac3dFlexibleSpeakerConfig * pFlexibleSpeakerConfig3d, int numSpeakers, ascparserBitStreamPtr bs);
#endif /* WD1_REFERENCE_LAYOUT_3D */


static void InitMemberVariables (ascparserUsacConfigPtr self)
{
  self->m_usacSamplingFrequency      = 0;
  self->m_usacSamplingFrequencyIndex = 0;
  self->m_outputFrameLength          = 0;
  self->m_sbrRatioIndex              = 0;
#ifndef WD1_REFERENCE_LAYOUT_3D
  self->m_channelConfigurationIndex  = 0;
#endif
}

#ifdef WD1_ELEMENT_MAPPING_3D
static int ascparserUsacConfig_findFirstEmptyIndex(UINT32 *chMap, int len)
{
  int i, ret = 0;
  
  /* Find next 'empty (-1)' channel index */
  for(i=0; i<len; i++) {
    if(((INT32)chMap[i]) < 0) break;
    ret++;
  }
  
  return ret;
}


static int ascparserUsacConfig_getChannelIndex(UINT32 *chMap, UINT32 nextChannel, UINT32 shift, UINT32 nChannels)
{
  UINT32  i;
  UINT32 channelMap[ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS];

  memset(channelMap, -1, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS*sizeof(int));

  for(i=0; i<nChannels; i++) {
    if((INT32)chMap[i] < 0) break;
    channelMap[chMap[i]] = chMap[i];
  }
  
  for(i=nextChannel; i<=nextChannel+shift+1; i++) {
    if((INT32)channelMap[i] > 0) {
      shift++;
    }
  }
  return nextChannel + shift + 1;
}

static int ascparserUsacConfig_getNumberOfSkippedChannels(UINT32 *chMap, UINT32 nextChannel, UINT32 nextEmptyChIdx, UINT32 nChannels)
{
  UINT32 i;
  UINT32 ch = 0;
  
  for(i=0; i<nextEmptyChIdx; i++) {
    assert((INT32)chMap[i] != nextChannel);
    if((INT32)chMap[i] > nextChannel) {
      ch++;
    }
  }
  
  return ch;
}


static int ascparserUsacConfig_findNextChannel(UINT32 *chMap, int len)
{
  int i,j;
  UINT32 ch = 0;

  for(i=0; i<len; i++) {
    for(j=0;j<len;j++) {
      if(chMap[j] == ch) {
        ch++;
        break;
      }
    }
    if(chMap[i]==-1) {
      break;
    }
  }
  
  return ch; /* Return next channel */
}
#endif

static INT32 __GetUsacSamplingFrequencyFromIndex(UINT32 sfIndex){

  INT32 samplingFrequency = -1;

  if(sfIndex < sizeof(UsacSamplingFrequencyTable)/sizeof(UsacSamplingFrequencyTable[0]) ){
    samplingFrequency = UsacSamplingFrequencyTable[sfIndex];
  }

  assert(samplingFrequency >= 0);

  return samplingFrequency;
}


static INT32 __GetSbrRatioIndex(UINT32 coreSbrFrameLengthIndex){

  INT32 sbrRatioIndex = -1;

  switch(coreSbrFrameLengthIndex){
  case 0:
  case 1:
    sbrRatioIndex = 0;
    break;
  case 2:
    sbrRatioIndex = 2;
    break;
  case 3:
    sbrRatioIndex = 3;
    break;
  case 4:
    sbrRatioIndex = 1;
    break;
  default:
    break;
  }  

  assert(sbrRatioIndex > -1);

  return sbrRatioIndex;
}

static INT32 __GetOutputFrameLength(UINT32 coreSbrFrameLengthIndex){

  INT32 outputFrameLength = -1;

  switch(coreSbrFrameLengthIndex){
  case 0:
    outputFrameLength = 768;
    break;
  case 1:
    outputFrameLength = 1024;
    break;
  case 2:
  case 3:
    outputFrameLength = 2048;
    break;
  case 4:
    outputFrameLength = 4096;
    break;
  default:
    break;
  }  

  assert(outputFrameLength > -1);

  return outputFrameLength;
}



static void __ParseUsacCoreConfig(ascparserUsacCoreConfigPtr self, ascparserBitStreamPtr bs){
  const UINT32 dummyId = 0;

  self->m_tw_mdct                 = __getBits(bs, dummyId, 1);
  self->m_fullbandLPD             = __getBits(bs, dummyId, 1);
  self->m_noiseFilling            = __getBits(bs, dummyId, 1);
  self->m_enhancedNoiseFilling    = __getBits(bs, dummyId, 1);
  if(self->m_enhancedNoiseFilling) {
    self->m_igfUseEnf             = __getBits(bs, dummyId, 1);
    self->m_igfUseHighRes         = __getBits(bs, dummyId, 1);
    self->m_igfUseWhitening       = __getBits(bs, dummyId, 1);
    self->m_igfAfterTnsSynth      = __getBits(bs, dummyId, 1);
    self->m_igfStartIndex         = __getBits(bs, dummyId, 5);
    self->m_igfStopIndex          = __getBits(bs, dummyId, 4);
  }
  return;
}

static void __ParseUsacSbrHeader(ascparserUsacSbrHeaderPtr self, ascparserBitStreamPtr bs){
  const UINT32 dummyId = 0;

  self->m_start_freq    = __getBits(bs, dummyId, 4);
  self->m_stop_freq     = __getBits(bs, dummyId, 4);
  self->m_header_extra1 = __getBits(bs, dummyId, 1);
  self->m_header_extra2 = __getBits(bs, dummyId, 1);

  if(self->m_header_extra1){
    self->m_freq_scale  = __getBits(bs, dummyId, 2);
    self->m_alter_scale = __getBits(bs, dummyId, 1);
    self->m_noise_bands = __getBits(bs, dummyId, 2);
  }

  if(self->m_header_extra2){
    self->m_limiter_bands  = __getBits(bs, dummyId, 2);
    self->m_limiter_gains  = __getBits(bs, dummyId, 2);
    self->m_interpol_freq  = __getBits(bs, dummyId, 1);
    self->m_smoothing_mode = __getBits(bs, dummyId, 1);
  }

  return;
}

static void __ParseUsacSbrConfig(ascparserUsacSbrConfigPtr self, ascparserBitStreamPtr bs){
  const UINT32 dummyId = 0;

  self->m_harmonicSBR = __getBits(bs, dummyId, 1);
  self->m_bs_interTes = __getBits(bs, dummyId, 1);
  self->m_bs_pvc      = __getBits(bs, dummyId, 1);
  
  __ParseUsacSbrHeader(&(self->m_sbrDfltHeader), bs);

  return;
}

static void __ParseUsacSceConfig(ascparserUsacSceConfigPtr self, ascparserBitStreamPtr bs, UINT32 *outputChannelIndex, int sbrRatioIndex){
#ifdef WD1_ELEMENT_MAPPING_3D
  int nextEmptyChIdx;
  int nextChannel;
#endif

  __ParseUsacCoreConfig(&(self->m_usacCoreConfig), bs);

  if(sbrRatioIndex > 0){
    __ParseUsacSbrConfig(&(self->m_usacSbrConfig), bs);
  }

#ifdef WD1_ELEMENT_MAPPING_3D
  nextChannel = ascparserUsacConfig_findNextChannel(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  nextEmptyChIdx = ascparserUsacConfig_findFirstEmptyIndex(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  outputChannelIndex[nextEmptyChIdx] = nextChannel;
#endif

  return;
}

static void __ParseUsacMps212Config(ascparserUsacMps212ConfigPtr self, ascparserBitStreamPtr bs, int bsResidualCoding){
  const UINT32 dummyId = 0;

  self->m_bsFreqRes              = __getBits(bs, dummyId, 3);
  self->m_bsFixedGainDMX         = __getBits(bs, dummyId, 3);
  self->m_bsTempShapeConfig      = __getBits(bs, dummyId, 2);
  self->m_bsDecorrConfig         = __getBits(bs, dummyId, 2);
  self->m_bsHighRateMode         = __getBits(bs, dummyId, 1);
  self->m_bsPhaseCoding          = __getBits(bs, dummyId, 1);
  self->m_bsOttBandsPhasePresent = __getBits(bs, dummyId, 1);
  
  if(self->m_bsOttBandsPhasePresent){
    self->m_bsOttBandsPhase      = __getBits(bs, dummyId, 5);
  }

  if(bsResidualCoding){
    self->m_bsResidualBands = __getBits(bs, dummyId, 5);
    self->m_bsOttBandsPhase = max(self->m_bsOttBandsPhase, self->m_bsResidualBands);
    self->m_bsPseudoLr      = __getBits(bs, dummyId, 1);
  }

  if(self->m_bsTempShapeConfig == 2){
    self->m_bsEnvQuantMode = __getBits(bs, dummyId, 1);
  }

  return;
}

static void __ParseUsacCpeConfig(ascparserUsacCpeConfigPtr self, ascparserBitStreamPtr bs, UINT32 numOutChannels, UINT32 *outputChannelIndex, int sbrRatioIndex){
  const UINT32 dummyId = 0;
#ifdef WD1_ELEMENT_MAPPING_3D
  UINT32 nextEmptyChIdx;
  UINT32 nextChannel;
  UINT32 nChannelsSkipped;
  UINT32 shift;
#endif

  __ParseUsacCoreConfig(&(self->m_usacCoreConfig), bs);

  if ( self->m_usacCoreConfig.m_enhancedNoiseFilling ) {
    /* igFIndependentTiling */
    __getBits(bs, dummyId, 1);
  }

  if(sbrRatioIndex > 0){
    __ParseUsacSbrConfig(&(self->m_usacSbrConfig), bs);
    self->m_stereoConfigIndex = __getBits(bs, dummyId, 2);
  } else {
    self->m_stereoConfigIndex = 0;
  }

  if(self->m_stereoConfigIndex > 0){
    UINT32 bsResidualCoding = (self->m_stereoConfigIndex > 1)?1:0;

    __ParseUsacMps212Config(&(self->m_usacMps212Config), bs, bsResidualCoding);
  }
  
  self->m_qceIndex = __getBits(bs, dummyId, 2);

#ifdef WD1_ELEMENT_MAPPING_3D  
  nextEmptyChIdx = ascparserUsacConfig_findFirstEmptyIndex(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  nextChannel = ascparserUsacConfig_findNextChannel(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  nChannelsSkipped = ascparserUsacConfig_getNumberOfSkippedChannels(outputChannelIndex, nextChannel, nextEmptyChIdx, numOutChannels);
  if(self->m_qceIndex > 0) {
    int doShift;
    int bitsToRead=0;
    doShift = __getBits(bs, dummyId, 1);
    if(doShift > 0) {
      /* Fixed number of bits for channel shift */
      int len = numOutChannels - 1;
      while(len>0) {
        bitsToRead++;
        len>>=1;
      }
      shift = __getBits(bs, dummyId, bitsToRead);
      nextChannel = ascparserUsacConfig_getChannelIndex(outputChannelIndex, nextChannel, shift, numOutChannels);
    }
  }
  outputChannelIndex[nextEmptyChIdx] = nextChannel;
  
  {
    int doShift;
    int bitsToRead=0;
    doShift = __getBits(bs, dummyId, 1);
    nextEmptyChIdx = ascparserUsacConfig_findFirstEmptyIndex(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
    nextChannel = ascparserUsacConfig_findNextChannel(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
    nChannelsSkipped = ascparserUsacConfig_getNumberOfSkippedChannels(outputChannelIndex, nextChannel, nextEmptyChIdx, numOutChannels);
    if(doShift > 0) {
      /* Fixed number of bits for channel shift */
      int len = numOutChannels - 1;
      while(len>0) {
        bitsToRead++;
        len>>=1;
      }
      shift = __getBits(bs, dummyId, bitsToRead);
      nextChannel = ascparserUsacConfig_getChannelIndex(outputChannelIndex, nextChannel, shift, numOutChannels);
    }
    outputChannelIndex[nextEmptyChIdx] = nextChannel;
  }
#endif

  if((sbrRatioIndex == 0) && (self->m_qceIndex == 0)){
    self->m_lpdStereoIndex = __getBits(bs, dummyId, 1);
  }
  
  return;
}

static void __ParseUsacLfeConfig(ascparserUsacLfeConfigPtr self, UINT32 *outputChannelIndex, ascparserBitStreamPtr bs){
#ifdef WD1_ELEMENT_MAPPING_3D
  int nextEmptyChIdx;
  int nextChannel;
#endif
  
  self->m_usacCoreConfig.m_tw_mdct      = 0;
  self->m_usacCoreConfig.m_noiseFilling = 0;

#ifdef WD1_ELEMENT_MAPPING_3D
  nextChannel = ascparserUsacConfig_findNextChannel(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  nextEmptyChIdx = ascparserUsacConfig_findFirstEmptyIndex(outputChannelIndex, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS);
  outputChannelIndex[nextEmptyChIdx] = nextChannel;
#endif

  return;
}

static void __ParseUsacConfigExtension_DownmixMatrix(ASCPARSER_USAC_DOWNMIX_CONFIG* pDownmixConfig, ascparserBitStreamPtr bs)
{
   if(pDownmixConfig != NULL){
     const UINT32 dummyId = 0;    
     unsigned int i = 0; 
     unsigned int nBitsRead = __getValidBits(bs);
     
     pDownmixConfig->downmixConfigType = __getBits(bs, dummyId, 2);
     if(pDownmixConfig->downmixConfigType == 0 || pDownmixConfig->downmixConfigType == 2)
     {
       pDownmixConfig->passiveDownmixFlag = __getBits(bs, dummyId, 1);
       if( pDownmixConfig->passiveDownmixFlag == 0 )
       {
         pDownmixConfig->phaseAlignStrength = __getBits(bs, dummyId, 3);
       }
       pDownmixConfig->immersiveDownmixFlag = __getBits(bs, dummyId, 1);
     }

     if(pDownmixConfig->downmixConfigType == 1 || pDownmixConfig->downmixConfigType == 2)
     {
       /* DownmixMatrixConfig() */
       pDownmixConfig->downmixIdCount = __getBits(bs, dummyId, 5);
 
       for ( i = 0; i <  pDownmixConfig->downmixIdCount; ++i )
       {
         int byteCnt = 0;
         int dmxMatrixLenBytes = 0;
         int dmxMatrixBitsLeft = 0;
       
         pDownmixConfig->downmixMatrix[i].DMX_ID = __getBits(bs, dummyId, 7);

         pDownmixConfig->downmixMatrix[i].downmixType = __getBits(bs, dummyId, 2);


         if(pDownmixConfig->downmixMatrix[i].downmixType == 0)
         {
           pDownmixConfig->downmixMatrix[i].CICPspeakerLayoutIdx = __getBits(bs, dummyId, 6);
         }
         else if (pDownmixConfig->downmixMatrix[i].downmixType == 1)
         {
           unsigned int l, m;
           pDownmixConfig->downmixMatrix[i].CICPspeakerLayoutIdx = __getBits(bs, dummyId, 6);
           pDownmixConfig->downmixMatrix[i].downmixMatrixCount   = ascparserUsacConfig_ParseEscapedValue(bs, 1, 3, 0) + 1;
           for( l = 0; l < pDownmixConfig->downmixMatrix[i].downmixMatrixCount; l++ )
           {
             pDownmixConfig->downmixMatrix[i].numAssignedGroupIDs[l] = ascparserUsacConfig_ParseEscapedValue(bs, 1, 4, 4) + 1;
             for( m = 0; m < pDownmixConfig->downmixMatrix[i].numAssignedGroupIDs[l]; m++ )
             {
               pDownmixConfig->downmixMatrix[i].signal_groupID[l][m] = __getBits(bs, dummyId, 5);
             }
             pDownmixConfig->downmixMatrix[i].DmxMatrixLenBits[l] = ascparserUsacConfig_ParseEscapedValue(bs, 8, 8, 12);

             dmxMatrixLenBytes = pDownmixConfig->downmixMatrix[i].DmxMatrixLenBits[l] / 8;
             dmxMatrixBitsLeft = pDownmixConfig->downmixMatrix[i].DmxMatrixLenBits[l] % 8;
             byteCnt = 0;
             while(dmxMatrixLenBytes--)
             {
               pDownmixConfig->downmixMatrix[i].DownmixMatrix[l][byteCnt++] = __getBits(bs, dummyId, 8);
             }
             if( dmxMatrixBitsLeft )
             {
               pDownmixConfig->downmixMatrix[i].DownmixMatrix[l][byteCnt] = __getBits(bs, dummyId, dmxMatrixBitsLeft) << ( 8 - dmxMatrixBitsLeft );
             }

           
           }
           
         }
       }
     }

    nBitsRead = nBitsRead - __getValidBits(bs);

    if (nBitsRead % 8)
      __getBits(bs, dummyId, 8 - (nBitsRead % 8) );

   }

  return;
}


static void __ParseUsacConfigExtension_SignalGroupInformation(ASCPARSER_USAC_SIG_GROUP_INFO* pSigGroupInfo, ascparserBitStreamPtr bs, UINT32 numSignalGroups)
{
  if (pSigGroupInfo != NULL){

    const UINT32 dummyId = 0;
    unsigned int i = 0;
    unsigned int nBitsRead = __getValidBits(bs);

    pSigGroupInfo->numSignalGroups = numSignalGroups;

    for (i = 0; i < pSigGroupInfo->numSignalGroups; i++)
    {
      pSigGroupInfo->groupPriority[i] = __getBits(bs, dummyId, 3);
      pSigGroupInfo->fixedPosition[i] = __getBits(bs, dummyId, 1);
    }

    nBitsRead = nBitsRead - __getValidBits(bs);

    if (nBitsRead % 8)
      __getBits(bs, dummyId, 8 - (nBitsRead % 8) );
  }
  
  return;
}

static void __ParseUsacConfigExtension_CompatibleProfileLevelSet(ASCPARSER_USAC_COMPATIBLE_PROFILE_LEVEL_SET* pCompatibleProfileLevelSet, ascparserBitStreamPtr bs)
{
  if (pCompatibleProfileLevelSet != NULL){

    const UINT32 dummyId = 0;
    unsigned int i = 0;
    unsigned int nBitsRead = __getValidBits(bs);

    pCompatibleProfileLevelSet->numCompatibleSets = __getBits(bs, dummyId, 4) + 1;
    pCompatibleProfileLevelSet->reserved = __getBits(bs, dummyId, 4);

    for (i = 0; i < pCompatibleProfileLevelSet->numCompatibleSets; i++)
    {
      pCompatibleProfileLevelSet->CompatibleSetIndication[i] = __getBits(bs, dummyId, 8);
    }

    nBitsRead = nBitsRead - __getValidBits(bs);

    if (nBitsRead % 8)
      __getBits(bs, dummyId, 8 - (nBitsRead % 8) );
  }
  
  return;
}

static void __ParseUsacConfigExtension_HoaMatrix(ASCPARSER_USAC_HOA_CONFIG* pHoaConfig, ascparserBitStreamPtr bs)
{
  if(pHoaConfig != NULL){
    const UINT32 dummyId = 0;
    unsigned int i = 0;
    unsigned int nBitsRead = __getValidBits(bs);

    pHoaConfig->numHoaMatrices = __getBits(bs, dummyId, 5);

    for ( i = 0; i <  pHoaConfig->numHoaMatrices; ++i )
    {
      int byteCnt = 0;
      int hoaMatrixLenBytes = 0;
      int hoaMatrixBitsLeft = 0;

      pHoaConfig->hoaMatrix[i].HOA_ID = __getBits(bs, dummyId, 7);

      pHoaConfig->hoaMatrix[i].CICPspeakerLayoutIdx = __getBits(bs, dummyId, 6);
      pHoaConfig->hoaMatrix[i].HoaMatrixLenBits = ascparserUsacConfig_ParseEscapedValue(bs, 8, 8, 12);
      hoaMatrixLenBytes = pHoaConfig->hoaMatrix[i].HoaMatrixLenBits / 8;
      hoaMatrixBitsLeft = pHoaConfig->hoaMatrix[i].HoaMatrixLenBits % 8;

      while(hoaMatrixLenBytes--)
      {
        pHoaConfig->hoaMatrix[i].HoaMatrix[byteCnt++] = __getBits(bs, dummyId, 8);
      }

      if (hoaMatrixBitsLeft)
      {
        pHoaConfig->hoaMatrix[i].HoaMatrix[byteCnt] = __getBits(bs, dummyId, hoaMatrixBitsLeft) << (8-hoaMatrixBitsLeft);
      }
    }

    nBitsRead = nBitsRead - __getValidBits(bs);

    if (nBitsRead % 8)
      __getBits(bs, dummyId, 8 - (nBitsRead % 8) );
  }

  return;
}


static void __ParseUsacConfigExtension(ascparserUsacConfigExtensionPtr self, ascparserBitStreamPtr bs, UINT32 numSignalGroups){
  
  const UINT32 dummyId = 0;
  unsigned int confExtIdx = 0;
  
  if(self){
    self->m_usacConfigExtNumConfExt = ascparserUsacConfig_ParseEscapedValue(bs, 2, 4, 8) + 1;
    for(confExtIdx = 0; confExtIdx < self->m_usacConfigExtNumConfExt; confExtIdx++){
      unsigned int byteCnt = 0;
      unsigned int usacConfigExtLength = 0;
      self->m_usacConfigExtType[confExtIdx]   = (ASCPARSER_USAC_CONFIG_EXTENSION_TYPE)ascparserUsacConfig_ParseEscapedValue(bs, 4, 8, 16);
      self->m_usacConfigExtLength[confExtIdx] = ascparserUsacConfig_ParseEscapedValue(bs, 4, 8, 16);

      usacConfigExtLength = self->m_usacConfigExtLength[confExtIdx];
      switch(self->m_usacConfigExtType[confExtIdx]){
      case ASCPARSER_USAC_ID_CONFIG_EXT_FILL:
        while(usacConfigExtLength--){
          /* Read and throw away */
          __getBits(bs, dummyId, 8); 
        }
        break;
      case ASCPARSER_USAC_ID_CONFIG_EXT_DOWNMIX_CONFIG:
        if ( usacConfigExtLength != 0 )
          __ParseUsacConfigExtension_DownmixMatrix( (ASCPARSER_USAC_DOWNMIX_CONFIG*) self->m_usacConfigExtPayload[confExtIdx],bs);
        break;
      case ASCPARSER_USAC_ID_CONFIG_EXT_LOUDNESS_INFO_SET:
        {
        for( byteCnt = 0; byteCnt < usacConfigExtLength; byteCnt++ ) { 
          self->m_usacConfigExtPayload[confExtIdx][byteCnt] = (unsigned char) __getBits(bs, dummyId, 8); 
        }
        break;
        }
      case ASCPARSER_USAC_ID_CONFIG_EXT_AUDIOSCENE_INFO:
        if ( usacConfigExtLength != 0 )
          Read_AudioSceneInfo( bs, &(self->m_audioSceneInfo) );
        break;
      case ASCPARSER_USAC_ID_CONFIG_EXT_HOA_CONFIG:
        if ( usacConfigExtLength != 0 )
          __ParseUsacConfigExtension_HoaMatrix( (ASCPARSER_USAC_HOA_CONFIG*) self->m_usacConfigExtPayload[confExtIdx],bs);
        break;
      case ASCPARSER_USAC_ID_CONFIG_EXT_SIG_GROUP_INFO:
        if ( usacConfigExtLength != 0 )
        {
          __ParseUsacConfigExtension_SignalGroupInformation( (ASCPARSER_USAC_SIG_GROUP_INFO*) self->m_usacConfigExtPayload[confExtIdx],bs,numSignalGroups);
        }
        break;
      case ASCPARSER_USAC_ID_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET:
        if ( usacConfigExtLength != 0 )
        {
          __ParseUsacConfigExtension_CompatibleProfileLevelSet( (ASCPARSER_USAC_COMPATIBLE_PROFILE_LEVEL_SET*) self->m_usacConfigExtPayload[confExtIdx],bs);
        }
        break;
      default:
        for(byteCnt = 0; byteCnt < usacConfigExtLength; byteCnt++){
          self->m_usacConfigExtPayload[confExtIdx][byteCnt] = (unsigned char) __getBits(bs, dummyId, 8); 
        }
        break;
      }
    }
  }
  return;
}

static void __ParseUsacExtConfig(ascparserUsacExtConfigPtr self, ascparserBitStreamPtr bs){
const UINT32 dummyId = 0;
int length;  

  self->m_usacExtElementType          = (ASCPARSER_USAC_EXT_ELEMENT_TYPE)ascparserUsacConfig_ParseEscapedValue(bs, 4, 8, 16);
  self->m_usacExtElementConfigLength  = ascparserUsacConfig_ParseEscapedValue(bs, 4, 8, 16);
    
  self->m_usacExtElementDefaultLengthPresent = __getBits(bs, dummyId, 1);

  if(self->m_usacExtElementDefaultLengthPresent) {
    self->m_usacExtElementDefaultLength = ascparserUsacConfig_ParseEscapedValue(bs, 8, 16, 0 ) + 1; 
  } else {
    self->m_usacExtElementDefaultLength = 0;
  }
  self->m_usacExtElementPayloadFrag = __getBits(bs, dummyId, 1);

  switch(self->m_usacExtElementType) 
    {
    case ASCPARSER_USAC_ID_EXT_ELE_FILL:
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_MPEGS:
      /* not implemented */    
      assert(0);        
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_SAOC:
      /* not implemented */    
      assert(0);        
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_SAOC_3D:
      length = self->m_usacExtElementConfigLength;
      while(length--) {
        self->m_usacExtElementConfigPayload[self->m_usacExtElementConfigLength -1 -length] = __getBits(bs, dummyId, 8);
      }
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_OBJ:
      length = self->m_usacExtElementConfigLength;
      while(length--) {
        self->m_usacExtElementConfigPayload[self->m_usacExtElementConfigLength -1 -length] = __getBits(bs, dummyId, 8);
      }
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_ENHANCED_OBJ_METADATA:
      length = self->m_usacExtElementConfigLength;
      while(length--) {
        self->m_usacExtElementConfigPayload[self->m_usacExtElementConfigLength -1 -length] = __getBits(bs, dummyId, 8);
      }
      break;
    case ASCPARSER_USAC_ID_EXT_ELE_PRODUCTION_METADATA:
      length = self->m_usacExtElementConfigLength;
      while(length--) {
        self->m_usacExtElementConfigPayload[self->m_usacExtElementConfigLength -1 -length] = __getBits(bs, dummyId, 8);
      }
      break;
    default:
      length = self->m_usacExtElementConfigLength;
      while(length--) {
        self->m_usacExtElementConfigPayload[self->m_usacExtElementConfigLength -1 -length] = __getBits(bs, dummyId, 8);
      }
      break;
  }

  return;
}

#ifdef WD1_FRAMEWORK_CONFIG_3D
static void __ParseSignals3d(ascparserUsacSignalsPtr m_signals3D,  ascparserBitStreamPtr bs)
{
#ifdef CD_SIGNALS_3D_SIGNAL_GROUPS

{
    UINT32 dummyId = 0;
    unsigned int i;
  
    m_signals3D->m_numAudioChannels         = 0;
    m_signals3D->m_numAudioObjects          = 0;
    m_signals3D->m_numSAOCTransportChannels = 0;
    m_signals3D->m_numHOATransportChannels  = 0;

    m_signals3D->m_numSignalGroups = __getBits(bs, dummyId, 5) + 1;
    for ( i = 0; i < m_signals3D->m_numSignalGroups; i++ ) {

      m_signals3D->m_signalGroupType[i] = (ASCPARSER_SIGNAL_GROUP_TYPE)__getBits(bs, dummyId, 3);
      m_signals3D->m_numberOfSignals[i] = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);
      
      switch( m_signals3D->m_signalGroupType[i] ) {
        case 0:
          m_signals3D->m_numAudioChannels += m_signals3D->m_numberOfSignals[i] + 1;

          m_signals3D->m_differsFromReferenceLayout[i] = __getBits(bs, dummyId, 1);
          if ( m_signals3D->m_differsFromReferenceLayout[i] ) {
               __ascparserUsacConfig_SpeakerConfig3d(&(m_signals3D->m_audioChannelLayout[i]), bs);
          }
          break;
        case 1:
          m_signals3D->m_numAudioObjects += m_signals3D->m_numberOfSignals[i] + 1;
          break;
        case 2:
          m_signals3D->m_numSAOCTransportChannels += m_signals3D->m_numberOfSignals[i] + 1;
#ifdef SUPPORT_SAOC_DMX_LAYOUT
          m_signals3D->m_saocDmxLayoutPresent = __getBits(bs, dummyId, 1);
          if ( m_signals3D->m_saocDmxLayoutPresent) {
               __ascparserUsacConfig_SpeakerConfig3d(&(m_signals3D->m_saocDmxChannelLayout), bs);
          }
#endif
          break;
        case 3:
          m_signals3D->m_numHOATransportChannels += m_signals3D->m_numberOfSignals[i] + 1;
          break;
      }
    }
  }

#else
  m_signals3D->m_numAudioChannels = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);
  m_signals3D->m_numAudioObjects  = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);
  m_signals3D->m_numSAOCTransportChannels = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);

#ifdef SUPPORT_HOA
  m_signals3D->m_numHOATransportChannels = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);
#endif
#endif
}

static void __ParseFrameworkConfig3d(ascparserUsacFrameworkPtr usacFramework, ascparserBitStreamPtr bs)
{  
  __ParseSignals3d(&(usacFramework->m_signals3D), bs);
}
#endif

static void __ParseUsacDecoderConfig(ascparserUsacDecoderConfigPtr self, ascparserUsacChannelConfig* chCfg, ascparserBitStreamPtr bs, int sbrRatioIndex){
  UINT32 elemIdx = 0;
  UINT32 dummyId = 0;
  UINT32 numOutChannels     = chCfg->m_numOutChannels;
  UINT32* outputChannelIndex = (UINT32*)chCfg->m_outputChannelsIndex;

  /* initialize to -1 */
  memset(outputChannelIndex, -1, ASCPARSER_USAC_MAX_NUM_OUT_CHANNELS*sizeof(INT32));

  self->m_numElements = ascparserUsacConfig_ParseEscapedValue(bs, 4, 8, 16) + 1;
  /* parse len bit present */
  self->m_elementLengthPresent = __getBits(bs, dummyId, 1); 

  for(elemIdx = 0; elemIdx < self->m_numElements; elemIdx++){
    self->m_usacElementType[elemIdx] = (ASCPARSER_USAC_ELEMENT_TYPE)__getBits(bs, dummyId, 2);

    switch(self->m_usacElementType[elemIdx]){
    case ASCPARSER_USAC_ELEMENT_TYPE_SCE:
      __ParseUsacSceConfig(&(self->m_usacElementConfig[elemIdx].m_usacSceConfig), bs, outputChannelIndex, sbrRatioIndex);
      break;
    case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
      __ParseUsacCpeConfig(&(self->m_usacElementConfig[elemIdx].m_usacCpeConfig), bs, numOutChannels, outputChannelIndex, sbrRatioIndex);
      break;
    case ASCPARSER_USAC_ELEMENT_TYPE_LFE:
      __ParseUsacLfeConfig(&(self->m_usacElementConfig[elemIdx].m_usacLfeConfig), outputChannelIndex, bs);
      break;
    case ASCPARSER_USAC_ELEMENT_TYPE_EXT:
      __ParseUsacExtConfig(&(self->m_usacElementConfig[elemIdx].m_usacExtConfig), bs);
      break;
    default:
      break;
    }
  }

  return;
}

UINT32 ascparserUsacConfig_GetSaocHeaderLength(const ascparserUsacConfig* self)
{
  unsigned int elemIdx = 0;

  for(elemIdx = 0; elemIdx < self->m_usacDecoderConfig.m_numElements; elemIdx++){
    switch(self->m_usacDecoderConfig.m_usacElementType[elemIdx]){
       case ASCPARSER_USAC_ELEMENT_TYPE_EXT:
         if ( self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementType  == ASCPARSER_USAC_ID_EXT_ELE_SAOC_3D )
           return self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigLength;
         break;

    default:
      break;

    }
  }

  return 0;
}


static int _enhObjConf_getBits(unsigned char* buffer, int n, int reset)
{
  int i;
  int val = 0;   /* output value */
  static int bitPos = 0;
  static int bytePos = 0;

  if (reset == 1)
  {
    bitPos = 0;
    bytePos = 0;
    return 1;
  }

  assert(n <= 32);                /* otherwise an int overflows */
  
  for (i = 0; i < n; i++) 
  {
    char mask = 0x00;
    mask |= 1 << bitPos;
    val |= ((buffer[bytePos] & mask) >> bitPos) << i;
    bitPos++;

    /* go to the next byte */
    if (bitPos > 7)
    {
      bitPos = 0;
      bytePos++;
    }
  }

  return val;
}



UINT32 ascparserUsacConfig_GetEnhancedObjectMetadataConfig(ascparserUsacConfig* self,
                                                           int *hasDiffuseness,
                                                           int *hasCommonGroupDiffuseness,
                                                           int *hasExcludedSectors,
                                                           int *hasCommonGroupExcludedSectors,
                                                           int *useOnlyPredefinedSectors,
                                                           int *hasClosestSpeakerCondition,
                                                           float *closestSpeakerThresholdAngle,
                                                           int *hasDivergence,
                                                           float *divergenceAzimuthRange
) {
  unsigned int elemIdx = 0;
  unsigned char bitmask = 0;
  unsigned int num_objects = self->m_usacFramework.m_signals3D.m_numAudioObjects;
  unsigned int nObj    = 0;

  *hasDiffuseness = 0;
  *hasCommonGroupDiffuseness = 1;
  *hasExcludedSectors = 0;
  *hasCommonGroupExcludedSectors = 1;
  *useOnlyPredefinedSectors = 1;
  *hasClosestSpeakerCondition = 0;
  *closestSpeakerThresholdAngle = 0.0f;
  *hasDivergence = 0;
  *divergenceAzimuthRange = 0.0f;

  for(elemIdx = 0; elemIdx < self->m_usacDecoderConfig.m_numElements; elemIdx++)
  {
    unsigned int ExtElementConfigLength = self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigLength;
    unsigned nExtensionBytes = 0;
    unsigned int bitsRead = 0;

    switch(self->m_usacDecoderConfig.m_usacElementType[elemIdx]){
      case ASCPARSER_USAC_ELEMENT_TYPE_EXT:
        if ( self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementType  == ASCPARSER_USAC_ID_EXT_ELE_ENHANCED_OBJ_METADATA )
        {
          int errorCode = 0;

          /* ascparserBitBuffer bitbuf;
          ascparserBitStreamPtr bs; 
          UINT32 bitbuflen = 0;
          UINT32 tmp = 0;
          bitbuflen = nExtensionBytes;
          tmp = bitbuflen;
          ascparserBitBuffer_Initialize(&bitbuf);
          ascparserBitBuffer_Feed(&bitbuf, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload, bitbuflen, &tmp);
          if (tmp != 0)
            return -1;
          bs = &(bitbuf.base); */

          /* diffuseness */
          *hasDiffuseness = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                1,
                                                0);

          if (*hasDiffuseness == 1) {
            *hasCommonGroupDiffuseness = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                             1,
                                                             0);
          }

          /* excluded sectors */
          *hasExcludedSectors = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                    1,
                                                    0);

          if (*hasExcludedSectors == 1) {
            *hasCommonGroupExcludedSectors = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                                 1,
                                                                 0);

            if (*hasCommonGroupExcludedSectors == 1) {
              useOnlyPredefinedSectors[0] = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                                1,
                                                                0);
            }
          }

          /* closest speaker playout */
          *hasClosestSpeakerCondition = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                            1,
                                                            0);

          if (*hasClosestSpeakerCondition == 1) {
            unsigned char closestSpeakerThresholdAngleBitfield;

            closestSpeakerThresholdAngleBitfield = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                                       7,
                                                                       0);
            *closestSpeakerThresholdAngle = min(max((float)closestSpeakerThresholdAngleBitfield * 1.5f, 0.0f), 180.0f);
          }

          /* divergence */
          for (nObj = 0; nObj < num_objects; ++nObj) {
            hasDivergence[nObj] = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                      1,
                                                      0);

            if (hasDivergence[nObj] == 1) {
              unsigned char divergenceAzimuthRangeBitfield;

              divergenceAzimuthRangeBitfield = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                                   6,
                                                                   0);

              *divergenceAzimuthRange = min(max((float)divergenceAzimuthRangeBitfield * 3.0f, 0.0f), 180.0f);
            }

            /* exclusion sectors */
            if (*hasCommonGroupExcludedSectors == 0) {
              useOnlyPredefinedSectors[nObj] = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                                                   1,
                                                                   0);
            }
          }
          { /* reset reader */
            int reset = -1;
            reset = _enhObjConf_getBits(self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload,
                                        1,
                                        1);
          }
        }
        break;
      default:
        break;
    }
  }
  return 0;
}

UINT32 ascparserUsacConfig_GetObjectMetadataConfig(const ascparserUsacConfig* self, int* mode, int* frameLength, int* hasScreenRelativeObjects, int* isScreenRelativeObject, int* hasDynamicObjectPriority, int* hasUniformSpread)
{
  unsigned int elemIdx = 0;
  unsigned int numBytesNeededForObjects = 0;
  unsigned int objectByte = 0;
  unsigned int bitCnt = 0;
  unsigned int scrRltPos = 0;
  unsigned int num_objects = self->m_usacFramework.m_signals3D.m_numAudioObjects;
  unsigned char bitmask = 0;
  OAM_BYTE_PARSER oamByteParser = { 0 };
  OAM_BYTE_PARSER_HANDLE hOamByteParser = (OAM_BYTE_PARSER_HANDLE)&oamByteParser;

  *mode = -1;
  *frameLength = -1;
  *hasScreenRelativeObjects = -1;
  *isScreenRelativeObject = -1;
  *hasDynamicObjectPriority = -1;
  *hasUniformSpread = -1;

  if ( num_objects > 0 ) {
    numBytesNeededForObjects = (int) ( ( num_objects ) / 8 + 1 );
  }

  for(elemIdx = 0; elemIdx < self->m_usacDecoderConfig.m_numElements; elemIdx++){

    unsigned int ExtElementConfigLength = self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigLength;
    unsigned nExtensionBytes = 0;

    switch(self->m_usacDecoderConfig.m_usacElementType[elemIdx]){

       case ASCPARSER_USAC_ELEMENT_TYPE_EXT:
         if ( self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementType  == ASCPARSER_USAC_ID_EXT_ELE_OBJ )
           {

             int hasCoreLength = 0;
             int errorCode     = 0;

             errorCode = _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
             if ( errorCode ) return 1;

             errorCode = _oamBitwiseByteParser( hOamByteParser, mode, 1); /* lowDelayMetadataCoding */
             if ( errorCode == 2 ) return 1;

             errorCode = _oamBitwiseByteParser( hOamByteParser, &hasCoreLength, 1); /* hasCoreLength */
             if ( errorCode == 2 ) return 1;


             if ( hasCoreLength ) {

               *frameLength = self->m_outputFrameLength;

             } else {

               unsigned char framelengthBitfield;

               errorCode = _oamBitclusterByteParser ( hOamByteParser, &framelengthBitfield, 6);
               if ( errorCode == 2 ) return 1;
               if ( errorCode == 1 ) {
                 if ( ++nExtensionBytes >= ExtElementConfigLength ) return 1;
                 _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
               }


               *frameLength = ( framelengthBitfield + 1 ) << 6;

             } /* hasCoreFramelength */

             errorCode = _oamBitwiseByteParser( hOamByteParser, hasScreenRelativeObjects, 1); /* hasScreenRelativeObjects */
             if ( errorCode == 2 ) return 1;
             if ( errorCode == 1 ) {
               if ( ++nExtensionBytes >= ExtElementConfigLength ) return 1;
               _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
             }





             if ( *hasScreenRelativeObjects ) {

               unsigned int nObj    = 0;

               for ( nObj = 0; nObj < num_objects; ++nObj ) {

                 errorCode = _oamBitwiseByteParser( hOamByteParser, &(isScreenRelativeObject[nObj]), 1);
                 if ( errorCode == 2 ) return 1;
                 if ( errorCode == 1 ) {
                   if ( ++nExtensionBytes >= ExtElementConfigLength ) return 1;
                   _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
                 }

               }
             }

             errorCode = _oamBitwiseByteParser( hOamByteParser, hasDynamicObjectPriority, 1); /* hasDynamicObjectPriority */
             if ( errorCode == 2 ) return 1;
             if ( errorCode == 1 ) {
               if ( ++nExtensionBytes >= ExtElementConfigLength ) return 1;
               _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
             }

             errorCode =  _oamBitwiseByteParser( hOamByteParser, hasUniformSpread, 1);/* hasUniformSpread */
             if ( errorCode == 2 ) return 1;
             if ( errorCode == 1 ) {
               if ( ++nExtensionBytes >= ExtElementConfigLength ) return 1;
               _oamFeedByteParser ( hOamByteParser, self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBytes] );
             }

           }
         break;
    default:
      break;
    }
  }

  return 0;
}


UINT32 ascparserUsacConfig_GetProductionMetadataConfig(const ascparserUsacConfig* self, int* has_reference_distance, int* bs_reference_distance, int* has_object_distance, int* directHeadphone, int* numObjectGroups, int* numChannelGroups)
{
  unsigned int elemIdx = 0;

  for (elemIdx = 0; elemIdx < self->m_usacDecoderConfig.m_numElements; elemIdx++) {
    unsigned int ExtElementConfigLength = self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigLength;
    unsigned nExtensionBytes = 0;
    unsigned int nExtensionBits = 0;

    switch (self->m_usacDecoderConfig.m_usacElementType[elemIdx]) {
      case ASCPARSER_USAC_ELEMENT_TYPE_EXT:
        if (self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementType  == ASCPARSER_USAC_ID_EXT_ELE_PRODUCTION_METADATA) {
          if (ExtElementConfigLength > 0) {
            int i;
            char tmp;
            tmp = ((unsigned char)self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBits/8] >> (7 - nExtensionBits % 8)) & 0x01;

            *has_reference_distance = tmp;
            nExtensionBits++;

            if (*has_reference_distance == 0) {
              *bs_reference_distance = 57;
            } else {
              tmp = self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBits/8] & 0x7F;
              *bs_reference_distance = tmp;
              nExtensionBits = nExtensionBits + 7;
            }

            /* parsing numChannelGroups and numObjectGroups */
            for (i = 0 ; i < self->m_usacFramework.m_signals3D.m_numSignalGroups; i++) {
              if (self->m_usacFramework.m_signals3D.m_signalGroupType[i] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS) {
                (*numChannelGroups)++;
              } else if (self->m_usacFramework.m_signals3D.m_signalGroupType[i] == ASCPARSER_SIGNAL_GROUP_TYPE_OBJECTS) {
                (*numObjectGroups)++;
              }
            }

            /* high resolution object distance */
            for (i = 0 ; i < (*numObjectGroups); i++) {
              has_object_distance[i] = ((unsigned char)self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBits/8] >> (7 - nExtensionBits % 8)) & 0x01;
              nExtensionBits++;
            }

            /* direct to headphone */
            for (i = 0 ; i < (*numChannelGroups); i++) {
              directHeadphone[i] = ((unsigned char)self->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig.m_usacExtElementConfigPayload[nExtensionBits/8] >> (7 - nExtensionBits % 8)) & 0x01;
              nExtensionBits++;
            }

          } else {
            *has_reference_distance = 0;
            *bs_reference_distance  = 57;
            *numObjectGroups        = -1; 
            *numChannelGroups       = -1;
            *has_object_distance    = -1;
            *directHeadphone        = -1;
          }

          nExtensionBytes = nExtensionBits / 8;
          if (++nExtensionBytes >= ExtElementConfigLength) {
            return 1;
          }
        }
        break;
      default:
        break;
    }
  }

  return 0;
}


UINT32 ascparserUsacConfig_ParseEscapedValue(ascparserBitStreamPtr bs, UINT32 nBits1, UINT32 nBits2, UINT32 nBits3){
  const UINT32 dummyId = 0;
  UINT32 valueAdd = 0;
  UINT32 maxValue1 = (1 << nBits1) - 1;
  UINT32 maxValue2 = (1 << nBits2) - 1;
  UINT32 value = __getBits(bs, dummyId, nBits1);

  if(value == maxValue1){
    valueAdd = __getBits(bs, dummyId, nBits2);
    value += valueAdd;

    if(valueAdd == maxValue2){
      valueAdd = __getBits(bs, dummyId, nBits3);
      value += valueAdd;
    }
  }

  return value;
}

void ascparserUsacConfig_Parse(ascparserUsacConfigPtr self, ascparserBitStreamPtr bs)
{
  const UINT32 dummyId = 0;
  UINT32 coreSbrFrameLengthIndex;
  UINT32 usacConfigExtensionPresent;
  INT32 nBitsRead = __getValidBits(bs);

  InitMemberVariables(self);

  self->m_mpegh3daProfileLevelIndication = __getBits(bs, dummyId, 8);

  /* usacSamplingFrequencyIndex and usacSamplingFrequency */
  self->m_usacSamplingFrequencyIndex = __getBits(bs, dummyId, 5);
  if( self->m_usacSamplingFrequencyIndex == 0x1f ){
    self->m_usacSamplingFrequency = __getBits(bs, dummyId, 24);
  } else {
    self->m_usacSamplingFrequency = __GetUsacSamplingFrequencyFromIndex(self->m_usacSamplingFrequencyIndex);
  }

  /* coreSbrFrameLengthIndex; determines outputFrameLength and sbrRatioIndex */
  coreSbrFrameLengthIndex   = __getBits(bs, dummyId, 3);
  self->m_outputFrameLength = __GetOutputFrameLength(coreSbrFrameLengthIndex);
  self->m_sbrRatioIndex     = __GetSbrRatioIndex(coreSbrFrameLengthIndex);

  self->m_coreQmfDelayCompensation  = __getBits(bs, dummyId, 1);
  self->m_receiverDelayCompensation = __getBits(bs, dummyId, 1);

#ifdef WD1_REFERENCE_LAYOUT_3D
  
  __ascparserUsacConfig_SpeakerConfig3d(&(self->m_referenceLayout), bs);
  
#else /* WD1_REFERENCE_LAYOUT_3D */
  /* channelConfigurationIndex and UsacChannelConfig() */
  self->m_channelConfigurationIndex = __getBits(bs, dummyId, 7);
  if(self->m_channelConfigurationIndex == 0){
    __ParseUsacChannelConfig(&self->m_usacChannelConfig, bs);
  } else {
    __GetUsacChannelConfigFromIndex(&(self->m_usacChannelConfig), self->m_channelConfigurationIndex);
  }

#endif /* WD1_REFERENCE_LAYOUT_3D */


#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
  /* num of coded objects */
#ifdef WD1_ESCAPED_OBJECTS_MAPPING_3D
  self->m_numObjects = ascparserUsacConfig_ParseEscapedValue(bs, 5, 8, 16);
#else
  self->m_numObjects = __getBits(bs, dummyId, 8);
#endif

#ifdef WD1_REFERENCE_LAYOUT_3D

  if (self->m_numObjects > 0) {
    self->m_differsFromReferenceLayout = __getBits(bs, dummyId, 1);

    if(self->m_differsFromReferenceLayout==1){
      __ascparserUsacConfig_SpeakerConfig3d(&(self->m_audioChannelLayout), bs);
    }
    else{
     self->m_audioChannelLayout = self->m_referenceLayout;
    }
  }
  else {
    self->m_audioChannelLayout = self->m_referenceLayout;
  }
#endif /* WD1_REFERENCE_LAYOUT_3D */
#endif

#ifdef WD1_FRAMEWORK_CONFIG_3D
  __ParseFrameworkConfig3d(&(self->m_usacFramework), bs);
  self->m_usacChannelConfig.m_numOutChannels = self->m_usacFramework.m_signals3D.m_numAudioChannels + 
                                               self->m_usacFramework.m_signals3D.m_numAudioObjects  + 
                                               self->m_usacFramework.m_signals3D.m_numHOATransportChannels + 
                                               self->m_usacFramework.m_signals3D.m_numSAOCTransportChannels;
  if( self->m_referenceLayout.speakerLayoutID == 3 ){ /* contribution mode */
    assert( self->m_usacFramework.m_signals3D.m_numSignalGroups == 1 );
    assert( self->m_usacFramework.m_signals3D.m_signalGroupType[0] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS );
    assert( self->m_usacFramework.m_signals3D.m_differsFromReferenceLayout[0] == 0 );
  }


#endif

  __ParseUsacDecoderConfig(&(self->m_usacDecoderConfig), &(self->m_usacChannelConfig), bs, self->m_sbrRatioIndex); 
 
  usacConfigExtensionPresent  = __getBits(bs,dummyId,1);
  if(usacConfigExtensionPresent){
    __ParseUsacConfigExtension(&self->m_usacConfigExtension, bs, self->m_usacFramework.m_signals3D.m_numSignalGroups);
  }

  nBitsRead = nBitsRead - __getValidBits(bs);

  if (nBitsRead % 8) {
    __getBits(bs, dummyId, 8 - (nBitsRead % 8));
    nBitsRead += 8 - (nBitsRead % 8);
  }
}


UINT32 ascparserUsacConfig_Compare(const ascparserUsacConfigPtr self, const ascparserUsacConfigPtr gasc, const ASCPARSER_AUDIO_OBJECT_TYPE aot)
{
  assert(0);
  
  return 0;
}

void ascparserUsacConfig_Copy(ascparserUsacConfigPtr self, const ascparserUsacConfigPtr usacConfig)
{
  self->m_usacSamplingFrequency      = usacConfig->m_usacSamplingFrequency;
  self->m_usacSamplingFrequencyIndex = usacConfig->m_usacSamplingFrequencyIndex;
  self->m_outputFrameLength          = usacConfig->m_outputFrameLength;
  self->m_sbrRatioIndex              = usacConfig->m_sbrRatioIndex;
#ifndef WD1_REFERENCE_LAYOUT_3D
  self->m_channelConfigurationIndex  = usacConfig->m_channelConfigurationIndex;
#endif
}



INT32 ascparserUsacConfig_Print(const ascparserUsacConfigPtr usacConfig, char string[], INT32 chanConf, const ASCPARSER_AUDIO_OBJECT_TYPE aot)
{
  int pos = 0;
  INT32 err = 0;

  sprintf(&string[pos], "%16s %d\n", "usacSamplingFrequencyIndex:", usacConfig->m_usacSamplingFrequencyIndex);
  pos = strlen(string);

  sprintf(&string[pos], "%16s %d\n", "usacSamplingFrequency:", usacConfig->m_usacSamplingFrequency);
  pos = strlen(string);

  sprintf(&string[pos], "%16s %d\n", "outputFrameLength:", usacConfig->m_outputFrameLength);
  pos = strlen(string);

  sprintf(&string[pos], "%16s %d\n", "sbrRatioIndex:", usacConfig->m_sbrRatioIndex);
  pos = strlen(string);
#ifndef WD1_REFERENCE_LAYOUT_3D
  sprintf(&string[pos], "%16s %d\n", "channelConfigurationIndex:", usacConfig->m_channelConfigurationIndex);
  pos = strlen(string);

  if (usacConfig->m_channelConfigurationIndex == 0) {
    
  }
#endif


  return err;
}




ASCPARSER_USAC_ELEMENT_TYPE ascparserUsacConfig_GetUsacElementType(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  ASCPARSER_USAC_ELEMENT_TYPE usacElementType = ASCPARSER_USAC_ELEMENT_TYPE_INVALID;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      usacElementType = pUsacConfig->m_usacDecoderConfig.m_usacElementType[elemIdx];
    }
  }

  return usacElementType;
}


const ascparserUsacCoreConfig * ascparserUsacConfig_GetUsacCoreConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  const ascparserUsacCoreConfig * pUsacCoreConfig = 0;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case ASCPARSER_USAC_ELEMENT_TYPE_SCE:
        pUsacCoreConfig = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacSceConfig.m_usacCoreConfig);
        break;
      case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
        pUsacCoreConfig = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacCpeConfig.m_usacCoreConfig);
        break;
      case ASCPARSER_USAC_ELEMENT_TYPE_LFE:
        pUsacCoreConfig = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacLfeConfig.m_usacCoreConfig);
        break;
      default:
        break;
      }
    }
  }

  return pUsacCoreConfig;
}


const ascparserUsacExtConfig * ascparserUsacConfig_GetUsacExtConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  const ascparserUsacExtConfig * pUsacExtConfig = 0;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);
      if(ASCPARSER_USAC_ELEMENT_TYPE_EXT == elemType){
        pUsacExtConfig = &pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacExtConfig;
      }
    }
  }

  return pUsacExtConfig;
}


const ascparserUsacSbrConfig * ascparserUsacConfig_GetUsacSbrConfig(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  const ascparserUsacSbrConfig * pUsacSbrConfig = 0;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case ASCPARSER_USAC_ELEMENT_TYPE_SCE:
        pUsacSbrConfig = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacSceConfig.m_usacSbrConfig);
        break;
      case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
        pUsacSbrConfig = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacCpeConfig.m_usacSbrConfig);
        break;
      default:
        break;
      }
    }
  }

  return pUsacSbrConfig;
}


const ascparserUsacMps212Config * ascparserUsacConfig_GetUsacMps212Config(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  const ascparserUsacMps212Config * pUsacMps212Config = 0;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
        pUsacMps212Config = &(pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacCpeConfig.m_usacMps212Config);
        break;
      default:
        break;
      }
    }
  }

  return pUsacMps212Config;
}


int ascparserUsacConfig_GetStereoConfigIndex(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx){

  int stereoConfigIndex = -1;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
        stereoConfigIndex = pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacCpeConfig.m_stereoConfigIndex;
        break;
      default:
        break;
      }
    }
  }

  return stereoConfigIndex;
}


int ascparserUsacConfig_GetQCEIndex(const ascparserUsacConfig * pUsacConfig, unsigned int elemIdx)
{
  int qceIndex = 0;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->m_usacDecoderConfig.m_numElements){
      ASCPARSER_USAC_ELEMENT_TYPE elemType = ascparserUsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case ASCPARSER_USAC_ELEMENT_TYPE_CPE:
        qceIndex = pUsacConfig->m_usacDecoderConfig.m_usacElementConfig[elemIdx].m_usacCpeConfig.m_qceIndex;
        break;
      default:
        break;
      }
    }
  }

  return qceIndex;
}

#ifdef WD1_REFERENCE_LAYOUT_3D

int ascparser_AzimuthAngleIdxToDegrees ( int idx, int direction, int precision ) {
  
  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

int ascparser_ElevationAngleIdxToDegrees ( int idx, int direction, int precision ) {

  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

static int __ascparserUsacConfig_SpeakerConfig3d ( SpeakerConfig3d* pSpeakerConfig3d, ascparserBitStreamPtr bs ) {

  unsigned int i;
  const UINT32 dummyId = 0;

  pSpeakerConfig3d->speakerLayoutID =  __getBits ( bs, dummyId, 2 );

  if ( pSpeakerConfig3d->speakerLayoutID == 0 ) {
    pSpeakerConfig3d->CICPspeakerLayoutIdx = __getBits ( bs, dummyId, 6 );
  } else {
    pSpeakerConfig3d->numSpeakers = ascparserUsacConfig_ParseEscapedValue ( bs, 5, 8, 16 ) + 1;

    if ( pSpeakerConfig3d->speakerLayoutID == 1 ) {
      for ( i = 0; i < pSpeakerConfig3d->numSpeakers; i++ ) {
        pSpeakerConfig3d->CICPspeakerIdx[i]  =  __getBits ( bs, dummyId, 7 );
      }
    }

    if ( pSpeakerConfig3d->speakerLayoutID == 2 ) {
      __ascparserUsacConfig_Usac3dFlexibleSpeakerConfig ( & ( pSpeakerConfig3d->flexibleSpeakerConfig ), pSpeakerConfig3d->numSpeakers, bs );
    }
  }

  return 0;
}

static int __ascparserUsacConfig_Usac3dFlexibleSpeakerConfig ( Usac3dFlexibleSpeakerConfig* pFlexibleSpeakerConfig3d, int numSpeakers, ascparserBitStreamPtr bs ) {

  const UINT32 dummyId = 0;
  int i = 0;
  int alsoAddSymmetricPair;

  pFlexibleSpeakerConfig3d->angularPrecision = __getBits ( bs, dummyId, 1 );

  while ( i < numSpeakers ) {

    int azimuth;

    __ascparserUsacConfig_Usac3dSpeakerDescription ( &pFlexibleSpeakerConfig3d->speakerDescription[i], pFlexibleSpeakerConfig3d->angularPrecision, bs );

    if(pFlexibleSpeakerConfig3d->speakerDescription[i].isCICPspeakerIdx) {
      CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
      cicp2geometry_get_geometry_from_cicp_loudspeaker_index( pFlexibleSpeakerConfig3d->speakerDescription[i].CICPspeakerIdx, &AzElLfe);
      azimuth = AzElLfe.Az;
    }
    else {
      azimuth = ascparser_AzimuthAngleIdxToDegrees ( pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthAngleIdx, pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection, pFlexibleSpeakerConfig3d->angularPrecision );
    }
 
    if ( (azimuth != 0 ) && (azimuth != 180 ) /* && (the symmetric speaker is not yet in the list) */ ) {
    
      alsoAddSymmetricPair  = __getBits ( bs, dummyId, 1 );

      if ( alsoAddSymmetricPair ) {
        /* (also add the speaker with the opposite AzimuthDirection); */
        i++;
        pFlexibleSpeakerConfig3d->speakerDescription[i] = pFlexibleSpeakerConfig3d->speakerDescription[i - 1];
        pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection = 1 - pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection;
      }
    }

    i++;
  }

  return 0;
}

static int __ascparserUsacConfig_Usac3dSpeakerDescription ( Usac3dSpeakerDescription* speakerDescription, int angularPrecision, ascparserBitStreamPtr bs ) {

  const UINT32 dummyId = 0;

  speakerDescription->isCICPspeakerIdx  = __getBits ( bs, dummyId, 1 );

  if ( speakerDescription->isCICPspeakerIdx ) {
    speakerDescription->CICPspeakerIdx  = __getBits ( bs, dummyId, 7 );
  } else {
    speakerDescription->ElevationClass = __getBits ( bs, dummyId, 2 );

    if ( speakerDescription->ElevationClass == 3 ) {
      if ( angularPrecision == 0 ) {
        speakerDescription->ElevationAngleIdx = __getBits ( bs, dummyId, 5 );
      } else {
        speakerDescription->ElevationAngleIdx = __getBits ( bs, dummyId, 7 );
      }

      if ( ascparser_ElevationAngleIdxToDegrees ( speakerDescription->ElevationAngleIdx, speakerDescription->ElevationDirection,  angularPrecision ) != 0 ) {
        speakerDescription->ElevationDirection = __getBits ( bs, dummyId, 1 );
      }
    }

    if ( angularPrecision == 0 ) {
      speakerDescription->AzimuthAngleIdx = __getBits ( bs, dummyId, 6 );
    } else {
      speakerDescription->AzimuthAngleIdx = __getBits ( bs, dummyId, 8 );
    }

    if ( ( ascparser_AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx, speakerDescription->AzimuthDirection, angularPrecision ) != 0 ) &&
         ( ascparser_AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx, speakerDescription->AzimuthDirection, angularPrecision ) != 180 ) ) {
      speakerDescription->AzimuthDirection = __getBits ( bs, dummyId, 1 );
    }

    speakerDescription->isLFE = __getBits ( bs, dummyId, 1 );
  }

  return 0;
}

#endif /* WD1_REFERENCE_LAYOUT_3D */
