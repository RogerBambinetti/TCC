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
#include "mhasReader.h"

/* OTHER INCLUDES */
#include "bitstreamReader.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define MAX_NUM_BYTES_MHAS_PACKET_PAYLOAD (1024*10)
/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct MHAS_READER {
  BITSTREAM_READER_HANDLE bitstreamReader;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

/*! checks whether return value is an error */
static int isError(enum MHAS_READER_RETURN retval){
  return (retval != MHAS_READER_RETURN_NO_ERROR);
}

static enum MHAS_READER_RETURN map_BITSTREAM_READER_RETURN( enum BITSTREAM_READER_RETURN bsReadRet ){
  switch(bsReadRet){
    case BITSTREAM_READER_RETURN_NO_ERROR:
      return MHAS_READER_RETURN_NO_ERROR;

    case BITSTREAM_READER_RETURN_EOF:
      return MHAS_READER_RETURN_EOF;

    case BITSTREAM_READER_RETURN_NUM_BITS_TO_READ_TOO_HIGH:
    case BITSTREAM_READER_RETURN_UNKNOWN_ERROR:
    default:
      return MHAS_READER_RETURN_UNKNOWN_ERROR;
  }
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/


/*! opens MHAS reader */
MHAS_READER_HANDLE mhasReader_Open(
  char const * const filename
){
  MHAS_READER_HANDLE mhasReader = NULL;

  /* ALLOCATE HANDLE */
  mhasReader = (MHAS_READER_HANDLE)calloc( 1, sizeof(*mhasReader));
  if( ! mhasReader ){
    return NULL;
  }

  /* OPEN FILE */
  mhasReader->bitstreamReader = bitstreamReader_Open(filename);
  if( ! mhasReader->bitstreamReader ){
    mhasReader_Close(&mhasReader);
    return NULL;
  }

  return mhasReader;
}

/*! closes MHAS reader */
void mhasReader_Close(
  MHAS_READER_HANDLE * const mhasReaderHandle
){
  if( mhasReaderHandle && *mhasReaderHandle ){

    bitstreamReader_Close(&(*mhasReaderHandle)->bitstreamReader);

    free(*mhasReaderHandle);
    *mhasReaderHandle = NULL;
  }
}

/*! read MHAS packet from file */
enum MHAS_READER_RETURN readPacket(
  MHAS_READER_HANDLE   const mhasReader,        /**< MHAS reader handle */
  struct MHAS_PACKET * const mhasPacket         /**< MHAS packet struct to read to */
){
  enum MHAS_READER_RETURN retVal = MHAS_READER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if( ! mhasReader || ! mhasPacket ){
    retVal = MHAS_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET OUTPUT */
  if( ! isError(retVal) ){
    memset( mhasPacket, 0, sizeof(*mhasPacket));
  }

  /* READ PACKET TYPE */
  if( ! isError(retVal) ){
    unsigned int tmp = 0;
    enum BITSTREAM_READER_RETURN bsReadRet = bitstreamReader_ReadEscapedValue(
      mhasReader->bitstreamReader,
      3,
      8,
      8,
      &tmp
    );
    retVal = map_BITSTREAM_READER_RETURN(bsReadRet);
    if( ! isError(retVal) ) mhasPacket->mhasPacketType = (enum PACTYP) tmp;
  }

  /* READ PACKET LABEL */
  if( ! isError(retVal) ){
    enum BITSTREAM_READER_RETURN bsReadRet = bitstreamReader_ReadEscapedValue(
      mhasReader->bitstreamReader,
      2,
      8,
      32,
      &mhasPacket->mhasPacketLabel
    );
    retVal = map_BITSTREAM_READER_RETURN(bsReadRet);
  }

  /* READ PACKET LENGTH */
  if( ! isError(retVal) ){
    unsigned int packetLengthTmp = 0;
    enum BITSTREAM_READER_RETURN bsReadRet = bitstreamReader_ReadEscapedValue(
      mhasReader->bitstreamReader,
      11,
      24,
      24,
      &packetLengthTmp
    );
    mhasPacket->mhasPacketLength = (size_t)packetLengthTmp;
    retVal = map_BITSTREAM_READER_RETURN(bsReadRet);
  }

  /* READ PACKET PAYLOAD */
  if( ! isError(retVal) ){
    size_t byteIdx;

    for( byteIdx = 0; byteIdx < mhasPacket->mhasPacketLength; byteIdx++){
      unsigned int tmp = 0;
      enum BITSTREAM_READER_RETURN bsReadRet = bitstreamReader_ReadBits(
        mhasReader->bitstreamReader,
        8,
        &tmp
      );
      retVal = map_BITSTREAM_READER_RETURN(bsReadRet);

      if( ! isError(retVal) ){
        if( tmp <= UCHAR_MAX ){
          mhasPacket->mhasPacketPayload[byteIdx] = (unsigned char)tmp;
        } else {
          retVal = MHAS_READER_RETURN_UNKNOWN_ERROR;
        }
      }

      if( isError(retVal) ) break;
    } /* for byteIdx */
  }


  return retVal;
}



