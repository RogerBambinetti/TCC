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
#ifndef MHAS_READER_H
#define MHAS_READER_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */
#include "mhas.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for MHAS writer */
enum MHAS_READER_RETURN {
  MHAS_READER_RETURN_NO_ERROR = 0,
  MHAS_READER_RETURN_EOF,
  MHAS_READER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

typedef struct MHAS_READER * MHAS_READER_HANDLE;

struct MHAS_PACKET {
  enum PACTYP mhasPacketType;
  unsigned int mhasPacketLabel;
  size_t mhasPacketLength;
  unsigned char mhasPacketPayload[MAX_NUM_BYTES_SUPPORTED_MHAS_PACKET];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens MHAS reader */
MHAS_READER_HANDLE mhasReader_Open(
  char const * const filename
);

/*! closes MHAS reader */
void mhasReader_Close(
  MHAS_READER_HANDLE * const mhasReaderHandle
);

/*! read MHAS packet from file */
enum MHAS_READER_RETURN readPacket(
  MHAS_READER_HANDLE   const mhasReader,        /**< MHAS reader handle */
  struct MHAS_PACKET * const mhasPacket         /**< MHAS packet struct to read to */
);

#endif /* ifndef MHAS_READER_H */
