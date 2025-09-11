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
 $Id: PreliminaryHOAComposition.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#include "PreliminaryHOAComposition.h"



// constructor 
PreliminaryHOAComposition::PreliminaryHOAComposition()
{
		m_bIsInit = false;
}


//reset function
bool PreliminaryHOAComposition::init(unsigned int p_uiFrameSize,
                                     int p_iMaxPARHOAOrder,
                                     unsigned int p_uiHOAOrder
                                     )
{
	m_uiFrameSize = p_uiFrameSize;
	m_uiHOAOrder  = p_uiHOAOrder;
	m_uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
	
   m_uiMaxNoOfPARUpmixSigs = static_cast<unsigned int>((p_iMaxPARHOAOrder+1)*(p_iMaxPARHOAOrder+1));

	m_bIsInit = true;

	return false;
}


//process function
bool PreliminaryHOAComposition::process(   const std::vector<std::vector<FLOAT> > & p_rvvFInputPreDomSoundHOACoeffsFrame,
                                           const std::vector<std::vector<FLOAT> > & p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame,
                                                 std::vector<std::vector<FLOAT> > & p_rvvFOutputPrelimComposedForPARHOACoeffsFrame,
                                           const std::vector<std::vector<FLOAT> > & p_rvvFInputAmbHOACoeffsFrame,
									                      std::vector<std::vector<FLOAT> > & p_rvvFOutputPrelimComposedHOACoeffsFrame)
{

	// check if HOAComposition object has already been initialized
	 if ( !m_bIsInit )
	 {
		 return true;
	 }

	// check if the buffer sizes are correct
	if ( checkBufferSizes(p_rvvFInputPreDomSoundHOACoeffsFrame, 
                          p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame,
                          p_rvvFInputAmbHOACoeffsFrame) )
	{
		return true;
	}

    p_rvvFOutputPrelimComposedHOACoeffsFrame.resize(m_uiNoOfHOACoeffs);

	for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
        p_rvvFOutputPrelimComposedHOACoeffsFrame[uiCoeffIdx].resize(m_uiFrameSize);
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
				p_rvvFOutputPrelimComposedHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFInputPreDomSoundHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] 
                                                                                          + p_rvvFInputAmbHOACoeffsFrame[uiCoeffIdx][uiSampleIdx];
		}
	}

    p_rvvFOutputPrelimComposedForPARHOACoeffsFrame.resize(m_uiMaxNoOfPARUpmixSigs);
    for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiMaxNoOfPARUpmixSigs; uiCoeffIdx++)
    {
        p_rvvFOutputPrelimComposedForPARHOACoeffsFrame[uiCoeffIdx].resize(m_uiFrameSize);
        for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
		{
            p_rvvFOutputPrelimComposedForPARHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] = p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame[uiCoeffIdx][uiSampleIdx] 
                                                                                                       + p_rvvFInputAmbHOACoeffsFrame[uiCoeffIdx][uiSampleIdx];
        }
    }

	return false;
}


bool PreliminaryHOAComposition::checkBufferSizes(	 const std::vector<std::vector<FLOAT> > & p_rvvFInputPreDomSoundHOACoeffsFrame, 
                                                    const std::vector<std::vector<FLOAT> > & p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame,
                                                    const std::vector<std::vector<FLOAT> > & p_rvvFInputAmbHOACoeffsFrame)
{
	 //// check if the sizes of the buffers are correct
	 if ( (p_rvvFInputPreDomSoundHOACoeffsFrame.size() != m_uiNoOfHOACoeffs) 
		    || (p_rvvFInputAmbHOACoeffsFrame.size()         != m_uiNoOfHOACoeffs) 
          || (p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame.size() != m_uiNoOfHOACoeffs)  
          )
	 {
		std::cout<< "PreliminaryHOAComposition: Buffer dimensions don't match with expected dimensions!" << std::endl;
		return true;
	 }

	 bool bError = false;

	 for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
	 {
		 if( (p_rvvFInputPreDomSoundHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize)  
             || (p_rvvFInputFadedPreDomSoundForPARHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize) 
			    || (p_rvvFInputAmbHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize)  )
		 {
			 bError = true;
		 }
	 }

	 if (bError)
	 {
		 std::cout<< "PreliminaryHOAComposition: Buffer lengths don't match!" << std::endl;
	 }

	 return bError;

}