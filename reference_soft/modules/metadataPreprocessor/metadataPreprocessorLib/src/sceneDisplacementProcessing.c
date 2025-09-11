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

Copyright (c) ISO/IEC 2016.

***********************************************************************************/

#include "sceneDisplacementProcessing.h"
#include <math.h>
#include <string.h>

static int MP_SD_writeIdxList(char* outpathIdxList, int num_SDchannels, int* SDchannelIdx)
{
  FILE * idxList = fopen(outpathIdxList, "w");
  int error = 0;
  int i;

  if ( !idxList )
  {
    return -1;
  }

  if (num_SDchannels > 0)
  {
    fprintf(idxList, "%i: ", num_SDchannels);
    for (i = 0; i < num_SDchannels;i++)
    {
      if (i < num_SDchannels -1)
      {
        fprintf(idxList, "%i, ",i);
      }
      else
      {
        fprintf(idxList, "%i",i);
      }
    }
  }

  fclose(idxList);

  return error;
}

int MP_SD_initFileWriting(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInformation, H_OAM_MP_CONFIG h_OamConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_INTERACT_MP_CONFIG h_InteractConfig, char* outpathOAM, char* outpathIdxList, H_LOCAL_SETUP_DATA localSetupConfig)
  {
  int error = 0;
  int num_SDchannels = 0;
  int i, j, ct = 0, idx = -1;
  int SDchannelIdx[MAE_MAX_NUM_ELEMENTS];
  int useTracking = 0;

  h_OamConfig->oamOutFile_SDchannels = NULL;
  h_OamConfig->num_SDchannels = 0;
  strcpy(h_OamConfig->oamOutpath_SDchannels, outpathOAM);
  h_OamConfig->oamSample_SDchannels_out = NULL;

  for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
  {
    int type = MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i);
    int sigGroupIdx = MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i,0);
    int doNotTrack = signalGroupInformation->fixedPosition[sigGroupIdx];

    if (type == 0)
    {
      if (doNotTrack == 0)
      {
        num_SDchannels += audioSceneConfig->asi.groups[i].groupNumMembers;
        for (j = 0; j < audioSceneConfig->asi.groups[i].groupNumMembers; j++)
        {
          idx++;
          SDchannelIdx[ct] = idx;
          ct++;
        }
      }
      else
      {
        idx += audioSceneConfig->asi.groups[i].groupNumMembers;
      }
    }
  }

  if (localSetupConfig->rendering_type == 0)
  {
    useTracking = localSetupConfig->LoudspeakerRendering->useTrackingMode;
  }
  else
  {
    useTracking = localSetupConfig->BinauralRendering->pBR->useTrackingMode;
  }

  if ((num_SDchannels > 0) && (useTracking == 1))
  {
    h_OamConfig->num_SDchannels = num_SDchannels;
    h_OamConfig->oamOutFile_SDchannels = oam_write_open(h_OamConfig->oamOutpath_SDchannels,num_SDchannels,h_OamConfig->oam_version,0,1);
    h_OamConfig->oamSample_SDchannels_out = oam_multidata_create(num_SDchannels,1);

    MP_SD_writeIdxList(outpathIdxList, num_SDchannels, SDchannelIdx);
  }

  h_OamConfig->oamSample_SDchannels_out = oam_multidata_create(num_SDchannels,1);

  return error;
}

