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


#ifndef __MPEG_3DAUDIO_CORE_DECLIB_H__
#define __MPEG_3DAUDIO_CORE_DECLIB_H__

#include "../../src_ic/ic_rom.h"

#ifndef IAR
#define IAR 1
#endif


typedef enum {
  MPEG_3DAUDIO_CORE_EXT_DATA_UNKNOWN               = -1,
  MPEG_3DAUDIO_CORE_EXT_DATA_SAOC_3D               =  1,
  MPEG_3DAUDIO_CORE_EXT_DATA_OBJ                   =  2,
  MPEG_3DAUDIO_CORE_EXT_DATA_HOA                   =  3,
  MPEG_3DAUDIO_CORE_EXT_DATA_DRC                   =  4,
#if IAR
  MPEG_3DAUDIO_CORE_EXT_DATA_FMC                   =  5,
#endif
  MPEG_3DAUDIO_CORE_EXT_DATA_MC                    =  6,
  MPEG_3DAUDIO_CORE_EXT_DATA_ENHANCED_OBJ_METADATA =  7,
  MPEG_3DAUDIO_CORE_EXT_DATA_PRODUCTION_METADATA   =  8
} MPEG_3DAUDIO_CORE_EXT_DATA;

typedef enum core_output_mode {
  CORE_OUTPUT_TD,
  CORE_OUTPUT_QMF,
  CORE_OUTPUT_AUTO
} CORE_OUTPUT_MODE;

typedef struct _MPEG_3DAUDIO_CORE_DECODER_INFO
{
  int nOutChannels;
  int maxNumQmfBands;    /* Max number of QMF bands, currently available num of QMF bands can differ due to IPF */
  int maxNumTimeSlots;   /* Max number of time slots, currently available num time slots can differ due to IPF */
  int maxFrameLength;    /* Max frame length, currently available frame length can differ due to IPF */

  int sampleRate;

  int hasEditlist;
  unsigned long editListStartOffset;
  unsigned long editListDurationTotal;
  CORE_OUTPUT_MODE coreOutputMode;
  int bIsChannelSignalGroupOnly;
#ifdef RM6_INTERNAL_CHANNEL
  int nOutChannelsIC;
  int ICMask[64];
#endif
  int numObjectGroups;
  int numChannelGroups;
} MPEG_3DAUDIO_CORE_DECODER_INFO;

typedef struct _MPEG_3DAUDIO_CORE_DECODER* MPEG_3DAUDIO_CORE_DECODER_HANDLE;

int MPEG_3DAudioCore_Declib_Open(MPEG_3DAUDIO_CORE_DECODER_HANDLE* phUsacDec, CORE_OUTPUT_MODE coreOutputMode);
int MPEG_3DAudioCore_Declib_Init(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, char *inFile);
int MPEG_3DAudioCore_Declib_GetAccessUnit(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* isLastFrame);
int MPEG_3DAudioCore_Declib_ParseAudioPreRoll(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* AudioPreRollExisting, int* numPrerollAU);
int MPEG_3DAudioCore_Declib_DecodeAudioPreRoll(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, const int AudioPreRollExisting, const unsigned int idxPrerollAU);
int MPEG_3DAudioCore_Declib_DecodeFrame(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* isLastFrame);
int MPEG_3DAudioCore_Declib_GetAudioTruncationInfoData(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, unsigned int* data, int* size);
int MPEG_3DAudioCore_Declib_GetDecodedSamples_TimeDomain(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float **outSamples, int *nOutSamples, int *nTruncSamples);
int MPEG_3DAudioCore_Declib_GetDecodedSamples_QmfDomain(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float ***qmfReal_outBuffer, float ***qmfImag_outBuffer, int *nTimeSlots, int *nQmfBands);

int MPEG_3DAudioCore_Declib_GetExtensionConfig(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA extensionType, int instance, char* data, int* size);
int MPEG_3DAudioCore_Declib_GetExtensionData(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA extensionType, int instance, char* data, int* size);
int MPEG_3DAudioCore_Declib_GetExtensionType(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int pos);
int MPEG_3DAudioCore_Declib_GetExtensionConfigType(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int pos);

int MPEG_3DAudioCore_Declib_Flush(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec);
int MPEG_3DAudioCore_Declib_Close(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec);

int MPEG_3DAudioCore_Declib_GetInfo(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_DECODER_INFO* info);


#endif
