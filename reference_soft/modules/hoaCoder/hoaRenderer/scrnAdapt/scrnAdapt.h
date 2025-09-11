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



#ifndef __scrnAdapt_H__
#define __scrnAdapt_H__ 


#include "SimpleMtrx.h"
#include "hoaRenderer.h"
#include <string>
#include <stdexcept>
#include <vector>
#include "HOArenderMtrx.h" 


class scrnAdapt
{
public:   
	// construct from screen information stored in file
	scrnAdapt(const std::string &p_sScreenSizeFile,				 // name of file with screen information
		const unsigned int p_uiNumAnchorPoints,				 // number of Anchor points used for computing the effect matrix
		HOArenderMtrx &p_inD,								 // rendering matrix prior adaptation
		HOArenderMtrx &p_outD);                               // adatepd rendering matrix

	// default, construct an empty object
	scrnAdapt();

private:      
	simpleMtrx m_Dmatrix;			// local storage of non-adapted rendering matrix without LFE channels
	simpleMtrx m_fxMatrix;			// local storage of processed effect matrix (m_uiNumCoeffs x m_uiNumCoeffs)
	unsigned int m_uiHoaOrder;
	unsigned int m_uiNumCoeffs;
	unsigned int m_uiNumSpeakers;
	unsigned int m_uiNumSatellites;
	double m_productionScreen_lrud[4]; // production screen size information [left side azimuth angle, right side azimuth angle, upper side elevation angle, bottom side elevation angle] 
	double m_localScreen_lrud[4];      // reproduction screen size information [left side azimuth angle, right side azimuth angle, upper side elevation angle, bottom side elevation angle] 
	double m_thetaGridWarped[900];		
	double m_phiGridWarped[900];
	void init(HOArenderMtrx &p_mInD, HOArenderMtrx &p_mOutD);
	void process(const unsigned int p_uiNumAnchorPoints, HOArenderMtrx &p_mInD, HOArenderMtrx &p_mOutD);                              
	void transformPositionSimple(const double *pThetaGridIn, const double *pPhiGridIn, const unsigned int p_uiNumAnchorPoints);
	void transformPosition(const double *pThetaGridIn, const double *pPhiGridIn, const unsigned int p_uiNumAnchorPoints);
	void intensity(simpleMtrx Mtx, const unsigned int p_uiNumSpeakers, const unsigned int p_uiNumAnchorPoints,  std::vector<double> &gain);
};

#endif // __scrnAdapt_H__