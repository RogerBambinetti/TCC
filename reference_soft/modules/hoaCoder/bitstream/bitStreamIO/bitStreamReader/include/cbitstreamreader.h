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
 $Id: cbitstreamreader.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/


#ifndef __BITSTREAMREADER__
#define __BITSTREAMREADER__

#include "cbitbuffer.h"
#include <fstream>

/*
 * class for file reading 
 */
class CStreamReader
{
  public:
    CStreamReader();
    ~CStreamReader();
    /** opens an binary file for reading */
    bool openStream(const char* cFileName);
    /** closes the file */
    bool closeStream();
    /* reads nNumBytes to pBuffer */
    unsigned int readBytes(unsigned char *pBuffer, unsigned int nNumBytes);
    /* sets the file pointer to a position in the file */
    unsigned int setStreamPosition (const unsigned int OffsetPosition, const std::ios_base::seekdir origin);               
    /* returns the current position in the file */
    unsigned getStreamPosition();        
    /* retruns true if end of file has been reached*/
    bool isEof();

/*--------------------------------------------------------------------------------------*/

private:
  std::fstream m_stream; // file pointer

};

/*
 * Class for bit-wise reading a bit stream using CStreamReader for file access 
 */ 
class CBitStreamReader : protected CBitBuffer
{
  public:
    CBitStreamReader();
    ~CBitStreamReader();
    /** Creates a new internal buffer of size nBufferSz bytes */
    bool createBufferR (const unsigned int nBufferSz);
    /** sets the stream reader inteface for the file (stream) access */
    bool setStreamReader(std::shared_ptr<CStreamReader> streamReader);
    /** reads one bit from the stream to the LSbit of nBit */
    unsigned int readBit(unsigned int &nBit);
    /** reads nNumBits LSBits from the stream (the max. number of bits is 31) to nBits*/
    unsigned int readNbits(unsigned int &nBit, const unsigned int nNumBits);
    /** reads nNumBytes from the stream 
      * the bit buffer is bytealigned before the byte-wise reading starts
      * the bits for byte-aligning the buffer are discarded
      */
    unsigned int readNbytes(unsigned char * pBuffer, const unsigned int nNumBytes); // starts with byte align
    /** moves the bit-buffer forward to the next byte boundary 
      * the discarded bits are returned by nBit
      */
    unsigned int byteAlignBuffer( unsigned int &nBit);
    /** resets the bit buffer */
    void resetBuffer();
    /* returns true if end of stream has been reached*/
    bool isEof();

  private:
    /** is called if the bit-buffer is empty
      * calls CStreamReader::readBytes
      */
    unsigned int loadBufferFromStream();

  private:
    std::shared_ptr<CStreamReader> m_streamReader; // ptr to CStreamReader set by setStreamReader

};

#endif
