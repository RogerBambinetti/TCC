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


#ifndef _USAC_MAIN_STRUCT_H_INCLUDED
#define _USAC_MAIN_STRUCT_H_INCLUDED

#include "allHandles.h"
#include "vmtypes.h"
#include "block.h"
#include "interface.h"
#include "usac_interface.h"
#include "tf_mainStruct.h"      /* for Info */
#include "usac_arith_dec.h"     /* for arithQ*/
#include "all.h"
#include "tf_mainHandle.h"
#include "usac_tcx_mdct.h"

#include "cnst.h"

#ifdef CT_SBR
#include "ct_sbrdecoder.h"
#include "ct_sbrconst.h"
#endif

#include "sac_polyphase.h"
#include "dec_IGF.h"
#include "dec_HREP.h"

#ifndef UNIFIED_RANDOMSIGN
#define UNIFIED_RANDOMSIGN
#endif

#define MAX_NUM_SIGNALGROUPS 64

#include "usac_multichannel.h"
#include "usac_ltpf.h"
#include "usac_td_resampler.h"
#include "dec_TBE.h"
#include "dec_lpd_stereo.h"

typedef struct tagTwMdct {
  int dummy;
} USAC_TW, *HANDLE_TW;

typedef struct tagUsacFdDecoder {
  int dummy;
} USAC_FD_DECODER, *HANDLE_USAC_FD_DECODER;

typedef struct {
  int   mode;              /* LPD mode (0=ACELP frame)                */
  int   nbits;             /* number of bits used by ACELP or TCX     */

  /* signal memory */
  float Aq[2*(M+1)];       /* for TCX overlap-add synthesis (2 subfr) */
  float Ai[2*(M+1)];       /* for OA weighted synthesis (2 subfr)     */
  float syn[M+128];       /* synthesis memory                        */
  float wsyn[1+128];      /* weighted synthesis memory               */

  /* ACELP memory */
  float Aexc[2*L_DIV_1024];     /* ACELP exc memory (Aq)                   */

  /* FAC memory */
  int   RE8prm[LFAC_1024];      /* MDCT RE8 coefficients (encoded in ACELP)*/

  /* TCX memory */
  float Txn[128];        /* TCX target memory (2 subfr, use Aq)     */
  float Txnq[1+(2*128)];  /* Q target (overlap or ACELP+ZIR, use Aq) */
  float Txnq_fac;          /* Q target with fac (use Aq)              */
} LPD_state;

typedef struct tagUsacTdDecoderConfig{
  int fullbandLPD;
  int lpdStereoIndex;
  int bUseNoiseFilling;
  int igf_active;
  int lFrame;                           /* Input frame length (long TCX)                    */
  int lFrameFB;                         /* Input frame length (long TCX) in full rate       */
  int lDiv;                             /* ACELP or short TCX frame length                  */
  int nbDiv;                            /* Number of frame (division) in a superframe       */
  int nbSubfr;                          /* Number of 5ms subframe per 20ms frame            */
  int fscale;                           /* scale factor (FSCALE_DENOM = 1.0)                */
  int fscaleFB;                         /* sampling rate at FB                              */
  int fac_FB;                           /* ratio factor between sampling rates of LB and FB */
} USAC_TD_DECODER_CONFIG, *HANDLE_USAC_TD_DECODER_CONFIG;

