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

#include "saoc_interpolate.h"
#include "saoc_kernels.h"

void saoc_interpolate(int nrows,
                 int ncols,
                 int nNumBins,
                 int nParamSets,
                 int nParamSetsMod,
                 int nHybridBands,
                 int LdMode,
                 int *pParamSlot,
                 float M[SAOC_MAX_PARAMETER_SETS][SAOC_MAX_PARAMETER_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS],
                 float R[SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_RENDER_CHANNELS]) {
  int irow, icol;
  int iset, ipar, ihyb, islot, islot1, islot2;
  int isetOld, isetNew;
  float alfa;
  int kernels[SAOC_MAX_HYBRID_BANDS][2];

  SAOC_GetKernels(nNumBins, nHybridBands, LdMode, kernels);

  for (iset = 1; iset <= nParamSetsMod; iset++) { 
    islot1 = pParamSlot[iset-1];
    islot2 = pParamSlot[iset];
    for (islot = islot1 + 1; islot <= islot2; islot++) {
      if (iset == nParamSetsMod && nParamSets != nParamSetsMod) { 
        alfa = 0.0f;
      }
      else {
        alfa = (float)(islot - islot1)/(float)(islot2 - islot1);
      }

      for (ihyb = 0; ihyb < nHybridBands; ihyb++) {
        ipar = kernels[ihyb][0];
        isetOld = iset-1;
        isetNew = iset;

        for (irow = 0; irow < nrows; irow++) {
          for (icol = 0; icol < ncols; icol++) {
            R[islot][ihyb][irow][icol] = (1.0f-alfa)*M[isetOld][ipar][irow][icol] +
                                               alfa *M[isetNew][ipar][irow][icol];
          }
        }

      }
    }
  }
}
