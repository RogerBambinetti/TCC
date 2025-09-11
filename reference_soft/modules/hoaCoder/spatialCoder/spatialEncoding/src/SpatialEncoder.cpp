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
 $Id: SpatialEncoder.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/


#include "SpatialEncoder.h"
#include <limits>


// constructor
SpatialEncoder::SpatialEncoder()
{
	m_bIsInit = false;
}


// init function
    bool SpatialEncoder::init(	unsigned int p_uiFrameSize, 
					            unsigned int p_uiHOAOrder, 
					            unsigned int p_uiMaxNoOfDirSigs, 
					            unsigned int p_uiMinOrderForAmbHOA, 
					            unsigned int p_uiTotalNoOfPercCoders,
					            unsigned int p_uiBitRatePerPercCoder,
					            unsigned int p_uiMaxNofOfDirSigsForPred,
					            unsigned int p_uiNoOfBitsPerScaleFactor,
					            unsigned int p_uiMaxAmplifyExponent,
					            unsigned int p_uiInterpSamples,
					            unsigned int p_uiInterpMethod,
					            unsigned int p_uiNoBitsForVecElemQuant,
					            unsigned int p_uiCodedVVecLength,
					            unsigned int p_uiMaxNoOfTransmittedHOACoeffs,
                           const std::vector<unsigned int> & p_rvuiSubBandWidths,
                           unsigned int p_uiDirGridTableIdx,
                           unsigned int p_uiMaxNoOfPredDirsPerSubBand,
                           unsigned int p_uiFirstSBRSubbandIdx,
                           const std::vector<unsigned int> & p_rvuiParUpmixHoaOrdersPerParSubBand,
                           const std::vector<unsigned int> & p_rvuiParSubbandWidths,
                           const std::vector<bool> & p_rvbUseRealCoeffsPerParSubband,
                           const HoaConfig &p_crAccessUnitHeader,
                           const HoaFrame &p_crHOAEmptyFrame)
{
	// init member variables
	m_uiFrameSize				        = p_uiFrameSize;
	m_uiHOAOrder				        = p_uiHOAOrder;
	m_uiMaxNoOfDirSigs			        = p_uiMaxNoOfDirSigs;
	m_uiMinOrderForAmbHOA		        = p_uiMinOrderForAmbHOA;
	m_uiTotalNoOfPercCoders             = p_uiTotalNoOfPercCoders;
	m_uiBitRatePerPercCoder             = p_uiBitRatePerPercCoder;
	m_uiMaxNofOfDirSigsForPred	        = p_uiMaxNofOfDirSigsForPred;
	m_uiNoOfBitsPerScaleFactor	        = p_uiNoOfBitsPerScaleFactor;
	m_uiMaxAmplifyExponent		        = p_uiMaxAmplifyExponent;
	m_uiNoBitsForVecElemQuant           = p_uiNoBitsForVecElemQuant;

   m_uiMaxNoOfTransmittedHOACoeffs     = p_uiMaxNoOfTransmittedHOACoeffs;
   m_vuiSubBandWidths = p_rvuiSubBandWidths;
   m_uiMaxNoOfPredDirsPerSubBand       = p_uiMaxNoOfPredDirsPerSubBand;
   m_uiNoOfSubBandsForDirPred          = p_rvuiSubBandWidths.size();

   m_vuiParSubBandWidths = p_rvuiParSubbandWidths;

   m_vuiParNoOfUpmixSignalsPerParSubBand.resize(m_vuiParSubBandWidths.size());

   for (unsigned int uiSubbandGroupIdx=0; uiSubbandGroupIdx < m_vuiParSubBandWidths.size(); uiSubbandGroupIdx++)
   {
      m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx] = (p_rvuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1)*
                                                                  (p_rvuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx]+1);
   }

   // for prediction of directional sub-band signals
   m_uiNoOfQuantIntervForAngleForDirPred = sizeof(g_pDecodedAngleDiffTable)/sizeof(HuffmanWord<int>);
   m_uiNoOfQuantIntervForMagnForDirPred  = sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>) - 2;

   m_iHighestQuantValForMagForDirPred = static_cast<int>(m_uiNoOfQuantIntervForMagnForDirPred/2);
   m_iHighestQuantValForAngleForDirPred  = static_cast<int>(m_uiNoOfQuantIntervForAngleForDirPred/2);
   m_iLowestQuantValForAngleForDirPred  = 1-m_iHighestQuantValForAngleForDirPred;

   m_FQuantIntLenForMagnForDirPred   = static_cast<FLOAT>( 2.0/static_cast<double>(m_uiNoOfQuantIntervForAngleForDirPred));
   m_FQuantIntLenForAngleForDirPred  = static_cast<FLOAT>( 2.0*static_cast<double>(OWN_PI)/static_cast<double>(m_uiNoOfQuantIntervForAngleForDirPred) );

   // for PAR
   m_uiNoOfQuantIntervForAngleForPAR = sizeof(g_pDecodedAngleDiffTable)/sizeof(HuffmanWord<int>);
   m_uiNoOfQuantIntervForMagnForPAR  = sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>) - 2;

   m_iHighestQuantValForMagForPAR = static_cast<int>(m_uiNoOfQuantIntervForMagnForPAR/2);
   m_iHighestQuantValForAngleForPAR = static_cast<int>(m_uiNoOfQuantIntervForAngleForPAR/2);
   m_iLowestQuantValForAngleForPAR = 1-m_iHighestQuantValForAngleForPAR;

   m_FQuantIntLenForMagnForPAR   = static_cast<FLOAT>( 2.0/static_cast<double>(m_uiNoOfQuantIntervForAngleForPAR));
   m_FQuantIntLenForAngleForPAR  = static_cast<FLOAT>( 2.0*static_cast<double>(OWN_PI)/static_cast<double>(m_uiNoOfQuantIntervForAngleForPAR) );

	m_bIsEndOfInputFile			= false;
	m_bIsEndOfOutputFile		= false;
	m_uiDelayFrameCounter		= 0;
	m_uiNoOfRequiredDelayFrames = 4;

	// init dependent variables
	m_uiMinNoOfCoeffsForAmbHOA = (m_uiMinOrderForAmbHOA + 1)*(m_uiMinOrderForAmbHOA + 1);
	m_uiNoOfHOACoeffs		   = (m_uiHOAOrder+1)*(m_uiHOAOrder+1);

	// compute the number of prediction indices to better code explicitly
	unsigned int uiNoOfBitsForSinglePredIdx = getCeilLog2( m_uiNoOfHOACoeffs );

	unsigned int uiNoOfBitsRequiredToCodeIndices = uiNoOfBitsForSinglePredIdx;
	m_uiNoOfPredIndToCodeExplicitly = 1;

	while ( uiNoOfBitsRequiredToCodeIndices < m_uiNoOfHOACoeffs )
	{
		m_uiNoOfPredIndToCodeExplicitly++;
		uiNoOfBitsRequiredToCodeIndices = m_uiNoOfPredIndToCodeExplicitly*uiNoOfBitsForSinglePredIdx + getCeilLog2(m_uiNoOfPredIndToCodeExplicitly);
	}

	m_uiNoOfPredIndToCodeExplicitly--;


	// compute preliminary number of channels with variable type
	int iPreliminaryNoOfAddPercCoders = static_cast<int>(p_uiTotalNoOfPercCoders - m_uiMinNoOfCoeffsForAmbHOA);

	// constrain the number of always transmitted coefficients of ambient HOA component to be not greater than the number of available coders
	if (iPreliminaryNoOfAddPercCoders < 0)
	{
		m_uiMinOrderForAmbHOA		= static_cast<unsigned int>( std::floor( sqrt ( std::max( 1.0, static_cast<double>(m_uiTotalNoOfPercCoders) ) ) ) - 1);
		m_uiMinNoOfCoeffsForAmbHOA	= (m_uiMinOrderForAmbHOA + 1)*(m_uiMinOrderForAmbHOA + 1);

		std::cout << "The number of always transmitted coefficients of ambient HOA component has been automatically corrected to a value of" << m_uiMinNoOfCoeffsForAmbHOA << "!" << std::endl;

		iPreliminaryNoOfAddPercCoders = static_cast<int>(p_uiTotalNoOfPercCoders - m_uiMinNoOfCoeffsForAmbHOA);
	}
	
	// set the number of channels with variable type
	m_uiNoOfAddPercCoders      = iPreliminaryNoOfAddPercCoders;

	// constrain maximum number of directional signals to be not greater than the number of channels with variable type
	if ( m_uiMaxNoOfDirSigs > m_uiNoOfAddPercCoders)
	{
		m_uiMaxNoOfDirSigs = m_uiNoOfAddPercCoders;
		std::cout << "Maximum number of directional signals has been automatically corrected to be not greater than the number of available additional perceptual coders, i.e. to a value of " << m_uiNoOfAddPercCoders << "!" << std::endl;
	}

	// init components //

	// init frame parameters
	m_shptrFrameParams = std::shared_ptr<FrameParams>( new FrameParams( m_uiTotalNoOfPercCoders, 
                                                                        m_uiNoOfHOACoeffs, 
                                                                        m_uiMaxNofOfDirSigsForPred,
                                                                        m_uiMaxNoOfTransmittedHOACoeffs,
                                                                        m_uiNoOfSubBandsForDirPred,
                                                                        m_uiMaxNoOfPredDirsPerSubBand,
                                                                        p_rvuiParUpmixHoaOrdersPerParSubBand));

	// init tabulated values
	m_shptrTabVals = std::shared_ptr<TabulatedValues> (new TabulatedValues());

	// init direction estimation
	m_shptrDirAndVecEst = std::shared_ptr<DirectionAndVectorEstimation> (new DirectionAndVectorEstimation() );
	m_shptrDirAndVecEst->init(	m_uiFrameSize,
								m_uiHOAOrder,
								m_uiMinNoOfCoeffsForAmbHOA,
								m_uiNoBitsForVecElemQuant,
								p_uiCodedVVecLength);

	// init HOA decomposition
	m_shptrHOADecomp = std::shared_ptr<HOADecomposition> (new HOADecomposition() );

	m_shptrHOADecomp->init(	m_uiHOAOrder, 
							m_uiFrameSize, 
							m_uiMaxNoOfDirSigs, 
							m_uiMinNoOfCoeffsForAmbHOA, 
							p_uiMaxNofOfDirSigsForPred, 
							p_uiNoOfBitsPerScaleFactor, 
							m_uiNoOfAddPercCoders, 
							m_uiBitRatePerPercCoder, 
							m_uiNoBitsForVecElemQuant,
							p_uiCodedVVecLength,
							p_uiInterpSamples, 
							p_uiInterpMethod,
							m_shptrTabVals);

    // init QMF analysis 
    QMFLIB_HYBRID_FILTER_MODE eHybridMode = QMFLIB_HYBRID_OFF;
    m_shptrQMFAnalysis = std::shared_ptr<CqmfAnalysis>( new CqmfAnalysis() );
    m_shptrQMFAnalysis->init(m_uiNoOfHOACoeffs, m_uiFrameSize, eHybridMode);

    unsigned int uiQMFSamples    = m_shptrQMFAnalysis->getNumFreqSamples();
    unsigned int uiNoOfFreqBands = m_shptrQMFAnalysis->getNumOfFreqBands();

    // init sub band direction estimation
    m_shptrSubBandDirEstimation = std::shared_ptr<SubbandDirEstimation> (new SubbandDirEstimation() );
    m_shptrSubBandDirEstimation->init(  m_uiFrameSize,
                                        m_uiHOAOrder,
                                        uiQMFSamples,
                                        m_vuiSubBandWidths,
                                        m_uiNoOfSubBandsForDirPred,
                                        m_uiMaxNoOfPredDirsPerSubBand,
                                        uiNoOfFreqBands,
                                        p_uiDirGridTableIdx,
                                        m_shptrTabVals );

    // init sub band directional signals prediction
    m_shptrSubBandDirSigsPred = std::shared_ptr<SubbandDirSigsPrediction>( new SubbandDirSigsPrediction() );
    m_shptrSubBandDirSigsPred->init(    m_uiFrameSize,
                                        m_uiHOAOrder,
                                        m_uiMaxNoOfTransmittedHOACoeffs,
                                        p_rvuiSubBandWidths,
                                        m_uiMaxNoOfPredDirsPerSubBand,
                                        p_uiDirGridTableIdx,
                                        p_uiFirstSBRSubbandIdx,
                                        m_shptrTabVals);

    // init spatial decoding

    // create access unit indicating no usage of PAR

    HoaConfig AccessUnitHeaderWithoutPAR(p_crAccessUnitHeader);

    AccessUnitHeaderWithoutPAR.m_vunParSubbandWidths.clear();
    AccessUnitHeaderWithoutPAR.m_unParSubBandTableIdx            = 0;
    AccessUnitHeaderWithoutPAR.m_unLastFirstOrderSubBandIdx      = 0;
    AccessUnitHeaderWithoutPAR.m_vunParUpmixHoaOrderPerParSubBandIdx.clear();    
    AccessUnitHeaderWithoutPAR.m_vbUseRealCoeffsPerParSubband .clear();

    m_shptrSpatDec = std::shared_ptr<SpatialDecoder> (new SpatialDecoder() );

    m_shptrSpatDec->init(AccessUnitHeaderWithoutPAR,  
       m_uiFrameSize);



    // init PAR encoder
    m_shptrPAREncoder = std::shared_ptr<PAREncoder> (new PAREncoder() );
    m_shptrPAREncoder->init(p_rvuiParUpmixHoaOrdersPerParSubBand, 
       p_rvuiParSubbandWidths,
       p_rvbUseRealCoeffsPerParSubband,
       m_uiFrameSize, 
       m_shptrTabVals);

    // init empty HOA frames
    m_PenultimateHOASIFrame = p_crHOAEmptyFrame;
    m_LastButTwoHOASIFrame  = p_crHOAEmptyFrame;

    // init ambient HOA component modification
    m_shptrAmbCompMod = std::shared_ptr<AmbientComponentModification> (new AmbientComponentModification() );
    m_shptrAmbCompMod->init(m_uiFrameSize, 
                            m_uiHOAOrder, 
                            p_uiMinOrderForAmbHOA, 
                            m_uiTotalNoOfPercCoders, 
                            m_uiNoOfAddPercCoders, 
                            m_uiBitRatePerPercCoder, 
                            m_shptrTabVals);

    // init channel assignment
    m_shptrChannAss = std::shared_ptr<ChannelAssignment> ( new ChannelAssignment() );
    m_shptrChannAss->init(  m_uiFrameSize, 
                            m_uiHOAOrder, 
                            m_uiTotalNoOfPercCoders, 
                            m_uiNoOfAddPercCoders);

    //init dynamic correction (vector of shared pointers to objects, size equal to number of channels)
    for (unsigned int uiIdx=0; uiIdx < m_uiTotalNoOfPercCoders; uiIdx++)
    {
       std::shared_ptr<DynCorrection> tmpDynCorr ( new DynCorrection());

       tmpDynCorr->init(m_uiFrameSize, m_uiHOAOrder, p_uiMaxAmplifyExponent);

       m_vshptrAllDynCorr.push_back(tmpDynCorr);
    }

    // init buffers
    std::vector<FLOAT> vFTmpSingleChannelBuffer ( m_uiFrameSize, static_cast<FLOAT>(0.0));

    // for zero input
    m_vvFZeroFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

    // for HOA decomposition
    m_vvFSmoothedPSSigsLastFrame.assign( m_uiMaxNoOfDirSigs,  vFTmpSingleChannelBuffer );
    m_vvFSmoothedPSSigsPenultimateFrame.assign( m_uiMaxNoOfDirSigs,  vFTmpSingleChannelBuffer );

    m_vvFPenultimateSmoothedPreDomSoundsHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer );
    m_vvFPenultimateAmbientHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer );
    m_vvFPredictedLastAmbientHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer );

    // for ambient HOA component modification
    m_vvFModifiedPenultimateAmbientHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
    m_vvFModifiedPredictedLastAmbientHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
    m_suiActiveAmbHOACoeffsIndices.clear();

   // for QMF analysis
   std::vector<std::complex<FLOAT> > vcFTmpFreqSamplesOfOneBand(uiQMFSamples, std::complex<FLOAT>( static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0) ) );
   std::vector<std::vector<std::complex<FLOAT> > > vvcFTmpFreqSamplesOfAllBands( uiNoOfFreqBands, vcFTmpFreqSamplesOfOneBand);

   m_vvvcFQMFLastAmbientHOAFrame.assign( m_uiNoOfHOACoeffs, vvcFTmpFreqSamplesOfAllBands );

   // for encoding side information related to complex prediction coefficients for sub band directional signals prediction
   std::vector<int>  viTmpZeroRowVec( m_uiMaxNoOfTransmittedHOACoeffs,  0 );
   std::vector<std::vector<int> >  vvFTmpZeroMat( m_uiMaxNoOfTransmittedHOACoeffs, viTmpZeroRowVec );

   m_vvviOldPredCoeffsIntQuantMagnitudes.assign( m_uiNoOfSubBandsForDirPred, vvFTmpZeroMat );
   m_vvviOldPredCoeffsIntQuantAngles.assign(  m_uiNoOfSubBandsForDirPred, vvFTmpZeroMat );

   // for spatial decoding
   m_vvFComposedDelLastButOneHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

   // for PAR encoding
   m_vvFLastInputHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
   m_vvFLastButOneInputHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
   m_vvFLastButTwoInputHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

   m_vvFComposedLastButTwoHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
   m_vvFComposedDelLastButTwoHOAFrame.assign( m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

   m_suiAmbCoeffIndicesToBeEnabledInLastButTwoFrame.clear();
	m_suiAmbCoeffIndicesToBeDisabledInLastButTwoFrame.clear();
	m_suiNonEnDisAbleActHOACoeffIndicesInLastButTwoFrame.clear();

   m_vvFGainAdaptedSigsToBeCodedLastButOneFrame.assign( m_uiTotalNoOfPercCoders, vFTmpSingleChannelBuffer );
   m_vvFGainAdaptedSigsToBeCodedLastButTwoFrame.assign( m_uiTotalNoOfPercCoders, vFTmpSingleChannelBuffer );

   // for encoding PAR related side information
   m_vvviOldParMixMatsIntQuantMagnitudes.resize( m_vuiParSubBandWidths.size() );
   m_vvviOldParMixMatsIntQuantAngles.resize(m_vuiParSubBandWidths.size()) ;

   for(unsigned int uiSubBandGroupIdx=0; uiSubBandGroupIdx < m_vuiParSubBandWidths.size(); uiSubBandGroupIdx++)
   {
      m_vvviOldParMixMatsIntQuantMagnitudes[uiSubBandGroupIdx].assign( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx], std::vector<int>( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx], 0 ));
      m_vvviOldParMixMatsIntQuantAngles[uiSubBandGroupIdx].assign( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx], std::vector<int>( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubBandGroupIdx], 0 ));
   }

		// for channel assignment
		m_vvFAllSigsToBeCodedFrame.assign( m_uiTotalNoOfPercCoders, vFTmpSingleChannelBuffer);
		m_vvFPredictedAllSigsToBeCodedNextFrame.assign( m_uiTotalNoOfPercCoders, vFTmpSingleChannelBuffer);

	// init vector with assignment information
	m_vAIAssignmentVector.clear();

	m_bIsInit = true;

	return false;
}


