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
#include "omfReader.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct OMF_READER {
  FILE * file;
  size_t numBytesInFile;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
/*! checks whether return value is an error */
static int isError(enum OMF_READER_RETURN retval) {
  return (retval != OMF_READER_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! opens OMF reader */
OMF_READER_HANDLE omfReader_Open(
  char const * const filename
) {
  OMF_READER_HANDLE omfReader = NULL;

  /* ALLOCATE HANDLE */
  omfReader = (OMF_READER_HANDLE)calloc( 1, sizeof(*omfReader));
  if (!omfReader) {
    return NULL;
  }

  /* OPEN FILE */
  omfReader->file = fopen(filename, "rb");
  if (!omfReader->file) {
    omfReader_Close(&omfReader);
    return NULL;
  }

  /* OBTAIN FILE SIZE */
  fseek(omfReader->file , 0 , SEEK_END);
  omfReader->numBytesInFile = ftell(omfReader->file);
  rewind(omfReader->file);

  return omfReader;
}

/*! closes OMF reader */
void omfReader_Close(
  OMF_READER_HANDLE * const omfReaderHandle
) {
  if (omfReaderHandle && *omfReaderHandle) {
    if ((*omfReaderHandle)->file) {
      fclose((*omfReaderHandle)->file);
    }

    free(*omfReaderHandle);
    *omfReaderHandle = NULL;
  }
}

/*! read OMF packet from file */
enum OMF_READER_RETURN omfReader_readFrame(
  OMF_READER_HANDLE    const omfReader,         /**< OMF reader handle */
  struct OMF_PACKET  * const omfPacket,         /**< OMF packet struct to read to */
  unsigned int         const numObjectGroups,
  unsigned int const * const num_objects
) {
  enum OMF_READER_RETURN retVal = OMF_READER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if (!omfReader || !omfPacket) {
    retVal = OMF_READER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET OUTPUT */
  if (!isError(retVal)) {
    memset(omfPacket, 0, sizeof(*omfPacket));
  }

  /* READ DATA BLOCK */
  if (!isError(retVal)) {
    unsigned int grp;
    omfPacket->numObjectGroups = numObjectGroups;

    for (grp = 0; grp < omfPacket->numObjectGroups; grp++) {
      unsigned int obj;
      omfPacket->objectGroup[grp].num_objects = num_objects[grp];

      for (obj = 0; obj < omfPacket->objectGroup[grp].num_objects; obj++) {
        if (!feof(omfReader->file)) {
          fscanf(omfReader->file, "%d", &omfPacket->objectGroup[grp].has_object_metadata[obj]);

          if ((omfPacket->objectGroup[grp].has_object_metadata[obj] < 0) || (omfPacket->objectGroup[grp].has_object_metadata[obj] >> 1)) {
            retVal = OMF_READER_RETURN_UNKNOWN_ERROR;
            break;
          }
        } else {
          retVal = OMF_READER_RETURN_UNKNOWN_ERROR;
          break;
        }
      }
    }
  }

  return retVal;
}
