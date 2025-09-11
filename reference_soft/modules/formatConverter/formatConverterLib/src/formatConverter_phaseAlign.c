/***********************************************************************************
 
 This software module was originally developed by 
 
 Juha Vilkamo (Aalto University) AND Fraunhofer IIS
 
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
 
 Juha Vilkamo AND Fraunhofer IIS retain full right to modify and use the code for 
 their own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef FORMATCONVERTER_LOWCOMPLEXITY

#include <stdio.h>
#include <stdlib.h>
#include "formatConverter_phaseAlign.h"
#include <math.h>

#ifdef RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
#ifndef M_PI
#define M_PI                3.14159265358979323846264338327950288f
#endif
#endif

void phaseAlignProcessFrame(void *handle);

void phaseAlignInit( void **handle, 
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                    float **dmxMtx, 
#else
                    float ***dmxMtx, 
#endif
                    int numInChans, 
                    int numOutChans, 
                    int numBands, 
                    float eqLimitMax, 
                    float eqLimitMin)
{
    int chA,chB,i,band;
    phaseAligner *h;

    *handle = malloc(sizeof(phaseAligner));
    h = (phaseAligner*)(*handle);
    
    /* Collect basic data */
    h->collectCounter = 0;
    h->numBands = numBands;
    h->numInChans = numInChans;
    h->numOutChans = numOutChans;
    h->epsilon = 1e-35f;
    
#ifdef RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
    for( chA=0; chA < numInChans; chA++ )
      for (band=0;band<h->numBands;band++)
        h->E_prev_thdiff[chA][band] = 0.f; 
#endif
   
    /* Memory allocations */
    h->lastRoundLastEnergies = (float*)malloc(sizeof(float)*h->numBands);
    for (band=0;band<h->numBands;band++)
    {
        h->lastRoundLastEnergies[band] = h->epsilon;
    }
    for (i=0;i<NUMTEMPVECTORS;i++)
    {
        h->channelTempFloatVectors[i] = (float*)malloc(sizeof(float)*h->numInChans);
        h->bandTempFloatVectors[i] = (float*)malloc(sizeof(float)*h->numBands);
    }
    
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            h->CxRelevant[chA][chB]=0;
        }
        for (chB=0;chB<=chA;chB++)
        {
            h->Cx[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->Cx[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->P[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->P[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->f_Cx[chA][chB] = (float*)calloc(sizeof(float),h->numBands);
            
        }
        h->inWindow[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        h->inFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        h->nextFirstHalfWindow[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        for (band=0;band<h->numBands;band++)
        {
            h->inFrame[chA][band].re = (float*)malloc(sizeof(float)*HOPSIZE);
            h->inFrame[chA][band].im = (float*)malloc(sizeof(float)*HOPSIZE);
            h->inWindow[chA][band].re = (float*)malloc(sizeof(float)*2*HOPSIZE);
            h->inWindow[chA][band].im = (float*)malloc(sizeof(float)*2*HOPSIZE);
            h->nextFirstHalfWindow[chA][band].re = (float*)calloc(sizeof(float),HOPSIZE);
            h->nextFirstHalfWindow[chA][band].im = (float*)calloc(sizeof(float),HOPSIZE);
        }
    }
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        h->outFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        h->nextOutFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        h->outWindow[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        for (band=0;band<h->numBands;band++)
        {
            h->outFrame[chA][band].re = (float*)calloc(sizeof(float),HOPSIZE);
            h->outFrame[chA][band].im = (float*)calloc(sizeof(float),HOPSIZE);
            h->nextOutFrame[chA][band].re = (float*)calloc(sizeof(float),HOPSIZE);
            h->nextOutFrame[chA][band].im = (float*)calloc(sizeof(float),HOPSIZE);
            h->outWindow[chA][band].re = (float*)malloc(sizeof(float)*2*HOPSIZE);
            h->outWindow[chA][band].im = (float*)malloc(sizeof(float)*2*HOPSIZE);
        }
    }

    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            h->M[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->M[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->M_cmp_prev[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->M_cmp_prev[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
        }
    }
 
    /* set DMX matrix and identify the relevant entries of Cx */  
    phaseAlignSetDmxMtx(h, dmxMtx);
     
    /* Gather EQ amplitude limits */
    h->eqLimitMax = (float)pow(10.0f,eqLimitMax/20.0f);
    h->eqLimitMin = (float)pow(10.0f,eqLimitMin/20.0f);
    
}

void phaseAlignSetDmxMtx( phaseAligner *h, 
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                         float **dmxMtx)
#else
                         float ***dmxMtx)
#endif
{
    int chA,chB,i,j,band;
    int numBands = h->numBands;

#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    /* Gather dmxMtx and identify its non-zero entries (for efficiency) */
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            for (band=0;band<numBands;band++)
            {
                h->M_cmp_prev[chA][chB].re[band] = dmxMtx[chB][chA];
            }
            if (dmxMtx[chB][chA] > 0)
            {
                h->dmxMtx[chA][chB]=dmxMtx[chB][chA];
                h->dmxMtxNonZero[chA][chB]=1;
            }
            else /* Note that less than zero dmxMtx entries are unnaccounted for in this software version */
            {
                h->dmxMtx[chA][chB]=0.0f;
                h->dmxMtxNonZero[chA][chB]=0;
            }
        }
        /* Identify the relevant entries of Cx (for efficiency) */
        for (i=0;i<h->numInChans;i++)
        {
            for (j=0;j<h->numInChans;j++)
            {
                if ((dmxMtx[j][chA] > 0) && (dmxMtx[i][chA] > 0))
                {
                    h->CxRelevant[i][j]=1;
                }
                
            }
        }
    }
#else
    /* init */
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            for (band=0;band<numBands;band++)
            {
                h->dmxMtx[band][chA][chB]=0.0f;
            }
            h->dmxMtxNonZero[chA][chB]=0;
        }
    }   
    /* Gather dmxMtx and identify its non-zero entries (for efficiency) */
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            for (band=0;band<numBands;band++)
            {
                h->M_cmp_prev[chA][chB].re[band] = dmxMtx[band][chB][chA];
                if (dmxMtx[band][chB][chA] > 0)
                {
                    h->dmxMtx[band][chA][chB]=dmxMtx[band][chB][chA];
                    h->dmxMtxNonZero[chA][chB]=1;
                }
            }
        }
        /* Identify the relevant entries of Cx (for efficiency) */
        for (i=0;i<h->numInChans;i++)
        {
            for (j=0;j<h->numInChans;j++)
            {
                for (band=0;band<numBands;band++) /* doing it for all bands guarantees that no entry is missed due to eq(bandXY) == 0 */
                {
                    if ((dmxMtx[band][j][chA] > 0) && (dmxMtx[band][i][chA] > 0))
                    {
                        h->CxRelevant[i][j]=1;
                    }
                }                    
            }
        }
    }     
