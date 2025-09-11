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
/*   dshtTc2sMtrx.h
*                   Copyright (c) 2014, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    Matrix class to transform from coefficient domain (HOA) to spatial domain,  (MPEG-H-RM3)
* authors:          Johannes Boehm (jb), 
* log:              created 24.07.2014, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: dshtTs2cMtrx.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#include "dshtTs2cMtrx.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
 dshtTs2cMtrx::dshtTs2cMtrx()
 : m_HOAorder(0), m_cols(0), nSpeakers(0)
{
}



//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from dshtTc2sMtrx object
dshtTs2cMtrx::dshtTs2cMtrx( int HOAorder, dshtTc2sMtrx T1)
: m_HOAorder(HOAorder), m_cols((HOAorder+1)*(HOAorder+1)), nSpeakers(m_cols)
{
  if (m_HOAorder>9) throw(std::runtime_error("dshtTc2sMtrx: Tables only provided up to HOA order 9"));
  //m_Dmatrix.resize(m_cols, m_cols); 
  SVD svd(*(T1.getMtrxPtr()));
  m_Dmatrix = svd.pinv(); 
    //cout<<m_Dmatrix<<endl;
 }




void dshtTs2cMtrx::overload(std::vector<std::vector<FLOAT> >  &renderingMatrix, int numHoaCoeffs, int numChannels)
{
 if (numHoaCoeffs != m_cols)
     throw(std::runtime_error("dshtTc2sMtrx: overload with rendering matrix number of hoa coefficients does not match"));
 nSpeakers=numChannels;
  simpleMtrx Dr(numChannels, numHoaCoeffs);
 for(int och=0; och<nSpeakers; och++)
  {
    
    for (int mN=0; mN<numHoaCoeffs; mN++)
    {
        Dr[och][mN]= renderingMatrix[och][mN];   
    }
  }

 m_Dmatrix= Dr * m_Dmatrix;
}


