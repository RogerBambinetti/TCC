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

#include <assert.h>

#include "3DAudioCoreDeclib.h"
#include "streamfile.h"
#include "decifc.h"
#include "dec_usac.h"
#include "common_m4a.h"
#include "mod_buf.h"
#include "qmflib.h"


#ifndef WD1_ELEMENT_MAPPING_3D
#define WD1_ELEMENT_MAPPING_3D
#endif

#define HREP_DELAY 128
#define HREP_HALF_BLOCK_SIZE 64

#define MPEG_3DAUDIO_CORE_MAX_AU 12  /* taken from decifc.c */

#define MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS         64
#define MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS      48
#define MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS          64

#define MPEG_3DAUDIO_CORE_XFADE_LEN 128 /* cross fade for 128 samples */

/* Global variables */
int   gUseFractionalDelayDecor  = 0;
char  g_mixMatrixName[FILENAME_MAX] = { '\0' };
char* g_mixMatrixFileName = g_mixMatrixName;
int   g_mixMatrixFileNameUsed = 0;


static int mpeg_3daudio_core_reset(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, AUDIO_SPECIFIC_CONFIG newASC);

static int mpeg_3daudio_core_frameDataInit(int allTracks, int streamCount, int maxLayer, 
                                           DEC_CONF_DESCRIPTOR decConf [MAX_TRACKS],DEC_DATA* decData);

static int mpeg_3daudio_core_frameDataFree(DEC_DATA* decData);

static void mpeg_3daudio_core_sbrDataInit(DEC_DATA *decData, FRAME_DATA* fD);

static int mpeg_3daudio_core_frameDataAddAccessUnit(DEC_DATA *decData, HANDLE_STREAM_AU au, int track);

static void mpeg_3daudio_core_resortAndScaleChannels(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float** outBuffer, unsigned int nOutSamples);

static int mpeg_3daudio_core_resortQMFBuffers(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float ***qmfReal_outBuffer, 
                                              float ***qmfImag_outBuffer,  int *nTimeSlots, int *nQmfBands );


static void mpeg_3daudio_core_xFade(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec);

static int mpeg_3daudio_core_parsePrerollData(HANDLE_STREAM_AU au, AUDIO_SPECIFIC_CONFIG* prerollASC, unsigned int* prerollASCLength,
                                              HANDLE_STREAM_AU* prerollAU, unsigned int* numPrerollAU, unsigned int* applyCrossFade,
                                              unsigned int const nFramesDone, StreamFileSyncSampleState const sampleIsSyncSampleInMp4);

static int mpeg_3daudio_core_transportAudioTruncationInfo(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, const int trackNr);

typedef struct dec_audioTruncInfo {
  int             isActive;
  int             truncFromBegin;
  int             nTruncSamples;
} DEC_AUDIO_TRUNC_INFO, *HDEC_AUDIO_TRUNC_INFO;

typedef struct _MPEG_3DAUDIO_CORE_DECODER
{
  char mp4InputFile[FILENAME_MAX];

  /* Edit List Information */
  int    hasEditlist;
  double startOffset;
  double durationTotal;

  DEC_DATA *decData;
  HANDLE_DECODER_GENERAL hFault;
  HANDLE_STREAM_AU inputAU;
  HANDLE_STREAM_AU prerollAU[MPEG_3DAUDIO_CORE_MAX_AU];
  HDEC_AUDIO_TRUNC_INFO audioTruncationInfo;
  unsigned int numPrerollAU;

  HANDLE_STREAMPROG prog;
  HANDLE_STREAMFILE stream;

  unsigned int frameLength;

  unsigned int framesDone;

  CORE_OUTPUT_MODE coreOutputMode;

  float  **xFadeBuffer;

  float ***qmfImagOutBuffer;
  float ***qmfRealOutBuffer;

  int doXFade;
  int isLastFrame;

} MPEG_3DAUDIO_CORE_DECODER;
 

int MPEG_3DAudioCore_Declib_Open(MPEG_3DAUDIO_CORE_DECODER_HANDLE* phUsacDec, CORE_OUTPUT_MODE coreOutputMode)
{
  int ts, channel;
  int i = 0;

  MPEG_3DAUDIO_CORE_DECODER_HANDLE tmp;
  tmp = (MPEG_3DAUDIO_CORE_DECODER_HANDLE)calloc(1, sizeof(MPEG_3DAUDIO_CORE_DECODER));

  tmp->coreOutputMode      = coreOutputMode;
  tmp->decData             = (DEC_DATA*)            calloc(1, sizeof(DEC_DATA));
  tmp->decData->frameData  = (FRAME_DATA*)          calloc(1, sizeof(FRAME_DATA));
  tmp->decData->usacData   = (USAC_DATA*)           calloc(1, sizeof(USAC_DATA));
  tmp->audioTruncationInfo = (HDEC_AUDIO_TRUNC_INFO)calloc(1, sizeof(DEC_AUDIO_TRUNC_INFO));

  /* initialize with 1, since it's true */
  for ( i = 0; i < MAX_TIME_CHANNELS; ++i ) {
    tmp->decData->usacData->First_frame_flag[i] = 1;
  }

  tmp->hFault = (T_DECODERGENERAL*) calloc(MAX_TF_LAYER, sizeof(T_DECODERGENERAL));

  tmp->inputAU = StreamFileAllocateAU(0);

  *phUsacDec = tmp;


  /* Initialize global QMF Buffers */
  tmp->qmfRealOutBuffer = (float ***) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS, sizeof(float**) );
  tmp->qmfImagOutBuffer = (float ***) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS, sizeof(float**) );

  for ( ts = 0; ts < MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS; ++ts ) {

    tmp->qmfRealOutBuffer[ts] = (float **) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS, sizeof(float*) );
    tmp->qmfImagOutBuffer[ts] = (float **) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS, sizeof(float*) );
    
    for ( channel = 0; channel < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++channel ) {
      tmp->qmfRealOutBuffer[ts][channel] = (float *) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS, sizeof(float) );
      tmp->qmfImagOutBuffer[ts][channel] = (float *) calloc( MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS, sizeof(float) );
    }
  }

  
   tmp->xFadeBuffer = (float**) calloc(MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS, sizeof(float*));

   for (i = 0; i < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++i ) {
     tmp->xFadeBuffer[i] = (float*) calloc(MPEG_3DAUDIO_CORE_XFADE_LEN, sizeof(float));
   }

   for (i = 0; i < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++i ) {
     tmp->decData->usacData->hrepInBuffer[i] = CreateFloatModuloBuffer( HREP_DELAY + 4096);
   }

 /* To get the printout warning displaying a program name */
  CommonProgName("3D-Audio Decoder Library:");

  return 0;
}


