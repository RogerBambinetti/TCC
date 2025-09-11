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
 $Id: DirBasedPreDomSoundSynthesis.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "DirBasedPreDomSoundSynthesis.h"
#include <cmath>

// constructor 
DirBasedPreDomSoundSynthesis::DirBasedPreDomSoundSynthesis( )
{
		m_bIsInit = false;
}



// reset member function
bool DirBasedPreDomSoundSynthesis::init(unsigned int p_uiNoOfChannelsWithVariableType, 
										unsigned int p_uiHOAOrder, 
										unsigned int p_uiFrameSize, 
										unsigned int p_uiMinNoOfCoeffsForAmbHOA,
										unsigned int p_uiMaxNofOfDirSigsForPred,
										unsigned int p_uiNoOfBitsPerScaleFactor,
										std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
		m_uiNoOfChannelsWithVariableType = p_uiNoOfChannelsWithVariableType;
		m_uiHOAOrder				= p_uiHOAOrder;
		m_uiFrameSize				= p_uiFrameSize;
		m_uiMinNoOfCoeffsForAmbHOA	= p_uiMinNoOfCoeffsForAmbHOA;
		m_uiMaxNofOfDirSigsForPred  = p_uiMaxNofOfDirSigsForPred;
		m_uiNoOfBitsPerScaleFactor  = p_uiNoOfBitsPerScaleFactor;

		m_uiNoOfHOACoeffs			= (m_uiHOAOrder+1)*(m_uiHOAOrder+1);

		// get mode matrix for coarse grid points
		if ( p_shptrTabVals->getModeMat(m_vvFModeMatAllCoarseGridPoints, m_uiHOAOrder) ) 
		{
			return true;
		}

		// get mode matrix for fine grid points
		if ( p_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, 2) )
		{
			return true;
		}

		// reset buffers
		std::vector<FLOAT> vFTmpRowVec ( m_uiFrameSize, static_cast<FLOAT>(0.0));
		//m_vvFSmoothedDirSigsHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpRowVec );
		m_vvFPredictedGridDirSigsLastHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpRowVec );
		m_vvFPredictedGridDirSigsCurrentHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpRowVec );
		m_vvFPredictedGridDirSigsSmoothedHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpRowVec );

		std::vector<unsigned int> vuiTmpVec (m_uiNoOfHOACoeffs , 0);
		m_vuiLastPredictionTypeVec.assign( vuiTmpVec.begin(), vuiTmpVec.end() );
		m_vvuiLastPredictionIndicesMat.assign( m_uiMaxNofOfDirSigsForPred, vuiTmpVec);

		std::vector<FLOAT> vFTmpVec (m_uiNoOfHOACoeffs , static_cast<FLOAT>(0.0) );
		m_vvFLastPredictionFactorsMat.assign( m_uiMaxNofOfDirSigsForPred, vFTmpVec);
		m_vvFPredictionFactorsMat.assign( m_uiMaxNofOfDirSigsForPred, vFTmpVec);

		m_vFOnesLongVec.assign(2*m_uiFrameSize, static_cast<FLOAT>(1.0));

		// clear sets
		m_muiLastActiveDirAndGridDirIncides.clear();
		m_suiSetOfOldPSSigIndices.clear();

		m_bIsInit = true;

		return false;
}



bool DirBasedPreDomSoundSynthesis::process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame, 
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
									const std::vector<FLOAT> &p_rvFFadeOutWinForVecSigs)
{

    computeHOAReprOfDirSigs(p_rvvFInputPSSigsFrame,
                            p_rmuiActiveDirAndGridDirIndices,
                            p_rsuiSetOfPSSigIndices,
							p_rvFFadeInWinForDirSigs,
							p_rvFFadeOutWinForDirSigs,
							p_rvFFadeOutWinForVecSigs,
                            p_rvvFOutputSmoothedDirSigsHOAFrame);

	computeSmoothedHOARepresentationOfSpatPredSigs(	p_rvvFInputPSSigsFrame,
													p_rvuiPredictionTypeVec,
									                p_rvvuiPredictionIndicesMat,
									                p_rvviQuantizedPredictionFactorsMat,
													p_rvFFadeInWinForDirSigs,
									                p_rvFFadeOutWinForDirSigs,
                                                    p_suiAmbCoeffIndicesToBeEnabled,
									                p_suiAmbCoeffIndicesToBeDisabled,
									                p_suiNonEnDisAbleActHOACoeffIndices,
                                                    m_vvFPredictedGridDirSigsSmoothedHOAFrame);


     // copy directional HOA component to output predominant sound HOA component
	for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
	{
		p_rvvFOutputDirBasedPreDomSoundsHOAFrame[uiRowIdx].assign( p_rvvFOutputSmoothedDirSigsHOAFrame[uiRowIdx].begin(), p_rvvFOutputSmoothedDirSigsHOAFrame[uiRowIdx].end() );
	}

	 // add predicted directional signals to output buffer to contain predominant sound component
	 for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	 {
		 // copy buffer (directional HOA component) to output
		 for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		 {
			p_rvvFOutputDirBasedPreDomSoundsHOAFrame[uiCoeffIdx][uiSampleIdx] += m_vvFPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdx][uiSampleIdx];
		 }
	 }

	return false;
}

