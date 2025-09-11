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
 $Id: basicbitstreamfields.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#ifndef __BASICBITSTREAMFIELDS__
#define __BASICBITSTREAMFIELDS__

#include <vector>
#include "ibitstreamfield.h"

/*
 * Definition of an integer field
 * T should be singed or unsigned  char, short, int or int64  
 */
template <class T>
class CIntegerField : public IBitStreamField
{
  public:
    CIntegerField() : m_value(0), m_bMsbFirst(true), m_bByteAlign(false), m_nSizeInBits(sizeof(T)<<3)
    {
    };
    virtual ~CIntegerField(){};
    
    /**
     * Set byte order to Most Significant Byte First (bIsMsbFirst = true)
     * or Least Significant Byte First (bIsMsbFirst = false) 
     * (only used if m_bByteAlign==true ) */
    void setWriteByteOrder(bool bIsMsbFirst)
    {
      m_bMsbFirst = bIsMsbFirst;
    };

    /** 
      * set this flag if the int. value shuld be representend byte wise
      * clear this flag for bit representation */
    void setByteAlignFlag(bool bByteAlign)
    {
      m_bByteAlign = bByteAlign;
    };

    /**
     * Set number of bits used for reading or writing from stream */
    void setFieldSizeInBits(unsigned int nNumBits)
    {
      if(nNumBits <= sizeof(T)<<3)
        m_nSizeInBits = nNumBits;
      // set num bits to a multiple of eight if m_bByteAlign==true
      if( ((nNumBits%8)!=0) & m_bByteAlign)
        m_nSizeInBits = (nNumBits/8) + 8;
    };

    /* use operator= to set the integer value */
    CIntegerField<T>& operator=(const T &in)
    {
      m_value = in;
      return *this;
    };

    /* assignment operator to copy the field */
    IBitStreamField& operator=(const IBitStreamField &in)
    {
      const CIntegerField<T> &tmp = (const CIntegerField<T> &)in;
      m_value = tmp.m_value;
      m_bMsbFirst = tmp.m_bMsbFirst;
      m_nSizeInBits = tmp.m_nSizeInBits;
      return *this;
    };

    /* use type cast to T to get the integer value */
    operator T()
    {
      return m_value;
    };

    /* return the size in bits */
    unsigned int getSizeInBit()
    {
      return m_nSizeInBits;
    }

    /* return the size in bytes */
    unsigned int getSizeInBytes()
    {
      return (m_nSizeInBits>>3) + ((m_nSizeInBits%8)>0);
    }

    /* return number of entries */
    unsigned int getNumEntries()
    {
      return 1;
    }

  protected: /* variables */
    T m_value; // actual value
    bool m_bMsbFirst; // Write MSB first
    bool m_bByteAlign; // integer represented as multiple of eight bits else LSBit-wise representation 
    unsigned int m_nSizeInBits; // number of bits used for writing
};

/* -------------------------------------------------------------------------- */

/*
 * Definition of a floating point field
 * T should be float or double
 */
template <class T>
class CFloatingPointField : public IBitStreamField
{
  public:
    CFloatingPointField() : m_value((T)0.0), m_bByteAlign(true)
    {
    };
    virtual ~CFloatingPointField(){};

    /** use the operator= to set a floating point value */
    CFloatingPointField& operator=(const T &in)
    {
      m_value = in;
      return *this;
    };

     /** use the operator= to copy the field */
    IBitStreamField& operator=(const IBitStreamField &in)
    {
      CFloatingPointField<T>& tmp = (CFloatingPointField<T>&)in;
      m_value = tmp.m_value;
      m_bByteAlign = tmp.m_bByteAlign;
      return *this;
    };

     /** use the operator= to copy the field */
    CFloatingPointField<T>& operator=(const CFloatingPointField<T> &in)
    {
      m_value = in.m_value;
      m_bByteAlign = in.m_bByteAlign;
      return *this;
    };

    /** use the cast operator to T to get the floating point value */
    operator T()
    {
      return m_value;
    };

    
    /* return the size in bits */
    unsigned int getSizeInBit()
    {
      return (sizeof(T))<<3;
    };

    /* return the size in bytes */
    unsigned int getSizeInBytes()
    {
      return (sizeof(T));
    };

    /* return number of entries */
    unsigned int getNumEntries()
    {
      return 1;
    }

     /** 
      * set this flag if the float. value should be represented byte wise
      * clear this flag for bit representation */
    void setByteAlignFlag(bool bByteAlign)
    {
      m_bByteAlign = bByteAlign;
    };

  protected: /* variables */
    T m_value; // actual float value
    bool m_bByteAlign; // byte align buffer before writing the float value
};


/* type definition for header field types */
typedef CFloatingPointField<float> FLOAT_F;
typedef CFloatingPointField<double> DOUBLE_F;
typedef CIntegerField<char> CHAR_F;
typedef CIntegerField<unsigned char> UCHAR_F;
typedef CIntegerField<short> SHORT_F;
typedef CIntegerField<unsigned short> USHORT_F;
typedef CIntegerField<int> INT_F;
typedef CIntegerField<unsigned int> UINT_F;
typedef CIntegerField<long long> INT64_F;
typedef CIntegerField<unsigned long long> UINT64_F;

/* helper functions */

// compute the size in bit of an STL container of a bit stream field */
template <class iterator>
static unsigned int getSizeInBits(iterator begin, iterator end, IBitStreamField & field)
{
  unsigned int  sizeInBits = 0;
  iterator it = begin;
  while(it != end)
  {
    sizeInBits += field.getSizeInBit();
    it++;
  }
  return sizeInBits;
};

// compute the size in byte of an STL container of a bit stream field */
template <class iterator>
static unsigned int getSizeInByte(iterator begin, iterator end, IBitStreamField & field)
{
  unsigned int  sizeInBytes = 0;
  iterator it = begin;
  while(it != end)
  {
    sizeInBytes += field.getSizeInBytes();
    it++;
  }
  return sizeInBytes;
};

#endif // __BASICBITSTREAMFIELDS__
