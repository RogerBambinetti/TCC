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
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: SpatialDecoder.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "SpatialDecoder.h"

// constructor
SpatialDecoder::SpatialDecoder()
{
	m_bIsInit = false;
}

bool SpatialDecoder::init(const HOA_CONFIG &p_rAccessUnitHeader,
						  unsigned int p_uiFrameSize)
{

	// fixed parameters

	// for prediction of directional sub-band signals
	m_uiNoOfQuantIntervForAngleForDirPred = sizeof(g_pDecodedAngleDiffTable) / sizeof(HuffmanWord<int>);
	m_uiNoOfQuantIntervForMagnForDirPred = sizeof(g_pDecodedMagDiffTable) / sizeof(HuffmanWord<int>) - 2;

	m_iLowestQuantValForAngleForDirPred = 1 - static_cast<int>(m_uiNoOfQuantIntervForAngleForDirPred / 2);

	m_FQuantIntLenForMagnForDirPred = static_cast<FLOAT>(2.0 / static_cast<double>(m_uiNoOfQuantIntervForAngleForDirPred));
	m_FQuantIntLenForAngleForDirPred = static_cast<FLOAT>(2.0 * static_cast<double>(OWN_PI) / static_cast<double>(m_uiNoOfQuantIntervForAngleForDirPred));

	// for PAR
	m_uiNoOfQuantIntervForAngleForPAR = sizeof(g_pDecodedAngleDiffTable) / sizeof(HuffmanWord<int>);
	m_uiNoOfQuantIntervForMagnForPAR = sizeof(g_pDecodedMagDiffTable) / sizeof(HuffmanWord<int>) - 2;

	m_iLowestQuantValForAngleForPAR = 1 - static_cast<int>(m_uiNoOfQuantIntervForAngleForPAR / 2);

	m_FQuantIntLenForMagnForPAR = static_cast<FLOAT>(2.0 / static_cast<double>(m_uiNoOfQuantIntervForAngleForPAR));
	m_FQuantIntLenForAngleForPAR = static_cast<FLOAT>(2.0 * static_cast<double>(OWN_PI) / static_cast<double>(m_uiNoOfQuantIntervForAngleForPAR));

	// global parameters
	m_uiFrameSize = p_uiFrameSize;

	// parameters from access frame
	updateParamsFromAccessUnitHeader(p_rAccessUnitHeader);

	// init frame parameters
	m_shptrFrameParams = std::shared_ptr<FrameParams>(new FrameParams(m_uiTotalNoOfPercCoders,
																	  m_uiNoOfHOACoeffs,
																	  m_uiMaxNofOfDirSigsForPred,
																	  m_uiMaxNoOfTransmittedHOACoeffs,
																	  m_uiNoOfSubBandsForDirPred,
																	  m_uiMaxNoOfPredDirsPerSubBand,
																	  m_vuiParUpmixHoaOrdersPerParSubBand));

	// init tabulated values
	m_shptrTabVals = std::shared_ptr<TabulatedValues>(new TabulatedValues());

	// init dynamic correction (vector of shared pointers to objects, size equal to number of channels)
	for (unsigned int uiIdx = 0; uiIdx < m_uiTotalNoOfPercCoders; uiIdx++)
	{
		std::shared_ptr<InverseDynCorrection> tmpDynCorr(new InverseDynCorrection());

		tmpDynCorr->init(m_uiFrameSize);

		m_vshptrAllDynCorr.push_back(tmpDynCorr);
	}

	// init reassigment
	m_shptrReAssign = std::shared_ptr<ChannelReAssignment>(new ChannelReAssignment());
	m_shptrReAssign->init(m_uiTotalNoOfPercCoders, m_uiNoOfDomDirs, m_uiNoOfHOACoeffs, m_uiFrameSize);

	// init predominant sound synthesis
	m_shptrPreDomSoundSynth = std::shared_ptr<PreDomSoundSynthesis>(new PreDomSoundSynthesis());

	m_shptrPreDomSoundSynth->init(m_uiNoOfDomDirs,
								  m_uiHOAOrder,
								  m_uiFrameSize,
								  m_uiMinNoOfCoeffsForAmbHOA,
								  m_uiMaxNofOfDirSigsForPred,
								  m_uiNoOfBitsPerScaleFactor,
								  m_uiVVecStartCh,
								  m_uiCodedVVecLength,
								  m_uiInterpSamples,
								  m_uiInterpMethod,
								  m_shptrTabVals);

	// init ambience synthesis
	m_shptrAmbSynth = std::shared_ptr<AmbienceSynthesis>(new AmbienceSynthesis());
	m_shptrAmbSynth->init(m_uiFrameSize, m_uiHOAOrder, m_iMinOrderForAmbHOA, m_bUsePhaseShiftDecorr, m_shptrTabVals);

	// init sub-band directional signals synthesis
	m_shprtSubBandDirSigSynth = std::shared_ptr<SubBandDirSigsSynthesis>(new SubBandDirSigsSynthesis());
	m_shprtSubBandDirSigSynth->init(m_uiHOAOrder,
									m_uiMaxNoOfTransmittedHOACoeffs,
									m_uiFrameSize,
									m_vuiSubBandWidths,
									m_uiNoOfSubBandsForDirPred,
									m_uiMaxNoOfPredDirsPerSubBand,
									m_uiDirGridTableIdx,
									m_shptrTabVals);

	unsigned int uiNoOfQMFDelaySamples = m_shprtSubBandDirSigSynth->getNoOfQMFDelaySamples();

	// init final HOA composition
	m_shptrFinalHOAComp = std::shared_ptr<FinalHOAComposition>(new FinalHOAComposition());
	m_shptrFinalHOAComp->init(m_uiFrameSize, m_uiHOAOrder, m_iMaxPARHOAOrder, uiNoOfQMFDelaySamples);

	// init PAR decoder
	m_shptrPARDecoder = std::shared_ptr<PARDecoder>(new PARDecoder());
	m_shptrPARDecoder->init(m_vuiParUpmixHoaOrdersPerParSubBand, m_vuiParSubbandWidths, m_uiFrameSize, m_shptrTabVals);

	// init preliminary HOA composition
	m_shptrPrelimHOAComp = std::shared_ptr<PreliminaryHOAComposition>(new PreliminaryHOAComposition());
	m_shptrPrelimHOAComp->init(m_uiFrameSize,
							   m_iMaxPARHOAOrder,
							   m_uiHOAOrder);

	// init buffers
	std::vector<FLOAT> vFTmpSingleChannelBuffer(m_uiFrameSize, static_cast<FLOAT>(0.0));

	// for decoding of side info related to vector based predominant sound synthesis
	muivFOldNominators.clear();

	// for decoding side information related to complex prediction coefficients for sub band directional signals prediction
	std::vector<int> viTmpZeroRowVec(m_uiMaxNoOfTransmittedHOACoeffs, 0);
	std::vector<std::vector<int>> vvFTmpZeroMat(m_uiMaxNoOfPredDirsPerSubBand, viTmpZeroRowVec);

	m_vvviOldPredCoeffsIntQuantMagnitudes.assign(m_uiNoOfSubBandsForDirPred, vvFTmpZeroMat);
	m_vvviOldPredCoeffsIntQuantAngles.assign(m_uiNoOfSubBandsForDirPred, vvFTmpZeroMat);

	// for preliminary HOA composition
	m_vvFPrelimComposedHOACoeffsFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for synthesis of directional sub-band signals
	m_vvFSubBandDirSigsCompositionHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for faded predominant sound HOA component to used for composition of input HOA representation to PAR
	m_vvFOutputFadedForPARPreDomSoundsHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for PAR
	unsigned int uiNoOfPARSubbandGroups = m_vuiParSubbandWidths.size();

	m_vvviCurrPARMixMatIntQuantMagnitudes.resize(uiNoOfPARSubbandGroups);

	for (unsigned int uiSubbandGroupIdx = 0; uiSubbandGroupIdx < uiNoOfPARSubbandGroups; uiSubbandGroupIdx++)
	{
		unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx];

		m_vvviCurrPARMixMatIntQuantMagnitudes[uiSubbandGroupIdx].resize(uiCurrNoOfUpmixSigs);

		for (unsigned int uiSigIdx = 0; uiSigIdx < uiCurrNoOfUpmixSigs; uiSigIdx++)
		{
			m_vvviCurrPARMixMatIntQuantMagnitudes[uiSubbandGroupIdx][uiSigIdx].assign(uiCurrNoOfUpmixSigs, 0);
		}
	}

	m_vvviCurrPARMixMatIntQuantAngles = m_vvviCurrPARMixMatIntQuantMagnitudes;
	m_vvviOldPARMixMatIntQuantMagnitudes = m_vvviCurrPARMixMatIntQuantMagnitudes;
	m_vvviOldPARMixMatIntQuantAngles = m_vvviCurrPARMixMatIntQuantMagnitudes;

	m_vvFInputForPARHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
	m_vvFPARHOAFrameBuffer.assign((m_iMaxPARHOAOrder + 1) * (m_iMaxPARHOAOrder + 1), vFTmpSingleChannelBuffer);

	// for inverse dynamic correction
	m_vvFDynCorrMCSampleBuffer.assign(m_uiTotalNoOfPercCoders, vFTmpSingleChannelBuffer);

	// for channel assignment
	m_vvFDecPSSigsFrameBuffer.assign(m_uiNoOfDomDirs, vFTmpSingleChannelBuffer);
	m_vvFPreliminaryDecAmbHOACoeffsFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for predominant sound synthesis
	m_vvFPreDomSoundsHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
	m_vvFQMFDelayedPreDomSoundsHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
	m_vvFDirSigsHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);
	m_vvFVecBasedPreDomSoundsHOAFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for ambience synthesis
	m_vvFAmbHOACoeffsFrameBuffer.assign(m_uiNoOfHOACoeffs, vFTmpSingleChannelBuffer);

	// for VVec VQ decoding
	// get mode matrix for coarse grid points
	if (m_shptrTabVals->getVVecVqMat(m_vvFInvModeMatAllCoarseGridPoints, m_uiHOAOrder))
	{
		return true;
	}
	// get mode matrix for fine grid points
	if (m_shptrTabVals->getTransposeModeMatForFineGrid(m_vvFTransposeModeMatAllFineGridPoints, m_uiHOAOrder, 2))
	{
		return true;
	}

	// get dict for CICP speaker locations
	if (m_shptrTabVals->getDictCicpSpeakerPoints(m_vvFDictCicpSpeakerPoints, m_uiHOAOrder))
	{
		return true;
	}

	// get dict for horizontal-only codebook
	if (m_shptrTabVals->getDict2DPoints(m_vvFDict2DPoints, m_uiHOAOrder))
	{
		return true;
	}

	if (m_shptrTabVals->getWeightingCdbk256x8(m_vvFWeightingCdbk256x8))
	{
		return true;
	}

	m_bIsInit = true;

	return false;
}

