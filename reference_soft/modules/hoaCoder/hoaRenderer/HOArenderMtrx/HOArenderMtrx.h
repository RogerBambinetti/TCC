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
/*   HOArenderMtrx.h 
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    calculate a HOA rendering matrix or load from file or table
* uses:             spacePositions to handle the speaker positions, robustPan to calculate the panning functions, simpleMtrx              
* authors:          Johannes Boehm (jb), 
* log:              created 23.08.2013, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: HOArenderMtrx.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/


#ifndef __HOARENDERMTRX_H__
#define __HOARENDERMTRX_H__

#include "spacePositions.h"
#include <string>
#include <stdexcept>
#include <vector>


class HOArenderMtrx
{
 public:

    // construct from matrix file
    HOArenderMtrx( unsigned long renderId,              // a rendering id to identify the matrix (if 0 -> selection by HOA order)
                   const std::string &matrixFileName,   // file name with matrix data
                   int addNlfeCh=0,                     // number of LFE channels to add at end (rows) of matrix (zero mixing coefficients) 
                   bool reTwiddleHOAcoeffs=true);       // true indicates HOA coefficients in matrixFileName are in MPEG4 sequence, false in linear sequence order (as used internally)   
    
    // construct from signaled rendering matrix file
    HOArenderMtrx( const std::string &matrixFileName);   // file name with matrix data

    // construct from matrix ROM table
    HOArenderMtrx( unsigned long renderId, 
                   double *pTable,                      // pointer to table with matrix coefficients, stored in memory as a C table [ hoaOrder][nChannels]
                   int hoaOrder,                             
                   int nChannels, 
                   int addNlfeCh=0, 
                   bool reTwiddleHOAcoeffs=true);

    // construct from loudspeaker positions stored in spacePositions object
    HOArenderMtrx(spacePositions *speakerPos,           // spacePositions object with the loudspeaker positions of the setup
                  int HOAorder, 
                  unsigned long renderID=0, 
                  int addNlfeCh=0);

    // construct from loudspeaker positions stored in file
    HOArenderMtrx(const std::string &fileNameWithLoudSpeakerPos, // name of file with loudspeaker positions
                  int fileType,                                  // 1 or 2, use 1 for MPEG related files, see examples in directory Positions 
                  int HOAorder, 
                  unsigned long renderID=0, 
                  bool addlfeCh=false);                          // indicated if LFE in  fileNameWithLoudSpeakerPos will be included or excluded at end of matrix
	
    // default, construct an empty object
    HOArenderMtrx();


    // Matrix access
    // operator overload for direct access as two dim array
    double* operator[](int row)  { return & m_Dmatrix.data()[row * m_cols];}   // allows access as C array: myHOArenderMtrxObj[channelIdx][HOAorderIdx]
    
    int resortSpeakerPositions(const std::vector< unsigned int> order);
    
    //ptr to simpleMtrx object
    simpleMtrx * getMtrxPtr()  {return &m_Dmatrix;} 

    
    // getters
    int getHOAorder() { return m_HOAorder;};
    int getNumberOfSpeakers() {return m_Lspeakers; };
    int getNumberOfSubwoofer() {return m_uiNumSubwoofer; };
    int getNumberOfSatellites() {return m_uiNumSatellites; };  
    unsigned long getRendererID() {return m_renderID;};
    
    const std::vector<std::vector<double >> & getSignaledSpeakerPositionRad(){ return m_vvFsignaledSpeakerPosRad;};
    const std::vector< unsigned int> & getSignaledSpeakerType(){ return m_uiLsTypes;};
 private:      
    int m_HOAorder;                      
    int m_Lspeakers; 
    int m_cols;     
     int m_uiNumSubwoofer;
     int m_uiNumSatellites;
    unsigned long m_renderID;
    std::vector<std::vector<double >> m_vvFsignaledSpeakerPosRad; //speaker positions in radian associated with optinlally signaled rendering matrix 
    std::vector<unsigned int > m_uiLsTypes;
    simpleMtrx m_Dmatrix;
    void init(spacePositions *speakerPos, int addNlfeCh);  // construct a matrix from loud speaker positions
    void retwiddle();                                      // convert HOA sequence  from MPEG-4 to linear sequence order
};


#endif // __HOARENDERMTRX_H__


