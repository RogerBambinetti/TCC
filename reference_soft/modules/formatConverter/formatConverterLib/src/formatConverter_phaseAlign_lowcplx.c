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

#include <stdio.h>
#include <stdlib.h>
#include "formatConverter_phaseAlign_lowcplx.h"
#include <math.h>

#define FLUSH_DENORMALS

#ifndef M_PI
#define M_PI                3.14159265358979323846264338327950288f
#endif

void phaseAlignProcessFrame(void *handle);

int phaseAlignInit( void **handle, 
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

    if((numInChans>MAX_CHANNELS)||(numOutChans>MAX_CHANNELS)) {
        fprintf(stderr,"Error: initialization of phase align DMX failed:\n");
        fprintf(stderr," --> number of channels exceeds max. allowed number (%d).\n",MAX_CHANNELS);
        return -1;
    }

    *handle = malloc(sizeof(phaseAligner));
    h = (phaseAligner*)(*handle);
    
    /* Collect basic data */
    h->collectCounter = 0;
    h->numBands = numBands;
    h->numInChans = numInChans;
    h->numOutChans = numOutChans;
    h->epsilon = 1e-35f;

    phaseAlignSetPAS(3, h);
    phaseAlignSetAES(7, h);
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
            h->CxTimeOverlap[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->CxTimeOverlap[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->P[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->P[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->f_Cx[chA][chB] = (float*)calloc(sizeof(float),h->numBands);
            
        }
        h->inFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        h->prevInFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        for (band=0;band<h->numBands;band++)
        {
            h->inFrame[chA][band].re = (float*)malloc(sizeof(float)*HOPSIZE);
            h->inFrame[chA][band].im = (float*)malloc(sizeof(float)*HOPSIZE);
            h->prevInFrame[chA][band].re = (float*)calloc(sizeof(float),HOPSIZE);
            h->prevInFrame[chA][band].im = (float*)calloc(sizeof(float),HOPSIZE);
            h->CxDiagPrev[chA][band]=0.0f;
        }
        
    }
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        h->outFrame[chA] = (compexVector*)malloc(sizeof(compexVector)*h->numBands);
        for (band=0;band<h->numBands;band++)
        {
            h->outFrame[chA][band].re = (float*)calloc(sizeof(float),HOPSIZE);
            h->outFrame[chA][band].im = (float*)calloc(sizeof(float),HOPSIZE);
        }
    }
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            h->M[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->M[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->M_current[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->M_current[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
            h->M_cmp_prev[chA][chB].re = (float*)calloc(sizeof(float),h->numBands);
            h->M_cmp_prev[chA][chB].im = (float*)calloc(sizeof(float),h->numBands);
        }
    }

    /* set DMX matrix and identify the relevant entries of Cx */  
    phaseAlignSetDmxMtx(h, dmxMtx);
    
    /* Gather EQ amplitude limits */
    h->eqLimitMax = (float)pow(10.0f,eqLimitMax/20.0f);
    h->eqLimitMin = (float)pow(10.0f,eqLimitMin/20.0f);
        
    return 0;
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
                h->M_cmp_prev[chA][chB].re[band] = 0*dmxMtx[chB][chA];
                h->M_current[chA][chB].re[band] = dmxMtx[chB][chA];
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
                h->M_cmp_prev[chA][chB].re[band] = 0*dmxMtx[band][chB][chA];
                h->M_current[chA][chB].re[band] = dmxMtx[band][chB][chA];
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
    
    if (h->numBands == 71)
    {
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
    } 
    else
    {
        fprintf(stderr, "\nERROR: this code is supposed to run in 71bands mode!\n");
        return;
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
    float *pr,*pi,*prNext,*piNext;
    float re1,im1,re2,im2,tempOffset;
   
    float *pRe;
    float *pIm;
    float *pPre;
    float *pPim;
    float amplitude;
    float re, im;
    float theta,thetaAbs;
    float thetaRe,thetaIm;
    float thetaDiff;
    float energy,EQfactor;

#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    float dmxMtxEntry;
#endif
    float thetaDiff_part2;
    float onsetValue;
    float thetaSum, thetaPrev;
    float *realizedEne = h->bandTempFloatVectors[0];
    float *targetEne = h->bandTempFloatVectors[1];
    float EQ;
    float incrementRe, incrementIm;
 
    /**********************************
     *  (no-longer adaptive windowing) 
     **********************************/
    
    
    float tempFloat;
   
#ifdef FLUSH_DENORMALS
    for (chA=0;chA<h->numInChans;chA++)
    {
      for (band=0;band<h->numBands;band++)
      {
        for (sample=0; sample < HOPSIZE; sample++)
        {
          if ( ((float) fabs( h->inFrame[chA][band].re[sample])) < 1e-12f )  /* 1e-12f value taken from MPS-Encoder: space_hybrid.c */
            h->inFrame[chA][band].re[sample] = 0.0f;

          if ( ((float) fabs(h->inFrame[chA][band].im[sample])) < 1e-12f )
            h->inFrame[chA][band].im[sample] = 0.0f;
        }
      }
    }
#endif




    /***************
     * ESTIMATE Cx 
    ****************/
    
    
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<=chA;chB++)
        {
            if (h->CxRelevant[chA][chB])
            {
                for (band=0;band<h->numBands;band++)
                {
                    h->Cx[chA][chB].re[band] = h->CxTimeOverlap[chA][chB].re[band];
                    h->Cx[chA][chB].im[band] = h->CxTimeOverlap[chA][chB].im[band];
                    h->CxTimeOverlap[chA][chB].re[band] = 0.0f;
                    h->CxTimeOverlap[chA][chB].im[band] = 0.0f;
                }
                
            }
        }
    }
    
    
    
    tempOffset = 2.0e-4f;
    for (chA=0;chA<h->numInChans;chA++)
    {
        for (chB=0;chB<=chA;chB++)
        {
            if (h->CxRelevant[chA][chB]) /* Process only relevant channel pairs */
            {
                pr = h->Cx[chA][chB].re;
                prNext = h->CxTimeOverlap[chA][chB].re;
                /* For diagonal process only real part */
                if (chA == chB)
                {
                    for (band=0;band<h->numBands;band++)
                    {
                        re1=0.0f;
                        re2=0.0f;
                        im1=0.0f;
                        im2=0.0f;
                        for (sample=0;sample<HOPSIZE/2;sample++)
                        {
                            re1 += h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].re[sample];
                            re1 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].im[sample];
                        }
                        for(;sample<HOPSIZE;sample++)
                        {
                            re2 += h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].re[sample];
                            re2 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].im[sample];
                        }
                        *pr += 4.0f*re1 + re2;
                        *prNext += re1 + 4.0f*re2;
                        /* Fix for issue: Downmixing bit-exact opposite phase signals, add small energy offset */
                        tempFloat =1.0f + chA*tempOffset;
                        *pr *= tempFloat;
                        
                        pr++;
                        prNext++;
                    }
                }
                /* For non-diagonal process also imaginary part */
                else
                {
                    pi = h->Cx[chA][chB].im;
                    piNext = h->CxTimeOverlap[chA][chB].im;
                    for (band=0;band<h->numBands;band++)
                    {
                        re1=0.0f;
                        re2=0.0f;
                        im1=0.0f;
                        im2=0.0f;
                        for (sample=0;sample<HOPSIZE/2;sample++)
                        {
                            re1 += h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].re[sample];
                            re1 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].im[sample];
                            im1 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].re[sample];
                            im1 -= h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].im[sample];
                        }
                        for (;sample<HOPSIZE;sample++)
                            
                        {
                            re2 += h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].re[sample];
                            re2 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].im[sample];
                            im2 += h->inFrame[chA][band].im[sample]*h->inFrame[chB][band].re[sample];
                            im2 -= h->inFrame[chA][band].re[sample]*h->inFrame[chB][band].im[sample];
                        }
                        /* Fix for issue: Downmixing bit-exact opposite phase signals, add small energy offset */
                        *pr += 4.0f*re1 + re2;
                        *pi += 4.0f*im1 + im2;
                        *prNext += re1 + 4.0f*re2;
                        *piNext += im1 + 4.0f*im2;
                        tempFloat =1.0f + (float)sqrt(chA*chB)*tempOffset;
                        *pr *= tempFloat;
                        *pi *= tempFloat;
                        
                        pr++;
                        pi++;
                        prNext++;
                        piNext++;
                    }
                }
            }
        }
    }
    
    {
        
        /********************************
         * FORMULATE DOWNMIXING MATRIX 
         ********************************/
        
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
#ifdef FLUSH_DENORMALS
                        if ( ((float) fabs( h->Cx[chA][chB].re[band])) < 1e-17f )
                          h->Cx[chA][chB].re[band]= 0.0f;

                        if ( ((float) fabs(h->Cx[chA][chB].im[band])) < 1e-17f )
                          h->Cx[chA][chB].im[band] = 0.0f;
#endif

                        crossTerm = h->Cx[chA][chB].re[band]*h->Cx[chA][chB].re[band] + h->Cx[chA][chB].im[band]*h->Cx[chA][chB].im[band];
                        ICC = (float)sqrt(crossTerm/(h->epsilon + h->Cx[chA][chA].re[band]*h->Cx[chB][chB].re[band]));
                        
                        /* PHASE ATTRACTION RULE */
                        if (chA == chB) {
                          
#ifdef FORMULA_ACCORDING_TO_N14462
                          f_ICC = (4.f * h->PasCurveSlope) * ICC + (4.f * h->PasCurveShift);

                          if( f_ICC < 0.0f )
                          {
                            f_ICC = 0.0f;
                          }
                          else if( f_ICC > 1.0f )
                          {
                            f_ICC = 1.0f;
                          }
#else
                          f_ICC = (2.5f * ICC) - 1.2f;
                          if(f_ICC < 0.f)
                          {
                            f_ICC = 0.f;
                          }
                          if(f_ICC > 1.f)
                          {
                            f_ICC = 1.f;
                          }
#endif
                        } else { /* chA != chB */

                          f_ICC = h->PasCurveSlope * ICC + h->PasCurveShift;
                          
                          if (f_ICC < 0.0f)
                          {
                            f_ICC = 0.0f;
                          } 
                          else if (f_ICC > h->PASMax)
                          {
                            f_ICC = h->PASMax;
                          }
                        }
                        /* Denormal processing */
                        if (f_ICC < 1e-17f)
                        {
                            f_ICC = 0.0f;
                        }
                        
                        /*fprintf(stderr, "f_ICC: %f\n", f_ICC);*/
                        /* Formulate matrix P = Cx.*f_ICC */
                        h->P[chA][chB].re[band] = h->Cx[chA][chB].re[band]*f_ICC;
                        h->P[chA][chB].im[band] = h->Cx[chA][chB].im[band]*f_ICC;
                        
                
                        /* Store f_ICC, for usage in the regularization part */
                        h->f_Cx[chA][chB][band] = f_ICC;
                    }
                }
            }
        }
        
        /* The phase adjustment coefficients are solved with matrix multiplication dmxMtx*P,
         and then the amplitudes of the resulting matrix are normalized to those of dmxMtx */

        
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
#ifdef FLUSH_DENORMALS
                        if ((fabs(*pRe) < 1e-17) && (fabs(*pIm) < 1e-17))
                        { /* Avoid denormal range */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                            *pRe = h->dmxMtx[chA][chB];
#else
                            *pRe = h->dmxMtx[band][chA][chB];
#endif
                            *pIm = 0.0f;
                        }
                        else
