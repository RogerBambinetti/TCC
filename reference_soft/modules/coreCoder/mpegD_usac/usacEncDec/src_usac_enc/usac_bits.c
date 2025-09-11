/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/

#include "../src_usac/acelp_plus.h"
#define MASK      0x0001
/*---------------------------------------------------------------------------*
 * function:  bin2int                                                        *
 *            ~~~~~~~                                                        *
 * Read "no_of_bits" bits from the array bitstream[] and convert to integer  *
 *--------------------------------------------------------------------------*/
int bin2int(           /* output: recovered integer value              */
  int   no_of_bits,    /* input : number of bits associated with value */
  short *bitstream     /* input : address where bits are read          */
)
{
   int   value, i;
   value = 0;
   for (i = 0; i < no_of_bits; i++)	
   {
     value <<= 1;
     value += (int)((*bitstream++) & MASK);
   }
   return(value);
}
/*---------------------------------------------------------------------------*
 * function:  int2bin                                                        *
 *            ~~~~~~~                                                        *
 * Convert integer to binary and write the bits to the array bitstream[].    *
 * Most significant bits (MSB) are output first                              *
 *--------------------------------------------------------------------------*/
void int2bin(
  int   value,         /* input : value to be converted to binary      */
  int   no_of_bits,    /* input : number of bits associated with value */
  short *bitstream     /* output: address where bits are written       */
)
{
   short *pt_bitstream;
   int   i;
   pt_bitstream = bitstream + no_of_bits;
   for (i = 0; i < no_of_bits; i++)
   {
     *--pt_bitstream = (short)(value & MASK);
     value >>= 1;
   }
}

int get_num_prm(int qn1, int qn2)
{
  return 2 + ((qn1 > 0) ? 9 : 0) + ((qn2 > 0) ? 9 : 0);
}
