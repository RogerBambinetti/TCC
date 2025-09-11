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
/*   DRC1hoa.h
*                   Copyright (c) 2014, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    DRC1 for HOA signals,  (MPEG-H-RM3)
* authors:          Johannes Boehm (jb), 
* log:              created 29.07.2014, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: DRC1hoa.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/

//#define ENABLE_HOA_DRC_MULTIBAND 

#ifndef __DRC1HOA_H__
#define __DRC1HOA_H__

#define ENABLE_HOA_DRC_MULTIBAND // define to enable DRC Multi-band

#include <vector>
#include <memory>
#ifdef  ENABLE_HOA_DRC_MULTIBAND
#include "QMF_Wrapper.h"
#else
#ifndef FLOAT
 #define FLOAT double
#endif
#endif


/** forward declaration of used classes */
class dshtTc2sMtrx;
class dshtTs2cMtrx;



class DRC1hoa
{
 public:
        DRC1hoa();
        DRC1hoa(int HOAorder, int mode=1);
        void enableRendering(std::vector<std::vector<FLOAT> > &renderingMatrix);   // enable rendering by providing a rendering matrix. 
                                                                                   // As a result the output of blockProcess() will be a channel signal
        bool outputsAreChannels(){return bOutputAreChannels;};
        bool outputsAreHOACoeffs(){return !bOutputAreChannels;};
        int blockProcess(  std::vector<std::vector<FLOAT> > &inBuffer,   // HOA input:  2D vector matrix, number of coefficients X time samples
                           std::vector<std::vector<FLOAT> > &drcBuffer,  // DRC input:  2D vector matrix, number of DRC coefficients X time samples
                           std::vector<std::vector<FLOAT> > &outBuffer,  // HOA or Channel output:  2D vector matrix, number of output HOAcoeffs or channels X time samples
                           bool bEnableClippingProtect=false,            // switch on clipping to avoid overflow and later wrap around, default off  
                           bool bInPlaceFlag=false                       //  enable for inplace processing, inBuffer==outBuffer
                        );        
 private:
        bool bOutputAreChannels; 
        int numCoeffs, numOutputs;
        std::shared_ptr<dshtTc2sMtrx> T1;
        std::shared_ptr<dshtTs2cMtrx> T2D;    
        std::vector<std::vector<FLOAT> > vDoverload;
        int tdMode;
#ifdef  ENABLE_HOA_DRC_MULTIBAND
        std::shared_ptr<CqmfAnalysis> m_pQmfAna;
        std::shared_ptr<CqmfSynthesis>m_pQmfSyn;
        std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  m_vFdBuffer;  
        std::vector<std::vector<FLOAT> > m_vTdBufer;
#endif
};

#endif