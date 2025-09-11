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
$Id: CommonFunctions.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#ifndef __COMMONFUNCTIONS__
#define __COMMONFUNCTIONS__

#include "integerFunctions.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <vector>
#include <iterator>
#include <complex>
#include "DataTypes.h"
#include "TabulatedValues.h"
//#include "uhd_fft.h" 



inline static bool quantizeUniform(FLOAT p_rFInputVal, 
								   unsigned int uiNoOfBits,
								   unsigned int & p_ruiOutputQuantizedVal  )
{
	if (uiNoOfBits == 0)
	{
		return true;
	}
	else
	{
		p_ruiOutputQuantizedVal = 
			static_cast<unsigned int>( std::min( static_cast<unsigned int>(std::floor( 0.5 + ( static_cast<double>(p_rFInputVal) + 1.0)*static_cast<double>(getPow2(uiNoOfBits-1)))), 
			static_cast<unsigned int>(getPow2(uiNoOfBits) - 1)   ) )  ; 
	}

	return false;

}

inline static bool dequantizeUniform(  unsigned int p_ruiInputQuantizedVal, 
									 unsigned int uiNoOfBits,
									 FLOAT & p_rFOutputVal  )
{
	if (uiNoOfBits == 0)
	{
		return true;
	}
	else
	{
		p_rFOutputVal = static_cast<FLOAT>(static_cast<double>(p_ruiInputQuantizedVal)/static_cast<double>(getPow2(uiNoOfBits-1)) - 1.0);
	}

	return false;		
}

// static function for computing smoothing window function
inline static bool computeFadeWinForDirBasedSynth(std::vector<FLOAT> & p_rvFOutputFadeWinForDirBasedSynth, unsigned int p_uiFrameSize)
{
	if (p_uiFrameSize == 0)
	{
		return true;
	}

	p_rvFOutputFadeWinForDirBasedSynth.resize(2*p_uiFrameSize);

	// compute values of Hann window
	for ( unsigned int uiSampleIdx = 0; uiSampleIdx < 2*p_uiFrameSize ; uiSampleIdx++)
	{
		p_rvFOutputFadeWinForDirBasedSynth[uiSampleIdx] = static_cast<FLOAT>( 0.5 * ( 1 - cos( 2.0*OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(2*p_uiFrameSize) ))  ) ); 	
	}

	return false;
}

inline static bool computeFadeWinForVecBasedSynth(std::vector<FLOAT> & p_rvFOutputFadeWinForVecBasedSynth, unsigned int p_uiFrameSize, unsigned int p_uiInterpSamples, unsigned int p_uiInterpMethod)
{
	if (p_uiInterpSamples > p_uiFrameSize)
	{
		return true;
	}

	p_rvFOutputFadeWinForVecBasedSynth.assign( 2*p_uiFrameSize, static_cast<FLOAT>(0.0) );

	if (0==p_uiInterpSamples)
	{
		for ( unsigned int uiSampleIdx = 0; uiSampleIdx < p_uiFrameSize ; uiSampleIdx++)
		{
			p_rvFOutputFadeWinForVecBasedSynth[uiSampleIdx] = static_cast<FLOAT>(1.0);
		}
	}
	else
	{

	// flat envelope (middle portion)
	for ( unsigned int uiSampleIdx = p_uiInterpSamples; uiSampleIdx < p_uiFrameSize ; uiSampleIdx++)
	{
		p_rvFOutputFadeWinForVecBasedSynth[uiSampleIdx] = static_cast<FLOAT>(1.0);
	}

	// fade in and out
	if (0 == p_uiInterpMethod) // linear curve
	{
		for ( unsigned int uiSampleIdx = 0; uiSampleIdx < p_uiInterpSamples ; uiSampleIdx++)
		{
			p_rvFOutputFadeWinForVecBasedSynth[uiSampleIdx] = 
				static_cast<FLOAT>(uiSampleIdx) / static_cast<FLOAT>(p_uiInterpSamples-1);
			p_rvFOutputFadeWinForVecBasedSynth[p_uiFrameSize+uiSampleIdx] = 
				static_cast<FLOAT>(1.0) - p_rvFOutputFadeWinForVecBasedSynth[uiSampleIdx];
		}
	}
	else if (1 == p_uiInterpMethod) // cosine curve 
	{
		for ( unsigned int uiSampleIdx = 0; uiSampleIdx < p_uiInterpSamples ; uiSampleIdx++)
		{
			p_rvFOutputFadeWinForVecBasedSynth[uiSampleIdx] = 
					//static_cast<FLOAT>( 0.5 * ( 1 - cos(OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(p_uiInterpSamples-1) ))  ) ); //pre-RM5	
					  static_cast<FLOAT>( 0.5 * ( 1 - cos(OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(p_uiInterpSamples  ) ))  ) );	
			p_rvFOutputFadeWinForVecBasedSynth[p_uiFrameSize+uiSampleIdx] = 
					//static_cast<FLOAT>( 0.5 * ( 1 + cos(OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(p_uiInterpSamples-1) ))  ) ); //pre-RM5
					  static_cast<FLOAT>( 0.5 * ( 1 + cos(OWN_PI * ( static_cast<double>(uiSampleIdx) ) /( static_cast<double>(p_uiInterpSamples  ) ))  ) );
			}
		}
	}

	return false;
}

