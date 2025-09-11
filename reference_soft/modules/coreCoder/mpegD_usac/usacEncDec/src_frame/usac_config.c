/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "allHandles.h"
#include "cicp2geometry.h"
#include "obj_descr.h"                /* structs */
#include "sac_dec_interface.h"
#include "spatial_bitstreamreader.h"
#include "usac_channelconf.h"
#include "bitstream.h"
#include "common_m4a.h"
#include "usac_config.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

#ifndef WD1_ELEMENT_MAPPING_3D
#define WD1_ELEMENT_MAPPING_3D
/*#define WD1_ELEMENT_MAPPING_3D_DEBUG*/
#endif

#ifdef WD1_REFERENCE_LAYOUT_3D
/*#define WD1_REFERENCE_LAYOUT_3D*/
#endif

/*************************************************************************************************************************************************************************************/

#ifdef WD1_REFERENCE_LAYOUT_3D
static int ascparserUsacConfig_Usac3dSpeakerDescription( USAC3DSPEAKERDESCRIPTION* speakerDescription, int angularPrecision, HANDLE_BSBITSTREAM bitStream);
static int ascparserUsacConfig_Usac3dFlexibleSpeakerConfig(USAC3DFLEXIBLESPEAKERCONFIG * pFlexibleSpeakerConfig3d, int numSpeakers, HANDLE_BSBITSTREAM bitStream);
static int ascparserUsacConfig_SpeakerConfig3d(SPEAKERCONFIG3D * pSpeakerConfig3d, HANDLE_BSBITSTREAM bitStream);
#endif

/*************************************************************************************************************************************************************************************/


USAC_SBR_RATIO_INDEX UsacConfig_GetSbrRatioIndex(unsigned int coreSbrFrameLengthIndex){

  USAC_SBR_RATIO_INDEX sbrRatioIndex = -1;

  switch(coreSbrFrameLengthIndex){
  case 0:
  case 1:
    sbrRatioIndex = USAC_SBR_RATIO_INDEX_NO_SBR;
    break;
  case 2:
    sbrRatioIndex = USAC_SBR_RATIO_INDEX_8_3;
    break;
  case 3:
    sbrRatioIndex = USAC_SBR_RATIO_INDEX_2_1;
    break;
  case 4:
    sbrRatioIndex = USAC_SBR_RATIO_INDEX_4_1;
    break;
  default:
    break;
  }  

  return sbrRatioIndex;
}

USAC_OUT_FRAMELENGTH UsacConfig_GetOutputFrameLength(unsigned int coreSbrFrameLengthIndex){

  USAC_OUT_FRAMELENGTH outputFrameLength = -1;

  switch(coreSbrFrameLengthIndex){
  case 0:
    outputFrameLength = USAC_OUT_FRAMELENGTH_768;
    break;
  case 1:
    outputFrameLength = USAC_OUT_FRAMELENGTH_1024;
    break;
  case 2:
  case 3:
    outputFrameLength = USAC_OUT_FRAMELENGTH_2048;
    break;
  case 4:
    outputFrameLength = USAC_OUT_FRAMELENGTH_4096;
    break;
  default:
    break;
  }  

  return outputFrameLength;
}


int UsacConfig_GetMps212NumSlots(unsigned int coreSbrFrameLengthIndex){

  int mps212NumSlots = -1;

  switch(coreSbrFrameLengthIndex){
  case 0:
  case 1:
    break;
  case 2:
  case 3:
    mps212NumSlots = 32;
    break;
  case 4:
    mps212NumSlots = 64;
    break;
  default:
    break;
  }  

  return mps212NumSlots;
}

static void initUsacChannelConfig(USAC_CHANNEL_CONFIG *pUsacChannelConfig, int WriteFlag){

  if(WriteFlag == 0) {
    int i;

    pUsacChannelConfig->numOutChannels = 0;
  
    for(i=0; i<USAC_MAX_NUM_OUT_CHANNELS; i++){
      pUsacChannelConfig->outputChannelPos[i] = USAC_OUTPUT_CHANNEL_POS_NA;
    }
  }

  return;
}

static void initUsacCoreConfig(USAC_CORE_CONFIG *pUsacCoreConfig){

  pUsacCoreConfig->tw_mdct.length                   = 1;
  pUsacCoreConfig->fullbandLPD.length               = 1;
  pUsacCoreConfig->noiseFilling.length              = 1;
  pUsacCoreConfig->enhancedNoiseFilling.length      = 1;
  pUsacCoreConfig->igf_UseENF.length                = 1;
  pUsacCoreConfig->igf_UseHighRes.length            = 1;
  pUsacCoreConfig->igf_UseWhitening.length          = 1;
  pUsacCoreConfig->igf_AfterTnsSynth.length         = 1;
  pUsacCoreConfig->igf_IndependentTiling.length     = 1;
  pUsacCoreConfig->igf_StartIdx.length              = 5;
  pUsacCoreConfig->igf_StopIdx.length               = 4;
}

#ifdef WD1_REFERENCE_LAYOUT_3D

static void initUsacLouspeakerConfig3d(SPEAKERCONFIG3D *pConfig){

  int i;

  pConfig->speakerLayoutType.length                                                   =  2;
  pConfig->CICPspeakerLayoutIdx.length                                                =  6;
  pConfig->numSpeakers.length                                                         = -1; /* escapedValue */

  for(i=0;i<USAC_MAX_NUM_OUT_CHANNELS;i++){
    pConfig->CICPspeakerIdx[i].length                                                 =  7;
  }

  pConfig->flexibleSpeakerConfig.angularPrecision.length                              =  1;

  for(i=0;i<USAC_MAX_NUM_OUT_CHANNELS;i++){

    pConfig->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx.length      =  1;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx.length        =  7;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].ElevationClass.length        =  2;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx.length     =  5;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].ElevationDirection.length    =  1;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx.length       =  6;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection.length      =  1;
    pConfig->flexibleSpeakerConfig.speakerDescription[i].isLFE.length                 =  1;
  }
}

#endif

static void initUsacSbrHeader(USAC_SBR_HEADER *pUsacSbrHeader){
  
  pUsacSbrHeader->start_freq.length     = 4;
  pUsacSbrHeader->stop_freq.length      = 4;
  pUsacSbrHeader->header_extra1.length  = 1;
  pUsacSbrHeader->header_extra2.length  = 1;
  pUsacSbrHeader->freq_scale.length     = 2;
  pUsacSbrHeader->alter_scale.length    = 1;
  pUsacSbrHeader->noise_bands.length    = 2;
  pUsacSbrHeader->limiter_bands.length  = 2;
  pUsacSbrHeader->limiter_gains.length  = 2;
  pUsacSbrHeader->interpol_freq.length  = 1;
  pUsacSbrHeader->smoothing_mode.length = 1;

  return;
}

static void initUsacSbrConfig(USAC_SBR_CONFIG *pUsacSbrConfig){

  pUsacSbrConfig->harmonicSBR.length = 1;
  pUsacSbrConfig->bs_interTes.length = 1;
  pUsacSbrConfig->bs_pvc.length      = 1;
  initUsacSbrHeader(&(pUsacSbrConfig->sbrDfltHeader));

  return;
}

static void initUsacMps212Config(USAC_MPS212_CONFIG *pUsacMps212Config){

  pUsacMps212Config->bsFreqRes.length              = 3;
  pUsacMps212Config->bsFixedGainDMX.length         = 3;
  pUsacMps212Config->bsTempShapeConfig.length      = 2;
  pUsacMps212Config->bsDecorrConfig.length         = 2;
  pUsacMps212Config->bsHighRateMode.length         = 1;
  pUsacMps212Config->bsPhaseCoding.length          = 1;
  pUsacMps212Config->bsOttBandsPhasePresent.length = 1;
  pUsacMps212Config->bsOttBandsPhase.length        = 5;
  pUsacMps212Config->bsResidualBands.length        = 5;
  pUsacMps212Config->bsPseudoLr.length             = 1;
  pUsacMps212Config->bsEnvQuantMode.length         = 1;

  return;
}

static void initUsacSceConfig(USAC_SCE_CONFIG *pUsacSceConfig){

  initUsacCoreConfig(&(pUsacSceConfig->usacCoreConfig));
  initUsacSbrConfig(&(pUsacSceConfig->usacSbrConfig));

  return;
}

static void initUsacCpeConfig(USAC_CPE_CONFIG *pUsacCpeConfig){

  initUsacCoreConfig(&(pUsacCpeConfig->usacCoreConfig));
  initUsacSbrConfig(&(pUsacCpeConfig->usacSbrConfig));
  pUsacCpeConfig->stereoConfigIndex.length = 2;
  initUsacMps212Config(&(pUsacCpeConfig->usacMps212Config));
  pUsacCpeConfig->lpdStereoIndex.length = 1;

  return;
}

