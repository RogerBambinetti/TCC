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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "acelp_plus.h"
#include "proto_func.h"
#include "re8.h"
#include "buffers.h"
#include "dec_lpd_stereo.h"

const float sineWindow160[160] = {
  0.00490872f, 0.01472568f, 0.02454123f, 0.03435441f, 0.04416428f, 0.05396989f, 0.06377030f, 0.07356456f,
  0.08335174f, 0.09313088f, 0.10290104f, 0.11266129f, 0.12241068f, 0.13214826f, 0.14187312f, 0.15158430f,
  0.16128086f, 0.17096189f, 0.18062644f, 0.19027357f, 0.19990237f, 0.20951190f, 0.21910124f, 0.22866946f,
  0.23821564f, 0.24773886f, 0.25723821f, 0.26671276f, 0.27616160f, 0.28558383f, 0.29497853f, 0.30434480f,
  0.31368174f, 0.32298845f, 0.33226402f, 0.34150757f, 0.35071820f, 0.35989504f, 0.36903718f, 0.37814376f,
  0.38721389f, 0.39624670f, 0.40524131f, 0.41419687f, 0.42311251f, 0.43198737f, 0.44082059f, 0.44961133f,
  0.45835873f, 0.46706195f, 0.47572016f, 0.48433252f, 0.49289819f, 0.50141636f, 0.50988620f, 0.51830690f,
  0.52667764f, 0.53499762f, 0.54326604f, 0.55148209f, 0.55964499f, 0.56775395f, 0.57580819f, 0.58380693f,
  0.59174941f, 0.59963485f, 0.60746249f, 0.61523159f, 0.62294139f, 0.63059115f, 0.63818013f, 0.64570760f,
  0.65317284f, 0.66057513f, 0.66791374f, 0.67518798f, 0.68239715f, 0.68954054f, 0.69661748f, 0.70362727f,
  0.71056925f, 0.71744274f, 0.72424708f, 0.73098162f, 0.73764570f, 0.74423869f, 0.75075995f, 0.75720885f,
  0.76358476f, 0.76988708f, 0.77611520f, 0.78226851f, 0.78834643f, 0.79434836f, 0.80027373f, 0.80612197f,
  0.81189252f, 0.81758481f, 0.82319831f, 0.82873246f, 0.83418673f, 0.83956061f, 0.84485357f, 0.85006509f,
  0.85519469f, 0.86024186f, 0.86520612f, 0.87008699f, 0.87488400f, 0.87959669f, 0.88422459f, 0.88876728f,
  0.89322430f, 0.89759523f, 0.90187965f, 0.90607715f, 0.91018732f, 0.91420976f, 0.91814408f, 0.92198992f,
  0.92574689f, 0.92941463f, 0.93299280f, 0.93648104f, 0.93987902f, 0.94318642f, 0.94640291f, 0.94952818f,
  0.95256194f, 0.95550388f, 0.95835373f, 0.96111122f, 0.96377607f, 0.96634802f, 0.96882685f, 0.97121229f,
  0.97350412f, 0.97570213f, 0.97780610f, 0.97981582f, 0.98173111f, 0.98355177f, 0.98527764f, 0.98690855f,
  0.98844433f, 0.98988485f, 0.99122996f, 0.99247953f, 0.99363345f, 0.99469160f, 0.99565388f, 0.99652019f,
  0.99729046f, 0.99796460f, 0.99854256f, 0.99902428f, 0.99940971f, 0.99969882f, 0.99989157f, 0.99998795f,
};

const float sineWindow80[80] = {
  0.00981732f, 0.02944817f, 0.04906767f, 0.06866826f, 0.08824237f, 0.10778246f, 0.12728100f, 0.14673047f,
  0.16612338f, 0.18545224f, 0.20470960f, 0.22388805f, 0.24298018f, 0.26197864f, 0.28087610f, 0.29966528f,
  0.31833893f, 0.33688985f, 0.35531090f, 0.37359497f, 0.39173501f, 0.40972403f, 0.42755509f, 0.44522133f,
  0.46271592f, 0.48003212f, 0.49716327f, 0.51410274f, 0.53084403f, 0.54738066f, 0.56370626f, 0.57981455f,
  0.59569930f, 0.61135441f, 0.62677382f, 0.64195160f, 0.65688190f, 0.67155895f, 0.68597711f, 0.70013081f,
  0.71401460f, 0.72762312f, 0.74095113f, 0.75399348f, 0.76674516f, 0.77920124f, 0.79135693f, 0.80320753f,
  0.81474848f, 0.82597533f, 0.83688375f, 0.84746954f, 0.85772861f, 0.86765701f, 0.87725091f, 0.88650662f,
  0.89542056f, 0.90398929f, 0.91220953f, 0.92007808f, 0.92759194f, 0.93474818f, 0.94154407f, 0.94797697f,
  0.95404440f, 0.95974404f, 0.96507367f, 0.97003125f, 0.97461487f, 0.97882275f, 0.98265328f, 0.98610498f,
  0.98917651f, 0.99186670f, 0.99417450f, 0.99609903f, 0.99763955f, 0.99879546f, 0.99956631f, 0.99995181f,
};


/* ISO/IEC 23008-3: Table 104 - Parameter band limits in term of DFT index k */
const short band_limits_erb2[] = {
  1, 3, 5, 7, 9, 13, 17, 21, 25, 33, 41, 49, 57, 73, 89, 105, 137, 177, 241, 337
};

/* ISO/IEC 23008-3: Table 104 - Parameter band limits in term of DFT index k */
const short band_limits_erb4[] = {
  1, 3, 7, 13, 21, 33, 49, 73, 105, 177, 241, 337
};

/* ISO/IEC 23008-3: Table 105 - Maximum number of bands for different code modes */
const short max_band[2][4] = {
  {0, 7, 9, 11},
  {0, 4, 5,  6}
};

/* ISO/IEC 23008-3: Table 106 - Inverse quantization table ild_q[] */
static const short ild_q[] = {
  -50, -45, -40, -35, -30, -25, -22, -19, -16, -13, -10, -8, -6, -4, -2, -0, 2, 4, 6, 8, 10, 13, 16, 19, 22, 25, 30, 35, 40, 45, 50
};

/* ISO/IEC 23008-3: Table 107 - Inverse quantization table res_pred_gain_q[] */
static const float res_pred_gain_q[] = {
  0.0f, 0.1170f, 0.2270f, 0.3407f, 0.4645f, 0.6051f, 0.7763f, 1.0f
};

static float *trigPtrFft = NULL;

