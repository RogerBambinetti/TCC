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
$Id: AmbienceSynthesis.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "AmbienceSynthesis.h"



// constructor 
AmbienceSynthesis::AmbienceSynthesis()
{
	m_bIsInit = false;
}

// destructor
AmbienceSynthesis::~AmbienceSynthesis()
{
	/* destroy fft handles */
	FFTlib_Destroy(&m_hFFT);
	FFTlib_Destroy(&m_hIFFT);
}

//reset function
bool AmbienceSynthesis::init(unsigned int p_uiFrameSize, 
                             unsigned int p_uiHOAOrder, 
                             int p_iMinOrderForAmbHOA, 
                             bool p_bUsePhaseShiftDecorr,
                             std::shared_ptr<TabulatedValues> p_shptrTabVals)
{
	m_uiFrameSize = p_uiFrameSize;
	m_uiHOAOrder  = p_uiHOAOrder;
	m_uiNoOfHOACoeffs = (p_uiHOAOrder+1)*(p_uiHOAOrder+1);
	m_hFFT = NULL;
	m_hIFFT = NULL;

	// get low order mode matrix 
	if ( p_shptrTabVals->getModeMat(m_vvLowOrderMat, p_iMinOrderForAmbHOA) ) 
	{
		return true;
	}

	m_vFZerosVec.assign( m_uiFrameSize, static_cast<FLOAT>( 0.0) );

	m_bIsInit = true;
 
	m_uiNoOfAmbiChs = (p_iMinOrderForAmbHOA+1) * (1+p_iMinOrderForAmbHOA);
	
    m_bUsePhaseShiftDecorr = p_bUsePhaseShiftDecorr;

    if (m_bUsePhaseShiftDecorr)
	{
		if ( p_shptrTabVals->getInvDecorrCoef(m_vInvDecorrCoef) ) 
		{
			return true;
		}
		computeFadeWinForDirBasedSynth(m_vFOverlapAddWin, p_uiFrameSize);
		std::vector<FLOAT> vFTmpAmbVec(m_uiFrameSize, static_cast<FLOAT>(0.0));
		m_rvvFDelayedLowOrderAmbFrame.assign(m_uiNoOfAmbiChs, vFTmpAmbVec);
		std::vector<float> vFTmpAmbVec2(m_uiFrameSize, static_cast<float>(0.0));
		m_rvvFShiftedOverlapFrame.assign(2, vFTmpAmbVec2);	
		/* init fft/ifft handles */
		FFTlib_Create(&m_hFFT, 2*m_uiFrameSize, 1);
		FFTlib_Create(&m_hIFFT, 2*m_uiFrameSize, -1);

	}

	return false;
}



