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

Copyright (c) ISO/IEC 1998.

*************************************************************************/

#ifndef _bitfct_h_
#define _bitfct_h_

#define UINT_BITLENGTH            32    /* Length in bits of an (unsigned) long variable */

typedef struct 
{
    unsigned char  *byte_ptr;           /* pointer to next byte             */
    unsigned char  byte;                /* next byte to write in buffer     */
    unsigned short valid_bits;          /* number of valid bits in byte     */
    unsigned long bit_count;            /* counts encoded bits              */
                                 
} BIT_BUF;    

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

void          WriteBitBuf( BIT_BUF*      data, 
                           unsigned long value, 
                           unsigned long no_of_bits );

void          InitWriteBitBuf( BIT_BUF*       bit_buf, 
                               unsigned char* buffer );

unsigned long FlushWriteBitBuf( BIT_BUF* bit_buf );

unsigned long GetBitCount( BIT_BUF* bit_buf );

void          InitReadBitBuf( BIT_BUF*      bit_buf, 
                              unsigned char* buffer );

unsigned long ReadBitBuf( BIT_BUF*      data, 
                          unsigned long no_of_bits );

void          TransferBitsBetweenBitBuf ( BIT_BUF *in, BIT_BUF *out, unsigned long noOfBits );
#endif /* _bitfct_h_ */
