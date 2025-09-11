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
$Id: SpatialDecoder.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#pragma once

#include "InverseDynCorrection.h"
#include <iostream>
#include "choabitstream.h"
#include "ChannelReAssignment.h"
#include "PreDomSoundSynthesis.h"
#include "AmbienceSynthesis.h"
#include "PreliminaryHOAComposition.h"
#include "FrameParams.h"
#include "TabulatedValues.h"
#include "QMF_Wrapper.h"
#include <complex>
#include "SubBandDirSigsSynthesis.h"
#include "FinalHOAComposition.h"
#include "PARDecoder.h"

#ifdef USAC_CORE
#define HOA_CONFIG HoaConfig
#define HOA_FRAME HoaFrame
#else
#define HOA_CONFIG CHoaBsAccessFrame
#define HOA_FRAME CHoaBsStdFrame
#endif

class SpatialDecoder
{
public:

   SpatialDecoder();

   bool init(const HOA_CONFIG &p_pAccessUnitHeader, unsigned int p_uiFrameSize);


   bool process(	const std::vector<std::vector<FLOAT> > &p_rvvFInputMCSampleBuffer,
      std::vector<std::vector<FLOAT> > &p_rvvFOutputComposedHOACoeffsFrameBuffer,
      const HOA_FRAME &p_rFrame);

   unsigned int getNumberOfHoaCoefficients();

   unsigned int getNoOfQMFDelaySamples();

private:

   FLOAT			m_FNFCDistance_m;

   int				m_iMinOrderForAmbHOA;

   unsigned int	m_uiHOAOrder;
   unsigned int	m_uiNoOfHOACoeffs;
   unsigned int	m_uiMinNoOfCoeffsForAmbHOA;
   unsigned int	m_uiNoOfDomDirs;
   unsigned int	m_uiTotalNoOfPercCoders;
   unsigned int	m_uiMaxNofOfDirSigsForPred;
   unsigned int	m_uiNoOfBitsPerScaleFactor;
   unsigned int	m_uiFrameSize;
   unsigned int	m_uiNoOfVecElems;
   unsigned int	m_uiVVecStartCh;
   unsigned int	m_uiInterpSamples;
   unsigned int	m_uiInterpMethod;
   unsigned int	m_uiCodedVVecLength;
   unsigned int	m_uiNoBitsForVecElemQuant;
   int            m_iMaxOrderToBeTransmitted;
   unsigned int    m_uiMaxNoOfTransmittedHOACoeffs;

   // for prediction of directional sub-band signals 
   unsigned int    m_uiMaxNoOfPredDirsPerSubBand;
   unsigned int    m_uiNoOfSubBandsForDirPred;
   unsigned int    m_uiDirGridTableIdx;
   std::vector<unsigned int> m_vuiSubBandWidths;

   // for PAR
   int m_iMaxPARHOAOrder;
   std::vector<bool> m_vbUseRealCoeffsPerParSubband;
   std::vector<unsigned int> m_vuiParSubbandWidths;
   std::vector<unsigned int> m_vuiParUpmixHoaOrdersPerParSubBand;
   std::vector<unsigned int> m_vuiParNoOfUpmixSignalsPerParSubBand;

   //////////////////////

   bool    m_bUsePhaseShiftDecorr;

   bool	m_bIsInit;
   bool	m_bUseSBR;
   bool	m_bNFCFlag;

   // constants for requantization of complex prediction coefficients for directional sub band prediction
   unsigned int  m_uiNoOfQuantIntervForAngleForDirPred;
   unsigned int  m_uiNoOfQuantIntervForMagnForDirPred;

   int           m_iLowestQuantValForAngleForDirPred;

   FLOAT         m_FQuantIntLenForMagnForDirPred;
   FLOAT         m_FQuantIntLenForAngleForDirPred;

   // constants for requantization of mixing matrix for PAR
   unsigned int  m_uiNoOfQuantIntervForAngleForPAR;
   unsigned int  m_uiNoOfQuantIntervForMagnForPAR;
   int           m_iLowestQuantValForAngleForPAR;
   FLOAT         m_FQuantIntLenForMagnForPAR;
   FLOAT         m_FQuantIntLenForAngleForPAR;

   std::shared_ptr<FrameParams> m_shptrFrameParams;

