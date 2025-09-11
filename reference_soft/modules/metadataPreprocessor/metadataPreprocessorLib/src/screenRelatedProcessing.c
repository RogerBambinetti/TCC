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

#include "screenRelatedProcessing.h"
#include <stdlib.h>

int MP_getLocalScreenConfig(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_LOCAL_SCREEN_INFO localScreen)
{
  int i;
  int hasScreenRelativeObjects = 0;

  for (i = 0; i < ASCPARSER_USAC_MAX_ELEMENTS; i++)
  {
    if (audioSceneConfig->screenRelativeObjects[i] == 1)
    {
      hasScreenRelativeObjects = 1;
      break;
    }
  }
  /*if (hasScreenRelativeObjects == 1) */
  {
    localScreen->hasLocalScreenInfo = localSetupConfig->LocalScreenInfo->hasLocalScreenInfo;
    localScreen->hasElevationInfo = localSetupConfig->LocalScreenInfo->hasElevationInfo;

    if (localSetupConfig->LocalScreenInfo->hasLocalScreenInfo == 1)
    {
      localScreen->az_left = localSetupConfig->LocalScreenInfo->az_left;
      localScreen->az_right = localSetupConfig->LocalScreenInfo->az_right;
      if (localSetupConfig->LocalScreenInfo->hasElevationInfo == 1)
      {
        if (localSetupConfig->LocalScreenInfo->el_top > localSetupConfig->LocalScreenInfo->el_bottom)
        {
          localScreen->el_top = localSetupConfig->LocalScreenInfo->el_top;
          localScreen->el_bottom = localSetupConfig->LocalScreenInfo->el_bottom;
        }
        else
        {
          localScreen->el_bottom = localSetupConfig->LocalScreenInfo->el_top;
          localScreen->el_top = localSetupConfig->LocalScreenInfo->el_bottom;
        }
      }
      else
      {
        localScreen->el_top = 0.0f;
        localScreen->el_bottom = 0.0f;
      }
    }
    else
    {
      localScreen->az_left = 0.0f;
      localScreen->az_right = 0.0f;
      localScreen->el_top = 0.0f;
      localScreen->el_bottom = 0.0f;
    }
  }

  return ( hasScreenRelativeObjects && localScreen->hasLocalScreenInfo );
}

