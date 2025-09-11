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
 $Id: SpatialEncoder.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/


#pragma once

#include <memory>
#include <map>
#include <algorithm>
#include "DirectionAndVectorEstimation.h"
#include "HOADecomposition.h"
#include "AmbientComponentModification.h"
#include "ChannelAssignment.h"
#include "DynCorrection.h"
#include "DataTypes.h"
#include "FrameParams.h"
#include "choabitstream.h"
#include "AssignmentInfo.h"
#include "QMF_Wrapper.h"
#include "SubbandDirEstimation.h"
#include "SubbandDirSigsPrediction.h"
#include "SpatialDecoder.h"
#include "PAREncoder.h"

#ifdef USAC_CORE
  #define HOA_CONFIG HoaConfig
  #define HOA_FRAME HoaFrame
#else
  #define HOA_CONFIG CHoaBsAccessFrame
  #define HOA_FRAME CHoaBsStdFrame
#endif

class SpatialEncoder
{
	public:

		SpatialEncoder();

		bool init(	unsigned int p_uiFrameSize, 
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
               const HoaFrame &p_crHOAEmptyFrame);

		bool process(const std::vector<std::vector<FLOAT>> &p_rvvFInputHOAFrame,
										   std::vector<std::vector<FLOAT>> &p_rvvFOutputGainAdaptedSigsToBeCodedFrame,
										   HOA_FRAME &p_rFrame);

		bool isEndOfOutputFile();

		void setEndOfInputFile();

		bool getBitRate(std::vector<unsigned int> &p_vuiBitRateVec);

	private:

		bool m_bIsInit;

		bool m_bIsEndOfInputFile;
		bool m_bIsEndOfOutputFile;

		std::vector<AssignmentInfo> m_vAITargetAssignmentVector;
		std::vector<AssignmentInfo> m_vAIAssignmentVector;

		unsigned int m_uiDelayFrameCounter;
		unsigned int m_uiNoOfRequiredDelayFrames;

		unsigned int m_uiFrameSize;
		unsigned int m_uiHOAOrder;
		unsigned int m_uiMaxNoOfDirSigs;
		unsigned int m_uiMinOrderForAmbHOA;
		unsigned int m_uiTotalNoOfPercCoders;
		unsigned int m_uiBitRatePerPercCoder;
		unsigned int m_uiNoOfAddPercCoders;
		unsigned int m_uiMaxNofOfDirSigsForPred;
		unsigned int m_uiNoOfBitsPerScaleFactor;
		unsigned int m_uiMaxAmplifyExponent;

		unsigned int m_uiMinNoOfCoeffsForAmbHOA;
		unsigned int m_uiNoOfHOACoeffs;
		unsigned int m_uiNoOfPredIndToCodeExplicitly;

		unsigned int m_uiNoBitsForVecElemQuant;

      unsigned int    m_uiMaxNoOfTransmittedHOACoeffs;
      unsigned int    m_uiMaxNoOfPredDirsPerSubBand;
      unsigned int    m_uiNoOfSubBandsForDirPred;


      // for directional sub-band signals prediction
      unsigned int m_uiNoOfQuantIntervForAngleForDirPred;
      unsigned int m_uiNoOfQuantIntervForMagnForDirPred;
      int m_iLowestQuantValForAngleForDirPred;
      int m_iHighestQuantValForAngleForDirPred;
      int m_iHighestQuantValForMagForDirPred;
      FLOAT m_FQuantIntLenForMagnForDirPred;
      FLOAT m_FQuantIntLenForAngleForDirPred;
        
      // for PAR
      unsigned int m_uiNoOfQuantIntervForAngleForPAR; 
      unsigned int m_uiNoOfQuantIntervForMagnForPAR; 
      int m_iLowestQuantValForAngleForPAR;
      int m_iHighestQuantValForAngleForPAR;
      int m_iHighestQuantValForMagForPAR;
      FLOAT m_FQuantIntLenForMagnForPAR;
      FLOAT m_FQuantIntLenForAngleForPAR;

		// buffers

		// for zero input
			std::vector<std::vector<FLOAT>> m_vvFZeroFrame;

		// for HOA decomposition
			std::vector<std::vector<FLOAT>> m_vvFSmoothedPSSigsLastFrame;
			std::vector<std::vector<FLOAT>> m_vvFSmoothedPSSigsPenultimateFrame;

