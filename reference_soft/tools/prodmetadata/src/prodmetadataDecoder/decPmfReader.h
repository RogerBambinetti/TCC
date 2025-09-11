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
#ifndef PMF_READER_H
#define PMF_READER_H
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

/*! return values for PMC reader */
enum PMF_READER_RETURN {
  PMF_READER_RETURN_NO_ERROR = 0,
  PMF_READER_RETURN_EOF,
  PMF_READER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
typedef struct PMF_READER * PMF_READER_HANDLE;

struct PMF_PACKET {
  size_t pmfPacketLength;
  unsigned char pmfPacketPayload[MAX_NUM_BYTES_SUPPORTED_PMF_PACKET];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens PMF reader */
PMF_READER_HANDLE pmfReader_Open(
  char const * const filename,
  unsigned int const frameNumber
);

/*! closes PMF reader */
void pmfReader_Close(
  PMF_READER_HANDLE * const pmfReaderHandle
);

  /*! read PMF data from file */
enum PMF_READER_RETURN pmfReader_readFrame(
  PMF_READER_HANDLE   const pmfReader,          /**< PMF reader handle */
  struct PMF_PACKET * const pmfPacket           /**< PMF packet struct to read to */
);

#endif /* ifndef PMC_READER_H */
