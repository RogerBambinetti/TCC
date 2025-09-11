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

Copyright (c) ISO/IEC 2014.

***********************************************************************************/
#include "localSetupInformationInterfaceLib_BitstreamIO.h"
#include <math.h>

void LS_DecInt_closeBitstreamWriter(H_LS_BITSTREAM_ENCODER h_bitstream)
{
  int no_restbits = 0;
  if (h_bitstream->bsFile != NULL)
  {
    if (h_bitstream)
    {
      no_restbits = h_bitstream->bitsInBuffer % 8*sizeof(unsigned char);
      if (no_restbits > 0)
      {
        h_bitstream->nextByte <<= (8*sizeof(unsigned char) - no_restbits);
        fwrite( &h_bitstream->nextByte,sizeof(unsigned char), 1, h_bitstream->bsFile);
      }

      fclose(h_bitstream->bsFile); h_bitstream->bsFile = NULL;
      free(h_bitstream); h_bitstream = NULL;
    }
  }  
}

int LS_DecInt_writeBits(H_LS_BITSTREAM_ENCODER h_bitstream, unsigned int data, int no_bits)
{
  int i = 0; 
  unsigned int next_bit;

  for (i=0; i < no_bits; i++)
  {
    next_bit = (data >> (no_bits-(i+1))) & 0x1;
    h_bitstream->nextByte <<= 1;
    h_bitstream->nextByte |= next_bit;

    h_bitstream->bitsInBuffer ++;

    if (!(h_bitstream->bitsInBuffer % 8*sizeof(unsigned char))) 
    {
      if (h_bitstream->bsFile != NULL)
      {
        fwrite(&h_bitstream->nextByte, sizeof(unsigned char), 1, h_bitstream->bsFile);
      }
      h_bitstream->nextByte = 0;
    }
  }
  return no_bits;
}

int LS_DecInt_initBitstreamWriter(H_LS_BITSTREAM_ENCODER *h_bitstream, char *file)
{
  H_LS_BITSTREAM_ENCODER temp = (H_LS_BITSTREAM_ENCODER)calloc(1, sizeof(struct _lsBitstreamEncoder));
  temp->bsFile = fopen(file, "wb");

  if (NULL == temp->bsFile && file[0] != '\0') {
    fprintf(stderr, "Could not open file %s.\n", file);
    return -1;
  }
  if (NULL == temp->bsFile) {
    return -1;
  }

  temp->bufferOffset = 0;
  temp->nextByte = 0;
  temp->bitsInBuffer = 0;

  *h_bitstream = temp;
  return 0;
}


void LS_DecInt_closeBitstreamReader(H_LS_BITSTREAM_DECODER h_bitstream)
{
  if (h_bitstream)
  {
    fclose(h_bitstream->bsFile);
    free(h_bitstream); h_bitstream = NULL;
  }
}

int LS_DecInt_initBitstreamReader(H_LS_BITSTREAM_DECODER *h_bitstream, char *file)
{
  H_LS_BITSTREAM_DECODER temp = (H_LS_BITSTREAM_DECODER)calloc(1, sizeof(struct _lsBitstreamDecoder));
  temp->bsFile = fopen(file, "rb");

  if (NULL == temp->bsFile && file[0] != '\0') {
    fprintf(stderr, "Could not open file %s.\n", file);
    return -1;
  }
  if (NULL == temp->bsFile) {
    return -1;
  }

  temp->bufferOffset = 0;
  temp->nextByte     = 0;
  temp->bitsInBuffer = 0;

  *h_bitstream = temp;
  return 0;
}

int LS_DecInt_readBits(H_LS_BITSTREAM_DECODER h_bitstream, unsigned int *data, int no_bits)
{
  int i, bit_pos, no_read;
  unsigned long next_bit;
  unsigned long out_data = 0;

  for( i=0; i<no_bits; i++ ) 
  {
    
    bit_pos = h_bitstream->bitsInBuffer % 8*sizeof(unsigned char);
    if( !bit_pos ) 
    {
      if( h_bitstream->bsFile != NULL ) 
      {
        no_read = fread( &h_bitstream->nextByte, sizeof(unsigned char), 1, h_bitstream->bsFile );
        if( no_read != 1 ) 
        {
          fprintf( stderr, "Error reading from file.\n" );
          return 0;
        }
      }
      else return 0;
    }

    bit_pos++;
    next_bit = (h_bitstream->nextByte >> (8*sizeof(unsigned char) - bit_pos)) & 0x1;

    out_data <<= 1;
    out_data  |= next_bit;

    h_bitstream->bitsInBuffer ++;

  }

  *data = (unsigned int)out_data;
  return no_bits;
}

int LS_DecInt_convertIfNegative(unsigned int value, int no_bits)
{
  unsigned int test_bit = 0;
  unsigned int temp;

  int mask = 0xffffffff;

  mask <<= no_bits;
  temp = value;

  test_bit = value >> (no_bits-1) & 0x1;
  if (test_bit == 1)
  { 
    temp |= mask;
  }
  return (int)temp;
}

int LS_DecInt_readEscapedValue(H_LS_BITSTREAM_DECODER h_bitstream, unsigned int *data, int nbits1, int nbits2, int nbits3)
{
  unsigned int value;
  unsigned int valueAdd1;
  unsigned int valueAdd2;

  int no_bitsRead = 0;

  no_bitsRead += LS_DecInt_readBits(h_bitstream,&value,nbits1);
  if (value == pow(2.0f,nbits1)-1)
  {
    no_bitsRead += LS_DecInt_readBits(h_bitstream,&valueAdd1,nbits2);
    value += valueAdd1;
    if (valueAdd1 == pow(2.0f,nbits2)-1)
    {
      no_bitsRead += LS_DecInt_readBits(h_bitstream,&valueAdd2,nbits3);
      value += valueAdd2;
    }
  }

  *data = value;
  return no_bitsRead;
}

int LS_DecInt_writeEscapedValue(H_LS_BITSTREAM_ENCODER h_bitstream, unsigned int data, int nbits1, int nbits2, int nbits3)
{
  unsigned int temp = data;
  unsigned int value;
  unsigned int valueAdd1;
  unsigned int valueAdd2;
  unsigned int rest, rest2;

  int no_bitsWritten = 0;

  if (temp >= pow(2.0f,nbits1)-1)
  {
    value = (unsigned int)pow(2.0f,nbits1)-1;
    no_bitsWritten += LS_DecInt_writeBits(h_bitstream,value,nbits1);
    rest = temp - value;
    if (rest >= (unsigned int)pow(2.0f,nbits2)-1)
    {
      valueAdd1 = (unsigned int)pow(2.0f,nbits2)-1;
      no_bitsWritten += LS_DecInt_writeBits(h_bitstream,valueAdd1,nbits2);
      rest2 = rest - valueAdd1;
      if (rest >= (unsigned int)pow(2.0f,nbits3)-1)
      {
        valueAdd2 = (unsigned int)pow(2.0f,nbits3)-1;
        no_bitsWritten += LS_DecInt_writeBits(h_bitstream,valueAdd2,nbits3);
      }
      else
      {
        valueAdd2 = rest2;
        no_bitsWritten += LS_DecInt_writeBits(h_bitstream,valueAdd2,nbits3);
      }
    }
    else
    {
      valueAdd1 = rest;
      no_bitsWritten += LS_DecInt_writeBits(h_bitstream,valueAdd1,nbits2);
    }
  }
  else
  {
    value = temp;
    no_bitsWritten += LS_DecInt_writeBits(h_bitstream,value,nbits1);
  }

  return no_bitsWritten;
}