bool SpatialEncoder::process(const std::vector<std::vector<FLOAT>> &p_rvvFInputHOAFrame,
								   std::vector<std::vector<FLOAT>> &p_rvvFOutputGainAdaptedSigsToBeCodedFrame,
								   HOA_FRAME &p_rFrame)
{
	// check if encoder has already been initialized
	if (!m_bIsInit)
	{
		return true;
	}

	const std::vector<std::vector<FLOAT>> * pvvFActualInputHOAFrame = &m_vvFZeroFrame;

	// check if input file end has already been reached
	if (!m_bIsEndOfInputFile)
	{
		// check input buffer sizes
		if( checkBufferSizes( p_rvvFInputHOAFrame) )
		{
			return true;
		}

		pvvFActualInputHOAFrame = &p_rvvFInputHOAFrame;
	}
	else
	{
		m_uiDelayFrameCounter++;
	}

	// start process

	// direction and vector estimation
   if(m_uiMaxNoOfDirSigs > 0)
   {
	   if ( m_shptrDirAndVecEst->process(*pvvFActualInputHOAFrame, 
									      m_shptrFrameParams->m_muiActiveDirAndGridDirIndices,
									      m_shptrFrameParams->m_muivFVectors) )
	   {
		   return true;
	   }
   }
   else
   {
      // use add. ambient coefficients only
      m_shptrFrameParams->m_muiActiveDirAndGridDirIndices.clear();
      m_shptrFrameParams->m_muivFVectors.clear();
   }

	// HOA decomposition
	if ( m_shptrHOADecomp->process( *pvvFActualInputHOAFrame, 
									m_shptrFrameParams->m_muiActiveDirAndGridDirIndices,
									m_shptrFrameParams->m_muivFVectors,
									m_vvFSmoothedPSSigsLastFrame,
									m_vvFSmoothedPSSigsPenultimateFrame,
									m_vvFPenultimateSmoothedPreDomSoundsHOAFrame,
									m_vvFPenultimateAmbientHOAFrame,
									m_vvFPredictedLastAmbientHOAFrame,
									m_shptrFrameParams->m_vuiPredictionTypeVec, 
									m_shptrFrameParams->m_vvuiPredictionIndicesMat,
									m_shptrFrameParams->m_vviQuantizedPredictionFactorsMat,
									m_vAITargetAssignmentVector) )
	{
		return true;
	}


	// ambient HOA component modification
	if ( m_shptrAmbCompMod->process( m_vvFPenultimateAmbientHOAFrame, 
									 m_vvFPredictedLastAmbientHOAFrame,
									 m_vAITargetAssignmentVector,
									 m_vvFModifiedPenultimateAmbientHOAFrame,
									 m_vvFModifiedPredictedLastAmbientHOAFrame,
									 m_vAIAssignmentVector,
                                     m_suiActiveAmbHOACoeffsIndices,
                                     m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled,
                                     m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled,
                                     m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices) )
	{
		return true;
	}

   // apply QMF analysis filter bank
   m_shptrQMFAnalysis->process(m_vvFPredictedLastAmbientHOAFrame, m_vvvcFQMFLastAmbientHOAFrame);

   // perform direction estimation on ambient HOA frame
   if ( m_shptrSubBandDirEstimation->process(  m_vvFPredictedLastAmbientHOAFrame,
                                               m_vvvcFQMFLastAmbientHOAFrame,
                                               m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred,
                                               m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides    ) )
   {
      return true;
	}

   // perform sub band directional signals prediction    
   if ( m_shptrSubBandDirSigsPred->process(m_vvvcFQMFLastAmbientHOAFrame,
                                           m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred,
                                           m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides,
                                           m_suiActiveAmbHOACoeffsIndices,
                                           m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats) )

   {
       return true;
   }

	// channel assignment
	if ( m_shptrChannAss->process(	m_vAIAssignmentVector,
									m_vAITargetAssignmentVector,
									m_vvFSmoothedPSSigsPenultimateFrame,
									m_vvFSmoothedPSSigsLastFrame,
									m_vvFModifiedPenultimateAmbientHOAFrame,
									m_vvFModifiedPredictedLastAmbientHOAFrame,
									m_vvFAllSigsToBeCodedFrame,
									m_vvFPredictedAllSigsToBeCodedNextFrame) )
	{
		return true;
	}

   // delay transport signals
   p_rvvFOutputGainAdaptedSigsToBeCodedFrame = m_vvFGainAdaptedSigsToBeCodedLastButTwoFrame;
   m_vvFGainAdaptedSigsToBeCodedLastButTwoFrame = m_vvFGainAdaptedSigsToBeCodedLastButOneFrame;

	// dynamic gain correction
	p_rvvFOutputGainAdaptedSigsToBeCodedFrame.resize(m_uiTotalNoOfPercCoders);

	for ( unsigned int uiChanIdx=0; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		bool bTmpException = false;


		if ( m_vshptrAllDynCorr[uiChanIdx]->process( m_vvFAllSigsToBeCodedFrame[uiChanIdx],
													 m_vvFPredictedAllSigsToBeCodedNextFrame[uiChanIdx],
                                        m_vvFGainAdaptedSigsToBeCodedLastButOneFrame[uiChanIdx],
													 bTmpException,
													 m_shptrFrameParams->m_viExponents[uiChanIdx] ) )
		{
			return true;
		}
		m_shptrFrameParams->m_vbExceptions[uiChanIdx]  = bTmpException;
	}

   // delay HOA frames
   p_rFrame = m_LastButTwoHOASIFrame;
   m_LastButTwoHOASIFrame = m_PenultimateHOASIFrame;

   // encode frame side info (without PAR info) 
   encodeFrameSideInfo(m_PenultimateHOASIFrame);

   // perform spatial decoding to obtain composed HOA representation without PAR
   m_shptrSpatDec->process(m_vvFGainAdaptedSigsToBeCodedLastButOneFrame,
                           m_vvFComposedDelLastButOneHOAFrame,
                           m_PenultimateHOASIFrame);

   // delay of composed HOA representation to align with frame borders
   for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
   {
      for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize-m_shptrSpatDec->getNoOfQMFDelaySamples(); uiSampleIdx++ )
      {
         m_vvFComposedLastButTwoHOAFrame[uiCoeffIdx][uiSampleIdx] = m_vvFComposedDelLastButTwoHOAFrame[uiCoeffIdx][m_shptrSpatDec->getNoOfQMFDelaySamples()+uiSampleIdx];
      }
      for (unsigned int uiSampleIdx=m_uiFrameSize-m_shptrSpatDec->getNoOfQMFDelaySamples(); uiSampleIdx < m_uiFrameSize; uiSampleIdx++ )
      {
         m_vvFComposedLastButTwoHOAFrame[uiCoeffIdx][uiSampleIdx] = m_vvFComposedDelLastButOneHOAFrame[uiCoeffIdx][uiSampleIdx-(m_uiFrameSize-m_shptrSpatDec->getNoOfQMFDelaySamples())];
      }
   }

   // PAR encoder
   if ( m_shptrPAREncoder->process( m_vvFLastButTwoInputHOAFrame,
                                    m_vvFComposedLastButTwoHOAFrame, 
                                    m_suiAmbCoeffIndicesToBeEnabledInLastButTwoFrame,
                                    m_suiAmbCoeffIndicesToBeDisabledInLastButTwoFrame,
                                    m_suiNonEnDisAbleActHOACoeffIndicesInLastButTwoFrame,
                                    m_shptrFrameParams->m_bUsePAR,
                                    m_shptrFrameParams->m_vvvcFPARMixingMats,
                                    m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices) )
   {
      return true;
   }

   encodePARFrameSideInfo(p_rFrame);


   // update buffers
   m_vvFLastButTwoInputHOAFrame = m_vvFLastButOneInputHOAFrame;
   m_vvFLastButOneInputHOAFrame = m_vvFLastInputHOAFrame;
   m_vvFLastInputHOAFrame       = *pvvFActualInputHOAFrame;
   m_vvFComposedDelLastButTwoHOAFrame    = m_vvFComposedDelLastButOneHOAFrame;


   m_suiAmbCoeffIndicesToBeEnabledInLastButTwoFrame = m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled;
   m_suiAmbCoeffIndicesToBeDisabledInLastButTwoFrame = m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled;
   m_suiNonEnDisAbleActHOACoeffIndicesInLastButTwoFrame = m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices;

   if ( m_uiDelayFrameCounter >= m_uiNoOfRequiredDelayFrames)
	{
		m_bIsEndOfOutputFile = true;
	}

	return false;
}