static void _lpd_stereo_bass_pf(
                    float                      *syn,
                    const int                  fac_FB,
                    const int                  *T_sf,
                    const float                *gainT_sf,
                    float                      *synth_out,
                    const int                   l_frame,
                    const int                   l_subfr,
                    const int                   l_next,
                    float                      *mem_bpf
) {
  const int delay                                                    = fac_FB * L_FILT + 1 + LTPF_MAX_DELAY;
  int i, j;
  int T, T2, lg, l;
  int sf                                                             = 0;
  int i_subfr                                                        = delay;
  float tmp, ener, corr, gain;
  float noise_buf[(2 * L_FILT + 1) * 2 + LTPF_MAX_DELAY + L_SUBFR*2] = {0.0f};
  float *noise                                                       = noise_buf + fac_FB * L_FILT + 1;
  float *noise_in                                                    = noise_buf + fac_FB * L_FILT + 1 + delay;
  float *x, *y;

  while (i_subfr < l_frame + delay) {
    l = l_subfr - (i_subfr % l_subfr);

    if (i_subfr + l > l_frame + delay) {
      l = l_frame + delay - i_subfr;
    }

    T    = fac_FB * T_sf[sf];
    gain = gainT_sf[sf];

    if (gain > 0.0f) {
      gain = get_gain(&syn[i_subfr],
                      &syn[i_subfr - T],
                      l);
    }

    if (gain > 1.0f) {
      gain = 1.0f;
    }
    if (gain < 0.0f) {
      gain = 0.0f;
    }

    /* pitch tracker: test pitch/2 to avoid continuous pitch doubling */
    /* Note: pitch is limited to PIT_MIN (34 = 376Hz) at the encoder  */
    T2 = T >> 1;
    x  = &syn[i_subfr      - L_EXTRA];
    y  = &syn[i_subfr - T2 - L_EXTRA];

    ener = 0.01f;
    corr = 0.01f;
    tmp  = 0.01f;

    for (i = 0; i < l + L_EXTRA; i++) {
      ener += x[i] * x[i];
      corr += x[i] * y[i];
      tmp  += y[i] * y[i];
    }

    /* use T2 if normalized correlation > 0.95 */
    tmp = corr / (float)sqrt(ener * tmp);

    if (tmp > 0.95f) {
      T = T2;
    }

    lg = l_frame + l_next - T - i_subfr;

    if (lg < 0) {
      lg = 0;
    }
    if (lg > l) {
      lg = l;
    }

    if (gain > 0) {
      /* limit gain to avoid problem on burst */
      if (lg > 0) {
        tmp = 0.01f;

        for (i = 0; i < lg; i++) {
          tmp += syn[i + i_subfr] * syn[i + i_subfr];
        }

        ener = 0.01f;
        for (i = 0; i < lg; i++) {
          ener += syn[i + i_subfr + T] * syn[i + i_subfr + T];
        }

        tmp = (float)sqrt(tmp / ener);
        if (tmp < gain) {
          gain = tmp;
        }
      }

      /* calculate noise based on voiced pitch */
      tmp = gain * 0.5f;
      for (i = 0; i < lg; i++) {
        noise_in[i] = tmp * (syn[i + i_subfr] - 0.5f * syn[i + i_subfr - T] - 0.5f * syn[i + i_subfr + T]);
      }

      for (i = lg; i < l; i++) {
        noise_in[i] = tmp * (syn[i + i_subfr] - syn[i + i_subfr - T]);
      }
    } else {
      set_zero(noise_in,
               l);
    }

    copyFLOAT(mem_bpf,
              noise_buf,
              fac_FB * L_FILT + 1 + delay);

    copyFLOAT(noise_buf + l,
              mem_bpf,
              fac_FB * L_FILT + 1 + delay);

    /* substract from voiced speech low-pass filtered noise */
    if (fac_FB == 2)  {

      for (i = 0; i < l; i++) {
        tmp = filt_lp2[0] * noise[i];

        for (j = 1; j <= 2 * L_FILT + 1; j++) {
          tmp += filt_lp2[j] * (noise[i - j] + noise[i + j]);
        }

        synth_out[i + i_subfr - delay] = syn[i + i_subfr - delay] - tmp;
      }
    } else {

      for (i = 0; i < l; i++) {
        tmp = filt_lp[0] * noise[i];

        for (j = 1; j <= L_FILT; j++) {
          tmp += filt_lp[j] * (noise[i - j] + noise[i + j]);
        }

        synth_out[i + i_subfr - delay] = syn[i + i_subfr - delay] - tmp;
      }
    } 
    i_subfr += l;
    sf++;
  }

  /* Filter LTPF_MAX_DELAY samples in the future for LTPF in TCX */
  if (fac_FB == 2)  {

    for (i = 0; i < LTPF_MAX_DELAY; i++) {
      tmp = filt_lp[0] * noise[l + i];

      for (j = 1; j <= L_FILT; j++) {
        tmp += filt_lp[j] * (noise[l + i - j] + noise[l + i + j]);
      }

      synth_out[i + i_subfr - delay] = syn[i + i_subfr - delay] - tmp;
    }
  } else {

    for (i = 0; i < LTPF_MAX_DELAY; i++) {
      tmp = filt_lp2[0] * noise[l + i];

      for (j = 1; j <= 2 * L_FILT + 1; j++) {
        tmp += filt_lp2[j] * (noise[l + i - j] + noise[l + i + j]);
      }

      synth_out[i + i_subfr - delay] = syn[i + i_subfr - delay] - tmp;
    }
  }
}

static int _lpd_stereo_band_config(
                    int                        *band_limits,
                    const STEREOLPD_RES_MODE    res_mode,
                    const int                   N
) {
  int nbands = 0;
 
  band_limits[0] = 1;
  while (band_limits[nbands++] < (N / 2)) {
    if (STEREOLPD_ERB_2 == res_mode){
      band_limits[nbands] = (int)band_limits_erb2[nbands];
    } else {
      band_limits[nbands] = (int)band_limits_erb4[nbands];
    }
  }

  nbands--;
  band_limits[nbands] = N / 2;

  return nbands;
}

static void _lpd_stereo_dequantize_ild(
                    float                      *ILD,
                    const int                  *ild_idx,
                    const int                   nbands
) {
  int b = 0;

  for (b = 0; b < nbands; b++) {
    ILD[b] = (float)ild_q[ild_idx[b]];
  }
}


static void _lpd_stereo_dequantize_ipd(
                    float                      *IPD,
                    const int                  *ipd_idx,
                    const int                   ipd_max_band
) {
  int b       = 0;
  float delta = (float)PI / 4.0f;

  for (b = 0; b < ipd_max_band; b++) {
    IPD[b] = ipd_idx[b] * delta - (float)PI;
  }

}

static void _lpd_stereo_dequantize_res_pred_gain(
                    float                      *pred_gain,
                    const int                  *pred_gain_idx,
                    const int                   nbands
) {
  int b = 0;

  for (b = 0; b < nbands; b++) {
    pred_gain[b] = res_pred_gain_q[pred_gain_idx[b]];
  }
}

