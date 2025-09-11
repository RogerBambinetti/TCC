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
#ifndef MHAS_WRITER_H
#define MHAS_WRITER_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */
#include "mhas.h"

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for MHAS writer */
enum MHAS_WRITER_RETURN {
  MHAS_WRITER_RETURN_NO_ERROR = 0,
  MHAS_WRITER_RETURN_COULD_NOT_WRITE_TO_FILE,
  MHAS_WRITER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! write MHAS packet to file */
enum MHAS_WRITER_RETURN writePacket(
  FILE                * const mhasFile,          /**< file pointer of opened (in "wb" mode) MHAS file to write to */
  enum PACTYP           const mhasPacketType,    /**< MHAS packet type to write */
  unsigned int          const mhasPacketLabel,   /**< packet label of the MHAS packet to write */
  size_t                const mhasPacketLength,  /**< number of bytes in the MHAS packet payload */
  unsigned char const * const mhasPacketPayload  /**< pointer to a buffer containing the MHAS packet payload to be written */
);

#endif /* ifndef MHAS_WRITER_H */