int MP_screenRelatedRemapAzimuth(float *azimuth, H_LOCAL_SCREEN_INFO localScreen, ProductionScreenData productionScreen)
{
  int error = 0;
  float az_temp_out = *azimuth;
  float width_repro= 0.0f, width_refer = 0.0f;
  float az_temp;

  if ((*azimuth < -180.0f) || (*azimuth > 180.0f))
  {
    error = -1;
    return error;
  }

  if (productionScreen.f_azimuth_left > 180.0f)
  {
    productionScreen.f_azimuth_left = productionScreen.f_azimuth_left - 360.0f;
  }
  if (productionScreen.f_azimuth_left < -180.0f)
  {
    productionScreen.f_azimuth_left = productionScreen.f_azimuth_left + 360.0f;
  }
  if (productionScreen.f_azimuth_right > 180.0f)
  {
    productionScreen.f_azimuth_right = productionScreen.f_azimuth_right - 360.0f;
  }
  if (productionScreen.f_azimuth_right < -180.0f)
  {
    productionScreen.f_azimuth_right = productionScreen.f_azimuth_right + 360.0f;
  }

  if (localScreen->az_left > 180.0f)
  {
    localScreen->az_left = localScreen->az_left - 360.0f;
  }
  if (localScreen->az_left < -180.0f)
  {
    localScreen->az_left = localScreen->az_left + 360.0f;
  }
  if (localScreen->az_right > 180.0f)
  {
    localScreen->az_right = localScreen->az_right - 360.0f;
  }
  if (localScreen->az_right < -180.0f)
  {
    localScreen->az_right = localScreen->az_right + 360.0f;
  }

  localScreen->az_right_temp = localScreen->az_right;
  localScreen->az_left_temp = localScreen->az_left;

  productionScreen.f_azimuth_right_temp = productionScreen.f_azimuth_right;
  productionScreen.f_azimuth_left_temp = productionScreen.f_azimuth_left;

  width_repro = localScreen->az_left_temp - localScreen->az_right_temp;
  width_refer = productionScreen.f_azimuth_left_temp - productionScreen.f_azimuth_right_temp;

  az_temp = *azimuth;

  if (localScreen->az_right > localScreen->az_left)
  {
    localScreen->offset = (360.0f + localScreen->az_left + localScreen->az_right) / 2.0f;
  } 
  else
  {
    localScreen->offset = (localScreen->az_left + localScreen->az_right) / 2.0f;
  }
    
  localScreen->az_right_temp = localScreen->az_right - localScreen->offset;
  localScreen->az_left_temp = localScreen->az_left - localScreen->offset;

  if (localScreen->az_left_temp > 180.0f)
  {
    localScreen->az_left_temp = localScreen->az_left_temp - 360.0f;
  }
  if (localScreen->az_left_temp < -180.0f)
  {
    localScreen->az_left_temp = localScreen->az_left_temp + 360.0f;
  }
  if (localScreen->az_right_temp > 180.0f)
  {
    localScreen->az_right_temp = localScreen->az_right_temp - 360.0f;
  }
  if (localScreen->az_right_temp < -180.0f)
  {
    localScreen->az_right_temp = localScreen->az_right_temp + 360.0f;
  }


  if (productionScreen.f_azimuth_right > productionScreen.f_azimuth_left)
  {
    productionScreen.offset = (360.0f + productionScreen.f_azimuth_left + productionScreen.f_azimuth_right ) / 2.0f;
  }
  else
  {
    productionScreen.offset = (productionScreen.f_azimuth_left + productionScreen.f_azimuth_right ) / 2.0f;
  }

  productionScreen.f_azimuth_right_temp = productionScreen.f_azimuth_right - productionScreen.offset;
  productionScreen.f_azimuth_left_temp = productionScreen.f_azimuth_left - productionScreen.offset;

  if (productionScreen.f_azimuth_left_temp > 180.0f)
  {
    productionScreen.f_azimuth_left_temp = productionScreen.f_azimuth_left_temp - 360.0f;
  }
  if (productionScreen.f_azimuth_left_temp < -180.0f)
  {
    productionScreen.f_azimuth_left_temp = productionScreen.f_azimuth_left_temp + 360.0f;
  }
  if (productionScreen.f_azimuth_right_temp  > 180.0f)
  {
    productionScreen.f_azimuth_right_temp  = productionScreen.f_azimuth_right_temp  - 360.0f;
  }
  if (productionScreen.f_azimuth_right_temp  < -180.0f)
  {
    productionScreen.f_azimuth_right_temp  = productionScreen.f_azimuth_right_temp  + 360.0f;
  }

  width_repro = (float)fabs(localScreen->az_left_temp - localScreen->az_right_temp);
  width_refer = (float)fabs(productionScreen.f_azimuth_left_temp - productionScreen.f_azimuth_right_temp);

  az_temp = *azimuth - productionScreen.offset;
  if (az_temp > 180.0f)
  {
    az_temp -= 360.0f;
  }
  if (az_temp < -180.0f)
  {
    az_temp += 360.0f;
  }

  if ((az_temp >= -180.0f) && (az_temp < (productionScreen.f_azimuth_right)))
  {
    float temp = ((localScreen->az_right_temp - (-180.0f))/((productionScreen.f_azimuth_right_temp) - (-180.0f))) * (az_temp - (-180.0f));
    az_temp_out = temp - 180.0f;
  }
  if ((az_temp >= (productionScreen.f_azimuth_right_temp)) && (az_temp < productionScreen.f_azimuth_left_temp))
  {
    float temp = (width_repro / width_refer) * (az_temp - (productionScreen.f_azimuth_right_temp));
    az_temp_out = temp + localScreen->az_right_temp;
  }
  if ((az_temp >= productionScreen.f_azimuth_left_temp) && (az_temp < 180.0f))
  {
    float temp = ((180.0f - localScreen->az_left_temp)/(180.0f - productionScreen.f_azimuth_left_temp)) * (az_temp - productionScreen.f_azimuth_left_temp);
    az_temp_out = temp + localScreen->az_left_temp;
  }

  az_temp_out = az_temp_out + localScreen->offset;
  if (az_temp_out > 180.0f)
  {
    az_temp_out -= 360.0f;
  }
  if (az_temp_out < -180.0f)
  {
    az_temp_out += 360.0f;
  }

  *azimuth = az_temp_out;
  return error;
}