static float _lpd_stereo_dequantize_gain(
                    const int                   index,
                    const int                   N
) {
  float global_gain = 0.0f;

  global_gain = (float)pow(10.0f, ((float)index) / (20.0f * 127.0f / 90.0f)) * (float)N;

  return global_gain;
}

static void _lpd_stereo_code_book_indices(
                    float                      *S,
                    HANDLE_RESILIENCE           hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER               hVm
) {
  int i       = 0;
  int n       = 0;
  int qn      = 0;
  int nk      = 0;
  int kv[8]   = {0};
  int prm[8]  = {0};
  long I      = 0;

  /* Unary code for codebook numbers */
  while (GetBits(1,
                  LPD_STEREO_DATA,
                  hResilience,
                  hEscInstanceData,
                  hVm) == 1) {
    qn += 1;
  }

  if (qn != 0) {
    qn += 1;
  }

  n = qn;
  if (qn > 4) {
    nk = (qn - 3) >> 1;
    n = qn - nk * 2;
  }

  /* read 4*n bits for base codebook index (I) */
  I = GetBits(4 * n,
              LPD_STEREO_DATA,
              hResilience,
              hEscInstanceData,
              hVm);

  /* Read 8*nk bits for Voronoi indices (kv[0...7]) */
  for (i = 0; i < 8; i++) {
    kv[i] = (int)GetBits(nk,
                          LPD_STEREO_DATA,
                          hResilience,
                          hEscInstanceData,
                          hVm);
  }

  /* ISO/IEC 23003-3:2012 - AVQ */
  RE8_dec(qn, I, kv, prm);

  /* store output */
  for (i = 0; i < 8; i++) {
    S[i] = (float)prm[i];
  }
}

static void _lpd_stereo_stream(
                    const HSTEREOLPD_CONFIG     lpdstereo_config,
                    HSTEREOLPD_BITSTREAM        lpdstereo_bitstream,
                    HSTEREOLPD_PARAMETER        lpdstereo_parameter,
                    HSTEREOLPD_HELPER           lpdstereo_helper,
                    HANDLE_RESILIENCE           hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER               hVm
) {
  int k    = 0;
  int b    = 0;
  int i    = 0;
  int nDiv = lpdstereo_config->ccfl / lpdstereo_config->frameSize_M;

  /* reset LPD stereo bitstream struct */
  memset(lpdstereo_bitstream, 0, sizeof(STEREOLPD_BITSTREAM));

  /* read LPD Stereo configuration */
  lpdstereo_bitstream->res_mode  = (STEREOLPD_RES_MODE)GetBits(1, LPD_STEREO_DATA, hResilience, hEscInstanceData, hVm);
  lpdstereo_bitstream->q_mode    =                (int)GetBits(1, LPD_STEREO_DATA, hResilience, hEscInstanceData, hVm);
  lpdstereo_bitstream->ipd_mode  =                (int)GetBits(2, LPD_STEREO_DATA, hResilience, hEscInstanceData, hVm);
  lpdstereo_bitstream->pred_mode =                (int)GetBits(1, LPD_STEREO_DATA, hResilience, hEscInstanceData, hVm);
  lpdstereo_bitstream->cod_mode  =                (int)GetBits(2, LPD_STEREO_DATA, hResilience, hEscInstanceData, hVm);

  /* set helper variables */
  lpdstereo_helper->nbands        = _lpd_stereo_band_config(lpdstereo_helper->band_limits,
                                                            lpdstereo_bitstream->res_mode,
                                                            lpdstereo_config->dftSize_N);
  lpdstereo_helper->ipd_band_max  = max_band[lpdstereo_bitstream->res_mode][lpdstereo_bitstream->ipd_mode];
  lpdstereo_helper->cod_band_max  = max_band[lpdstereo_bitstream->res_mode][lpdstereo_bitstream->cod_mode];
  lpdstereo_helper->cod_L         = 2 * (lpdstereo_helper->band_limits[lpdstereo_helper->cod_band_max] - 1);

  /* read LPD Stereo frame data */
  for (k = nDiv - 1; k >= 0; k--) {
    if ((lpdstereo_bitstream->q_mode == 0) || (k % 2 == 1)) {
      for (b = 0; b < lpdstereo_helper->nbands; b++) {
        lpdstereo_bitstream->ild_idx[k][b] = (int)GetBits(5,
                                                          LPD_STEREO_DATA,
                                                          hResilience,
                                                          hEscInstanceData,
                                                          hVm);
      }

      for (b = 0; b < lpdstereo_helper->ipd_band_max; b++) {
        lpdstereo_bitstream->ipd_idx[k][b] = (int)GetBits(3,
                                                          LPD_STEREO_DATA,
                                                          hResilience,
                                                          hEscInstanceData,
                                                          hVm);
      }

      if (1 == lpdstereo_bitstream->pred_mode) {
        for (b = lpdstereo_helper->cod_band_max; b < lpdstereo_helper->nbands; b++) {
          lpdstereo_bitstream->pred_gain_idx[k][b] = (int)GetBits(3,
                                                                  LPD_STEREO_DATA,
                                                                  hResilience,
                                                                  hEscInstanceData,
                                                                  hVm);
        }
      }
    }

    if (lpdstereo_bitstream->cod_mode > 0) {
      lpdstereo_bitstream->cod_gain_idx[k] = (int)GetBits(7,
                                                          LPD_STEREO_DATA,
                                                          hResilience,
                                                          hEscInstanceData,
                                                          hVm);

      for (i = 0; i < (lpdstereo_helper->cod_L >> 3); i++) {
        _lpd_stereo_code_book_indices(&lpdstereo_parameter->S[k][2 + 8 * i],
                                      hResilience,
                                      hEscInstanceData,
                                      hVm);
      }
    }
  }
}

