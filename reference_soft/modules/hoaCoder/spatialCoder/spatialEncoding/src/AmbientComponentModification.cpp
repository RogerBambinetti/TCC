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
 $Rev: 179 $
 $Author: technicolor-kf $
 $Date: 2015-05-07 15:08:57 +0200 (Do, 07 Mai 2015) $
 $Id: AmbientComponentModification.cpp 179 2015-05-07 13:08:57Z technicolor-kf $
*/

#include "AmbientComponentModification.h"


AmbientComponentModification::AmbientComponentModification()
{
	m_bIsInit = false;
}

bool AmbientComponentModification::init(unsigned int p_uiFrameSize, 
										unsigned int p_uiHOAOrder, 
										unsigned int p_uiMinOrderForAmbHOA, 
										unsigned int p_uiTotalNoOfPercCoders, 
										unsigned int p_uiNoOfChannelsWithVariableType,
										unsigned int p_uiBitRatePerPercCoder,
										std::shared_ptr<TabulatedValues> p_shptrTabVals)
{

	m_uiFrameSize						= p_uiFrameSize;
	m_uiHOAOrder						= p_uiHOAOrder;
	m_uiMinOrderForAmbHOA				= p_uiMinOrderForAmbHOA;
	m_uiTotalNoOfPercCoders				= p_uiTotalNoOfPercCoders;
	m_uiNoOfChannelsWithVariableType	= p_uiNoOfChannelsWithVariableType;
	m_uiBitRatePerPercCoder				= p_uiBitRatePerPercCoder;

	m_uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
	m_uiMinNoOfCoeffsForAmbHOA	= (p_uiMinOrderForAmbHOA+1)*(p_uiMinOrderForAmbHOA+1);

	m_suiPotentialAmbHOACoeffsIndices.clear();
	for(unsigned int uiCoeffIdx=1; uiCoeffIdx <= m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
		m_suiPotentialAmbHOACoeffsIndices.insert(uiCoeffIdx);
	}

	m_suiActiveAmbHOACoeffsIndices.clear();
	m_suiAmbHOACoeffsIndicesToBeEnabled.clear();
	m_suiAmbHOACoeffsIndicesToBeDisabled.clear();

	// init inverse of low order mode matrix
	p_shptrTabVals->getInvModeMat(m_vvFPInvLowOrderModeMat, p_uiMinOrderForAmbHOA);

	// compute smoothing window
	computeSmoothWinFunc(m_vFSmoothWinFunc, p_uiFrameSize);

	// init previous vector with target assignment info
	m_vAIAssignmentVector.resize(m_uiNoOfChannelsWithVariableType);
	m_vAIPreviousTargetAssignmentVector.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfChannelsWithVariableType; uiChanIdx++)
	{
		m_vAIPreviousTargetAssignmentVector[uiChanIdx].init();
		m_vAIAssignmentVector[uiChanIdx].init();

	}

	for (unsigned int uiChanIdx=m_uiNoOfChannelsWithVariableType; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		m_suiActiveAmbHOACoeffsIndices.insert( uiChanIdx - m_uiNoOfChannelsWithVariableType + 1 );
	}

	m_bIsInit = true;

	return false;
}