int MP_screenRelatedRemapElevation(float *elevation, H_LOCAL_SCREEN_INFO localScreen, ProductionScreenData productionScreen)
{
  int error = 0;
  float el_temp = *elevation;
  float height_rep = 0.0f, height_ref = 0.0f;
  if ((*elevation < -90.0f) || (*elevation > 90.0f))
  {
    error = -1;
    return error;
  }

  if (productionScreen.f_elevation_top > 90.0f)
  {
    productionScreen.f_elevation_top = 90.0f - (productionScreen.f_elevation_top - 90.0f);
  }
  if (productionScreen.f_elevation_top < -90.0f)
  {
    productionScreen.f_elevation_top = -90.0f + (productionScreen.f_elevation_top + 90.0f);
  }
  if (productionScreen.f_elevation_bottom > 90.0f)
  {
    productionScreen.f_elevation_bottom = 90.0f - (productionScreen.f_elevation_bottom - 90.0f);
  }
  if (productionScreen.f_elevation_bottom < -90.0f)
  {
    productionScreen.f_elevation_bottom = -90.0f + (productionScreen.f_elevation_bottom + 90.0f);
  }

  if (localScreen->el_top > 90.0f)
  {
    localScreen->el_top = 90.0f - (localScreen->el_top - 90.0f);
  }
  if (localScreen->el_top < -90.0f)
  {
    localScreen->el_top = -90.0f + (localScreen->el_top + 90.0f);
  }
  if (localScreen->el_bottom > 90.0f)
  {
    localScreen->el_bottom = 90.0f - (localScreen->el_bottom - 90.0f);
  }
  if (localScreen->el_bottom < -90.0f)
  {
    localScreen->el_bottom = -90.0f + (localScreen->el_bottom + 90.0f);
  }

  height_rep = (float)fabs(localScreen->el_top - localScreen->el_bottom);
  height_ref = (float)fabs(productionScreen.f_elevation_top - productionScreen.f_elevation_bottom);

  if ((*elevation >= -90.0f) && (*elevation < productionScreen.f_elevation_bottom))
  {
    float temp = ((localScreen->el_bottom - (-90.0f))/(productionScreen.f_elevation_bottom - (-90.0f)))*(*elevation - (-90.0f));
    el_temp =  temp + (-90.0f);
  }
  if ((*elevation >= productionScreen.f_elevation_bottom) && (*elevation < productionScreen.f_elevation_top))
  {
    float temp = (height_rep/height_ref)*(*elevation - productionScreen.f_elevation_bottom);
    el_temp = temp + localScreen->el_bottom;
  }
  if ((*elevation >= productionScreen.f_elevation_top) && (*elevation < 90.0f))
  {
    float temp = ((90.0f - localScreen->el_top)/(90.0f - productionScreen.f_elevation_top))*(*elevation - productionScreen.f_elevation_top);
    el_temp = temp + localScreen->el_top;
  }
  
  *elevation = el_temp;

  return error;
}


int MP_screenRelatedZooming(float *azimuth, float *elevation, H_ZOOM_AREA ZoomArea, H_LOCAL_SCREEN_INFO localScreen)
{
  int error = 0;
  float el_temp = *elevation;
  float az_temp = *azimuth;

  float za_az_left;
  float za_az_right;
  float za_el_bottom;
  float za_el_top;

  za_az_left = ZoomArea->AzCenter + ZoomArea->Az;
  za_az_right = ZoomArea->AzCenter - ZoomArea->Az;
  za_el_top =  ZoomArea->ElCenter +  ZoomArea->El;
  za_el_bottom =  ZoomArea->ElCenter -  ZoomArea->El;

  if ((*elevation < -90.0f) || (*elevation > 90.0f) || (*azimuth < -180.0f) || (*azimuth > 180.0f))
  {
    error = -1;
  }
  if (error == 0)
  {
    if ((*elevation >= -90.0f) && (*elevation < za_el_bottom))
    {
      el_temp = ((localScreen->el_bottom+90.0f)/(za_el_bottom+90.0f))*(*elevation + 90.0f) - 90.0f;
    }
    if ((*elevation >= za_el_bottom) && (*elevation < za_el_top))
    {
      el_temp = ((localScreen->el_top - localScreen->el_bottom)/(za_el_top - za_el_bottom))*(*elevation - za_el_bottom) + localScreen->el_bottom;
    }
    if ((*elevation >= za_el_top) && (*elevation < 90.0f))
    {
      el_temp = ((90.0f - localScreen->el_top)/(90.0f - za_el_top))*(*elevation - za_el_top) + localScreen->el_top;
    }


    if ((*azimuth >= -180.0f) && (*azimuth < za_az_right))
    {
      az_temp = ((localScreen->az_right+180.0f)/(za_az_right + 180.0f))*(*azimuth + 180.0f) - 180.0f;
    }
    if ((*azimuth >= za_az_right) && (*azimuth < za_az_left))
    {
      az_temp = ((localScreen->az_left - localScreen->az_right)/(za_az_left - za_az_right))*(*azimuth - za_az_right) + localScreen->az_right;
    }
    if ((*azimuth >= za_az_left) && (*azimuth < 180.0f))
    {
      az_temp = ((180.0f - localScreen->az_left)/(180.0f - za_az_left))*(*azimuth - za_az_left) + localScreen->az_left;
    }
  }

  *azimuth = az_temp;
  *elevation = el_temp;

  return error;
}