static void _lpd_stereo_inverse_quantization(
                    const HSTEREOLPD_CONFIG     lpdstereo_config,
                    const HSTEREOLPD_BITSTREAM  lpdstereo_bitstream,
                    HSTEREOLPD_PARAMETER        lpdstereo_parameter,
                    HSTEREOLPD_HELPER           lpdstereo_helper
) {
  int k     = 0;
  int N_div = lpdstereo_config->ccfl / lpdstereo_config->frameSize_M;

  /* reset LPD stereo parameter struct except residual signal S (already dequantized in _lpd_stereo_stream()) */
  memset(lpdstereo_parameter->ild,       0, sizeof(lpdstereo_parameter->ild));
  memset(lpdstereo_parameter->ipd,       0, sizeof(lpdstereo_parameter->ipd));
  memset(lpdstereo_parameter->pred_gain, 0, sizeof(lpdstereo_parameter->pred_gain));
  memset(lpdstereo_parameter->cod_gain,  0, sizeof(lpdstereo_parameter->cod_gain));

  for (k = N_div - 1; k >= 0; k--) {
    lpdstereo_parameter->cod_gain[k] = 1.f;

    if ((lpdstereo_bitstream->q_mode == 0) || (k % 2 == 1)) {
      _lpd_stereo_dequantize_ild(&lpdstereo_parameter->ild[k][0],
                                 &lpdstereo_bitstream->ild_idx[k][0],
                                 lpdstereo_helper->nbands);

      _lpd_stereo_dequantize_ipd(&lpdstereo_parameter->ipd[k][0],
                                 &lpdstereo_bitstream->ipd_idx[k][0],
                                 lpdstereo_helper->ipd_band_max);

      if (lpdstereo_bitstream->pred_mode) {
        _lpd_stereo_dequantize_res_pred_gain(&lpdstereo_parameter->pred_gain[k][0],
                                             &lpdstereo_bitstream->pred_gain_idx[k][0],
                                             lpdstereo_helper->nbands);
      }
    } else {
      copyFLOAT(&lpdstereo_parameter->ild[k + 1][0],
                &lpdstereo_parameter->ild[k][0],
                STEREO_LPD_BAND_MAX);

      copyFLOAT(&lpdstereo_parameter->ipd[k + 1][0],
                &lpdstereo_parameter->ipd[k][0],
                STEREO_LPD_BAND_MAX);

      copyFLOAT(&lpdstereo_parameter->pred_gain[k + 1][0],
                &lpdstereo_parameter->pred_gain[k][0],
                STEREO_LPD_BAND_MAX);
    }

    if (lpdstereo_bitstream->cod_mode > 0) {
      lpdstereo_parameter->cod_gain[k] = _lpd_stereo_dequantize_gain(lpdstereo_bitstream->cod_gain_idx[k],
                                                                     lpdstereo_config->frameSize_M);
    }
  }
}

void stereoLpd_init(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const int                   fs,
                    const int                   fullbandLPD,
                    const int                   ccfl
) {
  /* init configuration */
  stereolpdDecData->lpdstereo_config.fs   = fs;
  stereolpdDecData->lpdstereo_config.ccfl = ccfl;
  stereolpdDecData->fac_FB                = (1 == fullbandLPD) ? 2 : 1;
  stereolpdDecData->win                   = (1 == fullbandLPD) ? sineWindow160 : sineWindow80;

  /* ISO/IEC 23008-3: Table 103 - DFT and frame sizes of the stereo LPD */
  switch (ccfl) {
    case 768:
      stereolpdDecData->lpdstereo_config.dftSize_N     = (1 == fullbandLPD) ? 512 : 256;
      stereolpdDecData->lpdstereo_config.frameSize_M   = (1 == fullbandLPD) ? 384 : 192;
      stereolpdDecData->lpdstereo_config.overlapSize_L = (1 == fullbandLPD) ? 128 :  64;
      break;
    default:
      stereolpdDecData->lpdstereo_config.dftSize_N     = (1 == fullbandLPD) ? 672 : 336;
      stereolpdDecData->lpdstereo_config.frameSize_M   = (1 == fullbandLPD) ? 512 : 256;
      stereolpdDecData->lpdstereo_config.overlapSize_L = (1 == fullbandLPD) ? 160 :  80;
  }

  /* reset LPD stereo bitstream struct */
  memset(&stereolpdDecData->lpdstereo_bitstream, 0, sizeof(stereolpdDecData->lpdstereo_bitstream));

  /* reset LPD stereo parameter struct */
  memset(&stereolpdDecData->lpdstereo_parameter, 0, sizeof(stereolpdDecData->lpdstereo_parameter));

  /* Bands: find the number of bands, Nyquist freq. is not taken into account */
  stereolpdDecData->lpdstereo_helper.nbands       = _lpd_stereo_band_config(stereolpdDecData->lpdstereo_helper.band_limits,
                                                                            stereolpdDecData->lpdstereo_bitstream.res_mode,
                                                                            stereolpdDecData->lpdstereo_config.dftSize_N);
  stereolpdDecData->lpdstereo_helper.ipd_band_max = max_band[stereolpdDecData->lpdstereo_bitstream.res_mode][stereolpdDecData->lpdstereo_bitstream.ipd_mode];
  stereolpdDecData->lpdstereo_helper.cod_band_max = max_band[stereolpdDecData->lpdstereo_bitstream.res_mode][stereolpdDecData->lpdstereo_bitstream.cod_mode];
  stereolpdDecData->lpdstereo_helper.cod_L        = 2 * (stereolpdDecData->lpdstereo_helper.band_limits[stereolpdDecData->lpdstereo_helper.cod_band_max] - 1);

  /* set all other buffers to zero */
  memset(stereolpdDecData->bpf_pitch,      0, sizeof(stereolpdDecData->bpf_pitch));
  memset(stereolpdDecData->DFT_past_DMX,   0, sizeof(stereolpdDecData->DFT_past_DMX));
  memset(stereolpdDecData->TimeBuffer_DMX, 0, sizeof(stereolpdDecData->TimeBuffer_DMX));
  memset(stereolpdDecData->bpf_gain,       0, sizeof(stereolpdDecData->bpf_gain));
  memset(stereolpdDecData->past_L,         0, sizeof(stereolpdDecData->past_L));
  memset(stereolpdDecData->past_R,         0, sizeof(stereolpdDecData->past_R));
  memset(stereolpdDecData->old_noise_pf_L, 0, sizeof(stereolpdDecData->old_noise_pf_L));
  memset(stereolpdDecData->old_noise_pf_R, 0, sizeof(stereolpdDecData->old_noise_pf_R));

  /* init DFT */
  trigPtrFft = CreateSineTable(stereolpdDecData->lpdstereo_config.dftSize_N);
}

void stereoLpd_setDMX(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth
) {
  copyFLOAT(synth,
            stereolpdDecData->TimeBuffer_DMX,
            stereolpdDecData->lpdstereo_config.ccfl + stereolpdDecData->lpdstereo_config.ccfl / 2);
}

void stereoLpd_setPastL(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth,
                    const int                   length
) {
  set_zero(stereolpdDecData->past_L,
           2 * PIT_MAX_MAX);

  copyFLOAT(synth + 1,
            stereolpdDecData->past_L + 2 * PIT_MAX_MAX,
            length);

  set_zero(stereolpdDecData->past_L + 2 * PIT_MAX_MAX + length,
           stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024 - length);
}

void stereoLpd_setPastR(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const float                *synth,
                    const int                   length
) {
  set_zero(stereolpdDecData->past_R,
           2 * PIT_MAX_MAX);

  copyFLOAT(synth + 1,
            stereolpdDecData->past_R + 2 * PIT_MAX_MAX,
            length);

  set_zero(stereolpdDecData->past_R + 2 * PIT_MAX_MAX + length,
           stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024 - length);
}

