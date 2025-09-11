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

#include "localSetupInformationInterfaceLib_API.h"
#include "readonlybitbuf.h"
#include "cicp2geometry.h"

#ifndef MAX_NUM_WIRES
  #define MAX_NUM_WIRES 65535
#endif
#ifndef MAX_NUM_SPEAKERS
  #define MAX_NUM_SPEAKERS 65535 
#endif
#ifndef LS_MAX_LOUDSPEAKERS
  #define LS_MAX_LOUDSPEAKERS 32
#endif

typedef struct _LocalScreenInfoQuant
{
  int az_left;
  int az_right;
  int el_top;
  int el_bottom;
} *H_LOCAL_SCREEN_INFO_QUANT;

typedef struct _LoudspeakerRenderingQuant
{
  int *loudspeakerCalibrationGain;
  int *knownAzimuth;
  int *knownElevation;
} *H_LOUDSPEAKER_RENDERING_QUANT;


struct _LocalSetupHandle
{
  int rendering_type;
  int numWireOutputs;
  int *WireIDs;
  int noBits;

  H_LOCAL_SCREEN_INFO LocalScreenInfo;
  H_LOCAL_SCREEN_INFO_QUANT LocalScreenInfoQuant;
  H_LOUDSPEAKER_RENDERING LoudspeakerRendering;
  H_LOUDSPEAKER_RENDERING_QUANT LoudspeakerRenderingQuant;

  H_BINAURAL_RENDERING BinauralRendering;
};

typedef struct {
  int cicpLoudspeakerIndex; /**< index specifying cicp loudspeaker. -1 if only geometry is given. */
  int Az;                   /**< azimuth angle of the channel    */
  int El;                   /**< elevation angle of the channel  */
  int LFE;                  /**< flag whether channel is an LFE  */
  int dist;
  float calibGain;
} CHANNEL_GEOMETRY;

int LS_readBinauralRenderingBits(LOCAL_SETUP_HANDLE LocalSetup, robitbufHandle h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal);
int LS_writeBits_BinauralRendering(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal, char *filename);
int LS_quantize_LSRendering_Data(LOCAL_SETUP_HANDLE LocalSetup);
int LS_writeBits_LSRendering(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int LS_quantize_LocalScreen_Data(LOCAL_SETUP_HANDLE LocalSetup);
int LS_readLoudspeakerRenderingBits(LOCAL_SETUP_HANDLE LocalSetup, H_LS_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal);
int LS_invquantizeLoudspeakerRenderingData(LOCAL_SETUP_HANDLE LocalSetup);
int LS_invquantizeLocalScreenData(LOCAL_SETUP_HANDLE LocalSetup, int centeredInAz);
int LS_getGeometryFromFile(char *filename, CHANNEL_GEOMETRY AzElLfe[LS_MAX_LOUDSPEAKERS], int *numChannels, int *extDistComp, int *CICPLayoutIdx, const int verboseLvl);

int getBinauralFirData(BinauralFirData *pBinauralFirData, short RepresentationIndex, BinauralRendering *pBR);
int getFdBinauralRendererParam(FdBinauralRendererParam *pFdBinauralRendererParam, short RepresentationIndex, BinauralRendering *pBR);
int getTdBinauralRendererParam(TdBinauralRendererParam *pTdBinauralRendererParam, short RepresentationIndex, BinauralRendering *pBR);
int LS_setBRIRsFromBitstream(LOCAL_SETUP_HANDLE h_LocalSetupHandle,  char *path);

int getSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, BinauralRendering *pBR, int RepresentationIndex);

