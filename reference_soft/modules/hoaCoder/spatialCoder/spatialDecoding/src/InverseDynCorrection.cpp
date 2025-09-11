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
 $Id: InverseDynCorrection.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#include "InverseDynCorrection.h"	


// constructor
InverseDynCorrection::InverseDynCorrection()
{
	m_bIsInit = false;
}


//init function
bool InverseDynCorrection::init(unsigned int p_uiFrameSize)
{
	// reset values
	m_uiFrameSize = p_uiFrameSize;
	m_FLastGain   = static_cast<FLOAT>(1.0);

	// init window function
	
	if ( computeWinFunc(m_vFWinFunc, p_uiFrameSize) )
	{
		return true;
	}

	m_bIsInit = true;

	return false;
};

// process function
bool InverseDynCorrection::process(const std::vector<FLOAT>& p_vFInputSampleBuffer, std::vector<FLOAT>& p_vFOutputSampleBuffer, bool p_bException, int p_iExponent)
{
			// check if InverseDynCorrection object has already been initialized
			if (!m_bIsInit)
			{
				std::cout << "InverseDynCorrection: Processing cannot be performed without previous initialization!" << std::endl;
				return true;
			}

			// check if the buffers have the correct size
			if ( (p_vFInputSampleBuffer.size() != m_uiFrameSize) || (p_vFOutputSampleBuffer.size() != m_uiFrameSize) )
			{
				std::cout << "InverseDynCorrection: Buffers do not have the correct size!" << std::endl;
				return true;
			}

			// start actual processing
			for ( unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
					p_vFOutputSampleBuffer[uiSampleIdx] = p_vFInputSampleBuffer[uiSampleIdx]/m_FLastGain;
			}

			int iNegExponent = (-1)*p_iExponent;

			if ( !p_bException )
			{
				if ( p_iExponent != 0)
				{
					for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
					{
						p_vFOutputSampleBuffer[uiSampleIdx] *= static_cast<FLOAT>(pow(m_vFWinFunc[uiSampleIdx], iNegExponent));
					}
					m_FLastGain *= static_cast<FLOAT>(pow( m_vFWinFunc[m_uiFrameSize-1], p_iExponent ));
				}
			}
			else
			{
				for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
				{
					p_vFOutputSampleBuffer[uiSampleIdx] *= static_cast<FLOAT>(pow( 2.0,  p_iExponent ));
				}
				m_FLastGain *= static_cast<FLOAT>(pow( 2.0,  iNegExponent));
			}

			return false;
}

bool InverseDynCorrection::setLastGain(int iExponentToBase2)
{
    m_FLastGain = static_cast<FLOAT>(pow( 2.0,  iExponentToBase2));

    return false;
}



bool InverseDynCorrection::computeWinFunc(std::vector<FLOAT> &p_rvFOutWinFunc, unsigned int p_uiFrameSize)
{
			// change size of output vector if neccesary
			if ( p_rvFOutWinFunc.size() != p_uiFrameSize)
			{
				p_rvFOutWinFunc.resize( p_uiFrameSize );
			}
			
			// compute values of window function
			for ( unsigned int uiSampleIdx = 0; uiSampleIdx < p_uiFrameSize ; uiSampleIdx++)
			{
				p_rvFOutWinFunc[uiSampleIdx] = static_cast<FLOAT>( 0.25*cos( M_PI / ( static_cast<double>(p_uiFrameSize) - 1.0)* static_cast<double>(uiSampleIdx)) + 0.75); 	
			}

			return false;
}
