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

#include "par_decorr.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>


CparDecorrelator::CparDecorrelator()
  : m_nDecorrNumber(0)
{
}

CparDecorrelator::~CparDecorrelator()
{
}

bool CparDecorrelator::init(int nDecorrNumber)
{
  m_nDecorrNumber = nDecorrNumber;

  // init decorrelator: filter coeffs
  m_filter.init(nDecorrNumber);

  return false;
}

// processing of one frame of QMF band samples
void CparDecorrelator::process(const std::vector< std::complex<FLOAT> > &p_rvcQmfSamplesIn, 
                                     std::vector< std::complex<FLOAT> > &p_rvcQmfSamplesOut)
{
   // apply filter to input buffer 
   m_filter.process(p_rvcQmfSamplesIn, p_rvcQmfSamplesOut);
}

void CparDecorrelator::process_sample(const std::complex<FLOAT> &p_rcQmfSampleIn, 
                              std::complex<FLOAT> &p_rcQmfSampleOut)
{
   m_filter.process_sample(p_rcQmfSampleIn, p_rcQmfSampleOut);
}