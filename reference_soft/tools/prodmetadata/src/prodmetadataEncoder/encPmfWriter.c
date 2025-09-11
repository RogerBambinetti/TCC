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
#include "encPmfWriter.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct PMF_WRITER {
  FILE * file;
  size_t numBytesInFile;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
/*! checks whether return value is an error */
static int isError(enum PMF_WRITER_RETURN retval) {
  return (retval != PMF_WRITER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! opens PMF writer */
PMF_WRITER_HANDLE pmfWriter_Open(
  char const * const filename,
  unsigned int const frameNumber
) {
  PMF_WRITER_HANDLE pmfWriter = NULL;
  char fullFileName[FILENAME_MAX] = {'\0'};

  /* ALLOCATE HANDLE */
  pmfWriter = (PMF_WRITER_HANDLE)calloc( 1, sizeof(*pmfWriter));
  if (!pmfWriter) {
    return NULL;
  }

  /* GENERATE FILENAME */
  strncpy(fullFileName, filename, strlen(filename));
  sprintf(fullFileName, "%s_%d.dat", fullFileName, frameNumber);

  /* OPEN FILE */
  pmfWriter->file = fopen(fullFileName, "wb");
  if (!pmfWriter->file) {
    pmfWriter_Close(&pmfWriter);
    return NULL;
  }

  return pmfWriter;
}

/*! closes PMF writer */
void pmfWriter_Close(
  PMF_WRITER_HANDLE * const pmfWriterHandle
) {
  if (pmfWriterHandle && *pmfWriterHandle) {
    if ((*pmfWriterHandle)->file) {
      fclose((*pmfWriterHandle)->file);
    }

    free(*pmfWriterHandle);
    *pmfWriterHandle = NULL;
  }
}

/*! pack PMF data */
enum PMF_WRITER_RETURN pmfWriter_PackConfig(
  unsigned char const * const pmfPacketPayload,
  size_t                const pmfPacketLength,
  struct PMF_PACKET   * const pmfPacket         /**< PMF packet struct to write */
) {
  enum PMF_WRITER_RETURN retVal = PMF_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if (!pmfPacketPayload || !pmfPacket || pmfPacketLength > MAX_NUM_BYTES_SUPPORTED_PMF_PACKET) {
    retVal = PMF_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* PACK DATA */
  pmfPacket->pmfPacketLength = pmfPacketLength;
  memcpy(pmfPacket->pmfPacketPayload, pmfPacketPayload, pmfPacketLength);

  return retVal;
}

/*! write PMF packet to file */
enum PMF_WRITER_RETURN pmfWriter_writeFrame(
  PMF_WRITER_HANDLE         const pmfWriter,    /**< PMF writer handle */
  struct PMF_PACKET const * const pmfPacket     /**< PMF packet struct to write */
) {
  enum PMF_WRITER_RETURN retVal = PMF_WRITER_RETURN_NO_ERROR;
  size_t bytesWritten = 0;

  /* SANITY CHECK */
  if (!pmfWriter || !pmfPacket) {
    retVal = PMF_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE DATA BLOCK */
  if (!isError(retVal)) {
    bytesWritten += fwrite(pmfPacket->pmfPacketPayload, sizeof(unsigned char), pmfPacket->pmfPacketLength, pmfWriter->file);

    if (bytesWritten != pmfPacket->pmfPacketLength) {
      retVal = PMF_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}
