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
 $Id: cbitstreamreader.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#include "cbitstreamreader.h"
#include <ios>
#include <iostream>
#include <cstdio>
#include <cstring>

CBitStreamReader::CBitStreamReader() 
{
};

CBitStreamReader::~CBitStreamReader()
{

};

bool CBitStreamReader::createBufferR(const unsigned int nBufferSz)
{
	if (nBufferSz < 8) {
		printf("\n ERROR: buffer size (bufferSz) should be > 8");

		return false;
	}
  std::shared_ptr<std::vector<unsigned char> > pBuffer(new std::vector<unsigned char>);
  pBuffer->assign(nBufferSz, 0);
	setBuffer(pBuffer, nBufferSz);
  if (m_pStartAdr == NULL) {
		printf("\nERROR, could not allocate memory in crate Buffer(), bitStreamW.c");
		return false;
	}
	m_pRW = &m_pStartAdr->at(0);
	m_bitsFree = 0;                /* no data bits yet*/
	m_offsetRW = 7;                 /* bitPoiter: [7 6 5 4 3 2 1 0], 7 = MSB*/
	memset(&m_pStartAdr->at(0), 0, m_bufferLen);
	return true;
}


bool CBitStreamReader::setStreamReader(std::shared_ptr<CStreamReader> streamReader)
{
  m_streamReader = streamReader;
  return true;
}

unsigned int CBitStreamReader::readBit(unsigned int &nBit)
{
  nBit=0;
  if(!m_bitsFree)
  {
    loadBufferFromStream();
    if(!m_bitsFree)
      return 0;
  }
  nBit = 1 & (*m_pRW>>m_offsetRW);
  if((--m_offsetRW)<0)
  {
  m_offsetRW=7;
  m_pRW++; 
  }
  m_bitsFree--; 
  return 1;
}

unsigned int CBitStreamReader::readNbits(unsigned int &nBit, const unsigned int nNumBits)
{
  if(!m_streamReader)
    return false;
	unsigned int ii;
	nBit = 0;
	if (m_bitsFree < nNumBits) {
		loadBufferFromStream();
		if (m_bitsFree < nNumBits)
			return 0;
	}
	for (ii = 0; ii < nNumBits; ii++) {
		nBit <<= 1;           /* shift left to have space for next bit */
		nBit |= 1 & (*m_pRW >> m_offsetRW);
		if ((--m_offsetRW) < 0) {
			m_offsetRW = 7;
			m_pRW++;
		}
		m_bitsFree--;
	}
	return nNumBits;
}

unsigned int CBitStreamReader::readNbytes(unsigned char * pBuffer, const unsigned int nNumBytes)
{
  /* check for streamreader */
  if(!m_streamReader)
    return false; 
  /* reset input buffer */
  memset(pBuffer, 0, nNumBytes);

  /* byte align buffer */
  unsigned int tmp;
  unsigned int bitsDiscard = byteAlignBuffer(tmp);

  /* read bytes from internal buffer */
  unsigned int bitsRead = bitsDiscard;
  unsigned int bytesInBuffer = (m_bitsFree>>3);
  unsigned int readBytesFromBuffer = (bytesInBuffer > nNumBytes) ? (nNumBytes) : (bytesInBuffer);
  unsigned int idx = 0;
  for(idx = 0; idx < readBytesFromBuffer; ++idx, m_pRW++)
  {
    pBuffer[idx] = *m_pRW;
    m_bitsFree -= 8;
  }
  bitsRead += readBytesFromBuffer<<3;

  /* read bytes directly from file */ 
  unsigned int readBytesDirect = 0;
  if(readBytesFromBuffer < nNumBytes)
  {
    readBytesDirect = nNumBytes - readBytesFromBuffer;
    bitsRead +=m_streamReader->readBytes(&pBuffer[idx], readBytesDirect)<<3;
  }
  return bitsRead;
}

unsigned int CBitStreamReader::byteAlignBuffer( unsigned int &nBit)
{
  nBit = 0;
	unsigned int numBits2Discard = 1 + m_offsetRW;
	if (numBits2Discard < 8) {
		return readNbits(nBit, numBits2Discard);
	} else return 0;
}

void CBitStreamReader::resetBuffer()
{
  if(m_pStartAdr.use_count())
    m_pRW = &m_pStartAdr->at(0);
	m_bitsFree = 0;                 /* no data bits yet*/
	m_offsetRW = 7;                  /* bitPoiter: [7 6 5 4 3 2 1 0], 7 = MSB*/
	memset(&m_pStartAdr->at(0), 0, m_bufferLen);
}


unsigned int  CBitStreamReader::loadBufferFromStream()
{
	unsigned int ii, numFreeBytes;
	/* copy data in buffer to beginning */
	unsigned int numBytesInBuffer = m_bitsFree >> 3;
	if (m_offsetRW != 7) numBytesInBuffer++;
	numFreeBytes = m_bufferLen - numBytesInBuffer;
	if (numBytesInBuffer > numFreeBytes)
		return 0;                          /* call again later */
  unsigned char *pByte = &m_pStartAdr->at(0);
	for (ii = 0; ii < numBytesInBuffer; ii++)  /* copy unread data to beginning     */
		pByte[ii] = *m_pRW++;
	ii = m_streamReader->readBytes(&pByte[ii], numFreeBytes); /* read new data from stream */
	m_pRW = &m_pStartAdr->at(0);   /* update book keeping */
//  m_offsetRW; /* does not need to be changed */
	m_bitsFree += (ii << 3);
	return ii;
}

bool CBitStreamReader::isEof()
{
  // if the internal buffer is empty and we are not at the end of the file, 
  // try to refill the internal buffer. This is required to 
  // get the end of the file if the internal buffer has been empty and the
  // last reading call was readNbyte(). In this case the bytes were read directly from the file
  // without refilling the internal buffer and so the stream could not recognize the end of the
  // file. The same problem should hold for sizes files that are a multiple of the internal buffer size. 
  // Thus refilling the internal buffer will then indicate the end of the file by trying to read 
  // more bytes than are actually in the file. 
  if ( !m_streamReader->isEof() & (m_bitsFree == 0) ) {
	  loadBufferFromStream();
	}
  return m_streamReader->isEof() & (m_bitsFree == 0);
}

/*--------------------------------------------------------------------------------------*/

CStreamReader::CStreamReader() {};

CStreamReader::~CStreamReader()
{

}

bool CStreamReader::openStream(const char* cFileName)
{
  try{
    if(cFileName)
      m_stream.open(cFileName, std::ios_base::in | std::ios_base::binary);
    else
      return false;
  }catch(int e){
    std::cout << "Cannot open file " << cFileName << " error " << e << std::endl;
    return false;
  }
  return true;
}

bool CStreamReader::closeStream()
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

unsigned int  CStreamReader::readBytes(unsigned char *pBuffer, unsigned int nNumBytes)
{
  if(m_stream.is_open() && !m_stream.eof())
  {
    try{
      m_stream.read((char*)pBuffer, nNumBytes);
    }catch(int e){
      std::cout << "Cannot read bytes from file error " << e << std::endl;
    }
    return (static_cast<unsigned int>(m_stream.gcount()));
  }
  return 0;
}

/*-------------------------------------------------------------------------------------- */

unsigned int CStreamReader::setStreamPosition(const unsigned int OffsetPosition, const std::ios_base::seekdir origin)
{
  m_stream.seekp(OffsetPosition, origin);
	return m_stream.bad();
}


unsigned int CStreamReader::getStreamPosition()
{  
	return (unsigned int)m_stream.tellp();
}


bool CStreamReader::isEof()
{
  return m_stream.eof();
}
