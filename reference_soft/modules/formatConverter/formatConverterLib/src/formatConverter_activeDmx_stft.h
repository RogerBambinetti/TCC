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
 
 Copyright (c) ISO/IEC 2015.
 
 ***********************************************************************************/

#include "formatConverter_phaseAlign_lowcplx.h"
 /*#include "formatConverter_constants_stft.h"*/

#ifndef _FORMATCONVERTER_ACTIVE_DMX_STFT_H_
#define _FORMATCONVERTER_ACTIVE_DMX_STFT_H_

#define MAX_ERB_BANDS            (58)


typedef struct {
    int        numInChans;                                        /* number input channels */
    int        numOutChans;                                    /* number output channels */
    int        numStftBands;                                    /* number of fft bands */
    int        numErbBands;                                    /* number of ERB bands */
    int        *erbFreqIdx;                                    /* frequency band stop indices corresponding to ERB bands */
    int        frameSize;                                        /* frame size*/
    int        fftLength;                                        /* fft transform length*/
    float   ***dmxMtx;                                        /* frequency dependent downmix matrix */
    float    eqLimitMax;
    float    eqLimitMin;
    float    floatAES;
    float    targetEnePrev[MAX_CHANNELS][MAX_ERB_BANDS];
    float    targetEne[MAX_CHANNELS][MAX_ERB_BANDS];
    float    realizedEnePrev[MAX_CHANNELS][MAX_ERB_BANDS];
    float    realizedEne[MAX_CHANNELS][MAX_ERB_BANDS];
    float    eneSmoothingAlpha;
    float    epsilon;
} activeDownmixer;

/**********************************************************************************************/
void activeDmxProcess_STFT(    
                            void        *handle,
                            const float ** inputBufferStft, 
                            float        **outputBufferStft);
/**********************************************************************************************/
int activeDmxStftInit(        void    **handle,
                            int        numInChans, 
                            int        numOutChans, 
                            float    ***dmxMtx,
                            int        frameSize,
                            int        fftLength,
                            float    eqLimitMax,
                            float    eqLimitMin,
                            int        numStftBands,
                            int        numErbBands,
                            int        *stftErbFreqIdx
                            );
/**********************************************************************************************/
void activeDmxClose_STFT(void *handle);
/**********************************************************************************************/
void formulateDmxMatrixNoPhaseAlign_STFT(void *handle);
/**********************************************************************************************/
void activeDmxSetDmxMtx_STFT( activeDownmixer *h, const float ***dmxMtx);
/**********************************************************************************************/
int  activeDmxSetAES(int AES, void *handle);
/**********************************************************************************************/
void computeEQAndClip(void *handle, float * targetEne, float *realizedEne, float * EQ);
/**********************************************************************************************/

#endif /*_FORMATCONVERTER_ACTIVE_DMX_STFT_H_*/

