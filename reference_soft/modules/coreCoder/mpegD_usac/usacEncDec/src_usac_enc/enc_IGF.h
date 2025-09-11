/***********************************************************************************

 This software module was originally developed by

 Fraunhofer IIS

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

 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.

 This copyright notice must be included in all copies or derivative works.

 Copyright (c) ISO/IEC 2013.

 ***********************************************************************************/

#ifndef _ENC_IGF_
#define _ENC_IGF_

#define RM1_3D_BUGFIX_IGF_01
#define RM1_3D_BUGFIX_IGF_02
#define RM1_3D_BUGFIX_IGF_03
#define RM1_3D_BUGFIX_IGF_04
#define RM1_3D_BUGFIX_IGF_05
#define RM1_3D_BUGFIX_IGF_06
#define RM1_3D_BUGFIX_IGF_07
#define RM1_3D_BUGFIX_IGF_08

int IGF_level_enc(
          HANDLE_AACBITMUX  bitmux,
          int               indepFlag,
          int               igFAllZero,
          int              *igFLevel,
          int              *igFPrevLevel,
          int              *igFT,
          int              *igFD,
          int              *igFPrevWindow,
          int               isShortWindow,
          int               igFStartSfbLB,
          int               igFStopSfbLB
);

int IGF_data_enc(
          HANDLE_AACBITMUX  bitmux,
          int              *igFTileIdx
#ifdef RM1_3D_BUGFIX_IGF_05
          ,int              nT
#endif
#ifdef RM1_3D_BUGFIX_IGF_02
         ,int               bUsacIndependencyFlag
#endif
);

#ifdef RM1_3D_BUGFIX_IGF_05
int IGF_getNTiles_enc(
          int               samplingRate,
          int               blockSize,
          int               igfSfbStart,
          int               igfSfbStop,
          int               igFUseHighRes,
          int               isShortWin,
          const int        *swb_offset
);
#endif

void IGF_encode(
          double           *pSpectrum,
          int              *swb_offset,
          int               isShortWindow,
          int               igFUseHighRes,
          int               igFStartSfbLB,
          int               igFStopSfbLB,
          int              *igFTileIdx,
          int              *igFAllZero,
          int              *igFLevel
);
#endif