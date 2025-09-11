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
#include "dynamicProdMetadataFrameWriter.h"
#include "printHelper.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
static int isError(enum PROD_METADATA_FRAME_DYNAMIC_RETURN retval) {
  return (retval != PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR);
}

/*! maps from PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN to PROD_METADATA_FRAME_DYNAMIC_RETURN */
static enum PROD_METADATA_FRAME_DYNAMIC_RETURN map_PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN(enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN val) {
  if (val != PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR ) {
    return PROD_METADATA_FRAME_DYNAMIC_RETURN_CANNOT_WRITE_SIGNLE_DYNAMIC;
  }
  return PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! writes dynamicProdMetadataFrame() payload */
enum PROD_METADATA_FRAME_DYNAMIC_RETURN dynamicProdMetadataFrame_WritePayload(
  wobitbuf                              * const bitbuf,
  unsigned int                            const num_objects,
  unsigned int                    const * const has_object_metadata,
  unsigned int                            const fixed_distance,
  struct PRODMETADATAFRAMEDYNAMIC const * const dynamicProdMetadataFrame
) {
  enum PROD_METADATA_FRAME_DYNAMIC_RETURN retVal = PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!dynamicProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_DYNAMIC_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PRODMETADATAFRAMEDYNAMICO */
  if (!isError(retVal)) {
    unsigned int obj;

    wobitbuf_WriteBits(bitbuf, dynamicProdMetadataFrame->flag_dist_absolute, 1);

    for (obj = 0; obj < num_objects; obj++) {
      if (has_object_metadata[obj]) {
        enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN prodMetadatasingleDynamicReturn = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR;
        prodMetadatasingleDynamicReturn = singleDynamicProdMetadataFrame_WritePayload(bitbuf,
                                                                                      dynamicProdMetadataFrame->flag_dist_absolute,
                                                                                      fixed_distance,
                                                                                      &dynamicProdMetadataFrame->singleDynamicProdMetadataFrame[obj]);
        retVal = map_PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN(prodMetadatasingleDynamicReturn);
        if (retVal != PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR) {
          return retVal;
        }
      }
    }
  }

  return retVal;
}

/*! prints out content of the dynamicProdMetadataFrame() struct */
enum PROD_METADATA_FRAME_DYNAMIC_RETURN dynamicProdMetadataFrame_Print(
  FILE                                  *       file,
  struct PRODMETADATAFRAMEDYNAMIC const * const dynamicProdMetadataFrame,
  unsigned int                            const num_objects,
  unsigned int                    const * const has_object_metadata,
  unsigned int                            const fixed_distance,
  unsigned int                            const numSpacesIndentation
) {
  enum PROD_METADATA_FRAME_DYNAMIC_RETURN retVal = PROD_METADATA_FRAME_DYNAMIC_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!dynamicProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_DYNAMIC_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PRODMETADATAFRAMEDYNAMIC */
  if (!isError(retVal)) {
    unsigned int obj;

    fprintf_indented(file, numSpacesIndentation, "flag_dist_absolute = %u\n", dynamicProdMetadataFrame->flag_dist_absolute);

    for (obj = 0; obj < num_objects; obj++) {
      if (has_object_metadata[obj]) {
        singleDynamicProdMetadataFrame_Print(file,
                                             dynamicProdMetadataFrame->flag_dist_absolute,
                                             fixed_distance,
                                             &dynamicProdMetadataFrame->singleDynamicProdMetadataFrame[obj],
                                             2);
      }
    }
  }

  return retVal;
}
