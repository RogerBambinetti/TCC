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
 $Id: PreDomSoundSynthesis.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#pragma once

#include "DirBasedPreDomSoundSynthesis.h"
#include "VectorBasedPredomSoundSynthesis.h"
#include "CommonFunctions.h"

class PreDomSoundSynthesis
{
	public:

			PreDomSoundSynthesis();

			bool init(	unsigned int p_uiNoOfChannelsWithVariableType, 
						unsigned int p_uiHOAOrder, 
						unsigned int p_uiFrameSize, 
						unsigned int p_uiMinNoOfCoeffsForAmbHOA,
						unsigned int p_uiMaxNofOfDirSigsForPred,
						unsigned int p_uiNoOfBitsPerScaleFactor,
						unsigned int p_uiVVecStartCh,
						unsigned int p_uiCodedVVecLength,
						unsigned int p_uiInterpSamples,
						unsigned int p_uiInterpMethod,
						std::shared_ptr<TabulatedValues> p_shptrTabVals);

			bool process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame, 
									std::vector<std::vector<FLOAT> > &p_rvvFOutputPreDomSoundsHOAFrame,
                           std::vector<std::vector<FLOAT> > &p_rvvFOutputFadedForPARPreDomSoundsHOAFrame,
									std::vector<std::vector<FLOAT> > &p_rvvFOutputDirSigsHOAFrame,
									std::vector<std::vector<FLOAT> > &p_rvvFVecBasedPreDomSoundsHOAFrame,
							const std::map<unsigned int, unsigned int> &p_muiActiveDirAndGridDirIndices,
							const std::vector<unsigned int> &p_vuiPredictionTypeVec,
							const std::vector<std::vector<unsigned int> > &p_vvuiPredictionIndicesMat,
							const std::vector<std::vector<int> > &p_vviQuantizedPredictionFactorsMat,
							const std::set<unsigned int> &p_suiOldAmbCoeffIndicesToBeEnabled,
							const std::set<unsigned int> &p_suiOldAmbCoeffIndicesToBeDisabled,
							const std::set<unsigned int> &p_suiOldNonEnDisAbleActHOACoeffIndices,
							const std::map<unsigned int, std::vector<FLOAT> > &p_rmuivFVectors);

	private:

		bool m_bIsInit;

		unsigned int m_uiNoOfHOACoeffs;
		unsigned int m_uiFrameSize;

		std::vector<FLOAT> m_vFFadeInWinForDirSigs;
		std::vector<FLOAT> m_vFFadeOutWinForDirSigs;
		std::vector<FLOAT> m_vFFadeInWinForVecSigs;
		std::vector<FLOAT> m_vFFadeOutWinForVecSigs;

		// shared pointers to components
		std::shared_ptr<DirBasedPreDomSoundSynthesis> m_shptrDirBasedPreDomSoundSynth;
		std::shared_ptr<VectorBasedPredomSoundSynthesis> m_shptrVecBasedPreDomSoundSynth;


		// buffers
		std::vector<std::vector<FLOAT> > m_vvFDirBasedPreDomSoundsHOAFrameBuffer;

};