int MPEG_3DAudioCore_Declib_Init(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, char *inFile)
{
  AUDIO_SPECIFIC_CONFIG* asc;

  int progStop = 0;
  int decoderTracks = 0;
  int maxLayer = -1;
  int numLFE = 0;
  unsigned int i = 0;
  int programNr = 0;   /* Decode first program */

  /* init Audio Pre-Roll */
  hUsacDec->numPrerollAU = 0;
  memset(hUsacDec->prerollAU, 0, sizeof(HANDLE_STREAM_AU) * MPEG_3DAUDIO_CORE_MAX_AU);

  strncpy(hUsacDec->mp4InputFile,    inFile, FILENAME_MAX);

  hUsacDec->stream = StreamFileOpenRead(hUsacDec->mp4InputFile, FILETYPE_AUTO); 
  if ( !hUsacDec->stream )
    return -1;


  progStop = StreamFileGetProgramCount( hUsacDec->stream );
  if ( progStop > 1 )
  {
    fprintf(stderr," More than one program in stream, initializing only first program\n");

  }

  hUsacDec->prog = StreamFileGetProgram( hUsacDec->stream, programNr );
  if ( hUsacDec->prog->trackCount == 0 )
  {
    fprintf(stderr,"Error: No suitable track found\n");
    return -1;
  }

  /* Get number of output channels */

  asc = &(hUsacDec->prog->decoderConfig[0].audioSpecificConfig);
  asc->channelConfiguration.value = usac_get_channel_number(&asc->specConf.usacConfig, &numLFE);

  decoderTracks = mpeg_3daudio_core_frameDataInit(hUsacDec->prog->allTracks,
                                                  hUsacDec->prog->trackCount,
                                                  maxLayer,
                                                  hUsacDec->prog->decoderConfig,
                                                  hUsacDec->decData);

  if ( 0 == decoderTracks )
  {
    fprintf(stderr,"Error:  No decoder tracks found\n");
    return -1;
  }

  if ( decoderTracks > 1 )
  {
    fprintf(stderr,"More than one track found, initializing only first track\n");
    return -1;
  }


  DecUsacInit( NULL, NULL, 0,  hUsacDec->hFault, hUsacDec->decData->frameData, hUsacDec->decData->usacData, NULL);

  hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples;

  /* automatically detect whether qmf or time output shall be delivered */
  if (CORE_OUTPUT_AUTO == hUsacDec->coreOutputMode) {
    if ( hUsacDec->decData->usacData->sbrRatioIndex == 0 || 
         hUsacDec->decData->usacData->sbrRatioIndex  == 1 ) {
      hUsacDec->coreOutputMode = CORE_OUTPUT_TD;
    }
    else {
      hUsacDec->coreOutputMode = CORE_OUTPUT_QMF;
    }
  }

  switch(hUsacDec->decData->usacData->sbrRatioIndex)
  {
  case 0:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples;
    break;
  case 1:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples*4;
    break;
  case 2:
    hUsacDec->frameLength = (hUsacDec->decData->usacData->block_size_samples*8)/3;
    break;
  case 3:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples*2;
    break;
  default:
    hUsacDec->frameLength = 0;
    return -1;
    break;
  }

  hUsacDec->decData->frameData->scalOutObjectType = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.audioDecoderType.value;
  hUsacDec->decData->frameData->scalOutNumChannels = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.channelConfiguration.value;
  hUsacDec->decData->frameData->scalOutSamplingFrequency = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.samplingFrequency.value;

  mpeg_3daudio_core_sbrDataInit(hUsacDec->decData, hUsacDec->decData->frameData);

  /* Get Editlist Information */
  {
    double tmpStart, tmpDuration;
    if( 0 == StreamFileGetEditlist(hUsacDec->prog, 0, &tmpStart, &tmpDuration) )
    {
      hUsacDec->startOffset   = tmpStart;
      hUsacDec->durationTotal = tmpDuration;

      if(hUsacDec->durationTotal > -1)
      {
        double fSampleOut = hUsacDec->decData->frameData->scalOutSamplingFrequency;

        hUsacDec->hasEditlist = 1;

#ifndef ALIGN_PRECISION_NOFIX
        hUsacDec->startOffset   = hUsacDec->startOffset * fSampleOut + 0.5;
        hUsacDec->durationTotal = hUsacDec->durationTotal * fSampleOut + 0.5;
#endif
      }
    }
  }

  if (hUsacDec->decData->usacData->usacDecoder->hrepData.useHREP) {
    unsigned int nCh;
    nCh = hUsacDec->decData->frameData->scalOutNumChannels;

    for ( i = 0; i < nCh; ++i )
    {
      int j;

      ZeroFloatModuloBuffer( hUsacDec->decData->usacData->hrepInBuffer[i], HREP_HALF_BLOCK_SIZE);

      hUsacDec->decData->usacData->usacDecoder->hrepData.delayedGains[0][i] = 1;
      hUsacDec->decData->usacData->usacDecoder->hrepData.delayedGains[1][i] = 1;
      hUsacDec->decData->usacData->usacDecoder->hrepData.sampleCnt[i] = 0;

      for (j = 0; j < HREP_HALF_BLOCK_SIZE; j++) {
        hUsacDec->decData->usacData->usacDecoder->hrepData.delay_buf[j][i] = 0;
      }
    }

    hUsacDec->decData->usacData->usacDecoder->hrepData.hrepDelay = HREP_HALF_BLOCK_SIZE;
    hUsacDec->decData->usacData->usacDecoder->hrepData.numHrepBlocks = hUsacDec->frameLength / HREP_HALF_BLOCK_SIZE;
    hUsacDec->decData->usacData->usacDecoder->hrepData.numHrepChannels = nCh;
  }

#ifdef   RM6_INTERNAL_CHANNEL

#ifdef RM6_ICG_SYNTAX
  {
    unsigned int chn, elemIdx;
    USAC_CONFIG * usacConf = & hUsacDec->prog->decoderConfig[0].audioSpecificConfig.specConf.usacConfig;

    getSpkrLbl ( ICConfig.lbl, &ICConfig.nspkr, usacConf->referenceLayout.CICPspeakerLayoutIdx.value );

    ICConfig.numElements  = usacConf->usacDecoderConfig.numElements;
    for ( elemIdx=0, chn=0; elemIdx < ICConfig.numElements; elemIdx++ )
    {
      ICConfig.ElementType[elemIdx]    = usacConf->usacDecoderConfig.usacElementType[elemIdx];
      switch(usacConf->usacDecoderConfig.usacElementType[elemIdx])
      {
        case USAC_ELEMENT_TYPE_SCE:
          ICConfig.CPEOut[elemIdx][0]  = ICConfig.lbl[usacConf->usacDecoderConfig.usacChannelIndex[chn++]];
          ICConfig.CPEOut[elemIdx][1]  = CHN_EMPTY;
          break;
        case USAC_ELEMENT_TYPE_CPE:
          ICConfig.CPEOut[elemIdx][0]  = ICConfig.lbl[usacConf->usacDecoderConfig.usacChannelIndex[chn++]];
          ICConfig.CPEOut[elemIdx][1]  = ICConfig.lbl[usacConf->usacDecoderConfig.usacChannelIndex[chn++]];
          break;
        case USAC_ELEMENT_TYPE_LFE:
          ICConfig.CPEOut[elemIdx][0]  = ICConfig.lbl[usacConf->usacDecoderConfig.usacChannelIndex[chn++]];
          ICConfig.CPEOut[elemIdx][1]  = CHN_EMPTY;
          break;        
      }
    }
  }
#endif

  initICConfig ();
  
#endif

  return 0;
}


