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

Copyright (c) ISO/IEC 1998.

*************************************************************************/

#ifndef _OBJ_DESCR_H_INCLUDED
#define _OBJ_DESCR_H_INCLUDED

#include "allHandles.h"

typedef unsigned char UINT8;
typedef unsigned long UINT32;

typedef struct  {
  UINT8  length ;/* in bits */
  UINT32 value;
} DESCR_ELE;

#include "usac_config.h"          /* This header has a dependency on DESCR_ELE, so don't move it up... */

#define FRAMEWORK_MAX_LAYER 50

typedef struct {
  DESCR_ELE  frameLength;
  DESCR_ELE  dependsOnCoreCoder;
  DESCR_ELE  coreCoderDelay;
  DESCR_ELE  extension;
  DESCR_ELE  layerNr;
  HANDLE_PROG_CONFIG progConfig;
  DESCR_ELE  numOfSubFrame;
  DESCR_ELE  layer_length;
  DESCR_ELE  aacSectionDataResilienceFlag;
  DESCR_ELE  aacScalefactorDataResilienceFlag;
  DESCR_ELE  aacSpectralDataResilienceFlag;
  DESCR_ELE  extension3;
} TF_SPECIFIC_CONFIG;

#ifdef AAC_ELD

#define MAX_SBR_HEADER_SIZE 4
#define MAX_SBR_ELEMENTS 16

typedef enum {
  ELDEXT_TERM=0
} ELD_EXTENSION_TYPE;

typedef struct {
  DESCR_ELE frameLengthFlag;

  DESCR_ELE aacSectionDataResilienceFlag;
  DESCR_ELE aacScalefactorDataResilienceFlag;
  DESCR_ELE aacSpectralDataResilienceFlag;

  DESCR_ELE ldSbrPresentFlag;
  DESCR_ELE ldSbrSamplingRate;
  DESCR_ELE ldSbrCrcFlag;

  UINT8     sbrHeaderData[MAX_SBR_ELEMENTS][MAX_SBR_HEADER_SIZE];

} ELD_SPECIFIC_CONFIG;
#endif

typedef struct {
  DESCR_ELE  excitationMode ;
  DESCR_ELE  sampleRateMode ;
  DESCR_ELE  fineRateControl ;
  DESCR_ELE  RPE_Configuration ;
  DESCR_ELE  MPE_Configuration ;
  DESCR_ELE  numEnhLayers ;
  DESCR_ELE  bandwidthScalabilityMode ;
  DESCR_ELE  silenceCompressionSW;

#ifdef CORRIGENDUM1
  DESCR_ELE  BWS_Configuration ;
  DESCR_ELE  isBaseLayer;	/* 14496-3 COR1 */
  DESCR_ELE  isBWSLayer;	/* 14496-3 COR1 */
  DESCR_ELE  CELP_BRS_id;	/* 14496-3 COR1 */
#endif
} CELP_SPECIFIC_CONFIG;

#ifndef CORRIGENDUM1
typedef struct {
  DESCR_ELE  BWS_Configuration ;
} CELP_ENH_SPECIFIC_CONFIG;
#endif

/* HP 20000330 */
typedef struct {
#ifdef CORRIGENDUM1
  DESCR_ELE  isBaseLayer;	/* 14496-3 COR1 HP20001023 */
#endif
  /* base layer */
  DESCR_ELE  PARAmode;

  DESCR_ELE  HVXCvarMode;
  DESCR_ELE  HVXCrateMode;
  DESCR_ELE  extensionFlag;
  DESCR_ELE  vrScalFlag;

  DESCR_ELE  HILNquantMode;
  DESCR_ELE  HILNmaxNumLine;
  DESCR_ELE  HILNsampleRateCode;
  DESCR_ELE  HILNframeLength;
  DESCR_ELE  HILNcontMode;

  DESCR_ELE  PARAextensionFlag;

  /* enha/ext layer(s) */
  DESCR_ELE  HILNenhaLayer;
  DESCR_ELE  HILNenhaQuantMode;
} PARA_SPECIFIC_CONFIG;

