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
 $Id: PARDecoder.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#pragma once
#include "PARDecoder.h"

// constructor 
PARDecoder::PARDecoder( )
{
		m_bIsInit = false;
}

bool PARDecoder::init(  const std::vector<unsigned int> & p_rvuiParUpmixHoaOrdersPerParSubBand,
                        const std::vector<unsigned int> & p_rvuiParSubbandWidths,
                        unsigned int p_uiFrameSize,
                        std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
    m_vuiParUpmixHoaOrdersPerParSubBand = p_rvuiParUpmixHoaOrdersPerParSubBand;

    m_uiNoOfSubbandGroups = m_vuiParUpmixHoaOrdersPerParSubBand.size();

    if (m_uiNoOfSubbandGroups > 0)
    {
        m_iMaxPARHOAOrder = static_cast<int>(*std::max_element(m_vuiParUpmixHoaOrdersPerParSubBand.begin(), m_vuiParUpmixHoaOrdersPerParSubBand.end()));
    }
    else
    {
        m_iMaxPARHOAOrder = -1;
    }

    m_uiMaxNoOfPARUpmixSignals = static_cast<unsigned int>((m_iMaxPARHOAOrder+1)*(m_iMaxPARHOAOrder+1));

    m_uiFrameSize = p_uiFrameSize;

    m_bUsePARInPenultimateFrame = false;
    m_bUsePARInPrevFrame = false;

    // compute number of upmix signals per sub-band group
    m_vuiParNoOfUpmixSignalsPerParSubBand.resize(m_uiNoOfSubbandGroups);

    for (unsigned int uiSubbandGroupIdx=0; uiSubbandGroupIdx < m_uiNoOfSubbandGroups; uiSubbandGroupIdx++)
	{
        m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx] = (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1)*
                                                                    (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1);
    }

    // init sub band configuration for PAR
    if ( p_shptrTabVals->getSubBandConfiguration( p_rvuiParSubbandWidths, 
                                                  m_vvuiParQMFSubBandIndices) ) 
	{    
        return true;
    }

    // init QMF analysises and synthesises
    QMFLIB_HYBRID_FILTER_MODE eHybridMode = QMFLIB_HYBRID_OFF; 

    m_shptrQMFAnalysis = std::shared_ptr<CqmfAnalysis>( new CqmfAnalysis() );
    m_shptrQMFAnalysis->init(m_uiMaxNoOfPARUpmixSignals, m_uiFrameSize, eHybridMode);

    m_shptrQMFSynthesis = std::shared_ptr<CqmfSynthesis>( new CqmfSynthesis() );
    m_shptrQMFSynthesis->init(m_uiMaxNoOfPARUpmixSignals, m_uiFrameSize, eHybridMode);

    m_uiQMFSamples    = m_shptrQMFAnalysis->getNumFreqSamples();
    unsigned int uiNoOfFreqBands = m_shptrQMFAnalysis->getNumOfFreqBands();
    m_uiNoOfQMFDelaySamples      = m_shptrQMFAnalysis->getAnalSynthFilterDelay();

    // init de-correlation filters
    unsigned int uiFilterIndices[9] = {0, 1, 2, 3, 4, 5, 7, 8, 9};

    m_vvvshptrAllPARDecorrs.resize(m_uiNoOfSubbandGroups);

    for( unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx< m_uiNoOfSubbandGroups; uiSubBandGroupIdx++)
    {
        m_vvvshptrAllPARDecorrs[uiSubBandGroupIdx].resize( m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx].size() );

        for( unsigned int uiQMFIdx = 0; uiQMFIdx < m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx].size(); uiQMFIdx++)
        {
            for ( unsigned int uiUpmixSigIdx=0; uiUpmixSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx]; uiUpmixSigIdx++)
            {
                std::shared_ptr<CparDecorrelator> tmpParDecorr ( new CparDecorrelator());
                tmpParDecorr->init(uiFilterIndices[uiUpmixSigIdx]  );
                m_vvvshptrAllPARDecorrs[uiSubBandGroupIdx][uiQMFIdx].push_back(tmpParDecorr);
            }
        }
    }

    // init table for potential permutations and their inverses for de-correlation
    if ( p_shptrTabVals->getPARPermIdxVectorTable(m_vvuiPARPermIdxVectorTableOrderTwo, 
       m_vvuiPARInversePermIdxVectorTableOrderTwo, 2) )
    {
        return true;
    }
    if ( p_shptrTabVals->getPARPermIdxVectorTable(m_vvuiPARPermIdxVectorTableOrderOne, 
       m_vvuiPARInversePermIdxVectorTableOrderOne, 1) )
    {
        return true;
    }

    // init vector with indices in increasing order 
    if ( m_uiNoOfSubbandGroups > 0)
    {
        m_vuiIncreasingIndicesVec.resize( m_uiNoOfSubbandGroups );

        for(unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx< m_uiNoOfSubbandGroups; uiSubBandGroupIdx++)
        {
            m_vuiIncreasingIndicesVec[uiSubBandGroupIdx] = uiSubBandGroupIdx;
        }
    }

    // init fading vectors
    std::vector<FLOAT> vFQMFCrossFadeWin;

    if ( computeFadeWinForDirBasedSynth(vFQMFCrossFadeWin, m_uiQMFSamples) )
    {
        return true;
    }

    m_vFQMFFadeInWin.assign( vFQMFCrossFadeWin.begin(), vFQMFCrossFadeWin.begin() + m_uiQMFSamples);
    m_vFQMFFadeOutWin.assign( vFQMFCrossFadeWin.begin() + m_uiQMFSamples, vFQMFCrossFadeWin.end());

    // init (inverse) mode matrices for (inverse) spatial transform
    m_vvvFAllSpatTransfInvModeMats.resize( std::max(m_iMaxPARHOAOrder,0) );
    m_vvvFAllInvSpatTransfModeMats.resize( std::max(m_iMaxPARHOAOrder,0) );

    for (int iOrder=1; iOrder <= m_iMaxPARHOAOrder; iOrder++)
    {
        if ( p_shptrTabVals->getInvModeMat( m_vvvFAllSpatTransfInvModeMats[iOrder-1] , iOrder ) )
        {
            return true;
        }
        if ( p_shptrTabVals->getModeMat( m_vvvFAllInvSpatTransfModeMats[iOrder-1] , iOrder ) )
        {
            return true;
        }
    }

    // init internal buffers

        // for QMF analysis
        std::vector<std::complex<FLOAT> > vcFTmpFreqSamplesOfOneBand(m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
        std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpFreqSamplesOfAllBands( uiNoOfFreqBands, vcFTmpFreqSamplesOfOneBand);

        m_vvvcFQMFPrelimHOABuffer.assign( m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands );

        // for computation of sparse HOA representation
        m_vvcFTmpQMFSparseHOABuffer.assign(m_uiMaxNoOfPARUpmixSignals, vcFTmpFreqSamplesOfOneBand);

        // for spatial transform 
        m_vvcFTmpQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vcFTmpFreqSamplesOfOneBand);

        // for de-correlated signals
        m_vvcFTmpQMFDecorrInputSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vcFTmpFreqSamplesOfOneBand);
        m_vvcFTmpQMFDecorrOutputSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vcFTmpFreqSamplesOfOneBand);

        // indices for selection of assignment of virtual loudspeaker signals to de-correlators
        m_vuiOldPARDecorrSigsSelectionTableIndices.assign(m_uiNoOfSubbandGroups, 0 );

        // mixing matrices
        m_vvvcFOldPARMixingMats.resize(m_uiNoOfSubbandGroups);

        for (unsigned int uiSubbandGroupIdx=0;uiSubbandGroupIdx < m_uiNoOfSubbandGroups; uiSubbandGroupIdx++)
        {
            m_vvvcFOldPARMixingMats[uiSubbandGroupIdx].resize(m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]);

            for( unsigned int uiSigIdx=0; uiSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiSigIdx++)
            {
                m_vvvcFOldPARMixingMats[uiSubbandGroupIdx][uiSigIdx].assign( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx], 
                                                                             std::complex<FLOAT>(static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)) );
            }
        }

        // for PAR QMF HOA representation
        m_vvvcFQMFPARHOABuffer.assign( m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands );

        // for PAR time domain preliminary non-faded representation
        m_vvFNonFadedPreliminaryPARHOABuffer.assign( m_uiMaxNoOfPARUpmixSignals, std::vector<FLOAT>(m_uiFrameSize, static_cast<FLOAT>(0.0)) ); 

        // for coeff. modification
        m_suiAmbCoeffIndicesToBeEnabledInLastFrame.clear();
        m_suiAmbCoeffIndicesToBeDisabledInLastFrame.clear();
        m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.clear();

        std::vector<FLOAT> vFTimeDomCrossFadeWin;

        if ( computeFadeWinForDirBasedSynth(vFTimeDomCrossFadeWin, m_uiFrameSize) )
        {
            return true;
        }

        m_vFFadeInWin.assign( vFTimeDomCrossFadeWin.begin(), vFTimeDomCrossFadeWin.begin() + m_uiFrameSize);
        m_vFFadeOutWin.assign( vFTimeDomCrossFadeWin.begin() + m_uiFrameSize, vFTimeDomCrossFadeWin.end());

    return false;
}


