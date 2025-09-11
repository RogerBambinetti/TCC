/***********************************************************************************

This software module was originally developed by

VoiceAge Corp.

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

VoiceAge Corp. retains full right to modify and use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using
the code for products that do not conform to MPEG-related ITU Recommendations and/or
ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "proto_func.h"
#include "acelp_plus.h"
#include "buffers.h"

#define L_EXTRA  96

#define BUGFIX_BPF

extern const float lsf_init[16];

/* local functions */
static void bass_pf_1sf_delay(
  float *syn,       /* (i) : 12.8kHz synthesis to postfilter             */
  float *synFB,
  int fac_FB,
  int *T_sf,        /* (i) : Pitch period for all subframes (T_sf[16])   */
  float *gainT_sf,  /* (i) : Pitch gain for all subframes (gainT_sf[16]) */
  float *synth_out, /* (o) : filtered synthesis (with delay of 1 subfr)  */
  int l_frame,      /* (i) : frame length (should be multiple of l_subfr)*/
  int l_subfr,      /* (i) : sub-frame length (60/64)                    */
  int l_next,       /* (i) : look ahead for symetric filtering           */
  float mem_bpf[]); /* i/o : memory state [L_FILT+L_SUBFR]               */

void map_decoder_lf_config(HANDLE_USAC_TD_DECODER st, HANDLE_USAC_TD_DECODER_CONFIG config)
{
  st->fullbandLPD       = config->fullbandLPD;
  st->lpdStereoIndex    = config->lpdStereoIndex;
  st->bUseNoiseFilling  = config->bUseNoiseFilling;
  st->igf_active        = config->igf_active;
  st->lFrame            = config->lFrame;
  st->lFrameFB          = config->lFrameFB;
  st->lDiv              = config->lDiv;
  st->nbDiv             = config->nbDiv;
  st->nbSubfr           = config->nbSubfr;
  st->fscale            = config->fscale;
  st->fscaleFB          = config->fscaleFB;
  st->fac_FB            = config->fac_FB;
}

void init_decoder_lf(HANDLE_USAC_TD_DECODER st)
{
    TCX_MDCT_Open(&st->hTcxMdct);
    st->fdSynth=&(st->fdSynth_Buf[L_DIV_1024]);
    reset_decoder_lf(st, NULL, NULL, 0, 0);
}

void reset_decoder_lf(HANDLE_USAC_TD_DECODER st, float* pOlaBuffer, float* pOlaBufferFB, int lastWasShort, int twMdct)
{
  int   i;
  int   lfac;

  /* Static vectors to zero */
  if (lastWasShort == 1) {
    st->last_mode = -2;
    lfac = (st->lFrame)/16;
  } else {
    st->last_mode = -1;      /* -1 indicate AAC frame (0=ACELP, 1,2,3=TCX) */
    lfac = (st->lFrame)/8;
  }

  set_zero(st->old_Aq, 2*(M+1));
  set_zero(st->old_xnq, 1+(2*LFAC_1024));
  set_zero(st->old_xnqFB, (4*LFAC_1024));

  if (pOlaBuffer != NULL && (st->fullbandLPD == 1)) {
    mvr2r(&pOlaBuffer[2 * st->lDiv - lfac - 1], &st->old_xnq[(st->lDiv) / 2 - lfac], 2 * lfac + 1);
  }

  if (pOlaBufferFB && (st->fullbandLPD == 1)) {
    mvr2r(&pOlaBufferFB[st->lFrameFB / 2 - 2 * lfac], &st->old_xnqFB[(2 * st->lDiv) / 2 - 2 * lfac], 2 * 2 * lfac);
  }

  set_zero(st->old_exc, PIT_MAX_MAX+L_INTERPOL);   /* excitation */
  set_zero(st->old_synth, PIT_MAX_MAX+SYN_DELAY_1024);

  /* bass pf reset */
  set_zero(st->old_noise_pf, 2*L_FILT+1+2*L_SUBFR);
  for (i=0; i<SYN_SFD_1024; i++)
  {
    st->old_T_pf[i] = 64;
    st->old_gain_pf[i] = 0.0f;
  }

  set_zero(st->past_lsfq, M);               /* past isf quantizer */
  
  /* Initialize the ISFs */
  mvr2r(lsf_init, st->lsfold, M);

  for (i = 0; i < L_MEANBUF; i++) 
  {
    mvr2r(lsf_init, &(st->lsf_buf[i*M]), M);
  }
  /* Initialize the ISPs */
  for (i=0; i<M; i++) 
  {
    st->lspold[i] = (float)cos(3.141592654*(float)(i+1)/(float)(M+1));
  }

  st->old_T0 = 64;
  st->old_T0_frac = 0;
  st->seed_ace = 0;
#ifndef UNIFIED_RANDOMSIGN
  st->seed_tcx = 21845;
#endif

  st->past_gpit = 0.0;
  st->past_gcode = 0.0;
  st->gc_threshold = 0.0f;
  st->wsyn_rms = 0.0f;
  st->prev_BassPostFilter_activ=0;

  if ( pOlaBuffer != NULL && !twMdct && !st->fullbandLPD){
    const float *pWinCoeff;

    if  (lfac==48) {
      pWinCoeff = sineWindow96;
    }
    else if  (lfac==64) {
      pWinCoeff = sineWindow128;
    }
    else if (lfac==96) {
      pWinCoeff = sineWindow192;
    }
    else {
      pWinCoeff = sineWindow256;
    }

    for (i = 0; i < 2*lfac; i++){
      pOlaBuffer[(st->lFrame)/2-lfac+i] *= pWinCoeff[2*lfac-1-i];
    }
    for ( i = 0 ; i < (st->lFrame)/2-lfac; i++ ) {
      pOlaBuffer[(st->lFrame)/2+lfac+i] = 0.0f;
    }
  }

  st->pOlaBuffer = pOlaBuffer;

  /*set old_xnq for calculation of the excitation buffer*/
  if (pOlaBuffer != NULL && !st->fullbandLPD) {
    for (i = 0; i < 2 * lfac + 1; i++) {
      st->old_xnq[(st->lDiv) / 2 - lfac + i] = pOlaBuffer[i + st->lFrame / 2 - lfac - 1];
    }
  }

  st->pOlaBufferFB = pOlaBufferFB;

  if (st->fullbandLPD == 1) {
    set_zero(st->TD_resampler_mem, 2*L_FILT_MAX);
    st->TD_resampler_delay = (st->fscale < st->fscaleFB) ? L_FILT_MAX : L_FILT_MAX/2;
    set_zero(st->old_synthFB, 2*(PIT_MAX_MAX+SYN_DELAY_1024));
    TBE_init(&st->tbeDecData);
  }

  st->max_sfb = 0;
  st->window_shape = 0;
  st->tns_data_present = 0;
  memset(&st->tns, 0, sizeof(st->tns));

  st->fdp_data_present = 0;
  st->fdp_spacing_index = 0;
  memset(st->fdp_int, 0, sizeof(st->fdp_int));
  memset(st->fdp_quantSpecPrev, 0, sizeof(st->fdp_quantSpecPrev));
  memset(st->igfDecData->igf_infSpec, 0, sizeof(st->igfDecData->igf_infSpec));

  return;
}