typedef struct tagUsacTdDecoder {
  int fullbandLPD;                      /* fullband LPD index flag                      */
  int bUseNoiseFilling;                 /* use noise filling                            */
  int igf_active;                       /* use enhanced noise filling                   */
  int lpdStereoIndex;                   /* use stereo LPD                               */
  int lFrame;                           /* Input frame length (long TCX)                */
  int lFrameFB;                         /* Input frame length (long TCX) in full rate   */
  int lDiv;                             /* ACELP or short TCX frame length              */
  int nbSubfr;                          /* Number of 5ms subframe per 20ms frame        */
  int nbDiv;                            /* Number of frame (division) in a superframe   */

  /* dec_main.c */
  int last_mode;                         /* last mode in previous 80ms frame */

  /* dec_lf.c */
  float old_Aq[2*(M+1)];                 /* last tcx overlap synthesis       */
  float old_xnq[1+(2*LFAC_1024)];
  float old_xnqFB[(4*LFAC_1024)];
  float old_xnq_synth[1+(2*LFAC_1024)];
  float old_synth[PIT_MAX_MAX+SYN_DELAY_1024];/* synthesis memory                 */
  float old_synthFB[2*(PIT_MAX_MAX+SYN_DELAY_1024)];/* synthesis memory for FB signal               */
  float old_exc[PIT_MAX_MAX+L_INTERPOL];      /* old excitation vector (>20ms)    */

  /* bass_pf.c */
  float old_noise_pf[2*L_FILT+1+2*BPF_DELAY_1024];  /* bass post-filter: noise memory   */
  int old_T_pf[SYN_SFD_1024];                 /* bass post-filter: old pitch      */
  float old_gain_pf[SYN_SFD_1024];            /* bass post-filter: old pitch gain */

  /* FAC */
  float FAC_gain;
  float FAC_alfd[LFAC_1024/4];

  float lsfold[M];                     /* old isf (frequency domain) */
  float lspold[M];                     /* old isp (immittance spectral pairs) */
  float past_lsfq[M];                  /* past isf quantizer */
  float lsf_buf[L_MEANBUF*(M+1)];      /* old isf (for frame recovery) */
  int old_T0;                          /* old pitch value (for frame recovery) */
  int old_T0_frac;                     /* old pitch value (for frame recovery) */
  short seed_ace;                      /* seed memory (for random function) */
#ifdef UNIFIED_RANDOMSIGN
  unsigned int seed_tcx;               /* seed memory (for random function) */
#else
  short seed_tcx;                      /* seed memory (for random function) */
#endif
  float past_gpit;                     /* past gain of pitch (for frame recovery) */
  float past_gcode;                    /* past gain of code (for frame recovery) */
  float gc_threshold;
  float wsyn_rms;

  HIGF_DEC_DATA igfDecData;
  TBE_DEC_DATA tbeDecData;
  TNS_frame_info tns;
  int window_shape;
  WINDOW_SHAPE window_shape_prev;
  int max_sfb;
  int tns_data_present;
  LTPF_DATA ltpfData;
  int fdp_data_present;
  int fdp_spacing_index;
  int fdp_int[172];
  int fdp_quantSpecPrev[2][172];

  HANDLE_TCX_MDCT hTcxMdct;

  Tqi2 qbuf[N_MAX/2+4];

  int fscale;      /* scale factor (FSCALE_DENOM = 1.0)  */
  int fscaleFB;     /* sampling rate at FB    */
  int fac_FB;       /* ratio factor between sampling rates of LB and FB*/

  float fdSynth_Buf[3*L_DIV_1024+1];
  float *fdSynth;

  int prev_BassPostFilter_activ;

  float *pOlaBuffer;
  float *pOlaBufferFB;

  float TD_resampler_mem[2*L_FILT_MAX];
  int TD_resampler_delay;                /* TD resampler delay in samples at output sampling rate*/
} USAC_TD_DECODER, *HANDLE_USAC_TD_DECODER;

