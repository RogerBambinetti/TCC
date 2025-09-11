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

/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/
/* SYSTEM INCLUDES */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* INCLUDES OF THIS PROJECT */
#include "decPmfReader.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct PMF_READER {
  FILE * file;
  size_t numBytesInFile;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
/*! checks whether return value is an error */
static int isError(enum PMF_READER_RETURN retval) {
  return (retval != PMF_READER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! opens PMF reader */
PMF_READER_HANDLE pmfReader_Open(
  char const * const filename,
  unsigned int const frameNumber
) {
  PMF_READER_HANDLE pmfReader = NULL;
  char fullFileName[FILENAME_MAX] = {'\0'};

  /* ALLOCATE HANDLE */
  pmfReader = (PMF_READER_HANDLE)calloc( 1, sizeof(*pmfReader));
  if (!pmfReader) {
    return NULL;
  }

  /* GENERATE FILENAME */
  strncpy(fullFileName, filename, strlen(filename));
  sprintf(fullFileName, "%s_%d.dat", fullFileName, frameNumber);

  /* OPEN FILE */
  pmfReader->file = fopen(fullFileName, "rb");
  if (!pmfReader->file) {
    pmfReader_Close(&pmfReader);
    return NULL;
  }

  /* OBTAIN FILE SIZE */
  fseek(pmfReader->file , 0 , SEEK_END);
  pmfReader->numBytesInFile = ftell(pmfReader->file);
  rewind(pmfReader->file);

  return pmfReader;
}

/*! closes PMF reader */
void pmfReader_Close(
  PMF_READER_HANDLE * const pmfReaderHandle
) {
  if (pmfReaderHandle && *pmfReaderHandle) {
    if ((*pmfReaderHandle)->file) {
      fclose((*pmfReaderHandle)->file);
    }

    free(*pmfReaderHandle);
    *pmfReaderHandle = NULL;
  }
}

/*! read PMF packet from file */
enum PMF_READER_RETURN pmfReader_readFrame(
  PMF_READER_HANDLE   const pmfReader,          /**< PMF reader handle */
  struct PMF_PACKET * const pmfPacket           /**< PMF packet struct to read to */
) {
  enum PMF_READER_RETURN retVal = PMF_READER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if (!pmfReader || !pmfPacket || (pmfReader->numBytesInFile > ARRAY_LENGTH(pmfPacket->pmfPacketPayload))) {
    retVal = PMF_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET OUTPUT */
  if (!isError(retVal)) {
    memset(pmfPacket, 0, sizeof(*pmfPacket));
  }

  /* READ DATA BLOCK */
  if (!isError(retVal)) {
    pmfPacket->pmfPacketLength += fread(pmfPacket->pmfPacketPayload, sizeof(unsigned char), pmfReader->numBytesInFile, pmfReader->file);

    if (pmfPacket->pmfPacketLength != pmfReader->numBytesInFile) {
      retVal = PMF_READER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}