#endif
}

void phaseAlignProcess(void *handle, const float **in_QMF_real, const float **in_QMF_imag, float **out_QMF_real, float **out_QMF_imag)
{
    /* Function to process one QMF time slot. Buffers data and calls the frame based function */
    
    phaseAligner* h = (phaseAligner*)handle;
    int ch,band;
    
    /* Collect input to buffer */
    for (ch=0;ch<h->numInChans;ch++)
    {
        for (band=0;band<h->numBands;band++)
        {
            h->inFrame[ch][band].re[h->collectCounter] = in_QMF_real[ch][band];
            h->inFrame[ch][band].im[h->collectCounter] = in_QMF_imag[ch][band];
        }
    }
    
    /* Retreive processed from buffer */
    for (ch=0;ch<h->numOutChans;ch++)
    {
        for (band=0;band<h->numBands;band++)
        {
            out_QMF_real[ch][band]=h->outFrame[ch][band].re[h->collectCounter];
            out_QMF_imag[ch][band]=h->outFrame[ch][band].im[h->collectCounter];
        }
    }
    
    /* Process when collected */
    h->collectCounter++;
    if (h->collectCounter == HOPSIZE)
    {
        phaseAlignProcessFrame(handle);
        h->collectCounter=0;
    }
    
    
}

