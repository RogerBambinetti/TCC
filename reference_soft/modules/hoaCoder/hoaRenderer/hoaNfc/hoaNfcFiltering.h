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

#ifndef __HOANFCFILTERING_H__
#define __HOANFCFILTERING_H__

#ifndef FLOAT
 #define FLOAT double
#endif

class IIRSecondOrderCell
{
public:
	IIRSecondOrderCell()
	{
		m_a0 = 1; m_a1 = m_a2 = 0;
		m_b0 = 1; m_b1 = m_b2 = 0;
		m_mem_x1 = m_mem_x2 = m_mem_y1 = m_mem_y2 = 0;
	}
	IIRSecondOrderCell(IIRSecondOrderCell &fromCell)
	{
		m_a0 = fromCell.m_a0;
		m_a1 = fromCell.m_a1;
		m_a2 = fromCell.m_a2;
		m_b0 = fromCell.m_b0;
		m_b1 = fromCell.m_b1;
		m_b2 = fromCell.m_b2;
		m_mem_x1 = m_mem_x2 = m_mem_y1 = m_mem_y2 = 0;
	}
	virtual ~IIRSecondOrderCell() {};
	void SetCoeff(FLOAT b0, FLOAT b1, FLOAT b2, FLOAT a0, FLOAT a1, FLOAT a2)
	{
		m_a0 = a0;
		m_a1 = a1;
		m_a2 = a2;
		m_b0 = b0;
		m_b1 = b1;
		m_b2 = b2;
	}
	void ResetMemory() 
	{
		m_mem_x1 = m_mem_x2 = m_mem_y1 = m_mem_y2 = 0;
	}
	void DoFiltering(FLOAT *x, FLOAT *y, int L)
	{
		// NB: we assume m_a0 = 1;
		FLOAT x0; // temp variable to allow in-place filtering
		for (int i=0; i<L; i++)
		{
			x0 = x[i];
			y[i] = m_b0*x[i] + m_b1*m_mem_x1 + m_b2*m_mem_x2 - m_a1*m_mem_y1 - m_a2*m_mem_y2;
			m_mem_x2 = m_mem_x1;
			m_mem_x1 = x0;
			m_mem_y2 = m_mem_y1;
			m_mem_y1 = y[i];
		}
	}
private:
	FLOAT m_a0, m_a1, m_a2, m_b0, m_b1, m_b2;
	FLOAT m_mem_x1, m_mem_x2, m_mem_y1, m_mem_y2;
};

class IIRFirstOrderCell
{
public:
	IIRFirstOrderCell(IIRFirstOrderCell &fromCell)
	{
		m_a0 = fromCell.m_a0;
		m_a1 = fromCell.m_a1;
		m_b0 = fromCell.m_b0;
		m_b1 = fromCell.m_b1;
		m_mem_x1 = m_mem_y1 = 0;
	}
	IIRFirstOrderCell()
	{
		m_a0 = 1; m_a1 = 0;
		m_b0 = 1; m_b1 = 0;
		m_mem_x1 = m_mem_y1 = 0;
	}
	~IIRFirstOrderCell() {}
	void SetCoeff(FLOAT b0, FLOAT b1, FLOAT a0, FLOAT a1)
	{
		m_a0 = a0;
		m_a1 = a1;
		m_b0 = b0;
		m_b1 = b1;
	}
	void ResetMemory() 
	{
		m_mem_x1 = m_mem_y1 = 0;
	}
	void DoFiltering(FLOAT *x, FLOAT *y, int L)
	{
		// NB: we assume m_a0 = 1;
		FLOAT x0; // temp variable to allow in-place filtering
		for (int i=0; i<L; i++)
		{
			x0 = x[i];
			y[i] = m_b0*x[i] + m_b1*m_mem_x1 - m_a1*m_mem_y1;
			m_mem_x1 = x0;
			m_mem_y1 = y[i];
		}
	}
private:
	FLOAT m_a0, m_a1, m_b0, m_b1;
	FLOAT m_mem_x1, m_mem_y1;
};

