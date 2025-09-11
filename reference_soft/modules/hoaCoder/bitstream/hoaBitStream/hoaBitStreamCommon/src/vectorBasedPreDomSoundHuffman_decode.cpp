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
 $Id: vectorBasedPreDomSoundHuffman_decode.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#include "vectorBasedPreDomSoundHuffman_decode.h"

void getHuffmanTables(std::map <IDX1_CB1, Codebook> &huffmannTable)
{

  /* Codebook 6 */
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("0010")]             = 0;
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("000")]              = 1;
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("10")]               = 2;
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("01")]               = 3;
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("11")]               = 4;
  huffmannTable[6].codeBook[1][IDX3_CODEWORD("0011")]             = 5;

  huffmannTable[6].codeBook[2][IDX3_CODEWORD("000")]              = 0;
  huffmannTable[6].codeBook[2][IDX3_CODEWORD("11")]               = 1;
  huffmannTable[6].codeBook[2][IDX3_CODEWORD("01")]               = 2;
  huffmannTable[6].codeBook[2][IDX3_CODEWORD("10")]               = 3;
  huffmannTable[6].codeBook[2][IDX3_CODEWORD("0010")]             = 4;
  huffmannTable[6].codeBook[2][IDX3_CODEWORD("0011")]             = 5;

  huffmannTable[6].codeBook[3][IDX3_CODEWORD("1")]                = 0;
  huffmannTable[6].codeBook[3][IDX3_CODEWORD("01")]               = 1;
  huffmannTable[6].codeBook[3][IDX3_CODEWORD("000")]              = 2;
  huffmannTable[6].codeBook[3][IDX3_CODEWORD("0010")]             = 3;
  huffmannTable[6].codeBook[3][IDX3_CODEWORD("00110")]            = 4;
  huffmannTable[6].codeBook[3][IDX3_CODEWORD("00111")]            = 5;

  huffmannTable[6].codeBook[4][IDX3_CODEWORD("0")]                = 0;
  huffmannTable[6].codeBook[4][IDX3_CODEWORD("11")]               = 1;
  huffmannTable[6].codeBook[4][IDX3_CODEWORD("100")]              = 2;
  huffmannTable[6].codeBook[4][IDX3_CODEWORD("1010")]             = 3;
  huffmannTable[6].codeBook[4][IDX3_CODEWORD("10110")]            = 4;
  huffmannTable[6].codeBook[4][IDX3_CODEWORD("10111")]            = 5;

  huffmannTable[6].codeBook[5][IDX3_CODEWORD("01")]               = 0;
  huffmannTable[6].codeBook[5][IDX3_CODEWORD("10")]               = 1;
  huffmannTable[6].codeBook[5][IDX3_CODEWORD("11")]               = 2;
  huffmannTable[6].codeBook[5][IDX3_CODEWORD("000")]              = 3;
  huffmannTable[6].codeBook[5][IDX3_CODEWORD("0010")]             = 4;
  huffmannTable[6].codeBook[5][IDX3_CODEWORD("0011")]             = 5;

  /* Codebook 7 */
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("1110")]             = 0;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("110")]              = 1;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("001")]              = 2;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("10")]               = 3;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("01")]               = 4;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("000")]              = 5;
  huffmannTable[7].codeBook[1][IDX3_CODEWORD("1111")]             = 6;

  huffmannTable[7].codeBook[2][IDX3_CODEWORD("1110")]             = 0;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("001")]              = 1;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("000")]              = 2;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("01")]               = 3;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("10")]               = 4;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("110")]              = 5;
  huffmannTable[7].codeBook[2][IDX3_CODEWORD("1111")]             = 6;

  huffmannTable[7].codeBook[3][IDX3_CODEWORD("01")]               = 0;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("10")]               = 1;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("11")]               = 2;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("000")]              = 3;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("0010")]             = 4;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("00110")]            = 5;
  huffmannTable[7].codeBook[3][IDX3_CODEWORD("00111")]            = 6;

  huffmannTable[7].codeBook[4][IDX3_CODEWORD("0")]                = 0;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("11")]               = 1;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("101")]              = 2;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("1000")]             = 3;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("10010")]            = 4;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("100110")]           = 5;
  huffmannTable[7].codeBook[4][IDX3_CODEWORD("100111")]           = 6;

  huffmannTable[7].codeBook[5][IDX3_CODEWORD("001")]              = 0;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("011")]              = 1;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("11")]               = 2;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("10")]               = 3;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("000")]              = 4;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("0100")]             = 5;
  huffmannTable[7].codeBook[5][IDX3_CODEWORD("0101")]             = 6;

  /* Codebook 8 */
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("10001")]            = 0;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("1001")]             = 1;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("101")]              = 2;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("001")]              = 3;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("11")]               = 4;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("01")]               = 5;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("000")]              = 6;
  huffmannTable[8].codeBook[1][IDX3_CODEWORD("10000")]            = 7;

  huffmannTable[8].codeBook[2][IDX3_CODEWORD("00110")]            = 0;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("0010")]             = 1;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("100")]              = 2;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("000")]              = 3;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("01")]               = 4;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("11")]               = 5;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("101")]              = 6;
  huffmannTable[8].codeBook[2][IDX3_CODEWORD("00111")]            = 7;

  huffmannTable[8].codeBook[3][IDX3_CODEWORD("001")]              = 0;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("000")]              = 1;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("10")]               = 2;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("11")]               = 3;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("010")]              = 4;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("0110")]             = 5;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("01110")]            = 6;
  huffmannTable[8].codeBook[3][IDX3_CODEWORD("01111")]            = 7;

  huffmannTable[8].codeBook[4][IDX3_CODEWORD("1")]                = 0;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("000")]              = 1;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("010")]              = 2;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("011")]              = 3;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("0010")]             = 4;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("00110")]            = 5;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("001110")]           = 6;
  huffmannTable[8].codeBook[4][IDX3_CODEWORD("001111")]           = 7;

  huffmannTable[8].codeBook[5][IDX3_CODEWORD("0100")]             = 0;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("101")]              = 1;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("011")]              = 2;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("001")]              = 3;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("11")]               = 4;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("000")]              = 5;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("100")]              = 6;
  huffmannTable[8].codeBook[5][IDX3_CODEWORD("0101")]             = 7;

  /* Codebook 9 */
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("100001")]           = 0;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("100000")]           = 1;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("1001")]             = 2;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("101")]              = 3;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("001")]              = 4;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("11")]               = 5;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("01")]               = 6;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("000")]              = 7;
  huffmannTable[9].codeBook[1][IDX3_CODEWORD("10001")]            = 8;

  huffmannTable[9].codeBook[2][IDX3_CODEWORD("001110")]           = 0;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("00110")]            = 1;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("0010")]             = 2;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("100")]              = 3;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("000")]              = 4;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("01")]               = 5;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("11")]               = 6;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("101")]              = 7;
  huffmannTable[9].codeBook[2][IDX3_CODEWORD("001111")]           = 8;

  huffmannTable[9].codeBook[3][IDX3_CODEWORD("101")]              = 0;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("011")]              = 1;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("001")]              = 2;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("000")]              = 3;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("11")]               = 4;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("010")]              = 5;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("1000")]             = 6;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("10010")]            = 7;
  huffmannTable[9].codeBook[3][IDX3_CODEWORD("10011")]            = 8;

  huffmannTable[9].codeBook[4][IDX3_CODEWORD("00")]               = 0;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("010")]              = 1;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("011")]              = 2;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("100")]              = 3;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("101")]              = 4;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("110")]              = 5;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("1110")]             = 6;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("11110")]            = 7;
  huffmannTable[9].codeBook[4][IDX3_CODEWORD("11111")]            = 8;

  huffmannTable[9].codeBook[5][IDX3_CODEWORD("1100")]             = 0;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("111")]              = 1;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("101")]              = 2;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("011")]              = 3;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("010")]              = 4;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("000")]              = 5;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("001")]              = 6;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("100")]              = 7;
  huffmannTable[9].codeBook[5][IDX3_CODEWORD("1101")]             = 8;

  /* Codebook 10 */
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("0010011")]          = 0;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("0010010")]          = 1;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("001000")]           = 2;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("0011")]             = 3;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("101")]              = 4;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("100")]              = 5;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("11")]               = 6;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("01")]               = 7;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("000")]              = 8;
  huffmannTable[10].codeBook[1][IDX3_CODEWORD("00101")]            = 9;

  huffmannTable[10].codeBook[2][IDX3_CODEWORD("0010000")]          = 0;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("001001")]           = 1;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("00101")]            = 2;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("0011")]             = 3;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("100")]              = 4;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("000")]              = 5;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("01")]               = 6;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("11")]               = 7;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("101")]              = 8;
  huffmannTable[10].codeBook[2][IDX3_CODEWORD("0010001")]          = 9;

  huffmannTable[10].codeBook[3][IDX3_CODEWORD("1010")]             = 0;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("0001")]             = 1;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("100")]              = 2;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("011")]              = 3;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("001")]              = 4;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("11")]               = 5;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("010")]              = 6;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("0000")]             = 7;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("10110")]            = 8;
  huffmannTable[10].codeBook[3][IDX3_CODEWORD("10111")]            = 9;

  huffmannTable[10].codeBook[4][IDX3_CODEWORD("01")]               = 0;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("101")]              = 1;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("001")]              = 2;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("100")]              = 3;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("110")]              = 4;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("111")]              = 5;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("0000")]             = 6;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("00010")]            = 7;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("000110")]           = 8;
  huffmannTable[10].codeBook[4][IDX3_CODEWORD("000111")]           = 9;

  huffmannTable[10].codeBook[5][IDX3_CODEWORD("1111")]             = 0;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("1001")]             = 1;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("1000")]             = 2;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("110")]              = 3;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("011")]              = 4;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("010")]              = 5;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("000")]              = 6;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("001")]              = 7;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("101")]              = 8;
  huffmannTable[10].codeBook[5][IDX3_CODEWORD("1110")]             = 9;

  /* Codebook 11 */
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("0011111")]          = 0;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("0011110")]          = 1;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("001110")]           = 2;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("00110")]            = 3;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("1010")]             = 4;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("0010")]             = 5;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("100")]              = 6;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("11")]               = 7;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("01")]               = 8;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("000")]              = 9;
  huffmannTable[11].codeBook[1][IDX3_CODEWORD("1011")]             = 10;

  huffmannTable[11].codeBook[2][IDX3_CODEWORD("00100001")]         = 0;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("0010001")]          = 1;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("001001")]           = 2;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("00101")]            = 3;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("0011")]             = 4;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("100")]              = 5;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("000")]              = 6;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("01")]               = 7;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("11")]               = 8;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("101")]              = 9;
  huffmannTable[11].codeBook[2][IDX3_CODEWORD("00100000")]         = 10;

  huffmannTable[11].codeBook[3][IDX3_CODEWORD("100010")]           = 0;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("1001")]             = 1;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("0011")]             = 2;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("101")]              = 3;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("011")]              = 4;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("000")]              = 5;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("11")]               = 6;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("010")]              = 7;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("0010")]             = 8;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("10000")]            = 9;
  huffmannTable[11].codeBook[3][IDX3_CODEWORD("100011")]           = 10;

  huffmannTable[11].codeBook[4][IDX3_CODEWORD("10")]               = 0;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("0010")]             = 1;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("111")]              = 2;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("010")]              = 3;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("011")]              = 4;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("110")]              = 5;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("0000")]             = 6;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("0001")]             = 7;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("00110")]            = 8;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("001110")]           = 9;
  huffmannTable[11].codeBook[4][IDX3_CODEWORD("001111")]           = 10;

  huffmannTable[11].codeBook[5][IDX3_CODEWORD("11101")]            = 0;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("11100")]            = 1;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("1011")]             = 2;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("1010")]             = 3;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("110")]              = 4;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("011")]              = 5;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("010")]              = 6;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("000")]              = 7;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("001")]              = 8;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("100")]              = 9;
  huffmannTable[11].codeBook[5][IDX3_CODEWORD("1111")]             = 10;

  /* Codebook 12 */
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("00111111")]         = 0;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("00111110")]         = 1;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("0011110")]          = 2;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("001110")]           = 3;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("00110")]            = 4;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("1010")]             = 5;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("0010")]             = 6;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("100")]              = 7;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("11")]               = 8;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("01")]               = 9;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("000")]              = 10;
  huffmannTable[12].codeBook[1][IDX3_CODEWORD("1011")]             = 11;

  huffmannTable[12].codeBook[2][IDX3_CODEWORD("001000011")]        = 0;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("001000010")]        = 1;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("0010001")]          = 2;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("001001")]           = 3;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("00101")]            = 4;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("0011")]             = 5;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("100")]              = 6;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("000")]              = 7;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("01")]               = 8;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("11")]               = 9;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("101")]              = 10;
  huffmannTable[12].codeBook[2][IDX3_CODEWORD("00100000")]         = 11;

  huffmannTable[12].codeBook[3][IDX3_CODEWORD("0001010")]          = 0;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("000100")]           = 1;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("1001")]             = 2;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("1000")]             = 3;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("101")]              = 4;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("011")]              = 5;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("001")]              = 6;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("11")]               = 7;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("010")]              = 8;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("0000")]             = 9;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("00011")]            = 10;
  huffmannTable[12].codeBook[3][IDX3_CODEWORD("0001011")]          = 11;

  huffmannTable[12].codeBook[4][IDX3_CODEWORD("000")]              = 0;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("0101")]             = 1;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("0011")]             = 2;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("111")]              = 3;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("100")]              = 4;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("011")]              = 5;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("101")]              = 6;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("110")]              = 7;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("0010")]             = 8;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("01000")]            = 9;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("010010")]           = 10;
  huffmannTable[12].codeBook[4][IDX3_CODEWORD("010011")]           = 11;

  huffmannTable[12].codeBook[5][IDX3_CODEWORD("100111")]           = 0;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("100110")]           = 1;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("10010")]            = 2;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("1110")]             = 3;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("1000")]             = 4;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("110")]              = 5;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("011")]              = 6;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("010")]              = 7;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("000")]              = 8;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("001")]              = 9;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("101")]              = 10;
  huffmannTable[12].codeBook[5][IDX3_CODEWORD("1111")]             = 11;

  /* Codebook 13 */
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("001111111")]        = 0;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("001111110")]        = 1;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("00111110")]         = 2;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("0011110")]          = 3;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("001110")]           = 4;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("00110")]            = 5;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("1010")]             = 6;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("0010")]             = 7;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("100")]              = 8;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("11")]               = 9;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("01")]               = 10;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("000")]              = 11;
  huffmannTable[13].codeBook[1][IDX3_CODEWORD("1011")]             = 12;

  huffmannTable[13].codeBook[2][IDX3_CODEWORD("0010000111")]       = 0;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("0010000110")]       = 1;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("001000010")]        = 2;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("0010001")]          = 3;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("001001")]           = 4;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("00101")]            = 5;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("0011")]             = 6;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("100")]              = 7;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("000")]              = 8;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("01")]               = 9;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("11")]               = 10;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("101")]              = 11;
  huffmannTable[13].codeBook[2][IDX3_CODEWORD("00100000")]         = 12;

  huffmannTable[13].codeBook[3][IDX3_CODEWORD("1001110")]          = 0;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("100110")]           = 1;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("10010")]            = 2;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("00010")]            = 3;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("1000")]             = 4;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("101")]              = 5;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("011")]              = 6;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("001")]              = 7;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("11")]               = 8;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("010")]              = 9;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("0000")]             = 10;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("00011")]            = 11;
  huffmannTable[13].codeBook[3][IDX3_CODEWORD("1001111")]          = 12;

  huffmannTable[13].codeBook[4][IDX3_CODEWORD("001")]              = 0;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("01010")]            = 1;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("1110")]             = 2;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("0100")]             = 3;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("0000")]             = 4;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("100")]              = 5;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("011")]              = 6;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("101")]              = 7;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("110")]              = 8;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("0001")]             = 9;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("1111")]             = 10;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("010110")]           = 11;
  huffmannTable[13].codeBook[4][IDX3_CODEWORD("010111")]           = 12;

  huffmannTable[13].codeBook[5][IDX3_CODEWORD("1001011")]          = 0;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("1001010")]          = 1;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("100100")]           = 2;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("10011")]            = 3;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("1110")]             = 4;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("1000")]             = 5;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("110")]              = 6;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("011")]              = 7;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("010")]              = 8;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("000")]              = 9;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("001")]              = 10;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("101")]              = 11;
  huffmannTable[13].codeBook[5][IDX3_CODEWORD("1111")]             = 12;

  /* Codebook 14 */
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("0011011111")]       = 0;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("0011011110")]       = 1;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("001101110")]        = 2;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("00110110")]         = 3;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("0011010")]          = 4;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("001100")]           = 5;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("00111")]            = 6;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("1010")]             = 7;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("0010")]             = 8;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("100")]              = 9;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("11")]               = 10;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("01")]               = 11;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("000")]              = 12;
  huffmannTable[14].codeBook[1][IDX3_CODEWORD("1011")]             = 13;

  huffmannTable[14].codeBook[2][IDX3_CODEWORD("00100000111")]      = 0;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("00100000110")]      = 1;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("0010000010")]       = 2;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("001000000")]        = 3;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("0010001")]          = 4;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("001001")]           = 5;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("00101")]            = 6;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("0011")]             = 7;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("100")]              = 8;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("000")]              = 9;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("01")]               = 10;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("11")]               = 11;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("101")]              = 12;
  huffmannTable[14].codeBook[2][IDX3_CODEWORD("00100001")]         = 13;

  huffmannTable[14].codeBook[3][IDX3_CODEWORD("10010111")]         = 0;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("1001010")]          = 1;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("100100")]           = 2;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("10011")]            = 3;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("00010")]            = 4;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("1000")]             = 5;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("101")]              = 6;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("011")]              = 7;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("001")]              = 8;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("11")]               = 9;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("010")]              = 10;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("0000")]             = 11;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("00011")]            = 12;
  huffmannTable[14].codeBook[3][IDX3_CODEWORD("10010110")]         = 13;

  huffmannTable[14].codeBook[4][IDX3_CODEWORD("010")]              = 0;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("11100")]            = 1;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("00101")]            = 2;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("1111")]             = 3;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("0011")]             = 4;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("0000")]             = 5;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("100")]              = 6;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("011")]              = 7;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("101")]              = 8;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("110")]              = 9;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("0001")]             = 10;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("00100")]            = 11;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("111010")]           = 12;
  huffmannTable[14].codeBook[4][IDX3_CODEWORD("111011")]           = 13;

  huffmannTable[14].codeBook[5][IDX3_CODEWORD("01110111")]         = 0;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("01110110")]         = 1;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("0111010")]          = 2;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("011100")]           = 3;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("01111")]            = 4;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("1110")]             = 5;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("0110")]             = 6;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("110")]              = 7;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("100")]              = 8;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("010")]              = 9;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("000")]              = 10;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("001")]              = 11;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("101")]              = 12;
  huffmannTable[14].codeBook[5][IDX3_CODEWORD("1111")]             = 13;

  /* Codebook 15 */
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("00110111111")]      = 0;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("00110111110")]      = 1;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("0011011110")]       = 2;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("001101110")]        = 3;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("00110110")]         = 4;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("0011010")]          = 5;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("001100")]           = 6;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("00111")]            = 7;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("1010")]             = 8;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("0010")]             = 9;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("100")]              = 10;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("11")]               = 11;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("01")]               = 12;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("000")]              = 13;
  huffmannTable[15].codeBook[1][IDX3_CODEWORD("1011")]             = 14;

  huffmannTable[15].codeBook[2][IDX3_CODEWORD("00100101111")]      = 0;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("00100101110")]      = 1;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("0010010110")]       = 2;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("001001010")]        = 3;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("00100100")]         = 4;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("0010001")]          = 5;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("0010000")]          = 6;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("00101")]            = 7;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("0011")]             = 8;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("100")]              = 9;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("000")]              = 10;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("01")]               = 11;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("11")]               = 12;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("101")]              = 13;
  huffmannTable[15].codeBook[2][IDX3_CODEWORD("0010011")]          = 14;

  huffmannTable[15].codeBook[3][IDX3_CODEWORD("100100001")]        = 0;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("10010001")]         = 1;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("1001001")]          = 2;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("100101")]           = 3;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("10011")]            = 4;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("00010")]            = 5;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("1000")]             = 6;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("101")]              = 7;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("011")]              = 8;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("001")]              = 9;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("11")]               = 10;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("010")]              = 11;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("0000")]             = 12;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("00011")]            = 13;
  huffmannTable[15].codeBook[3][IDX3_CODEWORD("100100000")]        = 14;

  huffmannTable[15].codeBook[4][IDX3_CODEWORD("110")]              = 0;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0101")]             = 1;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("001100")]           = 2;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("00111")]            = 3;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("00000")]            = 4;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0100")]             = 5;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0001")]             = 6;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("100")]              = 7;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("011")]              = 8;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("101")]              = 9;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("111")]              = 10;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0010")]             = 11;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("00001")]            = 12;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0011010")]          = 13;
  huffmannTable[15].codeBook[4][IDX3_CODEWORD("0011011")]          = 14;

  huffmannTable[15].codeBook[5][IDX3_CODEWORD("011101111")]        = 0;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("011101110")]        = 1;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("01110110")]         = 2;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("0111010")]          = 3;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("011100")]           = 4;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("01111")]            = 5;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("1110")]             = 6;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("0110")]             = 7;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("110")]              = 8;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("100")]              = 9;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("010")]              = 10;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("000")]              = 11;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("001")]              = 12;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("101")]              = 13;
  huffmannTable[15].codeBook[5][IDX3_CODEWORD("1111")]             = 14;
}