void phaseAlignProcessFrame(void *handle)
{
    phaseAligner* h = (phaseAligner*)handle;
    int ch,sample,band,chA,chB;
    
    /***********************
     ADAPTIVE WINDOWING 
     ***************************/
    
    float tempFloat;
    float cumSum;
    float eneDiff, eneSum;
    float *energyPr;
    float *windowWeights = h->hopTempFloatVectors[1];
    for (band=0;band<h->numBands;band++)
    {
        energyPr = h->hopTempFloatVectors[0];
        *energyPr = h->lastRoundLastEnergies[band];
        energyPr++;
        cumSum=0.0f;
        
        /* Go through all samples in the window, and formulate an adaptive time 
         window shape which reacts to signal energy changes in decibels, weighted with energy.*/
        
        for (sample=0;sample<HOPSIZE;sample++)
        {
            *energyPr=h->epsilon;
            for (ch=0;ch<h->numInChans;ch++)
            {
                *energyPr += h->inFrame[ch][band].re[sample]*h->inFrame[ch][band].re[sample];
                *energyPr += h->inFrame[ch][band].im[sample]*h->inFrame[ch][band].im[sample];
            }
            eneDiff = *energyPr/(*(energyPr-1));
            eneSum = *energyPr + *(energyPr-1);
            cumSum += (float) fabs(10*(float)log10(eneDiff))*eneSum + h->epsilon;
            windowWeights[sample] = cumSum;
            energyPr++;
        }
        h->lastRoundLastEnergies[band] = *(energyPr-1);
        
        
        for (sample=0;sample<HOPSIZE;sample++)
        {
            windowWeights[sample] /= cumSum; /* Normalize the window to range from 0..1 */
            for (ch=0;ch<h->numInChans;ch++)
            {
                /* Retreve the first half of the window from the previous processing round */
                h->inWindow[ch][band].re[sample] = h->nextFirstHalfWindow[ch][band].re[sample];
                h->inWindow[ch][band].im[sample] = h->nextFirstHalfWindow[ch][band].im[sample];
                
                /* Window the second half of the window and store it for the next round */
                h->nextFirstHalfWindow[ch][band].re[sample] = windowWeights[sample]*h->inFrame[ch][band].re[sample];
                h->nextFirstHalfWindow[ch][band].im[sample] = windowWeights[sample]*h->inFrame[ch][band].im[sample];
                
                /* Window the first half of the window */
                h->inWindow[ch][band].re[sample+HOPSIZE] = h->inFrame[ch][band].re[sample] - h->nextFirstHalfWindow[ch][band].re[sample];
                h->inWindow[ch][band].im[sample+HOPSIZE] = h->inFrame[ch][band].im[sample] - h->nextFirstHalfWindow[ch][band].im[sample];
                
            }
        }       
    }
    
    /**************
     ESTIMATE Cx 
     ***************/
   {
    float *pr,*pi;
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<=chA;chB++)
        {
            if (h->CxRelevant[chA][chB]) /* Process only relevant channel pairs */
            {
                pr = h->Cx[chA][chB].re;
                /* For diagonal process only real part */
                if (chA == chB)
                {
                    for (band=0;band<h->numBands;band++)
                    {
                        *pr=0.0f;
                        for (sample=0;sample<2*HOPSIZE;sample++)
                        {
                            *pr += h->inWindow[chA][band].re[sample]*h->inWindow[chB][band].re[sample];
                            *pr += h->inWindow[chA][band].im[sample]*h->inWindow[chB][band].im[sample];
                        }
                        pr++;
                    }
                }
                /* For non-diagonal process also imaginary part */
                else
                {
                    pi = h->Cx[chA][chB].im;
                    for (band=0;band<h->numBands;band++)
                    {
                        *pr=0.0f;
                        *pi=0.0f;
                        for (sample=0;sample<2*HOPSIZE;sample++)
                        {
                            *pr += h->inWindow[chA][band].re[sample]*h->inWindow[chB][band].re[sample];
                            *pr += h->inWindow[chA][band].im[sample]*h->inWindow[chB][band].im[sample];
                            *pi += h->inWindow[chA][band].im[sample]*h->inWindow[chB][band].re[sample];
                            *pi -= h->inWindow[chA][band].re[sample]*h->inWindow[chB][band].im[sample];
                        } 
                        pr++;
                        pi++;
                    }
                }
            } 
        }
    }
   }
    
    /******************************
     FORMULATE DOWNMIXING MATRIX 
     *******************************/
    
   {
    float ICC,f_ICC; /* ICC = normalized phase corrected cross-correlation, f_ICC = the attraction coefficient (see publication draft) */
    float crossTerm;
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<=chA;chB++)
        {
            if (h->CxRelevant[chA][chB])
            {
                
                for (band=0;band<h->numBands;band++)
                {
                    crossTerm = h->Cx[chA][chB].re[band]*h->Cx[chA][chB].re[band] + h->Cx[chA][chB].im[band]*h->Cx[chA][chB].im[band];
                    ICC = (float)sqrt(crossTerm/(h->epsilon + h->Cx[chA][chA].re[band]*h->Cx[chB][chB].re[band]));
                    
                    /* PHASE ATTRACTION RULE */
                    f_ICC = 2.5f*ICC-1.2f;
                    if (f_ICC < 0.0f)
                    {
                        f_ICC=0.0f;
                    }
                    if (f_ICC > 1.0f)
                    {
                        f_ICC = 1.0f;
                    }
                    if (chA != chB) 
                    {
                        f_ICC = f_ICC * 0.25f;
                    }                    

                    /* Formulate matrix P = Cx.*f_ICC */
                    h->P[chA][chB].re[band] = h->Cx[chA][chB].re[band]*f_ICC;
                    h->P[chA][chB].im[band] = h->Cx[chA][chB].im[band]*f_ICC;
                    
                    /* Store f_ICC, for usage in the regularization part */
                    h->f_Cx[chA][chB][band] = f_ICC;
                }
            }
        }
    }
   }
    /* The phase adjustment coefficients are solved with matrix multiplication dmxMtx*P, 
     and then the amplitudes of the resulting matrix are normalized to those of dmxMtx */
   {
    float *pRe;
    float *pIm;
    float *pPre;
    float *pPim;
    float energy,EQfactor;
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    float dmxMtxEntry;
#endif
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            if (h->dmxMtxNonZero[chA][chB]) /* Formulate only relevant entries */
            {
                /* Clear memory */
                for (band=0;band<h->numBands;band++)
                {
                    h->M[chA][chB].re[band] = 0.0f;
                    h->M[chA][chB].im[band] = 0.0f;
                }
                
                /* Formulate output entry [chA][chB] in Matrix operation dmxMtx*P,
                   i.e. a dot product of a row vector in dmxMtx and a column vector in P.
                 Store the result in M. */
                for (ch=0;ch<h->numInChans;ch++)
                {
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                    dmxMtxEntry = h->dmxMtx[chA][ch]; /* Same for all bands */
#endif
                    pRe = h->M[chA][chB].re;
                    pIm = h->M[chA][chB].im;
                    
                    if (ch >= chB) /* This switch is implemented since only half of the conjugate symmetric matric P is available */
                    {
                        pPre = h->P[ch][chB].re;
                        pPim = h->P[ch][chB].im;
                        for (band=0;band<h->numBands;band++)
                        {
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                            *pRe +=  dmxMtxEntry * (*pPre);
                            *pIm +=  dmxMtxEntry * (*pPim);
#else
                            *pRe +=  h->dmxMtx[band][chA][ch] * (*pPre);
                            *pIm +=  h->dmxMtx[band][chA][ch] * (*pPim);                      
#endif
                            pRe++;
                            pIm++;
                            pPre++;
                            pPim++;
                        }
                        
                    }
                    else
                    {
                        pPre = h->P[chB][ch].re;
                        pPim = h->P[chB][ch].im;
                        for (band=0;band<h->numBands;band++)
                        {
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                            *pRe +=  dmxMtxEntry * (*pPre);
                            *pIm -=  dmxMtxEntry * (*pPim);
#else
                            *pRe +=  h->dmxMtx[band][chA][ch] * (*pPre);
                            *pIm -=  h->dmxMtx[band][chA][ch] * (*pPim);
#endif
                            pRe++;
                            pIm++;
                            pPre++;
                            pPim++;
                        }
                    }
                }
                
                /* Regularization of the amplitudes of the formulated mixing coefficients to those of dmxMtx */
                pRe = h->M[chA][chB].re;
                pIm = h->M[chA][chB].im;
                for (band=0;band<h->numBands;band++)
                {
                    energy = (*pRe)*(*pRe) + (*pIm)*(*pIm);
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                    EQfactor = h->dmxMtx[chA][chB]/(h->epsilon + (float)sqrt(energy));
#else
                    EQfactor = h->dmxMtx[band][chA][chB]/(h->epsilon + (float)sqrt(energy));              
#endif
                    *pRe *= EQfactor;
                    *pIm *= EQfactor;
                    pRe++;
                    pIm++;
                }
            }
        }
    }
   }
    
    /******************
     REGULARIZATION 
     ******************/
    /* Regularization across time only (option would be also across the frequency) */
    
   {
    float amplitude;
    float re, im;
    float theta,thetaAbs;
    float thetaRe,thetaIm;
    
    for (band=0;band<h->numBands;band++)
    {
        /* Formulate crossTermMatrix to contain energy weighted crossterms of M in respect to the M of the previous hop */
        /* Based on the resulting crossTermMatrix the abrupt phase changes are identified and regularized */
        for (chB=0;chB<h->numInChans;chB++)
        {
            amplitude=(float)sqrt(h->Cx[chB][chB].re[band]);
            for (chA=0;chA<h->numOutChans;chA++)
            {
                if (h->dmxMtxNonZero[chA][chB])
                {
                    h->crossTermMatrixRe[chA][chB] = h->M[chA][chB].re[band]*h->M_cmp_prev[chA][chB].re[band];
                    h->crossTermMatrixRe[chA][chB] += h->M[chA][chB].im[band]*h->M_cmp_prev[chA][chB].im[band];
                    h->crossTermMatrixRe[chA][chB] *= amplitude;
                    
                    h->crossTermMatrixIm[chA][chB] = h->M[chA][chB].im[band]*h->M_cmp_prev[chA][chB].re[band];
                    h->crossTermMatrixIm[chA][chB] -= h->M[chA][chB].re[band]*h->M_cmp_prev[chA][chB].im[band];
                    h->crossTermMatrixIm[chA][chB] *= amplitude;
                }
                else
                {
                    h->crossTermMatrixRe[chA][chB] = 0.0f;
                    h->crossTermMatrixIm[chA][chB] = 0.0f;
                }
            }
        }
        
        /* Mix the crossTermMatrix values with the attraction matrix f_Cx, 
         to keep those channels linked that were phase aligned in respect to each other.
         After it perform the regularization of the phase change. */
        for (chB=0;chB<h->numInChans;chB++)
        {
            float thdiff = 0.785398163f;
#ifdef RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
            float E_curr_thdiff, E_prev_thdiff, E_ratio=0.f;
            float k_thdiff = 0.001875f;
            float k0_thdiff = 0.0625f;

            E_curr_thdiff = h->Cx[chB][chB].re[band]; 
            E_prev_thdiff = h->E_prev_thdiff[chB][band];
            h->E_prev_thdiff[chB][band] = E_curr_thdiff;

            /* Unlock phase in case of onsets: R = E_curr/E_prev,
               full unlocking for R>=500, no unlocking for R<=100,
               transition inbetween                               */
            if(E_curr_thdiff > (500.f * E_prev_thdiff)) 
                thdiff = M_PI;
            else if(E_curr_thdiff > (100.f * E_prev_thdiff)) {
                E_ratio = (E_curr_thdiff+1e-20f)/(E_prev_thdiff+1e-20f);
                thdiff = M_PI * (k_thdiff * E_ratio + k0_thdiff); 
            }
#endif
            for (chA=0;chA<h->numOutChans;chA++)
            {
                if (h->dmxMtxNonZero[chA][chB])
                {
                    re=0.0f;
                    im=0.0f;
                    /* Mix the crossTermMatrix matrix with f_Cx */
                    for (ch=0;ch<h->numInChans;ch++)
                    {
                        
                        if (ch >= chB)
                        {
                            tempFloat = h->f_Cx[ch][chB][band];
                        }
                        else
                        {
                            tempFloat = h->f_Cx[chB][ch][band];
                        }
                        re += h->crossTermMatrixRe[chA][ch] * tempFloat;
                        im += h->crossTermMatrixIm[chA][ch] * tempFloat;
                    }
                    
                    /* Formulate phase change, and limit the maximum update of the phase change to thdiff */
                    theta = (float)atan2(im,re);
                    thetaAbs = (float)fabs(theta);
                    thetaAbs = thetaAbs - thdiff; 
                    if (thetaAbs < 0.0f)
                    {
                        thetaAbs = 0.0f;
                    }
                    if (theta > 0.0f)
                    {
                        theta = thetaAbs;
                    }
                    else
                    {
                        theta = -thetaAbs;
                    }
                    
                    /* Apply the phase regularization to the entries in M */
                    thetaRe = (float)cos(theta);
                    thetaIm = (float)-sin(theta);
                    tempFloat = thetaRe*h->M[chA][chB].re[band] - thetaIm*h->M[chA][chB].im[band];
                    h->M[chA][chB].im[band] = thetaRe*h->M[chA][chB].im[band] + thetaIm*h->M[chA][chB].re[band];
                    h->M[chA][chB].re[band] = tempFloat;
                    
                    /* Store the amplitude weighted and phase adjusted M for the next hop */
                    tempFloat = (float)sqrt(h->Cx[chB][chB].re[band]);
                    h->M_cmp_prev[chA][chB].re[band] = h->M[chA][chB].re[band] * tempFloat;
                    h->M_cmp_prev[chA][chB].im[band] = h->M[chA][chB].im[band] * tempFloat;   
                }
            }
        }
    }
   }         
    /**************
     ENERGY EQ 
     ***************/
   {
    float *realizedEne = h->bandTempFloatVectors[0];
    float *targetEne = h->bandTempFloatVectors[1];
    float EQ;
    float *pRe;
    float *pIm;
    pRe = h->bandTempFloatVectors[2];
    pIm = h->bandTempFloatVectors[3];
    for (chA=0;chA<h->numOutChans;chA++)
    {
        /* Flush */
        for (band=0;band<h->numBands;band++)
        {
            targetEne[band] = 0.0f;
            realizedEne[band]=0.0f;
        }
        /* Formulate downmix target energy (energy preserving)  */
        for (chB=0;chB<h->numInChans;chB++)
        {
            if (h->dmxMtxNonZero[chA][chB])
            {
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                tempFloat = (float) pow(h->dmxMtx[chA][chB],2.0f);
#endif
                for (band=0;band<h->numBands;band++)
                {
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
                    tempFloat = (float) pow(h->dmxMtx[band][chA][chB],2.0f);
#endif
                    targetEne[band] += h->Cx[chB][chB].re[band]*tempFloat;
                }
            } 
        }
        
        /* The following formulates the matrix multiplication M*Cx*M for one diagonal entry of the resulting matrix, i.e the realized frequency band output channel energy. */
        for (chB=0;chB<h->numInChans;chB++)
        {
            if (h->dmxMtxNonZero[chA][chB])
            {
                for (band=0;band<h->numBands;band++)
                {
                    pRe[band] = 0.0f;
                    pIm[band] = 0.0f;
                }
                for (ch=0;ch<h->numInChans;ch++)
                {
                    if (h->dmxMtxNonZero[chA][ch])
                    {
                        if (ch >= chB) /* Since only half of the conjugate symmetric Cx is available */
                        {
                            for (band=0;band<h->numBands;band++)
                            {
                                pRe[band] += h->M[chA][ch].re[band]*h->Cx[ch][chB].re[band];
                                pRe[band] -= h->M[chA][ch].im[band]*h->Cx[ch][chB].im[band];
                                pIm[band] += h->M[chA][ch].re[band]*h->Cx[ch][chB].im[band];
                                pIm[band] += h->M[chA][ch].im[band]*h->Cx[ch][chB].re[band];
                            }
                        }
                        else
                        {
                            for (band=0;band<h->numBands;band++)
                            {
                                pRe[band] += h->M[chA][ch].re[band]*h->Cx[chB][ch].re[band];
                                pRe[band] += h->M[chA][ch].im[band]*h->Cx[chB][ch].im[band];
                                pIm[band] -= h->M[chA][ch].re[band]*h->Cx[chB][ch].im[band];
                                pIm[band] += h->M[chA][ch].im[band]*h->Cx[chB][ch].re[band];
                            }
                        } 
                    }
                }
                for (band=0;band<h->numBands;band++)
                {
                    realizedEne[band] += pRe[band] * h->M[chA][chB].re[band];
                    realizedEne[band] += pIm[band] * h->M[chA][chB].im[band];
                }
            } 
        }
        /* Energy correcting equalization gain of this frequency band and output channel */
        for (band=0;band<h->numBands;band++)
        {
            EQ = (float) sqrt(targetEne[band]/(h->epsilon + realizedEne[band]));
            if (EQ > h->eqLimitMax)
            {
                EQ = h->eqLimitMax;
            }
            if (EQ < h->eqLimitMin)
            {
                EQ = h->eqLimitMin;
            }

            if (mode == 4)
            {
              EQ = h->floatAES * EQ + (1.0f - h->floatAES);
            }
            /* Apply the equalizer to the mixing gains */
            for (chB=0;chB<h->numInChans;chB++)
            {
                h->M[chA][chB].re[band] *= EQ;
                h->M[chA][chB].im[band] *= EQ;
            }
        }
    }
   }
    /**************
     MIX OUTPUT 
     ***************/
   {
    float *pMre,*pMim;
    float *pOutRe,*pOutIm;
    float *pInRe,*pInIm;
    for (chA=0;chA<h->numOutChans;chA++)
    {
        /* Flush the output window */
        for (band=0;band<h->numBands;band++)
        {
            for (sample=0;sample<2*HOPSIZE;sample++)
            {
                h->outWindow[chA][band].re[sample]=0.0f;
                h->outWindow[chA][band].im[sample]=0.0f;
            }
        }
        
        /* Mix the input signal window to the output signal window using the formulated M */
        for (chB=0;chB<h->numInChans;chB++)
        {
            if (h->dmxMtxNonZero[chA][chB])
            {
                pMre = h->M[chA][chB].re;
                pMim = h->M[chA][chB].im;
                for (band=0;band<h->numBands;band++)
                {
                    pOutRe = h->outWindow[chA][band].re;
                    pOutIm = h->outWindow[chA][band].im;
                    pInRe = h->inWindow[chB][band].re;
                    pInIm = h->inWindow[chB][band].im;
                    for (sample=0;sample<2*HOPSIZE;sample++)
                    {
                        (*pOutRe) += (*pMre)*(*pInRe) - (*pMim)*(*pInIm);
                        (*pOutIm) += (*pMim)*(*pInRe) + (*pMre)*(*pInIm);
                        pOutRe++;
                        pOutIm++;
                        pInRe++;
                        pInIm++;
                    }
                    pMre++;
                    pMim++;
                }
            }
        }
    }
   }
    /* Overlap-add processing - add previously produced second window half to the new first window half */
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (band=0;band<h->numBands;band++)
        {
            for (sample=0;sample<HOPSIZE;sample++)
            {
                h->outFrame[chA][band].re[sample] = h->nextOutFrame[chA][band].re[sample] + h->outWindow[chA][band].re[sample];
                h->outFrame[chA][band].im[sample] = h->nextOutFrame[chA][band].im[sample] + h->outWindow[chA][band].im[sample];
                h->nextOutFrame[chA][band].re[sample] = h->outWindow[chA][band].re[sample+HOPSIZE];
                h->nextOutFrame[chA][band].im[sample] = h->outWindow[chA][band].im[sample+HOPSIZE];
            }
        }
    }
    
    
}

