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

/* INCLUDES OF THIS PROJECT */
#include "prodMetadataFrameReader.h"
#include "printHelper.h"
#include "readonlybitbuf.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
static int isError(enum PROD_METADATA_FRAME_RETURN retval) {
  return (retval != PROD_METADATA_FRAME_RETURN_NO_ERROR);
}

static enum PROD_METADATA_FRAME_RETURN map_PROD_METADATA_FRAME_INTRACODED_RETURN(enum PROD_METADATA_FRAME_INTRACODED_RETURN val) {
  if (val != PROD_METADATA_FRAME_INTRACODED_RETURN_NO_ERROR ) {
    return PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR;
  }
  return PROD_METADATA_FRAME_RETURN_NO_ERROR;
}

static enum PROD_METADATA_FRAME_RETURN map_PROD_METADATA_FRAME_DYNAMIC_RETURN(enum PROD_METADATA_FRAME_DYNAMIC_RETURN val) {
  if (val != PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR ) {
    return PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR;
  }
  return PROD_METADATA_FRAME_RETURN_NO_ERROR;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! parses prodMetadataFrame() payload */
enum PROD_METADATA_FRAME_RETURN prodMetadataFrame_ParsePayload(
  unsigned char      const * const prodMetadataFramePayload,
  size_t                     const numBytesprodMetadataFramePayload,
  struct OBJDATA     const * const objData,
  struct PRODMETADATAFRAME * const prodMetadataFrame
) {
  enum PROD_METADATA_FRAME_RETURN retVal = PROD_METADATA_FRAME_RETURN_NO_ERROR;
  robitbuf bitbuf;

  /* SANITY CHECKS */
  if ((numBytesprodMetadataFramePayload > 0 && !prodMetadataFramePayload) || !prodMetadataFrame) {
    retVal = PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR;
  }

  /* INIT BITBUFFER */
  if (!isError(retVal)) {
    robitbuf_Init(&bitbuf, prodMetadataFramePayload, 8 * numBytesprodMetadataFramePayload, 0);
  }

  /* SET OBJECT DATA FROM EXTERNAL SOURCE */
  if (!isError(retVal)) {
    memcpy(&prodMetadataFrame->objData, objData, sizeof(struct OBJDATA));
  }

  /* PARSE PRODMETADATAFRAME INFO */
  if (!isError(retVal)) {
    unsigned int grp;

    for (grp = 0; grp < prodMetadataFrame->objData.numObjectGroups; grp++) {
      if (prodMetadataFrame->objData.has_object_distance[grp]) {
        if (grp >= ARRAY_LENGTH(prodMetadataFrame->has_intracoded_data)) {
          retVal = PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR;
        } else {
          prodMetadataFrame->has_intracoded_data[grp] = robitbuf_ReadBits(&bitbuf, 1);

          if (prodMetadataFrame->has_intracoded_data[grp]) {
            enum PROD_METADATA_FRAME_INTRACODED_RETURN intracodedReturn = PROD_METADATA_FRAME_INTRACODED_RETURN_NO_ERROR;
            intracodedReturn = intracodedProdMetadataFrame_ParsePayload(&bitbuf,
                                                                        prodMetadataFrame->objData.objectGroup[grp].num_objects,
                                                                        &prodMetadataFrame->intracodedProdMetadataFrame[grp]);
            retVal = map_PROD_METADATA_FRAME_INTRACODED_RETURN(intracodedReturn);
            if (retVal != PROD_METADATA_FRAME_RETURN_NO_ERROR) {
              return retVal;
            }

          } else {
            enum PROD_METADATA_FRAME_DYNAMIC_RETURN dynamicReturn = PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR;
            dynamicReturn = dynamicProdMetadataFrame_ParsePayload(&bitbuf,
                                                                  prodMetadataFrame->objData.objectGroup[grp].num_objects,
                                                                  prodMetadataFrame->objData.objectGroup[grp].has_object_metadata,
                                                                  prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance,
                                                                  &prodMetadataFrame->dynamicProdMetadataFrame[grp]);
            retVal = map_PROD_METADATA_FRAME_DYNAMIC_RETURN(dynamicReturn);
            if (retVal != PROD_METADATA_FRAME_RETURN_NO_ERROR) {
              return retVal;
            }
          }
        }
      }
    }
  }

  if (!isError(retVal)) {
    size_t bytesRead = (bitbuf.bitsRead + 7) >> 3;

    if (bytesRead > numBytesprodMetadataFramePayload) {
      retVal = PROD_METADATA_FRAME_RETURN_TOO_MAY_BITS_READ;
    } else if (bytesRead < numBytesprodMetadataFramePayload) {
      retVal = PROD_METADATA_FRAME_RETURN_TOO_MAY_BITS_LEFT;
    }
  }

  return retVal;
}

/*! prints out content of the prodMetadataFrame struct */
enum PROD_METADATA_FRAME_RETURN prodMetadataFrame_Print(
  FILE                           *       file,
  struct PRODMETADATAFRAME const * const prodMetadataFrame,
  unsigned int                     const numObjectGroups,
  unsigned int                     const numSpacesIndentation
) {
  enum PROD_METADATA_FRAME_RETURN retVal = PROD_METADATA_FRAME_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!prodMetadataFrame) {
    retVal = PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PRODMETADATAFRAME INFO */
  if (!isError(retVal)) {
    unsigned int grp;

    for (grp = 0; grp < numObjectGroups; grp++) {
      if (prodMetadataFrame->objData.has_object_distance[grp]) {
        fprintf_indented(file, numSpacesIndentation, "has_intracoded_data = %u\n", prodMetadataFrame->has_intracoded_data[grp]);

        if (prodMetadataFrame->has_intracoded_data[grp]) {
          intracodedProdMetadataFrame_Print(file,
                                            &prodMetadataFrame->intracodedProdMetadataFrame[grp],
                                            2);
        } else {
          dynamicProdMetadataFrame_Print(file,
                                         &prodMetadataFrame->dynamicProdMetadataFrame[grp],
                                         prodMetadataFrame->objData.objectGroup[grp].num_objects,
                                         prodMetadataFrame->objData.objectGroup[grp].has_object_metadata,
                                         prodMetadataFrame->intracodedProdMetadataFrame[grp].fixed_distance,
                                         2);
        }
      }
    }
  }

  return retVal;
}