			std::vector<std::vector<FLOAT>> m_vvFPenultimateSmoothedPreDomSoundsHOAFrame;
			std::vector<std::vector<FLOAT>> m_vvFPenultimateAmbientHOAFrame;
			std::vector<std::vector<FLOAT>> m_vvFPredictedLastAmbientHOAFrame;

		// for ambient HOA component modification
			std::vector<std::vector<FLOAT>> m_vvFModifiedPenultimateAmbientHOAFrame;
			std::vector<std::vector<FLOAT>> m_vvFModifiedPredictedLastAmbientHOAFrame;
               std::set<unsigned int>          m_suiActiveAmbHOACoeffsIndices;

		// for channel assignment
			std::vector<std::vector<FLOAT>> m_vvFAllSigsToBeCodedFrame;
			std::vector<std::vector<FLOAT>> m_vvFPredictedAllSigsToBeCodedNextFrame;

      // for QMF analysis
         std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  m_vvvcFQMFLastAmbientHOAFrame;

      // for directional sub-band signals prediction
         std::vector<unsigned int> m_vuiSubBandWidths;

      // for spatial decoding
         std::vector<std::vector<FLOAT> > m_vvFComposedDelLastButOneHOAFrame;

      // for PAR encoding
         std::vector<std::vector<FLOAT> > m_vvFLastInputHOAFrame;
         std::vector<std::vector<FLOAT> > m_vvFLastButOneInputHOAFrame;
         std::vector<std::vector<FLOAT> > m_vvFLastButTwoInputHOAFrame;

                
         std::vector<std::vector<FLOAT> > m_vvFComposedLastButTwoHOAFrame;
         std::vector<std::vector<FLOAT> > m_vvFComposedDelLastButTwoHOAFrame;

         std::set<unsigned int> m_suiAmbCoeffIndicesToBeEnabledInLastButTwoFrame;
	      std::set<unsigned int> m_suiAmbCoeffIndicesToBeDisabledInLastButTwoFrame;
	      std::set<unsigned int> m_suiNonEnDisAbleActHOACoeffIndicesInLastButTwoFrame;

         std::vector<unsigned int> m_vuiParSubBandWidths;
         std::vector<unsigned int> m_vuiParNoOfUpmixSignalsPerParSubBand;

         std::vector<std::vector<FLOAT> > m_vvFGainAdaptedSigsToBeCodedLastButOneFrame;
         std::vector<std::vector<FLOAT> > m_vvFGainAdaptedSigsToBeCodedLastButTwoFrame;
  
      // for encoding side information related to complex prediction coefficients for sub band directional signals prediction
         std::vector<std::vector<std::vector<int> > > m_vvviOldPredCoeffsIntQuantMagnitudes;
         std::vector<std::vector<std::vector<int> > > m_vvviOldPredCoeffsIntQuantAngles;

      // for encoding PAR related side information
         std::vector<std::vector<std::vector<int> > > m_vvviOldParMixMatsIntQuantMagnitudes;
         std::vector<std::vector<std::vector<int> > > m_vvviOldParMixMatsIntQuantAngles;

		//shared pointers to components
		std::shared_ptr<FrameParams> m_shptrFrameParams;

		std::shared_ptr<TabulatedValues>				m_shptrTabVals;
		std::shared_ptr<DirectionAndVectorEstimation>	m_shptrDirAndVecEst;
		std::shared_ptr<HOADecomposition>				m_shptrHOADecomp;
		std::shared_ptr<AmbientComponentModification>	m_shptrAmbCompMod;
		std::shared_ptr<ChannelAssignment>				m_shptrChannAss;

      std::shared_ptr<CqmfAnalysis>                   m_shptrQMFAnalysis;
      std::shared_ptr<SubbandDirEstimation>           m_shptrSubBandDirEstimation;
      std::shared_ptr<SubbandDirSigsPrediction>       m_shptrSubBandDirSigsPred;

      std::shared_ptr<SpatialDecoder>                 m_shptrSpatDec;
      std::shared_ptr<PAREncoder>                     m_shptrPAREncoder;

      HOA_FRAME m_PenultimateHOASIFrame;
      HOA_FRAME m_LastButTwoHOASIFrame;

      bool encodePARFrameSideInfo(HOA_FRAME &p_rFrame);

		std::vector< std::shared_ptr<DynCorrection> > m_vshptrAllDynCorr;

		// member function to check input buffer size
		bool checkBufferSizes(const std::vector<std::vector<FLOAT>> &p_rvvFInputHOAFrame);

		bool encodeFrameSideInfo(HOA_FRAME &p_rFrame);

		static double RoundNumber(double p_dNumber);

};