IDX2_CB2 getSubidx(unsigned int hoaCoeffIndex)
// determine codebook subindex dependent on the HOA component index (starting at 1)
{
  if(hoaCoeffIndex >= 1 && hoaCoeffIndex <= 4)
  {
    return(1);
  }
  else if(hoaCoeffIndex >= 5 && hoaCoeffIndex <= 9)
  {
    return(2);
  }
  else if(hoaCoeffIndex >= 10 && hoaCoeffIndex <= 49)
  {
    return(3);
  }
  else
  { // hoaCoeffIndex out of range
    return(0);
  }
}

IDX4_SYMBOL decodeHuffmanSymbol(std::map <IDX1_CB1, Codebook> &huffmannTable,
  IDX1_CB1 cbIndex1, bool predFlag, bool cbFlag, 
  IDX3_CODEWORD huffmanCodeword, unsigned int hoaCoeffIndex)
{
  IDX4_SYMBOL decValue;
  // determine the subindex for the codebook
  IDX2_CB2 cbIndex2;
  if (!predFlag && !cbFlag)
  {
    cbIndex2 = 5;
  }
  else if (predFlag && !cbFlag)
  {
    cbIndex2 = 4;
  }
  else
  {
    cbIndex2 = getSubidx(hoaCoeffIndex);
  }

  MAP_ENTRY_IT it = huffmannTable[cbIndex1].codeBook[cbIndex2].find(huffmanCodeword);
  if(it == huffmannTable[cbIndex1].codeBook[cbIndex2].end())
  {
    // huffmanCodeword not found in the Huffman table
    decValue = CODEWORD_NOT_FOUND;
  }
  else
  {
    decValue = it->second;
  }
  return decValue;
}

