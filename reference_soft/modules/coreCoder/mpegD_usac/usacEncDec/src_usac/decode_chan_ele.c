/*******************************************************************************
This software module was originally developed by


AT&T, Dolby Laboratories, Fraunhofer IIS, VoiceAge Corp.

and edited by
Yoshiaki Oikawa     (Sony Corporation),
Mitsuyuki Hatanaka  (Sony Corporation),
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
$Id: decode_chan_ele.c,v 1.28.4.1 2012-04-19 09:15:33 frd Exp $
*******************************************************************************/


#include <stdio.h>
#include <string.h>

#ifdef SONY_PVC
#include "../src_tf/sony_pvcprepro.h"
#endif  /* SONY_PVC */

#include "allHandles.h"
#include "block.h"
#include "buffer.h"

#include "all.h"                 /* structs */
#include "usac_mainStruct.h"     /* structs */
#include "tns.h"                 /* structs */

#include "bitstream.h"
#include "buffers.h"
#include "common_m4a.h"
#include "concealment.h"
#include "huffdec2.h"
#include "usac_port.h"
#include "resilience.h"
#include "concealment.h"

#include "usac_allVariables.h"        /* variables */
#include "usac_interface.h"
#include "usac_tw_tools.h"

#include "acelp_plus.h"
#include "usac_main.h"

#include "proto_func.h"

#include "usac_cplx_pred.h"

#ifdef DEBUGPLOT
#include "plotmtv.h"
#endif

#include "dec_lpd_stereo.h"

extern int g_bSplitTransform[CChans];

/*
 * read and decode the data for the next 1024 output samples
 * return MPEG3DACORE_RETURN_CODE
 *
 * This function corresponds to the bitstream element single_channel_element() and/or channel_pair_element().
 *
 */

