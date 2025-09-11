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

#ifndef __HUFF_NODES_H__
#define __HUFF_NODES_H__

typedef struct {

  const int nodeTab[39][2];

} HUFF_RES_NODES;

typedef struct {

  const int nodeTab[30][2];

} HUFF_CLD_NOD_1D;

typedef struct {

  const int nodeTab[ 7][2];

} HUFF_ICC_NOD_1D;

typedef struct {

  const int nodeTab[50][2];

} HUFF_CPC_NOD_1D;

typedef struct {

  const int nodeTab[15][2];

} HUFF_OLD_NOD_1D;

typedef struct {

  const int nodeTab[63][2];

} HUFF_NRG_NOD_1D;


typedef struct {

  const int lav3[15][2];
  const int lav5[35][2];
  const int lav7[63][2];
  const int lav9[99][2];

} HUFF_CLD_NOD_2D;

typedef struct {

  const int lav1[ 3][2];
  const int lav3[15][2];
  const int lav5[35][2];
  const int lav7[63][2];

} HUFF_ICC_NOD_2D;

typedef struct {

  const int lav3 [ 15][2];
  const int lav6 [ 48][2];
  const int lav9 [ 99][2];
  const int lav12[168][2];

} HUFF_CPC_NOD_2D;

typedef struct {

  const int lav3 [ 15][2];
  const int lav6 [ 48][2];
  const int lav9 [ 99][2];
  const int lav12[168][2];

} HUFF_OLD_NOD_2D;

typedef struct {

  const int lav3 [ 15][2];
  const int lav5 [ 35][2];
  const int lav7 [ 63][2];
  const int lav9 [ 99][2];

} HUFF_NRG_NOD_2D_df;

typedef struct {

  const int lav3 [ 15][2];
  const int lav6 [ 48][2];
  const int lav9 [ 99][2];
  const int lav12[168][2];

} HUFF_NRG_NOD_2D_dt;

typedef struct {

  HUFF_NRG_NOD_2D_df df[2];
  HUFF_NRG_NOD_2D_dt dt[2];
  HUFF_NRG_NOD_2D_df dp[2];

} HUFF_NRG_NOD_2D;

typedef struct {

  HUFF_CLD_NOD_1D h1D[3];
  HUFF_CLD_NOD_2D h2D[3][2];

} HUFF_CLD_NODES;

typedef struct {

  HUFF_ICC_NOD_1D h1D[3];
  HUFF_ICC_NOD_2D h2D[3][2];

} HUFF_ICC_NODES;

typedef struct {

  HUFF_CPC_NOD_1D h1D[3];
  HUFF_CPC_NOD_2D h2D[3][2];

} HUFF_CPC_NODES;

typedef struct {

  HUFF_OLD_NOD_1D h1D[3];
  HUFF_OLD_NOD_2D h2D[3][2];

} HUFF_OLD_NODES;

typedef struct {

  HUFF_NRG_NOD_1D h1D[3];
  HUFF_NRG_NOD_2D h2D;

} HUFF_NRG_NODES;


typedef struct {

  const int cld[30][2];
  const int icc[ 7][2];
  const int cpc[25][2];
  const int old[15][2];
  const int nrg[63][2];

} HUFF_PT0_NODES;


typedef struct {

  const int nodeTab[3][2];

} HUFF_LAV_NODES;

#endif
