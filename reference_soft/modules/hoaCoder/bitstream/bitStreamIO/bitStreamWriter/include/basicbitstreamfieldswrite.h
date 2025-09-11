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
 $Id: basicbitstreamfieldswrite.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#ifndef __BASICBITSTREAMFIELDSWRITE__
#define __BASICBITSTREAMFIELDSWRITE__

#include "basicbitstreamfields.h"
#include "ibitstreamfieldwrite.h"

/*
 * Definition of an integer field writer
 * T should be singed or unsigned  char, short, int or int64  (64 Bit only for byte wise writing)
 *
 * Writes the integer field byte-wise if CIntegerField::m_nSizeInBits is a multiple of eight
 * otherwise a bit wise writing of the lowest bits is performed where the most significant bit is first. 
 *  
 */
template <class T>
class CIntegerFieldWrite : public CIntegerField<T>, public IBitStreamFieldWrite 
{
  public:
    CIntegerFieldWrite(){};
    virtual ~CIntegerFieldWrite(){};
    int write();
    using  CIntegerField<T>::operator=;
};

/** 
Implementation of CIntegerFieldRead<T>::write() */
template <class T>
int CIntegerFieldWrite<T>::write()
{
  int bitsWritten = 0;
  /* check if numBits is multiple of eight */
  if(!this->m_bByteAlign)
  {
    /* write bits */
    unsigned int nTmp = this->m_value;
    bitsWritten = m_bitWriter->writeNbits(nTmp, this->m_nSizeInBits);
  }
  else
  {
    /* write as bytes */
    T nTmp = 0;
    /* swap byte positions */
    unsigned char *pOut = (unsigned char*)&nTmp;
    if(this->m_bMsbFirst)
    {
      for(int byte=0; byte < ((int)this->m_nSizeInBits>>3); byte++)
      {
        pOut[byte] = (unsigned char)((this->m_value>>((sizeof(T)-1-byte)<<3))&0xFF);
      }
    } 
    else
    {
      for(int byte=0; byte < ((int)this->m_nSizeInBits>>3); byte++)
      {
        pOut[(this->m_nSizeInBits>>3) - byte - 1] = (unsigned char)((this->m_value>>((sizeof(T)-1-byte)<<3))&0xFF);
      }
    }
    bitsWritten = m_bitWriter->writeNbytes(pOut, this->m_nSizeInBits>>3);
  }
  return bitsWritten;
}

/* -------------------------------------------------------------------------- */

/** 
  * Definition of a floating point field writer
  */
template <class T>
class CFloatingPointFieldWrite : public CFloatingPointField<T>, public IBitStreamFieldWrite
{
  public:
    CFloatingPointFieldWrite() {};
    virtual ~CFloatingPointFieldWrite(){};
    using  CFloatingPointField<T>::operator=;
    /** writes the value to the stream */
    int write()
    {
      if(this->m_bByteAlign)
      {
        return m_bitWriter->writeNbytes((unsigned char*)&this->m_value, sizeof(T));
      }
      else
      {
        unsigned char *pChar = (unsigned char*) &this->m_value;
        unsigned int nBitsWritten = 0;
        unsigned int n = 0;
        while(n<sizeof(T))
        {
          nBitsWritten += m_bitWriter->writeNbits(pChar[n], 8);
          n++;
        }
        return nBitsWritten;
      }
    };
};

/* type definition for header field types write*/
typedef CFloatingPointFieldWrite<float> FLOAT_FW;
typedef CFloatingPointFieldWrite<double> DOUBLE_FW;
typedef CIntegerFieldWrite<char> CHAR_FW;
typedef CIntegerFieldWrite<unsigned char> UCHAR_FW;
typedef CIntegerFieldWrite<short> SHORT_FW;
typedef CIntegerFieldWrite<unsigned short> USHORT_FW;
typedef CIntegerFieldWrite<int> INT_FW;
typedef CIntegerFieldWrite<unsigned int> UINT_FW;
typedef CIntegerFieldWrite<long long> INT64_FW;
typedef CIntegerFieldWrite<unsigned long long> UINT64_FW;

/** 
  * global helper function for writing into an STL container 
  */
template <class iterator, class FIELD>
unsigned int writeArray(iterator begin, iterator end, FIELD &field)
{
  unsigned int nNumBitsRead = 0;
  iterator it = begin;
  while(it != end)
  {
    field = *it;
    nNumBitsRead += field.write();
    it++;
  }
  return nNumBitsRead;
}

#endif // __BASICBITSTREAMFIELDSWRITE__
