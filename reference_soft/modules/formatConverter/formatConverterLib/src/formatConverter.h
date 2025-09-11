/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
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
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef _FORMATCONVERTER_H_
#define _FORMATCONVERTER_H_


#include "formatConverter_api.h"
#include "formatConverter_init.h"

#ifdef FORMATCONVERTER_LOWCOMPLEXITY
#include "formatConverter_phaseAlign_lowcplx.h"
#else
#include "formatConverter_phaseAlign.h"
#endif

#include "formatConverter_activeDmx_stft.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288f
#endif

/* default parameters */
#define FORMAT_CONVERTER_MAX_CHANNELS 24   
    
/**********************************************************************************************************************************/

typedef struct T_FORMAT_CONVERTER_PARAMS
{
    
    /** Sampling rate. */    
    int     samplingRate;
    
    
    /** Format converter processing mode */
    FORMAT_CONVERTER_MODE formatConverterMode;

    /** Needs to be set, if the FORMAT_CONVERTER_MODE == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN */
    float   phaseAlignStrength;
    float   adaptiveEQStrength;
    
    
    /** Format converter input format */
    FORMAT_CONVERTER_INPUT_FORMAT formatConverterInputFormat;
    
    /** Format converter output format */
    FORMAT_CONVERTER_OUTPUT_FORMAT formatConverterOutputFormat;
    
    
    /** format converter input format of internal init structure */
    converter_formatid_t formatConverterInputFormat_internal;
    
    /** format converter output format of internal init structure */
    converter_formatid_t formatConverterOutputFormat_internal;
    
    
    /** handle to internal init structure*/
    converter_pr_t *formatConverterParams_internal;
    
    /** flag signalling generic input,output formats */    
    int genericIOFmt;

    /** numInputChans */
    int numInputChans;
    
    /** numOutputChans */
    int numOutputChans;
       
    /** dmx matrix */
    float **dmxMtx;
    
    /** dmx matrix freq dependent */
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
    float ***dmxMtxFreq;
#endif

    /** flag signalling the DMX matrix coefficients have been set */
    int dmxMtxIsSet;
    
    /** mix matrix */
    int **mixMtx;
        
    /** gain trim [lin] */
    float *trimGains;
    
    /** delay trim [samples] */    
    int *trimDelays;
        
    
    /** formatConverterFrameSize */
    int formatConverterFrameSize;
    
    /** formatConverterDelay */
    int formatConverterDelay;
    
    
    /** numFreqBands */
    int numFreqBands;

    
    /** number stft processing bands */
    int stftNumErbBands;
    
    /** stft processing bands */
    int *stftErbFreqIdx;

    /** stft length */
    int stftLength;
    
    /** stft length */
    int stftFrameSize;

    /** stft input buffer */
    float ** inputBufferStft;
    
    /** stft output buffer */
    float ** outputBufferStft;

    /** center frequencies */
    float *centerFrequenciesNormalized;
    
    
    /** azimuthElevationDeviation */
    float *azimuthElevationDeviation;

    /** distance*/
    float *distance;
    
    
    /** eqLimitMax */
    float eqLimitMax;
    
    /** eqLimitMin */
    float eqLimitMin;
    
    
    /** applyEqFilters */
    int applyEqFilters;
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    /* eqControlMatrix */
    int **eqControlMatrix;
    
    /* eqControlMatrixSize */
    int eqControlMatrixSize;
#endif
    
    /* randomFlag */
    int randomFlag;
    
    /* trimFlag */    
    int trimFlag;
    
    /* trim_max */
    int trim_max;  
    
    /* gainCompensationControlMatrix*/
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    int *gainCompensationControlMatrix;
    int gainCompensationControlMatrixSize;
    float gainCompensationValue;
#endif
    
} FORMAT_CONVERTER_PARAMS;

/**********************************************************************************************************************************/

typedef struct T_FORMAT_CONVERTER_STATE
{
    
    /** handle for params and states of phase alignment dmx */
    void *handlePhaseAligner;
    
    /** handle for params and states of active dmx in stft domain */
    void *handleActiveDmxStft;  
    
    /** inputBufferFiltered_real */
    float **inputBufferFiltered_real;
    
    /** inputBufferFiltered_imag */
    float **inputBufferFiltered_imag;
    
    
} FORMAT_CONVERTER_STATE;

/***********************************************************************************************************/
/** internal functions **/
/***********************************************************************************************************/

int formatConverterGetNumInputChans( const FORMAT_CONVERTER_INPUT_FORMAT formatConverterInputFormat );                                    
    
/***********************************************************************************************************/

int formatConverterGetNumOutputChans( const FORMAT_CONVERTER_OUTPUT_FORMAT formatConverterOutputFormat );                                

/***********************************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


