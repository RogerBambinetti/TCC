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
/*   robustPan.h 
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    calculate panning gains
* uses:             spacePositions to handle the speaker positions,           
* authors:          Johannes Boehm (jb),
* log:              created 19.08.2013, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: robustPan.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/


#ifndef __ROBUSTPAN_H__
#define __ROBUSTPAN_H__

#include "spacePositions.h"
#include <string>
#include <complex>
#include <stdexcept>

typedef std::complex<double> dcmplx;                          // complex double data type

#define NUM_VIRTMICS  256                                     //number of virtual microphones

 class robustPan
 {
  public:
 
    
    // construct from space positions object
    robustPan(  spacePositions *speakerPos,                 //  positions of loud speakers
                double freq=1000,                           //  freq test frequency in Hz
                double micRadius=0,                         //  virtual microphone radius in m, value 0 (default)= calculated automatically
                bool clipSourceHight=false );               //  enables clipping of hight (low) of sources that do not fall into convex hull spanned by speakers, default off 

    // construct from  loud speaker positions in file      
    robustPan( const std::string &fileNameWithLoudSpeakerPos, // name of file with speaker positions (see examples in directory Positions)
               int fileType,                                 // for MPEG use value 1
               double freq=1000, 
               double micRadius=0, 
               bool clipSourceHight=false);
        
    // calculate gains,  returns simpleMatrix 
    // to pan a single source direction :  double radius=1.0, theta=PI, phi=-0.3;     pM= vbapPan.calculateGains(spacePositions(1, &radius, &theta, &phi));
    // beta: regularization factor >0, bNorm: gain normalization, default off;    
    simpleMtrx * calculateGains(      spacePositions  &sourcePositions,  //  spacePositions object with positions of sources to be panned 
                                      double beta,                       // regularization factor >0
                                      bool bNorm=false );                // gain normalization, default off (RM0);  

    // calculate gains,  returns pointer to double for array of size rows X cols, for all applications that don't use Eigen in their processing scope
    // Row Major Storage, array size:roes*cols=speakerPositions.getNumPositions()*sourcePositions.getNumPositions() 
    double* calculateGains( spacePositions  &sourcePositions, 
                            int * rows, 
                            int *cols, 
                            double beta, 
                            bool bNorm=false);

  private:      
      bool m_clipSourceHight;
      bool m_is2D;
      spacePositions m_SpeakerPos_Cart, m_SpeakerPos_Sp, m_virtMic_xyz;
      double m_maxInclination, m_minInclination;  // lowest and highest speaker positions as inclination
      double m_vMradius, m_k;
      simpleMtrx m_gainMtrx;
      simpleMtrx m_Hml_r,m_Hml_i ;
     void init();         
};


#endif // __ROBUSTPAN_H__


