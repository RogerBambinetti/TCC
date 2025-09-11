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

#ifndef USAC_TD_RESAMPLER_H__
#define USAC_TD_RESAMPLER_H__

#define L_FRAME_MAX          2048 /* Maximal frame/super-frame length at double rate */
#define L_FILT_MAX             30 /* Maximal delay in samples at maximal sampling rate */
#define UPSAMP_FILT_MEM_SIZE   30
#define DOWNSAMP_FILT_MEM_SIZE 60
#define UPSAMP_FILT_LEN        15
#define DOWNSAMP_FILT_LEN      30

void TD_resampler(                /* o  : length of output signal                                  */
      const float pInSignal[],    /* i  : signal to resample or filter                             */
            short numSamplesIn,   /* i  : length of input + 0 delay signaling                      */
        const int srIn,           /* i  : samplerate of input                                      */
            float pOutSignal[],   /* o  : resampled  or filtered signal                            */
        const int srOut,          /* i  : samplerate of output                                     */
            float filterMemory[], /* i/o: filter memory                                            */
              int just_filter     /* i  : only the interpolation filter is applied, no resampling  */
);

#endif /*USAC_TD_RESAMPLER_H__*/
