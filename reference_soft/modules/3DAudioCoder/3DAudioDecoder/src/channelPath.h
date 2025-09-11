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

Copyright (c) ISO/IEC 2013.

***********************************************************************************/

#ifndef __CHANNEL_PATH_H__
#define __CHANNEL_PATH_H__

#include "decode_types.h"
#ifndef IAR
#define IAR 1
#endif
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct ChannelPathData_* ChannelPathDataHandle;

int ChannelPathInit ( ChannelPathDataHandle* ppData, int audioChannelLayout, int reproductionLayout, char *inGeoFile, char *outGeoFile, char *outGeoDevFile, char *logfile, char* binaryFormatConverter, int passiveDownmixFlag, int numChannels
                      , DRC_PARAMS drcParams, DOMAIN_SWITCH domainSwitchingDecisionsPostDrc1, PROCESSING_DOMAIN moduleProcessingDomainsDrc1
#if IAR
                     ,char * binary_IarFormatConverter
#endif
                     , DOMAIN_SWITCH domainSwitchPreFormatConverter, char* binaryDomainSwitcher, int phaseAlignStrength, int immersiveDownmixFlag, int downmixId, int matrixBitSize, char* downmixMatrix, char* binaryDmxMatrixDecoder);
int ChannelPathExit ( ChannelPathDataHandle* ppData );
#if IAR
int ChannelPathRun ( ChannelPathDataHandle pData, char* infile, char* outfile, char* r3tfile, const PROFILE profile, const VERBOSE_LEVEL verboseLvl );
#else
int ChannelPathRun ( ChannelPathDataHandle pData, char* infile, char* outfile, const PROFILE profile, const VERBOSE_LEVEL verboseLvl );
#endif
int ChannelPathGetInfo ( ChannelPathDataHandle pData, int* pDelay );

/* helper functions */
int CompareInOutGeometry(int cicpIn, int cicpOut, char *inGeoFile, char *outGeoFile);

int getNumberOfOutputChannels( int cicpOut, char *outGeoFile, const int numChannelsAsc );

#ifdef __cplusplus
}
#endif
#endif
