/***********************************************************************************

This software module was originally developed by 

Qualcomm Technologies, Inc. (QTI)

in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
performance may not have been optimized. This software module is an implementation
of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
and make derivative works of this software module or modifications  thereof for use
in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
and which satisfy any specified conformance criteria. Those intending to use this 
software module in products are advised that its use may infringe existing patents. 
ISO/IEC have no liability for use of this software module or modifications thereof. 
Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
standard.

Qualcomm Technologies retains full right to modify and use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using 
the code for products that do not conform to MPEG-related ITU Recommendations and/or 
ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2013.

***********************************************************************************/



#ifndef __hoaRotation_H__
#define __hoaRotation_H__ 


#include "SimpleMtrx.h"
#include "hoaRenderer.h"
#include <string>
#include <stdexcept>
#include <vector>
#ifndef FLOAT
 #define FLOAT double
#endif


/** forward declaration of used classes */
class dshtTc2sMtrx;
class dshtTs2cMtrx;

class hoaRotation
{
 public:
        hoaRotation();
        hoaRotation(int HOAorder);
        int blockProcess(  std::vector<std::vector<FLOAT> > &inBuffer,   // HOA input:  2D vector matrix, number of coefficients X time samples
                           std::vector<std::vector<FLOAT> > &rotDataBuffer,  // Yaw-Pitch-Roll rotation data input: signals are in range -1 and 1 which corresponds to -pi and pi.
                           std::vector<std::vector<FLOAT> > &outBuffer  // HOA output:  2D vector matrix, number of output HOAcoeffs samples
                        );   
 private:
		void matrixUpdate(FLOAT fYaw, FLOAT fPitch, FLOAT fRoll);
        unsigned int m_uiNumCoeffs;
		unsigned int m_uiHoaOrder;
		unsigned int m_uiGridIdx;
		FLOAT m_fPrevYPR[3];
		FLOAT m_Threshold;
        simpleMtrx m_pinvT;  
		simpleMtrx m_hoaRotationMatrix;
		std::vector<std::vector<FLOAT> > m_fGridPointsCart;
		FLOAT m_vThetaRotatedPosGrid[49];
		FLOAT m_vPhiRotatedPosGrid[49];
		std::vector<std::vector<FLOAT> > m_fRotatedGridPointsCart;
};

#endif // __hoaRotation_H__