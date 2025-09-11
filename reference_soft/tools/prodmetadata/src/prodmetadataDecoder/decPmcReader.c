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
#include "decPmcReader.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct PMC_READER {
  FILE * file;
  size_t numBytesInFile;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
/*! checks whether return value is an error */
static int isError(enum PMC_READER_RETURN retval) {
  return (retval != PMC_READER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! opens PMC reader */
PMC_READER_HANDLE pmcReader_Open(
  char const * const filename
) {
  PMC_READER_HANDLE pmcReader = NULL;

  /* ALLOCATE HANDLE */
  pmcReader = (PMC_READER_HANDLE)calloc( 1, sizeof(*pmcReader));
  if (!pmcReader) {
    return NULL;
  }

  /* OPEN FILE */
  pmcReader->file = fopen(filename, "rb");
  if (!pmcReader->file) {
    pmcReader_Close(&pmcReader);
    return NULL;
  }

  /* OBTAIN FILE SIZE */
  fseek(pmcReader->file , 0 , SEEK_END);
  pmcReader->numBytesInFile = ftell(pmcReader->file);
  rewind(pmcReader->file);

  return pmcReader;
}

/*! closes PMC reader */
void pmcReader_Close(
  PMC_READER_HANDLE * const pmcReaderHandle
) {
  if (pmcReaderHandle && *pmcReaderHandle) {
    if ((*pmcReaderHandle)->file) {
      fclose((*pmcReaderHandle)->file);
    }

    free(*pmcReaderHandle);
    *pmcReaderHandle = NULL;
  }
}

/*! read PMC packet from file */
enum PMC_READER_RETURN pmcReader_readConfig(
  PMC_READER_HANDLE   const pmcReader,          /**< PMC reader handle */
  struct PMC_PACKET * const pmcPacket           /**< PMC packet struct to read to */
) {
  enum PMC_READER_RETURN retVal = PMC_READER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if (!pmcReader || !pmcPacket || (pmcReader->numBytesInFile > ARRAY_LENGTH(pmcPacket->pmcPacketPayload))) {
    retVal = PMC_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET OUTPUT */
  if (!isError(retVal)) {
    memset(pmcPacket, 0, sizeof(*pmcPacket));
  }

  /* READ DATA BLOCK */
  if (!isError(retVal)) {
    pmcPacket->pmcPacketLength += fread(pmcPacket->pmcPacketPayload, sizeof(unsigned char), pmcReader->numBytesInFile, pmcReader->file);

    if ((pmcPacket->pmcPacketLength) != pmcReader->numBytesInFile) {
      retVal = PMC_READER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}
