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

#ifndef _IAR_FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_
#define _IAR_FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_

#include "iar_rom.h"

#define IAR_MAX_CHANNELS 29
#define IAR_HOPSIZE 32
#define IAR_NUMTEMPVECTORS 4 /* For memory allocation of temporary vectors that are used in the processing */

#ifndef IAR_MAX_BANDS
#define IAR_MAX_BANDS 77
#endif

typedef struct {
    float *re;
    float *im;
} iar_compexVector;

typedef struct {
    int numInChans,numOutChans;
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    float dmxMtx[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
#else
    float dmxMtx[IAR_MAX_BANDS][IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
#if IAR    /* secondary downmix matrix M_DMX2 */
    float dmxMtx2[IAR_MAX_BANDS][IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
#endif
#endif

    int dmxMtxNonZero[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float eqLimitMax;
    float eqLimitMin;
    iar_compexVector Cx[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector CxTimeOverlap[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float CxDiagPrev[IAR_MAX_CHANNELS][77];
    float *f_Cx[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector P[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector M[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector M_current[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector M_cmp_prev[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    int CxRelevant[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector *inFrame[IAR_MAX_CHANNELS];
    iar_compexVector *prevInFrame[IAR_MAX_CHANNELS];
#if IAR    /* for the delay buffering */
    iar_compexVector *prevInFrameD2[IAR_MAX_CHANNELS];
#endif
    iar_compexVector *outFrame[IAR_MAX_CHANNELS];
    float hopTempFloatVectors[IAR_NUMTEMPVECTORS][IAR_HOPSIZE+1];
    float *channelTempFloatVectors[IAR_NUMTEMPVECTORS];
    float *bandTempFloatVectors[IAR_NUMTEMPVECTORS];
    float *lastRoundLastEnergies;
    float crossTermMatrixRe[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float crossTermMatrixIm[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    int collectCounter;
    float epsilon;
    int numBands;
    float PASMax;
    float floatAES;
    float PasCurveShift, PasCurveSlope;
} iar_phaseAligner;

int iar_phaseAlignInit( void **handle,  /* returns -1 if an error occured */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                     float **dmxMtx, 
#else
                     float ***dmxMtx, 
#endif
#if IAR
                     float ***dmxMtx2,
#endif
                     int numInChans, 
                     int numOutChans, 
                     int numBands, 
                     float eqLimitMax, 
                     float eqLimitMin);

void iar_phaseAlignProcess(void *handle, const float **in_QMF_real, const float **in_QMF_imag, float **out_QMF_real, float **out_QMF_imag, FILE *fpR3T);
#if IAR
void iar_phaseAlignProcess3D (void *handle, const float **in_QMF_real, const float **in_QMF_imag, float **out_QMF_real, float **out_QMF_imag, FILE * fpR3T);
#endif
void iar_phaseAlignClose(void *handle);

int iar_phaseAlignSetPAS(int PAS, void *handle);
int iar_phaseAlignSetAES(int AES, void *handle);

void iar_phaseAlignSetDmxMtx( iar_phaseAligner *h, 
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                          float **dmxMtx);
#else
                          float ***dmxMtx, float ***dmxMtx2);
#endif

#endif /* _FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_ */
