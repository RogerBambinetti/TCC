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
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#include "ascparser_machine.h"
#include "ascparser_bitbuf.h"

static INT32 ascparserBitBuffer_Get (ascparserBitBufferPtr self, INT32 elemId, INT32 n) ;
static INT32 ascparserBitBuffer_GetValidBits (ascparserBitBufferPtr self) ;


void ascparserBitBuffer_Initialize (ascparserBitBufferPtr self)
{
  UINT32 i ;
  self->base.Get = (INT32 (*)(ascparserBitStreamPtr, INT32, INT32)) ascparserBitBuffer_Get ;
  self->base.GetValidBits = (INT32 (*)(ascparserBitStreamPtr)) ascparserBitBuffer_GetValidBits ;

  self->base.m_BitsInCache = 0;
 
  self->m_ValidBits  = 0 ;
  self->m_ReadOffset = 0 ;
  self->m_BitCnt     = 0 ;
  self->m_BitNdx     = 0 ;

  for (i = 0 ; i < ascparserBitBufferSize ; i++)
  {
    self->m_Buffer [i] = 0 ;
  }

}


static INT32 ascparserBitBuffer_Get (ascparserBitBufferPtr self, INT32 elemID, INT32 nBits)
{
  UINT16 tmp = 0;
  self->m_ValidBits -= nBits ;

  while(nBits--) {
    tmp = ( tmp << 1) | ((self->m_Buffer[self->m_BitCnt>>3]>>(7-(self->m_BitCnt++ & 7))) & 1);
  }

  return tmp ;
}

static INT32 ascparserBitBuffer_GetValidBits (ascparserBitBufferPtr self)
{
  return self->m_ValidBits;
}


void ascparserBitBuffer_Feed(ascparserBitBufferPtr self, const UINT8 pBuf[], const UINT32 cbSize, UINT32 *cbValid)
{
  UINT32 bTotal = 0 ;

  UINT32 bToRead   = (ascparserBitBufferBits - self->m_ValidBits) >> 3 ;
  UINT32 noOfBytes = (bToRead < *cbValid) ? bToRead : *cbValid ;

  pBuf = &pBuf[cbSize - *cbValid];

  while (noOfBytes > 0)
  {
    UINT32 i ;

    /* split read to buffer size */

    bToRead = ascparserBitBufferSize - self->m_ReadOffset ;
    bToRead = (bToRead < noOfBytes) ? bToRead : noOfBytes ;

    /* copy 'bToRead' bytes from 'ptr' to inputbuffer */
    for (i = 0 ; i < bToRead ; i++) {
      self->m_Buffer[self->m_ReadOffset + i] = pBuf[i];
    }

    /* add noOfBits to number of valid bits in buffer */
    self->m_ValidBits += bToRead * 8;
    bTotal            += bToRead;
    pBuf              += bToRead;

    self->m_ReadOffset  = (self->m_ReadOffset + bToRead) & (ascparserBitBufferSize - 1) ;
    noOfBytes          -= bToRead ;
  }

  *cbValid -= bTotal ;
}
