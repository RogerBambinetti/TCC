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
#ifndef WAV_WRITER_H
#define WAV_WRITER_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for MHAS writer */
enum WAV_WRITER_RETURN {
  WAV_WRITER_RETURN_NO_ERROR = 0,
  WAV_WRITER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

typedef struct WAV_WRITER * WAV_WRITER_HANDLE;

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens WAV writer */
WAV_WRITER_HANDLE wavWriter_Open(
  char         const * const filename,
  unsigned int         const numChannels,
  unsigned int         const sampleRate,
  unsigned int         const numBitsPerSample,
  unsigned int         const numSamplesPerChannelPerFrame
);

/*! closes WAV writer */
void wavWriter_Close(
  WAV_WRITER_HANDLE * const wavWriterHandle
);

/*! write samples */
enum WAV_WRITER_RETURN wavWriter_WriteSamples(
  WAV_WRITER_HANDLE                 const wavWriter,         /**< wav writer handle */
  float             const * const * const samples            /**< sample buffer to be indexed with [channelIdx][sampleIdx] */
);

#endif /* ifndef WAV_WRITER_H */