int MP_applySceneDisplacement_CO(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig,  H_LOCAL_SETUP_DATA localSetupConfig, int oamDataAvailable, SceneDisplacementData *sceneDisplacementData, H_OAM_MP_CONFIG h_oamConfig, int divergenceObjectsAdded )
{
  int error = 0;
  float az_this = 0.0f, el_this = 0.0f, r_this = 1.0f;
  float x_this, y_this, z_this;
  int useTrackingMode = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoRep[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numGroups = audioSceneConfig->asi.numGroups;
  int i, j, k, s;
  int numReproSpeakers = 0;
  int countObjects = 0;
  int countChannels = 0;
  float maxLSdistance = 0.0f;


  if (localSetupConfig->rendering_type == 0)
  {
    useTrackingMode = localSetupConfig->LoudspeakerRendering->useTrackingMode;
  }
  else
  {
    useTrackingMode = localSetupConfig->BinauralRendering->pBR->useTrackingMode;
  }

  if (useTrackingMode == 1)
  {
    /* get reproduction setup */
    if (localSetupConfig->rendering_type == 0)
    {
      int layoutType = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType;
      int nChannels = 0;
      int nLFEs = 0;
      numReproSpeakers = localSetupConfig->LoudspeakerRendering->numSpeakers;

      for (i = 0; i < numReproSpeakers; i++)
      {
        if ((layoutType == 0) && (i == 0))
        {
          cicp2geometry_get_geometry_from_cicp(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
        }
        else if (layoutType == 1)
        {
          cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[i],&geoRep[i]);
        }
        else if (layoutType == 2)
        {
          if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[i] == 1)
          {
            cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[i], &geoRep[i]);
          }
          else
          {
            geoRep[i].Az = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[i];
            geoRep[i].El = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[i];
            geoRep[i].LFE = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[i];
            geoRep[i].screenRelative = 0;
          }
        }
      }
    }
    else
    {
      /* binaural reproduction */
      int layoutType = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType;
      int nChannels = 0;
      int nLFEs = 0;
      numReproSpeakers = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->nBrirPairs;

      for (i = 0; i < numReproSpeakers; i++)
      {
        if ((layoutType == 0) && (i == 0))
        {
          cicp2geometry_get_geometry_from_cicp(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
        }
        else if (layoutType == 1)
        {
          cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerIdx[i],&geoRep[i]);
        }
        else if (layoutType == 2)
        {
          if (localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx == 1)
          {
            cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx, &geoRep[i]);
          }
          else
          {
            int azAngleIdx = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx;
            int azDir = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection;
            int elAngleIdx = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx;
            int elDir = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].ElevationDirection;
            int prec = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.angularPrecision;

            geoRep[i].El = ascparser_ElevationAngleIdxToDegrees(elAngleIdx,elDir,prec);
            geoRep[i].Az = ascparser_AzimuthAngleIdxToDegrees(azAngleIdx,azDir,prec);
            geoRep[i].cicpLoudspeakerIndex = -1;
            geoRep[i].LFE = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.flexibleSpeakerConfig.speakerDescription[i].isLFE;
            geoRep[i].screenRelative = 0;
          }
        }
      }
    }

    countObjects = 0;
    countChannels = 0;

    for (i = 0; i < numGroups; i++)
    {
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      int type = MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i);
      int updatePositions = 0;

      if ((type == 1) && (!oamDataAvailable))
      {
        break;
        return -1;
      }

      /* check if group is active and not routed to a WIRE output */
      /* if ((h_InteractConfig->onOffStatusModified[i] == 1) && (h_InteractConfig->routeToWireID[i] == -1))
      { */
        /* check if scene displacement processing is enabled for the current group */
        if (signalGroupInfo->fixedPosition[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig, i, 0)] == 0) /* assume mae_group as subset of signal group */
        {
          updatePositions = 1;
        }

        if (updatePositions == 1)
        {
          int memberID = -1;
          int oamIdx = -1;
          h_InteractConfig->azimuthModified = (float*)calloc(numMembers,sizeof(float));
          h_InteractConfig->elevationModified = (float*)calloc(numMembers,sizeof(float));
          h_InteractConfig->distanceModified = (float*)calloc(numMembers,sizeof(float));

          maxLSdistance = 0.0f;
          for (s = 0; s < numReproSpeakers; s++)
          {
            if (localSetupConfig->rendering_type == 0)
            {
              int hasKnownPos = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][s];
              if (hasKnownPos == 1)
              {
                if ((localSetupConfig->LoudspeakerRendering->loudspeakerDistance != NULL) && (localSetupConfig->LoudspeakerRendering->loudspeakerDistance[s] > 0.0f))
                {
                  maxLSdistance = max(maxLSdistance,(float)localSetupConfig->LoudspeakerRendering->loudspeakerDistance[s]);
                }
              }
            }
          }

          for (k = 0; k < numMembers; k++)
          {
            if (type == 0) /* CHANNELS */
            {
              CICP2GEOMETRY_CHANNEL_GEOMETRY geo[CICP2GEOMETRY_MAX_LOUDSPEAKERS];

              /* get intended speaker position*/
              if (signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].CICPspeakerLayoutIdx > 0) /* CICP speaker layout idx */
              {
                int numChannels = -1;
                int numLFEs = -1;
                cicp2geometry_get_geometry_from_cicp(signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].CICPspeakerLayoutIdx, geo, &numChannels, &numLFEs);
                h_InteractConfig->azimuthModified[k] = (float)geo[k].Az;
                h_InteractConfig->elevationModified[k] = (float)geo[k].El;
                if (maxLSdistance > 0.0f)
                {
                  h_InteractConfig->distanceModified[k] = maxLSdistance;
                }
                else
                {
                  h_InteractConfig->distanceModified[k] = 1023.0f;
                }
                if ((numChannels + numLFEs) != numMembers)
                {
                  error = -1;
                  break;
                }
              }
              else if (signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].CICPspeakerIdx[k] >= 0) /* list of CICP speaker idx */
              {
                if (signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].numSpeakers != numMembers)
                {
                  error = -1;
                  break;
                }
                cicp2geometry_get_geometry_from_cicp_loudspeaker_index(signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].CICPspeakerIdx[k], &geo[k]);
                h_InteractConfig->azimuthModified[k] = (float)geo[k].Az;
                h_InteractConfig->elevationModified[k] = (float)geo[k].El;
                if (maxLSdistance > 0.0f)
                {
                  h_InteractConfig->distanceModified[k] = maxLSdistance;
                }
                else
                {
                  h_InteractConfig->distanceModified[k] = 1023.0f;
                }
              }
              else /* flexible signaling */
              {
                if (signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].numSpeakers != numMembers)
                {
                  error = -1;
                  break;
                }
                if (signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].flexibleSpeakerConfig.speakerDescription[k].isCICPspeakerIdx == 1)
                {
                  cicp2geometry_get_geometry_from_cicp_loudspeaker_index(signalGroupConfig->signalGroupChannelLayout[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k)].flexibleSpeakerConfig.speakerDescription[k].CICPspeakerIdx, &geo[k]);
                  h_InteractConfig->azimuthModified[k] = (float)geo[k].Az;
                  h_InteractConfig->elevationModified[k] = (float)geo[k].El;
                  if (maxLSdistance > 0.0f)
                  {
                    h_InteractConfig->distanceModified[k] = maxLSdistance;
                  }
                  else
                  {
                    h_InteractConfig->distanceModified[k] = 1023.0f;
                  }
                }
                else
                {
                  int sGroupIdx = MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig,i, k);
                  int azAngleIdx = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.speakerDescription[k].AzimuthAngleIdx;
                  int azDir = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.speakerDescription[k].AzimuthDirection;
                  int elAngleIdx = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.speakerDescription[k].ElevationAngleIdx;
                  int elDir = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.speakerDescription[k].ElevationDirection;
                  int prec = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.angularPrecision;

                  geo[k].El = ascparser_ElevationAngleIdxToDegrees(elAngleIdx,elDir,prec);
                  geo[k].Az = ascparser_AzimuthAngleIdxToDegrees(azAngleIdx,azDir,prec);
                  geo[k].cicpLoudspeakerIndex = -1;
                  geo[k].LFE = signalGroupConfig->signalGroupChannelLayout[sGroupIdx].flexibleSpeakerConfig.speakerDescription[k].isLFE;
                  geo[k].screenRelative = 0;

                  h_InteractConfig->azimuthModified[k] = (float)geo[k].Az;
                  h_InteractConfig->elevationModified[k] = (float)geo[k].El;
                  if (maxLSdistance > 0.0f)
                  {
                    h_InteractConfig->distanceModified[k] = maxLSdistance;
                  }
                  else
                  {
                    h_InteractConfig->distanceModified[k] = 1023.0f;
                  }
                }
              }

              /* check if the speaker exists in reproduction setup and include known position if so */
              for (s = 0; s < numReproSpeakers; s++)
              {
                int exists = cicp2geometry_compare_geometry(&geo[k],1,&geoRep[s],1,0);
                if (exists == 0)
                {
                  if (localSetupConfig->rendering_type == 0)
                  {
                    int hasKnownPos = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][s];
                    if (hasKnownPos == 1)
                    {
                      /* overwrite geometry with known reproduction speaker position */
                      h_InteractConfig->azimuthModified[k] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][s];
                      h_InteractConfig->elevationModified[k] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][s];
                      /* overwrite distance with speaker distance */
                      if ((localSetupConfig->LoudspeakerRendering->loudspeakerDistance != NULL) && (localSetupConfig->LoudspeakerRendering->loudspeakerDistance[s] > 0.0f))
                      {
                        h_InteractConfig->distanceModified[k] = (float)localSetupConfig->LoudspeakerRendering->loudspeakerDistance[s];
                      }
                    }
                  }
                  else /* binaural rendering */
                  {
                    /* set to BRIR measurement position */
                    h_InteractConfig->azimuthModified[k] = (float)geoRep[s].Az;
                    h_InteractConfig->elevationModified[k] = (float)geoRep[s].El;
                  }
                  break;
                }
              }
              countChannels++;
            }
            else if (type == 1) /* OBJECTS */
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
                h_InteractConfig->distanceModified[k] = h_InteractConfig->oamSampleModified_Divergence->radius[oamIdx];
              }
              else
              {
                h_InteractConfig->azimuthModified[k] = h_InteractConfig->oamSampleModified->azimuth[oamIdx];
                h_InteractConfig->elevationModified[k] = h_InteractConfig->oamSampleModified->elevation[oamIdx];
                h_InteractConfig->distanceModified[k] = h_InteractConfig->oamSampleModified->radius[oamIdx];
              }
              countObjects++;
            }
            az_this = h_InteractConfig->azimuthModified[k];
            el_this = h_InteractConfig->elevationModified[k];
            r_this = h_InteractConfig->distanceModified[k];

            MP_getCommonAzEl(&az_this, &el_this);
            MP_commonAzEl2Cart(az_this, el_this, r_this, &x_this, &y_this, &z_this);
            MP_rotateCartPos(&x_this, &y_this, &z_this, sceneDisplacementData);
            MP_cart2commonAzEl(x_this, y_this, z_this, &az_this, &el_this, &r_this);
            MP_getMmpegAzEl(&az_this, &el_this);

            if (type == 0) /* CHANNELS */
            {
              /* set oam output sample */
              h_oamConfig->oamSample_SDchannels_out->sample[countChannels-1] = h_oamConfig->oamSample_in->sample[0];

              h_oamConfig->oamSample_SDchannels_out->azimuth[countChannels-1] = az_this;
              h_oamConfig->oamSample_SDchannels_out->elevation[countChannels-1] = el_this;
              h_oamConfig->oamSample_SDchannels_out->radius[countChannels-1] = r_this;

              h_oamConfig->oamSample_SDchannels_out->gain[countChannels-1] = 1.0f;
              h_oamConfig->oamSample_SDchannels_out->spread[countChannels-1] = 0.0f;
              h_oamConfig->oamSample_SDchannels_out->spread_depth[countChannels-1] = 0.0f;
              h_oamConfig->oamSample_SDchannels_out->spread_height[countChannels-1] = 0.0f;
              h_oamConfig->oamSample_SDchannels_out->dynamic_object_priority[countChannels-1] = 7;
            }
            else if (type == 1) /* OBJECTS */
            {
              if (divergenceObjectsAdded > 0)
              {
                h_InteractConfig->oamSampleModified_Divergence->azimuth[oamIdx] = az_this;
                h_InteractConfig->oamSampleModified_Divergence->elevation[oamIdx] = el_this;
                h_InteractConfig->oamSampleModified_Divergence->radius[oamIdx] = r_this;
              }
              else
              {
                h_InteractConfig->oamSampleModified->azimuth[oamIdx] = az_this;
                h_InteractConfig->oamSampleModified->elevation[oamIdx] = el_this;
                h_InteractConfig->oamSampleModified->radius[oamIdx] = r_this;
              }
            }
          }
          free(h_InteractConfig->azimuthModified); h_InteractConfig->azimuthModified = NULL;
          free(h_InteractConfig->elevationModified); h_InteractConfig->elevationModified = NULL;
          free(h_InteractConfig->distanceModified); h_InteractConfig->distanceModified = NULL;
        }
      /* } */
    }
    if (h_oamConfig->num_SDchannels > 0)
    {
      oam_write_process(h_oamConfig->oamOutFile_SDchannels, h_oamConfig->oamSample_SDchannels_out, h_oamConfig->oam_version, 0, 1);
    }
  }
  return error;
}