bool DirBasedPreDomSoundSynthesis::computeSmoothedHOARepresentationOfSpatPredSigs(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
																		            const std::vector<unsigned int> &p_rvuiPredictionTypeVec,
									                                                const std::vector<std::vector<unsigned int> > &p_rvvuiPredictionIndicesMat,
									                                                const std::vector<std::vector<int> > &p_rvviQuantizedPredictionFactorsMat,
																		            const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
									                                                const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
                                                                                    const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
									                                                const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
									                                                const std::set<unsigned int> &p_suiNonEnDisAbleActHOACoeffIndices,
                                                                                    std::vector<std::vector<FLOAT> > & p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame)
{
    // compute HOA representation of predicted signals (fade out and fade in contributions)

         // dequantize prediction factors
	     for (unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNofOfDirSigsForPred; uiRowIdx++)
	     {
		     for (unsigned int uiGridDirIdx=0; uiGridDirIdx < m_uiNoOfHOACoeffs; uiGridDirIdx++)
		     {
			    FLOAT FDequantizedFactor = static_cast<FLOAT>( (static_cast<double>(p_rvviQuantizedPredictionFactorsMat[uiRowIdx][uiGridDirIdx]) + 0.5) * pow(2.0, - static_cast<int>(m_uiNoOfBitsPerScaleFactor) + 1) );
						
			    m_vvFPredictionFactorsMat[uiRowIdx][uiGridDirIdx] = FDequantizedFactor;
		     }
	     }


		bool bSpatialPredictionInLastFrame=false; 
        bool bSpatialPredictionInCurrentFrame=false; 

        if (p_rvvFInputPSSigsFrame.size() == 0)
        {
            std::vector<FLOAT> vFTmpRowVec ( m_uiFrameSize, static_cast<FLOAT>(0.0));
            m_vvFPredictedGridDirSigsLastHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpRowVec );
            m_vvFPredictedGridDirSigsCurrentHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpRowVec );

        }
        else
        {
            // fade out contribution
		    computeHOARepresentationOfSpatPred(	p_rvvFInputPSSigsFrame,
											    m_vvFModeMatAllCoarseGridPoints,
											    m_vuiLastPredictionTypeVec,
											    m_vvuiLastPredictionIndicesMat,
											    m_vvFLastPredictionFactorsMat,
											    p_rvFFadeOutWinForDirSigs,
											    m_vvFPredictedGridDirSigsLastHOAFrame,
											    bSpatialPredictionInLastFrame );

		    // fade in contribution
		    computeHOARepresentationOfSpatPred(	p_rvvFInputPSSigsFrame,
											    m_vvFModeMatAllCoarseGridPoints,
											    p_rvuiPredictionTypeVec,
											    p_rvvuiPredictionIndicesMat,
											    m_vvFPredictionFactorsMat,
											    p_rvFFadeInWinForDirSigs,
											    m_vvFPredictedGridDirSigsCurrentHOAFrame,
											    bSpatialPredictionInCurrentFrame );
        }

	// add both contributions
    p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame.resize(m_uiNoOfHOACoeffs);

	 for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	 {
		 // add only if corresponding ambient HOA coefficient is not permanently active 
		 std::set<unsigned int>::const_iterator suiItActivePredCoeffIndex =  p_suiNonEnDisAbleActHOACoeffIndices.find( uiCoeffIdx + 1 );

		 if ( suiItActivePredCoeffIndex == p_suiNonEnDisAbleActHOACoeffIndices.end() )
		 {
             p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdx].resize(m_uiFrameSize);

			 for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			 {
				 p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdx][uiSampleIdx] =     m_vvFPredictedGridDirSigsLastHOAFrame[uiCoeffIdx][uiSampleIdx]
																					 + m_vvFPredictedGridDirSigsCurrentHOAFrame[uiCoeffIdx][uiSampleIdx];
			 }
		 }
		 else
		 {
			 p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0) );
		 }
	 }

	// fade out predicted grid signal HOA coefficients IF ambient HOA coeffients are enabled and spatial prediction has been performed in current frame
	if ( bSpatialPredictionInCurrentFrame )
	{
		for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeEnabled.begin(); suiIt != p_suiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxForPredHOAToBeFadedOut = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdxForPredHOAToBeFadedOut][uiSampleIdx] *= p_rvFFadeOutWinForDirSigs[uiSampleIdx];
			}
		}
	}

	// fade in predicted grid signal HOA coefficients IF ambient HOA coeffients are disabled and spatial prediction has been performed in last frame
	if (bSpatialPredictionInLastFrame)
	{
		for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeDisabled.begin(); suiIt != p_suiAmbCoeffIndicesToBeDisabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxForPredHOAToBeFadedIn = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_vvFOutputPredictedGridDirSigsSmoothedHOAFrame[uiCoeffIdxForPredHOAToBeFadedIn][uiSampleIdx] *= p_rvFFadeInWinForDirSigs[uiSampleIdx];
			}

		}
	}

	 // copy intern buffers
	 m_vuiLastPredictionTypeVec.assign( p_rvuiPredictionTypeVec.begin(), p_rvuiPredictionTypeVec.end() );

	 for (unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNofOfDirSigsForPred; uiRowIdx++)
	 {
		 m_vvuiLastPredictionIndicesMat[uiRowIdx].assign( p_rvvuiPredictionIndicesMat[uiRowIdx].begin(), p_rvvuiPredictionIndicesMat[uiRowIdx].end() );
		 m_vvFLastPredictionFactorsMat[uiRowIdx].assign( m_vvFPredictionFactorsMat[uiRowIdx].begin(), m_vvFPredictionFactorsMat[uiRowIdx].end() );
	 }

     return false;
}


