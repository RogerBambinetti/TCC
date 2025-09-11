/***********************************************************************************
 
 This software module was originally developed by
 
 Orange Labs
 
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
 
 Orange Labs retains full right to modify
 and use the code for its own purpose, assign or donate the code to a third party and
 to inhibit third parties from using the code for products that do not conform to
 MPEG-related ITU Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include "hoaNfcFiltering.h"

double tab_NFroots_ReAbs[] = {
	-2.000000000000, // m=1
	-3.000000000000, 3.464101615138, // m=2
	-3.677814645374, 5.083082802191, -4.644370709252, // m=3
	-4.207578794359, 6.778731585443, -5.792421205641, 6.046529877633, // m=4
	-4.649348606363, 8.522045602660, -6.703912798307, 7.555787321856, -7.293477190659, // m=5
	-5.031864495622, 10.298354304270, -7.471416712652, 9.132978304452, -8.496718791727, 8.672054102592, // m=6
	-5.371353757887, 12.099055360957, -8.140278327276, 10.758540066991, -9.516581056309, 10.132412299698, -9.943573717056, // m=7
	-5.677967897795, 13.918623301554, -8.736578434405, 12.420829807239, -10.409681581274, 11.650706430973, -11.175772086526, 11.309681738808 // m=8
};

// class functions
void HoaNfcFilter::SetNfcParameters(FLOAT tNFC, FLOAT tNFS)
{
	m_nfcTime = tNFC;
	m_nfsTime = tNFS;

	int ind1 = (m_order*(m_order-1))/2;
	FLOAT 
		alpha_den = 4*m_nfcTime, 
		alpha_num = 4*m_nfsTime; 
	double a0, a1, a2, b0, b1, b2, ReX, AbsX, tmp, coeff;

//	int ic = 0;
	coeff = 1.f; // -> m_globalGain

	for (int n=0; n<m_nbCellsO2; n++) {
		ReX = tab_NFroots_ReAbs[ind1+2*n];
		AbsX = tab_NFroots_ReAbs[ind1+2*n+1];
		// 2nd order cell #n
		// denominator :
		tmp = AbsX/alpha_den; tmp *= tmp;
		a1 = 2*(tmp-1);
		tmp += 1;
		a0 = -2*ReX/alpha_den;
		a2 = -a0+tmp;
		a0 += tmp;
		// numerator :
		tmp = AbsX/alpha_num; tmp *= tmp;
		b1 = 2*(tmp-1);
		tmp += 1;
		b0 = -2*ReX/alpha_num;
		b2 = -b0+tmp;
		b0 += tmp;

		coeff *= b0/a0;

		m_CellO2[n].SetCoeff(1, (FLOAT)(b1/b0), (FLOAT)(b2/b0), 1, (FLOAT)(a1/a0), (FLOAT)(a2/a0));
	}
	if (m_nbCellsO1) {
		ReX = tab_NFroots_ReAbs[ind1+m_order-1];
		// 1st order cell at the end for odd order
		// denominator :
		tmp = ReX/alpha_den;
		a0 = 1-tmp;
		a1 = -(1+tmp);
		// numerator :
		tmp = ReX/alpha_num;
		b0 = 1-tmp;
		b1 = -(1+tmp);

		coeff *= b0/a0;
		m_CellO1.SetCoeff(1, (FLOAT)(b1/b0), 1, (FLOAT)(a1/a0));
	}

	m_globalGain = (FLOAT)coeff;

}


void HoaNfcFilter::DoFiltering(FLOAT *x, FLOAT *y, int L)
{
	// copy first to do systematic in-place filtering (for writing convenience)
	// by the way, apply global gain
	if (y!=x || m_globalGain!=1.0)
	{
		for (int i=0; i<L; i++)
			y[i] = m_globalGain*x[i];
	}
	for (int n=0; n<m_nbCellsO2; n++)
		m_CellO2[n].DoFiltering(y, y, L);
	if (m_nbCellsO1)
		m_CellO1.DoFiltering(y, y, L);

}
