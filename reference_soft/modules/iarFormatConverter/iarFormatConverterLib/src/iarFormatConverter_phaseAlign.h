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


#ifndef _IAR_FORMATCONVERTER_PHASE_ALIGN_H_
#define _IAR_FORMATCONVERTER_PHASE_ALIGN_H_

#ifndef FORMATCONVERTER_LOWCOMPLEXITY

#define IAR_MAX_CHANNELS 24
#define IAR_HOPSIZE 32
#define IAR_NUMTEMPVECTORS 4 /* For memory allocation of temporary vectors that are used in the processing */

#ifndef RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
#define RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
#endif

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
#endif
    int dmxMtxNonZero[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float eqLimitMax;
    float eqLimitMin;
    iar_compexVector Cx[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float *f_Cx[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector P[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector M[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector M_cmp_prev[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    int CxRelevant[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    iar_compexVector *inWindow[IAR_MAX_CHANNELS];
    iar_compexVector *outWindow[IAR_MAX_CHANNELS];
    iar_compexVector *inFrame[IAR_MAX_CHANNELS];
    iar_compexVector *outFrame[IAR_MAX_CHANNELS];
    iar_compexVector *nextFirstHalfWindow[IAR_MAX_CHANNELS];
    iar_compexVector *nextOutFrame[IAR_MAX_CHANNELS];
    float hopTempFloatVectors[IAR_NUMTEMPVECTORS][IAR_HOPSIZE+1];
    float *channelTempFloatVectors[IAR_NUMTEMPVECTORS];
    float *bandTempFloatVectors[IAR_NUMTEMPVECTORS];
    float *lastRoundLastEnergies;
    float crossTermMatrixRe[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    float crossTermMatrixIm[IAR_MAX_CHANNELS][IAR_MAX_CHANNELS];
    int collectCounter;
    float epsilon;
    int numBands;
#ifdef RM0_3D_BUGFIX_FORMATCONVERTER_ONSETSTABILITY
    float E_prev_thdiff[IAR_MAX_CHANNELS][IAR_MAX_BANDS];
#endif
} iar_phaseAligner;

void iar_phaseAlignInit( void **handle, 
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

void iar_phaseAlignSetDmxMtx( iar_phaseAligner *h, 
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
                          float **dmxMtx);
#else
                          float ***dmxMtx);
#endif

void iar_phaseAlignProcess(void *handle, const float **in_QMF_real, const float **in_QMF_imag, float **out_QMF_real, float **out_QMF_imag);
void iar_phaseAlignClose(void *handle);

#endif /* #ifndef FORMATCONVERTER_LOWCOMPLEXITY */

#endif /* _FORMATCONVERTER_PHASE_ALIGN_H_ */
