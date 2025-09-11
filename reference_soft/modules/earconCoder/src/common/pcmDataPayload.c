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
#include "pcmDataPayload.h"
#include "printHelper.h"

/* OTHER INCLUDES */


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

static int isError(enum PCM_DATA_PAYLOAD_RETURN retval){
  return (retval != PCM_DATA_PAYLOAD_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! gets frameSize for a pcmDataPayload */
unsigned int getPcmDataPayloadFrameSize(
  struct PCM_DATA_PAYLOAD const * const pcmDataPayload,    /**< struct containing pcmDataPayload() information */
  struct PCM_DATA_CONFIG  const * const pcmDataConfig      /**< struct containing pcmDataConfig() information */
){
  assert(pcmDataPayload);
  assert(pcmDataConfig);

  switch(pcmDataConfig->pcmFrameSizeIndex){
    case PCM_FRAME_SIZE_INDEX_AS_DEF_IN_ISOIEC_230003_3:
      assert(0);
      return 0;

    case PCM_FRAME_SIZE_INDEX_768:
      return 768;

    case PCM_FRAME_SIZE_INDEX_1024:
      return 1024;

    case PCM_FRAME_SIZE_INDEX_2048:
      return 2048;

    case PCM_FRAME_SIZE_INDEX_4096:
      return 4096;

    case PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE:
      return pcmDataConfig->pcmFixFrameSize;

    case PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE:
      return pcmDataPayload->numPcmSignalsInFrame;

    default:
      assert(0);
      return 0;
  }

}

/*! prints out content of the PCM data payload struct */
enum PCM_DATA_PAYLOAD_RETURN pcmDataPayload_Print(
  FILE                          *       file,
  struct PCM_DATA_CONFIG  const * const pcmDataConfig,
  struct PCM_DATA_PAYLOAD const * const pcmDataPayload,
  unsigned int                    const numSpacesIndentation
){
  enum PCM_DATA_PAYLOAD_RETURN retVal = PCM_DATA_PAYLOAD_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! file || ! pcmDataConfig ||! pcmDataPayload ){
    retVal = PCM_DATA_PAYLOAD_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PCM DATA PAYLOAD */
  if( ! isError(retVal) ){
    if (stdout == file) {
      unsigned int i;
      fprintf_indented(file, numSpacesIndentation, "numPcmSignalsInFrame = %u\n", pcmDataPayload->numPcmSignalsInFrame);

      for( i = 0; i < pcmDataPayload->numPcmSignalsInFrame + 1; i++){
        fprintf_indented(file, numSpacesIndentation, "pcmSignal_ID[%u] = %u\n", i, pcmDataPayload->pcmSignal_ID[i]);
      } /* for i */

      if( pcmDataConfig->pcmHasAttenuationGain == 2 ){
        fprintf_indented(file, numSpacesIndentation, "bsPcmAttenuationGain = %u\n", pcmDataPayload->bsPcmAttenuationGain);
      }

      if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE ){
        fprintf_indented(file, numSpacesIndentation, "pcmVarFrameSize = %u\n", pcmDataPayload->pcmVarFrameSize);
      }

      fprintf_indented(file, numSpacesIndentation, "(actual sample values are not printed here)\n");
    }
  }

  return retVal;
}