static void initUsacLfeConfig(USAC_LFE_CONFIG *pUsacLfeConfig){
  
  initUsacCoreConfig(&(pUsacLfeConfig->usacCoreConfig));
  
  return;
}

static void initUsacExtConfig(USAC_EXT_CONFIG *pUsacExtConfig){
  
  pUsacExtConfig->usacExtElementDefaultLengthPresent.length = 1;
  pUsacExtConfig->usacExtElementPayloadFrag.length          = 1;
  
  return;
}

static void initUsacElementConfig(USAC_ELEMENT_CONFIG *pUsacElementConfig){

  initUsacSceConfig(&(pUsacElementConfig->usacSceConfig));
  initUsacCpeConfig(&(pUsacElementConfig->usacCpeConfig));
  initUsacLfeConfig(&(pUsacElementConfig->usacLfeConfig));
  initUsacExtConfig(&(pUsacElementConfig->usacExtConfig));
  
  return;
}

static void initUsacDecoderConfig(USAC_DECODER_CONFIG *pUsacDecoderConfig, int WriteFlag){
  int i;

  if(WriteFlag == 0) pUsacDecoderConfig->numElements = 0;

  for(i=0; i<USAC_MAX_ELEMENTS; i++){
    if(WriteFlag == 0) pUsacDecoderConfig->usacElementType[i] = USAC_ELEMENT_TYPE_INVALID;
    initUsacElementConfig(&(pUsacDecoderConfig->usacElementConfig[i]));
  }

  return;
}


static void initUsacConfigExtension(USAC_CONFIG_EXTENSION *pUsacConfigExtension){
  unsigned int i;

  COMPATIBLE_PROFILE_LEVEL_SET* CompatibleProfileLevelSet = &(pUsacConfigExtension->usacCompatibleProfileLevelSet);
  CompatibleProfileLevelSet->numCompatibleSets.length = 4;
  CompatibleProfileLevelSet->reserved.length = 4;

  for (i = 0; i < USAC_MAX_COMPATIBLE_PROFILE_LEVELS; i++) {
    CompatibleProfileLevelSet->CompatibleSetIndication[i].length = 8;
  }

  return;
}

void UsacConfig_Init( USAC_CONFIG *usacConf, int WriteFlag )
{
  int i;

  usacConf->mpegh3daProfileLevelIndication.length   =  8;
  usacConf->usacSamplingFrequencyIndex.length       =  5;
  usacConf->usacSamplingFrequency.length            = 24;
  usacConf->coreSbrFrameLengthIndex.length          =  3;
  usacConf->frameLength.length                      =  1;
  usacConf->reservedCoreQmfDelayCompensation.length =  1;
  usacConf->receiverDelayCompensation.length        =  1;

#ifndef WD1_REFERENCE_LAYOUT_3D
  usacConf->channelConfigurationIndex.length        =  7;
#endif

#ifdef WD1_REFERENCE_LAYOUT_3D
  initUsacLouspeakerConfig3d(&(usacConf->referenceLayout));
#else
  initUsacChannelConfig(&(usacConf->usacChannelConfig), WriteFlag);
#endif /* WD1_REFERENCE_LAYOUT_3D */

  for( i = 0; i < USAC_MAX_AUDIO_CHANNEL_LAYOUTS; i++ ) {
    initUsacLouspeakerConfig3d(&(usacConf->frameworkConfig3d.signals3D.audioChannelLayout[i]));
  }

  initUsacDecoderConfig(&(usacConf->usacDecoderConfig), WriteFlag);

  usacConf->usacConfigExtensionPresent.length = 1;
  initUsacConfigExtension(&(usacConf->usacConfigExtension));

  return;
}

const long UsacSamplingFrequencyTable[32] =
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

int UsacConfig_ReadEscapedValue(HANDLE_BSBITSTREAM bitStream, unsigned int *pValue, unsigned int nBits1, unsigned int nBits2, unsigned int nBits3){

  int bitCount = 0;
  unsigned long value = 0;
  unsigned long valueAdd = 0;
  unsigned long maxValue1 = (1 << nBits1) - 1;
  unsigned long maxValue2 = (1 << nBits2) - 1;

  bitCount+=BsRWBitWrapper(bitStream, &value, nBits1, 0);
  if(value == maxValue1){
    bitCount+=BsRWBitWrapper(bitStream, &valueAdd, nBits2, 0);
    value += valueAdd;

    if(valueAdd == maxValue2){
      bitCount+=BsRWBitWrapper(bitStream, &valueAdd, nBits3, 0);
      value += valueAdd;
    }
  }

  *pValue = value;

  return bitCount;
}

static int writeEscapedValue(HANDLE_BSBITSTREAM bitStream, unsigned int value, unsigned int nBits1, unsigned int nBits2, unsigned int nBits3, unsigned int WriteFlag){

  int bitCount = 0;
  unsigned long valueLeft  = value;
  unsigned long valueWrite = 0;
  unsigned long maxValue1 = (1 << nBits1) - 1;
  unsigned long maxValue2 = (1 << nBits2) - 1;
  unsigned long maxValue3 = (1 << nBits3) - 1;

  valueWrite = min(valueLeft, maxValue1);
  bitCount+=BsRWBitWrapper(bitStream, &valueWrite, nBits1, WriteFlag);

  if(valueWrite == maxValue1){
    valueLeft = valueLeft - valueWrite;

    valueWrite = min(valueLeft, maxValue2);
    bitCount+=BsRWBitWrapper(bitStream, &valueWrite, nBits2, WriteFlag);

    if(valueWrite == maxValue2){
      valueLeft = valueLeft - valueWrite;

      valueWrite = min(valueLeft, maxValue3);
      bitCount+=BsRWBitWrapper(bitStream, &valueWrite, nBits3, WriteFlag);
      assert( (valueLeft - valueWrite) == 0 );
    }
  }

  return bitCount;
}

#ifdef WD1_ELEMENT_MAPPING_3D
static int findFirstEmptyIndex(int *chMap, int len)
{
  int i, ret = 0;
  
  /* Find next 'empty (-1)' channel index */
  for(i=0; i<len; i++) {
    if(chMap[i] < 0) break;
    ret++;
  }
  
  return ret;
}

static int getChannelSkip(int *chMap, int bsShift, int nextChannel, int nextEmptyChIdx, int nChannels)
{
  int i,j;
  int ch = 0;
  int end = 0;
  int shift = 0;

  for(i=nextChannel; i<nChannels; i++) {
    for(j=0; j<nChannels; j++) {
      shift++;
      if(chMap[j] == j) {
        shift--;
      }
      if(shift == bsShift+1) {
        end=1;
        break;
      }
    }
    if(end) break;
  }
  
  return shift;
}

static int getChannelIndex(int *chMap, int nextChannel, int shift, int nChannels)
{
  int i;
  int channelMap[USAC_MAX_NUM_OUT_CHANNELS];
  
  memset(channelMap, -1, USAC_MAX_NUM_OUT_CHANNELS*sizeof(int));
  
  for(i=0; i<nChannels; i++) {
    if(chMap[i] < 0) break;
    channelMap[chMap[i]] = chMap[i];
  }
  
  for(i=nextChannel; i<=nextChannel+shift+1; i++) {
    if(channelMap[i] > 0) {
      shift++;
    }
  }
  return nextChannel + shift + 1;
}

static int getNumberOfSkippedChannels(int *chMap, int nextChannel, int nextEmptyChIdx, int nChannels)
{
  int i;
  int ch = 0;
  
  for(i=0; i<nextEmptyChIdx; i++) {
    assert(chMap[i] != nextChannel);
    if(chMap[i] > nextChannel) {
      ch++;
    }
  }
  
  return ch;
}


static int findNextChannel(int *chMap, int len)
{
  int i,j, ch = 0;

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
#endif /* #ifndef WD1_ELEMENT_MAPPING_3D */


static int advanceUsacChannelConfig(HANDLE_BSBITSTREAM bitStream, USAC_CHANNEL_CONFIG *pUsacChannelConfig, int WriteFlag){
  int bitCount = 0;
  
  if(pUsacChannelConfig){
    unsigned int i;

    if(WriteFlag){
      bitCount += writeEscapedValue(bitStream, pUsacChannelConfig->numOutChannels, 5, 8, 16, WriteFlag);
    } else {
      bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacChannelConfig->numOutChannels), 5, 8, 16);
    }

    for(i=0; i<pUsacChannelConfig->numOutChannels; i++){
      unsigned long tmp = pUsacChannelConfig->outputChannelPos[i];
      bitCount+=BsRWBitWrapper(bitStream, &tmp, 5, WriteFlag);
      if(!WriteFlag) pUsacChannelConfig->outputChannelPos[i] = (USAC_OUTPUT_CHANNEL_POS) tmp;
      ObjDescPrintf(WriteFlag, "   audioSpC->usac.usacChannelConfig.outputChannelPos[%d]   : %ld", i, pUsacChannelConfig->outputChannelPos[i]);    
    }
  }

  return bitCount;
}

