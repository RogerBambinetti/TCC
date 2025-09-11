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

#ifndef _ENCODER_H_INCLUDED
#define _ENCODER_H_INCLUDED

/*#define RM11_RS_DELAY_CHAIN_FIX*/ /* Delay chain approach for the resampler / SBR */


#define DELAY_ENC_FRAMES 2

#define USAC_MAX_EXTENSION_PAYLOADS MAX_TIME_CHANNELS
#define USAC_MAX_EXTENSION_PAYLOAD_LEN 6144/8*MAX_TIME_CHANNELS     /* bytes */

#define USAC_MAX_CONFIG_LEN 128                                     /* bytes */

#define SUPPORTED_COMPATIBLEPROFILELEVEL_MIN (18)                   /* 0x12 */
#define SUPPORTED_COMPATIBLEPROFILELEVEL_MAX (20)                   /* 0x14 */

#include "obj_descr.h"
#include "mp4au.h"
#include "usac_interface.h"
#include "tf_mainHandle.h"

typedef float** ENCODER_DATA_TYPE;

typedef struct tagEncoderData *HANDLE_ENCODER;
struct tagEncoderData {
  struct tagEncoderSpecificData *data;
  struct tagTfEncoderSpecificData *dataTf;
  int (*getNumChannels)(HANDLE_ENCODER enc);
  ENCODER_DATA_TYPE (*getReconstructedTimeSignal)(HANDLE_ENCODER enc);
  enum MP4Mode (*getEncoderMode)(HANDLE_ENCODER enc);
  int (*getSbrEnable)(HANDLE_ENCODER enc, int *bitrate);
  int (*getFlag768)(HANDLE_ENCODER enc);
  int (*getSbrRatioIndex)(HANDLE_ENCODER enc);
  int (*getBitrate)(HANDLE_ENCODER enc);
  int ch_elm_tot; /* SAMSUNG_2005-09-30 : number of channel elements for BSAC Multichannel */
};

/* encoder modes: codec selection */
enum USAC_CODEC_MODE {
  USAC_SWITCHED,
  USAC_ONLY_FD,
  USAC_ONLY_TD,
  ___usac_codec_mode_end
};

/* encoder parameter data */
typedef struct encoderParameterData {
  int               debugLevel;
  int               wshape_flag;

  unsigned int      streamID_present;
  unsigned int      streamID;
  int               bitrate;
  int               bw_limit;  /* bandwidth limit of the spectrum */
#ifdef USE_FILL_ELEMENT
  int               bUseFillElement;
#endif
  /*int               embedIPF;*/

  /*--- TD/FD selection ---*/
  USAC_CORE_MODE    prev_coreMode[MAX_TIME_CHANNELS];
  USAC_CORE_MODE    coreMode[MAX_TIME_CHANNELS];
  enum USAC_CODEC_MODE    codecMode;

  int enhancedNoiseFilling;

  /*--- SBR Data ---*/
  int               sbrenable;
  int               sbrRatioIndex;

  /*---FD Data---*/
  int               flag_960;
  int               flag_768;
  int               flag_twMdct;
  TNS_COMPLEXITY    tns_select;
  int               flag_harmonicBE;
  int               flag_noiseFilling;
  int               ms_select;
  int               ep_config;
  int               aacAllowScalefacs;

  /*--- MPEGS 212 Data ---*/
  int               flag_ipd;
  int               flag_mpsres;
  int               hybridResidualMode;
  int               flag_mpsqce;
  int               flag_cplxPred;
  char              tsdInputFileName[FILENAME_MAX];
  int               noiseFillingMode;

  int               g_useSplitTransform;

  /* --- 3D Audio Extension Data */
  char              objFile[FILENAME_MAX];
  char              saocFile[FILENAME_MAX];
  char              saocBitrateFile[FILENAME_MAX];
#ifdef SPECIFY_INPUT_CONTENTTYPE
  INPUT_CONTENT_TYPE_3D contentType;
#endif
  char              hoaFile[FILENAME_MAX];
  char              hoaBitrateFile[FILENAME_MAX];
#if IAR
  char              iarFile[FILENAME_MAX];
#endif
  char              pmcFile[FILENAME_MAX];
  char              pmfFile[FILENAME_MAX];

  int               hasDynamicObjectPriority;
  int               lowDelayMetadataCoding;
  int               OAMFrameLength;

  int               cicpIn;
  int               mctMode;
  int               useLcProfile;
  int               useBlProfile;
  unsigned int      compatibleProfileLevel;
} ENC_PARA, *HANDLE_ENCPARA;


typedef struct EncoderMode {
  int  mode;
  char *modename;
  char* (*EncInfo)(FILE*,int);
  int (*EncInit)(int numChannel,                    /* in: num audio channels */
                 float fSample,                     /* in: sampling frequency [Hz] */
                 HANDLE_ENCPARA       encPara,                    /* in: encoder parameter struct */
                 int *frameNumSample,               /* out: num samples per frame */
                 int *frameMaxNumBit,               /* out: num samples per frame */
                 DEC_CONF_DESCRIPTOR *dec_conf,     /* out: decoder config */
                                                    /* descriptors for each track */
                 int *numTrack,                     /* out: number of tracks */
                 HANDLE_BSBITBUFFER  *asc,          /* buffers to hold output Audio Specific Config */
                 HANDLE_ENCODER core,               /* in:  core encoder or NULL */
                 HANDLE_ENCODER enc                 /* out: internal id-datastructure */
                 );
                 
  int (*EncFrame)(const ENCODER_DATA_TYPE    input,        /*in: input signal */
                  HANDLE_BSBITBUFFER        *au,           /*in: buffers to hold output AccessUnits */
                  const int                  nextIsIPF,    /* indicate that the next frame will be an IPF */
                  HANDLE_ENCODER             enc,
                  float                    **p_time_signal_orig);      /*20060107-2*/

  void (*EncFree)(HANDLE_ENCODER enc);
} EncoderMode;


typedef struct {
  int                     encoder_count;
  int*                    trackCount;
  struct EncoderMode*     enc_mode;
  HANDLE_ENCODER*         enc_data;
  DEC_CONF_DESCRIPTOR     dec_conf[MAX_TRACKS];
} ENC_FRAME_DATA;

#endif
