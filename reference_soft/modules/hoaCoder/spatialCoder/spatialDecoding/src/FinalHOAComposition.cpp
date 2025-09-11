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
 $Id: FinalHOAComposition.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#include "FinalHOAComposition.h"



// constructor 
FinalHOAComposition::FinalHOAComposition()
{
		m_bIsInit = false;
}


//reset function
bool FinalHOAComposition::init(    unsigned int p_uiFrameSize, 
                                   unsigned int p_uiHOAOrder, 
                                   int          p_iMaxPARHOAOrder,
                                   unsigned int p_uiNoOfQMFDelaySamples)
{
	m_uiFrameSize = p_uiFrameSize;
	m_uiHOAOrder  = p_uiHOAOrder;
	m_iMaxPARHOAOrder = p_iMaxPARHOAOrder;
    
    m_uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
    m_uiMaxNoOfPARHOACoeffs = (p_iMaxPARHOAOrder+1)*(p_iMaxPARHOAOrder+1);

    m_uiNoOfQMFDelaySamples = p_uiNoOfQMFDelaySamples;
	
    std::vector<FLOAT> vFTmpShortVec(m_uiNoOfQMFDelaySamples, static_cast<FLOAT>(0.0));

    m_vvFPrelimComposedHOACoeffsLastShortHOAFrame.assign(m_uiNoOfHOACoeffs, vFTmpShortVec);

	m_bIsInit = true;

	return false;
}


//process function
bool FinalHOAComposition::process(  const std::vector<std::vector<FLOAT> > & p_rvvFInputPrelimComposedHOACoeffsFrame, 
						            const std::vector<std::vector<FLOAT> > & p_rvvFInputPARHOACoeffsFrame,
                                    const std::vector<std::vector<FLOAT> > & p_rvvFInputSubBandDirSigsCompositionHOACoeffsFrame, 
							                std::vector<std::vector<FLOAT> > & p_rvvFOutputFinallyComposedHOACoeffsFrame)
{

	// check if HOAComposition object has already been initialized
	 if ( !m_bIsInit )
	 {
		 return true;
	 }

    // check if the buffer sizes are correct
	if ( checkBufferSizes(  p_rvvFInputPrelimComposedHOACoeffsFrame, 
                            p_rvvFInputPARHOACoeffsFrame, 
                            p_rvvFInputSubBandDirSigsCompositionHOACoeffsFrame,
                            p_rvvFOutputFinallyComposedHOACoeffsFrame) )
	{
		return true;
	}

    p_rvvFOutputFinallyComposedHOACoeffsFrame.resize(m_uiNoOfHOACoeffs);

    // add HOA representation of sub-band directional signals to output buffer
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
        p_rvvFOutputFinallyComposedHOACoeffsFrame[uiCoeffIdx].assign( p_rvvFInputSubBandDirSigsCompositionHOACoeffsFrame[uiCoeffIdx].begin(),
                                                               p_rvvFInputSubBandDirSigsCompositionHOACoeffsFrame[uiCoeffIdx].end() );
    }

    // add HOA representation of parametrically replicated ambience
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiMaxNoOfPARHOACoeffs; uiCoeffIdx++)
    {
        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
            p_rvvFOutputFinallyComposedHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] += p_rvvFInputPARHOACoeffsFrame[uiCoeffIdx][uiSampleIdx];
        }
    }

    // add delayed preliminary composed HOA representation to output buffer
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
    {
        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiNoOfQMFDelaySamples; uiSampleIdx++)
		{
            p_rvvFOutputFinallyComposedHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] += m_vvFPrelimComposedHOACoeffsLastShortHOAFrame[uiCoeffIdx][uiSampleIdx];
        }

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize-m_uiNoOfQMFDelaySamples; uiSampleIdx++)
		{
            p_rvvFOutputFinallyComposedHOACoeffsFrame[uiCoeffIdx][m_uiNoOfQMFDelaySamples+uiSampleIdx] +=   p_rvvFInputPrelimComposedHOACoeffsFrame[uiCoeffIdx][uiSampleIdx];
        }
    }

    // buffer sum of predom. sound and ambient HOA
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
        m_vvFPrelimComposedHOACoeffsLastShortHOAFrame[uiCoeffIdx].assign(   p_rvvFInputPrelimComposedHOACoeffsFrame[uiCoeffIdx].begin() 
                                                                                                    + m_uiFrameSize-m_uiNoOfQMFDelaySamples, 
                                                                            p_rvvFInputPrelimComposedHOACoeffsFrame[uiCoeffIdx].end() );
    }

	return false;
}


bool FinalHOAComposition::checkBufferSizes(	const std::vector<std::vector<FLOAT> > & p_rvvFInputPrelimComposedHOACoeffsFrame, 
										const std::vector<std::vector<FLOAT> > & p_rvvFInputPARHOACoeffsFrame,
                                        const std::vector<std::vector<FLOAT> > & p_rvvFInputPredDirHOACoeffsFrame,
											  std::vector<std::vector<FLOAT> > & p_rvvFOutputFinallyComposedHOACoeffsFrame)
{
	 //// check if the sizes of the buffers are correct
	 if ( (p_rvvFInputPrelimComposedHOACoeffsFrame.size() != m_uiNoOfHOACoeffs) || 
		  (p_rvvFInputPARHOACoeffsFrame.size()         != m_uiMaxNoOfPARHOACoeffs) ||
          p_rvvFInputPredDirHOACoeffsFrame.size()      != m_uiNoOfHOACoeffs)
	 {
		std::cout<< "FinalHOAComposition: Buffer dimensions don't match with expected dimensions!" << std::endl;
		return true;
	 }

	 if ( p_rvvFOutputFinallyComposedHOACoeffsFrame.size()   != m_uiNoOfHOACoeffs )
	 {
		 p_rvvFOutputFinallyComposedHOACoeffsFrame.resize(m_uiNoOfHOACoeffs);
	 }

	 bool bError = false;

     for( unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNoOfPARHOACoeffs; uiRowIdx++)
	 {
         if(p_rvvFInputPARHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize)
         {
             bError = true;
         }
     }

	 for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
	 {
		 if( (p_rvvFInputPrelimComposedHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize) || 
             (p_rvvFInputPredDirHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize) )
		 {
			 bError = true;
		 }

		 if(p_rvvFOutputFinallyComposedHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize)
		 {
			 p_rvvFOutputFinallyComposedHOACoeffsFrame[uiRowIdx].resize(m_uiFrameSize);
		 }
	 }

	 if (bError)
	 {
		 std::cout<< "FinalHOAComposition: Buffer lengths don't match!" << std::endl;
	 }

	 return bError;

}