static float MP_rad(float deg)
{
  return (deg * (float)M_PI / 180.0f);
}

static float MP_deg(float rad)
{
  return (rad * 180.0f / (float)M_PI);
}

void MP_getCommonAzEl(float *az, float *el)
{
  *az = *az + 90.0f;
  *el = 90.0f - *el;
}

void MP_cart2commonAzEl(float x, float y, float z, float *az, float *el, float *r)
{
  *r = (float)sqrt(x*x+y*y+z*z);
  *el = MP_deg((float)acos(z/(*r)));
  *az = MP_deg((float)atan2(y,x));
}

void MP_commonAzEl2Cart(float az, float el, float r, float *x, float *y, float *z)
{
  *x = r*(float)sin(MP_rad(el))*(float)cos(MP_rad(az));
  *y = r*(float)sin(MP_rad(el))*(float)sin(MP_rad(az));
  *z = r*(float)cos(MP_rad(el));
}

void MP_getMmpegAzEl(float *az, float *el)
{
  *az = *az - 90.0f;
  *el = 90.0f - *el;

  if (*az > 180.0f)
  {
    *az -= 360.0f;
  }
  if (*az < -180.0f)
  {
    *az += 360.0f;
  }

  if (*el > 90.0f)
  {
    *el = 90.0f - (*el - 90.0f);
  }
  if (*el < -90.0f)
  {
    *el = -90.0f + (*el + 90.0f);
  }
}

