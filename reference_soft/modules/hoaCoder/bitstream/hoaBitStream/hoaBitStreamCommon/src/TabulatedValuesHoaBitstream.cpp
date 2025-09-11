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
 $Rev: 196 $
 $Author: technicolor-ks $
 $Date: 2015-10-12 13:45:11 +0200 (Mo, 12 Okt 2015) $
 $Id: TabulatedValuesHoaBitstream.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#include "TabulatedValuesHoaBitstream.h"
#include <iostream>

bool TabulatedValuesHoaBitstream::getPredSubbandTable(std::vector<std::vector<unsigned int> > &rvvunPredSubbandTable)
{
  unsigned int pnNumSubbands[2] = {10, 20};
  unsigned int pnSubBandWidthTable[2][20] = {{1,1,1,2,2,2,3,6,11,35},
                                             {1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,4,5,7,11,18}};

  return getSubbandTable(pnNumSubbands, pnSubBandWidthTable, rvvunPredSubbandTable);
}


bool TabulatedValuesHoaBitstream::getParSubbandTable(std::vector<std::vector<unsigned int> > &rvvunParSubbandTable)
{
  unsigned int pnNumSubbands[2] = {4, 8};
  unsigned int pnSubBandWidthTable[2][20] = {{1,1,22,40},
                                             {1,1,1,2,2,5,10,42}};

  return getSubbandTable(pnNumSubbands, pnSubBandWidthTable, rvvunParSubbandTable);
}

bool TabulatedValuesHoaBitstream::getSubbandTable(unsigned int *pnNumSubbands, unsigned int (*pnSubBandWidthTable)[20], std::vector<std::vector<unsigned int>> &rvvunSubbandTable)
{
  std::vector<unsigned int> tmp;
  tmp.resize(0);
  rvvunSubbandTable.push_back(tmp);

  for(unsigned int nn = 0; nn < 2; nn++)
  {
    tmp.resize(pnNumSubbands[nn]);
    unsigned int bwSum = 0;
    for(unsigned int kk = 0; kk < pnNumSubbands[nn]; kk++)
    {
      tmp[kk] = pnSubBandWidthTable[nn][kk];
      bwSum += tmp[kk];
    }
    if (bwSum != 64)
    {
      std::cerr << "wrong number of QMF bands\n";
      return true;
    }
    rvvunSubbandTable.push_back(tmp);
  }

  return false;
}


bool TabulatedValuesHoaBitstream::getNumOfBitsPerDirIdxTable(std::vector<unsigned int> &rvunNumOfBitsPerDirIdxTable)
{
  rvunNumOfBitsPerDirIdxTable.clear();
  rvunNumOfBitsPerDirIdxTable.resize(4);
  rvunNumOfBitsPerDirIdxTable[0] = 8;
  rvunNumOfBitsPerDirIdxTable[1] = 9;
  rvunNumOfBitsPerDirIdxTable[2] = 10;
  rvunNumOfBitsPerDirIdxTable[3] = 0;
  return false;
}