int MP_applyScreenRelatedRemappingAndZooming(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int selectedPreset, int divergenceObjectsAdded)
{
  int i, k, j; 
  int error = 0;
  ProductionScreenData productionScreen;
  
  if (h_InteractConfig->enableScreenRelatedProcessing) /* local screen info is available + screen-relative objects are present */
  {
    /* use the standard production screen in case no preset is chosen or it has no production screen */
    productionScreen.hasNonStandardScreenSize = audioSceneConfig->asi.productionScreen.hasNonStandardScreenSize;
    productionScreen.f_azimuth_left = audioSceneConfig->asi.productionScreen.f_azimuth_left;
    productionScreen.f_azimuth_right = audioSceneConfig->asi.productionScreen.f_azimuth_right;
    productionScreen.f_elevation_top = audioSceneConfig->asi.productionScreen.f_elevation_top;
    productionScreen.f_elevation_bottom = audioSceneConfig->asi.productionScreen.f_elevation_bottom;
    if (productionScreen.hasNonStandardScreenSize == 0)
    {
      productionScreen.f_azimuth_left = 29.0f;
      productionScreen.f_azimuth_right = -29.0f;
      productionScreen.f_elevation_top = 17.5f;
      productionScreen.f_elevation_bottom = -17.5f;
    }

    /* check for off-centered screen extension of standard screen */
    if ( audioSceneConfig->asi.productionScreenExtensions.overwriteDefaultProductionScreenAzimuth )
    {
      productionScreen.f_azimuth_left = audioSceneConfig->asi.productionScreenExtensions.f_defaultScreenSizeLeftAz;
      productionScreen.f_azimuth_right = audioSceneConfig->asi.productionScreenExtensions.f_defaultScreenSizeRightAz;
    }

    /* preset is chosen -> use preset-dependent screen */
    if (selectedPreset > -1)
    {
      int hasPresetDepScreen = 0;
      int p = 0;
      int screenIdx = 0;
      for (p = 0; p < audioSceneConfig->asi.productionScreenExtensions.numPresetProductionScreens; p++)
      {
        if (audioSceneConfig->asi.productionScreenExtensions.productionScreenPresetID[p] == selectedPreset)
        {
          hasPresetDepScreen = 1;
          screenIdx = p;
          break;
        }
      }
      if ( hasPresetDepScreen == 1)
      {
        productionScreen.hasNonStandardScreenSize = audioSceneConfig->asi.productionScreenExtensions.hasNonStandardScreenSize[screenIdx];
        productionScreen.f_azimuth_left = audioSceneConfig->asi.productionScreenExtensions.f_azimuth_left[screenIdx];
        productionScreen.f_azimuth_right = audioSceneConfig->asi.productionScreenExtensions.f_azimuth_right[screenIdx];
        productionScreen.f_elevation_top = audioSceneConfig->asi.productionScreenExtensions.f_elevation_top[screenIdx];
        productionScreen.f_elevation_bottom = audioSceneConfig->asi.productionScreenExtensions.f_elevation_bottom[screenIdx];
      }
    }

    h_InteractConfig->screenRelatedObjects = (int*)calloc(MAE_MAX_NUM_ELEMENTS, sizeof(int));
    for (i = 0; i < h_InteractConfig->oamCnt; i++)
    {
      h_InteractConfig->screenRelatedObjects[i] = -1;
      if (audioSceneConfig->screenRelativeObjects[i] == 1) 
      
      {
        h_InteractConfig->screenRelatedObjects[h_InteractConfig->listOAM[i]] = 1; 
      }
    }

    /* check and correct value ranges */
    productionScreen.f_azimuth_left = min(max(productionScreen.f_azimuth_left, -180.0f), 180.0f);
    productionScreen.f_azimuth_right = min(max(productionScreen.f_azimuth_right, -180.0f), 180.0f);
    productionScreen.f_elevation_top = min(max(productionScreen.f_elevation_top, -90.0f), 90.0f);
    productionScreen.f_elevation_bottom = min(max(productionScreen.f_elevation_bottom, -90.0f), 90.0f);

    if (h_InteractConfig->localScreen->hasLocalScreenInfo == 1)
    {
      for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
      {
        /* check if group is active and not routed to a WIRE output */
        if ((h_InteractConfig->onOffStatusModified[i] == 1) && (h_InteractConfig->routeToWireID[i] == -1))
        {
          /* check if group is applicable for screen-related processing (object-based with OAM data) */
          if (signalGroupConfig->signalGroupType[i] == 1) /* not possible for SAOC at the moment */
          {
            int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
            int memberID = -1;
            int oamIdx = -1;
            h_InteractConfig->azimuthModified = (float*)calloc(numMembers,sizeof(float));
            h_InteractConfig->elevationModified = (float*)calloc(numMembers,sizeof(float));

            for (k = 0; k < numMembers; k++)
            {
              if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
              {
                memberID = audioSceneConfig->asi.groups[i].startID + k;
              }
              else
              {
                memberID = audioSceneConfig->asi.groups[i].metaDataElementID[k];
              }

              /* get OAM data for the current group member */
              for (j = 0; j < h_InteractConfig->oamCnt; j++)
              {
                 if (h_InteractConfig->listOAM[j] == memberID)
                 {
                   oamIdx = j;
                   break;
                 }
              }
              if ((oamIdx == -1) || (h_InteractConfig->oamSampleModified == NULL))
              {
                return -1;
              }
              if (divergenceObjectsAdded > 0)
              {
                h_InteractConfig->azimuthModified[k] = h_InteractConfig->oamSampleModified_Divergence->azimuth[oamIdx];
                h_InteractConfig->elevationModified[k] = h_InteractConfig->oamSampleModified_Divergence->elevation[oamIdx];
              }
              else
              {
                h_InteractConfig->azimuthModified[k] = h_InteractConfig->oamSampleModified->azimuth[oamIdx];
                h_InteractConfig->elevationModified[k] = h_InteractConfig->oamSampleModified->elevation[oamIdx];
              }

              if (h_InteractConfig->screenRelatedObjects[memberID] == 1)
              {
                /* screen related remapping */
                error = MP_screenRelatedRemapAzimuth(&h_InteractConfig->azimuthModified[k], h_InteractConfig->localScreen, productionScreen);
                if (h_InteractConfig->localScreen->hasElevationInfo == 1)
                {
                  error = MP_screenRelatedRemapElevation(&h_InteractConfig->elevationModified[k], h_InteractConfig->localScreen, productionScreen);
                }

                /* screen related zooming */
                if (h_InteractConfig->ElementInteractionData != NULL)
                {
                  if (h_InteractConfig->ElementInteractionData->ZoomArea->hasZoomArea == 1)
                  {
                    if (h_InteractConfig->localScreen->hasElevationInfo == 1) /* apply zooming only if local screen info is known */
                    {
                      error = MP_screenRelatedZooming(&h_InteractConfig->azimuthModified[k], &h_InteractConfig->elevationModified[k], h_InteractConfig->ElementInteractionData->ZoomArea, h_InteractConfig->localScreen);
                    }
                  }
                }
              }
              if (divergenceObjectsAdded > 0)
              {
                h_InteractConfig->oamSampleModified_Divergence->azimuth[oamIdx] = h_InteractConfig->azimuthModified[k];
                h_InteractConfig->oamSampleModified_Divergence->elevation[oamIdx] = h_InteractConfig->elevationModified[k];
              }
              else
              {
                h_InteractConfig->oamSampleModified->azimuth[oamIdx] = h_InteractConfig->azimuthModified[k];
                h_InteractConfig->oamSampleModified->elevation[oamIdx] = h_InteractConfig->elevationModified[k];
              }
            }
            free(h_InteractConfig->azimuthModified); h_InteractConfig->azimuthModified = NULL;
            free(h_InteractConfig->elevationModified); h_InteractConfig->elevationModified = NULL;
          }
        }
      }
    }
  }

  return error;
}
