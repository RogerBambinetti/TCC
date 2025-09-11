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

 Copyright (c) ISO/IEC 2015.

 ***********************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector_ops.h"
#include "usac_ltpf.h"

#define LPC_ORDER 24
#define ATT_FAC 0.95f
#define NUM_COEF1 8
#define NUM_COEF2 7

const float usac_ltpf_coef1[2][8] =
{
  {0.0000000f,0.0304386f,0.1162701f,0.2195613f,0.2674597f,0.2195613f,0.1162701f,0.0304386f},
  {0.0076226f,0.0676508f,0.1700032f,0.2547232f,0.2547232f,0.1700032f,0.0676508f,0.0076226f}
};
const float usac_ltpf_coef2[4][7] =
{
  {0.27150189f, 0.44286013f, 0.23027992f, 0.05759155f, -0.00172290f, -0.00045168f, -0.00005891f},
  {0.27581838f, 0.44682277f, 0.22783915f, 0.05410054f, -0.00353758f, -0.00092331f, -0.00011995f},
  {0.28044685f, 0.45103979f, 0.22519192f, 0.05037740f, -0.00545541f, -0.00141719f, -0.00018336f},
  {0.28543320f, 0.45554676f, 0.22230634f, 0.04638935f, -0.00749011f, -0.00193612f, -0.00024943f}
};

static void usac_ltpf_decode_params(
                    int                       data_present,
                    int                       pitch_index,
                    int                       gain_index,
                    int                      *pitch_int,
                    int                      *pitch_fr,
                    float                    *gain,
                    int                       pit_min,
                    int                       pit_fr1,
                    int                       pit_fr2,
                    int                       pit_max,
                    int                       pit_res
) {
  if ( data_present )
  {
    if ( pitch_index < (pit_fr2-pit_min)*pit_res )
    {
      *pitch_int = pit_min + (pitch_index/pit_res);
      *pitch_fr = pitch_index - (*pitch_int-pit_min)*pit_res;
    }
    else if (pitch_index < ( (pit_fr2-pit_min)*pit_res + (pit_fr1-pit_fr2)*(pit_res>>1) ) )
    {
#ifdef FULLBANDLPD_TBE_OPTIM_COMPATIBILITY_MODE
      *pitch_int = pit_fr2 + (pitch_index - (pit_fr2-pit_min)*pit_res)/(pit_res>>1);
#else
      *pitch_int = pit_fr2 + pitch_index - (pit_fr2-pit_min)*pit_res;
#endif
      *pitch_fr = 0;
    }
    else
    {
#ifdef FULLBANDLPD_TBE_OPTIM_COMPATIBILITY_MODE
      *pitch_int = pitch_index + pit_fr1 - ((pit_fr2-pit_min)*pit_res) - ((pit_fr1-pit_fr2)*(pit_res>>1));
#else
      *pitch_int = ( pitch_index - (pit_fr2-pit_min)*pit_res - (pit_fr1-pit_fr2) )*pit_res + pit_fr1;
#endif
      *pitch_fr = 0;
    }

    *gain = (float)(gain_index + 1) * 0.0625f;
  }
  else
  {
    *pitch_int = 0;
    *pitch_fr = 0;
    *gain = 0.0f;
  }

  return;
}

static void usac_ltpf_filter(
                    float                    *input,
                    float                    *output,
                    int                       length,
                    int                       pit_int,
                    int                       pit_fr,
                    float                     gain,
                    int                       gain_index,
                    int                       mode,
                    float                    *zir
) {
  float s1, s2, alpha, step;
  int n, k;

  if ( gain == 0 )
  {
    vcopy( input, output, 1, 1, length );
  }
  else
  {
    if ( mode == 2 )
    {
      alpha = 0.f;
      step = 1.f/(float)length;
    }
    else if ( mode == 3 )
    {
      alpha = 1.f;
      step = -1.f/(float)length;
    }

    for (n = 0; n < length; n++)
    {
      s1 = 0;
      for (k = 0; k < NUM_COEF1; k++)
      {
        s1 += output[n-pit_int+k-NUM_COEF1/2] * usac_ltpf_coef1[pit_fr][k];
      }

      s2 = 0;
      for (k = 0; k < NUM_COEF2; k++)
      {
        s2 += input[n-k] * usac_ltpf_coef2[gain_index][k];
      }

      if ( mode == 0 )
      {
        output[n] = input[n] + gain * s1 - ATT_FAC * gain * s2;
      }
      else if ( mode == 1 )
      {
        output[n] = input[n] + gain * s1 - ATT_FAC * gain * s2 - zir[n];
      }
      else
      {
        output[n] = input[n] + alpha * ( gain * s1 - ATT_FAC * gain * s2 );
      }

      if ( mode > 1 )
      {
        alpha += step;
      }
    }
  }

  return;
}