#endif 
                       {
                            energy = (*pRe)*(*pRe) + (*pIm)*(*pIm);
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                            EQfactor = h->dmxMtx[chA][chB]/(h->epsilon + (float)sqrt(energy));
#else
                            EQfactor = h->dmxMtx[band][chA][chB]/(h->epsilon +
                                                                  (float)sqrt(energy));
#endif
                            *pRe *= EQfactor;
                            *pIm *= EQfactor;
                        }
                        pRe++;
                        pIm++;
                    }

                }
            }
        }
        
        
        /*******************
         * REGULARIZATION 
         *******************/
        /* Regularization across time only (option would be also across the frequency) */
        
        for (band=0;band<h->numBands;band++)
        {
            /* add frequency axis to the weighting */
            if (band >0)
            {
                for (chB=0;chB<h->numInChans;chB++)
                {
                    for (chA=0;chA<h->numOutChans;chA++)
                    {
                        if (h->dmxMtxNonZero[chA][chB])
                        {
                            h->M_cmp_prev[chA][chB].re[band] += h->M_cmp_prev[chA][chB].re[band-1];
                            if(band==2) /* here happens the transition from f<0 to f>0, band==2 is the first for f>0, spectr. is conj. below */
                                h->M_cmp_prev[chA][chB].im[band] -= h->M_cmp_prev[chA][chB].im[band-1];
                            else
                                h->M_cmp_prev[chA][chB].im[band] += h->M_cmp_prev[chA][chB].im[band-1];                           
                        }
                        
                    }
                }
            }
            
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
                        
                        h->crossTermMatrixIm[chA][chB] = -h->M[chA][chB].im[band]*h->M_cmp_prev[chA][chB].re[band];
                        h->crossTermMatrixIm[chA][chB] += h->M[chA][chB].re[band]*h->M_cmp_prev[chA][chB].im[band];
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
                onsetValue = h->Cx[chB][chB].re[band]/(h->epsilon + h->Cx[chB][chB].re[band] + h->CxDiagPrev[chB][band]);
                h->CxDiagPrev[chB][band]=h->Cx[chB][chB].re[band];
                
                thetaDiff = 20.3f*onsetValue-19.0f;
                if (thetaDiff<0.15f)
                {
                    thetaDiff=0.15f;
                }
                
                thetaDiff *= M_PI;
                
                thetaDiff_part2 = thetaDiff;
                
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
                        
                        /* Formulate phase change, and limit the maximum update of the phase change adaptively */

#ifdef FLUSH_DENORMALS
                        if (fabs(im)>0.0f || fabs(re) > 0.0f)
                        {
                            thetaSum = (float)atan2(im,re);
                        }
                        else /* Avoid exceptions */
                        {
                            thetaSum = 0.0f;
                        }
#else
                        thetaSum = (float)atan2(im,re);
#endif
                        thetaAbs = (float)fabs(thetaSum);
                        
                        thetaAbs = thetaAbs - thetaDiff;
                        if (thetaAbs < 0.0f)
                        {
                            thetaAbs = 0.0f;
                        }
                        if (thetaSum > 0.0f)
                        {
                            thetaSum = thetaAbs;
                        }
                        else
                        {
                            thetaSum = -thetaAbs;
                        }
                        
                        
                        
                        im =  -h->M_current[chA][chB].re[band] * h->M[chA][chB].im[band];
                        im += h->M_current[chA][chB].im[band] * h->M[chA][chB].re[band];
                        re = h->M_current[chA][chB].re[band] * h->M[chA][chB].re[band];
                        re += h->M_current[chA][chB].im[band] * h->M[chA][chB].im[band];
                        thetaPrev = (float)atan2(im,re);
                        
                        theta = thetaSum - thetaPrev;
                        
                        if (theta < M_PI)
                        {
                            theta += 2*M_PI;
                        }
                        if (theta > M_PI)
                        {
                            theta -= 2*M_PI;
                        }
                        
                        if (theta >thetaDiff)
                        {
                            theta = thetaDiff;
                        }
                        if (theta <-thetaDiff)
                        {
                            theta = -thetaDiff;
                        }
                        
                        theta = thetaPrev + theta;
                       
                        /* Apply the phase regularization to the entries in M */
                        thetaRe = (float)cos(theta);
                        thetaIm = (float)sin(theta);
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
     * ENERGY EQ 
     **************/
    
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
                tempFloat = (float)pow(h->dmxMtx[chA][chB],2.0f);
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
        
        
        /* The following formulates the matrix multiplication M*Cx*M for one diagonal entry of the resulting matrix,
         i.e the realized frequency band output channel energy. */
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
#ifdef FLUSH_DENORMALS
          if (  realizedEne[band] < 0.0f ) realizedEne[band] = 0.0f;
          if (  targetEne[band] < 0.0f )   targetEne[band]   = 0.0f;
#endif
          
            EQ = (float)sqrt(targetEne[band]/(h->epsilon + realizedEne[band]));
            if (EQ > h->eqLimitMax)
            {
                EQ = h->eqLimitMax;
            }
            
            if (EQ < h->eqLimitMin)
            {
                EQ = h->eqLimitMin;
            }
            
            EQ = h->floatAES * EQ + (1.0f - h->floatAES);
            
            /* Apply the equalizer to the mixing gains */
            for (chB=0;chB<h->numInChans;chB++)
            {
                h->M[chA][chB].re[band] *= EQ;
                h->M[chA][chB].im[band] *= EQ;
            }
        }
    }
    
    
    
    /***************
     * MIX OUTPUT 
     ***************/
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        /* flush */
        for (band=0;band<h->numBands;band++)
        {
            for (sample=0;sample<HOPSIZE;sample++)
            {
                
                h->outFrame[chA][band].re[sample]=0.0f;
                h->outFrame[chA][band].im[sample]=0.0f;
                
            }
        }
        for (chB=0;chB<h->numInChans;chB++)
        {
            if (h->dmxMtxNonZero[chA][chB])
            {
                for (band=0;band<h->numBands;band++)
                {
                    re = h->M_current[chA][chB].re[band];
                    im = h->M_current[chA][chB].im[band];
                    incrementRe = (h->M[chA][chB].re[band] - h->M_current[chA][chB].re[band])/(float)HOPSIZE;
                    incrementIm = (h->M[chA][chB].im[band] - h->M_current[chA][chB].im[band])/(float)HOPSIZE;
                    for (sample=0;sample<HOPSIZE;sample++)
                    {
                        re += incrementRe;
                        im += incrementIm;
                        h->outFrame[chA][band].re[sample] += re*h->prevInFrame[chB][band].re[sample];
                        h->outFrame[chA][band].re[sample] -= im*h->prevInFrame[chB][band].im[sample];
                        h->outFrame[chA][band].im[sample] += re*h->prevInFrame[chB][band].im[sample];
                        h->outFrame[chA][band].im[sample] += im*h->prevInFrame[chB][band].re[sample];
                    }
                    h->M_current[chA][chB].re[band] = re;
                    h->M_current[chA][chB].im[band] = im;
                }
            }
        }
    }
    
    
    for (band=0;band<h->numBands;band++)
    {
        for (sample=0;sample<HOPSIZE;sample++)
        {
            for (ch=0;ch<h->numInChans;ch++)
            {
                h->prevInFrame[ch][band].re[sample] = h->inFrame[ch][band].re[sample];
                h->prevInFrame[ch][band].im[sample] = h->inFrame[ch][band].im[sample];
                
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
            free(h->CxTimeOverlap[chA][chB].re);
            free(h->CxTimeOverlap[chA][chB].im);
            free(h->P[chA][chB].re);
            free(h->P[chA][chB].im);
            free(h->f_Cx[chA][chB]);
        }
        
        for (band=0;band<h->numBands;band++)
        {
            free(h->inFrame[chA][band].re);
            free(h->inFrame[chA][band].im);
            free(h->prevInFrame[chA][band].re);
            free(h->prevInFrame[chA][band].im);
            
        }
        free(h->inFrame[chA]);
        free(h->prevInFrame[chA]);
    }
    
    for (chA=0;chA<h->numOutChans;chA++)
    {
        
        for (band=0;band<h->numBands;band++)
        {
            free(h->outFrame[chA][band].re);
            free(h->outFrame[chA][band].im);
        }
        free(h->outFrame[chA]);
    }
    for (chA=0;chA<h->numOutChans;chA++)
    {
        for (chB=0;chB<h->numInChans;chB++)
        {
            free(h->M[chA][chB].re);
            free(h->M[chA][chB].im);
            free(h->M_current[chA][chB].re);
            free(h->M_current[chA][chB].im);
            free(h->M_cmp_prev[chA][chB].re);
            free(h->M_cmp_prev[chA][chB].im);
        }
    }
    free(h);
}


