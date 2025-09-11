/*****************************************************************************
 *                                                                           *
SC 29 Software Copyright Licencing Disclaimer:

This software module was originally developed by
  AT&T

and edited by
  Olaf Kähler (Fraunhofer IIS-A)

in the course of development of the ISO/IEC 13818-7 and ISO/IEC 14496-3 
standards for reference purposes and its performance may not have been 
optimized. This software module is an implementation of one or more tools as 
specified by the ISO/IEC 13818-7 and ISO/IEC 14496-3 standards.
ISO/IEC gives users free license to this software module or modifications 
thereof for use in products claiming conformance to audiovisual and 
image-coding related ITU Recommendations and/or ISO/IEC International 
Standards. ISO/IEC gives users the same free license to this software module or 
modifications thereof for research purposes and further ISO/IEC standardisation.
Those intending to use this software module in products are advised that its 
use may infringe existing patents. ISO/IEC have no liability for use of this 
software module or modifications thereof. Copyright is not released for 
products that do not conform to audiovisual and image-coding related ITU 
Recommendations and/or ISO/IEC International Standards.
The original developer retains full right to modify and use the code for its 
own purpose, assign or donate the code to a third party and to inhibit third 
parties from using the code for products that do not conform to audiovisual and 
image-coding related ITU Recommendations and/or ISO/IEC International Standards.
This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 1999.
 *                                                                           *
 ****************************************************************************/

#ifndef _SAOC_BITINPUT_H_
#define _SAOC_BITINPUT_H_


#include "saoc_spatial_filereader.h"

typedef struct s_tagBitInput *HANDLE_S_BITINPUT;



long s_getbits(int n);
long s_GetBits(HANDLE_S_BITINPUT input, long n);
long s_GetBits2(HANDLE_S_BITINPUT input, long n, long* data);


int s_byte_align(void);
int s_byteAlign(HANDLE_S_BITINPUT input);

long s_getNumBits(HANDLE_S_BITINPUT input);

void s_set_log_level(int id, int add_bits_num, int add_bits);

HANDLE_S_BITINPUT	s_bitinput_open(HANDLE_BYTE_READER byteReader);

int s_bitinput_resetCounter(HANDLE_S_BITINPUT input);

void s_bitinput_close(HANDLE_S_BITINPUT input);

#endif
