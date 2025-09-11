/*******************************************************************************
This software module was originally developed by

Agere Systems, Coding Technologies, Fraunhofer IIS, Philips

and edited by

-

in the course of development of ISO/IEC 23003 for reference purposes and its 
performance may not have been optimized. This software module is an 
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives 
You a royalty-free, worldwide, non-exclusive, copyright license to copy, 
distribute, and make derivative works of this software module or modifications 
thereof for use in implementations of ISO/IEC 23003 in products that satisfy 
conformance criteria (if any). Those intending to use this software module in 
products are advised that its use may infringe existing patents. ISO/IEC have no 
liability for use of this software module or modifications thereof. Copyright is 
not released for products that do not conform to audiovisual and image-coding 
related ITU Recommendations and/or ISO/IEC International Standards. 

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in 
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC 
23003:
Agere Systems, Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all 
rights necessary to include the originally developed software module or 
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a 
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
and make derivative works for use in implementations of ISO/IEC 23003 in 
products that satisfy conformance criteria (if any), and to the extent that such 
originally developed software module or portions of it are included in ISO/IEC 
23003. To the extent that Agere Systems, Coding Technologies, Fraunhofer IIS, 
Philips own(s) patent rights that would be required to make, use, or sell the 
originally developed software module or portions thereof included in ISO/IEC 
23003 in a conforming product, Agere Systems, Coding Technologies, Fraunhofer 
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with 
applicants throughout the world. ISO/IEC gives You a free license to this 
software module or modifications thereof for the sole purpose of developing 
ISO/IEC 23003. 

#endif 

Agere Systems, Coding Technologies, Fraunhofer IIS, Philips retain full right to 
modify and use the code for its (their) own purpose, assign or donate the code 
to a third party and to inhibit third parties from using the code for products 
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC 
International Standards. This copyright notice must be included in all copies or 
derivative works. 

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

#include <string.h>

#include "saoc_stream.h"

void InitStream( Stream* strm, char* file_name, STREAM_TYPE strm_type )
{
  if ( file_name != NULL ) {
    switch( strm_type ) {
    case STREAM_READ:
      strm->p_file = fopen( file_name, "rb" );
      break;
    case STREAM_WRITE:
      strm->p_file = fopen( file_name, "wb" );
      break;
    default:
      fprintf( stderr, "Unknown stream type.\n" );
      return;
      break;
    }

    if( strm->p_file == NULL ) {
      fprintf( stderr, "Could not open file %s.\n", file_name );
      return;
    }
  }
  else {
    strm->p_file       = NULL;
    strm->bufferOffset = 0;
    strm->markers      = 0;
  }

  strm->nextByte     = 0;
  strm->bitsInBuffer = 0;
  strm->streamType   = strm_type;
}


int readBits( Stream* strm, int nbits, unsigned long* data )
{
  int i, bit_pos, no_read;
  unsigned long next_bit;

  unsigned long out_data = 0;

  if( strm->streamType != STREAM_READ ) {
    fprintf( stderr, "Wrong stream type.\n" );
    return 0;
  }

  for( i=0; i<nbits; i++ ) {
    
    bit_pos = strm->bitsInBuffer % 8*sizeof(unsigned char);
    
    if( !bit_pos ) {
      if( strm->p_file != NULL ) {
        no_read = fread( &strm->nextByte, sizeof(unsigned char), 1, strm->p_file );
        if( no_read != 1 ) {
          fprintf( stderr, "Error reading from stream.\n" );
          return 0;
        }
      }
      else return 0;
    }

    bit_pos++;
    next_bit = (strm->nextByte >> (8*sizeof(unsigned char) - bit_pos)) & 0x1;

    out_data <<= 1;
    out_data  |= next_bit;

    strm->bitsInBuffer ++;

  }

  *data = out_data;

  return 1;
}
void writeBits( Stream* strm, unsigned int data, int nbits )
{
  int i;
  unsigned int next_bit;

  if( strm->streamType != STREAM_WRITE ) {
    fprintf( stderr, "Wrong stream type.\n" );
    return;
  }

  for( i=0; i<nbits; i++ ) {
    next_bit = (data >> (nbits-(i+1))) & 0x1;

    strm->nextByte <<= 1;
    strm->nextByte  |= next_bit;

    strm->bitsInBuffer ++;

    if( !(strm->bitsInBuffer % 8*sizeof(unsigned char)) ) {
      if( strm->p_file != NULL ) {
        fwrite( &strm->nextByte, sizeof(unsigned char), 1, strm->p_file );
      }
      else {
        if(strm->bufferOffset < STREAM_MAX_BUFFER_SIZE) {
          strm->buffer[strm->bufferOffset++] = strm->nextByte;
        }
      }
      strm->nextByte = 0;
    }
  }
}

void CloseStream( Stream* strm )
{
  int no_restbits = 0;

  if( strm->p_file != NULL ) {

    if( strm->streamType == STREAM_WRITE ) {
      no_restbits = strm->bitsInBuffer % 8*sizeof(unsigned char);
      if( no_restbits > 0 ) {
        strm->nextByte <<= (8*sizeof(unsigned char) - no_restbits);
        fwrite( &strm->nextByte, sizeof(unsigned char), 1, strm->p_file );
      }
    }

    fclose( strm->p_file );
    strm->p_file = NULL;

  }
}


int GetBitsInStream( Stream* strm )
{
  return strm->bitsInBuffer;
}


void ByteAlignWrite( Stream* strm )
{
  int bitsLastByte = 0;

  bitsLastByte = GetBitsInStream( strm ) % (8*sizeof(unsigned char));

  if( bitsLastByte > 0 ) {
    writeBits( strm, 0, 8*sizeof(unsigned char) - bitsLastByte );
  }
}


void ByteAlignRead( Stream* strm )
{
  unsigned long dummy = 0;
  int bitsLastByte = 0;

  bitsLastByte = GetBitsInStream( strm ) % (8*sizeof(unsigned char));

  if( bitsLastByte > 0 ) {
    readBits( strm, 8*sizeof(unsigned char) - bitsLastByte, &dummy );
  }
}

void AddSectionMarker( Stream* strm )
{
  if( (strm->p_file == NULL) && (strm->markers < STREAM_MAX_MARKERS) ) {
    strm->marker[strm->markers] = strm->bufferOffset;
    strm->markers++;
  }
}

void GetFirstSection( Stream* strm, unsigned char** data, int* size )
{
  if( strm->markers > 0 ) {
    *data = strm->buffer;
    *size = strm->marker[0];
  }
  else {
    *data = NULL;
    *size = 0;
  }
}

void DeleteFirstSection( Stream* strm )
{
  int i;

  if( strm->markers > 0 ) {
    memmove( strm->buffer, &strm->buffer[strm->marker[0]], sizeof(*strm->buffer) * (strm->bufferOffset - strm->marker[0]) );
    strm->bufferOffset -= strm->marker[0];

    for( i = 1; i < strm->markers; i++ ) {
      strm->marker[i] -= strm->marker[0];
    }

    memmove( strm->marker, &strm->marker[1], sizeof(*strm->marker) * (STREAM_MAX_MARKERS - 1) );
    strm->markers--;
  }
}
