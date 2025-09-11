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

#ifndef __SAOC_PATH_H__
#define __SAOC_PATH_H__

#include "decode_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SUPPORT_SAOC_DMX_LAYOUT

typedef struct SaocPathData_* SaocPathDataHandle;

#ifdef SUPPORT_SAOC_DMX_LAYOUT
int SaocPathInit ( SaocPathDataHandle* ppData, int saocHeaderLength, int coreOutputFrameLength, int referenceLayout, char *inGeoFile, int hasDmxLayout, int dmxLayout, char *dmxGeoFile, int reproductionLayout, char *outGeoFile, char *outGeoDevFile, char* logfile, char* binaryName, char* binaryFormatConverter 
#else
int SaocPathInit ( SaocPathDataHandle* ppData, int saocHeaderLength, int coreOutputFrameLength, int referenceLayout, char *inGeoFile, int reproductionLayout, char *outGeoFile, char *outGeoDevFile, char* logfile, char* binaryName, char* binaryFormatConverter
#endif
    , DRC_PARAMS drcParams, PROCESSING_DOMAIN moduleProcessingDomainsDrc1);
int SaocPathExit ( SaocPathDataHandle* ppData );
int SaocPathRun ( SaocPathDataHandle pData, char* infile, char* outfile, int mdp_run, const VERBOSE_LEVEL verboseLvl );


#ifdef __cplusplus
}
#endif
#endif