// update parameters from access unit header
void SpatialDecoder::updateParamsFromAccessUnitHeader(const HOA_CONFIG &p_rAccessUnitHeader)
{
	// copy parameters from access unit header
	m_uiHOAOrder = p_rAccessUnitHeader.m_unHoaOrder;
	m_bNFCFlag = static_cast<bool>(p_rAccessUnitHeader.m_bUsesNfc != 0);
	m_FNFCDistance_m = static_cast<FLOAT>(p_rAccessUnitHeader.m_ufAmbNfcReferenceDistance);

	m_iMinOrderForAmbHOA = p_rAccessUnitHeader.m_nMinAmbHoaOrder;
	m_uiNoOfDomDirs = p_rAccessUnitHeader.m_unNumOfAdditionalCoders;
	m_uiTotalNoOfPercCoders = p_rAccessUnitHeader.m_unTotalNumCoders;
	m_uiMaxNofOfDirSigsForPred = p_rAccessUnitHeader.m_unMaxNoOfDirSigsForPrediction;
	m_uiNoOfBitsPerScaleFactor = p_rAccessUnitHeader.m_unNoOfBitsPerScaleFactor;
#ifndef USAC_CORE
	m_bUseSBR = static_cast<bool>(p_rAccessUnitHeader.m_bUseSBR != 0);
#endif
	m_uiTotalNoOfPercCoders = p_rAccessUnitHeader.m_unTotalNumCoders;
	m_uiInterpSamples = p_rAccessUnitHeader.m_unSpatialInterpolationTime;
	m_uiInterpMethod = p_rAccessUnitHeader.m_unSpatInterpMethod;
	m_uiCodedVVecLength = p_rAccessUnitHeader.m_unCodedVVecLength;

	m_iMaxOrderToBeTransmitted = p_rAccessUnitHeader.m_nMaxHoaOrderToBeTransmitted;

	m_uiMaxNoOfPredDirsPerSubBand = p_rAccessUnitHeader.m_unMaxNumOfPredDirsPerBand;
	m_uiNoOfSubBandsForDirPred = p_rAccessUnitHeader.m_unNumOfPredSubbands;

	m_vuiSubBandWidths = p_rAccessUnitHeader.m_vunPredSubbandWidths;
	m_uiDirGridTableIdx = p_rAccessUnitHeader.m_unDirGridTableIdx;

	m_bUsePhaseShiftDecorr = p_rAccessUnitHeader.m_bUsePhaseShiftDecorr;

	// PAR related initialization
	m_vuiParSubbandWidths = p_rAccessUnitHeader.m_vunParSubbandWidths;
	m_vuiParUpmixHoaOrdersPerParSubBand = p_rAccessUnitHeader.m_vunParUpmixHoaOrderPerParSubBandIdx;

	// compute number of upmix signals per sub-band group
	unsigned int uiNoOfSubbandGroups = m_vuiParUpmixHoaOrdersPerParSubBand.size();

	m_vuiParNoOfUpmixSignalsPerParSubBand.resize(uiNoOfSubbandGroups);

	for (unsigned int uiSubbandGroupIdx = 0; uiSubbandGroupIdx < uiNoOfSubbandGroups; uiSubbandGroupIdx++)
	{
		m_vuiParNoOfUpmixSignalsPerParSubBand[uiSubbandGroupIdx] = (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx] + 1) *
																   (m_vuiParUpmixHoaOrdersPerParSubBand[uiSubbandGroupIdx] + 1);
	}

	if (uiNoOfSubbandGroups > 0)
	{
		m_iMaxPARHOAOrder = static_cast<int>(*std::max_element(m_vuiParUpmixHoaOrdersPerParSubBand.begin(), m_vuiParUpmixHoaOrdersPerParSubBand.end()));
	}
	else
	{
		m_iMaxPARHOAOrder = -1;
	}

	// update dependent parameters
	m_uiNoOfHOACoeffs = (m_uiHOAOrder + 1) * (m_uiHOAOrder + 1);
	m_uiMinNoOfCoeffsForAmbHOA = static_cast<unsigned int>((m_iMinOrderForAmbHOA + 1) * (m_iMinOrderForAmbHOA + 1));
	m_uiMaxNoOfTransmittedHOACoeffs = (m_iMaxOrderToBeTransmitted + 1) * (m_iMaxOrderToBeTransmitted + 1);

	m_uiVVecStartCh = 0;
	if (0 < m_uiCodedVVecLength)
	{
		m_uiVVecStartCh = m_uiMinNoOfCoeffsForAmbHOA;
	}

	m_uiNoOfVecElems = m_uiNoOfHOACoeffs - m_uiVVecStartCh; // bug fix 2014-03-20 NP

	// currently assumed to be fixed
	// may be moved to access unit later to make it variable
	m_uiNoBitsForVecElemQuant = 8;
}

