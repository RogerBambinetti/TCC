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
 $Id: SubbandDirEstimation.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#pragma once

#include "SubbandDirEstimation.h"

// constructor
SubbandDirEstimation::SubbandDirEstimation()
{
		m_bIsInit = false;
}

// init function
bool SubbandDirEstimation::init(    unsigned int p_uiFrameSize,
									unsigned int p_uiHOAOrder,
                                    unsigned int p_uiNoOfSamplesPerQMFBand,
                                    const std::vector<unsigned int> p_vuiSubBandWidths,
                                    unsigned int p_uiNoOfSubBandsForDirPred,
                                    unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                                    unsigned int p_uiNoOfQMFBands,
                                    unsigned int p_uiDirGridTableIdx,
                                    std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
	m_uiFrameSize				= p_uiFrameSize;
	m_uiHOAOrder				= p_uiHOAOrder;
	
    m_uiNoOfSamplesPerQMFBand       = p_uiNoOfSamplesPerQMFBand;
    m_uiNoOfSubBandsForDirPred      = p_uiNoOfSubBandsForDirPred;
    m_uiMaxNoOfPredDirsPerSubBand   = p_uiMaxNoOfPredDirsPerSubBand;
    m_uiNoOfQMFBands                = p_uiNoOfQMFBands;

    // dependent variables
    m_uiNoOfHOACoeffs			= (p_uiHOAOrder+1)*(p_uiHOAOrder+1);

     // init sub band configuration for directional sub band predicition
    if ( p_shptrTabVals->getSubBandConfiguration( p_vuiSubBandWidths,
                                                  m_vvuiQMFSubBandIndices) ) 
	{    
        return true;
    }

    // get mode matrix for fine grid points
	if ( p_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, p_uiDirGridTableIdx) )
	{
		return true;
	}

    m_uiNoOfSearchDirs = m_vvFTransposeModeMatAllFineGridPoints.size();

    // init temporary vectors
    std::vector< std::complex<FLOAT> > vcFZeroVec( m_uiNoOfHOACoeffs, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
    m_vvcFZeroMat.assign(m_uiNoOfHOACoeffs, vcFZeroVec);
    m_vvvcFSubBandCovarMats.assign(m_uiNoOfSubBandsForDirPred, m_vvcFZeroMat);

    std::vector<FLOAT> vFTmpZeroVecForDirs( m_uiNoOfSearchDirs, static_cast<FLOAT>(0.0) );

    m_vvFSubBandDirectionalPowerDistributions.assign( m_uiNoOfSubBandsForDirPred, vFTmpZeroVecForDirs );

    std::vector< std::complex<FLOAT> > vcFZeroSamplesVec( m_uiNoOfSamplesPerQMFBand, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
    std::vector<std::vector<std::complex<FLOAT> > > vvcFZeroSamplesMat( m_uiNoOfQMFBands, vcFZeroSamplesVec);

    m_vvvcFFreqDomHOAOldFrame.assign(m_uiNoOfHOACoeffs, vvcFZeroSamplesMat);


	m_bIsInit = true;

	return false;
}

bool SubbandDirEstimation::process(  const std::vector<std::vector<FLOAT> >                               &p_rvvFHOAInputBuffer,
                                     const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomHOAInputBuffer,
                                           std::set<unsigned int>                                         &p_rsuiAllActiveDirAndGridDirIncidesForSubbandPred,
                                           std::vector<std::map<unsigned int, unsigned int> >             &p_rvmuiActiveSubBandDirAndGridDirIncides    )
{
    //// this is a very simple and non-intelligent algorithm to determine the directions with only one single direction per sub band
    
    // note that direction estimation is assumed to be done on two successive frames
    p_rsuiAllActiveDirAndGridDirIncidesForSubbandPred.clear();

    // determine the direction in each sub band from the maximum of the directional power distribution
    m_vvvcFSubBandCovarMats.assign(m_uiNoOfSubBandsForDirPred, m_vvcFZeroMat);

    for (unsigned int uiSubBandIdx=0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
    {
        // clear sub band related directions map
        p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].clear();

        // compute correlation matrix of HOA coefficients for each sub band
        for (unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
        {
            for (unsigned int uiColIdx=uiRowIdx; uiColIdx < m_uiNoOfHOACoeffs; uiColIdx++)
            {
                // add contribution of each QMF band in sub band
                for (unsigned int uiQMFIdx = m_vvuiQMFSubBandIndices[uiSubBandIdx][0]; 
                        uiQMFIdx < m_vvuiQMFSubBandIndices[uiSubBandIdx][0] + m_vvuiQMFSubBandIndices[uiSubBandIdx].size(); uiQMFIdx++)
                {
                    // for each sample in QMF band 
                    for (unsigned int uiQMFSampleIdx=0; uiQMFSampleIdx < m_uiNoOfSamplesPerQMFBand ; uiQMFSampleIdx++)
                    {
                        m_vvvcFSubBandCovarMats[uiSubBandIdx][uiRowIdx][uiColIdx] += 
                        p_rvvvcFFreqDomHOAInputBuffer[uiRowIdx][uiQMFIdx][uiQMFSampleIdx]*
                        std::conj(p_rvvvcFFreqDomHOAInputBuffer[uiColIdx][uiQMFIdx][uiQMFSampleIdx]);

                        m_vvvcFSubBandCovarMats[uiSubBandIdx][uiRowIdx][uiColIdx] +=
                        m_vvvcFFreqDomHOAOldFrame[uiRowIdx][uiQMFIdx][uiQMFSampleIdx]*
                        std::conj(m_vvvcFFreqDomHOAOldFrame[uiColIdx][uiQMFIdx][uiQMFSampleIdx]);
                    }
                }

                // exploit hermitian property of correlation matrix
                m_vvvcFSubBandCovarMats[uiSubBandIdx][uiColIdx][uiRowIdx] = std::conj(m_vvvcFSubBandCovarMats[uiSubBandIdx][uiRowIdx][uiColIdx]);
            }
        }

        // compute directional power distribution for each sub band
        m_vvFSubBandDirectionalPowerDistributions[uiSubBandIdx].assign( m_uiNoOfSearchDirs, static_cast<FLOAT>(0.0) );

        for (unsigned int uiDirGridIdx=0; uiDirGridIdx < m_uiNoOfSearchDirs; uiDirGridIdx++)
        {
            for (unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
            {
                for (unsigned int uiColIdx=0; uiColIdx < m_uiNoOfHOACoeffs; uiColIdx++)
                {
                    m_vvFSubBandDirectionalPowerDistributions[uiSubBandIdx][uiDirGridIdx] += std::real(
                        m_vvFTransposeModeMatAllFineGridPoints[uiDirGridIdx][uiRowIdx]*
                        m_vvvcFSubBandCovarMats[uiSubBandIdx][uiRowIdx][uiColIdx]*
                        m_vvFTransposeModeMatAllFineGridPoints[uiDirGridIdx][uiColIdx]);
                }
            }
        }

        // compute maximum of directional power distribution
        FLOAT FTmpMaxVal;
        unsigned int uiTmpMaxIdx;

        computeMaxAbsVal(m_vvFSubBandDirectionalPowerDistributions[uiSubBandIdx], FTmpMaxVal, uiTmpMaxIdx);

        // insert direction into map
        p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].insert( std::pair<unsigned int, unsigned int>(1, uiTmpMaxIdx+1) );

        // insert direction into set of all directions
        p_rsuiAllActiveDirAndGridDirIncidesForSubbandPred.insert(uiTmpMaxIdx+1);

    }

    // buffer QMF HOA frame

    m_vvvcFFreqDomHOAOldFrame = p_rvvvcFFreqDomHOAInputBuffer;


    return false;
}