bool SpatialEncoder::checkBufferSizes(const std::vector<std::vector<FLOAT>> &p_rvvFInputHOAFrame)
{
	if (p_rvvFInputHOAFrame.size() != m_uiNoOfHOACoeffs )
	{
		return true;
	}

	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
		if (p_rvvFInputHOAFrame[uiCoeffIdx].size() != m_uiFrameSize )
		{
			return true;
		}
	}

	return false;
}
							  
bool SpatialEncoder::encodeFrameSideInfo(HOA_FRAME &p_rFrame )
{
   // encode gain correction exponents and exceptions
   p_rFrame.m_vGainCorrectionData.resize(m_uiTotalNoOfPercCoders);

   for (unsigned int uiChanIdx=0; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
   {
      // encode gain correction exponent for current channel
      int iCurrExponent = m_shptrFrameParams->m_viExponents[uiChanIdx];

      unsigned int uiCurrCodeLength;

      switch( iCurrExponent ) 
      {
      case 0:
         {
            uiCurrCodeLength = 1;
            break;
         }
      case -1:
         {
            uiCurrCodeLength = 2;
            break;
         }
      default:
         {
            uiCurrCodeLength = iCurrExponent + 2;
         }
      }

      p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bCodedGainCorrectionExp.resize(0);

      for (unsigned int uiBitIdx=0; uiBitIdx < uiCurrCodeLength - 1; uiBitIdx++)
      {
         p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bCodedGainCorrectionExp.push_back(false);
      }

      p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bCodedGainCorrectionExp.push_back(true);


      // encode gain correction exception for current channel
      p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bGainCorrectionException = m_shptrFrameParams->m_vbExceptions[uiChanIdx];

      // set exponent to base 2 of absolute amplitude change by gain control
      p_rFrame.m_vGainCorrectionData[uiChanIdx].m_nGainCorrAbsAmpExp = m_vshptrAllDynCorr[uiChanIdx]->getLastExp();

   }

   // encode channel side info data
   p_rFrame.m_vChannelSideInfo.resize(m_uiNoOfAddPercCoders);

    std::set<unsigned int> suiAllActHOACoeffIndices;
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx++)
    {
      suiAllActHOACoeffIndices.insert(uiCoeffIdx+1);
    }

   for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfAddPercCoders; uiChanIdx++)
   {
      p_rFrame.m_vChannelSideInfo[uiChanIdx] = m_vAIAssignmentVector[uiChanIdx].m_shptrChanSideInfoData;
      if(p_rFrame.m_vChannelSideInfo[uiChanIdx]->m_unChannelType == ADD_HOA_CHANNEL)
      {
         CAddAmbHoaInfoChannel *tmp = dynamic_cast<CAddAmbHoaInfoChannel*>(p_rFrame.m_vChannelSideInfo[uiChanIdx].get());
         suiAllActHOACoeffIndices.insert(tmp->m_unAmbCoeffIdx);
      }
   }


   // encode prediction parameters

   // compute number of active predictions
   unsigned int uiNoOfActPred = 0;

   for (unsigned int uiGridDirIdx = 0; uiGridDirIdx < m_uiNoOfHOACoeffs; uiGridDirIdx++)
   {
      if ( m_shptrFrameParams->m_vuiPredictionTypeVec[uiGridDirIdx] > 0)
      {
         uiNoOfActPred++;
      }
   }

   // set bool indicating whether prediction is to be performed
   if ( uiNoOfActPred == 0) 
   {
      p_rFrame.m_spatPredictionData.m_bPerformPrediction = false;
   }
   else
   {
      p_rFrame.m_spatPredictionData.m_bPerformPrediction = true;

      if ( uiNoOfActPred <= m_uiNoOfPredIndToCodeExplicitly)
      {
         p_rFrame.m_spatPredictionData.m_bKindOfCodedPredIds = true;
      }
      else
      {
         p_rFrame.m_spatPredictionData.m_bKindOfCodedPredIds = false;
      }


      p_rFrame.m_spatPredictionData.m_bActivePred.resize(m_uiNoOfHOACoeffs);
      p_rFrame.m_spatPredictionData.m_bPredType.resize(uiNoOfActPred);
      p_rFrame.m_spatPredictionData.m_unPredIds.resize(m_uiMaxNofOfDirSigsForPred*m_uiNoOfHOACoeffs);
      p_rFrame.m_spatPredictionData.m_nPredGains.resize(m_uiMaxNofOfDirSigsForPred*m_uiNoOfHOACoeffs);

      unsigned int uiCurrNoOfActPred = 0;
      unsigned int uiCurrPredDirIdx  = 0;
      unsigned int uiCurrPredScaleIdx = 0;

      for(unsigned int uiGridDirIdx = 0; uiGridDirIdx < m_uiNoOfHOACoeffs; uiGridDirIdx++)
      {
         unsigned int uiCurrPredictionType = m_shptrFrameParams->m_vuiPredictionTypeVec[uiGridDirIdx];

         p_rFrame.m_spatPredictionData.m_bActivePred[uiGridDirIdx] = (uiCurrPredictionType > 0);

         if ( uiCurrPredictionType > 0)
         {
            p_rFrame.m_spatPredictionData.m_bPredType[uiCurrNoOfActPred] = ( (uiCurrPredictionType - 1) != 0 );
            uiCurrNoOfActPred++;

            for (unsigned int uiPredIdx=0; uiPredIdx < m_uiMaxNofOfDirSigsForPred; uiPredIdx++)
            {
               p_rFrame.m_spatPredictionData.m_unPredIds[uiCurrPredDirIdx] = m_shptrFrameParams->m_vvuiPredictionIndicesMat[uiPredIdx][uiGridDirIdx];
               uiCurrPredDirIdx++;

               if ( m_shptrFrameParams->m_vvuiPredictionIndicesMat[uiPredIdx][uiGridDirIdx] != 0)
               {
                  p_rFrame.m_spatPredictionData.m_nPredGains[uiCurrPredScaleIdx] = m_shptrFrameParams->m_vviQuantizedPredictionFactorsMat[uiPredIdx][uiGridDirIdx];
                  uiCurrPredScaleIdx++;
               }
            }
         }
      }

      // resize array after lengths are known
      p_rFrame.m_spatPredictionData.m_unPredIds.resize(uiCurrPredDirIdx);
      p_rFrame.m_spatPredictionData.m_nPredGains.resize(uiCurrPredScaleIdx);

   }

   // encode side information related to sub band directional signals prediction

   p_rFrame.m_hoaDirectionalPredictionInfo.m_bUseDirPred = (m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.size() > 0);
   p_rFrame.m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs = m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.size();
   p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.resize(p_rFrame.m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs);


   // set global set of directions
   unsigned int uiDirIdx = 0;

   for ( std::set<unsigned int>::iterator suiIt = m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.begin();
      suiIt != m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.end(); suiIt++)
   {
      unsigned int uiDirGridIdx = *suiIt;

      p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds[uiDirIdx] = uiDirGridIdx;

      uiDirIdx++;
   }

   // encode sub band related directions
   p_rFrame.m_hoaDirectionalPredictionInfo.m_vvbDirIsActive.resize(m_uiNoOfSubBandsForDirPred); 

   for (unsigned int uiSubBandIdx=0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
   {
      p_rFrame.m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[uiSubBandIdx].assign(m_uiMaxNoOfPredDirsPerSubBand, false);

      for (std::map<unsigned int, unsigned int>::const_iterator muicIt = m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].begin();
         muicIt != m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].end(); muicIt++)
      {
         unsigned int uiDirIdx     = muicIt->first - 1;
         unsigned int uiDirGridIdx = muicIt->second;

         p_rFrame.m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[uiSubBandIdx][uiDirIdx] = true;


         std::vector<unsigned int>::iterator vuiItRelPos = find( p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.begin(),
            p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.end(), uiDirGridIdx);

         if ( vuiItRelPos != p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.end() )
         {
            unsigned int uiRelPos = std::distance(p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.begin(), vuiItRelPos);

            p_rFrame.m_hoaDirectionalPredictionInfo.m_vvunRelDirGridIdx[uiSubBandIdx][uiDirIdx] = uiRelPos;
         }
         else
         {
            // it is actually not possible to reach this state if the direction estimation was done correctly
            std::cout << "Error at coding of directions for sub band directional signals predictions occured!" << std::endl;
            return true;
         }
      }
   }

   // always use Huffman coding for reference software
   p_rFrame.m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffMag.assign(m_uiNoOfSubBandsForDirPred, true );
   p_rFrame.m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffAngle.assign(m_uiNoOfSubBandsForDirPred, true );

   // encode complex prediction coefficients
   for (unsigned int uiSubBandIdx=0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
   {
      for (unsigned int uiDirIdx=0; uiDirIdx < m_uiMaxNoOfPredDirsPerSubBand; uiDirIdx++)
      {
         for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiMaxNoOfTransmittedHOACoeffs; uiCoeffIdx++)
         {
            if ( suiAllActHOACoeffIndices.find(uiCoeffIdx+1) != suiAllActHOACoeffIndices.end() )
            {
              FLOAT   FTmpAbsOfElemToBeQuantized  = std::abs( m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats[uiSubBandIdx][uiDirIdx][uiCoeffIdx] );
              int     iOldQuantMag                = m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoeffIdx];
              int     iOldQuantAngle              = m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoeffIdx];

              int iCurrentQuantMag    = 0;

              // compute quantized magnitude
              if(FTmpAbsOfElemToBeQuantized<=static_cast<FLOAT>(1.0))
              {
                 iCurrentQuantMag = 
                    static_cast<int>(std::floor(FTmpAbsOfElemToBeQuantized/m_FQuantIntLenForMagnForDirPred + static_cast<FLOAT>(0.5)));
              }
              else
              {
                 iCurrentQuantMag = 
                    static_cast<int>(std::floor(   log(static_cast<FLOAT>(FTmpAbsOfElemToBeQuantized))
                    / log(static_cast<FLOAT>(m_iHighestQuantValForMagForDirPred)/static_cast<FLOAT>(m_iHighestQuantValForMagForDirPred-1) ) 
                    + static_cast<FLOAT>(m_iHighestQuantValForMagForDirPred) + static_cast<FLOAT>(0.5)));
              }

              int iCurrentQuantMagDiff    = iCurrentQuantMag   - iOldQuantMag;

              // if the magnitude after quantization is equal to zero the transmitted difference in quantized angle differences is ignored
              // hence in that case it is reasonable to code the difference as efficient as possible by setting it to zero
              int iCurrentQuantAngle  = 0;
              int iCurrentQuantAngleDiff  = 0;

              if ( iCurrentQuantMag != 0)
              {
                 FLOAT FTmpAngleOfElemToBeQuantized = std::arg( m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats[uiSubBandIdx][uiDirIdx][uiCoeffIdx]);

                 if ( FTmpAngleOfElemToBeQuantized < static_cast<FLOAT>(m_iLowestQuantValForAngleForDirPred)*m_FQuantIntLenForAngleForDirPred 
                    - m_FQuantIntLenForAngleForDirPred/static_cast<FLOAT>(2.0) )
                 {
                    iCurrentQuantAngle = m_iHighestQuantValForAngleForDirPred;
                 }
                 else
                 {
                    iCurrentQuantAngle = static_cast<int>(std::floor(FTmpAngleOfElemToBeQuantized
                       /m_FQuantIntLenForAngleForDirPred + static_cast<FLOAT>(0.5)));

                 }

                 iCurrentQuantAngleDiff  = iCurrentQuantAngle - iOldQuantAngle;

                 // constrain the angle difference to lie in interval ]-pi,pi]
                 if ( iCurrentQuantAngleDiff < m_iLowestQuantValForAngleForDirPred )
                 {
                    iCurrentQuantAngleDiff += m_uiNoOfQuantIntervForAngleForDirPred;
                 }
                 else
                 {
                    if( iCurrentQuantAngleDiff > m_iHighestQuantValForAngleForDirPred)
                    {
                       iCurrentQuantAngleDiff -= m_uiNoOfQuantIntervForAngleForDirPred;
                    }
                 }
              }

              // set magnitude and angle differences in frame 
              p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[uiSubBandIdx][uiDirIdx][uiCoeffIdx]    = iCurrentQuantMagDiff;
              p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[uiSubBandIdx][uiDirIdx][uiCoeffIdx]  = iCurrentQuantAngleDiff;

              // buffer old quantization values
              m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoeffIdx] = iCurrentQuantMag;
              m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoeffIdx]     = iCurrentQuantAngle;
            }
            else
            {
               // set magnitude and angle differences in frame 
               p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[uiSubBandIdx][uiDirIdx][uiCoeffIdx]    = static_cast<FLOAT>(0.0);
               p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[uiSubBandIdx][uiDirIdx][uiCoeffIdx]  = static_cast<FLOAT>(0.0);

               // buffer old quantization values
               m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoeffIdx] = static_cast<FLOAT>(0.0);
               m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoeffIdx]     = static_cast<FLOAT>(0.0);
            }
         }
      }
   }

   return false;
}


