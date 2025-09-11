/**************************************************************************************
  This software module was originally developed by  
 Deutsche Thomson OHG (DTO)
  in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. 
  In no event shall the above provision be qualified, deemed, or construed as granting 
 to use and any third party, either expressly, by implication or by way of estoppel,
 any license or any authorization or other right to license, sell, distribute, under 
 any patent or patent application and any intellectual property rights other than the 
 copyrights owned by Company which are embodied in such software module and expressly 
 licensed hereunder. 
 Those intending to use this software module in products are advised that its use 
 may implement third party intellectual property rights, and in particular existing 
 patents or patent application which licenses to use are not of Company or ISO/IEC 
 responsibility, which hereby fully disclaim any warranty and liability of infringement 
 of free enjoyment with respect to the software module and its use.
 The software modules is provided as is, without warranty of any kind. 
 DTO and ISO/IEC have no liability for use of this software module or modifications
 thereof.
 Copyright hereunder is not released licensed for products that do not conform to the 
 ISO/IEC 23008-3 standard.
 DTO retains full right to modify and use the code for its own  purpose, assign or 
 donate the code to a third party and to inhibit third parties from  using the code 
 for products that do not conform to MPEG-related ITU Recommendations  and/or 
 ISO/IEC International Standards.
 This copyright notice must be included in all copies or derivative works. 
 This copyright license shall be construed according to the laws of Germany. 
  Copyright (c) ISO/IEC 2015.
*****************************************************************************************/


/*
 $Rev: 157 $
 $Author: technicolor-kf $
 $Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
 $Id: cbitstreamwriter.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#include <iostream>
#include "cbitstreamwriter.h"
#include <string.h>

CBitStreamWriter::CBitStreamWriter()
{

}

CBitStreamWriter::~CBitStreamWriter()
{
  flushBufferToStream();
}

bool CBitStreamWriter::createBufferW (const unsigned int nBufferSz)
{
	m_pStartAdr = std::shared_ptr<std::vector<unsigned char> >(new std::vector<unsigned char>);
  m_pStartAdr->assign(nBufferSz, 0);
	m_bufferLen = nBufferSz;
	if (m_pStartAdr->size() == 0) {
		printf("\nERROR, could not allocate memory in crate Buffer(), bitStreamW.c");
		return false;
	}
	m_pRW = &m_pStartAdr->at(0);
	m_bitsFree = m_bufferLen << 3; /* length in bytes*8*/
	m_offsetRW = 7;                 /* bitPoiter: [7 6 5 4 3 2 1 0], 7 = MSB*/
	memset(m_pRW, 0, m_bufferLen);
	return true;
}

bool CBitStreamWriter::setStreamWriter(std::shared_ptr<CStreamWriter> streamWriter)
{
  m_streamWriter = streamWriter;
  return true;
}

unsigned int CBitStreamWriter::writeBit(const unsigned int nBit)
{
	if (m_bitsFree == 0) {
		flushToStream();
	}
	*m_pRW |= (unsigned char)(nBit & 1) << m_offsetRW; /* bit mask, shift to bit psoition and add (or) bit to buffer */
	m_bitsFree--;
	if ((--m_offsetRW) < 0) {
		m_offsetRW = 7;                      /* if (next)bit pointer eceeds byte boundary, reset and increment byte pointer */
		m_pRW++;
	}
  return 1;
}

unsigned int CBitStreamWriter::writeNbits(const unsigned int nBits, const unsigned int nNumBits)
{
	unsigned int numBits2 = nNumBits;
	if (m_bitsFree < (unsigned)nNumBits) {
			flushToStream();
	}
	while (numBits2) {
		*m_pRW |= (unsigned char)((nBits >> (--numBits2)) & 1) << m_offsetRW;
		m_bitsFree--;
		if ((--m_offsetRW) < 0) {
			m_offsetRW = 7;
			m_pRW++;
		}
	}
	return(nNumBits - numBits2);
}

