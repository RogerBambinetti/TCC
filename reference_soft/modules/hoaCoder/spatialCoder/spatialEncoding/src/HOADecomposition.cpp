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
 $Id: HOADecomposition.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/


#include "HOADecomposition.h"
#include "stdHoaFrame.h"

HOADecomposition::HOADecomposition()
{
	m_bIsInit = false;
}

bool HOADecomposition::init(unsigned int p_uiHOAOrder,
							unsigned int p_uiFrameSize,
							unsigned int p_uiMaxNoOfDirSigs,
							unsigned int p_uiMinNoOfCoeffsForAmbHOA,
							unsigned int p_uiMaxNofOfDirSigsForPred,
							unsigned int p_uiNoOfBitsPerScaleFactor,
							unsigned int p_uiNoOfChannelsWithVariableType,
							unsigned int p_uiBitRatePerPercCoder,
							unsigned int p_uiNoBitsForVecElemQuant,
							unsigned int p_uiCodedVVecLength,
							unsigned int p_uiInterpSamples,
							unsigned int p_uiInterpMethod,
							std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
	m_uiHOAOrder  = p_uiHOAOrder;
	m_uiFrameSize = p_uiFrameSize; 
	m_uiMaxNoOfDirSigs = p_uiMaxNoOfDirSigs;
	m_uiMinNoOfCoeffsForAmbHOA = p_uiMinNoOfCoeffsForAmbHOA;
	m_uiMaxNofOfDirSigsForPred = p_uiMaxNofOfDirSigsForPred;
	m_uiNoOfBitsPerScaleFactor = p_uiNoOfBitsPerScaleFactor;
	m_uiNoOfChannelsWithVariableType = p_uiNoOfChannelsWithVariableType;
	m_uiBitRatePerPercCoder          = p_uiBitRatePerPercCoder;
	m_uiNoBitsForVecElemQuant		= p_uiNoBitsForVecElemQuant;

	m_uiNOOFHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);

	switch (p_uiCodedVVecLength){
	case 0:
		m_fMinVVecFactor = 1.0;
		m_uiVVecStartCh = 0;
		break;
	case 1:
		m_fMinVVecFactor = 0.0;
		m_uiVVecStartCh = m_uiMinNoOfCoeffsForAmbHOA;
		break;
	case 2:
		m_fMinVVecFactor = 1.0;
		m_uiVVecStartCh = m_uiMinNoOfCoeffsForAmbHOA;
		break;
	}


	if ( p_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, 2) )
	{
			return true;
	}

	// compute fading window for directional signals
	if ( computeFadeWinForDirBasedSynth(m_vFCrossFadeWinForDirSigs, p_uiFrameSize) )
	{
		return true;
	}

	// compute fading window for vector based signals
	if ( computeFadeWinForVecBasedSynth(m_vFCrossFadeWinForVecSigs, p_uiFrameSize, p_uiInterpSamples, p_uiInterpMethod) )
	{
		return true;
	}

	if ( p_shptrTabVals->getModeMat(m_vvFModeMat, m_uiHOAOrder) ) 
	{
		return true;
	}

	// init buffers

	std::vector<FLOAT> vFTmpVec(m_uiFrameSize, static_cast<FLOAT>( 0.0));

	m_vvFLastHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );
	m_vvFPenultimateHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );

	m_vvFSmoothedDirSigsLastHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );
	m_vvFSmoothedDirSigsHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );

	m_vvSmoothedVecSigsLastHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );
	m_vvSmoothedVecSigsHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );

   mvvFFadedForPARPreDomSoundsHOAFrame.assign( m_uiNOOFHOACoeffs, vFTmpVec );

	std::vector<FLOAT> vFTmp2Vec(2*m_uiFrameSize, static_cast<FLOAT>( 0.0));
	m_vvFSmoothedPSSigsLastFrame.assign(m_uiNoOfChannelsWithVariableType, vFTmp2Vec);

	m_muiLastDirSigAndGridDirIndices.clear();
	m_muivFLastVectors.clear();

	// init predominant sound synthesis
	m_shptrPreDomSoundSynth = std::shared_ptr<PreDomSoundSynthesis>( new PreDomSoundSynthesis());
	m_shptrPreDomSoundSynth->init(	m_uiNoOfChannelsWithVariableType,
									m_uiHOAOrder,
									m_uiFrameSize,
									m_uiMinNoOfCoeffsForAmbHOA,
									m_uiMaxNofOfDirSigsForPred,
									m_uiNoOfBitsPerScaleFactor,
									m_uiVVecStartCh,
									p_uiCodedVVecLength,
									p_uiInterpSamples,
									p_uiInterpMethod,
									p_shptrTabVals);

	m_bIsInit = true;

	return false;
}

