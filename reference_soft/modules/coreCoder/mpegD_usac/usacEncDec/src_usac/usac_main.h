/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/

/* 28-Aug-1996  NI: added "NO_SYNCWORD" to enum TRANSPORT_STREAM */

#ifndef _USAC_H_INCLUDED
#define _USAC_H_INCLUDED


#include "block.h"
#include "ntt_conf.h"
#include "usac_mainStruct.h"       /* structs */
#include "td_frame.h"
#include "dec_IGF.h"

void usacBuffer2freq( double           p_in_data[],      /* Input: Time signal              */
                  double           p_out_mdct[],     /* Output: MDCT cofficients        */
                  double           p_overlap[],
                  WINDOW_SEQUENCE  windowSequence,
                  WINDOW_SHAPE     wfun_select,      /* offers the possibility to select different window functions */
                  WINDOW_SHAPE     wfun_select_prev,
                  int              nlong,            /* shift length for long windows   */
                  int              nshort,           /* shift length for short windows  */
                  Mdct_in          overlap_select,   /* select mdct input *TK*          */
                  int              num_short_win);   /* number of short windows to
                                                        transform                       */


void usac_imdct_float(float in_data[], float out_data[],
                      const int transformKernelType,  /* MDCT-IV, MDST-II, MDCT-II, MDST-IV */
                      const int len);
void usac_tw_imdct(float in_data[], float out_data[], int len);
void calc_window( float window[], int len, WINDOW_SHAPE wfun_select);
void calc_window_ratio( float window[], int len, int prev_len, WINDOW_SHAPE wfun_select, WINDOW_SHAPE prev_wfun_select);
void calc_tw_window( float window[], int len, WINDOW_SHAPE wfun_select);

void fd2buffer(float                      p_in_data[],      /* Input: MDCT coefficients                */
               float                      p_out_data[],     /* Output:time domain reconstructed signal */
               float                      p_overlap[],
               WINDOW_SEQUENCE            windowSequence,
               WINDOW_SEQUENCE            windowSequenceLast,
               int                        nlong,            /* shift length for long windows   */
               int                        nshort,           /* shift length for short windows  */
               WINDOW_SHAPE               wfun_select,      /* offers the possibility to select different window functions */
               WINDOW_SHAPE               wfun_select_prev, /* YB : 971113 */
               Imdct_out                  overlap_select,   /* select imdct output *TK*        */
               int                        num_short_win,    /* number of short windows to transform */
               int                        FrameWasTd,
               int                        twMdct,
               float                      sample_pos[],
               float                      tw_trans_len[],
               int                        tw_start_stop[],
               float                      past_sample_pos[],
               float                      past_tw_trans_len[],
               int                        past_tw_start_stop[],
               float                      past_warped_time_sample_vector[],
               const int                  pastAliasingSymmetry, /* left-side boundary condition */
               const int                  currAliasingSymmetry, /* right-side boundary symmetry */
               int                        ApplyFac,
               int                        facPrm[],
               float                      lastLPC[],
               float                      acelpZIR[],
               float                      tbe_synth[],
               HSTEREOLPD_DEC_DATA        stereolpdDecData,
               int                        lpdStereoIndex,
               int                        ch,
               HANDLE_USAC_TD_DECODER     st,
               HANDLE_RESILIENCE          hResilience,
               HANDLE_ESC_INSTANCE_DATA   hEscInstanceData,
               int                        bSplitTransform
);

void  TDAliasing( float                 inputData[],
                  float                 Output[],
                  int                   nlong,
                  int                   nshort,
                  int                   pos);

void
usac_tns_decode_subblock(float *spec,
                         int nbands,
                         const short *sfb_top,
                         int islong,
                         TNSinfo *tns_info,
                         const QC_MOD_SELECT qc_select,
                         const int samplFreqIdx,
                         IGF_CONFIG *igfConfig
);


/* functions from tvqAUDecode.e */


#endif  /* #ifndef _TF_MAIN_H_INCLUDED */
