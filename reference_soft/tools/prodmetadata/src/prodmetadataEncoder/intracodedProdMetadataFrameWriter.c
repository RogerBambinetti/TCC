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
#include "intracodedProdMetadataFrameWriter.h"
#include "printHelper.h"
#include "tables.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
static int isError(enum PROD_METADATA_FRAME_INTRACODED_RETURN retval) {
  return (retval != PROD_METADATA_FRAME_INTRACODED_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! writes intracodedProdMetadataFrame() payload */
enum PROD_METADATA_FRAME_INTRACODED_RETURN intracodedProdMetadataFrame_WritePayload(
  wobitbuf                                 * const bitbuf,
  unsigned int                               const num_objects,
  struct PRODMETADATAFRAMEINTRACODED const * const intracodedProdMetadataFrame
) {
  enum PROD_METADATA_FRAME_INTRACODED_RETURN retVal = PROD_METADATA_FRAME_INTRACODED_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!intracodedProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_INTRACODED_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PRODMETADATAFRAMEINTRACODED */
  if (!isError(retVal)) {
    if (intracodedProdMetadataFrame->num_objects > 1) {
      wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->fixed_distance, 1);

      if (intracodedProdMetadataFrame->fixed_distance) {
        wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->default_distance, 9);
      } else {
        wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->common_distance, 1);

        if (intracodedProdMetadataFrame->common_distance) {
          wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->default_distance, 9);
        } else {
          unsigned int obj;

          for (obj = 0; obj < intracodedProdMetadataFrame->num_objects; obj++) {
            wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->position_distance[obj], 9);
          }
        }
      }
    } else {
      wobitbuf_WriteBits(bitbuf, intracodedProdMetadataFrame->position_distance[0], 9);
    }
  }

  return retVal;
}

/*! prints out content of the prodMetadataConfig struct */
enum PROD_METADATA_FRAME_INTRACODED_RETURN intracodedProdMetadataFrame_Print(
  FILE                                     *       file,
  struct PRODMETADATAFRAMEINTRACODED const * const intracodedProdMetadataFrame,
  unsigned int                               const numSpacesIndentation
) {
  enum PROD_METADATA_FRAME_INTRACODED_RETURN retVal = PROD_METADATA_FRAME_INTRACODED_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!intracodedProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_INTRACODED_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PRODMETADATAFRAMEINTRACODED */
  if (!isError(retVal)) {
    if (intracodedProdMetadataFrame->num_objects > 1) {
      fprintf_indented(file, numSpacesIndentation, "fixed_distance = %u\n", intracodedProdMetadataFrame->fixed_distance);

      if (intracodedProdMetadataFrame->fixed_distance) {
        float default_distance = position_distance_to_distance_table[intracodedProdMetadataFrame->default_distance];

        fprintf_indented(file, numSpacesIndentation, "bs_default_distance = %u\n", intracodedProdMetadataFrame->default_distance);
        fprintf_indented(file, numSpacesIndentation, "   default_distance = %f m\n", default_distance);
      } else {
        fprintf_indented(file, numSpacesIndentation, "common_distance = %u\n", intracodedProdMetadataFrame->common_distance);

        if (intracodedProdMetadataFrame->common_distance) {
          float default_distance = position_distance_to_distance_table[intracodedProdMetadataFrame->default_distance];

          fprintf_indented(file, numSpacesIndentation, "bs_default_distance = %u\n", intracodedProdMetadataFrame->default_distance);
          fprintf_indented(file, numSpacesIndentation, "   default_distance = %f m\n", default_distance);
        } else {
          unsigned int obj;

          for (obj = 0; obj < intracodedProdMetadataFrame->num_objects; obj++) {
            float position_distance = position_distance_to_distance_table[intracodedProdMetadataFrame->position_distance[obj]];

            fprintf_indented(file, numSpacesIndentation, "bs_position_distance = %u\n", intracodedProdMetadataFrame->position_distance[obj]);
            fprintf_indented(file, numSpacesIndentation, "   position_distance = %f m\n", position_distance);
          }
        }
      }
    } else {
      float position_distance = position_distance_to_distance_table[intracodedProdMetadataFrame->position_distance[0]];

      fprintf_indented(file, numSpacesIndentation, "bs_position_distance = %u\n", intracodedProdMetadataFrame->position_distance[0]);
      fprintf_indented(file, numSpacesIndentation, "   position_distance = %f m\n", position_distance);
    }
  }

  return retVal;
}
