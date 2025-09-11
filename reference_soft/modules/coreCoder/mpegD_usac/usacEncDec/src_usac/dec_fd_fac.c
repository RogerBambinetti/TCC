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


#include "interface.h"
#include "cnst.h"


#include "proto_func.h"
#include "table_decl.h"
#include "int3gpp.h"
#include "re8.h"
#include "acelp_plus.h"

#include <math.h>

void decode_fdfac(int   *facPrm, 
                  int   lDiv, 
                  int   lfac, 
                  float *Aq, 
                  float *zir, 
                  float *facDec, 
                  short fullbandLPD,
                  int   stereoLPD,
                  int   fscale,
                  int   fscaleFB,
                  float *facDecFB) {
  float x[LFAC_1024];
  float xn2[2*LFAC_1024+M];
  float gain;
  int i;
  const float *sineWindow;
  float facWindow[2*LFAC_1024];
  float Ap[M+1];
  HANDLE_TCX_MDCT hTcxMdct=NULL;
  int j = 0;
  int lfacFB;
  int rs_delay;
  float tmp_fac[2*LFAC_1024+L_FILT_MAX];

  lfacFB=lfac;
  if(fullbandLPD){
    lfac=lfac/2;
  }

  if (lfac == 32) {
    sineWindow = sineWindow64;
  }
  else if (lfac == 48) {
    sineWindow = sineWindow96;
  }
  else if (lfac == 64) {
    sineWindow = sineWindow128;
  }
  else if (lfac == 96) {
    sineWindow = sineWindow192;
  }
  else {
    sineWindow = sineWindow256;
  }

  if ( Aq != NULL && facDec != NULL ) {
    if(!hTcxMdct) TCX_MDCT_Open(&hTcxMdct);

    /* Build FAC spectrum */
    gain = (float)pow(10.0f, ((float)facPrm[0])/28.0f);
    for ( i = 0 ; i < lfac ; i++ ) {
      x[i] = (float) facPrm[i+1]*gain;
    }

    /* Compute inverse DCT */
    TCX_DCT4_Apply(TCX_MDCT_DCT4_GetHandle(hTcxMdct, lfac), x, xn2);

    /* Apply synthesis filter */
    E_LPC_a_weight(Aq, Ap, GAMMA1, M);

    set_zero(xn2+lfac, lfac);
    E_UTIL_synthesis(Ap, xn2, facDec, 2*lfac, xn2+lfac, 0);

    /* add ACELP Zir, if available and if StereoLPD in FullbandLPD-mode is not used (add it after FACsynth is upsampled) */
    if ( zir != NULL && (stereoLPD == 0 || fullbandLPD == 0) ) {
      for (i=0; i<lfac; i++)
      {
        facWindow[i] = sineWindow[i]*sineWindow[(2*lfac)-1-i];
        facWindow[lfac+i] = 1.0f - (sineWindow[lfac+i]*sineWindow[lfac+i]);
      }
      for (i=0; i<lfac; i++)
      {
        facDec[i] +=   zir[1+(lDiv/2)+i]*facWindow[lfac+i]
                  + zir[1+(lDiv/2)-1-i]*facWindow[lfac-1-i];
      }
    }
    /* get FAC FB by resample FAC+folded ACELP+ZIR for FB */
    if (fullbandLPD == 1)  {
      rs_delay = L_FILT_MAX;
      mvr2r(facDec, tmp_fac, 2*lfac);
      set_zero(tmp_fac+2*lfac, rs_delay/2);
      TD_resampler(tmp_fac, 2*lfac, fscale, facDecFB, 
                   fscaleFB, NULL, 0);

      /* add ACELP Zir, if available and if StereoLPD in FullbandLPD-mode is used (ZIR has been upsampled before StereoLPD-processing) */
      if ( zir != NULL && (stereoLPD == 1 && fullbandLPD == 1) ) {

        int lDivFB = 2*lDiv; /* always fullbandLPD */

        /* build the FAC window */
        switch (lfac)  {

          case 32: sineWindow = sineWindow128; break;
          case 64: sineWindow = sineWindow256; break;
          default: assert(0); 
        }

        /* sanity check for facWindow size */
        if (2*lfacFB > 2*LFAC_1024) {assert(0);} 

        for (i=0; i<lfacFB; i++)
        {
          facWindow[i] = sineWindow[i]*sineWindow[(2*lfacFB)-1-i];
          facWindow[lfacFB+i] = 1.0f - (sineWindow[lfacFB+i]*sineWindow[lfacFB+i]);
        }

        /* apply FAC window to ZIR and folded ACELP synth and add it to FAC signal */
        for (i=0; i<lfacFB; i++)
        {
          facDecFB[i] +=   zir[1+(lDivFB/2)+i]*facWindow[lfacFB+i]
                      + zir[1+(lDivFB/2)-1-i]*facWindow[lfacFB-1-i];
        }
      }
    }
  }

  return;
}

void switch_core_mode(int   lDiv, 
                      int   lfac, 
                      int   fscale,
                      int   fscaleFB,
                      int   applyFAC,
                      float *facDecFB,
                      float *currBuffer,
                      float *prevBuffer,
                      float *tbe_synth) 
{
  int i;
  int lfacFB;
  int rs_delay;
  float TD_mem[2*L_FILT_MAX];
  float tmp_fac[2*LFAC_1024+L_FILT_MAX];
  float LB_buffer[2*L_DIV_1024+2*LFAC_1024];

  rs_delay = L_FILT_MAX;

  /* only used for FullbandLPD -> consider different fac-lengths */
  lfacFB = lfac;
  lfac = lfac/2;

  /* 2) add FAC FB to currBuffer */
  if (applyFAC == 1)  {
  
    addr2r(facDecFB, currBuffer+2*lDiv, currBuffer+2*lDiv, 2*lfacFB);
  }

  mvr2r(prevBuffer, LB_buffer, 2*lDiv + 2*lfacFB);

  /* 3) Filter prevBuffer->currBuffer*/
  mvr2r(currBuffer+2*lDiv, LB_buffer+2*lDiv, 2*lfacFB);
  mvr2r(LB_buffer+2*lDiv-rs_delay, TD_mem, 2*rs_delay);

  TD_resampler(LB_buffer+2*lDiv+rs_delay, rs_delay, fscaleFB, tmp_fac,
               fscale, TD_mem, 1);

  mvr2r(tmp_fac, LB_buffer+2*lDiv, rs_delay);

  /*4) Add TD-BWE*/
  addr2r(LB_buffer+2*lDiv-lDiv, tbe_synth, LB_buffer+2*lDiv-lDiv, (lDiv+rs_delay));

  /*5) x-fading*/
  for (i=0;i<rs_delay;i++) {

    currBuffer[2*lDiv+i] *= (i) / ((float)rs_delay);
    currBuffer[2*lDiv+i] += (LB_buffer[2*lDiv+i] * (rs_delay-i)) / ((float)rs_delay);    
  }
      
  mvr2r(LB_buffer, prevBuffer, 2*lDiv);  
  set_zero(facDecFB, 2*lfacFB);
}