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


#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "acelp_plus.h"
#include "re8.h"
#include "usac_allVariables.h"
#include "usac_port.h"

#ifdef UNIFIED_RANDOMSIGN
static float randomSign(unsigned int *seed);
#else
static void rnd_sgn16(short *seed, float *xri, int lg);
#endif

static unsigned short fdp_s1[129] = { /*  49152 * fdp_scl .* fdp_sin */
       0,     726,    1451,    2176,    2901,    3625,    4348,    5071,
    5792,    6512,    7230,    7947,    8662,    9376,   10087,   10796,
   11503,   12207,   12908,   13607,   14303,   14995,   15685,   16371,
   17054,   17732,   18408,   19079,   19746,   20409,   21068,   21722,
   22372,   23017,   23658,   24293,   24924,   25549,   26169,   26784,
   27394,   27998,   28596,   29189,   29776,   30357,   30932,   31501,
   32064,   32621,   33171,   33715,   34253,   34784,   35309,   35827,
   36339,   36844,   37282,   37698,   38105,   38503,   38892,   39272,
   39643,   40005,   40359,   40703,   41039,   41366,   41684,   41994,
   42296,   42589,   42874,   43151,   43419,   43680,   43933,   44179,
   44416,   44646,   44869,   45085,   45293,   45494,   45689,   45876,
   46057,   46232,   46400,   46562,   46717,   46867,   47011,   47149,
   47281,   47408,   47529,   47645,   47756,   47862,   47963,   48059,
   48150,   48237,   48319,   48397,   48471,   48540,   48605,   48666,
   48724,   48777,   48826,   48872,   48914,   48953,   48988,   49019,
   49047,   49072,   49093,   49111,   49126,   49137,   49146,   49150,   49152};

static   signed short fdp_s2[129] = { /* -18432 * fdp_scl .* fdp_scl */
  -26681,  -26680,  -26678,  -26675,  -26670,  -26665,  -26658,  -26650,
  -26641,  -26630,  -26618,  -26605,  -26591,  -26576,  -26559,  -26541,
  -26522,  -26502,  -26481,  -26459,  -26436,  -26411,  -26386,  -26359,
  -26331,  -26303,  -26273,  -26242,  -26211,  -26178,  -26145,  -26110,
  -26075,  -26039,  -26002,  -25964,  -25926,  -25887,  -25847,  -25806,
  -25765,  -25723,  -25680,  -25637,  -25593,  -25549,  -25504,  -25458,
  -25413,  -25366,  -25320,  -25273,  -25225,  -25178,  -25130,  -25082,
  -25033,  -24985,  -24856,  -24710,  -24564,  -24417,  -24272,  -24126,
  -23981,  -23836,  -23691,  -23547,  -23404,  -23262,  -23121,  -22980,
  -22841,  -22702,  -22565,  -22429,  -22295,  -22162,  -22030,  -21900,
  -21771,  -21644,  -21519,  -21396,  -21274,  -21155,  -21037,  -20921,
  -20808,  -20697,  -20587,  -20481,  -20376,  -20274,  -20174,  -20077,
  -19982,  -19889,  -19799,  -19712,  -19627,  -19546,  -19466,  -19390,
  -19316,  -19245,  -19177,  -19112,  -19049,  -18990,  -18933,  -18879,
  -18829,  -18781,  -18736,  -18695,  -18656,  -18620,  -18588,  -18558,
  -18532,  -18508,  -18488,  -18471,  -18457,  -18446,  -18438,  -18434,  -18432};

static const unsigned int pow10gainDiv28[128] = {
     1024,     1112,     1207,     1311,     1423,     1545,     1677,     1821,     1977,     2146,
     2330,     2530,     2747,     2983,     3238,     3516,     3817,     4144,     4499,     4885,
     5304,     5758,     6252,     6788,     7370,     8001,     8687,     9432,    10240,    11118,
    12071,    13105,    14228,    15448,    16772,    18210,    19770,    21465,    23305,    25302,
    27471,    29825,    32382,    35157,    38171,    41442,    44994,    48851,    53038,    57584,
    62519,    67878,    73696,    80012,    86870,    94316,   102400,   111177,   120706,   131052,
   142284,   154480,   167720,   182096,   197703,   214649,   233047,   253021,   274708,   298254,
   323817,   351572,   381706,   414422,   449943,   488508,   530378,   575838,   625193,   678779,
   736958,   800124,   868703,   943161,  1024000,  1111768,  1207059,  1310517,  1422843,  1544797,
  1677203,  1820958,  1977034,  2146488,  2330466,  2530213,  2747080,  2982536,  3238172,  3515720,
  3817056,  4144220,  4499426,  4885077,  5303782,  5758375,  6251932,  6787792,  7369581,  8001236,
  8687031,  9431606, 10240000, 11117682, 12070591, 13105175, 14228434, 15447969, 16772032, 18209581,
 19770345, 21464883, 23304662, 25302131, 27470805, 29825358, 32381723, 35157197};

