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
 $Rev: 157 $
 $Author: technicolor-kf $
 $Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
 $Id: ChannelAssignment.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#include "ChannelAssignment.h"


ChannelAssignment::ChannelAssignment()
{
	m_bIsInit = false;
}

bool ChannelAssignment::init(	unsigned int p_uiFrameSize, 
								unsigned int p_uiHOAOrder, 
								unsigned int p_uiTotalNoOfPercCoders,
								unsigned int p_uiNoOfChannelsWithVariableType)
{
	m_uiFrameSize = p_uiFrameSize;
	m_uiHOAOrder  = p_uiHOAOrder;
	m_uiTotalNoOfPercCoders = p_uiTotalNoOfPercCoders;
	m_uiNoOfChannelsWithVariableType = p_uiNoOfChannelsWithVariableType;

	m_bIsInit = true;

	return false;
}

bool ChannelAssignment::process(const std::vector<AssignmentInfo> &p_vAIInputCurrentAssignmentVector,
								const std::vector<AssignmentInfo> &p_vAIInputPredictedAssignmentVector,
								const std::vector<std::vector<FLOAT>> &p_rvvFInputSmoothedPSSigsFrame,
								const std::vector<std::vector<FLOAT>> &p_rvvFInputSmoothedPSSigsNextFrame,
								const std::vector<std::vector<FLOAT>> &p_rvvFInputModifiedAmbientHOAFrame,
								const std::vector<std::vector<FLOAT>> &p_rvvFInputModifiedPredictedNextAmbientHOAFrame,
									  std::vector<std::vector<FLOAT>> &p_rvvFOutputAllSigsToBeCodedFrame,
									  std::vector<std::vector<FLOAT>> &p_rvvFOutputPredictedAllSigsToBeCodedNextFrame)
{
	if ( p_vAIInputCurrentAssignmentVector.size() != m_uiNoOfChannelsWithVariableType)
	{
		return true;
	}

	if ( p_vAIInputPredictedAssignmentVector.size() != m_uiNoOfChannelsWithVariableType)
	{
		return true;
	}

	p_rvvFOutputAllSigsToBeCodedFrame.resize(m_uiTotalNoOfPercCoders);

	for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNoOfChannelsWithVariableType; uiChanIdx++)
	{
		// get current channel type
		unsigned int uiCurrentChannelType;

		if ( p_vAIInputCurrentAssignmentVector[uiChanIdx].m_bIsAvailable)
		{
			uiCurrentChannelType = p_vAIInputCurrentAssignmentVector[uiChanIdx].m_shptrChanSideInfoData->m_unChannelType;
		}
		else
		{
			uiCurrentChannelType = EMPTY_CHANNEL;	
		}

		switch ( uiCurrentChannelType ) 
		{
			case DIR_CHANNEL:
			case VEC_CHANNEL:

				p_rvvFOutputAllSigsToBeCodedFrame[uiChanIdx].assign( p_rvvFInputSmoothedPSSigsFrame[uiChanIdx].begin(), p_rvvFInputSmoothedPSSigsFrame[uiChanIdx].end() );

				break;

			case ADD_HOA_CHANNEL:
			{
				CAddAmbHoaInfoChannel	* pAAHIC = dynamic_cast<CAddAmbHoaInfoChannel *>(p_vAIInputCurrentAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());

				unsigned int uiAmbCoeffIdx = pAAHIC->m_unAmbCoeffIdx;

				p_rvvFOutputAllSigsToBeCodedFrame[uiChanIdx].assign( p_rvvFInputModifiedAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].begin(), p_rvvFInputModifiedAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].end() );

				break;
			}	
			case EMPTY_CHANNEL:
				p_rvvFOutputAllSigsToBeCodedFrame[uiChanIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0));
		}
		
		// get predicted channel type
		unsigned int uiPredictedChannelType;

		if ( p_vAIInputPredictedAssignmentVector[uiChanIdx].m_bIsAvailable)
		{
			uiPredictedChannelType = p_vAIInputPredictedAssignmentVector[uiChanIdx].m_shptrChanSideInfoData->m_unChannelType;
		}
		else
		{
			uiPredictedChannelType = EMPTY_CHANNEL;
		}

		switch ( uiPredictedChannelType ) 
		{
			case DIR_CHANNEL:
			case VEC_CHANNEL:

				p_rvvFOutputPredictedAllSigsToBeCodedNextFrame[uiChanIdx].assign( p_rvvFInputSmoothedPSSigsNextFrame[uiChanIdx].begin(), p_rvvFInputSmoothedPSSigsNextFrame[uiChanIdx].end() );
				
				break;

			case EMPTY_CHANNEL:
			{
				if ( uiCurrentChannelType == ADD_HOA_CHANNEL )
				{
					CAddAmbHoaInfoChannel	* pAAHIC = dynamic_cast<CAddAmbHoaInfoChannel *>(p_vAIInputCurrentAssignmentVector[uiChanIdx].m_shptrChanSideInfoData.get());

					unsigned int uiAmbCoeffIdx = pAAHIC->m_unAmbCoeffIdx;

					p_rvvFOutputPredictedAllSigsToBeCodedNextFrame[uiChanIdx].assign( p_rvvFInputModifiedPredictedNextAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].begin(), p_rvvFInputModifiedPredictedNextAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].end());
				}
                else
                {
                    p_rvvFOutputPredictedAllSigsToBeCodedNextFrame[uiChanIdx].assign(m_uiFrameSize, static_cast<FLOAT>(0.0));
                }
			}
		}
	}

	for (unsigned int uiChanIdx=m_uiNoOfChannelsWithVariableType; uiChanIdx < m_uiTotalNoOfPercCoders; uiChanIdx++)
	{
		unsigned int uiAmbCoeffIdx = uiChanIdx - m_uiNoOfChannelsWithVariableType + 1;

		p_rvvFOutputAllSigsToBeCodedFrame[uiChanIdx].assign( p_rvvFInputModifiedAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].begin(), p_rvvFInputModifiedAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].end() );
		p_rvvFOutputPredictedAllSigsToBeCodedNextFrame[uiChanIdx].assign( p_rvvFInputModifiedPredictedNextAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].begin(), p_rvvFInputModifiedPredictedNextAmbientHOAFrame[ uiAmbCoeffIdx - 1 ].end());

	}


	return false;
}
