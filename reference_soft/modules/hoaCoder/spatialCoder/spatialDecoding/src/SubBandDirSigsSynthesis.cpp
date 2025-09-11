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
 $Id: SubBandDirSigsSynthesis.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#include "SubBandDirSigsSynthesis.h"

// constructor
SubBandDirSigsSynthesis::SubBandDirSigsSynthesis()
{
	m_bIsInit = false;
}

bool SubBandDirSigsSynthesis::init( unsigned int p_uiHOAOrder,
                                    unsigned int p_uiMaxNoOfTransmittedHOACoeffs,
                                    unsigned int p_uiFrameSize,
                                    const std::vector<unsigned int>& p_rvuiSubBandWidths,
                                    unsigned int p_uiNoOfSubBandsForDirPred,
                                    unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                                    unsigned int p_uiDirGridTableIdx,
                                    std::shared_ptr<TabulatedValues> m_shptrTabVals) 
{
    m_uiHOAOrder        = p_uiHOAOrder;
    m_uiFrameSize       = p_uiFrameSize;
    m_uiNoOfHOACoeffs   = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);

    // init QMF analysis and synthesis
    QMFLIB_HYBRID_FILTER_MODE eHybridMode = QMFLIB_HYBRID_OFF;
    m_shptrQMFAnalysis = std::shared_ptr<CqmfAnalysis>( new CqmfAnalysis() );
    m_shptrQMFAnalysis->init(p_uiMaxNoOfTransmittedHOACoeffs, m_uiFrameSize, eHybridMode);

    m_shptrQMFSynthesis = std::shared_ptr<CqmfSynthesis>( new CqmfSynthesis() );
    m_shptrQMFSynthesis->init(m_uiNoOfHOACoeffs, m_uiFrameSize, eHybridMode);

    unsigned int uiQMFSamples    = m_shptrQMFAnalysis->getNumFreqSamples();
    unsigned int uiNoOfFreqBands = m_shptrQMFAnalysis->getNumOfFreqBands();
    m_uiNoOfQMFDelaySamples      = m_shptrQMFAnalysis->getAnalSynthFilterDelay();

    // init sub band directional prediction
    m_shptrDirSubBandHOASynth = std::shared_ptr<DirSubBandHOASynthesis> ( new DirSubBandHOASynthesis() );
    m_shptrDirSubBandHOASynth->init(    uiQMFSamples,
                                        p_rvuiSubBandWidths,
                                        p_uiNoOfSubBandsForDirPred,
                                        p_uiMaxNoOfPredDirsPerSubBand,
                                        uiNoOfFreqBands,
                                        p_uiHOAOrder,
                                        p_uiMaxNoOfTransmittedHOACoeffs,
                                        p_uiDirGridTableIdx,
                                        m_shptrTabVals);

    // for QMF analysis
    std::vector<std::complex<FLOAT> > vcFTmpFreqSamplesOfOneBand(uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
    std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpFreqSamplesOfAllBands( uiNoOfFreqBands, vcFTmpFreqSamplesOfOneBand);

    m_vvvcFQMFHOABuffer.assign( p_uiMaxNoOfTransmittedHOACoeffs, vvcFTmpFreqSamplesOfAllBands );

    // for sub band directional prediction
    m_vvvcFFreqDomPredDirHOABuffer.assign( m_uiNoOfHOACoeffs, vvcFTmpFreqSamplesOfAllBands );

    std::vector<FLOAT> vFTmpSingleChannelBuffer ( p_uiFrameSize, static_cast<FLOAT>(0.0));

    m_vvFPredDirHOABuffer.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

    // for coeff. modification
    m_suiAmbCoeffIndicesToBeEnabledInLastFrame.clear();
    m_suiAmbCoeffIndicesToBeDisabledInLastFrame.clear();
    m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.clear();

    m_bDirSubBandPredInLastFrame = false;
    m_bDirSubBandPredInPenultimateFrame = false;

    std::vector<FLOAT> vFTimeDomCrossFadeWin;

    if ( computeFadeWinForDirBasedSynth(vFTimeDomCrossFadeWin, m_uiFrameSize) )
    {
        return true;
    }

    m_vFFadeInWin.assign( vFTimeDomCrossFadeWin.begin(), vFTimeDomCrossFadeWin.begin() + m_uiFrameSize);
    m_vFFadeOutWin.assign( vFTimeDomCrossFadeWin.begin() + m_uiFrameSize, vFTimeDomCrossFadeWin.end());

    m_bIsInit = true;

    return false;
}