static int getNoiseBandwidth(WINDOW_SEQUENCE            win,
                             byte                      *group,
                             const int                  mode,
                             const int                  max_sfb,
                             const int                  specLength,
                             const int                  winOffset
) {
  const short *swb_offset;
  int swb_shift;
  int sfbTotal;
  Info *sfbInfo = usac_winmap[win];

  swb_offset = sfbInfo->sbk_sfb_top[winOffset];
  sfbTotal   = sfbInfo->sfb_per_sbk[winOffset];
  swb_shift  = mode << 2;

  if (max_sfb == 0) {
    return 0;
  }
  if ((max_sfb < 0) || (max_sfb > sfbTotal)) {
    return specLength;
  }
  if ((swb_offset[max_sfb - 1] * swb_shift) > specLength) {
    return specLength;
  }
  return (swb_offset[max_sfb - 1] * swb_shift);
}

static void tns_lpd_SynthesisFilter(float              *spec,
                                    const int           size,
                                    const int           direction,
                                    float              *lpc,
                                    const int           order
) {
  int i, j;
  int increment;
  float y, state[8];

  if (direction) {
    increment = -1;
  }  else {
    increment = 1;
  }

  if (size > 0) {
    for (i = 0; i < order; i++) {
      state[i] = 0.0f;
    }
    if (increment == -1) {
      spec += size - 1;
    }
    for (i = 0; i < size; i++) {
      y = *spec;
      for (j = 0; j < order; j++) {
        y -= lpc[j+1] * state[j];
      }
      for (j = order - 1; j > 0; j--) {
        state[j] = state[j-1];
      }
      state[0] = *spec = y;
      spec += increment;
    }
  }
}

static void tns_lpd_PrepareLPCoefficients(const TNSfilt      *filter,
                                          float              *lpc,
                                          const int           coef_res
) {
  const float twoDivPi = 0.63661977f; /* 2/pi */
  const float iqFac_n = 1.0f / (((1 << (coef_res - 1)) + 0.5f) * twoDivPi);
  const float iqFac_p = 1.0f / (((1 << (coef_res - 1)) - 0.5f) * twoDivPi);
  float tmp[8], b[8];
  int  i, m;

  if (filter->order > 0) {
    for (i = 0; i < filter->order; i++) {
      tmp[i+1] = (float)sin((double)filter->coef[i] * (double)((filter->coef[i] < 0) ? iqFac_n : iqFac_p));
    }
    lpc[0] = 1.0f;
    for (m = 1; m <= filter->order; m++) {
      b[0] = lpc[0];
      for (i = 1; i < m; i++) {
        b[i] = lpc[i] + tmp[m] * lpc[m-i];
      }
      b[m] = tmp[m];
      for (i = 0; i <= m; i++) {
        lpc[i] = b[i];
      }
    }
  }
}

static void decode_tns_lpd(WINDOW_SEQUENCE            win,
                           byte                      *group,
                           TNS_frame_info            *tns,
                           const int                  subFrameIndex,
                           const int                  subFrameCounter,
                           const int                  mode,
                           float                     *specCoeffs,
                           const int                  max_sfb,
                           const int                  winOffset
) {
  const TNSfilt *f = tns->info[subFrameIndex].filt;
  const short *swb_offset;
  float filterCoeffs[8];
  int filterStart;
  int filterStop;
  int swb_shift;
  Info *sfbInfo = usac_winmap[win];

  swb_offset = sfbInfo->sbk_sfb_top[winOffset];
  swb_shift = mode << 2;

  if (f->stop_band > 15) {
    return;
  }
  if (!tns->info[subFrameIndex].n_filt || (f->order <= 0)) {
    return;
  }
  tns_lpd_PrepareLPCoefficients(f,
                                filterCoeffs,
                                tns->info[subFrameIndex].coef_res);

  filterStart = swb_offset[f->start_band-1] * swb_shift;
  filterStop  = swb_offset[min(f->stop_band, max_sfb)-1] * swb_shift;

  tns_lpd_SynthesisFilter(specCoeffs + filterStart,
                          filterStop - filterStart,
                          f->direction,
                          filterCoeffs,
                          f->order);
}

