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
 $Rev: 203 $
 $Author: technicolor-ks $
 $Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
 $Id: TabulatedValues.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/


#pragma once

#include <vector>
#include <iostream>

#include <fstream>
#include "DataTypes.h"
#include "sphericalHarmonic.h"

const double OWN_PI = static_cast<double>(3.14159265358979323846);

class TabulatedValues
{
	public:

		TabulatedValues();

		bool getModeMat(std::vector<std::vector<FLOAT> > & p_rvvFModeMat, int p_iOrder);
		bool getInvModeMat(std::vector<std::vector<FLOAT> > & p_rvvFInvModeMat, int p_iOrder);
		bool getPInvHighCoeffsModeMat(std::vector<std::vector<FLOAT> > & p_rvvFInvModeMat, unsigned int p_uiOrder);

		bool getTransposeModeMatForFineGrid(std::vector<std::vector<FLOAT> > & p_rvvFTransposeModeMatForFineGrid, 
                                                unsigned int p_uiOrder, 
                                                unsigned int uiDirGridTableIdx);

      bool getDirections( std::vector< std::vector<FLOAT> > & p_rvvDirections_ria,
                          unsigned int   p_uiOrder);    

      bool getSubBandConfiguration(  const std::vector<unsigned int> m_vunParSubbandWidths,
                                     std::vector< std::vector< unsigned int> > &p_rvvuiQMFSubBandIndices);
      // functions for PAR
      bool getPARPermIdxVectorTable(  std::vector<std::vector<unsigned int> > & p_rvvuiPARPermIdxVectorTable,
                                      std::vector<std::vector<unsigned int> > & p_rvvuiPARInversePermIdxVectorTable,
                                      unsigned int unHoaOrder);

		static bool readDoubleMatFromFile(const std::string & p_rsMatName, std::vector<std::vector<double> > & p_rvvdMat );
		static bool writeDoubleMatToUnsignedCharTxtFile(const std::string & p_rsMatName, const std::vector<std::vector<double> > & p_rvvdMat );

		// functions for for VQ coding of V-vector data, Qualcomm
		bool getWeightingCdbk256x8(std::vector<std::vector<FLOAT> > & p_weightingCdbk256x8);
		bool getVVecVqMat(std::vector<std::vector<FLOAT> > & p_rvvFModeMat, unsigned int p_uiOrder);
		bool getDictCicpSpeakerPoints(std::vector<std::vector<FLOAT> > & p_rvvFModeMat, unsigned int p_uiOrder);
		bool getDict2DPoints(std::vector<std::vector<FLOAT> > & p_rvvFModeMat, unsigned int p_uiOrder);

		bool getInvDecorrCoef(std::vector<FLOAT> & p_invDecorrCoef);
		bool getDecorrCoef(std::vector<FLOAT> & p_DecorrCoef);

	private:

        static unsigned char ucAzimuths[9][8*100];
        static unsigned char ucInclinations[9][8*100];

        static unsigned char ucSearchGridAzimuths[3][8*900];
        static unsigned char ucSearchGridInclinations[3][8*900];

		static unsigned char ucCodedInvModeMatElems[7][19208];
		static unsigned char ucCodedPInvHighCoeffsModeMatElems[5][17640];

		static double m_cdN3DCorrectionFactor;
		static double m_cdN3DCorrectionInverseFactor;

        // for PAR
        static unsigned char ucParPermIdxVectorTableOrderTwo[4][9];
        static unsigned char ucParPermIdxVectorTableOrderOne[4][4];

		// tables for VQ coding of V-vector data, Qualcomm
		static unsigned char ucInvDecorrCoef[56];
		static unsigned char ucDecorrCoef[72];

		static unsigned char ucWeightingCdbk256x8[8192];
		static unsigned char ucCodedVVecVqMatElems[7][19208];	
		static unsigned int uiCicpSpeakerInclinations[51];
		static unsigned int uiCicpSpeakerdAzimuths[51];
		

};
