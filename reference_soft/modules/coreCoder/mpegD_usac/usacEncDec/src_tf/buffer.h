/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
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

Copyright (c) ISO/IEC 1996.

*************************************************************************/

#ifndef _buffer_h_
#define _buffer_h_

enum {
  VIRTUELL               = 0,
  MAX_CW_LENGTH          = 49,
  AAC_MAX_INPUT_BUF_BITS = 12288
};

/* code table for aac bitstream elements */
#define E(BITSTREAM_ELEMENT) BITSTREAM_ELEMENT
typedef enum {
#include "er_main_elements.h"
  MAX_ELEMENTS,
#include "elements.h"
  MAX_EXTRA_ELEMENTS ,
#include "../src_usac/usac_elements.h"
  MAX_USAC_ELEMENTS
} CODE_TABLE;
#undef E

#endif /*_buffer_h_*/