unsigned int roundedSquareRoot(const unsigned int positiveNumber
) {
  unsigned int number = positiveNumber;
  unsigned int value = 0;
  unsigned int bitSet = 1 << 30;

  while (bitSet > number) {
    bitSet >>= 2;
  }
  while (bitSet) {
    if (number >= value + bitSet) {
      number -= value + bitSet;
      value  += bitSet << 1;
    }
    value  >>= 1;
    bitSet >>= 2;
  }
  number = value + 1;
  if (number * number - positiveNumber < positiveNumber - value * value) {
    return number;
  }
  return value;
}

static void decode_fdp(const int            fdp_data_present,
                       const int            fdp_spacing_index,
                       int                 *fdp_int,
                       int                  quantSpecPrev[2][172],
                       float               *outputSpecCurr,
                       const int           *quantSpecCurr,
                       const int            quantGainCurr,
                       const float          i_gain,
                       const int            maxLines,
                       const int            lg,
                       int                  predictionBandwidth
) {
  int i, s1, s2, x;
  int fdp_spacing_value;
  int harmonicSpacing;
  int harmIndex = -128;
  int compIndex = 256;

  /* step 1 */
  fdp_spacing_value = (894 / 3) - fdp_spacing_index;
  harmonicSpacing = (894 * 512 + fdp_spacing_value) / (2 * fdp_spacing_value);

  /* step 2 */
  predictionBandwidth = MIN(lg, predictionBandwidth);

  /* step 3*/
  s1 = 0;
  s2 = 0;
  quantSpecPrev[1][0] = 0;

  if (fdp_data_present) {
    for (i = 0; i < predictionBandwidth; i++) {
      if (abs(i * 256 - harmIndex) >= 384) {
        fdp_int[i] = 0;
      } else {
        const int reg32 = s1 * quantSpecPrev[0][i] + s2 * quantSpecPrev[1][i];
        fdp_int[i] = (((unsigned int)abs(reg32) + 16384) >> 15);
        if (reg32 < 0) {
          fdp_int[i] *= -1;
        }
        outputSpecCurr[i] += i_gain * fdp_int[i];
      }
      if (i * 256 == compIndex) {
        harmIndex += harmonicSpacing;
        compIndex = harmIndex & 255;
        if (compIndex > 128) {
          compIndex = 256 - compIndex;
        }

        s1 = fdp_s1[compIndex];
        s2 = fdp_s2[compIndex];

        compIndex = harmIndex >> 8;
        if ((compIndex & 1) == 0) {
          s1 *= -1;
        }
        compIndex = 256 + ((harmIndex + 128) >> 8) * 256;
      }
    }
  }

  /* step 4 */
  for (i = 0; i < predictionBandwidth; i++) {
    x = quantSpecCurr[i] * quantGainCurr;
    if (fdp_data_present) {
      x += fdp_int[i];
    }
    quantSpecPrev[1][i] = quantSpecPrev[0][i];
    quantSpecPrev[0][i] = (short)MIN(MAX(x, -31775), 31775);
  }

  for (i = min(i, maxLines); i < 172; i++) {
    quantSpecPrev[1][i] = quantSpecPrev[0][i] = 0;
  }
}

