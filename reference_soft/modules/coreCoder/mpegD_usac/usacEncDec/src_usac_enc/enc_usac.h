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

#ifndef _enc_usac_h_
#define _enc_usac_h_


#include <stdio.h>              /* typedef FILE */

#include "allHandles.h"
#include "encoder.h"
#include "ms.h"
#include "ntt_win_sw.h"
#include "sac_enc.h"

#define TD_BUFFER_OFFSET 448+64

enum {
      USAC_CHANNEL_ELEMENTTYPE_LFE     = 0,
      USAC_CHANNEL_ELEMENTTYPE_SCE     = 1,
      USAC_CHANNEL_ELEMENTTYPE_CPE_L   = 2,
      USAC_CHANNEL_ELEMENTTYPE_CPE_R   = 3,
      USAC_CHANNEL_ELEMENTTYPE_CPE_DMX = 4,
      USAC_CHANNEL_ELEMENTTYPE_CPE_RES = 5 };


/* ---------- functions ---------- */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagEncoderSpecificData ENCODER_DATA, *HANDLE_ENCODER_DATA;
typedef struct mpegh3daFrame_data MPEGH3DAFRAME_DATA, *HANDLE_MPEGH3DAFRAME_DATA;
typedef struct mpegh3daConfig_data MPEGH3DACONFIG_DATA, *HANDLE_MPEGH3DACONFIG_DATA;


char *EncUsacInfo (FILE *helpStream, int extendedHelp);

int EncUsacInit (
  int                 numChannel,
  float               fSample,
  HANDLE_ENCPARA      encPara,
  int                 *frameNumSample,
  int                 *frameMaxNumBit, /* SAMSUNG_2005-09-30 */
  DEC_CONF_DESCRIPTOR *dec_conf,
  int                 *numTrack,
  HANDLE_BSBITBUFFER  *asc,
  HANDLE_ENCODER      core,
  HANDLE_ENCODER      enc
);

  /*int Get_sbrenable(HANDLE_ENCODER encoderStruct, int *bitrate);*/

int EncUsacFrame (
  const ENCODER_DATA_TYPE input,
  HANDLE_BSBITBUFFER *au,                   /* buffers to hold output AccessUnits */
  const int nextIsIPF,                      /* indicate that the next frame will be an IPF */
  HANDLE_ENCODER enc,
  float **p_time_signal_orig
);

void EncUsacFree (HANDLE_ENCODER enc);


int EncUsac_getUsacDelay(HANDLE_ENCODER encoderStruct);
int EncUsac_getUsacEncoderDelay(HANDLE_ENCODER encoderStruct);
int EncUsac_getusac212enable(HANDLE_ENCODER encoderStruct);
HANDLE_SPATIAL_ENC EncUsac_getSpatialEnc(HANDLE_ENCODER encoderStruct, int elemIndex);
void EncUsac_setSpatialData(HANDLE_ENCODER encoderStruct, unsigned char *databuf, unsigned long size, int elemIndex);
int EncUsac_getSpatialOutputBufferDelay(HANDLE_ENCODER encoderStruct);
int EncUsac_getIndependencyFlag(HANDLE_ENCODER encoderStruct, int offset);
int EncUsac_getChannelElementType(HANDLE_ENCODER encoderStruct, int ch);
int EncUsac_getChannelElementIndex(HANDLE_ENCODER encoderStruct, int ch);
int EncUsac_getIsQceElement(HANDLE_ENCODER encoderStruct, int elemIdx) ;
int EncUsac_getFlagMpsRes(HANDLE_ENCODER encoderStruct);
COMPATIBLE_PROFILE_LEVEL_SET EncUsac_getCompatibleProfileLevelSet(HANDLE_ENCODER encoderStruct);
#ifdef __cplusplus
}
#endif

#endif  /* #ifndef _enc_usac_h_ */

/* end of enc_usac.h */
