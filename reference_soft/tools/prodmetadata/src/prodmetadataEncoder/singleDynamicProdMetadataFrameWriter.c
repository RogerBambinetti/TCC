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
#include <math.h>

/* INCLUDES OF THIS PROJECT */
#include "singleDynamicProdMetadataFrameWriter.h"
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
static int isError(enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN retval) {
  return (retval != PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR);
}

static enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN evaluate_nBitsDistance(
  unsigned int const num_bits,
  unsigned int const position_distance_difference
) {
  enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR;
  unsigned int maxIdx = (unsigned int)floor(pow(2.0, (double)num_bits)) - 1;

  if (maxIdx < position_distance_difference) {
    fprintf(stderr, "\nERROR: num_bits underruns position_distance_difference in singleDynamicProdMetadataFrame()\n");
    fprintf(stderr, "\tnum_bits = %d\n\tsignaled position_distance_difference = %d\n\tallowed position_distance_difference  = %d\n", num_bits, position_distance_difference, maxIdx);
    retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NBITS_MISMATCH;
  }

  return retVal;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! writes singleDynamicProdMetadataFrame() payload */
enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN singleDynamicProdMetadataFrame_WritePayload(
  wobitbuf                                    * const bitbuf,
  unsigned int                                  const flag_dist_absolute,
  unsigned int                                  const fixed_distance,
  struct PRODMETADATAFRAMESIGNLEDYNAMIC const * const singleDynamicProdMetadataFrame
) {
  enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (! singleDynamicProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PRODMETADATAFRAMESIGNLEDYNAMIC */
  if (!isError(retVal)) {
    if (flag_dist_absolute) {
      /* fixed_distance given in the preceding intracodedProdMetadataFrame() */
      if (!fixed_distance) {
        wobitbuf_WriteBits(bitbuf, singleDynamicProdMetadataFrame->position_distance, 9);
      }
    } else {
      /* fixed_distance given in the preceding intracodedProdMetadataFrame() */
      if (!fixed_distance) {
        wobitbuf_WriteBits(bitbuf, singleDynamicProdMetadataFrame->flag_distance, 1);

        if (singleDynamicProdMetadataFrame->flag_distance) {
          unsigned int num_bits = singleDynamicProdMetadataFrame->nBitsDistance + 2;;

          retVal = evaluate_nBitsDistance(num_bits, singleDynamicProdMetadataFrame->position_distance_difference);

          wobitbuf_WriteBits(bitbuf, singleDynamicProdMetadataFrame->nBitsDistance, 3);
          wobitbuf_WriteBits(bitbuf, singleDynamicProdMetadataFrame->position_distance_difference, num_bits);
        }
      }
    }
  }

  return retVal;
}

/*! prints out content of the singleDynamicProdMetadataFrame() struct */
enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN singleDynamicProdMetadataFrame_Print(
  FILE                                        *       file,
  unsigned int                                  const flag_dist_absolute,
  unsigned int                                  const fixed_distance,
  struct PRODMETADATAFRAMESIGNLEDYNAMIC const * const singleDynamicProdMetadataFrame,
  unsigned int                                  const numSpacesIndentation
) {
  enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!singleDynamicProdMetadataFrame) {
    retVal = PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PRODMETADATAFRAMESIGNLEDYNAMIC */
  if (!isError(retVal)) {
    if (flag_dist_absolute) {
      if (!fixed_distance) {
        float position_distance = position_distance_to_distance_table[singleDynamicProdMetadataFrame->position_distance];

        fprintf_indented(file, numSpacesIndentation, "bs_position_distance = %u\n", singleDynamicProdMetadataFrame->position_distance);
        fprintf_indented(file, numSpacesIndentation, "   position_distance = %f m\n", position_distance);
      }
    } else {
      if (!fixed_distance) {
        fprintf_indented(file, numSpacesIndentation, "flag_distance = %u\n", singleDynamicProdMetadataFrame->flag_distance);

        if (singleDynamicProdMetadataFrame->flag_distance) {
          float position_distance_difference = position_distance_to_distance_table[singleDynamicProdMetadataFrame->position_distance_difference];

          fprintf_indented(file, numSpacesIndentation, "nBitsDistance = %u\n", singleDynamicProdMetadataFrame->nBitsDistance);
          fprintf_indented(file, numSpacesIndentation, "bs_position_distance_difference = %u\n", singleDynamicProdMetadataFrame->position_distance_difference);
          fprintf_indented(file, numSpacesIndentation, "   position_distance_difference = %f m\n", position_distance_difference);
        }
      }
    }
  }

  return retVal;
}
