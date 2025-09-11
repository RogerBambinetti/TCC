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
#include <string.h>
#include <stdio.h>

#include "saoc_nlc_dec.h"
#include "saoc_huff_nodes.h"
#include "saoc_bitinput.h"
#include "saoc_const.h"
#include "error.h"


extern const HUFF_CLD_NODES huffCLDNodes;
extern const HUFF_ICC_NODES huffICCNodes;
extern const HUFF_CPC_NODES huffCPCNodes;
extern const HUFF_OLD_NODES huffOLDNodes;
extern const HUFF_NRG_NODES huffNRGNodes;

extern const HUFF_PT0_NODES huffPart0Nodes;
extern const HUFF_LAV_NODES huffLavIdxNodes;


static int pcm_decode( HANDLE_S_BITINPUT strm,
                       int*    out_data_1,
                       int*    out_data_2,
                       int     offset,
                       int     num_val,
                       int     num_levels )
{
  int i = 0, j = 0, idx = 0;
  int max_grp_len = 0, grp_len = 0, next_val = 0, grp_val = 0;
  unsigned long data = 0;

  float ld_nlev = 0.f;

  int pcm_chunk_size[7] = { 0 };


  switch( num_levels )
  {
    case  3: max_grp_len = 5; break;
    case  5: max_grp_len = 3; break;
    case  6: max_grp_len = 5; break;
    case  7: max_grp_len = 6; break;
    case  9: max_grp_len = 5; break;
    case 11: max_grp_len = 2; break;
    case 13: max_grp_len = 4; break;
    case 19: max_grp_len = 4; break;
    case 25: max_grp_len = 3; break;
    case 51: max_grp_len = 4; break;
    default: max_grp_len = 1;
  }

  ld_nlev  = (float)log( (float)num_levels );
  ld_nlev /= (float)log(2.f);

  for( i=1; i<=max_grp_len; i++ ) {
    pcm_chunk_size[i] = (int) ceil( (float)(i) * ld_nlev );
  }


  for( i=0; i<num_val; i+=max_grp_len ) {
    grp_len = min( max_grp_len, num_val-i );
    if( ERROR_NONE != s_GetBits2(strm, pcm_chunk_size[grp_len], &data) ) return ERROR_PARSING_BITSTREAM;

    grp_val = data;

    for( j=0; j<grp_len; j++ ) {
      idx = i+(grp_len-j-1);
      next_val = grp_val%num_levels;

      if( out_data_2 == NULL ) {
        out_data_1[idx] = next_val - offset;
      }
      else if( out_data_1 == NULL ) {
        out_data_2[idx] = next_val - offset;
      }
      else {
        if(idx%2) {
          out_data_2[idx/2] = next_val - offset;
        }
        else {
          out_data_1[idx/2] = next_val - offset;
        }
      }

      grp_val = (grp_val-next_val)/num_levels;
    }
  }

  return ERROR_NONE;
}


static int huff_read( HANDLE_S_BITINPUT     strm,
                      const int   (*nodeTab)[][2],
                      int*        out_data )
{
  int node = 0;
  unsigned long next_bit = 0;

  do {
    if( ERROR_NONE != s_GetBits2(strm, 1, &next_bit) ) return ERROR_PARSING_BITSTREAM;
    node = (*nodeTab)[node][next_bit];
  } while( node > 0 );

  *out_data = node;

  return ERROR_NONE;
}


static int huff_read_2D( HANDLE_S_BITINPUT          strm,
                         const int        (*nodeTab)[][2],
                         int              out_data[2],
                         int*             escape )

