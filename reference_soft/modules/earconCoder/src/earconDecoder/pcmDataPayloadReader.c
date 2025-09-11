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
#include "pcmDataPayloadReader.h"
#include "twosComplement.h"

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
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum PCM_DATA_PAYLOAD_READER_RETURN retval){
  return (retval != PCM_DATA_PAYLOAD_READER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/


/*! parses pcmDataPayload() payload */
enum PCM_DATA_PAYLOAD_READER_RETURN pcmDataPayloadReader_ParsePayload(
  unsigned char          const * const pcmDataPayload,
  size_t                         const numBytesInPcmDataPayload,
  struct PCM_DATA_CONFIG const * const pcmDataConfig,
  struct PCM_DATA_PAYLOAD      * const pcmDataPayloadStruct
){
  enum PCM_DATA_PAYLOAD_READER_RETURN retVal = PCM_DATA_PAYLOAD_READER_RETURN_NO_ERROR;

  robitbuf bitbuf;

  /* SANITY CHECKS */
  if( ( numBytesInPcmDataPayload > 0 && ! pcmDataPayload)  || ! pcmDataPayloadStruct || ! pcmDataConfig ){
    retVal = PCM_DATA_PAYLOAD_READER_RETURN_UNKNOWN_ERROR;
  } else if( ! pcmDataConfig ){
    retVal = PCM_DATA_PAYLOAD_READER_RETURN_NO_PCM_DATA_CONFIG_GIVEN;
  }

  /* RESET PCM DATA PAYLOAD STRUCT */
  if( ! isError(retVal) ){
    memset(pcmDataPayloadStruct, 0, sizeof(*pcmDataPayloadStruct));
  }

  /* INIT BITBUFFER */
  if( ! isError(retVal) ){
    robitbuf_Init(&bitbuf, pcmDataPayload, 8 * numBytesInPcmDataPayload, 0);
  }

  /* PARSE PAYLOAD */
  if( ! isError(retVal) ){
    unsigned int frameSize;

    pcmDataPayloadStruct->numPcmSignalsInFrame = robitbuf_ReadBits(&bitbuf, 7);

    {
      unsigned int i;
      for( i = 0; i < pcmDataPayloadStruct->numPcmSignalsInFrame + 1; i++ ){
        pcmDataPayloadStruct->pcmSignal_ID[i] = robitbuf_ReadBits(&bitbuf, 7);
      } /* for i */
    }

    if( pcmDataConfig->pcmHasAttenuationGain == 2 ){
      pcmDataPayloadStruct->bsPcmAttenuationGain = robitbuf_ReadBits(&bitbuf, 8);
    }

    if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE ){
      pcmDataPayloadStruct->pcmVarFrameSize = robitbuf_ReadBits(&bitbuf, 16);
    }

    frameSize = getPcmDataPayloadFrameSize(pcmDataPayloadStruct, pcmDataConfig);

    {
      unsigned int i;
      for( i = 0; i < frameSize; i++){
        unsigned int j;
        for( j = 0; j < pcmDataPayloadStruct->numPcmSignalsInFrame + 1; j++){
          const unsigned int numBits = getNumBitsForPcmBitsPerSampleIdx(pcmDataConfig->pcmBitsPerSampleIdx);
          unsigned int sample_tc = robitbuf_ReadBits(&bitbuf, numBits);

          pcmDataPayloadStruct->pcmSample[j][i] = twosComplementToFloat(sample_tc, numBits);
        } /* for j */
      } /* for i */
    }
  }

  return retVal;
}
