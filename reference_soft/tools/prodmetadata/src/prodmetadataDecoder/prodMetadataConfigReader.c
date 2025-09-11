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
#include "prodMetadataConfigReader.h"
#include "printHelper.h"
#include "readonlybitbuf.h"
#include "tables.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
static int isError(enum PROD_METADATA_CONFIG_RETURN retval) {
  return (retval != PROD_METADATA_CONFIG_RETURN_NO_ERROR);
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
/*! parses prodMetadataConfig() payload */
enum PROD_METADATA_CONFIG_RETURN prodMetadataConfig_ParsePayload(
  unsigned char       const * const prodMetadataConfigPayload,
  size_t                      const numBytesprodMetadataConfigPayload,
  unsigned int                const numObjectGroups,
  unsigned int                const numChannelGroups,
  struct PRODMETADATACONFIG * const prodMetadataConfig
) {
  enum PROD_METADATA_CONFIG_RETURN retVal = PROD_METADATA_CONFIG_RETURN_NO_ERROR;
  robitbuf bitbuf;

  /* SANITY CHECKS */
  if ((numBytesprodMetadataConfigPayload > 0 && !prodMetadataConfigPayload) || ! prodMetadataConfig) {
    retVal = PROD_METADATA_CONFIG_RETURN_UNKNOWN_ERROR;
  }

  /* RESET PRODMETADATACONFIG */
  if (!isError(retVal)) {
    memset(prodMetadataConfig, 0, sizeof(*prodMetadataConfig));
  }

  /* INIT BITBUFFER */
  if (!isError(retVal)) {
    robitbuf_Init(&bitbuf, prodMetadataConfigPayload, 8 * numBytesprodMetadataConfigPayload, 0);
  }

  /* PARSE PRODMETADATACONFIG */
  if (!isError(retVal)) {
    unsigned int i;

    prodMetadataConfig->has_reference_distance = robitbuf_ReadBits(&bitbuf, 1);

    if (prodMetadataConfig->has_reference_distance) {
      prodMetadataConfig->bs_reference_distance = robitbuf_ReadBits(&bitbuf, 7);
    } else {
      prodMetadataConfig->bs_reference_distance = 57;
    }

    /* high resolution object distance */
    for (i = 0; i < numObjectGroups; i++) {
      if (i >= ARRAY_LENGTH(prodMetadataConfig->has_object_distance)) {
        retVal = PROD_METADATA_CONFIG_RETURN_UNKNOWN_ERROR;
      } else {
        prodMetadataConfig->has_object_distance[i] = robitbuf_ReadBits(&bitbuf, 1);
      }

      if (isError(retVal)) {
        break;
      }
    }

    /* direct to headphone */
    for (i = 0; i < numChannelGroups; i++) {
      if (i >= ARRAY_LENGTH(prodMetadataConfig->directHeadphone)) {
        retVal = PROD_METADATA_CONFIG_RETURN_UNKNOWN_ERROR;
      } else {
        prodMetadataConfig->directHeadphone[i] = robitbuf_ReadBits(&bitbuf, 1);
      }

      if (isError(retVal)) {
        break;
      }
    }
  }

  if (!isError(retVal)) {
    size_t bytesRead = (bitbuf.bitsRead + 7) >> 3;

    if (bytesRead > numBytesprodMetadataConfigPayload) {
      retVal = PROD_METADATA_CONFIG_RETURN_TOO_MAY_BITS_READ;
    } else if (bytesRead < numBytesprodMetadataConfigPayload) {
      retVal = PROD_METADATA_CONFIG_RETURN_TOO_MAY_BITS_LEFT;
    }
  }

  return retVal;
}

/*! prints out content of the prodMetadataConfig struct */
enum PROD_METADATA_CONFIG_RETURN prodMetadataConfig_Print(
  FILE                            *       file,
  struct PRODMETADATACONFIG const * const prodMetadataConfig,
  unsigned int                      const numObjectGroups,
  unsigned int                      const numChannelGroups,
  unsigned int                      const numSpacesIndentation
) {
  enum PROD_METADATA_CONFIG_RETURN retVal = PROD_METADATA_CONFIG_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if (!prodMetadataConfig) {
    retVal = PROD_METADATA_CONFIG_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PRODMETADATACONFIG */
  if (!isError(retVal)) {
    unsigned int i;
    float reference_distance = bs_reference_distance_to_reference_distance_table[prodMetadataConfig->bs_reference_distance];

    fprintf_indented(file, numSpacesIndentation, "has_reference_distance = %u\n", prodMetadataConfig->has_reference_distance);
    fprintf_indented(file, numSpacesIndentation, "bs_reference_distance = %u\n", prodMetadataConfig->bs_reference_distance);
    fprintf_indented(file, numSpacesIndentation, "   reference_distance = %f m\n", reference_distance);

    for (i = 0; i < numObjectGroups; i++) {
      fprintf_indented(file, numSpacesIndentation, "has_object_distance = %u\n", prodMetadataConfig->has_object_distance[i]);
    }

    for (i = 0; i < numChannelGroups; i++) {
      fprintf_indented(file, numSpacesIndentation, "directHeadphone = %u\n", prodMetadataConfig->directHeadphone[i]);
    }
  }

  return retVal;
}