void stereoLpd_setBPFprm(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    const int                  *pitch,
                    const float                *gain,
                    const int                   nbSubfrSuperfr
) {
  int n = 0;

  for (n = 0; n < BPF_SFD; n++){
    stereolpdDecData->bpf_pitch[n] = stereolpdDecData->bpf_pitch[n + nbSubfrSuperfr];
    stereolpdDecData->bpf_gain[n]  = stereolpdDecData->bpf_gain[n + nbSubfrSuperfr];
  }

  for (n = 0; n < nbSubfrSuperfr; n++){
    stereolpdDecData->bpf_pitch[n + BPF_SFD] = pitch[n];
    stereolpdDecData->bpf_gain[n + BPF_SFD]  = gain[n];
  }
}

void stereoLpd_data(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    HANDLE_RESILIENCE           hResilience,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_BUFFER               hVm,
                    const int                   aceStartFlag
) {
  if (aceStartFlag) {
    memset(stereolpdDecData->DFT_past_DMX,  0, sizeof(stereolpdDecData->DFT_past_DMX));
  }

  /* read LPD Stereo bitstream data */
  _lpd_stereo_stream(&stereolpdDecData->lpdstereo_config,
                     &stereolpdDecData->lpdstereo_bitstream,
                     &stereolpdDecData->lpdstereo_parameter,
                     &stereolpdDecData->lpdstereo_helper,
                     hResilience,
                     hEscInstanceData,
                     hVm);

  /* de-quantize LPD Stereo bitstream data */
  _lpd_stereo_inverse_quantization(&stereolpdDecData->lpdstereo_config,
                                   &stereolpdDecData->lpdstereo_bitstream,
                                   &stereolpdDecData->lpdstereo_parameter,
                                   &stereolpdDecData->lpdstereo_helper);
}

