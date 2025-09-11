/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria.
Those intending to use this software module in products are advised that
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2003.

*************************************************************************/

#include <float.h>

#include "ms.h"

#include "common_m4a.h"
#include "allHandles.h"
#include "tf_mainStruct.h"       /* structs */
#include "tf_main.h"

#undef DEBUG_MS

static
void ntt_select_ms(double *spectral_line_vector[MAX_TIME_CHANNELS],
		int          samples_per_window,
		int          num_window_groups,
		const int    window_group_length[MAX_SHORT_WINDOWS],
		const int    sfb_offset[MAX_SCFAC_BANDS+1],
		int          startSfb,
		int          endSfb,
		MSInfo       *msInfo,
		int          sfb_individual,
		int          debugLevel,
    int          chOffset)
{
  int mscounter = 0;
  int lrcounter = 0;

  {
    int sfb;
    int group, win;
    int group_offset;
    int win_offset;
    int ismp;
    double m, s;
    double lp, rp, mp, sp;
    double ratio_lr, ratio_sm;

    /*for (jj=index->last_max_sfb[iscl+1]; jj<index->max_sfb[iscl+1]; jj++) */
    for (sfb=startSfb; sfb<endSfb; sfb++) {
      group_offset = 0;
      for (group=0; group<num_window_groups; group++) {
        lp=rp=mp=sp=0.0;
        for (win=0; win<window_group_length[group]; win++) {
          win_offset = group_offset + samples_per_window*win;
          for (ismp=win_offset+sfb_offset[sfb]; ismp<win_offset+sfb_offset[sfb+1]; ismp++){
            lp += (spectral_line_vector[chOffset+0][ismp]*spectral_line_vector[chOffset+0][ismp]);
            rp += (spectral_line_vector[chOffset+1][ismp]*spectral_line_vector[chOffset+1][ismp]);
            m =  (spectral_line_vector[chOffset+0][ismp]
                 +spectral_line_vector[chOffset+1][ismp]);
            s =  (spectral_line_vector[chOffset+0][ismp]
                 -spectral_line_vector[chOffset+1][ismp]);
            mp += m*m;
            sp += s*s;
          }
        }
        group_offset += samples_per_window * window_group_length[group];
        ratio_lr = ntt_max(lp/(rp+0.1), rp/(lp+0.1));
        ratio_sm = ntt_max(mp/(sp+0.1), sp/(mp+0.1));
	if (ratio_lr > ratio_sm) {
          msInfo->ms_used[group][sfb] = 0;
          lrcounter++;
        } else {
          msInfo->ms_used[group][sfb] = 1;
          mscounter++;
        }
      }
    }
  }
  if (sfb_individual) {
    msInfo->ms_mask = 1;
  } else {
    if (mscounter > lrcounter) msInfo->ms_mask = 2;
    else msInfo->ms_mask = 0;
  }

}