int MPEG_3DAudioCore_Declib_GetInfo(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_DECODER_INFO* info)
{
  AUDIO_SPECIFIC_CONFIG asc;

  if (!hUsacDec)
    return -1;

  asc = hUsacDec->prog->decoderConfig->audioSpecificConfig;

#ifdef RM6_INTERNAL_CHANNEL
  if ( ICConfig.isStereoOut )
    info->nOutChannelsIC          = ICConfig.nspkrOut;
  else
    info->nOutChannelsIC          = asc.channelConfiguration.value;
#endif

  info->nOutChannels              = asc.channelConfiguration.value;
  info->maxNumQmfBands            = MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS;
  info->maxNumTimeSlots           = MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS;
  info->maxFrameLength            = MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS * MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS; /* Use max value in case of IPF */

  info->sampleRate                = asc.sbrPresentFlag.value == 1 ? asc.extensionSamplingFrequency.value : asc.samplingFrequency.value;

  info->hasEditlist               = hUsacDec->hasEditlist;
  info->editListDurationTotal     = (unsigned long) hUsacDec->durationTotal;
  info->editListStartOffset       = (unsigned long) hUsacDec->startOffset;

  info->coreOutputMode            = hUsacDec->coreOutputMode;
  info->bIsChannelSignalGroupOnly = hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numAudioChannels > 0 &&
                                    hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numAudioObjects == 0 &&
                                    hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numSAOCTransportChannels == 0 &&
                                    hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.numHOATransportChannels == 0;

  {
    int i;
    int numObjectGroups = 0;
    int numChannelGroups = 0;
    int numSignalGroups = hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.m_numSignalGroups;

    for (i = 0; i < numSignalGroups + 1; i++) {
      if (hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.m_signalGroupType[i] == 0) {
        numChannelGroups++;
      } else if (hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.frameworkConfig3d.signals3D.m_signalGroupType[i] == 1) {
        numObjectGroups++;
      }
    }

    info->numChannelGroups = numChannelGroups;
    info->numObjectGroups = numObjectGroups;
  }

  return 0;
}

int MPEG_3DAudioCore_Declib_GetAccessUnit(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* isLastFrame)
{
  int err     = 0;
  int trackNr = 0;

  hUsacDec->doXFade     = 0;
  hUsacDec->isLastFrame = 0;
  *isLastFrame          = 0;

  err = StreamGetAccessUnit(hUsacDec->prog, trackNr, hUsacDec->inputAU);

  if (0 != err) {
    if (-2 == err) {
      hUsacDec->isLastFrame = 1;
      *isLastFrame          = 1;
      return 0;
    } else {
      return err;
    }
  }

  err = mpeg_3daudio_core_transportAudioTruncationInfo(hUsacDec, trackNr);

  return err;
}

int MPEG_3DAudioCore_Declib_ParseAudioPreRoll(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* AudioPreRollExisting, int* numPrerollAU)
{
  int err = 0;

  *AudioPreRollExisting = 0;
  *numPrerollAU = 0;

  /* if first extension element is USAC_ID_EXT_ELE_AUDIOPREROLL this stream supports switching */
  if (hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[0].usacExtConfig.usacExtElementType == USAC_ID_EXT_ELE_AUDIOPREROLL) {
    AUDIO_SPECIFIC_CONFIG prerollASC;
    unsigned int prerollASCLength = 0;
    unsigned int applyCrossFade   = 0;

    /* Prepare preroll-ASC layout - needed to have the correct number of bits for every entry */
    memcpy(&prerollASC, &(hUsacDec->prog->decoderConfig[0].audioSpecificConfig), sizeof(AUDIO_SPECIFIC_CONFIG));

    /* Parse preroll data if present */
    if (mpeg_3daudio_core_parsePrerollData(hUsacDec->inputAU, &prerollASC, &prerollASCLength, hUsacDec->prerollAU, &hUsacDec->numPrerollAU, &applyCrossFade, hUsacDec->framesDone, hUsacDec->prog->stssSignalsSyncSample[0])) {
      int ascDiffer = 0;

      if (applyCrossFade) {
        if (prerollASCLength) {
          /* Check if new ASC differs from current one */
          ascDiffer = memcmp(&(hUsacDec->prog->decoderConfig[0].audioSpecificConfig), &prerollASC, sizeof(AUDIO_SPECIFIC_CONFIG));

          if (ascDiffer) {
            CommonExit(1,"Implicit config change with crossfade not supported\n") ;
          } else {
            fprintf(stderr, "Warning: Ignoring x-Fade flag for possible config change\n");
          }
        } else {
          fprintf(stderr, "Warning: Ignoring x-Fade flag for possible config change\n");
        }
      } else {
        if (prerollASCLength) {
          /* Check if new ASC differs from current one */
          ascDiffer = memcmp(&(hUsacDec->prog->decoderConfig[0].audioSpecificConfig), &prerollASC, sizeof(AUDIO_SPECIFIC_CONFIG));

          if (ascDiffer) {
            CommonExit(1,"Implicit config change without crossfade not supported\n") ;
          }
        }
      }

      /* Decode Preroll frames only in first frame */
      if (0 == hUsacDec->framesDone) {
        *AudioPreRollExisting = 1;
        *numPrerollAU         = hUsacDec->numPrerollAU;
      }
    }
  }

  return err;
}

int MPEG_3DAudioCore_Declib_DecodeAudioPreRoll(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, const int AudioPreRollExisting, const unsigned int idxPrerollAU)
{
  int err           = 0;
  int numChannelOut = 0;

  if ((idxPrerollAU >= hUsacDec->numPrerollAU) || (idxPrerollAU >= MPEG_3DAUDIO_CORE_MAX_AU)) {
    return 1;
  }

  if ((1 == AudioPreRollExisting) && (0 == hUsacDec->framesDone)) {
    err = mpeg_3daudio_core_frameDataAddAccessUnit(hUsacDec->decData, hUsacDec->prerollAU[idxPrerollAU], 0);

    DecUsacFrame(hUsacDec->decData->frameData->scalOutNumChannels,
                 hUsacDec->decData->frameData,
                 hUsacDec->decData->usacData,
                 hUsacDec->hFault,
                 &numChannelOut,
                 hUsacDec->qmfRealOutBuffer,
                 hUsacDec->qmfImagOutBuffer);

    StreamFileFreeAU(hUsacDec->prerollAU[idxPrerollAU]);
  }

  return err;
}

int MPEG_3DAudioCore_Declib_DecodeFrame(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int* isLastFrame)
{
  int err           = 0;
  int numChannelOut = 0;

  err = mpeg_3daudio_core_frameDataAddAccessUnit(hUsacDec->decData, hUsacDec->inputAU, 0);

  if (hUsacDec->decData->frameData->layer[0].NoAUInBuffer == 0) {
    hUsacDec->isLastFrame = 1;
    *isLastFrame          = 1;
    return 0;
  }

  DecUsacFrame(hUsacDec->decData->frameData->scalOutNumChannels,
               hUsacDec->decData->frameData,
               hUsacDec->decData->usacData,
               hUsacDec->hFault,
               &numChannelOut,
               hUsacDec->qmfRealOutBuffer,
               hUsacDec->qmfImagOutBuffer);

  hUsacDec->framesDone++;

  return err;
}