void stereoLpd_apply(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *synth_L,
                    float                      *synth_R,
                    const int                   aceStartFlag
) {
  int k                                                                             = 0;
  int i                                                                             = 0;
  int b                                                                             = 0;
  int offset                                                                        = 0;
  int offset_dmx                                                                    = 0;
  int fac_FB                                                                        = stereolpdDecData->fac_FB;
  const int past                                                                    = fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl / 2 - 256;
  const int delay                                                                   = stereolpdDecData->lpdstereo_config.overlapSize_L;
  float TimeBuffer_L[2 * PIT_MAX_MAX + L_FRAME_1024 / 2 + L_FRAME_1024 + LFAC_1024] = {0.0f};
  float TimeBuffer_R[2 * PIT_MAX_MAX + L_FRAME_1024 / 2 + L_FRAME_1024 + LFAC_1024] = {0.0f};
  float outputBuffer[L_FRAME_1024 + LTPF_MAX_DELAY]                                 = {0.0f};
  float DFT_L[STEREO_LPD_DFT_SIZE]                                                  = {0.0f};
  float DFT_R[STEREO_LPD_DFT_SIZE]                                                  = {0.0f};
  float DFT_DMX[STEREO_LPD_DFT_SIZE]                                                = {0.0f};
  float pTimeBuf[STEREO_LPD_DFT_SIZE]                                               = {0.0f};
  float alpha                                                                       = 0.0f;
  float beta                                                                        = 0.0f;
  float g, c, g2, nrg_L, nrg_R, sum_real, sum_img, sum_abs;
  float *pIld, *pIpd, *pPredGain, *pDFT_RES;

  if (aceStartFlag) {
    copyFLOAT(stereolpdDecData->past_L,
              TimeBuffer_L,
              fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024);

    set_zero(TimeBuffer_L + fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024,
             L_FRAME_1024);

    copyFLOAT(stereolpdDecData->past_R,
              TimeBuffer_R,
              fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024);

    set_zero(TimeBuffer_R + fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024,
             L_FRAME_1024);

    for (i = 0; i < BPF_SFD; i++) {
      stereolpdDecData->bpf_pitch[i] = 0;
    }

    set_zero(stereolpdDecData->bpf_gain,
             BPF_SFD);

    set_zero(stereolpdDecData->old_noise_pf_L,
             (2 * L_FILT + 1) * 2 + LTPF_MAX_DELAY);

    set_zero(stereolpdDecData->old_noise_pf_R,
             (2 * L_FILT + 1) * 2 + LTPF_MAX_DELAY);
  } else {
    copyFLOAT(stereolpdDecData->past_L,
              TimeBuffer_L,
              past);

    copyFLOAT(stereolpdDecData->past_R,
              TimeBuffer_R,
              past);
  }

  for (k = 0; k < stereolpdDecData->lpdstereo_config.ccfl / stereolpdDecData->lpdstereo_config.frameSize_M; k++) {
    offset     = past + k * stereolpdDecData->lpdstereo_config.frameSize_M - delay;
    offset_dmx = offset - fac_FB * PIT_MAX_MAX;

    copyFLOAT(&stereolpdDecData->TimeBuffer_DMX[offset_dmx],
              pTimeBuf,
              stereolpdDecData->lpdstereo_config.dftSize_N);

    for (i = 0; i < delay; i++) {
      pTimeBuf[i]                                                *= stereolpdDecData->win[i];
      pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N-1-i] *= stereolpdDecData->win[i];
    }

    copyFLOAT(pTimeBuf,
              DFT_DMX,
              stereolpdDecData->lpdstereo_config.dftSize_N);

    RFFTN(DFT_DMX,
          trigPtrFft,
          stereolpdDecData->lpdstereo_config.dftSize_N,
          -1);

    pIld      = &stereolpdDecData->lpdstereo_parameter.ild[k][0];
    pIpd      = &stereolpdDecData->lpdstereo_parameter.ipd[k][0];
    pPredGain = &stereolpdDecData->lpdstereo_parameter.pred_gain[k][0];
    pDFT_RES  = &stereolpdDecData->lpdstereo_parameter.S[k][0];

    DFT_L[0] = DFT_DMX[0];
    DFT_R[0] = DFT_DMX[0];

    for (b = 0; b < stereolpdDecData->lpdstereo_helper.nbands; b++) {
      c     = (float)pow(10.0, pIld[b]/20.0);
      g     = ((c - 1) / (c + 1));
      nrg_L = STEREO_LPD_FLT_MIN;
      nrg_R = STEREO_LPD_FLT_MIN;

      for (i = stereolpdDecData->lpdstereo_helper.band_limits[b]; i < stereolpdDecData->lpdstereo_helper.band_limits[b+1]; i++) {
        g2               = pPredGain[b];
        DFT_L[2 * i]     = DFT_DMX[2 * i]     + g * DFT_DMX[2 * i]     + g2 * stereolpdDecData->DFT_past_DMX[2 * i];
        DFT_L[2 * i + 1] = DFT_DMX[2 * i + 1] + g * DFT_DMX[2 * i + 1] + g2 * stereolpdDecData->DFT_past_DMX[2 * i + 1];
        DFT_R[2 * i]     = DFT_DMX[2 * i]     - g * DFT_DMX[2 * i]     - g2 * stereolpdDecData->DFT_past_DMX[2 * i];
        DFT_R[2 * i + 1] = DFT_DMX[2 * i + 1] - g * DFT_DMX[2 * i + 1] - g2 * stereolpdDecData->DFT_past_DMX[2 * i + 1];

        if (b < stereolpdDecData->lpdstereo_helper.cod_band_max) {
            DFT_L[2 * i]     += stereolpdDecData->lpdstereo_parameter.cod_gain[k] * pDFT_RES[2 * i];
            DFT_L[2 * i + 1] += stereolpdDecData->lpdstereo_parameter.cod_gain[k] * pDFT_RES[2 * i + 1];
            DFT_R[2 * i]     -= stereolpdDecData->lpdstereo_parameter.cod_gain[k] * pDFT_RES[2 * i];
            DFT_R[2 * i + 1] -= stereolpdDecData->lpdstereo_parameter.cod_gain[k] * pDFT_RES[2 * i + 1];
        } else {
          nrg_L += DFT_L[2 * i] * DFT_L[2 * i] + DFT_L[2 * i + 1] * DFT_L[2 * i + 1];
          nrg_R += DFT_R[2 * i] * DFT_R[2 * i] + DFT_R[2 * i + 1] * DFT_R[2 * i + 1];
        }
      }

      if (b < stereolpdDecData->lpdstereo_helper.ipd_band_max) {
        alpha = pIpd[b];
        beta  = (float)(atan2(sin(alpha), (cos(alpha) + c)));
      }

      for (i = stereolpdDecData->lpdstereo_helper.band_limits[b]; i < stereolpdDecData->lpdstereo_helper.band_limits[b + 1]; i++) {
        nrg_L             = DFT_L[2 * i] * DFT_L[2 * i] + DFT_L[2 * i + 1] * DFT_L[2 * i + 1];
        nrg_R             = DFT_R[2 * i] * DFT_R[2 * i] + DFT_R[2 * i + 1] * DFT_R[2 * i + 1] + STEREO_LPD_FLT_MIN;
        sum_real          = DFT_DMX[2 * i];
        sum_img           = DFT_DMX[2 * i + 1];
        sum_abs           = sum_real * sum_real + sum_img * sum_img + STEREO_LPD_FLT_MIN;
        c                 = (float)sqrt(2.0 * sum_abs / (nrg_L + nrg_R));
        c                 = MAX(MIN(c, STERO_LPD_DMX_LIMIT), 1.0f / STERO_LPD_DMX_LIMIT);
        DFT_L[2 * i]     *=c;
        DFT_L[2 * i + 1] *=c;
        DFT_R[2 * i]     *=c;
        DFT_R[2 * i + 1] *=c;

        if (b < stereolpdDecData->lpdstereo_helper.ipd_band_max) {
          sum_real         = DFT_L[2 * i] * (float)cos(beta) - DFT_L[2 * i + 1] * (float)sin(beta);
          DFT_L[2 * i + 1] = DFT_L[2 * i] * (float)sin(beta) + DFT_L[2 * i + 1] * (float)cos(beta);
          DFT_L[2 * i]     = sum_real;
          sum_real         = DFT_R[2 * i] * (float)cos(-alpha + beta) - DFT_R[2 * i + 1] * (float)sin(-alpha + beta);
          DFT_R[2 * i + 1] = DFT_R[2 * i] * (float)sin(-alpha + beta) + DFT_R[2 * i + 1] * (float)cos(-alpha + beta);
          DFT_R[2 * i]     = sum_real;
        }
      }
    }

    DFT_L[1]  = DFT_DMX[1] + g * DFT_DMX[1];
    DFT_R[1]  = DFT_DMX[1] - g * DFT_DMX[1];
    nrg_L     = DFT_L[1] * DFT_L[1];
    nrg_R     = DFT_R[1] * DFT_R[1] + STEREO_LPD_FLT_MIN;
    sum_real  = DFT_DMX[1];
    sum_abs   = sum_real * sum_real + STEREO_LPD_FLT_MIN;
    c         = (float)sqrt(2.0 * sum_abs / (nrg_L + nrg_R));
    c         = MAX(MIN(c, STERO_LPD_DMX_LIMIT), 1.0f / STERO_LPD_DMX_LIMIT);
    DFT_L[1] *=c;
    DFT_R[1] *=c;

    copyFLOAT(DFT_DMX,
              stereolpdDecData->DFT_past_DMX,
              stereolpdDecData->lpdstereo_config.dftSize_N);
    copyFLOAT(DFT_L,
              pTimeBuf,
              stereolpdDecData->lpdstereo_config.dftSize_N);

    RFFTN(pTimeBuf,
          trigPtrFft,
          stereolpdDecData->lpdstereo_config.dftSize_N,
          1);

    if (k == 0 && aceStartFlag) {
      for (i = 0; i < delay; i++) {
        TimeBuffer_L[offset + i] = TimeBuffer_L[offset + i] * stereolpdDecData->win[delay - 1 - i] * stereolpdDecData->win[delay - 1 - i] + pTimeBuf[i] * stereolpdDecData->win[i];
        TimeBuffer_L[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
      }

      for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++) {
        TimeBuffer_L[offset + delay + i] = pTimeBuf[i + delay];
      }
    } else {
      for (i = 0; i < delay; i++) {
        TimeBuffer_L[offset + i] += pTimeBuf[i] * stereolpdDecData->win[i];
        TimeBuffer_L[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
      }

      for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++) {
        TimeBuffer_L[offset + delay + i] = pTimeBuf[i + delay];
      }
    }

    copyFLOAT(DFT_R,
              pTimeBuf,
              stereolpdDecData->lpdstereo_config.dftSize_N);

    RFFTN(pTimeBuf,
          trigPtrFft,
          stereolpdDecData->lpdstereo_config.dftSize_N,
          1);

    if (k == 0 && aceStartFlag) {
      for (i = 0; i < delay; i++) {
        TimeBuffer_R[offset + i] = TimeBuffer_R[offset + i] * stereolpdDecData->win[delay - 1 - i] * stereolpdDecData->win[delay - 1 - i] + pTimeBuf[i] * stereolpdDecData->win[i];
        TimeBuffer_R[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
      }

      for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++) {
        TimeBuffer_R[offset + delay + i] = pTimeBuf[i + delay];
      }
    } else {
      for (i = 0; i < delay; i++) {
        TimeBuffer_R[offset + i] += pTimeBuf[i] * stereolpdDecData->win[i];
        TimeBuffer_R[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
      }

      for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++ ) {
        TimeBuffer_R[offset + delay + i] = pTimeBuf[i + delay];
      }
    }
  }

  copyFLOAT(TimeBuffer_L + stereolpdDecData->lpdstereo_config.ccfl,
            stereolpdDecData->past_L,
            past);

  copyFLOAT(TimeBuffer_R + stereolpdDecData->lpdstereo_config.ccfl,
            stereolpdDecData->past_R,
            past);

  _lpd_stereo_bass_pf(TimeBuffer_L + fac_FB * PIT_MAX_MAX,
                      fac_FB,
                      stereolpdDecData->bpf_pitch,
                      stereolpdDecData->bpf_gain,
                      outputBuffer,
                      stereolpdDecData->lpdstereo_config.ccfl,
                      fac_FB * L_SUBFR,
                      past - fac_FB * PIT_MAX_MAX - delay,
                      stereolpdDecData->old_noise_pf_L);

  copyFLOAT(outputBuffer,
            synth_L,
            stereolpdDecData->lpdstereo_config.ccfl);

  _lpd_stereo_bass_pf(TimeBuffer_R + fac_FB * PIT_MAX_MAX,
                      fac_FB,
                      stereolpdDecData->bpf_pitch,
                      stereolpdDecData->bpf_gain,
                      outputBuffer,
                      stereolpdDecData->lpdstereo_config.ccfl,
                      fac_FB * L_SUBFR,
                      past - fac_FB * PIT_MAX_MAX - delay,
                      stereolpdDecData->old_noise_pf_R);

  copyFLOAT(outputBuffer,
            synth_R,
            stereolpdDecData->lpdstereo_config.ccfl);
}