bool PARDecoder::process(const std::vector<std::vector<FLOAT> > & p_rvvFInputPrelimComposedHOACoeffsFrame,
                         const std::vector< std::vector< std::vector< std::complex<FLOAT> > > > & p_rvvvcFPARMixingMats,
                         const std::vector< unsigned int >  &p_rvuiPARDecorrSigsSelectionTableIndices,
                         const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					     const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				         const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                               bool p_bUsePAR,
                               std::vector<std::vector<FLOAT> > & p_rvvFOutputPARHOACoeffsFrame)
{
    // apply QMF analysis filter bank
    m_shptrQMFAnalysis->process(p_rvvFInputPrelimComposedHOACoeffsFrame, m_vvvcFQMFPrelimHOABuffer);

    // init HOA PAR QMF frame to zero
    for (unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx < m_uiNoOfSubbandGroups; uiSubBandGroupIdx++)
    {
        unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx];

        for (unsigned int uiBandIdx = 0; uiBandIdx < m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx].size(); uiBandIdx++)
        {
            unsigned int uiQMFIdx = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx][uiBandIdx];

            for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < uiCurrNoOfUpmixSigs; uiHOACoeffIdx++)
            {
                m_vvvcFQMFPARHOABuffer[uiHOACoeffIdx][uiQMFIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), 
                                                                                                                static_cast<FLOAT>(0.0) ));
            }
        }
    }

    // compute set of indices of all active/transmitted HOA coefficients
    std::set<unsigned int> suiActiveHOACoeffIndices;
    std::set<unsigned int> suiTmp;

    setUnion(suiTmp, p_rsuiAmbCoeffIndicesToBeEnabled, p_rsuiAmbCoeffIndicesToBeDisabled );
    setUnion(suiActiveHOACoeffIndices, suiTmp, p_rsuiNonEnDisAbleActHOACoeffIndices);

    // for each sub-band group
    for (unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx < m_uiNoOfSubbandGroups; uiSubBandGroupIdx++)
    {
        unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx];
        
        const std::vector< std::vector< FLOAT> >  & vvFCurrSpatTransfInvModeMat = m_vvvFAllSpatTransfInvModeMats[m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx]-1];
        const std::vector< std::vector< FLOAT> >  & vvFCurrInvSpatTransfModeMat = m_vvvFAllInvSpatTransfModeMats[m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx]-1];

        unsigned int * puiCurrPermVec = &(m_vuiIncreasingIndicesVec[0]);
        unsigned int * puiOldPermVec  = &(m_vuiIncreasingIndicesVec[0]);
        unsigned int * puiCurrInvPermVec = &(m_vuiIncreasingIndicesVec[0]);
        unsigned int * puiOldInvPermVec  = &(m_vuiIncreasingIndicesVec[0]);

        if ( m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx] == 2)
        {
            unsigned int uiCurrDecorrSigsSelectionTableIdx = p_rvuiPARDecorrSigsSelectionTableIndices[uiSubBandGroupIdx];
            unsigned int uiOldDecorrSigsSelectionTableIdx  = m_vuiOldPARDecorrSigsSelectionTableIndices[uiSubBandGroupIdx];
            puiCurrPermVec = &(m_vvuiPARPermIdxVectorTableOrderTwo[uiCurrDecorrSigsSelectionTableIdx][0]);
            puiOldPermVec  = &(m_vvuiPARPermIdxVectorTableOrderTwo[uiOldDecorrSigsSelectionTableIdx][0]);
            puiCurrInvPermVec = &(m_vvuiPARInversePermIdxVectorTableOrderTwo[uiCurrDecorrSigsSelectionTableIdx][0]);
            puiOldInvPermVec  = &(m_vvuiPARInversePermIdxVectorTableOrderTwo[uiOldDecorrSigsSelectionTableIdx][0]);
        }
        else if( m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx] == 1)
        {
            unsigned int uiCurrDecorrSigsSelectionTableIdx = p_rvuiPARDecorrSigsSelectionTableIndices[uiSubBandGroupIdx];
            unsigned int uiOldDecorrSigsSelectionTableIdx  = m_vuiOldPARDecorrSigsSelectionTableIndices[uiSubBandGroupIdx];
            puiCurrPermVec = &(m_vvuiPARPermIdxVectorTableOrderOne[uiCurrDecorrSigsSelectionTableIdx][0]);
            puiOldPermVec  = &(m_vvuiPARPermIdxVectorTableOrderOne[uiOldDecorrSigsSelectionTableIdx][0]);
            puiCurrInvPermVec = &(m_vvuiPARInversePermIdxVectorTableOrderOne[uiCurrDecorrSigsSelectionTableIdx][0]);
            puiOldInvPermVec  = &(m_vvuiPARInversePermIdxVectorTableOrderOne[uiOldDecorrSigsSelectionTableIdx][0]);
        }
        else
           return true;

        // for each QMF band in sub-band group
        for (unsigned int uiBandIdx = 0; uiBandIdx < m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx].size(); uiBandIdx++)
        {
            unsigned int uiQMFIdx = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx][uiBandIdx];

            // compute sparse HOA representation
            m_vvcFTmpQMFSparseHOABuffer.resize(uiCurrNoOfUpmixSigs);

            for (unsigned int uiIdx=0; uiIdx < uiCurrNoOfUpmixSigs; uiIdx++)
            {
                m_vvcFTmpQMFSparseHOABuffer[uiIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));
            }

            for (std::set<unsigned int>::iterator suiIt = suiActiveHOACoeffIndices.begin(); suiIt != suiActiveHOACoeffIndices.end(); suiIt++)
            {
                unsigned int uiCurrHOACoeffIdx = *suiIt;
                if(uiCurrHOACoeffIdx <= uiCurrNoOfUpmixSigs)
                {
                   m_vvcFTmpQMFSparseHOABuffer[uiCurrHOACoeffIdx-1].assign( m_vvvcFQMFPrelimHOABuffer[uiCurrHOACoeffIdx-1][uiQMFIdx].begin(), 
                                                                            m_vvvcFQMFPrelimHOABuffer[uiCurrHOACoeffIdx-1][uiQMFIdx].end() ) ;
                }
            }

            // perform spatial transform
            m_vvcFTmpQMFVirtLdspkSigsBuffer.resize(uiCurrNoOfUpmixSigs);

            for (unsigned int uiSpatSigIdx=0; uiSpatSigIdx < uiCurrNoOfUpmixSigs; uiSpatSigIdx++)
            {
                m_vvcFTmpQMFVirtLdspkSigsBuffer[uiSpatSigIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));

                for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                {
                    for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < uiCurrNoOfUpmixSigs; uiHOACoeffIdx++)
                    {
                        m_vvcFTmpQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiSampleIdx] += vvFCurrSpatTransfInvModeMat[uiSpatSigIdx][uiHOACoeffIdx]*
                                                                                        m_vvcFTmpQMFSparseHOABuffer[uiHOACoeffIdx][uiSampleIdx];
                    }
                }
            }

            // compute input signals to de-correlators
            m_vvcFTmpQMFDecorrInputSigsBuffer.resize(uiCurrNoOfUpmixSigs);

            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
            {
                m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx].resize(m_uiQMFSamples);

                for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                {
                    m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx][uiSampleIdx] = 
                        m_vvcFTmpQMFVirtLdspkSigsBuffer[puiCurrInvPermVec[uiDecorrSigIdx]][uiSampleIdx] * m_vFQMFFadeInWin[uiSampleIdx] +
                            m_vvcFTmpQMFVirtLdspkSigsBuffer[ puiOldInvPermVec[uiDecorrSigIdx] ][uiSampleIdx] * m_vFQMFFadeOutWin[uiSampleIdx];
                }
            }

            // apply de-correlators
            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
            {
                m_vvvshptrAllPARDecorrs[uiSubBandGroupIdx][uiBandIdx][uiDecorrSigIdx]->process( m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx], 
                                                                                                m_vvcFTmpQMFDecorrOutputSigsBuffer[uiDecorrSigIdx] );
            }

            // compute contribution to HOA PAR QMF frame only if PAR is used in current frame or was used in previous frame
            if (p_bUsePAR | m_bUsePARInPrevFrame )
            {
                // ambience replication by multiplication with mixing matrices using overlap add
                m_vvcFTmpQMFUpmixSigsBuffer.resize(uiCurrNoOfUpmixSigs);

                for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < uiCurrNoOfUpmixSigs; uiUpMixSigIdx++)
                {
                    std::vector< std::complex<FLOAT> > vcFTmpOldContrib( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) )  );
                    std::vector< std::complex<FLOAT> > vcFTmpCurrContrib( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) )  );

                    if (m_bUsePARInPrevFrame)
                    {
                        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                        {
                            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
                            {   
                                vcFTmpOldContrib[uiSampleIdx] += m_vvvcFOldPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiDecorrSigIdx]*
                                                                    m_vvcFTmpQMFDecorrOutputSigsBuffer[puiOldPermVec[uiDecorrSigIdx]][uiSampleIdx];
                            }
                            vcFTmpOldContrib[uiSampleIdx]*=m_vFQMFFadeOutWin[uiSampleIdx];
                        }
                    }

                    if(p_bUsePAR)
                    {
                        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                        {
                            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
                            {
                                vcFTmpCurrContrib[uiSampleIdx] += p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiDecorrSigIdx]*
                                                                    m_vvcFTmpQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiDecorrSigIdx]][uiSampleIdx];
                            }
                            vcFTmpCurrContrib[uiSampleIdx] *= m_vFQMFFadeInWin[uiSampleIdx];
                        }
                    }

                    m_vvcFTmpQMFUpmixSigsBuffer[uiUpMixSigIdx].resize(m_uiQMFSamples);

                    for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                    {
                        m_vvcFTmpQMFUpmixSigsBuffer[uiUpMixSigIdx][uiSampleIdx] = vcFTmpOldContrib[uiSampleIdx] + vcFTmpCurrContrib[uiSampleIdx];
                    }
                }

                // inverse spatial transform
                for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < uiCurrNoOfUpmixSigs; uiHOACoeffIdx++)
                {
                    m_vvvcFQMFPARHOABuffer[uiHOACoeffIdx][uiQMFIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), 
                                                                                                                    static_cast<FLOAT>(0.0) ));
                    for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                    {
                        for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < uiCurrNoOfUpmixSigs; uiUpMixSigIdx++)
                        {
                            m_vvvcFQMFPARHOABuffer[uiHOACoeffIdx][uiQMFIdx][uiSampleIdx] +=  vvFCurrInvSpatTransfModeMat[uiHOACoeffIdx][uiUpMixSigIdx]*
                                                                                                m_vvcFTmpQMFUpmixSigsBuffer[uiUpMixSigIdx][uiSampleIdx];
                        }
                    }
                }
            }
        }
    }

    // apply QMF synthesis filter bank
    m_shptrQMFSynthesis->process(m_vvvcFQMFPARHOABuffer, m_vvFNonFadedPreliminaryPARHOABuffer);

    // modify coefficient sequences of HOA representation of PAR component
    // i.e. set to zero, fade in or fade out
    // depending on state of coefficient sequences of ambient HOA component
    CoeffModification(  m_vvFNonFadedPreliminaryPARHOABuffer,
                        p_rsuiAmbCoeffIndicesToBeEnabled,
					              p_rsuiAmbCoeffIndicesToBeDisabled,
				                p_rsuiNonEnDisAbleActHOACoeffIndices,
                        p_bUsePAR, 
                        p_rvvFOutputPARHOACoeffsFrame);

    // update buffers
    m_vuiOldPARDecorrSigsSelectionTableIndices = p_rvuiPARDecorrSigsSelectionTableIndices;
    m_vvvcFOldPARMixingMats = p_rvvvcFPARMixingMats;
    m_bUsePARInPenultimateFrame = m_bUsePARInPrevFrame;
    m_bUsePARInPrevFrame    = p_bUsePAR;

    // buffer indices of active amb. HOA coefficient indices to be enabled, disabled or none of both
    m_suiAmbCoeffIndicesToBeEnabledInLastFrame      = p_rsuiAmbCoeffIndicesToBeEnabled;
    m_suiAmbCoeffIndicesToBeDisabledInLastFrame     = p_rsuiAmbCoeffIndicesToBeDisabled;
    m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame  = p_rsuiNonEnDisAbleActHOACoeffIndices;

    return false;
}


