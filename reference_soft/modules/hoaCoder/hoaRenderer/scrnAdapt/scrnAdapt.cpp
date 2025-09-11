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

#include "scrnAdapt.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "sHmodeMtrx.h"
#include "dshtScrnAdaptPositions.h"

using namespace std;


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
scrnAdapt::scrnAdapt()
{
}

// construct from screen information in file
scrnAdapt::scrnAdapt(const std::string &p_sScreenSizeFile, const unsigned int p_uiNumAnchorPoints, HOArenderMtrx &p_mInD, HOArenderMtrx &p_mOutD)	 
{
	string line;
	ifstream myfile (p_sScreenSizeFile);
	if (!myfile.is_open())
	{
		throw(std::runtime_error("Cannot open ScreenSize file"));
	}else
	{   
		bool bAdapt = false;		 
		getline (myfile,line); // # production screen azimuth
		getline (myfile,line);
		m_productionScreen_lrud[0] = atof(line.c_str())*PI/180.0;
		getline (myfile,line);
		m_productionScreen_lrud[1] = atof(line.c_str())*PI/180.0;
		getline (myfile,line); // # production screen elevation
		getline (myfile,line);
		m_productionScreen_lrud[2] = atof(line.c_str())*PI/180.0;
		getline (myfile,line);
		m_productionScreen_lrud[3] = atof(line.c_str())*PI/180.0;
		getline (myfile,line); // # local screen azimuth
		getline (myfile,line);
		m_localScreen_lrud[0] = atof(line.c_str())*PI/180.0;
		getline (myfile,line);
		m_localScreen_lrud[1] = atof(line.c_str())*PI/180.0;
		getline (myfile,line); // # local screen elevation
		getline (myfile,line);
		m_localScreen_lrud[2] = atof(line.c_str())*PI/180.0;
		getline (myfile,line);
		m_localScreen_lrud[3] = atof(line.c_str())*PI/180.0;	
	

		for (int idx=0; idx<4; ++idx)
		{
			if (m_localScreen_lrud[idx] != m_productionScreen_lrud[idx])
				bAdapt = true;
		}

		init(p_mInD, p_mOutD);

		if (bAdapt && m_uiHoaOrder) //nothing to adapt if m_uiHoaOrder==0
		{
			unsigned int uiNumAnchorPoints = (unsigned int)sqrt(p_uiNumAnchorPoints);
			uiNumAnchorPoints *= uiNumAnchorPoints; // preventing unsuported number of points

			if ((uiNumAnchorPoints!=900 && uiNumAnchorPoints>144) || (uiNumAnchorPoints < 16))
				throw(std::runtime_error("Screen Adaptation: Number of anchor points is not supported"));

			process(uiNumAnchorPoints, p_mInD, p_mOutD);
		}	
	}
}

void scrnAdapt::init(HOArenderMtrx &p_mInD, HOArenderMtrx &p_mOutD) 

{
	m_uiNumSpeakers = p_mInD.getNumberOfSpeakers();
	m_uiNumSatellites = p_mInD.getNumberOfSatellites();	
	m_uiHoaOrder = p_mInD.getHOAorder();	
	m_uiNumCoeffs = (m_uiHoaOrder+1)*(m_uiHoaOrder+1);

	p_mOutD = p_mInD;

	simpleMtrx inputD = *p_mInD.getMtrxPtr();
	m_Dmatrix.resize(m_uiNumSatellites, m_uiNumCoeffs);	

	//copy only sattelites speakers into local matrix
	unsigned int localSpkIdx = 0;		

	for (unsigned int idxSpk=0; idxSpk<m_uiNumSpeakers; idxSpk++) 
	{   
		if (p_mInD.getSignaledSpeakerType()[idxSpk] == 0)
		{
			for (unsigned int idx=0; idx<m_uiNumCoeffs; idx++)  
			{					
				m_Dmatrix[localSpkIdx][idx] = inputD[idxSpk][idx];	
			}				
			localSpkIdx++;
		}
	}
}

