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

#ifndef __SAC_NLC_ENC_H__
#define __SAC_NLC_ENC_H__


#ifdef __cplusplus
extern "C"
{
#endif


#include "saoc_stream.h"
#include "saoc_types.h"

#include "saoc_types.h" 
#include "saoc_bitinput.h" 
#include "const.h" 

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

typedef enum {

  BACKWARDS = 0x0,
  FORWARDS  = 0x1

} DIRECTION;


typedef enum
{

  DIFF_FREQ = 0x0,
  DIFF_TIME = 0x1

} DIFF_TYPE;


typedef enum
{

  HUFF_1D = 0x0,
  HUFF_2D = 0x1

} CODING_SCHEME;

typedef enum
{

  FREQ_PAIR = 0x0,
  TIME_PAIR = 0x1

} PAIRING;

#define PAIR_SHIFT   4
#define PAIR_MASK  0xf

#define PBC_MIN_BANDS 5

#define MAX_OBJ    32
#define MAX_COR   496   /* maximum number of correlations = (MAX_OBJ*(MAX_OBJ-1))/2 */
#define MAXPARAM    5   /* maximum number of parameter vectors                      */
#define MAXSETS     7   /* maximum number of data sets per parameter vector         */
#define MAXBANDS   28   /* maximum number of frequency bands                        */

#ifndef MAX_NUM_BINS
#define MAX_NUM_BINS 28
#endif

#ifndef MAX_OBJECTS
#define MAX_OBJECTS 32
#endif

int EcDataPairEnc_SAOC( Stream*    strm,
                   int        aaInData[][max(MAX_NUM_BINS,MAX_OBJECTS)],
                   int        aHistory[max(MAX_NUM_BINS,MAX_OBJECTS)],
                   DATA_TYPE  data_type,
                   int        setIdx,
                   int        startBand,
                   int        dataBands,
                   int        pair_flag,
                   int        coarse_flag,
                   int        independency_flag);


#ifdef __cplusplus
}
#endif


#endif