typedef struct tagUsacDecoder {
  int blockSize;
  int twMdct[MAX_NUM_ELEMENTS];

  float *coef[Chans];
  float *coefSave[Chans]; /* save output for cplx prediction downmix */
  float *data[Chans];
  float *state[Chans];

  WINDOW_SEQUENCE wnd[Winds];
  Wnd_Shape wnd_shape[Winds];

  byte *mask[Winds]; /* MS-Mask */
  byte *cb_map[Chans];
  short *factors[Chans];
  byte *group[Winds];

  TNS_frame_info *tns[Chans];
  int tns_data_present[Chans];
  int tns_ext_present[Chans];

  Info *info;

  int samplingRate;
  int samplingRateIdx;
  int fscale;       /* derived from sampling rate */

  /* data stream */

  int d_tag, d_cnt;
  byte d_bytes[Avjframe];

  MC_Info *mip;

  HREP_DATA hrepData;

  /*
   SSR Profile
   */
  int short_win_in_long;

  int max_band[Chans];

  USAC_TD_DECODER_CONFIG tdconfig[MAX_NUM_ELEMENTS];
  HANDLE_USAC_TD_DECODER tddec[Chans];
  HANDLE_USAC_FD_DECODER fddec;

  SBRBITSTREAM ct_sbrBitStr[2];

  SAC_POLYPHASE_ANA_FILTERBANK *filterbank[MAX_NUM_ELEMENTS];

  /* TW-MDCT members */
  int tw_data_present[Chans];
  int *tw_ratio[Chans];

  /* Arith coding */
  int  arithPreviousSize[Chans];
  Tqi2 arithQ[Chans][1024/2+4];

  /*Noise Filling*/
  int bUseNoiseFilling[MAX_NUM_ELEMENTS];

  /* IGF data */
  IGF_CONFIG igfConfig[MAX_NUM_ELEMENTS];
  IGF_DEC_DATA igfDecData[Chans];

  unsigned int nfSeed[Chans];

  /* FAC data*/
  int ApplyFAC[Chans];
  int facData[Chans][LFAC_1024+1];
  short quantSpecPrev[Chans][2*172];
  int currAliasingSymmetry[Chans];
  int prevAliasingSymmetry[Chans];
  LTPF_DATA ltpfData[Chans];
  MULTICHANNEL_DATA *mcData[MAX_NUM_SIGNALGROUPS];
  float scaleFactors[Chans][MAX_SCFAC_BANDS];
  int receiverDelayCompensation;

  int fullbandLPD[MAX_NUM_ELEMENTS];
  int lpdStereoIndex[MAX_NUM_ELEMENTS];
  STEREOLPD_DEC_DATA stereolpdDecData[MAX_NUM_ELEMENTS];
#ifdef PDAM3_PHASE2_TCC
  int tccAplied;
#endif
} T_USAC_DECODER;

typedef enum {
  USAC_EXT_TYPE_UNDEFINED              = -1,
  USAC_EXT_TYPE_FILL                   = 0,
  USAC_EXT_TYPE_MPEGS                  = 1,
  USAC_EXT_TYPE_SAOC                   = 2,
  USAC_EXT_TYPE_AUDIOPREROLL           = 3,
  USAC_EXT_TYPE_UNI_DRC                = 4,
  USAC_EXT_TYPE_OBJ                    = 5,
  USAC_EXT_TYPE_SAOC_3D                = 6,
  USAC_EXT_TYPE_HOA                    = 7,
#if IAR
  USAC_EXT_TYPE_FMC                    = 8,
#endif
  USAC_EXT_TYPE_MC                     = 9,
#ifdef PDAM3_PHASE2_TCC
  USAC_EXT_TYPE_TCC                    = 10,
#endif
#ifdef PDAM3_HOA_ENH_LAYER
  USAC_EXT_TYPE_HOA_ENH_LAYER          = 11,
#endif
#ifdef PDAM3_HREP
  USAC_EXT_TYPE_HREP                   = 12,
#endif
   USAC_EXT_TYPE_ENHANCED_OBJ_METADATA = 13,
   USAC_EXT_TYPE_PRODUCTION_METADATA   = 14
} USAC_EXT_TYPE;

typedef struct _usac_ext_element {
  UINT32  usacExtElementPresent;
  UINT32  usacExtElementUseDefaultLength;
  unsigned int  usacExtElementPayloadLength;
  UINT32  usacExtElementPayloadFrag;
  UINT32  usacExtElementStart;
  UINT32  usacExtElementStop;
#ifdef EXT_PAYLOAD_NOFIX
  UINT32 usacExtElementSegmentData[6144/8];
#else
  unsigned char usacExtElementSegmentData[6144/8*MAX_CHANNELS];
#endif
} USAC_EXT_ELEMENT;