void scrnAdapt::process(const unsigned int p_uiNumAnchorPoints, HOArenderMtrx &p_mInD, HOArenderMtrx &p_mOutD) 
{
	std::vector<double>gain_Num(p_uiNumAnchorPoints);
	std::vector<double>gain_Denum(p_uiNumAnchorPoints);
	simpleMtrx DFx(m_uiNumSatellites,p_uiNumAnchorPoints); 
	simpleMtrx A(p_uiNumAnchorPoints, p_uiNumAnchorPoints); 
	simpleMtrx outputD = *p_mInD.getMtrxPtr();

	unsigned int gridIdx;

	if (p_uiNumAnchorPoints==900)
	{
		gridIdx = 0;
	}else
	{
		gridIdx = sqrt(p_uiNumAnchorPoints)-2;
	}

	//load point grid, could be moved into init function 
	sHmodeMtrx temp(m_uiHoaOrder, pThetaGridSA[gridIdx], pPhiGridSA[gridIdx], p_uiNumAnchorPoints); 		
	simpleMtrx T = *temp.getMtrxPtr();		
	SVD svdT(T);		
	simpleMtrx pinvT = svdT.pinv();

	//spatial transformation 		
	transformPosition(pPhiGridSA[gridIdx], pThetaGridSA[gridIdx], p_uiNumAnchorPoints);

	//create transformed point grid 
	sHmodeMtrx temp2(m_uiHoaOrder, m_thetaGridWarped, m_phiGridWarped, p_uiNumAnchorPoints);
	simpleMtrx TT = *temp2.getMtrxPtr();

	//4. create preliminary transform p_mOutD
	m_fxMatrix = TT * pinvT;

	//5. compute loudnenss adaptation	
	DFx = m_Dmatrix * TT;
	intensity(DFx, m_uiNumSatellites, p_uiNumAnchorPoints, gain_Num);		

	DFx = m_Dmatrix * m_fxMatrix * T;
	intensity(DFx, m_uiNumSatellites, p_uiNumAnchorPoints, gain_Denum);

	A.zeros();		
	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{
		A[idx][idx] = sqrt(gain_Num[idx] / gain_Denum[idx]); //TODO: check for divide by zero?
	}

	//create final transform matrix
	m_fxMatrix = TT * A * pinvT;

	//adapt rendering matrix
	m_Dmatrix = m_Dmatrix * m_fxMatrix;		

	//extend matrix with existing subwoofer channels 	
	unsigned int localSpkIdx = 0;
	for (unsigned int idxSpk=0; idxSpk<m_uiNumSpeakers; idxSpk++) 
	{   
		if (p_mInD.getSignaledSpeakerType()[idxSpk] == 0)
		{
			for (unsigned int idx=0; idx<m_uiNumCoeffs; idx++)  
			{
				outputD[idxSpk][idx] = m_Dmatrix[localSpkIdx][idx];						
			}				
			localSpkIdx++;
		}
	}
	//return final matrix
	*(p_mOutD.getMtrxPtr()) = outputD;
}	


void scrnAdapt::transformPositionSimple(const double *pPhiGridIn, const double *pThetaGridIn, const unsigned int p_uiNumAnchorPoints)
{  
	double offset = PI; 
	double thisGridAngle;
	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{
		thisGridAngle = pPhiGridIn[idx];

		if (thisGridAngle < m_productionScreen_lrud[1])
		{
			m_phiGridWarped[idx] = (m_localScreen_lrud[1]+offset)/(m_productionScreen_lrud[1]+offset) * (thisGridAngle+offset) - offset;
		}
		else if (thisGridAngle < m_productionScreen_lrud[0])
		{
			m_phiGridWarped[idx] = (m_localScreen_lrud[0]-m_localScreen_lrud[1]) / (m_productionScreen_lrud[0]-m_productionScreen_lrud[1]) * (thisGridAngle-m_productionScreen_lrud[1]) + m_localScreen_lrud[1];
		}
		else
		{
			m_phiGridWarped[idx] = (offset - m_localScreen_lrud[0]) / (offset - m_productionScreen_lrud[0])*(thisGridAngle-m_productionScreen_lrud[0]) + m_localScreen_lrud[0];
		}		
	}

	// elevation
	offset = PI*0.5;
	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{
		thisGridAngle = -pThetaGridIn[idx]+offset;		

		if (thisGridAngle < m_productionScreen_lrud[3])
		{
			m_thetaGridWarped[idx] = PI-(m_localScreen_lrud[3]+offset)/(m_productionScreen_lrud[3]+offset) * (thisGridAngle+offset);
		}
		else if (thisGridAngle < m_productionScreen_lrud[2])
		{
			m_thetaGridWarped[idx] = (m_localScreen_lrud[2]-m_localScreen_lrud[3]) / (m_productionScreen_lrud[2]-m_productionScreen_lrud[3]) * (thisGridAngle-m_productionScreen_lrud[3]) + m_localScreen_lrud[3];
			m_thetaGridWarped[idx] = offset-m_thetaGridWarped[idx];	
		}
		else
		{
			m_thetaGridWarped[idx] = (offset - m_localScreen_lrud[2]) / (offset - m_productionScreen_lrud[2])*(thisGridAngle-m_productionScreen_lrud[2]) + m_localScreen_lrud[2];
			m_thetaGridWarped[idx] = offset-m_thetaGridWarped[idx];	
		}			
	}
}