// process function
bool HOADecomposition::process( const std::vector<std::vector<FLOAT>> &p_rvvFInputHOAFrame,
							    const std::map<unsigned int, unsigned int> & p_rmuiInputDirSigAndGridDirIndices,
								const std::map<unsigned int, std::vector<FLOAT> > &p_rmuivFInputVectors,
									 std::vector<std::vector<FLOAT>> &p_rvvFOutputSmoothedPSSigsLastFrame,
									 std::vector<std::vector<FLOAT>> &p_rvvFOutputSmoothedPSSigsPenultimateFrame,
									 std::vector<std::vector<FLOAT>> &p_rvvFOutputPenultimateSmoothedPreDomSoundsHOAFrame,
									 std::vector<std::vector<FLOAT>> &p_rvvFOutputPenultimateAmbientHOAFrame,
									 std::vector<std::vector<FLOAT>> &p_rvvFOutputPredictedLastAmbientHOAFrame,
									 std::vector<unsigned int> &p_vuiOutputPredictionTypeVec,
									 std::vector<std::vector<unsigned int>> &p_vvuiOutputPredictionIndicesMat,
									 std::vector<std::vector<int>> &p_vviOutputQuantizedPredictionFactorsMat,
									 std::vector<AssignmentInfo> &p_vAITargetAssignmentVector)	   
{
	if (!m_bIsInit)
	{
		return true;
	}

	// set penultimate frame of directional smoothed signals
	p_rvvFOutputSmoothedPSSigsPenultimateFrame.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiDirIdx=0; uiDirIdx < m_uiNoOfChannelsWithVariableType; uiDirIdx++)
	{
		p_rvvFOutputSmoothedPSSigsPenultimateFrame[uiDirIdx].assign( m_vvFSmoothedPSSigsLastFrame[uiDirIdx].begin(), m_vvFSmoothedPSSigsLastFrame[uiDirIdx].end() );
	}

	// compute frame of all smoothed directional signals

	// init all samples to zero
	m_vvFSmoothedPSSigsLastFrame.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiDirSigIdx=0; uiDirSigIdx < m_uiNoOfChannelsWithVariableType; uiDirSigIdx++)
	{
		m_vvFSmoothedPSSigsLastFrame[uiDirSigIdx].assign(m_uiFrameSize, static_cast<FLOAT>( 0.0));
	}

	// add contributions from last frame
		
		// directional signals
		for ( std::map<unsigned int, unsigned int>::const_iterator muicIt =  m_muiLastDirSigAndGridDirIndices.begin(); muicIt != m_muiLastDirSigAndGridDirIndices.end(); muicIt++)
		{
			unsigned int uiCurrDirIdx     = muicIt->first - 1;
			unsigned int uiCurrGridDirIdx = muicIt->second - 1;

			// compute pseudo inverse of mode matrix related to dominant direction
			std::vector<FLOAT> vFPseudoInvModeVec;
	
			if ( computePseudoInverseOfVec( m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx], vFPseudoInvModeVec) )
			{
				return true;
			}

			// set shape of fade out window
			FLOAT *pFCurrFadeWin = &(m_vFCrossFadeWinForDirSigs[0]);
			
			std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFcItCurrVecIdx = p_rmuivFInputVectors.find( uiCurrDirIdx + 1 );

			if ( muivFcItCurrVecIdx != p_rmuivFInputVectors.end() )
			{
				pFCurrFadeWin = &(m_vFCrossFadeWinForVecSigs[0]);
			}

			//

			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				FLOAT FCurrContribution = static_cast<FLOAT>(0.0);

				for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
				{
					FCurrContribution += vFPseudoInvModeVec[uiCoeffIdx]*m_vvFLastHOAFrame[uiCoeffIdx][uiSampleIdx];
				}

				FCurrContribution *= pFCurrFadeWin[m_uiFrameSize + uiSampleIdx];

				m_vvFSmoothedPSSigsLastFrame[uiCurrDirIdx][uiSampleIdx] += FCurrContribution;
			}
		}

		// vector-based signals
		for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFcIt = m_muivFLastVectors.begin(); muivFcIt != m_muivFLastVectors.end(); muivFcIt++)
		{
			unsigned int uiCurrVecIdx            = muivFcIt->first - 1;
			const std::vector<FLOAT> &vFCurrVec  = muivFcIt->second;

			// compute pseudo inverse of current vector
			std::vector<FLOAT> vFPseudoInvModeVec;
	
			if ( computePseudoInverseOfVec( vFCurrVec, vFPseudoInvModeVec) )
			{
				return true;
			}

			// set shape of fade out window
			FLOAT *pFCurrFadeWin = &(m_vFCrossFadeWinForDirSigs[0]);

			std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFcItCurrVecIdx = p_rmuivFInputVectors.find( uiCurrVecIdx + 1 );

			if ( muivFcItCurrVecIdx != p_rmuivFInputVectors.end() )
			{
				pFCurrFadeWin = &(m_vFCrossFadeWinForVecSigs[0]);
			}
		
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				FLOAT FCurrContribution = static_cast<FLOAT>(0.0);

				for (unsigned int uiCoeffIdx = m_uiVVecStartCh; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
				{
					FCurrContribution += vFPseudoInvModeVec[uiCoeffIdx-m_uiVVecStartCh]*m_vvFLastHOAFrame[uiCoeffIdx][uiSampleIdx];
				}

				FCurrContribution *= pFCurrFadeWin[m_uiFrameSize+uiSampleIdx];

				m_vvFSmoothedPSSigsLastFrame[uiCurrVecIdx][uiSampleIdx] += FCurrContribution;
			}
		}
	
	// add contribution from current frame

		// directional signals
		for ( std::map<unsigned int, unsigned int>::const_iterator muicIt =  p_rmuiInputDirSigAndGridDirIndices.begin(); 
					muicIt != p_rmuiInputDirSigAndGridDirIndices.end(); muicIt++)
		{
			unsigned int uiCurrDirIdx     = muicIt->first - 1;
			unsigned int uiCurrGridDirIdx = muicIt->second - 1;

			// compute pseudo inverse of mode matrix related to dominant direction
			std::vector<FLOAT> vFPseudoInvModeVec;
	
			if ( computePseudoInverseOfVec( m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx], vFPseudoInvModeVec) )
			{
				return true;
			}

			// set shape of fade in window
			FLOAT *pFCurrFadeWin = &(m_vFCrossFadeWinForDirSigs[0]);
			//

			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				FLOAT FCurrContribution = static_cast<FLOAT>(0.0);

				for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
				{
					FCurrContribution += vFPseudoInvModeVec[uiCoeffIdx]*m_vvFLastHOAFrame[uiCoeffIdx][uiSampleIdx];
				}

				FCurrContribution *= pFCurrFadeWin[uiSampleIdx];

				m_vvFSmoothedPSSigsLastFrame[uiCurrDirIdx][uiSampleIdx] += FCurrContribution;
			}

		}

		// vector-based signals
		for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFcIt = p_rmuivFInputVectors.begin(); muivFcIt != p_rmuivFInputVectors.end(); muivFcIt++)
		{
			unsigned int uiCurrVecIdx            = muivFcIt->first - 1;
			const std::vector<FLOAT> &vFCurrVec  = muivFcIt->second;

			// compute pseudo inverse of current vector
			std::vector<FLOAT> vFPseudoInvModeVec;
	
			if ( computePseudoInverseOfVec( vFCurrVec, vFPseudoInvModeVec) )
			{
				return true;
			}

			// set shape of fade in window
			FLOAT *pFCurrFadeWin = &(m_vFCrossFadeWinForVecSigs[0]);		

			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				FLOAT FCurrContribution = static_cast<FLOAT>(0.0);

				for (unsigned int uiCoeffIdx = m_uiVVecStartCh; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
				{
					FCurrContribution += vFPseudoInvModeVec[uiCoeffIdx-m_uiVVecStartCh]*m_vvFLastHOAFrame[uiCoeffIdx][uiSampleIdx];
				}

				FCurrContribution *= pFCurrFadeWin[uiSampleIdx];

				m_vvFSmoothedPSSigsLastFrame[uiCurrVecIdx][uiSampleIdx] += FCurrContribution;
			}

		}
		////////////////////////////////////////////////

	// copy frame of smoothed PS signals
	p_rvvFOutputSmoothedPSSigsLastFrame.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiDirIdx=0; uiDirIdx < m_uiNoOfChannelsWithVariableType; uiDirIdx++)
	{
		p_rvvFOutputSmoothedPSSigsLastFrame[uiDirIdx].assign( m_vvFSmoothedPSSigsLastFrame[uiDirIdx].begin(), m_vvFSmoothedPSSigsLastFrame[uiDirIdx].end() );
	}

	// copy HOA frame of smoothed directional signals
	m_vvFSmoothedDirSigsLastHOAFrame.resize(m_uiNOOFHOACoeffs);
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
	{
		m_vvFSmoothedDirSigsLastHOAFrame[uiCoeffIdx].assign( m_vvFSmoothedDirSigsHOAFrame[uiCoeffIdx].begin(), m_vvFSmoothedDirSigsHOAFrame[uiCoeffIdx].end() );
	}


	// copy HOA frame of smoothed vector based signals
	m_vvSmoothedVecSigsLastHOAFrame.resize(m_uiNOOFHOACoeffs);
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
	{
		m_vvSmoothedVecSigsLastHOAFrame[uiCoeffIdx].assign( m_vvSmoothedVecSigsHOAFrame[uiCoeffIdx].begin(), m_vvSmoothedVecSigsHOAFrame[uiCoeffIdx].end() );
	}

	// compute prediction parameters, here NO prediction is assumed
		// set prediction type vector
		p_vuiOutputPredictionTypeVec.assign( m_uiNOOFHOACoeffs, 0  );

		// set prediction indices matrix
		p_vvuiOutputPredictionIndicesMat.resize( m_uiMaxNofOfDirSigsForPred );

		for (unsigned int uiPredDirSigIdx=0; uiPredDirSigIdx < m_uiMaxNofOfDirSigsForPred; uiPredDirSigIdx++)
		{
			p_vvuiOutputPredictionIndicesMat[uiPredDirSigIdx].assign(m_uiNOOFHOACoeffs, 0  );
		}

		// set prediction factors matrix
		p_vviOutputQuantizedPredictionFactorsMat.resize( m_uiMaxNofOfDirSigsForPred );
		for (unsigned int uiPredDirSigIdx=0; uiPredDirSigIdx < m_uiMaxNofOfDirSigsForPred; uiPredDirSigIdx++)
		{
			p_vviOutputQuantizedPredictionFactorsMat[uiPredDirSigIdx].assign(m_uiNOOFHOACoeffs, 0 );
		}


	// perform predominant sound synthesis
	std::set<unsigned int> suiOldAmbCoeffIndicesToBeEnabled;
	std::set<unsigned int> suiOldAmbCoeffIndicesToBeDisabled;
	std::set<unsigned int> suiOldNonEnDisAbleActHOACoeffIndices;
	std::vector<unsigned int> vuiAmbHOATransitionVec;

	m_shptrPreDomSoundSynth->process(	p_rvvFOutputSmoothedPSSigsLastFrame,
										p_rvvFOutputPenultimateSmoothedPreDomSoundsHOAFrame,
                              mvvFFadedForPARPreDomSoundsHOAFrame,
										m_vvFSmoothedDirSigsHOAFrame,
										m_vvSmoothedVecSigsHOAFrame,
										p_rmuiInputDirSigAndGridDirIndices,
										p_vuiOutputPredictionTypeVec,
										p_vvuiOutputPredictionIndicesMat,
										p_vviOutputQuantizedPredictionFactorsMat,
										suiOldAmbCoeffIndicesToBeEnabled,
										suiOldAmbCoeffIndicesToBeDisabled,
										suiOldNonEnDisAbleActHOACoeffIndices,
										p_rmuivFInputVectors);

	// compute ambient HOA component
	p_rvvFOutputPenultimateAmbientHOAFrame.resize( m_uiNOOFHOACoeffs );
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
	{
		p_rvvFOutputPenultimateAmbientHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0) );

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			p_rvvFOutputPenultimateAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] =  m_vvFPenultimateHOAFrame[uiCoeffIdx][uiSampleIdx] 
																				- m_vvFSmoothedDirSigsLastHOAFrame[uiCoeffIdx][uiSampleIdx]
																				- (m_fMinVVecFactor * m_vvSmoothedVecSigsLastHOAFrame[uiCoeffIdx][uiSampleIdx]);
		}
	}

	// compute predicted ambient HOA component
	p_rvvFOutputPredictedLastAmbientHOAFrame.resize( m_uiNOOFHOACoeffs );
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
	{
		p_rvvFOutputPredictedLastAmbientHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0) );

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			p_rvvFOutputPredictedLastAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] = m_vvFLastHOAFrame[uiCoeffIdx][uiSampleIdx] 
																				- m_vvFSmoothedDirSigsHOAFrame[uiCoeffIdx][uiSampleIdx]
																				- (m_fMinVVecFactor * m_vvSmoothedVecSigsHOAFrame[uiCoeffIdx][uiSampleIdx]);
		}
	}


	// compute target channel assignment specified by the HOA decomposition
	p_vAITargetAssignmentVector.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiChanIdx = 0; uiChanIdx < m_uiNoOfChannelsWithVariableType; uiChanIdx++)
	{
		bool bDirIsCurrentlyActive = (p_rmuiInputDirSigAndGridDirIndices.find(uiChanIdx+1) != p_rmuiInputDirSigAndGridDirIndices.end());
		bool bDirWasActiveBefore   = (m_muiLastDirSigAndGridDirIndices.find(uiChanIdx+1) !=  m_muiLastDirSigAndGridDirIndices.end());

		bool bVecIsCurrentlyActive = (p_rmuivFInputVectors.find(uiChanIdx+1) != p_rmuivFInputVectors.end());
		bool bVecWasActiveBefore   = (m_muivFLastVectors.find(uiChanIdx+1) != m_muivFLastVectors.end());

		if ( bDirIsCurrentlyActive || bDirWasActiveBefore || bVecIsCurrentlyActive || bVecWasActiveBefore )
		{
			p_vAITargetAssignmentVector[uiChanIdx].m_bIsAvailable = true;
			p_vAITargetAssignmentVector[uiChanIdx].m_uiBitRate    = m_uiBitRatePerPercCoder;

			if ( bDirIsCurrentlyActive ||  (bDirWasActiveBefore && !bDirIsCurrentlyActive && !bVecIsCurrentlyActive)  )
			{
				p_vAITargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData= std::shared_ptr<CChannelSideInfoData>(new CDirectionalInfoChannel() );
				CDirectionalInfoChannel	* pDIC = dynamic_cast<CDirectionalInfoChannel *>(p_vAITargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());

				if (bDirIsCurrentlyActive)
				{
					pDIC->m_unActiveDirIds = p_rmuiInputDirSigAndGridDirIndices.at(uiChanIdx+1);
				}
				else
				{
					pDIC->m_unActiveDirIds = 0;
				}
			}

			if ( bVecIsCurrentlyActive || (bVecWasActiveBefore && !bDirIsCurrentlyActive && !bVecIsCurrentlyActive ) )
			{
				p_vAITargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData= std::shared_ptr<CChannelSideInfoData>(new CVectorBasedInfoChannel() );
				CVectorBasedInfoChannel	* pVIC = dynamic_cast<CVectorBasedInfoChannel *>(p_vAITargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());
				
				
				if (c8BitQuantizerWord == pVIC->m_unNbitsQ )
				{
					pVIC->m_bSameHeaderPrevFrame = 1;// signaling use of prev. vector decoding method
				}				
				else
				{
					pVIC->m_bSameHeaderPrevFrame = 0;
					pVIC->m_unNbitsQ = c8BitQuantizerWord;
				}

				pVIC->m_vun8bitCodedVelement.resize(m_uiNOOFHOACoeffs-m_uiVVecStartCh);

				std::vector<FLOAT> vFCurrVector;

				if (bVecIsCurrentlyActive)
				{
					vFCurrVector = p_rmuivFInputVectors.at(uiChanIdx+1);
				}
				else
				{
					vFCurrVector = m_muivFLastVectors.at(uiChanIdx+1);
				}

				// quantize vector elements
				for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNOOFHOACoeffs-m_uiVVecStartCh;uiElemIdx++)
				{
					FLOAT FCurrScaledElem = vFCurrVector[uiElemIdx];
					// AK proposal: devide vector by 1/(N+1) before quantization to ensure the value of each element is not greater than 1
					FCurrScaledElem /= static_cast<FLOAT>(m_uiHOAOrder+1);

					quantizeUniform( FCurrScaledElem, m_uiNoBitsForVecElemQuant,  pVIC->m_vun8bitCodedVelement[uiElemIdx] );
				}
			}
		}
		else
		{
			p_vAITargetAssignmentVector[uiChanIdx].init();
		}
	}

	// refresh buffers
	m_muiLastDirSigAndGridDirIndices = p_rmuiInputDirSigAndGridDirIndices;
	m_muivFLastVectors				 = p_rmuivFInputVectors;

	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNOOFHOACoeffs; uiCoeffIdx++)
	{
		m_vvFPenultimateHOAFrame[uiCoeffIdx].assign( m_vvFLastHOAFrame[uiCoeffIdx].begin(), m_vvFLastHOAFrame[uiCoeffIdx].end() );
		m_vvFLastHOAFrame[uiCoeffIdx].assign( p_rvvFInputHOAFrame[uiCoeffIdx].begin(), p_rvvFInputHOAFrame[uiCoeffIdx].end() );
	}

	return false;
}