{
  int huff_2D_8bit = 0;
  int node = 0;

  if( ERROR_NONE != huff_read(strm, nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  *escape = (node == 0);

  if( *escape ) {
    out_data[0] = 0;
    out_data[1] = 1;
  }
  else {
    huff_2D_8bit = -(node+1);
    out_data[0] = huff_2D_8bit >> 4;
    out_data[1] = huff_2D_8bit & 0xf;
  }

  return ERROR_NONE;
}


static int sym_restore( HANDLE_S_BITINPUT strm,
                        int     lav,
                        int     data[2] )
{
  int tmp = 0;
  unsigned long sym_bit = 0;

  int sum_val  = data[0]+data[1];
  int diff_val = data[0]-data[1];

  if( sum_val > lav ) {
    data[0] = - sum_val + (2*lav+1);
    data[1] = - diff_val;
  }
  else {
    data[0] = sum_val;
    data[1] = diff_val;
  }

  if( data[0]+data[1] != 0 ) {
    if( ERROR_NONE != s_GetBits2(strm, 1, &sym_bit) ) return ERROR_PARSING_BITSTREAM;
    if( sym_bit ) {
      data[0] = -data[0];
      data[1] = -data[1];
    }
  }

  if( data[0]-data[1] != 0 ) {
    if( ERROR_NONE != s_GetBits2(strm, 1, &sym_bit) ) return ERROR_PARSING_BITSTREAM;
    if( sym_bit ) {
      tmp     = data[0];
      data[0] = data[1];
      data[1] = tmp;
    }
  }

  return ERROR_NONE;
}


static int huff_dec_cld_1D( HANDLE_S_BITINPUT                strm,
                            const HUFF_CLD_NOD_1D* huffNodes,
                            int*                   out_data,
                            int                    num_val,
                            int                    p0_flag )
{
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  unsigned long data = 0;
  
  if( p0_flag ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cld, &node) ) return ERROR_PARSING_BITSTREAM;
    out_data[0] = -(node+1);
    offset = 1;
  }
  
  for( i=offset; i<num_val; i++ ) {
    
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffNodes->nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
    od = -(node+1);
    
    if( od != 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
      od_sign = data;
      
      if( od_sign ) od = -od;
    }
    
    out_data[i] = od;
  }
  
  return ERROR_NONE;
}


static int huff_dec_icc_1D( HANDLE_S_BITINPUT                strm,
                            const HUFF_ICC_NOD_1D* huffNodes,
                            int*                   out_data,
                            int                    num_val,
                            int                    p0_flag )
{
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  unsigned long data = 0;
  
  if( p0_flag ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.icc, &node) ) return ERROR_PARSING_BITSTREAM;
    out_data[0] = -(node+1);
    offset = 1;
  }
  
  for( i=offset; i<num_val; i++ ) {
    
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffNodes->nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
    od = -(node+1);
    
    if( od != 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
      od_sign = data;
      
      if( od_sign ) od = -od;
    }
    
    out_data[i] = od;
  }
  
  return ERROR_NONE;
}


static int huff_dec_cpc_1D( HANDLE_S_BITINPUT                strm,
                            const HUFF_CPC_NOD_1D* huffNodes,
                            int*                   out_data,
                            int                    num_val,
                            int                    p0_flag )
{
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  unsigned long data = 0;
  
  if( p0_flag ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cpc, &node) ) return ERROR_PARSING_BITSTREAM;
    out_data[0] = -(node+1);
    offset = 1;
  }
  
  for( i=offset; i<num_val; i++ ) {
    
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffNodes->nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
    od = -(node+1);
    
    if( od != 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
      od_sign = data;
      
      if( od_sign ) od = -od;
    }
    
    out_data[i] = od;
  }
  
  return ERROR_NONE;
}


static int huff_dec_old_1D( HANDLE_S_BITINPUT                strm,
                            const HUFF_OLD_NOD_1D* huffNodes,
                            int*                   out_data,
                            int                    num_val,
                            int                    p0_flag )
{
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  unsigned long data = 0;
  
  if( p0_flag ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.old, &node) ) return ERROR_PARSING_BITSTREAM;
    out_data[0] = -(node+1);
    offset = 1;
  }
  
  for( i=offset; i<num_val; i++ ) {
    
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffNodes->nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
    od = -(node+1);
    
    if( od != 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
      od_sign = data;
      
      if( od_sign ) od = -od;
    }
    
    out_data[i] = od;
  }
  
  return ERROR_NONE;
}


