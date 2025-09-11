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

#include "iarFormatConverter_activeDmx_stft.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define SAMSUNG 1

int activeDmxStftInit(  void **handle,
                        int numInChans, 
                        int numOutChans, 
                        float ***dmxMtx,
#if IAR
                        float ***dmxMtx2,
#endif
                        int        frameSize,
                        int        fftLength,
                        float      eqLimitMax,
                        float      eqLimitMin,
                        int        numStftBands,
                        int        numErbBands,
                        int        *stftErbFreqIdx
                        )
{
    activeDownmixer *h;

    if((numInChans>IAR_MAX_CHANNELS)||(numOutChans>IAR_MAX_CHANNELS)) {
        return -1;
    }

    *handle = calloc(1, sizeof(activeDownmixer));
    
    h = (activeDownmixer*)(*handle);
    
    
    h->numInChans         = numInChans;
    h->numOutChans        = numOutChans;
    h->numErbBands        = numErbBands;
    h->frameSize          = frameSize;
    h->fftLength          = fftLength;
    h->numStftBands       = numStftBands;
    h->eqLimitMax         = eqLimitMax;
    h->eqLimitMin         = eqLimitMin;
    h->eneSmoothingAlpha  = 0.0435f;
    h->epsilon            = 1e-35f;
    h->erbFreqIdx         = stftErbFreqIdx;


    {
        int chIn, chOut;
        h->dmxMtx = (float***) malloc(h->numOutChans*sizeof (float**));
#if IAR    
        h->dmxMtx2 = (float***) malloc(h->numOutChans*sizeof (float**));
#endif
        for(chOut=0;chOut<h->numOutChans;chOut++)
        {
            h->dmxMtx[chOut] = (float**) malloc(h->numInChans*sizeof (float*));
#if IAR    
            h->dmxMtx2[chOut] = (float**) malloc(h->numInChans*sizeof (float*));
#endif
            
            for(chIn=0;chIn<h->numInChans;chIn++)
            {
                h->dmxMtx[chOut][chIn] = (float*) calloc(h->numStftBands, sizeof (float));
#if IAR    
                h->dmxMtx2[chOut][chIn] = (float*) calloc(h->numStftBands, sizeof (float));
#endif
            }
        }
    }


#if IAR     /* allocate delay buffer: prevInFrame[chIn][STFTbin] */
   {
        int chIn;
        h->prevInFrame = (float**) calloc(h->numInChans, sizeof (float*));
        for(chIn=0;chIn<h->numInChans;chIn++)
        {
            h->prevInFrame[chIn] = (float*) calloc(fftLength, sizeof (float));
        }
    }
#endif


#if IAR    
    activeDmxSetDmxMtx_STFT(h, (const float ***)dmxMtx, (const float ***)dmxMtx2);
#else
    activeDmxSetDmxMtx_STFT(h, (const float ***)dmxMtx);
#endif

    /* Gather EQ amplitude limits */
    h->eqLimitMax = (float)pow(10.0f,eqLimitMax/20.0f);
    h->eqLimitMin = (float)pow(10.0f,eqLimitMin/20.0f);


    activeDmxSetAES(7, h);

    return 0;
}

/*****************************************************************************************************************/

/*****************************************************************************************************************/
/*****************************************************************************************************************/
#if IAR    
void activeDmxSetDmxMtx_STFT( activeDownmixer    *h, 
                              const float        ***dmxMtx,
                              const float        ***dmxMtx2)
#else
void activeDmxSetDmxMtx_STFT( activeDownmixer    *h, 
                              const float        ***dmxMtx)
#endif
{
    int chA, chB,band;

    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            for(band=0;band<h->numStftBands;band++)
            {
                h->dmxMtx[chA][chB][band]=dmxMtx[band][chB][chA];
#if IAR    
                h->dmxMtx2[chA][chB][band]=dmxMtx2[band][chB][chA];
#endif
            }
        }
    }   
}
/*****************************************************************************************************************/


/*****************************************************************************************************************/
void activeDmxClose_STFT(void *handle)
{
    activeDownmixer *h;
    int chIn, chOut;

    h = (activeDownmixer*)(handle);

    if(h == NULL) /* nothing to free */
        return;

#if IAR     /* free delay buffer: prevInFrame[chIn][STFTbin] */
   {
        int chIn;
        for(chIn=0;chIn<h->numInChans;chIn++)
        {
            free(h->prevInFrame[chIn]);
        }
        free(h->prevInFrame);
    }
#endif

    for(chOut=0;chOut<h->numOutChans;chOut++)
    {
        for(chIn=0;chIn<h->numInChans;chIn++)
        {
            free(h->dmxMtx[chOut][chIn]);
#if IAR    
            free(h->dmxMtx2[chOut][chIn]);
#endif
        }
        free(h->dmxMtx[chOut]);
#if IAR    
        free(h->dmxMtx2[chOut]);
#endif
    }
    free(h->dmxMtx);
#if IAR    
    free(h->dmxMtx2);
#endif

    free(h);
}