bool SpatialEncoder::encodePARFrameSideInfo(HOA_FRAME &p_rFrame )
{
    p_rFrame.m_hoaParInfo.m_bPerformPar = m_shptrFrameParams->m_bUsePAR;

    for (unsigned int uiSubbandGroupIdx=0; uiSubbandGroupIdx < m_vuiParSubBandWidths.size(); uiSubbandGroupIdx++)
    {
        p_rFrame.m_hoaParInfo.m_vunParDecorrSigsSelectionTableIdx[uiSubbandGroupIdx] = m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices[uiSubbandGroupIdx];
        
        switch (m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx])
        {
        case 9:
           p_rFrame.m_hoaParInfo.updateSelectionTableInSubBand(uiSubbandGroupIdx,  2);
           break;
        case 4:
           p_rFrame.m_hoaParInfo.updateSelectionTableInSubBand(uiSubbandGroupIdx,  1);
           break;
        default:
           return true;
        }

        //
        p_rFrame.m_hoaParInfo.m_vvbUseParUpmixSig[uiSubbandGroupIdx].assign( m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx], false  );

        for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiUpMixSigIdx++)
        {
            for (unsigned int uiDecSigIdx=0; uiDecSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiDecSigIdx++)
            {
                if ( std::abs(m_shptrFrameParams->m_vvvcFPARMixingMats[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx]) > std::numeric_limits<FLOAT>::min() )
                {
                    p_rFrame.m_hoaParInfo.m_vvbUseParUpmixSig[uiSubbandGroupIdx][uiUpMixSigIdx] = true;
                }
            }
        }

        //
        p_rFrame.m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[uiSubbandGroupIdx] = false;
        for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiUpMixSigIdx++)
        {
            if ( p_rFrame.m_hoaParInfo.m_vvbUseParUpmixSig[uiSubbandGroupIdx][uiUpMixSigIdx] == false)
            {
                 p_rFrame.m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[uiSubbandGroupIdx] = true;
                 break;
            }
        }
    }

    // always use conventional coding for reference software
    p_rFrame.m_hoaParInfo.m_vbUseParHuffmanCodingDiffAbs.assign(m_vuiParSubBandWidths.size(), false );
    p_rFrame.m_hoaParInfo.m_vbUseParHuffmanCodingDiffAngle.assign(m_vuiParSubBandWidths.size(), false );

    // coding of mixing matrix elements
    for (unsigned int uiSubbandGroupIdx=0; uiSubbandGroupIdx < m_vuiParSubBandWidths.size(); uiSubbandGroupIdx++)
    {
        // init flag to indicate to keep previous matrix 
        p_rFrame.m_hoaParInfo.m_vbKeepPrevMatrixFlag[uiSubbandGroupIdx] = true;

        for (unsigned int uiUpMixSigIdx=0; uiUpMixSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiUpMixSigIdx++)
        {
            for (unsigned int uiDecSigIdx=0; uiDecSigIdx < m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx]; uiDecSigIdx++)
            {
                FLOAT   FTmpParAbsOfElemToBeQuantized  = std::abs( m_shptrFrameParams->m_vvvcFPARMixingMats[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx] );
                int     iOldParQuantMag                = m_vvviOldParMixMatsIntQuantMagnitudes[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx];
                int     iOldParQuantAngle              = m_vvviOldParMixMatsIntQuantAngles[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx];

                int iCurrentParQuantMag    = 0;

                // compute quantized magnitude
                if(FTmpParAbsOfElemToBeQuantized<=static_cast<FLOAT>(1.0))
                {
                    iCurrentParQuantMag = 
                        static_cast<int>(std::floor(FTmpParAbsOfElemToBeQuantized/m_FQuantIntLenForMagnForPAR + static_cast<FLOAT>(0.5)));
                }
                else
                {
                    iCurrentParQuantMag = 
                        static_cast<int>(std::floor(   log(static_cast<FLOAT>(FTmpParAbsOfElemToBeQuantized))
                                                    / log(static_cast<FLOAT>(m_iHighestQuantValForMagForPAR)/static_cast<FLOAT>(m_iHighestQuantValForMagForPAR-1) ) 
                                                        + static_cast<FLOAT>(m_iHighestQuantValForMagForPAR) + static_cast<FLOAT>(0.5)));
                }

                int iCurrentParQuantMagDiff    = iCurrentParQuantMag   - iOldParQuantMag;

                // if the magnitude after quantization is equal to zero the transmitted difference in quantized angle differences is ignored
                // hence in that case it is reasonable to code the difference as efficient as possible by setting it to zero
                int iCurrentParQuantAngle  = 0;
                int iCurrentParQuantAngleDiff  = 0;

                if ( iCurrentParQuantMag != 0)
                {
                    FLOAT FTmpParAngleOfElemToBeQuantized = std::arg( m_shptrFrameParams->m_vvvcFPARMixingMats[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx]);

                    if ( FTmpParAngleOfElemToBeQuantized < static_cast<FLOAT>(m_iLowestQuantValForAngleForPAR)*m_FQuantIntLenForAngleForPAR 
                                                        - m_FQuantIntLenForAngleForPAR/static_cast<FLOAT>(2.0) )
                    {
                        iCurrentParQuantAngle = m_iHighestQuantValForAngleForPAR;
                    }
                    else
                    {
                        iCurrentParQuantAngle = static_cast<int>(std::floor(FTmpParAngleOfElemToBeQuantized
                                                    /m_FQuantIntLenForAngleForPAR + static_cast<FLOAT>(0.5)));
                                        
                    }

                    iCurrentParQuantAngleDiff  = iCurrentParQuantAngle - iOldParQuantAngle;

                    // constrain the angle difference to lie in interval ]-pi,pi]
                    if ( iCurrentParQuantAngleDiff < m_iLowestQuantValForAngleForPAR )
                    {
                        iCurrentParQuantAngleDiff += m_uiNoOfQuantIntervForAngleForPAR;
                    }
                    else
                    {
                        if( iCurrentParQuantAngleDiff > m_iHighestQuantValForAngleForPAR)
                        {
                            iCurrentParQuantAngleDiff -= m_uiNoOfQuantIntervForAngleForPAR;
                        }
                    }
                }

                // set magnitude and angle differences in frame 
                p_rFrame.m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx] = iCurrentParQuantMagDiff;
                p_rFrame.m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx] = iCurrentParQuantAngleDiff;

                if ((iCurrentParQuantMagDiff != 0) | (iCurrentParQuantAngleDiff != 0))
                {
                    p_rFrame.m_hoaParInfo.m_vbKeepPrevMatrixFlag[uiSubbandGroupIdx] = false;
                }

                // buffer old quantization values
                m_vvviOldParMixMatsIntQuantMagnitudes[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx] = iCurrentParQuantMag;
                m_vvviOldParMixMatsIntQuantAngles[uiSubbandGroupIdx][uiUpMixSigIdx][uiDecSigIdx] = iCurrentParQuantAngle;

            }
        }
    }
    return false;
}


double SpatialEncoder::RoundNumber(double p_dNumber)
{
    return p_dNumber < 0.0 ? std::ceil(p_dNumber - 0.5) : std::floor(p_dNumber + 0.5);
}


void SpatialEncoder::setEndOfInputFile()
{
	m_bIsEndOfInputFile = true;
}

bool SpatialEncoder::isEndOfOutputFile()
{
	return m_bIsEndOfOutputFile;
}

bool SpatialEncoder::getBitRate(std::vector<unsigned int> &p_vuiBitRateVec)
{
	if (!m_bIsInit)
	{
		return true;
	}
	else
	{
		p_vuiBitRateVec.resize(m_uiTotalNoOfPercCoders);

		for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfAddPercCoders; uiChanIdx++)
		{
			p_vuiBitRateVec[uiChanIdx] = m_vAIAssignmentVector[uiChanIdx].m_uiBitRate;
		}

		for (unsigned int uiChanIdx=m_uiNoOfAddPercCoders; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
		{
			p_vuiBitRateVec[uiChanIdx] = m_uiBitRatePerPercCoder;
		}
	}
  return false;
}
