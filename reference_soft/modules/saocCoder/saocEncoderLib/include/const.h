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

#ifndef __MIN_MAX_H__
#define __MIN_MAX_H__

#define EPSILON                     1e-9f
#define ABS_THR                     EPSILON /* ( 1e-9f * 32768 * 32768 )*/

#define MAX_NUM_BINS                28
#define MAX_OBJECTS                 32
#define MAX_PARAM_SETS              8
#define MAX_CHANNELS                22
#define MAX_PARAM_BANDS             MAX_NUM_BINS
#define MAX_NUM_INPUTCHANNELS       32
#define MAX_NUM_OUTPUTCHANNELS      22 

#define MAX_NUM_QMF_BANDS           128
#define QMFBANDS                    64
#define TIMESLOTS                   32
#define MAX_PARAMETER_BANDS         MAX_NUM_BINS


/* #define MAX_HYBRID_BANDS            ( MAX_NUM_QMF_BANDS - 3 + 10 )*/
/* #define HYBRID_BANDS                ( QMFBANDS - 3 + 10 )*/

/* #define MAX_TIME_SLOTS                  ( 72 )*/

#define QMF_BANDS_TO_HYBRID             ( 3 )  /* 3 bands are filtered again in "28 bands" case */
/* #define MAX_HYBRID_ONLY_BANDS_PER_QMF   ( 8 )
#define PROTO_LEN                       ( 13 )
#define BUFFER_LEN_LF                   (   PROTO_LEN - 1       + MAX_TIME_SLOTS )
#define BUFFER_LEN_HF                   ( ( PROTO_LEN - 1 ) / 2 + MAX_TIME_SLOTS )*/
#define HYBRID_FILTER_DELAY             ( ( PROTO_LEN - 1 ) / 2 )

#define MAX_NUM_BINS   28

typedef enum {
  DEFAULT = 0,
  KEEP = 1,
  INTERPOLATE = 2,
  FINECOARSE = 3
} DATA_MODE;

typedef struct {
    int                     bsFramingType;
    int                     bsNumParamSets;
    int                     bsParamSlots[MAX_PARAM_SETS];
} FRAMINGINFO;

typedef enum {

  t_CLD,
  t_ICC,
  t_CPC,
  t_OLD,
  t_IOC,
  t_NRG,
  t_DMG

} DATA_TYPE;



/*****************************************/




#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif



#endif
