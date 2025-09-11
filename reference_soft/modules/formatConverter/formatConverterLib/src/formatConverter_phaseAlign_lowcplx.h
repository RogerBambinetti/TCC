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

#ifndef _FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_
#define _FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_


#define MAX_CHANNELS 29
#define HOPSIZE 32
#define NUMTEMPVECTORS 4 /* For memory allocation of temporary vectors that are used in the processing */

#ifndef MAX_BANDS
#define MAX_BANDS 77
#endif

typedef struct {
    float *re;
    float *im;
} compexVector;

typedef struct {
    int numInChans,numOutChans;
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    float dmxMtx[MAX_CHANNELS][MAX_CHANNELS];
#else
    float dmxMtx[MAX_BANDS][MAX_CHANNELS][MAX_CHANNELS];
#endif

    int dmxMtxNonZero[MAX_CHANNELS][MAX_CHANNELS];
    float eqLimitMax;
    float eqLimitMin;
    compexVector Cx[MAX_CHANNELS][MAX_CHANNELS];
    compexVector CxTimeOverlap[MAX_CHANNELS][MAX_CHANNELS];
    float CxDiagPrev[MAX_CHANNELS][77];
    float *f_Cx[MAX_CHANNELS][MAX_CHANNELS];
    compexVector P[MAX_CHANNELS][MAX_CHANNELS];
    compexVector M[MAX_CHANNELS][MAX_CHANNELS];
    compexVector M_current[MAX_CHANNELS][MAX_CHANNELS];
    compexVector M_cmp_prev[MAX_CHANNELS][MAX_CHANNELS];
    int CxRelevant[MAX_CHANNELS][MAX_CHANNELS];
    compexVector *inFrame[MAX_CHANNELS];
    compexVector *prevInFrame[MAX_CHANNELS];
    compexVector *outFrame[MAX_CHANNELS];
    float hopTempFloatVectors[NUMTEMPVECTORS][HOPSIZE+1];
    float *channelTempFloatVectors[NUMTEMPVECTORS];
    float *bandTempFloatVectors[NUMTEMPVECTORS];
    float *lastRoundLastEnergies;
    float crossTermMatrixRe[MAX_CHANNELS][MAX_CHANNELS];
    float crossTermMatrixIm[MAX_CHANNELS][MAX_CHANNELS];
    int collectCounter;
    float epsilon;
    int numBands;
    float PASMax;
    float floatAES;
    float PasCurveShift, PasCurveSlope;
} phaseAligner;

int phaseAlignInit( void **handle,  /* returns -1 if an error occured */
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                     float **dmxMtx, 
#else
                     float ***dmxMtx, 
#endif
                     int numInChans, 
                     int numOutChans, 
                     int numBands, 
                     float eqLimitMax, 
                     float eqLimitMin);

void phaseAlignProcess(void *handle, const float **in_QMF_real, const float **in_QMF_imag, float **out_QMF_real, float **out_QMF_imag);
void phaseAlignClose(void *handle);

int phaseAlignSetPAS(int PAS, void *handle);
int phaseAlignSetAES(int AES, void *handle);

void phaseAlignSetDmxMtx( phaseAligner *h, 
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                          float **dmxMtx);
#else
                          float ***dmxMtx);
#endif

#endif /* _FORMATCONVERTER_PHASE_ALIGN_LOWCPLX_H_ */