void scrnAdapt::transformPosition(const double *pPhiGridIn, const double *pThetaGridIn, const unsigned int p_uiNumAnchorPoints)
{   
	static const double TWO_PI = 2*PI; 	
	double angle_temp;
	double temp;
	
	//Azimuth

	// introduction of a temporary reproduction screen 
	double reproduction_screen_offset = (m_localScreen_lrud[0] + m_localScreen_lrud[1])/2;

	if (m_localScreen_lrud[1] > m_localScreen_lrud[0])
	{
		reproduction_screen_offset += PI;
	} 

	double reproduction_screen_right_temp = m_localScreen_lrud[1] - reproduction_screen_offset;
	double reproduction_screen_left_temp  = m_localScreen_lrud[0] - reproduction_screen_offset;

	if (reproduction_screen_left_temp > PI)
	{
		reproduction_screen_left_temp -= TWO_PI;
	}

	if (reproduction_screen_left_temp < -PI)
	{
		reproduction_screen_left_temp += TWO_PI;
	}

	if (reproduction_screen_right_temp > PI)
	{
		reproduction_screen_right_temp -= TWO_PI;
	}

	if (reproduction_screen_right_temp < -PI)
	{
		reproduction_screen_right_temp += TWO_PI;
	}

	// introduction of a temporary reference screen 
	double reference_screen_offset = (m_productionScreen_lrud[0] + m_productionScreen_lrud[1])/2;

	if (m_productionScreen_lrud[1] > m_productionScreen_lrud[0])
	{
		reference_screen_offset += PI; 
	}

	double reference_screen_right_temp = m_productionScreen_lrud[1] - reference_screen_offset;
	double reference_screen_left_temp = m_productionScreen_lrud[0] - reference_screen_offset;

	if (reference_screen_left_temp > PI)
	{
		reference_screen_left_temp -= TWO_PI;
	}
	if (reference_screen_left_temp < -PI)
	{
		reference_screen_left_temp += TWO_PI;
	}

	if (reference_screen_right_temp > PI)
	{
		reference_screen_right_temp -= TWO_PI;
	}

	if (reference_screen_right_temp < -PI)
	{
		reference_screen_right_temp += TWO_PI;
	}

	//calculation of screen width ratio 
	double widthRatio = abs(reproduction_screen_left_temp - reproduction_screen_right_temp) / abs(reference_screen_left_temp - reference_screen_right_temp);
	

	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{   		
		// "rotation" of the curve along the input axis (reference screen axis)
		angle_temp = pPhiGridIn[idx] - reference_screen_offset;
		if (angle_temp > PI)
		{
			angle_temp -= TWO_PI;
		} 

		if (angle_temp < -PI)
		{
			angle_temp += TWO_PI;
		}  

		//REMAPPING  
		if ((angle_temp >= -PI) && (angle_temp < reference_screen_right_temp))
		{
			temp = (reproduction_screen_right_temp + PI)/(reference_screen_right_temp + PI);
			m_phiGridWarped[idx] = temp * (angle_temp + PI) -PI;
		}
		else if ((angle_temp >= reference_screen_right_temp) && (angle_temp < reference_screen_left_temp))
		{			
			m_phiGridWarped[idx] = widthRatio * (angle_temp - reference_screen_right_temp) + reproduction_screen_right_temp;
		}
		else if ((angle_temp <= PI) && (angle_temp >= reference_screen_left_temp))
		{
			temp = (PI - reproduction_screen_left_temp)/(PI - reference_screen_left_temp);
			m_phiGridWarped[idx] = temp * (angle_temp - reference_screen_left_temp) + reproduction_screen_left_temp;
		}		

		// "rotation" of the curve along the output axis (reproduction screen axis)
		m_phiGridWarped[idx] += reproduction_screen_offset;
		if (m_phiGridWarped[idx] > PI)
		{
			m_phiGridWarped[idx] -= TWO_PI;
		}
		if (m_phiGridWarped[idx] < -PI) 
		{
			m_phiGridWarped[idx] += TWO_PI;
		}
	}


	// elevation	
	static const double offset = PI*0.5;
	static const double heightRatio = abs(m_localScreen_lrud[2]-m_localScreen_lrud[3]) / abs(m_productionScreen_lrud[2]-m_productionScreen_lrud[3]);

	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{
		angle_temp = -pThetaGridIn[idx]+offset;//(PI*0.5);	

		if (angle_temp < m_productionScreen_lrud[3])
		{
			m_thetaGridWarped[idx] = PI-(m_localScreen_lrud[3]+offset)/(m_productionScreen_lrud[3]+offset) * (angle_temp+offset);
		}
		else if (angle_temp < m_productionScreen_lrud[2])
		{
			m_thetaGridWarped[idx] = heightRatio * (angle_temp-m_productionScreen_lrud[3]) + m_localScreen_lrud[3];
			m_thetaGridWarped[idx] = offset-m_thetaGridWarped[idx];	
		}
		else
		{
			m_thetaGridWarped[idx] = (offset - m_localScreen_lrud[2]) / (offset - m_productionScreen_lrud[2])*(angle_temp-m_productionScreen_lrud[2]) + m_localScreen_lrud[2];
			m_thetaGridWarped[idx] = offset-m_thetaGridWarped[idx];	
		}			
	}
}

void scrnAdapt::intensity(simpleMtrx Mtx, const unsigned int p_uiNumSpeakers, const unsigned int p_uiNumAnchorPoints, std::vector<double> &gain)
{  		
	for (unsigned int idx=0; idx< p_uiNumAnchorPoints; idx++)
	{	
		gain[idx] = 0.0;
		for (unsigned int idxSpk=0; idxSpk < p_uiNumSpeakers; idxSpk++) 
		{
			gain[idx] += Mtx[idxSpk][idx]*Mtx[idxSpk][idx];
		}

	}
}
