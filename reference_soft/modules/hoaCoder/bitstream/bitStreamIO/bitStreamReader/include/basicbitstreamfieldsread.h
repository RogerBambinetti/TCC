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
 $Id: basicbitstreamfieldsread.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#ifndef __BASICBITSTREAMFIELDSREAD__
#define __BASICBITSTREAMFIELDSREAD__

#include "basicbitstreamfields.h"
#include "ibitstreamfieldread.h"


/*
 * Definition of an integer field reader
 * T should be singed or unsigned  char, short, int or int64 (64 Bit only for byte wise reading)
 *
 * Reads the integer field byte-wise if CIntegerField::m_nSizeInBits is a multiple of eight
 * otherwise a bit wise reading of the lowest bits is performed where the most significant bit is first. 
 *  
 */
template <class T>
class CIntegerFieldRead : public CIntegerField<T>, public IBitStreamFieldRead
{
  public:
    CIntegerFieldRead(){};
    virtual ~CIntegerFieldRead(){};
    int read();

};


/** 
Implementation of CIntegerFieldRead<T>::read() */
template <class T>
int CIntegerFieldRead<T>::read()
{
  int bitsRead = 0;
  /* check if numBits is multiple of eight */
  if(!this->m_bByteAlign)
  {
    /* write bits */
    unsigned int nTmp = 0;
    bitsRead = m_bitReader->readNbits(nTmp, this->m_nSizeInBits);
    this->m_value = static_cast<T>(nTmp<<((sizeof(T)<<3)-this->m_nSizeInBits));
    this->m_value >>= ((sizeof(T)<<3)-this->m_nSizeInBits);;
  }
  else
  {
    /* write as bytes */
    T nTmp = 0;
    bitsRead = m_bitReader->readNbytes((unsigned char*)&nTmp, this->m_nSizeInBits>>3);
    /* swap byte positions */
    unsigned char *pIn = (unsigned char*)&nTmp;
    this->m_value = 0;
    if(this->m_bMsbFirst)
    {
      for(int byte=0; byte < ((int)this->m_nSizeInBits>>3); byte++)
      {
        this->m_value <<= 8;
        this->m_value += pIn[byte];
      } 
    }
    else
    {
      for(int byte=0; byte < ((int)this->m_nSizeInBits>>3); byte++)
      {
        this->m_value <<= 8;
        this->m_value += pIn[(this->m_nSizeInBits>>3)-byte-1];
      } 
    }
    if( (this->m_nSizeInBits>>3) < sizeof(T))
      this->m_value <<= (sizeof(T)<<3) - this->m_nSizeInBits;
  }
  return bitsRead;
}

/* -------------------------------------------------------------------------- */

/** 
  * Definition of a floating point field reader
  */
template <class T>
class CFloatingPointFieldRead : public CFloatingPointField<T>, public IBitStreamFieldRead
{
  public:
    CFloatingPointFieldRead() {};
    virtual ~CFloatingPointFieldRead(){};

    /** reads the value from the stream */
    int read()
    {
      if(this->m_bByteAlign)
        return m_bitReader->readNbytes((unsigned char*)&this->m_value, sizeof(T));
      else
      {
        unsigned char *pChar = (unsigned char*) &this->m_value;
        unsigned int nBitsRead = 0;
        unsigned int n = 0;
        while(n<sizeof(T))
        {
          unsigned int nTmp = 0;
          nBitsRead += m_bitReader->readNbits(nTmp, 8);
          pChar[n] = static_cast<char>(nTmp);
          n++;
        }
        return nBitsRead;
      }
    };
};

/* type definition for header field types read*/
typedef CFloatingPointFieldRead<float> FLOAT_FR;
typedef CFloatingPointFieldRead<double> DOUBLE_FR;
typedef CIntegerFieldRead<char> CHAR_FR;
typedef CIntegerFieldRead<unsigned char> UCHAR_FR;
typedef CIntegerFieldRead<short> SHORT_FR;
typedef CIntegerFieldRead<unsigned short> USHORT_FR;
typedef CIntegerFieldRead<int> INT_FR;
typedef CIntegerFieldRead<unsigned int> UINT_FR;
typedef CIntegerFieldRead<long long> INT64_FR;
typedef CIntegerFieldRead<unsigned long long> UINT64_FR;

/** 
  * global helper function for reading into an STL container 
  */
template <class iterator, class FIELD>
unsigned int readArray(iterator begin, iterator end, FIELD &field)
{
  unsigned int nNumBitsRead = 0;
  iterator it = begin;
  while(it != end)
  {
    nNumBitsRead += field.read();
    *it = field;
    it++;
  }
  return nNumBitsRead;
}

// template specialization for std::vector<bool>
template <class FIELD>
unsigned int readArray(std::vector<bool>::iterator begin, std::vector<bool>::iterator end, FIELD &field)
{
  unsigned int nNumBitsRead = 0;
  std::vector<bool>::iterator it = begin;
  while(it != end)
  {
    nNumBitsRead += field.read();
    *it = field==1;
    it++;
  }
  return nNumBitsRead;
}

#endif // __BASICBITSTREAMFIELDSREAD__
