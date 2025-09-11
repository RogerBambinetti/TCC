/*******************************************************************************
This software module was originally developed by

This software module was originally developed by

AT&T, Dolby Laboratories, Fraunhofer Gesellschaft IIS, and VoiceAge Corp.

Initial author:
Yoshiaki Oikawa     (Sony Corporation),
Mitsuyuki Hatanaka  (Sony Corporation),
Ralph Sperschneider (Fraunhofer Gesellschaft IIS)
Ali Nowbakht-Irani  (Fraunhofer Gesellschaft IIS)
Markus Werner       (SEED / Software Development Karlsruhe)

and edited by:
Jeremie Lecomte     (Fraunhofer IIS)
Markus Multrus      (Fraunhofer IIS)
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
$Id: usac_fd_dec.c,v 1.16.4.1 2012-04-19 09:15:33 frd Exp $
*******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "allHandles.h"
#include "block.h"
#include "buffer.h"

#include "usac_all.h"                 /* structs */
#include "usac_mainStruct.h"       /* structs */
#include "tns.h"                 /* structs */

#include "usac_allVariables.h"        /* variables */

#include "huffdec2.h"
#include "huffdec3.h"
#include "usac_port.h"
#include "common_m4a.h"
#include "resilience.h"
#include "concealment.h"
#include "reorderspec.h"
#include "bitfct.h"
#include "buffers.h"
#include "bitstream.h"
#include "interface.h"

#include "concealment.h"
#include "rvlcScfResil.h"
#include "huffScfResil.h"

#include "dec_usac.h"
#include "aac.h"

#include "usac_arith_dec.h"
#include "usac_cplx_pred.h"
#include "proto_func.h"

#define MAX_NR_OF_SWB       120

int g_bSplitTransform[CChans]={0};

typedef struct {
  unsigned short nrOfSwb;
  unsigned short swbReal;
} ENTRY_POINT;

static void PrintSpectralValues ( int *quant )
{
  unsigned short i;
  for ( i = 0; i < LN2; i++ )
    printf ( "bno %13li line %4i value %13i\n", bno, i, quant[i] );
}

void calc_gsfb_table ( Info *info,
                       const byte *group )
{
  int group_offset;
  int group_idx;
  int offset;
  short * group_offset_p;
  int sfb,len;
  /* first calc the group length*/
  if (info->islong){
    return;
  } else {
    group_offset = 0;
    group_idx = 0;
    do  {
      info->group_len[group_idx] = group[group_idx] - group_offset;
      group_offset=group[group_idx];
      group_idx++;
    } while (group_offset<8);
    info->num_groups = group_idx;
    group_offset_p = info->bk_sfb_top;
    offset=0;
    for ( group_idx = 0; group_idx < info->num_groups; group_idx++) {
      len = info->group_len[group_idx];
      for (sfb=0;sfb<info->sfb_per_sbk[group_idx];sfb++){
        offset += info->sfb_width_short[sfb] * len;
        *group_offset_p++ = offset;
      }
    }
  }
}


