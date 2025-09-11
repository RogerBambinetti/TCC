/*******************************************************************************
This software module was originally developed by

Coding Technologies, Fraunhofer IIS, Philips

and edited by

-

in the course of development of ISO/IEC 23003 for reference purposes and its
performance may not have been optimized. This software module is an
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives
You a royalty-free, worldwide, non-exclusive, copyright license to copy,
distribute, and make derivative works of this software module or modifications
thereof for use in implementations of ISO/IEC 23003 in products that satisfy
conformance criteria (if any). Those intending to use this software module in
products are advised that its use may infringe existing patents. ISO/IEC have no
liability for use of this software module or modifications thereof. Copyright is
not released for products that do not conform to audiovisual and image-coding
related ITU Recommendations and/or ISO/IEC International Standards.

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:
Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Coding Technologies, Fraunhofer IIS,
Philips own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Coding Technologies, Fraunhofer
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

/* sac_decor.h */

#ifndef SAOC_DECOR_H
#define SAOC_DECOR_H

#include "saoc_const.h"
#include "qmflib_hybfilter.h"
#include "cicp2geometry.h"


struct SAOC_DECORR_DEC {

    int            decorr_seed;
    int            numbins;
    int            partiallyComplex;
    int            lowDelay;
 
    struct DECORR_FILTER_INSTANCE  *Filter[SAOC_MAX_HYBRID_BANDS];
    struct DUCKER_INSTANCE         *ducker;
};

typedef struct SAOC_DECORR_DEC *SAOC_HANDLE_DECORR_DEC;

/****************************************************************
  Functions
 ****************************************************************/

int   SAOC_SpatialDecDecorrelateCreate(SAOC_HANDLE_DECORR_DEC *hDecorrDec,
                                  int  maxNumHybridBands,
                                  int  partiallyComplex);

int SAOC_SpatialDecDecorrelateInit(SAOC_HANDLE_DECORR_DEC  hDecorrDec,
                             int  subbands,
                             int  seed,
                             int  decorrType,
                             int  decorrConfig,
                             QMFLIB_HYBRID_FILTER_MODE hybridMode,
                             int  lowDelay);

void   SAOC_SpatialDecDecorrelateDestroy(SAOC_HANDLE_DECORR_DEC *hDecorrDec, int nHybridBands);
                                                      

int   SAOC_SpatialDecDecorrelateApply(SAOC_HANDLE_DECORR_DEC hDecorrDec,
                                      float InputReal[SAOC_MAX_HYBRID_BANDS],
                                      float InputImag[SAOC_MAX_HYBRID_BANDS],
                                      float OutputReal[SAOC_MAX_HYBRID_BANDS],
                                      float OutputImag[SAOC_MAX_HYBRID_BANDS],
                                      int   startHybBand
                              );

int SAOC_GetHybridSubbands(int qmfSubbands, QMFLIB_HYBRID_FILTER_MODE hybridMode, int LdMode);


void SAOC_DecorrMpreMpostInit(unsigned int DecorrelationLevel,
                                unsigned int numOutCh,
                                unsigned int numLFEs,
                                int outCICPIndex,
                                CICP2GEOMETRY_CHANNEL_GEOMETRY* saocOutputGeometryInfo,
                                unsigned int *numDecorrelators,
                                float Mpre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Mpost[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                                float Hproto[SAOC_MAX_RENDER_CHANNELS][2*SAOC_MAX_RENDER_CHANNELS]);


#endif /*SAOC_DECOR_H*/
