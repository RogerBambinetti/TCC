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
/*   dshtTs2cMtrx.h
*                   Copyright (c) 2014, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    Matrix class to transform from  spatial domain to coefficient domain (HOA),  (MPEG-H-RM3)
* authors:          Johannes Boehm (jb), 
* log:              created 24.07.2014, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: dshtTs2cMtrx.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/


#ifndef __DSHTS2CMTRX_H__
#define __DSHTS2CMTRX_H__


#include <stdexcept>
#include <vector>
#include "dshtTc2sMtrx.h"

/** forward declaration of used classes */


#ifndef FLOAT
 #define FLOAT double
#endif


class dshtTs2cMtrx
{
 public:
    dshtTs2cMtrx( int HOAorder, dshtTc2sMtrx T1);
    dshtTs2cMtrx();
    // Matrix access
    // operator overload for direct access as two dim array
    double* operator[](int row)  { return & m_Dmatrix.data()[row * m_cols];}   // allows access as C array: mydshtTc2sMtrxObj[(m_HOAorder+1)^2][(m_HOAorder+1)^2]
    //ptr to simpleMtrx -> returns a pointer to an simpleMatrix member 
    simpleMtrx * getMtrxPtr()  {return & m_Dmatrix;} 
    void overload(std::vector<std::vector<FLOAT> >  &renderingMatrix, int numHoaCoeffs, int numChannels);
    
    // getters
    int getHOAorder() { return m_HOAorder;}
    int getNumberOfSpeakers() {return nSpeakers; }
    int getNumberOfCoefficients(){return m_cols; }

 private:      
    int m_HOAorder;                      
    int m_cols, nSpeakers;             
    simpleMtrx m_Dmatrix;
};



#endif // __DSHTS2CMTRX_H__


