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

#ifndef __FDBINAURAL_H__
#define __FDBINAURAL_H__

#include "decode_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct FdBinauralParametrizationData_* FdBinauralParametrizationDataHandle;
typedef struct FdBinauralRenderingData_* FdBinauralRenderingDataHandle;
typedef struct FdBinauralBitstreamWriterData_* FdBinauralBitstreamWriterDataHandle;

int FdBinauralBitstreamWriterInit ( FdBinauralBitstreamWriterDataHandle* ppData, char* brirFolderPath, char *executablePath, char* logfilename, char* bitstreamOutPath, char* geoInPath  );
int FdBinauralParametrizationInit ( FdBinauralParametrizationDataHandle* ppData, char* brirBitstreamPath, char* executablePath, char*  logfilename, char *bitstreamOutPath );
int FdBinauralRendererInit ( FdBinauralRenderingDataHandle* ppData, char* infile, char* outfile, int outputFormat, int readBrirMetadataType, char *readBrirLocation, char* executablePath, char* outGeoPath, char*  logfilename, int profile, int level);

int FdBinauralBitstreamWriterRun ( FdBinauralBitstreamWriterDataHandle pData, const VERBOSE_LEVEL verboseLvl );
int FdBinauralParametrizationRun (  FdBinauralParametrizationDataHandle pData, const VERBOSE_LEVEL verboseLvl );
int FdBinauralRendererRun ( FdBinauralRenderingDataHandle pData, const VERBOSE_LEVEL verboseLvl );

int FdBinauralBitstreamWriterExit (FdBinauralBitstreamWriterDataHandle* ppData);
int FdBinauralParametrizationExit(FdBinauralParametrizationDataHandle* ppData);
int FdBinauralRendererExit(FdBinauralRenderingDataHandle* ppData);



#ifdef __cplusplus
}
#endif
#endif