int MPEG_3DAudioCore_Declib_GetDecodedSamples_TimeDomain(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float **outSamples, int *nOutSamples, int *nTruncSamples)
{
  if (NULL == outSamples || NULL == nOutSamples || NULL == nTruncSamples) {
    return -1;
  }

  if ( hUsacDec->isLastFrame ) {
    return 0;
  }
  else
  {
    if ( hUsacDec->doXFade )
    {
     fprintf(stderr, "Cross-fading due to new IPF signal\n");
     mpeg_3daudio_core_xFade(hUsacDec);
     hUsacDec->doXFade = 0;
    }

    mpeg_3daudio_core_resortAndScaleChannels(hUsacDec, outSamples, hUsacDec->frameLength);

    *nOutSamples = hUsacDec->frameLength;

    if (1 == hUsacDec->audioTruncationInfo->isActive) {
      if (1 == hUsacDec->audioTruncationInfo->truncFromBegin) {
        /* left truncation */
        *nTruncSamples -= hUsacDec->audioTruncationInfo->nTruncSamples;
      } else {
        /* right truncation */
        *nTruncSamples  = hUsacDec->audioTruncationInfo->nTruncSamples;
      }
    }
  }

   return 0;
}


int MPEG_3DAudioCore_Declib_GetDecodedSamples_QmfDomain(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float ***qmfReal_outBuffer, float ***qmfImag_outBuffer, int *nTimeSlots, int *nQmfBands)
{
  int ret = 0;

  if ( hUsacDec->isLastFrame )
  {
    /* No delay line for QMF samples, nothing to flush  */
     *nTimeSlots = 0;
     *nQmfBands  = 0;

     return 0;
  }
  else
  {
    ret = mpeg_3daudio_core_resortQMFBuffers(hUsacDec, qmfReal_outBuffer, qmfImag_outBuffer, nTimeSlots, nQmfBands);

    return ret;
  }
}

int MPEG_3DAudioCore_Declib_GetExtensionConfig(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA extensionType, int instance, char* data, int* size)
{
  USAC_ID_EXT_ELE type = USAC_ID_EXT_ELE_UNDEFINED;
  unsigned int i = 0;
  int instanceCounter = 1;
  int contribMode = 0;

  /* -1 means no extension data of type extensionType found */
  *size = -1;

  switch (extensionType)
  {
  case MPEG_3DAUDIO_CORE_EXT_DATA_SAOC_3D:
    type = USAC_ID_EXT_ELE_SAOC_3D;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_OBJ:
    type = USAC_ID_EXT_ELE_OBJ;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_HOA:
    type = USAC_ID_EXT_ELE_HOA;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_DRC:
    type = USAC_ID_EXT_ELE_UNI_DRC;
    break;
#if IAR
  case MPEG_3DAUDIO_CORE_EXT_DATA_FMC:
  /* no config for MPEG_3DAUDIO_CORE_EXT_DATA_FMC */
  break;
#endif
  case MPEG_3DAUDIO_CORE_EXT_DATA_PRODUCTION_METADATA:
    type = USAC_ID_EXT_TYPE_PRODUCTION_METADATA;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_MC:
    type = USAC_ID_EXT_ELE_MC;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_ENHANCED_OBJ_METADATA:
    type = USAC_ID_EXT_ELE_ENHANCED_OBJECT_METADATA;
    break;
  default:
    if (hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.referenceLayout.speakerLayoutType.value == 3 ) {
      contribMode = 1;
    }
    else {
      return -1;
    }
  }

  for ( i = 0; i < hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.numElements; ++i )
  {
    if ( hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[i].usacExtConfig.usacExtElementType == type || contribMode)
    {
      if ( instanceCounter == instance ) {
        
        *size = hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[i].usacExtConfig.usacExtElementConfigLength;
        memcpy(data, hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[i].usacExtConfig.usacExtElementConfigPayload, *size);
        
        return instance;
      }
      instanceCounter++;
    }
  }

  return 0;
}

int MPEG_3DAudioCore_Declib_GetExtensionConfigType(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int pos){
  if( pos >= (int)hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.numElements || pos < 0){
    return -1;
  }
  return hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacElementConfig[pos].usacExtConfig.usacExtElementType;
}

int MPEG_3DAudioCore_Declib_GetExtensionData(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA extensionType, int instance, char* data, int* size)
{
  USAC_EXT_TYPE type = USAC_EXT_TYPE_UNDEFINED;
  int i = 0;
  int instanceCounter = 1;
  int contribMode     = 0;
  
  /* -1 means no extension data of type extensionType found */
  *size = -1;

  switch (extensionType)
  {
  case MPEG_3DAUDIO_CORE_EXT_DATA_SAOC_3D:
    type = USAC_EXT_TYPE_SAOC_3D;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_OBJ:
    type = USAC_EXT_TYPE_OBJ;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_HOA:
    type = USAC_EXT_TYPE_HOA;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_DRC:
    type = USAC_EXT_TYPE_UNI_DRC;
    break;
#if IAR
  case MPEG_3DAUDIO_CORE_EXT_DATA_FMC:
    type = USAC_EXT_TYPE_FMC;
    break;
#endif
  case MPEG_3DAUDIO_CORE_EXT_DATA_MC:
    type = USAC_EXT_TYPE_MC;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_ENHANCED_OBJ_METADATA:
    type = USAC_EXT_TYPE_ENHANCED_OBJ_METADATA;
    break;
  case MPEG_3DAUDIO_CORE_EXT_DATA_PRODUCTION_METADATA:
    type = USAC_EXT_TYPE_PRODUCTION_METADATA;
    break;
  default:
    if (hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.referenceLayout.speakerLayoutType.value == 3 ) {
      contribMode = 1;
    }
    else {
      return -1;
    }
  }

  for ( i = 0; i < hUsacDec->decData->usacData->usacExtNumElements; ++i )
  {
    if ( hUsacDec->decData->usacData->usacExtElementType[i] == type || contribMode )
    {

      if ( instanceCounter == instance ) {
        *size = 0;
        if ( hUsacDec->decData->usacData->usacExtElementComplete[i] == 1 )
        {
          int j = 0;
          *size = hUsacDec->decData->usacData->usacExtElement[i].usacExtElementPayloadLength;

          for ( j = 0; j < *size; ++j )
          {
            data[j] = hUsacDec->decData->usacData->usacExtElement[i].usacExtElementSegmentData[j];
          }

          /* Reset complete flag to avoid second read of same payload */
          hUsacDec->decData->usacData->usacExtElementComplete[i] = 0;
        }
        return instance;
      }
      instanceCounter++;
    }
  }

  return 0;
}

int MPEG_3DAudioCore_Declib_GetExtensionType(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, int pos){

  if( pos >= hUsacDec->decData->usacData->usacExtNumElements || pos < 0){
    return -1;
  }
  return hUsacDec->decData->usacData->usacExtElementType[pos];

}

int MPEG_3DAudioCore_Declib_GetAudioTruncationInfoData(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, unsigned int* data, int* size)
{
  int nBits = 0;

  /* -1 means no audioTruncationInfo data found */
  *size = -1;

  if (0 != hUsacDec->prog->audioTruncationinfo[0].isActive.value) {
    nBits += hUsacDec->prog->audioTruncationinfo[0].isActive.length;
    nBits += hUsacDec->prog->audioTruncationinfo[0].reserved.length;
    nBits += hUsacDec->prog->audioTruncationinfo[0].truncFromBegin.length;
    nBits += hUsacDec->prog->audioTruncationinfo[0].nTruncSamples.length;

    if (0 != nBits % 8) {
      return (int)MPEG3DACORE_ERROR_INVALID_STREAM;
    }

    data[0] = hUsacDec->prog->audioTruncationinfo[0].isActive.value;
    data[1] = hUsacDec->prog->audioTruncationinfo[0].reserved.value;
    data[2] = hUsacDec->prog->audioTruncationinfo[0].truncFromBegin.value;
    data[3] = hUsacDec->prog->audioTruncationinfo[0].nTruncSamples.value;

    *size = 4;
  }

  return (int)MPEG3DACORE_OK;
}

