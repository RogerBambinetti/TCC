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
 
 Qualcomm Technologies, Inc. (QTI) retains full right to modify and use the code 
 for its own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2015.
 
 ***********************************************************************************/
#include "VectorBasedPredomSoundSynthesis.h"

// constructor 
VectorBasedPredomSoundSynthesis::VectorBasedPredomSoundSynthesis( )
{
      m_bIsInit = false;
}

// reset member function
bool VectorBasedPredomSoundSynthesis::init(
                        unsigned int p_uiNoOfChannelsWithVariableType, 
                        unsigned int p_uiHOAOrder, 
                        unsigned int p_uiFrameSize,
						unsigned int p_uiMinNoOfCoeffsForAmbHOA,
						unsigned int p_uiVVecStartCh,
						unsigned int p_uiCodedVVecLength,
						unsigned int p_uiInterpSamples
            )
{
      m_uiMaxNoOfDirSigs        = p_uiNoOfChannelsWithVariableType;
      m_uiHOAOrder				= p_uiHOAOrder;
      m_uiFrameSize				= p_uiFrameSize;
      m_uiNoOfHOACoeffs			= (m_uiHOAOrder+1)*(m_uiHOAOrder+1);
      m_uiNoOfAmbiChs           = p_uiMinNoOfCoeffsForAmbHOA;
	  m_uiVVecStartCh           = p_uiVVecStartCh;
	  m_uiCodedVVecLength		= p_uiCodedVVecLength;
	  m_uiInterpSamples			= p_uiInterpSamples;
	  m_muivFPreviousVectors.clear();
	  m_suiSetOfPreviousPSSigIndices.clear();

      // setup delayed HOA frame 
      //std::vector<FLOAT> vFTmpFgVec(m_uiFrameSize, static_cast<FLOAT>(0.0));
      //m_rvvFDelayedVecBasedPredomSoundsHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpFgVec);

	  m_vFOnesLongVec.assign(2*m_uiFrameSize, static_cast<FLOAT>(1.0));

      m_bIsInit = true;

      return false;
}




