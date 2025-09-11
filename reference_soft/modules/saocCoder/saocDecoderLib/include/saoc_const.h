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

#ifndef __SAOC_CONST_H__
#define __SAOC_CONST_H__

#ifndef __MIN_MAX_H__
#define __MIN_MAX_H__
#endif

#define SAOC_AVOID_NEGATIVE_IOCS
#define SAOC_PRINT_INFO
#define SAOC_DEC_APP_VERSION             "1.0"

#define SAOC_EPSILON                     1e-9f
#define SAOC_ABS_THR                     SAOC_EPSILON 
#define SAOC_PI                          3.14159265358979f

#define SAOC_MAX_NUM_BINS   28


#define SAOC_MAX_OBJECTS                 128
#define SAOC_MAX_DMX_CHANNELS            22    

#define SAOC_MAX_GROUP_OBJECTS           32
#define SAOC_MAX_GROUP_DMX_CHANNELS      16

#define SAOC_MAX_RENDER_CHANNELS         32
#define SAOC_MAX_LFE_CHANNELS            4


/*#define SAOC_MAX_CHANNELS                max(SAOC_MAX_OBJECTS, max(SAOC_MAX_DMX_CHANNELS, SAOC_MAX_RENDER_CHANNELS))*/
#define SAOC_MAX_CHANNELS                2*SAOC_MAX_RENDER_CHANNELS
#define SAOC_MAX_NUM_DECORRELATORS       11

#define SAOC_MAX_PARAM_SETS              10
#define SAOC_MAX_DECORRELATION            4

#define SAOC_MAX_NUM_ELEVATIONS          1
#define SAOC_MAX_PARAM_BANDS             SAOC_MAX_NUM_BINS
#define SAOC_MAX_PARAMETER_BANDS         SAOC_MAX_NUM_BINS

#define SAOC_MAX_NUM_QMF_BANDS           128
#define SAOC_QMFBANDS                    64

#define SAOC_MAX_PARAMETER_SETS          8

#define SAOC_HYBRID_BANDS                ( SAOC_QMFBANDS - 3 + 10 )

#define SAOC_MAX_HYBRID_BANDS            ( SAOC_MAX_NUM_QMF_BANDS - 3 + 16 )
#define SAOC_MAX_TIME_SLOTS              ( 72 )

#define SAOC_QMF_BANDS_TO_HYBRID             ( 3 )  
#define SAOC_MAX_HYBRID_ONLY_BANDS_PER_QMF   ( 8 )
#define SAOC_PROTO_LEN                       ( 13 )
#define SAOC_BUFFER_LEN_LF                   (   SAOC_PROTO_LEN - 1       + SAOC_MAX_TIME_SLOTS )
#define SAOC_BUFFER_LEN_HF                   ( ( SAOC_PROTO_LEN - 1 ) / 2 + SAOC_MAX_TIME_SLOTS )
#define SAOC_HYBRID_FILTER_DELAY             ( ( SAOC_PROTO_LEN - 1 ) / 2 )

#define SAOC_SYSTEM_DELAY_HQ             ( 961 )   /* 320 samples artifical delay removed */
#define SAOC_SYSTEM_DELAY_LD             ( 256 )
#define SAOC_SYSTEM_DELAY_LP             ( 1281+320 )

#define SAOC_QMF_DELAY_ANA               ( 320)
#define SAOC_QMF_DELAY_SYN               ( 257 )
#define SAOC_QMF_DELAY_HYB               ( 384 )
#define SAOC_QMF_DELAY_ANA_LD            ( 160 )
#define SAOC_QMF_DELAY_SYN_LD            ( 96 )

#define SAOC_PC_FILTERLENGTH             ( 11 )
#define SAOC_PC_FILTERDELAY              ( ( SAOC_PC_FILTERLENGTH - 1 ) / 2 )
#define SAOC_PC_NUM_BANDS                ( 8 )
#define SAOC_PC_NUM_HYB_BANDS            ( SAOC_PC_NUM_BANDS - 3 + 10 )

#define SAOC_MAX_M_INPUT                 ( SAOC_MAX_DMX_CHANNELS * 2 + 1 )
#define SAOC_MAX_M_OUTPUT                SAOC_MAX_RENDER_CHANNELS 

#define SAOC_MAX_EXT_CNT                  (16)

#define SAOC_DECODE_BITSTREAM             0
#define SAOC_FLUSH_DECODER                1
#define SAOC_END_DECODING                 2

typedef enum {
  DEFAULT = 0,
  KEEP = 1,
  INTERPOLATE = 2,
  FINECOARSE = 3
} DATA_MODE;

typedef struct {
    int                     bsFramingType;
    int                     bsNumParamSets;
    int                     bsParamSlots[SAOC_MAX_PARAM_SETS];
} FRAMINGINFO;

typedef enum {

  t_CLD,
  t_ICC,
  t_CPC,
  t_OLD,
  t_IOC,
  t_NRG

} DATA_TYPE;

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


#endif
