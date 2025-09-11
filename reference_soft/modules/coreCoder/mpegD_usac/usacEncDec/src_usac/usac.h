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

#ifndef _usac_h_
#define _usac_h_

/* ----------------------------------------------------------
   This part is connection between DecUsacFrame in dec_tf.c
   and the aac-decoder starting in decoder.c
  ----------------------------------------------------------- */
#ifdef CT_SBR
#include "ct_sbrdecoder.h"
#endif

#include "block.h"

/* --- Init AAC-decoder --- */
HANDLE_USAC_DECODER USACDecodeInit(int samplingRate,
                                   char *usacDebugStr,
                                   const int  block_size_samples,
                                   Info ***sfbInfo,
                                   int  profile,
                                   AUDIO_SPECIFIC_CONFIG *streamConfig
);

/* ----- Decode one frame with the AAC-decoder  ----- */
int USACDecodeFrame (USAC_DATA               *usacData,
                     HANDLE_BSBITSTREAM       fixed_stream,
                     float*                  spectral_line_vector[MAX_TIME_CHANNELS],
                     WINDOW_SEQUENCE          windowSequence[MAX_TIME_CHANNELS],
                     WINDOW_SHAPE             window_shape[MAX_TIME_CHANNELS],
                     byte                     max_sfb[Winds],
                     int                      numChannels,
                     int*                     numOutChannels,
                     Info**                   sfbInfo,
                     byte                     sfbCbMap[MAX_TIME_CHANNELS][MAXBANDS],
                     short*                   pFactors[MAX_TIME_CHANNELS],
                     HANDLE_DECODER_GENERAL   hFault,
                     QC_MOD_SELECT            qc_select,
                     FRAME_DATA*              fd,
                     float***                 qmfRealOutBuffer,
                     float***                 qmfImagOutBuffer
);

void USACDecodeFree (HANDLE_USAC_DECODER hUsacDecoder);

void usacAUDecode ( int numChannels,
                   FRAME_DATA* frameData,
                   USAC_DATA* tfData,
                   HANDLE_DECODER_GENERAL hFault,
                   int *numOutChannels,
                   float*** qmfRealOutBuffer,
                   float*** qmfImagOutBuffer
);

#ifdef CT_SBR

SBRBITSTREAM *getUsacSbrBitstream   ( void *pusacDec );
void          setUsacMaxSfb             ( void *pusacDec, int maxSfb );
int           getUsacMaxSfbFreqLine     ( void *pusacDec );

#endif


#endif