MPEG3DACORE_RETURN_CODE parseChannelElement(int                      id,
                                            MC_Info*                 mip,
                                            USAC_DATA               *usacData,
                                            WINDOW_SEQUENCE*         win,
                                            Wnd_Shape*               wshape,
                                            byte**                   cb_map,
                                            short**                  factors,
                                            byte**                   group,
                                            byte*                    hasmask,
                                            byte**                   mask,
                                            byte*                    max_sfb,
                                            TNS_frame_info**         tns,
                                            float**                  coef,
                                            Info*                    sfbInfo,
                                            QC_MOD_SELECT            qc_select,
                                            HANDLE_DECODER_GENERAL   hFault,
                                            int                      elementTag,
                                            int                      bNoiseFilling,
                                            HSTEREOLPD_DEC_DATA      stereolpdDecData,
                                            int                      lpdStereoIndex,
                                            IGF_CONFIG              *igfConfig,
                                            int                      channelOffset,
                                            int                     *stereoFilling,
                                            int                     *tns_on_lr,
                                            int                     *core_mode,
                                            unsigned char           *max_sfb1,
                                            unsigned char           *max_sfb_ste,
                                            unsigned char           *max_sfb_ste_clear,
                                            int                     *max_spec_coefficients,
                                            int                     *complex_coef,
                                            int                     *use_prev_frame,
                                            byte                    *igf_hasmask,
                                            int                     *igf_pred_dir,
                                            int                     *igf_SfbStart,
                                            int                     *igf_SfbStop,
                                            int                     *pred_dir,
                                            int                     *common_window
#ifdef SONY_PVC_DEC
                                           ,int                     *cm
#endif /* SONY_PVC_DEC */
#ifndef CPLX_PRED_NOFIX
                                           ,int                     *stereoConfigIndex
#endif /* CPLX_PRED_NOFIX */
) {
  MPEG3DACORE_RETURN_CODE error_3da = MPEG3DACORE_OK;
  int i=0, k=0;
  int common_tw = 0;
  int wn = 0;
  int tns_data_present[48] = {0};
  int tns_active=0, common_tns=0, tns_present_both=0;
  unsigned char common_max_sfb;
  int common_ltpf = 0;
  float synth_stereo[Chans][L_FRAME_1024];
  float prevBuffer[L_FRAME_1024];

  HANDLE_USAC_DECODER      hDec             = usacData->usacDecoder;
  HANDLE_BUFFER            hVm              = hFault[0].hVm;
  HANDLE_BUFFER            hHcrSpecData     = hFault[0].hHcrSpecData;
  HANDLE_RESILIENCE        hResilience      = hFault[0].hResilience;
  HANDLE_HCR               hHcrInfo         = hFault[0].hHcrInfo;
  HANDLE_ESC_INSTANCE_DATA hEscInstanceData = hFault[0].hEscInstanceData;
  HANDLE_CONCEALMENT       hConcealment     = hFault[0].hConcealment;

  int bad_frame[4] = {0, 0, 0, 0};
  float synth_buf[128+L_FRAME_1024+LTPF_MAX_DELAY] = {0};
  float *synth = NULL;
  td_frame_data td_frame;
  int   isBassPostFilter;
  int   isAceStart = 0;
  int   short_fac_flag = 0;
  int   nChannelsCoreCoder = (id == ID_CPE) ? 2 : 1;

  int left =  channelOffset;
  int right = (id != ID_CPE) ? channelOffset : (channelOffset+1);

  memset(&td_frame, 0, sizeof(td_frame));

  synth = synth_buf + 128;
  
  if(*stereoConfigIndex > 0) {
    *stereoConfigIndex = 0;
  }

  BsSetChannel(1);

  if ((id == ID_SCE) || (id == ID_CPE)) {
    for (i = 0; i < nChannelsCoreCoder; i++) {
      core_mode[i] = GetBits(LEN_CORE_MODE,
                             CORE_MODE,
                             hResilience,
                             hEscInstanceData,
                             hVm);
      /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
      if ((usacData->mpegh3daProfileLevelIndication >= MPEG3DA_PLI_BASELINE_L1) && (usacData->mpegh3daProfileLevelIndication <= MPEG3DA_PLI_BASELINE_L5)) {
        if (CORE_MODE_FD != core_mode[i]) {
          fprintf(stderr,"core_mode[ch] shall be 0 in Baseline Profile.\n");
          return MPEG3DACORE_ERROR_INVALID_STREAM;
        }
      }
    }
  }

  /* ISO/IEC 23008-3, 4.8.2.2 Restrictions for the Low Complexity Profile and Levels */
  if ((usacData->mpegh3daProfileLevelIndication >= MPEG3DA_PLI_LC_L1) && (usacData->mpegh3daProfileLevelIndication <= MPEG3DA_PLI_LC_L5)) {
    if (usacData->usacDecoder->samplingRate > 32000) {
      if ((core_mode[0] == CORE_MODE_TD) || (core_mode[1] == CORE_MODE_TD)) {
        CommonWarning("In the LC profile the LPD mode shall only be employed at 3DA core coder sampling rates <= 32000 Hz.");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }
  }

#ifdef SONY_PVC_DEC
  *cm = core_mode[0];
#endif /* SONY_PVC_DEC */

  *common_window = 0;

  if ((id == ID_SCE) && (core_mode[0] == CORE_MODE_FD)) {
      tns_data_present[left] = GetBits(LEN_TNS_ACTIVE,
                                       TNS_ACTIVE,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm);
  }

  if ((id == ID_CPE) && (core_mode[0] == CORE_MODE_FD) && (core_mode[1] == CORE_MODE_FD)) { /* parse StereoCoreToolInfo() */
    tns_active = GetBits( LEN_TNS_ACTIVE,
                          TNS_ACTIVE,
                          hResilience,
                          hEscInstanceData,
                          hVm );

    *common_window = GetBits( LEN_COM_WIN,
                              COMMON_WINDOW,
                              hResilience,
                              hEscInstanceData,
                              hVm );

    if (!(*common_window) || usacData->usacIndependencyFlag || usacData->FrameIsTD[left] || usacData->FrameIsTD[right]) {
      /* reset prediction coefficient history */
      for (i = 0; i < SFB_NUM_MAX; i++) {
        usacData->alpha_q_re_prev[left][i] = 0;
        usacData->alpha_q_im_prev[left][i] = 0;
      }
    }

    if (*common_window) {
      usacData->FrameWasTD[left]  = usacData->FrameIsTD[left];
      usacData->FrameWasTD[right] = usacData->FrameIsTD[right];

      usacData->FrameIsTD[left]  = usacData->coreMode[left]  = (USAC_CORE_MODE)core_mode[0];
      usacData->FrameIsTD[right] = usacData->coreMode[right] = (USAC_CORE_MODE)core_mode[1];

      usac_get_ics_info (&win[left],
                         &wshape[left].this_bk,
                         group[left],
                         &max_sfb[left],
                         qc_select,
                         hResilience,
                         hEscInstanceData,
                         hVm);

      *sfbInfo = *usac_winmap[win[left]];
      calc_gsfb_table(sfbInfo, group[left]);

      common_max_sfb = (unsigned char) GetBits( LEN_COMMON_MAX_SFB,
                                COMMON_MAX_SFB,
                                hResilience,
                                hEscInstanceData,
                                hVm );

    /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
    if ((usacData->mpegh3daProfileLevelIndication >= MPEG3DA_PLI_BASELINE_L1) && (usacData->mpegh3daProfileLevelIndication <= MPEG3DA_PLI_BASELINE_L5)) {
      if (1 != common_max_sfb) {
         fprintf(stderr,"common_max_sfb shall be 1 in Baseline Profile.\n");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }

      if (common_max_sfb == 0) {
        if (win[left] == EIGHT_SHORT_SEQUENCE) {
          *max_sfb1 = (unsigned char) GetBits( LEN_MAX_SFBS,
                              MAX_SFB,
                              hResilience,
                              hEscInstanceData,
                              hVm );
        }
        else {
          *max_sfb1 = (unsigned char) GetBits( LEN_MAX_SFBL,
                              MAX_SFB,
                              hResilience,
                              hEscInstanceData,
                              hVm );
        }
      }
      else {
        *max_sfb1 = max_sfb[left];
      }

      *max_sfb_ste = max(max_sfb[left], *max_sfb1);
      *max_sfb_ste_clear = SFB_NUM_MAX;
      if (igfConfig->igf_active) {
        if (sfbInfo->islong) {
          *igf_SfbStart = igfConfig->igf_grid[IGF_LONG].igf_SfbStart;
          *igf_SfbStop  = igfConfig->igf_grid[IGF_LONG].igf_SfbStop;
        }
        else {
          *igf_SfbStart = igfConfig->igf_grid[IGF_SHORT].igf_SfbStart;
          *igf_SfbStop  = igfConfig->igf_grid[IGF_SHORT].igf_SfbStop;
        }
        if (igfConfig->igf_IndependentTiling == IGF_STEREO) {
          *max_sfb_ste       = min(*max_sfb_ste,       *igf_SfbStart);
          *max_sfb_ste_clear = min(*max_sfb_ste_clear, *igf_SfbStart);
        }
      }

      win[left] = usacMapWindowSequences(win[left], usacData->windowSequenceLast[left]);

      hasmask[left] = getmask ( usac_winmap[win[left]],
                                group[left],
                                *max_sfb_ste,
                                mask[left],
                                hResilience,
                                hEscInstanceData,
                                hVm );

#ifndef CPLX_PRED_NOFIX
      /* zero out complete prediction coefficient history upon a transform length change, as specified in the 23003-3 standard text */
      if ((win[left] == EIGHT_SHORT_SEQUENCE && usacData->windowSequenceLast[left] != EIGHT_SHORT_SEQUENCE) || /* first ch. in CPE */
          (usacData->windowSequenceLast[left] == EIGHT_SHORT_SEQUENCE && win[left] != EIGHT_SHORT_SEQUENCE) ||
          (win[left] == EIGHT_SHORT_SEQUENCE && usacData->windowSequenceLast[right] != EIGHT_SHORT_SEQUENCE) || /* 2nd ch. in CPE */
          (usacData->windowSequenceLast[right] == EIGHT_SHORT_SEQUENCE && win[left] != EIGHT_SHORT_SEQUENCE))
      {
        for (i = 0; i < SFB_NUM_MAX; i++) {
          usacData->alpha_q_re_prev[left][i] = 0;
          usacData->alpha_q_im_prev[left][i] = 0;
        }
      }
      if ((hasmask[left] == 3) && (*stereoConfigIndex == 0))
#else
      if (hasmask[left] == 3)
#endif /* CPLX_PRED_NOFIX */
      {
        usac_get_cplx_pred_data(usacData->alpha_q_re[left],
                                usacData->alpha_q_im[left],
                                usacData->alpha_q_re_prev[left],
                                usacData->alpha_q_im_prev[left],
                                usacData->cplx_pred_used[left],
                                pred_dir,
                                complex_coef,
                                use_prev_frame,
                                sfbInfo->num_groups,
                                *max_sfb_ste,
                                *max_sfb_ste_clear,
                                usacData->usacIndependencyFlag,
                                hResilience,
                                hEscInstanceData,
                                hVm);
      }

      if (igfConfig->igf_IndependentTiling == IGF_STEREO && igfConfig->igf_active) {
        igf_hasmask[left] = IGF_get_mask(hVm,
                                         hResilience,
                                         hEscInstanceData,
                                         igfConfig,
                                         (!(sfbInfo->islong)) ? IGF_SHORT : IGF_LONG,
                                         sfbInfo,
                                         group[left],
                                         mask[left]);

#ifndef CPLX_PRED_NOFIX
        if ((igf_hasmask[left] == 3) && (*stereoConfigIndex == 0)) {
#else
        if (igf_hasmask[left] == 3) {
#endif /* CPLX_PRED_NOFIX */
          IGF_get_cplx_pred_data(hVm,
                                 hResilience,
                                 hEscInstanceData,
                                 igfConfig,
                                 (!(sfbInfo->islong)) ? IGF_SHORT : IGF_LONG,
                                 usacData->alpha_q_re[left],
                                 usacData->alpha_q_im[left],
                                 usacData->alpha_q_re_prev[left],
                                 usacData->alpha_q_im_prev[left],
                                 usacData->cplx_pred_used[left],
                                 igf_pred_dir,
                                 sfbInfo->num_groups,
                                 usacData->usacIndependencyFlag);
        }
      }
      else {
        igf_hasmask[left] = 0;
      }

      win[right] = win[left];
      memcpy(group[right], group[left], NSHORT*sizeof(unsigned char));
      wshape[right].this_bk = wshape[left].this_bk;
      *mask[right] = *mask[left];
      igf_hasmask[right] = igf_hasmask[left];
    } /* common_window */

    else{
      usacData->FrameWasTD[left]  = usacData->FrameIsTD[left];
      usacData->FrameWasTD[right] = usacData->FrameIsTD[right];

      usacData->FrameIsTD[left]  = usacData->coreMode[left]  = (USAC_CORE_MODE)core_mode[0];
      usacData->FrameIsTD[right] = usacData->coreMode[right] = (USAC_CORE_MODE)core_mode[1];

      hasmask[left] = 0;
      hasmask[right] = 0;
      igf_hasmask[left] = 0;
      igf_hasmask[right] = 0;
    }
    if ( hDec->twMdct[0] == 1 ) {           
      common_tw = GetBits ( LEN_COM_TW,
                            COMMON_TIMEWARPING,
                            hResilience,
                            hEscInstanceData,
                            hVm );
      /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
      if ((usacData->mpegh3daProfileLevelIndication >= MPEG3DA_PLI_BASELINE_L1) && (usacData->mpegh3daProfileLevelIndication <= MPEG3DA_PLI_BASELINE_L5)) {
        if (0 != common_tw) {
           fprintf(stderr,"common_tw shall be 0 in Baseline Profile.\n");
          return MPEG3DACORE_ERROR_INVALID_STREAM;
        }
      }
      if ( common_tw == 1) {
        get_tw_data(&hDec->tw_data_present[left],
                    hDec->tw_ratio[left],
                    hResilience,
                    hEscInstanceData,
                    hVm );
        hDec->tw_data_present[right]=hDec->tw_data_present[left];
        for ( i = 0 ; i < NUM_TW_NODES ; i++ ) {
          hDec->tw_ratio[right][i] = hDec->tw_ratio[left][i];
        }
      }
    }

    common_ltpf = GetBits(1,
                          COMMON_LTPF,
                          hResilience,
                          hEscInstanceData,
                          hVm);

    /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
    if ((usacData->mpegh3daProfileLevelIndication == MPEG3DA_PLI_BASELINE_L3) && (usacData->numChannels > 16) && (usacData->numChannels <= 24)) {
      if (0 != common_ltpf) {
         fprintf(stderr,"common_ltpf shall be 0 in Baseline Profile Level 3 with more than 16 objects.\n");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }
    if (common_ltpf) {
      hDec->ltpfData[left].data_present = (int) GetBits(1,
                                                        LTP_DATA_PRESENT,
                                                        hResilience,
                                                        hEscInstanceData,
                                                        hVm);
      hDec->ltpfData[right].data_present = hDec->ltpfData[left].data_present;
      if (hDec->ltpfData[left].data_present) {
        hDec->ltpfData[left].pitch_index = (int) GetBits(9,
                                                         LTP_LAG,
                                                         hResilience,
                                                         hEscInstanceData,
                                                         hVm);
        hDec->ltpfData[right].pitch_index = hDec->ltpfData[left].pitch_index;
        hDec->ltpfData[left].gain_index = (int) GetBits(2,
                                                        LTP_COEF,
                                                        hResilience,
                                                        hEscInstanceData,
                                                        hVm);
        hDec->ltpfData[right].gain_index = hDec->ltpfData[left].gain_index;
      }
    }

    if (tns_active) {
      if (*common_window) {
        common_tns = GetBits( LEN_COMMON_TNS,
                              COMMON_TNS,
                              hResilience,
                              hEscInstanceData,
                              hVm );
      }
      else {
        common_tns = 0;
      }
      if (igfConfig->igf_active && !igfConfig->igf_AfterTnsSynth) {
        *tns_on_lr = 1;
      } else {
        *tns_on_lr = GetBits(LEN_TNS_ON_LR,
                             TNS_ON_LR,
                             hResilience,
                             hEscInstanceData,
                             hVm);
      }
    /* ISO/IEC 23008-3, 4.8.2.6 Restrictions for the Baseline Profile and Levels */
    if ((usacData->mpegh3daProfileLevelIndication >= MPEG3DA_PLI_BASELINE_L1) && (usacData->mpegh3daProfileLevelIndication <= MPEG3DA_PLI_BASELINE_L5)) {
      if (1 != *tns_on_lr) {
         fprintf(stderr,"tns_on_lr shall be 1 in Baseline Profile.\n");
        return MPEG3DACORE_ERROR_INVALID_STREAM;
      }
    }

      if (common_tns) {
        usac_get_tns(sfbInfo, tns[left], hResilience, hEscInstanceData, hVm);
        *tns[right] = *tns[left];
        tns_data_present[left]  = -1;   /* -1 signals that TNS data should */
        tns_data_present[right] = -1;   /* neither be read nor cleared */
      }
      else {
        tns_present_both = GetBits( LEN_TNS_PRESENT_BOTH,
                                    TNS_PRESENT_BOTH,
                                    hResilience,
                                    hEscInstanceData,
                                    hVm );

        if (tns_present_both) {
          tns_data_present[left]  = 1;
          tns_data_present[right] = 1;
        }
        else {
          tns_data_present[right] = GetBits( LEN_TNS_DATA_PRESENT1,
                                         TNS_DATA_PRESENT,
                                         hResilience,
                                         hEscInstanceData,
                                         hVm );

          tns_data_present[left] = 1 - tns_data_present[right];
        }
      }
    }
    else {
      common_tns = 0;
      tns_data_present[left] = 0;
      tns_data_present[right] = 0;
    }
  } /* both channels fd */
  else{
    *common_window = 0;
    common_tw      = 0;
    common_ltpf    = 0;

    switch(id){
    case ID_CPE:
      usacData->FrameWasTD[right] = usacData->FrameIsTD[right];
      usacData->FrameIsTD[right]  = usacData->coreMode[right] = (USAC_CORE_MODE)core_mode[1];
      break;

    case ID_SCE:
    case ID_LFE:
      hasmask[left] = 0;
      igf_hasmask[left] = 0;
      break;

    default:
      CommonWarning("Invalid channel element (decode_chan_ele.c)");
      return MPEG3DACORE_ERROR_INVALID_ELEMENT;
      break;
    }
    usacData->FrameWasTD[left] = usacData->FrameIsTD[left];
    usacData->FrameIsTD[left]  = usacData->coreMode[left] = (USAC_CORE_MODE)core_mode[0];

  }

  for (i = left; i <= right; i++) {
    if (usacData->coreMode[i] == CORE_MODE_FD) {
      BsSetChannel(i) ;

      fltclrs(coef[i], LN2);
      ConcealmentInitChannel(i, right-left+1, hConcealment);

      if (hDec->tddec[i] && usacData->FrameWasTD[i]) {
        if( i == left ) {
          decoder_LPD_end(hDec->tddec[i] ,hVm, usacData,i);
        } else {
          if(lpdStereoIndex == 1){
            mvr2r(usacData->acelpZIR[left], usacData->acelpZIR[right], 1+(2*usacData->block_size_samples/8));
            mvr2r(usacData->lastLPC[left], usacData->lastLPC[right], 1+M);
            mvr2r(usacData->tbe_synth[left], usacData->tbe_synth[right], L_DIV_1024+2*hDec->tddec[i]->TD_resampler_delay);
            mvr2r(hDec->tddec[left]->old_gain_pf, hDec->tddec[right]->old_gain_pf, (LPD_SFD_1024/2) - BPF_SFD);
            mvi2i(hDec->tddec[left]->old_T_pf, hDec->tddec[right]->old_T_pf, (LPD_SFD_1024/2) - BPF_SFD); 
            hDec->tddec[right]->last_mode = hDec->tddec[left]->last_mode;

            stereoLpd_applyEnd(stereolpdDecData,
                               usacData->overlap_buffer[left]-1,
                               usacData->overlap_buffer[right]-1);

            mvr2r(usacData->overlap_buffer[left]+hDec->tddec[i]->lDiv-1, usacData->acelpZIR[left], 2*hDec->tddec[i]->lDiv+1);
            mvr2r(usacData->overlap_buffer[right]+hDec->tddec[i]->lDiv-1, usacData->acelpZIR[right], 2*hDec->tddec[i]->lDiv+1);
          } else {
            decoder_LPD_end(hDec->tddec[i] ,hVm, usacData,i);
          }
        }
      }

      if ((id == ID_CPE) && (usacData->coreMode[left] != usacData->coreMode[right]) ){

        tns_data_present[i]= GetBits( LEN_TNS_ACTIVE,
                                             TNS_ACTIVE,
                                             hResilience,
                                             hEscInstanceData,
                                             hVm );
      }
      {
        float tmpCoef[2048];
        short global_gain;

        error_3da = usac_get_fdcs(id,
                                  hDec,
                                  sfbInfo+(i-left),
                                  usacData,
                                  *common_window,
                                  common_tw,
                                  tns_data_present[i],
                                  core_mode,
                                  left,
                                  right,
                                  &win[i],
                                  &wshape[i].this_bk,
                                  group[i],
                                  (i > left) ? max_sfb1 : &max_sfb[i],
                                  cb_map[i],
                                  tmpCoef,
                                  max_spec_coefficients + (i-left),
                                  &global_gain,
                                  factors[i],
                                  &hDec->arithPreviousSize[i],
                                  hDec->arithQ[i],
                                  tns[i],
                                  &hDec->tw_data_present[i],
                                  hDec->tw_ratio[i],
                                  hResilience,
                                  hHcrSpecData,
                                  hHcrInfo,
                                  hEscInstanceData,
                                  hConcealment,
                                  qc_select,
                                  &(hDec->nfSeed[i]),
                                  hVm,
                                  usacData->windowSequenceLast[i],
                                  usacData->FrameWasTD[i],
                                  usacData->usacIndependencyFlag,
                                  usacData->dmx_re_prev[left],
                                  i,
                                  bNoiseFilling,
                                  stereoFilling,
                                  *pred_dir,
                                  common_ltpf,
                                  igfConfig,
                                  i-left);
        if (MPEG3DACORE_OK != error_3da) {
          CommonWarning("usac_get_fdcs returns error");
          return error_3da;
        }

        for(k=0;k<hDec->blockSize + hDec->blockSize/8; k++){
          hDec->coef[i][k] = tmpCoef[k];
        }
      }

#ifdef UNIFIED_RANDOMSIGN
      if (hDec->tddec[i]) {
        /* sync random noise seeds, tddec is not available for the LFE */
        hDec->tddec[i]->seed_tcx = hDec->nfSeed[i];
      }
#endif

    } /* CORE_MODE_FD */
    else {
#ifdef UNIFIED_RANDOMSIGN
      if (hDec->tddec[i]) {
        /* sync random noise seeds, tddec is not available for the LFE */
        hDec->tddec[i]->seed_tcx = hDec->nfSeed[i];
      }
#endif

      BsSetChannel(i);

      usac_past_tw(  usacData->overlap_buffer[i],
                     usacData->block_size_samples,
                     usacData->block_size_samples/8,
                     usacData->windowShape[i],
                     usacData->prev_windowShape[i],
                     usacData->windowSequenceLast[i],
                     td_frame.mod[0],
                     usacData->FrameWasTD[i],
                     usacData->twMdct[0],                   
                     usacData->prev_sample_pos[i],
                     usacData->prev_tw_trans_len[i],
                     usacData->prev_tw_start_stop[i],
                     usacData->prev_warped_time_sample_vector[i]);


      if (!usacData->FrameWasTD[i]) {
        if ( usacData->twMdct[0] ) {            
          reset_decoder_lf(hDec->tddec[i],&usacData->overlap_buffer[i][512], NULL, (usacData->windowSequenceLast[i] == EIGHT_SHORT_SEQUENCE), 1); /*JEREMIE: -1 for lastWasShort, put correct value here*/
          hDec->tddec[i]->igfDecData->igf_memory.igf_prevWinType = IGF_CODEC_TRANSITION;
        }
        else {
          if (hDec->tddec[i]->fullbandLPD) {
            int k;
            const float *pWinCoeff;
            float bufferLB[1+2*L_DIV_1024+L_FRAME_1024];
            int lfac = (usacData->windowSequenceLast[i] == EIGHT_SHORT_SEQUENCE) ? 64 : 128;
            int TD_resampler_delay = (hDec->tddec[i]->fscaleFB < hDec->tddec[i]->fscale) ? L_FILT_MAX : L_FILT_MAX/2;

            /*set to zero FD past samples*/
            set_zero(bufferLB,1+2*L_DIV_1024+L_FRAME_1024);

            /* window overlap buffer prior to resampling */
            hDec->tddec[i]->window_shape_prev = wshape[i].prev_bk;
            if  (lfac==48) {
              pWinCoeff = (wshape[i].prev_bk == 0) ? sineWindow96 : kbdWindow96;
            }
            else if  (lfac==64) {
              pWinCoeff = (wshape[i].prev_bk == 0) ? sineWindow128 : kbdWindow128; 
            }
            else if (lfac==96) {
              pWinCoeff = (wshape[i].prev_bk == 0) ? sineWindow192 : kbdWindow192;;
            }
            else {
              pWinCoeff = (wshape[i].prev_bk == 0) ? sineWindow256 : kbdWindow256;;
            }
            if ((lpdStereoIndex == 0) || ((lpdStereoIndex == 1) && (i == left))) {
              if (lpdStereoIndex == 1) {
                for (k = 0; k < 2*lfac; k++){
                  usacData->overlap_buffer[left][(hDec->tddec[i]->lFrame)-lfac+k] *= pWinCoeff[2*lfac-1-k];
                  usacData->overlap_buffer[right][(hDec->tddec[i]->lFrame)-lfac+k] *= pWinCoeff[2*lfac-1-k];
                }

                set_zero(usacData->overlap_buffer[left]+(hDec->tddec[i]->lFrame)+lfac, (hDec->tddec[i]->lFrame)-lfac);
                set_zero(usacData->overlap_buffer[right]+(hDec->tddec[i]->lFrame)+lfac, (hDec->tddec[i]->lFrame)-lfac);

                stereoLpd_setPastL(stereolpdDecData,
                                   usacData->overlap_buffer[left]-1,
                                   usacData->block_size_samples/2 + lfac);
                stereoLpd_setPastR(stereolpdDecData,
                                   usacData->overlap_buffer[right]-1,
                                   usacData->block_size_samples/2 + lfac);

                for (k = 0; k < usacData->block_size_samples; k++) {
                  prevBuffer[k] = 0.5f*(usacData->overlap_buffer[left][k] + usacData->overlap_buffer[right][k]);
                }
              } else {
                for (k = 0; k < 2*lfac; k++){
                  usacData->overlap_buffer[i][(hDec->tddec[i]->lFrame)-lfac+k] *= pWinCoeff[2*lfac-1-k];
                }

                set_zero(usacData->overlap_buffer[i]+(hDec->tddec[i]->lFrame)+lfac, (hDec->tddec[i]->lFrame)-lfac);

                for (k = 0; k < usacData->block_size_samples; k++) {
                  prevBuffer[k] = usacData->overlap_buffer[i][k];
                }
              }

              /* downsample FD past samples by 2 with an offset equal to the delay introduced by the resampler*/
              TD_resampler(prevBuffer, 1+2*hDec->tddec[i]->lDiv+lfac, hDec->tddec[i]->fscaleFB,
                           bufferLB+hDec->tddec[i]->lDiv, hDec->tddec[i]->fscale, NULL, 0);

              reset_decoder_lf(hDec->tddec[i], bufferLB, prevBuffer, (usacData->windowSequenceLast[i] == EIGHT_SHORT_SEQUENCE), 0);
            }
            hDec->tddec[i]->igfDecData->igf_memory.igf_prevWinType = IGF_CODEC_TRANSITION;
          } else {
            reset_decoder_lf(hDec->tddec[i], usacData->overlap_buffer[i], NULL, (usacData->windowSequenceLast[i] == EIGHT_SHORT_SEQUENCE), 0);
          }
        }
      }

      if ((lpdStereoIndex == 0) || ((lpdStereoIndex == 1) && (i == left))) {
        usac_get_tdcs( hDec->tddec[i],
                       usacData->mpegh3daProfileLevelIndication,
                       &hDec->arithPreviousSize[i],
                       hDec->arithQ[i],
                       &td_frame,
                       igfConfig,
                       hResilience,
                       hEscInstanceData,
                       hVm,
                       &isAceStart,
                       &short_fac_flag,
                       &isBassPostFilter,
                       usacData->usacIndependencyFlag,
                       &(hDec->ltpfData[i])
                       );

        if (usacData->First_frame_flag[i] == 1) {
          usacData->First_frame_flag[i] = 0;
          hDec->tddec[i]->window_shape_prev = (WINDOW_SHAPE)hDec->tddec[i]->window_shape;
        }

        decoder_LPD(hDec->tddec[i],
                    &td_frame,
                    usacData->usacIndependencyFlag,
                    group[i],
                    igfConfig,
                    stereolpdDecData,
                    hDec->fscale,
                    synth,
                    bad_frame,
                    isAceStart,
                    short_fac_flag,
                    isBassPostFilter);

        if (lpdStereoIndex == 1) {
          stereoLpd_data(stereolpdDecData,
                         hResilience,
                         hEscInstanceData,
                         hVm,
                         !usacData->FrameWasTD[i]);
          stereoLpd_apply(stereolpdDecData,
                          synth_stereo[left],
                          synth_stereo[right],
                          !usacData->FrameWasTD[i]);
          usac_ltpf_bs_duplicate(hDec->ltpfData[left],&(hDec->ltpfData[right]));
        }

        if (!usacData->FrameWasTD[i]) {
          int lfac = (usacData->windowSequenceLast[i] == EIGHT_SHORT_SEQUENCE) ? 64 : 128;
          int lDiv = hDec->tddec[i]->lDiv;

          if (lpdStereoIndex == 1) {

            mvr2r(synth_stereo[left], usacData->overlap_buffer[left], 2*lDiv-lfac);
            mvr2r(synth_stereo[right], usacData->overlap_buffer[right], 2*lDiv-lfac);

            mvr2r(prevBuffer+2*lDiv-lfac, usacData->overlap_buffer[left]+2*lDiv-lfac, lfac);
            mvr2r(prevBuffer+2*lDiv-lfac, usacData->overlap_buffer[right]+2*lDiv-lfac, lfac);

            set_zero(usacData->overlap_buffer[left]+2*lDiv-lfac, 2*lfac);
            set_zero(usacData->overlap_buffer[right]+2*lDiv-lfac, 2*lfac);
          }
          else if (hDec->tddec[i]->fullbandLPD) {

            mvr2r(prevBuffer+2*lDiv-lfac, usacData->overlap_buffer[i]+2*lDiv-lfac, lfac);
            set_zero(usacData->overlap_buffer[i]+2*lDiv-lfac, 2*lfac);
          }
        }
      }

      if (lpdStereoIndex == 0) {
        usac_td2buffer(synth,
                       usacData->time_sample_vector[i],
                       usacData->overlap_buffer[i],
                       usacData->block_size_samples,
                       usacData->block_size_samples/8,
                       usacData->windowSequenceLast[i],
                       td_frame.mod[0],
                       usacData->FrameWasTD[i],
                       usacData->twMdct[0],                       
                       usacData->prev_sample_pos[i],
                       usacData->prev_tw_trans_len[i],
                       usacData->prev_tw_start_stop[i],
                       usacData->prev_warped_time_sample_vector[i]);

        usac_ltpf_process(&hDec->ltpfData[i], usacData->time_sample_vector[i]);  
      }
      else if(i == left) {
        for (k=left; k<=right; k++) {
          usac_td2buffer(synth_stereo[k],
                         usacData->time_sample_vector[k],
                         usacData->overlap_buffer[k],
                         usacData->block_size_samples,
                         usacData->block_size_samples/8,
                         usacData->windowSequenceLast[k],
                         td_frame.mod[0],
                         usacData->FrameWasTD[k],
                         usacData->twMdct[0],                       
                         usacData->prev_sample_pos[k],
                         usacData->prev_tw_trans_len[k],
                         usacData->prev_tw_start_stop[k],
                         usacData->prev_warped_time_sample_vector[k]);

          usac_ltpf_process(&hDec->ltpfData[k], usacData->time_sample_vector[k]);
        }
      }

      usacData->prev_windowShape[i] = wshape[i].prev_bk = (WINDOW_SHAPE)hDec->tddec[i]->window_shape;
      usacData->windowSequenceLast[i] = EIGHT_SHORT_SEQUENCE;

      for (k = 0; k < 2*172; k++) {
        hDec->quantSpecPrev[i][k] = 0; /* reset FD prediction memory */
      }
      hDec->prevAliasingSymmetry[i] = hDec->currAliasingSymmetry[i] = 0;

    } /* usacData->coreMode[i] == CORE_MODE_FD */

#ifdef UNIFIED_RANDOMSIGN
    if (hDec->tddec[i]) {
      /* sync random noise seeds, tddec is not available for the LFE */
      hDec->nfSeed[i] = hDec->tddec[i]->seed_tcx;
    }
#endif

  } /* end channel loop */

  return MPEG3DACORE_OK;
}

int processChannelElement ( int                      id,
                            MC_Info*                 mip,
                            USAC_DATA               *usacData,
                            WINDOW_SEQUENCE*         win,
                            Wnd_Shape*               wshape,
                            byte**                   cb_map,
                            short**                  factors,
                            byte**                   group,
                            byte*                    hasmask,
                            byte**                   mask,
                            byte*                    max_sfb,
                            TNS_frame_info**         tns,
                            float**                  coef,
                            Info*                    sfbInfo,
                            QC_MOD_SELECT            qc_select,
                            HANDLE_DECODER_GENERAL   hFault,
                            int                      elementTag,
                            int                      bNoiseFilling,
                            HSTEREOLPD_DEC_DATA      stereolpdDecData,
                            int                      lpdStereoIndex,
                            IGF_CONFIG              *igfConfig,
                            int                      channelOffset,
                            int                      stereoFilling,
                            int                      tns_on_lr,
                            int                     *core_mode,
                            unsigned char            max_sfb1,
                            unsigned char            max_sfb_ste,
                            unsigned char            max_sfb_ste_clear,
                            int                     *max_spec_coefficients,
                            int                      complex_coef,
                            int                      use_prev_frame,
                            byte                    *igf_hasmask,
                            int                      igf_pred_dir,
                            int                      igf_SfbStart,
                            int                      igf_SfbStop,
                            int                      pred_dir,
                            int                      common_window
#ifdef SONY_PVC_DEC
                           ,int                     *cm
#endif /* SONY_PVC_DEC */
#ifndef CPLX_PRED_NOFIX
                           ,int                      stereoConfigIndex
#endif /* CPLX_PRED_NOFIX */
                            )
{
  int i=0, k=0,i_ch=0, ch=0, widx=0;
  int wn = 0;
  int igf_Cnt = 0;
  int igf_NT  = 4;
  Wnd_Shape tmpWshape;
  HANDLE_USAC_DECODER      hDec             = usacData->usacDecoder;
  HANDLE_ESC_INSTANCE_DATA hEscInstanceData = hFault[0].hEscInstanceData;

  int left  = channelOffset;
  int right = (id != ID_CPE) ? channelOffset : (channelOffset+1);

  tmpWshape = wshape[left];

  for (k = 0; k < 2; k++) { /* order of TNS and stereo processing depends on tns_on_lr */
    if (k != tns_on_lr) {   /* if tns_on_lr apply stereo processing first */
      if(core_mode[0] == CORE_MODE_FD && core_mode[1] == CORE_MODE_FD && id == ID_CPE && common_window == 1){

        /* stereo processing */
        if(((channelOffset+k) == left)||(!tns_on_lr)){ /* take tns_on_lr == 0 into account!!! */
          wn = left;
          hDec->info = usac_winmap[hDec->wnd[wn]];
          calc_gsfb_table(hDec->info, group[wn]);
          map_mask(hDec->info, hDec->group[wn], hDec->mask[wn], hDec->cb_map[right], hasmask[wn]);
          if (hasmask[wn] != 3) {
            int sfb;
            for (sfb = 0; sfb < max_sfb_ste_clear ; sfb++) {
              usacData->alpha_q_re_prev[left][sfb] = 0;
              usacData->alpha_q_im_prev[left][sfb] = 0;
            }
          }

          if (igfConfig->igf_active) {
            if(hDec->info->islong) {
              igf_NT = igfConfig->igf_grid[IGF_LONG].igf_NTiles;
            }
            else {
              igf_NT = igfConfig->igf_grid[IGF_SHORT].igf_NTiles;
            }
          }

#ifndef CPLX_PRED_NOFIX
          if ((hasmask[wn] == 3) && (stereoConfigIndex == 0)) {
#else
          if (hasmask[wn] == 3) {
#endif /* CPLX_PRED_NOFIX */
            if (igfConfig->igf_active) {
              for (igf_Cnt = 0; igf_Cnt < igf_NT; igf_Cnt++) {
                usac_decode_cplx_pred(sfbInfo,
                                      group[wn],
                                      &hDec->igfDecData[left].igf_infSpec[2048 * igf_Cnt],
                                      &hDec->igfDecData[right].igf_infSpec[2048 * igf_Cnt],
                                      usacData->dmx_re_prev[left],
                                      usacData->alpha_q_re[left],
                                      usacData->alpha_q_im[left],
                                      usacData->cplx_pred_used[left],
                                      pred_dir,
                                      complex_coef,
                                      use_prev_frame,
                                      0,
                                      max_sfb_ste,
                                      win[wn],
                                      tmpWshape.this_bk,
                                      tmpWshape.prev_bk,
                                      g_bSplitTransform[left]);
              }
            }
            usac_decode_cplx_pred(sfbInfo,
                                  group[wn],
                                  hDec->coef[left],
                                  hDec->coef[right],
                                  usacData->dmx_re_prev[left],
                                  usacData->alpha_q_re[left],
                                  usacData->alpha_q_im[left],
                                  usacData->cplx_pred_used[left],
                                  pred_dir,
                                  complex_coef,
                                  use_prev_frame,
                                  0,
                                  max_sfb_ste,
                                  win[wn],
                                  tmpWshape.this_bk,
                                  tmpWshape.prev_bk,
                                  g_bSplitTransform[left]);
          } else if (hasmask[wn]) {
            if(igfConfig->igf_active) {
              for (igf_Cnt = 0; igf_Cnt < igf_NT; igf_Cnt++) {
                usac_synt(hDec->info,
                          hDec->group[wn],
                          hDec->mask[wn],
                          0,
                          igf_SfbStart,
                          &hDec->igfDecData[right].igf_infSpec[2048 * igf_Cnt],
                          &hDec->igfDecData[left].igf_infSpec[2048 * igf_Cnt]);
              }
            }
            usac_synt(hDec->info,
                      hDec->group[wn],
                      hDec->mask[wn],
                      0,
                      max_sfb_ste,
                      hDec->coef[right],
                      hDec->coef[left]);
          }

          /* joint stereo IGF if before TnsSynth */
          if(
                 igfConfig->igf_IndependentTiling == IGF_STEREO
              && igfConfig->igf_active
              && !igfConfig->igf_AfterTnsSynth
              ) {
            IGF_apply_stereo(&(hDec->igfDecData[left]),
                             &(hDec->igfDecData[right]),
                             igfConfig,
                             (win[wn] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                             (win[wn] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                             hDec->coef[left],
                             hDec->coef[right],
                             &(hDec->nfSeed[left]),
                             &(hDec->nfSeed[right]),
                             win[wn],
                             group[wn],
                             mask[wn],
                             (const int **)usacData->cplx_pred_used[wn],
                             igf_hasmask[wn],
                             hDec->info->num_groups,
                             hDec->info->group_len,
                             hDec->info->bins_per_sbk,
                             hDec->info->sfb_per_sbk,
                             hDec->blockSize,
                             hDec->samplingRate);

            if (igf_hasmask[wn] != 3) {
              int sfb;
              for (sfb = igf_SfbStart; sfb < SFB_NUM_MAX ; sfb++) {
                usacData->alpha_q_re_prev[left][sfb] = 0;
                usacData->alpha_q_im_prev[left][sfb] = 0;
              }
            }

#ifndef CPLX_PRED_NOFIX
            if ((igf_hasmask[wn] == 3) && (stereoConfigIndex == 0))
#else
            if (igf_hasmask[wn] == 3)
#endif /* CPLX_PRED_NOFIX */
            {
              usac_decode_cplx_pred(sfbInfo,
                                    group[wn],
                                    hDec->coef[left],
                                    hDec->coef[right],
                                    usacData->dmx_re_prev[left],
                                    usacData->alpha_q_re[left],
                                    usacData->alpha_q_im[left],
                                    usacData->cplx_pred_used[left],
                                    igf_pred_dir,
                                    0, /* complex_coef */
                                    0, /* use_prev_frame */
                                    igf_SfbStart,
                                    igf_SfbStop,
                                    win[wn],
                                    tmpWshape.this_bk,
                                    tmpWshape.prev_bk,
                                    g_bSplitTransform[left]);

              if (igfConfig->igf_UseTNF && EIGHT_SHORT_SEQUENCE != win[wn]) {
                usac_decode_cplx_pred(sfbInfo,
                                      group[wn],
                                      hDec->igfDecData[left].igf_tnfSpec,
                                      hDec->igfDecData[right].igf_tnfSpec,
                                      usacData->dmx_re_prev[left],
                                      usacData->alpha_q_re[left],
                                      usacData->alpha_q_im[left],
                                      usacData->cplx_pred_used[left],
                                      igf_pred_dir,
                                      0, /* complex_coef */
                                      0, /* use_prev_frame */
                                      igf_SfbStart,
                                      igf_SfbStop,
                                      win[wn],
                                      tmpWshape.this_bk,
                                      tmpWshape.prev_bk,
                                      g_bSplitTransform[left]);
              }
            }
            else
            if (igf_hasmask[wn]) {
              usac_synt(hDec->info,
                        hDec->group[wn],
                        hDec->mask[wn],
                        igf_SfbStart,
                        igf_SfbStop,
                        hDec->coef[right],
                        hDec->coef[left]);

              if (igfConfig->igf_UseTNF && EIGHT_SHORT_SEQUENCE != win[wn]) {
                usac_synt(hDec->info,
                          hDec->group[wn],
                          hDec->mask[wn],
                          igf_SfbStart,
                          igf_SfbStop,
                          hDec->igfDecData[right].igf_tnfSpec,
                          hDec->igfDecData[left].igf_tnfSpec);
              }
            }
            for (ch = left; ch <= right; ch++) {
              IGF_apply_tnf(&(hDec->igfDecData[ch]),
                            igfConfig,
                            (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                            (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                            hDec->coef[ch],
                            win[ch],
                            group[ch]);
            }
          }
        }
      }
    }
    else {  /* if not tns_on_lr apply TNS first */
    for(ch = left; ch <= right; ch++){

        if(usacData->coreMode[ch] == CORE_MODE_FD){
          BsSetChannel(ch);

          /* if (!(mip->ch_info[ch].present)) continue; */
          wn = ch;
          hDec->info = usac_winmap[hDec->wnd[wn]];
          hDec->wnd_shape[ch].prev_bk = hDec->wnd_shape[wn].this_bk;
          calc_gsfb_table(hDec->info, group[ch]);
          if (! ( hEscInstanceData && ( BsGetErrorForDataElementFlagEP ( N_FILT       , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( COEF_RES     , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( LENGTH       , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( ORDER        , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( DIRECTION    , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( COEF_COMPRESS, hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( COEF         , hEscInstanceData ) ||
                                        BsGetErrorForDataElementFlagEP ( GLOBAL_GAIN  , hEscInstanceData ) ) ) )
            {

              int i, j;

              if (igfConfig->igf_active) {
                if(hDec->info->islong) {
                  igf_NT = igfConfig->igf_grid[IGF_LONG].igf_NTiles;
                }
                else {
                  igf_NT = igfConfig->igf_grid[IGF_SHORT].igf_NTiles;
                }
              }

              /* IGF if before TnsSynth */
              if(
                    igfConfig->igf_active
                 && !igfConfig->igf_AfterTnsSynth
                 && (igfConfig->igf_IndependentTiling == IGF_DUALMONO || common_window == 0)
                 ) {
                IGF_apply_mono(&hDec->igfDecData[ch],
                               igfConfig,
                               (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                               (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                               hDec->coef[ch],
                               &(hDec->nfSeed[ch]),
                               win[ch],
                               group[ch],
                               hDec->info->num_groups,
                               hDec->info->group_len,
                               hDec->info->bins_per_sbk,
                               hDec->blockSize,
                               hDec->samplingRate);

                IGF_apply_tnf(&hDec->igfDecData[ch],
                              igfConfig,
                              (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                              (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                              hDec->coef[ch],
                              win[ch],
                              group[ch]);
              }

              /*  de-interleave spectra in case of transform splitting (which is neccessary if TNS is applied) */
              if(((win[ch]==STOP_START_SEQUENCE)||(win[ch]==LONG_START_SEQUENCE)) && g_bSplitTransform[ch]) {

                float pcmbuf[2048]={0};
                float tmpbuf1[2048]={0};
                float tmpbuf2[2048]={0};
                float* spectrum;
                int i,i2;
                const int longBlockSize=(hDec->info)->bins_per_bk;
                int halfLowpassLine=max_spec_coefficients[ch-left]/2;

                if (igfConfig->igf_active) {
                  halfLowpassLine = usac_winmap[win[ch]]->sbk_sfb_top[0][igfConfig->igf_grid[IGF_LONG].igf_SfbStop - 1] / 2;
                }

                spectrum=hDec->coef[ch];

                i2 = 0;
                for (i = 0; i < halfLowpassLine; i++, i2 += 2) {
                  spectrum[i] = spectrum[i2]; /* 1st window */
                  pcmbuf[i] = spectrum[1+i2]; /* 2nd window */
                }

                memcpy(spectrum+halfLowpassLine, pcmbuf, halfLowpassLine*sizeof(float));
              }
              for (i = j = 0; i < hDec->tns[ch]->n_subblocks; i++) {
                if (debug['T']) {
                  if ( win[wn] == EIGHT_SHORT_SEQUENCE ) {
                    fprintf(stderr, "%ld %d %d\n", bno, ch, i);
                    print_tns( &(hDec->tns[ch]->info[i]));
                  }
                }


                if (igfConfig->igf_active && igfConfig->igf_AfterTnsSynth) {
                  for (igf_Cnt = 0; igf_Cnt < igf_NT; igf_Cnt++) {
                    usac_tns_decode_subblock(&hDec->igfDecData[ch].igf_infSpec[2048 * igf_Cnt + j],
                                             (ch > left) ? max_sfb1 : max_sfb[wn],
                                             hDec->info->sbk_sfb_top[i],
                                             hDec->info->islong,
                                              &(hDec->tns[ch]->info[i]),
                                             qc_select,
                                             hDec->samplingRateIdx,
                                             igfConfig);
                  }
                }
                usac_tns_decode_subblock(&hDec->coef[ch][j],
                                         (ch > left) ? max_sfb1 : max_sfb[wn],
                                         hDec->info->sbk_sfb_top[i],
                                         hDec->info->islong,
                                         &(hDec->tns[ch]->info[i]),
                                         qc_select,
                                         hDec->samplingRateIdx,
                                         igfConfig);

                j += hDec->info->bins_per_sbk[i];
              }

              /* always interleave spectra again in case of transform splitting */
              /* as all stereo tools expect interleaved spectra */
              if(((win[ch] == STOP_START_SEQUENCE)||(win[ch]==LONG_START_SEQUENCE)) && g_bSplitTransform[ch]){

                float* spectrum;
                float pcmbuf[2048]={0};
                int i,i2;
                const int longBlockSize=(hDec->info)->bins_per_bk;
                int halfLowpassLine=max_spec_coefficients[ch-left]/2;

                if (igfConfig->igf_active) {
                  halfLowpassLine = usac_winmap[win[ch]]->sbk_sfb_top[0][igfConfig->igf_grid[IGF_LONG].igf_SfbStop - 1] / 2;
                }

                spectrum=hDec->coef[ch];

                /* re-interleave spectra */
                memcpy(pcmbuf, spectrum, halfLowpassLine*sizeof(float));
                i2 = 0;
                for (i = 0; i < halfLowpassLine; i++, i2 += 2) {
                  spectrum[i2]   = pcmbuf[i];
                  spectrum[i2+1] = spectrum[i+halfLowpassLine];
                }
                for (/*i2*/; i2 < longBlockSize; i2++) {
                  spectrum[i2]   = 0.0F;
                }
              }

              /* IGF if after TnsSynth */
              if(
                    igfConfig->igf_active
                 && igfConfig->igf_AfterTnsSynth
                 && (igfConfig->igf_IndependentTiling == IGF_DUALMONO || common_window == 0)
                  ) {
                IGF_apply_mono(&hDec->igfDecData[ch],
                               igfConfig,
                               (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                               (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                               hDec->coef[ch],
                               &(hDec->nfSeed[ch]),
                               win[ch],
                               group[ch],
                               hDec->info->num_groups,
                               hDec->info->group_len,
                               hDec->info->bins_per_sbk,
                               hDec->blockSize,
                               hDec->samplingRate);

                IGF_apply_tnf(&hDec->igfDecData[ch],
                              igfConfig,
                              (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                              (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                              hDec->coef[ch],
                              win[ch],
                              group[ch]);
              }
            }
          else {
            if (!hEscInstanceData || BsGetEpDebugLevel ( hEscInstanceData ) >= 2) {
              printf( "AacDecodeFrame: tns data disturbed\n" );
              printf( "AacDecodeFrame: --> tns disabled (channel %d)\n", ch );
            }
          }
        }
      } /* end channel loop */
    }
  }

  /* joint stereo IGF if after TnsSynth */
  if(
        igfConfig->igf_active
     && igfConfig->igf_IndependentTiling == IGF_STEREO
     && igfConfig->igf_AfterTnsSynth
     && common_window == 1
     ) {
    wn = left;
    hDec->info = usac_winmap[hDec->wnd[wn]];
    calc_gsfb_table(hDec->info, group[wn]);
    IGF_apply_stereo(&hDec->igfDecData[left],
                     &hDec->igfDecData[right],
                     igfConfig,
                     (win[wn] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                     (win[wn] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                     hDec->coef[left],
                     hDec->coef[right],
                     &(hDec->nfSeed[left]),
                     &(hDec->nfSeed[right]),
                     win[wn],
                     group[wn],
                     mask[wn],
                     (const int **)usacData->cplx_pred_used[wn],
                     igf_hasmask[wn],
                     hDec->info->num_groups,
                     hDec->info->group_len,
                     hDec->info->bins_per_sbk,
                     hDec->info->sfb_per_sbk,
                     hDec->blockSize,
                     hDec->samplingRate);

    if (igf_hasmask[wn] != 3) {
      int sfb;
      for (sfb = igf_SfbStart; sfb < SFB_NUM_MAX ; sfb++) {
        usacData->alpha_q_re_prev[left][sfb] = 0;
        usacData->alpha_q_im_prev[left][sfb] = 0;
      }
    }

#ifndef CPLX_PRED_NOFIX
    if ((igf_hasmask[wn] == 3) && (stereoConfigIndex == 0)) {
#else
    if (igf_hasmask[wn] == 3) {
#endif /* CPLX_PRED_NOFIX */
      usac_decode_cplx_pred(sfbInfo,
                            group[wn],
                            hDec->coef[left],
                            hDec->coef[right],
                            usacData->dmx_re_prev[left],
                            usacData->alpha_q_re[left],
                            usacData->alpha_q_im[left],
                            usacData->cplx_pred_used[left],
                            igf_pred_dir,
                            0, /* complex_coef */
                            0, /* use_prev_frame */
                            igf_SfbStart,
                            igf_SfbStop,
                            win[wn],
                            tmpWshape.this_bk,
                            tmpWshape.prev_bk,
                            g_bSplitTransform[left]);

      if (igfConfig->igf_UseTNF && EIGHT_SHORT_SEQUENCE != win[wn]) {
        usac_decode_cplx_pred(sfbInfo,
                              group[wn],
                              hDec->igfDecData[left].igf_tnfSpec,
                              hDec->igfDecData[right].igf_tnfSpec,
                              usacData->dmx_re_prev[left],
                              usacData->alpha_q_re[left],
                              usacData->alpha_q_im[left],
                              usacData->cplx_pred_used[left],
                              igf_pred_dir,
                              0, /* complex_coef */
                              0, /* use_prev_frame */
                              igf_SfbStart,
                              igf_SfbStop,
                              win[wn],
                              tmpWshape.this_bk,
                              tmpWshape.prev_bk,
                              g_bSplitTransform[left]);
      }
    }
    else
    if (igf_hasmask[wn]) {
      usac_synt(hDec->info,
                hDec->group[wn],
                hDec->mask[wn],
                igf_SfbStart,
                igf_SfbStop,
                hDec->coef[right],
                hDec->coef[left]);

      if (igfConfig->igf_UseTNF && EIGHT_SHORT_SEQUENCE != win[wn]) {
        usac_synt(hDec->info,
                  hDec->group[wn],
                  hDec->mask[wn],
                  igf_SfbStart,
                  igf_SfbStop,
                  hDec->igfDecData[right].igf_tnfSpec,
                  hDec->igfDecData[left].igf_tnfSpec);
      }
    }
    for (ch = left; ch <= right; ch++) {
      IGF_apply_tnf(&(hDec->igfDecData[ch]),
                    igfConfig,
                    (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_SHORT : IGF_LONG,
                    (win[ch] == EIGHT_SHORT_SEQUENCE) ? IGF_ID_SHORT : IGF_ID_LONG,
                    hDec->coef[ch],
                    win[ch],
                    group[ch]);
    }
  }

  if(core_mode[0] == CORE_MODE_FD && core_mode[1] == CORE_MODE_FD && id == ID_CPE){

    /* downmix has to be calculated in the next frame because */
    /* it depends on that frame's prediction direction        */
    const int zeroCoefSave = ((win[left] == EIGHT_SHORT_SEQUENCE && win[right] != EIGHT_SHORT_SEQUENCE) ||
                              (win[left] != EIGHT_SHORT_SEQUENCE && win[right] == EIGHT_SHORT_SEQUENCE) ||
                              (g_bSplitTransform[left] == 0 && g_bSplitTransform[right] != 0) ||
                              (g_bSplitTransform[left] != 0 && g_bSplitTransform[right] == 0));

    usac_cplx_save_prev(sfbInfo,
                        hDec->coef[left],
                        hDec->coef[right],
                        hDec->coefSave[left],
                        hDec->coefSave[right],
                        zeroCoefSave);
  }

  for (ch=left; ch<=right; ch++){
    if(usacData->coreMode[ch] == CORE_MODE_FD){
      for (i=0;i<(hDec->blockSize); i++){
        usacData->spectral_line_vector[ch][i] = hDec->coef[ch][i];
      }
    }
  }


  for (i_ch=left; i_ch<=right; i_ch++){
    if(usacData->coreMode[i_ch] == CORE_MODE_FD){
      widx = i_ch;
      usacData->windowShape[i_ch] = wshape[widx].this_bk;
      usacData->windowSequence[i_ch] = win[widx];
    }
  }
  for (i_ch=left; i_ch<=right; i_ch++){

    if(usacData->coreMode[i_ch] == CORE_MODE_FD){
      float sample_pos[1024*3];
      float tw_trans_len[2];
      int   tw_start_stop[2];

      if ( usacData->twMdct[0] ) {            
        WINDOW_SEQUENCE winSeq = usacData->windowSequence[i_ch];
        WINDOW_SEQUENCE winSeqLast = usacData->windowSequenceLast[i_ch];

        if ( usacData->FrameWasTD[i_ch] ) {
          tw_reset(usacData->block_size_samples,
                   usacData->warp_cont_mem[i_ch],
                   usacData->warp_sum[i_ch]);
        }

        tw_calc_tw(hDec->tw_data_present[i_ch],
                   usacData->FrameWasTD[i_ch],
                   hDec->tw_ratio[i_ch],
                   winSeq,
                   usacData->warp_cont_mem[i_ch],
                   sample_pos,
                   tw_trans_len,
                   tw_start_stop,
                   usacData->warp_sum[i_ch],
                   usacData->block_size_samples
                   );


        tw_adjust_past(winSeq,
                       winSeqLast,
                       usacData->FrameWasTD[i_ch],
                       usacData->prev_tw_start_stop[i_ch],
                       usacData->prev_tw_trans_len[i_ch],
                       usacData->block_size_samples);

      }

      if(usacData->First_frame_flag[i_ch]==1){
        usacData->prev_windowShape[i_ch]=usacData->windowShape[i_ch];
        usacData->First_frame_flag[i_ch]=0;
      }

      fd2buffer (usacData->spectral_line_vector[i_ch],
                 usacData->time_sample_vector[i_ch],
                 usacData->overlap_buffer[i_ch],
                 usacData->windowSequence[i_ch],
                 usacData->windowSequenceLast[i_ch],
                 usacData->block_size_samples,
                 usacData->block_size_samples/8,
                 usacData->windowShape[i_ch],
                 usacData->prev_windowShape[i_ch],
                 OVERLAPPED_MODE,
                 8,
                 usacData->FrameWasTD[i_ch],
                 usacData->twMdct[0],           
                 sample_pos,
                 tw_trans_len,
                 tw_start_stop,
                 usacData->prev_sample_pos[i_ch],
                 usacData->prev_tw_trans_len[i_ch],
                 usacData->prev_tw_start_stop[i_ch],
                 usacData->prev_warped_time_sample_vector[i_ch],
                 hDec->prevAliasingSymmetry[i_ch], /* left-side boundary condition */
                 hDec->currAliasingSymmetry[i_ch], /* right-side boundary symmetry */
                 hDec->ApplyFAC[i_ch],
                 hDec->facData[i_ch],
                 usacData->lastLPC[i_ch],
                 usacData->acelpZIR[i_ch],
                 usacData->tbe_synth[i_ch],
                 stereolpdDecData,
                 lpdStereoIndex,
                 (i_ch == right), /* relative indexing 0->left, 1->right */
                 hDec->tddec[i_ch],
                 NULL,
                 hEscInstanceData,
                 g_bSplitTransform[i_ch]);

      usac_ltpf_process(&hDec->ltpfData[i_ch], usacData->time_sample_vector[i_ch]);

      usacData->prev_windowShape[i_ch] = usacData->windowShape[i_ch];
      usacData->windowSequenceLast[i_ch] =  usacData->windowSequence[i_ch];

#define REDUCE_SIGNAL_TO_16_
#ifdef REDUCE_SIGNAL_TO_16
      for(i = 0; i < usacData->block_size_samples; i++){
        /* Additional noise for demonstration */
#define AAA (0.5)
        usacData->time_sample_vector[i_ch][i]+=(AAA/RAND_MAX)*(float)rand()-AAA/2;
        usacData->overlap_buffer[i_ch][i]+=    (AAA/RAND_MAX)*(float)rand()-AAA/2;
      }
#endif


#ifdef DEBUGPLOT
      if (i_ch==0){
        plotSend("L", "timeOut",  MTV_DOUBLE,usacData->block_size_samples,  usacData->time_sample_vector[i_ch] , NULL);
      } else {
        plotSend("R", "timeOutR",  MTV_DOUBLE,usacData->block_size_samples,  usacData->time_sample_vector[i_ch] , NULL);
      }
#endif

    }
  }
  for (i_ch=left; i_ch<=right; i_ch++){
    usacData->prev_coreMode[i_ch] = usacData->coreMode[i_ch];
    usacData->prev_coreMode[i_ch] = usacData->coreMode[i_ch];
  }

  return 0;
}


void usac_get_ics_info ( WINDOW_SEQUENCE*          win,
                         WINDOW_SHAPE*             wshape,
                         byte*                     group,
                         byte*                     max_sfb,
                         QC_MOD_SELECT             qc_select,
                         HANDLE_RESILIENCE         hResilience,
                         HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                         HANDLE_BUFFER             hVm)

{
  Info *info;
  int tmp;

  tmp = (WINDOW_SEQUENCE)GetBits ( LEN_WIN_SEQ,
                                   WINDOW_SEQ,
                                   hResilience,
                                   hEscInstanceData,
                                   hVm );
  if (bno&&debug['V']) {
    if ((((tmp==ONLY_LONG_SEQUENCE)||(tmp==LONG_START_SEQUENCE)) &&((*win==EIGHT_SHORT_SEQUENCE)||(*win==LONG_START_SEQUENCE)))||
        (((tmp==LONG_STOP_SEQUENCE)||(tmp==EIGHT_SHORT_SEQUENCE))&&((*win==ONLY_LONG_SEQUENCE)  ||(*win==LONG_STOP_SEQUENCE))))
      fprintf(stderr,"Non meaningful window sequence transition %i %i %i\n",tmp, *win, LONG_STOP_SEQUENCE);
  }
  *win = (WINDOW_SEQUENCE)tmp;

  tmp = GetBits ( LEN_WIN_SH,
                  WINDOW_SHAPE_CODE,
                  hResilience,
                  hEscInstanceData,
                  hVm );
  if (bno&debug['V']) {
    if (tmp-*wshape) fprintf(stderr,"# WndShapeSW detected\n");
  }
  *wshape = (WINDOW_SHAPE)tmp;
  if ( ( info = usac_winmap[*win] ) == NULL ) {
    CommonExit(1,"bad window code");
  }
  /*
   * max scale factor, scale factor grouping and prediction flags
   */
  if (info->islong) {
    *max_sfb = (unsigned char) GetBits(LEN_MAX_SFBL,
                                        MAX_SFB,
                                        hResilience,
                                        hEscInstanceData,
                                        hVm);
    group[0] = 1;


  }
  else {  /* EIGHT_SHORT_SEQUENCE */
    *max_sfb = (unsigned char) GetBits(LEN_MAX_SFBS,
                                       MAX_SFB,
                                       hResilience,
                                       hEscInstanceData,
                                       hVm);
    getgroup ( info,
               group,
               hResilience,
               hEscInstanceData,
               hVm );
  }

  if ( *max_sfb > info->sfb_per_sbk[0] ) {
    if ( ! ( hResilience && GetEPFlag ( hResilience ) ) ) {
      CommonExit ( 2, "get_ics_info: max_sfb (%2d) > sfb_per_sbk (%2d) (decode_chan_ele.c)", *max_sfb, info->sfb_per_sbk[0]);
    }
    else {
      if ( !hEscInstanceData || BsGetEpDebugLevel ( hEscInstanceData ) >= 2 ) {
        printf ( "get_ics_info: max_sfb (%2d) > sfb_per_sbk (%2d)\n", *max_sfb, info->sfb_per_sbk[0]);
        printf ( "get_ics_info: --> max_sfb is set to %2d\n", info->sfb_per_sbk[0]);
      }
      *max_sfb = info->sfb_per_sbk[0];
    }
  }

  if(debug['v']) {
    fprintf(stderr,"win %d, wsh %d\n", *win, *wshape);
    fprintf(stderr,"max_sf %d\n", *max_sfb);
  }


}
