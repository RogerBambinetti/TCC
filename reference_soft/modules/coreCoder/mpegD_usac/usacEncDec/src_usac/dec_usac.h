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

#ifndef _dec_usac_h_
#define _dec_usac_h_



#include "allHandles.h"
#include "lpc_common.h"         /* TDS */
#include "all.h"
#include "tns.h"
#include "huffdec2.h"
#include "resilience.h"
#include "flex_mux.h"
#include "ntt_conf.h"
#include "dec_hvxc.h"
#include "usac_mainStruct.h"

#ifdef CT_SBR
#include "ct_sbrdecoder.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* ---------- functions ---------- */
/* DecUsacInfo() */
/* Get info about usac decoder core. */

char *DecUsacInfo (
  FILE *helpStream);		/* in: print decPara help text to helpStream */
				/*     if helpStream not NULL */
				/* returns: core version string */


/* DecUsacInit() */
/* Init t/f-based decoder core. */

void DecUsacInit(
               char*               decPara,          /* in: decoder parameter string */
               char*               usacDebugStr,
               int                 epDebugLevel,
               HANDLE_DECODER_GENERAL hFault,
               FRAME_DATA*         frameData,
               USAC_DATA*          tfData,
               char*               infoFileName
             );


/* DecUsacFrame() */
/* Decode one bit stream frame into one audio frame with */
/* t/f-based decoder core. */
void DecUsacFrame(int           numChannels,
                  FRAME_DATA*  fd, /* config data , obj descr. etc. */
                  USAC_DATA*     tfData, /* actual frame input/output data */
                  HANDLE_DECODER_GENERAL hFault,/* additional data for error protection */
                  int *numOutChannels, /* SAMSUNG_2005-09-30 */
                  float ***qmfImagOutBuffer,
                  float ***qmfRealOutBuffer
                );


/* DecUsacFree() */
/* Free memory allocated by t/f-based decoder core. */

void DecUsacFree (
                USAC_DATA *usacData,
                HANDLE_DECODER_GENERAL hFault
                );


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef _dec_tf_h_ */

/* end of dec_tf.h */
