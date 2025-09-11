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

Copyright (c) ISO/IEC 2015.

***********************************************************************************/

#include "elementInteractionInterfaceLib_API.h"
#include "localSetupInformationInterfaceLib_API.h"
#include "ascparser.h"
#include "oam_read.h"
#include "metadataPreprocessor.h"

int MP_getLocalScreenConfig(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_LOCAL_SCREEN_INFO localScreen);
int MP_screenRelatedRemapAzimuth(float *azimuth, H_LOCAL_SCREEN_INFO localScreen, ProductionScreenData productionScreen);
int MP_screenRelatedRemapElevation(float *elevation, H_LOCAL_SCREEN_INFO localScreen, ProductionScreenData productionScreen);
int MP_screenRelatedZooming(float *azimuth, float *elevation, H_ZOOM_AREA ZoomArea, H_LOCAL_SCREEN_INFO localScreen);
int MP_applyScreenRelatedRemappingAndZooming(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int selectedPreset, int divergenceObjectsAdded);