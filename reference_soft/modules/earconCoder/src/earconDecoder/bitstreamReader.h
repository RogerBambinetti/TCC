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
#ifndef BITSTREAM_READER_H
#define BITSTREAM_READER_H
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

/*! return values for bitstream writer */
enum BITSTREAM_READER_RETURN {
  BITSTREAM_READER_RETURN_NO_ERROR = 0,
  BITSTREAM_READER_RETURN_EOF,
  BITSTREAM_READER_RETURN_NUM_BITS_TO_READ_TOO_HIGH,
  BITSTREAM_READER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

typedef struct BITSTREAM_READER * BITSTREAM_READER_HANDLE;

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens bitstream reader */
BITSTREAM_READER_HANDLE bitstreamReader_Open(
  char const * const filename
);

/*! closes bitstream reader */
void bitstreamReader_Close(
  BITSTREAM_READER_HANDLE * const bitstreamReaderHandle
);

/*! reads bits */
enum BITSTREAM_READER_RETURN bitstreamReader_ReadBits(
  BITSTREAM_READER_HANDLE   const bitstreamReader,
  size_t                    const numBits,
  unsigned int            * const value
);

/*! reads escaped values */
enum BITSTREAM_READER_RETURN bitstreamReader_ReadEscapedValue(
  BITSTREAM_READER_HANDLE   const bitstreamReader,
  size_t                    const numBits1,
  size_t                    const numBits2,
  size_t                    const numBits3,
  unsigned int            * const value
);


#endif /* ifndef BITSTREAM_READER_H */