void close_decoder_lf(HANDLE_USAC_TD_DECODER st){
  if(st){
    if(st->hTcxMdct){
      TCX_MDCT_Close(&st->hTcxMdct);
    }
    st->hTcxMdct = NULL;
  }
}

void decoder_LPD(
  HANDLE_USAC_TD_DECODER      st,
  td_frame_data              *td,
  int const                   bUsacIndependencyFlag,
  byte                       *group,
  HIGF_CONFIG                 igfConfig,
  HSTEREOLPD_DEC_DATA         stereolpdDecData,
  int                         pit_adj,
  float                       fsynth[],
  int                         bad_frame[],
  int                         isAceStart,
  int                         short_fac_flag,
  int                         isBassPostFilter
) {
  float synth_buf[PIT_MAX_MAX+SYN_DELAY_1024+L_FRAME_1024];
  float exc_buf[PIT_MAX_MAX+L_INTERPOL+L_FRAME_1024+1]; 
  float lsf[(NB_DIV+1)*M];
  float lspnew[M];
  float lsfnew[M];
  float Aq[(NB_SUBFR_SUPERFR_1024+1)*(M+1)]; /* A(z) for all subframes     */
  float *synth, *exc;
  int   pitch[NB_SUBFR_SUPERFR_1024+SYN_SFD_1024];
  float pit_gain[NB_SUBFR_SUPERFR_1024+SYN_SFD_1024];
  int   lFAC_FB;
  float synth_bufFB[2*(LPD_SFD_1024*L_SUBFR+L_FRAME_1024)] = {0};
  float *synthFB;
  float synthRS[8*L_DIV_1024];
  int   rs_delay;

  /* Scalars */
  int i, k, T, mode;
  int *mod;
  float tmp, gain, stab_fac;

  /* Variables dependent on the frame length */
  int lFrame;
  int lDiv;
  int nbSubfr;
  int nbSubfrSuperfr;
  int SynSfd;
  int SynDelay;

  TBE_FRAME_CLASS tbe_frm_class;
  float tbe_synth[2*L_FRAME_1024];
  float bwe_exc_extended[2*L_DIV_1024+NL_BUFF_OFFSET_MAX];
  float pitch_buf[NB_SUBFR_1024];
  float voice_factors[NB_SUBFR_1024];
  int   tbe_delay = 0;
  int   tbe_delay_internal;
  float tbe_synth_align[2*L_FRAME_1024];
  float tmp_tbe[2*L_FRAME_1024];
  int   first_tbe_frame;

  /* Set variables dependent on frame length */
  lFrame   = st->lFrame;
  lDiv     = st->lDiv;
  nbSubfr  = st->nbSubfr;
  nbSubfrSuperfr = st->nbDiv*nbSubfr;
  SynSfd   = (nbSubfrSuperfr/2)-BPF_SFD;
  SynDelay = SynSfd*L_SUBFR;
  set_zero(synth_buf, PIT_MAX_MAX+SYN_DELAY_1024+L_FRAME_1024);
  set_zero(exc_buf, PIT_MAX_MAX+L_INTERPOL+L_FRAME_1024+1);

  /* Initialize pointers */
  synth = synth_buf+PIT_MAX_MAX+SynDelay;
  mvr2r(st->old_synth, synth_buf, PIT_MAX_MAX+SynDelay);
  exc = exc_buf+PIT_MAX_MAX+L_INTERPOL;

  if (st->fullbandLPD) {
    tbe_delay_internal = TBE_DEC_DELAY;
    tbe_delay = 2*(lDiv/2 - tbe_delay_internal);
    set_zero(tbe_synth, 2*lFrame);
  }

  if (st->fullbandLPD) {
    synthFB = synth_bufFB+(PIT_MAX_MAX+SynDelay)*st->fac_FB;
    mvr2r(st->old_synthFB, synth_bufFB, (PIT_MAX_MAX+SynDelay)*st->fac_FB);
  }

  mvr2r(st->old_exc, exc_buf, PIT_MAX_MAX+L_INTERPOL);

  mod       = td->mod;

  /* for bass postfilter */
  for (i=0; i<SynSfd; i++) {
    pitch[i] = st->old_T_pf[i];
    pit_gain[i] = st->old_gain_pf[i];
  }

  for (i=0; i<nbSubfrSuperfr; i++) {
    pitch[i+SynSfd] = L_SUBFR;
    pit_gain[i+SynSfd] = 0.0f;
  }
  /* Decode LPC parameters */

  if (!isAceStart) {
     E_LPC_lsp_lsf_conversion(st->lspold, lsf, M);
  }

  dlpc_avq(td, isAceStart, &lsf[M], st->past_lsfq, td->mod, bad_frame[0], st->nbDiv);

  if (isAceStart) {
     mvr2r(&lsf[0], st->lsfold, M);
     E_LPC_lsf_lsp_conversion(st->lsfold, st->lspold, M);
  }

  if ( isAceStart && (mod[0] == 0 || mod[1] == 0 || (!st->fullbandLPD && mod[2] == 0 && lDiv != L_DIV_1024)) ) {
    /* read and decode FD-FAC data */
/*    int facBits = 0;*/
    float mem=0.0f;
    float ANull[9*(M+1)];
    float tmp_buf[3*L_DIV_1024+M];
    float tmpRes_buf[3*L_DIV_1024];
    float *tmp=&(tmp_buf[L_DIV_1024]);
    float *tmpRes=&(tmpRes_buf[L_DIV_1024]);
    int tmp_start;
    int   nSamp = 0;
    float facTimeSig[2*L_DIV_1024];
    float facTimeSigFB[2*L_DIV_1024];

    int_lpc_acelp(st->lspold, st->lspold, ANull, 8, M);

    /*fill old_Aq for later generating an excitation signal on an overlapped signal between FD and TCX*/
    memcpy(st->old_Aq,ANull,(M+1)*sizeof(float));
    memcpy(st->old_Aq+M+1,ANull,(M+1)*sizeof(float));

    if (mod[0] == 0) {
      int lfac;
      int lfacFB;

      if ( short_fac_flag ) {
        lfac = (st->lFrame)/16;
      }
      else {
        lfac = (st->lFrame)/8;
      }

      lfacFB = lfac;
      if (st->fullbandLPD) {
        lfacFB=2*lfac;
      }

      decode_fdfac(td->fdFac, lDiv, lfacFB, ANull, NULL, facTimeSig, st->fullbandLPD, 0, st->fscale, st->fscaleFB, facTimeSigFB);

      addr2r(facTimeSig,st->pOlaBuffer+2*lDiv-lfac,st->pOlaBuffer+2*lDiv-lfac,lfac);

      if (st->fullbandLPD) {
        if (st->pOlaBufferFB!=NULL) {
          addr2r(facTimeSigFB,st->pOlaBufferFB+2*lDiv-lfacFB,st->pOlaBufferFB+2*lDiv-lfacFB,lfacFB);
          set_zero(st->pOlaBufferFB+2*lDiv,lfacFB);
          mvr2r(st->pOlaBufferFB,synthFB-2*lDiv,2*lDiv);
        }
        else  {
          mvr2r(facTimeSigFB,synthFB-lfacFB,lfacFB);
        }
      }

      set_zero(st->pOlaBuffer+2*lDiv,lfac);
    }

    if (mod[0] == 0 && lDiv!=L_DIV_1024) {
      mvr2r(st->pOlaBuffer-lDiv, st->fdSynth-lDiv, 3*lDiv);
      nSamp = min(3*lDiv,PIT_MAX_MAX+SynDelay);
    } else {
      mvr2r(st->pOlaBuffer,st->fdSynth,2*lDiv);
      nSamp = min(2*lDiv,PIT_MAX_MAX+SynDelay);
    }

    mvr2r(st->fdSynth,synth-2*lDiv,2*lDiv);

    if (mod[0] == 0 && lDiv!=L_DIV_1024) {
      E_UTIL_f_preemph(st->fdSynth-lDiv,PREEMPH_FAC,3*lDiv,&mem);
    } else {
      E_UTIL_f_preemph(st->fdSynth,PREEMPH_FAC,2*lDiv,&mem);
    }

    if (mod[0] == 0 && lDiv!=L_DIV_1024) {
      set_zero(tmp-lDiv, M);
      mvr2r(st->fdSynth-lDiv,tmp-lDiv+M,3*lDiv);
      tmp_start=-lDiv;
    } else {
      set_zero(tmp, M);
      mvr2r(st->fdSynth,tmp+M,2*lDiv);
      tmp_start=0;
    }
    set_zero(tmpRes-lDiv, 3*lDiv);
    for ( i = tmp_start ; i < 2*lDiv ; i+= L_SUBFR ) {
      E_UTIL_residu(ANull,&tmp[M+i],&tmpRes[i],L_SUBFR);
    }

    if (mod[0] != 0 && (lDiv==L_DIV_1024 || mod[1]!=0) ) {
      nSamp = min(lDiv,PIT_MAX_MAX+L_INTERPOL);
    } else if (mod[0] == 0 && lDiv!=L_DIV_1024) {
      nSamp = min(3*lDiv,PIT_MAX_MAX+L_INTERPOL);
    } else {
      nSamp = min(2*lDiv,PIT_MAX_MAX+L_INTERPOL);
    }
    mvr2r(tmpRes+2*lDiv-nSamp,exc-nSamp,nSamp);
  }

  if (isAceStart && mod[0] > 0 && st->fullbandLPD && st->pOlaBufferFB != NULL) {
    mvr2r(st->pOlaBufferFB,synthFB-2*lDiv,2*lDiv);
  }

  k = 0;
  while (k < st->nbDiv)
  {
    mode = mod[k];
    if ((st->last_mode == 0) && (mode > 0) && (k!=0 || st->prev_BassPostFilter_activ==1)){
      i = (k*nbSubfr)+SynSfd;
      pitch[i+1] = pitch[i] = pitch[i-1];
      pit_gain[i+1] = pit_gain[i] = pit_gain[i-1];
    }

    /* decode LSFs and convert LSFs to cosine domain */
    if ((mode==0) || (mode==1))
       mvr2r(&lsf[(k+1)*M], lsfnew, M);
    else if (mode==2)
       mvr2r(&lsf[(k+2)*M], lsfnew, M);
    else /*if (mode==3)*/
       mvr2r(&lsf[(k+4)*M], lsfnew, M);

    E_LPC_lsf_lsp_conversion(lsfnew, lspnew, M);

    /* Check stability on lsf : distance between old lsf and current lsf */
    tmp = 0.0f;
    for (i=0; i<M; i++) 
    {
      tmp += (lsfnew[i]-st->lsfold[i])*(lsfnew[i]-st->lsfold[i]);
    }
    stab_fac = (float)(1.25f - (tmp/400000.0f));
    if (stab_fac > 1.0f) {
      stab_fac = 1.0f;
    }
    if (stab_fac < 0.0f) {
      stab_fac = 0.0f;
    }

    /* - interpolate Ai in ISP domain (Aq) and save values for upper-band (Aq_lpc)
       - decode other parameters according to mode
       - set overlap size for next decoded frame (ovlp_size)
       - set mode for upper-band decoder (mod[]) */
    switch (mode) {
    case 0:
    case 1:
      if (mode == 0) {
        /* ACELP frame */
        int_lpc_acelp(st->lspold, lspnew, Aq, nbSubfr, M);

        decoder_acelp_fac(st,
                          td,
                          k,
                          Aq,
                          td->core_mode_index,
                          bad_frame[k],
                          &exc[k*lDiv],
                          &synth[k*lDiv],
                          &synthFB[k*lDiv*st->fscaleFB/st->fscale],
                          &pitch[(k*nbSubfr)+SynSfd],
                          &pit_gain[(k*nbSubfr)+SynSfd],
                          stab_fac,
                          pitch_buf,
                          voice_factors);

        if (st->fullbandLPD) {
          tbe_frm_class = (k == 0) ? TBE_CLASS_ACELP_FIRST : TBE_CLASS_ACELP_SECOND;
          first_tbe_frame = 0;
          if ((k==0 && st->last_mode!=0) || (k==1 && mod[0]!=0)) {
            first_tbe_frame = 1;
          }

          /* TBE main decoder function */
          TBE_apply(&st->tbeDecData,
                    tbe_frm_class,
                    bwe_exc_extended,
                    voice_factors,
                    &tbe_synth[2*k*st->lDiv],
                    pitch_buf,
                    first_tbe_frame,
                    (Aq+2*(M+1)),
                    L_DIV_1024);

          /* delay alignment */
          mvr2r(&tbe_synth[2*k*st->lDiv], tmp_tbe, 2*lDiv);
          mvr2r(st->tbeDecData.tbe_synth_buffer, tbe_synth_align, tbe_delay);
          mvr2r(tmp_tbe, tbe_synth_align+tbe_delay, 2*lDiv-tbe_delay);
          mvr2r(tmp_tbe + (2*lDiv-tbe_delay), st->tbeDecData.tbe_synth_buffer, tbe_delay);
        }

        if ((st->last_mode != 0)
#ifdef BUGFIX_BPF
             && isBassPostFilter
#endif
             ) /* bass-postfilter also FD-ACELP FAC area */
        {
          i = (k*nbSubfr)+SynSfd;
          pitch[i-1] = pitch[i];
          pit_gain[i-1] = pit_gain[i];
#ifdef BUGFIX_BPF
          if(st->last_mode != -2){
#endif
            pitch[i-2]    = pitch[i];
            pit_gain[i-2] = pit_gain[i];
#ifdef BUGFIX_BPF
          }
#endif
        }

        if (st->fullbandLPD) {
          if (st->last_mode!=0) {
            mvr2r(synth+k*lDiv-lDiv/2-st->TD_resampler_delay/2,st->TD_resampler_mem,st->TD_resampler_delay);
          }
          
          TD_resampler(synth+k*lDiv-lDiv/2+st->TD_resampler_delay/2, lDiv, st->fscale, synthRS,
                       st->fscaleFB, st->TD_resampler_mem, 0);

          /* add TBE signal to to resampled ACELP signal */
          addr2r(synthRS, tbe_synth_align, synthRS, st->fac_FB*lDiv);

          /*TCX->ACELP*/
          if (st->last_mode!=0) {

            rs_delay=st->TD_resampler_delay;

            /* crossfade from FB-TCX (incl. FAC) to resampled LB synth */
            for(i=0;i<rs_delay;i++) {
              synthFB[k*lDiv*st->fac_FB-rs_delay+i] *= (rs_delay-i) / ((float)rs_delay);
              synthFB[k*lDiv*st->fac_FB-rs_delay+i] += (synthRS[lDiv+i-rs_delay] * i) / ((float)rs_delay);
            }

            /* copy the rest of resampled LB synth to FB synth */
            mvr2r(synthRS+lDiv, synthFB+(k*lDiv*st->fac_FB), lDiv);

          }
          /*ACELP->ACELP*/
          else {
            /* copy from resampled LB synth to FB synth */
             mvr2r(synthRS, synthFB+(k*lDiv*st->fac_FB)-lDiv, lDiv*st->fac_FB);
          }

          /* reset previous TCX window shape if current frame is ACELP */
          st->window_shape_prev = WS_FHG;
        }
      } else {
        /* short TCX frame */

        if (st->fullbandLPD && st->last_mode == 0) {
          TBE_genTransition(&st->tbeDecData,
                            (lDiv+st->TD_resampler_delay)-tbe_delay,
                            &tbe_synth[2*k*lDiv]);

          /* delay alignment */
          mvr2r(&tbe_synth[2*k*lDiv], tmp_tbe, 2*lDiv);
          mvr2r(st->tbeDecData.tbe_synth_buffer, tbe_synth_align, tbe_delay);
          mvr2r(tmp_tbe, tbe_synth_align+tbe_delay, 2*lDiv-tbe_delay);
          mvr2r(tmp_tbe + (2*lDiv-tbe_delay), st->tbeDecData.tbe_synth_buffer, tbe_delay);

          TBE_reset(&st->tbeDecData);
        }

        int_lpc_tcx(st->lspold, lspnew, Aq, nbSubfr, M);
        decoder_tcx_fac(td, k, Aq, lDiv, &exc[k*lDiv], &synth[k*lDiv], &synthFB[k*lDiv*st->fscaleFB/st->fscale], igfConfig, group, bUsacIndependencyFlag, st);

        if (st->fullbandLPD && st->last_mode == 0) {

          /*ACELP -> TCX*/
          TD_resampler(synth+k*lDiv-lDiv/2+st->TD_resampler_delay/2, lDiv/2+st->TD_resampler_delay/2,
                       st->fscale, synthRS, st->fscaleFB, st->TD_resampler_mem, 0);

          lFAC_FB = (lDiv/2)*st->fac_FB;
          rs_delay=st->TD_resampler_delay;

          /* add TBE transition signal */
          addr2r(synthRS, tbe_synth_align, synthRS, (lDiv+st->TD_resampler_delay));

          /* copy the beginning of resampled LB synth to FB synth */
          mvr2r(synthRS, synthFB+(k*lDiv*st->fac_FB)-lDiv, lFAC_FB);

          /* crossfade from resampled LB synth to FB-TCX (incl. FAC) */
          for(i=0;i<rs_delay;i++){
              synthFB[k*lDiv*st->fac_FB-lDiv+lFAC_FB+i] *= (i) / ((float)rs_delay);
              synthFB[k*lDiv*st->fac_FB-lDiv+lFAC_FB+i] += (synthRS[lFAC_FB+i] * (rs_delay-i)) / ((float)rs_delay);
          }
        }
      }
      k++;
      break;

    case 2:
      /* medium TCX frame */

      if (st->fullbandLPD && st->last_mode == 0)  {
        TBE_genTransition(&st->tbeDecData,
                          (lDiv+st->TD_resampler_delay)-tbe_delay,
                          &tbe_synth[2*k*lDiv]);

        /* delay alignment */
        copyFLOAT(&tbe_synth[2*k*lDiv], tmp_tbe, 2*lDiv);
        copyFLOAT(st->tbeDecData.tbe_synth_buffer, tbe_synth_align, tbe_delay);
        copyFLOAT(tmp_tbe, tbe_synth_align+tbe_delay, 2*lDiv-tbe_delay);
        copyFLOAT(tmp_tbe + (2*lDiv-tbe_delay), st->tbeDecData.tbe_synth_buffer, tbe_delay);

        TBE_reset(&st->tbeDecData);
      }

      int_lpc_tcx(st->lspold, lspnew, Aq, (nbSubfrSuperfr/2), M);
      decoder_tcx_fac(td, k, Aq, 2*lDiv, &exc[k*lDiv], &synth[k*lDiv], &synthFB[k*lDiv*st->fscaleFB/st->fscale], igfConfig, group, bUsacIndependencyFlag, st);

      if (st->fullbandLPD && st->last_mode == 0)  {

        /*ACELP -> TCX*/
        TD_resampler(synth+k*lDiv-lDiv/2+st->TD_resampler_delay/2, lDiv/2+st->TD_resampler_delay/2,
                     st->fscale, synthRS, st->fscaleFB, st->TD_resampler_mem, 0);

        lFAC_FB = (lDiv/2)*st->fac_FB;
        rs_delay=st->TD_resampler_delay;

        /* add TBE transition signal */
        addr2r(synthRS, tbe_synth_align, synthRS, (lDiv+st->TD_resampler_delay));

        /* copy the beginning of resampled LB synth to FB synth */
        mvr2r(synthRS, synthFB+(k*lDiv*st->fac_FB)-lDiv, lFAC_FB);

        /* crossfade from resampled LB synth to FB-TCX (incl. FAC) */
        for(i=0;i<rs_delay;i++){
          synthFB[k*lDiv*st->fac_FB-lDiv+lFAC_FB+i] *= (i) / ((float)rs_delay);
          synthFB[k*lDiv*st->fac_FB-lDiv+lFAC_FB+i] += (synthRS[lFAC_FB+i] * (rs_delay-i)) / ((float)rs_delay);  
        }
      }

      k+=2;
      break;
 
   case 3:
      /* long TCX frame */

      if (st->fullbandLPD) {
        printf("decoder error: no mode 3 in fullbandLPD!\n");
      }

      int_lpc_tcx(st->lspold, lspnew, Aq, nbSubfrSuperfr, M);
      decoder_tcx_fac(td, k, Aq, 4*lDiv, &exc[k*lDiv], &synth[k*lDiv], &synthFB[k*lDiv*st->fscaleFB/st->fscale], igfConfig, group, bUsacIndependencyFlag, st);

      k+=4;
      break;

    default:
      printf("decoder error: mode > 3!\n");
      exit(0);
    }

    st->last_mode = mode;

    /* update lspold[] and lsfold[] for the next frame */
    mvr2r(lspnew, st->lspold, M);
    mvr2r(lsfnew, st->lsfold, M);
  }

  /*----- update signal for next superframe -----*/
  mvr2r(exc_buf+lFrame, st->old_exc, PIT_MAX_MAX+L_INTERPOL);

  mvr2r(synth_buf+lFrame, st->old_synth, PIT_MAX_MAX+SynDelay);

  if (st->fullbandLPD)  {
    mvr2r(synth_bufFB+lFrame*st->fac_FB, st->old_synthFB, (PIT_MAX_MAX+SynDelay)*st->fac_FB);
  }

  /* reset FD predictor states */
  if (mode != (st->fullbandLPD ? 2 : 3)) { 
    int lines = st->lDiv == 192 ? 120 : 160;
    for (i = 0; i < lines; i++) {
      st->fdp_quantSpecPrev[0][i] = st->fdp_quantSpecPrev[1][i] = 0;
    }
  }

  /* check whether usage of bass postfilter was de-activated in the bitstream;
     if yes, set pitch gain to 0 */
  if (!isBassPostFilter) {
    if(mod[0]!=0 && st->prev_BassPostFilter_activ) {
      for (i=2; i<nbSubfrSuperfr; i++)
        pit_gain[SynSfd+i] = 0.0;
    } else {
      for (i=0; i<nbSubfrSuperfr; i++) {
        pit_gain[SynSfd+i] = 0.0;
      }
    }
  }

  /* for bass postfilter */
  for (i=0; i<SynSfd; i++){
    st->old_T_pf[i] = pitch[nbSubfrSuperfr+i];
    st->old_gain_pf[i] = pit_gain[nbSubfrSuperfr+i];
  }
  st->prev_BassPostFilter_activ=isBassPostFilter;

  /* bass post filter */
  if (st->lpdStereoIndex == 1) {
    
    if (st->fullbandLPD == 1) {
      
      stereoLpd_setDMX(stereolpdDecData,
                       synth_bufFB + (PIT_MAX_MAX-BPF_SFD * L_SUBFR) * st->fac_FB);
      
      mvr2r(synth_bufFB+(PIT_MAX_MAX-BPF_SFD*L_SUBFR)*st->fac_FB, fsynth, lFrame*st->fac_FB);
    } else {
      
      stereoLpd_setDMX(stereolpdDecData,
                       synth_buf + (PIT_MAX_MAX-BPF_SFD * L_SUBFR));
      
      mvr2r(synth_buf+(PIT_MAX_MAX-BPF_SFD*L_SUBFR), fsynth, st->lFrame);
    }

    stereoLpd_setBPFprm(stereolpdDecData,
                        pitch,
                        pit_gain,
                        nbSubfrSuperfr);

  } else {
    synth = synth_buf+PIT_MAX_MAX;

    if (st->fullbandLPD) {
      synthFB = synth_bufFB+PIT_MAX_MAX*st->fac_FB;
    }

    /* recalculate pitch gain to allow postfiltering on FAC area */
    for (i = 0; i < nbSubfrSuperfr; i++){
      T = pitch[i];
      gain = pit_gain[i];
      if (gain > 0.0f){
        gain = get_gain(&synth[i*L_SUBFR], &synth[(i*L_SUBFR)-T], L_SUBFR);
        pit_gain[i] = gain;
      }
    }

    /* If the last frame was ACELP, the signal is correct computed up to the end of the superframe, 
     * only if the last frame is TCX, you shouldn't use the overlapp/FAC area */
    if (mode == 0 && st->fullbandLPD == 0) {
      bass_pf_1sf_delay(synth, synthFB, st->fac_FB, pitch, pit_gain, fsynth,
                        lFrame, L_SUBFR, SynDelay, st->old_noise_pf);
    } else {
      bass_pf_1sf_delay(synth, synthFB, st->fac_FB, pitch, pit_gain, fsynth,
                        lFrame, L_SUBFR, SynDelay-(lDiv/2), st->old_noise_pf);
    }
  }

  return;
}

