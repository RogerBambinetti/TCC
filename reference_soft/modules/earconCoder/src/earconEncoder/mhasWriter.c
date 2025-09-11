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

/* INCLUDES OF THIS PROJECT */
#include "mhasWriter.h"

/* OTHER INCLUDES */
#include "writeonlybitbuf.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

#define NUM_VALS_ESCAPED_VALUE (3)

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

/*! checks whether return value is an error */
static int isError(enum MHAS_WRITER_RETURN retval){
  return (retval != MHAS_WRITER_RETURN_NO_ERROR);
}

/*! calculates the N-th power of two, returns !=0 in case of an error (e.g. overflow in calculation) and 0 if everything is OK */
static enum MHAS_WRITER_RETURN NthPowerOfTwo(unsigned int const N, unsigned int * const NthPowerOfTwo) {
  const unsigned int maxN = sizeof(unsigned int) * CHAR_BIT;

  if( ! NthPowerOfTwo ){
    return MHAS_WRITER_RETURN_UNKNOWN_ERROR; /* pointer for output not valid */
  } else if( N > maxN ){
    return MHAS_WRITER_RETURN_UNKNOWN_ERROR; /* result would not fit into unsigned int */
  }

  if (N == maxN){
    *NthPowerOfTwo = UINT_MAX;
  } else {
    *NthPowerOfTwo = ( 1u << N ) - 1;
  }

  return MHAS_WRITER_RETURN_NO_ERROR;
}

/*! writes escaped value */
static enum MHAS_WRITER_RETURN writeEscapedValue(
  wobitbufHandle bitbuf,
  unsigned int   value,
  unsigned int   numBits1,
  unsigned int   numBits2,
  unsigned int   numBits3
){
  enum MHAS_WRITER_RETURN retVal = MHAS_WRITER_RETURN_NO_ERROR;
  
  unsigned int escapeVals[NUM_VALS_ESCAPED_VALUE] = { 0 };
  unsigned int numBits[NUM_VALS_ESCAPED_VALUE] = { numBits1, numBits2, numBits3 };

  /* SANITY CHECKS */
  if( ! bitbuf ){
    retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* CALC ESCAPE VALS */
  if( ! isError(retVal) ){
    size_t n;
    for( n = 0 ; n < NUM_VALS_ESCAPED_VALUE; n++){
      retVal = NthPowerOfTwo(numBits[n], &escapeVals[n]);
      if( isError(retVal) ) break;
    } /* for n */
  }

  /* WRITE ESCAPED VALUE */
  if( ! isError(retVal) ){
    unsigned int n = 0;

    for( n = 0; n < NUM_VALS_ESCAPED_VALUE; n ++){
      if ( value < escapeVals[n] ) {
        int err = wobitbuf_WriteBits ( bitbuf, value, numBits[n] );
        if( err ){
          retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
        }
        value = 0;
        break;
      } else {
        int err = wobitbuf_WriteBits ( bitbuf, escapeVals[n], numBits[n] );
        if( err ){
          retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
        }
        value -= escapeVals[n];
      }
      if( isError(retVal) ) break;
    }

    if( ! isError(retVal) ){
      if ( value ) {
        /* data too large for escapedValue! */
        retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
      }
    }
  }

  return retVal;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/


/*! write MHAS packet to file */
enum MHAS_WRITER_RETURN writePacket(
  FILE                * const mhasFile,          /**< file pointer of opened (in "wb" mode) MHAS file to write to */
  enum PACTYP           const mhasPacketType,    /**< MHAS packet type to write */
  unsigned int          const mhasPacketLabel,   /**< packet label of the MHAS packet to write */
  size_t                const mhasPacketLength,  /**< number of bytes in the MHAS packet payload */
  unsigned char const * const mhasPacketPayload  /**< pointer to a buffer containing the MHAS packet payload to be written */
){
  enum MHAS_WRITER_RETURN retVal = MHAS_WRITER_RETURN_NO_ERROR;

  wobitbuf bitbuf;

  unsigned char packetBuffer[MAX_NUM_BYTES_SUPPORTED_MHAS_PACKET] = {0};

  /* SANITY CHECKS */
  if( ! mhasFile || ! mhasPacketPayload ){
    retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* INIT BIT BUFFER */
  if( ! isError(retVal) ){
    int ret = wobitbuf_Init(&bitbuf, packetBuffer, sizeof(packetBuffer)*CHAR_BIT, 0);
    if( ret != 0 ){
      retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE PACKET TYPE */
  if( ! isError(retVal) ){
    retVal = writeEscapedValue(&bitbuf, mhasPacketType, 3, 8, 8);
  }

  /* WRITE PACKET LABEL */
  if( ! isError(retVal) ){
    retVal = writeEscapedValue(&bitbuf, mhasPacketLabel, 2, 8, 32);
  }

  /* WRITE PACKET LENGTH */
  if( ! isError(retVal) ){
    retVal = writeEscapedValue(&bitbuf, mhasPacketLength, 11, 24, 24);
  }

  /* WRITE PAYLOAD */
  if( ! isError(retVal) ){
    int ret = wobitbuf_WriteBytes(&bitbuf, mhasPacketPayload, mhasPacketLength);
    if( ret != 0 ){
      retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* CHECK BYTE ALIGNMENT */
  if( ! isError(retVal) ){
    if( wobitbuf_GetBitsWritten(&bitbuf) % CHAR_BIT != 0 ){
      retVal = MHAS_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE TO FILE */
  if( ! isError(retVal) ){
    size_t numBytesToWrite = wobitbuf_GetBitsWritten(&bitbuf) / CHAR_BIT;
    size_t numBytesWritten = fwrite(packetBuffer, sizeof(unsigned char), numBytesToWrite, mhasFile);

    if( numBytesToWrite != numBytesWritten ){
      retVal = MHAS_WRITER_RETURN_COULD_NOT_WRITE_TO_FILE;
    }
  }


  return retVal;
}