void decoder_tcx_fac(td_frame_data           *td,
                     int                      frame_index,    /* input: index of the presemt frame to decode*/
                     float                   *A,              /* input: coefficients NxAz[M+1]    */
                     int                      L_frame,        /* input: frame length              */
                     float                   *exc,            /* output: exc[-lg..lg]             */
                     float                   *synth,          /* in/out: synth[-M..lg]            */
                     float                   *synthFB,        /* in/out: synthFB[-LFAC_FB..lgFB]  */
                     HIGF_CONFIG              igfConfig,      /* input: IGF config                */
                     byte                    *group,          /* usac group mem                   */
                     int const                bUsacIndependencyFlag,
                     HANDLE_USAC_TD_DECODER   st              /* i/o :  coder memory state        */
) {
  int i, k, i_subfr, index, lg, lext, mode;
  int *ptr_quant;
  float tmp, gain, gain_tcx, noise_level, ener;
  float *p_A, Ap[M+1];
  float mem_xnq;
  const float *sinewindowPrev, *sinewindowNext;
  int lfacPrev;
  int TTT;
  int lFAC;
  int nfBgn;
  int nfEnd;
  float alfd_gains[N_MAX/(4*8)];
#ifndef UNIFIED_RANDOMSIGN
  float ns[N_MAX];
#endif
  float x[N_MAX], buf[M+L_FRAME_1024];
  float gainlpc[N_MAX], gainlpc2[N_MAX];
  float xn_buf[L_FRAME_1024+(2*LFAC_1024)];
  float *xn;
  float facelp[LFAC_1024];
  float xn1[2*LFAC_1024], facwindow[2*LFAC_1024];
  float xn_bufFB[2*(L_FRAME_1024+(2*LFAC_1024))] = {0.0f};
  float xn1FB[2*2*LFAC_1024]                     = {0.0f};
  int igf_inf_mask[N_MAX]                        = {0};
  int igf_bgn;
  int igf_Cnt;
  IGF_WIN_TYPE igf_win_type;
  IGF_FRAME_ID igf_frm_id;
  int igf_tcx_frm;
  int lFAC_FB;
  int L_frameFB = L_frame;
  int facFB;
  const float *sinewindowPrevFB;
  const float *sinewindowNextFB;

  /* Set variables dependent on frame length */
  /* mode TCX: 1/2/3 = TCX20/40/80 */
  mode = L_frame/(st->lDiv);
  if (mode > 2) {
    mode = 3;
  }

  lFAC   = (st->lDiv)/2;
  if (st->fullbandLPD) {
    facFB = st->fscaleFB/st->fscale;
    lFAC_FB = lFAC*facFB;
    L_frameFB = L_frame*facFB;
  }

  if (st->igf_active) {
    igf_win_type = (IGF_WIN_TYPE)(mode + 1);
    igf_frm_id   = (IGF_TCX_LONG == igf_win_type) ? IGF_ID_TCX_LONG : (IGF_FRAME_ID)(mode + 1 + frame_index);
    igf_tcx_frm  = (IGF_ID_TCX_MEDIUM_SECOND == igf_frm_id) ? 1 : 0;
    igf_bgn      = IGF_get_igfBgn(igfConfig,
                                  igf_win_type,
                                  EIGHT_SHORT_SEQUENCE,
                                  group,
                                  0);
    for (i = 0; i < N_MAX; i++) {
      igf_inf_mask[i] = 1;
    }
  }

  if (st->fullbandLPD) {
    if (st->last_mode == -2 ) {
      lfacPrev = lFAC/4;
    }
    else if (st->last_mode == -1 ) {
      lfacPrev = lFAC/2;
    }
    else {
      lfacPrev = lFAC;
    }
  } 
  else {
    if (st->last_mode == -2 ) {
      lfacPrev = (st->lFrame)/16;
    }
    else {
      lfacPrev = lFAC;
    }
  }

  switch (lfacPrev) {
    case 32:
      sinewindowPrev = (st->window_shape_prev == 1) ? kbdWindow64  : sineWindow64;
      break;
    case 48:
      sinewindowPrev = (st->window_shape_prev == 1) ? kbdWindow96  : sineWindow96;
      break;
    case 64:
      sinewindowPrev = (st->window_shape_prev == 1) ? kbdWindow128 : sineWindow128;
      break;
    case 96:
      sinewindowPrev = (st->window_shape_prev == 1) ? kbdWindow192 : sineWindow192;
      break;
    default:
      sinewindowPrev = (st->window_shape_prev == 1) ? kbdWindow256 : sineWindow256;
  }

  switch (lFAC) {
    case 32:
      sinewindowNext = (st->window_shape == 1) ? kbdWindow64  : sineWindow64;
      break;
    case 48:
      sinewindowNext = (st->window_shape == 1) ? kbdWindow96  : sineWindow96;
      break;
    case 64:
      sinewindowNext = (st->window_shape == 1) ? kbdWindow128 : sineWindow128;
      break;
    case 96:
      sinewindowNext = (st->window_shape == 1) ? kbdWindow192 : sineWindow192;
      break;
    default:
      sinewindowNext = (st->window_shape == 1) ? kbdWindow256 : sineWindow256;
  }


  if (st->fullbandLPD) {
    switch (lfacPrev) {
      case 32:
        sinewindowPrevFB = (st->window_shape_prev == 1) ? kbdWindow128 : sineWindow128;
        break;
      case 48:
        sinewindowPrevFB = (st->window_shape_prev == 1) ? kbdWindow192 : sineWindow192;
        break;
      case 64:
        sinewindowPrevFB = (st->window_shape_prev == 1) ? kbdWindow256 : sineWindow256;
        break;
      case 96:
        sinewindowPrevFB = (st->window_shape_prev == 1) ? kbdWindow384 : sineWindow384;
        break;
      default:
        sinewindowPrevFB = (st->window_shape_prev == 1) ? kbdWindow512 : sineWindow512;
    }
    switch (lFAC) {
      case 32:
        sinewindowNextFB = (st->window_shape == 1) ? kbdWindow128 : sineWindow128;
        break;
      case 48:
        sinewindowNextFB = (st->window_shape == 1) ? kbdWindow192 : sineWindow192;
        break;
      case 64:
        sinewindowNextFB = (st->window_shape == 1) ? kbdWindow256 : sineWindow256;
        break;
      case 96:
        sinewindowNextFB = (st->window_shape == 1) ? kbdWindow384 : sineWindow384;
        break;
      default:
        sinewindowNextFB = (st->window_shape == 1) ? kbdWindow512 : sineWindow512;
    }
  }

  st->window_shape_prev = (WINDOW_SHAPE)st->window_shape;

  lg = L_frame;
  lext = lFAC;
  /* set target pointer (pointer to current frame) */
  xn = xn_buf + lFAC;

  /* window past overlap */
  if (st->last_mode != 0 ) {
    if(st->last_mode>0) {
      for (i=0; i<(2*lfacPrev); i++) {
        st->old_xnq[i+lFAC-lfacPrev+1] *= sinewindowPrev[(2*lfacPrev)-1-i];
      }
      if (st->fullbandLPD) {
        for (i = 0; i < (2 * lfacPrev * facFB); i++) {
          st->old_xnqFB[i+lFAC_FB-lfacPrev*facFB] *= sinewindowNextFB[(2*lfacPrev*facFB)-1-i];
        }
      }
    }
    for (i = 0; i < lFAC-lfacPrev; i++) {
      st->old_xnq[i+lFAC+lfacPrev+1] = 0.0f;
    }
    if (st->fullbandLPD) {
      for (i = 0; i < (lFAC_FB - lfacPrev * facFB); i++) {
        st->old_xnqFB[i+lFAC_FB+lfacPrev*facFB] *= 0.0f;
      }
    }
  }

  /* decode noise level (noise_level) (stored in 2nd packet on TCX80) */
  index = td->tcx_noise_factor[frame_index];
  noise_level = 0.0625f*(8.0f - ((float)index));   /* between 0.5 and 0.0625 */

  /* read index of global TCX gain */
  index =  td->tcx_global_gain[frame_index];

  /* decoded MDCT coefficients */
  ptr_quant = td->tcx_quant;
  for (i = 0; i < frame_index; i++) {
    ptr_quant += td->tcx_lg[i];
  }
  if ((lg * st->fac_FB) != td->tcx_lg[i]) {
    exit(1);
  }
  for (i = 0; i < L_frameFB; i++) {
    x[i] = (float) ptr_quant[i];
  }

#ifndef UNIFIED_RANDOMSIGN
  /* generate random excitation buffer */
  set_zero(ns, lg);
  rnd_sgn16(&(st->seed_tcx), &ns[lg/6], lg-(lg/6)); /*random Sign for MDCT (instead of phase)*/
#endif

  /*----------------------------------------------*
   * noise fill-in on unquantized subvector       *
   * injected only from 1066Hz to 6400Hz.         *
   *----------------------------------------------*/
  if (st->fullbandLPD) {
    int nbw = getNoiseBandwidth(EIGHT_SHORT_SEQUENCE,
                                group,
                                mode,
                                st->max_sfb,
                                L_frameFB,
                                0);
    nfBgn = (L_frameFB/6) & 2040;
    nfEnd = (st->igf_active) ? igf_bgn : L_frameFB;
    nfEnd = min(nfEnd, min(L_frameFB, nbw));
  } else {
    nfBgn = (lg/6);
    nfEnd = lg;
  }
  for (i = nfBgn; i < nfEnd; i += 8) {
    int maxK = min(nfEnd, i+8);
    tmp = 0.0f;

    for (k = i; k < maxK; k++) {
      tmp += x[k] * x[k];
    }

    if (tmp == 0.0f) {
      for (k = i; k < maxK; k++) {
#ifdef UNIFIED_RANDOMSIGN
        x[k] = noise_level * randomSign(&(st->seed_tcx));
#else
        x[k] = noise_level * ns[k];
#endif
        if (st->igf_active) {
          igf_inf_mask[k] = 0;
        }
      }
    }
  }

  ener = 0.01f;
  for (i = 0; i < lg; i++) {
    ener += x[i] * x[i];
  }
  if (mode == (st->fullbandLPD ? 2 : 3)) {
    int quantGainCurr = 0;
    for (i = 0; i < lg; i++) {
      const int xInt = (int)(x[i] * 16.0f);
      quantGainCurr += xInt * xInt;
    }

    tmp = (float)sqrt(ener) * 64.f;
    gain_tcx = tmp / (0.5f * lg * (float)pow(10.0f, (float)index/28.0f));
    quantGainCurr = roundedSquareRoot(41 + 16*(unsigned int)quantGainCurr);
    if (st->fullbandLPD) {
      quantGainCurr = (int)(0.5f + (float)quantGainCurr * 1024.0f / (float)lg);
    }

    quantGainCurr = (pow10gainDiv28[index] + quantGainCurr) / (2*quantGainCurr);

    if (bUsacIndependencyFlag)  {
      /* reset FD prediction memory */
      for (i = 0; i < 172; i++) {
        st->fdp_quantSpecPrev[0][i] = 0;  
        st->fdp_quantSpecPrev[1][i] = 0;  
      }
    }

    decode_fdp(st->fdp_data_present,
               st->fdp_spacing_index,
               st->fdp_int,
               st->fdp_quantSpecPrev,
               x,
               ptr_quant,
               quantGainCurr,
               gain_tcx,
               st->lDiv == 192 ? 120 : 160,
               lg,
               st->lDiv == 192 ? 120 : 160);
  }

  E_LPC_a_weight(A+(M+1), Ap, GAMMA1, M);
  lpc2mdct(Ap, M, gainlpc, (FDNS_NPTS_1024*(st->lDiv))/L_DIV_1024);

  E_LPC_a_weight(A+(2*(M+1)), Ap, GAMMA1, M);
  lpc2mdct(Ap, M, gainlpc2, (FDNS_NPTS_1024*(st->lDiv))/L_DIV_1024);

  AdaptLowFreqDeemph(x,
                     lg,
                     st->fullbandLPD,
                     gainlpc,
                     gainlpc2,
                     alfd_gains);

 /*-----------------------------------------------------------*
  * decode TCX global gain.                                   *
  *-----------------------------------------------------------*/
  if (!st->fullbandLPD) {
    ener = 0.01f;
    for (i = 0; i < lg; i++) {
      ener += x[i] * x[i];
    }
  }

  tmp = 2.0f*(float)sqrt(ener)/lg;
  gain_tcx = (float)pow(10.0f, ((float)index)/28.0f) / tmp;

 /*-----------------------------------------------------------*
  * Intelligent Gap Filling (IGF)                             *
  *-----------------------------------------------------------*/
  if (st->igf_active) {
    const short group_len[MAX_SBK] = {1,0,0,0,0,0,0,0};
    const float noise_level_gain = noise_level * gain_tcx;
    gain = gain_tcx * 0.5f * (float)sqrt(((float)lFAC) / (float)L_frame);
    st->FAC_gain = gain;
    smulFLOAT(gain_tcx, x, x, MIN(L_frameFB, igf_bgn));

    if (!st->igfDecData->igf_bitstream[igf_tcx_frm].igf_allZero)  {
    for (i = 0; i < igf_bgn; i++) {
      for (igf_Cnt = 0; igf_Cnt < igfConfig->igf_grid[igf_win_type].igf_NTiles; igf_Cnt++) {
        if (igfConfig->igf_UseINF && igf_inf_mask[i] == 0.0f) {
          st->igfDecData->igf_infSpec[igf_Cnt * 2048 + i] = noise_level_gain * randomSign(&(st->seed_tcx));
        } else {
          st->igfDecData->igf_infSpec[igf_Cnt * 2048 + i] = x[i];
        }
      }
    }

    IGF_apply_mono(st->igfDecData,
                   igfConfig,
                   igf_win_type,
                   igf_frm_id,
                   x,
                   &st->seed_tcx,
                   EIGHT_SHORT_SEQUENCE,
                   group,
                   1,
                   group_len,
                   &lg,
                   L_frameFB,
                   st->fscaleFB);

    IGF_apply_tnf(st->igfDecData,
                  igfConfig,
                  igf_win_type,
                  igf_frm_id,
                  x,
                  EIGHT_SHORT_SEQUENCE,
                  group);
    }
  }

 /*-----------------------------------------------------------*
  * Noise shaping in frequency domain (1/Wz)                  *
  *-----------------------------------------------------------*/
  mdct_IntNoiseShaping(x, lg, (FDNS_NPTS_1024*(st->lDiv))/L_DIV_1024, gainlpc, gainlpc2);
  mdct_IntNoiseShaping(x + lg, L_frameFB - lg, 1, &gainlpc[(FDNS_NPTS_1024*(st->lDiv))/L_DIV_1024 - 1], &gainlpc2[(FDNS_NPTS_1024*(st->lDiv))/L_DIV_1024 - 1]);

 /*-----------------------------------------------------------*
  * Temporal Noise Shaping (TNS)                              *
  *-----------------------------------------------------------*/
  decode_tns_lpd(EIGHT_SHORT_SEQUENCE,
                 group,
                 &st->tns,
                 frame_index,
                 L_frameFB/st->lDiv,
                 mode,
                 x,
                 st->max_sfb,
                 0);

 /*-----------------------------------------------------------*
  * Compute inverse MDCT of x[].                              *
  *-----------------------------------------------------------*/
  if (st->fullbandLPD) {
    TCX_MDCT_ApplyInverse(st->hTcxMdct, x, xn_bufFB, (2*lFAC_FB), L_frameFB-(2*lFAC_FB), (2*lFAC_FB));

    /*Normalization is done with lg since adjustment is done at encoder.*/
    smulFLOAT((2.0f/lg), xn_bufFB, xn_bufFB, L_frameFB+(2*lFAC_FB));
  }

  TCX_MDCT_ApplyInverse(st->hTcxMdct, x, xn_buf, (2*lFAC), L_frame-(2*lFAC), (2*lFAC));
  smulFLOAT((2.0f/lg), xn_buf, xn_buf, L_frame+(2*lFAC));

  if (!st->igf_active) {
    gain = gain_tcx*0.5f*(float) sqrt(((float)lFAC)/(float)L_frame);
    st->FAC_gain = gain;
  }

  for (i = 0; i < lFAC/4; i++) {
    k = i*lg/(8*lFAC);
    st->FAC_alfd[i] = alfd_gains[k];
  }

 /*-----------------------------------------------------------*
  * Decode FAC in case where previous frame is ACELP.         *
  *-----------------------------------------------------------*/
  if (st->last_mode == 0) {
    /* build FAC window (should be in ROM) */
    for (i=0; i<lfacPrev; i++) {
      facwindow[i] = sinewindowPrev[i]*sinewindowPrev[(2*lfacPrev)-1-i];
      facwindow[lfacPrev+i] = 1.0f - (sinewindowPrev[lfacPrev+i]*sinewindowPrev[lfacPrev+i]);
    }

    /* windowing and folding of ACELP part including the ZIR */
    for (i=0; i<lFAC; i++) {
      facelp[i] = st->old_xnq[1+lFAC+i]*facwindow[lFAC+i]
                + st->old_xnq[lFAC-i]*facwindow[lFAC-1-i];
    }

    /* retrieve MDCT coefficients */
    for(i=0; i<lFAC; i++) {
      x[i] = (float)td->fac[frame_index*LFAC_1024+i];
    }

    /* apply gains */
    for(i=0; i<lFAC; i++) {
      x[i] *= gain;
    }
    for(i=0; i<lFAC/4; i++) {
      x[i] *= st->FAC_alfd[i];
    }

    /* Inverse DCT4 */
    TCX_DCT4_Apply(TCX_MDCT_DCT4_GetHandle(st->hTcxMdct, lFAC), x, xn1);
    smulr2r((2.0f/(float)lFAC), xn1, xn1, lFAC);

    set_zero(xn1+lFAC, lFAC);

    E_LPC_a_weight(A+(M+1), Ap, GAMMA1, M);       /* Wz of old LPC */
    E_UTIL_synthesis(Ap, xn1, xn1, 2*lFAC, xn1+lFAC, 0);

    /* add folded ACELP + ZIR */
    for (i=0; i<lFAC; i++) {
      xn1[i] += facelp[i];
    }

    if (st->fullbandLPD) {
      /* resample FAC+folded ACELP+ZIR for FB */
      set_zero(xn1FB, 2*2*LFAC_1024);
      TD_resampler(xn1, 2*lFAC, st->fscale, xn1FB, st->fscaleFB, NULL, 0);
    }
  }

 /*-----------------------------------------------------------*
  * TCX gain, windowing, overlap and add.                     *
  *-----------------------------------------------------------*/
  if (st->fullbandLPD) {
    /*HB*/
    /*Past overlapping region windowing*/
    if (!st->igf_active) {
      for (i = 0; i < L_frameFB + (2*lFAC_FB); i++) {
        xn_bufFB[i] *= gain_tcx;
      }
    }
    for (i = 0; i < (2 * lfacPrev * facFB); i++) {
      xn_bufFB[i+lFAC_FB - lfacPrev * facFB] *= sinewindowPrevFB[i];
    }
    for (i=0; i<(lFAC_FB - lfacPrev * facFB); i++) {
      xn_bufFB[i] = 0.0f;
    }

    /*Overlap-add*/
    if (st->last_mode != 0) {  /* if previous frame is not ACELP */
      /* overlap-add with previous TCX frame */
      for (i = lFAC_FB - lfacPrev * facFB; i < (lFAC_FB + lfacPrev * facFB); i++) {
        xn_bufFB[i] += st->old_xnqFB[i];
      }
    }
    else {
      /* aliasing cancellation with upsampled FAC */
      for (i=lFAC_FB-lfacPrev*facFB; i<(lFAC_FB+lfacPrev*facFB); i++) {
        xn_bufFB[i+lFAC_FB] += xn1FB[i];
      }
    }

    /* update old_xnq for next frame */
    mvr2r(xn_bufFB+L_frameFB, st->old_xnqFB, (2*lFAC_FB));

    /*Next overlapping region windowing*/
    for (i=0; i<(2*lFAC_FB); i++) {
      xn_bufFB[i+L_frameFB] *= sinewindowNextFB[(2*lFAC_FB)-1-i];
    }

    /*Copy past overlapping region + present frame*/
    mvr2r(xn_bufFB+lFAC_FB-lfacPrev*facFB, synthFB-lfacPrev*facFB, lfacPrev*facFB+L_frameFB);
  }

  /* LB */
  if (!st->igf_active) {
    for (i=0; i<L_frame+(2*lFAC); i++) {
      xn_buf[i] *= gain_tcx;
    }
  }

  for (i=0; i<(2*lfacPrev); i++) {
    xn_buf[i+lFAC-lfacPrev] *= sinewindowPrev[i];
  }
  for ( i = 0 ; i < lFAC-lfacPrev; i++ ) {
    xn_buf[i] = 0.0f;
  }

  if (st->last_mode != 0) {  /* if previous frame is not ACELP */
    /* overlap-add with previous TCX frame */
	for (i=lFAC-lfacPrev; i<(lFAC+lfacPrev); i++) {
      xn_buf[i] += st->old_xnq[1+i];
    }
    mem_xnq = st->old_xnq[0];
  }
  else {
    /* aliasing cancellation with FAC */
	for (i=lFAC-lfacPrev; i<(lFAC+lfacPrev); i++) {
      xn_buf[i+lFAC] += xn1[i];
    }
    mem_xnq = st->old_xnq[lfacPrev];
  }

  /* update old_xnq for next frame */
  mvr2r(xn_buf+L_frame-1, st->old_xnq, 1+(2*lFAC));

  for (i=0; i<(2*lFAC); i++) {
    xn_buf[i+L_frame] *= sinewindowNext[(2*lFAC)-1-i];
  }

 /*-----------------------------------------------------------*
  * Resynthesis of past overlap (FAC method only)             *
  *-----------------------------------------------------------*/
	if (st->last_mode != 0)	{	/* if previous frame is TCX or FD */

      mvr2r(xn_buf+lFAC-lfacPrev, synth-lfacPrev, lfacPrev);

      for(i=0; i<M+lFAC; i++) {
        buf[i] = synth[i-M-lFAC] - (PREEMPH_FAC*synth[i-M-lFAC-1]);
      }

      p_A = st->old_Aq;
      TTT = lFAC%L_SUBFR;
      if (TTT!=0) {
        E_UTIL_residu(p_A, &buf[M], &exc[-lFAC], TTT);
        p_A += (M+1);
      }

      for (i_subfr=TTT; i_subfr<lFAC; i_subfr+=L_SUBFR) {
        E_UTIL_residu(p_A, &buf[M+i_subfr], &exc[i_subfr-lFAC], L_SUBFR);
        p_A += (M+1);
      }
    }

 /*-----------------------------------------------------------*
  * find synthesis and excitation for ACELP frame             *
  *-----------------------------------------------------------*/
  mvr2r(xn, synth, L_frame);
  mvr2r(synth-M-1, xn-M-1, M+1);
  tmp = xn[-M-1];
  E_UTIL_f_preemph(xn-M, PREEMPH_FAC, M+L_frame, &tmp);

  p_A = A+(2*(M+1));

  E_UTIL_residu(p_A, xn, exc, L_frame);

  /* update old_Aq */
  mvr2r(p_A, st->old_Aq, M+1);
  mvr2r(p_A, st->old_Aq+M+1, M+1);

  return;
}



#ifdef UNIFIED_RANDOMSIGN

static float randomSign(unsigned int *seed)
{
  float sign = 0.f;
  *seed = ((*seed) * 69069) + 5;
  if ( ((*seed) & 0x10000) > 0) {
    sign = -1.f;
  } else {
    sign = +1.f;
  }
  return sign;
}

#else

static void rnd_sgn16(short *seed, float *xri, int lg)
{
  int i;

  for (i=0; i<lg; i++)
  {
    /* random phase from 0 to 15 */
    if(E_UTIL_random(seed)>=0)
    {
      xri[i] = 1.f;
    }
    else
    {
      xri[i] = -1.f;
    }
  }
  return;
}

#endif
