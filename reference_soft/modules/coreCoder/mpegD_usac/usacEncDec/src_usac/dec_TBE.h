/***********************************************************************************
 
 This software module was originally developed by 
 
 Qualcomm Techonologies, Inc. (QTI) and Fraunhofer IIS
 
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
 
 Qualcomm Techonologies, Inc. (QTI) and Fraunhofer IIS 
 retain full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2015.
 
 ***********************************************************************************/

#ifndef _DEC_TBE_
#define _DEC_TBE_

#define L_FRAME_TBE         L_DIV_1024
#define NB_SUBFR            4
#define NUM_SUBFR           16
#define L_SUBFR_TBE         (L_FRAME_TBE / NB_SUBFR)
#define PI_INV              1.0/PI
#define NUM_SUBGAINS        4
#define AP_NUM              3
#define LPC_ORDER           10
#define LPC_WHTN_ORDER      4
#define NL_BUFF_OFFSET      12
#define NL_BUFF_OFFSET_MAX  32
#define RESP_ORDER          15
#define TBE_LAHEAD          16
#define TBE_OVERLAP_LEN     16
#define TBE_DEC_DELAY       (TBE_LAHEAD + (NL_BUFF_OFFSET / 2))
#define TBE_PIT_MAX         434

typedef enum tbe_nl_level {
  NL_HARMONIC,
  NL_SMOOTH,
  NL_HYBRID
} TBE_NL_LEVEL;

typedef enum tbe_frame_class {
  TBE_CLASS_ACELP_FIRST,
  TBE_CLASS_ACELP_SECOND
} TBE_FRAME_CLASS;

typedef enum tbe_lsp2lsf_dir {
  TBE_LSP2LSF,
  TBE_LSF2LSP
} TBE_LSP2LSF_DIR;

typedef struct tbe_bitstream_struct {
  int tbe_heMode;
  int idxFrameGain;
  int idxSubGains;
  int lsf_idx[2];
  int idxMixConfig;
  int idxShbFrGain;
  int idxResSubGains;
  int idxShbExcResp[2];
  int tbe_hrConfig;
  int tbe_nlConfig;
} TBE_BITSTREAM, *HTBE_BITSTREAM;

typedef struct tbe_memory_struct {
  float gen_synth_state_lsyn_filt_local[(2 * AP_NUM + 1)];
  float mem_csfilt[2];
  float mem_gen_exc_filt_down[(2 * AP_NUM + 1)];
  float mem_resp_excWhtnd[RESP_ORDER];
  float mem_whtn_filt[LPC_WHTN_ORDER];
  float old_tbe_exc[2 * TBE_PIT_MAX];
  float old_tbe_exc_extended[NL_BUFF_OFFSET];
  float state_lpc_syn[LPC_ORDER];
  float state_syn_exc[TBE_LAHEAD];
  float syn_overlap[TBE_LAHEAD];
  float wn_ana_mem[LPC_ORDER];
  float wn_syn_mem[LPC_ORDER];
} TBE_MEMORY, *HTBE_MEMORY;

typedef struct tbe_data_struct {
  TBE_BITSTREAM tbe_bitstream[2];
  TBE_MEMORY tbe_memory;
  short tbe_seed[2];
  float lsp_prev_interp[LPC_ORDER];
  float old_tbe_synth[2 * L_DIV_1024];
  float tbe_synth_buffer[2 * L_DIV_1024];
  float tbe_excitation[(TBE_PIT_MAX + (L_DIV_1024 + 1) + L_SUBFR) * 2];
} TBE_DEC_DATA, *HTBE_DEC_DATA;

void TBE_apply(
                    HTBE_DEC_DATA             tbeDecData,
                    const TBE_FRAME_CLASS     tbeFrmClass,
                    float                    *tbe_exc_extended,
                    const float              *voice_factors,
                    float                    *synth,
                    float                    *pitch_buf,
                    const int                 first_frame,
                    float                    *cur_sub_Aq,
                    const int                 L_frame
);

void TBE_data(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HTBE_DEC_DATA             tbeDecData,
                    const TBE_FRAME_CLASS     tbeFrmClass
);

void TBE_genTransition(
                    HTBE_DEC_DATA             tbeDecData,
                    int                       length,
                    float                    *output
);

void TBE_init(
                    HTBE_DEC_DATA             tbeDecData
);

void TBE_mixPastExc(
                    HTBE_DEC_DATA             tbeDecData,
                    float                    *error,
                    const int                 sFrameIdx,
                    const int                 T0,
                    const int                 T0_frac
);

void TBE_prepExc(
                    HTBE_DEC_DATA             tbeDecData,
                    const int                 L_frame,
                    const int                 i_subfr,
                    const float               gain_pit,
                    const float               gain_code,
                    const float              *code,
                    const float              *code_formSharp,
                    const float               voice_fac,
                    float                    *voice_factors,
                    const float              *exc,
                    const float              *exc2
);

void TBE_reset(
                    HTBE_DEC_DATA             tbeDecData
);

#endif