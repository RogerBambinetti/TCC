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
$Id: dshtTc2sMtrx.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#include "dshtTc2sMtrx.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sHmodeMtrx.h"
#ifdef USE_PREPROCESSEDMTRX
#include "dshtDrcMtrxRom1.h"
#endif
#include "dshtDrcPositions.h"
using namespace std;


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
 dshtTc2sMtrx::dshtTc2sMtrx()
 : m_HOAorder(0), m_cols(0)
{
}



//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from table
#ifdef USE_PREPROCESSEDMTRX
dshtTc2sMtrx::dshtTc2sMtrx(  int hoaOrder, bool bCalcFromPositions)
#else
dshtTc2sMtrx::dshtTc2sMtrx(  int hoaOrder)
#endif
: m_HOAorder(hoaOrder), m_cols((hoaOrder+1)*(hoaOrder+1))
{
  if (m_HOAorder>9) throw(std::runtime_error("dshtTc2sMtrx: Tables only provided up to HOA order 9"));
  m_Dmatrix.resize(m_cols, m_cols); 
#ifdef USE_PREPROCESSEDMTRX
  if (!bCalcFromPositions) 
    {
    double *Ddst= reinterpret_cast  <double*> (pTc2[m_HOAorder-1]);     
    // read from ROM: preprocessed  matrix  and map it to matrix  // more elegant with
    for (int c=0; c< m_cols; c++)        // loop over coefficients 
        {
        for (int r=0; r<m_cols; r++)  // loop over spherical positions
             m_Dmatrix[c][r] = *Ddst++;        
        }
    } else
#endif
    {
    // calculation of mode matrix from DRC positions
    sHmodeMtrx psi(m_HOAorder, pThetaDrc[m_HOAorder-1], pPhiDrc[m_HOAorder-1], m_cols);
    // weight mode matrix with quadrature values from table and store in matrix G
    double *qudratureWeights=pqDrc[m_HOAorder-1];
    simpleMtrx G(m_cols, m_cols); G.zeros();
    for (int ll=0;  ll<m_cols ; ll++) G[ll][ll]= qudratureWeights[ll];  
   // cout << * psi.getMtrxPtr() << endl;
    G = * psi.getMtrxPtr() * G;
    // Perform an SVD of transpose     
    G.transpose();
    SVD svd(G);
    // Build a energy preserving matrix
    svd.V.transpose();
    G=  svd.U * svd.V;
    // normalize
    double normG = 1.0/G.normFro();
    G.multScalar(normG);
    m_Dmatrix=G;
    //  asum= sum(D_LO);    r=zeros(1,L); r(1)=1;    e=(r-asum)./L;      D_LO= D_LO+ repmat(e, L,1);
    std::vector<double> e(m_cols); 
    for(int c=0; c<m_cols;c++)
        e[c]= m_Dmatrix[0][c];
    for (int ll=1; ll<m_cols; ll++)
        for(int c=0; c<m_cols;c++)
           e[c] +=  m_Dmatrix[ll][c];
    e[0]-=1;
    for(int c=0; c<m_cols;c++)
         e[c] *= (1.0/ m_cols);
    for (int c=0; c< m_cols; c++)     
        for (int r=0; r<m_cols; r++)  
          m_Dmatrix[r][c]= m_Dmatrix[r][c] - e[c];            
 }
}