static void usac_ltpf_get_lpc(
                    float                    *input,
                    int                       length,
                    float                    *A,
                    int                       lpcorder
) {
  int i, j;
  float s, r[LPC_ORDER+1], rc[LPC_ORDER];
  float Value, Sum, Sigma2;

  /* Autocorrelation */
  for (i = 0; i <= lpcorder; i++)
  {
    s = 0.0;

    for (j = 0; j < length-i; j++)
    {
      s += input[j-length]*input[j-length+i];
    }
    r[i] = s;
  }

  if (r[0] < 100.0f)
  {
    r[0] = 100.0f;
  }
  r[0] *= 1.0001f;

  /* Levinson-Durbin */
  A[0] = 1.0f;
  rc[0] = -r[1]/r[0];
  A[1] = rc[0];
  Sigma2 = r[0] + r[1] * rc[0];
  for (i=2; i<=lpcorder; i++)
  {
    Sum = 0.0f;
    for (j=0; j<i; j++)
    {
      Sum += r[i-j]*A[j];
    }
    rc[i-1] = -Sum/Sigma2;
    Sigma2 = Sigma2*(1.0f - rc[i-1]*rc[i-1]);
    if (Sigma2<=1.0E-09f)
    {
      Sigma2 = 1.0E-09f;
      for (i=i; i<=lpcorder; i++)
      {
        rc[i-1] = 0.0f;
        A[i] = 0.0f;
      }
      break;
    }
    for (j=1; j<=(i/2); j++)
    {
      Value = A[j] + rc[i-1]*A[i-j];
      A[i-j] += rc[i-1]*A[j];
      A[j] = Value;
    }
    A[i] = rc[i-1];
  }

  return;
}

static void usac_ltpf_get_zir(
                    float                    *input,
                    float                    *output,
                    float                    *zir,
                    int                       length,
                    float                    *A,
                    int                       lpcorder,
                    float                     gain,
                    int                       gain_index,
                    int                       pit_int,
                    int                       pit_fr
) {
  float *buf;
  float alpha, step;
  float s1, s2;
  int n, k;

  buf = (float *) calloc(lpcorder+length, sizeof(float));

  for (n = 0; n < lpcorder; n++)
  {
    s1 = 0;
    for (k = 0; k < NUM_COEF1; k++)
    {
      s1 += output[n-lpcorder-pit_int+k-4] * usac_ltpf_coef1[pit_fr][k];
    }

    s2 = 0;
    for (k = 0; k < NUM_COEF2; k++)
    {
      s2 += input[n-lpcorder-k] * usac_ltpf_coef2[gain_index][k];
    }

    buf[n] = ( input[n-lpcorder] - ATT_FAC * gain * s2 ) - ( output[n-lpcorder] - gain * s1 );
  }

  for (n = 0; n < length; n++)
  {
    for (k = 1; k <= lpcorder; k++ )
    {
      buf[lpcorder+n] -= A[k] * buf[lpcorder+n-k];
    }
  }

  vcopy( buf+lpcorder, zir, 1, 1, length );

  alpha = 1.f;
  step = 1.f/(float)(length/2);

  for (n = length/2; n < length; n++ )
  {
    zir[n] *= alpha;
    alpha -= step;
  }

  free(buf);

  return;
}

void usac_ltpf_init(
                    LTPF_DATA                *ltpfData,
                    int                       samplingRate,
                    int                       block_size_samples
) {
  ltpfData->L_frame = block_size_samples;
  ltpfData->L_transition = block_size_samples / 8;
  ltpfData->enc_dec_delay = block_size_samples / 2;

  ltpfData->pit_min = (int)( 34.f * ( (float)samplingRate / 2.f ) / 12800.f + 0.5f ) * 2;
  ltpfData->pit_fr2 = 324 - ltpfData->pit_min;
  ltpfData->pit_fr1 = 320;
  ltpfData->pit_max = 54 + 6 * ltpfData->pit_min;
  ltpfData->pit_res = 2;

  ltpfData->data_present = 0;
  ltpfData->pitch_index = 0;
  ltpfData->gain_index = 0;

  ltpfData->pitch_int_past = 0;
  ltpfData->pitch_fr_past = 0;
  ltpfData->gain_past = 0.f;
  ltpfData->gain_index_past = 0;
  ltpfData->mem_in = (float *) calloc( NUM_COEF2-1, sizeof(float) );
  ltpfData->mem_out = (float *) calloc( ltpfData->pit_max+NUM_COEF1/2, sizeof(float) );
}

void usac_ltpf_bs_duplicate(LTPF_DATA ltpfDataL, LTPF_DATA *ltpfDataR){

  ltpfDataR->data_present = ltpfDataL.data_present;
  ltpfDataR->pitch_index  = ltpfDataL.pitch_index;
  ltpfDataR->gain_index   = ltpfDataL.gain_index; 

}

void usac_ltpf_free(
                    LTPF_DATA                *ltpfData
) {
  if (NULL != ltpfData->mem_in) {
    free(ltpfData->mem_in);
  }
  if (NULL != ltpfData->mem_out) {
    free( ltpfData->mem_out );
  }
}

