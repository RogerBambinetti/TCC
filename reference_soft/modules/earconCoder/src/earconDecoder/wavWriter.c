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
#include <stdlib.h>

/* INCLUDES OF THIS PROJECT */
#include "wavWriter.h"

/* OTHER INCLUDES */
#include "wavIO.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct WAV_WRITER {
  FILE * wavFile;
  WAVIO_HANDLE hWavIo;
  unsigned int numSamplesPerChannelPerFrame;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

/*! checks whether return value is an error */
static int isError(enum WAV_WRITER_RETURN retval){
  return (retval != WAV_WRITER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/


/*! opens WAV writer */
WAV_WRITER_HANDLE wavWriter_Open(
  char         const * const filename,
  unsigned int         const numChannels,
  unsigned int         const sampleRate,
  unsigned int         const numBitsPerSample,
  unsigned int         const numSamplesPerChannelPerFrame
){
  WAV_WRITER_HANDLE wavWriter = NULL;

  /* SANITY CHECKS */
  if( ! filename || numChannels < 1 ||numBitsPerSample%8 != 0 || numSamplesPerChannelPerFrame < 1){
    return NULL;
  }

  /* ALLOCATE HANDLE */
  wavWriter = (WAV_WRITER_HANDLE)calloc( 1, sizeof(*wavWriter));
  if( ! wavWriter ){
    return NULL;
  }

  /* OPEN FILE */
  wavWriter->wavFile = fopen(filename, "wb");
  if( ! wavWriter->wavFile ){
    wavWriter_Close(&wavWriter);
    return NULL;
  }

  /* OPEN WAV IO HANDLE */
  {
    int ret = wavIO_init(&wavWriter->hWavIo, numSamplesPerChannelPerFrame, 0, 0);
    if( ret != 0 || ! wavWriter->hWavIo ){
      wavWriter_Close(&wavWriter);
      return NULL;
    }
    ret = wavIO_openWrite(wavWriter->hWavIo, wavWriter->wavFile, numChannels, sampleRate, numBitsPerSample/8);
    if( ret != 0 || ! wavWriter->hWavIo ){
      wavWriter_Close(&wavWriter);
      return NULL;
    }
  }

  wavWriter->numSamplesPerChannelPerFrame = numSamplesPerChannelPerFrame;

  return wavWriter;
}

/*! closes WAV writer */
void wavWriter_Close(
  WAV_WRITER_HANDLE * const wavWriterHandle
){
  if( wavWriterHandle && *wavWriterHandle ){

    if( (*wavWriterHandle)->hWavIo ){
      unsigned long tmp = 0;
      wavIO_updateWavHeader((*wavWriterHandle)->hWavIo, &tmp);
      wavIO_close((*wavWriterHandle)->hWavIo);
    }

    /*if( (*wavWriterHandle)->wavFile ) fclose( (*wavWriterHandle)->wavFile );*/ /* is closed by wavIO_close */

    free(*wavWriterHandle);
    *wavWriterHandle = NULL;
  }
}

/*! write samples */
enum WAV_WRITER_RETURN wavWriter_WriteSamples(
  WAV_WRITER_HANDLE                 const wavWriter,         /**< wav writer handle */
  float             const * const * const samples            /**< sample buffer to be indexed with [channelIdx][sampleIdx] */
){
  enum WAV_WRITER_RETURN retVal = WAV_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if( ! wavWriter || ! samples ){
    retVal = WAV_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE SAMPLES */
  if( ! isError(retVal) ){
    unsigned int tmp = 0;
    int ret = wavIO_writeFrame(wavWriter->hWavIo, samples, wavWriter->numSamplesPerChannelPerFrame, &tmp);
    if( ret != 0 || wavWriter->numSamplesPerChannelPerFrame != tmp ){
      retVal = WAV_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}