typedef struct {
  MPEG3DAPROFILELEVEL mpegh3daProfileLevelIndication;

  int tnsBevCore;
  int output_select;
  float* time_sample_vector[MAX_OSF * MAX_TIME_CHANNELS];
  float* spectral_line_vector[MAX_TIME_CHANNELS];
  WINDOW_SHAPE windowShape[MAX_TIME_CHANNELS];
  WINDOW_SHAPE prev_windowShape[MAX_TIME_CHANNELS];
  WINDOW_SEQUENCE windowSequence[MAX_TIME_CHANNELS];

  int First_frame_flag[MAX_TIME_CHANNELS];

  USAC_CORE_MODE coreMode[MAX_TIME_CHANNELS];
  USAC_CORE_MODE prev_coreMode[MAX_TIME_CHANNELS];

  float* overlap_buffer_buf[MAX_OSF * MAX_TIME_CHANNELS];
  float* overlap_buffer[MAX_OSF * MAX_TIME_CHANNELS];

  Info** sfbInfo;
  int block_size_samples;
  float* sampleBuf[MAX_TIME_CHANNELS];
  HANDLE_MODULO_BUF_VM coreModuloBuffer;
  float* mdct_overlap_buffer;
  WINDOW_SEQUENCE windowSequenceLast[MAX_TIME_CHANNELS];
  int samplFreqFacCore;
  int decoded_bits;
  int mdctCoreOnly; /* debug switch */
  int twMdct[MAX_ELEMENTS];
  int bStereoSbr[MAX_ELEMENTS];
  int bPseudoLr[MAX_ELEMENTS];

  int FrameIsTD[MAX_TIME_CHANNELS];
  int FrameWasTD[MAX_TIME_CHANNELS];
  HANDLE_MODULO_BUF_VM hrepInBuffer[MAX_TIME_CHANNELS];

  /* tw-mdct */
  float                warp_sum[MAX_TIME_CHANNELS][2];
  float               *warp_cont_mem[MAX_TIME_CHANNELS];
  float               *prev_sample_pos[MAX_TIME_CHANNELS];
  float               prev_tw_trans_len[MAX_TIME_CHANNELS][2];
  int               prev_tw_start_stop[MAX_TIME_CHANNELS][2];
  float               *prev_warped_time_sample_vector[MAX_TIME_CHANNELS];
  int osf; /* this defaults to 1 for non-sls */
  int sbrRatioIndex;

#ifdef CT_SBR
  int bDownSampleSbr;
  int runSbr;
#ifdef PARAMETRICSTEREO
  int sbrEnablePS;
#endif
#endif

  /*
   USAC Decoder data
   */
  HANDLE_USAC_DECODER usacDecoder;

  /*
   sbr Decoder
   */

#ifdef CT_SBR
  SBRDECODER ct_sbrDecoder;
  float sbrQmfBufferReal[TIMESLOT_BUFFER_SIZE][QMF_BUFFER_SIZE];
  float sbrQmfBufferImag[TIMESLOT_BUFFER_SIZE][QMF_BUFFER_SIZE];

#endif

  /* FAC transition*/
  float  lastLPC[MAX_TIME_CHANNELS][M+1];
  float  acelpZIR[MAX_TIME_CHANNELS][2*(1+(2*LFAC_1024))]; 

  /* last TBE synth */
  float tbe_synth[MAX_TIME_CHANNELS][2*L_DIV_1024];

  int ***alpha_q_re, ***alpha_q_im;
  int *alpha_q_re_prev[Chans], *alpha_q_im_prev[Chans];
  float *dmx_re_prev[Chans];
  int ***cplx_pred_used;

  /* usac independency flag */
  int usacIndependencyFlag;

  /* for debug purposes only, remove before MPEG upload! */
  int frameNo;

#ifdef SONY_PVC_DEC
  int	debug_array[64];
  int	sbr_mode;
#endif /* SONY_PVC_DEC */


  int              usacExtNumElements;
  USAC_EXT_TYPE    usacExtElementType[MAX_NUM_ELEMENTS];              /* extension type found in ext element */
  USAC_EXT_ELEMENT usacExtElement[MAX_NUM_ELEMENTS];
  int              usacExtElementComplete[MAX_NUM_ELEMENTS];

  int              usacElementLength[MAX_NUM_ELEMENTS];

  int              silentDecoder;

  /* parse info */
  int tns_on_lr[MAX_TIME_CHANNELS];
  int core_mode_element[MAX_TIME_CHANNELS][2];
  unsigned char max_sfb_ste[MAX_TIME_CHANNELS];
  unsigned char max_sfb1[MAX_TIME_CHANNELS];
  unsigned char max_sfb_ste_clear[MAX_TIME_CHANNELS];
  int max_spec_coefficients[MAX_TIME_CHANNELS][2];
  int complex_coef[MAX_TIME_CHANNELS];
  int use_prev_frame[MAX_TIME_CHANNELS];
  byte igf_hasmask[MAX_TIME_CHANNELS][Winds];
  int igf_pred_dir[MAX_TIME_CHANNELS];
  int igf_SfbStart[MAX_TIME_CHANNELS];
  int igf_SfbStop[MAX_TIME_CHANNELS];
  int pred_dir[MAX_TIME_CHANNELS];
  int stereoFilling[MAX_TIME_CHANNELS];
  int common_window[MAX_TIME_CHANNELS];
  int stereoConfigIndexElement[MAX_TIME_CHANNELS];
  Info sfbInfoElement[MAX_TIME_CHANNELS];

  int numChannels;
} USAC_DATA;