static int getUsacChannelConfigFromIndex(USAC_CHANNEL_CONFIG *pUsacChannelConfig, unsigned int channelConfigurationIndex){
  int nChannels = -1;

  if(pUsacChannelConfig){
    switch(channelConfigurationIndex)
     {
    case 1:
      pUsacChannelConfig->numOutChannels = 1;
      break;
    case 2:
      pUsacChannelConfig->numOutChannels = 2;
      break;
    case 3:
      pUsacChannelConfig->numOutChannels = 3;
      break;
    case 4:
      pUsacChannelConfig->numOutChannels = 4;
      break;
    case 5:
      pUsacChannelConfig->numOutChannels = 5;
      break;
    case 6:
      pUsacChannelConfig->numOutChannels = 6;
      break;
    case 7:
      pUsacChannelConfig->numOutChannels = 8;
      break;
    case 8:
      pUsacChannelConfig->numOutChannels = 2;
      break;
    case 9:
      pUsacChannelConfig->numOutChannels = 3;
      break;
    case 10:
      pUsacChannelConfig->numOutChannels = 4;
      break;
    case 11:
      pUsacChannelConfig->numOutChannels = 7;
      break;
    case 12:
      pUsacChannelConfig->numOutChannels = 8;
      break;
    case 13:
      pUsacChannelConfig->numOutChannels = 24;
      break;
    case 14:
    case 15:
      pUsacChannelConfig->numOutChannels = 24;
      break;

    
    case 17:
    case 18:
    case 19:
      pUsacChannelConfig->numOutChannels = 14;
      break;

    case 20:
    case 21:
    case 22:
      pUsacChannelConfig->numOutChannels = 12;
      break;

    case 23:
    case 24:
    case 25:
      pUsacChannelConfig->numOutChannels = 9;
      break;

    case 31:
      pUsacChannelConfig->numOutChannels = 0;  /* Objects only */
      break;
      
    case 42:
      pUsacChannelConfig->numOutChannels = 14;
      break;
    case 43:
      pUsacChannelConfig->numOutChannels = 12;
      break;
    case 44:
      pUsacChannelConfig->numOutChannels = 9;
      break;

    default:
      assert(0); 
      break;
    }
  }

  return nChannels;
}

static int advanceSbrHeader(HANDLE_BSBITSTREAM bitStream, USAC_SBR_HEADER *pSbrHeader, int WriteFlag){

  int bitCount = 0;

  /* start_freq */
  bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->start_freq.value), pSbrHeader->start_freq.length, WriteFlag);

  /* stop_freq */
  bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->stop_freq.value), pSbrHeader->stop_freq.length, WriteFlag);

  /* header_extra1 */
  bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->header_extra1.value), pSbrHeader->header_extra1.length, WriteFlag);

  /* header_extra2 */
  bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->header_extra2.value), pSbrHeader->header_extra2.length, WriteFlag);

  if(pSbrHeader->header_extra1.value){
    
    /* freq_scale */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->freq_scale.value), pSbrHeader->freq_scale.length, WriteFlag);

    /* alter_scale */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->alter_scale.value), pSbrHeader->alter_scale.length, WriteFlag);

    /* noise_bands */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->noise_bands.value), pSbrHeader->noise_bands.length, WriteFlag);
  }

  if(pSbrHeader->header_extra2.value){

    /* limiter_bands */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->limiter_bands.value), pSbrHeader->limiter_bands.length, WriteFlag);

    /* limiter_gains */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->limiter_gains.value), pSbrHeader->limiter_gains.length, WriteFlag);

    /* interpol_freq */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->interpol_freq.value), pSbrHeader->interpol_freq.length, WriteFlag);

    /* smoothing_mode */
    bitCount += BsRWBitWrapper(bitStream, &(pSbrHeader->smoothing_mode.value), pSbrHeader->smoothing_mode.length, WriteFlag);
  }

  return bitCount;
}

static int advanceUsacSbrConfig(HANDLE_BSBITSTREAM bitStream, USAC_SBR_CONFIG *pUsacSbrConfig, int WriteFlag){

  int bitCount = 0;

  /* harmonicSBR */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacSbrConfig->harmonicSBR.value), pUsacSbrConfig->harmonicSBR.length, WriteFlag);

  /* bs_interTes */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacSbrConfig->bs_interTes.value), pUsacSbrConfig->bs_interTes.length, WriteFlag);

  /* bs_pvc */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacSbrConfig->bs_pvc.value), pUsacSbrConfig->bs_pvc.length, WriteFlag);

  /* SbrDfltHeader */
  bitCount += advanceSbrHeader(bitStream, &(pUsacSbrConfig->sbrDfltHeader), WriteFlag);

  return bitCount;
}

static int advanceUsacCoreConfig(HANDLE_BSBITSTREAM bitStream, USAC_CORE_CONFIG *pUsacCoreConfig, int WriteFlag){

  int bitCount = 0;

  /* tw_mdct */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->tw_mdct.value), pUsacCoreConfig->tw_mdct.length, WriteFlag);

  /* fullband LPD core */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->fullbandLPD.value), pUsacCoreConfig->fullbandLPD.length, WriteFlag);

  /* noiseFilling */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->noiseFilling.value), pUsacCoreConfig->noiseFilling.length, WriteFlag);

  /* intelligent gap filling */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->enhancedNoiseFilling.value), pUsacCoreConfig->enhancedNoiseFilling.length, WriteFlag);

  if (pUsacCoreConfig->enhancedNoiseFilling.value) {
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_UseENF.value), pUsacCoreConfig->igf_UseENF.length, WriteFlag);
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_UseHighRes.value), pUsacCoreConfig->igf_UseHighRes.length, WriteFlag);
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_UseWhitening.value), pUsacCoreConfig->igf_UseWhitening.length, WriteFlag);
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_AfterTnsSynth.value), pUsacCoreConfig->igf_AfterTnsSynth.length, WriteFlag);
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_StartIdx.value), pUsacCoreConfig->igf_StartIdx.length, WriteFlag);
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCoreConfig->igf_StopIdx.value), pUsacCoreConfig->igf_StopIdx.length, WriteFlag);
  }

  return bitCount;
}

static int advanceUsacSceConfig(HANDLE_BSBITSTREAM bitStream, USAC_SCE_CONFIG *pUsacSceConfig, int sbrRatioIndex, int *outputChannelPos, int WriteFlag){

  int bitCount = 0;
#ifdef WD1_ELEMENT_MAPPING_3D
  int nextEmptyChIdx;
  int nextChannel;
#endif

  bitCount += advanceUsacCoreConfig(bitStream, &(pUsacSceConfig->usacCoreConfig), WriteFlag);

  if(pUsacSceConfig->usacCoreConfig.enhancedNoiseFilling.value) {
    pUsacSceConfig->usacCoreConfig.igf_IndependentTiling.value = 1;
  }

  if(sbrRatioIndex > 0){
    bitCount += advanceUsacSbrConfig(bitStream, &(pUsacSceConfig->usacSbrConfig), WriteFlag);
  }

#ifdef WD1_ELEMENT_MAPPING_3D
  nextChannel = findNextChannel(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  nextEmptyChIdx = findFirstEmptyIndex(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  outputChannelPos[nextEmptyChIdx] = nextChannel;
#endif
  return bitCount;
}

static int advanceUsacLfeConfig(HANDLE_BSBITSTREAM bitStream, USAC_LFE_CONFIG *pUsacLfeConfig, int *outputChannelPos, int WriteFlag){

  int bitCount = 0;
#ifdef WD1_ELEMENT_MAPPING_3D
  int nextEmptyChIdx;
  int nextChannel;
#endif

  if(!WriteFlag){
    pUsacLfeConfig->usacCoreConfig.tw_mdct.value = 0;
    pUsacLfeConfig->usacCoreConfig.noiseFilling.value = 0;
  }

#ifdef WD1_ELEMENT_MAPPING_3D
  nextChannel = findNextChannel(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  nextEmptyChIdx = findFirstEmptyIndex(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  outputChannelPos[nextEmptyChIdx] = nextChannel;
#endif
  return bitCount;
}

static int advanceUsacExtConfig(HANDLE_BSBITSTREAM bitStream, USAC_EXT_CONFIG *pUsacExtConfig, int WriteFlag){

  int bitCount = 0;
  unsigned int i;
  unsigned int nBytesPayloadRead = 0;
  unsigned int dummy = 0;
  DESCR_ELE tmp;
  tmp.length = 8;

  /* usacExtElementType */
  if(WriteFlag){
    bitCount += writeEscapedValue(bitStream, pUsacExtConfig->usacExtElementType, 4, 8, 16, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &dummy, 4, 8, 16);
    pUsacExtConfig->usacExtElementType = (USAC_ID_EXT_ELE)dummy;
  }

  /* usacExtElementConfigLength */
  if(WriteFlag){
    bitCount += writeEscapedValue(bitStream, pUsacExtConfig->usacExtElementConfigLength, 4, 8, 16, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacExtConfig->usacExtElementConfigLength), 4, 8, 16);
  }

  /* usacExtElementDefaultLengthPresent */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacExtConfig->usacExtElementDefaultLengthPresent.value), pUsacExtConfig->usacExtElementDefaultLengthPresent.length, WriteFlag);

  if(pUsacExtConfig->usacExtElementDefaultLengthPresent.value){
    if(WriteFlag){
      bitCount += writeEscapedValue(bitStream, pUsacExtConfig->usacExtElementDefaultLength-1, 8, 16, 0, WriteFlag);
    } else {
      bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacExtConfig->usacExtElementDefaultLength), 8, 16, 0);
      pUsacExtConfig->usacExtElementDefaultLength += 1;
    }
  } else {
    if(!WriteFlag) pUsacExtConfig->usacExtElementDefaultLength = 0;
  }
    
  /* usacExtElementDefaultLengthPresent */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacExtConfig->usacExtElementPayloadFrag.value), pUsacExtConfig->usacExtElementPayloadFrag.length, WriteFlag);

  switch(pUsacExtConfig->usacExtElementType){
  case USAC_ID_EXT_ELE_FILL:
#if IAR
  case USAC_ID_EXT_ELE_FMT_CNVTR:
#endif
    /* no configuration payload */
    break;
  default:
    for (i = 0; i < pUsacExtConfig->usacExtElementConfigLength; i++) {
      if (!WriteFlag) {
        unsigned long tmp = 0;
        bitCount += BsRWBitWrapper(bitStream, &tmp, 8, WriteFlag);
        pUsacExtConfig->usacExtElementConfigPayload[i] = (unsigned char) tmp & 0xFF;
      } else {
        unsigned long data = pUsacExtConfig->usacExtElementConfigPayload[i];
        bitCount += BsRWBitWrapper(bitStream, &data, 8, WriteFlag);
      }
    }
    nBytesPayloadRead = i;
    break;
  }

  assert(nBytesPayloadRead == pUsacExtConfig->usacExtElementConfigLength);

  return bitCount;
}

