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
 $Id: PAREncoder.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#pragma once
#include "PAREncoder.h"
#include <numeric>
#include <functional>
#include <limits>

// constructor 
PAREncoder::PAREncoder( )
{
		m_bIsInit = false;
}

bool PAREncoder::init(  const std::vector<unsigned int> & p_rvuiParUpmixHoaOrdersPerParSubBand,
                        const std::vector<unsigned int> & p_rvuiParSubbandWidths,
                        const std::vector<bool> & p_rvbUseRealCoeffsPerParSubband,
                        unsigned int p_uiFrameSize,
                        std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
    m_vuiParUpmixHoaOrdersPerParSubBand = p_rvuiParUpmixHoaOrdersPerParSubBand;
    m_vbUseRealCoeffsPerParSubband      = p_rvbUseRealCoeffsPerParSubband;

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

    // compute number of upmix signals per sub-band group
    m_vuiParNoOfUpmixSignalsPerParSubBand.resize(m_uiNoOfSubbandGroups);

    for (unsigned int uiSubbandGroupIdx=0; uiSubbandGroupIdx < m_uiNoOfSubbandGroups; uiSubbandGroupIdx++)
	{
        m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx] = (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1)*
                                                                    (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1);
    }

    // init sub band configuration for directional sub band predicition
    if ( p_shptrTabVals->getSubBandConfiguration( p_rvuiParSubbandWidths, 
                                                  m_vvuiParQMFSubBandIndices) ) 
	{    
        return true;
    }

    // init QMF analysises and synthesises
    QMFLIB_HYBRID_FILTER_MODE eHybridMode = QMFLIB_HYBRID_OFF; 

    m_shptrQMFAnalysisForOrigHOA = std::shared_ptr<CqmfAnalysis>( new CqmfAnalysis() );
    m_shptrQMFAnalysisForCompHOA = std::shared_ptr<CqmfAnalysis>( new CqmfAnalysis() );

    m_shptrQMFAnalysisForOrigHOA->init(m_uiMaxNoOfPARUpmixSignals, m_uiFrameSize, eHybridMode);
    m_shptrQMFAnalysisForCompHOA->init(m_uiMaxNoOfPARUpmixSignals, m_uiFrameSize, eHybridMode);


    m_uiQMFSamples    = m_shptrQMFAnalysisForOrigHOA->getNumFreqSamples();
    unsigned int uiNoOfFreqBands = m_shptrQMFAnalysisForOrigHOA->getNumOfFreqBands();
    m_uiNoOfQMFDelaySamples      = m_shptrQMFAnalysisForOrigHOA->getAnalSynthFilterDelay();

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
                tmpParDecorr->init( uiFilterIndices[uiUpmixSigIdx]  );
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

    // init mode matrices for spatial transform
    m_vvvFAllSpatTransfInvModeMats.resize( std::max(m_iMaxPARHOAOrder,0) );

    for (int iOrder=1; iOrder <= m_iMaxPARHOAOrder; iOrder++)
    {
        if ( p_shptrTabVals->getInvModeMat( m_vvvFAllSpatTransfInvModeMats[iOrder-1] , iOrder ) )
        {
            return true;
        }
    }

    // init internal buffers

        //// for QMF analysis
        std::vector<std::complex<FLOAT> > vcFTmpFreqSamplesOfOneBand(m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
        std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpFreqSamplesOfAllBands( uiNoOfFreqBands, vcFTmpFreqSamplesOfOneBand);

        m_vvvcFQMFOrigHOABuffer.assign( m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands );
        m_vvvcFQMFCompHOABuffer.assign( m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands );

        //// for computation of sparse HOA representation
        m_vvcFTmpQMFSparseOrigHOABuffer.assign(m_uiMaxNoOfPARUpmixSignals, vcFTmpFreqSamplesOfOneBand);

        // for spatial transform 
        m_vvvcFOrigQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);
        m_vvvcFOldOrigQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);
        m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);
        m_vvvcFCompQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);
        m_vvvcFOldCompQMFVirtLdspkSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);

        //// for de-correlated signals
        m_vvvcFQMFDecorrOutputSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);
        m_vvvcFOldQMFDecorrOutputSigsBuffer.assign(m_uiMaxNoOfPARUpmixSignals, vvcFTmpFreqSamplesOfAllBands);

        // indices for selection of assignment of virtual loudspeaker signals to de-correlators
        m_vuiOldPARDecorrSigsSelectionTableIndices.assign(m_uiNoOfSubbandGroups, 0 );

    return false;
}

