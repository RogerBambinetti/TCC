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
 $Id: cbitstreamwriter.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#ifndef __BITSTREAMWRITER__
#define __BITSTREAMWRITER__

#include <fstream>
#include <memory>
#include "cbitbuffer.h"

/*
 * class for file writing 
 */
class CStreamWriter
{
  public:
    CStreamWriter();
    ~CStreamWriter();
    /** opens a new binary file for writing */
    bool openStream(const char* cFileName);
    /** closes the file */
    bool closeStream();
    /** wirtes nNumBytes for pBuffer to the file */
    unsigned int writeBytes(const unsigned char *pBuffer, const unsigned int nNumBytes);
    /* sets the file pointer to a position in the file */ 
    unsigned int setStreamPosition (const unsigned int OffsetPosition, const std::ios_base::seekdir origin);             
    /* returns the current position in the file */  
    unsigned int getStreamPosition();    

/*--------------------------------------------------------------------------------------*/
private:
  std::fstream m_stream; // file pointer

};

/*
 * Class for bit-wise writing a bit stream using CStreamWriter for file access 
 */ 
class CBitStreamWriter : protected CBitBuffer
{
  public:
    CBitStreamWriter();
    ~CBitStreamWriter();
    /** Creates a new internal buffer of size nBufferSz bytes */
    bool createBufferW (const unsigned int nBufferSz);
    /** sets the stream writer inteface for the file (stream) access */
    bool setStreamWriter(std::shared_ptr<CStreamWriter> streamWriter);
    /** writes the LSBit form nBit to the buffer 
      * bufer is automatically flushed to the stream if it is full 
      */
    unsigned int writeBit(const unsigned int nBit);
    /* wirtes nNumBits LSBit from nBits to the buffer
     * bufer is automatically flushed to the stream if it is full 
     */
    unsigned int writeNbits(const unsigned int nBits, const unsigned int nNumBits);
    /** writes nNumBytes to the buffer
      * the bit buffer is bytealigned before the byte-wise writing starts
      * the bits for byte-aligning the buffer are discarded
      * bufer is automatically flushed to the stream if it is full 
      */
    unsigned int writeNbytes(const unsigned char *pBuffer, const unsigned int nNumBytes);
    /** moves the bit-buffer forward to the next byte boundary 
      * zero bits are written to the stream
      */
    unsigned int byteAlignBuffer();
    /** resets the bit buffer */
    void resetBuffer();
    /** flushes the bit-buffer to the stream
      * the bit buffer is automatically byte aligned before flushing
      */
    unsigned int flushBufferToStream();

  private:
    /**
      * flushes the current bit-buffer to the stream without byte-aligning
      * saves the bits of the last byte into the new buffer
      * calls CStreamWriter::writeNBytes
      */
    unsigned int flushToStream();
    std::shared_ptr<CStreamWriter> m_streamWriter;  // ptr to the stream writer
};

#endif
