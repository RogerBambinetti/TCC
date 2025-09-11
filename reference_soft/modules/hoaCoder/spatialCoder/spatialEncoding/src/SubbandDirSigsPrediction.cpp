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
 $Id: SubbandDirSigsPrediction.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#pragma once

#include "SubbandDirSigsPrediction.h"

// constructor
SubbandDirSigsPrediction::SubbandDirSigsPrediction()
{
		m_bIsInit = false;
}

bool SubbandDirSigsPrediction::init(    unsigned int p_uiFrameSize,
									    unsigned int p_uiHOAOrder,
                                        unsigned int p_uiMaxNoOfTransmittedHOACoeffs,
                                        const std::vector<unsigned int> p_vuiSubBandWidths,
                                        unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                                        unsigned int p_uiDirGridTableIdx,
                                        unsigned int p_uiFirstSBRSubbandIdx,
                                        std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
    m_uiFrameSize				= p_uiFrameSize;
	m_uiHOAOrder				= p_uiHOAOrder;
    m_uiNoOfSubBandsForDirPred  = p_vuiSubBandWidths.size();
    m_uiMaxNoOfTransmittedHOACoeffs = p_uiMaxNoOfTransmittedHOACoeffs;
    m_uiMaxNoOfPredDirsPerSubBand   = p_uiMaxNoOfPredDirsPerSubBand;
    m_uiFirstSBRSubbandIdx          = p_uiFirstSBRSubbandIdx;

    // dependent variables
    m_uiNoOfHOACoeffs			= (p_uiHOAOrder+1)*(p_uiHOAOrder+1);

    // get mode matrix for fine grid points
	if ( p_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, p_uiDirGridTableIdx) )
	{
		return true;
	}

    // init sub band configuration for directional sub band predicition
    if ( p_shptrTabVals->getSubBandConfiguration( p_vuiSubBandWidths,
                                                  m_vvuiQMFSubBandIndices) ) 
	{    
        return true;
    }


    std::vector<std::complex<FLOAT> > vcFTmpRowVec ( m_uiMaxNoOfTransmittedHOACoeffs, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)) );
    m_vvcFZeroCoeffMat.assign( p_uiMaxNoOfPredDirsPerSubBand, vcFTmpRowVec);

    m_vFTmpModeVec.assign( m_uiMaxNoOfTransmittedHOACoeffs, static_cast<FLOAT>(0.0) );
    m_vFTmpPInvModeVec.assign( m_uiMaxNoOfTransmittedHOACoeffs, static_cast<FLOAT>(0.0) );

    m_suiOldActiveAmbHOACoeffsIndices.clear();

    m_bIsInit = true;

    return false;
}


bool SubbandDirSigsPrediction::process(   const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomTruncHOAInputBuffer,
                                          const std::set<unsigned int>                                         &p_rsuiAllActiveDirAndGridDirIncidesForSubbandPred,
                                          const std::vector<std::map<unsigned int, unsigned int> >             &p_rvmuiActiveSubBandDirAndGridDirIncides,
                                          const std::set<unsigned int>                                         &p_rsuiActiveAmbHOACoeffsIndices,
                                              std::vector< std::vector< std::vector< std::complex<FLOAT> > > > &p_rvvvcFComplexDirPredictionCoeffsMat)
{
    // NOTE THAT THE FOLLOWING COMPUTATION OF COMPLEX PREDICTION COEFFICIENTS IS NOT OPTIMAL IN ANY SENCE
    // IT SERVES JUST FOR DEMONSTRATION PURPOSES

    // compute intersection index set between active indices of current frame and old frame
    std::set<unsigned int> suiIntersectOldAndCurrentActiveIndices;
    setIntersect(suiIntersectOldAndCurrentActiveIndices, p_rsuiActiveAmbHOACoeffsIndices, m_suiOldActiveAmbHOACoeffsIndices );

    // compute for each sub band the prediction coefficients
    for (unsigned int uiSubBandIdx=0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
    {
        // initialize all prediction coefficient by zero
        p_rvvvcFComplexDirPredictionCoeffsMat[uiSubBandIdx] = m_vvcFZeroCoeffMat;

        for (std::map<unsigned int, unsigned int>::const_iterator muicIt = p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].begin();
                muicIt != p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].end(); muicIt++)
        {
            unsigned int uiDirIdx     = muicIt->first - 1;
            unsigned int uiDirGridIdx = muicIt->second - 1;

            // get mode vector whose elements are only non-zero if the corresponding ambient HOA coefficient is active
            m_vFTmpModeVec.assign(m_uiMaxNoOfTransmittedHOACoeffs, static_cast<FLOAT>(0.0));
            for (std::set<unsigned int>::const_iterator suicIt = suiIntersectOldAndCurrentActiveIndices.begin();
                        suicIt != suiIntersectOldAndCurrentActiveIndices.end(); suicIt++)
            {
                unsigned int uiActAmbCoeffIdx = *suicIt - 1;

                m_vFTmpModeVec[uiActAmbCoeffIdx] = m_vvFTransposeModeMatAllFineGridPoints[uiDirGridIdx][uiActAmbCoeffIdx];

                if ( uiSubBandIdx >= m_uiFirstSBRSubbandIdx)
                {
                    m_vFTmpModeVec[uiActAmbCoeffIdx] = std::abs(m_vFTmpModeVec[uiActAmbCoeffIdx]);
                    break;
                }
            }


            // compute pseudo inverse
            computePseudoInverseOfVec(m_vFTmpModeVec, m_vFTmpPInvModeVec);

            // set predicition coefficients for the direction of interest to pseudo inverse of mode vector
            for (std::set<unsigned int>::const_iterator suicIt = suiIntersectOldAndCurrentActiveIndices.begin();
                        suicIt != suiIntersectOldAndCurrentActiveIndices.end(); suicIt++)
            {
                unsigned int uiActAmbCoeffIdx = *suicIt - 1;

                p_rvvvcFComplexDirPredictionCoeffsMat[uiSubBandIdx][uiDirIdx][uiActAmbCoeffIdx] = m_vFTmpPInvModeVec[uiActAmbCoeffIdx];
            }
        }

    }

    // buffer old active HOA coeff. indices
    m_suiOldActiveAmbHOACoeffsIndices = p_rsuiActiveAmbHOACoeffsIndices;


    return false;
}
