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
 $Id: DynCorrection.cpp 179 2015-05-07 13:08:57Z technicolor-kf $
*/

#include "DynCorrection.h"
#include <limits>

DynCorrection::DynCorrection()
{
	m_bIsInit = false;
}


bool DynCorrection::init(unsigned int p_uiFrameSize, unsigned int p_uiHOAOrder, unsigned int p_uiMaxAmplifyExponent)
{
	 m_uiFrameSize			= p_uiFrameSize;
	 m_uiMaxAmplifyExponent = p_uiMaxAmplifyExponent;

	 computeWinFunc( m_vFWinFunc, m_uiFrameSize);

     m_iLastExpToBase2      = 0;
     m_iPrevExpToBase2      = 0;

	 m_FLastGain			= static_cast<FLOAT>(1.0);
	 m_FLastMaxAbsValue		= static_cast<FLOAT>(0.0);
	 m_FSmallestDelta		= static_cast<FLOAT>( pow( 2.0, -31) );
	 m_FMaxValueAbsorption	= static_cast<FLOAT>( 0.1 );

	 //init allowed exponents
	 unsigned int uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);

	 m_uiMaxAttenuateExponent = m_uiMaxAmplifyExponent + static_cast<unsigned int>( ceil( log( static_cast<double>(uiNoOfHOACoeffs))/log(2.0) ));

//	 unsigned int uiNoOfAllowedExponents = m_uiMaxAmplifyExponent + m_uiMaxAttenuateExponent + 1;

	 m_siAllowedExponents.clear();
	 m_siAllowedExponents.insert(0);

	 for (unsigned int uiExpIdx=1; uiExpIdx <= m_uiMaxAmplifyExponent; uiExpIdx++)
	 {
		 m_siAllowedExponents.insert( (-1)*static_cast<int>( uiExpIdx) );
	 }
	 
	 for (unsigned int uiExpIdx=1; uiExpIdx <= m_uiMaxAttenuateExponent; uiExpIdx++)
	 {
		 m_siAllowedExponents.insert( static_cast<int>(uiExpIdx) );
	 }

	 m_bIsInit = true;

	 return false;
}


