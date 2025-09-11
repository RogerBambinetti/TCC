/*******************************************************************************
This software module was originally developed by

AT&T, Dolby Laboratories, Fraunhofer IIS, and VoiceAge Corp.

Initial author:

and edited by
Yoshiaki Oikawa     (Sony Corporation),
Mitsuyuki Hatanaka  (Sony Corporation),
Ralph Sperschneider (Fraunhofer IIS)
Ali Nowbakht-Irani  (Fraunhofer IIS)
Markus Werner       (SEED / Software Development Karlsruhe)

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

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:s
Fraunhofer IIS, VoiceAge Corp. grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Fraunhofer IIS, VoiceAge Corp.
own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Fraunhofer IIS, VoiceAge Corp.
will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

Fraunhofer IIS, VoiceAge Corp. retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2008.
$Id: usac_td_dec.c,v 1.19 2011-05-02 09:11:02 mul Exp $
*******************************************************************************/


#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vector_ops.h"
#include "td_frame.h"
#include "allHandles.h"
#include "buffers.h"
#include "usac_port.h"
#include "usac_main.h"
#include "proto_func.h"
#include "usac_tw_tools.h"
#include "usac_tw_defines.h"
#include "re8.h"
#include "usac_allVariables.h"        /* variables */

int fac_decoding( int                      lFac,
                  int                      k,
                  int                     *facPrm,
                  HANDLE_RESILIENCE        hResilience,
                  HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
                  HANDLE_BUFFER            hVm )
{
  int i, iii, n, qn, nk, kv[8];
  long I;
  int nbits_fac=0;

  for (i=0; i<lFac; i+=8){
    /* Unary code for codebook numbers */
    qn = 0;
    while (GetBits (1, TD_FAC_NQ, hResilience, hEscInstanceData, hVm) == 1) {
      qn += 1;
      nbits_fac++;
    }
    if (qn != 0) qn += 1;
    
    nk = 0;
    n = qn;
    if (qn > 4) {
      nk = (qn-3)>>1;
      n = qn - nk*2;
    }
    
    I = GetBits (4*n, TD_FAC_I, hResilience, hEscInstanceData, hVm);

    /* Read 8*nk bits for Voronoi indices (kv[0...7]) */
    for (iii=0; iii<8; iii++) {
      kv[iii] = GetBits (nk, TD_FAC_KV, hResilience, hEscInstanceData, hVm);
    }
    
    RE8_dec(qn, I, kv, &facPrm[k*LFAC_1024+i]);
    
    nbits_fac += 4*qn;
    
  }

  return nbits_fac;
}