void stereoLpd_applyEnd(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *synth_L,
                    float                      *synth_R
) {
  int k                                                                             = 0;
  int i                                                                             = 0;
  int b                                                                             = 0;
  int offset                                                                        = 0;
  const int past                                                                    = stereolpdDecData->lpdstereo_config.ccfl / 2 - 256;
  const int delay                                                                   = stereolpdDecData->lpdstereo_config.overlapSize_L;
  float TimeBuffer_L[L_FRAME_1024 / 2 + L_FRAME_1024 + LFAC_1024]                   = {0.0f};
  float TimeBuffer_R[L_FRAME_1024 / 2 + L_FRAME_1024 + LFAC_1024]                   = {0.0f};
  float pDFT_L[STEREO_LPD_DFT_SIZE]                                                 = {0.0f};
  float pDFT_R[STEREO_LPD_DFT_SIZE]                                                 = {0.0f};
  float pDFT_DMX[STEREO_LPD_DFT_SIZE]                                               = {0.0f};
  float pTimeBuf[STEREO_LPD_DFT_SIZE]                                               = {0.0f};
  float alpha                                                                       = 0.0f;
  float beta                                                                        = 0.0f;
  float g, c, nrg_L, nrg_R, sum_real, sum_img, sum_abs;
  float *pIld, *pIpd;

  copyFLOAT(synth_L + 1,
            stereolpdDecData->TimeBuffer_DMX + stereolpdDecData->lpdstereo_config.ccfl,
            stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024);

  set_zero(stereolpdDecData->TimeBuffer_DMX + stereolpdDecData->lpdstereo_config.ccfl + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024,
           stereolpdDecData->lpdstereo_config.ccfl / 2 - LFAC_1024);

  k      = stereolpdDecData->lpdstereo_config.ccfl / stereolpdDecData->lpdstereo_config.frameSize_M;
  offset = past + k * stereolpdDecData->lpdstereo_config.frameSize_M - delay;
  k      = stereolpdDecData->lpdstereo_config.ccfl / stereolpdDecData->lpdstereo_config.frameSize_M - 1;

  copyFLOAT(stereolpdDecData->TimeBuffer_DMX + offset,
            pTimeBuf,
            stereolpdDecData->lpdstereo_config.dftSize_N);

  for (i = 0; i < delay; i++) {
    pTimeBuf[i]                                                    *= stereolpdDecData->win[i];
    pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] *= stereolpdDecData->win[i];
  }

  copyFLOAT(pTimeBuf,
            pDFT_DMX,
            stereolpdDecData->lpdstereo_config.dftSize_N);

  RFFTN(pDFT_DMX,
        trigPtrFft,
        stereolpdDecData->lpdstereo_config.dftSize_N,
        -1);

  pIld = &stereolpdDecData->lpdstereo_parameter.ild[k][0];
  pIpd = &stereolpdDecData->lpdstereo_parameter.ipd[k][0];

  pDFT_L[0] = pDFT_DMX[0];
  pDFT_R[0] = pDFT_DMX[0];

  for (b = 0; b < stereolpdDecData->lpdstereo_helper.nbands; b++) {
    c = (float)pow(10.0, pIld[b] / 20.0);
    g = ((c - 1) / (c + 1));

    for (i = stereolpdDecData->lpdstereo_helper.band_limits[b]; i < stereolpdDecData->lpdstereo_helper.band_limits[b + 1]; i++) {
      pDFT_L[2 * i]     = pDFT_DMX[2 * i]     + g * pDFT_DMX[2 * i];
      pDFT_L[2 * i + 1] = pDFT_DMX[2 * i + 1] + g * pDFT_DMX[2 * i + 1];
      pDFT_R[2 * i]     = pDFT_DMX[2 * i]     - g * pDFT_DMX[2 * i];
      pDFT_R[2 * i + 1] = pDFT_DMX[2 * i + 1] - g * pDFT_DMX[2 * i + 1];
    }

    if (b < stereolpdDecData->lpdstereo_helper.ipd_band_max) {
      alpha = pIpd[b];
      beta  = (float)(atan2(sin(alpha), (cos(alpha) + c)));
    }

    for (i = stereolpdDecData->lpdstereo_helper.band_limits[b]; i < stereolpdDecData->lpdstereo_helper.band_limits[b + 1]; i++) {
      nrg_L              = pDFT_L[2 * i] * pDFT_L[2 * i] + pDFT_L[2 * i + 1] * pDFT_L[2 * i + 1];
      nrg_R              = pDFT_R[2 * i] * pDFT_R[2 * i] + pDFT_R[2 * i + 1] * pDFT_R[2 * i + 1] + STEREO_LPD_FLT_MIN;
      sum_real           = pDFT_DMX[2 * i];
      sum_img            = pDFT_DMX[2 * i + 1];
      sum_abs            = sum_real * sum_real + sum_img * sum_img + STEREO_LPD_FLT_MIN;
      c                  = (float)sqrt(2.0 * sum_abs / (nrg_L + nrg_R));
      c                  = MAX(MIN(c, STERO_LPD_DMX_LIMIT), 1.0f / STERO_LPD_DMX_LIMIT);
      pDFT_L[2 * i]     *=c;
      pDFT_L[2 * i + 1] *=c;
      pDFT_R[2 * i]     *=c;
      pDFT_R[2 * i + 1] *=c;

      if (b < stereolpdDecData->lpdstereo_helper.ipd_band_max) {
        sum_real          = pDFT_L[2 * i] * (float)cos(beta) - pDFT_L[2 * i + 1] * (float)sin(beta);
        pDFT_L[2 * i + 1] = pDFT_L[2 * i] * (float)sin(beta) + pDFT_L[2 * i + 1] * (float)cos(beta);
        pDFT_L[2 * i]     = sum_real;
        sum_real          = pDFT_R[2 * i] * (float)cos(-alpha + beta) - pDFT_R[2 * i + 1] * (float)sin(-alpha + beta);
        pDFT_R[2 * i + 1] = pDFT_R[2 * i] * (float)sin(-alpha + beta) + pDFT_R[2 * i + 1] * (float)cos(-alpha + beta);
        pDFT_R[2 * i]     = sum_real;
      }
    }
  }

  pDFT_L[1]  = pDFT_DMX[1] + g * pDFT_DMX[1];
  pDFT_R[1]  = pDFT_DMX[1] - g * pDFT_DMX[1];
  nrg_L      = pDFT_L[1] * pDFT_L[1];
  nrg_R      = pDFT_R[1] * pDFT_R[1] + STEREO_LPD_FLT_MIN;
  sum_real   = pDFT_DMX[1];
  sum_abs    = sum_real * sum_real + STEREO_LPD_FLT_MIN;
  c          = (float)sqrt(2.0 * sum_abs / (nrg_L + nrg_R));
  c          = MAX(MIN(c, STERO_LPD_DMX_LIMIT), 1.0f / STERO_LPD_DMX_LIMIT);
  pDFT_L[1] *= c;
  pDFT_R[1] *= c;

  copyFLOAT(stereolpdDecData->past_L + 2 * PIT_MAX_MAX,
            TimeBuffer_L,
            past);

  copyFLOAT(stereolpdDecData->past_R + 2 * PIT_MAX_MAX,
            TimeBuffer_R,
            past);

  offset = past - delay;

  copyFLOAT(pDFT_L,
            pTimeBuf,
            stereolpdDecData->lpdstereo_config.dftSize_N);

  RFFTN(pTimeBuf,
        trigPtrFft,
        stereolpdDecData->lpdstereo_config.dftSize_N,
        1);

  for (i = 0; i < delay; i++) {
    TimeBuffer_L[offset + i] += pTimeBuf[i] * stereolpdDecData->win[i];
    TimeBuffer_L[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
  }

  for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++) {
    TimeBuffer_L[offset + delay + i] = pTimeBuf[i + delay];
  }

  copyFLOAT(pDFT_R,
            pTimeBuf,
            stereolpdDecData->lpdstereo_config.dftSize_N);

  RFFTN(pTimeBuf,
        trigPtrFft,
        stereolpdDecData->lpdstereo_config.dftSize_N,
        1);

  for (i = 0; i < delay; i++) {
    TimeBuffer_R[offset + i] += pTimeBuf[i] * stereolpdDecData->win[i];
    TimeBuffer_R[offset + stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] = pTimeBuf[stereolpdDecData->lpdstereo_config.dftSize_N - 1 - i] * stereolpdDecData->win[i];
  }

  for (i = 0; i < stereolpdDecData->lpdstereo_config.frameSize_M - delay; i++) {
    TimeBuffer_R[offset + delay + i] = pTimeBuf[i + delay];
  }

  copyFLOAT(TimeBuffer_L,
            synth_L + 1,
            stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024);

  set_zero(synth_L + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024 + 1,
           stereolpdDecData->lpdstereo_config.ccfl / 2 - LFAC_1024 - 1);

  copyFLOAT(TimeBuffer_R,
            synth_R + 1,
            stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024);

  set_zero(synth_R + stereolpdDecData->lpdstereo_config.ccfl / 2 + LFAC_1024 + 1,
           stereolpdDecData->lpdstereo_config.ccfl / 2 - LFAC_1024 - 1);
}