static int huff_dec_nrg_1D( HANDLE_S_BITINPUT                strm,
                            const HUFF_NRG_NOD_1D* huffNodes,
                            int*                   out_data,
                            int                    num_val,
                            int                    p0_flag )
{
  int i = 0, node = 0, offset = 0;
  int od = 0, od_sign = 0;
  unsigned long data = 0;
  
  if( p0_flag ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.nrg, &node) ) return ERROR_PARSING_BITSTREAM;
    out_data[0] = -(node+1);
    offset = 1;
  }
  
  for( i=offset; i<num_val; i++ ) {
    
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffNodes->nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
    od = -(node+1);
    
    if( od != 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
      od_sign = data;
      
      if( od_sign ) od = -od;
    }
    
    out_data[i] = od;
  }
  
  return ERROR_NONE;
}


static int huff_dec_cld_2D( HANDLE_S_BITINPUT                strm,
                            const HUFF_CLD_NOD_2D* huffNodes,
                            int                    out_data[][2],
                            int                    num_val,
                            int                    stride,
                            int*                   p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)][2] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 2*data + 3;  
  
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cld, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cld, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 5:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav5, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 7:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav7, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 9:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav9, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    default:
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_dec_icc_2D( HANDLE_S_BITINPUT                strm,
                            const HUFF_ICC_NOD_2D* huffNodes,
                            int                    out_data[][2],
                            int                    num_val,
                            int                    stride,
                            int*                   p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 2*data + 1;  
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.icc, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.icc, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 1:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav1, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 5:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav5, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 7:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav7, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_dec_cpc_2D( HANDLE_S_BITINPUT                strm,
                            const HUFF_CPC_NOD_2D* huffNodes,
                            int                    out_data[][2],
                            int                    num_val,
                            int                    stride,
                            int*                   p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 3*data + 3;
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cpc, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.cpc, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3 , out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 6:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav6 , out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 9:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav9 , out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 12:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav12, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_dec_old_2D( HANDLE_S_BITINPUT                strm,
                            const HUFF_OLD_NOD_2D* huffNodes,
                            int                    out_data[][2],
                            int                    num_val,
                            int                    stride,
                            int*                   p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 3*data + 3;  
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.old, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.old, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 6:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav6, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 9:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav9, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 12:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav12, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_dec_nrg_2D_df( HANDLE_S_BITINPUT                   strm,
                               const HUFF_NRG_NOD_2D_df* huffNodes,
                               int                       out_data[][2],
                               int                       num_val,
                               int                       stride,
                               int*                      p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 2*data + 3;  
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.nrg, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.nrg, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 5:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav5, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 7:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav7, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 9:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav9, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_dec_nrg_2D_dt( HANDLE_S_BITINPUT                   strm,
                               const HUFF_NRG_NOD_2D_dt* huffNodes,
                               int                       out_data[][2],
                               int                       num_val,
                               int                       stride,
                               int*                      p0_data[2] )
{
  int i = 0, lav = 0, escape = 0, escCntr = 0;
  int node = 0;
  unsigned long data = 0;
  
  int esc_data[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int escIdx[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffLavIdxNodes.nodeTab, &node) ) return ERROR_PARSING_BITSTREAM;
  data = -(node+1);
  
  lav = 3*data + 3;  
  
  if( p0_data[0] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.nrg, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[0] = -(node+1);
  }
  if( p0_data[1] != NULL ) {
    if( ERROR_NONE != huff_read(strm, (HANDLE_HUFF_NODE)&huffPart0Nodes.nrg, &node) ) return ERROR_PARSING_BITSTREAM;
    *p0_data[1] = -(node+1);
  }
  
  for( i=0; i<num_val; i+=stride ) {
    
    switch( lav ) {
    case 3:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav3, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 6:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav6, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 9:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav9, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    case 12:
      if( ERROR_NONE != huff_read_2D(strm, (HANDLE_HUFF_NODE)&huffNodes->lav12, out_data[i], &escape) ) return ERROR_PARSING_BITSTREAM;
      break;
    }
    
    if( escape ) {
      escIdx[escCntr++] = i;
    }
    else {
      if( ERROR_NONE != sym_restore(strm, lav, out_data[i]) ) return ERROR_PARSING_BITSTREAM;
    }
    
  }
  
  if( escCntr > 0 ) {
    if( ERROR_NONE != pcm_decode(strm, esc_data[0], esc_data[1], 0, 2*escCntr, (2*lav+1)) ) return ERROR_PARSING_BITSTREAM;
    
    for( i=0; i<escCntr; i++ ) {
      out_data[escIdx[i]][0] = esc_data[0][i] - lav;
      out_data[escIdx[i]][1] = esc_data[1][i] - lav;
    }
  }
  
  return ERROR_NONE;
}


static int huff_decode( HANDLE_S_BITINPUT         strm,
                        int*            out_data_1,
                        int*            out_data_2,
                        DATA_TYPE       data_type,
                        DIFF_TYPE       diff_type_1,
                        DIFF_TYPE       diff_type_2,
                        int             num_val,
                        CODING_SCHEME   *cdg_scheme )
{
 
  int i = 0;
  unsigned long data = 0;
  
  int pair_vec[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)][2];
  
  int* p0_data_1[2] = {NULL, NULL};
  int* p0_data_2[2] = {NULL, NULL};
  
  int p0_flag[2];
  
  int num_val_1_int = num_val;
  int num_val_2_int = num_val;
  
  int* out_data_1_int = out_data_1;
  int* out_data_2_int = out_data_2;
  
  int df_rest_flag_1 = 0;
  int df_rest_flag_2 = 0;
  
  int hufYY1;
  int hufYY2;
 
  if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
  *cdg_scheme = data << PAIR_SHIFT;
  
  if( *cdg_scheme >> PAIR_SHIFT == HUFF_2D ) {
      *cdg_scheme |= FREQ_PAIR;
  }
  
  
    hufYY1 = diff_type_1;
    hufYY2 = diff_type_2;
 
  
  switch( *cdg_scheme >> PAIR_SHIFT ) {
  case HUFF_1D:
    
    p0_flag[0] = (diff_type_1 == DIFF_FREQ); 
    p0_flag[1] = (diff_type_2 == DIFF_FREQ); 
    
    switch( data_type ) {
    case t_CLD:
      if( out_data_1 != NULL ) {
        if( ERROR_NONE != huff_dec_cld_1D(strm, &huffCLDNodes.h1D[hufYY1], out_data_1, num_val_1_int, p0_flag[0]) ) return ERROR_PARSING_BITSTREAM;
      }
      if( out_data_2 != NULL ) {
        if( ERROR_NONE != huff_dec_cld_1D(strm, &huffCLDNodes.h1D[hufYY2], out_data_2, num_val_2_int, p0_flag[1]) ) return ERROR_PARSING_BITSTREAM;
      }
      
      break;
      
    case t_ICC:
      if( out_data_1 != NULL ) {
        if( ERROR_NONE != huff_dec_icc_1D(strm, &huffICCNodes.h1D[hufYY1], out_data_1, num_val_1_int, p0_flag[0]) ) return ERROR_PARSING_BITSTREAM;
      }
      if( out_data_2 != NULL ) {
        if( ERROR_NONE != huff_dec_icc_1D(strm, &huffICCNodes.h1D[hufYY2], out_data_2, num_val_2_int, p0_flag[1]) ) return ERROR_PARSING_BITSTREAM;
      }
      
      break;
      
    case t_CPC:
      if( out_data_1 != NULL ) {
        if( ERROR_NONE != huff_dec_cpc_1D(strm, &huffCPCNodes.h1D[hufYY1], out_data_1, num_val_1_int, p0_flag[0]) ) return ERROR_PARSING_BITSTREAM;
      }
      if( out_data_2 != NULL ) {
        if( ERROR_NONE != huff_dec_cpc_1D(strm, &huffCPCNodes.h1D[hufYY2], out_data_2, num_val_2_int, p0_flag[1]) ) return ERROR_PARSING_BITSTREAM;
      }
      
      break;
      
    case t_OLD:
      if( out_data_1 != NULL ) {
        if( ERROR_NONE != huff_dec_old_1D(strm, &huffOLDNodes.h1D[hufYY1], out_data_1, num_val_1_int, p0_flag[0]) ) return ERROR_PARSING_BITSTREAM;
      }
      if( out_data_2 != NULL ) {
        if( ERROR_NONE != huff_dec_old_1D(strm, &huffOLDNodes.h1D[hufYY2], out_data_2, num_val_2_int, p0_flag[1]) ) return ERROR_PARSING_BITSTREAM;
      }
      
      break;
      
    case t_NRG:
      if( out_data_1 != NULL ) {
        if( ERROR_NONE != huff_dec_nrg_1D(strm, &huffNRGNodes.h1D[hufYY1], out_data_1, num_val_1_int, p0_flag[0]) ) return ERROR_PARSING_BITSTREAM;
      }
      if( out_data_2 != NULL ) {
        if( ERROR_NONE != huff_dec_nrg_1D(strm, &huffNRGNodes.h1D[hufYY2], out_data_2, num_val_2_int, p0_flag[1]) ) return ERROR_PARSING_BITSTREAM;
      }
      
      break;

    default:
      break;
    }
    
    break;
    
  case HUFF_2D:
    
   
      if( out_data_1 != NULL ) {
    if(diff_type_1 == DIFF_FREQ ) {
          p0_data_1[0] = &out_data_1[0];
          p0_data_1[1] = NULL;
          
          num_val_1_int  -= 1;
          out_data_1_int += 1;
        }
        df_rest_flag_1 = num_val_1_int % 2;
        if( df_rest_flag_1 ) num_val_1_int -= 1;
      }
      if( out_data_2 != NULL ) {
   if(diff_type_2 == DIFF_FREQ ) {
          p0_data_2[0] = NULL;
          p0_data_2[1] = &out_data_2[0];
          
          num_val_2_int  -= 1;
          out_data_2_int += 1;
        }
        df_rest_flag_2 = num_val_2_int % 2;
        if( df_rest_flag_2 ) num_val_2_int -= 1;
      }
      
      switch( data_type ) {
      case t_CLD:
        
        if( out_data_1 != NULL ) {
          if( ERROR_NONE != huff_dec_cld_2D(strm, &huffCLDNodes.h2D[hufYY1][FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_1 ) {
            if( ERROR_NONE != huff_dec_cld_1D(strm, &huffCLDNodes.h1D[hufYY1], out_data_1_int+num_val_1_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        if( out_data_2 != NULL ) {
          if( ERROR_NONE != huff_dec_cld_2D(strm, &huffCLDNodes.h2D[hufYY2][FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_2 ) {
            if( ERROR_NONE != huff_dec_cld_1D(strm, &huffCLDNodes.h1D[hufYY2], out_data_2_int+num_val_2_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        break;
        
      case t_ICC:
        if( out_data_1 != NULL ) {
          if( ERROR_NONE != huff_dec_icc_2D(strm, &huffICCNodes.h2D[hufYY1][FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_1 ) {
            if( ERROR_NONE != huff_dec_icc_1D(strm, &huffICCNodes.h1D[hufYY1], out_data_1_int+num_val_1_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        if( out_data_2 != NULL ) {
          if( ERROR_NONE != huff_dec_icc_2D(strm, &huffICCNodes.h2D[hufYY2][FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_2 ) {
            if( ERROR_NONE != huff_dec_icc_1D(strm, &huffICCNodes.h1D[hufYY2], out_data_2_int+num_val_2_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        break;
        
      case t_CPC:
        if( out_data_1 != NULL ) {
          if( ERROR_NONE != huff_dec_cpc_2D(strm, &huffCPCNodes.h2D[hufYY1][FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_1 ) {
            if( ERROR_NONE != huff_dec_cpc_1D(strm, &huffCPCNodes.h1D[hufYY1], out_data_1_int+num_val_1_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        if( out_data_2 != NULL ) {
          if( ERROR_NONE != huff_dec_cpc_2D(strm, &huffCPCNodes.h2D[hufYY2][FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_2 ) {
            if( ERROR_NONE != huff_dec_cpc_1D(strm, &huffCPCNodes.h1D[hufYY2], out_data_2_int+num_val_2_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        break;
        
      case t_OLD:
        if( out_data_1 != NULL ) {
          if( ERROR_NONE != huff_dec_old_2D(strm, &huffOLDNodes.h2D[hufYY1][FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_1 ) {
            if( ERROR_NONE != huff_dec_old_1D(strm, &huffOLDNodes.h1D[hufYY1], out_data_1_int+num_val_1_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        if( out_data_2 != NULL ) {
          if( ERROR_NONE != huff_dec_old_2D(strm, &huffOLDNodes.h2D[hufYY2][FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
          if( df_rest_flag_2 ) {
            if( ERROR_NONE != huff_dec_old_1D(strm, &huffOLDNodes.h1D[hufYY2], out_data_2_int+num_val_2_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        break;
        
      case t_NRG:
        if( out_data_1 != NULL ) {
          switch( hufYY1 ) {
          case DIFF_FREQ:
            if( ERROR_NONE != huff_dec_nrg_2D_df(strm, &huffNRGNodes.h2D.df[FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
            break;
          case DIFF_TIME:
            if( ERROR_NONE != huff_dec_nrg_2D_dt(strm, &huffNRGNodes.h2D.dt[FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
            break;
          default:
            if( ERROR_NONE != huff_dec_nrg_2D_df(strm, &huffNRGNodes.h2D.dp[FREQ_PAIR], pair_vec  , num_val_1_int, 2, p0_data_1) ) return ERROR_PARSING_BITSTREAM;
            break;
          }
          if( df_rest_flag_1 ) {
            if( ERROR_NONE != huff_dec_nrg_1D(strm, &huffNRGNodes.h1D[hufYY1], out_data_1_int+num_val_1_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        if( out_data_2 != NULL ) {
          switch( hufYY2 ) {
          case DIFF_FREQ:
            if( ERROR_NONE != huff_dec_nrg_2D_df(strm, &huffNRGNodes.h2D.df[FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
            break;
          case DIFF_TIME:
            if( ERROR_NONE != huff_dec_nrg_2D_dt(strm, &huffNRGNodes.h2D.dt[FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
            break;
          default:
            if( ERROR_NONE != huff_dec_nrg_2D_df(strm, &huffNRGNodes.h2D.dp[FREQ_PAIR], pair_vec+1, num_val_2_int, 2, p0_data_2) ) return ERROR_PARSING_BITSTREAM;
            break;
          }
          if( df_rest_flag_2 ) {
            if( ERROR_NONE != huff_dec_nrg_1D(strm, &huffNRGNodes.h1D[hufYY2], out_data_2_int+num_val_2_int, 1, 0) ) return ERROR_PARSING_BITSTREAM;
          }
        }
        break;

      default:
        break;
      }
      
      if( out_data_1 != NULL ) {
        for( i=0; i<num_val_1_int-1; i+=2 ) {
          out_data_1_int[i  ] = pair_vec[i][0];
          out_data_1_int[i+1] = pair_vec[i][1];
        }
      }
      if( out_data_2 != NULL ) {
        for( i=0; i<num_val_2_int-1; i+=2 ) {
          out_data_2_int[i  ] = pair_vec[i+1][0];
          out_data_2_int[i+1] = pair_vec[i+1][1];
        }
      }
            
    
    break;
    
  default:
    break;
  }
  
  return ERROR_NONE;
}


static void diff_freq_decode( int* diff_data,
                              int* out_data,
                              int  num_val )
{
  int i = 0;
  
  out_data[0] = diff_data[0];
  
  for( i=1; i<num_val; i++ ) {
    out_data[i] = out_data[i-1] + diff_data[i];
  }
}


static void diff_time_decode_backwards( int* prev_data,
                                        int* diff_data,
                                        int* out_data,
                                        int  mixed_diff_type,
                                        int  num_val )
{
  int i = 0;
  
  if( mixed_diff_type ) {
    out_data[0] = diff_data[0];
    for( i=1; i<num_val; i++ ) {
      out_data[i] = prev_data[i] + diff_data[i];
    }
  }
  else {
    for( i=0; i<num_val; i++ ) {
      out_data[i] = prev_data[i] + diff_data[i];
    }
  }
}


static int attach_lsb( HANDLE_S_BITINPUT               strm,
                       int*                  in_data_msb,
                       int                   offset,
                       int                   num_lsb,
                       int                   num_val,
                       int*                  out_data )
{
  int i = 0, lsb = 0, msb = 0;
  unsigned long data = 0;
  
  for( i=0; i<num_val; i++ ) {
    
    msb = in_data_msb[i];
    
    if( num_lsb > 0 ) {
      if( ERROR_NONE != s_GetBits2(strm, num_lsb, &data) ) return ERROR_PARSING_BITSTREAM;
      lsb = data;
      
      out_data[i] = ((msb << num_lsb) | lsb) - offset;
    }
    else out_data[i] = msb - offset;
    
  }
  
  return ERROR_NONE;
}


int EcDataPairDec( HANDLE_S_BITINPUT    strm,
                   short      aaOutData[][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)],
                   short      aHistory[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)],
                   DATA_TYPE  data_type,
				   int        setIdx,
                   int        dataBands,
                   int        pair_flag,
                   int        coarse_flag,
                   int        independency_flag )
     
{
  int allowDiffTimeBack_flag = !independency_flag || (setIdx>0);
  int attachLsb_flag = 0;
  int pcmCoding_flag = 0;
  int mixed_time_pair = 0, numValPcm = 0;
  int quant_levels = 0, quant_offset = 0;
  unsigned long data = 0;
  
  int aaDataPair[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  int aaDataDiff[2][max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {{0}};
  
  int aHistoryMsb[max(SAOC_MAX_NUM_BINS,SAOC_MAX_OBJECTS)] = {0};
  
  int* pDataVec[2] = { NULL, NULL };
  
  
  DIFF_TYPE     diff_type[2] = { DIFF_FREQ, DIFF_FREQ };
  CODING_SCHEME cdg_scheme   = HUFF_1D;
  DIRECTION     direction    = BACKWARDS;
  
  
  switch( data_type ) {
  case t_CLD:
    if( coarse_flag ) {
      attachLsb_flag   =  0;
      quant_levels     = 15;
      quant_offset     =  7;
    }
    else {
      attachLsb_flag   =  0;
      quant_levels     = 31;
      quant_offset     = 15;
    }
    
    break;
    
  case t_ICC:
    if( coarse_flag ) {
      attachLsb_flag   =  0;
      quant_levels     =  4;
      quant_offset     =  0;
    }
    else {
      attachLsb_flag   =  0;
      quant_levels     =  8;
      quant_offset     =  0;
    }
    
    break;
    
  case t_CPC:
    if( coarse_flag ) {
      attachLsb_flag   =  0;
      quant_levels     = 26;
      quant_offset     = 10;
    }
    else {
      attachLsb_flag   =  1;
      quant_levels     = 51;
      quant_offset     = 20;
    }
    
    break;
    
  case t_OLD:
    if( coarse_flag ) {
	attachLsb_flag   =  0;
	quant_levels     =  8;
	quant_offset     =  0;
    }
    else {
      attachLsb_flag   =  0;
      quant_levels     =  16;
      quant_offset     =  0;
    }
    
    break;

  case t_NRG:
    if( coarse_flag ) {
      attachLsb_flag   =  0;
      quant_levels     = 32;
      quant_offset     =  0;
    }
    else {
      attachLsb_flag   =  0;
      quant_levels     = 64;
      quant_offset     =  0;
   }
    
    break;

  default:
    fprintf( stderr, "Unknown type of data!\n" );
    return ERROR_PARSING_BITSTREAM;
  }
  
  if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
  pcmCoding_flag = data;

 if( pcmCoding_flag) {
    
    if( pair_flag ) {
      pDataVec[0] = aaDataPair[0];
      pDataVec[1] = aaDataPair[1];
      numValPcm   = 2*dataBands;
    }
    else {
      pDataVec[0] = aaDataPair[0];
      pDataVec[1] = NULL;
      numValPcm   = dataBands;
    }
    
    if( ERROR_NONE != pcm_decode( strm,
		     pDataVec[0],
		     pDataVec[1],
		     quant_offset,
		     numValPcm,
		     quant_levels ) ) return ERROR_PARSING_BITSTREAM;	
    
  }
  else {
    
    if( pair_flag ) {
      pDataVec[0] = aaDataDiff[0];
      pDataVec[1] = aaDataDiff[1];
    }
    else {
      pDataVec[0] = aaDataDiff[0];
      pDataVec[1] = NULL;
    }
    
    diff_type[0] = DIFF_FREQ;
    diff_type[1] = DIFF_FREQ;
    
    direction = BACKWARDS;
    
	if( pair_flag || allowDiffTimeBack_flag ) {
		if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
		diff_type[0] = data;
	}
	
	if( pair_flag && ((diff_type[0] == DIFF_FREQ) || allowDiffTimeBack_flag) ) {
		if( ERROR_NONE != s_GetBits2(strm, 1, &data) ) return ERROR_PARSING_BITSTREAM;
		diff_type[1] = data;
	}
    
    if( ERROR_NONE != huff_decode( strm,
		      pDataVec[0],
		      pDataVec[1],
		      data_type,
		      diff_type[0],
		      diff_type[1],
		      dataBands,
		      &cdg_scheme )
        )
      {
        return ERROR_PARSING_BITSTREAM;
      }
    
      mixed_time_pair = 0; 
      
        if( diff_type[0] == DIFF_FREQ || !allowDiffTimeBack_flag ) {
          diff_freq_decode( aaDataDiff[0], aaDataPair[0], dataBands );
        }
        else {
          int i;
          for( i=0; i<dataBands; i++ ) {
            aHistoryMsb[i] = aHistory[i] + quant_offset;
            if( attachLsb_flag ) {
              aHistoryMsb[i] >>= 1;
            }
          }
          diff_time_decode_backwards( aHistoryMsb, aaDataDiff[0], aaDataPair[0], mixed_time_pair, dataBands );
        }
        if( diff_type[1] == DIFF_FREQ ) {
          diff_freq_decode( aaDataDiff[1], aaDataPair[1], dataBands );
        }
        else {
          diff_time_decode_backwards( aaDataPair[0], aaDataDiff[1], aaDataPair[1], mixed_time_pair, dataBands );
        }
    
  
    
    attach_lsb( strm, aaDataPair[0], quant_offset, attachLsb_flag ? 1 : 0, dataBands, aaDataPair[0] );
    if( pair_flag ) {
      attach_lsb( strm, aaDataPair[1], quant_offset, attachLsb_flag ? 1 : 0, dataBands, aaDataPair[1] );
    }
    
  } 

  {
    int pb=0;

    for( pb=0; pb<dataBands; pb++ ) {
      aaOutData[0][pb] = aaDataPair[0][pb];
      if( pair_flag ) {
        aaOutData[1][pb] = aaDataPair[1][pb];
      }
    }
  }
 
  return ERROR_NONE;
}
