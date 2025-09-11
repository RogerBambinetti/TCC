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
#include "earconInfoReader.h"
#include "twosComplement.h"

/* OTHER INCLUDES */
#include "readonlybitbuf.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

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
static int isError(enum EARCON_INFO_READER_RETURN retval){
  return (retval != EARCON_INFO_READER_RETURN_NO_ERROR);
}

static enum EARCON_INFO_READER_RETURN parse_language(
  robitbufHandle                      const bitbuf,
  struct EARCON_TEXT_LABEL_LANGUAGE * const language
){
  enum EARCON_INFO_READER_RETURN retVal = EARCON_INFO_READER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! bitbuf ){
    retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
  }


  if( ! isError(retVal) ){
    unsigned int c;
    language->language[0] = (char)robitbuf_ReadBits(bitbuf, 8);
    language->language[1] = (char)robitbuf_ReadBits(bitbuf, 8);
    language->language[2] = (char)robitbuf_ReadBits(bitbuf, 8);

    language->textDataLength = robitbuf_ReadBits(bitbuf, 8);

    for( c = 0; c < language->textDataLength; c++){

      if( c >= ARRAY_LENGTH(language->textData) ){
        retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
      } else {
        language->textData[c] = (char)robitbuf_ReadBits(bitbuf, 8);
      }

      if( isError(retVal) ) break;
    }
  }

  return retVal;
}

static enum EARCON_INFO_READER_RETURN parse_textLabel(
  robitbufHandle             const bitbuf,
  struct EARCON_TEXT_LABEL * const textLabel
){
  enum EARCON_INFO_READER_RETURN retVal = EARCON_INFO_READER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! textLabel ){
    retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
  }

  if( ! isError(retVal) ){
    unsigned int n;
    textLabel->numLanguages = robitbuf_ReadBits(bitbuf, 4);

    for( n = 0; n < textLabel->numLanguages; n++){

      if( n >= ARRAY_LENGTH(textLabel->language) ){
        retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
      } else {
        retVal = parse_language(bitbuf, &textLabel->language[n]);
      }

      if( isError(retVal) ) break;
    } /* for n */
  }

  return retVal;
}

static enum EARCON_INFO_READER_RETURN parse_earcon(
  robitbufHandle   const bitbuf,
  struct EARCON  * const earcon
){
  enum EARCON_INFO_READER_RETURN retVal = EARCON_INFO_READER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! bitbuf || ! earcon ){
    retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET EARCON */
  if( ! isError(retVal) ){
    memset(earcon, 0, sizeof(*earcon));
  }

  /* READ EARCON */
  if( ! isError(retVal) ){
    earcon->isIndependent = robitbuf_ReadBits(bitbuf, 1);

    earcon->id = robitbuf_ReadBits(bitbuf, 7);

    earcon->type = robitbuf_ReadBits(bitbuf, 4);

    earcon->active = robitbuf_ReadBits(bitbuf, 1);

    earcon->positionType = robitbuf_ReadBits(bitbuf, 2);

    if( earcon->positionType == 0 ){
      earcon->CICPspeakerIdx = robitbuf_ReadBits(bitbuf, 7);
    } else {
      if( earcon->positionType == 1 ){
        earcon->azimuth = robitbuf_ReadBits(bitbuf, 8);
        earcon->elevation = robitbuf_ReadBits(bitbuf, 6);
        earcon->distance = robitbuf_ReadBits(bitbuf, 9);
      } else {
        earcon->azimuth = 0;
        earcon->elevation = 0;
        earcon->distance = 177; /* default reference distance */
      }

    }

    earcon->hasGain = robitbuf_ReadBits(bitbuf, 1);

    if( earcon->hasGain ){
      earcon->gain = robitbuf_ReadBits(bitbuf, 7);
    }

    earcon->hasTextLabel = robitbuf_ReadBits(bitbuf, 1);

    if( earcon->hasTextLabel ){
      retVal = parse_textLabel(bitbuf, &earcon->textLabel);
    }
  }

  return retVal;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/


/*! parses earconInfo() payload */
enum EARCON_INFO_READER_RETURN earconInfoReader_ParsePayload(
  unsigned char      const * const earconInfoPayload,
  size_t                     const numBytesEarconInfoPayload,
  struct EARCON_INFO       * const earconInfo
){
  enum EARCON_INFO_READER_RETURN retVal = EARCON_INFO_READER_RETURN_NO_ERROR;
  robitbuf bitbuf;

  /* SANITY CHECKS */
  if( (numBytesEarconInfoPayload > 0 && ! earconInfoPayload) || ! earconInfo ){
    retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET EARCON INFO */
  if( ! isError(retVal) ){
    memset(earconInfo, 0, sizeof(*earconInfo));
  }

  /* INIT BITBUFFER */
  if( ! isError(retVal) ){
    robitbuf_Init(&bitbuf, earconInfoPayload, 8 * numBytesEarconInfoPayload, 0);
  }

  /* PARSE EARCON INFO */
  if( ! isError(retVal) ){
    unsigned int i;
    earconInfo->numEarcons = robitbuf_ReadBits(&bitbuf, 7);
    
    for( i = 0; i < earconInfo->numEarcons + 1; i++ ){

      if( i >= ARRAY_LENGTH(earconInfo->earcons) ){

        retVal = EARCON_INFO_READER_RETURN_UNKNOWN_ERROR;
      } else {
        retVal = parse_earcon(&bitbuf, &earconInfo->earcons[i]);
      }

      if( isError(retVal) ) break;
    } /* for i */
  }

  return retVal;
}

