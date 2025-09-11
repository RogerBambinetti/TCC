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

// phase-based decorrelation: 
// Copyright (c) 2014 Qualcomm Technologies, Inc. (QTI).

/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: AmbienceSynthesis.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#pragma once

#define _USE_MATH_DEFINES 

extern "C" {
#include "fftlib.h" 
}
#include <cmath>
#include "DataTypes.h"
#include "TabulatedValues.h"
#include "CommonFunctions.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <memory>





class AmbienceSynthesis
{
public:
   AmbienceSynthesis();

   bool init(  unsigned int p_uiFrameSize, 
      unsigned int p_uiHOAOrder, 
      int p_iMinOrderForAmbHOA, 
      bool p_bUsePhaseShiftDecorr,
      std::shared_ptr<TabulatedValues> p_shptrTabVals);

   bool process(  const std::vector<std::vector<FLOAT> > & p_rvvFInputPreliminaryDecAmbHOACoeffsFrame,
      std::vector<std::vector<FLOAT> > & p_rvvFOutputFinalAmbHOACoeffsFrame);

   // Destructor
    ~AmbienceSynthesis(); 
private:
	
   bool m_bIsInit;		

   unsigned int m_uiFrameSize;
   unsigned int m_uiHOAOrder;
   unsigned int m_uiNoOfHOACoeffs;

   std::vector< std::vector<FLOAT> > m_vvLowOrderMat;

   std::vector<FLOAT> m_vFZerosVec;

   bool m_bUsePhaseShiftDecorr;
   unsigned int  m_uiNoOfAmbiChs;
   std::vector< FLOAT > m_vInvDecorrCoef;
   std::vector< FLOAT > m_vFOverlapAddWin;
   std::vector< std::vector<FLOAT> > m_rvvFDelayedLowOrderAmbFrame;
   std::vector< std::vector<float> > m_rvvFShiftedOverlapFrame;	
  	/** handle to FFT/IFFT */
	HANDLE_FFTLIB m_hFFT;
	HANDLE_FFTLIB m_hIFFT;


   // member functions
   bool checkBufferSizes(const std::vector<std::vector<FLOAT> > & p_rvvFInputPreliminaryDecAmbHOACoeffsFrame,
      std::vector<std::vector<FLOAT> > & p_rvvFOutputDelayedFinalAmbHOACoeffsFrame);

};
