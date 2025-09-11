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
 $Id: PreDomSoundSynthesis.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#include "PreDomSoundSynthesis.h"

// constructor 
PreDomSoundSynthesis::PreDomSoundSynthesis( )
{
		m_bIsInit = false;
}

// reset member function
bool PreDomSoundSynthesis::init(unsigned int p_uiNoOfChannelsWithVariableType, 
								unsigned int p_uiHOAOrder, 
								unsigned int p_uiFrameSize, 
								unsigned int p_uiMinNoOfCoeffsForAmbHOA,
								unsigned int p_uiMaxNofOfDirSigsForPred,
								unsigned int p_uiNoOfBitsPerScaleFactor,
								unsigned int p_uiVVecStartCh,
								unsigned int p_uiCodedVVecLength,
								unsigned int p_uiInterpSamples,
								unsigned int p_uiInterpMethod,
								std::shared_ptr<TabulatedValues> p_shptrTabVals)
{

	m_uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
	m_uiFrameSize     = p_uiFrameSize;

	//init cross fade windows
	
		// compute fading window for directional signals 
		std::vector<FLOAT> vFCrossFadeWinForDirSigs;
		computeFadeWinForDirBasedSynth(vFCrossFadeWinForDirSigs, p_uiFrameSize);

		// compute fading window for vector based signals
		std::vector<FLOAT> vFCrossFadeWinForVecSigs;
		computeFadeWinForVecBasedSynth(vFCrossFadeWinForVecSigs, p_uiFrameSize, p_uiInterpSamples, p_uiInterpMethod);

		// set fade in and out windows
		m_vFFadeInWinForDirSigs.assign( vFCrossFadeWinForDirSigs.begin(), vFCrossFadeWinForDirSigs.begin() + m_uiFrameSize);
		m_vFFadeOutWinForDirSigs.assign( vFCrossFadeWinForDirSigs.begin() + m_uiFrameSize, vFCrossFadeWinForDirSigs.end());
		m_vFFadeInWinForVecSigs.assign( vFCrossFadeWinForVecSigs.begin() , vFCrossFadeWinForVecSigs.begin() + m_uiFrameSize );
		m_vFFadeOutWinForVecSigs.assign( vFCrossFadeWinForVecSigs.begin() + m_uiFrameSize, vFCrossFadeWinForVecSigs.end() );

	// init direction based predominant sound synthesis
	m_shptrDirBasedPreDomSoundSynth = std::shared_ptr<DirBasedPreDomSoundSynthesis>( new DirBasedPreDomSoundSynthesis() ); 
	m_shptrDirBasedPreDomSoundSynth->init(	p_uiNoOfChannelsWithVariableType, 
											p_uiHOAOrder,
											p_uiFrameSize,
											p_uiMinNoOfCoeffsForAmbHOA,
											p_uiMaxNofOfDirSigsForPred,
											p_uiNoOfBitsPerScaleFactor,
											p_shptrTabVals);

	// init vector based predominant sound synthesis
	m_shptrVecBasedPreDomSoundSynth = std::shared_ptr<VectorBasedPredomSoundSynthesis> ( new VectorBasedPredomSoundSynthesis() );
	m_shptrVecBasedPreDomSoundSynth->init( p_uiNoOfChannelsWithVariableType,
										   p_uiHOAOrder,
										   p_uiFrameSize,
										   p_uiMinNoOfCoeffsForAmbHOA,
										   p_uiVVecStartCh,
										   p_uiCodedVVecLength,
										   p_uiInterpSamples);

	// init buffers
	std::vector<FLOAT> vFTmpSingleChannelBuffer ( p_uiFrameSize, static_cast<FLOAT>(0.0));

	m_vvFDirBasedPreDomSoundsHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	m_bIsInit = true;

	return false;
}