bool VectorBasedPredomSoundSynthesis::process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputDirSigsFrame, 
													  std::vector<std::vector<FLOAT> > &p_rvvFOutputVecBasedPreDomSoundsHOAFrame,
												const std::map<unsigned int, std::vector<FLOAT> > &p_rmuivFVectors,
												const std::set<unsigned int> &p_rsuiSetOfPSSigIndices,
												const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeEnabled,
												const std::set<unsigned int> &p_suiAmbCoeffIndicesToBeDisabled,
												const std::vector<FLOAT> &p_rvFInterpolationFadeInWinForDirSigs,
												const std::vector<FLOAT> &p_rvFInterpolationFadeOutWinForDirSigs,
												const std::vector<FLOAT> &p_rvFInterpolationFadeInWinForVecSigs,
												const std::vector<FLOAT> &p_rvFInterpolationFadeOutWinForVecSigs)
{
   // check if PreDomSoundSynthesis object has already been initialized
    if ( !m_bIsInit )
    {
       return true;
    }

    // check if the buffer sizes are correct
    if ( checkBufferSizes (p_rvvFInputDirSigsFrame) )
    {
      return true;
    }
	unsigned int uiInterpSamples;

	// init buffer to zero
	p_rvvFOutputVecBasedPreDomSoundsHOAFrame.resize(m_uiNoOfHOACoeffs);
	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
		p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdx].assign(m_uiFrameSize, static_cast<FLOAT>(0.0));
	}

	// interpolation contribution from previous vectors
	for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFIt=m_muivFPreviousVectors.begin(); muivFIt !=m_muivFPreviousVectors.end(); muivFIt++)
	{
		unsigned int uiVecSigIdx      = muivFIt->first;
		std::vector<FLOAT> rvFCurrVec = muivFIt->second;

		// look if the PS signal is also active in the current frame
		std::set<unsigned int>::const_iterator suiItActiveDirIdx = p_rsuiSetOfPSSigIndices.find( uiVecSigIdx );

		if ( suiItActiveDirIdx != p_rsuiSetOfPSSigIndices.end() )
		{
			// default to crossfade window for dir based signals
			const FLOAT *pFCurrInterpWin = &(p_rvFInterpolationFadeOutWinForDirSigs[0]);
			uiInterpSamples = m_uiFrameSize;
			// look if the vector based signal is also active in the current frame
			std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFItActiveVecIdx = p_rmuivFVectors.find( uiVecSigIdx);

			// if so, switch to vector based interpolation window
			if ( muivFItActiveVecIdx != p_rmuivFVectors.end() )
			{
				pFCurrInterpWin = &(p_rvFInterpolationFadeOutWinForVecSigs[0]); 
				uiInterpSamples = m_uiInterpSamples;
			}

			for (unsigned int uiCoeffIdx = m_uiVVecStartCh ; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
			{
				for (unsigned int uiSampleIdx=0; uiSampleIdx < uiInterpSamples; uiSampleIdx++)
				{
					// interpolate v vector
					FLOAT FInterpolatedVVec = rvFCurrVec[uiCoeffIdx-m_uiVVecStartCh]*pFCurrInterpWin[uiSampleIdx];
					// generate HOA signals with the interpolated v vector
					FLOAT FCurrHOAContribution = FInterpolatedVVec*p_rvvFInputDirSigsFrame[uiVecSigIdx-1][uiSampleIdx];
					// mix
					p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdx][uiSampleIdx] += FCurrHOAContribution;
				}
			}
		}
	}

	// interpolation contribution from current v vectors
	for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFIt = p_rmuivFVectors.begin(); muivFIt!=p_rmuivFVectors.end(); muivFIt++)
	{
		unsigned int uiVecSigIdx      = muivFIt->first;
		std::vector<FLOAT> rvFCurrVec = muivFIt->second;

		// default to interpolation window with all zeros
		const FLOAT *pFCurrInterpWin = &(m_vFOnesLongVec[0]);

		// look if predominant sound signal was also active in the last frame
		std::set<unsigned int>::const_iterator suiItLastActPSIdx = m_suiSetOfPreviousPSSigIndices.find( uiVecSigIdx);

		// if so, set to vector based interpolation window
		if ( suiItLastActPSIdx != m_suiSetOfPreviousPSSigIndices.end())
		{
			pFCurrInterpWin = &(p_rvFInterpolationFadeInWinForVecSigs[0]);
		}
		
		for (unsigned int uiCoeffIdx = m_uiVVecStartCh ; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
		{
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				// interpolate v vector
				FLOAT FInterpolatedVVec = rvFCurrVec[uiCoeffIdx-m_uiVVecStartCh]*pFCurrInterpWin[uiSampleIdx];
				// generate HOA signals with the interpolated v vector
				FLOAT FCurrHOAContribution = FInterpolatedVVec*p_rvvFInputDirSigsFrame[uiVecSigIdx-1][uiSampleIdx];
				// mix
				p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdx][uiSampleIdx] += FCurrHOAContribution;
			}
		}
	}

	// account for additional HOA channels in transition
	if (m_uiCodedVVecLength==1) //bugfix 2014-03-20 NP: the fade in/outs are only necessary in this VVecLength mode NP
	{
		// HOA fades in, VecBasedPredomSoundsHOAFrame must fade out
		for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeEnabled.begin(); suiIt != p_suiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxToBeFadedOut = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdxToBeFadedOut][uiSampleIdx] *= p_rvFInterpolationFadeOutWinForDirSigs[uiSampleIdx];
			}	
		}
		// HOA fades out, VecBasedPredomSoundsHOAFrame must fade in
		for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeDisabled.begin(); suiIt != p_suiAmbCoeffIndicesToBeDisabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxToBeFadedIn = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFOutputVecBasedPreDomSoundsHOAFrame[uiCoeffIdxToBeFadedIn][uiSampleIdx] *= p_rvFInterpolationFadeInWinForDirSigs[uiSampleIdx];
			}	
		}

		// updating p_rmuivFVectors - bugfix NP 2015-02-13
		m_muivFPreviousVectors.clear();
		for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator muivFIt = p_rmuivFVectors.begin(); muivFIt!=p_rmuivFVectors.end(); muivFIt++)
		{
			unsigned int uiVecSigIdx      = muivFIt->first;
			std::vector<FLOAT> rvFCurrVec = muivFIt->second;

			for (std::set<unsigned int>::iterator suiIt = p_suiAmbCoeffIndicesToBeEnabled.begin(); suiIt != p_suiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
			{
				unsigned int uiCoeffIdxToBeFadedOut = *suiIt - 1;
				rvFCurrVec[uiCoeffIdxToBeFadedOut - m_uiVVecStartCh] = 0;
			}
			m_muivFPreviousVectors.insert( std::pair<unsigned int, std::vector<FLOAT> >( uiVecSigIdx, rvFCurrVec) );
		}		
	}
	else
	{
    m_muivFPreviousVectors			= p_rmuivFVectors; 
	}

	m_suiSetOfPreviousPSSigIndices  = p_rsuiSetOfPSSigIndices;

    return false;
}

bool VectorBasedPredomSoundSynthesis::checkBufferSizes(const std::vector<std::vector<FLOAT> > &p_rvvFInputVecSigsFrame)
{
    //// check if the sizes of the buffers are correct
    if ( p_rvvFInputVecSigsFrame.size() != m_uiMaxNoOfDirSigs)
    {
      std::cout<< "VectorBasedPredomSoundSynthesis: Buffer dimensions don't match with expected dimensions!" << std::endl;
      return true;
    }
        bool bError = false;

    for( unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNoOfDirSigs; uiRowIdx++)
    {
       if( p_rvvFInputVecSigsFrame[uiRowIdx].size() != m_uiFrameSize)
       {
          bError = true;
       }
    }
    for( unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNoOfDirSigs; uiRowIdx++)
    {
       if( p_rvvFInputVecSigsFrame[uiRowIdx].size() != m_uiFrameSize)
       {
          bError = true;
       }
    }
    if (bError)
    {
       std::cout<< "VectorBasedPredomSoundSynthesis: Buffer lengths don't match!" << std::endl;
    }
    return bError;
}

