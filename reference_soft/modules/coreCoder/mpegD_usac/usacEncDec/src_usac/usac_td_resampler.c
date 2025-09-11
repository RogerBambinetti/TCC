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

#include <stdlib.h>
#include <stdio.h>
#include "proto_func.h"
#include "usac_td_resampler.h"

static float filter_interp[L_FILT_MAX+1] = {
 1.00000000f,
 0.63487604f,  0.00000000f,
-0.20701353f, -0.00000000f,
 0.11879487f,  0.00000000f,
-0.07926575f, -0.00000000f,
 0.05615642f,  0.00000000f,
-0.04070711f, -0.00000000f,
 0.02957617f,  0.00000000f,
-0.02122066f, -0.00000000f,
 0.01483115f,  0.00000000f,
-0.00993903f, -0.00000000f,
 0.00624819f,  0.00000000f,
-0.00355476f, -0.00000000f,
 0.00170582f, -0.00000000f,
-0.00057701f, -0.00000000f,
 0.00006013f,  0.00000000f,
};

/*-------------------------------------------------------------------------
 * TD_resampler()
 * Function for resampling of signals in time domain with FIRs
 *-------------------------------------------------------------------------*/
void TD_resampler(                /* o  : length of output signal                                  */
      const float pInSignal[],    /* i  : signal to resample or filter                             */
            short numSamplesIn,   /* i  : length of input + 0 delay signaling                      */
        const int srIn,           /* i  : samplerate of input                                      */
            float pOutSignal[],   /* o  : resampled  or filtered signal                            */
        const int srOut,          /* i  : samplerate of output                                     */
            float filterMemory[], /* i/o: filter memory                                            */
              int just_filter     /* i  : only the interpolation filter is applied, no resampling  */
)
{
  short i, j, step_size, n=0;
  short numSamplesOut;
  float signal[2*L_FILT_MAX + L_FRAME_MAX];

  if(srIn<srOut) /* upsample ratio of 1:2 */
  {
    /* set resampled signal length */
    numSamplesOut = numSamplesIn * 2;

    /* append filter memory */
    if (filterMemory) {
      mvr2r( filterMemory, signal, UPSAMP_FILT_MEM_SIZE );
    } else if (UPSAMP_FILT_MEM_SIZE/2 < numSamplesIn) {
      for (i = 0; i < UPSAMP_FILT_MEM_SIZE/2; i++) {
        signal[UPSAMP_FILT_MEM_SIZE/2 -1 -  i] = 2 * pInSignal[0] - pInSignal[i + 1];
        signal[numSamplesIn+UPSAMP_FILT_MEM_SIZE/2 + i] = 2 * pInSignal[numSamplesIn - 1] - pInSignal[numSamplesIn - 2 - i];
      }
    }

    /* feed in input signal */
    if(filterMemory) {
      mvr2r( pInSignal, signal + UPSAMP_FILT_MEM_SIZE, numSamplesIn );
    } else {
      mvr2r( pInSignal, signal + UPSAMP_FILT_MEM_SIZE/2, numSamplesIn );
    }

    /* interpolate and apply interpolation filter */
    for(i=0; i<numSamplesIn; i++)
    {
      pOutSignal[(i*2)] = signal[i+UPSAMP_FILT_LEN];

      pOutSignal[(i*2)+1] = 0.f;
      for (j=0; j<UPSAMP_FILT_LEN; j++)
      {
        pOutSignal[(i*2)+1] += signal[i+UPSAMP_FILT_LEN-j]   * filter_interp[(j*2)+1] + 
                               signal[i+1+UPSAMP_FILT_LEN+j] * filter_interp[(j*2)+1];
      }
    }

    /* update the filter memory */
    if (filterMemory) {
      mvr2r(signal+numSamplesIn, filterMemory, UPSAMP_FILT_MEM_SIZE);
    }

  }
  else /* downsample ratio of 2:1 or just filter */
  {
    /* set resampled signal length */
    step_size        = (1==just_filter) ? 1 : 2;    /* no decimation is applied if just_filter==1 */ 
    numSamplesOut    = (1==just_filter) ? numSamplesIn : numSamplesIn / 2; 

    /* append filter memory */
    if (filterMemory) {
      mvr2r(filterMemory, signal, DOWNSAMP_FILT_MEM_SIZE);
    } else if (DOWNSAMP_FILT_MEM_SIZE/2 < numSamplesIn) {
      for (i = 0; i < DOWNSAMP_FILT_MEM_SIZE/2; i++) {
        signal[DOWNSAMP_FILT_MEM_SIZE/2 - 1 - i] = 2 * pInSignal[0] - pInSignal[i + 1];
        signal[numSamplesIn+DOWNSAMP_FILT_MEM_SIZE/2 + i] = 2 * pInSignal[numSamplesIn - 1] - pInSignal[numSamplesIn - 2 - i];
      }
    }
    
    /* feed in input signal */
    if (filterMemory) {
      mvr2r( pInSignal, signal + DOWNSAMP_FILT_MEM_SIZE, numSamplesIn );
    } else {
      mvr2r( pInSignal, signal + DOWNSAMP_FILT_MEM_SIZE/2, numSamplesIn );
    }

    /* decimate and apply interpolation filter and rescaling */    
    for(i=0; i<numSamplesOut; i++)
    {
      pOutSignal[i] = 0.f;
      for (j=0; j<DOWNSAMP_FILT_LEN; j++)
      {
        pOutSignal[i] += signal[n+DOWNSAMP_FILT_LEN-j]   * filter_interp[j] + 
                         signal[n+1+DOWNSAMP_FILT_LEN+j] * filter_interp[j+1];
      }
      
      pOutSignal[i] *= 0.5f; 
      n += step_size;
    }

    /* update the filter memory */
    if (0 == just_filter && filterMemory) {
      mvr2r(signal+numSamplesIn, filterMemory, DOWNSAMP_FILT_MEM_SIZE);
    }
  }
}