#ifdef EXT2PAR
typedef struct {

  DESCR_ELE  DecoderLevel;
  DESCR_ELE  UpdateRate;
  DESCR_ELE  SynthesisMethod;
  DESCR_ELE	 ModeExt;
  DESCR_ELE	 PsReserved;

} SSC_SPECIFIC_CONFIG;
#endif

#ifdef I2R_LOSSLESS
typedef struct {
  DESCR_ELE  SLSpcmWordLength;
  DESCR_ELE  SLSaacCorePresent;
  DESCR_ELE  SLSlleMainStream;
  DESCR_ELE  SLSreserved;
  DESCR_ELE  SLSframeLength;
  HANDLE_PROG_CONFIG progConfig;
} SLS_SPECIFIC_CONFIG;
#endif

/* AI 990616 */
typedef struct {
#ifdef CORRIGENDUM1
  DESCR_ELE  isBaseLayer;	/* 14496-3 COR1 (AI 2000/10) */
#endif
  DESCR_ELE  HVXCvarMode;
  DESCR_ELE  HVXCrateMode;
  DESCR_ELE  extensionFlag;
  DESCR_ELE  vrScalFlag;      /* VR scalable mode (YM 990728) */
} HVXC_SPECIFIC_CONFIG;

typedef struct {
  DESCR_ELE  lengthEscape;
  DESCR_ELE  rateEscape;
  DESCR_ELE  crclenEscape;
  DESCR_ELE  concatenateFlag;
  DESCR_ELE  fecType;
  DESCR_ELE  terminationSwitch;
  DESCR_ELE  interleaveSwitch;
  DESCR_ELE  classOptional;
  DESCR_ELE  numberOfBitsForLength;
  DESCR_ELE  classLength;
  DESCR_ELE  classRate;
  DESCR_ELE  classCrclen;
  DESCR_ELE  classOutputOrder;
} EP_CLASS_CONFIG;

typedef struct {
  DESCR_ELE  numberOfClass;
  DESCR_ELE  classReorderedOutput;
  EP_CLASS_CONFIG *epClassConfig;
} PRED_SET_CONFIG;

typedef struct {
  DESCR_ELE  numberOfPredifinedSet;
  DESCR_ELE  interleaveType;
  DESCR_ELE  bitStuffing;
  DESCR_ELE  numberOfConcatenatedFrame;

  PRED_SET_CONFIG *predSetConfig;

  DESCR_ELE  headerProtection;
  DESCR_ELE  headerRate;
  DESCR_ELE  headerCrclen;

  UINT8 *pEpConfigBuf;
  long  EpConfigBufLength;
} EP_SPECIFIC_CONFIG;

typedef struct {
  DESCR_ELE  Extension;
} MPEG_1_2_SPECIFIC_CONFIG;

typedef struct {
  DESCR_ELE  audioDecoderType        ;
  DESCR_ELE  samplingFreqencyIndex; /* HP20001010: "u" missing ;-) */
  DESCR_ELE  samplingFrequency; /* HP20001010 */
  DESCR_ELE  channelConfiguration;
  DESCR_ELE  extensionChannelConfiguration;

#ifdef CT_SBR
  DESCR_ELE  extensionAudioDecoderType;
  DESCR_ELE  extensionSamplingFrequencyIndex;
  DESCR_ELE  extensionSamplingFrequency;
  DESCR_ELE  sbrPresentFlag;
  DESCR_ELE  syncExtensionType;
#ifdef PARAMETRICSTEREO
  DESCR_ELE  psPresentFlag;
#endif
#endif

#ifndef CORRIGENDUM1
  int        BWS_on; /* CelpBandWidthScalablity on ? */
#endif

  union {
    TF_SPECIFIC_CONFIG TFSpecificConfig;
    USAC_CONFIG usacConfig;
    CELP_SPECIFIC_CONFIG celpSpecificConfig;
#ifndef CORRIGENDUM1
    CELP_ENH_SPECIFIC_CONFIG celpEnhSpecificConfig;
#endif
#ifdef AAC_ELD
    ELD_SPECIFIC_CONFIG eldSpecificConfig;
#endif
    PARA_SPECIFIC_CONFIG paraSpecificConfig;
    HVXC_SPECIFIC_CONFIG hvxcSpecificConfig;	/* AI 990616 */
#ifdef EXT2PAR
    SSC_SPECIFIC_CONFIG  sscSpecificConfig;
#endif
#ifdef MPEG12
    MPEG_1_2_SPECIFIC_CONFIG  MPEG_1_2_SpecificConfig;
#endif
  } specConf;

#ifdef I2R_LOSSLESS
  SLS_SPECIFIC_CONFIG  slsSpecificConfig;   /* make it outside of union: used together with TF */
#endif

  DESCR_ELE epConfig;
  EP_SPECIFIC_CONFIG epSpecificConfig;
  DESCR_ELE directMapping;
}  AUDIO_SPECIFIC_CONFIG, *HAUDIO_SPECIFIC_CONFIG;