int  decoder_LPD_end(HANDLE_USAC_TD_DECODER tddec,
                     HANDLE_BUFFER     hVm,
                     USAC_DATA *usac_data,
                     int i_ch)
{
  int err = 0;
  int i;
  int tbe_delay, tbe_delay_internal;
  float tmp_tbe[L_DIV_1024*2];
  float dsynth[L_FRAME_1024];
  
  /* Decode the LPD data */
  decoder_Synth_end( dsynth, tddec);
  
  if ( usac_data->twMdct[0] == 1 ){    
    for (i = 0; i < usac_data->block_size_samples; i++){
      usac_data->overlap_buffer[i_ch][i + (usac_data->block_size_samples/2)] = dsynth[i];
    }
  } else {
    for (i = 0; i <  usac_data->block_size_samples; i++){
      usac_data->overlap_buffer[i_ch][i] = dsynth[i];
    }
  }

  usac_data->prev_windowShape[i_ch]   = (tddec->last_mode == 0) ? WS_FHG : tddec->window_shape_prev;
  usac_data->windowSequenceLast[i_ch] = EIGHT_SHORT_SEQUENCE;
  usac_data->FrameWasTD[i_ch]         = 1;

  /* get last LPC and ACELP ZIR */
  if ( tddec->last_mode == 0 ) {
    float *lastLPC = getLastLpc(tddec);
    float *acelpZIR = getAcelpZir(tddec);
    copyFLOAT(lastLPC, usac_data->lastLPC[i_ch], M+1);
    copyFLOAT(acelpZIR, usac_data->acelpZIR[i_ch], 1+(2*LFAC_1024));

    if (tddec->fullbandLPD) {
      /* get internal TBE decoder delay */
      tbe_delay_internal = TBE_DEC_DELAY;
      tbe_delay = 2*(tddec->lDiv/2 - tbe_delay_internal);

      /* get TBE transition signal */
      TBE_genTransition(&tddec->tbeDecData,
                        (tddec->lDiv+tddec->TD_resampler_delay)-tbe_delay,
                        tmp_tbe);

      /* delay alignment */
      copyFLOAT(tddec->tbeDecData.tbe_synth_buffer, usac_data->tbe_synth[i_ch], tbe_delay);
      copyFLOAT(tmp_tbe, usac_data->tbe_synth[i_ch]+tbe_delay, 2*tddec->lDiv-tbe_delay);
      copyFLOAT(tmp_tbe + (2*tddec->lDiv-tbe_delay), tddec->tbeDecData.tbe_synth_buffer, tbe_delay);

      /* reset TBE */
      TBE_reset(&tddec->tbeDecData);
    }
  }

  return err;
}

