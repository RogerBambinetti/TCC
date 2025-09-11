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
 $Id: FrameParams.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#include "FrameParams.h"

FrameParams::FrameParams(   unsigned int p_uiTotalNoOfPercCoders, 
                            unsigned int p_uiNoOfHOACoeffs, 
                            unsigned int p_uiMaxNofOfDirSigsForPred,
                            unsigned int p_uiMaxNoOfTransmittedHOACoeffs,
                            unsigned int p_uiNoOfSubBandsForDirPred,
                            unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                            const std::vector<unsigned int> & p_rvuiParUpmixHoaOrdersPerParSubBand)
{
	m_vbExceptions.assign( p_uiTotalNoOfPercCoders, false);
	m_viExponents.assign( p_uiTotalNoOfPercCoders, 0);

	m_vuiAmbHOAAssignmentVec.assign(p_uiTotalNoOfPercCoders, 0);

	m_muiActiveDirAndGridDirIndices.clear();
	m_muivFVectors.clear();

	m_vuiPredictionTypeVec.assign( p_uiNoOfHOACoeffs, 0);

    std::vector<std::complex<FLOAT> > vcFTmpRowVec ( p_uiMaxNoOfTransmittedHOACoeffs, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)) );
    std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpMat ( p_uiMaxNoOfPredDirsPerSubBand, vcFTmpRowVec);
    m_vvvcFComplexDirPredictionCoeffsMats.assign(p_uiNoOfSubBandsForDirPred, vvcFTmpMat);

    m_bDoDirSubBandPred = false;

    m_vmuiActiveSubBandDirAndGridDirIncides.resize( p_uiNoOfSubBandsForDirPred );

    for (unsigned int uiSubBandIdx=0; uiSubBandIdx < p_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
    {
        m_vmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].clear();
    }

    m_suiAllActiveDirAndGridDirIncidesForSubbandPred.clear();

    // for PAR
    unsigned int uiNoOfPARSubbandGroups = p_rvuiParUpmixHoaOrdersPerParSubBand.size();

    m_vuiParDecorrSigsSelectionTableIndices.assign(uiNoOfPARSubbandGroups, 0);
    m_vvvcFPARMixingMats.resize(uiNoOfPARSubbandGroups);

    for (unsigned int uiSubbandGroupIdx=0;uiSubbandGroupIdx < uiNoOfPARSubbandGroups; uiSubbandGroupIdx++)
    {
        unsigned int uiCurrNoOfUpmixSigs = (p_rvuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1)*
                                           (p_rvuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1);

        m_vvvcFPARMixingMats[uiSubbandGroupIdx].resize(uiCurrNoOfUpmixSigs);

        for( unsigned int uiSigIdx=0; uiSigIdx < uiCurrNoOfUpmixSigs; uiSigIdx++)
        {
            m_vvvcFPARMixingMats[uiSubbandGroupIdx][uiSigIdx].assign( uiCurrNoOfUpmixSigs, std::complex<FLOAT>(static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)) );
        }
    }

	std::vector<unsigned int> vuiTmpRowVecForPredIndices( p_uiNoOfHOACoeffs, 0);
	m_vvuiPredictionIndicesMat.assign(p_uiMaxNofOfDirSigsForPred, vuiTmpRowVecForPredIndices);

	std::vector<int> viTmpRowVec( p_uiNoOfHOACoeffs, 0);
	m_vviQuantizedPredictionFactorsMat.assign( p_uiMaxNofOfDirSigsForPred, viTmpRowVec);

	m_suiAmbCoeffIndicesToBeEnabled.clear();
	m_suiAmbCoeffIndicesToBeDisabled.clear();
	m_suiNonEnDisAbleActHOACoeffIndices.clear();
}
