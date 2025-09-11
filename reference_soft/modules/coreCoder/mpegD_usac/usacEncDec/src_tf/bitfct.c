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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "bitfct.h"
/****************************************************************************
 ***    
 ***    TransferBitsBetweenBitBuf()
 ***    
 ***    transfers 'NoOfBits' bits from the bitstream-buffer 'in' in the
 ***    bitstream-buffer 'out', MSB first
 ***    
 ***    R. Sperschneider     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

void TransferBitsBetweenBitBuf ( BIT_BUF *in, BIT_BUF *out, unsigned long noOfBits )
{
  while ( noOfBits )
    {
      unsigned long transferBits;
      unsigned long tempCodeword;
      transferBits = MIN ( noOfBits, UINT_BITLENGTH );
      tempCodeword = ReadBitBuf ( in, transferBits );
      WriteBitBuf ( out, tempCodeword, transferBits );
      noOfBits -= transferBits;
    }
}

/****************************************************************************
 ***    
 ***    WriteBitBuf()
 ***    
 ***    writes 'no_of_bits' bits from the variable 'value' in the
 ***    bitstream-buffer 'data', MSB first
 ***    
 ***    M.Dietz     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

void WriteBitBuf( BIT_BUF *data, unsigned long value, unsigned long no_of_bits )

{
    unsigned long mask;             /* bit mask                         */

    if( no_of_bits == 0 ) 
      return;

    mask = 0x1;
    mask <<= no_of_bits - 1;

    data->bit_count += no_of_bits;

    while( no_of_bits > 0 )
      {
        while( no_of_bits > 0 && data->valid_bits < 8 ) 
	  {
            data->byte <<= 1;
            if( value & mask ) data->byte |= 0x1;
            value <<= 1;
            no_of_bits--;
            data->valid_bits++;
	  }
        if( data->valid_bits == 8 ) 
	  {
            *data->byte_ptr++ = data->byte;
            data->byte = 0;
            data->valid_bits = 0;
          }
      }
}



/****************************************************************************
 ***    
 ***    InitWriteBitBuf()
 ***    
 ***    inititalizes a bitstream buffer
 ***    
 ***    M.Dietz     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

void InitWriteBitBuf( BIT_BUF *bit_buf, unsigned char *buffer )
{
    bit_buf->byte_ptr = buffer;
    bit_buf->byte = 0;
    bit_buf->valid_bits = 0;
    bit_buf->bit_count = 0;
}


/****************************************************************************
 ***    
 ***    FlushWriteBitBuf()
 ***    
 ***    flushes the last, possibly not stored byte of a bitstream written
 ***    with writeBitBuf
 ***    
 ***    M.Dietz     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

unsigned long FlushWriteBitBuf( BIT_BUF *bit_buf )
{
    unsigned long flushbits;
    flushbits = 8 - bit_buf->bit_count % 8;
    if( flushbits < 8 ) {
      WriteBitBuf( bit_buf, 0x0, flushbits );
      bit_buf->bit_count -= flushbits; 
    }

    return bit_buf->bit_count;
}

/****************************************************************************
 ***    
 ***    GetBitCount()
 ***    
 ***    returns the number of bits written/read
 ***    
 ***    M.Dietz     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

unsigned long GetBitCount( BIT_BUF *bit_buf )
{
    return bit_buf->bit_count;
}



/****************************************************************************
 ***
 ***    InitReadBitBuf()
 ***
 ***    inititalizes a bitstream buffer for reading
 ***
 ***    M.Dietz     FhG IIS/INEL
 ***
 ****************************************************************************/

void InitReadBitBuf( BIT_BUF *bit_buf, unsigned char *buffer )
{
    bit_buf->byte_ptr   =  buffer;
    bit_buf->byte       = *buffer;
    bit_buf->valid_bits = 8;
    bit_buf->bit_count  = 0;
}



/****************************************************************************
 ***
 ***    ReadBitBuf()
 ***
 ***    reads 'no_of_bits' bits from the bitstream buffer 'data'
 ***    in the variable 'value'
 ***
 ***    M.Dietz     FhG IIS/INEL                                          
 ***                                                                      
 ****************************************************************************/

unsigned long ReadBitBuf ( BIT_BUF*        data,       /* bitstream-buffer                     */
                           unsigned long   no_of_bits) /* number of bits to read               */
{
    unsigned long mask;             /* bit mask */
    unsigned long value;

    mask  = 0x80;
    value = 0;
    if ( no_of_bits == 0 ) 
      return ( value );

    data->bit_count += no_of_bits;

    while ( no_of_bits > 0 )
      {
        while ( no_of_bits > 0 && data->valid_bits > 0 ) 
	  {
	    value <<= 1;
            if ( data->byte & mask ) 
              value |= 0x1;
            data->byte <<= 1;
            no_of_bits--;
            data->valid_bits--;
	  }
        if ( data->valid_bits == 0 ) 
	  {
            data->byte_ptr ++;
            data->byte = *data->byte_ptr;
            data->valid_bits = 8;
	  }
    }
    return ( value );
}