void decoder_Synth_end(
  float signal_out[],/* output: signal with LPD delay (7 subfrs) */
  HANDLE_USAC_TD_DECODER st)       /* i/o:    decoder memory state pointer     */
{
  int i;
  float synth_buf[2*(PIT_MAX_MAX+SYN_DELAY_1024)+L_FRAME_1024];
  float synthRS[2*L_DIV_1024+L_FILT_MAX];
  float *synth;
  int lFrame, LpdSfd, LpdDelay, SynSfd, SynDelay, lFAC;

  if (!st->fullbandLPD)  {
    lFrame   = st->lFrame;
    LpdSfd   = (st->nbDiv*st->nbSubfr)/2;
    LpdDelay = LpdSfd*L_SUBFR;
    SynSfd   = LpdSfd-BPF_SFD;
    SynDelay = SynSfd*L_SUBFR;
    lFAC     = (st->lDiv)/2;

    set_zero(synth_buf,PIT_MAX_MAX+SynDelay+lFrame);
    /* Initialize pointers for synthesis */
    synth = synth_buf+PIT_MAX_MAX+SynDelay;
    mvr2r(st->old_synth, synth_buf, PIT_MAX_MAX+SynDelay);

    /* copy last unwindowed TXC ovlp part to synth */
    for ( i = 0 ; i < 2*lFAC; i++ ){
      synth[i-lFAC] = st->old_xnq[i+1];
    }
    synth = synth_buf+PIT_MAX_MAX-(BPF_SFD*L_SUBFR);
    mvr2r(synth, signal_out, lFrame);
    set_zero(signal_out + LpdDelay + lFAC, lFrame - LpdDelay - lFAC);
  } else {
    lFrame   = st->lFrameFB;
    LpdSfd   = (st->nbDiv*st->nbSubfr)/2;
    LpdDelay = LpdSfd*L_SUBFR;
    SynSfd   = LpdSfd-BPF_SFD;
    SynDelay = SynSfd*L_SUBFR;
    lFAC     = (st->lDiv)/2;

    set_zero(synth_buf,(PIT_MAX_MAX+SynDelay)*st->fac_FB+lFrame);
    
    /* Initialize pointers for synthesis */
    synth = synth_buf+(PIT_MAX_MAX+SynDelay)*st->fac_FB;
    if (st->last_mode == 0)  {   
      
      mvr2r(st->old_synthFB, synth_buf, (PIT_MAX_MAX+SynDelay)*st->fac_FB-st->lDiv-st->TD_resampler_delay);

      /*last resampling section*/
      mvr2r(st->old_synth+(PIT_MAX_MAX+SynDelay)-st->lDiv/2-st->TD_resampler_delay, st->TD_resampler_mem, st->TD_resampler_delay);
      TD_resampler(st->old_xnq+1, lFAC*2, st->fscale, synthRS, st->fscaleFB, st->TD_resampler_mem, 0);

      /* copy last unwindowed ACELP ovlp part to synth and a part of ZIR */
      mvr2r(synthRS, synth-lFAC*2-st->TD_resampler_delay, 4*lFAC);
      set_zero(synth+2*lFAC-st->TD_resampler_delay, st->TD_resampler_delay);
    } else {
      mvr2r(st->old_synthFB, synth_buf, (PIT_MAX_MAX+SynDelay)*st->fac_FB);
      /* copy last unwindowed TXC ovlp part to synth */
      mvr2r(st->old_xnqFB, synth-2*lFAC, 4*lFAC);
    }

    synth = synth_buf+(PIT_MAX_MAX-(BPF_SFD*L_SUBFR))*st->fac_FB;
    mvr2r(synth, signal_out, lFrame);
    set_zero(signal_out + LpdDelay*st->fac_FB + lFAC, lFrame - LpdDelay*st->fac_FB - lFAC);
  }

  return;
}