unsigned int SpatialDecoder::getNumberOfHoaCoefficients()
{
	return m_uiNoOfHOACoeffs;
}

unsigned int SpatialDecoder::getNoOfQMFDelaySamples()
{
	return m_shprtSubBandDirSigSynth->getNoOfQMFDelaySamples();
}

bool SpatialDecoder::process(const std::vector<std::vector<FLOAT>> &p_rvvFInputMCSampleBuffer,
							 std::vector<std::vector<FLOAT>> &p_rvvFOutputFinalComposedHOACoeffsFrameBuffer,
							 const HOA_FRAME &p_rFrame)
{
	if (!m_bIsInit)
	{
		return true;
	}

	// decode frame side info
	if (decodeFrameSideInfo(p_rFrame, m_shptrFrameParams))
	{
		return true;
	}

	// set last gain to that provided by the independency frame
	if (p_rFrame.m_bHoaIndependencyFlag)
	{
		for (unsigned int uiIdx = 0; uiIdx < m_uiTotalNoOfPercCoders; uiIdx++)
		{
			m_vshptrAllDynCorr[uiIdx]->setLastGain(p_rFrame.m_vGainCorrectionData[uiIdx].m_nGainCorrAbsAmpExp);
		}
	}

	// process inverse dynamic correction
	for (unsigned int uiIdx = 0; uiIdx < m_uiTotalNoOfPercCoders; uiIdx++)
	{
		if (m_vshptrAllDynCorr[uiIdx]->process(p_rvvFInputMCSampleBuffer[uiIdx], m_vvFDynCorrMCSampleBuffer[uiIdx], m_shptrFrameParams->m_vbExceptions[uiIdx], m_shptrFrameParams->m_viExponents[uiIdx]))
		{
			return true;
		}
	}

	// process channel reassignment
	if (m_shptrReAssign->process(m_vvFDynCorrMCSampleBuffer,
								 m_vvFDecPSSigsFrameBuffer,
								 m_vvFPreliminaryDecAmbHOACoeffsFrameBuffer,
								 m_shptrFrameParams->m_vuiAmbHOAAssignmentVec,
								 m_shptrFrameParams->m_muiActiveDirAndGridDirIndices,
								 m_shptrFrameParams->m_muivFVectors))
	{
		return true;
	}

	// process predominant sound synthesis
	if (m_shptrPreDomSoundSynth->process(m_vvFDecPSSigsFrameBuffer,
										 m_vvFPreDomSoundsHOAFrameBuffer,
										 m_vvFOutputFadedForPARPreDomSoundsHOAFrame,
										 m_vvFDirSigsHOAFrameBuffer,
										 m_vvFVecBasedPreDomSoundsHOAFrameBuffer,
										 m_shptrFrameParams->m_muiActiveDirAndGridDirIndices,
										 m_shptrFrameParams->m_vuiPredictionTypeVec,
										 m_shptrFrameParams->m_vvuiPredictionIndicesMat,
										 m_shptrFrameParams->m_vviQuantizedPredictionFactorsMat,
										 m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled,
										 m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled,
										 m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices,
										 m_shptrFrameParams->m_muivFVectors))
	{
		return true;
	}

	// process ambience synthesis

	if (m_shptrAmbSynth->process(m_vvFPreliminaryDecAmbHOACoeffsFrameBuffer,
								 m_vvFAmbHOACoeffsFrameBuffer))
	{
		return true;
	}

	// process preliminary HOA composition
	if (m_shptrPrelimHOAComp->process(m_vvFPreDomSoundsHOAFrameBuffer,
									  m_vvFOutputFadedForPARPreDomSoundsHOAFrame,
									  m_vvFInputForPARHOAFrameBuffer,
									  m_vvFAmbHOACoeffsFrameBuffer,
									  m_vvFPrelimComposedHOACoeffsFrameBuffer))
	{
		return true;
	}

	// process synthesis fo HOA component of predicted directional sub-band signals
	if (m_shprtSubBandDirSigSynth->process(m_vvFAmbHOACoeffsFrameBuffer,
										   m_shptrFrameParams,
										   m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled,
										   m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled,
										   m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices,
										   m_shptrFrameParams->m_bDoDirSubBandPred,
										   m_vvFSubBandDirSigsCompositionHOAFrameBuffer))
	{
		return true;
	}

	// process PAR decoding
	if (m_shptrPARDecoder->process(m_vvFInputForPARHOAFrameBuffer,
								   m_shptrFrameParams->m_vvvcFPARMixingMats,
								   m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices,
								   m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled,
								   m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled,
								   m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices,
								   m_shptrFrameParams->m_bUsePAR,
								   m_vvFPARHOAFrameBuffer))
	{
		return true;
	}

	// process final HOA composition
	if (m_shptrFinalHOAComp->process(m_vvFPrelimComposedHOACoeffsFrameBuffer,
									 m_vvFPARHOAFrameBuffer,
									 m_vvFSubBandDirSigsCompositionHOAFrameBuffer,
									 p_rvvFOutputFinalComposedHOACoeffsFrameBuffer))
	{
		return true;
	}

	return false;
}