bool AmbienceSynthesis::process( const	std::vector<std::vector<FLOAT> > & p_rvvFInputPreliminaryDecAmbHOACoeffsFrame,
								std::vector<std::vector<FLOAT> > & p_rvvFOutputFinalAmbHOACoeffsFrame)
{

	// check if AmbienceSynthesis object has already been initialized
	if ( !m_bIsInit )
	{
		return true;
	}

	// check if the buffer sizes are correct
	if ( checkBufferSizes(p_rvvFInputPreliminaryDecAmbHOACoeffsFrame, p_rvvFOutputFinalAmbHOACoeffsFrame) )
	{
		return true;
	}

	// reset to zero
	p_rvvFOutputFinalAmbHOACoeffsFrame.assign( m_uiNoOfHOACoeffs, m_vFZerosVec);

	// copy high order coefficients
	for (unsigned int uiCoeffIdx=m_vvLowOrderMat.size(); uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
	{
		p_rvvFOutputFinalAmbHOACoeffsFrame[uiCoeffIdx].assign(p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[uiCoeffIdx].begin(), p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[uiCoeffIdx].end() );
	}

	if (!m_bUsePhaseShiftDecorr)
	{
		// multiply low order coefficient by mode matrix for recorrelation
		// store result in buffer
		for (unsigned int uiCoffIdx=0; uiCoffIdx < m_vvLowOrderMat.size(); uiCoffIdx++)
		{
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; uiSampleIdx++)
			{
				p_rvvFOutputFinalAmbHOACoeffsFrame[uiCoffIdx][uiSampleIdx] = static_cast<FLOAT>(0.0);

				for (unsigned int uiSigIdx=0; uiSigIdx <  m_vvLowOrderMat.size(); uiSigIdx++)
				{
					p_rvvFOutputFinalAmbHOACoeffsFrame[uiCoffIdx][uiSampleIdx] += m_vvLowOrderMat[uiCoffIdx][uiSigIdx] * p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[uiSigIdx][uiSampleIdx];
				}
			}
		}
	}
	else
	{ // m_bUsePhaseShiftDecorr==1
		FLOAT ifftRescale = -0.5/m_uiFrameSize;
		std::vector<float> vfTemp1(m_uiFrameSize*2, static_cast<float>(0.0));
		std::vector< std::vector<float> > vfTemp;
		vfTemp.assign(2, vfTemp1);
		std::vector<FLOAT> vFTemp3(m_uiFrameSize, static_cast<FLOAT>(0.0));
		std::vector< std::vector<FLOAT> >  vvFPhaseShifted;
		vvFPhaseShifted.assign(2, vFTemp3);
		std::vector<FLOAT> vFTemp4(m_uiFrameSize*2, static_cast<FLOAT>(0.0));
		std::vector< std::vector<FLOAT> >  vvFintermediateMixtureLong;
		vvFintermediateMixtureLong.assign(3, vFTemp4);
	
		// prepare signals for phase shift
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; ++uiSampleIdx)
		{   
			vvFintermediateMixtureLong[0][uiSampleIdx] = m_rvvFDelayedLowOrderAmbFrame[0][uiSampleIdx]+m_rvvFDelayedLowOrderAmbFrame[1][uiSampleIdx];
			vvFintermediateMixtureLong[1][uiSampleIdx] = m_rvvFDelayedLowOrderAmbFrame[0][uiSampleIdx]-m_rvvFDelayedLowOrderAmbFrame[1][uiSampleIdx];	
			vvFintermediateMixtureLong[2][uiSampleIdx] = m_rvvFDelayedLowOrderAmbFrame[2][uiSampleIdx];
			vvFintermediateMixtureLong[0][uiSampleIdx+m_uiFrameSize] = p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[0][uiSampleIdx]+p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[1][uiSampleIdx];
			vvFintermediateMixtureLong[1][uiSampleIdx+m_uiFrameSize] = p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[0][uiSampleIdx]-p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[1][uiSampleIdx];
			vvFintermediateMixtureLong[2][uiSampleIdx+m_uiFrameSize] = p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[2][uiSampleIdx];
		}

		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize*2; ++uiSampleIdx)
		{
			vfTemp[0][uiSampleIdx] = static_cast<float>(m_vFOverlapAddWin[uiSampleIdx]*(m_vInvDecorrCoef[0]*vvFintermediateMixtureLong[1][uiSampleIdx] + vvFintermediateMixtureLong[2][uiSampleIdx]));  
			vfTemp[1][uiSampleIdx] = static_cast<float>(m_vFOverlapAddWin[uiSampleIdx]* m_vInvDecorrCoef[1]*vvFintermediateMixtureLong[0][uiSampleIdx]);	
		}

		// perform phase shift
		for (unsigned int uiChannelIdx=0; uiChannelIdx<2; ++uiChannelIdx)
		{
			std::vector<float> fVecImag(2*m_uiFrameSize, static_cast<float>(0.0));
			FFTlib_Apply(m_hFFT, &vfTemp[uiChannelIdx][0], &fVecImag[0], &vfTemp[uiChannelIdx][0], &fVecImag[0]);

			for (unsigned int uiSampleIdx=1; uiSampleIdx < m_uiFrameSize; ++uiSampleIdx)
			{
				vfTemp[uiChannelIdx][uiSampleIdx] *= 2.0; 
				fVecImag[uiSampleIdx] *= 2.0;
			}
			for (unsigned int uiSampleIdx=m_uiFrameSize+1; uiSampleIdx < 2*m_uiFrameSize; ++uiSampleIdx)
			{
				vfTemp[uiChannelIdx][uiSampleIdx]  = 0.0;
				fVecImag[uiSampleIdx] = 0.0;
			}
			FFTlib_Apply(m_hIFFT, &vfTemp[uiChannelIdx][0], &fVecImag[0], &vfTemp[uiChannelIdx][0], &fVecImag[0]);

			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; ++uiSampleIdx)
			{
				vvFPhaseShifted[uiChannelIdx][uiSampleIdx] = m_rvvFShiftedOverlapFrame[uiChannelIdx][uiSampleIdx] + ifftRescale*fVecImag[uiSampleIdx];
				m_rvvFShiftedOverlapFrame[uiChannelIdx][uiSampleIdx] = ifftRescale*fVecImag[uiSampleIdx+m_uiFrameSize];							
			}
		}
		
		// reconstruct HOA coefficients
		for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; ++uiSampleIdx)
		{
			p_rvvFOutputFinalAmbHOACoeffsFrame[0][uiSampleIdx] = m_vInvDecorrCoef[2] * vvFintermediateMixtureLong[0][uiSampleIdx] + m_vInvDecorrCoef[3] * vvFPhaseShifted[0][uiSampleIdx];
			p_rvvFOutputFinalAmbHOACoeffsFrame[1][uiSampleIdx] = m_vInvDecorrCoef[5] * vvFintermediateMixtureLong[1][uiSampleIdx] + m_vInvDecorrCoef[6] * m_rvvFDelayedLowOrderAmbFrame[2][uiSampleIdx]	+ vvFPhaseShifted[1][uiSampleIdx];			
			p_rvvFOutputFinalAmbHOACoeffsFrame[2][uiSampleIdx] = m_rvvFDelayedLowOrderAmbFrame[3][uiSampleIdx];			
			p_rvvFOutputFinalAmbHOACoeffsFrame[3][uiSampleIdx] = m_vInvDecorrCoef[4] * vvFintermediateMixtureLong[0][uiSampleIdx] - vvFPhaseShifted[0][uiSampleIdx];	
		}

		// buffering
		for (unsigned int uiChannelIdx = 0; uiChannelIdx < m_uiNoOfAmbiChs; ++uiChannelIdx)
		{
			for (unsigned int uiSampleIdx=0; uiSampleIdx < m_uiFrameSize; ++uiSampleIdx)
			{
				m_rvvFDelayedLowOrderAmbFrame[uiChannelIdx][uiSampleIdx] = p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[uiChannelIdx][uiSampleIdx];
			}
		}
	}

	return false;
}


bool AmbienceSynthesis::checkBufferSizes(const std::vector<std::vector<FLOAT> > & p_rvvFInputPreliminaryDecAmbHOACoeffsFrame,
										 std::vector<std::vector<FLOAT> > & rvvFOutputDelayedFinalAmbHOACoeffsFrame)
{
	//// check if the sizes of the buffers are correct
	if ( (p_rvvFInputPreliminaryDecAmbHOACoeffsFrame.size() != m_uiNoOfHOACoeffs) || 
		(rvvFOutputDelayedFinalAmbHOACoeffsFrame.size()         != m_uiNoOfHOACoeffs)  )
	{
		std::cout<< "AmbienceSynthesis: Buffer dimensions don't match with expected dimensions!" << std::endl;
		return true;
	}

	bool bError = false;

	for( unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
	{
		if( (p_rvvFInputPreliminaryDecAmbHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize) || (rvvFOutputDelayedFinalAmbHOACoeffsFrame[uiRowIdx].size() != m_uiFrameSize) )
		{
			bError = true;
		}
	}

	if (bError)
	{
		std::cout<< "AmbienceSynthesis: Buffer lengths don't match!" << std::endl;
	}

	return bError;

}
