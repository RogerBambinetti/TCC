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

Copyright (c) ISO/IEC 2019.

*************************************************************************/
#ifndef PMF_WRITER_H
#define PMF_WRITER_H
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
#ifndef MAX_AUDIO_GROUPS
#define MAX_AUDIO_GROUPS (32)
#endif
#ifndef MAX_NUM_OBJECTS
#define MAX_NUM_OBJECTS (28)
#endif
#define MAX_NUM_BITS_INTRACODED (20 + MAX_NUM_OBJECTS * 9)           /* max: 272 bits */
#define MAX_NUM_BITS_DYNAMIC (1 + MAX_NUM_OBJECTS * (9 + 1 + 3 + 9)) /* max: 617 bits */
#define MAX_NUM_BYTES_SUPPORTED_PMF_PACKET ((7 + MAX_AUDIO_GROUPS + MAX_AUDIO_GROUPS * MAX_NUM_BITS_DYNAMIC) >> 3)

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for PMC writer */
enum PMF_WRITER_RETURN {
  PMF_WRITER_RETURN_NO_ERROR = 0,
  PMF_WRITER_RETURN_EOF,
  PMF_WRITER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
typedef struct PMF_WRITER * PMF_WRITER_HANDLE;

struct PMF_PACKET {
  size_t pmfPacketLength;
  unsigned char pmfPacketPayload[MAX_NUM_BYTES_SUPPORTED_PMF_PACKET];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens PMF writer */
PMF_WRITER_HANDLE pmfWriter_Open(
  char const * const filename,
  unsigned int const frameNumber
);

/*! closes PMF writer */
void pmfWriter_Close(
  PMF_WRITER_HANDLE * const pmfWriterHandle
);

/*! pack PMF data */
enum PMF_WRITER_RETURN pmfWriter_PackConfig(
  unsigned char const * const pmfPacketPayload,
  size_t                const pmfPacketLength,
  struct PMF_PACKET   * const pmfPacket         /**< PMF packet struct to write */
);

/*! write PMF data from file */
enum PMF_WRITER_RETURN pmfWriter_writeFrame(
  PMF_WRITER_HANDLE         const pmfWriter,    /**< PMF writer handle */
  struct PMF_PACKET const * const pmfPacket     /**< PMF packet struct to write */
);

#endif /* ifndef PMC_WRITER_H */