typedef struct tagUsacTdEncoder {
  int fullbandLPD;                      /* fullband LPD index flag    */
  int igf_active;                       /* use enhanced noise filling */
  int lFrame;   /* Input frame length (long TCX)         */
  int lDiv;     /* ACELP or short TCX frame length       */
  int nbSubfr;  /* Number of 5ms subframe per 20ms frame */

  short mode;                  /* ACELP core mode: 0..7 */
  int fscale;

  float mem_lp_decim2[3];              /* wsp decimation filter memory */

  /* cod_main.c */
  int decim_frac;
  float mem_sig_in[4];                 /* hp50 filter memory */
  float mem_preemph;                   /* speech preemph filter memory */

  float old_speech_pe[L_OLD_SPEECH_HIGH_RATE+L_LPC0_1024];
  float old_speech[L_OLD_SPEECH_HIGH_RATE+L_LPC0_1024];   /* old speech vector at 12.8kHz */
  float old_synth[M];                  /* synthesis memory */

  /* cod_lf.c */
  float wsig[128];
  LPD_state LPDmem;
  float old_d_wsp[PIT_MAX_MAX/OPL_DECIM];  /* Weighted speech vector */
  float old_exc[PIT_MAX_MAX+L_INTERPOL];   /* old excitation vector */

  float old_mem_wsyn;                  /* weighted synthesis memory */
  float old_mem_w0;                    /* weighted speech memory */
  float old_mem_xnq;                   /* quantized target memory */
  int old_ovlp_size;                   /* last tcx overlap size */

  float isfold[M];                     /* old isf (frequency domain) */
  float ispold[M];                     /* old isp (immittance spectral pairs) */
  float ispold_q[M];                   /* quantized old isp */
  float mem_wsp;                       /* wsp vector memory */

  /* memory of open-loop LTP */
  float ada_w;
  float ol_gain;
  short ol_wght_flg;

  long int old_ol_lag[5];
  int old_T0_med;
  float hp_old_wsp[L_FRAME_1024/OPL_DECIM+(PIT_MAX_MAX/OPL_DECIM)];
  float hp_ol_ltp_mem[/* HP_ORDER*2 */ 3*2+1];

  /* LP analysis window */
  float window[L_WINDOW_HIGH_RATE_1024];

  /* mdct-based tcx*/
  HANDLE_TCX_MDCT hTcxMdct;
  float xn_buffer[128];
  Tqi2  arithQ[1024/2+4];
  int   arithReset;

  short prev_mod;
  short codingModeSelection;

  int  nb_bits; /*number of bits used for a superframe*/

  /* ACELP startup, past FD synthesis including FD-FAC */
  float fdSynth[2*L_DIV_1024+1+M];
  float fdOrig[2*L_DIV_1024+1+M];
  int lowpassLine;

  /* left and right short overlap if short FD -> LPD */
  int lastWasShort;
  int nextIsShort;

} USAC_TD_ENCODER, *HANDLE_USAC_TD_ENCODER;


#endif  /* #ifndef _USAC_STRUCT_MAIN_H_INCLUDED */