static int advanceUsacMps212Config(HANDLE_BSBITSTREAM bitStream, USAC_MPS212_CONFIG *pUsacMps212Config, int stereoConfigIndex, int WriteFlag){

  int bitCount = 0;
  int bsResidualCoding = (stereoConfigIndex > 1)?1:0;

  /* bsFreqRes */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsFreqRes.value), pUsacMps212Config->bsFreqRes.length, WriteFlag);
  
  /* bsFixedGainDMX */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsFixedGainDMX.value), pUsacMps212Config->bsFixedGainDMX.length, WriteFlag);

  /* bsTempShapeConfig */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsTempShapeConfig.value), pUsacMps212Config->bsTempShapeConfig.length, WriteFlag);

  /* bsDecorrConfig */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsDecorrConfig.value), pUsacMps212Config->bsDecorrConfig.length, WriteFlag);

  /* bsHighRateMode */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsHighRateMode.value), pUsacMps212Config->bsHighRateMode.length, WriteFlag);

  /* bsPhaseCoding */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsPhaseCoding.value), pUsacMps212Config->bsPhaseCoding.length, WriteFlag);

  /* bsOttBandsPhasePresent */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsOttBandsPhasePresent.value), pUsacMps212Config->bsOttBandsPhasePresent.length, WriteFlag);

  if(pUsacMps212Config->bsOttBandsPhasePresent.value){
    /* bsOttBandsPhase */
    bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsOttBandsPhase.value), pUsacMps212Config->bsOttBandsPhase.length, WriteFlag);
  }

  if(bsResidualCoding){
    /* bsResidualBands */
    bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsResidualBands.value), pUsacMps212Config->bsResidualBands.length, WriteFlag);
    if(!WriteFlag) pUsacMps212Config->bsOttBandsPhase.value = max(pUsacMps212Config->bsOttBandsPhase.value, pUsacMps212Config->bsResidualBands.value);

    /* bsPseudoLr */
    bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsPseudoLr.value), pUsacMps212Config->bsPseudoLr.length, WriteFlag);
  }

  if(pUsacMps212Config->bsTempShapeConfig.value == 2){
    /* bsResidualBands */
    bitCount += BsRWBitWrapper(bitStream, &(pUsacMps212Config->bsEnvQuantMode.value), pUsacMps212Config->bsEnvQuantMode.length, WriteFlag);
  }

  return bitCount;
}

static int advanceUsacCpeConfig(HANDLE_BSBITSTREAM bitStream, USAC_CPE_CONFIG *pUsacCpeConfig, int sbrRatioIndex, int numOutChannels, int *outputChannelPos, int WriteFlag){

  int bitCount = 0;
#ifdef WD1_ELEMENT_MAPPING_3D
  int nextEmptyChIdx = 0;
  int nextChannel = 0;
  int nChannelsSkipped = 0;
  UINT32 shift = 0;
#endif

  bitCount += advanceUsacCoreConfig(bitStream, &(pUsacCpeConfig->usacCoreConfig), WriteFlag);

  if(pUsacCpeConfig->usacCoreConfig.enhancedNoiseFilling.value) {
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCpeConfig->usacCoreConfig.igf_IndependentTiling.value), pUsacCpeConfig->usacCoreConfig.igf_IndependentTiling.length, WriteFlag);
  }

  if(sbrRatioIndex > 0){
    bitCount += advanceUsacSbrConfig(bitStream, &(pUsacCpeConfig->usacSbrConfig), WriteFlag);

    /* stereoConfigIndex */
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCpeConfig->stereoConfigIndex.value), pUsacCpeConfig->stereoConfigIndex.length, WriteFlag);
  } else {
    if(!WriteFlag) pUsacCpeConfig->stereoConfigIndex.value = 0;
  }

  if(pUsacCpeConfig->stereoConfigIndex.value > 0){
    bitCount += advanceUsacMps212Config(bitStream, &(pUsacCpeConfig->usacMps212Config), pUsacCpeConfig->stereoConfigIndex.value, WriteFlag);
  }
  
  /* qceIndex, 2 bit */
  bitCount += BsRWBitWrapper(bitStream, &(pUsacCpeConfig->qceFlag), 2, WriteFlag);
#ifdef WD1_ELEMENT_MAPPING_3D_DEBUG
  printf("\nQCE: %d", pUsacCpeConfig->qceFlag);
#endif

