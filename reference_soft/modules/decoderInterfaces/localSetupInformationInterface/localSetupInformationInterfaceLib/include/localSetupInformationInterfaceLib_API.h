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

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#ifndef LS_API
#define LS_API 

#ifndef NULL
  #define NULL 0
#endif

#include "bitstreamBinaural.h"

typedef struct _LocalScreenInfo
{
  int hasLocalScreenInfo;
  float az_left;
  float az_right;
  int hasElevationInfo;
  float el_top;
  float el_bottom;
  float az_left_temp;
  float az_right_temp;
  float offset;
} *H_LOCAL_SCREEN_INFO;

typedef struct _FlexibleSpeakerConfig
{
  int angularPrecision;
  int *isCICPspeakerIdx;
  int *CICPspeakerIdx;
  int *ElevationClass;
  int *ElevationAngleIdx;
  int *ElevationDirection;
  float *Elevation;
  int *AzimuthAngleIdx;
  int *AzimuthDirection;
  float *Azimuth;
  int *isLFE;
  int *alsoAddSymmetricPair;
} *H_FLEXIBLE_SPEAKER_CONFIG;

typedef struct _LoudspeakerSetup
{
  int speakerLayoutType;
  int CICPspeakerLayoutIdx;
  int numSpeakers;
  int numExplicitelySignaledSpeakers;
  int *CICPspeakerIdx;
  H_FLEXIBLE_SPEAKER_CONFIG FlexibleSpeakerConfig;
  float **knownPositions;
} *H_LOUDSPEAKER_SETUP;

typedef struct _LoudspeakerRendering
{
  int numSpeakers;
  int useTrackingMode;
  int *loudspeakerDistance;
  float *loudspeakerCalibrationGain;
  H_LOUDSPEAKER_SETUP Setup_SpeakerConfig3D;
  int externalDistanceCompensation;
} *H_LOUDSPEAKER_RENDERING;

typedef struct _BinauralRendering
{
  BinauralRendering *pBR;
} *H_BINAURAL_RENDERING;


typedef struct _LocalSetupData
{
  int rendering_type;
  int numWireOutputs;
  int *WireIDs;
  H_LOCAL_SCREEN_INFO LocalScreenInfo;
  H_LOUDSPEAKER_RENDERING LoudspeakerRendering;
  H_BINAURAL_RENDERING BinauralRendering;
  int dmxID;
} *H_LOCAL_SETUP_DATA;

typedef struct _LocalSetupHandle *LOCAL_SETUP_HANDLE;
/* typedef struct _LocalSetupData *H_LOCAL_SETUP_DATA; */

typedef struct _lsBitstreamEncoder *H_LS_BITSTREAM_ENCODER;
typedef struct _lsBitstreamDecoder *H_LS_BITSTREAM_DECODER;

int LS_setBRIRsFromWav_HOA(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *path, int hoaOrder, int useTrackingMode);
int LS_setBRIRsFromWav_CICP_GEO(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *path, int cicp, char*geo_path, int useTrackingMode);
int LS_setBRIRsFromBitstream(LOCAL_SETUP_HANDLE h_LocalSetupHandle,  char *path);

int LS_initHandle(LOCAL_SETUP_HANDLE *h_LocalSetupHandle);
int LS_initRenderingType(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int rendering_type);
int LS_initWireOutputs(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int numWireOutputs, int *WireIDs);
int LS_setLocalScreenSize(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LOCAL_SCREEN_INFO LocalScreenInfo);
int LS_closeHandle(LOCAL_SETUP_HANDLE LocalSetup);

int LS_setLoudspeakerRenderingConfig(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int extDistComp, int numLS, int *LSdist, float *LScalibGain, int useTrackingMode);
int LS_setFixedLoudspeakerLayout(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int CICPspeakerLayoutIdx, int *CICPspeakerIdx, int *hasKnownPosition, float *Azimuth, float *Elevation);
int LS_setFlexibleLoudspeakerLayout(LOCAL_SETUP_HANDLE h_LocalSetupHandle, int *CICPspeakerIdx, float *Azimuth, float *Elevation, int *isLFE, int *alsoAddSymmetricPair, int numExplicitelySignaledSpeakers);
int LS_getLocalSetupData(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LOCAL_SETUP_DATA *LocalSetupData);
int LS_freeLocalSetupData(H_LOCAL_SETUP_DATA LocalSetupData);
int LS_setLoudspeakerLayout_fromFile(LOCAL_SETUP_HANDLE h_LocalSetupHandle, char *filename, int *numSpeakers, int *extDistComp, int **LSdistance, float **LscalibGain);

int LS_writeLocalSetupBitstream(LOCAL_SETUP_HANDLE h_LocalSetupHandle, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, char * filename);
int LS_readLocalSetupBitstream(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_DECODER h_bitstream, unsigned long *no_BitsRead);

int LS_DecInt_initBitstreamWriter(H_LS_BITSTREAM_ENCODER *h_bitstream, char *file);
void LS_DecInt_closeBitstreamWriter(H_LS_BITSTREAM_ENCODER h_bitstream);
int LS_DecInt_initBitstreamReader(H_LS_BITSTREAM_DECODER *h_bitstream, char *file);
void LS_DecInt_closeBitstreamReader(H_LS_BITSTREAM_DECODER h_bitstream);

int LS_getAzimuthFromCICPspeakerIdx(int CICPspeakerIdx);
int LS_getSymmetricCICPspeakerIdx(int CICPspeakerIdx);
/* int LS_getNumSpeakerFromCICPidx(int CICPidx); */

#endif