void phaseAlignClose(void *handle)
{
    int chA,chB,band,i;
    phaseAligner* h = (phaseAligner*)handle;
    
    if(h == NULL) /* nothing to free */
        return;

    free(h->lastRoundLastEnergies);
    
    for (i=0;i<NUMTEMPVECTORS;i++)
    {
        free(h->channelTempFloatVectors[i]);
        free(h->bandTempFloatVectors[i]);
    }
    
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<=chA;chB++)
        {
            free(h->Cx[chA][chB].re);
            free(h->Cx[chA][chB].im);
            free(h->P[chA][chB].re);
            free(h->P[chA][chB].im);
            free(h->f_Cx[chA][chB]);   
        }
        
        for (band=0;band<h->numBands;band++)
        {
            free(h->inFrame[chA][band].re);
            free(h->inFrame[chA][band].im);
            free(h->inWindow[chA][band].re);
            free(h->inWindow[chA][band].im);
            free(h->nextFirstHalfWindow[chA][band].re);
            free(h->nextFirstHalfWindow[chA][band].im);
        }
        free(h->inWindow[chA]);
        free(h->inFrame[chA]);
        free(h->nextFirstHalfWindow[chA]);
    }
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        
        for (band=0;band<h->numBands;band++)
        {
            free(h->outFrame[chA][band].re);
            free(h->outFrame[chA][band].im);
            free(h->nextOutFrame[chA][band].re);
            free(h->nextOutFrame[chA][band].im);
            free(h->outWindow[chA][band].re);
            free(h->outWindow[chA][band].im);
        }
        free(h->outFrame[chA]);
        free(h->nextOutFrame[chA]);
        free(h->outWindow[chA]);
    }
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            free(h->M[chA][chB].re);            
            free(h->M[chA][chB].im);
            free(h->M_cmp_prev[chA][chB].re);
            free(h->M_cmp_prev[chA][chB].im);
        }
    }
    free(h);
}

#endif /* #ifndef FORMATCONVERTER_LOWCOMPLEXITY */