#ifdef WD1_ELEMENT_MAPPING_3D
  nextEmptyChIdx = findFirstEmptyIndex(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  nextChannel = findNextChannel(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
  nChannelsSkipped = getNumberOfSkippedChannels(outputChannelPos, nextChannel, nextEmptyChIdx, numOutChannels);
  if(pUsacCpeConfig->qceFlag > 0) {
    UINT32 doShift = 0;
    int bitsToRead=0;
    bitCount += BsRWBitWrapper(bitStream, &(doShift), 1, WriteFlag);
    if(doShift > 0) {
      /* Fixed number of bits for channel shift */
      int len = numOutChannels - 1;
      while(len>0) {
        bitsToRead++;
        len>>=1;
      }
      bitCount += BsRWBitWrapper(bitStream, &shift, bitsToRead, WriteFlag);
#ifdef WD1_ELEMENT_MAPPING_3D_DEBUG
      printf("\nRead CPE 0: %d %d %d", doShift, bitsToRead, shift);
#endif
      nextChannel = getChannelIndex(outputChannelPos, nextChannel, shift, numOutChannels);
    }
  }
  outputChannelPos[nextEmptyChIdx] = nextChannel;
  
  {
    UINT32 doShift = 0;
    int bitsToRead=0;
    bitCount += BsRWBitWrapper(bitStream, &(doShift), 1, WriteFlag);
    nextEmptyChIdx = findFirstEmptyIndex(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
    nextChannel = findNextChannel(outputChannelPos, USAC_MAX_NUM_OUT_CHANNELS);
    nChannelsSkipped = getNumberOfSkippedChannels(outputChannelPos, nextChannel, nextEmptyChIdx, numOutChannels);
    if(doShift > 0) {
      /* Fixed number of bits for channel shift */
      int len = numOutChannels - 1;
      while(len>0) {
        bitsToRead++;
        len>>=1;
      }
      bitCount += BsRWBitWrapper(bitStream, &shift, bitsToRead, WriteFlag);
#ifdef WD1_ELEMENT_MAPPING_3D_DEBUG
      printf("\nRead CPE 1: %d %d %d", doShift, bitsToRead, shift);
#endif
      nextChannel = getChannelIndex(outputChannelPos, nextChannel, shift, numOutChannels);
    }
    outputChannelPos[nextEmptyChIdx] = nextChannel;
  }
#endif

  if((sbrRatioIndex == 0) && (pUsacCpeConfig->qceFlag == 0)){
    bitCount += BsRWBitWrapper(bitStream, &(pUsacCpeConfig->lpdStereoIndex.value), pUsacCpeConfig->lpdStereoIndex.length, WriteFlag);
  }

  return bitCount;
}

#ifdef WD1_FRAMEWORK_CONFIG_3D

static int advanceSignals3d(HANDLE_BSBITSTREAM bitStream, SIGNALS_3D *pSignals3d, int WriteFlag)
{
  int bitCount = 0;

#ifdef CD_SIGNALS_3D_SIGNAL_GROUPS

  UINT32 numSignalGroups = 0;
  
  if(WriteFlag) {
    UINT32 numSignalGroups = -1;
    UINT32 signalGroupType;
    int numSignals;
    UINT32 differsFromReferenceLayout = 0;

    /* count groups */
    numSignalGroups += pSignals3d->numAudioChannels         > 0 ? 1 : 0;
    numSignalGroups += pSignals3d->numAudioObjects          > 0 ? 1 : 0;
    numSignalGroups += pSignals3d->numSAOCTransportChannels > 0 ? 1 : 0;
    numSignalGroups += pSignals3d->numHOATransportChannels  > 0 ? 1 : 0;

    bitCount += BsRWBitWrapper(bitStream, &numSignalGroups, 5, WriteFlag);

    if(pSignals3d->numAudioChannels > 0) {
      signalGroupType = 0;
      bitCount += BsRWBitWrapper(bitStream, &signalGroupType, 3, WriteFlag);

      numSignals = pSignals3d->numAudioChannels - 1;
      bitCount += writeEscapedValue(bitStream, numSignals, 5, 8, 16, WriteFlag);

      bitCount += BsRWBitWrapper(bitStream, &differsFromReferenceLayout, 1, WriteFlag);
    }
    if(pSignals3d->numAudioObjects > 0) {
      signalGroupType = 1;
      bitCount += BsRWBitWrapper(bitStream, &signalGroupType, 3, WriteFlag);

      numSignals = pSignals3d->numAudioObjects - 1;
      bitCount += writeEscapedValue(bitStream, numSignals, 5, 8, 16, WriteFlag);
    }
    if(pSignals3d->numSAOCTransportChannels > 0) {
      signalGroupType = 2;
      bitCount += BsRWBitWrapper(bitStream, &signalGroupType, 3, WriteFlag);

      numSignals = pSignals3d->numSAOCTransportChannels - 1;
      bitCount += writeEscapedValue(bitStream, numSignals, 5, 8, 16, WriteFlag);
    }
    if(pSignals3d->numHOATransportChannels > 0) {
      signalGroupType = 3;
      bitCount += BsRWBitWrapper(bitStream, &signalGroupType, 3, WriteFlag);

      numSignals = pSignals3d->numHOATransportChannels - 1;
      bitCount += writeEscapedValue(bitStream, numSignals, 5, 8, 16, WriteFlag);
    }  
  }
  else{

    UINT32 i;
    UINT32 signalGroupType;
    unsigned int numberOfSignals;
    UINT32 differsFromReferenceLayout;
    int audioChannelLayoutCounter = 0;

    pSignals3d->numAudioChannels = 0;
    pSignals3d->numAudioObjects = 0;
    pSignals3d->numSAOCTransportChannels = 0;
    pSignals3d->numHOATransportChannels = 0;

    bitCount += BsRWBitWrapper(bitStream, &numSignalGroups, 5, WriteFlag);
    pSignals3d->m_numSignalGroups = numSignalGroups;
    for ( i = 0; i < numSignalGroups + 1; i++ ) {

      bitCount += BsRWBitWrapper(bitStream, &signalGroupType, 3, WriteFlag);
      pSignals3d->m_signalGroupType[i] = signalGroupType;

      bitCount += UsacConfig_ReadEscapedValue(bitStream, &numberOfSignals, 5, 8, 16);
      pSignals3d->m_numberOfSignals[i] = numberOfSignals;

      switch( signalGroupType ) {
        case 0:
          pSignals3d->numAudioChannels += numberOfSignals + 1; /* "+=" because there could be more than one instance */

          bitCount += BsRWBitWrapper(bitStream, &differsFromReferenceLayout, 1, WriteFlag);
          if ( differsFromReferenceLayout ) {
               bitCount += ascparserUsacConfig_SpeakerConfig3d(&(pSignals3d->audioChannelLayout[audioChannelLayoutCounter]), bitStream);
          }
          audioChannelLayoutCounter++;

          break;
        case 1:
          pSignals3d->numAudioObjects += numberOfSignals + 1;
          break;
        case 2:
          pSignals3d->numSAOCTransportChannels += numberOfSignals + 1;
#ifdef SUPPORT_SAOC_DMX_LAYOUT
          bitCount += BsRWBitWrapper(bitStream, &pSignals3d->saocDmxLayoutPresent, 1, WriteFlag);
          if ( pSignals3d->saocDmxLayoutPresent) {
            bitCount += ascparserUsacConfig_SpeakerConfig3d(&(pSignals3d->saocDmxChannelLayout), bitStream);
          }
#endif

          break;
        case 3:
          pSignals3d->numHOATransportChannels += numberOfSignals + 1;
          break;
      }
    }
  }
#else

  if(WriteFlag) {
    bitCount += writeEscapedValue(bitStream, pSignals3d->numAudioChannels, 5, 8, 16, WriteFlag);
    bitCount += writeEscapedValue(bitStream, pSignals3d->numAudioObjects, 5, 8, 16, WriteFlag);
    bitCount += writeEscapedValue(bitStream, pSignals3d->numSAOCTransportChannels, 5, 8, 16, WriteFlag);
    bitCount += writeEscapedValue(bitStream, pSignals3d->numHOATransportChannels, 5, 8, 16, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pSignals3d->numAudioChannels), 5, 8, 16);
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pSignals3d->numAudioObjects), 5, 8, 16);
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pSignals3d->numSAOCTransportChannels), 5, 8, 16);
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pSignals3d->numHOATransportChannels), 5, 8, 16);
  }
#endif  
  return bitCount;
}

static int advanceFrameworkConfig3d(HANDLE_BSBITSTREAM bitStream, FRAMEWORK_CONFIG_3D *pFrameworkConfig3d, int WriteFlag)
{
  int bitCount = 0;
  
  bitCount += advanceSignals3d(bitStream, &(pFrameworkConfig3d->signals3D), WriteFlag);
  
  return bitCount;
}
#endif /* #ifdef WD1_FRAMEWORK_CONFIG_3D */

static int advanceUsacDecoderConfig(HANDLE_BSBITSTREAM bitStream, USAC_DECODER_CONFIG *pUsacDecoderConfig, int sbrRatioIndex, SIGNALS_3D *pSignals3D, int WriteFlag){

  int bitCount = 0;
  unsigned int elemIdx = 0;
  int numOutChannels;
  int *outputChannelIndex = NULL;

  
  numOutChannels = pSignals3D->numAudioChannels + pSignals3D->numAudioObjects + pSignals3D->numHOATransportChannels + pSignals3D->numSAOCTransportChannels;
  outputChannelIndex = pUsacDecoderConfig->usacChannelIndex;
  
  /* initialize to -1 */
  memset(outputChannelIndex, -1, USAC_MAX_NUM_OUT_CHANNELS*sizeof(int));

  if(WriteFlag){
    bitCount += writeEscapedValue(bitStream, pUsacDecoderConfig->numElements-1, 4, 8, 16, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacDecoderConfig->numElements), 4, 8, 16);
    pUsacDecoderConfig->numElements += 1;
  }

  bitCount += BsRWBitWrapper(bitStream, &(pUsacDecoderConfig->usacElementLengthPresent), 1, WriteFlag);

  for(elemIdx = 0; elemIdx < pUsacDecoderConfig->numElements; elemIdx++){
    unsigned long tmp = pUsacDecoderConfig->usacElementType[elemIdx];
    bitCount += BsRWBitWrapper(bitStream, &tmp, 2, WriteFlag);
    if(!WriteFlag) pUsacDecoderConfig->usacElementType[elemIdx] = (USAC_ELEMENT_TYPE) tmp;
#ifdef WD1_ELEMENT_MAPPING_3D_DEBUG
    printf("\n Element: %d", tmp);
#endif

    switch(pUsacDecoderConfig->usacElementType[elemIdx]){
    case USAC_ELEMENT_TYPE_SCE:
      bitCount += advanceUsacSceConfig(bitStream, &(pUsacDecoderConfig->usacElementConfig[elemIdx].usacSceConfig), sbrRatioIndex, outputChannelIndex, WriteFlag);
      break;
    case USAC_ELEMENT_TYPE_CPE:
      bitCount += advanceUsacCpeConfig(bitStream, &(pUsacDecoderConfig->usacElementConfig[elemIdx].usacCpeConfig), sbrRatioIndex, numOutChannels, outputChannelIndex, WriteFlag);
      break;
    case USAC_ELEMENT_TYPE_LFE:
      bitCount += advanceUsacLfeConfig(bitStream, &(pUsacDecoderConfig->usacElementConfig[elemIdx].usacLfeConfig), outputChannelIndex, WriteFlag);
      break;
    case USAC_ELEMENT_TYPE_EXT:
      bitCount += advanceUsacExtConfig(bitStream, &(pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig), WriteFlag);
      break;
    default:
      assert(0);
      break;
    }

    if ((0 != elemIdx) && (USAC_ID_EXT_ELE_AUDIOPREROLL == pUsacDecoderConfig->usacElementConfig[elemIdx].usacExtConfig.usacExtElementType)) {
      /* ISO/IEC 23008-3: 5.5.6 Audio pre-roll: The first element of every frame shall be an extension element (mpegh3daExtElement) of type ID_EXT_ELE_AUDIOPREROLL. */
      fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
      fprintf(stderr, "\n!!WARNING!!  mpegh3daExtElement of type ID_EXT_ELE_AUDIOPREROLL shall be the first element in the stream!");
      fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
      fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
      fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
    }
  }
  
  pUsacDecoderConfig->numChannels = numOutChannels;

  return bitCount;
}

