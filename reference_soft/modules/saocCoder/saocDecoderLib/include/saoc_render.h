/*******************************************************************************
This software module was originally developed by

Coding Technologies, Fraunhofer IIS, Philips

and edited by

-

in the course of development of ISO/IEC 23003 for reference purposes and its
performance may not have been optimized. This software module is an
implementation of one or more tools as specified by ISO/IEC 23003. ISO/IEC gives
You a royalty-free, worldwide, non-exclusive, copyright license to copy,
distribute, and make derivative works of this software module or modifications
thereof for use in implementations of ISO/IEC 23003 in products that satisfy
conformance criteria (if any). Those intending to use this software module in
products are advised that its use may infringe existing patents. ISO/IEC have no
liability for use of this software module or modifications thereof. Copyright is
not released for products that do not conform to audiovisual and image-coding
related ITU Recommendations and/or ISO/IEC International Standards.

#ifdef NOT_PUBLISHED

Assurance that the originally developed software module can be used (1) in
ISO/IEC 23003 once ISO/IEC 23003 has been adopted; and (2) to develop ISO/IEC
23003:
Coding Technologies, Fraunhofer IIS, Philips grant(s) ISO/IEC all
rights necessary to include the originally developed software module or
modifications thereof in ISO/IEC 23003 and to permit ISO/IEC to offer You a
royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
and make derivative works for use in implementations of ISO/IEC 23003 in
products that satisfy conformance criteria (if any), and to the extent that such
originally developed software module or portions of it are included in ISO/IEC
23003. To the extent that Coding Technologies, Fraunhofer IIS,
Philips own(s) patent rights that would be required to make, use, or sell the
originally developed software module or portions thereof included in ISO/IEC
23003 in a conforming product, Coding Technologies, Fraunhofer
IIS, Philips will assure the ISO/IEC that it is (they are) willing to negotiate
licenses under reasonable and non-discriminatory terms and conditions with
applicants throughout the world. ISO/IEC gives You a free license to this
software module or modifications thereof for the sole purpose of developing
ISO/IEC 23003.

#endif

Coding Technologies, Fraunhofer IIS, Philips retain full right to
modify and use the code for its (their) own purpose, assign or donate the code
to a third party and to inhibit third parties from using the code for products
that do not conform to MPEG-related ITU Recommendations and/or ISO/IEC
International Standards. This copyright notice must be included in all copies or
derivative works.

Copyright (c) ISO/IEC 2007.
*******************************************************************************/

#ifndef _SAOC_RENDER_H
#define _SAOC_RENDER_H


#include "saoc_const.h"
#include "oam_common.h"

float *SAOC_InitTimeInstances(char* pInName,int *num_time_instances);
float ***SAOC_InitRenderMats(char* pInName,int num_time_instances,float *time_instances,int nMode);
void SAOC_deleteTimeInstances(float *time_instances);
void SAOC_deleteRenderMats(int num_time_instances,float ***pMats);

void SAOC_Interpolate_oamRenMat(int numOamPerFrame,
                                int framePossOAM[32],
                                float frameRenderMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
                                float pRenderMatTmp[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
                                int env_p);

void SAOC_getRenderMat(float pMat[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],float ***RenderMats,int num_time_instances,float *time_instances,int env_p);

int saoc_get_RenMat_ChannelPath(int inSaocCICPIndex,
                                int outSaocCICPIndex,
                                int saocInputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
                                int saocInputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                                int nInChannels,
                                int nInLFEs,
                                int saocOutputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
                                int saocOutputLFE_Mapping[SAOC_MAX_LFE_CHANNELS],
                                int nOutChannels,
                                int nOutLFEs,
                                char* pInName,
                                float RenMatCh[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
                                float pRenderMat_lfe[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_LFE_CHANNELS]);


#endif