int MPEG_3DAudioCore_Declib_Close(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec)
{
  unsigned int i = 0;

  for (i = 0; i < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++i )
  {
    DeleteFloatModuloBuffer( hUsacDec->decData->usacData->hrepInBuffer[i] );
  }

  mpeg_3daudio_core_frameDataFree(hUsacDec->decData);
  DecUsacFree(hUsacDec->decData->usacData, hUsacDec->hFault);

  if (hUsacDec->stream != NULL) 
    StreamFileClose (hUsacDec->stream); 

  if ( hUsacDec->decData->frameData )
    free ( hUsacDec->decData->frameData );

  if ( hUsacDec->decData->usacData )
    free ( hUsacDec->decData->usacData );

  if ( hUsacDec->hFault )
    free ( hUsacDec->hFault );

  if ( hUsacDec->decData )
    free ( hUsacDec->decData );

  if (hUsacDec->audioTruncationInfo) {
    free(hUsacDec->audioTruncationInfo);
  }

  StreamFileFreeAU(hUsacDec->inputAU);

  if ( hUsacDec->qmfRealOutBuffer )
  {
    int ts, channel;
    for ( ts = 0; ts < MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS; ++ts ) 
    {
      for ( channel = 0; channel < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++channel ) 
        free( hUsacDec->qmfRealOutBuffer[ts][channel] );
 
      free ( hUsacDec->qmfRealOutBuffer[ts] );
    }
    free ( hUsacDec->qmfRealOutBuffer );
  }

  if ( hUsacDec->qmfImagOutBuffer )
  {
    int ts, channel;
    for ( ts = 0; ts < MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS; ++ts ) 
    {
      for ( channel = 0; channel < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++channel ) 
        free( hUsacDec->qmfImagOutBuffer[ts][channel] );

      free ( hUsacDec->qmfImagOutBuffer[ts] );
    }
    free ( hUsacDec->qmfImagOutBuffer );
  }

  if ( hUsacDec->xFadeBuffer )
  {
    int i = 0;
    for (i = 0; i < MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS; ++i )
    {
      if (hUsacDec->xFadeBuffer[i])
        free(hUsacDec->xFadeBuffer[i]);
    }
    free(hUsacDec->xFadeBuffer);
  }

  free(hUsacDec);

  return 0;
}

int MPEG_3DAudioCore_Declib_Flush(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec)
{
  int numChannelOut = 0;
  int err = 0;

  /* Empty AU with the size of the current AU */
  HANDLE_STREAM_AU zeroAU = StreamFileAllocateAU(hUsacDec->inputAU->numBits);
  memset(zeroAU->data, 0, hUsacDec->inputAU->numBits / 8);

  /* Indicate indpendent frame by setting the indepedency flag to one */ 
  zeroAU->data[0] = 1 << 7;

  err = mpeg_3daudio_core_frameDataAddAccessUnit(hUsacDec->decData, zeroAU, 0);

  DecUsacFrame(hUsacDec->decData->frameData->scalOutNumChannels,
    hUsacDec->decData->frameData,
    hUsacDec->decData->usacData,
    hUsacDec->hFault,
    &numChannelOut,
    hUsacDec->qmfRealOutBuffer,
    hUsacDec->qmfImagOutBuffer);

 
  StreamFileFreeAU(zeroAU);

  return 0;
}

static int mpeg_3daudio_core_transportAudioTruncationInfo(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, const int trackNr)
{
  int err = 0;
  AUDIO_TRUNCATION_INFO ati;

  err = StreamGetAudioTruncationInfo(hUsacDec->prog, trackNr, &ati);

  if (0 != err) {
    if (-2 == err) {
      hUsacDec->audioTruncationInfo->isActive       = 0;
      hUsacDec->audioTruncationInfo->truncFromBegin = 0;
      hUsacDec->audioTruncationInfo->nTruncSamples  = 0;
      err = 0;
    }
  } else {
    hUsacDec->audioTruncationInfo->isActive       = ati.isActive.value;
    hUsacDec->audioTruncationInfo->truncFromBegin = ati.truncFromBegin.value;
    hUsacDec->audioTruncationInfo->nTruncSamples  = ati.nTruncSamples.value;
  }

  return err;
}

static int mpeg_3daudio_core_reset(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, AUDIO_SPECIFIC_CONFIG newASC)
{
  int dummy = 0;
  int maxLayer = -1;

  mpeg_3daudio_core_frameDataFree(hUsacDec->decData);
  DecUsacFree(hUsacDec->decData->usacData, hUsacDec->hFault);
 
  /* Get new ASC in */
  memcpy(&(hUsacDec->prog->decoderConfig[0].audioSpecificConfig), &newASC, sizeof(AUDIO_SPECIFIC_CONFIG));

  hUsacDec->prog->decoderConfig[0].audioSpecificConfig.channelConfiguration.value = 
    usac_get_channel_number(&(hUsacDec->prog->decoderConfig[0].audioSpecificConfig.specConf.usacConfig), &dummy);


 mpeg_3daudio_core_frameDataInit(hUsacDec->prog->allTracks, hUsacDec->prog->trackCount, maxLayer, hUsacDec->prog->decoderConfig,
                                hUsacDec->decData);

 DecUsacInit( NULL, NULL, 0,  hUsacDec->hFault, hUsacDec->decData->frameData, hUsacDec->decData->usacData, NULL);

 hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples;

 switch(hUsacDec->decData->usacData->sbrRatioIndex)
  {
  case 0:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples;
    break;
  case 1:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples*4;
    break;
  case 2:
    hUsacDec->frameLength = (hUsacDec->decData->usacData->block_size_samples*8)/3;
    break;
  case 3:
    hUsacDec->frameLength = hUsacDec->decData->usacData->block_size_samples*2;
    break;
  default:
    hUsacDec->frameLength = 0;
    return -1;
    break;
  }

 hUsacDec->decData->frameData->scalOutObjectType = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.audioDecoderType.value;
 hUsacDec->decData->frameData->scalOutNumChannels = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.channelConfiguration.value;
 hUsacDec->decData->frameData->scalOutSamplingFrequency = hUsacDec->decData->frameData->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.samplingFrequency.value;

 mpeg_3daudio_core_sbrDataInit(hUsacDec->decData, hUsacDec->decData->frameData);



  return 0;
}

static void mpeg_3daudio_core_xFade(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec)
{
  unsigned int channel = 0, j = 0;
  for ( channel = 0; channel < hUsacDec->prog->decoderConfig[0].audioSpecificConfig.channelConfiguration.value; ++channel )
  { 
    for ( j = 0; j < MPEG_3DAUDIO_CORE_XFADE_LEN; ++j )
    {
      float xFadeValue = (float) j / (float) MPEG_3DAUDIO_CORE_XFADE_LEN;

      hUsacDec->decData->usacData->sampleBuf[channel][j] = xFadeValue * hUsacDec->decData->usacData->sampleBuf[channel][j] 
                                                     + ( 1.f - xFadeValue ) * hUsacDec->xFadeBuffer[channel][j]; 
    }
  }
}