void usac_get_tns ( Info*             info,
                    TNS_frame_info*   tns_frame_info,
                    HANDLE_RESILIENCE hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER     hVm )
{
  int                       f, t, top, res, res2, compress;
  int                       short_flag, s;
  short                     *sp, tmp, s_mask, n_mask;
  TNSfilt                   *tns_filt;
  TNSinfo                   *tns_info;
  static short              sgn_mask[] = {     0x2, 0x4, 0x8     };
  static short              neg_mask[] = {     (short) 0xfffc, (short)0xfff8, (short)0xfff0     };

  short_flag = (!info->islong);
  tns_frame_info->n_subblocks = info->nsbk;

  for (s=0; s<tns_frame_info->n_subblocks; s++) {
    tns_info = &tns_frame_info->info[s];

    if (!(tns_info->n_filt = GetBits ( short_flag ? 1 : 2,
                                       N_FILT,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm )))
      continue;

    tns_info -> coef_res = res = GetBits ( 1,
                                           COEF_RES,
                                           hResilience,
                                           hEscInstanceData,
                                           hVm ) + 3;
    top = info->sfb_per_sbk[s];
    tns_filt = &tns_info->filt[ 0 ];
    for (f=tns_info->n_filt; f>0; f--)  {
      tns_filt->stop_band = top;
      top = tns_filt->start_band = top - GetBits ( short_flag ? 4 : 6,
                                                   LENGTH,
                                                   hResilience,
                                                   hEscInstanceData,
                                                   hVm );
      tns_filt->order = GetBits ( short_flag ? 3 : 4,
                                  ORDER,
                                  hResilience,
                                  hEscInstanceData,
                                  hVm );

      if (tns_filt->order)  {
        tns_filt->direction = GetBits ( 1,
                                        DIRECTION,
                                        hResilience,
                                        hEscInstanceData,
                                        hVm );
        compress = GetBits ( 1,
                             COEF_COMPRESS,
                             hResilience,
                             hEscInstanceData,
                             hVm );

        res2 = res - compress;
        s_mask = sgn_mask[ res2 - 2 ];
        n_mask = neg_mask[ res2 - 2 ];

        sp = tns_filt -> coef;
        for (t=tns_filt->order; t>0; t--)  {
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
  }   /* subblock loop */
}

static int scfcb ( byte*                    sect,
                   Info                     info,
                   int                      tot_sfb,
                   int                      sfb_per_sbk,
                   byte                     max_sfb,
                   unsigned char            numGroups)
{
  int            nsect;
  int            nrSfb;
  unsigned char  group = 0;
  unsigned char  currentMaxSfb;
  unsigned short vcb11Flag;
  unsigned short nrOfCodewords = 0;

  if (debug['s']) {
    fprintf(stderr, "total sfb %d\n", tot_sfb);
    fprintf(stderr, "   sect    top     cb\n");
  }
  nsect = 0;
  nrSfb = 0;

  while ( ( nrSfb < tot_sfb ) && ( nsect < tot_sfb ) ) {
    currentMaxSfb = sfb_per_sbk * group + max_sfb;
    vcb11Flag = 0; /* virtual codebooks flag is false */
    *sect = 11;
    sect++;
    *sect = nrSfb = currentMaxSfb;
    sect++;
    nsect++;
    /* insert a zero section for regions above max_sfb for each group */
    if ( sect[-1] == currentMaxSfb ) {           /* changed from ((sect[-1] % sfb_per_sbk) == max_sfb) */
      nrSfb = ( sfb_per_sbk * ( group + 1 ) );    /* changed from nrSfb += (sfb_per_sbk - max_sfb) */
      group++;
      if ( currentMaxSfb != sfb_per_sbk * group ) { /* this inserts a section with codebook 0 */
        *sect++ = 0;
        *sect++ = nrSfb;
        nsect++;

        if (debug['s'])
          fprintf(stderr, "(%6d %6d %6d)\n", nsect, sect[-1], sect[-2]);
      }
    }
  }
  return ( nsect );
}

void get_tw_data(int                     *tw_data_present,
                 int                     *tw_ratio,
                 HANDLE_RESILIENCE        hResilience,
                 HANDLE_ESC_INSTANCE_DATA hEscInstanceData,
                 HANDLE_BUFFER            hVm
) {

  *tw_data_present = (int) GetBits(LEN_TW_PRES,
                                   TW_DATA_PRESENT,
                                   hResilience,
                                   hEscInstanceData,
                                   hVm);

  if ( *tw_data_present ) {
    int i;
    for ( i = 0 ; i < NUM_TW_NODES ; i++ ) {
      tw_ratio[i] = (int) GetBits(LEN_TW_RATIO,
                                  TW_RATIO,
                                  hResilience,
                                  hEscInstanceData,
                                  hVm);
    }
  }
}

WINDOW_SEQUENCE
usacMapWindowSequences(WINDOW_SEQUENCE windowSequenceCurr, WINDOW_SEQUENCE windowSequenceLast){

  WINDOW_SEQUENCE windowSequence;

  switch( windowSequenceCurr ) {
  case ONLY_LONG_SEQUENCE :
    windowSequence = ONLY_LONG_SEQUENCE;
    break;

  case LONG_START_SEQUENCE :
    if ( (windowSequenceLast == LONG_START_SEQUENCE)  ||
         (windowSequenceLast == EIGHT_SHORT_SEQUENCE) ||
         (windowSequenceLast == STOP_START_SEQUENCE) ){/* Stop Start Window*/
      windowSequence = STOP_START_SEQUENCE;
    } else { /*Start Window*/
      windowSequence = LONG_START_SEQUENCE;
    }
    break;

  case LONG_STOP_SEQUENCE :
    windowSequence = LONG_STOP_SEQUENCE;
    break;

  case EIGHT_SHORT_SEQUENCE :
    windowSequence = EIGHT_SHORT_SEQUENCE;
    break;

  default :
    CommonExit( 1, "Unknown window type" );
  }

  return windowSequence;
}

MPEG3DACORE_RETURN_CODE usac_get_fdcs(int                       id,
                                      HANDLE_USAC_DECODER       hDec,
                                      Info*                     info,
                                      USAC_DATA*                usacData,
                                      int                       common_window,
                                      int                       common_tw,
                                      int                       tns_data_present,
                                      int*                      core_mode,
                                      int                       left,
                                      int                       right,
                                      WINDOW_SEQUENCE*          win,
                                      WINDOW_SHAPE*             wshape,
                                      byte*                     group,
                                      byte*                     max_sfb,
                                      byte*                     cb_map,
                                      float*                    coef,
                                      int*                      max_spec_coefficients,
                                      short*                    global_gain,
                                      short*                    factors,
                                      int*                      arithPreviousSize,
                                      Tqi2                      arithQ[],
                                      TNS_frame_info*           tns,
                                      int*                      tw_data_present,
                                      int*                      tw_ratio,
                                      HANDLE_RESILIENCE         hResilience,
                                      HANDLE_BUFFER             hHcrSpecData,
                                      HANDLE_HCR                hHcrInfo,
                                      HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                                      HANDLE_CONCEALMENT        hConcealment,
                                      QC_MOD_SELECT             qc_select,
                                      unsigned int*             nfSeed,
                                      HANDLE_BUFFER             hVm,
                                      WINDOW_SEQUENCE           windowSequenceLast,
                                      int                       frameWasTD,
                                      int                       bUsacIndependencyFlag,
                                      float*                    dmx_re_prev,
                                      int                       ch,
                                      int                       bUseNoiseFilling,
                                      int*                      stereoFilling,
                                      int                       pred_dir,
                                      int                       common_ltpf,
                                      IGF_CONFIG*               igfConfig,
                                      int                       leftOrRightChannel
) {
  int            nsect                    = 0;
  int            i                        = 0;
  int            cb                       = 0;
  int            top                      = 0;
  int            bot                      = 0;
  int            tot_sfb                  = 0;
  int            fdp_spacing_index        = -1;
  int            read_FAC_bits            = 0;
  byte           sect[2 * (MAXBANDS + 1)] = {0};
  int            noise_level              = 0;
  int            reset                    = 0;
  int            arithSize                = 0;
  int            fdp_data_present         = 0;
  int            igfAllZero               = 0;

  /*
   * global gain
   */
  *global_gain = (short) GetBits(8,
                                 GLOBAL_GAIN,
                                 hResilience,
                                 hEscInstanceData,
                                 hVm);

  if (debug['f']) {
    printf("global gain: %3d\n", *global_gain);
  }

  /*
   * Noise Filling
   */
  if (bUseNoiseFilling) {
    noise_level = (short) GetBits(8,
                                  NOISE_LEVEL,
                                  hResilience,
                                  hEscInstanceData,
                                  hVm);
  } else {
    noise_level = 0;
  }

  if (ID_LFE == id && 0 != bUseNoiseFilling) {
    /* ISO/IEC 23008-3: noiseFilling shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tnoiseFilling shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /*
   * ICS info
   */
  if (!common_window) {
    usac_get_ics_info(win,
                      wshape,
                      group,
                      max_sfb,
                      qc_select,
                      hResilience,
                      hEscInstanceData,
                      hVm);

    *win = usacMapWindowSequences(*win, windowSequenceLast);
  }

  if (ID_LFE == id && 0 != common_window) {
    /* ISO/IEC 23008-3: common_window shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tcommon_window shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }
  memcpy(info, usac_winmap[*win], sizeof(Info));

  *stereoFilling = 0;

  if (common_window) {
    if (leftOrRightChannel == 0) {
      /* only for left channel of channel pair */
      g_bSplitTransform[ch] = g_bSplitTransform[ch+1] = 0;

      if ((noise_level > 0) && (noise_level < 32) && ((*win==STOP_START_SEQUENCE)||(*win==LONG_START_SEQUENCE))) {
        g_bSplitTransform[ch] = g_bSplitTransform[ch+1] = 1;

        /* noise filling: 3-bit level, 2-bit offset */
        noise_level = noise_level << 3;
      }
    } else {
      if ((noise_level > 0) && (noise_level < 32)) {
        *stereoFilling = 1;

        /* noise filling: 3-bit level, 2-bit offset */
        noise_level = noise_level << 3;
      }
    }
  } else {
    g_bSplitTransform[ch] = 0;

    if ((noise_level > 0) && (noise_level < 32) && ((*win==STOP_START_SEQUENCE)||(*win==LONG_START_SEQUENCE))) {
      g_bSplitTransform[ch] = 1;

      /* noise filling: 3-bit level, 2-bit offset */
      noise_level = noise_level << 3;
    }
  }

  /*
   * TW data
   */
  if ((hDec->twMdct[0] == 1) && !common_tw) {
    get_tw_data(tw_data_present,
                tw_ratio,
                hResilience,
                hEscInstanceData,
                hVm );
  }

  if (ID_LFE == id && (0 != hDec->twMdct[0])) {
    /* ISO/IEC 23008-3: tw_mdct shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\ttw_mdct shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /* calculate total number of sfb for this grouping */
  if (*max_sfb == 0) {
    tot_sfb = 0;
  }
  else {
    i=0;
    tot_sfb = info->sfb_per_sbk[0];
    if (debug['f'])printf("tot sfb %d %d\n", i, tot_sfb);
    while (group[i++] < info->nsbk) {
      tot_sfb += info->sfb_per_sbk[0];
      if (debug['f'])printf("tot sfb %d %d\n", i, tot_sfb);
    }
  }

  /*
   * LTPF data
   */
  if (!common_ltpf) {
    hDec->ltpfData[ch].data_present = (int) GetBits(1,
                                                    LTP_DATA_PRESENT,
                                                    hResilience,
                                                    hEscInstanceData,
                                                    hVm);

    /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
    if ((usacData->mpegh3daProfileLevelIndication == MPEG3DA_PLI_BASELINE_L3) && (usacData->numChannels > 16) && (usacData->numChannels <= 24)) {
      if (0 != hDec->ltpfData[ch].data_present) {
        fprintf(stderr,"ltpf_data_present shall be 0 in Baseline Profile Level 3 with more than 16 objects.\n");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }

    if (hDec->ltpfData[ch].data_present) {
      hDec->ltpfData[ch].pitch_index = (int)GetBits(9,
                                                    LTP_LAG,
                                                    hResilience,
                                                    hEscInstanceData,
                                                    hVm);
      hDec->ltpfData[ch].gain_index = (int)GetBits(2,
                                                   LTP_COEF,
                                                   hResilience,
                                                   hEscInstanceData,
                                                   hVm);
    }
  }

  if (ID_LFE == id && (0 != common_ltpf || 0 != hDec->ltpfData[ch].data_present)) {
    /* ISO/IEC 23008-3: common_ltpf and ltpf_data_present shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tcommon_ltpf and ltpf_data_present shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /*
   * FDP data
   */
  if (info->islong && !bUsacIndependencyFlag) {
    fdp_data_present = (int)GetBits(1,
                                    PREDICTOR_DATA_PRESENT,
                                    hResilience,
                                    hEscInstanceData,
                                    hVm);

    /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
    if ((usacData->mpegh3daProfileLevelIndication == MPEG3DA_PLI_BASELINE_L3) && (usacData->numChannels > 16) && (usacData->numChannels <= 24)) {
      if (0 != fdp_data_present) {
        fprintf(stderr,"fdp_data_present shall be 0 in Baseline Profile Level 3 with more than 16 objects.\n");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }

    if (fdp_data_present) {
      fdp_spacing_index = (int)GetBits(8,
                                       PREDICTION_USED,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm);
    }
  }

  if (ID_LFE == id && 0 != fdp_data_present) {
    /* ISO/IEC 23008-3: fdp_data_present shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tfdp_data_present shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /*
   * Aliasing Symmetry
   */
  if (bUsacIndependencyFlag) {
    hDec->prevAliasingSymmetry[ch] = (int)GetBits(1,
                                                  WINDOW_SHAPE_CODE,
                                                  hResilience,
                                                  hEscInstanceData,
                                                  hVm);
  } else {
    hDec->prevAliasingSymmetry[ch] = hDec->currAliasingSymmetry[ch];
  }
  hDec->currAliasingSymmetry[ch] = (int)GetBits(1,
                                                WINDOW_SHAPE_CODE,
                                                hResilience,
                                                hEscInstanceData,
                                                hVm);

  if (ID_LFE == id && (0 != hDec->prevAliasingSymmetry[ch] || 0 != hDec->currAliasingSymmetry[ch])) {
    /* ISO/IEC 23008-3: prev_aliasing_symmerty and curr_aliasing_symmerty shall be 0 for a LFE element */
    printf("\nISO/IEC 23008-3:\n\tprev_aliasing_symmerty and curr_aliasing_symmerty shall be 0\n\tfor a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /* calculate band offsets
   * (because of grouping and interleaving this cannot be
   * a constant: store it in info.bk_sfb_top)
   */
  calc_gsfb_table(info, group);

  /* set correct sections */

  nsect = scfcb(sect,
                *info,
                tot_sfb,
                info->sfb_per_sbk[0],
                *max_sfb,
                info->num_groups);

  /* generate "linear" description from section info
   * stored as codebook for each scalefactor band and group
   */
  if (nsect) {
    bot = 0;
    for (i=0; i<nsect; i++) {
      cb = sect[2*i];
      top = sect[2*i + 1];
      for (; bot<top; bot++)
        *cb_map++ = cb;
      bot = top;
    }
  }
  else {
    for (i=0; i<MAXBANDS; i++)
      cb_map[i] = 0;
  }

  /*
   * scale factor data
   */
  if(!hufffac( info,
               group,
               nsect,
               sect,
               *global_gain,
               1,
               factors,
               hResilience,
               hEscInstanceData,
               hVm ) ) {
    if ( !hEscInstanceData || BsGetEpDebugLevel ( hEscInstanceData ) >= 3 ) {
      printf("getics: hufffac returns zero (huffdec2.c, getics())\n");
    }
    return MPEG3DACORE_ERROR_HUFFFAC;
  }

  /*
   * enhanced noise filling data
   */
  if (igfConfig->igf_active) {
    igfAllZero = IGF_level(hVm,
                           hResilience,
                           hEscInstanceData,
                           &hDec->igfDecData[ch],
                           igfConfig,
                           (*win == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                           (*win == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                           info->num_groups,
                           bUsacIndependencyFlag);
    if (!igfAllZero) {
      IGF_data(hVm,
               hResilience,
               hEscInstanceData,
               &hDec->igfDecData[ch],
               igfConfig,
               (*win == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
               (*win == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
               bUsacIndependencyFlag);
    }
  }

  if (ID_LFE == id && 0 != igfConfig->igf_active) {
    /* ISO/IEC 23008-3: enhancedNoiseFilling shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tenhancedNoiseFilling shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  /*
   * tns data
   */
  if (tns_data_present == 1) {
    usac_get_tns(info,
                 tns,
                 hResilience,
                 hEscInstanceData,
                 hVm);
  } else if (tns_data_present == 0) {
    clr_tns(info,
            tns);
  }

  if (ID_LFE == id && 0 != tns_data_present) {
    /* ISO/IEC 23008-3: tns_data_present shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\ttns_data_present shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  if (tns_data_present && debug['V']) {
    fprintf(stderr,"# TNS detected\n");
  }

  if(*max_sfb > 0){
    *max_spec_coefficients = info->bk_sfb_top[*max_sfb-1]/info->group_len[0];
  } else {
    *max_spec_coefficients = 0;
  }

  /*
   * ac data
   */
  if(!bUsacIndependencyFlag){
    reset = GetBits(LEN_RESET_ARIT_DEC,
                    RESET_ARIT_DEC,
                    hResilience,
                    hEscInstanceData,
                    hVm);
  } else {
    reset = 1;
  }


  switch (*win) {
    case EIGHT_SHORT_SEQUENCE:
      arithSize = hDec->blockSize/8;
      break;
    default:
      arithSize = hDec->blockSize;
      break;
  }

  {
    byte max_noise_sfb = *max_sfb;
    if (igfConfig->igf_active) {
      if(*win == EIGHT_SHORT_SEQUENCE ) {
        max_noise_sfb = min(max_noise_sfb, igfConfig->igf_grid[IGF_SHORT].igf_SfbStart);
      } else {
        max_noise_sfb = min(max_noise_sfb, igfConfig->igf_grid[IGF_LONG].igf_SfbStart);
      }
    }

    if (core_mode[0] == CORE_MODE_FD && core_mode[1] == CORE_MODE_FD && id == ID_CPE && common_window == 1) {
      usac_cplx_pred_prev_dmx(info,
                              hDec->coefSave[left],
                              hDec->coefSave[right],
                              usacData->dmx_re_prev[left],
                              pred_dir,
                              usacData->usacIndependencyFlag || usacData->FrameWasTD[left] || usacData->FrameWasTD[right]);
    }

    acSpecFrame(hDec,
                info,
                coef,
               *max_spec_coefficients,
                hDec->blockSize,
                noise_level,
                factors,
                arithSize,
                arithPreviousSize,
                arithQ,
                hVm,
               *max_sfb,
                max_noise_sfb,
                reset,
                nfSeed,
                bUseNoiseFilling,
               *stereoFilling,
                fdp_spacing_index, /* -1: no FDP */
                ch, /* overall channel index */
                bUsacIndependencyFlag,
                igfConfig->igf_active,
                info->islong ? igfConfig->igf_grid[IGF_LONG].igf_NTiles : igfConfig->igf_grid[IGF_SHORT].igf_NTiles,
                igfConfig->igf_UseINF,
                igfAllZero,
                hDec->igfDecData[ch].igf_infSpec,
                (leftOrRightChannel == 1) ? dmx_re_prev : NULL);
  }

  /*
   * fac data
   */
  hDec->ApplyFAC[ch] = (short) GetBits(LEN_FAC_DATA_PRESENT,
                                       NOISE_LEVEL,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm);

  if (hDec->ApplyFAC[ch]) {
    int lFac;
    if ((*win) == EIGHT_SHORT_SEQUENCE) {
      lFac = (hDec->blockSize) / 16;
    } else {
      lFac = (hDec->blockSize) / 8;
    }
    if (hDec->tddec[ch]->fullbandLPD) {    
      lFac = lFac/2;
    }

    read_FAC_bits = ReadFacData(lFac,
                                hDec->facData[ch],
                                hResilience,
                                hEscInstanceData,
                                hVm);
  }

  if (ID_LFE == id && 0 != hDec->ApplyFAC[ch]) {
    /* ISO/IEC 23008-3: fac_data_present shall be 0 for a LFE element */
    printf("ISO/IEC 23008-3:\n\tfac_data_present shall be 0 for a LFE element.\n");
    return MPEG3DACORE_ERROR_INVALID_STREAM;
  }

  return MPEG3DACORE_OK;
}



int ReadFacData(int lfac,
                int                      *facData,
                HANDLE_RESILIENCE         hResilience,
                HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                HANDLE_BUFFER             hVm

                )
{
  int count = 0;
  int cnt = 512;
  int ReadBits = 0;
  int Extension_Type = 0;
  int unalignedBits = 0;

  /* read gain */
  facData[0] = GetBits(7,
                       FAC_GAIN,
                       hResilience,
                       hEscInstanceData,
                       hVm);

  ReadBits+=7;

  /* read FAC data */
  ReadBits+=fac_decoding(lfac,
                         0,
                         &facData[1],
                         hResilience,
                         hEscInstanceData,
                         hVm);


  return ReadBits;
}
