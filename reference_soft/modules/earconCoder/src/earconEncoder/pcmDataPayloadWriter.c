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
#include <assert.h>

/* INCLUDES OF THIS PROJECT */
#include "pcmDataPayloadWriter.h"
#include "twosComplement.h"

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
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum PCM_DATA_PAYLOAD_WRITER_RETURN retval){
  return (retval != PCM_DATA_PAYLOAD_WRITER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! writes pcmDataPayload() payload */
enum PCM_DATA_PAYLOAD_WRITER_RETURN pcmDataPayloadWriter_WritePayload(
  struct PCM_DATA_PAYLOAD const * const pcmDataPayload,    /**< struct containing information to be written */
  struct PCM_DATA_CONFIG  const * const pcmDataConfig,     /**< struct containing pcmDataConfig() information required to write pcmDataPayload() */
  unsigned char                 * const buffer,            /**< buffer to write pcmDataPayload() payload to */
  size_t                          const maxNumBytesToWrite,/**< number of bytes to write */
  size_t                        * const numBytesWritten    /**< number of bytes written for pcmDataPayload() */
){
  enum PCM_DATA_PAYLOAD_WRITER_RETURN retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_NO_ERROR;

  wobitbuf bitbuf;

  unsigned int frameLength = 0;
  unsigned int numBitsPerSample = 0;

  /* SANITY CHECKS */
  if( ! pcmDataPayload || ! pcmDataConfig || ! buffer || ! numBytesWritten ){
    retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* INIT BITBUF */
  if( ! isError(retVal) ){
    int ret = wobitbuf_Init(&bitbuf, buffer, maxNumBytesToWrite*CHAR_BIT, 0);
    if( ret != 0){
      retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* DETERMINE FRAME LENGTH */
  if( ! isError(retVal) ){
    switch(pcmDataConfig->pcmFrameSizeIndex ){
      case PCM_FRAME_SIZE_INDEX_768:
        frameLength = 768;
        break;

      case PCM_FRAME_SIZE_INDEX_1024:
        frameLength = 1024;
        break;

      case PCM_FRAME_SIZE_INDEX_2048:
        frameLength = 2048;
        break;

      case PCM_FRAME_SIZE_INDEX_4096:
        frameLength = 4096;
        break;

      case PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE:
        frameLength = pcmDataConfig->pcmFixFrameSize;
        break;

      case PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE:
        frameLength = pcmDataPayload->pcmVarFrameSize;
        break;

      case PCM_FRAME_SIZE_INDEX_AS_DEF_IN_ISOIEC_230003_3:
        retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_NOT_SUPPORTED;
        break;

      default:
        retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* DETERMINE NUMBER OF BITS PER SAMPLE */
  if( ! isError(retVal) ){
    switch(pcmDataConfig->pcmBitsPerSampleIdx){
      case PCM_BITS_PER_SAMPLE_IDX_16_BITS:
        numBitsPerSample = 16;
        break;

      case PCM_BITS_PER_SAMPLE_IDX_24_BITS:
        numBitsPerSample = 24;
        break;

      case PCM_BITS_PER_SAMPLE_IDX_32_BITS:
        numBitsPerSample = 32;
        break;

      default:
         retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* CHECK WHETHER FLOAT SAMPLES ARE IN RANGE */
  if( ! isError(retVal) ){
    unsigned int i;
    for( i = 0; i < frameLength; i++){
      unsigned int j;
      for( j = 0; j < pcmDataConfig->numPcmSignals + 1; j++){
        float sample = pcmDataPayload->pcmSample[j][i];
        if( sample > 1.0f || sample < -1.0f ){
          retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_SAMPLE_NOT_IN_ALLOWED_RANGE;
        }
        if( isError(retVal) ) break;
      } /* for j */
      if( isError(retVal) ) break;
    } /* for i */
  }


  /* WRITE pcmDataPayload() */
  if( ! isError(retVal) ){
    int ret = 0;
    unsigned int i;

    if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataPayload->numPcmSignalsInFrame, 7);

    for( i = 0; i < pcmDataPayload->numPcmSignalsInFrame + 1; i++ ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataPayload->pcmSignal_ID[i], 7);
    }

    if( pcmDataConfig->pcmHasAttenuationGain == 2 ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataPayload->bsPcmAttenuationGain, 8);
    }

    if( pcmDataConfig->pcmFrameSizeIndex == 6 ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, pcmDataPayload->pcmVarFrameSize, 16);
    }

    {
      unsigned int i;
      for( i = 0; i < frameLength; i++){
        unsigned int j;
        for( j = 0; j < pcmDataConfig->numPcmSignals + 1; j++){
          float sample = pcmDataPayload->pcmSample[j][i];
          
          if( ret == 0 ) ret = wobitbuf_WriteBits( &bitbuf, floatToTwosComplement(sample, numBitsPerSample), numBitsPerSample);

        } /* for j */

      } /* for i */
    }

    /* map bitbuf error */
    if( ret != 0 ){
      retVal = PCM_DATA_PAYLOAD_WRITER_RETURN_UNKNOWN_ERROR;
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