static int mpeg_3daudio_core_parsePrerollData(HANDLE_STREAM_AU au, AUDIO_SPECIFIC_CONFIG* prerollASC, unsigned int*  prerollASCLength,
                                              HANDLE_STREAM_AU* prerollAU, unsigned int* numPrerollAU, unsigned int* applyCrossFade,
                                              unsigned int const nFramesDone, StreamFileSyncSampleState const sampleIsSyncSampleInMp4)
{
  HANDLE_BSBITBUFFER bb = NULL;
  HANDLE_BSBITSTREAM bs;
  
  unsigned int i = 0;
  int retVal = 0;

  unsigned long indepFlag = 0;
  unsigned long extPayloadPresentFlag = 0;
  unsigned long useDefaultLengthFlag  = 0;
  unsigned int ascSize = 0;
  unsigned long dummy = 0;
  int nLFE = 0;
  int ascBitCounter = 0;
  
  unsigned int totalPayloadLength = 0;

  bb = BsAllocPlainDirtyBuffer(au->data, au->numBits);
  bs = BsOpenBufferRead(bb);

  *applyCrossFade = 0;

  /* Indep flag must be one */
  BsGetBit(bs, &indepFlag, 1);
 
  /* Payload present flag must be one */
  BsGetBit(bs, &extPayloadPresentFlag, 1);

  /* ISO/IEC 23008-3: 20.2 Random access and stream access: [IPFs] shall be signalled as sync samples according to ISO/IEC 14496-12. */
  if (extPayloadPresentFlag && (STSS_SYNCSAMPLE_NOTSET == sampleIsSyncSampleInMp4)) {
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
    fprintf(stderr, "\n!!WARNING!!  IPF shall be signalled as SyncSample in ISOBMFF!");
    fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
    fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
  } else if (!extPayloadPresentFlag && (STSS_SYNCSAMPLE_SET == sampleIsSyncSampleInMp4)) {
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
    fprintf(stderr, "\n!!WARNING!!  A SyncSample in ISOBMFF shall be an IPF!");
    fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
    fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
  }

  if (!indepFlag && extPayloadPresentFlag) {
    /* ISO/IEC 23008-3: 5.5.6 Audio pre-roll: '1': usacIndependencyFlag; */
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
    fprintf(stderr, "\n!!WARNING!!  usacIndependencyFlag shall be 1 for an IPF!");
    fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
    fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
    goto freeMem;
  }

  if (!extPayloadPresentFlag) {
    if (0 == nFramesDone) {
      /* ISO/IEC 23008-3: Annex N: "Retaining original file length with MPEG-H 3D audio" */
      fprintf(stderr, "\nInfo:\n\tThe first frame does not contain an ID_EXT_ELE_AUDIOPREROLL element.\n");
    }
    goto freeMem;
  }

  /* Default length flag must be zero */
  BsGetBit(bs, &useDefaultLengthFlag, 1);
  if (useDefaultLengthFlag) {
    /* ISO/IEC 23008-3: 5.5.6 Audio pre-roll: '0': usacExtElementUseDefaultLength (referring to audio pre-roll extension element). */
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
    fprintf(stderr, "\n!!WARNING!!  usacExtElementUseDefaultLength shall be 0 for an IPF!");
    fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
    fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
    fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
    goto freeMem;
  }


  /* Seems to be a valid preroll extension */
  retVal = 1;

  /* read overall ext payload length */
  UsacConfig_ReadEscapedValue(bs, &totalPayloadLength, 8, 16, 0);

  /* read ASC size */
  UsacConfig_ReadEscapedValue(bs, &ascSize, 4, 4, 8);
  *prerollASCLength = ascSize;

  /* read ASC */
  if ( ascSize )
  {
    ascBitCounter = UsacConfig_Advance(bs, &(prerollASC->specConf.usacConfig), 0);
    prerollASC->channelConfiguration.value = usac_get_channel_number(&prerollASC->specConf.usacConfig, &nLFE);
  }

  /* Skip remaining bits from ASC that were not parsed */
  {
    unsigned int nBitsToSkip =  ascSize * 8 - ascBitCounter;

    while ( nBitsToSkip )
    {
      int nMaxBitsToRead = min(nBitsToSkip, LONG_NUMBIT);
      BsGetBit(bs, &dummy, nMaxBitsToRead);
      nBitsToSkip -= nMaxBitsToRead;
    }
  }

   BsGetBit(bs, (unsigned long*)applyCrossFade, 1);
   BsGetBit(bs, &dummy, 1);

  /* read num preroll AU's */
  UsacConfig_ReadEscapedValue(bs, numPrerollAU, 2, 4, 0);
  assert(*numPrerollAU <= MPEG_3DAUDIO_CORE_MAX_AU);

  if (0 == *numPrerollAU) {
    fprintf(stderr, "\nInfo:\n\tnumPreRollFrames should be > 0 if the stream is to be used for rate adaptation.\n");
  }
  if ((0 == *applyCrossFade) &&  (0 < nFramesDone)) {
    fprintf(stderr, "\nInfo:\n\tapplyCrossfade should be 1 if the stream is to be used for rate adaptation.\n");
  }

  for ( i = 0; i < *numPrerollAU; ++i )
  {
    unsigned int auLength = 0;
    unsigned int j = 0;

    /* For every AU get length and allocate memory  to hold the data */
    UsacConfig_ReadEscapedValue(bs, &auLength, 16, 16, 0);
    prerollAU[i] = StreamFileAllocateAU(auLength * 8);

    for ( j = 0; j < auLength; ++j )
    {
      unsigned long auByte = 0;

      /* Read AU bytewise and copy into AU data buffer */
      BsGetBit(bs, &auByte, 8);
      memcpy(&(prerollAU[i]->data[j]), &auByte, 1 ); 
    }
  }

  freeMem:

    BsCloseRemove( bs, 0 );
    if ( bb )
      free(bb);

  return retVal;
}

static void mpeg_3daudio_core_resortAndScaleChannels(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float** outBuffer, unsigned int nOutSamples)
{
  unsigned int i, j = 0;
  unsigned int numOutChannels          = hUsacDec->prog->decoderConfig->audioSpecificConfig.channelConfiguration.value;

#ifndef WD1_ELEMENT_MAPPING_3D
  int chIdx = hUsacDec->prog->decoderConfig[0].audioSpecificConfig.specConf.usacConfig.channelConfigurationIndex.value;
#endif
  int *outputChannelIndex = hUsacDec->prog->decoderConfig[0].audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacChannelIndex;

#ifndef WD1_ELEMENT_MAPPING_3D
  /* No resorting possible */
  if ( -1 == ch_assign[chIdx][0] )
    noResorting = 1;
#endif

  /* Resort Signals */
  for ( i = 0; i < numOutChannels; ++i )
  { 
#ifdef WD1_ELEMENT_MAPPING_3D
    int invIdx = outputChannelIndex[i];
#else
    int invIdx = ch_assign[chIdx][i];

    if ( noResorting )
      invIdx = i;
#endif

    for ( j = 0; j < nOutSamples; ++j )
    {
      outBuffer[invIdx][j] = hUsacDec->decData->usacData->sampleBuf[i][j] / 32768.f;
    }
  }
}


