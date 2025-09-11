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
$Id: hoaHuffmanTables.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/


#ifndef __HOAHUFFMANTABLES_H__
#define __HOAHUFFMANTABLES_H__

#include "DataTypes.h"

static const unsigned int g_dirGridTable[4][2] = { {226, 8},
{485, 9},
{901, 10},
{0, 0} };

/**
Class for storing elements of an Huffman table */
template <class T>
class HuffmanWord
{
public:
  HuffmanWord(unsigned int unHuffmanWord, 
    T codedValue, 
    unsigned int unCodeWordLength,
    bool bLastWord = false) : 
  m_unHuffmanWord(unHuffmanWord),
    m_codedValue(codedValue),
    m_unCodeWordLength(unCodeWordLength),
    m_bLastWord(bLastWord)
  {};

public:
  const unsigned int m_unHuffmanWord;
  const T m_codedValue;
  const unsigned int m_unCodeWordLength;
  const bool m_bLastWord;

private:
  HuffmanWord& operator=(HuffmanWord& rhs);
};

// Huffman table for angle differences in non SBR subbands
static const HuffmanWord<int> g_pDecodedAngleDiffTable[16] = 
{ 
  HuffmanWord<int>(126, -7, 7), 
  HuffmanWord<int>(118, -6, 7), 
  HuffmanWord<int>(56, -5, 6), 
  HuffmanWord<int>(6, -4, 6), 
  HuffmanWord<int>(30, -3, 6), 
  HuffmanWord<int>(0, -2, 4), 
  HuffmanWord<int>(2, -1, 3), 
  HuffmanWord<int>(1, 0, 1), 
  HuffmanWord<int>(4, 1, 3), 
  HuffmanWord<int>(14, 2, 5), 
  HuffmanWord<int>(8, 3, 5), 
  HuffmanWord<int>(38, 4, 6), 
  HuffmanWord<int>(24, 5, 6), 
  HuffmanWord<int>(54, 6, 7), 
  HuffmanWord<int>(62, 7, 7), 
  HuffmanWord<int>(22, 8, 6, true) 
};

// Huffman table for magnitude differences in non SBR subbands
static const HuffmanWord<int> g_pDecodedMagDiffTable[18] = 
{ 
  HuffmanWord<int>(42, -8, 7), 
  HuffmanWord<int>(954, -7, 10), 
  HuffmanWord<int>(314, -6, 9), 
  HuffmanWord<int>(26, -5, 8), 
  HuffmanWord<int>(10, -4, 7), 
  HuffmanWord<int>(90, -3, 7), 
  HuffmanWord<int>(2, -2, 5), 
  HuffmanWord<int>(0, -1, 2), 
  HuffmanWord<int>(1, 0, 1), 
  HuffmanWord<int>(6, 1, 3), 
  HuffmanWord<int>(18, 2, 5), 
  HuffmanWord<int>(122, 3, 7), 
  HuffmanWord<int>(106, 4, 7), 
  HuffmanWord<int>(154, 5, 8), 
  HuffmanWord<int>(186, 6, 9), 
  HuffmanWord<int>(58, 7, 9), 
  HuffmanWord<int>(442, 8, 10), 
  HuffmanWord<int>(74, 9, 7, true) 
};

// Huffman table for magnitude differences in SBR subbands
static const HuffmanWord<int> g_pDecodedMagDiffSbrTable[18] = 
{ 
  HuffmanWord<int>(242, -8, 10), 
  HuffmanWord<int>(4082, -7, 12), 
  HuffmanWord<int>(754, -6, 10), 
  HuffmanWord<int>(114, -5, 8), 
  HuffmanWord<int>(66, -4, 7), 
  HuffmanWord<int>(34, -3, 6), 
  HuffmanWord<int>(10, -2, 5), 
  HuffmanWord<int>(0, -1, 2), 
  HuffmanWord<int>(1, 0, 1), 
  HuffmanWord<int>(6, 1, 3), 
  HuffmanWord<int>(26, 2, 5), 
  HuffmanWord<int>(18, 3, 6), 
  HuffmanWord<int>(50, 4, 7), 
  HuffmanWord<int>(2, 5, 7), 
  HuffmanWord<int>(498, 6, 10), 
  HuffmanWord<int>(3058, 7, 12), 
  HuffmanWord<int>(1010, 8, 12), 
  HuffmanWord<int>(2034, 9, 12, true) 
};