bool DirBasedPreDomSoundSynthesis::computeHOAReprOfDirSigs(const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
                                                           const std::map<unsigned int, unsigned int> &p_rmuiActiveDirAndGridDirIndices,
                                                           const std::set<unsigned int> &p_rsuiSetOfPSSigIndices,
									                       const std::vector<FLOAT> &p_rvFFadeInWinForDirSigs,
									                       const std::vector<FLOAT> &p_rvFFadeOutWinForDirSigs,
									                       const std::vector<FLOAT> &p_rvFFadeOutWinForVecSigs,
                                                           std::vector<std::vector<FLOAT> > &p_rvvFOutputSmoothedDirSigsHOAFrame)
{
    // check if DirBasedPreDomSoundSynthesis object has already been initialized
	 if ( !m_bIsInit )
	 {
		 return true;
	 }

	 // check if the buffer sizes are correct
	 if ( checkBufferSizes (p_rvvFInputPSSigsFrame) )
	 {
	 	return true;
	 }

	 // compute directional component

		 // reset directional HOA component to zero
		 p_rvvFOutputSmoothedDirSigsHOAFrame.resize(m_uiNoOfHOACoeffs); 
		 for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
		 {
			// reset directional HOA component to zero
			p_rvvFOutputSmoothedDirSigsHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0)); 
		 }

	// add contribution from last frame
	for (std::map<unsigned int, unsigned int>::const_iterator muiIt = m_muiLastActiveDirAndGridDirIncides.begin(); muiIt != m_muiLastActiveDirAndGridDirIncides.end(); muiIt++)
	{
		unsigned int uiCurrDirIdx	  = muiIt->first;
		unsigned int uiCurrGridDirIdx = muiIt->second;

		// grid direction index of zero means that the direction is invalid
		if ( uiCurrGridDirIdx != 0)
		{
			// fade only out contribution if direction is also active in current frame
			std::map<unsigned int, unsigned int>::const_iterator muiItActiveDirIndex = p_rmuiActiveDirAndGridDirIndices.find( uiCurrDirIdx );
				
			const FLOAT *pFCurrFadeOutWin = &(m_vFOnesLongVec[0]);

			if ( muiItActiveDirIndex != p_rmuiActiveDirAndGridDirIndices.end() )
			{
				if( muiItActiveDirIndex->second != 0)
				{
					pFCurrFadeOutWin = &(p_rvFFadeOutWinForDirSigs[0]);
				}
			}
			else
			{
				std::set<unsigned int>::const_iterator suicItActivePSIndex = p_rsuiSetOfPSSigIndices.find( uiCurrDirIdx );

				if ( suicItActivePSIndex != p_rsuiSetOfPSSigIndices.end() )
				{
					pFCurrFadeOutWin = &(p_rvFFadeOutWinForVecSigs[0]);
				}
			}

			for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
			{
				for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
				{
					FLOAT FCurrHOAContribution = m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx-1][uiCoeffIdx]*p_rvvFInputPSSigsFrame[uiCurrDirIdx-1][uiSampleIdx];

					FCurrHOAContribution *= pFCurrFadeOutWin[uiSampleIdx];
						
					p_rvvFOutputSmoothedDirSigsHOAFrame[uiCoeffIdx][uiSampleIdx] += FCurrHOAContribution;  
				}
			}
		}
	}


	// add contribution from current frame
	for (std::map<unsigned int, unsigned int>::const_iterator muiIt = p_rmuiActiveDirAndGridDirIndices.begin(); muiIt != p_rmuiActiveDirAndGridDirIndices.end(); muiIt++)
	{
		unsigned int uiCurrDirIdx	  = muiIt->first;
		unsigned int uiCurrGridDirIdx = muiIt->second;

		// grid direction index of zero means that the direction is invalid
		if ( uiCurrGridDirIdx != 0)
		{
			// fade only in contribution if direction was also active in last frame
			const FLOAT *pFCurrFadeInWin = &(m_vFOnesLongVec[0]);
				
			std::map<unsigned int, unsigned int>::const_iterator muiItActiveDirIndex =  m_muiLastActiveDirAndGridDirIncides.find( uiCurrDirIdx );

			if (muiItActiveDirIndex != m_muiLastActiveDirAndGridDirIncides.end() )
			{
				if ( muiItActiveDirIndex->second != 0)
				{
					pFCurrFadeInWin = &(p_rvFFadeInWinForDirSigs[0]);
				}
			}
			else
			{
				std::set<unsigned int>::const_iterator suicIt = m_suiSetOfOldPSSigIndices.find( uiCurrDirIdx );

				if ( suicIt != m_suiSetOfOldPSSigIndices.end() )
				{
					pFCurrFadeInWin = &(p_rvFFadeInWinForDirSigs[0]);
				}
			}

			for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
			{
				for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
				{
					FLOAT FCurrHOAContribution = m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx-1][uiCoeffIdx] * p_rvvFInputPSSigsFrame[uiCurrDirIdx-1][uiSampleIdx];
					
					FCurrHOAContribution *= pFCurrFadeInWin[uiSampleIdx];

					p_rvvFOutputSmoothedDirSigsHOAFrame[uiCoeffIdx][uiSampleIdx] += FCurrHOAContribution;
				}
			}
		}
	}

     // copy intern buffers
	 m_muiLastActiveDirAndGridDirIncides	= p_rmuiActiveDirAndGridDirIndices;
	 m_suiSetOfOldPSSigIndices				= p_rsuiSetOfPSSigIndices;

    return false;
}