bool SubBandDirSigsSynthesis::process(   const std::vector<std::vector<FLOAT> > &p_rvvFAmbHOACoeffsFrame,
                                         const std::shared_ptr<FrameParams>      p_shprtFrameParams,
                                         const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					                     const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				                         const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                                               bool p_bbDirSubBandPredInCurrentFrame, 
                                               std::vector<std::vector<FLOAT> > &p_rvvFSubBandDirSigsCompositionHOAFrame)
{
    // apply QMF analysis filter bank
    m_shptrQMFAnalysis->process(p_rvvFAmbHOACoeffsFrame, m_vvvcFQMFHOABuffer);


    // perform sub band directional prediction
    if ( m_shptrDirSubBandHOASynth->process(m_vvvcFQMFHOABuffer,
                                            p_shprtFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides,
                                            p_shprtFrameParams->m_vvvcFComplexDirPredictionCoeffsMats,
                                            p_bbDirSubBandPredInCurrentFrame,
                                            m_vvvcFFreqDomPredDirHOABuffer) )
    {
		return true;
	}

    // apply QMF synthesis filter bank
    m_shptrQMFSynthesis->process(m_vvvcFFreqDomPredDirHOABuffer, m_vvFPredDirHOABuffer);

    // modify coefficient sequences of HOA representation of composition of all directional sub-band signals
    // i.e. set to zero, fade in or fade out
    // depending on state of coefficient sequences of ambient HOA component
    CoeffModification(  m_vvFPredDirHOABuffer, 
                        p_rsuiAmbCoeffIndicesToBeEnabled,
                        p_rsuiAmbCoeffIndicesToBeDisabled,
                        p_rsuiNonEnDisAbleActHOACoeffIndices,
                        p_bbDirSubBandPredInCurrentFrame,
                        p_rvvFSubBandDirSigsCompositionHOAFrame);

    return false;
}

unsigned int SubBandDirSigsSynthesis::getNoOfQMFDelaySamples()
{
    return m_uiNoOfQMFDelaySamples;
}