bool PAREncoder::process(   const std::vector<std::vector<FLOAT> > & p_rvvFInputOrigHOAFrame,
                        const std::vector<std::vector<FLOAT> > & p_rvvFInputComposedHOAFrame,
                        const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					    const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				        const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                            bool & p_rbUse_PAR,
                            std::vector< std::vector< std::vector< std::complex<FLOAT> > > > & p_rvvvcFPARMixingMats,
                            std::vector< unsigned int >  &p_rvuiPARDecorrSigsSelectionTableIndices)
{
    // apply QMF analysis filter bank for original HOA representation
    m_shptrQMFAnalysisForOrigHOA->process(p_rvvFInputOrigHOAFrame, m_vvvcFQMFOrigHOABuffer);

    // apply QMF analysis filter bank for composed HOA representation
    m_shptrQMFAnalysisForCompHOA->process(p_rvvFInputComposedHOAFrame, m_vvvcFQMFCompHOABuffer);

    //// perform spatial transform for original and composed HOA representation
    p_rvuiPARDecorrSigsSelectionTableIndices.resize(m_uiNoOfSubbandGroups);

    p_rvvvcFPARMixingMats.resize(m_uiNoOfSubbandGroups);

    p_rbUse_PAR = false;

    // for each sub-band group
    for (unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx < m_uiNoOfSubbandGroups; uiSubBandGroupIdx++)
    {
        // PART 1 //

        unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx];
        
        const std::vector< std::vector< FLOAT> >  & vvFCurrSpatTransfInvModeMat = m_vvvFAllSpatTransfInvModeMats[m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx]-1];

        unsigned int uiNoOfSubBandsInGroup = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx].size();

        // for each QMF band in sub-band group
        for (unsigned int uiBandIdx = 0; uiBandIdx < uiNoOfSubBandsInGroup; uiBandIdx++)
        {
            unsigned int uiQMFIdx = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx][uiBandIdx];

            // apply truncation and coefficient selection to original HOA representation
            m_vvcFTmpQMFSparseOrigHOABuffer.resize(uiCurrNoOfUpmixSigs);

            for (unsigned int uiIdx=0; uiIdx < uiCurrNoOfUpmixSigs; uiIdx++)
            {
                m_vvcFTmpQMFSparseOrigHOABuffer[uiIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));
            }

            std::set<unsigned int> suiActAmbHOACoeffIndices;

            setUnion( suiActAmbHOACoeffIndices, p_rsuiAmbCoeffIndicesToBeEnabled, p_rsuiAmbCoeffIndicesToBeDisabled);
            setUnion( suiActAmbHOACoeffIndices, suiActAmbHOACoeffIndices, p_rsuiNonEnDisAbleActHOACoeffIndices);

            for (std::set<unsigned int>::iterator suiIt = suiActAmbHOACoeffIndices.begin(); suiIt != suiActAmbHOACoeffIndices.end(); suiIt++)
            {
                unsigned int uiCurrHOACoeffIdx = *suiIt;
                if(uiCurrHOACoeffIdx < uiCurrNoOfUpmixSigs)
                   m_vvcFTmpQMFSparseOrigHOABuffer[uiCurrHOACoeffIdx-1].assign( m_vvvcFQMFOrigHOABuffer[uiCurrHOACoeffIdx-1][uiQMFIdx].begin(), 
                                                                                m_vvvcFQMFOrigHOABuffer[uiCurrHOACoeffIdx-1][uiQMFIdx].end() ) ;
            }

            for (unsigned int uiSpatSigIdx=0; uiSpatSigIdx < uiCurrNoOfUpmixSigs; uiSpatSigIdx++)
            {
                m_vvvcFOrigQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));
                m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));
                m_vvvcFCompQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx].assign( m_uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ));
                
                for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                {
                    for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < uiCurrNoOfUpmixSigs; uiHOACoeffIdx++)
                    {
                        m_vvvcFOrigQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx][uiSampleIdx] += vvFCurrSpatTransfInvModeMat[uiSpatSigIdx][uiHOACoeffIdx]*
                                                                                                    m_vvvcFQMFOrigHOABuffer[uiHOACoeffIdx][uiQMFIdx][uiSampleIdx];
                        m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx][uiSampleIdx] += vvFCurrSpatTransfInvModeMat[uiSpatSigIdx][uiHOACoeffIdx]*
                                                                                                    m_vvcFTmpQMFSparseOrigHOABuffer[uiHOACoeffIdx][uiSampleIdx];
                        m_vvvcFCompQMFVirtLdspkSigsBuffer[uiSpatSigIdx][uiQMFIdx][uiSampleIdx] += vvFCurrSpatTransfInvModeMat[uiSpatSigIdx][uiHOACoeffIdx]*
                                                                                                    m_vvvcFQMFCompHOABuffer[uiHOACoeffIdx][uiQMFIdx][uiSampleIdx];
                    }
                }
            }
        }

        // determine the number of decorrelated signals to be used for mixing of upmix signals
        // from m_vvvcFOrigQMFVirtLdspkSigsBuffer and m_vvvcFCompQMFVirtLdspkSigsBuffer
        // Here for simplicity it is assumed that this number is equal to 4, which corresponds to a table index of zero
        p_rvuiPARDecorrSigsSelectionTableIndices[uiSubBandGroupIdx] = 0;

        // PART 2 //
        
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
        else if ( m_vuiParUpmixHoaOrdersPerParSubBand[uiSubBandGroupIdx] == 1)
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
        for (unsigned int uiBandIdx = 0; uiBandIdx < uiNoOfSubBandsInGroup; uiBandIdx++)
        {
            unsigned int uiQMFIdx = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx][uiBandIdx];

            // compute input signals to de-correlators
            m_vvcFTmpQMFDecorrInputSigsBuffer.resize(uiCurrNoOfUpmixSigs);

            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
            {
                m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx].resize(m_uiQMFSamples);

                for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiQMFSamples; uiSampleIdx++)
                {
                    m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx][uiSampleIdx] = 
                        m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer[puiCurrInvPermVec[uiDecorrSigIdx]][uiQMFIdx][uiSampleIdx] * m_vFQMFFadeInWin[uiSampleIdx] +
                        m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer[puiOldInvPermVec[uiDecorrSigIdx] ][uiQMFIdx][uiSampleIdx] * m_vFQMFFadeOutWin[uiSampleIdx];
                }
            }

            // apply de-correlators
            for (unsigned int uiDecorrSigIdx=0; uiDecorrSigIdx < uiCurrNoOfUpmixSigs; uiDecorrSigIdx++)
            {
                m_vvvshptrAllPARDecorrs[uiSubBandGroupIdx][uiBandIdx][uiDecorrSigIdx]->process( m_vvcFTmpQMFDecorrInputSigsBuffer[uiDecorrSigIdx], 
                                                                                                m_vvvcFQMFDecorrOutputSigsBuffer[uiDecorrSigIdx][uiQMFIdx] );
            }
        }

        // compute mixing matrix for current sub band group
        // this is just a very simple and non-optimal computation,
        // where the mixing matrix is assumed to be diagonal
        p_rvvvcFPARMixingMats[uiSubBandGroupIdx].resize(uiCurrNoOfUpmixSigs);

        std::vector< std::complex<FLOAT> > vcFInnerProds(uiCurrNoOfUpmixSigs, std::complex<FLOAT> ( static_cast<FLOAT>(0.0),static_cast<FLOAT>(0.0) ) );
        std::vector< std::complex<FLOAT> > vcFCrossProds(uiCurrNoOfUpmixSigs, std::complex<FLOAT> ( static_cast<FLOAT>(0.0),static_cast<FLOAT>(0.0) ) );

        std::vector<std::complex<FLOAT>> vcFCurrResidualFrame(m_uiQMFSamples);

        for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < uiCurrNoOfUpmixSigs; uiUpMixSigIdx++)
        {
            for (unsigned int uiBandIdx = 0; uiBandIdx < uiNoOfSubBandsInGroup; uiBandIdx++)
            {
                unsigned int uiQMFIdx = m_vvuiParQMFSubBandIndices[uiSubBandGroupIdx][uiBandIdx];

               

                vcFInnerProds[uiUpMixSigIdx] = std::inner_product(  m_vvvcFQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(), 
                                                                    m_vvvcFQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].end(),
                                                                    m_vvvcFQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(),
                                                                    vcFInnerProds[uiUpMixSigIdx],
                                                                    std::plus<std::complex<FLOAT>>(),
                                                                    complexConjugateProduct);

                vcFInnerProds[uiUpMixSigIdx] = std::inner_product(  m_vvvcFOldQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(), 
                                                                    m_vvvcFOldQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].end(),
                                                                    m_vvvcFOldQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(),
                                                                    vcFInnerProds[uiUpMixSigIdx],
                                                                    std::plus<std::complex<FLOAT>>(),
                                                                    complexConjugateProduct);

                for (unsigned int uiQMFSampleIdx=0; uiQMFSampleIdx < m_uiQMFSamples; uiQMFSampleIdx++)
                {
                    vcFCurrResidualFrame[uiQMFSampleIdx] = m_vvvcFOrigQMFVirtLdspkSigsBuffer[uiUpMixSigIdx][uiQMFIdx][uiQMFSampleIdx] - 
                                                        m_vvvcFCompQMFVirtLdspkSigsBuffer[uiUpMixSigIdx][uiQMFIdx][uiQMFSampleIdx];
                }

                vcFCrossProds[uiUpMixSigIdx] = std::inner_product(  vcFCurrResidualFrame.begin(),
                                                                    vcFCurrResidualFrame.end(),
                                                                    m_vvvcFQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(), 
                                                                    vcFCrossProds[uiUpMixSigIdx],
                                                                    std::plus<std::complex<FLOAT>>(),
                                                                    complexConjugateProduct);

                 for (unsigned int uiQMFSampleIdx=0; uiQMFSampleIdx < m_uiQMFSamples; uiQMFSampleIdx++)
                {
                    vcFCurrResidualFrame[uiQMFSampleIdx] = m_vvvcFOldOrigQMFVirtLdspkSigsBuffer[uiUpMixSigIdx][uiQMFIdx][uiQMFSampleIdx] - 
                                                        m_vvvcFOldCompQMFVirtLdspkSigsBuffer[uiUpMixSigIdx][uiQMFIdx][uiQMFSampleIdx];
                }

                vcFCrossProds[uiUpMixSigIdx] = std::inner_product(  vcFCurrResidualFrame.begin(),
                                                                    vcFCurrResidualFrame.end(),
                                                                    m_vvvcFOldQMFDecorrOutputSigsBuffer[puiCurrPermVec[uiUpMixSigIdx]][uiQMFIdx].begin(), 
                                                                    vcFCrossProds[uiUpMixSigIdx],
                                                                    std::plus<std::complex<FLOAT>>(),
                                                                    complexConjugateProduct);
            }

            if ( std::abs(vcFInnerProds[uiUpMixSigIdx]) < std::numeric_limits<FLOAT>::min() )
            {
                p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] = std::complex<FLOAT> ( static_cast<FLOAT>(0.0),static_cast<FLOAT>(0.0) );
            }
            else
            {
                p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] = vcFCrossProds[uiUpMixSigIdx]/vcFInnerProds[uiUpMixSigIdx];

                // costrain the magnitude to be not greater than 0.9
                FLOAT FCurrentMagnitude = std::abs( p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] );
                FLOAT FCurrentAngle = std::arg( p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] );

                if ( m_vbUseRealCoeffsPerParSubband[uiSubBandGroupIdx] )
                {
                    p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] = FCurrentMagnitude;
                }
                else
                {
                    p_rvvvcFPARMixingMats[uiSubBandGroupIdx][uiUpMixSigIdx][uiUpMixSigIdx] = FCurrentMagnitude * std::complex<FLOAT>( std::cos(FCurrentAngle), std::sin(FCurrentAngle)  );
                }
                

                if ( (p_rbUse_PAR == false) & ( FCurrentMagnitude > std::numeric_limits<FLOAT>::min()) )
                {
                    p_rbUse_PAR = true;
                }
            }
        }

    }

    // update buffers
    m_vuiOldPARDecorrSigsSelectionTableIndices = p_rvuiPARDecorrSigsSelectionTableIndices;
    m_vvvcFOldQMFDecorrOutputSigsBuffer = m_vvvcFQMFDecorrOutputSigsBuffer;
    m_vvvcFOldOrigQMFVirtLdspkSigsBuffer = m_vvvcFOrigQMFVirtLdspkSigsBuffer;
    m_vvvcFOldCompQMFVirtLdspkSigsBuffer = m_vvvcFCompQMFVirtLdspkSigsBuffer;

    return false;
}