/* ms_select:
    -1: decide on either 0 or 2
    0: disable MS for all sfbs
    1: decide MS for each sfb individually
    2: force MS for all sfbs
*/
void select_ms( double *spectral_line_vector[MAX_TIME_CHANNELS],
		int         blockSizeSamples,
		int         num_window_groups,
		const int   window_group_length[MAX_SHORT_WINDOWS],
		const int   sfb_offset[MAX_SCFAC_BANDS+1],
		int         startSfb,
		int         endSfb,
		MSInfo      *msInfo,
		int         ms_select,
		int         debugLevel,
    int         chOffset)
{
  int ii,jj;
  int allSame = 1, lastChecked;
  int samples_per_window;
  {
    int total_windows = 0;
    for (ii=0; ii<num_window_groups; ii++)
      total_windows += window_group_length[ii];
    samples_per_window = blockSizeSamples/total_windows; 
  }
  
  if ((ms_select==1)||(ms_select==-1)) {
    ntt_select_ms(spectral_line_vector,
		samples_per_window,
		num_window_groups,
		window_group_length,
		sfb_offset,
		startSfb,
		endSfb,
		msInfo,
		(ms_select==1),
		debugLevel,
    chOffset);
  } else {
    msInfo->ms_mask = ms_select;
  }

/*index->last_max_sfb[iscl+1]...index->max_sfb[iscl+1];*/
  lastChecked = msInfo->ms_used[0][startSfb];

  for(ii=0; ii<8; ii++){
    if(msInfo->ms_mask == 2 || msInfo->ms_mask == 3){
      for(jj=startSfb; jj<endSfb; jj++){
        msInfo->ms_used[ii][jj]=1;    /* force ms_mask */
      }
    }
/* for index->ms_mask =1  no optimized encoder
    else if(index->ms_mask == 1){
      for(jj=index->last_max_sfb[iscl+1]; jj<index->max_sfb[iscl+1]; jj++){
        index->msMask[ii][jj]=(jj+ii)%2;
      }
    }*/
    else if(msInfo->ms_mask == 1){
      for(jj=startSfb; jj<endSfb; jj++){
        if (msInfo->ms_used[ii][jj] != lastChecked) allSame=0;
        lastChecked = msInfo->ms_used[ii][jj];
      }
    }
    else if(msInfo->ms_mask == 0){
      for(jj=startSfb; jj<endSfb; jj++){
        msInfo->ms_used[ii][jj]=0;
      }
    }
  }
  if ((msInfo->ms_mask==1)&&(allSame)) msInfo->ms_mask = lastChecked*2;
}


static void MSApplyMatrix(int blockSizeSamples, double *left, double *right)
{
  int i;
  double dtmp;

  for (i=0; i<blockSizeSamples; i++) {
    dtmp = left[i] - right[i];

    left[i] = (left[i] + right[i]) * 0.5;
    right[i] = dtmp * 0.5;
  }
}

static void MSInverseMatrix(int num_samples, double *left, double *right)
{
  int i;
  double dtmp;
	
  for(i = 0; i < num_samples; i++) {
    dtmp = left[i] + right[i];
    
    right[i] = left[i] - right[i];
    left[i] = dtmp;
  }
}

void MSInverse(int num_sfb,
  int *sfb_offset,
  int ms_used[MAX_SHORT_WINDOWS][SFB_NUM_MAX],
  int num_windows,
  int blockSizeSamples,
  double *left,
  double *right)
{
  int win, sfb;
  int start, end=0;
  int win_offset;

  for (win=0; win<num_windows; win++) {
    win_offset = blockSizeSamples*win/num_windows;
    for (sfb=0; sfb<num_sfb; sfb++) {
      start = sfb_offset[sfb] + win_offset;
      end = sfb_offset[sfb+1] + win_offset;
#ifdef DEBUG_MS
      fprintf(stderr,"MSinv: start %i end %i active %i\n",start,end,ms_used[win][sfb]); 
#endif
      if (ms_used[win][sfb])
        MSInverseMatrix(end-start, &left[start], &right[start]);
    }
  }
}

void MSApply(int num_sfb,
  int *sfb_offset,
  int ms_used[MAX_SHORT_WINDOWS][SFB_NUM_MAX],
  int num_windows,
  int blockSizeSamples,
  double *left,
  double *right)
{
  int win, sfb;
  int start, end;
  int win_offset;

  for (win=0; win<num_windows; win++) {
    win_offset = blockSizeSamples*win/num_windows;
    for (sfb=0; sfb<num_sfb; sfb++) {
      start = sfb_offset[sfb] + win_offset;
      end = sfb_offset[sfb+1] + win_offset;
#ifdef DEBUG_MS
      fprintf(stderr,"MS: start %i end %i active %i\n",start,end,ms_used[win][sfb]); 
#endif
      if (ms_used[win][sfb])
        MSApplyMatrix(end-start, &left[start], &right[start]);
    }
  }
}

