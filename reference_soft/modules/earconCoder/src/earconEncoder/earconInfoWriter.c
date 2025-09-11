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
#include "earconInfoWriter.h"

/* OTHER INCLUDES */
#include "writeonlybitbuf.h"


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

static int isError(enum EARCON_INFO_WRITER_RETURN retval){
  return (retval != EARCON_INFO_WRITER_RETURN_NO_ERROR);
}


static enum EARCON_INFO_WRITER_RETURN  writeLanguage(
  wobitbufHandle                                  bitbuf,
  struct EARCON_TEXT_LABEL_LANGUAGE const * const language
){
  enum EARCON_INFO_WRITER_RETURN retVal = EARCON_INFO_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! language ){
    retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE DATA */
  if( ! isError(retVal) ){
    int ret = 0;
    unsigned int k;
    unsigned int c;

    for( k = 0; k < 3; k++ ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, language->language[k], 8); /* write earconLanguage */
    } /* for k */

    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, language->textDataLength, 8); /* write earconTextDataLength */

    for( c = 0; c < language->textDataLength; c++){
      if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, language->textData[c], 8); /* write earconTextData */
    } /* for c */
    
    /* map wobitbuf error */
    if( ret !=0 ) {
      retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}

static enum EARCON_INFO_WRITER_RETURN  writeEarconTextLabel(
  wobitbufHandle                         bitbuf,
  struct EARCON_TEXT_LABEL const * const textLabel
){
  enum EARCON_INFO_WRITER_RETURN retVal = EARCON_INFO_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! textLabel ){
    retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE TEXT LABEL */
  if( ! isError(retVal) ){
    if( ! isError(retVal) ){
      int ret = wobitbuf_WriteBits( bitbuf, textLabel->numLanguages, 4); /* write earconNumLanguages */
      if( ret != 0 ){
        retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
      }
    }

    if( ! isError(retVal) ){
      unsigned int n;
      for( n = 0; n < textLabel->numLanguages; n++){
        retVal = writeLanguage( bitbuf, &textLabel->language[n]);
        if( isError(retVal) ) break;
      } /* for n */
    }
  }

  return retVal;
}

static enum EARCON_INFO_WRITER_RETURN writeEarcon(
  wobitbufHandle              bitbuf,
  struct EARCON const * const earcon
){
  enum EARCON_INFO_WRITER_RETURN retVal = EARCON_INFO_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! earcon ){
    retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE EARCON (UP TO TEXT LABEL) */
  if( ! isError(retVal) ){
    int ret = 0;

    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->isIndependent, 1); /* write earconIsIndependent */
    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->id, 7); /* write earconID */
    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->type, 4); /* write earconType */
    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->active, 1); /* write earconActive */

    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->positionType, 2); /* write earconPositionType */

    if( earcon->positionType == 0 ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->CICPspeakerIdx, 7); /* write earcon_CICPspeakerIdx */
    } else {
      if( earcon->positionType == 1 ){
        if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->azimuth, 8); /* write earcon_azimuth */
        if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->elevation, 6); /* write earcon_elevation */
        if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->distance, 9); /* write earcon_distance */
      }
    }

    if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->hasGain, 1); /* write earconHasGain */
    if( earcon->hasGain ){
      if( ret == 0 ) ret = wobitbuf_WriteBits( bitbuf, earcon->gain, 7); /* write earcon_gain */
    }

    /* map wobitbuf error */
    if( ret !=0 ) {
      retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE TEXT LABEL */
  if( ! isError(retVal) ){

    if( ! isError(retVal) ){
      int ret = wobitbuf_WriteBits( bitbuf, earcon->hasTextLabel, 1); /* write earconHasTextLabel */
      if( ret != 0 ){
        retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
      }
    }

    if( ! isError(retVal) ){
      if( earcon->hasTextLabel ){
        retVal = writeEarconTextLabel( bitbuf, &earcon->textLabel);
      }
    }
  }

  return retVal;
}
/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! writes earconInfo() payload */
enum EARCON_INFO_WRITER_RETURN earconInfoWriter_WritePayload(
  struct EARCON_INFO const * const earconInfo,        /**< struct containing information to be written */
  unsigned char            * const buffer,            /**< buffer to write earconInfo() payload to */
  size_t                     const maxNumBytesToWrite,/**< number of bytes to write */
  size_t                   * const numBytesWritten    /**< number of bytes written for earconInfo() */
){
  enum EARCON_INFO_WRITER_RETURN retVal = EARCON_INFO_WRITER_RETURN_NO_ERROR;

  wobitbuf bitbuf;

  /* SANITY CHECKS */
  if( ! earconInfo || ! buffer || ! numBytesWritten ){
    retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* INIT BITBUF */
  if( ! isError(retVal) ){
    int ret = wobitbuf_Init(&bitbuf, buffer, maxNumBytesToWrite*CHAR_BIT, 0);
    if( ret != 0){
      retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE earconInfo() */
  if( ! isError(retVal) ){

    /* write numEarcons */
    if( ! isError(retVal) ){
      int ret = wobitbuf_WriteBits( &bitbuf, earconInfo->numEarcons, 7);
      if( ret != 0 ){
        retVal = EARCON_INFO_WRITER_RETURN_UNKNOWN_ERROR;
      }
    }

    /* write earcons */
    if( ! isError(retVal) ){
      unsigned int i;
      for( i = 0; i < earconInfo->numEarcons + 1; i++){
        retVal = writeEarcon(&bitbuf, &earconInfo->earcons[i]);

        if( isError(retVal) ) break;
      } /* for i */
    }
  }
  
  /* DO BYTE ALIGNMENT */
  if( ! isError(retVal) ){
    wobitbuf_ByteAlign(&bitbuf);
  }

  if( ! isError(retVal) ){
    *numBytesWritten = wobitbuf_GetBitsWritten(&bitbuf) / CHAR_BIT;
  }

  return retVal;
}

