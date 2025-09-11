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

#define _USE_MATH_DEFINES 
#include <cmath>
#include "hoaRotation.h"
#include "sHmodeMtrx.h"
#include "dshtRotationPositions.h"

//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
hoaRotation::hoaRotation()
{
}
//-----------------------------------------------------------------------------------------------------------------------------------------
// construct using HOA order of content
hoaRotation::hoaRotation(int HOAorder)
	: m_uiHoaOrder(HOAorder), m_uiNumCoeffs( (HOAorder+1)*(HOAorder+1)), m_uiGridIdx(HOAorder-1), m_Threshold(M_PI/180)
{			
	sHmodeMtrx temp(m_uiHoaOrder, pThetaRotPosGrid[m_uiGridIdx], pPhiRotPosGrid[m_uiGridIdx], m_uiNumCoeffs); 		
	simpleMtrx T = *temp.getMtrxPtr();		
	SVD svdT(T);		
	m_pinvT = svdT.pinv();
	m_hoaRotationMatrix = T * m_pinvT;				
	m_fPrevYPR[0]=0; m_fPrevYPR[1]=0; m_fPrevYPR[2]=0;

	// converting grid points to cart. coordintates 
	m_fGridPointsCart.assign(m_uiNumCoeffs, std::vector<FLOAT>(3, 0.0));
	m_fRotatedGridPointsCart.assign(m_uiNumCoeffs, std::vector<FLOAT>(3, 0.0));
	for (unsigned int idx=0; idx< m_uiNumCoeffs; idx++)
	{
		m_fGridPointsCart[idx][0] = sin( pThetaRotPosGrid[m_uiGridIdx][idx] ) * cos( pPhiRotPosGrid[m_uiGridIdx][idx] ) ;
		m_fGridPointsCart[idx][1] = sin( pThetaRotPosGrid[m_uiGridIdx][idx] ) * sin( pPhiRotPosGrid[m_uiGridIdx][idx] ) ;
		m_fGridPointsCart[idx][2] = cos( pThetaRotPosGrid[m_uiGridIdx][idx] );		
	}	

}
//-----------------------------------------------------------------------------------------------------------------------------------------

void hoaRotation::matrixUpdate(FLOAT fYaw, FLOAT fPitch, FLOAT fRoll )
{   
	if ((fabs(m_fPrevYPR[0]-fYaw)>m_Threshold) || (fabs(m_fPrevYPR[1]-fPitch)>m_Threshold) || (fabs(m_fPrevYPR[2]-fRoll)>m_Threshold))
	{
		// 1. updating rotation kernel		
		FLOAT sinYaw = sin(fYaw);
		FLOAT cosYaw = cos(fYaw);
		FLOAT cosPitch = cos(fPitch);
		FLOAT sinPitch = sin(fPitch);
		FLOAT cosRoll = cos(fRoll);
		FLOAT sinRoll = sin(fRoll);

		simpleMtrx rollKernel(3,3);
		simpleMtrx pitchKernel(3,3);
		simpleMtrx yawKernel(3,3);
		simpleMtrx rotKernel(3,3);

		rollKernel[0][0] = cosRoll; rollKernel[0][1] = 0.0; rollKernel[0][2] = sinRoll;
		rollKernel[1][0] = 0.0;		rollKernel[1][1] = 1.0;	rollKernel[1][2] = 0.0;
		rollKernel[2][0] =-sinRoll; rollKernel[2][1] = 0.0;	rollKernel[2][2] = cosRoll;

		pitchKernel[0][0] = 1.0; pitchKernel[0][1] = 0.0;	   pitchKernel[0][2] = 0.0;
		pitchKernel[1][0] = 0.0; pitchKernel[1][1] = cosPitch; pitchKernel[1][2] =-sinPitch;
		pitchKernel[2][0] = 0.0; pitchKernel[2][1] = sinPitch; pitchKernel[2][2] = cosPitch;

		yawKernel[0][0] = cosYaw; yawKernel[0][1] = sinYaw; yawKernel[0][2]=0.0;
		yawKernel[1][0] =-sinYaw; yawKernel[1][1] = cosYaw; yawKernel[1][2]=0.0;
		yawKernel[2][0] = 0.0;	  yawKernel[2][1] = 0.0;	yawKernel[2][2]=1.0;

		rotKernel = yawKernel * pitchKernel * rollKernel;

		// 2. updating cart. position of grid points
		for (unsigned int idx=0; idx< m_uiNumCoeffs; idx++)
		{   
			for (unsigned int x=0; x<3; x++)
			{   
				FLOAT fval = 0;
				for (unsigned int y=0; y<3; y++)
				{
					fval += rotKernel[x][y] * m_fGridPointsCart[idx][y];
				}
				m_fRotatedGridPointsCart[idx][x] = fval;
			}
		}
		// 3. converting rotated gridpoints to cart. positions
		for (unsigned int idx=0; idx< m_uiNumCoeffs; idx++)
		{
			m_vThetaRotatedPosGrid[idx] = acos( m_fRotatedGridPointsCart[idx][2] );
			m_vPhiRotatedPosGrid[idx] = atan2( m_fRotatedGridPointsCart[idx][1], m_fRotatedGridPointsCart[idx][0] );
		}
		//create rotated point grid 
		sHmodeMtrx Tnew(m_uiHoaOrder, m_vThetaRotatedPosGrid, m_vPhiRotatedPosGrid, m_uiNumCoeffs);
		// compute rotation matrix
		m_hoaRotationMatrix = *Tnew.getMtrxPtr() * m_pinvT;

		// storing current rotation 
		m_fPrevYPR[0] = fYaw;
		m_fPrevYPR[1] = fPitch;
		m_fPrevYPR[2] = fRoll;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------
// apply Rotation to HOA signal
int hoaRotation::blockProcess(  std::vector<std::vector<FLOAT> > &inBuffer, 
						    std::vector<std::vector<FLOAT> > &rotDataBuffer,  
                            std::vector<std::vector<FLOAT> > &outBuffer)
{
    unsigned int uiFrameSize = inBuffer[0].size();

    // resize tempBuffer   
	std::vector<std::vector<FLOAT> >tempBuffer;
	tempBuffer.assign(m_uiNumCoeffs, std::vector<FLOAT>(uiFrameSize, 0.0));

   unsigned int minCh;
   unsigned int maxCh;
   for (unsigned int nt=0; nt<uiFrameSize; nt++)
   {   
	   // upating rotation matrix. 
	   // The update rate can be slower (e.g., every frame), based on the actual polling frequency from the scene-displacement interface.
	   // Some interloation may be necessary in this case.	  
	   matrixUpdate(M_PI*rotDataBuffer[0][nt],M_PI*rotDataBuffer[1][nt],M_PI*rotDataBuffer[2][nt]); 

	   for(unsigned int N=0; N< m_uiHoaOrder+1; N++) 
	   {
		   minCh = N*N;
		   maxCh = minCh+2*N+1;
		   for(unsigned int och=minCh; och<maxCh; och++) 
		   {   			   
			   for (unsigned int mN=minCh; mN<maxCh; mN++)
			   {   				    
				   tempBuffer[och][nt] += (FLOAT) (m_hoaRotationMatrix[och][mN] * inBuffer[mN][nt]);   
			   }			    
		   }		   
	   } 
   }
	outBuffer = tempBuffer;

  return 0;

}             