/*****************************************************************************************************************/
#if IAR
void activeDmxProcess_STFT(
                            void         *handle,
                            const float  **inputBufferStft,
                            float        **outputBufferStft,
                            const int    rendering3DType)
{
   activeDmxProcess_STFT3D(handle,
                           inputBufferStft,
                           outputBufferStft,
                           rendering3DType);
}
void activeDmxProcess_STFT3D(
                            void         *handle,
                            const float  **inputBufferStft, 
                            float        **outputBufferStft,
                            const int    rendering3DType)
#else
void activeDmxProcess_STFT(
                            void        *handle,
                            const float ** inputBufferStft,
                            float       **outputBufferStft)
#endif
{
    
    activeDownmixer* h = (activeDownmixer*)handle;
    unsigned int bandRe, bandIm;
    int erb, chIn, chOut, fftBand, fftBandStop;
    float EQ;
    float **realizedSig = outputBufferStft;
    float tmpSig;
    float ***dmxMtxCur;
    int r3t;
    const float **inBufStftProc;

    /*********************************************/
    /*                Clear Buffer                 */
    /*********************************************/

    for(chOut=0;chOut<h->numOutChans;chOut++)
    {
        for(fftBand=0;fftBand<h->fftLength;fftBand++)
        {
            realizedSig[chOut][fftBand] = 0.0f;
        }

        for(erb=0;erb<h->numErbBands;erb++)
        {
            h->targetEne[chOut][erb] = 0.0f;
            h->realizedEne[chOut][erb] = 0.0f;
        }
    }

    /*********************************************/
    /* Compute target energy and realized signal */
    /*********************************************/

    r3t = R3T;
    dmxMtxCur= h->dmxMtx; /* default, equivalent to R3T = 1 define in iar_rom.h */
#if IAR
    if ( rendering3DType != 1 )
    {
        r3t = 0;
        dmxMtxCur= h->dmxMtx2;
    }
#endif

    for(chOut=0;chOut<h->numOutChans;chOut++)
    {
        for(chIn=0;chIn<h->numInChans;chIn++)
        {
            if  ( ( chIn == topIn[TFL] || chIn == topIn[TFC] || chIn == topIn[TFR] || chIn == topIn[TFLA] || chIn == topIn[TFRA] ) && midOut[chOut] > SW && r3t == 1 )
                inBufStftProc = (const float **) h->prevInFrame;  /* delayed input signal (1 HOP delay) */
            else
                inBufStftProc = inputBufferStft; /* undelayed input signal */
#if SAMSUNG
            if (dmxMtxCur[chOut][chIn][h->erbFreqIdx[31]])
#else
            if (dmxMtxCur[chOut][chIn][0])
#endif
            {
                tmpSig                    = dmxMtxCur[chOut][chIn][0] * inBufStftProc[chIn][0];
                realizedSig[chOut][0]     += tmpSig;
                h->targetEne[chOut][0]    += tmpSig*tmpSig;

                tmpSig                    = dmxMtxCur[chOut][chIn][h->numStftBands-1] * inBufStftProc[chIn][1];
                realizedSig[chOut][1]                    += tmpSig;
                h->targetEne[chOut][h->numErbBands-1]    += tmpSig*tmpSig;

                fftBand = 1;
                for(erb=0;erb<h->numErbBands;erb++)
                {
                    fftBandStop = h->erbFreqIdx[erb];
                    if(erb == (h->numErbBands-1))
                        fftBandStop -=1;

                    while(fftBand < fftBandStop){
                        bandRe = fftBand*2;
                        bandIm = fftBand*2+1;

                        tmpSig = dmxMtxCur[chOut][chIn][fftBand] * inBufStftProc[chIn][bandRe];
                        realizedSig[chOut][bandRe]        += tmpSig;
                        h->targetEne[chOut][erb]          += tmpSig * tmpSig;

                        tmpSig = dmxMtxCur[chOut][chIn][fftBand] * inBufStftProc[chIn][bandIm];
                        realizedSig[chOut][bandIm]        += tmpSig;
                        h->targetEne[chOut][erb]          += tmpSig * tmpSig;

                        fftBand++;
                    }
                }
            }
        } 
    }


    /*********************************************/
    /*            Update delay buffer            */
    /*********************************************/
    for(chIn=0;chIn<h->numInChans;chIn++)
        for(fftBand=0;fftBand<h->fftLength;fftBand++)
            h->prevInFrame[chIn][fftBand] = inputBufferStft[chIn][fftBand];


    /*********************************************/
    /*          Compute realized energy          */
    /*********************************************/
    for(chOut=0;chOut<h->numOutChans;chOut++)
    {

        h->realizedEne[chOut][0]                   = realizedSig[chOut][0] * realizedSig[chOut][0];
        h->realizedEne[chOut][h->numErbBands-1]    = realizedSig[chOut][1] * realizedSig[chOut][1];

        fftBand = 1;
        for(erb=0;erb<h->numErbBands;erb++)
        {
            fftBandStop = h->erbFreqIdx[erb];
            if(erb == (h->numErbBands-1))
                fftBandStop -=1;

            while(fftBand < fftBandStop){
                bandRe = fftBand*2;
                bandIm = fftBand*2+1;

                h->realizedEne[chOut][erb] += realizedSig[chOut][bandRe] * realizedSig[chOut][bandRe];
                h->realizedEne[chOut][erb] += realizedSig[chOut][bandIm] * realizedSig[chOut][bandIm];

                fftBand++;
            }
        }
    }


    /*********************************************/
    /*    Energy smoothing and buffer update     */
    /*********************************************/
    for(chOut=0;chOut<h->numOutChans;chOut++)
    {
        for(erb=0;erb<h->numErbBands;erb++)
        {
            /* energy smoothing*/
            h->targetEne[chOut][erb]    =    h->eneSmoothingAlpha * h->targetEne[chOut][erb]      + (1-h->eneSmoothingAlpha) * h->targetEnePrev[chOut][erb];
            h->realizedEne[chOut][erb]  =    h->eneSmoothingAlpha * h->realizedEne[chOut][erb]    + (1-h->eneSmoothingAlpha) * h->realizedEnePrev[chOut][erb];

            /* save current energies for smoothing in next frame */
            h->targetEnePrev[chOut][erb]    = h->targetEne[chOut][erb];
            h->realizedEnePrev[chOut][erb]  = h->realizedEne[chOut][erb];
        }
    }



    /*********************************************/
    /*          Compute and apply EQ             */
    /*********************************************/


    for(chOut=0;chOut<h->numOutChans;chOut++)
    {
        /* DC real coefficient */
        computeEQAndClip(    handle, &h->targetEne[chOut][0], &h->realizedEne[chOut][0], &EQ);
        realizedSig[chOut][0]                *= EQ ;
        
        /* real coefficient at nFFT/2+1*/
        computeEQAndClip(handle, &h->targetEne[chOut][h->numErbBands-1], &h->realizedEne[chOut][h->numErbBands-1],&EQ);
        realizedSig[chOut][1]                *= EQ;
        
        fftBand = 1;            
        for(erb=0;erb<h->numErbBands;erb++)
        {
            computeEQAndClip(handle, &h->targetEne[chOut][erb], &h->realizedEne[chOut][erb], &EQ);

            fftBandStop = h->erbFreqIdx[erb];
            if(erb == (h->numErbBands-1))
                fftBandStop -=1;

            while(fftBand < fftBandStop){
                bandRe = fftBand*2;
                bandIm = fftBand*2+1;
                realizedSig[chOut][bandRe] *= EQ;
                realizedSig[chOut][bandIm] *= EQ;
                fftBand++;
            }
        }
    }


}


/*****************************************************************************************************************/

int activeDmxSetAES(int AES, void *handle)
{
    activeDownmixer* h = (activeDownmixer*) handle;
    h->floatAES = ((float)AES)/(7.f);
    return 0;
}
/*****************************************************************************************************************/

void computeEQAndClip(
                        void  *handle, 
                        float *targetEne, 
                        float *realizedEne, 
                        float *EQ)
{
    activeDownmixer* h = (activeDownmixer*) handle;

    *EQ = (float)sqrt(*targetEne/(h->epsilon + *realizedEne));
    if (*EQ > h->eqLimitMax)
    {
        *EQ = h->eqLimitMax;
    }

    if (*EQ < h->eqLimitMin)
    {
        *EQ = h->eqLimitMin;
    }
    
    *EQ = h->floatAES * *EQ + (1.0f - h->floatAES);
}