unsigned int CBitStreamWriter::writeNbytes(const unsigned char *pBuffer, const unsigned int nNumBytes)
{
  /* byte align and flush current buffer to stream*/
  unsigned int nBitsFlushed = byteAlignBuffer();
  flushToStream(); 

  /* write input buffer directly to the stream */
  return (m_streamWriter->writeBytes(pBuffer, nNumBytes)<<3) + nBitsFlushed;
}

unsigned int CBitStreamWriter::byteAlignBuffer()
{
  unsigned int nBitDiscard = 0; 
  if(m_offsetRW < 7)
  {
    nBitDiscard = m_offsetRW+1;
    m_bitsFree -= nBitDiscard;
    m_offsetRW = 7;                      /* if (next)bit pointer exceeds byte boundary, reset and increment byte pointer */
	  m_pRW++;
  }
  return nBitDiscard;
}

unsigned int CBitStreamWriter::flushBufferToStream()
{
  if(m_pStartAdr.use_count())
  {
    unsigned int unbitsFlushed = byteAlignBuffer();
    flushToStream();
    return unbitsFlushed;
  }
  return 0;  
}

unsigned int CBitStreamWriter::flushToStream()
{
  if(!m_streamWriter)
    return 0;
	unsigned char saveByte = 0;
	unsigned int numBits = (m_bufferLen << 3) - m_bitsFree; /* number of bits in buffer */
	unsigned int numBytesInBuffer = numBits >> 3;
	numBits &= 7;                                               /* now numBits %=8 */

	if (numBits) { /* saveByte has first numBits bits of last byte in buffer*/
		saveByte = m_pStartAdr->at(numBytesInBuffer);
	}                        /* site_t */

	m_streamWriter->writeBytes(&m_pStartAdr->at(0), numBytesInBuffer); /* write buffer bytes to stream */
  m_pRW = &m_pStartAdr->at(0);                   /* set write pointer */
	memset(m_pRW, 0, m_bufferLen);                 /* clear buffer */
	*m_pRW = saveByte;                             /* overwrite left over bits */
	m_offsetRW = 7 - numBits;                      /* set bitPointer */
	m_bitsFree = (m_bufferLen << 3) - numBits;     /* set number of free bits in bitbuffer */
	
	return numBits;                      /* return number of bits left in internal buffer*/
}


void CBitStreamWriter::resetBuffer()
{
  if(m_pStartAdr.use_count())
	  m_pRW = &m_pStartAdr->at(0);
	m_bitsFree = m_bufferLen << 3;
	m_offsetRW = 7;
	memset(m_pRW, 0, m_bufferLen);
}

/*-------------------------------------------------------------------------------------- */

CStreamWriter::CStreamWriter()
{

}

CStreamWriter::~CStreamWriter()
{
  
}

bool CStreamWriter::openStream(const char* cFileName)
{
  try{
    m_stream.open(cFileName, std::ios_base::out | std::ios_base::binary);
  }catch(int e){
    std::cout << "Cannot open file " << cFileName << " error " << e << std::endl;
    return false;
  }
  return true;
}

bool CStreamWriter::closeStream()
{
  if(m_stream.is_open())
  {
    try{
      m_stream.close();
    } catch(int e) {
      std::cout << "Cannot close file error " << e << std::endl;
      return false;  
    }
  }
  return true;
}

unsigned int CStreamWriter::writeBytes(const unsigned char *pBuffer, const unsigned int nNumBytes)
{
  if(m_stream.is_open() && !m_stream.eof())
  {
    unsigned int nPosition = (int)m_stream.tellp();
    try{
      m_stream.write((const char*)pBuffer, nNumBytes);
    }catch(int e){
      std::cout << "Cannot write bytes from file error " << e << std::endl;
    }
    return ((unsigned int)m_stream.tellp() - nPosition);
  }
  return 0;  
}


unsigned int CStreamWriter::setStreamPosition (const unsigned int OffsetPosition, const std::ios_base::seekdir origin)
{
  m_stream.seekp(OffsetPosition, origin);
	return m_stream.bad();
}

unsigned int CStreamWriter::getStreamPosition()
{
	return (unsigned int)m_stream.tellp();
}     