static int advanceUsacConfigExtension_CompatibleProfileLevelSet(HANDLE_BSBITSTREAM bitStream, COMPATIBLE_PROFILE_LEVEL_SET* const pCompatibleProfileLevelSet, int WriteFlag) {
  int bitCount = 0;

  if (pCompatibleProfileLevelSet != NULL) {
    unsigned int i = 0;
    unsigned long reserved = 0;

    if (WriteFlag) {
      const unsigned long bsNumCompatibleSets = (unsigned long)(pCompatibleProfileLevelSet->numCompatibleSets.value) - 1;
      bitCount += BsRWBitWrapper(bitStream, &bsNumCompatibleSets, pCompatibleProfileLevelSet->numCompatibleSets.length, WriteFlag);
    } else {
      unsigned long bsNumCompatibleSets;
      bitCount += BsRWBitWrapper(bitStream, &bsNumCompatibleSets, pCompatibleProfileLevelSet->numCompatibleSets.length, WriteFlag);
      pCompatibleProfileLevelSet->numCompatibleSets.value = (UINT32)(bsNumCompatibleSets + 1);
    }

    bitCount += BsRWBitWrapper(bitStream, &(pCompatibleProfileLevelSet->reserved.value), pCompatibleProfileLevelSet->reserved.length, WriteFlag);

    for (i = 0; i < pCompatibleProfileLevelSet->numCompatibleSets.value; i++) {
      bitCount += BsRWBitWrapper(bitStream, &(pCompatibleProfileLevelSet->CompatibleSetIndication[i].value), pCompatibleProfileLevelSet->CompatibleSetIndication[i].length, WriteFlag);
    }
  }

  return bitCount;
}

static int advanceUsacConfigExtension(HANDLE_BSBITSTREAM bitStream, USAC_CONFIG_EXTENSION *pUsacConfigExtension, int WriteFlag){
  int bitCount = 0;
  unsigned int confExtIdx;
  unsigned int i;
  unsigned int dummy = 0;

  if(WriteFlag){
    bitCount += writeEscapedValue(bitStream, pUsacConfigExtension->numConfigExtensions-1, 2, 4, 8, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacConfigExtension->numConfigExtensions), 2, 4, 8);
    pUsacConfigExtension->numConfigExtensions += 1;
  }  

  for(confExtIdx = 0; confExtIdx < pUsacConfigExtension->numConfigExtensions; confExtIdx++){
    unsigned long tmp;
    unsigned long fillByte = 0xa5;
    /* usacConfigExtType */
    if(WriteFlag){
      bitCount += writeEscapedValue(bitStream, pUsacConfigExtension->usacConfigExtType[confExtIdx], 4, 8, 16, WriteFlag);
    } else {
      bitCount += UsacConfig_ReadEscapedValue(bitStream, &dummy, 4, 8, 16);
      pUsacConfigExtension->usacConfigExtType[confExtIdx] = (USAC_CONFIG_EXT_TYPE)dummy;
    }  

    /* usacConfigExtLength */
    if(WriteFlag){
      bitCount += writeEscapedValue(bitStream, pUsacConfigExtension->usacConfigExtLength[confExtIdx], 4, 8, 16, WriteFlag);
    } else {
      bitCount += UsacConfig_ReadEscapedValue(bitStream, &(pUsacConfigExtension->usacConfigExtLength[confExtIdx]), 4, 8, 16);
    }  

    switch(pUsacConfigExtension->usacConfigExtType[confExtIdx]){
    case USAC_CONFIG_EXT_TYPE_FILL:
      for(i=0; i<pUsacConfigExtension->usacConfigExtLength[confExtIdx]; i++){
        bitCount += BsRWBitWrapper(bitStream, &fillByte, 8, WriteFlag);
        if(!WriteFlag) assert(fillByte == 0xa5);
      }
      break;
#ifdef RM6_ICG_SYNTAX
    case USAC_CONFIG_EXT_ICG_CONFIG:
      {
        unsigned long totalValue = 0;
        unsigned long icg_val = 0;

        for(i=0; i<pUsacConfigExtension->usacConfigExtLength[confExtIdx]; i++){
          bitCount += BsRWBitWrapper(bitStream, &icg_val, 8, WriteFlag);
          (ICConfig.bitstream[i]) = (unsigned char)icg_val;
        }
      }
      break;
#endif
    case USAC_CONFIG_EXT_COMPATIBLE_PROFILELVL_SET:
      bitCount += advanceUsacConfigExtension_CompatibleProfileLevelSet(bitStream, &(pUsacConfigExtension->usacCompatibleProfileLevelSet), WriteFlag);
      break;
    default:
      for(i=0; i<pUsacConfigExtension->usacConfigExtLength[confExtIdx]; i++){
        bitCount += BsRWBitWrapper(bitStream, &tmp, 8, WriteFlag);
      }
      break;
    }
  }

  return bitCount;
}


int UsacConfig_Advance(HANDLE_BSBITSTREAM bitStream, USAC_CONFIG *usacConf, int WriteFlag)
{
  int bitCount = 0;

  /*****************************************************************************************************************************************/
  /*                                                                                                                                       */
  /* General information, relevant for playout etc.                                                                                        */
  /*                                                                                                                                       */ 


  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->mpegh3daProfileLevelIndication.value), usacConf->mpegh3daProfileLevelIndication.length, WriteFlag);

  /* sampling rate */
  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->usacSamplingFrequencyIndex.value), usacConf->usacSamplingFrequencyIndex.length, WriteFlag);
  ObjDescPrintf(WriteFlag, "   audioSpC->usac.usacSamplingFrequencyIndex   : %ld", usacConf->usacSamplingFrequencyIndex.value);

  if(usacConf->usacSamplingFrequencyIndex.value == 0x1f){
    bitCount+=BsRWBitWrapper(bitStream, &(usacConf->usacSamplingFrequency.value), usacConf->usacSamplingFrequency.length, WriteFlag);
  } else {
    if(!WriteFlag){
      usacConf->usacSamplingFrequency.value = UsacSamplingFrequencyTable[usacConf->usacSamplingFrequencyIndex.value];
    }
  }

  /* frame length, SBR on/off */
  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->coreSbrFrameLengthIndex.value), usacConf->coreSbrFrameLengthIndex.length, WriteFlag);
  ObjDescPrintf(WriteFlag, "   audioSpC->usac.coreSbrFrameLengthIndex   : %ld", usacConf->coreSbrFrameLengthIndex.value);
  if(!WriteFlag) usacConf->frameLength.value = 0; 
  usacConf->frameLength.value = 0; 

  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->reservedCoreQmfDelayCompensation.value), usacConf->reservedCoreQmfDelayCompensation.length, WriteFlag);

  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->receiverDelayCompensation.value), usacConf->receiverDelayCompensation.length, WriteFlag);

#ifdef SUPPORT_DAM3_CONSTANT_DELAY
  if ( usacConf->receiverDelayCompensation.value ) {
    usacConf->reservedCoreQmfDelayCompensation.value = 1;
  }
  else {
    usacConf->reservedCoreQmfDelayCompensation.value = 0;
  }
#endif

#ifndef WD1_REFERENCE_LAYOUT_3D

  /* channel configuration */
  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->channelConfigurationIndex.value), usacConf->channelConfigurationIndex.length, WriteFlag);
  ObjDescPrintf(WriteFlag, "   audioSpC->usac.channelConfigurationIndex   : %ld", usacConf->channelConfigurationIndex.value);

  if(usacConf->channelConfigurationIndex.value == 0){
    bitCount += advanceUsacChannelConfig(bitStream, &(usacConf->usacChannelConfig), WriteFlag);
  } else {
    if(!WriteFlag) getUsacChannelConfigFromIndex(&(usacConf->usacChannelConfig), usacConf->channelConfigurationIndex.value);
  }