bool PreDomSoundSynthesis::process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame, 
										  std::vector<std::vector<FLOAT> > &p_rvvFOutputPreDomSoundsHOAFrame,
                                std::vector<std::vector<FLOAT> > &p_rvvFOutputFadedForPARPreDomSoundsHOAFrame,
										  std::vector<std::vector<FLOAT> > &p_rvvFOutputDirSigsHOAFrame,
										  std::vector<std::vector<FLOAT> > &p_rvvFOutputVecBasedPreDomSoundsHOAFrame,
									const std::map<unsigned int, unsigned int> &p_muiActiveDirAndGridDirIndices,
									const std::vector<unsigned int> &p_vuiPredictionTypeVec,
									const std::vector<std::vector<unsigned int> > &p_vvuiPredictionIndicesMat,
									const std::vector<std::vector<int> > &p_vviQuantizedPredictionFactorsMat,
									const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
									const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
									const std::set<unsigned int> &p_suiNonEnDisAbleActHOACoeffIndices,
									const std::map<unsigned int, std::vector<FLOAT> > &p_rmuivFVectors)
{

	// create set of PS signal indices
	std::set<unsigned int> suiSetOfPSSigIndices;

	for ( std::map<unsigned int, unsigned int>::const_iterator muicIt = p_muiActiveDirAndGridDirIndices.begin(); muicIt != p_muiActiveDirAndGridDirIndices.end(); muicIt++)
	{
		suiSetOfPSSigIndices.insert( muicIt->first);
	}

	for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muiFcIt = p_rmuivFVectors.begin(); muiFcIt!=p_rmuivFVectors.end(); muiFcIt++)
	{
		suiSetOfPSSigIndices.insert( muiFcIt->first);
	}



	// process direction based predominant sound synthesis
	if ( m_shptrDirBasedPreDomSoundSynth->process(	p_rvvFInputPSSigsFrame, 
													m_vvFDirBasedPreDomSoundsHOAFrameBuffer,
													p_rvvFOutputDirSigsHOAFrame,
													p_muiActiveDirAndGridDirIndices,
													p_vuiPredictionTypeVec,
													p_vvuiPredictionIndicesMat,
													p_vviQuantizedPredictionFactorsMat,
													p_suiAmbCoeffIndicesToBeEnabled,
													p_suiAmbCoeffIndicesToBeDisabled,
													p_suiNonEnDisAbleActHOACoeffIndices,
													suiSetOfPSSigIndices,
													m_vFFadeInWinForDirSigs,
													m_vFFadeOutWinForDirSigs,
													m_vFFadeOutWinForVecSigs) )
	{
		return true;
	}

	// process vector based predominant sound synthesis
	if ( m_shptrVecBasedPreDomSoundSynth->process(	p_rvvFInputPSSigsFrame,
													p_rvvFOutputVecBasedPreDomSoundsHOAFrame,
													p_rmuivFVectors,
													suiSetOfPSSigIndices,
													p_suiAmbCoeffIndicesToBeEnabled,
													p_suiAmbCoeffIndicesToBeDisabled,
													m_vFFadeInWinForDirSigs,
													m_vFFadeOutWinForDirSigs,
													m_vFFadeInWinForVecSigs,
													m_vFFadeOutWinForVecSigs) )
	{
		return true;
	}

	// add direction based and vector based components
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			p_rvvFOutputPreDomSoundsHOAFrame[uiCoeffIdx][uiSampleIdx] =   m_vvFDirBasedPreDomSoundsHOAFrameBuffer[uiCoeffIdx][uiSampleIdx] 
																		+ p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdx][uiSampleIdx];
		}
	}

    p_rvvFOutputFadedForPARPreDomSoundsHOAFrame.resize(m_uiNoOfHOACoeffs);

    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
        p_rvvFOutputFadedForPARPreDomSoundsHOAFrame[uiCoeffIdx].assign( p_rvvFOutputPreDomSoundsHOAFrame[uiCoeffIdx].begin(),
                                                                        p_rvvFOutputPreDomSoundsHOAFrame[uiCoeffIdx].end());
    }

    for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeEnabled.begin(); suiIt !=p_suiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
    {
        unsigned int uiCurrFadeInHOAIdx = *suiIt;

        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
            p_rvvFOutputFadedForPARPreDomSoundsHOAFrame[uiCurrFadeInHOAIdx-1][uiSampleIdx] *= m_vFFadeInWinForDirSigs[uiSampleIdx];
        }
    }

    for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeDisabled.begin(); suiIt !=p_suiAmbCoeffIndicesToBeDisabled.end(); suiIt++)
    {
        unsigned int uiCurrFadeOutHOAIdx = *suiIt;

        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
            p_rvvFOutputFadedForPARPreDomSoundsHOAFrame[uiCurrFadeOutHOAIdx-1][uiSampleIdx] *= m_vFFadeOutWinForDirSigs[uiSampleIdx];
        }
    }
	
	return false;
}