class HoaNfcFilter
{
public:
	HoaNfcFilter()
	{
		m_order = 0;
		m_nbCellsO1 = 0;
		m_nbCellsO2 = 0;
		m_CellO2 = 0x0;
		m_globalGain = 1.0;
	}
	HoaNfcFilter(const HoaNfcFilter &originalFilter)
	{
		m_order = originalFilter.m_order;
		m_nbCellsO1 = originalFilter.m_nbCellsO1;
		m_nbCellsO2 = originalFilter.m_nbCellsO2;
		if (m_nbCellsO1)
			m_CellO1 = originalFilter.m_CellO1;
		if (m_nbCellsO2>0)
		{
			m_CellO2 = new IIRSecondOrderCell[m_nbCellsO2];
			for (int c=0; c<m_nbCellsO2; c++)
				m_CellO2[c] = originalFilter.m_CellO2[c];
		}
		else
			m_CellO2 = 0x0;
		m_globalGain = originalFilter.m_globalGain;
	}

	HoaNfcFilter(int order)
	{
		m_order = order;
		m_nbCellsO1 = m_order%2;
		m_nbCellsO2 = (m_order-m_nbCellsO1)/2;
		if (m_nbCellsO2>0) {
			m_CellO2 = new IIRSecondOrderCell[m_nbCellsO2];
		}
		else
			m_CellO2 = 0x0;
		m_globalGain = 1.0;
	}

	HoaNfcFilter(int order, FLOAT tNFC, FLOAT tNFS)
	{
		m_order = order;
		m_nbCellsO1 = m_order%2;
		m_nbCellsO2 = (m_order-m_nbCellsO1)/2;
		if (m_nbCellsO2>0) 
		{
			m_CellO2 = new IIRSecondOrderCell[m_nbCellsO2];
		}
		else
			m_CellO2 = 0x0;
		SetNfcParameters(tNFC, tNFS);
	}

	virtual ~HoaNfcFilter()
	{
		delete [] m_CellO2;
	}

	HoaNfcFilter & operator=(const HoaNfcFilter &originalFilter)
	{
		m_order = originalFilter.m_order;
		m_nbCellsO1 = originalFilter.m_nbCellsO1;
		m_nbCellsO2 = originalFilter.m_nbCellsO2;
		if (m_nbCellsO1)
			m_CellO1 = originalFilter.m_CellO1;
		if (m_nbCellsO2>0)
		{
			if (m_CellO2!=0x0)
				delete m_CellO2;
			m_CellO2 = new IIRSecondOrderCell[m_nbCellsO2];
			for (int c=0; c<m_nbCellsO2; c++)
				m_CellO2[c] = originalFilter.m_CellO2[c];
		}
		else
			m_CellO2 = 0x0;
		m_globalGain = originalFilter.m_globalGain;
		return *this;
	}

	void SetNfcParameters(FLOAT tNFC, FLOAT tNFS);

	void DoFiltering(FLOAT *x, FLOAT *y, int L);

private:
	int m_order;
	int m_nbCellsO2; // = floor(m_order/2)
	int m_nbCellsO1; // = 0 or 1
	IIRSecondOrderCell *m_CellO2;
	IIRFirstOrderCell m_CellO1;
	FLOAT m_globalGain;

	// Before calling SetNfcParameters, distance parameters have to be converted and represented as time values in samples
	// nfc stands for 'near field compensation' (numerator) and nfs stands for 'near field simulation' (denominator)
	// m_nfcTime = nfcRadius/soundSpeed*sampFreq
	// m_nfsTime = nfsRadius/soundSpeed*sampFreq
	// WARNING: 
	//	for spatial decoding, the near field to simulate is the one that has been pre-compensated, i.e. associated to fNfcDistance
	//	              whereas the near field to compensate for is associated to the actual loudspeaker distance

	FLOAT m_nfcTime, m_nfsTime; 
};

#endif