bool DynCorrection::process(const std::vector<FLOAT>& p_vFInputSampleBuffer, 
							const std::vector<FLOAT>& p_vFInputPredictedNextSampleBuffer,
								  std::vector<FLOAT>& p_vFOutputSampleBuffer, 
								  bool &p_rbException, 
								  int  &p_riExponent)
{

	// compute maximum absolute value of input sample buffer and predicted input sample buffer
		FLOAT FMaxAbsValInput;
		FLOAT FMaxAbsValPredictedNextInput;

		unsigned int uiMaxAbsIndexInput;
		unsigned int uiMaxAbsIndexPredictedNextInput;

		if ( computeMaxAbsVal(p_vFInputSampleBuffer, FMaxAbsValInput, uiMaxAbsIndexInput) )
		{
			return true;
		}

		if ( computeMaxAbsVal(p_vFInputPredictedNextSampleBuffer, FMaxAbsValPredictedNextInput, uiMaxAbsIndexPredictedNextInput) )
		{
			return true;
		}

		FLOAT FMaxAbsValTotalInput;

		unsigned int uiMaxAbsIndexTotalInput;
	
		if (FMaxAbsValPredictedNextInput > FMaxAbsValInput )
		{
			FMaxAbsValTotalInput    = FMaxAbsValPredictedNextInput;
			uiMaxAbsIndexTotalInput = m_uiFrameSize-1;
		}
		else
		{
			FMaxAbsValTotalInput	= FMaxAbsValInput;
			uiMaxAbsIndexTotalInput = uiMaxAbsIndexInput;
		}

	// apply gain of last block to input
	p_vFOutputSampleBuffer.resize(m_uiFrameSize);

	for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
	{
		p_vFOutputSampleBuffer[uiSampleIdx] = m_FLastGain*p_vFInputSampleBuffer[uiSampleIdx];
	}

	// test if scaled input buffer creates clipping
	unsigned int uiMaxRangeFirstIndex  = m_uiFrameSize-1;
	unsigned int uiMaxRangeSecondIndex = uiMaxAbsIndexTotalInput;
	bool bIsOutOfRange = false;

	for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
	{
		if (  (  p_vFOutputSampleBuffer[uiSampleIdx] > ( (static_cast<FLOAT>(1.0) - m_FSmallestDelta) ) )
			|| ( p_vFOutputSampleBuffer[uiSampleIdx] < static_cast<FLOAT>(-1.0) ) )
		{
			uiMaxRangeFirstIndex = uiSampleIdx;
			bIsOutOfRange        = true;
			break;
		}
	}

	// test if scaled predicted input buffer creates clipping
	for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
	{
		if (  (  m_FLastGain*p_vFInputPredictedNextSampleBuffer[uiSampleIdx] > ( (static_cast<FLOAT>(1.0) - m_FSmallestDelta) ) )
			|| ( m_FLastGain*p_vFInputPredictedNextSampleBuffer[uiSampleIdx] < static_cast<FLOAT>(-1.0) ) )
		{
			bIsOutOfRange = true;
		}
	}

	if ( !bIsOutOfRange)
	{
		// no new clipping
		p_rbException = false; 

		//if ( m_FLastGain == static_cast<FLOAT>(pow( 2.0, static_cast<int>(m_uiMaxAmplifyExponent)) ) )
		if ( m_iLastExpToBase2 == static_cast<int>(m_uiMaxAmplifyExponent) || FMaxAbsValTotalInput < std::numeric_limits<FLOAT>::min() )
        {
			p_riExponent  = 0;
            m_iPrevExpToBase2  = m_iLastExpToBase2;
			return false;
		}
		else
		{
			// smooth maximum value with previous maximum value
			FLOAT FSmoothedMaxAbsValTotalInput = m_FMaxValueAbsorption*FMaxAbsValTotalInput + (static_cast<FLOAT>(1.0) - m_FMaxValueAbsorption)*m_FLastMaxAbsValue;

			//  replace last maximum absolute value by smoothed version
			m_FLastMaxAbsValue = FSmoothedMaxAbsValTotalInput;

			 // check head room
			 // assume multiplication by the greatest const. gain
			FLOAT FMaxAbsValAfterGainIncrease = std::max(FSmoothedMaxAbsValTotalInput,FMaxAbsValTotalInput)*m_FLastGain*static_cast<FLOAT>(2.0);

			// check if maximum value after the amplification is still within the required range
			if ( FMaxAbsValAfterGainIncrease < (static_cast<FLOAT>(1.0) - m_FSmallestDelta) )
			{
				// gain can be applied
				p_riExponent = -1;
			}
			else
			{
				// gain would result in out of range values
				// keep last gain
				p_riExponent = 0;
			}
		}
	}
	else
	{
		// update last maximum absolute value
		m_FLastMaxAbsValue = FMaxAbsValTotalInput;
	}
	
	// compute (possibly smoothed) maximum absolute value multiplied by last gain
	FLOAT FCorrectedMaxAbsValue = m_FLastMaxAbsValue*m_FLastGain;

	// do out of range processing
	if ( bIsOutOfRange )
	{
		if ( uiMaxRangeSecondIndex == 0 )
		{
			p_rbException = true;
		}
		else
		{
			// estimate the required exponent to convert corrected maximum value to one
			FLOAT FReqExponent = - log(FCorrectedMaxAbsValue + m_FSmallestDelta)/ log(m_vFWinFunc[uiMaxRangeSecondIndex]);
		
			// set max exponent by rounding to next integer
			p_riExponent = static_cast<int>( ceil(FReqExponent) );

			// check if exponent sufficiently compensates the dynamic range for all
			//  samples in the range from the first out of range sample to the index of max. value 

			bool bSuccess = false;

			while (!bSuccess)
			{
				bSuccess = true;

				// compute value of compensated signal in relevant range
				for (unsigned int uiSampleIdx=uiMaxRangeFirstIndex; uiSampleIdx <= uiMaxRangeSecondIndex; uiSampleIdx++)
				{
					FLOAT FCompSigVal =  p_vFOutputSampleBuffer[uiSampleIdx] * static_cast<FLOAT>(pow( static_cast<double>(m_vFWinFunc[uiSampleIdx]), p_riExponent ) );

					if ( FCompSigVal > (static_cast<FLOAT>(1.0) - m_FSmallestDelta) || FCompSigVal < static_cast<FLOAT>(-1.0) )
					{
						bSuccess = false;
						break;
					}
				}

				if (!bSuccess)
				{
					p_riExponent++;
				}

				if (p_riExponent > static_cast<int>(m_uiMaxAttenuateExponent))
				{
					p_rbException = true;
					break;
				}
			}

			if (bSuccess)
			{
				p_rbException = false;
			}
		}
	}

	if (!p_rbException)
	{
		// apply compensation function to input samples
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			p_vFOutputSampleBuffer[uiSampleIdx] *= static_cast<FLOAT>(pow( static_cast<double>(m_vFWinFunc[uiSampleIdx]), p_riExponent )) ;
		}

		// set new gain, which is the product of the last and the new gain (both were applied to the input samples)
		m_FLastGain *= pow( static_cast<double>(m_vFWinFunc[m_uiFrameSize-1]), p_riExponent );

	}
	else
	{
		// compute required floating point exponent
		FLOAT FReqExp = static_cast<FLOAT>(-log( static_cast<double>(FCorrectedMaxAbsValue + m_FSmallestDelta) )/log(2.0)); 

		// round to smallest integer and change sign
		p_riExponent = static_cast<int>(-floor(FReqExp));

		// apply gain
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
			p_vFOutputSampleBuffer[uiSampleIdx] *= pow( static_cast<double>(2.0), -p_riExponent );
		}
			
		// set new gain, which is the product of the last and the new gain (both were applied to the input samples)
		m_FLastGain *= pow( static_cast<double>(2.0), -p_riExponent );
	}

    m_iPrevExpToBase2  = m_iLastExpToBase2;
    m_iLastExpToBase2 -= p_riExponent;

	return false;
}