void acelp_decoding(int                      codec_mode,
                    int                      fullbandLPD,
                    HTBE_DEC_DATA            tbeDecData,
                    int                      k,
                    int                      nb_subfr,
                    td_frame_data            *td,
                    HANDLE_RESILIENCE        hResilience,
                    HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
                    HANDLE_BUFFER            hVm )
{
  int sfr;
  int kk;
  TBE_FRAME_CLASS tbe_frm_class;

  td->mean_energy[k] = GetBits(2, TD_MEAN_ENERGY, hResilience, hEscInstanceData, hVm);

  for (sfr=0; sfr<nb_subfr; sfr++) {
    kk = k*NB_SUBFR_1024 + sfr;

    if ((sfr==0) || ((nb_subfr==NB_SUBFR_1024) && (sfr==2))) {
        td->acb_index[kk] = GetBits(9, TD_ACB_INDEX, hResilience, hEscInstanceData, hVm);
    } else {
        td->acb_index[kk] = GetBits(6, TD_ACB_INDEX, hResilience, hEscInstanceData, hVm);
    }
    td->ltp_filtering_flag[kk] = GetBits (1, TD_LTP_FILTERING_FLAG, hResilience, hEscInstanceData, hVm);

    switch(codec_mode) {
      case MODE_8k0:
        td->icb_index[kk][0] = GetBits ( 1, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 1, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_8k8:
        td->icb_index[kk][0] = GetBits ( 1, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_9k6:
        td->icb_index[kk][0] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_11k2:
        td->icb_index[kk][0] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 5, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_12k8:
        td->icb_index[kk][0] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_14k4:
        td->icb_index[kk][0] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 9, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_16k:
        td->icb_index[kk][0] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits (13, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      case MODE_18k4:
        td->icb_index[kk][0] = GetBits ( 2, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][1] = GetBits ( 2, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][2] = GetBits ( 2, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][3] = GetBits ( 2, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][4] = GetBits (14, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][5] = GetBits (14, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][6] = GetBits (14, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        td->icb_index[kk][7] = GetBits (14, TD_ICB_INDICES, hResilience, hEscInstanceData, hVm );
        break;
      }

    td->gains[kk] = GetBits(7, TD_GAINS, hResilience, hEscInstanceData, hVm);
  }

  if (fullbandLPD == 1) {
    tbe_frm_class = (k == 0) ? TBE_CLASS_ACELP_FIRST : TBE_CLASS_ACELP_SECOND;
    TBE_data(hVm,
             hResilience,
             hEscInstanceData,
             tbeDecData,
             tbe_frm_class);
  }
}

static void get_tns_lpd(TNS_frame_info           *tns_frame_info,
                        const int                 nsbk,
                        const int                 sbkIdx,
                        const int                 islong,
                        const int                 sfb_per_sbk,
                        HANDLE_RESILIENCE         hResilience,
                        HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                        HANDLE_BUFFER             hVm
) {
  int                       f, t, top, res, res2, compress;
  int                       short_flag, s;
  short                     *sp, tmp, s_mask, n_mask;
  TNSfilt                   *tns_filt;
  TNSinfo                   *tns_info;
  static const short        sgn_mask[] = { 0x2, 0x4, 0x8 };
  static const short        neg_mask[] = { (short) 0xfffc, (short)0xfff8, (short)0xfff0 };

  short_flag = (!islong);
  tns_frame_info->n_subblocks = nsbk;

  for (s = sbkIdx; s < sbkIdx + tns_frame_info->n_subblocks; s++) {
    tns_info = &tns_frame_info->info[s];

    if (!(tns_info->n_filt = GetBits(short_flag ? 1 : 2,
                                     N_FILT,
                                     hResilience,
                                     hEscInstanceData,
                                     hVm))) {
      continue;
    }
    tns_info -> coef_res = res = GetBits(1,
                                         COEF_RES,
                                         hResilience,
                                         hEscInstanceData,
                                         hVm) + 3;

    top = sfb_per_sbk;
    tns_filt = &tns_info->filt[ 0 ];
    for (f = tns_info->n_filt; f > 0; f--) {
      tns_filt->stop_band = top;
      top = tns_filt->start_band = top - GetBits(short_flag ? 4 : 6,
                                                 LENGTH,
                                                 hResilience,
                                                 hEscInstanceData,
                                                 hVm);

      tns_filt->order = GetBits(short_flag ? 3 : 5,
                                ORDER,
                                hResilience,
                                hEscInstanceData,
                                hVm);

      if (tns_filt->order) {
        tns_filt->direction = GetBits(1,
                                      DIRECTION,
                                      hResilience,
                                      hEscInstanceData,
                                      hVm);

        compress = GetBits(1,
                           COEF_COMPRESS,
                           hResilience,
                           hEscInstanceData,
                           hVm);

        res2 = res - compress;
        s_mask = sgn_mask[ res2 - 2 ];
        n_mask = neg_mask[ res2 - 2 ];

        sp = tns_filt -> coef;
        for (t = tns_filt->order; t > 0; t--)  {
          tmp = (short) GetBits(res2,
                                COEF,
                                hResilience,
                                hEscInstanceData,
                                hVm);
          *sp++ = (tmp & s_mask) ? (tmp | n_mask) : tmp;
        }
      }
      tns_filt++;
    }
  }
}

void tcx_decoding(HANDLE_USAC_TD_DECODER    hDec,
                  int                       mode,
                  int                      *quant,
                  int                       k,
                  int                       last_mode,
                  int                       first_tcx_flag,
                  int                      *arithPreviousSize,
                  Tqi2                      arithQ[],
                  td_frame_data            *td,
                  IGF_CONFIG               *igfConfig,
                  HANDLE_RESILIENCE         hResilience,
                  HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                  HANDLE_BUFFER             hVm,
                  int const                 bUsacIndependencyFlag,
                  LTPF_DATA                *ltpfData
) {
  int num_windows;
  int igf_allZero;
  IGF_WIN_TYPE igf_win_type;
  IGF_FRAME_ID igf_frm_id;
  Info *sfbInfo = usac_winmap[EIGHT_SHORT_SEQUENCE];

  if(hDec->bUseNoiseFilling){
    td->tcx_noise_factor[k] = GetBits (LEN_NOISE_FAC,
                                       TD_NOISE_FAC,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm );
  } else {
    td->tcx_noise_factor[k] = 8;
  }

  td->tcx_global_gain[k] = GetBits (LEN_GLOBAL_GAIN_TCX,
                                    TD_GLOBAL_GAIN_TCX,
                                    hResilience,
                                    hEscInstanceData,
                                    hVm );

  if (((1 == hDec->fullbandLPD) && (2 == mode)) || ((0 == hDec->fullbandLPD) && (3 == mode))) {
    ltpfData->data_present = (int)GetBits(1,
                                          LTP_DATA_PRESENT,
                                          hResilience,
                                          hEscInstanceData,
                                          hVm);
    if (ltpfData->data_present == 1) {
      ltpfData->pitch_index = (int)GetBits(9,
                                           LTP_LAG,
                                           hResilience,
                                           hEscInstanceData,
                                           hVm);
      ltpfData->gain_index = GetBits(2,
                                     LTP_COEF,
                                     hResilience,
                                     hEscInstanceData,
                                     hVm);
    } else {
      ltpfData->data_present = 0;
    }
  } else { 
      ltpfData->data_present = 0;
  }

  if ((0 == bUsacIndependencyFlag) && (((1 == hDec->fullbandLPD) && (2 == mode)) || ((0 == hDec->fullbandLPD) && (3 == mode)))) {
    hDec->fdp_data_present = (int)GetBits(1,
                                          PREDICTOR_DATA_PRESENT,
                                          hResilience,
                                          hEscInstanceData,
                                          hVm);
    if (1 == hDec->fdp_data_present) {
      hDec->fdp_spacing_index = (int)GetBits(8,
                                             PREDICTION_USED,
                                             hResilience,
                                             hEscInstanceData,
                                             hVm);
      }
  } else {
    hDec->fdp_data_present = 0;
  }

  if (hDec->igf_active){
    num_windows  = 1;
    igf_win_type = (IGF_WIN_TYPE)(mode + 1);
    igf_frm_id   = (IGF_TCX_LONG == igf_win_type) ? IGF_ID_TCX_LONG : (IGF_FRAME_ID)(mode + 1 + k);
    igf_allZero  = IGF_level(hVm,
                             hResilience,
                             hEscInstanceData,
                             hDec->igfDecData,
                             igfConfig,
                             igf_win_type,
                             igf_frm_id,
                             num_windows,
                             bUsacIndependencyFlag);
    if (!igf_allZero) {
      IGF_data(hVm,
               hResilience,
               hEscInstanceData,
               hDec->igfDecData,
               igfConfig,
               igf_win_type,
               igf_frm_id,
               bUsacIndependencyFlag);
    }
  }

  if (hDec->tns_data_present){
    num_windows = 1;
    get_tns_lpd(&hDec->tns,
                num_windows,
                k,
                0,
                sfbInfo->sfb_per_sbk[0],
                hResilience,
                hEscInstanceData,
                hVm);
  }

  switch (mode) {
    case 1:
      td->tcx_lg[k] = hDec->lDiv;
      break;
    case 2:
      td->tcx_lg[k] = 2*(hDec->lDiv);
      break;
    case 3:
      td->tcx_lg[k] = 4*(hDec->lDiv);
      break;
  }

  td->tcx_lg[k] *= hDec->fac_FB;

  if (!first_tcx_flag){
    if (bUsacIndependencyFlag){
      td->tcx_arith_reset_flag = 1;
    } else {
      td->tcx_arith_reset_flag = GetBits (1,
                                          TD_ARITH_RESET,
                                          hResilience,
                                          hEscInstanceData,
                                          hVm );
    }
  }
  tcxArithDec(td->tcx_lg[k],quant,arithPreviousSize,arithQ,hVm, ((!first_tcx_flag) && td->tcx_arith_reset_flag) );
}

int usac_get_tdcs(HANDLE_USAC_TD_DECODER    hDec,
                  MPEG3DAPROFILELEVEL       mpegh3daProfileLevelIndication,
                  int                      *arithPreviousSize,
                  Tqi2                      arithQ[],
                  td_frame_data            *td,
                  IGF_CONFIG               *igfConfig,
                  HANDLE_RESILIENCE         hResilience,
                  HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                  HANDLE_BUFFER             hVm,
                  int                      *isAceStart,           /* (o): indicator of first TD frame */
                  int                      *short_fac_flag,       /* (o): short fac flag */
                  int                      *isBassPostFilter,     /* (o): bassfilter on/off */
                  int const                 bUsacIndependencyFlag,
                  LTPF_DATA                *ltpfData
) {
  int mode, k;
  int first_tcx_flag;
  int *quant;

  int lastMode = hDec->last_mode;
  int read_FAC_bits = 0;
  int alignBits = 0;

  int tmp, j, nb, qn1, qn2, q_type, skip, skip_avq;
  int nbits_lpc = 0;
  int core_mode_last, fac_data_present;
  int nbDiv = hDec->nbDiv;

  if(hDec->fullbandLPD) {
    hDec->tns_data_present = GetBits(LEN_TNS_ACTIVE,
                                     TNS_DATA_PRESENT,
                                     hResilience,
                                     hEscInstanceData,
                                     hVm);
    if ( hDec->bUseNoiseFilling || hDec->tns_data_present ) {
      hDec->window_shape = GetBits(1,
                                   WINDOW_SHAPE_CODE,
                                   hResilience,
                                   hEscInstanceData,
                                   hVm);
      hDec->max_sfb = GetBits(4,
                              MAX_SFB,
                              hResilience,
                              hEscInstanceData,
                              hVm);
    } else {
      hDec->window_shape = hDec->max_sfb = 0;
    }
  } else {
    hDec->tns_data_present = 0;
  }

  td->core_mode_index = GetBits(LEN_ACELP_CORE_MODE_IDX,
                                TD_CORE_MODE_IDX,
                                hResilience,
                                hEscInstanceData,
                                hVm );

  if(hDec->fullbandLPD) {
    /* Decode mode information */
    mode = GetBits (LEN_MODE_FULLBAND_LPD,
                    TD_MODE,
                    hResilience,
                    hEscInstanceData,
                    hVm );

    if (mode == 4)  {
      td->mod[0] = td->mod[1] = 2;
    }
    else  {
      td->mod[0] =  mode     & 1;
      td->mod[1] = (mode>>1) & 1;
    }

    /* bpf_control_info */
    *isBassPostFilter = 1;
  } else {
    /* Decode mode information */
    mode = GetBits (LEN_MODE,
                    TD_MODE,
                    hResilience,
                    hEscInstanceData,
                    hVm );
    if (mode == 25) {
      td->mod[0] = td->mod[1] = td->mod[2] = td->mod[3] = 3;
    }
    else if (mode == 24) {
      td->mod[0] = td->mod[1] = td->mod[2] = td->mod[3] = 2;
    }
    else {
      if (mode >= 20) {
        mode -= 20;
        td->mod[0] = mode & 1;
        td->mod[1] = (mode>>1) & 1;
        td->mod[2] = td->mod[3] = 2;
      }
      else if (mode >= 16) {
        mode -= 16;
        td->mod[0] = td->mod[1] = 2;
        td->mod[2] = mode & 1;
        td->mod[3] = (mode>>1) & 1;
      }
      else {
        td->mod[0] = mode & 1;
        td->mod[1] = (mode>>1) & 1;
        td->mod[2] = (mode>>2) & 1;
        td->mod[3] = (mode>>3) & 1;
      }
    }

    /* bpf_control_info */
    *isBassPostFilter = GetBits (LEN_BPF_CONTROL,
                                 TD_BPF_CONTROL,
                                 hResilience,
                                 hEscInstanceData,
                                 hVm );
  }

  /* core_mode_last */
  core_mode_last = GetBits(LEN_CORE_MODE_LAST,
                           CORE_MODE_LAST,
                           hResilience,
                           hEscInstanceData,
                           hVm);

  /* fac_data_present */
  fac_data_present = GetBits(LEN_FAC_DATA_PRESENT,
                             FAC_DATA_PRESENT,
                             hResilience,
                             hEscInstanceData,
                             hVm);

  /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
  if ((mpegh3daProfileLevelIndication >= MPEG3DA_PLI_BASELINE_L1) && (mpegh3daProfileLevelIndication <= MPEG3DA_PLI_BASELINE_L5)) {
    if (0 != fac_data_present) {
      fprintf(stderr,"fac_data_present shall be 0 in Baseline Profile.\n");
      return MPEG3DACORE_ERROR_INVALID_STREAM;
    }
  }

  *isAceStart = (core_mode_last == 0)?1:0;

  /* Loop over the 4 frames of the super-frame */
  quant = td->tcx_quant;
  first_tcx_flag=0;
  k = 0;

  while (k < nbDiv ) {
    hDec->tns.info[k].n_filt = 0;

    /* Retreive FAC information for transitions between TCX and ACELP */

    skip = 1;
    if( k == 0){
      if ( (core_mode_last == 1) && (fac_data_present == 1) ) skip=0;
    } else {
      if ( ((lastMode==0) && (td->mod[k]>0)) || ((lastMode>0) && (td->mod[k]==0)) ) skip=0;
    }
    
    if (!skip) {
      fac_decoding((hDec->lDiv)/2, k, td->fac, hResilience, hEscInstanceData, hVm);
    }

    switch(td->mod[k]) {
    case 0:
      acelp_decoding(td->core_mode_index,
                     hDec->fullbandLPD,
                     &hDec->tbeDecData,
                     k,
                     hDec->nbSubfr,
                     td,
                     hResilience,
                     hEscInstanceData,
                     hVm);
      lastMode = td->mod[k];
      td->tcx_lg[k] = 0;
      k += 1;
      ltpfData->data_present = 0;
      ltpfData->gain_index = 0;
      ltpfData->pitch_index = 0;
      hDec->igfDecData->igf_memory.igf_prevWinType = IGF_CODEC_TRANSITION;
      break;
    case 1:
      tcx_decoding(hDec,
                   td->mod[k],
                   quant,
                   k,
                   lastMode,
                   first_tcx_flag,
                   arithPreviousSize,
                   arithQ,
                   td,
                   igfConfig,
                   hResilience,
                   hEscInstanceData,
                   hVm,
                   bUsacIndependencyFlag,
                   ltpfData);
      lastMode = td->mod[k];
      quant += td->tcx_lg[k];
      k += 1;
      first_tcx_flag=1;
      break;
    case 2:
      tcx_decoding(hDec,
                   td->mod[k],
                   quant,
                   k,
                   lastMode,
                   first_tcx_flag,
                   arithPreviousSize,
                   arithQ,
                   td,
                   igfConfig,
                   hResilience,
                   hEscInstanceData,
                   hVm,
                   bUsacIndependencyFlag,
                   ltpfData);
      lastMode = td->mod[k];
      quant += td->tcx_lg[k];
      td->tcx_lg[k+1]=0;
      k += 2;
      first_tcx_flag=1;
      break;
    case 3:
      tcx_decoding(hDec,
                   td->mod[k],
                   quant,
                   k,
                   lastMode,
                   first_tcx_flag,
                   arithPreviousSize,
                   arithQ,
                   td,
                   igfConfig,
                   hResilience,
                   hEscInstanceData,
                   hVm,
                   bUsacIndependencyFlag,
                   ltpfData);
      lastMode = td->mod[k];
      quant += td->tcx_lg[k];
      td->tcx_lg[k+1]=0;
      td->tcx_lg[k+2]=0;
      td->tcx_lg[k+3]=0;
      k += 4;
      first_tcx_flag=1;
      break;
    }
  }
  
  /* Retreive information related to LPC filters */

  j=0;

  /* Decode remaining LPC filters */
  for (k=0; k<nbDiv+1; k++)            /* loop for LPC 4,0,2,1,3 */
  {
      if ((k==1) && !(*isAceStart)) k++;     /* skip LPC0 */

      /* Determine if this LPC filter is transmitted */

      skip=1;
      if (k<2) skip=0;                    /* LPC4,LPC0 */
      if ((k==2) && (td->mod[0]<2) && (nbDiv==2)) skip=0;   /* LPC2 */
      if ((k==2) && (td->mod[0]<3) && (nbDiv==4)) skip=0;   /* LPC2 */
      if ((k==3) && (td->mod[0]<2)) skip=0;   /* LPC1 */
      if ((k==4) && (td->mod[2]<2)) skip=0;   /* LPC3 */

      /* Skip unecessary LPC filters */

      if (!skip) {
        /* Decode quantizer type */

        if (k==0) {     /* LPC4 always abs */
          q_type = 0;
          nb = 0;
        }
        else if (((k==2) && (nbDiv==2)) || ((k==3) && (nbDiv==4))) {   /* Unary code for LPC1 */
          tmp = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
          if (tmp==0) {
            q_type = 2;         /* relR */
            nb = 1;
          }
          else {
            tmp = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
            if (tmp==0) {
              q_type = 0;      /* abs */
              nb = 2;
            }
            else {
              q_type = 1;      /* mid0 */
              nb = 2;
            }
          }
        }
        else if (k==4) {   /* Unary code for LPC3 */
          tmp = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
          if (tmp==0) {
            q_type = 1;         /* mid */
            nb = 1;
          }
          else {
            tmp = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
            if (tmp==0) {
              q_type = 0;      /* abs */
              nb = 2;
            }
            else {
              tmp = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
              if (tmp==0) {
                q_type = 2;   /* relL */
              }
              else {
                q_type = 3;   /* relR */
              }
              nb = 3;
            }
          }
        }
        else {       /* LPC2 or LPC0: 0=abs or 1=relR */
          nb = 1;
          q_type = GetBits (1, TD_MODE_LPC, hResilience, hEscInstanceData, hVm );
        }

        if (nb > 0) {
          td->lpc[j] = q_type; j++;

          nbits_lpc += nb;
        }

        /* Decode quantization indices */

        skip_avq=0;
        if ((k==2) && (q_type==1) && (nbDiv==2)) skip_avq=1;       /* LPC1 mid0 */
        if ((k==3) && (q_type==1) && (nbDiv==4)) skip_avq=1;       /* LPC1 mid0 */

        if (!skip_avq) {
          if (q_type==0) {
            /* Absolute quantizer with 1st stage stochastic codebook */
            td->lpc[j] = GetBits (8, TD_LPC_1STAGE, hResilience, hEscInstanceData, hVm ); j++;
            nbits_lpc += 8;
          }

          /* decode codebook number */

          if ((q_type == 1) && ((k==3) || (k==4))) {
            /* Unary code for mid LPC1/LPC3 */
            /* Q0=0, Q2=10, Q3=110, ... */
            nb = unary_decode(&qn1, hResilience, hEscInstanceData, hVm);
            td->lpc[j] = qn1; j++;
            nbits_lpc += nb;

            nb = unary_decode(&qn2, hResilience, hEscInstanceData, hVm);
            td->lpc[j] = qn2; j++;
            nbits_lpc += nb;
          }
          else {
            /* 2 bits to specify Q2,Q3,Q4,ext */
            nbits_lpc += 4;
                
            qn1 = 2 + GetBits(2, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm);
            qn2 = 2 + GetBits(2, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm);
                
            if ((q_type > 1) && ((k==3) || (k==4))) {
              /* Unary code for rel LPC1/LPC3 */
              /* Q0 = 0, Q5=10, Q6=110, ... */
              if (qn1 > 4) {
                nb = unary_decode(&qn1, hResilience, hEscInstanceData, hVm);
                if (nb > 1) qn1 += 3;
                nbits_lpc += nb;
              }
              if (qn2 > 4) {
                nb = unary_decode(&qn2, hResilience, hEscInstanceData, hVm);
                if (nb > 1) qn2 += 3;
                nbits_lpc += nb;
              }
            }
            else {
              /* Unary code for abs and rel LPC0/LPC2 */
              /* Q5 = 0, Q6=10, Q0=110, Q7=1110, ... */
              if (qn1 > 4) {
                nb = unary_decode(&qn1, hResilience, hEscInstanceData, hVm);
                if (nb == 1) qn1 += 5;
                else if (nb == 2) qn1 += 4;
                else if (nb == 3) qn1 = 0;
                else qn1 += 3;
                nbits_lpc += nb;
              }
              if (qn2 > 4) {
                nb = unary_decode(&qn2, hResilience, hEscInstanceData, hVm);
                if (nb == 1) qn2 += 5;
                else if (nb == 2) qn2 += 4;
                else if (nb == 3) qn2 = 0;
                else qn2 += 3;
                nbits_lpc += nb;
              }
            }
            td->lpc[j] = qn1; j++;
            td->lpc[j] = qn2; j++;
          }
              
          /* Decode Split-by-2 algebraic VQ */
          if (qn1 > 0){
            int n, nk, i;
                
            /* Get bit count for codebook index and voronoi indices */
            nk = 0;
            n = qn1;
            if (qn1 > 4){
              nk = (qn1-3)>>1;
              n = qn1 - nk*2;
            }
            /* Read codebook index I */
            td->lpc[j++] = GetBits (4*n, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm );
            /* Read voronoi incices */
            for (i=0; i<8; i++) {
              td->lpc[j++] = GetBits (nk, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm );
            }
            nbits_lpc += 4*n + 8*nk;
          }

          if (qn2 > 0){
            int n, nk, i;
            /* Get bit count for codebook index and voronoi indices */
            nk = 0;
            n = qn2;
            if (qn2 > 4){
              nk = (qn2-3)>>1;
              n = qn2 - nk*2;
            }
            /* Read codebook index I */
            td->lpc[j++] = GetBits (4*n, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm );
            /* Read voronoi incices */
            for (i=0; i<8; i++) {
              td->lpc[j++] = GetBits (nk, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm );
            }
            nbits_lpc += 4*n + 8*nk;
          }
        }
      }
    }


  if( (core_mode_last == 0) && (fac_data_present == 1) ){
    int fac_length;
    *short_fac_flag = GetBits(LEN_SHORT_FAC_FLAG,
                              SHORT_FAC_FLAG,
                              hResilience,
                              hEscInstanceData,
                              hVm);
  
    fac_length = (*short_fac_flag)?((hDec->lFrame)/16):((hDec->lFrame)/8);
    read_FAC_bits=ReadFacData(fac_length, td->fdFac,hResilience,hEscInstanceData,hVm);
  }

  return(0);
}

int unary_decode(
   int                      *ind,
   HANDLE_RESILIENCE        hResilience,
   HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
   HANDLE_BUFFER            hVm
)
{

int nb_bits;

  nb_bits = 1;

  /* Index bits */

  *ind = 0;
  while (GetBits (1, TD_LPC_AVQ, hResilience, hEscInstanceData, hVm) == 1) {
    *ind += 1;
    nb_bits++;
  }

  if (*ind != 0) *ind += 1;

  return(nb_bits);

}

#define MAX_SHIFT_LEN_LONG 4096

void usac_past_tw(float           p_overlap[],
                  int             nlong,            /* shift length for long windows   */
                  int             nshort,           /* shift length for short windows  */
                  WINDOW_SHAPE    wfun_select,
                  WINDOW_SHAPE    prev_wfun_select,
                  WINDOW_SEQUENCE windowSequenceLast,
                  int             mod0,
                  int             frameWasTd,
                  int             twMdct,
                  float           past_sample_pos[],
                  float           past_tw_trans_len[],
                  int             past_tw_start_stop[],
                  float           past_warped_time_sample_vector[]) {

  if ( !frameWasTd &&  twMdct ) {
    float tw_transf_buf_past[ 2*MAX_SHIFT_LEN_LONG ];
    float transf_buf_past[ 2*MAX_SHIFT_LEN_LONG ]={0.0};
    int nmdct=nlong;
    float           os_window_prev[MAX_SHIFT_LEN_LONG*TW_OS_FACTOR_WIN];
    calc_tw_window (os_window_prev, nlong*TW_OS_FACTOR_WIN, prev_wfun_select);
    /* map to correct window sequence */
    switch ( windowSequenceLast ) {
    case ONLY_LONG_SEQUENCE:
    case LONG_START_SEQUENCE:
    case LONG_STOP_SEQUENCE:
    case STOP_START_SEQUENCE:
    case EIGHT_SHORT_SEQUENCE:
      nmdct = nlong;
      break;
    default:
      /* do nothing */
      break;
    }

    vcopy( past_warped_time_sample_vector, tw_transf_buf_past, 1 ,1, 3*nlong);
    if ( windowSequenceLast != EIGHT_SHORT_SEQUENCE ) {

      tw_adjust_past((WINDOW_SEQUENCE)0,
                     windowSequenceLast,
                     1,
                     past_tw_start_stop,
                     past_tw_trans_len,
                     nmdct);
        
      tw_windowing_past(tw_transf_buf_past+TW_IPLEN2S,
                        tw_transf_buf_past+TW_IPLEN2S,
                        past_tw_start_stop[1],
                        nmdct,
                        past_tw_trans_len[1],
                        os_window_prev);
    }

    tw_resamp(tw_transf_buf_past,
              nlong+nlong/8,
              2*nmdct+2*TW_IPLEN2S,
              3*nlong,
              past_sample_pos,
              0.5f,
              transf_buf_past);
    /* OLA */
    vadd(transf_buf_past + nlong+nlong/8 ,p_overlap+nlong/8, p_overlap+nlong/8,1,1,1,2*nlong-nlong/8);
  }

}


void usac_td2buffer(float           p_in_data[],
		    float 	    p_out_data[],
		    float 	    p_overlap[],
		    int             nlong,            /* shift length for long windows   */
		    int             nshort,           /* shift length for short windows  */
		    WINDOW_SEQUENCE windowSequenceLast,
		    int             mod0,
		    int 	    frameWasTd,
		    int             twMdct,
                    float           past_sample_pos[],
                    float           past_tw_trans_len[],
                    int             past_tw_start_stop[],
                    float           past_warped_time_sample_vector[])
{
  int i;
  int lfac;

  if (!frameWasTd) {
    if (windowSequenceLast == EIGHT_SHORT_SEQUENCE) {
      lfac = nshort/2;
    }
    else {
      lfac = nshort;
    }
  }
  
  if ( !frameWasTd && (mod0 == 0) ) {
    for ( i = 0 ; i < (nlong/2) - lfac - (BPF_SFD*L_SUBFR); i++ ) {/* keep postfiltered synth area */
     p_in_data[i] = 0.0f;
    }
  }
  else if ( !frameWasTd && (mod0 > 0) ) {
    for ( i = 0 ; i < (nlong/2)-lfac; i++ ) {
      p_in_data[i] = 0.0f;
    }
  }

  if ( twMdct ) {
    if (  !frameWasTd && (mod0 == 0) ) {
      for ( i = (nlong/2) - lfac -  (BPF_SFD*L_SUBFR); i < nlong/2; i++  ) {
        p_overlap[i + (nlong/2)] = 0.0f; /* replace FD synth by base postfiltered version for the FAC area */
      }
    }
    for ( i = 0 ; i < nlong/2 ; i++ ) {
      p_out_data[i] = p_overlap[i];
      p_out_data[i+nlong/2] = p_overlap[i+nlong/2] + p_in_data[i];
      p_overlap[i] = p_in_data[i+(nlong/2)] + p_overlap[i+nlong];
      p_overlap[i+(nlong/2)] = 0.0f;
      p_overlap[i+nlong] = 0.0f;
      p_overlap[i+nlong+(nlong/2)] = 0.0f;
    }
  }
  else {
    if (  !frameWasTd && (mod0 == 0) ) {
      for ( i = (nlong/2) - lfac - (BPF_SFD*L_SUBFR); i < nlong/2; i++  ) {
        p_overlap[i] = 0.0f; /* replace FD synth by base postfiltered version for the FAC area */
      }
    }
    else if( !frameWasTd ) {
      for (i = (nlong/2) - lfac; i < nlong; i++ ) {
        p_overlap[i] = 0.0f;
      }
    }
    for (i = 0; i < nlong; i++){
      p_out_data[i] = p_overlap[i] + p_in_data[i];
      p_overlap[i] = 0.0f;
    }
  }
}