static int mpeg_3daudio_core_resortQMFBuffers(MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec, float ***qmfReal_outBuffer, float ***qmfImag_outBuffer,  int *nTimeSlots, int *nQmfBands )
{
  float (*tmpQmfBufferReal)[MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS][MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS];
  float (*tmpQmfBufferImag)[MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS][MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS];

  int ts, band, channel;
 
  int coreSbrFrameLengthIndex = hUsacDec->prog->decoderConfig->audioSpecificConfig.specConf.usacConfig.coreSbrFrameLengthIndex.value;
  int numOutChannels          = hUsacDec->prog->decoderConfig->audioSpecificConfig.channelConfiguration.value;
  int *outputChannelIndex     = hUsacDec->prog->decoderConfig[0].audioSpecificConfig.specConf.usacConfig.usacDecoderConfig.usacChannelIndex;

  *nTimeSlots = 0;
  *nQmfBands  = 0;

  tmpQmfBufferReal = malloc(sizeof(float[MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS][MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS][MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS]));
  tmpQmfBufferImag = malloc(sizeof(float[MPEG_3DAUDIO_CORE_MAX_NUM_TIMESLOTS][MPEG_3DAUDIO_CORE_MAX_NUM_OUT_CHANNELS][MPEG_3DAUDIO_CORE_MAX_NUM_QMFBANDS]));

  if (NULL == tmpQmfBufferReal || NULL == tmpQmfBufferImag) {
    return -1;
  }

  switch ( coreSbrFrameLengthIndex )
  {
  case 0:
  case 1:
    return -1;
  break;

  case 2:
  case 3:
    *nTimeSlots = 32;
    *nQmfBands  = 64;
  break;
  case 4:
    *nTimeSlots = 64;
    *nQmfBands  = 64;
  break;
  default:
    return -1;
  }


   /* Copy unsorted QMF into temp buffer */
  for ( channel = 0; channel < numOutChannels; ++channel ) {
    for ( ts = 0; ts < *nTimeSlots; ++ts ) {
      for ( band = 0; band < *nQmfBands; ++band ) {
        tmpQmfBufferReal[ts][channel][band] = hUsacDec->qmfRealOutBuffer[ts][channel][band];
        tmpQmfBufferImag[ts][channel][band] = hUsacDec->qmfImagOutBuffer[ts][channel][band];
      }
    }
  }

   for ( channel = 0; channel < numOutChannels; ++channel ) {
    for ( ts = 0; ts < *nTimeSlots; ++ts ) {
      for ( band = 0; band < *nQmfBands; ++band ) {
        qmfReal_outBuffer[ts][outputChannelIndex[channel]][band] = tmpQmfBufferReal[ts][channel][band];
        qmfImag_outBuffer[ts][outputChannelIndex[channel]][band] = tmpQmfBufferImag[ts][channel][band];
      }
    }
  }

   free(tmpQmfBufferReal);
   free(tmpQmfBufferImag);

  return 0;
}


/* sbrDataInit code taken from decifc.c function audioDecInit() */

static void mpeg_3daudio_core_sbrDataInit(DEC_DATA *decData, FRAME_DATA* fD)
{
  int HEaacProfileLevel = 5;
  int bUseHQtransposer = 0;

  if (decData->usacData != NULL) 
  {
    AUDIO_SPECIFIC_CONFIG * hAsc = &fD->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig;
    USAC_CONFIG      *pUsacConfig     = &(fD->od->ESDescriptor[0]->DecConfigDescr.audioSpecificConfig.specConf.usacConfig);
    int bs_interTes[MAX_NUM_ELEMENTS] = {0};
    int bs_pvc[MAX_NUM_ELEMENTS] = {0};
    int harmonicSBR[MAX_NUM_ELEMENTS] = {0};

    USAC_DECODER_CONFIG * pUsacDecoderConfig = &pUsacConfig->usacDecoderConfig;
    int const nElements = pUsacDecoderConfig->numElements;
    int elemIdx = 0;

    for(elemIdx = 0; elemIdx < nElements; elemIdx++)
    {
      USAC_SBR_CONFIG  *pUsacSbrConfig  = UsacConfig_GetUsacSbrConfig(pUsacConfig, elemIdx);
      bs_interTes[elemIdx] = (pUsacSbrConfig != NULL)?pUsacSbrConfig->bs_interTes.value:0;
      bs_pvc[elemIdx] = (pUsacSbrConfig != NULL)?pUsacSbrConfig->bs_pvc.value:0;
      harmonicSBR[elemIdx] = (pUsacSbrConfig != NULL) ? pUsacSbrConfig->harmonicSBR.value:0;
    }

    decData->usacData->bDownSampleSbr = 0;

    if (HEaacProfileLevel < 5 && hAsc->samplingFrequency.value > 24000) 
    {
      decData->usacData->bDownSampleSbr = 1;
    }

    if (decData->usacData->sbrRatioIndex > 0) 
    { 
      if (hAsc->extensionSamplingFrequency.value == hAsc->samplingFrequency.value) 
      {
        decData->usacData->bDownSampleSbr = 1;
      }
      if (decData->usacData->bDownSampleSbr == 0) {
        if (decData->usacData->sbrRatioIndex == 1) {
          fD->scalOutSamplingFrequency = 4 * fD->scalOutSamplingFrequency;
        } else {
          fD->scalOutSamplingFrequency = 2 * fD->scalOutSamplingFrequency;
        }
      }

      /* Initiate the SBR decoder. */
      decData->usacData->ct_sbrDecoder = openSBR(fD->layer->sampleRate,
                                                 decData->usacData->bDownSampleSbr,
                                                 decData->usacData->block_size_samples,
                                                 (SBR_RATIO_INDEX)decData->usacData->sbrRatioIndex,
#ifdef AAC_ELD
                                                 0,
                                                 NULL,
#endif
                                                 harmonicSBR,
                                                 bs_interTes,
                                                 bs_pvc,
                                                 bUseHQtransposer);

      if ( decData->usacData->ct_sbrDecoder == NULL )
      {
        CommonExit(1,"can't open SBR decoder\n") ;
      }

      for(elemIdx = 0; elemIdx < nElements; elemIdx++)
      {
        USAC_ELEMENT_TYPE usacElType = pUsacConfig->usacDecoderConfig.usacElementType[elemIdx];

        if((usacElType != USAC_ELEMENT_TYPE_LFE) && (usacElType != USAC_ELEMENT_TYPE_EXT))
        {
          USAC_SBR_CONFIG  *pUsacSbrConfig  = UsacConfig_GetUsacSbrConfig(pUsacConfig, elemIdx);
          SBRDEC_USAC_SBR_HEADER usacDfltHeader;

          USAC_SBR_HEADER *pConfigDfltHeader = &(pUsacSbrConfig->sbrDfltHeader);


          memset(&usacDfltHeader, 0, sizeof(SBRDEC_USAC_SBR_HEADER));

          usacDfltHeader.start_freq     = pConfigDfltHeader->start_freq.value;
          usacDfltHeader.stop_freq      = pConfigDfltHeader->stop_freq.value;
          usacDfltHeader.header_extra1  = pConfigDfltHeader->header_extra1.value;
          usacDfltHeader.header_extra2  = pConfigDfltHeader->header_extra2.value;
          usacDfltHeader.freq_scale     = pConfigDfltHeader->freq_scale.value;
          usacDfltHeader.alter_scale    = pConfigDfltHeader->alter_scale.value;
          usacDfltHeader.noise_bands    = pConfigDfltHeader->noise_bands.value;
          usacDfltHeader.limiter_bands  = pConfigDfltHeader->limiter_bands.value;
          usacDfltHeader.limiter_gains  = pConfigDfltHeader->limiter_gains.value;
          usacDfltHeader.interpol_freq  = pConfigDfltHeader->interpol_freq.value;
          usacDfltHeader.smoothing_mode = pConfigDfltHeader->smoothing_mode.value;

          initUsacDfltHeader(decData->usacData->ct_sbrDecoder, &usacDfltHeader, elemIdx);
        }
      }

      { /* clear qmf interface */
        int s,c;
        for (s=0;s<TIMESLOT_BUFFER_SIZE;++s)
        {
          for (c=0;c<QMF_BUFFER_SIZE; ++c)
          {
            decData->usacData->sbrQmfBufferReal[s][c] = .0f;
            decData->usacData->sbrQmfBufferImag[s][c] = .0f;
          }
        }
      }
    }
  }
}

