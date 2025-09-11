/***********************************************************************************
 
 This software module was originally developed by 
 
 Qualcomm Technologies, Inc. (QTI)
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 QTI retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2015.
 
 ***********************************************************************************/
#pragma once

#include "DataTypes.h"
#include "TabulatedValues.h"
#include <vector>
#include <map>
#include <set>

class VectorBasedPredomSoundSynthesis
{
	public:

			VectorBasedPredomSoundSynthesis();

			bool init(unsigned int p_uiNoOfChannelsWithVariableType, 
						 unsigned int p_uiHOAOrder, 
						 unsigned int p_uiFrameSize,
						 unsigned int p_uiMinNoOfCoeffsForAmbHOA,
						 unsigned int p_uiVVecStartCh,
						 unsigned int p_uiCodedVVecLength,
						 unsigned int p_uiInterpSamples);

         bool process(	const std::vector<std::vector<FLOAT>> &p_rvvFInputDirSigsFrame, 
							  std::vector<std::vector<FLOAT>> &p_rvvFOutputVecBasedPreDomSoundsHOAFrame,
                        const std::map<unsigned int, std::vector<FLOAT>> &p_rmuivFVectors,
						const std::set<unsigned int> &p_rsuiSetOfPSSigIndices,
						const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
						const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
						const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
						const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
						const std::vector<FLOAT> &p_rvFFadeInWinForVecSigs,
						const std::vector<FLOAT> &p_rvFFadeOutWinForVecSigs);

	private:

      // basic params
		bool m_bIsInit;

		unsigned int m_uiMaxNoOfDirSigs;
		unsigned int m_uiHOAOrder;
		unsigned int m_uiFrameSize;
		unsigned int m_uiNoOfHOACoeffs;
        unsigned int m_uiNoOfAmbiChs;
	    unsigned int m_uiVVecStartCh;
		unsigned int m_uiCodedVVecLength;
		unsigned int m_uiInterpSamples;

	  //ones vector
	  std::vector<FLOAT> m_vFOnesLongVec;

	  std::map<unsigned int, std::vector<FLOAT> > m_muivFPreviousVectors;

	  std::set<unsigned int> m_suiSetOfPreviousPSSigIndices;

		// member functions
	  bool checkBufferSizes(const std::vector<std::vector<FLOAT> > &p_rvvFInputVecSigsFrame);
};
