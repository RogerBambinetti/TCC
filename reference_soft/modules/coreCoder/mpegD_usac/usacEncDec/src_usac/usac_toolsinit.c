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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allHandles.h"

#include "usac_all.h"                 /* structs */
#include "obj_descr.h"           /* structs */
#include "tf_mainStruct.h"       /* structs */
#include "tns.h"                 /* structs */

#include "usac_allVariables.h"        /* variables */

#include "huffdec3.h"
#include "usac_port.h"


void usac_infoinit ( USAC_SR_Info *sip,
                     int           block_size_samples )
{
  int i, j, k, n, ws;
  const short *sfbands;
  Info *ip;

  /* long block info */
  ip = &only_long_info;
  usac_win_seq_info[ONLY_LONG_SEQUENCE] = ip;
  ip->islong = 1;
  ip->nsbk = 1;
  ip->bins_per_bk =  block_size_samples ;
  ip->longFssGroups = 0;
  for (i=0; i<ip->nsbk; i++) {
    ip->sectbits[i] = LONG_SECT_BITS;
    if (block_size_samples==960) {
      ip->sbk_sfb_top[i] = sip->SFbands960;
      ip->sfb_per_sbk[i] = sip->nsfb960;
    }
    if (block_size_samples==1024) {
      ip->sfb_per_sbk[i] = sip->nsfb1024;
      ip->sbk_sfb_top[i] = sip->SFbands1024;
    }
  }
  ip->sfb_width_short = NULL;
  ip->num_groups = 1;
  ip->group_len[0] = 1;
  ip->group_offs[0] = 0;

  /* short block info */
  ip = &eight_short_info;
  usac_win_seq_info[EIGHT_SHORT_SEQUENCE] = ip;
  ip->islong = 0;
  ip->nsbk = NSHORT;
  ip->bins_per_bk = block_size_samples;
  ip->shortFssWidth = 0;
  for (i=0; i<ip->nsbk; i++) {
    ip->sectbits[i] = SHORT_SECT_BITS;
    if (block_size_samples==960) {
      ip->sbk_sfb_top[i] = sip->SFbands120;
      ip->sfb_per_sbk[i] = sip->nsfb120;
    }
    if (block_size_samples==1024) {
      ip->sbk_sfb_top[i] = sip->SFbands128;
      ip->sfb_per_sbk[i] = sip->nsfb128;
    }
  }
  /* construct sfb width table */
  ip->sfb_width_short = decdata_sfbwidthShort;
  for (i=0, j=0, n=ip->sfb_per_sbk[0]; i<n; i++) {
    k = ip->sbk_sfb_top[0][i];
    ip->sfb_width_short[i] = k - j; /*  insure: writing array out of range  */
    j = k;
  }

  /* common to long and short */
  for (ws=0; ws<NUM_WIN_SEQ; ws++) {
    if ((ip = usac_win_seq_info[ws]) == NULL)
      continue;
    ip->sfb_per_bk = 0;
    k = 0;
    n = 0;
    for (i=0; i<ip->nsbk; i++) {
      /* compute bins_per_sbk */
      ip->bins_per_sbk[i] = ip->bins_per_bk / ip->nsbk;

      /* compute sfb_per_bk */
      ip->sfb_per_bk += ip->sfb_per_sbk[i];

      /* construct default (non-interleaved) bk_sfb_top[] */
      sfbands = ip->sbk_sfb_top[i];
      for (j=0; j < ip->sfb_per_sbk[i]; j++)
        ip->bk_sfb_top[j+k] = sfbands[j] + n;

      n += ip->bins_per_sbk[i];
      k += ip->sfb_per_sbk[i];
    }

    if (debug['I']) {
      fprintf(stderr,"\nsampling rate %d\n", sip->samp_rate);
      fprintf(stderr,"win_info\t%d has %d windows\n", ws, ip->nsbk);
      fprintf(stderr,"\tbins_per_bk\t%d\n", ip->bins_per_bk);
      fprintf(stderr,"\tsfb_per_bk\t%d\n", ip->sfb_per_bk);
      for (i=0; i<ip->nsbk; i++) {
        fprintf(stderr,"window\t%d\n", i);
        fprintf(stderr,"\tbins_per_sbk\t%d\n", ip->bins_per_sbk[i]);
        fprintf(stderr,"\tsfb_per_sbk	%d\n", ip->sfb_per_sbk[i]);
      }
      if (ip->sfb_width_short != NULL) {
        fprintf(stderr,"sfb top and width\n");
        for (i=0; i<ip->sfb_per_sbk[0]; i++)
          fprintf(stderr,"%6d %6d %6d\n", i,
                  ip->sbk_sfb_top[0][i],
                  ip->sfb_width_short[i]);
      }
    }
  }
}