void decoder_LPD_BPF_end(
  int   isShort,
  float out_buffer[],/* i/o: signal with LPD delay (7 subfrs) */
  HANDLE_USAC_TD_DECODER st)       /* i/o:    decoder memory state pointer     */
{
  int i, T;
  float synth_buf[2*(PIT_MAX_MAX+SYN_DELAY_1024)+L_FRAME_1024];
  float signal_out[L_FRAME_1024];
  float *synth;
  int   pitch[LPD_SFD_1024+3];
  float gain, pit_gain[LPD_SFD_1024+3];
  int lFrame, LpdSfd, LpdDelay, SynSfd, SynDelay, lFAC;

  if (!st->fullbandLPD) {
    lFrame   = st->lFrame;
    LpdSfd   = (st->nbDiv*st->nbSubfr)/2;
    LpdDelay = LpdSfd*L_SUBFR;
    SynSfd   = LpdSfd-BPF_SFD;
    SynDelay = SynSfd*L_SUBFR;
    lFAC     = (st->lDiv)/2;

    set_zero(synth_buf, PIT_MAX_MAX + SynDelay + lFrame);
    /* Initialize pointers for synthesis */
    synth = synth_buf + PIT_MAX_MAX + SynDelay;
    mvr2r(st->old_synth, synth_buf, PIT_MAX_MAX + SynDelay);
    mvr2r(out_buffer, synth_buf + PIT_MAX_MAX - (BPF_SFD*L_SUBFR), SynDelay + lFrame + (BPF_SFD*L_SUBFR));
  } else  {
    lFrame   = st->lFrameFB;
    LpdSfd   = (st->nbDiv*st->nbSubfr)/2;
    LpdDelay = LpdSfd*L_SUBFR;
    SynSfd   = LpdSfd-BPF_SFD;
    SynDelay = SynSfd*L_SUBFR;
    lFAC     = (st->lDiv)/2;

    set_zero(synth_buf, (PIT_MAX_MAX + SynDelay)*st->fac_FB+ lFrame);
    /* Initialize pointers for synthesis */
    synth = synth_buf + (PIT_MAX_MAX + SynDelay)*st->fac_FB;
    mvr2r(st->old_synthFB, synth_buf, (PIT_MAX_MAX + SynDelay)*st->fac_FB);
    mvr2r(out_buffer, synth_buf + (PIT_MAX_MAX - (BPF_SFD*L_SUBFR))*st->fac_FB, lFrame );
  }

  for (i=0; i<SynSfd; i++){
    pitch[i] = st->old_T_pf[i];
    pit_gain[i] = st->old_gain_pf[i];
  }
  for (i=SynSfd; i<LpdSfd + 3; i++){
    pitch[i] = L_SUBFR;
    pit_gain[i] = 0.0f;
  }
  if ( st->last_mode == 0 ) {
    pitch[SynSfd]    = pitch[SynSfd - 1];
    pit_gain[SynSfd] = pit_gain[SynSfd - 1];
    if ( !isShort ) {
      pitch[SynSfd + 1]    = pitch[SynSfd];
      pit_gain[SynSfd + 1] = pit_gain[SynSfd];
    }
  }

  /* bass post filter */
  synth = synth_buf+PIT_MAX_MAX*st->fac_FB;

  /* recalculate pitch gain to allow postfilering on FAC area */
  for (i=0; i<SynSfd + 2; i++){
    T = pitch[i];
    gain = pit_gain[i];

    if (gain > 0.0f) {
      gain = get_gain(&synth[i*L_SUBFR*st->fac_FB], &synth[(i*L_SUBFR*st->fac_FB)-st->fac_FB*T], L_SUBFR*st->fac_FB);
      pit_gain[i] = gain;
    }
  }

  bass_pf_1sf_delay(synth, synth, st->fac_FB, pitch, pit_gain, signal_out,
                    (LpdSfd+2)*L_SUBFR+(BPF_SFD*L_SUBFR), L_SUBFR, lFrame-(LpdSfd+4)*L_SUBFR, st->old_noise_pf);

  mvr2r(signal_out,out_buffer,st->fac_FB*((LpdSfd+2)*L_SUBFR+(BPF_SFD*L_SUBFR)));

  return;
}

