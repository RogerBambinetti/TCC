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
#include <math.h>

/* INCLUDES OF THIS PROJECT */
#include "encPmfReader.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y)) 
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

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

/*! encode position_distance and default_distance */
static unsigned int encodePositionDistance(
  float const distance
) {
  unsigned int bs_distance = 0;
  float f_distance;

  if (distance >= 0.0f) {
    f_distance = (float)log10(distance * 100.0f) / 0.014214299201363f + 1.0f; /* 0.014214299201363 = 0.0472188798661443*log10(2); */
    bs_distance = (unsigned int)(f_distance + 0.5);
  }

  return bs_distance;
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
  sprintf(fullFileName, "%s_%d.txt", fullFileName, frameNumber);

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
  PMF_READER_HANDLE                 const pmfReader,
  struct PRODMETADATAFRAME        * const prodMetadataFrame,
  struct PRODMETADATACONFIG const * const prodMetadataConfig,
  struct OBJDATA                    const * const objData
) {
  enum PMF_READER_RETURN retVal = PMF_READER_RETURN_NO_ERROR;
  int elementsRead = 0;
  int elementsToRead = 0;

  /* SANITY CHECKS */
  if (!prodMetadataFrame || !prodMetadataConfig || !objData || !pmfReader) {
    retVal = PMF_READER_RETURN_UNKNOWN_ERROR;
  }

  /* SET OBJECT DATA FROM EXTERNAL SOURCE */
  if (!isError(retVal)) {
    memcpy(&prodMetadataFrame->objData, objData, sizeof(struct OBJDATA));
  }

  /* READ DATA FROM FILE */
  if (!isError(retVal)) {
    unsigned int grp;
    unsigned int bs_val;
    FILE* fptr = pmfReader->file;

    /* read prodMetadataFrame() */
    for (grp = 0; grp < objData->numObjectGroups; grp++) {
      if (prodMetadataConfig->has_object_distance[grp]) {
        if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
        prodMetadataFrame->has_intracoded_data[grp] = MIN(bs_val, 1);
        elementsToRead++;

        if (prodMetadataFrame->has_intracoded_data[grp]) {
          /* read intracodedProdMetadataFrame() */
          prodMetadataFrame->intracodedProdMetadataFrame[grp].num_objects = objData->objectGroup[grp].num_objects;

          if (objData->objectGroup[grp].num_objects > 1) {
            if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
            prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance = MIN(bs_val, 1);
            elementsToRead++;

            if (prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance) {
              float default_distance = 0.0f;

              if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &default_distance);
              elementsToRead++;

              prodMetadataFrame->intracodedProdMetadataFrame[grp].default_distance = encodePositionDistance(default_distance);
            } else {
              if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
              prodMetadataFrame->intracodedProdMetadataFrame[grp].common_distance = MIN(bs_val, 1);
              elementsToRead++;

              if (prodMetadataFrame->intracodedProdMetadataFrame[grp].common_distance) {
                float default_distance = 0.0f;

                if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &default_distance);
                elementsToRead++;

                prodMetadataFrame->intracodedProdMetadataFrame[grp].default_distance = encodePositionDistance(default_distance);
              } else {
                unsigned int obj;

                for (obj = 0; obj < objData->objectGroup[grp].num_objects; obj++) {
                  float position_distance = 0.0f;

                  if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &position_distance);
                  elementsToRead++;

                  prodMetadataFrame->intracodedProdMetadataFrame[grp].position_distance[obj] = encodePositionDistance(position_distance);
                }
              }
            }
          } else {
            float position_distance = 0.0f;

            if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &position_distance);
            elementsToRead++;

            prodMetadataFrame->intracodedProdMetadataFrame[grp].position_distance[0] = encodePositionDistance(position_distance);
          }
        } else {
          /* read Syntax of dynamicProdMetadataFrame() */
          unsigned int obj;

          if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
          prodMetadataFrame->dynamicProdMetadataFrame[grp].flag_dist_absolute = MIN(bs_val, 1);
          elementsToRead++;

          for (obj = 0; obj < objData->objectGroup[grp].num_objects; obj++) {
            if (objData->objectGroup[grp].has_object_metadata[obj]) {
              /* read singleDynamicProdMetadataFrame() */

              if (prodMetadataFrame->dynamicProdMetadataFrame[grp].flag_dist_absolute) {
                if (!prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance) {
                  float position_distance = 0.0f;

                  if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &position_distance);
                  elementsToRead++;

                  prodMetadataFrame->dynamicProdMetadataFrame[grp].singleDynamicProdMetadataFrame[obj].position_distance = encodePositionDistance(position_distance);
                }
              } else {
                if (!prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance) {
                  if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
                  prodMetadataFrame->dynamicProdMetadataFrame[grp].singleDynamicProdMetadataFrame[obj].flag_distance = MIN(bs_val, 1);
                  elementsToRead++;

                  if (prodMetadataFrame->dynamicProdMetadataFrame[grp].singleDynamicProdMetadataFrame[obj].flag_distance) {
                    float position_distance_difference = 0.0f;

                    if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
                    prodMetadataFrame->dynamicProdMetadataFrame[grp].singleDynamicProdMetadataFrame[obj].nBitsDistance = MIN(bs_val, 7);
                    elementsToRead++;

                    if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &position_distance_difference);
                    elementsToRead++;

                    prodMetadataFrame->dynamicProdMetadataFrame[grp].singleDynamicProdMetadataFrame[obj].position_distance_difference = encodePositionDistance(position_distance_difference);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (!isError(retVal)) {
    if ((!feof(pmfReader->file) && (0 < pmfReader->numBytesInFile)) || (elementsToRead != elementsRead)) {
      retVal = PMF_READER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}