int DynCorrection::getLastExp()
{
    return m_iPrevExpToBase2;
}


bool DynCorrection::computeWinFunc(std::vector<FLOAT> &p_rvFOutWinFunc, unsigned int p_uiFrameSize)
{
			// change size of output vector if neccesary
			if ( p_rvFOutWinFunc.size() != p_uiFrameSize)
			{
				p_rvFOutWinFunc.resize( p_uiFrameSize );
			}
			
			// compute values of window function
			for ( unsigned int uiSampleIdx = 0; uiSampleIdx < p_uiFrameSize ; uiSampleIdx++)
			{
				p_rvFOutWinFunc[uiSampleIdx] = static_cast<FLOAT>( 0.25*cos( OWN_PI / ( static_cast<double>(p_uiFrameSize) - 1.0)* static_cast<double>(uiSampleIdx)) + 0.75); 	
			}

			return false;
}

//bool DynCorrection::computeMaxAbsVal(const std::vector<FLOAT> &p_rvFInVec, FLOAT & p_rFMaxAbsVal, unsigned int & p_ruiMaxAbsIndex)
//{
//	if ( p_rvFInVec.size() == 0)
//	{
//		return true;
//	}
//
//	p_ruiMaxAbsIndex = 0;
//	p_rFMaxAbsVal = std::abs( p_rvFInVec[p_ruiMaxAbsIndex]);
//	
//
//	for (unsigned int uiElemIdx=1; uiElemIdx < p_rvFInVec.size(); uiElemIdx++ )
//	{
//		FLOAT FCurrAbsVal = std::abs( p_rvFInVec[uiElemIdx]);
//
//		if (FCurrAbsVal > p_rFMaxAbsVal)
//		{
//			p_ruiMaxAbsIndex = uiElemIdx;
//			p_rFMaxAbsVal	 = FCurrAbsVal;
//		}
//	}
//
//	return false;
//}
