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
 $Id: DirSubBandHOASynthesis.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/


#include "DirSubBandHOASynthesis.h"

// constructor
DirSubBandHOASynthesis::DirSubBandHOASynthesis()
{
	m_bIsInit = false;
}


bool DirSubBandHOASynthesis::init(  unsigned int p_uiNoOfSamplesPerQMFBand,
                                    const std::vector<unsigned int> & p_rvuiSubBandWidths,
                                    unsigned int p_uiNoOfSubBandsForDirPred,
                                    unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                                    unsigned int p_uiNoOfQMFBands,
                                    unsigned int p_uiHOAOrder,
                                    unsigned int p_uiMaxNoOfTransmittedHOACoeffs,
                                    unsigned int p_uiDirGridTableIdx,
                                    std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
    if ( p_uiNoOfSamplesPerQMFBand == 0)
    {
        return true;
    }
    else
    {
        m_uiNoOfSamplesPerQMFBand = p_uiNoOfSamplesPerQMFBand;
    }

    m_uiNoOfSubBandsForDirPred      = p_uiNoOfSubBandsForDirPred;
    m_uiMaxNoOfPredDirsPerSubBand   = p_uiMaxNoOfPredDirsPerSubBand;
    m_uiNoOfQMFBands                = p_uiNoOfQMFBands;
    m_uiHOAOrder                    = p_uiHOAOrder;
    m_uiMaxNoOfTransmittedHOACoeffs = p_uiMaxNoOfTransmittedHOACoeffs;

    m_uiNoOfHOACoeffs = (m_uiHOAOrder+1)*(m_uiHOAOrder+1);

    m_vmuiLastActiveSubBandDirAndGridDirIncides.resize(p_uiNoOfSubBandsForDirPred);

    for (unsigned int uiSubBandIdx=0; uiSubBandIdx  < p_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
    {
        m_vmuiLastActiveSubBandDirAndGridDirIncides[uiSubBandIdx].clear();
    }

    std::vector<std::complex<FLOAT> > vcFTmpRowVec ( p_uiMaxNoOfTransmittedHOACoeffs, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)) );
    std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpMat ( p_uiMaxNoOfPredDirsPerSubBand, vcFTmpRowVec);
    m_vvvcFLastComplexDirPredictionCoeffsMat.assign(p_uiNoOfSubBandsForDirPred, vvcFTmpMat);

    // init sub band configuration for directional sub band predicition
    if ( p_shptrTabVals->getSubBandConfiguration( p_rvuiSubBandWidths, 
                                                  m_vvuiQMFSubBandIndices) ) 
	{    
        return true;
    }

    // get mode matrix for fine grid points
	if ( p_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, p_uiDirGridTableIdx) )
	{
		return true;
	}

    std::vector<FLOAT> vFQMFCrossFadeWin;

    if ( computeFadeWinForDirBasedSynth(vFQMFCrossFadeWin, m_uiNoOfSamplesPerQMFBand) )
    {
        return true;
    }

    m_vFQMFFadeInWin.assign( vFQMFCrossFadeWin.begin(), vFQMFCrossFadeWin.begin() + m_uiNoOfSamplesPerQMFBand);
    m_vFQMFFadeOutWin.assign( vFQMFCrossFadeWin.begin() + m_uiNoOfSamplesPerQMFBand, vFQMFCrossFadeWin.end());

    m_bDoDirSubBandPredInPrevFrame = false;

    m_bIsInit = true;

    return false;
}