   // shared pointers to components
   std::vector< std::shared_ptr<InverseDynCorrection> > m_vshptrAllDynCorr;

   std::shared_ptr<ChannelReAssignment> m_shptrReAssign; 

   std::shared_ptr<PreDomSoundSynthesis> m_shptrPreDomSoundSynth;

   std::shared_ptr<AmbienceSynthesis> m_shptrAmbSynth; 

   std::shared_ptr<PreliminaryHOAComposition> m_shptrPrelimHOAComp;

   std::shared_ptr<TabulatedValues> m_shptrTabVals;

   std::shared_ptr<SubBandDirSigsSynthesis> m_shprtSubBandDirSigSynth;

   std::shared_ptr<FinalHOAComposition> m_shptrFinalHOAComp;

   std::shared_ptr<PARDecoder> m_shptrPARDecoder;


   std::vector<std::vector<FLOAT> > m_vvFInvModeMatAllCoarseGridPoints;
   std::vector<std::vector<FLOAT> > m_vvFTransposeModeMatAllFineGridPoints;
   std::vector<std::vector<FLOAT> > m_vvFDictCicpSpeakerPoints;
   std::vector<std::vector<FLOAT> > m_vvFDict2DPoints;
   std::vector<std::vector<FLOAT> > m_vvFWeightingCdbk256x8;		

   // buffers

   // for decoding of side info related to vector based predominant sound synthesis
   std::map<unsigned int, std::vector<FLOAT> > muivFOldNominators;

   // for decoding side information related to complex prediction coefficients for sub band directional signals prediction
   std::vector<std::vector<std::vector<int> > > m_vvviOldPredCoeffsIntQuantMagnitudes;
   std::vector<std::vector<std::vector<int> > > m_vvviOldPredCoeffsIntQuantAngles;

   // for decoding side information related to mixing matrix for PAR
   std::vector<std::vector<std::vector<int> > > m_vvviCurrPARMixMatIntQuantMagnitudes;
   std::vector<std::vector<std::vector<int> > > m_vvviCurrPARMixMatIntQuantAngles;

   std::vector<std::vector<std::vector<int> > > m_vvviOldPARMixMatIntQuantMagnitudes;
   std::vector<std::vector<std::vector<int> > > m_vvviOldPARMixMatIntQuantAngles;

   // for inverse dynamic correction
   std::vector<std::vector<FLOAT> > m_vvFDynCorrMCSampleBuffer;

   // for channel assignment
   std::vector<std::vector<FLOAT> > m_vvFDecPSSigsFrameBuffer;
   std::vector<std::vector<FLOAT> > m_vvFPreliminaryDecAmbHOACoeffsFrameBuffer;

   // for predominant sound synthesis
   std::vector<std::vector<FLOAT> > m_vvFPreDomSoundsHOAFrameBuffer;
   std::vector<std::vector<FLOAT> > m_vvFQMFDelayedPreDomSoundsHOAFrameBuffer;
   std::vector<std::vector<FLOAT> > m_vvFDirSigsHOAFrameBuffer;
   std::vector<std::vector<FLOAT> > m_vvFVecBasedPreDomSoundsHOAFrameBuffer;

   // for ambience synthesis
   std::vector<std::vector<FLOAT> > m_vvFAmbHOACoeffsFrameBuffer;

   // for faded predominant sound HOA component to used for composition of input HOA representation to PAR
   std::vector<std::vector<FLOAT> > m_vvFOutputFadedForPARPreDomSoundsHOAFrame;

   // for synthesis of directional sub-band signals
   std::vector<std::vector<FLOAT> > m_vvFSubBandDirSigsCompositionHOAFrameBuffer;

   // for synthesis of parametrially replicated ambience
   std::vector<std::vector<FLOAT> > m_vvFInputForPARHOAFrameBuffer;
   std::vector<std::vector<FLOAT> > m_vvFPARHOAFrameBuffer;

   // for preliminary HOA composition
   std::vector<std::vector<FLOAT> > m_vvFPrelimComposedHOACoeffsFrameBuffer;

   bool decodeFrameSideInfo(const HOA_FRAME &p_rFrame, std::shared_ptr<FrameParams>);
   void updateParamsFromAccessUnitHeader(const HOA_CONFIG &p_rAccessUnitHeader);

   bool checkSideInfo(const HOA_FRAME &p_rFrame);
};
