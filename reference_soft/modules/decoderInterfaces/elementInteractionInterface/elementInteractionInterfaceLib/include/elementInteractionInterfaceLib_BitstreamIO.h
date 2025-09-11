/***********************************************************************************

This software module was originally developed by

Fraunhofer IIS

in the course of development of the ISO/IEC 23008-3 for reference purposes and its
performance may not have been optimized. This software module is an implementation
of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives
you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works of this software module or modifications  thereof for use
in implementations or products claiming conformance to the ISO/IEC 23008-3 standard
and which satisfy any specified conformance criteria. Those intending to use this
software module in products are advised that its use may infringe existing patents.
ISO/IEC have no liability for use of this software module or modifications thereof.
Copyright is not released for products that do not conform to the ISO/IEC 23008-3
standard.

Fraunhofer IIS retains full right to modify and use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using
the code for products that do not conform to MPEG-related ITU Recommendations and/or
ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2015.

***********************************************************************************/

#ifndef DECODER_INTERFACE_BISTREAM_IO
#define DECODER_INTERFACE_BISTREAM_IO

#ifndef BS_MAX_BUFFER_SIZE
  #define BS_MAX_BUFFER_SIZE 3000
#endif

#include <stdio.h>
#include <stdlib.h>
#include "elementInteractionInterfaceLib_API.h"

struct _eiBitstreamEncoder
{
  FILE *bsFile;
  unsigned char nextByte;
  int bitsInBuffer;
  int bufferOffset;
};

struct _eiBitstreamDecoder
{
  FILE *bsFile;
  unsigned char nextByte;
  int bitsInBuffer;
  int bufferOffset;
};


int EI_DecInt_convertIfNegative(unsigned int value, int no_bits);
int EI_DecInt_writeEscapedValue(H_EI_BITSTREAM_ENCODER h_bitstream, unsigned int data, int nbits1, int nbits2, int nbits3);
int EI_DecInt_readEscapedValue(H_EI_BITSTREAM_DECODER h_bitstream, unsigned int *data, int nbits1, int nbits2, int nbits3);

int EI_DecInt_writeBits(H_EI_BITSTREAM_ENCODER h_bitstream, unsigned int data, int no_bits);
int EI_DecInt_readBits(H_EI_BITSTREAM_DECODER h_bitstream, unsigned int *data, int no_bits);

#endif