bool DirSubBandHOASynthesis::process( const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomTruncHOAInputBuffer,
                                      const std::vector<std::map<unsigned int, unsigned int> >             &p_rvmuiActiveSubBandDirAndGridDirIncides,
                                      const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFComplexDirPredictionCoeffsMat,
                                            bool p_bDoDirSubBandPred,
						                    std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomPredDirHOAOutputBuffer)
{

    // init output samples by zeros
    p_rvvvcFFreqDomPredDirHOAOutputBuffer.resize(m_uiNoOfHOACoeffs);

    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
    {
        p_rvvvcFFreqDomPredDirHOAOutputBuffer[uiCoeffIdx].resize( m_uiNoOfQMFBands );

        for (unsigned int uiQMFBandIdx=0; uiQMFBandIdx < m_uiNoOfQMFBands; uiQMFBandIdx++)
        {
            p_rvvvcFFreqDomPredDirHOAOutputBuffer[uiCoeffIdx][uiQMFBandIdx].assign( m_uiNoOfSamplesPerQMFBand, 
                                                                                std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
        }
    }

    // for each sub band
    for (unsigned int uiSubBandIdx=0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
    {
        // compute contribution for each sub band direction active in last frame
        for (std::map<unsigned int, unsigned int>::const_iterator muiIt = m_vmuiLastActiveSubBandDirAndGridDirIncides[uiSubBandIdx].begin(); 
                muiIt != m_vmuiLastActiveSubBandDirAndGridDirIncides[uiSubBandIdx].end(); muiIt++)
	    {
            unsigned int uiCurrDirIdx	  = muiIt->first;
		    unsigned int uiCurrGridDirIdx = muiIt->second;

            // for each QMF band in sub band
            for (unsigned int uiQMFIdx = m_vvuiQMFSubBandIndices[uiSubBandIdx][0]; 
                              uiQMFIdx < m_vvuiQMFSubBandIndices[uiSubBandIdx][0] + m_vvuiQMFSubBandIndices[uiSubBandIdx].size(); uiQMFIdx++)
            {
                // for each sample in QMF band
                for (unsigned int uiQMFSampleIdx=0; uiQMFSampleIdx < m_uiNoOfSamplesPerQMFBand ; uiQMFSampleIdx++)
                {
                    // init predicted QMF sample to zero
                    std::complex<FLOAT> cFCurrPredFreqDomSample = std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) ; 

                    // add contributions from all HOA coefficients to prediction
                    for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < m_uiMaxNoOfTransmittedHOACoeffs; uiHOACoeffIdx++)
                    {
                        cFCurrPredFreqDomSample += m_vvvcFLastComplexDirPredictionCoeffsMat[uiSubBandIdx][uiCurrDirIdx-1][uiHOACoeffIdx] 
                                                    * p_rvvvcFFreqDomTruncHOAInputBuffer[uiHOACoeffIdx][uiQMFIdx][uiQMFSampleIdx];
                    }

                    // apply fading out window
                    cFCurrPredFreqDomSample *= m_vFQMFFadeOutWin[uiQMFSampleIdx];

                    // multiply predicted signal by mode vector with respect to corresponding direction
                    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
                    {
                            p_rvvvcFFreqDomPredDirHOAOutputBuffer[uiCoeffIdx][uiQMFIdx][uiQMFSampleIdx] 
                             += m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx-1][uiCoeffIdx] * cFCurrPredFreqDomSample;
                    }
                }
            }
        
        }

        if (p_bDoDirSubBandPred)
        {
            // compute contribution for each sub band direction active in current frame
            for (std::map<unsigned int, unsigned int>::const_iterator muiIt = p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].begin(); 
                    muiIt != p_rvmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].end(); muiIt++)
	        {
                unsigned int uiCurrDirIdx	  = muiIt->first;
		        unsigned int uiCurrGridDirIdx = muiIt->second;

                // for each QMF band in sub band
                for (unsigned int uiQMFIdx = m_vvuiQMFSubBandIndices[uiSubBandIdx][0]; 
                                  uiQMFIdx < m_vvuiQMFSubBandIndices[uiSubBandIdx][0] + m_vvuiQMFSubBandIndices[uiSubBandIdx].size(); uiQMFIdx++)
                {
                    // for each sample in QMF band
                    for (unsigned int uiQMFSampleIdx=0; uiQMFSampleIdx < m_uiNoOfSamplesPerQMFBand ; uiQMFSampleIdx++)
                    {
                        // init predicted QMF sample to zero
                        std::complex<FLOAT> cFCurrPredFreqDomSample = std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) ; 

                        // add contributions from all HOA coefficients to prediction
                        for (unsigned int uiHOACoeffIdx=0; uiHOACoeffIdx < m_uiMaxNoOfTransmittedHOACoeffs; uiHOACoeffIdx++)
                        {
                            cFCurrPredFreqDomSample += p_rvvvcFComplexDirPredictionCoeffsMat[uiSubBandIdx][uiCurrDirIdx-1][uiHOACoeffIdx] 
                                                        * p_rvvvcFFreqDomTruncHOAInputBuffer[uiHOACoeffIdx][uiQMFIdx][uiQMFSampleIdx];
                        }

                        // apply fading in window
                        cFCurrPredFreqDomSample *= m_vFQMFFadeInWin[uiQMFSampleIdx];

                        // multiply predicted signal by mode vector with respect to corresponding direction
                        for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
                        {
                                p_rvvvcFFreqDomPredDirHOAOutputBuffer[uiCoeffIdx][uiQMFIdx][uiQMFSampleIdx] 
                                 += m_vvFTransposeModeMatAllFineGridPoints[uiCurrGridDirIdx-1][uiCoeffIdx] * cFCurrPredFreqDomSample;
                        }
                    }
                }
            }
        }
    }

    // update buffers
    m_bDoDirSubBandPredInPrevFrame = p_bDoDirSubBandPred;

    // buffer grid indices for sub band directions
    m_vmuiLastActiveSubBandDirAndGridDirIncides = p_rvmuiActiveSubBandDirAndGridDirIncides;

    // buffer prediction coefficient matrices
    m_vvvcFLastComplexDirPredictionCoeffsMat = p_rvvvcFComplexDirPredictionCoeffsMat;

    

    return false;
}