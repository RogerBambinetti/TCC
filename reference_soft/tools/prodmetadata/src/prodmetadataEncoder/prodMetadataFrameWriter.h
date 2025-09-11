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
#ifndef PROD_METADATA_FRAME_H
#define PROD_METADATA_FRAME_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */
#include "intracodedProdMetadataFrameWriter.h"
#include "dynamicProdMetadataFrameWriter.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#ifndef MAX_AUDIO_GROUPS
#define MAX_AUDIO_GROUPS (32)
#endif
#ifndef MAX_NUM_OBJECTS
#define MAX_NUM_OBJECTS (28)
#endif

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/
enum PROD_METADATA_FRAME_RETURN {
  PROD_METADATA_FRAME_RETURN_NO_ERROR = 0,
  PROD_METADATA_FRAME_RETURN_TOO_MAY_BITS_LEFT,
  PROD_METADATA_FRAME_RETURN_TOO_MAY_BITS_READ,
  PROD_METADATA_FRAME_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct OBJECTGROUP {
  unsigned int num_objects;
  unsigned int has_object_metadata[MAX_NUM_OBJECTS];
};

struct OBJDATA {
  unsigned int numObjectGroups;
  struct OBJECTGROUP objectGroup[MAX_AUDIO_GROUPS];
  unsigned int has_object_distance[MAX_AUDIO_GROUPS];
};

struct PRODMETADATAFRAME {
  unsigned int has_intracoded_data[MAX_AUDIO_GROUPS];
  struct PRODMETADATAFRAMEINTRACODED intracodedProdMetadataFrame[MAX_AUDIO_GROUPS];
  struct PRODMETADATAFRAMEDYNAMIC dynamicProdMetadataFrame[MAX_AUDIO_GROUPS];
  struct OBJDATA objData;
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/
/*! writes prodMetadataFrame() payload */
enum PROD_METADATA_FRAME_RETURN prodMetadataFrame_WritePayload(
  struct PRODMETADATAFRAME  const * const prodMetadataFrame,
  unsigned char                   * const buffer,
  size_t                            const maxNumBytesToWrite,
  size_t                          * const numBytesWritten,
  unsigned int                      const numObjectGroups,
  unsigned int                      const numChannelGroups
);

/*! prints out content of the prodMetadataFrame struct */
enum PROD_METADATA_FRAME_RETURN prodMetadataFrame_Print(
  FILE                           *       file,
  struct PRODMETADATAFRAME const * const prodMetadataFrame,
  unsigned int                     const numObjectGroups,
  unsigned int                     const numSpacesIndentation
);

#endif /* ifndef PROD_METADATA_FRAME_H */