#else /* WD1_REFERENCE_LAYOUT_3D */
  /*****************************************************************************************************************************************/
  /*                                                                                                                                       */
  /* Reference Layout 3D                                                                                                                   */
  /*                                                                                                                                       */ 

  if(!WriteFlag){
    bitCount += ascparserUsacConfig_SpeakerConfig3d(&(usacConf->referenceLayout), bitStream);
  }
  else{
    assert(usacConf->referenceLayout.speakerLayoutType.value == 0); /* shall be 0 (0 means CICPspeakerLayoutIdx is signalized), otherwise not supported */
    bitCount += BsRWBitWrapper(bitStream, &(usacConf->referenceLayout.speakerLayoutType.value), usacConf->referenceLayout.speakerLayoutType.length, WriteFlag); 
    bitCount += BsRWBitWrapper(bitStream, &(usacConf->referenceLayout.CICPspeakerLayoutIdx.value), usacConf->referenceLayout.CICPspeakerLayoutIdx.length, WriteFlag);
  }

#endif /* WD1_REFERENCE_LAYOUT_3D */


#ifndef CD_SIGNALS_3D_SIGNAL_GROUPS
#ifdef WD1_ESCAPED_OBJECTS_MAPPING_3D
  /* 3DA: numObjects = 0; 8bit */ 
  if(WriteFlag){
    bitCount += writeEscapedValue(bitStream, usacConf->numObjects.value, 5, 8, 16, WriteFlag);
  } else {
    bitCount += UsacConfig_ReadEscapedValue(bitStream, &(usacConf->numObjects.value), 5, 8, 16);
  }  
#else
  bitCount+=BsRWBitWrapper(bitStream, &(usacConf->numObjects.value), usacConf->numObjects.length, WriteFlag);
#endif

#ifdef WD1_REFERENCE_LAYOUT_3D

  if(usacConf->numObjects.value > 0) {

    DESCR_ELE differsFromReferenceLayout;
    differsFromReferenceLayout.value = 0;
    differsFromReferenceLayout.length = 1;
    
    if(!WriteFlag) {

      bitCount += BsRWBitWrapper(bitStream, &(differsFromReferenceLayout.value), differsFromReferenceLayout.length, WriteFlag);

      if(differsFromReferenceLayout.value == 1) {
        bitCount += ascparserUsacConfig_SpeakerConfig3d(&(usacConf->audioChannelLayout), bitStream);
      }
      else {
        usacConf->audioChannelLayout = usacConf->referenceLayout;
      }
    }
    else {
      bitCount += BsRWBitWrapper(bitStream, &(differsFromReferenceLayout.value), differsFromReferenceLayout.length, WriteFlag);
    }
  }
  else if(!WriteFlag) {
    usacConf->audioChannelLayout = usacConf->referenceLayout;
  }

#endif /* WD1_REFERENCE_LAYOUT_3D */
#endif /* CD_SIGNALS_3D_SIGNAL_GROUPS */

#ifdef WD1_FRAMEWORK_CONFIG_3D
  /*****************************************************************************************************************************************/
  /*                                                                                                                                       */
  /* Framework Config 3D                                                                                                                   */
  /*                                                                                                                                       */ 
  bitCount += advanceFrameworkConfig3d(bitStream, &(usacConf->frameworkConfig3d), WriteFlag);

#endif /* #ifdef WD1_FRAMEWORK_CONFIG_3D */
  
  /*****************************************************************************************************************************************/
  /*                                                                                                                                       */
  /* USAC decoder setup                                                                                                                    */
  /*                                                                                                                                       */ 

  bitCount += advanceUsacDecoderConfig(bitStream, &(usacConf->usacDecoderConfig), UsacConfig_GetSbrRatioIndex(usacConf->coreSbrFrameLengthIndex.value), 
                                       &(usacConf->frameworkConfig3d.signals3D) ,WriteFlag);

  /*****************************************************************************************************************************************/
  /*                                                                                                                                       */
  /* Extension to config                                                                                                                   */
  /*                                                                                                                                       */ 

  bitCount += BsRWBitWrapper(bitStream, &(usacConf->usacConfigExtensionPresent.value), usacConf->usacConfigExtensionPresent.length,WriteFlag);
  if(usacConf->usacConfigExtensionPresent.value){
    bitCount += advanceUsacConfigExtension(bitStream, &(usacConf->usacConfigExtension), WriteFlag);
  }

  return bitCount;
}

USAC_ELEMENT_TYPE UsacConfig_GetUsacElementType(USAC_CONFIG *pUsacConfig, unsigned int elemIdx){

  USAC_ELEMENT_TYPE usacElementType = USAC_ELEMENT_TYPE_INVALID;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->usacDecoderConfig.numElements){
      usacElementType = pUsacConfig->usacDecoderConfig.usacElementType[elemIdx];
    }
  }

  return usacElementType;
}


USAC_CORE_CONFIG * UsacConfig_GetUsacCoreConfig(USAC_CONFIG *pUsacConfig, unsigned int elemIdx){

  USAC_CORE_CONFIG *pUsacCoreConfig = NULL;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->usacDecoderConfig.numElements){
      USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case USAC_ELEMENT_TYPE_SCE:
        pUsacCoreConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacSceConfig.usacCoreConfig);
        break;
      case USAC_ELEMENT_TYPE_CPE:
        pUsacCoreConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.usacCoreConfig);
        break;
      case USAC_ELEMENT_TYPE_LFE:
        pUsacCoreConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacLfeConfig.usacCoreConfig);
        break;
      default:
        break;
      }
    }
  }

  return pUsacCoreConfig;
}


USAC_SBR_CONFIG * UsacConfig_GetUsacSbrConfig(USAC_CONFIG *pUsacConfig, unsigned int elemIdx){

  USAC_SBR_CONFIG *pUsacSbrConfig = NULL;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->usacDecoderConfig.numElements){
      USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case USAC_ELEMENT_TYPE_SCE:
        pUsacSbrConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacSceConfig.usacSbrConfig);
        break;
      case USAC_ELEMENT_TYPE_CPE:
        pUsacSbrConfig = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.usacSbrConfig);
        break;
      default:
        break;
      }
    }
  }

  return pUsacSbrConfig;
}


USAC_MPS212_CONFIG * UsacConfig_GetUsacMps212Config(USAC_CONFIG *pUsacConfig, unsigned int elemIdx){

  USAC_MPS212_CONFIG *pUsacMps212Config = NULL;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->usacDecoderConfig.numElements){
      USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case USAC_ELEMENT_TYPE_CPE:
        pUsacMps212Config = &(pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.usacMps212Config);
        break;
      default:
        break;
      }
    }
  }

  return pUsacMps212Config;
}


int UsacConfig_GetStereoConfigIndex(USAC_CONFIG *pUsacConfig, unsigned int elemIdx){

  int stereoConfigIndex = -1;

  if(pUsacConfig){
    if(elemIdx < pUsacConfig->usacDecoderConfig.numElements){
      USAC_ELEMENT_TYPE elemType = UsacConfig_GetUsacElementType(pUsacConfig, elemIdx);

      switch(elemType){
      case USAC_ELEMENT_TYPE_CPE:
        stereoConfigIndex = pUsacConfig->usacDecoderConfig.usacElementConfig[elemIdx].usacCpeConfig.stereoConfigIndex.value;
        break;
      default:
        break;
      }
    }
  }

  return stereoConfigIndex;
}


int UsacConfig_GetNumElements(int numChannels) {
  int numElements = 0;

  switch (numChannels) {
    case 1:
    case 2:
      numElements = 1;
      break;
    case 6:
      numElements = 4;
      break;
    case 9:
      numElements = 5;
      break;
    case 12:
      numElements = 7;
      break;
    case 24:
      numElements = 16;
      break;
    default:
      numElements = numChannels; /* code unknown configurations as n single channels */
      break;
  } 

  return numElements;
}

int  UsacConfig_GetCICPIndex(int numChannels) {
  int index = 0;

  switch (numChannels ) {
    case 1:
    case 2:
      index = numChannels;
      break;
    case 6:
      index = 6;
      break;
    case 24:
      index = 13;
      break;
    default:
      fprintf(stderr,"No matching CICP index, for %d input channels\n", numChannels);
      assert(0);
      break;
  } 

  return index;
}

/* ********************************************************************************************* */
static USAC_ELEMENT_TYPE elMap_5_1[] = {USAC_ELEMENT_TYPE_CPE,
                                        USAC_ELEMENT_TYPE_SCE,
                                        USAC_ELEMENT_TYPE_LFE,
                                        USAC_ELEMENT_TYPE_CPE};