static int mpeg_3daudio_core_frameDataFree(DEC_DATA* decData)
{
  int layer = 0;
  for ( layer = 0; layer < (signed)decData->frameData->scalOutSelect+1; layer++ )
  {
    int decoderStreams = 0;
    for (decoderStreams=0; decoderStreams < 1; decoderStreams++)
    {
      free( decData->frameData->layer[decoderStreams].AULength );
      free( decData->frameData->layer[decoderStreams].AUPaddingBits );
      if ( decData->frameData->layer[decoderStreams].bitBuf ) 
        BsFreeBuffer( decData->frameData->layer[decoderStreams].bitBuf);

      if ( decData->frameData->od )
        closeESDescr(&(decData->frameData->od->ESDescriptor[decoderStreams]));
    }
  }
  
  if ( decData->frameData->od )
    free( decData->frameData->od );

  return 0;
}


/* frameDataInit function taken from decifc.c */

static int mpeg_3daudio_core_frameDataInit(int                 allTracks,
                                           int                 streamCount,            /* in: number of ESs */
                                           int                 maxLayer,
                                           DEC_CONF_DESCRIPTOR decConf [MAX_TRACKS],
                                           DEC_DATA*           decData
) {
  int         layer;
  int         track;
  int         decoderStreams;
  FRAME_DATA* fD;

  fD = decData->frameData;

  /* save original layer number for ER_BSAC */
  fD->bsacDecLayer = maxLayer;

  if (maxLayer != -1) {
    allTracks = maxLayer+1;
  }

  if (streamCount < allTracks) {
    CommonWarning("audioObjectType of last %d Tracks not implemented, decoding only %ld Tracks", allTracks-streamCount, streamCount);
  }


  /* determine real number of layers and streams */
  if (maxLayer < 0) maxLayer = streamCount-1;
  if (decConf[0].audioSpecificConfig.audioDecoderType.value == ER_BSAC)
    maxLayer = fD->scalOutSelect = streamCount-1;
  tracksPerLayer(decConf, &maxLayer, &streamCount, fD->tracksInLayer, NULL /* not interested in ext payload track count */);
  fD->scalOutSelect = maxLayer;

  /* now allocate internal structures */
  if(!(fD->od = (OBJECT_DESCRIPTOR *)calloc(1,sizeof(OBJECT_DESCRIPTOR)))) return -1;

  decData->usacData->mpegh3daProfileLevelIndication = MPEG3DA_PLI_UNKNOWN;

  if (decConf[0].MPEGHAudioProfileLevel.value < MPEG3DA_PLI_UNKNOWN) {
    decData->usacData->mpegh3daProfileLevelIndication = (MPEG3DAPROFILELEVEL)decConf[0].MPEGHAudioProfileLevel.value;
  }

  initObjDescr(fD->od);
  presetObjDescr(fD->od);

  decoderStreams = track = 0;
  for ( layer = 0; layer < (signed)fD->scalOutSelect+1; layer++ ) {
    DEC_CONF_DESCRIPTOR *dC = &decConf[track];

    for (decoderStreams = 0; decoderStreams < 1; decoderStreams++) {
      fD->layer[decoderStreams].AULength = (unsigned int *)malloc(MPEG_3DAUDIO_CORE_MAX_AU*sizeof(unsigned int));
      fD->layer[decoderStreams].AUPaddingBits = (unsigned int *)malloc(MPEG_3DAUDIO_CORE_MAX_AU*sizeof(unsigned int));
      fD->layer[decoderStreams].bitBuf = BsAllocBuffer(MAX_BITBUF);

      initESDescr(&(fD->od->ESDescriptor[decoderStreams]));
      fD->od->ESDescriptor[decoderStreams]->DecConfigDescr = *dC;

      fD->layer[decoderStreams].sampleRate = dC->audioSpecificConfig.samplingFrequency.value ;
      fD->layer[decoderStreams].bitRate = dC->avgBitrate.value;

      
    }
    track += fD->tracksInLayer[layer];
  }

  fD->od->streamCount.value = decoderStreams;

  return decoderStreams;
}

/* frameDataAddAccessUnit function taken from mp4ifc.c */
static int mpeg_3daudio_core_frameDataAddAccessUnit(DEC_DATA *decData,
                                  HANDLE_STREAM_AU au,
                                  int track)
{
  unsigned long unitSize, unitPad;
  HANDLE_BSBITBUFFER bb;
  HANDLE_BSBITSTREAM bs;
  HANDLE_BSBITBUFFER AUBuffer  = NULL;
  int idx = track;

  unitSize = (au->numBits+7)/8;
  unitPad = (unitSize*8) - au->numBits;

  bb = BsAllocPlainDirtyBuffer(au->data, /*au->numBits*/ unitSize<<3);
  bs = BsOpenBufferRead(bb);

  AUBuffer = decData->frameData->layer[idx].bitBuf ;
  if (AUBuffer!=0 ) {
    if ((BsBufferFreeBits(AUBuffer)) > (long)unitSize*8 ) {
      int auNumber = decData->frameData->layer[idx].NoAUInBuffer;
      BsGetBufferAppend(bs, AUBuffer, 1, unitSize*8);
      decData->frameData->layer[idx].AULength[auNumber]= unitSize*8;
      decData->frameData->layer[idx].AUPaddingBits[auNumber]= unitPad;
      decData->frameData->layer[idx].NoAUInBuffer++;/* each decoder must decrease this by the number of decoded AU */       
      /* each decoder must remove the first element in the list  by shifting the elements down */
      DebugPrintf(6,"AUbuffer %2i: %i AUs, last size %i, lastPad %i\n",idx, decData->frameData->layer[idx].NoAUInBuffer, decData->frameData->layer[idx].AULength[auNumber], decData->frameData->layer[idx].AUPaddingBits[auNumber]);
    } else {
      BsGetSkip(bs,unitSize*8);
      CommonWarning (" input buffer overflow for layer %d ; skipping next AU",idx);
    }
  } else {
    BsGetSkip(bs,unitSize*8);
  }
  BsCloseRemove (bs, 0);
  free(bb);
  return 0;
}
