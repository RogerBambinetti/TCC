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
#include "encPmcWriter.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct PMC_WRITER {
  FILE * file;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
/*! checks whether return value is an error */
static int isError(enum PMC_WRITER_RETURN retval) {
  return (retval != PMC_WRITER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! opens PMC writer */
PMC_WRITER_HANDLE pmcWriter_Open(
  char const * const filename
) {
  PMC_WRITER_HANDLE pmcWriter = NULL;

  /* ALLOCATE HANDLE */
  pmcWriter = (PMC_WRITER_HANDLE)calloc( 1, sizeof(*pmcWriter));
  if (!pmcWriter) {
    return NULL;
  }

  /* OPEN FILE */
  pmcWriter->file = fopen(filename, "wb");
  if (!pmcWriter->file) {
    pmcWriter_Close(&pmcWriter);
    return NULL;
  }

  return pmcWriter;
}

/*! closes PMC writer */
void pmcWriter_Close(
  PMC_WRITER_HANDLE * const pmcWriterHandle
) {
  if (pmcWriterHandle && *pmcWriterHandle) {
    if ((*pmcWriterHandle)->file) {
      fclose((*pmcWriterHandle)->file);
    }

    free(*pmcWriterHandle);
    *pmcWriterHandle = NULL;
  }
}

/*! pack PMC data */
enum PMC_WRITER_RETURN pmcWriter_PackConfig(
  unsigned char const * const pmcPacketPayload,
  size_t                const pmcPacketLength,
  struct PMC_PACKET   * const pmcPacket         /**< PMC packet struct to write */
) {
  enum PMC_WRITER_RETURN retVal = PMC_WRITER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if (!pmcPacketPayload || !pmcPacket || pmcPacketLength > MAX_NUM_BYTES_SUPPORTED_PMC_PACKET) {
    retVal = PMC_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* PACK DATA */
  pmcPacket->pmcPacketLength = pmcPacketLength;
  memcpy(pmcPacket->pmcPacketPayload, pmcPacketPayload, pmcPacketLength);

  return retVal;
}

/*! write PMC packet from file */
enum PMC_WRITER_RETURN pmcWriter_writeConfig(
  PMC_WRITER_HANDLE         const pmcWriter,    /**< PMC writer handle */
  struct PMC_PACKET const * const pmcPacket     /**< PMC packet struct to write */
) {
  enum PMC_WRITER_RETURN retVal = PMC_WRITER_RETURN_NO_ERROR;
  size_t bytesWritten = 0;

  /* SANITY CHECK */
  if (!pmcWriter || !pmcPacket) {
    retVal = PMC_WRITER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE DATA BLOCK */
  if (!isError(retVal)) {
    bytesWritten += fwrite(pmcPacket->pmcPacketPayload, sizeof(unsigned char), pmcPacket->pmcPacketLength, pmcWriter->file);

    if (bytesWritten != pmcPacket->pmcPacketLength) {
      retVal = PMC_WRITER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}