inline static bool computeMaxAbsVal(const std::vector<FLOAT> &p_rvFInVec, FLOAT & p_rFMaxAbsVal, unsigned int & p_ruiMaxAbsIndex)
{
	if ( p_rvFInVec.size() == 0)
	{
		return true;
	}

	p_ruiMaxAbsIndex = 0;
	p_rFMaxAbsVal = std::abs( p_rvFInVec[p_ruiMaxAbsIndex]);
	

	for (unsigned int uiElemIdx=1; uiElemIdx < p_rvFInVec.size(); uiElemIdx++ )
	{
		FLOAT FCurrAbsVal = std::abs( p_rvFInVec[uiElemIdx]);

		if (FCurrAbsVal > p_rFMaxAbsVal)
		{
			p_ruiMaxAbsIndex = uiElemIdx;
			p_rFMaxAbsVal	 = FCurrAbsVal;
		}
	}

	return false;
}

inline static bool computePseudoInverseOfVec(const std::vector<FLOAT> & p_rvFInputVec, std::vector<FLOAT> & p_rvFOutputPseudoInvVec)
{
	// set size of output pseudo inverse vector
	p_rvFOutputPseudoInvVec.resize(p_rvFInputVec.size());

	FLOAT FTmPower = static_cast<FLOAT>(0.0);

	for (unsigned int uiElemIdx=0; uiElemIdx < p_rvFInputVec.size(); uiElemIdx++)
	{
		FTmPower += p_rvFInputVec[uiElemIdx]*p_rvFInputVec[uiElemIdx];
	}

	if ( FTmPower == static_cast<FLOAT>(0.0) )
	{
		return true;
	}
	else
	{
		for (unsigned int uiElemIdx=0; uiElemIdx < p_rvFInputVec.size(); uiElemIdx++)
		{
			p_rvFOutputPseudoInvVec[uiElemIdx] = p_rvFInputVec[uiElemIdx]/FTmPower;
		}

		return false;
	}
}


inline static void setUnion(std::set<unsigned int> &p_rvuiOutputUnionSet, const std::set<unsigned int> &p_suiFirstSet, const std::set<unsigned int> &p_suiSecondSet )
{
		// compute set union
		p_rvuiOutputUnionSet.clear();
		std::set_union( p_suiFirstSet.begin(), p_suiFirstSet.end(), p_suiSecondSet.begin(), p_suiSecondSet.end(), std::inserter(p_rvuiOutputUnionSet, p_rvuiOutputUnionSet.begin()) );
}

inline static void setDiff(std::set<unsigned int> &p_rvuiOutputDiffSet, const std::set<unsigned int> &p_rsuiInputMainSet, const std::set<unsigned int> &p_rsuiInputSubSet )
{
		// compute set difference
		p_rvuiOutputDiffSet.clear();
		std::set_difference(p_rsuiInputMainSet.begin(), p_rsuiInputMainSet.end(), p_rsuiInputSubSet.begin(), p_rsuiInputSubSet.end(), std::inserter(p_rvuiOutputDiffSet, p_rvuiOutputDiffSet.begin() ) );
}

inline static void setIntersect(std::set<unsigned int> &p_rvuiOutputIntersectSet, const std::set<unsigned int> &p_suiFirstSet, const std::set<unsigned int> &p_suiSecondSet )
{
		// compute set difference
		p_rvuiOutputIntersectSet.clear();
		std::set_intersection(p_suiFirstSet.begin(), p_suiFirstSet.end(), p_suiSecondSet.begin(), p_suiSecondSet.end(), std::inserter(p_rvuiOutputIntersectSet, p_rvuiOutputIntersectSet.begin() ) );
}


inline static void setDiff(std::set<unsigned int> &p_rvuiOutputDiffSet, const std::vector<unsigned int> &p_rvuiInputMainVec, unsigned int p_uiSubValue )
{
		// create from input vector a set of unique elements
		std::set<unsigned int> suiTmpOrigSet;

		for (std::vector<unsigned int>::const_iterator uiIt = p_rvuiInputMainVec.begin(); uiIt != p_rvuiInputMainVec.end(); uiIt++)
		{
				suiTmpOrigSet.insert(*uiIt);
		}

		// create a set which contains only a single desired value
		std::set<unsigned int> suiSetWithSingleVal;
		suiSetWithSingleVal.insert( p_uiSubValue);

		// compute set difference
		p_rvuiOutputDiffSet.clear();
		std::set_difference(suiTmpOrigSet.begin(), suiTmpOrigSet.end(), suiSetWithSingleVal.begin(), suiSetWithSingleVal.end(), std::inserter(p_rvuiOutputDiffSet, p_rvuiOutputDiffSet.begin() ) );
}


inline static bool setFromVecElements(std::set<unsigned int> &p_rvuiOutputSet, const std::vector<unsigned int> &p_vuiVec, const std::set<unsigned int> &p_suiIndices)
{
        p_rvuiOutputSet.clear();

		for (std::set<unsigned int>::iterator suiIt= p_suiIndices.begin(); suiIt != p_suiIndices.end(); suiIt++)
		{
			if( *suiIt <= p_vuiVec.size() )
			{
				// add to set the element of vector with desired index
				// Note: subtraction of one because indexing for vectors starts at zero and not at one
				p_rvuiOutputSet.insert( p_vuiVec[*suiIt-1] );
			}
			else
			{
				return true;
			}
		}

		return false;
}

inline static std::complex<FLOAT> complexConjugateProduct ( const std::complex<FLOAT>& cFFirstMultiplicand, const std::complex<FLOAT>& cFSecondMultiplicand)
{ 
    return cFFirstMultiplicand*std::conj(cFSecondMultiplicand) ; 
}


#endif
