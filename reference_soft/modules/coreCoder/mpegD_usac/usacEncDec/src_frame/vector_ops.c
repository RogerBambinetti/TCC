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

#include "vector_ops.h"

void vcopy( const float src[], float dest[], int inc_src, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src;
    dest += inc_dest;
    src  += inc_src;
  }
  if (i<vlen)
    *dest = *src;

}

void vcopy_db( const double src[], double dest[], int inc_src, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src;
    dest += inc_dest;
    src  += inc_src;
  }
  if (i<vlen)
    *dest = *src;

}

void vmult( const float src1[], const float src2[], float dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 * *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 * *src2;

}

void vmult_db( const double src1[], const double src2[], double dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 * *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 * *src2;

}


void vdiv( const float src1[], const float src2[], float dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 / *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 / *src2;

}


void vdiv_db( const double src1[], const double src2[], double dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 / *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 / *src2;

}


void vadd( const float src1[], const float src2[], float dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 + *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 + *src2;
}


void vadd_db( const double src1[], const double src2[], double dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 + *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 + *src2;
}


void vsub( const float src1[], const float src2[], float dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 - *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 - *src2;

}


void vsub_db( const double src1[], const double src2[], double dest[], int inc_src1, int inc_src2, int inc_dest, int vlen )
{
  int i;

  for( i=0; i<vlen-1; i++ ) {
    *dest = *src1 - *src2;
    dest += inc_dest;
    src1 += inc_src1;
    src2 += inc_src2;
  }
  if (i<vlen)
    *dest = *src1 - *src2;

}
