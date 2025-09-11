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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* INCLUDES OF THIS PROJECT */
#include "bitstreamReader.h"

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

/*! handle struct */
struct BITSTREAM_READER {
  FILE * file;
  size_t numBitsInTmpBuffer;
  unsigned char tmpBuffer;

};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

/*! checks whether return value is an error */
static int isError(enum BITSTREAM_READER_RETURN retval){
  return (retval != BITSTREAM_READER_RETURN_NO_ERROR);
}

static unsigned char getBit(unsigned char byte, size_t bitIdx) {
  return ((0x01 << (CHAR_BIT-bitIdx-1)) & byte ) != 0;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! opens bitstream reader */
BITSTREAM_READER_HANDLE bitstreamReader_Open(
  char const * const filename
){
  BITSTREAM_READER_HANDLE hBitstreamReader = NULL;

  assert(CHAR_BIT == 8);

  if( ! filename ){
    return NULL;
  }

  hBitstreamReader = (BITSTREAM_READER_HANDLE)calloc( 1, sizeof(*hBitstreamReader));
  if( ! hBitstreamReader ){
    bitstreamReader_Close(&hBitstreamReader);
    return NULL;
  }

  hBitstreamReader->file = fopen(filename, "rb");
  if( ! hBitstreamReader->file ){
    bitstreamReader_Close(&hBitstreamReader);
    return NULL;
  }


  return hBitstreamReader;
}

/*! closes bitstream reader */
void bitstreamReader_Close(
  BITSTREAM_READER_HANDLE * const bitstreamReaderHandle
){
  if( bitstreamReaderHandle && *bitstreamReaderHandle ){

    if( (*bitstreamReaderHandle)->file ) fclose((*bitstreamReaderHandle)->file);

    free(*bitstreamReaderHandle);
    *bitstreamReaderHandle = NULL;
  }
}

/*! reads bits */
enum BITSTREAM_READER_RETURN bitstreamReader_ReadBits(
  BITSTREAM_READER_HANDLE   const bitstreamReader,
  size_t                    const numBits,
  unsigned int            * const value
){
  enum BITSTREAM_READER_RETURN retVal = BITSTREAM_READER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitstreamReader || ! value ){
    retVal = BITSTREAM_READER_RETURN_UNKNOWN_ERROR;
  } else if( numBits > sizeof(*value) * CHAR_BIT){
    return BITSTREAM_READER_RETURN_NUM_BITS_TO_READ_TOO_HIGH;
  }

  /* READ BITS */
  if( ! isError(retVal) ){
    size_t n;
    *value = 0;

    for (n = 0; n < numBits; n++) {
      unsigned char bit;

      if( bitstreamReader->numBitsInTmpBuffer == 0 ){

        size_t numBytesRead = fread( &bitstreamReader->tmpBuffer, sizeof(bitstreamReader->tmpBuffer), 1, bitstreamReader->file);

        if( numBytesRead != 1 ){
          if( feof(bitstreamReader->file) ){
            retVal = BITSTREAM_READER_RETURN_EOF;
          } else if( ferror(bitstreamReader->file) ){
            retVal = BITSTREAM_READER_RETURN_UNKNOWN_ERROR;
          } else {
            retVal = BITSTREAM_READER_RETURN_UNKNOWN_ERROR;
          }
        } else {
          bitstreamReader->numBitsInTmpBuffer = sizeof(bitstreamReader->tmpBuffer)*CHAR_BIT;
        }

      }

      if( ! isError(retVal) ){
        bit = getBit(bitstreamReader->tmpBuffer, CHAR_BIT - bitstreamReader->numBitsInTmpBuffer );

        *value  += bit * (1<<(numBits-n-1));

        bitstreamReader->numBitsInTmpBuffer--;
      }

      if( isError(retVal) ) break;
    } /* for n */
  }

  return retVal;
}

/*! reads escaped values */
enum BITSTREAM_READER_RETURN bitstreamReader_ReadEscapedValue(
  BITSTREAM_READER_HANDLE   const bitstreamReader,
  size_t                    const numBits1,
  size_t                    const numBits2,
  size_t                    const numBits3,
  unsigned int            * const value
){
  enum BITSTREAM_READER_RETURN retVal = BITSTREAM_READER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitstreamReader || ! value ){
    retVal = BITSTREAM_READER_RETURN_UNKNOWN_ERROR;
  }

  /* READ BITS */
  if( ! isError(retVal) ){
    const unsigned int maxValue1 = (1 << numBits1) - 1;
    const unsigned int maxValue2 = (1 << numBits2) - 1;
    *value = 0;

    /* read numBits1 */
    if( ! isError(retVal) ) retVal = bitstreamReader_ReadBits(bitstreamReader, numBits1, value);

    if( *value == maxValue1) {
      unsigned int valueAdd = 0;

      /* read numBits2 */
      if( ! isError(retVal) ) retVal = bitstreamReader_ReadBits(bitstreamReader, numBits2, &valueAdd);

      *value += valueAdd;

      if (valueAdd == maxValue2) {
        /* read numBits3 */
        if( ! isError(retVal) ) retVal = bitstreamReader_ReadBits(bitstreamReader, numBits3, &valueAdd);
        *value += valueAdd;
      }
    }

  }

  return retVal;
}


