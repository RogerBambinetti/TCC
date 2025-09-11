/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or 
products claiming conformance to the ISO/IEC 23008-3 standard and which 
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 1996, 1999.

*************************************************************************/
 
#ifndef _reorderspec_h_
#define _reorderspec_h_

#include "allHandles.h"
#include "bitfct.h"

/*** T_HCR ****/

#define LCW_CONCEALMENT_PATCH
#define VCB11_CONCEALMENT_PATCH
#define COMPLETION_CONCEALMENT_PATCH

HANDLE_HCR     CreateHcrInfo ( void );

void DeleteHcrInfo (HANDLE_HCR hHcrInfo) ;

unsigned short GetLenOfLongestCw ( const HANDLE_HCR hHcrInfo );

unsigned short GetLenOfSpecData ( const HANDLE_HCR hHcrInfo );

unsigned short GetReorderStatusFlag ( const HANDLE_HCR hHcrInfo );

void ReadLenOfLongestCw ( HANDLE_HCR         hHcrInfo, 
                          HANDLE_RESILIENCE  hResilience,
                          HANDLE_BUFFER      hVm,
                          HANDLE_ESC_INSTANCE_DATA     hEscInstanceData );

void ReadLenOfSpecData ( HANDLE_HCR         hHcrInfo, 
                         HANDLE_RESILIENCE  hResilience,
                         HANDLE_BUFFER      hVm, 
                         HANDLE_ESC_INSTANCE_DATA     hEscInstanceData);

void ReadEntryPoints ( HANDLE_HCR         hHcrInfo, 
                       HANDLE_RESILIENCE  hResilience,
                       HANDLE_BUFFER      hVm, 
                       HANDLE_ESC_INSTANCE_DATA     hEscInstanceData );
/*** T_HCR end ***/

void DecodedBitCnt ( HANDLE_HCR hHcrInfo, unsigned short codewordLen );

unsigned short GetDecodedBitCnt ( const HANDLE_HCR hHcrInfo );

void CheckDecodingProgress ( HANDLE_BUFFER      hSpecData,
                             HANDLE_ESC_INSTANCE_DATA     hEscInstanceData,
                             HANDLE_HCR         hHcrInfo,
                             HANDLE_RESILIENCE  hResilience );

HANDLE_BUFFER GetNonPcwBufPtrHdl ( const HANDLE_HCR     hHcrInfo );

void           InitHcr ( HANDLE_BUFFER hSpecData, 
                         HANDLE_HCR    hHcrInfo );

unsigned char  GetCurrentMaxCodewordLen ( unsigned short   maxCWLen, 
                                          const HANDLE_HCR hHcrInfo );

void ReadNonPcwsNew ( unsigned short*    codewordLen, 
                      int*               qp, 
                      Hcb*               hcb, 
                      Huffman*           hcw, 
                      unsigned short     step,
                      unsigned short     table,
                      unsigned short*    codewordsInSet,
                      HANDLE_RESILIENCE  hResilience,
                      HANDLE_HCR         hHcrInfo,
                      HANDLE_ESC_INSTANCE_DATA     hEscInstanceData,
                      HANDLE_CONCEALMENT hConcealment );

void ReadPcws ( unsigned short*    codewordLen, 
                int*               qp,
                Hcb*               hcb, 
                Huffman*           hcw, 
                unsigned short     codebook, 
                unsigned short     maxCWLen, 
                unsigned short     step, 
                HANDLE_RESILIENCE  hResilience,
                HANDLE_BUFFER      hSpecData,
                HANDLE_HCR         hHcrInfo, 
                HANDLE_ESC_INSTANCE_DATA     hEscInstanceData,
                HANDLE_CONCEALMENT hConcealment );

void ReadNonPcws ( unsigned short*    codewordLen, 
                   int*               qp, 
                   Hcb*               hcb, 
                   Huffman*           hcw, 
                   unsigned short     step,
                   unsigned short     table,
                   HANDLE_RESILIENCE  hResilience,
                   HANDLE_HCR         hHcrInfo,
                   HANDLE_ESC_INSTANCE_DATA     hEscInstanceData,
                   HANDLE_CONCEALMENT hConcealment ); 

void ReorderSpecDecPCWFinishedCheck ( unsigned short     maxCWLen,
                                      HANDLE_BUFFER      hSpecData,
                                      HANDLE_RESILIENCE  hResilience,
                                      HANDLE_HCR         hHcrInfo, 
                                      HANDLE_ESC_INSTANCE_DATA     hEPInfo );

void SetNrOfCodewords ( unsigned short nrOfCodewords,
                        const HANDLE_HCR     hHcrInfo  );

void Vcb11ConcealmentPatch ( char*             calledFrom,
                             unsigned short    codebook,
                             int               lavInclEsc,
                             int*              qp,
                             unsigned short    step,
                             HANDLE_RESILIENCE hResilience );

void LcwConcealmentPatch ( char*          calledFrom,
                           unsigned short codewordLen,
                           unsigned short maxCWLen,
                           int*           qp,
                           unsigned short step,
                           HANDLE_HCR     hHcrInfo );

#endif /* _reorderspec_h_  */