bool AmbientComponentModification::process( const std::vector<std::vector<FLOAT>> &p_rvvFInputAmbientHOAFrame,
						                    const std::vector<std::vector<FLOAT>> &p_rvvFInputPredictedNextAmbientHOAFrame,
						                    const std::vector<AssignmentInfo> &p_vAITargetAssignmentVector,
							                      std::vector<std::vector<FLOAT>> &p_rvvFOutputModifiedAmbientHOAFrame,
							                      std::vector<std::vector<FLOAT>> &p_rvvFOutputModifiedPredictedNextAmbientHOAFrame,
							                      std::vector<AssignmentInfo> &p_vAIOutputFinalAssignmentVector,
                                                  std::set<unsigned int> &p_rsuiActiveAmbHOACoeffsIndices,
                                                  std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeEnabled,
                                                  std::set<unsigned int> &p_rsuiAmbCoeffIndicesToBeDisabled,
                                                  std::set<unsigned int> &p_rsuiNonEnDisAbleActHOACoeffIndices)
{

	if ( (m_uiNoOfChannelsWithVariableType > m_uiTotalNoOfPercCoders) )
	{
		return true;
	}

	if ( (m_uiMinNoOfCoeffsForAmbHOA + m_uiNoOfChannelsWithVariableType) > m_uiTotalNoOfPercCoders  )
	{ 
		return true;
	}

	//// determine set of indices of ambient HOA coefficients that may be activated  
	std::set<unsigned int> m_suiReallyActiveAmbHOACoeffsIndices;
	std::set<unsigned int> suiIndicesOfAmbHOACoeffsToBePossiblyActivated;
	
	setDiff(m_suiReallyActiveAmbHOACoeffsIndices, m_suiActiveAmbHOACoeffsIndices, m_suiAmbHOACoeffsIndicesToBeDisabled);
	setDiff(suiIndicesOfAmbHOACoeffsToBePossiblyActivated, m_suiPotentialAmbHOACoeffsIndices, m_suiReallyActiveAmbHOACoeffsIndices);


	m_suiActiveAmbHOACoeffsIndices.clear();
	m_suiAmbHOACoeffsIndicesToBeEnabled.clear();
	m_suiAmbHOACoeffsIndicesToBeDisabled.clear();

	// set iterator to first element of set of potential ambient HOA coefficients to be enabled
	std::set<unsigned int>::iterator suiItCurrHOACoeffIdx = suiIndicesOfAmbHOACoeffsToBePossiblyActivated.begin();

	for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfChannelsWithVariableType; uiChanIdx++)
	{
		// get old channel type
		if ( !m_vAIAssignmentVector[uiChanIdx].m_bIsAvailable)
		{
			m_vAIAssignmentVector[uiChanIdx].init();
		}
		unsigned int uiOldChanType				= m_vAIAssignmentVector[uiChanIdx].m_shptrChanSideInfoData->m_unChannelType;

		// get current target channel type
		if ( !m_vAIPreviousTargetAssignmentVector[uiChanIdx].m_bIsAvailable)
		{
			m_vAIPreviousTargetAssignmentVector[uiChanIdx].init();
		}
		unsigned int uiCurrentTargetChanType	= m_vAIPreviousTargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData->m_unChannelType;

		// get next target channel type
		unsigned int uiNextTargetChanType;

		if ( !p_vAITargetAssignmentVector[uiChanIdx].m_bIsAvailable)
		{
			uiNextTargetChanType = EMPTY_CHANNEL;
		}
		else
		{
			uiNextTargetChanType = p_vAITargetAssignmentVector[uiChanIdx].m_shptrChanSideInfoData->m_unChannelType;
		}


		switch( uiOldChanType ) 
		{
			case DIR_CHANNEL:
			case VEC_CHANNEL:
			case EMPTY_CHANNEL:
			{
				if ( (uiCurrentTargetChanType == EMPTY_CHANNEL) && ( uiNextTargetChanType == EMPTY_CHANNEL) )
				{
					if ( suiItCurrHOACoeffIdx != suiIndicesOfAmbHOACoeffsToBePossiblyActivated.end() )
					{
						m_vAIAssignmentVector[uiChanIdx].m_bIsAvailable = true;
						m_vAIAssignmentVector[uiChanIdx].m_uiBitRate   = m_uiBitRatePerPercCoder;
						m_vAIAssignmentVector[uiChanIdx].m_shptrChanSideInfoData = std::shared_ptr<CChannelSideInfoData>(new CAddAmbHoaInfoChannel() );

						unsigned int uiAmbCoeffIdx = *suiItCurrHOACoeffIdx;

						CAddAmbHoaInfoChannel	* pAAHIC = dynamic_cast<CAddAmbHoaInfoChannel *>(m_vAIAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());

						pAAHIC->m_unAmbCoeffIdx       = uiAmbCoeffIdx;
						pAAHIC->m_bAmbCoeffIdxChanged = true;
						pAAHIC->m_unAmbCoeffIdxTransitionState = 1;

						m_suiAmbHOACoeffsIndicesToBeEnabled.insert(uiAmbCoeffIdx);
						m_suiActiveAmbHOACoeffsIndices.insert(uiAmbCoeffIdx);

						suiItCurrHOACoeffIdx++;
					}
				}
				else
				{
					m_vAIAssignmentVector[uiChanIdx] = m_vAIPreviousTargetAssignmentVector[uiChanIdx];
				}
				break;
			}
			case ADD_HOA_CHANNEL:
			{
				if ( uiCurrentTargetChanType == EMPTY_CHANNEL)
				{
					// channel can be assigned by ambient HOA coefficient
					m_vAIAssignmentVector[uiChanIdx].m_uiBitRate   = m_uiBitRatePerPercCoder;

					CAddAmbHoaInfoChannel	* pAAHIC = dynamic_cast<CAddAmbHoaInfoChannel *>(m_vAIAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());

					unsigned int uiAmbCoeffIdx = pAAHIC->m_unAmbCoeffIdx;

					m_suiActiveAmbHOACoeffsIndices.insert(uiAmbCoeffIdx);

					if ( uiNextTargetChanType != EMPTY_CHANNEL)
					{
						// ambient HOA coefficient index has to faded out in this frame 
						m_suiAmbHOACoeffsIndicesToBeDisabled.insert(uiAmbCoeffIdx);

						pAAHIC->m_bAmbCoeffIdxChanged = true;
						pAAHIC->m_unAmbCoeffIdxTransitionState = 2;
					}
					else
					{
						pAAHIC->m_bAmbCoeffIdxChanged          = false;
						pAAHIC->m_unAmbCoeffIdxTransitionState = 0;
					}
				}
				else
				{
					m_vAIAssignmentVector[uiChanIdx] = m_vAIPreviousTargetAssignmentVector[uiChanIdx];
				}
			}

		}
	}

	for (unsigned int uiChanIdx=m_uiNoOfChannelsWithVariableType; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		m_suiActiveAmbHOACoeffsIndices.insert(uiChanIdx - m_uiNoOfChannelsWithVariableType + 1);
	}

    p_rsuiActiveAmbHOACoeffsIndices   = m_suiActiveAmbHOACoeffsIndices;
    p_rsuiAmbCoeffIndicesToBeEnabled  = m_suiAmbHOACoeffsIndicesToBeEnabled;
    p_rsuiAmbCoeffIndicesToBeDisabled = m_suiAmbHOACoeffsIndicesToBeDisabled;
    
    std::set<unsigned int> suiTmp;

    setUnion(suiTmp, m_suiAmbHOACoeffsIndicesToBeEnabled, m_suiAmbHOACoeffsIndicesToBeDisabled);
    setDiff( p_rsuiNonEnDisAbleActHOACoeffIndices, m_suiActiveAmbHOACoeffsIndices, suiTmp);

	// copy to output assignment vector

	p_vAIOutputFinalAssignmentVector.resize(m_uiNoOfChannelsWithVariableType);

	for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfChannelsWithVariableType; uiChanIdx++)
	{
		p_vAIOutputFinalAssignmentVector[uiChanIdx] = m_vAIAssignmentVector[uiChanIdx];
	}

	/////////////////////////////////////////////////////
	//// compute ambient HOA component to be transmitted
	////////////////////////////////////////////////////

	p_rvvFOutputModifiedAmbientHOAFrame.resize(m_uiNoOfHOACoeffs);

	// apply fixed spherical harmonic transform for decorrelation purposes to low order ambient HOA coefficients
	for (unsigned int uiCoeffIdx = 0; uiCoeffIdx < m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx++)
	{
		p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			for (unsigned int uiElemIdx=0; uiElemIdx < m_uiMinNoOfCoeffsForAmbHOA; uiElemIdx++)
			{
				p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] += m_vvFPInvLowOrderModeMat[uiCoeffIdx][uiElemIdx]*p_rvvFInputAmbientHOAFrame[uiElemIdx][uiSampleIdx];
			}
		}
	}

	// add additional coeffcients
	for(unsigned int uiCoeffIdx = m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++ )
	{
		p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));

		if ( m_suiActiveAmbHOACoeffsIndices.find(uiCoeffIdx+1) != m_suiActiveAmbHOACoeffsIndices.end() )
		{
			p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx].assign( p_rvvFInputAmbientHOAFrame[uiCoeffIdx].begin(), p_rvvFInputAmbientHOAFrame[uiCoeffIdx].end() );
		}

		// fade in channels to be activated
		if ( m_suiAmbHOACoeffsIndicesToBeEnabled.find(uiCoeffIdx+1) != m_suiAmbHOACoeffsIndicesToBeEnabled.end() )
		{
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] *= m_vFSmoothWinFunc[uiSampleIdx];
			}
		}

		// fade out channels to be deactivated 
		if ( m_suiAmbHOACoeffsIndicesToBeDisabled.find(uiCoeffIdx+1) != m_suiAmbHOACoeffsIndicesToBeDisabled.end() )
		{
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFOutputModifiedAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] *= m_vFSmoothWinFunc[m_uiFrameSize + uiSampleIdx];
			}
		}

	}

	// compute predicted ambient HOA frame
	p_rvvFOutputModifiedPredictedNextAmbientHOAFrame.resize(m_uiNoOfHOACoeffs);

	// apply fixed spherical harmonic transform for decorrelation purposes to low order ambient HOA coefficients
	for (unsigned int uiCoeffIdx = 0; uiCoeffIdx < m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx++)
	{
		p_rvvFOutputModifiedPredictedNextAmbientHOAFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			for (unsigned int uiElemIdx=0; uiElemIdx < m_uiMinNoOfCoeffsForAmbHOA; uiElemIdx++)
			{
				p_rvvFOutputModifiedPredictedNextAmbientHOAFrame[uiCoeffIdx][uiSampleIdx] += m_vvFPInvLowOrderModeMat[uiCoeffIdx][uiElemIdx]*p_rvvFInputPredictedNextAmbientHOAFrame[uiElemIdx][uiSampleIdx];
			}
		}
	}

	// copy from input for higher coefficients
	for(unsigned int uiCoeffIdx = m_uiMinNoOfCoeffsForAmbHOA; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++ )
	{
		p_rvvFOutputModifiedPredictedNextAmbientHOAFrame[uiCoeffIdx].assign( p_rvvFInputPredictedNextAmbientHOAFrame[uiCoeffIdx].begin(), p_rvvFInputPredictedNextAmbientHOAFrame[uiCoeffIdx].end() );

	}

	// buffer target assignment vector
	m_vAIPreviousTargetAssignmentVector = p_vAITargetAssignmentVector;

	return false;
}

// static function for computing smoothing window function
void AmbientComponentModification::computeSmoothWinFunc(std::vector<FLOAT> & p_rvFOutputSmoothWinFunc, unsigned int p_uiFrameSize)
{
	p_rvFOutputSmoothWinFunc.resize(2*p_uiFrameSize);

	// compute values of Hann window
	for ( unsigned int uiSampleIdx = 0; uiSampleIdx < 2*p_uiFrameSize ; uiSampleIdx++)
	{
		p_rvFOutputSmoothWinFunc[uiSampleIdx] = static_cast<FLOAT>( 0.5 * ( 1 - cos( 2.0*OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(2*p_uiFrameSize) ))  ) ); 	
	}
}