int phaseAlignSetPAS(int PAS, void *handle)
{
  phaseAligner* h = (phaseAligner*)handle;
  h->PasCurveSlope = 0.f;
  h->PasCurveShift = 0.f;
  switch(PAS)
  {
    case 0:
      h->PASMax        = 0.0f;
      h->PasCurveSlope = 0.0f;
      h->PasCurveShift = 0.0f;
      break;
    case 1:
      h->PASMax        = 0.0714f;
      h->PasCurveSlope = 0.1701f;
      h->PasCurveShift = -0.0891f;
      break;
    case 2:
      h->PASMax        = 0.1548f;
      h->PasCurveSlope = 0.3771f;
      h->PasCurveShift = -0.1896f;
      break;
    case 3:
      h->PASMax      = 0.25f;
      h->PasCurveSlope = 0.625f;
      h->PasCurveShift = -0.3f;
      break;
    case 4:
      h->PASMax        = 0.3571f;
      h->PasCurveSlope = 0.9184f;
      h->PasCurveShift = -0.4184f;
      break;
    case 5:
      h->PASMax        = 0.4762f;
      h->PasCurveSlope = 1.2623f;
      h->PasCurveShift = -0.5427f;
      break;
    case 6:
      h->PASMax        = 0.6071f;
      h->PasCurveSlope = 1.6624f;
      h->PasCurveShift = -0.6707f;
      break;
    case 7:
      h->PASMax        = 0.75f;
      h->PasCurveSlope = 2.125f;
      h->PasCurveShift = -0.8f;
      break;
    default:
      fprintf(stderr, "Warning: Invalid PAS-Mode. Phase align strength is set to 0.\n");
      h->PASMax        = 0.0f;
      h->PasCurveSlope = 0.0f;
      h->PasCurveShift = 0.0f;
      break;
  }
 
    return 0;
}

int phaseAlignSetAES(int AES, void *handle){
    phaseAligner* h = (phaseAligner*)handle;
    h->floatAES = ((float)AES) / 7.f;
    return 0;
}