bool DirBasedPreDomSoundSynthesis::computeHOARepresentationOfSpatPred(	const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame,
																	    const std::vector<std::vector<FLOAT> > &p_rvvFModeMatAllCoarseGridPoints,
																		const std::vector<unsigned int> &p_rvuiPredictionTypeVec,
																		const std::vector<std::vector<unsigned int> > &p_rvvuiPredictionIndicesMat,
																		const std::vector<std::vector<FLOAT> > &p_rvvFPredictionFactorsMat,
																		const std::vector<FLOAT> &p_rvFFadeWin,
																		std::vector<std::vector<FLOAT> > &p_rvvFPredictedGridDirSigsHOAFrame,
																		bool &p_bSpatialPredictionInCurrentFrame )
{

	unsigned int uiMaxNofOfDirSigsForPred = p_rvvuiPredictionIndicesMat.size();
	unsigned int uiFrameSize			  = p_rvFFadeWin.size(); //was p_rvvFInputPSSigsFrame[0].size(), but crashes when p_rvvFInputPSSigsFrame.size()==0
	unsigned int uiNoOfHOACoeffs		  = p_rvuiPredictionTypeVec.size();

	// init HOA frame of predicted directional signals 
	std::vector<FLOAT> vFTmpRowVec = std::vector<FLOAT>( uiFrameSize, static_cast<FLOAT>(0.0));
	p_rvvFPredictedGridDirSigsHOAFrame.assign( uiNoOfHOACoeffs, vFTmpRowVec);

	std::vector<FLOAT> vFTmpFadedPredictedGridDirSigsFrame( uiFrameSize, static_cast<FLOAT>(0.0));

	p_bSpatialPredictionInCurrentFrame = false;

	for (unsigned int uiGridIdx=0; uiGridIdx < p_rvuiPredictionTypeVec.size(); uiGridIdx++)
	{
		 // if prediction is to be performed
		 if (p_rvuiPredictionTypeVec[uiGridIdx] != 0)
		 {
				p_bSpatialPredictionInCurrentFrame = true;

				// init windowed predicted grid signal
				vFTmpFadedPredictedGridDirSigsFrame.assign( uiFrameSize, static_cast<FLOAT>(0.0));

				// compute number of signals from which grid signals are to be predicted
				// computation is based on zero elements in m_vvuiLastPredictionIndicesMat
				unsigned int uiNoOfValidSigsForPred = 0;
				for (unsigned int uiSigIdx=0; uiSigIdx < uiMaxNofOfDirSigsForPred; uiSigIdx++)
				{
					if ( p_rvvuiPredictionIndicesMat[uiSigIdx][uiGridIdx] != 0)
					{
						uiNoOfValidSigsForPred++;
					}
					else
					{
						break;
					}
				}

				for (unsigned int uiSampleIdx=0; uiSampleIdx < uiFrameSize; uiSampleIdx++)
				{
					for (unsigned int uiSigIdx=0; uiSigIdx < uiNoOfValidSigsForPred; uiSigIdx++)
					{
							vFTmpFadedPredictedGridDirSigsFrame[uiSampleIdx] += p_rvvFPredictionFactorsMat[uiSigIdx][uiGridIdx]*
																				p_rvvFInputPSSigsFrame[p_rvvuiPredictionIndicesMat[uiSigIdx][uiGridIdx]-1][uiSampleIdx];
					}
					vFTmpFadedPredictedGridDirSigsFrame[uiSampleIdx] *= p_rvFFadeWin[uiSampleIdx];
				}
			
				// update HOA frame of predicted directional signals 
				for (unsigned int uiCoeffIdx=0; uiCoeffIdx < uiNoOfHOACoeffs; uiCoeffIdx++)
				{
						for (unsigned int uiSampleIdx=0; uiSampleIdx < uiFrameSize; uiSampleIdx++)
						{
							p_rvvFPredictedGridDirSigsHOAFrame[uiCoeffIdx][uiSampleIdx] += p_rvvFModeMatAllCoarseGridPoints[uiCoeffIdx][uiGridIdx]
																								*vFTmpFadedPredictedGridDirSigsFrame[uiSampleIdx];

						}
				}
		 }
	 }

	return false;
}



bool DirBasedPreDomSoundSynthesis::checkBufferSizes(const std::vector<std::vector<FLOAT> > &p_rvvFInputPSSigsFrame)
{
	 //// check if the sizes of the buffers are correct
	 if ( (p_rvvFInputPSSigsFrame.size() != m_uiNoOfChannelsWithVariableType) )
	 {
		std::cout<< "DirBasedPreDomSoundSynthesis: Buffer dimensions don't match with expected dimensions!" << std::endl;
		return true;
	 }

	 bool bError = false;

	 for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfChannelsWithVariableType; uiRowIdx++)
	 {
		 if( p_rvvFInputPSSigsFrame[uiRowIdx].size() != m_uiFrameSize)
		 {
			 bError = true;
		 }
	 }

	 for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfChannelsWithVariableType; uiRowIdx++)
	 {
		 if( p_rvvFInputPSSigsFrame[uiRowIdx].size() != m_uiFrameSize)
		 {
			 bError = true;
		 }
	 }

	 if (bError)
	 {
		 std::cout<< "DirBasedPreDomSoundSynthesis: Buffer lengths don't match!" << std::endl;
	 }

	 return bError;

}
