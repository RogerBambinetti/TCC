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

#ifndef __GVBAPRENDERER_H__
#define __GVBAPRENDERER_H__

#include "cicp2geometry.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MPEG3DAGVBAP_ERROR_FIRST -666

typedef enum _RETURN_CODE
{
  MPEG3DAGVBAP_ERROR_INIT            = MPEG3DAGVBAP_ERROR_FIRST,
  MPEG3DAGVBAP_ERROR_INVALID_STREAM,
  MPEG3DAGVBAP_ERROR_GENERIC         = -1,
  MPEG3DAGVBAP_OK                    = 0
} MPEG3DAGVBAP_RETURN_CODE;

/* This structure can hold a single OAM sample. */
typedef struct
{
  float azimuth;      /**< azimuth angle in radians   */
  float elevation;    /**< elevation angle in radians */
  float radius;       /**< radius value in meter     */
  float gain;         /**< linear gain value         */
  float spread_angle;
  float spread_angle_height;
  float spread_depth;

} OAM_SAMPLE;


typedef struct _GVBAPRENDERER* HANDLE_GVBAPRENDERER;



int gVBAPRenderer_Open(HANDLE_GVBAPRENDERER* phgVBAPRenderer, int numObjects, int frameLength, int outCICPIndex, 
                       CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo, int outChannels);

int gVBAPRenderer_RenderFrame_Time(HANDLE_GVBAPRENDERER hgVBAPRenderer, 
                                   float** inputBuffer, float** outputBuffer, OAM_SAMPLE* oamStartSample, 
                                   OAM_SAMPLE* oamStopSample, int hasUniformSpread);

int gVBAPRenderer_RenderFrame_Frequency(HANDLE_GVBAPRENDERER hgVBAPRenderer, 
                                   float** inputBuffer, float** outputBuffer, OAM_SAMPLE* oamStartSample, 
                                   OAM_SAMPLE* oamStopSample, int numQmfBands, int hasUniformSpread);

int gVBAPRenderer_GetGains(HANDLE_GVBAPRENDERER hgVBAPRenderer, OAM_SAMPLE* oamSample, float** gainMatrix, int hasUniformSpread);

int gVBAPRenderer_Close(HANDLE_GVBAPRENDERER hgVBAPRenderer);

int gVBAPRenderer_GetStaticGains(CICP2GEOMETRY_CHANNEL_GEOMETRY* inGeometryInfo, int numObjects, int outCICPIndex, 
                       CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo, int outChannels, float** staticGainsMatrix);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
