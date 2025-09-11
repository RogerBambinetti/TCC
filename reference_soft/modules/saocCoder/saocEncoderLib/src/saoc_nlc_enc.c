/*******************************************************************************
This software module was originally developed by

Coding Technologies, Fraunhofer IIS, Philips

and edited by

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

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:
Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Coding Technologies, Fraunhofer IIS,
Philips own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Coding Technologies, Fraunhofer
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

#include <math.h>
#include <memory.h>

#include "saoc_stream.h"
#include "saoc_nlc_enc.h"
#include "saoc_huff_tab.h"
#include "saoc_types.h"


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))


extern const HUFF_CLD_TABLE huffCLDTab;
extern const HUFF_ICC_TABLE huffICCTab;
extern const HUFF_CPC_TABLE huffCPCTab;
extern const HUFF_OLD_TABLE huffOLDTab;
extern const HUFF_NRG_TABLE huffNRGTab;


static int split_lsb( int* in_data,
                      int  offset,
                      int  num_lsb,
                      int  num_val,
                      int* out_data_lsb,
                      int* out_data_msb )
{
  int i = 0, val = 0, lsb = 0, msb = 0;

  unsigned int mask = (1 << num_lsb) - 1;
  int no_lsb_bits = 0;

  for( i=0; i<num_val; i++ ) {

    val = in_data[i] + offset;

    lsb = val & mask;
    msb = val >> num_lsb;

    if( out_data_lsb != NULL) out_data_lsb[i] = lsb;
    if( out_data_msb != NULL) out_data_msb[i] = msb;

    no_lsb_bits += num_lsb;

  }

  return no_lsb_bits;
}


static void apply_lsb_coding( Stream* strm,
                              int*    in_data_lsb,
                              int     num_lsb,
                              int     num_val )
{
  int i = 0;

  for( i=0; i<num_val; i++ ) {
    writeBits( strm, in_data_lsb[i], num_lsb );
  }
}


static void calc_diff_freq( int* in_data,
                            int* out_data,
                            int  num_val )
{
  int i = 0;

  out_data[0] = in_data[0];

  for( i=1; i<num_val; i++ ) {
    out_data[i] = in_data[i] - in_data[i-1];
  }
}


static void calc_diff_time( int*      in_data,
                            int*      prev_data,
                            int*      out_data,
                            DIRECTION direction,
                            int       num_val )
{
  int i = 0;

  out_data[-1] = (direction == BACKWARDS) ? in_data[0] : prev_data[0];

  for( i=0; i<num_val; i++ ) {
    out_data[i] = in_data[i] - prev_data[i];
  }
}


static int calc_pcm_bits( int num_val,
                          int num_levels )
{
  int num_complete_chunks = 0, rest_chunk_size = 0;
  int max_grp_len = 0, bits_pcm = 0;

  float ld_nlev = 0.f;

  switch( num_levels )
    {
    case  3: max_grp_len = 5; break;
    case  6: max_grp_len = 5; break;
    case  7: max_grp_len = 6; break;
    case 11: max_grp_len = 2; break;
    case 13: max_grp_len = 4; break;
    case 19: max_grp_len = 4; break;
    case 25: max_grp_len = 3; break;
    case 51: max_grp_len = 4; break;
    default: max_grp_len = 1;
    }

  ld_nlev = (float)(log((float)num_levels)/log(2.f));


  num_complete_chunks = num_val/max_grp_len;
  rest_chunk_size     = num_val%max_grp_len;

  bits_pcm  = ((int) ceil((float)(max_grp_len)*ld_nlev)) * num_complete_chunks;
  bits_pcm += (int) ceil((float)(rest_chunk_size)*ld_nlev);

  return bits_pcm;
}


static void apply_pcm_coding( Stream* strm,
                              int*    in_data_1,
                              int*    in_data_2,
                              int     offset,
                              int     num_val,
                              int     num_levels )
{
  int i = 0, j = 0, idx = 0;
  int max_grp_len = 0, grp_len = 0, next_val = 0, grp_val = 0;

  float ld_nlev = 0.f;

  int pcm_chunk_size[7] = { 0 };


  switch( num_levels )
    {
    case  3: max_grp_len = 5; break;
    case  6: max_grp_len = 5; break;
    case  7: max_grp_len = 6; break;
    case 11: max_grp_len = 2; break;
    case 13: max_grp_len = 4; break;
    case 19: max_grp_len = 4; break;
    case 25: max_grp_len = 3; break;
    case 51: max_grp_len = 4; break;
    default: max_grp_len = 1;
    }

  ld_nlev = (float)(log((float)num_levels)/log(2.f));

  for( i=1; i<=max_grp_len; i++ ) {
    pcm_chunk_size[i] = (int) ceil( (float)(i) * ld_nlev );
  }


  for( i=0; i<num_val; i+=max_grp_len ) {
    grp_len = min( max_grp_len, num_val-i );
    grp_val = 0;
    for( j=0; j<grp_len; j++ ) {
      idx = i+j;
      if( in_data_2 == NULL ) {
        next_val = in_data_1[idx];
      }
      else if( in_data_1 == NULL ) {
        next_val = in_data_2[idx];
      }
      else {
        next_val = ((idx%2) ? in_data_2[idx/2] : in_data_1[idx/2]);
      }
      next_val += offset;
      grp_val = grp_val*num_levels + next_val;
    }

    writeBits( strm, grp_val, pcm_chunk_size[grp_len] );
  }

}


static int count_huff_cld( const HUFF_CLD_TAB* huffTabPt0,
                           const HUFF_CLD_TAB* huffTabDiff,
                           int*                in_data,
                           int                 num_val,
                           int                 p0_flag )
{
  int i = 0, id = 0;
  int huffBits = 0;
  int offset = 0;

  if( p0_flag ) {
    huffBits += huffTabPt0->length[in_data[0]];
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      if( id < 0 ) {
        id = -id;
      }
      huffBits += 1;
    }

    huffBits += huffTabDiff->length[id];
  }

  return huffBits;
}


static int count_huff_icc( const HUFF_ICC_TAB* huffTabPt0,
                           const HUFF_ICC_TAB* huffTabDiff,
                           int*                in_data,
                           int                 num_val,
                           int                 p0_flag )
{
  int i = 0, id = 0;
  int huffBits = 0;
  int offset = 0;

  if( p0_flag ) {
    huffBits += huffTabPt0->length[in_data[0]];
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      if( id < 0 ) {
        id = -id;
      }
      huffBits += 1;
    }

    huffBits += huffTabDiff->length[id];
  }

  return huffBits;
}


static int count_huff_cpc( const HUFF_CPC_TAB* huffTabPt0,
                           const HUFF_CPC_TAB* huffTabDiff,
                           int*                in_data,
                           int                 num_val,
                           int                 p0_flag )
{
  int i = 0, id = 0;
  int huffBits = 0;
  int offset = 0;

  if( p0_flag ) {
    huffBits += huffTabPt0->length[in_data[0]];
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      if( id < 0 ) {
        id = -id;
      }
      huffBits += 1;
    }

    huffBits += huffTabDiff->length[id];
  }

  return huffBits;
}

static short count_huff_old( const HUFF_OLD_TAB* huffTabPt0,
                             const HUFF_OLD_TAB* huffTabDiff,
                             int*                 in_data,
                             int                  num_val,
                             int                  p0_flag )
{
  short i = 0, id = 0;
  short huffBits = 0;
  short offset = 0;

  if( p0_flag ) {
    huffBits += huffTabPt0->length[in_data[0]];
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      if( id < 0 ) {
        id = -id;
      }
      huffBits += 1;
    }

    huffBits += huffTabDiff->length[id];
  }

  return huffBits;
}


static short count_huff_nrg( const HUFF_NRG_TAB* huffTabPt0,
                             const HUFF_NRG_TAB* huffTabDiff,
                             int*                 in_data,
                             int                  num_val,
                             int                  p0_flag )
{
  short i = 0, id = 0;
  short huffBits = 0;
  short offset = 0;

  if( p0_flag ) {
    huffBits += huffTabPt0->length[in_data[0]];
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      if( id < 0 ) {
        id = -id;
      }
      huffBits += 1;
    }

    huffBits += huffTabDiff->length[id];
  }

  return huffBits;
}


static void huff_enc_cld( Stream*             strm,
                          const HUFF_CLD_TAB* huffTabPt0,
                          const HUFF_CLD_TAB* huffTabDiff,
                          int*                in_data,
                          int                 num_val,
                          int                 p0_flag )
{
  int i = 0, id = 0, id_sign = 0;
  int offset = 0;

  if( p0_flag ) {
    writeBits( strm, huffTabPt0->value [in_data[0]],
                     huffTabPt0->length[in_data[0]] );
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      id_sign = 0;
      if( id < 0 ) {
        id = -id;
        id_sign = 1;
      }
    }

    writeBits( strm, huffTabDiff->value[id], huffTabDiff->length[id] );

    if( id != 0 ) {
      writeBits( strm, id_sign, 1 );
    }

  }
}


static void huff_enc_icc( Stream*             strm,
                          const HUFF_ICC_TAB* huffTabPt0,
                          const HUFF_ICC_TAB* huffTabDiff,
                          int*                in_data,
                          int                 num_val,
                          int                 p0_flag )
{
  int i = 0, id = 0, id_sign = 0;
  int offset = 0;

  if( p0_flag ) {
    writeBits( strm, huffTabPt0->value [in_data[0]],
                     huffTabPt0->length[in_data[0]] );
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      id_sign = 0;
      if( id < 0 ) {
        id = -id;
        id_sign = 1;
      }
    }

    writeBits( strm, huffTabDiff->value[id], huffTabDiff->length[id] );

    if( id != 0 ) {
      writeBits( strm, id_sign, 1 );
    }

  }
}

static void huff_enc_cpc( Stream*             strm,
                          const HUFF_CPC_TAB* huffTabPt0,
                          const HUFF_CPC_TAB* huffTabDiff,
                          int*                in_data,
                          int                 num_val,
                          int                 p0_flag )
{
  int i = 0, id = 0, id_sign = 0;
  int offset = 0;

  if( p0_flag ) {
    writeBits( strm, huffTabPt0->value [in_data[0]],
                     huffTabPt0->length[in_data[0]] );
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      id_sign = 0;
      if( id < 0 ) {
        id = -id;
        id_sign = 1;
      }
    }

    writeBits( strm, huffTabDiff->value[id], huffTabDiff->length[id] );

    if( id != 0 ) {
      writeBits( strm, id_sign, 1 );
    }

  }
}

static void huff_enc_old( Stream*                strm,
                             const HUFF_OLD_TAB* huffTabPt0,
                             const HUFF_OLD_TAB* huffTabDiff,
                             int*                 in_data,
                             int                  num_val,
                             int                  p0_flag )
{
  short i = 0, id = 0, id_sign = 0;
  short offset = 0;

  if( p0_flag ) {
    writeBits( strm, huffTabPt0->value [in_data[0]],
                     huffTabPt0->length[in_data[0]] );
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      id_sign = 0;
      if( id < 0 ) {
        id = -id;
        id_sign = 1;
      }
    }

    writeBits( strm, huffTabDiff->value[id], huffTabDiff->length[id] );

    if( id != 0 ) {
      writeBits( strm, id_sign, 1 );
    }

  }
}


static void huff_enc_nrg( Stream*                strm,
                             const HUFF_NRG_TAB* huffTabPt0,
                             const HUFF_NRG_TAB* huffTabDiff,
                             int*                 in_data,
                             int                  num_val,
                             int                  p0_flag )
{
  short i = 0, id = 0, id_sign = 0;
  short offset = 0;

  if( p0_flag ) {
    writeBits( strm, huffTabPt0->value [in_data[0]],
                     huffTabPt0->length[in_data[0]] );
    offset = 1;
  }

  for( i=offset; i<num_val; i++ ) {

    id = in_data[i];

    if( id != 0 ) {
      id_sign = 0;
      if( id < 0 ) {
        id = -id;
        id_sign = 1;
      }
    }

    writeBits( strm, huffTabDiff->value[id], huffTabDiff->length[id] );

    if( id != 0 ) {
      writeBits( strm, id_sign, 1 );
    }

  }
}


static int calc_huff_bits( int*           in_data_1,
                           int*           in_data_2,
                           DATA_TYPE      data_type,
                           DIFF_TYPE      diff_type_1,
                           DIFF_TYPE      diff_type_2,
                           int            num_val )
{
  int* p0_data_1[2] = {NULL, NULL};
  int* p0_data_2[2] = {NULL, NULL};

  int p0_flag[2];

  int i = 0;

  int df_rest_flag_1 = 0, df_rest_flag_2 = 0;

  int offset_1 = (diff_type_1 == DIFF_TIME) ? 1 : 0;
  int offset_2 = (diff_type_2 == DIFF_TIME) ? 1 : 0;

  int bit_count_huff      = 0;
  int bit_count_min     = 0;

  int num_val_1_int = 0;
  int num_val_2_int = 0;

  int* in_data_1_int = in_data_1 + offset_1;
  int* in_data_2_int = in_data_2 + offset_2;


  bit_count_huff = 1;

  num_val_1_int = num_val;
  num_val_2_int = num_val;

  p0_flag[0] = (diff_type_1 == DIFF_FREQ);
  p0_flag[1] = (diff_type_2 == DIFF_FREQ);

  switch( data_type ) {
  case t_CLD:
    if( in_data_1 != NULL ) bit_count_huff += count_huff_cld( &huffCLDTab.huffPt0, &huffCLDTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) bit_count_huff += count_huff_cld( &huffCLDTab.huffPt0, &huffCLDTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_ICC:
    if( in_data_1 != NULL ) bit_count_huff += count_huff_icc( &huffICCTab.huffPt0, &huffICCTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) bit_count_huff += count_huff_icc( &huffICCTab.huffPt0, &huffICCTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_CPC:
    if( in_data_1 != NULL ) bit_count_huff += count_huff_cpc( &huffCPCTab.huffPt0, &huffCPCTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) bit_count_huff += count_huff_cpc( &huffCPCTab.huffPt0, &huffCPCTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_OLD:
    if( in_data_1 != NULL ) bit_count_huff += count_huff_old( &huffOLDTab.huffPt0, &huffOLDTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) bit_count_huff += count_huff_old( &huffOLDTab.huffPt0, &huffOLDTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_NRG:
    if( in_data_1 != NULL ) bit_count_huff += count_huff_nrg( &huffNRGTab.huffPt0, &huffNRGTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) bit_count_huff += count_huff_nrg( &huffNRGTab.huffPt0, &huffNRGTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  default:
    break;
  }


  return bit_count_huff;

}


static void apply_huff_coding( Stream*          strm,
                               int*             in_data_1,
                               int*             in_data_2,
                               DATA_TYPE        data_type,
                               DIFF_TYPE        diff_type_1,
                               DIFF_TYPE        diff_type_2,
                               int              num_val )
{
  int i = 0;

  int* p0_data_1[2] = {NULL, NULL};
  int* p0_data_2[2] = {NULL, NULL};

  int p0_flag[2];

  int df_rest_flag_1 = 0, df_rest_flag_2 = 0;

  int num_val_1_int = num_val;
  int num_val_2_int = num_val;

  int* in_data_1_int = in_data_1;
  int* in_data_2_int = in_data_2;

  if( diff_type_1 == DIFF_TIME ) in_data_1_int += 1;
  if( diff_type_2 == DIFF_TIME ) in_data_2_int += 1;



  writeBits( strm, HUFF_1D, 1 );

  p0_flag[0] = (diff_type_1 == DIFF_FREQ);
  p0_flag[1] = (diff_type_2 == DIFF_FREQ);

  switch( data_type ) {
  case t_CLD:
    if( in_data_1 != NULL ) huff_enc_cld( strm, &huffCLDTab.huffPt0, &huffCLDTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) huff_enc_cld( strm, &huffCLDTab.huffPt0, &huffCLDTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_ICC:
    if( in_data_1 != NULL ) huff_enc_icc( strm, &huffICCTab.huffPt0, &huffICCTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) huff_enc_icc( strm, &huffICCTab.huffPt0, &huffICCTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_CPC:
    if( in_data_1 != NULL ) huff_enc_cpc( strm, &huffCPCTab.huffPt0, &huffCPCTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) huff_enc_cpc( strm, &huffCPCTab.huffPt0, &huffCPCTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_OLD:
    if( in_data_1 != NULL ) huff_enc_old( strm, &huffOLDTab.huffPt0, &huffOLDTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) huff_enc_old( strm, &huffOLDTab.huffPt0, &huffOLDTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  case t_NRG:
    if( in_data_1 != NULL ) huff_enc_nrg( strm, &huffNRGTab.huffPt0, &huffNRGTab.huffDiff[diff_type_1], in_data_1_int, num_val_1_int, p0_flag[0] );
    if( in_data_2 != NULL ) huff_enc_nrg( strm, &huffNRGTab.huffPt0, &huffNRGTab.huffDiff[diff_type_2], in_data_2_int, num_val_2_int, p0_flag[1] );
    break;

  default:
    break;
  }

}


int EcDataPairEnc_SAOC( Stream*    strm,
                   int        aaInData[][max(MAX_NUM_BINS,MAX_OBJECTS)],
                   int        aHistory[max(MAX_NUM_BINS,MAX_OBJECTS)],
                   DATA_TYPE  data_type,
                   int        setIdx,
                   int        startBand,
                   int        dataBands,
                   int        pair_flag,
                   int        coarse_flag,
                   int        independency_flag)
{
  int dummy = 0, reset = 0;
  int quant_levels = 0, quant_offset = 0, num_pcm_val = 0;

  int splitLsb_flag  = 0;
  int pcmCoding_flag = 0;

  int allowDiffTimeBack_flag = !independency_flag || (setIdx > 0);

  int num_lsb_bits[2] = { 0, 0 };
  int num_pcm_bits    = 0;

  int aDataHist[max(MAXBANDS,MAX_OBJECTS)] = {0};
  int aaDataPair[2][max(MAXBANDS,MAX_OBJECTS)] = {{0}};

  int quant_data_lsb[2][max(MAXBANDS,MAX_OBJECTS)] = {{0}};
  int quant_data_msb[2][max(MAXBANDS,MAX_OBJECTS)] = {{0}};

  int quant_data_hist_lsb[max(MAXBANDS,MAX_OBJECTS)] = {0};
  int quant_data_hist_msb[max(MAXBANDS,MAX_OBJECTS)] = {0};

  int data_diff_freq   [2][max(MAXBANDS,MAX_OBJECTS)  ] = {{0}};
  int data_diff_time_bw[2][max(MAXBANDS,MAX_OBJECTS)+1] = {{0}};
  int data_diff_time_fw   [max(MAXBANDS,MAX_OBJECTS)+1] =  {0} ;

  int* p_data_pcm         [2] = {NULL};
  int* p_data_diff_freq   [2] = {NULL};
  int* p_data_diff_time_bw[2] = {NULL};
  int* p_data_diff_time_fw    =  NULL ;

  int min_bits_all = 0;
  int min_found    = 0;

  int min_bits_df_df   = -1;
  int min_bits_df_dt   = -1;
  int min_bits_dtbw_df = -1;
  int min_bits_dt_dt   = -1;


  switch( data_type ) {
  case t_CLD:
    if( coarse_flag ) {
      splitLsb_flag   =  0;
      quant_levels    = 15;
      quant_offset    =  7;
    }
    else {
      splitLsb_flag   =  0;
      quant_levels    = 31;
      quant_offset    = 15;
    }

    break;

  case t_ICC:
    if( coarse_flag ) {
      splitLsb_flag   =  0;
      quant_levels    =  4;
      quant_offset    =  0;
    }
    else {
      splitLsb_flag   =  0;
      quant_levels    =  8;
      quant_offset    =  0;
    }

    break;

  case t_CPC:
    if( coarse_flag ) {
      splitLsb_flag   =  0;
      quant_levels    = 26;
      quant_offset    =  0;
    }
    else {
      splitLsb_flag   =  1;
      quant_levels    = 51;
      quant_offset    =  0;
    }

    break;

  case t_OLD:
    splitLsb_flag   =  0;
    quant_levels    = 16;
    quant_offset    =  0;

    break;

  case t_NRG:
    splitLsb_flag   =  0;
    quant_levels    = 64;
    quant_offset    =  0;

    break;

  default:
    fprintf( stderr, "Unknown type of data!\n" );
    return 0;
  }



  memcpy( aDataHist, aHistory+startBand, sizeof(int)*dataBands );

  memcpy( aaDataPair[0], aaInData[setIdx]+startBand, sizeof(int)*dataBands );
  p_data_pcm[0] = aaDataPair[0];
  if( pair_flag ) {
    memcpy( aaDataPair[1], aaInData[setIdx+1]+startBand, sizeof(int)*dataBands );
    p_data_pcm[1] = aaDataPair[1];
  }



  num_lsb_bits[0] = split_lsb( aaDataPair[0],
                               quant_offset,
                               splitLsb_flag ? 1 : 0,
                               dataBands,
                               quant_data_lsb[0],
                               quant_data_msb[0] );

  if( pair_flag ) {
    num_lsb_bits[1] = split_lsb( aaDataPair[1],
                                 quant_offset,
                                 splitLsb_flag ? 1 : 0,
                                 dataBands,
                                 quant_data_lsb[1],
                                 quant_data_msb[1] );
  }

  if( allowDiffTimeBack_flag ) {
    dummy = split_lsb( aDataHist,
                       quant_offset,
                       splitLsb_flag ? 1 : 0,
                       dataBands,
                       quant_data_hist_lsb,
                       quant_data_hist_msb );
  }



  calc_diff_freq( quant_data_msb[0],
                  data_diff_freq[0],
                  dataBands );
  p_data_diff_freq[0] = data_diff_freq[0];

  if( pair_flag ) {
    calc_diff_freq( quant_data_msb[1],
                    data_diff_freq[1],
                    dataBands );
    p_data_diff_freq[1] = data_diff_freq[1];
  }



  if( allowDiffTimeBack_flag ) {
    calc_diff_time( quant_data_msb[0],
                    quant_data_hist_msb,
                    data_diff_time_bw[0]+1,
                    BACKWARDS,
                    dataBands );
    p_data_diff_time_bw[0] = data_diff_time_bw[0];
  }

  if( pair_flag ) {
    calc_diff_time( quant_data_msb[1],
                    quant_data_msb[0],
                    data_diff_time_bw[1]+1,
                    BACKWARDS,
                    dataBands );
    p_data_diff_time_bw[1] = data_diff_time_bw[1];

  }





  if( pair_flag ) {
    num_pcm_bits = calc_pcm_bits( 2*dataBands, quant_levels );
    num_pcm_val   = 2*dataBands;
  }
  else {
    num_pcm_bits = calc_pcm_bits( dataBands, quant_levels );
    num_pcm_val   = dataBands;
  }


  min_bits_all = num_pcm_bits;


  if( (p_data_diff_freq[0] != NULL) || (p_data_diff_freq[1] != NULL) ) {
    min_bits_df_df = calc_huff_bits( p_data_diff_freq[0],
                                     p_data_diff_freq[1],
                                     data_type,
                                     DIFF_FREQ, DIFF_FREQ,
                                     dataBands );

    if( pair_flag || allowDiffTimeBack_flag ) min_bits_df_df += 1;
    if( pair_flag                           ) min_bits_df_df += 1;

    if( min_bits_df_df < min_bits_all ) {
      min_bits_all = min_bits_df_df;
    }
  }



  if( (p_data_diff_freq[0] != NULL) || (p_data_diff_time_bw[1] != NULL) ) {
    min_bits_df_dt = calc_huff_bits( p_data_diff_freq[0],
                                     p_data_diff_time_bw[1],
                                     data_type,
                                     DIFF_FREQ, DIFF_TIME,
                                     dataBands );

    if( pair_flag || allowDiffTimeBack_flag ) min_bits_df_dt += 1;
    if( pair_flag                           ) min_bits_df_dt += 1;

    if( min_bits_df_dt < min_bits_all ) {
      min_bits_all = min_bits_df_dt;
    }
  }



  if( allowDiffTimeBack_flag ) {


    if( (p_data_diff_time_bw[0] != NULL) || (p_data_diff_freq[1] != NULL) ) {
      min_bits_dtbw_df = calc_huff_bits( p_data_diff_time_bw[0],
                                         p_data_diff_freq[1],
                                         data_type,
                                         DIFF_TIME, DIFF_FREQ,
                                         dataBands );

      min_bits_dtbw_df += 1;
      if( pair_flag ) {
        min_bits_dtbw_df += 1;
      }

      if( min_bits_dtbw_df < min_bits_all ) {
        min_bits_all = min_bits_dtbw_df;
      }
    }



    if( (p_data_diff_time_bw[0] != NULL) || (p_data_diff_time_bw[1] != NULL) ) {
      min_bits_dt_dt = calc_huff_bits( p_data_diff_time_bw[0],
                                       p_data_diff_time_bw[1],
                                       data_type,
                                       DIFF_TIME, DIFF_TIME,
                                       dataBands );

      min_bits_dt_dt += 1;
      if( pair_flag ) min_bits_dt_dt += 1;

      if( min_bits_dt_dt < min_bits_all ) {
        min_bits_all = min_bits_dt_dt;
      }
    }


  }









  pcmCoding_flag = (min_bits_all == num_pcm_bits);

  writeBits( strm, pcmCoding_flag, 1 );


  if( pcmCoding_flag ) {

    apply_pcm_coding( strm,
                      p_data_pcm[0],
                      p_data_pcm[1],
                      quant_offset,
                      num_pcm_val,
                      quant_levels );
  }
  else {



    min_found = 0;


    if( min_bits_all == min_bits_df_df ) {

      if( pair_flag || allowDiffTimeBack_flag ) {
        writeBits( strm, DIFF_FREQ, 1 );
      }

      if( pair_flag ) {
        writeBits( strm, DIFF_FREQ, 1 );
      }

      apply_huff_coding( strm,
                         p_data_diff_freq[0],
                         p_data_diff_freq[1],
                         data_type,
                         DIFF_FREQ, DIFF_FREQ,
                         dataBands );

      min_found = 1;
    }



    if( !min_found && (min_bits_all == min_bits_df_dt) ) {

      if( pair_flag || allowDiffTimeBack_flag ) {
        writeBits( strm, DIFF_FREQ, 1 );
      }

      if( pair_flag ) {
        writeBits( strm, DIFF_TIME, 1 );
      }

      apply_huff_coding( strm,
                         p_data_diff_freq[0],
                         p_data_diff_time_bw[1],
                         data_type,
                         DIFF_FREQ, DIFF_TIME,
                         dataBands );

      min_found = 1;
    }


    if( allowDiffTimeBack_flag ) {


      if( !min_found && (min_bits_all == min_bits_dtbw_df) ) {

        writeBits( strm, DIFF_TIME, 1 );

        if( pair_flag ) {
          writeBits( strm, DIFF_FREQ, 1 );
        }

        apply_huff_coding( strm,
                           p_data_diff_time_bw[0],
                           p_data_diff_freq[1],
                           data_type,
                           DIFF_TIME, DIFF_FREQ,
                           dataBands );

        min_found = 1;
      }



      if( !min_found && (min_bits_all == min_bits_dt_dt) ) {

        writeBits( strm, DIFF_TIME, 1 );

        if( pair_flag ) {
          writeBits( strm, DIFF_TIME, 1 );
        }

        apply_huff_coding( strm,
                           p_data_diff_time_bw[0],
                           p_data_diff_time_bw[1],
                           data_type,
                           DIFF_TIME, DIFF_TIME,
                           dataBands );
      }


    }





    if( splitLsb_flag ) {

      apply_lsb_coding( strm,
                        quant_data_lsb[0],
                        1,
                        dataBands );

      if( pair_flag ) {
        apply_lsb_coding( strm,
                          quant_data_lsb[1],
                          1,
                          dataBands );
      }

    }


  }


  return reset;
}