static const HuffmanWord<int> g_pDecodedParAngleDiffTable[16] = 
{ 
  HuffmanWord<int>(57, -7, 6), 
  HuffmanWord<int>(39, -6, 6), 
  HuffmanWord<int>(1, -5, 5), 
  HuffmanWord<int>(9, -4, 5), 
  HuffmanWord<int>(12, -3, 4), 
  HuffmanWord<int>(0, -2, 3), 
  HuffmanWord<int>(3, -1, 3), 
  HuffmanWord<int>(2, 0, 2), 
  HuffmanWord<int>(5, 1, 3), 
  HuffmanWord<int>(15, 2, 4), 
  HuffmanWord<int>(4, 3, 4), 
  HuffmanWord<int>(17, 4, 5), 
  HuffmanWord<int>(55, 5, 6), 
  HuffmanWord<int>(23, 6, 6), 
  HuffmanWord<int>(25, 7, 6), 
  HuffmanWord<int>(7, 8, 6, true) 
};

static const HuffmanWord<int> g_pDecodedParMagDiffTable[18] = 
{ 
  HuffmanWord<int>(28, -8, 9), 
  HuffmanWord<int>(156, -7, 8), 
  HuffmanWord<int>(36, -6, 6), 
  HuffmanWord<int>(20, -5, 5), 
  HuffmanWord<int>(5, -4, 5), 
  HuffmanWord<int>(2, -3, 4), 
  HuffmanWord<int>(13, -2, 4), 
  HuffmanWord<int>(1, -1, 3), 
  HuffmanWord<int>(3, 0, 2), 
  HuffmanWord<int>(6, 1, 3), 
  HuffmanWord<int>(0, 2, 3), 
  HuffmanWord<int>(10, 3, 4), 
  HuffmanWord<int>(21, 4, 5), 
  HuffmanWord<int>(12, 5, 5), 
  HuffmanWord<int>(60, 6, 6), 
  HuffmanWord<int>(4, 7, 6), 
  HuffmanWord<int>(92, 8, 7), 
  HuffmanWord<int>(284, 9, 9, true) 
};

static const HuffmanWord<int> g_pDecodedParMagDiffSbrTable[18] = 
{ 
  HuffmanWord<int>(97, -8, 10), 
  HuffmanWord<int>(191, -7, 8), 
  HuffmanWord<int>(353, -6, 9), 
  HuffmanWord<int>(255, -5, 8), 
  HuffmanWord<int>(1, -4, 6), 
  HuffmanWord<int>(15, -3, 5), 
  HuffmanWord<int>(9, -2, 4), 
  HuffmanWord<int>(3, -1, 3), 
  HuffmanWord<int>(0, 0, 1), 
  HuffmanWord<int>(5, 1, 3), 
  HuffmanWord<int>(7, 2, 4), 
  HuffmanWord<int>(17, 3, 5), 
  HuffmanWord<int>(31, 4, 6), 
  HuffmanWord<int>(33, 5, 7), 
  HuffmanWord<int>(225, 6, 8), 
  HuffmanWord<int>(63, 7, 8), 
  HuffmanWord<int>(127, 8, 8), 
  HuffmanWord<int>(609, 9, 10, true) 
};

template <class T>
static int getIdxFromValue(T value, const HuffmanWord<T> *pTable) 
{
  int idx = 0;
  bool bNotFound = false;
  while( !bNotFound & (pTable[idx].m_codedValue != value) )
  {
    bNotFound = pTable[idx].m_bLastWord;
    idx++;
  }
  if(bNotFound)
    idx = -1;
  return idx;
}

template <class T>
static int getIdxFromCodeWord(unsigned int codeWord, const HuffmanWord<T> *pTable, const unsigned int CodeWordLen) 
{
  int idx = 0;
  bool bNotFound = false;
  while( !bNotFound & ((pTable[idx].m_unHuffmanWord != codeWord) | (pTable[idx].m_unCodeWordLength != CodeWordLen)))
  { 
    bNotFound = pTable[idx].m_bLastWord;
    idx++;
  }
  if(bNotFound)
    idx = -1;
  return idx;
}

#endif