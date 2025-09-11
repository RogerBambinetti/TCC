/***********************************************************************************

This software module was originally developed by 

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute

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

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#define BUGFIX_KCONV_KMAX_15JUNE

#ifdef  BUGFIX_KCONV_KMAX_15JUNE
#define DEFAULT_KMAX	 48
#define DEFAULT_KCONV    32
#endif

/* Define statement for bug fixing */
#define BUGFIX_BINAURALPARAM
/* #define BUG_BINAURALPARAM_TDL0 */
/* when BUGFIX_BINAURALPARAM is set to a comment(Bug is not fixed) and you want to get parameters for TDL = 0, 
please uncomment BUG_BINAURALPARAM_TDL0. */

#ifdef BUGFIX_BINAURALPARAM
#define BUGFIX_BINAURALPARAM_CURVEFIT
#define BUGFIX_BINAURALPARAM_DIVISION
#endif
/* #define BUGFIX_REVERB_ANA */

#ifndef _BinauralFdParameterization_H

#define _BinauralFdParameterization_H
#include "bitstreamBinaural.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef NULL
#define NULL 0
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef PI
#define PI 3.14159265358979323846f
#endif

	/*--------------------------------*/

#ifndef BrirAnalysisStruct
	typedef struct BrirAnalysisStruct
	{
		float **timedomain_left;
		float **timedomain_right;
		float ***QMFdomain_left_real;
		float ***QMFdomain_right_real;
		float ***QMFdomain_left_imag;
		float ***QMFdomain_right_imag;
		int channels;
		int timedomain_samples;
		int QMF_Timeslots;
		int QMFbands;
		int fs_brirs;
		int numLFEs;
		int *LFE_IND;
		int *transition;
	} BRIRANALYSISSTRUCT, *H_BRIRANALYSISSTRUCT;
#endif

#define BINAURALPARAM_RIFF_HEADER_SIZE 36
#define BINAURALPARAM_NUM_OUTCHANNELS 2
#ifndef QMFLIB_NUMBANDS
#define QMFLIB_NUMBANDS 64
#endif


	float binauralParam_mean(float *data, int start, int l);
	void binauralParam_findmax(float *data, int l, float *max, int *idx);

	void binauralParam_analyzeLR(H_BRIRANALYSISSTRUCT h_BrirSet,  FdBinauralRendererParam *pFdBinauralRendererParam, int end_analysis);
	void binauralParam_applyfilter(double *B, double *A, float *signal, int filterlength, int signallength, float ****signal_bandwise, int position, int channel, int numband);
	void binauralParam_estimate_noisefloor(float ****BRIRs_bandwise, int *startband, int *endband, int pos, int numband, int channel, float *noiselevel, int *endcalc, int minv, int fs);

	int binauralParam_find_last(float *data, float test, int modus, int start, int l_data);
	int binauralParam_find_first(float *data, float test, int modus, int start, int l_data);

	void binauralParam_getRT60(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *RT60);
	void binauralParam_getNRG(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *NRG);
	void binauralParam_compensatedelay(float ****BRIRs_bandwise, int length, int numpos, int numch, int numbands);

	int binauralParam_BRIRtoQMF(H_BRIRANALYSISSTRUCT h_BRIRs, int maxband, int min_length);

	void binauralParam_linearfit(FdBinauralRendererParam *pFdBinauralRendererParam, int x_begin, int x_end, float* MixingTime);
	int binauralParam_createFdBinauralParam(BinauralRepresentation *pBinauralRepresentation, int framesize);
	void binauralParam_PropagationTimeCalc(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam, float ***ptr_timedomain);
	int binauralParam_FilterConversion(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam);
	void binauralParam_VoffParamGeneration(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam, int framesize);
	void binauralParam_QtdlParamGeneration(BRIRANALYSISSTRUCT *h_BrirSet, FdBinauralRendererParam *pFdBinauralRendererParam);
#endif
