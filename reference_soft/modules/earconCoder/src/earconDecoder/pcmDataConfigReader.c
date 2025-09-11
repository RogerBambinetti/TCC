/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2018.

*************************************************************************/

/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/
/* SYSTEM INCLUDES */
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* INCLUDES OF THIS PROJECT */
#include "pcmDataConfigReader.h"

/* OTHER INCLUDES */
#include "readonlybitbuf.h"

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ############################## const data ############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum PCM_DATA_CONFIG_READER_RETURN retval){
  return (retval != PCM_DATA_CONFIG_READER_RETURN_NO_ERROR);
}


/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! parses pcmDataConfig() payload */
enum PCM_DATA_CONFIG_READER_RETURN pcmDataConfigReader_ParsePayload(
  unsigned char          const * const pcmDataConfigPayload,
  size_t                         const numBytesInPcmDataConfigPayload,
  struct PCM_DATA_CONFIG       * const pcmDataConfig
){
  enum PCM_DATA_CONFIG_READER_RETURN retVal = PCM_DATA_CONFIG_READER_RETURN_NO_ERROR;
  robitbuf bitbuf;

  /* SANITY CHECKS */
  if( ( numBytesInPcmDataConfigPayload > 0 && ! pcmDataConfigPayload)  || ! pcmDataConfig ){
    retVal = PCM_DATA_CONFIG_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET PCM DATA CONFIG STRUCT */
  if( ! isError(retVal) ){
    memset(pcmDataConfig, 0, sizeof(*pcmDataConfig));
  }

  /* INIT BITBUFFER */
  if( ! isError(retVal) ){
    robitbuf_Init(&bitbuf, pcmDataConfigPayload, 8 * numBytesInPcmDataConfigPayload, 0);
  }

  /* PARSE PAYLOAD */
  if( ! isError(retVal) ){
    pcmDataConfig->numPcmSignals = robitbuf_ReadBits(&bitbuf, 7);

    pcmDataConfig->pcmAlignAudioFlag = robitbuf_ReadBits(&bitbuf, 1);

    pcmDataConfig->pcmSamplingRateIdx = (enum PCM_SAMPLING_RATE_INDEX) robitbuf_ReadBits(&bitbuf, 5);

    if( pcmDataConfig->pcmSamplingRateIdx == PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE ){
      pcmDataConfig->pcmSamplingRate = robitbuf_ReadBits(&bitbuf, 24);
    }

    pcmDataConfig->pcmBitsPerSampleIdx = (enum PCM_BITS_PER_SAMPLE_IDX) robitbuf_ReadBits(&bitbuf, 3);

    pcmDataConfig->pcmFrameSizeIndex = (enum PCM_FRAME_SIZE_INDEX) robitbuf_ReadBits(&bitbuf, 3);
    if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE ){
      pcmDataConfig->pcmFixFrameSize = robitbuf_ReadBits(&bitbuf, 16);
    }

    {
      unsigned int i;
      for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
        pcmDataConfig->pcmSignal_ID[i] = robitbuf_ReadBits(&bitbuf, 7);
      } /* for i */
    }

    pcmDataConfig->bsPcmLoudnessValue = robitbuf_ReadBits(&bitbuf, 8);

    pcmDataConfig->pcmHasAttenuationGain = robitbuf_ReadBits(&bitbuf, 2);
    if( pcmDataConfig->pcmHasAttenuationGain == 1 ){
      pcmDataConfig->bsPcmAttenuationGain = robitbuf_ReadBits(&bitbuf, 8);;
    }
  }

  return retVal;
}

