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

#ifndef _DEC_IGF_
#define _DEC_IGF_

typedef enum igf_spec_flatten_level {
  IGF_FLAT_MEDIUM,
  IGF_FLAT_OFF,
  IGF_FLAT_STRONG,
  IGF_FLAT_NOTPRESENT
} IGF_FLAT_LEVEL;

typedef enum igf_tnf_level {
  IGF_TNF_OFF,
  IGF_TNF_ON,
  IGF_TNF_NOTPRESENT
} IGF_TNF_LEVEL;

typedef enum igf_tnf_mask {
  IGF_TNF_INCLUDE,
  IGF_TNF_EXCLUDE
} IGF_TNF_MASK;

typedef enum igf_win_type {
  IGF_LONG,
  IGF_SHORT,
  IGF_TCX_MEDIUM,
  IGF_TCX_LONG,
  IGF_CODEC_TRANSITION
} IGF_WIN_TYPE;

typedef enum igf_frame_id {
  IGF_ID_LONG,
  IGF_ID_SHORT,
  IGF_ID_TCX_MEDIUM_FIRST,
  IGF_ID_TCX_MEDIUM_SECOND,
  IGF_ID_TCX_LONG
} IGF_FRAME_ID;

typedef enum igf_cpe_mode {
  IGF_STEREO,
  IGF_DUALMONO
} IGF_CPE_MODE;

typedef struct igf_grid_config_struct {
  int igf_SfbStart;
  int igf_SfbStop;
  int igf_NTiles;
  int igf_Min;
} IGF_GRID_CONFIG, *HIGF_GRID_CONFIG;

typedef struct igf_config_struct {
  int igf_active;
  int igf_UseHighRes;
  int igf_UseINF;
  int igf_UseTNF;
  int igf_UseWhitening;
  int igf_AfterTnsSynth;
  IGF_CPE_MODE igf_IndependentTiling;
  IGF_GRID_CONFIG igf_grid[4];
} IGF_CONFIG, *HIGF_CONFIG;

typedef struct igf_bitstream_struct {
  int igf_allZero;
  int igf_TileNum[4];
  float igf_curr[MAX_SHORT_WINDOWS][NSFB_LONG];
  IGF_FLAT_LEVEL igf_WhiteningLevel[4];
  IGF_TNF_LEVEL igf_isTnfFlat;
} IGF_BITSTREAM, *HIGF_BITSTREAM;

typedef struct igf_memory_struct {
  int igf_prevTileNum[4];
  IGF_FLAT_LEVEL igf_prevWhiteningLevel[4];
  IGF_WIN_TYPE igf_prevWinType;
} IGF_MEMORY, *HIGF_MEMORY;

typedef struct igf_envelope_struct {
  int igf_arith_t;
  int igf_PrevD;
  int igf_prev[NSFB_LONG];
  float igf_sN[NSFB_LONG];
  float igf_pN[NSFB_LONG];
} IGF_ENVELOPE, *HIGF_ENVELOPE;

typedef struct igf_data_struct {
  IGF_MEMORY igf_memory;
  IGF_BITSTREAM igf_bitstream[2];
  IGF_ENVELOPE igf_envelope;
  float igf_infSpec[4 * 2048];
  float igf_tnfSpec[2048];
  IGF_TNF_MASK igf_tnfMask[2048];
} IGF_DEC_DATA, *HIGF_DEC_DATA;

int IGF_get_igfBgn(
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const int                 winOffset
);

int IGF_get_mask(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const Info               *info,
                    const byte               *group,
                    byte                     *mask
);

void IGF_get_cplx_pred_data(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    int                     **alpha_q_re,
                    int                     **alpha_q_im,
                    int                      *alpha_q_re_prev,
                    int                      *alpha_q_im_prev,
                    int                     **cplx_pred_used,
                    int                      *pred_dir,
                    const int                 num_window_groups,
                    const int                 bUsacIndependenceFlag
);

int IGF_level(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    const int                 num_window_groups,
                    const int                 bUsacIndependencyFlag
);

void IGF_data(
                    HANDLE_BUFFER             hVm,
                    HANDLE_RESILIENCE         hResilience,
                    HANDLE_ESC_INSTANCE_DATA  hEscInstanceData,
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    const int                 bUsacIndependencyFlag
);

void IGF_init(
                    HIGF_DEC_DATA             igfDecDataL,
                    HIGF_DEC_DATA             igfDecDataR,
                    HIGF_CONFIG               igfConfig,
                    const int                 igf_active,
                    const int                 igf_UseHighRes,
                    const int                 igf_UseENF,
                    const int                 igf_UseWhitening,
                    const int                 igf_AfterTnsSynth,
                    const int                 igf_IndependentTiling,
                    const int                 igf_StartIdx,
                    const int                 igf_StopIdx,
                    const int                 ccfl,
                    const int                 sampleRate
);

void IGF_apply_tnf(
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coef,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group
);

void IGF_apply_mono(
                    HIGF_DEC_DATA             igfDecData,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coef,
                    unsigned int             *nfSeed,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const int                 num_groups,
                    const short              *group_len,
                    const int                *bins_per_sbk,
                    const int                 blockSize,
                    const int                 samplingRate
);

void IGF_apply_stereo(
                    HIGF_DEC_DATA             igfDecDataL,
                    HIGF_DEC_DATA             igfDecDataR,
                    const HIGF_CONFIG         igfConfig,
                    const IGF_WIN_TYPE        igfWinType,
                    const IGF_FRAME_ID        igfFrmID,
                    float                    *coefL,
                    float                    *coefR,
                    unsigned int             *nfSeedL,
                    unsigned int             *nfSeedR,
                    const WINDOW_SEQUENCE     win,
                    const byte               *group,
                    const byte               *mask,
                    const int               **cplx_pred_used,
                    const int                 hasmask,
                    const int                 num_groups,
                    const short              *group_len,
                    const int                *bins_per_sbk,
                    const int                *sfb_per_sbk,
                    const int                 blockSize,
                    const int                 samplingRate
);
#endif
