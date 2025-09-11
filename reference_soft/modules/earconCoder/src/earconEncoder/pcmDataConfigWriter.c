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
#include "pcmDataConfigWriter.h"

/* OTHER INCLUDES */
#include "writeonlybitbuf.h"

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

static int isError(enum PCM_DATA_CONFIG_WRITER_RETURN retval){
  return (retval != PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR);
}


/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! writes pcmDataConfig() payload */
enum PCM_DATA_CONFIG_WRITER_RETURN pcmDataConfigWriter_WritePayload(
  struct PCM_DATA_CONFIG const * const pcmDataConfig,     /**< struct containing information to be written */
  unsigned char                * const buffer,            /**< buffer to write pcmDataConfig() payload to */
  size_t                         const maxNumBytesToWrite,/**< number of bytes to write */
  size_t                       * const numBytesWritten    /**< number of bytes written for pcmDataConfig() */
){
  enum PCM_DATA_CONFIG_WRITER_RETURN retVal = PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR;

  wobitbuf bitbuf;

  /* SANITY CHECKS */
  if( ! pcmDataConfig || ! buffer || ! numBytesWritten ){
    retVal = PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR;
  }

  /* INIT BITBUF */
  if( ! isError(retVal) ){
    int ret = wobitbuf_Init(&bitbuf, buffer, maxNumBytesToWrite*CHAR_BIT, 0);
    if( ret != 0){
      retVal = PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR;
    }
  }

  /* WRITE pcmDataConfig() */
  if( ! isError(retVal) ){
    int ret = 0;
    unsigned int i;

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->numPcmSignals, 7);

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmAlignAudioFlag, 1);

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmSamplingRateIdx, 5);
    if( pcmDataConfig->pcmSamplingRateIdx == 0x1f ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmSamplingRate, 24);
    }

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmBitsPerSampleIdx, 3);

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmFrameSizeIndex, 3);
    if( pcmDataConfig->pcmFrameSizeIndex == 5 ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmFixFrameSize, 16);
    }

    for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmSignal_ID[i], 7);
    }

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->bsPcmLoudnessValue, 8);

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->pcmHasAttenuationGain, 2);
    if( pcmDataConfig->pcmHasAttenuationGain ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataConfig->bsPcmAttenuationGain, 8);
    }

    /* map bitbuf error */
    if( ret != 0 ){
      retVal = PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR;
    }
  }
  
  /* DO BYTE ALIGNMENT */
  if( ! isError(retVal) ){
    wobitbuf_ByteAlign(&bitbuf);
  }

  if( ! isError(retVal) ){
    *numBytesWritten = wobitbuf_GetBitsWritten(&bitbuf) / CHAR_BIT;
  }

  return retVal;
}