static USAC_ELEMENT_TYPE elMap_9_0[] = {USAC_ELEMENT_TYPE_CPE,
                                        USAC_ELEMENT_TYPE_SCE,
                                        USAC_ELEMENT_TYPE_CPE,
                                        USAC_ELEMENT_TYPE_CPE,
                                        USAC_ELEMENT_TYPE_CPE};

static USAC_ELEMENT_TYPE elMap_11_1[] = {USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_LFE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE};

static USAC_ELEMENT_TYPE elMap_22_2[] = {USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_LFE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_LFE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_CPE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_SCE,
                                         USAC_ELEMENT_TYPE_CPE};

static int elMap_22_2_isQce[] = {0,
                                 0,
                                 0,
                                 1,
                                 2,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0};


USAC_ELEMENT_TYPE UsacConfig_GetElementType(int elemIdx, int numChannels) {
  USAC_ELEMENT_TYPE elType = USAC_ELEMENT_TYPE_INVALID;

  switch (numChannels) {
    case 1:
      elType = USAC_ELEMENT_TYPE_SCE;
      break;
    case 2:
      elType = USAC_ELEMENT_TYPE_CPE;
      break;
    case 6:
      elType = elMap_5_1[elemIdx];
      break;
    case 9:
      elType = elMap_9_0[elemIdx];
      break;
    case 12:
      elType = elMap_11_1[elemIdx];
      break;
    case 24:
      elType = elMap_22_2[elemIdx];
      break;
    default:
      elType = USAC_ELEMENT_TYPE_SCE;
      break;
  }

  return elType;
}

int UsacConfig_GetIsQceElement(int elemIdx, int numChannels, int allowQce) {
  int isQce = 0;

  if (!allowQce)
    return 0;

  switch (numChannels) {
    case 1:
      break;
    case 2:
      break;
    case 9:
      break;
    case 12:
      break;
    case 24:
      isQce = elMap_22_2_isQce[elemIdx];
      break;
    default:
      break;
  }

  return isQce;
}

#ifdef WD1_REFERENCE_LAYOUT_3D

static int AzimuthAngleIdxToDegrees ( int idx, int direction, int precision ) {

  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

static int ElevationAngleIdxToDegrees ( int idx, int direction, int precision ) {

  int retVal = 0;
  if ( precision == 0 ) {
    retVal = idx * 5;
  } else {
    retVal = idx;
  }

  if(direction == 1) retVal *= -1;

  return retVal;
}

static int ascparserUsacConfig_SpeakerConfig3d ( SPEAKERCONFIG3D* pSpeakerConfig3d, HANDLE_BSBITSTREAM bitStream ) {

  UINT32 i;
  int bitCnt = 0;
  int WriteFlag = 0;

  bitCnt += BsRWBitWrapper ( bitStream, & ( pSpeakerConfig3d->speakerLayoutType.value ), pSpeakerConfig3d->speakerLayoutType.length, WriteFlag );

  if ( pSpeakerConfig3d->speakerLayoutType.value == 0 ) {
    bitCnt += BsRWBitWrapper ( bitStream, & ( pSpeakerConfig3d->CICPspeakerLayoutIdx.value ), pSpeakerConfig3d->CICPspeakerLayoutIdx.length, WriteFlag );
  } else {
    bitCnt += UsacConfig_ReadEscapedValue ( bitStream, & ( pSpeakerConfig3d->numSpeakers.value ), 5, 8, 16 );
    pSpeakerConfig3d->numSpeakers.value++;

    if ( pSpeakerConfig3d->speakerLayoutType.value == 1 ) {
      for ( i = 0; i < pSpeakerConfig3d->numSpeakers.value; i++ ) {
        bitCnt += BsRWBitWrapper ( bitStream, & ( pSpeakerConfig3d->CICPspeakerIdx[i].value ), pSpeakerConfig3d->CICPspeakerIdx[i].length, WriteFlag );
      }
    }

    if ( pSpeakerConfig3d->speakerLayoutType.value == 2 ) {
      bitCnt += ascparserUsacConfig_Usac3dFlexibleSpeakerConfig ( & ( pSpeakerConfig3d->flexibleSpeakerConfig ), pSpeakerConfig3d->numSpeakers.value, bitStream );
    }
  }

  return bitCnt;
}

static int ascparserUsacConfig_Usac3dFlexibleSpeakerConfig ( USAC3DFLEXIBLESPEAKERCONFIG* pFlexibleSpeakerConfig3d, int numSpeakers, HANDLE_BSBITSTREAM bitStream ) {

  int i = 0;
  int bitCnt = 0;
  int WriteFlag = 0;

  DESCR_ELE alsoAddSymmetricPair;
  alsoAddSymmetricPair.length = 1;

  bitCnt += BsRWBitWrapper ( bitStream, & ( pFlexibleSpeakerConfig3d->angularPrecision.value ), pFlexibleSpeakerConfig3d->angularPrecision.length, WriteFlag );

  while ( i < numSpeakers ) {

   int azimuth;

    bitCnt += ascparserUsacConfig_Usac3dSpeakerDescription ( &(pFlexibleSpeakerConfig3d->speakerDescription[i]), pFlexibleSpeakerConfig3d->angularPrecision.value, bitStream );

    if(pFlexibleSpeakerConfig3d->speakerDescription[i].isCICPspeakerIdx.value) {
      CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
      cicp2geometry_get_geometry_from_cicp_loudspeaker_index( pFlexibleSpeakerConfig3d->speakerDescription[i].CICPspeakerIdx.value, &AzElLfe);
      azimuth = AzElLfe.Az;
    }
    else {
      azimuth = AzimuthAngleIdxToDegrees ( pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthAngleIdx.value, pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection.value, pFlexibleSpeakerConfig3d->angularPrecision.value );
    }
 
    if ( (azimuth != 0 ) && (azimuth != 180 ) /* && (the symmetric speaker is not yet in the list) */ ) {
           
      bitCnt += BsRWBitWrapper ( bitStream, & ( alsoAddSymmetricPair.value ), alsoAddSymmetricPair.length, WriteFlag );

      if ( alsoAddSymmetricPair.value ) {
        /*(also add the speaker with the opposite AzimuthDirection);*/
        i++;
        pFlexibleSpeakerConfig3d->speakerDescription[i] = pFlexibleSpeakerConfig3d->speakerDescription[i-1];
        pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection.value = 1 - pFlexibleSpeakerConfig3d->speakerDescription[i].AzimuthDirection.value;
      }
    }

    i++;
  }

  return  bitCnt;
}

static int ascparserUsacConfig_Usac3dSpeakerDescription ( USAC3DSPEAKERDESCRIPTION* speakerDescription, int angularPrecision, HANDLE_BSBITSTREAM bitStream ) {

  int bitCnt = 0;
  int WriteFlag = 0;

  bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->isCICPspeakerIdx.value ), speakerDescription->isCICPspeakerIdx.length, WriteFlag );

  if ( speakerDescription->isCICPspeakerIdx.value ) {
    bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->CICPspeakerIdx.value ), speakerDescription->CICPspeakerIdx.length, WriteFlag );
  } else {
    bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->ElevationClass.value ), speakerDescription->ElevationClass.length, WriteFlag );

    if ( speakerDescription->ElevationClass.value == 3 ) {
      if ( angularPrecision == 0 ) {
        bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->ElevationAngleIdx.value ), speakerDescription->ElevationAngleIdx.length, WriteFlag );
      } else {
        bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->ElevationAngleIdx.value ), speakerDescription->ElevationAngleIdx.length + 2, WriteFlag );
      }

      if ( ElevationAngleIdxToDegrees ( speakerDescription->ElevationAngleIdx.value, speakerDescription->ElevationDirection.value, angularPrecision ) != 0 ) {
        bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->ElevationDirection.value ), speakerDescription->ElevationDirection.length, WriteFlag );
      }
    }

    if ( angularPrecision == 0 ) {
      bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->AzimuthAngleIdx.value ), speakerDescription->AzimuthAngleIdx.length, WriteFlag );
    } else {
      bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->AzimuthAngleIdx.value ), speakerDescription->AzimuthAngleIdx.length + 2, WriteFlag );
    }

    if ( ( AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx.value, speakerDescription->AzimuthDirection.value, angularPrecision ) != 0 ) && 
         ( AzimuthAngleIdxToDegrees ( speakerDescription->AzimuthAngleIdx.value, speakerDescription->AzimuthDirection.value, angularPrecision ) != 180 ) ) {
      bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->AzimuthDirection.value ), speakerDescription->AzimuthDirection.length, WriteFlag );
    }

    bitCnt += BsRWBitWrapper ( bitStream, & ( speakerDescription->isLFE.value ), speakerDescription->isLFE.length, WriteFlag );
  }

  return bitCnt;
}

#endif /* WD1_REFERENCE_LAYOUT_3D */