bool SubBandDirSigsSynthesis::CoeffModification(    const std::vector<std::vector<FLOAT> > &p_rvvFOriginalSubBandDirSigsCompositionHOAFrame,
                                                    const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					                                const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				                                    const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                                                          bool p_bbDirSubBandPredInCurrentFrame, 
                                                          std::vector<std::vector<FLOAT> > &p_rvvFModifiedSubBandDirSigsCompositionHOAFrame)
{



    p_rvvFModifiedSubBandDirSigsCompositionHOAFrame.resize(m_uiNoOfHOACoeffs);

    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
    {
        p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));

        // add predicted directional sub band HOA component to output sum (for last samples) only if corresponding ambient HOA coefficient is not permanently active in current frame
        std::set<unsigned int>::const_iterator suiItActivePredCoeffIndex =  p_rsuiNonEnDisAbleActHOACoeffIndices.find( uiCoeffIdx + 1 );

        if ( suiItActivePredCoeffIndex == p_rsuiNonEnDisAbleActHOACoeffIndices.end() )
		{
            for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
                p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFOriginalSubBandDirSigsCompositionHOAFrame[uiCoeffIdx][uiSampleIdx];
            }
        }

        // add predicted directional sub band HOA component to output sum (for first samples) only if corresponding ambient HOA coefficient was not permanently active in last frame
        std::set<unsigned int>::const_iterator suiItActivePredCoeffIndexInLastFrame =  m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.find( uiCoeffIdx + 1 );

        if ( suiItActivePredCoeffIndexInLastFrame == m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.end() )
		{
            for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			{
                 p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFOriginalSubBandDirSigsCompositionHOAFrame[uiCoeffIdx][uiSampleIdx];
            }
        }

    }

    // fade out predicted directional sub band HOA coefficients IF ambient HOA coeffients are enabled and directional sub band prediction has been performed in current frame
	if ( p_bbDirSubBandPredInCurrentFrame )
	{
		for (std::set<unsigned int>::iterator suiIt = p_rsuiAmbCoeffIndicesToBeEnabled.begin(); suiIt != p_rsuiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxForPredHOAToBeFadedOut = *suiIt - 1;
			for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdxForPredHOAToBeFadedOut][uiSampleIdx] *= m_vFFadeOutWin[uiSampleIdx-m_uiNoOfQMFDelaySamples];
			}
		}
	}


	if (m_bDirSubBandPredInLastFrame)
	{
        // fade in predicted directional sub band HOA coefficients IF ambient HOA coeffients are disabled and directional sub band prediction has been performed in last frame
		for (std::set<unsigned int>::iterator suiIt = p_rsuiAmbCoeffIndicesToBeDisabled.begin(); suiIt != p_rsuiAmbCoeffIndicesToBeDisabled.end(); suiIt++)
		{
			unsigned int uiCoeffIdxForPredHOAToBeFadedIn = *suiIt - 1;
			for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdxForPredHOAToBeFadedIn][uiSampleIdx] *= m_vFFadeInWin[uiSampleIdx-m_uiNoOfQMFDelaySamples];
			}

		}

        for (std::set<unsigned int>::iterator suiIt = m_suiAmbCoeffIndicesToBeEnabledInLastFrame.begin(); suiIt != m_suiAmbCoeffIndicesToBeEnabledInLastFrame.end(); suiIt++)
        {
            unsigned int uiCoeffIdxForPredHOAToBeFadedOut = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			{
				p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdxForPredHOAToBeFadedOut][uiSampleIdx] *= m_vFFadeOutWin[m_uiFrameSize-m_uiNoOfQMFDelaySamples+uiSampleIdx];
			}
        }

	}

    if (m_bDirSubBandPredInPenultimateFrame)
    {
        for (std::set<unsigned int>::iterator suiIt = m_suiAmbCoeffIndicesToBeDisabledInLastFrame.begin(); suiIt != m_suiAmbCoeffIndicesToBeDisabledInLastFrame.end(); suiIt++)
		{
            unsigned int uiCoeffIdxForPredHOAToBeFadedIn = *suiIt - 1;
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			{
				p_rvvFModifiedSubBandDirSigsCompositionHOAFrame[uiCoeffIdxForPredHOAToBeFadedIn][uiSampleIdx] *= m_vFFadeInWin[m_uiFrameSize-m_uiNoOfQMFDelaySamples+uiSampleIdx];
			}
        }
    }

    // buffer indices of active amb. HOA coefficient indices to be enabled, disabled or none of both
    m_suiAmbCoeffIndicesToBeEnabledInLastFrame      = p_rsuiAmbCoeffIndicesToBeEnabled;
    m_suiAmbCoeffIndicesToBeDisabledInLastFrame     = p_rsuiAmbCoeffIndicesToBeDisabled;
    m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame  = p_rsuiNonEnDisAbleActHOACoeffIndices;

    // buffer indication of apllication of dir. sub band predicition
    m_bDirSubBandPredInPenultimateFrame = m_bDirSubBandPredInLastFrame;
    m_bDirSubBandPredInLastFrame        = p_bbDirSubBandPredInCurrentFrame;

    return false;
}