void stereoLpd_applyBPFend(
                    HSTEREOLPD_DEC_DATA         stereolpdDecData,
                    float                      *out_buffer,
                    const int                  *old_pitch,
                    const float                *old_gain,
                    const int                   last_mode,
                    const int                   ch,
                    const int                   isShort
) {
  int i                                                                             = 0;
  const int fac_FB                                                                  = stereolpdDecData->fac_FB;
  const int LpdSfd                                                                  = LPD_SFD_1024 / 2;
  const int SynSfd                                                                  = LpdSfd - BPF_SFD;
  int pitch[LPD_SFD_1024 + 3]                                                       = {0};
  float synth_buf[2 * PIT_MAX_MAX + L_FRAME_1024]                                   = {0.0f};
  float signal_out[L_FRAME_1024]                                                    = {0.0f};
  float pit_gain[LPD_SFD_1024 + 3]                                                  = {0.0f};
  float *synth                                                                      = synth_buf + fac_FB * PIT_MAX_MAX;

  set_zero(synth_buf,
           fac_FB * PIT_MAX_MAX + stereolpdDecData->lpdstereo_config.ccfl);

  if (0 == ch) {
    copyFLOAT(stereolpdDecData->past_L,
              synth_buf,
              fac_FB * PIT_MAX_MAX);
  } else {
    copyFLOAT(stereolpdDecData->past_R,
              synth_buf,
              fac_FB * PIT_MAX_MAX);
  }

  copyFLOAT(out_buffer,
            synth,
            stereolpdDecData->lpdstereo_config.ccfl);

  pit_gain[0] = stereolpdDecData->bpf_gain[2 * LpdSfd];

  copyFLOAT(old_gain,
            pit_gain + 1,
            SynSfd);

  set_zero(pit_gain + 1 + SynSfd,
           LpdSfd + 3-SynSfd - 1);

  pitch[0] = stereolpdDecData->bpf_pitch[2 * LpdSfd];

  for (i = 0; i < SynSfd; i++) {
    pitch[i + 1] = old_pitch[i];
  }

  for (i = SynSfd; i < LpdSfd + 3 - 1; i++) {
    pitch[i + 1] = L_SUBFR;
  }

  if (0 == ch) {
    _lpd_stereo_bass_pf(synth,
                        fac_FB,
                        pitch,
                        pit_gain,
                        signal_out,
                        (LpdSfd + 2) * L_SUBFR * fac_FB,
                        fac_FB * L_SUBFR,
                        fac_FB * L_SUBFR * 2,
                        stereolpdDecData->old_noise_pf_L);
  } else {
    _lpd_stereo_bass_pf(synth,
                        fac_FB,
                        pitch,
                        pit_gain,
                        signal_out,
                        (LpdSfd + 2) * L_SUBFR * fac_FB,
                        fac_FB * L_SUBFR,
                        fac_FB * L_SUBFR * 2,
                        stereolpdDecData->old_noise_pf_R);
  }

  copyFLOAT(signal_out,
            out_buffer,
            (LpdSfd + 2) * L_SUBFR * fac_FB);
}