bool SpatialDecoder::decodeFrameSideInfo(const HOA_FRAME &p_rFrame, std::shared_ptr<FrameParams> p_shprtFrameParams)
{

	// check dimensions
	if (checkSideInfo(p_rFrame))
	{
		return true;
	}

	// decode gain correction exponents

	for (unsigned int uiChanIdx = 0; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		unsigned int uiTmpCodedExp = 0;

		bool bFoundOne = false;

		while (!bFoundOne)
		{
			bFoundOne = p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bCodedGainCorrectionExp[uiTmpCodedExp];
			uiTmpCodedExp++;
		}

		switch (uiTmpCodedExp)
		{
		case 1:
		{
			p_shprtFrameParams->m_viExponents[uiChanIdx] = 0;
			break;
		}
		case 2:
		{
			p_shprtFrameParams->m_viExponents[uiChanIdx] = -1;
			break;
		}
		default:
		{
			p_shprtFrameParams->m_viExponents[uiChanIdx] = uiTmpCodedExp - 2;
		}
		}
	}

	// decode gain correction exceptions
	for (unsigned int uiChanIdx = 0; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		p_shprtFrameParams->m_vbExceptions[uiChanIdx] = p_rFrame.m_vGainCorrectionData[uiChanIdx].m_bGainCorrectionException;
	}

	// decode prediction parameters
	if (!p_rFrame.m_spatPredictionData.m_bPerformPrediction)
	{
		m_shptrFrameParams->m_vuiPredictionTypeVec.assign(m_uiNoOfHOACoeffs, 0);
	}
	else
	{
		unsigned int uiTotalPredictionTypeVecIdx = 0;
		unsigned int uiTotalPredictionIndicesMatIdx = 0;
		unsigned int uiTotalFPredictionFactorsMatIdx = 0;

		for (unsigned int uiGridDirIdx = 0; uiGridDirIdx < m_uiNoOfHOACoeffs; uiGridDirIdx++)
		{
			if (p_rFrame.m_spatPredictionData.m_bActivePred[uiGridDirIdx])
			{
				m_shptrFrameParams->m_vuiPredictionTypeVec[uiGridDirIdx] = 1;

				uiTotalPredictionTypeVecIdx++;

				for (unsigned int uiPredIdx = 0; uiPredIdx < m_uiMaxNofOfDirSigsForPred; uiPredIdx++)
				{
					m_shptrFrameParams->m_vvuiPredictionIndicesMat[uiPredIdx][uiGridDirIdx] = p_rFrame.m_spatPredictionData.m_unPredIds[uiTotalPredictionIndicesMatIdx];
					uiTotalPredictionIndicesMatIdx++;

					if (m_shptrFrameParams->m_vvuiPredictionIndicesMat[uiPredIdx][uiGridDirIdx] != 0)
					{
						m_shptrFrameParams->m_vviQuantizedPredictionFactorsMat[uiPredIdx][uiGridDirIdx] = p_rFrame.m_spatPredictionData.m_nPredGains[uiTotalFPredictionFactorsMatIdx];

						uiTotalFPredictionFactorsMatIdx++;
					}
				}
			}
			else
			{
				m_shptrFrameParams->m_vuiPredictionTypeVec[uiGridDirIdx] = 0;
			}
		}
	}

	// decode active direction indices and respective grid indices
	// as well as indices of coefficient sequences of the ambient HOA component

	// buffer old assignment vector
	std::vector<unsigned int> vuiOldAmbHOAAssignmentVec(m_shptrFrameParams->m_vuiAmbHOAAssignmentVec.begin(), m_shptrFrameParams->m_vuiAmbHOAAssignmentVec.end());

	// map for new nominators
	std::map<unsigned int, std::vector<FLOAT>> muivFNewNominators;

	// clear maps
	m_shptrFrameParams->m_muiActiveDirAndGridDirIndices.clear();
	m_shptrFrameParams->m_muivFVectors.clear();

	// clear index sets
	m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled.clear();
	m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled.clear();
	m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices.clear();

	// create set of all active ambient HOA coefficient indices
	std::set<unsigned int> suiAllActHOACoeffIndices;
	;
	for (unsigned int uiCoeffIdx = 0; uiCoeffIdx < m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx++)
	{
		suiAllActHOACoeffIndices.insert(uiCoeffIdx + 1);
	}

	// set predefined values values of assignment vector
	m_shptrFrameParams->m_vuiAmbHOAAssignmentVec.assign(m_uiTotalNoOfPercCoders, 0);
	for (unsigned int uiCoeffIdx = 0; uiCoeffIdx < m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx++)
	{
		m_shptrFrameParams->m_vuiAmbHOAAssignmentVec[m_uiTotalNoOfPercCoders - m_uiMinNoOfCoeffsForAmbHOA + uiCoeffIdx] = uiCoeffIdx + 1;
		m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices.insert(uiCoeffIdx + 1);
	}

	FLOAT FDenominator = static_cast<FLOAT>(getPow2(15));
	unsigned int uiChannelIdx = 0;

	// read side information for each channel
	for (std::vector<std::shared_ptr<CChannelSideInfoData>>::const_iterator it = p_rFrame.m_vChannelSideInfo.begin(); it < p_rFrame.m_vChannelSideInfo.end(); it++)
	{
		switch ((*it)->m_unChannelType)
		{
			// directional channel
		case DIR_CHANNEL:
		{
			const CDirectionalInfoChannel *pDirInfoChannelData = dynamic_cast<CDirectionalInfoChannel *>(it->get());
			m_shptrFrameParams->m_muiActiveDirAndGridDirIndices.insert(std::pair<unsigned int, unsigned int>(uiChannelIdx + 1, pDirInfoChannelData->m_unActiveDirIds));
		}
		break;
			// add HOA Ambient channel
		case ADD_HOA_CHANNEL:
		{
			const CAddAmbHoaInfoChannel *pAddAmbHOAInfoData = dynamic_cast<CAddAmbHoaInfoChannel *>(it->get());

			if (1 == pAddAmbHOAInfoData->m_unAmbCoeffIdxTransitionState)
			{
				m_shptrFrameParams->m_vuiAmbHOAAssignmentVec[uiChannelIdx] = pAddAmbHOAInfoData->m_unAmbCoeffIdx;

				m_shptrFrameParams->m_suiAmbCoeffIndicesToBeEnabled.insert(pAddAmbHOAInfoData->m_unAmbCoeffIdx);
			}
			else
			{
				if (p_rFrame.m_bHoaIndependencyFlag)
					m_shptrFrameParams->m_vuiAmbHOAAssignmentVec[uiChannelIdx] = pAddAmbHOAInfoData->m_unAmbCoeffIdx;
				else
					m_shptrFrameParams->m_vuiAmbHOAAssignmentVec[uiChannelIdx] = vuiOldAmbHOAAssignmentVec[uiChannelIdx];

				if (2 == pAddAmbHOAInfoData->m_unAmbCoeffIdxTransitionState)
				{
					m_shptrFrameParams->m_suiAmbCoeffIndicesToBeDisabled.insert(pAddAmbHOAInfoData->m_unAmbCoeffIdx);
				}
				else
				{
					m_shptrFrameParams->m_suiNonEnDisAbleActHOACoeffIndices.insert(pAddAmbHOAInfoData->m_unAmbCoeffIdx);
				}
			}
			suiAllActHOACoeffIndices.insert(pAddAmbHOAInfoData->m_unAmbCoeffIdx);
		}
		break;

			// vector-based channel
		case VEC_CHANNEL:
		{
			const CVectorBasedInfoChannel *pVecInfoChannelData = dynamic_cast<CVectorBasedInfoChannel *>(it->get());

			// init vector by zeros
			std::vector<FLOAT> vFTmpVec(m_uiNoOfVecElems, static_cast<FLOAT>(0.0));
			std::vector<FLOAT> vFTmpVecLong(m_uiNoOfHOACoeffs, static_cast<FLOAT>(0.0));
			// init vector of new nominators
			muivFNewNominators.insert(std::pair<unsigned int, std::vector<FLOAT>>(uiChannelIdx + 1, vFTmpVec));

			if (pVecInfoChannelData->m_unNbitsQ > 15)
			{
				return true;
			}

			if (cVVecVQWord == pVecInfoChannelData->m_unNbitsQ)

			{
				FLOAT FWeight;
				FLOAT FInvNormCoeff = getPow2(14) / static_cast<FLOAT>(m_uiHOAOrder + 1); // bugfix NP 2015-03-01

				const FLOAT *pFCurrVqVec = &(m_vvFTransposeModeMatAllFineGridPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][m_uiVVecStartCh]);

				bool bNormalize = 1;
				if (pVecInfoChannelData->m_unVVecDirections == 1)
				{

					switch (pVecInfoChannelData->m_unIndicesCodebookIdx)
					{
					case 0:
						pFCurrVqVec = &(m_vvFTransposeModeMatAllFineGridPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][0]);
						break;
					case 1:
						pFCurrVqVec = &(m_vvFDictCicpSpeakerPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][0]);
						break;
					case 2:
						pFCurrVqVec = &(m_vvFDictCicpSpeakerPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][0]);
						break;
					case 3:
						pFCurrVqVec = &(m_vvFDict2DPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][0]);
						break;
					case 7:
						pFCurrVqVec = &m_vvFInvModeMatAllCoarseGridPoints[pVecInfoChannelData->m_vunDirectionIndices[0] - 1][0];
					}
					FWeight = 2 * pVecInfoChannelData->m_vbSign[0] - 1;
					for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; ++uiElemIdx)
					{
						vFTmpVec[uiElemIdx] = FWeight * pFCurrVqVec[uiElemIdx + m_uiVVecStartCh];
					}
				}
				else
				{
					for (unsigned int uiVecIdx = 0; uiVecIdx < pVecInfoChannelData->m_unVVecDirections; ++uiVecIdx)
					{
						bNormalize = 1;
						switch (pVecInfoChannelData->m_unIndicesCodebookIdx)
						{
						case 0:
							pFCurrVqVec = &(m_vvFTransposeModeMatAllFineGridPoints[pVecInfoChannelData->m_vunDirectionIndices[uiVecIdx] - 1][0]);
							break;
						case 1:
							pFCurrVqVec = &(m_vvFDictCicpSpeakerPoints[pVecInfoChannelData->m_vunDirectionIndices[uiVecIdx] - 1][0]);
							break;
						case 2:
							bNormalize = 0;
							pFCurrVqVec = &(m_vvFDictCicpSpeakerPoints[pVecInfoChannelData->m_vunDirectionIndices[uiVecIdx] - 1][0]);
							break;
						case 3:
							pFCurrVqVec = &(m_vvFDict2DPoints[pVecInfoChannelData->m_vunDirectionIndices[uiVecIdx] - 1][0]);
							break;
						case 7:
							pFCurrVqVec = &m_vvFInvModeMatAllCoarseGridPoints[pVecInfoChannelData->m_vunDirectionIndices[uiVecIdx] - 1][0];
						}

						if (uiVecIdx < 8)
						{
							FWeight = m_vvFWeightingCdbk256x8[pVecInfoChannelData->m_unWeightingCodebookIdx][uiVecIdx];
						}
						else
						{
							FWeight = m_vvFWeightingCdbk256x8[pVecInfoChannelData->m_unWeightingCodebookIdx][uiVecIdx % 2 + 6];
						}

						FWeight *= 2 * pVecInfoChannelData->m_vbSign[uiVecIdx] - 1;
						for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfHOACoeffs; ++uiElemIdx)
						{
							vFTmpVecLong[uiElemIdx] += FWeight * pFCurrVqVec[uiElemIdx];
						}
					}

					// rescaling to L2 norm = (m_uiHOAOrder+1) and removing elements with indices smaller than m_uiVVecStartCh
					FLOAT FNorm = 1.0;
					if (1 == bNormalize)
					{
						FNorm = 0.0;
						for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfHOACoeffs; ++uiElemIdx)
						{
							FNorm += vFTmpVecLong[uiElemIdx] * vFTmpVecLong[uiElemIdx];
						}
						FNorm = static_cast<FLOAT>(m_uiHOAOrder + 1) / sqrt(FNorm);
					}

					for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; ++uiElemIdx)
					{
						vFTmpVec[uiElemIdx] = vFTmpVecLong[uiElemIdx + m_uiVVecStartCh] * FNorm;
					}
				}
				// 2. zeroing vector elements corresponding to ADD_HOA coefficients when codedVVecLength = 1
				if (1 == m_uiCodedVVecLength)
				{
					for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; ++uiElemIdx)
					{
						if (!pVecInfoChannelData->m_vbAdditionalInfo[uiElemIdx])
						{ // bugfix 2015-03-01
							vFTmpVec[uiElemIdx] = 0.0;
						}
					}
				}

				// 3. updating buffer used for SQ predictive coding
				for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; ++uiElemIdx)
				{
					muivFNewNominators[uiChannelIdx + 1][uiElemIdx] = 2.0 * floor(0.5 + (vFTmpVec[uiElemIdx] * FInvNormCoeff)); // bugfix NP 2015-03-01
				}
			}

			else if (c8BitQuantizerWord == pVecInfoChannelData->m_unNbitsQ)

			{
				// simple 8 bits dequantization
				// decode each vector element
				for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; uiElemIdx++)
				{
					if (dequantizeUniform(pVecInfoChannelData->m_vun8bitCodedVelement[uiElemIdx], m_uiNoBitsForVecElemQuant, vFTmpVec[uiElemIdx]))
					{
						return true;
					}

					// updating buffer used for prediction
					muivFNewNominators[uiChannelIdx + 1][uiElemIdx] = vFTmpVec[uiElemIdx] * static_cast<unsigned int>(FDenominator);
					vFTmpVec[uiElemIdx] *= static_cast<FLOAT>(m_uiHOAOrder + 1);
				}
			}

			else if (c8BitQuantizerWord < pVecInfoChannelData->m_unNbitsQ)

			{
				FLOAT FNominator;
				// decode each vector element
				for (unsigned int uiElemIdx = 0; uiElemIdx < m_uiNoOfVecElems; uiElemIdx++)
				{
					FNominator = 0.0;
					if (pVecInfoChannelData->m_vunDecodedHuffmannWord[uiElemIdx])
					{
						int iSign = pVecInfoChannelData->m_vbSign[uiElemIdx] * 2 - 1;
						int iVal = iSign * (static_cast<int>(getPow2(pVecInfoChannelData->m_vunDecodedHuffmannWord[uiElemIdx] - 1) + pVecInfoChannelData->m_vunAdditionalValue[uiElemIdx]));
						FNominator = static_cast<FLOAT>(static_cast<int>(getPow2(16 - pVecInfoChannelData->m_unNbitsQ)) * iVal);
					}

					if (pVecInfoChannelData->m_bPFlag && pVecInfoChannelData->m_vbElementBitmask[uiElemIdx]) // bugfix 29-05-2015 NP
					{
						if (muivFOldNominators.find(uiChannelIdx + 1) != muivFOldNominators.end())
						{
							FNominator += muivFOldNominators[uiChannelIdx + 1][uiElemIdx];
						}
					}

					// set new nominator
					muivFNewNominators[uiChannelIdx + 1][uiElemIdx] = FNominator;

					// set new vector element
					vFTmpVec[uiElemIdx] = FNominator / FDenominator;
					vFTmpVec[uiElemIdx] *= static_cast<FLOAT>(m_uiHOAOrder + 1);
				}
			}

			// insert vector into map
			m_shptrFrameParams->m_muivFVectors.insert(std::pair<unsigned int, std::vector<FLOAT>>(uiChannelIdx + 1, vFTmpVec));
		}

		break;

			// Empty and default channel
		default:;
		}

		uiChannelIdx++;
	}

	// buffer nominators
	muivFOldNominators = muivFNewNominators;

	m_shptrFrameParams->m_bDoDirSubBandPred = p_rFrame.m_hoaDirectionalPredictionInfo.m_bUseDirPred;

	// decode grid indices for directions of predicted sub band signals
	// as well as complex weighting factors for subband directional predicition
	m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.clear();
	for (std::vector<unsigned int>::const_iterator vecIt = p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.begin();
		 vecIt != p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.end(); ++vecIt)
	{
		m_shptrFrameParams->m_suiAllActiveDirAndGridDirIncidesForSubbandPred.insert(*vecIt);
	}

	for (unsigned int uiSubBandIdx = 0; uiSubBandIdx < m_uiNoOfSubBandsForDirPred; uiSubBandIdx++)
	{
		if (!p_rFrame.m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[uiSubBandIdx])
		{
			m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].clear();
			for (unsigned int uiDirIdx = 0; uiDirIdx < m_uiMaxNoOfPredDirsPerSubBand; uiDirIdx++)
			{
				if (m_shptrFrameParams->m_bDoDirSubBandPred & p_rFrame.m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[uiSubBandIdx][uiDirIdx])
				{
					m_shptrFrameParams->m_vmuiActiveSubBandDirAndGridDirIncides[uiSubBandIdx].insert(
						std::pair<unsigned int, unsigned int>(uiDirIdx + 1,
															  p_rFrame.m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds[p_rFrame.m_hoaDirectionalPredictionInfo.m_vvunRelDirGridIdx[uiSubBandIdx][uiDirIdx]]));

					for (unsigned int uiCoffIdx = 0; uiCoffIdx < m_uiMaxNoOfTransmittedHOACoeffs; uiCoffIdx++)
					{
						if (suiAllActHOACoeffIndices.find(uiCoffIdx + 1) != suiAllActHOACoeffIndices.end())
						{

							int iCurrentIntQuantMagnitude = m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoffIdx] + p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[uiSubBandIdx][uiDirIdx][uiCoffIdx];
							int iCurrentIntQuantAngle = 0;

							if (iCurrentIntQuantMagnitude != 0)
							{
								iCurrentIntQuantAngle = m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoffIdx] + p_rFrame.m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[uiSubBandIdx][uiDirIdx][uiCoffIdx];

								// constrain the quantized angle to lie in interval [-7,...,8] (corresponding to ]-pi,pi])
								iCurrentIntQuantAngle = (iCurrentIntQuantAngle - m_iLowestQuantValForAngleForDirPred) % static_cast<int>(m_uiNoOfQuantIntervForAngleForDirPred) + m_iLowestQuantValForAngleForDirPred;
							}

							FLOAT FCurrentManitude = static_cast<FLOAT>(0.0);
							if (iCurrentIntQuantMagnitude <= 8)
								FCurrentManitude = static_cast<FLOAT>(iCurrentIntQuantMagnitude) * m_FQuantIntLenForMagnForDirPred;
							else
							{
								FCurrentManitude = pow(static_cast<FLOAT>(8.0 / 7.0),
													   static_cast<FLOAT>(iCurrentIntQuantMagnitude - 8));
							}
							FLOAT FCurrentAngle = static_cast<FLOAT>(iCurrentIntQuantAngle) * m_FQuantIntLenForAngleForDirPred;

							m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats[uiSubBandIdx][uiDirIdx][uiCoffIdx] =
								FCurrentManitude * std::complex<FLOAT>(std::cos(FCurrentAngle), std::sin(FCurrentAngle));
							// FCurrentManitude * std::exp( std::complex<FLOAT>( static_cast<FLOAT>(0.0), FCurrentAngle ) );

							m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoffIdx] = iCurrentIntQuantMagnitude;
							m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoffIdx] = iCurrentIntQuantAngle;
						}
						else
						{
							m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats[uiSubBandIdx][uiDirIdx][uiCoffIdx] = static_cast<FLOAT>(0.0);
							m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx][uiCoffIdx] = static_cast<FLOAT>(0.0);
							m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx][uiCoffIdx] = static_cast<FLOAT>(0.0);
						}
					}
				}
				else
				{
					m_shptrFrameParams->m_vvvcFComplexDirPredictionCoeffsMats[uiSubBandIdx][uiDirIdx].assign(m_uiMaxNoOfTransmittedHOACoeffs,
																											 std::complex<FLOAT>(static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)));

					m_vvviOldPredCoeffsIntQuantMagnitudes[uiSubBandIdx][uiDirIdx].assign(m_uiMaxNoOfTransmittedHOACoeffs, 0);
					m_vvviOldPredCoeffsIntQuantAngles[uiSubBandIdx][uiDirIdx].assign(m_uiMaxNoOfTransmittedHOACoeffs, 0);
				}
			}
		}
	}

	// PAR related side information
	m_shptrFrameParams->m_bUsePAR = p_rFrame.m_hoaParInfo.m_bPerformPar;
	m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices.resize(m_vuiParSubbandWidths.size());

	if (m_shptrFrameParams->m_bUsePAR)
	{
		// for each PAR sub-band group
		for (unsigned int uiPARSubBandGroupIdx = 0; uiPARSubBandGroupIdx < m_vuiParSubbandWidths.size(); uiPARSubBandGroupIdx++)
		{
			unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiPARSubBandGroupIdx];

			if (!p_rFrame.m_hoaParInfo.m_vbKeepPrevMatrixFlag[uiPARSubBandGroupIdx])
			{

				std::vector<std::vector<unsigned int>> vvuiCurrParSelectedDecorrSigsIdxMatrix = p_rFrame.m_hoaParInfo.m_vvvunParSelectedDecorrSigsIdxMatrix[uiPARSubBandGroupIdx];

				// get number of used de-correlated signals to create an upmix signal
				unsigned int uiCurrNoOfDecorrSigs = vvuiCurrParSelectedDecorrSigsIdxMatrix[0].size();

				for (unsigned int uiUpMixSigIdx = 0; uiUpMixSigIdx < uiCurrNoOfUpmixSigs; uiUpMixSigIdx++)
				{
					m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);
					m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);

					m_shptrFrameParams->m_vvvcFPARMixingMats[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs,
																										 std::complex<FLOAT>(static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)));

					if (p_rFrame.m_hoaParInfo.m_vvbUseParUpmixSig[uiPARSubBandGroupIdx][uiUpMixSigIdx])
					{
						for (unsigned int uiIdx = 0; uiIdx < uiCurrNoOfDecorrSigs; uiIdx++)
						{
							unsigned int uiCurrColIdx = vvuiCurrParSelectedDecorrSigsIdxMatrix[uiUpMixSigIdx][uiIdx];

							m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] =
								m_vvviOldPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] +
								p_rFrame.m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx];

							if (m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] == 0)
							{
								m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] = 0;
							}
							else
							{
								m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] =
									m_vvviOldPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] +
									p_rFrame.m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx];

								// constrain the quantized angle to lie in interval [-7,...,8] (corresponding to ]-pi,pi])
								m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] = (m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] - m_iLowestQuantValForAngleForPAR) % static_cast<int>(m_uiNoOfQuantIntervForAngleForPAR) + m_iLowestQuantValForAngleForPAR;
							}

							FLOAT FCurrentManitude = static_cast<FLOAT>(0.0);
							int iCurrentIntQuantMagnitude = m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx];
							if (iCurrentIntQuantMagnitude <= 8)
								FCurrentManitude = static_cast<FLOAT>(iCurrentIntQuantMagnitude) * m_FQuantIntLenForMagnForPAR;
							else
							{
								FCurrentManitude = pow(static_cast<FLOAT>(8.0 / 7.0),
													   static_cast<FLOAT>(iCurrentIntQuantMagnitude - 8));
							}
							FLOAT FCurrentAngle = static_cast<FLOAT>(m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx]) * m_FQuantIntLenForAngleForPAR;

							m_shptrFrameParams->m_vvvcFPARMixingMats[uiPARSubBandGroupIdx][uiUpMixSigIdx][uiCurrColIdx] =
								FCurrentManitude * std::complex<FLOAT>(std::cos(FCurrentAngle), std::sin(FCurrentAngle));
						}
					}

					// update buffers
					m_vvviOldPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].begin(),
																									 m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].end());
					m_vvviOldPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].begin(),
																								 m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].end());
				}
			}
		}

		// indices signaling the permutation to de-correlation filters for each sub-band group
		for (unsigned int uiPARSubBandGroupIdx = 0; uiPARSubBandGroupIdx < m_vuiParSubbandWidths.size(); uiPARSubBandGroupIdx++)
		{
			m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices[uiPARSubBandGroupIdx] = p_rFrame.m_hoaParInfo.m_vunParDecorrSigsSelectionTableIdx[uiPARSubBandGroupIdx];
		}
	}
	else
	{
		// for each PAR sub-band group
		for (unsigned int uiPARSubBandGroupIdx = 0; uiPARSubBandGroupIdx < m_vuiParSubbandWidths.size(); uiPARSubBandGroupIdx++)
		{
			unsigned int uiCurrNoOfUpmixSigs = m_vuiParNoOfUpmixSignalsPerParSubBand[uiPARSubBandGroupIdx];

			for (unsigned int uiUpMixSigIdx = 0; uiUpMixSigIdx < uiCurrNoOfUpmixSigs; uiUpMixSigIdx++)
			{
				m_vvviCurrPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);
				m_vvviCurrPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);

				m_shptrFrameParams->m_vvvcFPARMixingMats[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs,
																									 std::complex<FLOAT>(static_cast<FLOAT>(0.0), static_cast<FLOAT>(0.0)));
				// update buffers to zero
				m_vvviOldPARMixMatIntQuantMagnitudes[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);
				m_vvviOldPARMixMatIntQuantAngles[uiPARSubBandGroupIdx][uiUpMixSigIdx].assign(uiCurrNoOfUpmixSigs, 0);
			}

			m_shptrFrameParams->m_vuiParDecorrSigsSelectionTableIndices[uiPARSubBandGroupIdx] = 0;
		}
	}

	return false;
}

bool SpatialDecoder::checkSideInfo(const HOA_FRAME &p_rFrame)
{
	if ((p_rFrame.m_vChannelSideInfo.size() != m_uiTotalNoOfPercCoders - m_uiMinNoOfCoeffsForAmbHOA) ||
		(p_rFrame.m_spatPredictionData.m_bActivePred.size() != m_uiNoOfHOACoeffs))
	{
		return true;
	}

	return false;
}
