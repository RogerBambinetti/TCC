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
 $Id: DirectionAndVectorEstimation.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/


#include "DirectionAndVectorEstimation.h"

// constructor
DirectionAndVectorEstimation::DirectionAndVectorEstimation()
{
		m_bIsInit = false;
}


// init function
bool DirectionAndVectorEstimation::init(  unsigned int p_uiFrameSize,
										  unsigned int p_uiHOAOrder,
										  unsigned int p_uiMinNoOfCoeffsForAmbHOA,
										  unsigned int p_uiNoBitsForVecElemQuant,
										  unsigned int p_uiCodedVVecLength)
{
	m_uiFrameSize				= p_uiFrameSize;
	m_uiHOAOrder				= p_uiHOAOrder;
	m_uiNoOfHOACoeffs			= (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
	m_uiMinNoOfCoeffsForAmbHOA	= p_uiMinNoOfCoeffsForAmbHOA;
	m_uiNoBitsForVecElemQuant   = p_uiNoBitsForVecElemQuant;
	if (p_uiCodedVVecLength)
	{
		m_uiNoOfVecElems = m_uiNoOfHOACoeffs - m_uiMinNoOfCoeffsForAmbHOA;
	}
	else {
		m_uiNoOfVecElems = m_uiNoOfHOACoeffs;
	}


	m_uiFrameCounter = 0;

	m_bDir1Active = false;
	m_bDir2Active = false;

	m_bIsInit = true;

	return false;
}




bool DirectionAndVectorEstimation::process(const std::vector<std::vector<FLOAT>> & p_rvvFInputHOAFrame, 
											     std::map<unsigned int, unsigned int> & p_rmuiOutputDirSigAndGridDirIndices,
											     std::map<unsigned int, std::vector<FLOAT> > & p_rmuivFVectors)
{
	if(!m_bIsInit)
	{
		return true;
	}

	m_uiFrameCounter++;

	// perform decision
	DECISION eDecision = EMPTY;

	unsigned int uiRemainder = (m_uiFrameCounter % 20);

	if ( uiRemainder == 10)
	{
		eDecision = EMPTY; 
	}
	else
	{
		if ( uiRemainder < 9 ) 
		{
			eDecision = DIR;
		}
		else
		{
			if (uiRemainder > 11) 
			{
				eDecision = VEC;
			}
		}
	}

	// clear sets
	p_rmuiOutputDirSigAndGridDirIndices.clear();
	p_rmuivFVectors.clear();
   // Disable for low bitrates 
	if (eDecision == DIR )
	{
		if ( (m_uiFrameCounter % 10) == 1)
		{
			m_bDir1Active = !m_bDir1Active;	
		}

		if ( (m_uiFrameCounter % 7) == 1)
		{
			m_bDir2Active = !m_bDir2Active;	
		}

		if (m_bDir1Active)
		{
			p_rmuiOutputDirSigAndGridDirIndices.insert( std::pair<unsigned int, unsigned int>( 1, static_cast<unsigned int>((m_uiFrameCounter % 3) + 1 ) ) );
		}

		if (m_bDir2Active)
		{
			p_rmuiOutputDirSigAndGridDirIndices.insert( std::pair<unsigned int, unsigned int>( 3, static_cast<unsigned int>(( m_uiFrameCounter % 187) + 1 ) ) );
		}
	}

	if (eDecision == VEC )
	{
		std::vector<FLOAT> vVec1(m_uiNoOfVecElems, static_cast<FLOAT>(0.0));
		std::vector<FLOAT> vVec2(m_uiNoOfVecElems, static_cast<FLOAT>(0.0));

		FLOAT FNormvec1 = static_cast<FLOAT>(0.0);
		FLOAT FNormvec2 = static_cast<FLOAT>(0.0);

		// init vector elements arbitrary
		for (unsigned int uiElemIdx=0; uiElemIdx < m_uiNoOfVecElems; uiElemIdx++)
		{
			vVec1[uiElemIdx] = static_cast<FLOAT>(uiElemIdx);
			FNormvec1 += vVec1[uiElemIdx]*vVec1[uiElemIdx];

			vVec2[uiElemIdx] = static_cast<FLOAT>(m_uiNoOfVecElems - uiElemIdx);
			FNormvec2 += vVec2[uiElemIdx]*vVec2[uiElemIdx];
		}

		FNormvec1 = sqrt(FNormvec1);
		FNormvec2 = sqrt(FNormvec2);

		unsigned int uiTmpForQuant;

		// normalize to have unit Euclidean norm
		for (unsigned int uiElemIdx=0; uiElemIdx < m_uiNoOfVecElems; uiElemIdx++)
		{
			vVec1[uiElemIdx] /= FNormvec1;
			vVec2[uiElemIdx] /= FNormvec2;

			//quantize and dequantize vector elements
			quantizeUniform( vVec1[uiElemIdx],  m_uiNoBitsForVecElemQuant, uiTmpForQuant );
			dequantizeUniform( uiTmpForQuant,   m_uiNoBitsForVecElemQuant, vVec1[uiElemIdx] );

			//AK proposal: normalize to have Euclidean norm of (N+1) to be consistent with mode vectors
			vVec1[uiElemIdx] *= static_cast<FLOAT>(m_uiHOAOrder+1);

			//quantize and dequantize vector elements
			quantizeUniform( vVec2[uiElemIdx],  m_uiNoBitsForVecElemQuant, uiTmpForQuant );
			dequantizeUniform( uiTmpForQuant,   m_uiNoBitsForVecElemQuant, vVec2[uiElemIdx] );

			//AK proposal: normalize to have Euclidean norm of (N+1) to be consistent with mode vectors
			vVec2[uiElemIdx] *= static_cast<FLOAT>(m_uiHOAOrder+1);
		}

		p_rmuivFVectors.insert( std::pair<unsigned int, std::vector<FLOAT> >( 2, vVec1) );
		p_rmuivFVectors.insert( std::pair<unsigned int, std::vector<FLOAT> >( 4, vVec2) );

	}

	return false;
}

