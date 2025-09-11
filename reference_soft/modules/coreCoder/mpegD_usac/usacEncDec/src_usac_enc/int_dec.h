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

#ifndef _INT_DEC_H
#define _INT_DEC_H


#include "interface.h"
#include "tf_mainHandle.h"
#include "ms.h"
#include "tns3.h"
#include "usac_interface.h"

/*#define INTDEC_WAVEOUT*/

#ifdef INTDEC_WAVEOUT
#include "audio.h"
#endif



typedef struct T_TFDEC* HANDLE_TFDEC;

void CreateIntDec(HANDLE_TFDEC *phTfDEC, unsigned int nChannels, unsigned int nSamplingRate, unsigned int nGranuleLength);

void AdvanceIntDecUSAC(HANDLE_TFDEC           htfDec,
                       double                *reconstructed_spectrum[MAX_TIME_CHANNELS],
                       WINDOW_SEQUENCE        windowSequence[MAX_TIME_CHANNELS],
                       WINDOW_SHAPE           windowShape[MAX_TIME_CHANNELS],
                       WINDOW_SHAPE           prevWindowShape[MAX_TIME_CHANNELS],
                       int                    nr_of_sfb[MAX_TIME_CHANNELS],
                       int                    max_sfb[MAX_TIME_CHANNELS],
                       int                    block_size_samples[MAX_TIME_CHANNELS],
                       int                    sfb_offset[MAX_TIME_CHANNELS][MAX_SCFAC_BANDS+1],
                       MSInfo                *msInfo,
                       TNS_INFO              *tnsInfo[MAX_TIME_CHANNELS],
                       unsigned int           channels,
                       int                    common_window,
                       USAC_CORE_MODE         coreMode[MAX_TIME_CHANNELS],
                       USAC_CORE_MODE         prev_coreMode[MAX_TIME_CHANNELS],
                       USAC_CORE_MODE         next_coreMode[MAX_TIME_CHANNELS],
                       double                 reconstructed_time_signal[MAX_TIME_CHANNELS][4096]);

int DeleteIntDec(HANDLE_TFDEC *phtfDec);

#endif /* #ifndef _INT_DEC_H */ 
