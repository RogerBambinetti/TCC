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
#ifndef _DEC_LPD_STEREO_
#define _DEC_LPD_STEREO_

#include "cnst.h"

#define STEREO_LPD_DFT_SIZE         (512 + 160)
#define STEREO_LPD_DEC_DFT_NB       NB_DIV
#define STEREO_LPD_BAND_MAX         20
#define STEREO_LPD_FLT_MIN          1e-10f
#define STERO_LPD_DMX_LIMIT         15.8489f

#ifndef MAX
#define MAX(x1,x2) ((x1)>(x2) ? (x1):(x2))
#endif

#ifndef MIN
#define MIN(x1,x2) ((x1)<(x2) ? (x1):(x2))
#endif

#ifndef ABS
#define ABS(A) ((A) < 0 ? (-A) : (A))
#endif

#ifndef PI
#define PI  3.14159265358979323846264338327950288
#endif

typedef enum stereolpd_res_mode {
  STEREOLPD_ERB_2,
  STEREOLPD_ERB_4
} STEREOLPD_RES_MODE, *HSTEREOLPD_RES_MODE;

typedef struct stereolpd_config_struct {
  int fs;
  int ccfl;
  int dftSize_N;
  int frameSize_M;
  int overlapSize_L;
} STEREOLPD_CONFIG, *HSTEREOLPD_CONFIG;

typedef struct stereolpd_bitstream_struct {
  STEREOLPD_RES_MODE res_mode;
  int q_mode;
  int ipd_mode;
  int pred_mode;
  int cod_mode;
  int ild_idx[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  int ipd_idx[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  int pred_gain_idx[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  int cod_gain_idx[STEREO_LPD_DEC_DFT_NB];
} STEREOLPD_BITSTREAM, *HSTEREOLPD_BITSTREAM;

typedef struct stereolpd_parameter_struct {
  float ild[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  float ipd[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  float pred_gain[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_BAND_MAX];
  float cod_gain[STEREO_LPD_DEC_DFT_NB];
  float S[STEREO_LPD_DEC_DFT_NB][STEREO_LPD_DFT_SIZE];
} STEREOLPD_PARAMETER, *HSTEREOLPD_PARAMETER;

typedef struct stereolpd_helper_struct {
  int band_limits[STEREO_LPD_BAND_MAX + 1];
  int nbands;
  int ipd_band_max;
  int cod_band_max;
  int cod_L;
} STEREOLPD_HELPER, *HSTEREOLPD_HELPER;

typedef struct stereolpd_data_struct {
  short fac_FB;
  const float *win;
  float TimeBuffer_DMX[L_FRAME_1024/2+L_FRAME_1024+L_FRAME_1024/2];
  float past_L[2*PIT_MAX_MAX+L_FRAME_1024/2+LFAC_1024];
  float past_R[2*PIT_MAX_MAX+L_FRAME_1024/2+LFAC_1024];
  float DFT_past_DMX[STEREO_LPD_DFT_SIZE];
  int bpf_pitch[NB_SUBFR_SUPERFR_1024+BPF_SFD];
  float bpf_gain[NB_SUBFR_SUPERFR_1024+BPF_SFD];
  float old_noise_pf_L[(2*L_FILT+1)*2+LTPF_MAX_DELAY];
  float old_noise_pf_R[(2*L_FILT+1)*2+LTPF_MAX_DELAY];

  STEREOLPD_CONFIG    lpdstereo_config;
  STEREOLPD_BITSTREAM lpdstereo_bitstream;
  STEREOLPD_PARAMETER lpdstereo_parameter;
  STEREOLPD_HELPER    lpdstereo_helper;
} STEREOLPD_DEC_DATA, *HSTEREOLPD_DEC_DATA;

void stereoLpd_init(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const int                   fs,
                    const int                   fullbandLPD,
                    const int                   ccfl
);

void stereoLpd_setDMX(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth
);

void stereoLpd_setPastL(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth,
                    const int                   length
);

void stereoLpd_setPastR(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth,
                    const int                   length
);

void stereoLpd_setBPFprm(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const int                  *pitch,
                    const float                *gain,
                    const int                   nbSubfrSuperfr
);

void stereoLpd_data(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    HANDLE_RESILIENCE           hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER               hVm,
                    const int                   aceStartFlag
);

void stereoLpd_apply(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *synth_L,
                    float                      *synth_R,
                    const int                   aceStartFlag
);

void stereoLpd_applyEnd(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *synth_L,
                    float                      *synth_R
);

void stereoLpd_applyBPFend(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *out_buffer,
                    const int                  *old_pitch,
                    const float                *old_gain,
                    const int                   last_mode,
                    const int                   ch,
                    const int                   isShort
);

#endif
