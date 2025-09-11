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
 $Rev: 179 $
 $Author: technicolor-kf $
 $Date: 2015-05-07 15:08:57 +0200 (Do, 07 Mai 2015) $
 $Id: AmbientComponentModification.h 179 2015-05-07 13:08:57Z technicolor-kf $
*/


#pragma once


#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>
#include <memory>
#include "TabulatedValues.h"
#include "AssignmentInfo.h"
#include "CommonFunctions.h"

#include <fstream>
#include <string>
#include <sstream>

class AmbientComponentModification
{
	public:

		AmbientComponentModification();

		bool init(	unsigned int p_uiFrameSize, 
					unsigned int p_uiHOAOrder, 
					unsigned int p_uiMinOrderForAmbHOA, 
					unsigned int p_uiTotalNoOfPercCoders, 
					unsigned int p_uiNoOfChannelsWithVariableType,
					unsigned int p_uiBitRatePerPercCoder,
					std::shared_ptr<TabulatedValues> p_shptrTabVals);

		bool process(	const std::vector<std::vector<FLOAT>> &p_rvvFInputAmbientHOAFrame,
						const std::vector<std::vector<FLOAT>> &p_rvvFInputPredictedNextAmbientHOAFrame,
						const std::vector<AssignmentInfo> &p_vAITargetAssignmentVector,
							  std::vector<std::vector<FLOAT>> &p_rvvFOutputModifiedAmbientHOAFrame,
							  std::vector<std::vector<FLOAT>> &p_rvvFOutputModifiedPredictedNextAmbientHOAFrame,
							  std::vector<AssignmentInfo> &p_vAIOutputFinalAssignmentVector,
                              std::set<unsigned int> &p_rsuiActiveAmbHOACoeffsIndices,
                              std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
                              std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
                              std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices);

	private:

		bool m_bIsInit;

		unsigned int m_uiFrameSize;
		unsigned int m_uiHOAOrder;
		unsigned int m_uiNoOfHOACoeffs;
		unsigned int m_uiMinOrderForAmbHOA;
		unsigned int m_uiMinNoOfCoeffsForAmbHOA;
		unsigned int m_uiTotalNoOfPercCoders;
		unsigned int m_uiNoOfChannelsWithVariableType;
		unsigned int m_uiBitRatePerPercCoder;

		std::set<unsigned int> m_suiActiveAmbHOACoeffsIndices;
		std::set<unsigned int> m_suiPotentialAmbHOACoeffsIndices;

		std::set<unsigned int> m_suiAmbHOACoeffsIndicesToBeEnabled;
		std::set<unsigned int> m_suiAmbHOACoeffsIndicesToBeDisabled;

		std::vector<std::vector<FLOAT>> m_vvFPInvLowOrderModeMat;

		std::vector<FLOAT> m_vFSmoothWinFunc;

		std::vector<AssignmentInfo> m_vAIPreviousTargetAssignmentVector;
		std::vector<AssignmentInfo> m_vAIAssignmentVector;

		static void computeSmoothWinFunc(std::vector<FLOAT> & p_rvFOutputSmoothWinFunc, unsigned int p_uiFrameSize);
};