void MP_rotateCartPos(float *x, float *y, float *z, SceneDisplacementData const *sceneDsplData)
{
  float matrix[3][3] = {{0}};
  float temp[3][3] = {{0}};
  float posOut[3] = {0};
  float posIn[3] = {0};
  float rot1[3][3];
  float rot2[3][3];
  float rot3[3][3];

  /* Rotational scene displacement parameters */
  float const yaw = sceneDsplData->rotSceneDsplData.yaw;
  float const pitch = sceneDsplData->rotSceneDsplData.pitch;
  float const roll = sceneDsplData->rotSceneDsplData.roll;

  /* Positional scene displacement parameters */
  float dxyz[3] = { 0 };
  float sd_azimuth = sceneDsplData->posSceneDsplData.sd_azimuth;
  float sd_elevation = sceneDsplData->posSceneDsplData.sd_elevation;

  MP_getCommonAzEl(&sd_azimuth, &sd_elevation);
  MP_commonAzEl2Cart(sd_azimuth,
                     sd_elevation,
                     sceneDsplData->posSceneDsplData.sd_radius,
                     &dxyz[0], &dxyz[1], &dxyz[2]);

  posIn[0] = *x;
  posIn[1] = *y;
  posIn[2] = *z;

  rot1[0][0] = (float)cos(MP_rad(yaw));
  rot1[0][1] = (float)sin(MP_rad(yaw));
  rot1[0][2] = 0;
  rot1[1][0] = (float)-sin(MP_rad(yaw));
  rot1[1][1] = (float)cos(MP_rad(yaw));
  rot1[1][2] = 0;
  rot1[2][0] = 0;
  rot1[2][1] = 0;
  rot1[2][2] = 1;

  rot2[0][0] = 1;
  rot2[0][1] = 0;
  rot2[0][2] = 0;
  rot2[1][0] = 0;
  rot2[1][1] = (float)cos(MP_rad(pitch));
  rot2[1][2] = (float)-sin(MP_rad(pitch));
  rot2[2][0] = 0;
  rot2[2][1] = (float)sin(MP_rad(pitch));
  rot2[2][2] = (float)cos(MP_rad(pitch));

  rot3[0][0] = (float)cos(MP_rad(roll));
  rot3[0][1] = 0;
  rot3[0][2] = (float)sin(MP_rad(roll));
  rot3[1][0] = 0;
  rot3[1][1] = 1;
  rot3[1][2] = 0;
  rot3[2][0] = (float)-sin(MP_rad(roll));
  rot3[2][1] = 0;
  rot3[2][2] = (float)cos(MP_rad(roll));

  { /* multiply rot1, rot2 */
    int l, c, i;
    for (l = 0; l < 3; l++)
    {
      for (c = 0; c < 3; c++)
      {
        (temp[l][c]) = 0.0f;
        for (i = 0; i < 3; i++)
        {
          (temp[l][c]) += (rot1[l][i] * rot2[i][c]);
        }
      }
    }
  }
  { /* multiply temp, rot3 */
    int l, c, i;
    for (l = 0; l < 3; l++)
    {
      for (c = 0; c < 3; c++)
      {
        (matrix[l][c]) = 0.0f;
        for (i = 0; i < 3; i++)
        {
          (matrix[l][c]) += (temp[l][i] * rot3[i][c]);
        }
      }
    }
  }

  { /* post-multiply with position */
    int i,k;
    for (i = 0; i < 3; i++)
    {
      posOut[i] = 0.0f;
      for (k = 0; k < 3; k++)
      {
        posOut[i] += matrix[i][k] * (posIn[k] + dxyz[k]);
      }
    }
  }

  *x = posOut[0];
  *y = posOut[1];
  *z = posOut[2];
}