void usac_ltpf_process(
                    LTPF_DATA                *ltpfData,
                    float                    *time_sample_vector
) {
  int pitch_int, pitch_fr;
  float A[LPC_ORDER+1], gain;
  float *buf_in, *buf_out, *sig_in, *sig_out, *zir;

  /******** Init ********/

  /* Input buffer */
  buf_in = (float *) calloc( NUM_COEF2 - 1 + ltpfData->L_frame, sizeof(float) );
  sig_in = buf_in + NUM_COEF2 - 1;
  vcopy( ltpfData->mem_in, buf_in, 1, 1, NUM_COEF2 - 1 );
  vcopy( time_sample_vector, sig_in, 1, 1, ltpfData->L_frame );
  vcopy( buf_in+ltpfData->L_frame, ltpfData->mem_in, 1, 1, NUM_COEF2 - 1 );

  /* Output buffer */
  buf_out = (float *) calloc( ltpfData->pit_max+NUM_COEF1/2 + ltpfData->L_frame, sizeof(float) );
  sig_out = buf_out + ltpfData->pit_max+NUM_COEF1/2;
  vcopy( ltpfData->mem_out, buf_out, 1, 1, ltpfData->pit_max+NUM_COEF1/2 );

  /* ZIR */
  zir = (float *) calloc( ltpfData->L_transition, sizeof(float) );

  /* Filter parameters: integer part of the pitch lag, fractional part of the pitch lag, gain */
  usac_ltpf_decode_params(ltpfData->data_present,
                          ltpfData->pitch_index,
                          ltpfData->gain_index,
                          &pitch_int,
                          &pitch_fr,
                          &gain,
                          ltpfData->pit_min,
                          ltpfData->pit_fr1,
                          ltpfData->pit_fr2,
                          ltpfData->pit_max,
                          ltpfData->pit_res);

  /******** Previous-frame part ********/
  usac_ltpf_filter( sig_in,
                    sig_out,
                    ltpfData->enc_dec_delay,
                    ltpfData->pitch_int_past,
                    ltpfData->pitch_fr_past,
                    ltpfData->gain_past,
                    ltpfData->gain_index_past,
                    0,
                    NULL );

  /******** Transition part ********/
  if ( gain==ltpfData->gain_past && pitch_int==ltpfData->pitch_int_past && pitch_fr==ltpfData->pitch_fr_past )
  {
    usac_ltpf_filter( sig_in+ltpfData->enc_dec_delay,
                      sig_out+ltpfData->enc_dec_delay,
                      ltpfData->L_transition,
                      pitch_int,
                      pitch_fr,
                      gain,
                      ltpfData->gain_index,
                      0,
                      NULL );
  }
  else if ( ltpfData->gain_past==0.f )
  {
    usac_ltpf_filter( sig_in+ltpfData->enc_dec_delay,
                      sig_out+ltpfData->enc_dec_delay,
                      ltpfData->L_transition,
                      pitch_int,
                      pitch_fr,
                      gain,
                      ltpfData->gain_index,
                      2,
                      NULL );
  }
  else if ( gain==0.f )
  {
    usac_ltpf_filter( sig_in+ltpfData->enc_dec_delay,
                      sig_out+ltpfData->enc_dec_delay,
                      ltpfData->L_transition,
                      ltpfData->pitch_int_past,
                      ltpfData->pitch_fr_past,
                      ltpfData->gain_past,
                      ltpfData->gain_index_past,
                      3,
                      NULL );
  }
  else
  {
    usac_ltpf_get_lpc( sig_out+ltpfData->enc_dec_delay,
                       ltpfData->L_frame/4,
                       A,
                       LPC_ORDER );

    usac_ltpf_get_zir( sig_in+ltpfData->enc_dec_delay,
                       sig_out+ltpfData->enc_dec_delay,
                       zir,
                       ltpfData->L_transition,
                       A,
                       LPC_ORDER,
                       gain,
                       ltpfData->gain_index,
                       pitch_int,
                       pitch_fr );

    usac_ltpf_filter( sig_in+ltpfData->enc_dec_delay,
                      sig_out+ltpfData->enc_dec_delay,
                      ltpfData->L_transition,
                      pitch_int,
                      pitch_fr,
                      gain,
                      ltpfData->gain_index,
                      1,
                      zir );
  }

  /******** Current-frame part ********/
  usac_ltpf_filter( sig_in+ltpfData->enc_dec_delay+ltpfData->L_transition,
                    sig_out+ltpfData->enc_dec_delay+ltpfData->L_transition,
                    ltpfData->L_frame-ltpfData->enc_dec_delay-ltpfData->L_transition,
                    pitch_int,
                    pitch_fr,
                    gain,
                    ltpfData->gain_index,
                    0,
                    NULL );

  /******** Finalize ********/

  /* copy to output */
  vcopy( sig_out, time_sample_vector, 1, 1, ltpfData->L_frame );

  /* Update */
  ltpfData->pitch_int_past = pitch_int;
  ltpfData->pitch_fr_past = pitch_fr;
  ltpfData->gain_past = gain;
  ltpfData->gain_index_past = ltpfData->gain_index;
  vcopy( buf_out+ltpfData->L_frame, ltpfData->mem_out, 1, 1, ltpfData->pit_max+NUM_COEF1/2 );

  /* Free */
  free( buf_in );
  free( buf_out );
  free( zir );

}
