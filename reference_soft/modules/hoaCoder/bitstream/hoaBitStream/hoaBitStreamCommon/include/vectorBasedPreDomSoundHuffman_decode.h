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
 $Id: vectorBasedPreDomSoundHuffman_decode.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#ifndef __VECTORBASEDPREDOMSOUNDHUFFMAN_DECODE__
#define __VECTORBASEDPREDOMSOUNDHUFFMAN_DECODE__

#include <map>
#include <string>

// idx 1: codebook index 6..16
// idx 2: codebook index2: 1..5
// idx 3: string with codeword from bitstream
// idx 4: decoded value
typedef unsigned short  IDX1_CB1;
typedef unsigned short  IDX2_CB2;
typedef std::string   IDX3_CODEWORD;
typedef unsigned short  IDX4_SYMBOL;

typedef std::map <IDX3_CODEWORD, IDX4_SYMBOL> MAP_ENTRY;
typedef MAP_ENTRY::iterator MAP_ENTRY_IT;

#define CODEWORD_NOT_FOUND 256

/*
  To avoid Visual Studio 2012 compiler warning "Compiler Warning (level 1) C4503":
  reduce number of map levels and use a struct
*/
struct Codebook
{
  std::map <IDX2_CB2, MAP_ENTRY> codeBook;
};

/*
  fill map huffmannTable with Huffman table entries
*/
void getHuffmanTables(std::map <IDX1_CB1, Codebook> &huffmannTable);

/*
  decoding of one Huffman coded value for reconstruction of V vector
  huffmannTable  - map with the Huffman table
  cbIndex1       - codebook index in 6..16
  predFlag       - prediction flag 
  cbFlag         - codebook flag
  huffmanCodeword - Huffman code word as string
  hoaCoeffIndex  - index of the HOA coeff to be decoded, starting at 1

  return value: decoded value, CODEWORD_NOT_FOUND if code word is not found in the table
*/
IDX4_SYMBOL decodeHuffmanSymbol(std::map <IDX1_CB1, Codebook> &huffmannTable,
  IDX1_CB1 cbIndex1, bool predFlag, bool cbFlag, 
  IDX3_CODEWORD huffmanCodeword, unsigned int hoaCoeffIndex);

#endif //__VECTORBASEDPREDOMSOUNDHUFFMAN_DECODE__