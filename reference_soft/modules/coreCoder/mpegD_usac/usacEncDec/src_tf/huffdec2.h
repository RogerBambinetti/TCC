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

Copyright (c) ISO/IEC 1996.

*************************************************************************/
#ifndef _HUFFDEC2_H_
#define _HUFFDEC2_H_

#include "allHandles.h"
#include "all.h"
#include "block.h"
#include "tns.h"

#include "tf_mainStruct.h"       /* typedefs */

void deinterleave ( void                    *inptr,  /* formerly pointer to type int */
                    void                    *outptr, /* formerly pointer to type int */
                    short                   length,  /* sizeof base type of inptr and outptr in chars */
                    const short             nsubgroups[], 
                    const int               ncells[], 
                    const short             cellsize[],
                    int                     nsbk,
                    int                     blockSize,
                    const HANDLE_RESILIENCE hResilience,
                    short                   ngroups );

void          clr_tns ( Info*           info, 
                        TNS_frame_info* tns_frame_info );

void          getgroup ( Info*             info, 
                         unsigned char*    group,
                         HANDLE_RESILIENCE hResilience, 
                         HANDLE_ESC_INSTANCE_DATA    hEPInfo,
                         HANDLE_BUFFER     hVm  );

int getics ( HANDLE_AACDECODER        hDec,
             Info*                    info, 
             int                      common_window, 
             WINDOW_SEQUENCE*         win, 
             WINDOW_SHAPE* 	      wshape, 
             unsigned char*           group, 
             unsigned char*           max_sfb, 
             PRED_TYPE                pred_type, 
             int*                     lpflag, 
             int*                     prstflag, 
             byte*                    cb_map, 
             double*                   coef, 
             int                      max_spec_coefficients,
             short*                   global_gain, 
             short*                   factors,
             NOK_LT_PRED_STATUS*      nok_ltp, 
             TNS_frame_info*          tns,
             HANDLE_BSBITSTREAM       gc_streamCh, 
             enum AAC_BIT_STREAM_TYPE bitStreamType,
             HANDLE_RESILIENCE        hResilience,
             HANDLE_BUFFER            hHcrSpecData,
             HANDLE_HCR               hHcrInfo,
             HANDLE_ESC_INSTANCE_DATA           hEPInfo,
             HANDLE_CONCEALMENT       hConcealment,
             QC_MOD_SELECT            qc_select,
             HANDLE_BUFFER            hVm,
             /* needful for ER_AAC_Scalable */
             int                      extensionLayerFlag,   /* signaled an extension layer */
             int                      er_channel            /* 0:left 1:right channel      */
#ifdef I2R_LOSSLESS
             , int*                   sls_quant_mono_temp
#endif
             );


int           getmask ( Info*             info, 
                        unsigned char*    group, 
                        unsigned char     max_sfb, 
                        unsigned char*    mask,
                        HANDLE_RESILIENCE hResilience, 
                        HANDLE_ESC_INSTANCE_DATA    hEPInfo,
                        HANDLE_BUFFER     hVm  );

unsigned short HuffSpecKernelPure ( int*                     qp, 
                                    Hcb*                     hcb, 
                                    Huffman*                 hcw, 
                                    int                      step,
                                    HANDLE_HCR               hHcrInfo,
                                    HANDLE_RESILIENCE        hResilience,
                                    HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
                                    HANDLE_CONCEALMENT       hConcealment,
                                    HANDLE_BUFFER            hSpecData );

unsigned char Vcb11Used ( unsigned short ); 

#endif  /* #ifndef _HUFFDEC2_H_ */
