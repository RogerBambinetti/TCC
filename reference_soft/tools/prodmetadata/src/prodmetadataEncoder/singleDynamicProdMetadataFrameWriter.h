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
#ifndef PROD_METADATA_SIGNLE_DYNAMIC_H
#define PROD_METADATA_SIGNLE_DYNAMIC_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */
#include "writeonlybitbuf.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#ifndef MAX_AUDIO_GROUPS
#define MAX_AUDIO_GROUPS (32)
#endif

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/
enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN {
  PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NO_ERROR = 0,
  PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_NBITS_MISMATCH,
  PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct PRODMETADATAFRAMESIGNLEDYNAMIC {
  unsigned int position_distance;
  unsigned int flag_distance;
  unsigned int nBitsDistance;
  unsigned int position_distance_difference;
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! writes singleDynamicProdMetadataFrame() payload */
enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN singleDynamicProdMetadataFrame_WritePayload(
  wobitbuf                                    * const bitbuf,
  unsigned int                                  const flag_dist_absolute,
  unsigned int                                  const fixed_distance,
  struct PRODMETADATAFRAMESIGNLEDYNAMIC const * const singleDynamicProdMetadataFrame
);

/*! prints out content of the singleDynamicProdMetadataFrame() struct */
enum PROD_METADATA_FRAME_SIGNLE_DYNAMIC_RETURN singleDynamicProdMetadataFrame_Print(
  FILE                                        *       file,
  unsigned int                                  const flag_dist_absolute,
  unsigned int                                  const fixed_distance,
  struct PRODMETADATAFRAMESIGNLEDYNAMIC const * const singleDynamicProdMetadataFrame,
  unsigned int                                  const numSpacesIndentation
);

#endif /* ifndef PROD_METADATA_SIGNLE_DYNAMIC_H */