bool TabulatedValuesHoaBitstream::getParSelectionTables(std::map<unsigned int, std::vector<std::vector<unsigned int> > > &rmvvunParSelectionTables, unsigned int unHoaOrder)
{
   rmvvunParSelectionTables.clear();
   switch(unHoaOrder)
   {
   case 1:
      rmvvunParSelectionTables[0] = std::vector<std::vector<unsigned int> > (4, std::vector<unsigned int>(1,0)); 
      rmvvunParSelectionTables[0][0][0] = 0; rmvvunParSelectionTables[0][1][0] = 1; rmvvunParSelectionTables[0][2][0] = 2; rmvvunParSelectionTables[0][3][0] = 3; 

      rmvvunParSelectionTables[1] = std::vector<std::vector<unsigned int> > (4, std::vector<unsigned int>(2,0)); 
      rmvvunParSelectionTables[1][0][0] = 0; rmvvunParSelectionTables[1][0][1] = 2; rmvvunParSelectionTables[1][1][0] = 1; rmvvunParSelectionTables[1][1][1] = 3; 
      rmvvunParSelectionTables[1][2][0] = 2; rmvvunParSelectionTables[1][2][1] = 3; rmvvunParSelectionTables[1][3][0] = 2; rmvvunParSelectionTables[1][3][1] = 3; 

      rmvvunParSelectionTables[2] = std::vector<std::vector<unsigned int> > (4, std::vector<unsigned int>(3,0)); 
      rmvvunParSelectionTables[2][0][0] = 0; rmvvunParSelectionTables[2][0][1] = 1; rmvvunParSelectionTables[2][0][2] = 2; rmvvunParSelectionTables[2][1][0] = 0; 
      rmvvunParSelectionTables[2][1][1] = 1; rmvvunParSelectionTables[2][1][2] = 3; rmvvunParSelectionTables[2][2][0] = 0; rmvvunParSelectionTables[2][2][1] = 2; 
      rmvvunParSelectionTables[2][2][2] = 3; rmvvunParSelectionTables[2][3][0] = 1; rmvvunParSelectionTables[2][3][1] = 2; rmvvunParSelectionTables[2][3][2] = 3; 

      rmvvunParSelectionTables[3] = std::vector<std::vector<unsigned int> > (4, std::vector<unsigned int>(4,0)); 
      rmvvunParSelectionTables[3][0][0] = 0; rmvvunParSelectionTables[3][0][1] = 1; rmvvunParSelectionTables[3][0][2] = 2; rmvvunParSelectionTables[3][0][3] = 3; 
      rmvvunParSelectionTables[3][1][0] = 0; rmvvunParSelectionTables[3][1][1] = 1; rmvvunParSelectionTables[3][1][2] = 2; rmvvunParSelectionTables[3][1][3] = 3; 
      rmvvunParSelectionTables[3][2][0] = 0; rmvvunParSelectionTables[3][2][1] = 1; rmvvunParSelectionTables[3][2][2] = 2; rmvvunParSelectionTables[3][2][3] = 3; 
      rmvvunParSelectionTables[3][3][0] = 0; rmvvunParSelectionTables[3][3][1] = 1; rmvvunParSelectionTables[3][3][2] = 2; rmvvunParSelectionTables[3][3][3] = 3; 
      break;
   case 2:
      rmvvunParSelectionTables[0] = std::vector<std::vector<unsigned int> > (9, std::vector<unsigned int>(1,0)); 
      rmvvunParSelectionTables[0][0][0] = 0; rmvvunParSelectionTables[0][1][0] = 1; rmvvunParSelectionTables[0][2][0] = 2; rmvvunParSelectionTables[0][3][0] = 3; 
      rmvvunParSelectionTables[0][4][0] = 4; rmvvunParSelectionTables[0][5][0] = 5; rmvvunParSelectionTables[0][6][0] = 6; rmvvunParSelectionTables[0][7][0] = 7; 
      rmvvunParSelectionTables[0][8][0] = 8; 
      rmvvunParSelectionTables[1] = std::vector<std::vector<unsigned int> > (9, std::vector<unsigned int>(2,0)); 
      rmvvunParSelectionTables[1][0][0] = 0; rmvvunParSelectionTables[1][0][1] = 2; rmvvunParSelectionTables[1][1][0] = 1; rmvvunParSelectionTables[1][1][1] = 4; 
      rmvvunParSelectionTables[1][2][0] = 2; rmvvunParSelectionTables[1][2][1] = 4; rmvvunParSelectionTables[1][3][0] = 3; rmvvunParSelectionTables[1][3][1] = 5; 
      rmvvunParSelectionTables[1][4][0] = 1; rmvvunParSelectionTables[1][4][1] = 4; rmvvunParSelectionTables[1][5][0] = 3; rmvvunParSelectionTables[1][5][1] = 5; 
      rmvvunParSelectionTables[1][6][0] = 1; rmvvunParSelectionTables[1][6][1] = 6; rmvvunParSelectionTables[1][7][0] = 3; rmvvunParSelectionTables[1][7][1] = 7; 
      rmvvunParSelectionTables[1][8][0] = 1; rmvvunParSelectionTables[1][8][1] = 8; 
      rmvvunParSelectionTables[2] = std::vector<std::vector<unsigned int> > (9, std::vector<unsigned int>(4,0)); 
      rmvvunParSelectionTables[2][0][0] = 0; rmvvunParSelectionTables[2][0][1] = 2; rmvvunParSelectionTables[2][0][2] = 3; rmvvunParSelectionTables[2][0][3] = 8; 
      rmvvunParSelectionTables[2][1][0] = 1; rmvvunParSelectionTables[2][1][1] = 4; rmvvunParSelectionTables[2][1][2] = 7; rmvvunParSelectionTables[2][1][3] = 8; 
      rmvvunParSelectionTables[2][2][0] = 0; rmvvunParSelectionTables[2][2][1] = 2; rmvvunParSelectionTables[2][2][2] = 4; rmvvunParSelectionTables[2][2][3] = 5; 
      rmvvunParSelectionTables[2][3][0] = 0; rmvvunParSelectionTables[2][3][1] = 3; rmvvunParSelectionTables[2][3][2] = 5; rmvvunParSelectionTables[2][3][3] = 7; 
      rmvvunParSelectionTables[2][4][0] = 1; rmvvunParSelectionTables[2][4][1] = 2; rmvvunParSelectionTables[2][4][2] = 4; rmvvunParSelectionTables[2][4][3] = 5; 
      rmvvunParSelectionTables[2][5][0] = 2; rmvvunParSelectionTables[2][5][1] = 3; rmvvunParSelectionTables[2][5][2] = 4; rmvvunParSelectionTables[2][5][3] = 5; 
      rmvvunParSelectionTables[2][6][0] = 1; rmvvunParSelectionTables[2][6][1] = 3; rmvvunParSelectionTables[2][6][2] = 4; rmvvunParSelectionTables[2][6][3] = 6; 
      rmvvunParSelectionTables[2][7][0] = 1; rmvvunParSelectionTables[2][7][1] = 3; rmvvunParSelectionTables[2][7][2] = 7; rmvvunParSelectionTables[2][7][3] = 8; 
      rmvvunParSelectionTables[2][8][0] = 0; rmvvunParSelectionTables[2][8][1] = 1; rmvvunParSelectionTables[2][8][2] = 2; rmvvunParSelectionTables[2][8][3] = 8; 

      rmvvunParSelectionTables[3] = std::vector<std::vector<unsigned int> > (9, std::vector<unsigned int>(9,0)); 
      rmvvunParSelectionTables[3][0][0] = 0; rmvvunParSelectionTables[3][0][1] = 1; rmvvunParSelectionTables[3][0][2] = 2; rmvvunParSelectionTables[3][0][3] = 3; 
      rmvvunParSelectionTables[3][0][4] = 4; rmvvunParSelectionTables[3][0][5] = 5; rmvvunParSelectionTables[3][0][6] = 6; rmvvunParSelectionTables[3][0][7] = 7; 
      rmvvunParSelectionTables[3][0][8] = 8; rmvvunParSelectionTables[3][1][0] = 0; rmvvunParSelectionTables[3][1][1] = 1; rmvvunParSelectionTables[3][1][2] = 2; 
      rmvvunParSelectionTables[3][1][3] = 3; rmvvunParSelectionTables[3][1][4] = 4; rmvvunParSelectionTables[3][1][5] = 5; rmvvunParSelectionTables[3][1][6] = 6; 
      rmvvunParSelectionTables[3][1][7] = 7; rmvvunParSelectionTables[3][1][8] = 8; rmvvunParSelectionTables[3][2][0] = 0; rmvvunParSelectionTables[3][2][1] = 1; 
      rmvvunParSelectionTables[3][2][2] = 2; rmvvunParSelectionTables[3][2][3] = 3; rmvvunParSelectionTables[3][2][4] = 4; rmvvunParSelectionTables[3][2][5] = 5; 
      rmvvunParSelectionTables[3][2][6] = 6; rmvvunParSelectionTables[3][2][7] = 7; rmvvunParSelectionTables[3][2][8] = 8; rmvvunParSelectionTables[3][3][0] = 0; 
      rmvvunParSelectionTables[3][3][1] = 1; rmvvunParSelectionTables[3][3][2] = 2; rmvvunParSelectionTables[3][3][3] = 3; rmvvunParSelectionTables[3][3][4] = 4; 
      rmvvunParSelectionTables[3][3][5] = 5; rmvvunParSelectionTables[3][3][6] = 6; rmvvunParSelectionTables[3][3][7] = 7; rmvvunParSelectionTables[3][3][8] = 8; 
      rmvvunParSelectionTables[3][4][0] = 0; rmvvunParSelectionTables[3][4][1] = 1; rmvvunParSelectionTables[3][4][2] = 2; rmvvunParSelectionTables[3][4][3] = 3; 
      rmvvunParSelectionTables[3][4][4] = 4; rmvvunParSelectionTables[3][4][5] = 5; rmvvunParSelectionTables[3][4][6] = 6; rmvvunParSelectionTables[3][4][7] = 7; 
      rmvvunParSelectionTables[3][4][8] = 8; rmvvunParSelectionTables[3][5][0] = 0; rmvvunParSelectionTables[3][5][1] = 1; rmvvunParSelectionTables[3][5][2] = 2; 
      rmvvunParSelectionTables[3][5][3] = 3; rmvvunParSelectionTables[3][5][4] = 4; rmvvunParSelectionTables[3][5][5] = 5; rmvvunParSelectionTables[3][5][6] = 6; 
      rmvvunParSelectionTables[3][5][7] = 7; rmvvunParSelectionTables[3][5][8] = 8; rmvvunParSelectionTables[3][6][0] = 0; rmvvunParSelectionTables[3][6][1] = 1; 
      rmvvunParSelectionTables[3][6][2] = 2; rmvvunParSelectionTables[3][6][3] = 3; rmvvunParSelectionTables[3][6][4] = 4; rmvvunParSelectionTables[3][6][5] = 5; 
      rmvvunParSelectionTables[3][6][6] = 6; rmvvunParSelectionTables[3][6][7] = 7; rmvvunParSelectionTables[3][6][8] = 8; rmvvunParSelectionTables[3][7][0] = 0; 
      rmvvunParSelectionTables[3][7][1] = 1; rmvvunParSelectionTables[3][7][2] = 2; rmvvunParSelectionTables[3][7][3] = 3; rmvvunParSelectionTables[3][7][4] = 4; 
      rmvvunParSelectionTables[3][7][5] = 5; rmvvunParSelectionTables[3][7][6] = 6; rmvvunParSelectionTables[3][7][7] = 7; rmvvunParSelectionTables[3][7][8] = 8; 
      rmvvunParSelectionTables[3][8][0] = 0; rmvvunParSelectionTables[3][8][1] = 1; rmvvunParSelectionTables[3][8][2] = 2; rmvvunParSelectionTables[3][8][3] = 3; 
      rmvvunParSelectionTables[3][8][4] = 4; rmvvunParSelectionTables[3][8][5] = 5; rmvvunParSelectionTables[3][8][6] = 6; rmvvunParSelectionTables[3][8][7] = 7; 
      rmvvunParSelectionTables[3][8][8] = 8; 
      break;
   }
   return false;
}