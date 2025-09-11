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
 $Id: PAREncoder.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#pragma once

#include "QMF_Wrapper.h"
#include "par_decorr.h"
#include "FrameParams.h"
#include "TabulatedValues.h"
#include "CommonFunctions.h"
#include <memory>
#include <algorithm>

class PAREncoder
{
	public:

		PAREncoder();

		bool init(  const std::vector<unsigned int> & p_rvuiParUpmixHoaOrdersPerParSubBand,
                    const std::vector<unsigned int> & p_rvuiParSubbandWidths,
                    const std::vector<bool> & p_rvbUseRealCoeffsPerParSubband,
                    unsigned int p_uiFrameSize,
                    std::shared_ptr<TabulatedValues> p_shptrTabVals);

        bool process(   const std::vector<std::vector<FLOAT> > & p_rvvFInputOrigHOAFrame,
                        const std::vector<std::vector<FLOAT> > & p_rvvFInputComposedHOAFrame,
                        const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
					    const std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
				        const std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices,
                            bool & p_rbUse_PAR,
                            std::vector< std::vector< std::vector< std::complex<FLOAT> > > > & p_rvvvcFPARMixingMats,
                            std::vector< unsigned int >  &p_rvuiNosOfDecorrSigs);

    private:
        
		bool m_bIsInit;

        unsigned int m_uiNoOfSubbandGroups;
        int m_iMaxPARHOAOrder;
        unsigned int m_uiMaxNoOfPARUpmixSignals;

        unsigned int m_uiFrameSize;
        unsigned int m_uiQMFSamples;
        unsigned int m_uiNoOfQMFDelaySamples;

        std::vector<unsigned int>  m_vuiParUpmixHoaOrdersPerParSubBand;
        std::vector<unsigned int>  m_vuiParNoOfUpmixSignalsPerParSubBand;

        std::vector<bool>  m_vbUseRealCoeffsPerParSubband;

        std::vector<unsigned int>  m_vuiIncreasingIndicesVec;

        // table for potential permutations and their inverses for de-correlation
        std::vector<std::vector<unsigned int> > m_vvuiPARPermIdxVectorTableOrderTwo;
        std::vector<std::vector<unsigned int> > m_vvuiPARInversePermIdxVectorTableOrderTwo;
        std::vector<std::vector<unsigned int> > m_vvuiPARPermIdxVectorTableOrderOne;
        std::vector<std::vector<unsigned int> > m_vvuiPARInversePermIdxVectorTableOrderOne;

        // fading vectors
        std::vector<FLOAT> m_vFQMFFadeInWin;
		  std::vector<FLOAT> m_vFQMFFadeOutWin;

        // mode matrices for spatial transform
        std::vector< std::vector< std::vector< FLOAT> > > m_vvvFAllSpatTransfInvModeMats;

        // sub band configuration for PAR
        std::vector< std::vector< unsigned int> > m_vvuiParQMFSubBandIndices;

        // shared pointers for QMF analysis for original and composed HOA representations
		std::shared_ptr<CqmfAnalysis>  m_shptrQMFAnalysisForOrigHOA;
        std::shared_ptr<CqmfAnalysis>  m_shptrQMFAnalysisForCompHOA;

        // de-correlator instances
        std::vector< std::vector< std::vector< std::shared_ptr<CparDecorrelator> > > > m_vvvshptrAllPARDecorrs;

        // for QMF analysis
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  m_vvvcFQMFOrigHOABuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  m_vvvcFQMFCompHOABuffer;

        // for computation of sparse HOA representation
        std::vector<std::vector<std::complex<FLOAT> > > m_vvcFTmpQMFSparseOrigHOABuffer;

        // for spatial transform 
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFOrigQMFVirtLdspkSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFOldOrigQMFVirtLdspkSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFSparseOrigQMFVirtLdspkSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFCompQMFVirtLdspkSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFOldCompQMFVirtLdspkSigsBuffer;

        // for de-correlated signals
        std::vector<std::vector<std::complex<FLOAT> > > m_vvcFTmpQMFDecorrInputSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFQMFDecorrOutputSigsBuffer;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > > m_vvvcFOldQMFDecorrOutputSigsBuffer;

        // buffers
        std::vector< unsigned int> m_vuiOldPARDecorrSigsSelectionTableIndices;

};
