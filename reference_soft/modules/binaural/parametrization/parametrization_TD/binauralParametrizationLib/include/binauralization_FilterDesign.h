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

#ifndef BINAURALIZATION_FILTERDESIGN_H
#define BINAURALIZATION_FILTERDESIGN_H


#define _BUGFIX_DIRECT_FC_
#define _BUGFIX_DIFFUSE_FC_


#include "bitstreamBinaural.h"


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------- Binaural Filter Design Functions ------------------------------------------------*/



/**
*   Compute parameters for a given BRIR
*
*   @param    pParam		: dest ptr to a TdBinauralRendererParam struct for storing computed params
*   @param    pFirData      : source ptr to BinauralFirData struct
*   @param    nBrirPairs    : number of BRIR pairs in pFirData
*
*   @return    0 if no error, 1 otherwise
*/
int computeFilterParams(
	TdBinauralRendererParam *pParam, 
	BinauralFirData *pFirData, 
	int nBrirPairs);


/**
*   Print filter parameters
*
*   @param    FilterParams *pParam    : ptr to a FilterParams struct 
*
*   @return   void
*/
void printFilterParams(TdBinauralRendererParam *pParam);



/**
*   Compute magnitude of a Fourier Transform (UDH complex FFT format)
*
*   @param    float *pXr_32f    : ptr to real part of the spectrum          (FFTsize_32s points)
*   @param    float *pXr_32f    : ptr to imaginary part of the spectrum     (FFTsize_32s points)
*   @param    float *pMag_32f   : ptr to the magnitude spectrum             (FFTsize_32s points)
*   @param    int   FFTsize_32s : fft size
*
*   @return    void
*/
void FFT_Complex_to_Mag(float *pXr_32f, float	*pXi_32f, float	*pMag_32f, int	FFTsize_32s);



/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
#endif /* BINAURALIZATION_FILTERDESIGN_H */