typedef struct {
  DESCR_ELE profileAndLevelIndication;
  DESCR_ELE streamType      ;
  DESCR_ELE upsteam;      /* OK 20030130: "r" missing ;-) */
  DESCR_ELE specificInfoFlag;
  DESCR_ELE bufferSizeDB;
  DESCR_ELE maxBitrate;
  DESCR_ELE avgBitrate;
  DESCR_ELE specificInfoLength;
  AUDIO_SPECIFIC_CONFIG audioSpecificConfig;

  /* MPEG-H Specific */
  UINT32 transportMask;               /* No bitstream element, just a mask to decide what boxes to write: */
                                      /* (1 << 0): write configuration box (only for streaming format) */
                                      /* (1 << 1),    write bitrate box */
                                      /* (1 << 2),    use streaming format (MHM box) */
                                      /* (1 << 3),    use multiple streams format */
  DESCR_ELE MPEGHAudioProfileLevel;   /* Audio Codec Profile Level Indication */
} DEC_CONF_DESCRIPTOR, *HDEC_CONF_DESCRIPTOR;

typedef struct {
  DESCR_ELE isActive;
  DESCR_ELE reserved;
  DESCR_ELE truncFromBegin;
  DESCR_ELE nTruncSamples;
} AUDIO_TRUNCATION_INFO, *HAUDIO_TRUNCATION_INFO;

typedef struct {
  DESCR_ELE useAccessUnitStartFlag;
  DESCR_ELE useAccessUnitEndFlag;
  DESCR_ELE useRandomAccessPointFlag;
  DESCR_ELE usePaddingFlag;
  DESCR_ELE seqNumLength;
  /* to be completed */
} AL_CONF_DESCRIPTOR ;

typedef struct {
  DESCR_ELE ESNumber;
  DESCR_ELE streamDependence;
  DESCR_ELE URLFlag;
  DESCR_ELE OCRFlag;
  DESCR_ELE streamPriority;
  DESCR_ELE URLlength;
  DESCR_ELE URLstring;
  DESCR_ELE OCR_ES_id;
  DESCR_ELE dependsOn_Es_number;
  DEC_CONF_DESCRIPTOR DecConfigDescr;
  AL_CONF_DESCRIPTOR ALConfigDescriptor;
} ES_DESCRIPTOR ;


typedef struct {
  DESCR_ELE  ODLength;
  DESCR_ELE  ODescrId;
  DESCR_ELE  streamCount;
  DESCR_ELE  extensionFlag;
  ES_DESCRIPTOR *ESDescriptor[50];
} OBJECT_DESCRIPTOR ;

typedef struct {
  HANDLE_BSBITBUFFER      bitBuf;
  int                     sampleRate;
  int                     bitRate;
  unsigned int            NoAUInBuffer;
  unsigned int            *AULength;
  unsigned int            *AUPaddingBits;
} LAYER_DATA ;


typedef struct {
  OBJECT_DESCRIPTOR* od ;
  LAYER_DATA layer[50];
  HANDLE_EPCONVERTER ep_converter[FRAMEWORK_MAX_LAYER];
  int  tracksInLayer[FRAMEWORK_MAX_LAYER];
  unsigned int scalOutSelect;
  int  scalOutObjectType;
  int  scalOutNumChannels;
  int  scalOutSamplingFrequency;
  int  bsacDecLayer;		/* Nice hyungk !!! (2003.1.20) / For -maxl option supporting in ER_BSAC */
} FRAME_DATA ;


void ObjDescPrintf(int WriteFlag, char *message, ...);

#endif
