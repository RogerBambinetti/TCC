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

Copyright (c) ISO/IEC 2018.

*************************************************************************/
#ifndef PCM_DATA_CONFIG_WRITER_H
#define PCM_DATA_CONFIG_WRITER_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */
#include "pcmDataConfig.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

enum PCM_DATA_CONFIG_WRITER_RETURN {
  PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR = 0,
  PCM_DATA_CONFIG_WRITER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! writes pcmDataConfig() payload */
enum PCM_DATA_CONFIG_WRITER_RETURN pcmDataConfigWriter_WritePayload(
  struct PCM_DATA_CONFIG const * const pcmDataConfig,     /**< struct containing information to be written */
  unsigned char                * const buffer,            /**< buffer to write pcmDataConfig() payload to */
  size_t                         const maxNumBytesToWrite,/**< number of bytes to write */
  size_t                       * const numBytesWritten    /**< number of bytes written for pcmDataConfig() */
);

#endif /* ifndef PCM_DATA_CONFIG_WRITER_H */