static void bass_pf_1sf_delay(
  float *syn,       /* (i) : 12.8kHz synthesis to postfilter             */
  float *synFB,
  int fac_FB,
  int *T_sf,        /* (i) : Pitch period for all subframes (T_sf[16])   */
  float *gainT_sf,  /* (i) : Pitch gain for all subframes (gainT_sf[16]) */
  float *synth_out, /* (o) : filtered synthesis (with delay of 1 subfr)  */
  int l_frame,      /* (i) : frame length (should be multiple of l_subfr)*/
  int l_subfr,      /* (i) : sub-frame length (60/64)                    */
  int l_next,       /* (i) : look ahead for symetric filtering           */
  float mem_bpf[])  /* i/o : memory state [L_FILT+L_SUBFR]               */
{
  int i, j, sf, i_subfr, T, T2, lg;
  float tmp, ener, corr, gain;
  float noise_buf[2*L_FILT+1+(2*2*L_SUBFR)], *noise, *noise_in, *x, *y;

  if ( fac_FB==2 ) {
    noise = noise_buf + 2*L_FILT+1;
    noise_in = noise_buf + 2*L_FILT+1 + 2*l_subfr;
  } 
  else {
    noise = noise_buf + L_FILT;
    noise_in = noise_buf + L_FILT + l_subfr;
  }

  if ( fac_FB==2 ) {
    mvr2r(synFB-2*l_subfr, synth_out, 2*l_frame);
  } 
  else {
    mvr2r(syn-l_subfr, synth_out, l_frame);
  }

  /* Because the BPF does not always operate on multiples of 64 samples */
  if (l_frame%64) {
    if ( fac_FB==2 ) {
      set_zero(synth_out+2*l_frame,2*(L_SUBFR-l_frame%64));
    }
    else  {
      set_zero(synth_out+l_frame,L_SUBFR-l_frame%64);
    }
  }

  sf = 0;
  for (i_subfr=0; i_subfr<l_frame; i_subfr+=l_subfr, sf++){
    T = T_sf[sf];
    gain = gainT_sf[sf];

    if (gain > 1.0f) gain = 1.0f;
    if (gain < 0.0f) gain = 0.0f;

    /* pitch tracker: test pitch/2 to avoid continuous pitch doubling */
    /* Note: pitch is limited to PIT_MIN (34 = 376Hz) at the encoder  */
    T2 = T>>1;
    x = &syn[i_subfr-L_EXTRA];
    y = &syn[i_subfr-T2-L_EXTRA];

    ener = 0.01f;
    corr = 0.01f;
    tmp  = 0.01f;
    for (i=0; i<l_subfr+L_EXTRA; i++){
      ener += x[i]*x[i];
      corr += x[i]*y[i];
      tmp  += y[i]*y[i];
    }
    /* use T2 if normalized correlation > 0.95 */
    tmp = corr / (float)sqrt(ener*tmp);
    if (tmp > 0.95f) T = T2;

    lg = l_frame + l_next - T - i_subfr;
    if (lg < 0) lg = 0;
    if (lg > l_subfr) lg = l_subfr;

    if (gain > 0){
      /* limit gain to avoid problem on burst */
      if (lg > 0){
        tmp = 0.01f;
        for (i=0; i<lg; i++){
          tmp += syn[i+i_subfr] * syn[i+i_subfr];
        }
        ener = 0.01f;
        for (i=0; i<lg; i++){
          ener += syn[i+i_subfr+T] * syn[i+i_subfr+T];
        }
        tmp = (float)sqrt(tmp / ener);
        if (tmp < gain) gain = tmp;
      }

      /* calculate noise based on voiced pitch */
      tmp = gain*0.5f;
      if ( fac_FB==2 ) {
        for (i=0; i<2*lg; i++)  {
          noise_in[i] = tmp * (synFB[i+2*i_subfr] - 0.5f*synFB[i+2*i_subfr-2*T] - 0.5f*synFB[i+2*i_subfr+2*T]);
        }
        for (i=2*lg; i<2*l_subfr; i++)  {
          noise_in[i] = tmp * (synFB[i+2*i_subfr] - synFB[i+2*i_subfr-2*T]);
        }
      } 
      else {
        for (i=0; i<lg; i++){
          noise_in[i] = tmp * (syn[i+i_subfr] - 0.5f*syn[i+i_subfr-T] - 0.5f*syn[i+i_subfr+T]);
        }
        for (i=lg; i<l_subfr; i++){
          noise_in[i] = tmp * (syn[i+i_subfr] - syn[i+i_subfr-T]);
        }
      }
    } else {
      if ( fac_FB==2 ) {
        set_zero(noise_in, 2*l_subfr);
      }
      else  {
        set_zero(noise_in, l_subfr);
      }
    }

    if ( fac_FB==2 ) {
      mvr2r(mem_bpf, noise_buf, 2*L_FILT+1+2*l_subfr);
      mvr2r(noise_buf+2*l_subfr, mem_bpf, 2*L_FILT+1+2*l_subfr);
    }
    else  {
      mvr2r(mem_bpf, noise_buf, L_FILT+l_subfr);
      mvr2r(noise_buf+l_subfr, mem_bpf, L_FILT+l_subfr);
    }

    /* substract from voiced speech low-pass filtered noise */
    if ( fac_FB==2 ) {
      for (i=0; i<2*l_subfr; i++){
        tmp = filt_lp2[0] * noise[i];
        for(j=1; j<=2*L_FILT+1; j++){
          tmp += filt_lp2[j] * (noise[i-j] + noise[i+j]);
        }
        synth_out[i+2*i_subfr] -= tmp;
      }
    }
    else  {
      for (i=0; i<l_subfr; i++){
        tmp = filt_lp[0] * noise[i];
        for(j=1; j<=L_FILT; j++){
          tmp += filt_lp[j] * (noise[i-j] + noise[i+j]);
        }
        synth_out[i+i_subfr] -= tmp;
      }
    }
  }

  /* Filter LTPF_MAX_DELAY samples in the future for LTPF in TCX */
  if ( fac_FB==2 ) {
    for (i=2*l_frame; i<2*l_frame+LTPF_MAX_DELAY; i++)  {
      tmp = filt_lp2[0] * noise[i+2*l_subfr-2*l_frame];
      for(j=1; j<=2*L_FILT+1; j++)  {
        tmp += filt_lp2[j] * (noise[i+2*l_subfr-2*l_frame-j] + noise[i+2*l_subfr-2*l_frame+j]);
      }
      synth_out[i] = synFB[i-2*l_subfr] - tmp;
    }
  } 
  else {
    for (i=l_frame; i<l_frame+LTPF_MAX_DELAY; i++)  {
      tmp = filt_lp[0] * noise[i+l_subfr-l_frame];
      for(j=1; j<=L_FILT; j++) {
        tmp += filt_lp[j] * (noise[i+l_subfr-l_frame-j] + noise[i+l_subfr-l_frame+j]);
      }
      synth_out[i] = syn[i-l_subfr] - tmp;
    }
  }

  return;
}


float *getLastLpc(HANDLE_USAC_TD_DECODER st) {
  return &st->old_Aq[M+1];
}

float *getAcelpZir(HANDLE_USAC_TD_DECODER st) {
  return st->old_xnq;
}
