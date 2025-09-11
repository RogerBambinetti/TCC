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
 $Id: DirBasedPreDomSoundSynthesis.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#pragma once

#include "DataTypes.h"
#include "TabulatedValues.h"
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

class DirBasedPreDomSoundSynthesis
{
	public:

			DirBasedPreDomSoundSynthesis();

			bool init(	unsigned int p_uiNoOfChannelsWithVariableType, 
						unsigned int p_uiHOAOrder, 
						unsigned int p_uiFrameSize, 
						unsigned int p_uiMinNoOfCoeffsForAmbHOA,
						unsigned int p_uiMaxNofOfDirSigsForPred,
						unsigned int p_uiNoOfBitsPerScaleFactor,
						std::shared_ptr<TabulatedValues> p_shptrTabVals);

			bool process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame, 
								  std::vector<std::vector<FLOAT> > &p_rvvFOutputDirBasedPreDomSoundsHOAFrame,
								  std::vector<std::vector<FLOAT> > &p_rvvFOutputSmoothedDirSigsHOAFrame,
							const std::map<unsigned int, unsigned int> &p_rmuiActiveDirAndGridDirIndices,
							const std::vector<unsigned int> &p_rvuiPredictionTypeVec,
							const std::vector<std::vector<unsigned int> > &p_rvvuiPredictionIndicesMat,
							const std::vector<std::vector<int> > &p_rvviQuantizedPredictionFactorsMat,
							const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
							const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
							const std::set<unsigned int> &p_suiNonEnDisAbleActHOACoeffIndices,
							const std::set<unsigned int> &p_rsuiSetOfPSSigIndices,
							const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
							const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
							const std::vector<FLOAT> &p_rvFFadeOutWinForVecSigs);

            bool computeHOAReprOfDirSigs(const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
                                     const std::map<unsigned int, unsigned int> &p_rmuiActiveDirAndGridDirIndices,
                                     const std::set<unsigned int> &p_rsuiSetOfPSSigIndices,
									 const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
									 const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
									 const std::vector<FLOAT> &p_rvFFadeOutWinForVecSigs,
                                           std::vector<std::vector<FLOAT> > &p_rvvFOutputSmoothedDirSigsHOAFrame);

            bool computeSmoothedHOARepresentationOfSpatPredSigs(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
																	const std::vector<unsigned int> &p_rvuiPredictionTypeVec,
									                                const std::vector<std::vector<unsigned int> > &p_rvvuiPredictionIndicesMat,
									                                const std::vector<std::vector<int> > &p_rvviQuantizedPredictionFactorsMat,
																	const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
									                                const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
																	const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
									                                const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
									                                const std::set<unsigned int> &p_suiNonEnDisAbleActHOACoeffIndices,
                                                                    std::vector<std::vector<FLOAT> > & p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame);

	private:

		bool m_bIsInit;

		unsigned int m_uiNoOfChannelsWithVariableType;
		unsigned int m_uiHOAOrder;
		unsigned int m_uiFrameSize;
		unsigned int m_uiNoOfHOACoeffs;
		unsigned int m_uiMinNoOfCoeffsForAmbHOA;
		unsigned int m_uiMaxNofOfDirSigsForPred;
		unsigned int m_uiNoOfBitsPerScaleFactor;
		
		unsigned int m_uiFilterDelay;

		std::vector<std::vector<FLOAT> > m_vvFModeMatAllCoarseGridPoints;
		std::vector<std::vector<FLOAT> > m_vvFTransposeModeMatAllFineGridPoints;

		std::vector<FLOAT> m_vFOnesLongVec;

		// buffers
		std::vector<std::vector<FLOAT> > m_vvFPredictedGridDirSigsLastHOAFrame;
		std::vector<std::vector<FLOAT> > m_vvFPredictedGridDirSigsCurrentHOAFrame;
		std::vector<std::vector<FLOAT> > m_vvFPredictedGridDirSigsSmoothedHOAFrame;

		// delay buffers
		std::map<unsigned int, unsigned int> m_muiLastActiveDirAndGridDirIncides;
		std::set<unsigned int> m_suiSetOfOldPSSigIndices;

		std::vector<unsigned int>               m_vuiLastPredictionTypeVec;
		std::vector<std::vector<unsigned int> > m_vvuiLastPredictionIndicesMat;
		std::vector<std::vector<FLOAT> >        m_vvFLastPredictionFactorsMat;
		std::vector<std::vector<FLOAT> >        m_vvFPredictionFactorsMat;

		//member functions
		bool checkBufferSizes(const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame);

		static bool computeHOARepresentationOfSpatPred(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
														const std::vector<std::vector<FLOAT> > &p_rvvFModeMatAllCoarseGridPoints,
														const std::vector<unsigned int> &p_rvuiPredictionTypeVec,
														const std::vector<std::vector<unsigned int> > &p_rvvuiPredictionIndicesMat,
														const std::vector<std::vector<FLOAT> > &p_rvvFPredictionFactorsMat,
														const std::vector<FLOAT> &p_rvFFadeWin,
														std::vector<std::vector<FLOAT> > &p_rvvFPredictedGridDirSigsHOAFrame,
														bool &p_bSpatialPredictionInCurrentFrame );
};