bool PARDecoder::CoeffModification(   const std::vector<std::vector<FLOAT> > &p_rvvFOriginalPARHOAFrame,
                                      const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					                            const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				                              const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                                            bool p_bbDoPARInCurrentFrame, 
                                            std::vector<std::vector<FLOAT> > &p_rvvFModifiedPARHOAFrame)
{
    p_rvvFModifiedPARHOAFrame.resize(m_uiMaxNoOfPARUpmixSignals);

    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiMaxNoOfPARUpmixSignals; uiCoeffIdx++)
    {
        p_rvvFModifiedPARHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));

        // add PAR HOA component to output sum (for last samples) only if corresponding ambient HOA coefficient is not permanently active in current frame
        std::set<unsigned int>::const_iterator suiItActivePredCoeffIndex =  p_rsuiNonEnDisAbleActHOACoeffIndices.find( uiCoeffIdx + 1 );

        if ( suiItActivePredCoeffIndex == p_rsuiNonEnDisAbleActHOACoeffIndices.end() )
		    {
            for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			      {
                p_rvvFModifiedPARHOAFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFOriginalPARHOAFrame[uiCoeffIdx][uiSampleIdx];
            }
        }

        // add PAR HOA component to output sum (for first samples) only if corresponding ambient HOA coefficient was not permanently active in last frame
        std::set<unsigned int>::const_iterator suiItActivePredCoeffIndexInLastFrame =  m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.find( uiCoeffIdx + 1 );

        if ( suiItActivePredCoeffIndexInLastFrame == m_suiNonEnDisAbleActHOACoeffIndicesInLastFrame.end() )
		    {
            for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			      {
                 p_rvvFModifiedPARHOAFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFOriginalPARHOAFrame[uiCoeffIdx][uiSampleIdx];
            }
        }

    }

    // fade out PAR HOA coefficients IF ambient HOA coeffients are enabled and PAR has been performed in current frame
	  if ( p_bbDoPARInCurrentFrame )
	  {
		    for (std::set<unsigned int>::iterator suiIt = p_rsuiAmbCoeffIndicesToBeEnabled.begin(); suiIt != p_rsuiAmbCoeffIndicesToBeEnabled.end(); suiIt++)
		    {
			      unsigned int uiCoeffIdxForPredHOAToBeFadedOut = *suiIt - 1;
			      for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			      {
				      p_rvvFModifiedPARHOAFrame[uiCoeffIdxForPredHOAToBeFadedOut][uiSampleIdx] *= m_vFFadeOutWin[uiSampleIdx-m_uiNoOfQMFDelaySamples];
			      }
		    }
	  }


	  if (m_bUsePARInPrevFrame)
	  {
        // fade in PAR HOA coefficients IF ambient HOA coeffients are disabled and PAR has been performed in last frame
		    for (std::set<unsigned int>::iterator suiIt = p_rsuiAmbCoeffIndicesToBeDisabled.begin(); suiIt != p_rsuiAmbCoeffIndicesToBeDisabled.end(); suiIt++)
		    {
			      unsigned int uiCoeffIdxForPredHOAToBeFadedIn = *suiIt - 1;
			      for (unsigned int uiSampleIdx=m_uiNoOfQMFDelaySamples; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			      {
				      p_rvvFModifiedPARHOAFrame[uiCoeffIdxForPredHOAToBeFadedIn][uiSampleIdx] *= m_vFFadeInWin[uiSampleIdx-m_uiNoOfQMFDelaySamples];
			      }
		    }

        for (std::set<unsigned int>::iterator suiIt = m_suiAmbCoeffIndicesToBeEnabledInLastFrame.begin(); suiIt != m_suiAmbCoeffIndicesToBeEnabledInLastFrame.end(); suiIt++)
        {
            unsigned int uiCoeffIdxForPredHOAToBeFadedOut = *suiIt - 1;
			      for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			      {
				        p_rvvFModifiedPARHOAFrame[uiCoeffIdxForPredHOAToBeFadedOut][uiSampleIdx] *= m_vFFadeOutWin[m_uiFrameSize-m_uiNoOfQMFDelaySamples+uiSampleIdx];
			      }
        }

	  }

    if (m_bUsePARInPenultimateFrame)
    {
        for (std::set<unsigned int>::iterator suiIt = m_suiAmbCoeffIndicesToBeDisabledInLastFrame.begin(); suiIt != m_suiAmbCoeffIndicesToBeDisabledInLastFrame.end(); suiIt++)
		    {
            unsigned int uiCoeffIdxForPredHOAToBeFadedIn = *suiIt - 1;
			      for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
			      {
				        p_rvvFModifiedPARHOAFrame[uiCoeffIdxForPredHOAToBeFadedIn][uiSampleIdx] *= m_vFFadeInWin[m_uiFrameSize-m_uiNoOfQMFDelaySamples+uiSampleIdx];
			      }
        }
    }

    return false;
}


