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

#include "metadataPreprocessor.h"
#include "screenRelatedProcessing.h"
#include "sceneDisplacementProcessing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "speakerConfig3d.h"
#include "oam_common.h"

int MP_getSignalGroupIndex(ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int groupIndex, int memberIndex)
{
  int i, j;
  int sGroupIndex = -1;
  int maeID = -1;
  int maeIDin = -1;

  for (i = 0; i < memberIndex; i++)
  {
    if (audioSceneConfig->asi.groups[groupIndex].hasConjunctMembers == 1)
    {
      maeIDin = audioSceneConfig->asi.groups[groupIndex].startID + memberIndex;
    }
    else
    {
      maeIDin = audioSceneConfig->asi.groups[groupIndex].metaDataElementID[memberIndex];
    }
  }

  for (i = 0; i < (int)signalGroupConfig->numSignalGroups; i++)
  {
    for (j = 0; j < (int)signalGroupConfig->numberOfSignals[i]; j++)
    {
      maeID++;
      if (maeID == maeIDin)
      {
        sGroupIndex = i;
        break;
      }
    }
  }

  if (sGroupIndex == -1)
  {
    /* old signaling as fallback*/
     sGroupIndex = groupIndex;
  }

  return sGroupIndex;
}

int MP_getGroupID(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int elementID)
{
  int i, j, maeID;

  for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
  {
    for (j = 0; j < audioSceneConfig->asi.groups[i].groupNumMembers; j++)
    {
      if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 1)
      {
        maeID = audioSceneConfig->asi.groups[i].startID + j;
      }
      else
      {
        maeID = audioSceneConfig->asi.groups[i].metaDataElementID[j];
      }

      if (maeID == elementID)
      {
        return audioSceneConfig->asi.groups[i].groupID;
      }
    }
  }
  return -1;
}

int MP_getSignalGroupType(ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int groupIndex)
{

  /* new signaling, assuming all members from one signal group (and of equal type) */
  int startID = audioSceneConfig->asi.groups[groupIndex].startID;

  int i, j;
  int sGroupIndex = -1;
  int groupType = -1;
  int maeID = -1;

  for (i = 0; i < (int)signalGroupConfig->numSignalGroups; i++)
  {
    for (j = 0; j < (int)signalGroupConfig->numberOfSignals[i]; j++)
    {
      maeID++;
      if (maeID == startID)
      {
        sGroupIndex = i;
        break;
      }
    }
  }

  if (sGroupIndex == -1)
  {
    /* old signaling as fallback*/
     groupType = signalGroupConfig->signalGroupType[groupIndex];
  }
  else
  {
    groupType = signalGroupConfig->signalGroupType[sGroupIndex];
  }

  return groupType;
}

int MP_applyDivergenceProcessing(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, int* overallNumDivergenceObjectsAdded, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig)
{
  int i,j,k,m;
  int error = 0;
  int ct = 0;
  int n;

  if (h_InteractConfig->numObjects_out > h_InteractConfig->numObjects_in)
  {
    if (h_InteractConfig->divergence_ASImodified == 0)
    {
      for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
      {
        h_InteractConfig->origNumGroupMembers[i] = audioSceneConfig->asi.groups[i].groupNumMembers;
      }
    }

    /* copy modified OAM data to divergence multidata instance "oamSampleModified_Divergence" */
    error = MP_copyOamFrame(h_InteractConfig->oamSampleModified_Divergence, h_InteractConfig->oamSampleModified, h_InteractConfig->numObjects_in, 0);

    /* copy all original audio data to divergence buffer */
    if (h_AudioConfig->decode_qmf)
    {
      for (j = 0; j < h_InteractConfig->numElements_in; j++)
      {
        for (k = 0; k < QMFLIB_NUMBANDS; k++)
        {
          for (i = 0; i < h_AudioConfig->numTimeslots; i++)
          {
            h_AudioConfig->audioQmfBuffer_divergence_real[k][j][i] = h_AudioConfig->audioQmfBuffer_real[k][j][i];
            h_AudioConfig->audioQmfBuffer_divergence_imag[k][j][i] = h_AudioConfig->audioQmfBuffer_imag[k][j][i];
          }
        }
      }
    }
    else if (h_AudioConfig->decode_wav)
    {
      for (j = 0; j < h_InteractConfig->numElements_in; j++)
      {
        for (k = 0; k < h_AudioConfig->audio_blocksize; k++)
        {
          h_AudioConfig->audioTdBuffer_divergence[j][k] = h_AudioConfig->audioTdBuffer[j][k];
        }
      }
    }

    *overallNumDivergenceObjectsAdded = 0;
    for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
    {
      int newOamSize = h_InteractConfig->oamSampleModified->size1;
      float divergence_az = 0.0f;
      float divergence_gain_orig = 0.0f;
      int screenRelatedStatusOrig = 0;
      int count = 0;
      int maxMaeID = 0;
      int source_el, maeID = 0;
      int bnd, ts;
      float tempAz = 0.0f;
      float tempGain_orig = 1.0f, tempGain_virt = 1.0f, modGain_orig = 1.0f, modGain_virt = 1.0f;
      int hasDivergence = 0;
      float divergence = 0.0f;

      if (MP_getSignalGroupType(audioSceneConfig,signalGroupConfig,i) == 1)
      {
        if (h_InteractConfig->divergence_ASImodified == 1)
        {
          n = h_InteractConfig->origNumGroupMembers[i];
        }
        else
        {
          n = audioSceneConfig->asi.groups[i].groupNumMembers;
        }

        for (k = 0; k < n; k++)
        {
          hasDivergence = enhancedObjectMetadataConfig->hasDivergence[ct];

          maeID = 0;
          if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
          {
            maeID = audioSceneConfig->asi.groups[i].startID + k;
          }
          else
          {
            maeID = audioSceneConfig->asi.groups[i].metaDataElementID[k];
          }

          if (hasDivergence)
          {
            for ( m = 0; m < h_InteractConfig->oamCnt; m++ )
            {
              if ( h_InteractConfig->listOAM[m] == maeID )
              {
                divergence_az = h_InteractConfig->oamSampleModified_Divergence->azimuth[m];
                divergence_gain_orig = h_InteractConfig->oamSampleModified_Divergence->gain[m];
                screenRelatedStatusOrig = audioSceneConfig->screenRelativeObjects[m];
                break;
              }
            }

            tempAz = divergence_az - enhancedObjectMetadataConfig->divergenceAzimuthRange[ct];

            if (tempAz > 180.0f)
            {
              tempAz -= 360.0f;
            }
            if (tempAz < -180.0f)
            {
              tempAz += 360.0f;
            }

            divergence = h_enhObjMdFrame->divergence[ct];

            tempGain_orig = (float)sqrt(1.0f - (float)pow(divergence, 0.58497f));
            modGain_orig = divergence_gain_orig*tempGain_orig;
            tempGain_virt = (float)sqrt(0.5f * ((float)pow(divergence, 0.58497f)));
            modGain_virt =divergence_gain_orig*tempGain_virt;

            h_InteractConfig->oamSampleModified_Divergence->gain[m] = modGain_orig;

            /* add first virtual object */
            h_InteractConfig->oamSampleModified_Divergence->azimuth[h_InteractConfig->numObjects_in + count] = tempAz;
            h_InteractConfig->oamSampleModified_Divergence->gain[h_InteractConfig->numObjects_in + count] = modGain_virt;

            h_InteractConfig->oamSampleModified_Divergence->sample[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->sample[m];
            h_InteractConfig->oamSampleModified_Divergence->spread[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->spread[m];
            h_InteractConfig->oamSampleModified_Divergence->spread_depth[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->spread_depth[m];
            h_InteractConfig->oamSampleModified_Divergence->spread_height[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->spread_height[m];
            h_InteractConfig->oamSampleModified_Divergence->dynamic_object_priority[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->dynamic_object_priority[m];
            h_InteractConfig->oamSampleModified_Divergence->elevation[h_InteractConfig->numObjects_in + count] = h_InteractConfig->oamSampleModified_Divergence->elevation[m];

            (*overallNumDivergenceObjectsAdded)++;
            maxMaeID = MP_getMaxMaeID(audioSceneConfig);
            h_InteractConfig->listOAM[h_InteractConfig->oamCnt - 1 + *overallNumDivergenceObjectsAdded] = maxMaeID + *overallNumDivergenceObjectsAdded;

            /* update group definition, part 1*/
            if (h_InteractConfig->divergence_ASImodified == 0)
            {
              if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
              {
                audioSceneConfig->asi.groups[i].metaDataElementID[k] = audioSceneConfig->asi.groups[i].startID + k;
              }
              audioSceneConfig->asi.groups[i].metaDataElementID[audioSceneConfig->asi.groups[i].groupNumMembers] = maxMaeID + *overallNumDivergenceObjectsAdded;

              /* update screen-relative object list */
              audioSceneConfig->screenRelativeObjects[h_InteractConfig->oamCnt - 1 + *overallNumDivergenceObjectsAdded] = screenRelatedStatusOrig;
            }

            /* duplicate audio data */
            source_el = maeID;
            if (h_AudioConfig->decode_qmf)
            {
              for (bnd = 0; bnd < QMFLIB_NUMBANDS; bnd++)
              {
                for (ts = 0; ts < h_AudioConfig->numTimeslots; ts++)
                {
                  h_AudioConfig->audioQmfBuffer_divergence_real[bnd][h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioQmfBuffer_divergence_real[bnd][source_el][ts];
                  h_AudioConfig->audioQmfBuffer_divergence_imag[bnd][h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioQmfBuffer_divergence_imag[bnd][source_el][ts];
                }
              }
            }
            else if (h_AudioConfig->decode_wav)
            {
              for (ts = 0; ts < h_AudioConfig->audio_blocksize; ts++)
              {
                h_AudioConfig->audioTdBuffer_divergence[h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioTdBuffer_divergence[source_el][ts];
              }
            }

            /* add second virtual object */
            tempAz = divergence_az + enhancedObjectMetadataConfig->divergenceAzimuthRange[ct];
            if (tempAz > 180.0f)
            {
              tempAz -= 360.0f;
            }
            if (tempAz < -180.0f)
            {
              tempAz += 360.0f;
            }

            h_InteractConfig->oamSampleModified_Divergence->azimuth[h_InteractConfig->numObjects_in + count  + 1] = tempAz;
            h_InteractConfig->oamSampleModified_Divergence->gain[h_InteractConfig->numObjects_in + count + 1] = modGain_virt;

            h_InteractConfig->oamSampleModified_Divergence->sample[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->sample[m];
            h_InteractConfig->oamSampleModified_Divergence->spread[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->spread[m];
            h_InteractConfig->oamSampleModified_Divergence->spread_depth[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->spread_depth[m];
            h_InteractConfig->oamSampleModified_Divergence->spread_height[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->spread_height[m];
            h_InteractConfig->oamSampleModified_Divergence->dynamic_object_priority[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->dynamic_object_priority[m];
            h_InteractConfig->oamSampleModified_Divergence->elevation[h_InteractConfig->numObjects_in + count + 1] = h_InteractConfig->oamSampleModified_Divergence->elevation[m];

            (*overallNumDivergenceObjectsAdded)++;
            h_InteractConfig->listOAM[h_InteractConfig->oamCnt - 1 + *overallNumDivergenceObjectsAdded] = maxMaeID + *overallNumDivergenceObjectsAdded;

            if (h_InteractConfig->divergence_ASImodified == 0)
            {
               /* update group definition, part 2*/
              audioSceneConfig->asi.groups[i].metaDataElementID[audioSceneConfig->asi.groups[i].groupNumMembers + 1 ] = maxMaeID + *overallNumDivergenceObjectsAdded;

              /* update screen-relative object list */
              audioSceneConfig->screenRelativeObjects[h_InteractConfig->oamCnt - 1 + *overallNumDivergenceObjectsAdded] = screenRelatedStatusOrig;
            }

            /* duplicate audio data */
            source_el = maeID;
            if (h_AudioConfig->decode_qmf)
            {
              for (bnd = 0; bnd < QMFLIB_NUMBANDS; bnd++)
              {
                for (ts = 0; ts < h_AudioConfig->numTimeslots; ts++)
                {
                  h_AudioConfig->audioQmfBuffer_divergence_real[bnd][h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioQmfBuffer_divergence_real[bnd][source_el][ts];
                  h_AudioConfig->audioQmfBuffer_divergence_imag[bnd][h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioQmfBuffer_divergence_imag[bnd][source_el][ts];
                }
              }
            }
            else if (h_AudioConfig->decode_wav)
            {
              for (ts = 0; ts < h_AudioConfig->audio_blocksize; ts++)
              {
                h_AudioConfig->audioTdBuffer_divergence[h_InteractConfig->numElements_in - 1 + *overallNumDivergenceObjectsAdded][ts] = h_AudioConfig->audioTdBuffer_divergence[source_el][ts];
              }
            }
            count += 2;
            ct++;
          }
        }
        /* update size information of OAM multidata */
        newOamSize += count;
        h_InteractConfig->oamCnt = newOamSize;
        h_InteractConfig->oamSampleModified_Divergence->size1 = newOamSize;

        if (h_InteractConfig->divergence_ASImodified == 0)
        {
          /* final update group definition */
          audioSceneConfig->asi.groups[i].hasConjunctMembers = 0;
          audioSceneConfig->asi.groups[i].groupNumMembers += count;
        }
      }
    }
    if ((h_InteractConfig->numObjects_in + *overallNumDivergenceObjectsAdded) != h_InteractConfig->numObjects_out)
    {
      error = -1;
    }
    if (h_InteractConfig->divergence_ASImodified == 0)
    {
      h_InteractConfig->divergence_ASImodified = 1;
    }
  }
  return error;
}

float  MP_getDiffuseDecorrOutputSample(float inputSample, int speakerIndex, H_INTERACT_MP_CONFIG h_InteractConfig)
{
  float result = 0.0f;
  int index = 0;
  int i;
  int length = h_InteractConfig->diffuse_filterLength;

  index = h_InteractConfig->diffuse_counter;
  h_InteractConfig->diffuse_decorrStates[speakerIndex][index] = inputSample;

  for (i = 0; i < length; i++)
  {
    result += h_InteractConfig->diffuse_decorrFilters[speakerIndex][i] * h_InteractConfig->diffuse_decorrStates[speakerIndex][index--];
    if (index < 0)
    {
      index = length-1;
    }
  }
  if (++h_InteractConfig->diffuse_counter >= length)
  {
    h_InteractConfig->diffuse_counter = 0;
  }

  return result;
}

int MP_initDiffusenessRendering(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, int decoderDomain, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig)
{
  int error = 0;
  int i, j;
  int diffuse_enableProcessing = 0;

  /* matlab code for decorrelation filters */
  /*
  r = rand(128,1);
  g = zeros(128,1);
  for n = 1:1:128
      if (n <= 0.45*128)
          g(n) = 1-2*n/128;
      else
          g(n) = 0.1;
      end
      g2(n) = max(1-2*n/128,0.1);
      H(n) = exp(1i*(-pi + 2*pi * r(n))*g(n));
  end
  re = real(H);
  im = imag(H);
  im(1) = 0;
  re2 = [re 0 fliplr(re(2:end))];
  im2 = [im 0 -fliplr(im(2:end))];
  H2 = re2 + 1i*im2;
  fir = ifft(H2,256);
  e_fir = sum(fir.^2);
  fir_out = fir ./ sqrt(e_fir); */

  static float decorrFilters[MAE_MAX_NUM_SPEAKERS][MAE_DIFFUSE_DECORR_LENGTH] = {
    {0.77983f,-0.17504f,-0.11389f,-0.055489f,-0.030905f,0.0089118f,-0.046661f,0.021093f,0.039129f,0.06625f,0.058199f,0.034139f,-0.0068986f,0.00080282f,-0.004651f,-0.051595f,-0.049802f,-0.057208f,-0.081991f,-0.049547f,-0.0082012f,-0.0021248f,-0.00067349f,0.026082f,0.00013452f,0.01621f,-0.021305f,-0.023604f,-0.044458f,0.014735f,0.019507f,0.035908f,0.044972f,0.049512f,0.049148f,0.044871f,0.063502f,0.094869f,0.068237f,0.030415f,-0.033047f,-0.047439f,-0.049462f,-0.051311f,-0.070446f,-0.05625f,-0.030716f,-0.018834f,0.048657f,0.06486f,0.057254f,0.0085426f,-0.029804f,-0.015407f,0.00092508f,0.018974f,0.030149f,0.00070771f,-0.037566f,-0.05099f,-0.053878f,-0.047075f,-0.062686f,-0.058217f,-0.035275f,-0.013549f,-0.013847f,-7.2594e-005f,-0.031638f,-0.0078732f,0.0031178f,0.033577f,0.0037455f,-0.0094416f,-0.019669f,-0.012652f,0.014781f,0.015726f,0.017716f,0.030595f,0.0073588f,-0.012022f,-0.02574f,0.0021469f,0.029711f,0.02077f,0.020749f,0.0070189f,0.029297f,0.035136f,0.036121f,0.022442f,-0.009795f,0.011243f,0.0090043f,0.036698f,0.037206f,0.031144f,0.03605f,0.010766f,0.0014669f,0.0046666f,-0.046344f,-0.023854f,-0.019658f,-0.036855f,-0.063983f,-0.048336f,-0.03465f,0.007731f,0.012967f,0.012109f,-0.03996f,-0.040814f,-0.018994f,0.0063998f,0.031572f,0.02336f,0.029836f,-0.00023275f,-0.016708f,-0.026619f,-0.041605f,-0.032598f,-0.031062f,-0.026946f,-0.023124f,-0.010088f,0.0057728f,0.031286f,0.013192f,0.014359f,-0.0078033f,0.0016929f,-0.0080451f,-0.011159f,-0.037182f,-0.04153f,-0.07601f,-0.05501f,-0.08341f,-0.046316f,-0.041767f,0.0010627f,-0.0079757f,-0.041103f,-0.059795f,-0.059237f,-0.055762f,-0.038903f,-0.025237f,-0.013512f,-0.029915f,-0.00052533f,0.0044298f,-0.022712f,-0.020161f,0.0038121f,-0.027782f,-0.01311f,-0.03896f,-0.026544f,-0.0015239f,0.027775f,0.05529f,0.039603f,0.011645f,0.021385f,0.0017516f,0.027643f,-0.013676f,-0.0065787f,-0.033773f,0.014304f,0.033339f,0.035334f,-0.00035375f,-0.014079f,-0.018306f,0.0018528f,-0.0076291f,0.034247f,0.019922f,0.022047f,-0.0090541f,-0.020388f,-0.008542f,0.016605f,0.017516f,0.0028627f,0.0014961f,0.018498f,0.015943f,0.041416f,0.015632f,0.0067995f,0.0012419f,0.028652f,0.012352f,-0.010779f,-0.06008f,-0.030289f,-0.0082725f,0.054044f,0.083918f,0.072259f,0.0035642f,-0.015339f,-0.053473f,-0.0086679f,-0.029299f,0.012239f,0.02491f,0.031138f,0.0013741f,-0.005567f,-0.044764f,-0.060961f,-0.071013f,-0.028416f,0.017534f,0.059485f,0.039394f,0.045209f,0.012113f,0.0085666f,-0.026648f,-0.02944f,0.011546f,0.020778f,0.012261f,-0.0012128f,0.010847f,0.010612f,0.033666f,0.05368f,0.028916f,0.069824f,0.06247f,0.033452f,0.0051511f,0.030575f,-0.014258f,0.0053565f,0.0023522f,-0.021415f,-0.058513f,-0.052193f,-0.040524f,-0.01651f,0.026715f,-0.021723f,-0.027882f,-0.064028f,-0.13942f,-0.17684f},
    {0.79441f,-0.15308f,-0.15421f,-0.063888f,-0.016922f,0.0041544f,0.013308f,0.061753f,0.063618f,0.05659f,-0.012307f,-0.061649f,-0.10884f,-0.085396f,-0.019198f,-0.01841f,-0.020766f,-0.049864f,-0.037069f,-0.001075f,-0.0077944f,0.016606f,0.019071f,0.030458f,-0.023041f,0.0074262f,-0.013306f,0.024298f,0.023051f,0.007551f,-0.029434f,-0.014947f,-0.055498f,-0.0067576f,0.00030882f,0.012847f,0.036265f,0.041861f,0.023958f,0.013003f,-0.0071375f,0.00031742f,-0.0077474f,-0.0084637f,-0.013461f,-0.0051033f,-0.0061928f,0.024021f,0.053208f,0.067218f,0.066116f,0.044948f,0.010785f,0.045911f,0.0022419f,0.019022f,-0.010434f,0.0070246f,0.0092632f,0.0043237f,0.023221f,-0.0029261f,-0.023528f,-0.022564f,-0.0074655f,-0.0025177f,-0.0071703f,-0.027912f,-0.019376f,0.016292f,0.025185f,0.029922f,0.039005f,0.025722f,0.0024732f,-0.0081487f,-0.0046469f,-0.031259f,-0.02568f,-0.029958f,-0.033358f,-0.0034853f,0.021459f,0.016905f,-0.023412f,-0.019822f,-0.024298f,0.027212f,0.034235f,0.043529f,-0.0021644f,0.027618f,0.04918f,0.03234f,0.018303f,0.016333f,-0.0015888f,0.019694f,-0.0054869f,0.017155f,-0.0067941f,-0.00044663f,-0.0085457f,-0.0069967f,-0.035723f,-0.019745f,-0.030462f,-0.018902f,-0.025731f,-0.020751f,-0.055679f,-0.033817f,-0.02009f,0.0020738f,-0.02168f,-0.032695f,-0.025626f,-0.0036561f,0.02016f,0.019269f,0.031966f,0.012158f,0.003108f,0.0051326f,-0.029677f,-0.039278f,-0.076052f,-0.064291f,-0.01089f,0.049254f,0.025504f,-0.020122f,-0.056545f,-0.072209f,-0.070587f,-0.04436f,-0.050494f,-0.0066615f,-0.010471f,0.028699f,0.026683f,0.036108f,0.0031686f,-0.01329f,-0.010783f,0.024302f,0.035693f,0.016949f,0.0029225f,0.00041417f,-0.011565f,-0.006243f,0.0031305f,0.008375f,-0.0083584f,-0.0024298f,-0.017674f,-0.020543f,-0.008638f,-0.011019f,-0.0099143f,-0.019218f,-0.032707f,-0.011717f,-0.019004f,0.034767f,0.056008f,0.029773f,0.022771f,0.044073f,0.073242f,0.068625f,0.039153f,0.015208f,-0.0059504f,0.03858f,0.053419f,0.058768f,0.024265f,0.038688f,0.00036975f,0.022883f,0.0089621f,0.017189f,0.0042417f,0.04391f,0.036698f,0.048986f,0.036207f,0.023372f,-0.060224f,-0.074458f,-0.076858f,-0.023378f,-0.0038185f,0.0061039f,-0.033131f,0.011242f,0.010819f,0.046465f,0.060898f,0.044957f,0.040788f,0.012695f,0.041239f,0.032445f,-0.0034911f,-0.00063293f,-0.017428f,0.010126f,0.017789f,0.035294f,0.019715f,0.017021f,-0.01542f,-0.013322f,-0.0049663f,0.012996f,0.013291f,0.01646f,-0.0054671f,0.0056658f,-0.022433f,-0.022895f,0.0061498f,-0.013458f,-0.00069762f,-0.002726f,-0.017433f,0.0065464f,0.037507f,0.024867f,0.020691f,-0.039827f,-0.066196f,-0.055015f,-0.04224f,-0.02337f,0.017365f,0.064923f,0.04428f,0.075707f,0.059193f,0.12158f,0.11249f,0.07559f,0.028615f,-0.0042448f,-0.016891f,-0.013256f,-0.0038841f,0.0004312f,-0.025324f,-0.03178f,-0.0693f,-0.16597f},
    {0.8107f,-0.16892f,-0.11951f,-0.039167f,-0.0028317f,0.0027528f,0.023432f,0.015344f,-0.037275f,-0.059887f,-0.071623f,-0.035776f,0.0085817f,0.019479f,0.040533f,0.034815f,0.008207f,0.011108f,0.013958f,0.022704f,-0.0074943f,0.0037001f,-0.030319f,-0.049061f,-0.023368f,0.0022389f,0.03167f,-0.024018f,-0.0029013f,-0.017632f,0.0037213f,0.011789f,0.045188f,0.029164f,0.05226f,0.046361f,0.055482f,0.070681f,0.046978f,0.050914f,0.04287f,0.039024f,0.054982f,0.048374f,0.022996f,0.041303f,-0.02091f,-0.041973f,-0.07262f,-0.064913f,-0.026501f,0.027654f,0.033453f,0.057063f,0.035456f,0.022684f,-0.031014f,-0.041403f,-0.068352f,-0.045949f,-0.0095272f,-0.0037997f,-0.012013f,-0.013574f,-0.013416f,-0.02329f,0.0019228f,0.0066405f,-0.0029137f,0.0087924f,0.018706f,0.01465f,0.014125f,-0.0075558f,-0.027711f,-0.0026253f,-0.012213f,0.0076082f,-0.0051358f,-0.0055578f,-0.011993f,-0.011029f,0.0093214f,0.0089976f,0.0048579f,0.011493f,-0.022775f,-0.022359f,-0.029614f,0.010124f,0.061687f,0.064629f,0.023348f,0.0038804f,0.003507f,0.02489f,0.047932f,0.037947f,0.0024546f,0.038237f,0.033882f,0.057511f,0.026968f,0.027015f,0.030756f,-0.00016564f,0.026742f,0.037808f,0.0074322f,0.017742f,0.011243f,0.0034445f,-0.014317f,-0.0058152f,-0.030448f,-0.043617f,-0.073967f,-0.028703f,-0.015317f,0.010484f,-0.0034859f,-0.007884f,-0.019319f,-0.022849f,-0.014405f,-0.019393f,-0.037886f,-0.003915f,-0.00025683f,0.023542f,0.047661f,0.051379f,0.026608f,0.038747f,0.0059445f,-0.0043155f,-0.037227f,-0.049004f,-0.033313f,-0.00043019f,0.024976f,0.0074157f,-0.028782f,-0.038206f,-0.038311f,-0.026959f,-0.037677f,-0.022392f,-0.020063f,-0.022435f,-0.0073521f,0.055106f,0.018812f,0.02675f,-0.0071256f,-0.051998f,-0.059677f,-0.052282f,-0.029494f,-0.046792f,-0.069949f,-0.037449f,-0.024345f,-0.0042031f,-0.02109f,-0.037618f,-0.051289f,-0.0034997f,-0.00066414f,-0.0034255f,-0.011951f,-0.019216f,-0.00302f,0.02574f,0.017115f,0.04349f,0.0022456f,-0.0089696f,-0.041362f,-0.025197f,-0.0053411f,0.022522f,0.049042f,0.053454f,0.024428f,0.045603f,0.026119f,0.044368f,0.028002f,0.012633f,-0.023371f,-0.015264f,-0.063075f,-0.049699f,-0.055741f,-0.03136f,-0.01821f,0.038358f,0.042947f,0.025228f,0.002751f,-0.022819f,-0.029789f,-0.016746f,-8.2008e-006f,0.010883f,0.03055f,0.067778f,0.053269f,0.047124f,0.030089f,0.005758f,0.032667f,0.033188f,0.0032312f,0.013827f,-0.026686f,-0.026342f,-0.028326f,-0.024667f,-0.016493f,0.013569f,-0.0029422f,0.048029f,0.017954f,0.05842f,0.039317f,0.073313f,0.047544f,0.083842f,0.0089866f,0.035614f,0.015959f,0.039275f,-0.0019404f,-0.014671f,-0.008052f,-0.017641f,-0.017948f,0.0077883f,-0.00066186f,-0.0076401f,-0.032222f,-0.0025018f,-0.017151f,0.031432f,0.046982f,0.05324f,0.010014f,-0.023882f,-0.051947f,-0.019305f,-0.058072f,-0.068492f,-0.10358f,-0.12732f},
    {0.8039f,-0.18185f,-0.14616f,-0.083479f,-0.048892f,-0.023439f,-0.019831f,-0.016117f,-0.0063966f,-0.0047059f,-0.023075f,-0.0061089f,-0.0050965f,-0.0055328f,-0.022519f,-0.018631f,-0.0040282f,0.049149f,0.04437f,0.023562f,0.00179f,-0.027839f,-0.03382f,0.0086571f,0.021841f,0.050806f,0.033551f,0.059287f,0.043962f,0.039196f,0.0030353f,0.027294f,-0.0069039f,0.012463f,-0.012986f,-0.042034f,-0.027023f,-0.010349f,0.016262f,0.026016f,0.038661f,0.029726f,0.0095417f,0.038658f,0.040095f,0.030595f,0.01136f,0.039233f,0.041778f,0.053211f,0.047687f,0.037519f,0.012845f,-0.021233f,-0.037507f,-0.039571f,-0.050249f,-0.08566f,-0.060744f,0.0062998f,0.020144f,0.051353f,0.03103f,-0.0069339f,-0.017389f,-0.03283f,-0.020444f,-0.011408f,-0.022808f,-0.045881f,-0.062296f,-0.085074f,-0.035821f,0.031376f,0.072223f,0.090376f,0.063783f,0.034259f,0.0057925f,-0.029843f,-0.039373f,-0.0073596f,-0.0016183f,0.025313f,0.063167f,0.059341f,0.015341f,0.010581f,0.010508f,0.0052308f,0.041855f,0.047361f,0.045208f,0.051187f,0.030755f,0.035542f,0.046749f,0.047469f,0.025367f,0.036274f,0.014996f,0.036505f,-0.016192f,-0.009561f,-0.026284f,0.0046086f,0.0035309f,-0.020023f,-0.063164f,-0.029074f,-0.003236f,0.026063f,0.03671f,0.050118f,0.045953f,0.013159f,0.021316f,0.024753f,0.021166f,0.053618f,0.062872f,0.065007f,0.00050976f,-0.045418f,-0.054024f,-0.064933f,-0.02422f,-0.0052737f,-0.026673f,-0.040426f,-0.032967f,0.030468f,0.028502f,0.056135f,0.021429f,-0.017291f,-0.020576f,0.013396f,0.025725f,0.018139f,-0.025934f,-0.027945f,-0.080603f,-0.049451f,-0.029165f,-0.00029498f,-0.0081432f,-0.0025306f,-0.010611f,-0.043828f,-0.067887f,-0.034816f,-0.0061297f,-0.0013941f,-0.001148f,-0.030586f,-0.011966f,0.0021536f,0.0032161f,-0.0078168f,-0.027237f,-0.012304f,-0.032641f,-0.037213f,-0.032254f,-0.01359f,-0.033219f,-0.0025703f,-0.034619f,-0.015062f,-0.026498f,-0.038356f,-0.047007f,0.0091723f,0.021376f,0.042098f,0.045849f,0.045265f,-0.0035546f,-0.0017246f,-0.026696f,-0.027073f,-0.036875f,-0.011124f,0.012405f,0.064512f,0.025661f,0.036802f,0.0041307f,0.0089161f,-0.00047153f,0.03509f,0.019564f,0.035065f,-0.01715f,-0.035017f,-0.037735f,-0.022383f,0.017724f,0.047836f,-0.0083735f,-0.00848f,-0.020744f,-0.0069138f,-0.028255f,-0.014378f,-0.031897f,-0.023868f,-0.043512f,-0.04019f,-0.023129f,-0.014677f,-0.028766f,-0.0022857f,0.018645f,0.020777f,0.004302f,0.029449f,0.010763f,0.036816f,0.027704f,0.06256f,0.028008f,0.035126f,0.053733f,0.052975f,0.069552f,0.044686f,0.0090446f,-0.0026622f,-0.0044711f,-0.014961f,-0.010237f,-0.0011755f,0.010185f,0.013415f,-0.025461f,-0.011801f,-0.036911f,-0.025989f,-0.0010252f,0.014959f,-0.0058469f,-0.004348f,-0.014139f,0.0069101f,0.010535f,-0.001142f,-0.030448f,-0.012342f,-0.018546f,0.012433f,0.0057256f,-0.0072599f,-0.068289f,-0.12737f},
    {0.78636f,-0.17291f,-0.10936f,-0.049513f,0.00070143f,0.010418f,-0.011567f,0.018591f,0.012528f,0.026472f,0.021218f,0.052295f,0.042304f,0.022881f,-0.014028f,-0.0024368f,-0.020798f,0.002675f,-0.0098526f,-0.009325f,-0.00078268f,0.020242f,0.014745f,0.0067965f,-0.0012233f,0.01836f,-0.00078065f,0.030276f,0.04923f,0.030849f,0.047475f,0.052045f,0.043249f,0.047954f,0.074284f,0.078099f,0.030816f,0.00043742f,-0.037629f,-0.010658f,0.0020771f,-0.0027849f,-0.0084833f,0.0083957f,0.030226f,0.01656f,0.025014f,-0.038083f,-0.061703f,-0.034698f,-0.036045f,-0.016274f,-0.0046088f,0.0096669f,0.037566f,0.042535f,0.037517f,-0.0040506f,-0.040832f,-0.059288f,-0.043495f,-0.040783f,-0.022398f,-0.0095172f,-0.0071048f,0.024678f,0.022743f,0.018936f,0.016125f,0.0068314f,0.010143f,0.01425f,-0.025238f,0.0016457f,-0.023807f,-0.01036f,-0.0071381f,0.010136f,0.031768f,0.042701f,-0.0037778f,-0.032181f,-0.039746f,0.0040618f,-0.0070445f,0.0018923f,-0.034025f,-0.0379f,-0.057982f,-0.0069826f,0.0017152f,0.032402f,0.037173f,0.043471f,0.038382f,0.04891f,0.025079f,0.0012068f,-0.015259f,-0.042009f,-0.037164f,-0.015969f,-0.024029f,-0.030145f,-0.064364f,-0.05896f,-0.073241f,-0.021836f,-0.0020131f,0.046121f,0.011319f,-0.0065975f,-0.05028f,-0.058329f,-0.067827f,-0.043484f,-0.04086f,-0.010338f,0.020937f,-0.0035394f,-0.0034496f,0.004486f,-0.018645f,0.014055f,0.0065595f,0.039385f,0.016112f,0.0099868f,-0.0094413f,-0.0087865f,-0.028917f,-0.032777f,-0.014739f,0.0031863f,0.02969f,0.033764f,0.035744f,0.043406f,-0.019033f,0.009122f,0.005287f,0.026862f,0.044255f,0.061371f,0.041617f,0.0081944f,-0.037637f,-0.076169f,-0.058902f,-0.039724f,-0.027287f,-0.045135f,-0.062841f,-0.072902f,-0.0712f,-0.036187f,-0.0081824f,0.030613f,0.0068034f,0.018734f,-0.0098449f,-0.024537f,-0.052718f,-0.067967f,-0.087556f,-0.070787f,-0.047715f,-0.0077069f,0.045006f,0.057509f,0.049191f,0.03424f,0.026482f,0.026646f,0.050513f,0.048806f,0.0023645f,-0.031809f,-0.025999f,0.018434f,0.017846f,0.015481f,-0.016666f,-0.049982f,-0.053395f,-0.06711f,-0.058852f,-0.018575f,-0.021217f,0.0056898f,-0.0095789f,0.0076929f,0.016787f,0.022549f,0.014676f,0.041038f,0.036629f,0.073613f,0.050058f,0.037698f,-0.018628f,-0.015691f,-0.03325f,0.0011942f,-0.019402f,-0.015959f,-0.036437f,-0.031414f,-0.0086535f,0.0061152f,-0.041995f,-0.0025595f,-0.017593f,0.017629f,0.0082259f,-0.0025869f,-0.037402f,-0.010509f,0.021219f,0.024424f,-0.0076482f,-0.042856f,-0.066689f,-0.032166f,-0.042148f,-0.025105f,-0.015441f,0.031668f,0.0005445f,0.015738f,-0.0020414f,-0.047678f,-0.070928f,-0.063268f,-0.066088f,-0.030156f,-0.013451f,0.0030072f,-0.023947f,-0.024577f,-0.015855f,-0.00019234f,0.0080162f,-0.001818f,-0.034539f,-0.027272f,-0.0095391f,0.0096734f,0.015478f,0.023947f,0.029417f,0.0047757f,-0.043041f,-0.061888f,-0.13918f,-0.17171f},
    {0.79462f,-0.14892f,-0.077624f,-0.019497f,-0.014044f,-0.0023491f,0.011675f,0.014719f,0.02077f,0.042586f,0.036295f,0.067268f,0.050956f,-0.0017375f,-0.043939f,-0.09735f,-0.072398f,-0.024336f,-0.0010364f,0.043153f,0.052168f,0.038242f,0.0098841f,-0.0076626f,-0.018703f,0.050182f,0.031879f,0.061559f,0.025196f,0.039863f,0.01204f,0.040088f,0.028499f,0.035771f,-0.026812f,-0.014088f,-0.041235f,-0.036646f,-0.057004f,-0.0069503f,-0.039946f,-0.027744f,-0.027635f,-0.0074348f,0.010777f,0.042865f,-0.0095553f,0.0054226f,-0.00058365f,0.019241f,0.045053f,0.064873f,0.052115f,0.037674f,0.0074638f,0.020365f,0.021976f,0.04196f,0.026377f,0.023412f,0.03346f,0.0032562f,-0.0052912f,0.018748f,0.025655f,0.04441f,0.011135f,-0.014589f,-0.063114f,-0.060729f,-0.0043456f,0.03487f,0.054427f,0.049921f,0.037136f,0.048831f,0.028409f,0.052062f,0.02281f,0.03204f,0.01831f,-0.026056f,-0.027043f,-0.022834f,-0.021925f,0.033032f,0.045644f,0.041898f,0.022661f,0.06159f,0.04821f,0.011884f,0.025408f,0.0044329f,0.0082562f,0.027344f,-0.0076235f,-0.038607f,-0.069805f,-0.036087f,-0.035564f,0.021479f,0.029467f,0.04745f,0.011403f,-0.013946f,0.0053293f,0.03292f,0.032404f,0.038523f,0.021877f,0.030001f,-0.0097171f,-0.019937f,-0.069042f,-0.071086f,-0.01915f,-0.017177f,-0.011915f,-0.0098968f,-0.026935f,0.00212f,-0.0073803f,0.001453f,-0.016012f,0.0055179f,-0.029039f,-0.022343f,-0.041596f,-0.056727f,-0.081729f,-0.10396f,-0.087615f,-0.05764f,-0.014149f,0.023821f,0.042374f,0.022638f,-0.025747f,-0.028845f,-0.050689f,0.030406f,0.026531f,0.011234f,0.0047058f,-0.0037753f,0.0014928f,0.0048645f,-0.008327f,0.0039462f,0.013353f,0.042383f,0.0033345f,-0.026384f,-0.040612f,-0.035395f,-0.0084533f,0.015699f,0.051522f,0.060718f,0.036074f,0.03057f,0.034114f,0.050878f,0.0075495f,0.021582f,-0.036164f,-0.034824f,-0.013597f,-0.019803f,-0.047543f,-0.021108f,0.011855f,0.018385f,0.0096392f,0.031346f,-0.0047533f,0.019027f,0.02801f,0.013841f,0.011572f,0.0046017f,0.0016378f,0.011f,-0.0059927f,0.010348f,0.014436f,0.058446f,0.034268f,0.0013907f,-0.030539f,-0.040799f,-0.032746f,-0.016954f,-0.014226f,-0.011025f,-0.047318f,-0.010663f,-0.014442f,-0.01647f,-0.0152f,-0.0014165f,-0.0039783f,-0.018348f,-0.050197f,-0.051598f,-0.048232f,-0.0041577f,-0.0052175f,-0.007147f,-0.016519f,-0.045191f,-0.018143f,0.027289f,0.033333f,0.040814f,0.02654f,0.014237f,0.055425f,0.063967f,0.051658f,0.034225f,0.03088f,-0.020882f,-0.018944f,-0.0017428f,0.028714f,0.028353f,0.027537f,-0.014184f,-0.016316f,-0.027777f,0.028462f,0.034377f,-0.0054971f,-0.030898f,-0.071699f,-0.052868f,-0.022288f,0.0036524f,0.017543f,0.034156f,-0.034628f,-0.030445f,-0.052394f,-0.004356f,0.017095f,0.0098454f,-0.026711f,-0.040748f,-0.06626f,-0.022499f,-0.026064f,-0.061805f,-0.13361f,-0.16578f},
    {0.79869f,-0.14126f,-0.096441f,-0.031325f,-0.012232f,0.021119f,0.00091598f,-0.006531f,-0.011974f,0.025814f,0.067926f,0.072561f,0.010191f,-0.0045757f,-0.033099f,0.0052871f,0.029943f,0.038436f,0.035358f,0.035993f,-0.015798f,0.0014534f,0.018142f,0.0041827f,0.0093998f,0.0091979f,-0.0093714f,-0.0060574f,0.007318f,0.00066311f,-0.042859f,-0.027715f,-0.010878f,0.014943f,0.0057425f,-0.02993f,-0.034283f,-0.018814f,-0.00047857f,-0.026798f,0.0015193f,0.021155f,0.0072993f,-0.0097406f,-0.01849f,0.044474f,0.015406f,0.020831f,0.039892f,0.020957f,0.0064715f,0.015734f,0.024089f,0.015017f,-0.017531f,-0.02861f,-0.027664f,0.016957f,0.039087f,0.0559f,0.028861f,0.026718f,-0.038801f,-0.01495f,-0.036533f,-0.0027895f,0.034371f,0.035956f,-0.016342f,-0.010073f,-0.036463f,-0.031228f,-0.019737f,-0.047397f,-0.061797f,-0.045207f,-0.066318f,-0.021087f,-0.025665f,-0.0047629f,-0.010212f,0.0063262f,0.01472f,0.035445f,0.054334f,0.050643f,0.010633f,-0.017798f,-0.049422f,-0.063245f,-0.046931f,-0.021306f,-0.0020468f,0.011172f,0.008994f,0.017455f,0.0075307f,0.0061394f,-0.013859f,-0.044233f,-0.054333f,-0.045557f,-0.0035681f,0.024207f,0.046476f,0.045534f,0.0053603f,-0.017385f,-0.040147f,0.022208f,0.012648f,0.02766f,0.015185f,-0.03185f,-0.033997f,-0.019535f,-0.027265f,-0.033473f,-0.022277f,-0.015661f,-0.007669f,-0.004929f,0.010324f,-0.019827f,0.0011661f,-0.010729f,0.038071f,0.034087f,-0.0043384f,-0.026446f,-0.047375f,0.015478f,-0.011115f,0.031319f,-0.0039103f,0.038285f,0.043651f,0.060416f,0.027379f,0.011681f,-0.042325f,-0.038933f,-0.02389f,0.01117f,-0.047381f,-0.061157f,-0.079026f,-0.074102f,-0.014553f,0.00090314f,-0.010193f,-0.020923f,-0.015212f,0.041621f,0.060325f,0.11855f,0.10154f,0.099755f,0.046746f,0.044723f,0.019648f,0.0087699f,-0.015737f,-0.010039f,-0.0067337f,0.036545f,0.045375f,0.07718f,0.04861f,0.046254f,0.02385f,0.017224f,-0.011187f,0.0035552f,-0.015082f,-0.01037f,-0.0069343f,0.0079201f,0.0099409f,0.012331f,0.02428f,0.011042f,0.0071472f,0.016545f,-0.013678f,0.025624f,0.023733f,0.014504f,0.001071f,-0.035684f,-0.053452f,-0.00096166f,0.020856f,0.024031f,0.034488f,-0.014506f,-0.018865f,-0.024981f,-0.026587f,-0.0093977f,-0.0039028f,-0.0027916f,-0.034841f,-0.048114f,-0.065815f,-0.034093f,-0.033123f,-0.017522f,-0.039497f,0.0042103f,-0.0081268f,-0.031449f,0.011296f,0.010777f,-0.021049f,-0.015822f,-0.0036706f,0.046468f,0.022057f,0.063586f,0.071192f,0.070459f,-0.0018734f,-0.016757f,-0.019117f,0.019027f,0.031588f,0.013929f,-0.0051029f,0.014441f,0.0090697f,0.026886f,0.031537f,0.080571f,0.068754f,0.098322f,0.083967f,0.010499f,-0.036868f,-0.045636f,-0.063565f,-0.027923f,-0.0084379f,-0.01954f,-0.032043f,-0.040653f,-0.012569f,0.056647f,0.068298f,0.050845f,0.0090018f,-0.0040849f,-0.005057f,-0.033957f,-0.10012f,-0.16146f},
    {0.77159f,-0.19086f,-0.14774f,-0.07501f,-0.036637f,-0.0055187f,-0.0060064f,0.011387f,0.0038178f,0.023306f,0.0033421f,0.021627f,-0.011137f,-0.0073945f,-0.030498f,-0.030206f,-0.064124f,-0.05401f,-0.072365f,-0.041689f,-0.042506f,0.039638f,0.079895f,0.082714f,0.049061f,0.035712f,0.023058f,-0.016604f,-0.050745f,-0.036205f,-0.029636f,0.013377f,-0.00013889f,0.018008f,-0.0021567f,-0.0064902f,-0.034723f,-0.016464f,-0.008268f,-0.0063906f,-0.011348f,-0.0025491f,-0.010005f,0.031875f,0.04645f,0.06031f,0.039701f,0.012434f,-0.015703f,-0.042548f,-0.041676f,-0.043249f,-0.073836f,-0.022677f,-0.049166f,-0.055702f,-0.063552f,-0.048355f,-0.017105f,0.04583f,0.061183f,0.059436f,0.030928f,0.02335f,0.011562f,0.042448f,0.011293f,0.032825f,0.015869f,0.040057f,0.0064458f,0.040742f,0.0048437f,0.009673f,-0.0060104f,0.011563f,-0.027781f,-0.019784f,0.0023896f,0.030332f,0.03375f,0.01899f,-0.0029214f,-0.0027416f,-0.032176f,-0.030774f,-0.024346f,-0.026386f,0.03068f,0.045062f,0.02645f,0.036295f,0.01711f,0.058193f,0.045635f,0.074586f,0.041149f,0.030124f,0.02403f,0.011101f,0.042544f,0.041585f,0.03055f,0.033319f,0.02362f,0.0028089f,-0.0041043f,0.014583f,0.00086263f,-0.0023574f,0.014242f,-0.0057886f,-0.046647f,-0.02303f,-0.00030072f,0.0078398f,0.010714f,-0.011946f,-0.029565f,-0.0011532f,-0.017255f,-0.0017363f,-0.033678f,-0.014828f,-0.01201f,0.028608f,0.014685f,0.006768f,-0.057837f,-0.095724f,-0.089689f,-0.057497f,-0.012594f,0.0033821f,-0.0092537f,-0.047084f,-0.052652f,-0.049488f,-0.025253f,-0.01248f,-0.041292f,-0.0099422f,-0.016648f,0.010493f,0.0083675f,-0.026937f,-0.062403f,-0.01961f,-0.019384f,0.0039575f,0.023013f,0.045834f,0.016381f,0.026556f,0.0044033f,-0.017896f,-0.06123f,-0.025212f,-0.04406f,-0.013232f,-0.016719f,-0.025273f,-0.00067752f,0.012139f,0.045668f,0.037674f,0.021185f,0.0013857f,-0.005769f,0.070501f,0.050321f,0.062943f,0.04102f,0.011672f,-0.022008f,-0.052878f,-0.1035f,-0.093682f,-0.065408f,-4.7431e-005f,0.013798f,-0.00081783f,-0.010939f,-0.021006f,-0.029773f,-0.042748f,-0.0084792f,-0.027169f,-0.021461f,-0.041824f,-0.046623f,-0.062935f,-0.033789f,-0.010798f,-0.0098681f,-0.0035191f,-0.019999f,0.001599f,0.022394f,0.042753f,0.018789f,0.024046f,0.0038473f,0.0072448f,0.046101f,0.025583f,-0.004043f,0.024271f,-0.0013866f,0.0063669f,-0.01894f,-0.018291f,-0.01396f,0.013087f,0.03245f,0.022936f,-0.0061377f,-0.023732f,-0.054829f,-0.036054f,-0.028592f,-0.029967f,-0.02879f,-0.022454f,-0.019655f,-0.025258f,-0.010432f,0.0045491f,0.0073586f,0.0098584f,-0.020399f,-0.00087217f,-0.023242f,-0.042737f,-0.068398f,-0.020694f,0.041357f,0.049888f,0.053881f,0.036819f,0.017566f,-0.0096366f,-0.019651f,-0.025101f,-0.037899f,-0.048441f,-0.036308f,-0.021597f,0.0031854f,0.018408f,0.018032f,0.015191f,-0.019094f,-0.055151f,-0.12453f,-0.1783f},
    {0.79736f,-0.13459f,-0.093559f,-0.033496f,0.011121f,0.0087006f,-0.0069605f,-0.012031f,-0.014793f,-0.024063f,-0.042635f,-0.014403f,-0.026967f,-0.037341f,-0.039039f,-0.032603f,-0.016235f,0.0054935f,-0.028321f,-0.00098865f,-0.025175f,-0.011549f,-0.013484f,0.030147f,0.0092108f,0.016662f,0.010121f,0.025876f,0.004614f,-0.0083394f,-0.012126f,-0.0090779f,-0.0099087f,0.025236f,0.015726f,0.056195f,0.04491f,0.029566f,-0.00077386f,-0.029135f,-0.037323f,-0.038289f,-0.029031f,-0.019741f,-0.062554f,-0.069035f,-0.053555f,-0.022171f,0.0045924f,0.0032196f,-0.034465f,-0.029856f,0.0039969f,0.064617f,0.060794f,0.080525f,0.066438f,0.028328f,0.0085865f,0.03114f,-0.0076333f,-0.0069638f,-0.0065382f,0.026911f,0.049014f,0.061827f,0.014558f,-0.038239f,-0.013585f,-0.00093737f,0.027134f,0.0061966f,0.0021593f,-0.026627f,-0.0019968f,0.0040489f,0.018285f,0.011835f,-0.022378f,-0.021958f,-0.02083f,-0.042188f,-0.037965f,-0.03158f,-0.026549f,-0.020027f,-0.025293f,0.0043578f,-0.018609f,-5.9797e-005f,-0.010434f,-0.064667f,-0.064441f,-0.059872f,-0.041923f,-0.015975f,0.031021f,0.056191f,0.071732f,0.022324f,-0.015409f,-0.038763f,-0.023507f,0.037726f,0.069202f,0.068468f,0.0022776f,0.022509f,0.00099664f,0.017086f,0.0049926f,-0.00095599f,-0.013034f,-0.018668f,0.0086402f,0.025881f,0.031144f,0.015574f,0.0062816f,-0.010884f,-0.018834f,0.028268f,0.02927f,0.072324f,0.046772f,0.036183f,0.012398f,-0.021846f,-0.023045f,0.0013796f,-0.028051f,-0.012976f,-0.023349f,-0.028078f,-0.0098418f,0.0043934f,0.036489f,0.052658f,0.035412f,0.048194f,0.019604f,0.026219f,0.016136f,0.044717f,0.014297f,0.013162f,-0.014294f,-0.012243f,-0.0085047f,-0.01513f,-0.0081f,-0.05611f,-0.076952f,-0.042521f,-0.0073154f,0.023756f,-0.00045208f,0.0062402f,-0.034134f,-0.0033665f,-0.017975f,0.0098388f,-0.0018927f,0.026736f,0.027532f,0.049715f,-0.017436f,-0.015935f,-0.022499f,-0.040508f,-0.038733f,-0.031318f,-0.028262f,0.0061859f,0.0070958f,0.021152f,-0.01992f,-0.0034883f,-0.010787f,-0.016755f,-0.029109f,-0.0035287f,-0.014462f,0.024518f,-0.0085424f,0.0022848f,-0.037139f,-0.00055696f,-0.0092417f,0.034608f,-0.023732f,-0.052309f,-0.06836f,-0.056814f,-0.061103f,-0.050061f,-0.042571f,-0.029411f,0.0085888f,0.027522f,-0.012825f,-0.015036f,-0.030776f,-0.059981f,-0.06746f,-0.056118f,-0.079996f,-0.10109f,-0.094682f,-0.032964f,0.0035859f,0.047384f,0.04025f,0.019584f,0.0020636f,0.01683f,-0.0025586f,0.004823f,-0.022855f,-0.027703f,-0.059958f,-0.060406f,-0.042151f,-0.022633f,0.022496f,0.067274f,0.070442f,0.075259f,0.029593f,0.011369f,0.0020164f,0.013933f,0.0055795f,0.0032225f,0.017029f,0.023714f,0.015977f,0.0059617f,0.011423f,-0.013335f,-0.022602f,0.00072858f,-0.018826f,-0.015991f,-0.044085f,-0.038071f,-0.0062177f,0.016397f,0.0028945f,0.011021f,-0.015808f,-0.021427f,-0.067872f,-0.072753f,-0.13779f,-0.18494f},
    {0.80541f,-0.13332f,-0.09471f,-0.030876f,-0.023515f,-0.017011f,-0.019945f,-0.041096f,-0.059696f,-0.058797f,-0.052092f,-0.048531f,-0.015974f,0.015504f,-0.0057984f,-0.010563f,-0.07842f,-0.055085f,-0.051379f,-0.0026996f,0.057282f,0.047217f,0.028039f,0.025374f,0.012453f,0.020096f,0.0014326f,-0.00971f,-0.042587f,0.0065092f,0.0067479f,0.024666f,-0.00098178f,0.0036906f,-0.02938f,-0.04003f,-0.066115f,-0.054402f,-0.039229f,-0.021856f,-0.046005f,-0.0085315f,-0.038325f,-0.030578f,-0.052391f,0.00086145f,0.008961f,0.030406f,0.0052612f,-0.010822f,-0.029749f,-0.0059557f,-0.0072664f,-0.0045588f,-0.020603f,-0.031461f,-0.05802f,-0.011589f,0.02695f,0.043344f,0.0038969f,0.017168f,-0.041938f,-0.041595f,-0.043257f,-0.058114f,-0.048065f,-0.043151f,-0.052212f,-0.037059f,-0.015089f,0.02045f,0.041997f,0.076566f,0.083641f,0.071254f,0.06544f,0.055439f,0.025852f,-0.028284f,-0.043077f,-0.044384f,-0.01932f,-0.011044f,-0.035502f,-0.029984f,-0.045632f,0.017318f,0.042712f,0.040512f,0.017634f,0.023894f,0.0088006f,0.031508f,-0.010637f,-0.024409f,-0.022197f,0.0074953f,-0.0032767f,0.0085336f,-5.8512e-005f,-0.039447f,-0.036f,-0.02711f,0.0087736f,0.033284f,0.013239f,-0.012676f,-0.0064226f,0.019125f,0.036685f,0.031445f,-0.017761f,-0.01047f,-0.033258f,-0.00056134f,-0.030793f,-0.0061927f,-0.02852f,-0.036839f,-0.066425f,-0.045759f,-0.032521f,-0.06524f,-0.032647f,0.019175f,0.019876f,0.0069771f,-0.019792f,-0.034915f,-0.063191f,-0.057596f,-0.047325f,-0.015303f,-0.062435f,-0.019689f,-0.0047851f,-0.0080235f,-0.029967f,-0.027942f,-0.019357f,-0.025771f,-0.00016202f,-0.0024999f,-0.0024603f,-0.023656f,-0.029567f,0.017339f,0.039304f,0.061963f,0.019832f,0.010577f,0.0018377f,0.038817f,0.027402f,0.051881f,0.013934f,0.021269f,0.0032668f,-0.01306f,-0.020045f,-0.005527f,-0.016182f,-0.012727f,0.013885f,0.027193f,0.026836f,0.022963f,0.001151f,0.026462f,0.054214f,0.026282f,0.0012796f,-0.014802f,-0.016048f,0.042513f,0.045417f,0.059593f,0.0019773f,-0.013668f,-0.037558f,-0.02854f,-0.047759f,-0.022395f,-0.0088914f,0.0079103f,-0.0012412f,0.015514f,-0.00333f,-0.0038134f,-0.0099356f,0.025222f,0.0052941f,0.035268f,0.041012f,0.012757f,0.013954f,-0.021555f,-0.040505f,0.00079971f,0.02807f,0.018806f,-0.00091738f,0.003891f,-0.0076297f,0.01531f,0.042178f,0.047447f,0.019813f,0.0033352f,-0.0023999f,0.010623f,0.051583f,0.063211f,0.0632f,0.037421f,0.028216f,-0.016177f,-0.028258f,0.012661f,0.033771f,0.044411f,0.026914f,0.001295f,-0.020833f,-0.048405f,-0.062155f,-0.064053f,-0.03315f,-0.029906f,-0.030936f,-0.0073247f,-0.00024935f,0.0017308f,-0.026288f,-0.034997f,-0.0728f,-0.016003f,-0.0058041f,-0.0044791f,0.0017449f,-0.034356f,-0.039901f,-0.032582f,-0.011714f,0.044955f,0.043973f,0.063173f,0.03914f,0.015705f,-0.02887f,-0.016466f,-0.039729f,-0.069956f,-0.12098f,-0.16979f},
    {0.77797f,-0.18756f,-0.13022f,-0.047969f,-0.0022708f,0.0038455f,-0.010565f,-0.016388f,-0.061266f,-0.060759f,-0.043033f,-0.032733f,-0.063033f,-0.012204f,-0.047787f,-0.032399f,-0.026279f,-0.003167f,0.01427f,-0.0062206f,-0.0086566f,0.0035335f,0.0095952f,0.011391f,0.011256f,0.013218f,-0.028464f,-0.029618f,-0.039233f,-0.041782f,-0.023042f,-0.035022f,-0.018884f,-0.0056086f,0.029307f,0.059647f,0.013079f,-0.0031074f,-0.02721f,-0.0029364f,-0.029452f,-0.010358f,0.00091529f,-0.021099f,-0.026563f,0.0077305f,0.0040238f,0.030454f,-0.005866f,-0.0044342f,-0.030069f,0.0092582f,-0.024125f,-0.019243f,-0.0092609f,0.0069746f,-0.019636f,-0.014734f,-0.035874f,-0.022263f,-0.039544f,-0.01998f,-0.013388f,-0.0091374f,0.0036252f,0.01163f,0.019807f,0.00067907f,-0.0048958f,0.022277f,0.0090321f,-0.010567f,-0.0058321f,-0.0066509f,0.00062159f,0.010134f,-0.011258f,0.0087768f,-0.030693f,-0.018181f,0.0035924f,0.03166f,0.026311f,0.02076f,-0.019612f,-0.0018449f,-0.027074f,-0.024036f,-0.058486f,-0.020386f,0.0093298f,0.038639f,0.016827f,-0.0069469f,-0.0044899f,-0.0099758f,-0.004943f,0.0063781f,-0.059099f,-0.07841f,-0.084f,-0.046874f,0.020491f,0.077121f,0.011747f,-0.017653f,-0.061f,-0.028024f,-0.019789f,0.013753f,-0.013563f,-0.010617f,-0.045575f,-0.0083681f,-0.033322f,0.043514f,0.069902f,0.12049f,0.12542f,0.092499f,0.046202f,0.006723f,0.0095441f,0.019927f,0.02529f,0.02879f,0.055005f,0.049355f,0.026394f,0.014941f,-0.0087709f,0.028636f,0.010003f,0.02428f,0.023748f,0.062579f,0.031999f,0.020573f,-0.020919f,-0.0010177f,0.012548f,0.024735f,0.043023f,-0.0044842f,-0.0019749f,-0.019091f,-0.009305f,0.0029347f,0.034625f,0.047838f,0.036065f,-0.015558f,-0.070914f,-0.12563f,-0.075611f,0.018073f,0.047754f,0.066855f,0.039855f,-0.0092986f,-0.0080874f,0.0079944f,-0.0099144f,0.0049014f,-0.02363f,-0.00751f,0.030638f,0.080896f,0.085149f,0.029148f,-0.021572f,-0.049692f,-0.039751f,-0.05095f,-0.059448f,-0.051571f,-0.040545f,-0.00018504f,-0.01608f,-0.037771f,-0.038433f,-0.036495f,-0.023088f,0.023562f,0.026892f,0.053764f,0.018069f,0.018046f,0.022485f,0.02487f,-0.01407f,-0.0003719f,-0.013742f,0.0094123f,-0.011404f,-0.00020252f,0.00021623f,-0.0029743f,0.0065319f,0.018204f,0.021548f,0.020912f,0.023193f,0.044083f,0.020773f,-0.0059842f,0.01243f,-0.0080461f,-0.035958f,-0.06602f,-0.060051f,-0.050461f,-0.029914f,-0.017111f,-0.058613f,-0.040625f,-0.031446f,-0.027527f,0.0077518f,0.027147f,0.013157f,-0.015956f,-0.0087634f,0.030483f,0.021204f,0.055192f,0.028236f,0.061897f,0.039922f,0.049817f,0.036316f,0.017492f,-0.0033888f,-0.0016218f,-0.036108f,-0.024656f,-0.027559f,-0.014534f,-0.049009f,-0.023198f,-0.0070115f,0.022812f,0.036812f,0.019107f,0.054472f,0.039157f,0.031464f,0.054928f,0.021264f,-0.023308f,-0.04227f,-0.027704f,-0.045478f,-0.045784f,-0.10433f,-0.15865f},
    {0.78092f,-0.18297f,-0.13585f,-0.061804f,-0.020485f,-0.0042222f,-0.030902f,-0.065604f,-0.071082f,-0.049235f,-0.050977f,-0.018875f,-0.011266f,0.0092978f,-0.030766f,-0.033411f,-0.0092791f,-0.0046774f,0.017087f,0.017762f,-0.0053036f,-0.0082559f,-0.010197f,0.019531f,0.017457f,0.011548f,-0.0037201f,-0.011609f,-0.026852f,-0.0013399f,0.012875f,0.040485f,0.042525f,0.077198f,0.0922f,0.045561f,-0.013072f,-0.01219f,-0.011783f,0.022779f,0.04655f,0.027433f,-0.00045467f,-0.033092f,-0.067916f,-0.01135f,0.0056141f,0.029037f,0.027392f,0.04442f,0.037429f,0.055448f,0.021847f,0.032214f,0.0075187f,0.017986f,0.0076503f,0.031996f,0.035446f,0.043027f,0.047348f,0.052355f,0.0073934f,0.017435f,0.0075068f,0.032531f,-0.0094606f,-0.021531f,-0.047951f,0.02205f,0.056035f,0.097652f,0.04973f,0.042346f,0.0064728f,0.04474f,0.037192f,0.0084917f,0.0096192f,0.0063097f,-0.019076f,0.017491f,-0.016972f,-0.010396f,-0.040028f,0.003908f,0.0056381f,0.011797f,0.01455f,0.010987f,-0.030273f,-0.032861f,-0.020602f,-0.018495f,-0.021671f,-0.025524f,-0.0031952f,0.01249f,0.011594f,0.002796f,-0.0066703f,-0.020629f,-0.040505f,-0.067354f,-0.077781f,-0.052228f,-0.057558f,-0.011752f,-0.0055424f,0.02708f,0.036831f,0.061144f,0.03028f,0.063377f,0.036364f,0.041043f,-0.020944f,-0.016009f,-0.0046968f,-0.0029856f,0.0072926f,0.0033159f,0.011694f,0.01298f,-0.0028138f,-0.030874f,-0.026328f,-0.015508f,0.0015042f,0.025323f,0.0060927f,0.0032256f,-0.053467f,-0.054466f,-0.055307f,-0.014123f,-0.016279f,0.0066755f,-0.0048396f,0.024919f,0.031907f,0.0028393f,0.011685f,0.018205f,0.0513f,0.029459f,0.018676f,0.012615f,-0.0044974f,-0.0096974f,0.0046056f,0.0083523f,0.01994f,0.036298f,0.013373f,0.032915f,0.016588f,0.016042f,-0.035384f,-0.052886f,-0.070149f,-0.037288f,-0.056202f,-0.050955f,-0.06798f,-0.034404f,-0.034639f,-0.027012f,-0.013448f,0.02536f,0.014838f,0.031138f,0.051912f,0.040883f,0.033152f,0.0078394f,0.02244f,0.01337f,-0.0010805f,0.024133f,-0.0098629f,-0.001813f,0.021924f,-0.0012299f,-0.018614f,-0.048825f,-0.035652f,-0.0035253f,0.036069f,0.025373f,0.013388f,0.0050882f,0.034416f,0.0445f,0.031255f,-0.0073894f,-0.036634f,-0.03058f,-0.03987f,-0.018274f,0.0014493f,0.015947f,0.024042f,0.027539f,0.019821f,-0.011503f,-0.040901f,-0.071797f,-0.098557f,-0.08217f,-0.057136f,-0.010124f,0.030511f,0.0037156f,-0.051982f,-0.051764f,-0.059828f,0.0012506f,0.032581f,0.051578f,0.04889f,0.022215f,-0.021721f,0.0056636f,0.0055583f,-0.0051434f,-0.019948f,-0.0017125f,0.0079106f,0.0041567f,-0.025555f,-0.025776f,-0.04593f,-0.020606f,0.001574f,0.027294f,0.019176f,0.012543f,-0.014535f,0.016995f,0.00099492f,0.046028f,0.033467f,0.0047351f,0.0083698f,0.024542f,0.038897f,0.044922f,0.030256f,0.02442f,-0.03942f,-0.058591f,-0.082091f,-0.084074f,-0.13297f,-0.17546f},
    {0.81339f,-0.16501f,-0.12079f,-0.028089f,0.0048551f,0.031813f,-0.0037099f,0.013483f,0.0039313f,0.0054233f,0.019793f,0.014423f,0.029859f,0.06583f,0.055322f,0.020709f,0.00048379f,0.019106f,-0.0118f,-0.0042158f,0.014468f,0.030124f,0.035439f,0.045179f,0.011455f,-0.003382f,-0.027843f,-0.050781f,-0.063737f,-0.059265f,-0.034213f,-0.054277f,-0.039372f,0.037942f,0.014633f,0.022186f,-0.0012731f,0.015019f,-0.0298f,-0.01348f,0.0013488f,0.022096f,0.022335f,0.024003f,0.0020075f,-0.012036f,-0.011858f,-0.0042654f,0.010059f,-0.019283f,-0.02601f,-0.027799f,-0.029397f,-0.036844f,-0.022922f,0.0047051f,-0.014809f,0.020905f,0.0090652f,-0.0073557f,-0.032346f,-0.042607f,-0.023053f,0.038944f,0.054579f,0.041532f,0.014407f,0.0054882f,0.01036f,0.031409f,0.024769f,0.036785f,0.032452f,0.056509f,0.045383f,0.05038f,0.039513f,0.018836f,0.023279f,0.072024f,0.051391f,0.041534f,-0.024119f,-0.051537f,-0.061815f,-0.040422f,-0.032994f,0.012931f,0.0068773f,0.0057147f,-0.012641f,0.0088896f,-0.042243f,-0.01136f,-0.017472f,0.015978f,0.035755f,0.066831f,0.047195f,-0.0070396f,-0.044895f,-0.063646f,-0.036439f,-0.0052098f,0.013945f,0.026726f,0.045133f,0.049262f,0.073451f,0.06807f,0.084988f,0.091981f,0.029363f,0.015884f,0.0018888f,0.028289f,0.032269f,0.039661f,0.017182f,-0.010779f,-0.035517f,-0.02542f,-0.003379f,0.028272f,0.034825f,0.040746f,0.035308f,0.030342f,0.02786f,0.019079f,-0.031185f,-0.042923f,-0.053394f,-0.02177f,-0.0016372f,0.040629f,0.025443f,0.0024867f,-0.047243f,-0.058521f,-0.079384f,-0.061606f,-0.046056f,-0.020625f,-0.022122f,-0.044438f,-0.046177f,-0.011295f,-0.027178f,0.01541f,0.008604f,0.03822f,0.020449f,0.032376f,0.031451f,0.059886f,0.015927f,-0.0044649f,-0.06295f,-0.058143f,-0.042979f,-0.024714f,-0.026031f,-0.031516f,-0.017048f,-0.039029f,-0.015062f,-0.0124f,-0.038878f,-0.049179f,-0.039083f,-0.020975f,-0.0044433f,0.023959f,0.016193f,0.0029766f,0.012979f,0.030473f,0.059546f,0.059789f,-0.0037775f,-0.016104f,-0.029296f,-0.020192f,-0.0097416f,-0.015086f,-0.031999f,-0.035612f,-0.033237f,-0.0036456f,-0.015455f,-0.011111f,-0.026623f,0.012216f,0.054001f,0.073871f,0.030441f,0.0069299f,-0.039731f,-0.039966f,-0.028921f,-0.02609f,-0.011609f,0.028665f,0.0092213f,0.026469f,0.013325f,0.024938f,-0.01724f,0.015096f,0.003582f,0.016484f,-0.0099134f,-0.022057f,-0.046487f,-0.048555f,-0.05234f,-0.024915f,-0.0058804f,-0.0076594f,0.0080459f,0.00027889f,-0.020327f,-0.047499f,-0.0070715f,0.0086109f,-0.031039f,0.014457f,0.018403f,0.031791f,0.0067552f,0.0024914f,-0.017698f,-0.024274f,-0.017469f,0.0086777f,-0.00052962f,0.017867f,-0.0076111f,-0.019906f,0.0072061f,0.036728f,0.015819f,0.019782f,0.027047f,0.03451f,-0.0069154f,0.0068796f,-0.017577f,-0.0233f,-0.017956f,-0.034737f,-0.047794f,-0.062165f,-0.086255f,-0.13015f},
    {0.80341f,-0.12866f,-0.065714f,-0.054531f,-0.045877f,-0.033352f,-0.033522f,-0.00040769f,0.025291f,0.026945f,-0.010429f,-0.0034022f,-0.0079255f,-0.022719f,-0.020292f,-0.021782f,-0.017193f,-0.01136f,-0.031363f,-0.051725f,-0.02788f,-0.04436f,-0.03972f,0.015722f,0.029179f,0.050639f,0.020033f,0.044916f,0.034574f,0.014884f,-0.0064613f,-0.019673f,-0.045998f,-0.0028569f,0.035699f,0.028844f,-0.014054f,0.00058747f,-0.043295f,0.018096f,0.029907f,0.028482f,0.028812f,0.010746f,-0.0021376f,0.0015401f,-0.042744f,-0.013411f,-0.037056f,-0.059673f,-0.071267f,-0.063549f,-0.045526f,-0.031586f,-0.0017167f,0.02122f,0.037739f,0.06078f,0.063359f,0.036416f,-0.0083037f,0.021708f,0.02521f,0.041231f,0.025874f,0.033327f,0.017414f,0.048607f,0.021195f,0.025368f,0.015303f,-0.011226f,-0.020119f,0.024385f,0.013637f,0.0024663f,-0.024474f,-0.0062138f,-0.013827f,0.019934f,0.021593f,0.038551f,0.033426f,0.022638f,-0.011333f,-0.014578f,-0.010556f,-0.01496f,-0.011699f,-0.0069946f,0.032202f,0.049634f,0.0094931f,-0.014809f,-0.040726f,-0.029529f,-0.031463f,-0.0050079f,0.014778f,0.040102f,0.010348f,-0.030694f,-0.041861f,-0.043377f,-0.02632f,-0.010389f,-0.0078695f,-0.0017892f,-0.019082f,0.017188f,0.034052f,0.037209f,-0.001972f,-0.048271f,-0.073328f,-0.016217f,0.024971f,0.049763f,0.054368f,0.026118f,0.010461f,0.034708f,0.015427f,0.017035f,-0.0098924f,0.0067012f,0.022942f,0.014993f,0.002335f,0.0065482f,-0.018894f,0.01679f,0.024578f,0.023191f,0.017471f,0.019515f,0.030474f,0.031033f,-0.011832f,0.0011452f,-0.00032073f,0.047828f,0.088689f,0.087403f,0.024777f,-0.018043f,-0.058674f,-0.056355f,-0.041894f,-0.033656f,-0.043916f,-0.033127f,-0.036123f,-0.0089825f,-0.015595f,0.0092414f,-0.02006f,-0.012494f,0.0072929f,0.046514f,0.054844f,0.060747f,0.055467f,0.060991f,0.040255f,0.035824f,0.041176f,0.084483f,0.054149f,0.052979f,0.01671f,0.038414f,0.032869f,0.038535f,0.028673f,0.039586f,0.029252f,0.030602f,0.026315f,0.017029f,0.0071491f,-0.003503f,-0.019966f,0.0030314f,0.051567f,0.065997f,0.022405f,0.018359f,0.0028896f,-0.0028946f,0.015966f,0.014678f,0.0023851f,-0.012375f,-0.01558f,0.0087337f,0.033254f,0.0048222f,-0.048714f,-0.049388f,-0.059569f,-0.037479f,-0.034204f,0.009149f,0.014854f,0.057144f,0.053154f,0.05831f,0.019668f,0.0039413f,0.0076304f,-0.020493f,-0.031032f,-0.027683f,-0.054564f,-0.038604f,-0.067948f,-0.049669f,-0.0086523f,-0.024285f,-0.010524f,-0.032241f,-0.060204f,-0.010785f,0.026771f,0.032184f,0.018133f,0.025039f,-0.013796f,-0.015093f,-0.01098f,-0.038064f,-0.036355f,-0.015963f,0.018012f,0.030354f,-0.015876f,-0.0009934f,-0.050431f,-0.055669f,-0.03514f,0.033991f,0.053665f,0.084564f,0.037577f,0.023527f,-0.0034106f,-0.041506f,-0.069307f,-0.033467f,-0.01102f,0.016973f,0.011163f,-0.02395f,-0.13326f,-0.17539f},
    {0.8163f,-0.15439f,-0.12114f,-0.052199f,-0.039282f,-0.028851f,-0.052872f,-0.029906f,-0.033095f,0.0091509f,0.013319f,0.041408f,0.045693f,0.047658f,-0.0062763f,0.042752f,-0.0051281f,0.033181f,0.0018501f,-0.016704f,-0.024444f,-0.043678f,-0.010206f,-0.0020157f,-0.017165f,0.024874f,0.0077619f,-0.019483f,-0.065384f,-0.061097f,-0.057476f,-0.012586f,0.0062826f,0.028519f,0.011319f,0.023532f,0.019101f,-0.01592f,-0.046539f,-0.035477f,-0.063493f,-0.0274f,-0.0072361f,0.016274f,0.017137f,0.0091716f,-0.037772f,-0.06575f,-0.071171f,-0.045698f,-0.053797f,-0.008386f,-0.011899f,-0.00378f,-0.017899f,0.0038118f,-0.0033431f,0.028699f,0.042759f,0.049028f,0.045802f,0.047863f,0.048688f,0.05481f,0.070535f,0.049481f,0.025511f,0.013307f,-0.0016922f,0.015305f,-0.023929f,-0.059152f,-0.04314f,-0.0007867f,0.0062852f,0.018666f,-0.034701f,-0.04545f,-0.070614f,-0.035689f,-0.011123f,-0.010856f,-0.026415f,-0.038303f,-0.051613f,-0.028817f,-0.0014447f,0.034234f,0.030188f,0.057294f,0.033034f,0.028222f,0.060114f,0.061107f,0.019812f,0.0004777f,-0.046849f,-0.015545f,-0.036458f,-0.020063f,0.0079747f,0.0093404f,0.017841f,0.059559f,0.043637f,-0.010872f,-0.031518f,-0.0031017f,-0.011589f,0.039468f,0.016764f,0.013459f,0.0086087f,-0.015147f,-0.031529f,-0.028365f,-0.027517f,0.019413f,0.049968f,0.053629f,0.024631f,0.021844f,-0.022057f,-0.020534f,-0.030507f,-0.012869f,-0.044163f,-0.015919f,-0.009818f,0.019012f,0.045714f,0.038912f,0.045933f,0.049964f,0.045923f,0.024903f,0.018772f,0.010618f,-0.0099226f,0.017465f,0.028333f,0.036624f,0.028592f,0.046883f,0.027492f,0.045722f,0.022986f,0.013221f,0.046236f,0.05456f,0.063769f,0.05557f,-0.012475f,-0.0072173f,0.034116f,0.05803f,0.024326f,0.03401f,0.0074026f,-0.0039146f,0.027779f,0.027398f,0.0074613f,-0.012501f,-0.028711f,0.021777f,0.012411f,-0.00075524f,-0.0071851f,-0.019289f,-0.027985f,-0.0010704f,0.010154f,0.011165f,-0.019698f,-0.011983f,-0.014768f,0.035816f,0.051235f,0.029145f,-0.013703f,-0.053031f,-0.0434f,0.0007249f,0.046891f,0.091778f,0.041313f,0.017356f,0.0072636f,0.0063344f,-0.013934f,-0.012067f,-0.02753f,0.01751f,0.013572f,0.022935f,-0.0054341f,-0.008004f,-0.034339f,-0.01693f,0.0027987f,0.0066352f,0.010485f,-0.000696f,-0.022043f,-0.023556f,0.00043235f,0.010774f,0.023931f,0.039217f,0.0018708f,-0.03007f,-0.056157f,-0.039483f,-0.027767f,0.014299f,0.0425f,0.036394f,0.04267f,0.037955f,-0.01228f,-0.0023586f,-0.032917f,-0.068006f,-0.090667f,-0.081711f,-0.050584f,-0.014209f,0.0071977f,0.00973f,-0.014464f,-0.012786f,0.0064564f,0.0062105f,0.0064105f,0.059359f,0.027278f,0.033982f,-0.0023915f,-0.013706f,0.024027f,0.003364f,0.04182f,-0.00066563f,-0.028329f,-0.01953f,-0.014608f,0.0045806f,0.025038f,0.029357f,0.038704f,0.03786f,0.026176f,-0.011151f,-0.06266f,-0.12307f},
    {0.79639f,-0.12963f,-0.084858f,-0.044216f,-0.035136f,-0.012269f,-0.044961f,-0.017579f,-0.059784f,-0.07684f,-0.08918f,-0.0296f,-0.0039621f,0.028813f,0.029031f,0.058162f,0.023186f,0.015358f,-0.0098352f,-0.013944f,0.0019626f,0.0041873f,-0.01146f,0.011858f,0.028597f,0.084615f,0.049759f,0.024886f,-0.041567f,-0.06606f,-0.064092f,-0.026155f,-0.0091372f,0.0035861f,-0.028544f,0.0029482f,-0.017966f,0.019241f,-0.027231f,0.027312f,0.037093f,0.039426f,0.0029655f,0.03148f,0.025069f,0.023184f,0.036321f,0.0020731f,-0.0058071f,0.017297f,-0.015567f,0.024912f,0.0006707f,0.011665f,0.023565f,0.006327f,-0.0090828f,0.026831f,0.02832f,0.035528f,-0.02985f,-0.022769f,-0.035736f,-0.0037114f,0.025564f,0.028696f,0.019649f,0.013217f,0.0082697f,-0.002424f,-0.025093f,-0.0060455f,-0.020011f,-0.004671f,0.0068829f,0.0036476f,-0.026251f,0.019312f,0.0064268f,-0.0030406f,-0.010363f,0.013646f,0.0055741f,0.013269f,0.0044914f,0.0017068f,-0.033415f,-0.068316f,-0.040379f,-0.022278f,0.0002561f,0.018372f,-0.011587f,0.024966f,0.017048f,0.032949f,0.0025882f,0.0158f,0.011241f,0.045448f,0.080143f,0.12842f,0.073621f,0.017279f,0.0064929f,0.0014059f,0.00044336f,0.0097425f,0.0064831f,0.016469f,-0.0096087f,0.0035622f,0.011173f,0.032702f,0.04926f,0.069054f,0.065575f,0.042519f,0.02255f,-0.035883f,-0.030762f,-0.046212f,-0.042609f,-0.044691f,-0.080613f,-0.077708f,-0.088442f,-0.084277f,-0.035481f,0.027126f,0.0049671f,-0.00193f,-0.027969f,-0.044005f,-0.046067f,-0.0069316f,-0.012095f,0.026695f,-0.027022f,-0.010913f,-0.033127f,-0.010786f,-0.010221f,0.0025099f,-0.0090602f,0.0067045f,0.016825f,0.02656f,0.034347f,0.029795f,-0.0032075f,-0.019639f,-0.052174f,-0.040122f,-0.098974f,-0.12508f,-0.093004f,-0.040866f,-0.015675f,-0.0066688f,-0.017663f,-0.041246f,-0.046789f,-0.034683f,0.00034772f,-0.0063005f,-0.012582f,0.00050699f,-0.015628f,0.026695f,-0.0066159f,-0.0012781f,0.0056396f,0.017784f,0.016398f,0.013494f,0.013712f,0.011682f,-0.025815f,-0.03832f,-0.01769f,-0.038193f,-0.047456f,-0.0017859f,0.01566f,0.023855f,0.024526f,0.016704f,0.00068197f,0.027937f,0.018543f,0.012923f,-0.014082f,0.014824f,0.034515f,0.040018f,0.032684f,-0.01633f,-0.030439f,-0.0047557f,0.026665f,0.029306f,-0.013901f,0.00055536f,-0.017704f,-0.029969f,0.0054947f,-0.00088274f,0.02767f,0.039759f,-0.012166f,0.0011456f,-0.045238f,-0.04842f,-0.026275f,-0.016234f,0.00023569f,0.034084f,0.059853f,0.0017875f,-0.0029951f,-0.030339f,-0.035828f,-0.056164f,-0.066358f,-0.026544f,0.014446f,0.049771f,0.02562f,-0.0094967f,-0.038621f,-0.051203f,-0.013039f,0.026703f,0.03559f,0.025295f,-0.0037071f,0.02456f,0.010983f,0.0079635f,-0.0083463f,-0.019858f,-0.014491f,-0.010163f,-0.012641f,0.0097042f,0.044627f,0.049791f,0.015714f,-0.0082806f,0.010298f,-0.0048464f,-0.01442f,-0.052118f,-0.13489f,-0.1851f},
    {0.77068f,-0.18633f,-0.17252f,-0.10504f,-0.037049f,0.012179f,0.0055775f,0.0046f,-0.033758f,-0.011895f,-0.018635f,-0.01006f,0.016785f,0.013797f,-0.0046978f,0.0071692f,0.050245f,0.10254f,0.10468f,0.08656f,0.047278f,0.028113f,0.0027445f,0.013183f,0.0054045f,-0.052262f,-0.047425f,-0.036177f,0.0024324f,0.052226f,0.053805f,0.095475f,0.068909f,0.091858f,0.048549f,0.040492f,-0.018811f,-0.00064433f,-0.040451f,-0.043512f,-0.014094f,-0.0082277f,-0.020528f,-0.051976f,-0.065104f,-0.021457f,-0.024485f,0.0016844f,-0.00036024f,0.0013566f,-0.0065573f,0.0017903f,-0.0059937f,0.0025449f,0.02675f,0.046138f,0.0088926f,0.023479f,-0.018916f,-0.0005824f,0.047773f,0.041426f,0.014232f,0.019529f,0.026804f,0.023101f,0.034469f,0.026307f,-0.0012259f,-0.042221f,-0.047258f,-0.0015075f,-0.0013735f,0.034802f,0.014366f,0.018421f,-0.0014828f,-0.0025426f,-0.020894f,-0.0049822f,0.012611f,0.050692f,0.042723f,0.039679f,-0.013436f,-0.011398f,-0.026356f,0.0075735f,0.00097668f,0.039407f,0.0051318f,0.027477f,-0.0051976f,0.0036039f,-0.035653f,-0.022907f,-0.011431f,0.014885f,0.02245f,0.024931f,0.012121f,0.033809f,0.050089f,0.058129f,0.027595f,0.015192f,-0.02543f,0.0053263f,-0.030328f,0.014514f,0.031794f,0.036756f,0.044294f,0.039876f,-0.01548f,0.0033672f,-0.069485f,-0.077559f,-0.07418f,-0.097932f,-0.069951f,-0.030509f,-0.031234f,-0.020004f,-0.026936f,0.011441f,0.0040747f,0.01623f,-0.007334f,-0.019598f,-0.023171f,-0.012289f,0.010979f,0.021569f,0.010988f,-0.00064167f,0.00047287f,0.025004f,-0.018176f,0.0076792f,-0.0047779f,-0.045824f,-0.032121f,-0.062148f,-0.065563f,-0.03513f,-0.032329f,0.0082468f,0.040736f,0.020426f,0.023727f,-0.0066444f,-0.039215f,-0.053294f,-0.056088f,-0.01462f,-0.008932f,-0.0073748f,-0.034482f,-0.022188f,-0.016056f,0.01705f,0.03182f,0.020877f,0.024472f,0.008123f,0.021673f,0.0015442f,0.022251f,0.021849f,0.031439f,0.02402f,0.014984f,-0.017442f,-0.030303f,-0.010113f,0.025365f,0.064515f,0.057021f,0.019909f,-0.0325f,-0.051321f,-0.049131f,-0.035935f,-0.0016417f,0.015474f,0.034855f,0.045508f,-0.0044374f,-0.016111f,-0.03108f,-0.0087691f,-0.021909f,-0.0050234f,-0.011433f,-0.025136f,-0.050664f,0.010628f,0.014746f,-0.00051178f,0.0087098f,0.0026535f,0.01496f,0.049997f,0.018305f,-0.0059797f,-0.036694f,-0.029727f,-0.032351f,-0.0049294f,0.006703f,-0.0055739f,-0.0051386f,-0.010707f,-0.049657f,-0.031662f,-0.033872f,0.0092464f,-0.0098005f,-0.025325f,-0.0001167f,-0.011273f,-0.0095921f,-0.025474f,-0.011766f,-0.017573f,0.010135f,0.019091f,0.029105f,0.051242f,0.023401f,0.037348f,-0.029488f,-0.002722f,0.013095f,0.027821f,0.005756f,-0.014525f,-0.057232f,-0.063161f,-0.059726f,-0.024894f,-0.049255f,-0.05554f,-0.072259f,-0.019162f,-0.011572f,0.0024669f,0.0021002f,-0.028352f,-0.051771f,-0.052629f,-0.047829f,-0.042301f,-0.10625f,-0.183f},
    {0.76448f,-0.1717f,-0.12131f,-0.043083f,-0.016581f,0.0014073f,0.0049664f,0.0067678f,-0.0034117f,-0.00045595f,-0.038611f,-0.023611f,-0.01923f,0.0016333f,0.0034093f,-0.011235f,-0.015427f,-0.016477f,0.022319f,0.06853f,0.075831f,0.036566f,0.0092599f,-0.0084341f,-0.04265f,0.00053809f,-0.025057f,-0.032588f,-0.048123f,-0.046523f,-0.037046f,-0.018407f,-0.032952f,-0.028035f,0.0058686f,0.020559f,0.03703f,0.041211f,0.029423f,0.010593f,0.0015996f,0.00924f,0.008481f,0.0031024f,-0.031124f,-0.05125f,-0.071845f,-0.017022f,-0.019713f,0.0051479f,-0.019121f,-0.0092447f,-0.045784f,0.021042f,0.04088f,0.038593f,0.060076f,0.014045f,-0.026653f,-0.035005f,-0.033744f,0.011417f,-0.004787f,0.016118f,-0.0013077f,0.033075f,0.010386f,0.0054922f,-0.015708f,-0.018403f,-0.051264f,-0.052445f,-0.052161f,-0.0011129f,0.013171f,0.027237f,0.013381f,-0.0076897f,-0.035892f,-0.018354f,-0.017561f,0.034533f,0.026479f,0.036029f,0.010206f,0.011653f,-0.0060019f,0.024383f,0.030915f,-0.021372f,-0.049879f,-0.052254f,-0.036432f,0.043699f,0.03248f,0.028329f,-0.044998f,-0.032258f,-0.024953f,-0.028882f,-0.0003116f,0.013321f,0.02225f,0.044805f,-0.0038631f,-0.011027f,-0.039228f,-0.035949f,0.0026571f,0.024494f,0.015451f,0.066221f,0.016916f,0.029413f,0.034065f,0.044937f,0.010005f,0.0071513f,-0.02621f,0.012638f,-0.024048f,-0.0084579f,-0.017054f,0.050814f,0.049287f,0.11645f,0.11716f,0.092831f,0.042333f,0.001804f,-0.047043f,-0.043648f,-0.01474f,-0.020969f,0.01314f,0.0012712f,-0.0078873f,-0.0283f,0.0012524f,0.0089423f,0.019029f,0.036389f,0.052816f,0.065116f,0.043634f,-0.022485f,-0.0047051f,-0.00317f,0.011814f,0.084262f,0.0818f,0.081898f,0.069457f,0.041734f,0.045455f,0.056778f,0.035509f,0.064172f,0.036891f,0.05638f,0.050899f,-0.011459f,-0.037866f,-0.047411f,0.0068631f,0.026713f,0.0087828f,-0.0070141f,-0.069655f,-0.056435f,-0.061354f,-0.078594f,-0.088874f,-0.082623f,-0.054184f,-0.021025f,0.026856f,0.035996f,0.026415f,0.012155f,-0.015215f,-0.015149f,-0.021265f,-0.010346f,0.011767f,0.02328f,0.0061198f,-0.011864f,-0.031507f,-0.0362f,-0.044456f,-0.035437f,-0.0033327f,0.00090366f,0.0042761f,0.0022962f,0.036844f,0.058676f,0.040441f,0.024545f,-0.03483f,-0.0029758f,-0.021324f,0.011881f,0.063126f,0.035584f,0.021294f,-0.000515f,0.0031817f,0.0093572f,0.056126f,0.071416f,0.042418f,0.025594f,-0.015341f,-0.018377f,-0.025336f,-0.0021364f,-0.021792f,-0.0058279f,-0.01914f,-0.0076638f,-0.022624f,0.016321f,-0.0053187f,-0.012386f,-0.019804f,-0.0058367f,-0.026276f,-0.014964f,-0.013456f,0.0070219f,0.066224f,0.069916f,0.049591f,0.035551f,-0.039158f,-0.041949f,-0.039958f,0.0017352f,-0.016892f,0.00091034f,-0.025766f,-0.0067857f,0.00056967f,0.030651f,0.037396f,0.0092852f,-0.019431f,-0.023307f,-0.030468f,0.0058184f,-0.00082362f,-0.042351f,-0.1248f,-0.1938f},
    {0.79292f,-0.16687f,-0.13469f,-0.072585f,-0.028298f,0.028629f,0.064771f,0.052227f,-0.014477f,-0.020666f,-0.024838f,-0.00328f,0.018539f,0.030802f,0.00062994f,-0.013165f,-0.015196f,-0.0054198f,-0.022797f,-0.0025824f,-0.038639f,-0.021644f,-0.057171f,-0.054757f,-0.085248f,-0.089391f,-0.052493f,-0.0059103f,0.032396f,0.031581f,-0.022539f,-0.036806f,-0.02993f,0.005416f,0.0326f,0.066868f,0.0602f,0.047537f,0.019427f,0.041068f,0.056832f,0.049127f,0.06846f,0.040713f,0.012f,-0.00020747f,0.012951f,0.060141f,0.031353f,0.026925f,0.010031f,0.023074f,0.027259f,0.030149f,-0.0020689f,-0.0030417f,-0.031667f,0.0076435f,0.045252f,0.091176f,0.062453f,0.04603f,-0.019526f,-0.029736f,-0.0087696f,-0.021272f,0.010852f,-0.016138f,-0.01467f,0.0045007f,0.017031f,3.893e-005f,0.017144f,0.03634f,0.044947f,0.066032f,0.064412f,0.041805f,-0.013313f,-0.032683f,-0.07105f,-0.038994f,-0.075609f,-0.05304f,-0.017108f,0.034f,0.017686f,0.0069472f,-0.015717f,0.002001f,-0.045637f,-0.050574f,-0.051899f,-0.055912f,-0.040546f,-0.027007f,-0.05798f,-0.025953f,-0.0019607f,0.004873f,0.0010868f,0.025101f,0.0023581f,-0.0063207f,-0.0081396f,-0.010852f,-0.031911f,-0.020172f,-0.0089035f,0.0077149f,0.020966f,-0.016022f,-0.012153f,-0.027739f,-0.050232f,-0.020011f,-0.017723f,0.012967f,0.041676f,0.027798f,0.033101f,0.019151f,0.011232f,0.022856f,0.017985f,0.012518f,-0.035379f,-0.02595f,-0.018191f,0.0029465f,-0.010392f,-0.039126f,-0.063305f,-0.044201f,-0.030255f,-7.0801e-005f,-0.011654f,0.010761f,-0.024581f,0.008313f,0.0082446f,0.0083625f,0.0078577f,-0.0048911f,-0.031508f,-0.0013042f,-0.04732f,-0.023643f,-0.031341f,-0.020253f,-0.03314f,-0.039086f,-0.050933f,-0.023992f,-0.02496f,-0.011512f,0.0061651f,0.014187f,-0.0047108f,0.015138f,0.021289f,0.0049654f,0.0063876f,0.03771f,0.021012f,0.03546f,0.0108f,-0.022649f,-0.016878f,-0.012624f,-0.04215f,-0.055329f,-0.042826f,-0.0022652f,-0.0050047f,-0.036249f,-0.01109f,-0.014935f,-0.011041f,-0.00078236f,-0.010406f,0.0066156f,-0.0010442f,-0.0057067f,-0.02603f,-0.013956f,-0.052294f,-0.017854f,0.011462f,0.059275f,0.027251f,0.064579f,0.0039042f,0.010917f,-0.026746f,-0.064931f,-0.077221f,-0.070437f,-0.036369f,0.01612f,0.032132f,0.011808f,-0.0044807f,-0.010886f,-0.0062852f,0.034003f,0.045393f,0.040829f,0.0068622f,-0.019503f,0.0048537f,0.036468f,0.013567f,0.003478f,-0.039004f,0.00083853f,-0.015944f,0.021212f,0.02606f,0.010508f,-0.013794f,-0.0043122f,0.0005567f,0.013033f,-0.0095099f,-0.023444f,-0.061075f,-0.08741f,-0.084446f,-0.0248f,-0.010812f,0.024892f,-0.0018393f,-0.0012291f,0.025286f,0.039358f,0.057209f,0.026568f,0.0069298f,-0.025061f,-0.035492f,-0.015728f,-0.025565f,-0.036214f,-0.040568f,-0.0035288f,0.01151f,0.024184f,-0.0086493f,-0.074785f,-0.10986f,-0.049641f,-0.010378f,-0.0065522f,-0.075353f,-0.15214f},
    {0.79736f,-0.11536f,-0.083901f,-0.054393f,-0.045808f,-0.017605f,-0.021671f,-0.0014367f,0.025559f,0.072646f,0.04424f,0.047633f,-0.0014456f,-0.028111f,-0.014383f,-0.018114f,-0.016355f,-0.021223f,-0.031281f,0.0050634f,0.024868f,0.033722f,-0.032078f,-0.022421f,-0.013829f,0.050736f,0.053856f,0.0098335f,0.00070483f,0.0074712f,0.022431f,0.072816f,0.049228f,0.030662f,0.026263f,-0.015802f,0.015945f,0.024797f,-0.0025418f,-0.018644f,-0.038722f,-0.038966f,-0.0312f,-0.03521f,0.014622f,0.0036742f,0.0063475f,-0.0079588f,-0.0071907f,0.0072346f,-0.024722f,-0.0011287f,-0.0083127f,0.015883f,-0.037642f,0.0063736f,-0.01038f,-0.0051602f,-0.0053478f,0.0019896f,-0.020374f,-0.026001f,-0.035011f,-0.0044764f,0.030144f,0.072799f,0.099034f,0.080367f,0.041375f,0.017182f,0.013682f,0.02703f,0.032788f,0.021746f,-0.0099102f,-0.054498f,-0.074542f,-0.054217f,-0.041853f,-0.010574f,-0.046658f,-0.015665f,-0.023514f,-0.029612f,-0.031616f,-0.042042f,-0.054777f,-0.029079f,-0.0030957f,0.010852f,0.018083f,-0.0098111f,-0.047765f,-0.054771f,-0.090385f,-0.076951f,-0.087679f,-0.049705f,-0.024764f,-0.010002f,0.016689f,-0.026786f,-0.0039718f,0.016933f,0.0031393f,0.002889f,0.016013f,-0.031587f,-0.046375f,-0.0090674f,-0.00047012f,0.01584f,0.048966f,0.029358f,-0.0052245f,-0.015714f,0.0059731f,-0.00027258f,0.024955f,0.030141f,0.0012801f,-0.037376f,-0.06098f,-0.041763f,-0.022261f,-0.026123f,-0.012349f,-0.014027f,0.0070429f,0.048896f,0.034793f,0.051882f,-0.000241f,-0.0044421f,-0.022522f,-0.030373f,-0.072983f,-0.067159f,-0.071777f,-0.027253f,-0.033748f,0.019016f,0.022301f,0.022262f,-0.0084679f,0.018787f,-0.013006f,-0.024889f,-0.014705f,-0.010244f,-0.053489f,-0.0076702f,-0.0027659f,0.0065704f,0.025158f,0.066832f,0.0021742f,0.021143f,-0.01649f,-0.0093405f,-0.0068922f,-0.0044718f,0.0091903f,0.016577f,0.013772f,0.0009393f,-0.053717f,-0.05133f,-0.071302f,-0.031093f,-0.013714f,0.0071312f,-0.011048f,0.0016158f,-0.02608f,-0.015927f,0.0083267f,-0.0038046f,0.021983f,0.054962f,0.055208f,0.048608f,-0.011894f,-0.019909f,-0.029506f,0.019555f,0.043459f,0.07308f,0.037099f,-0.0015235f,-0.061903f,-0.050845f,-0.04711f,0.0041082f,0.037713f,0.054828f,0.039086f,0.022861f,0.0027993f,0.020512f,0.018825f,0.032752f,0.062548f,0.024707f,0.034202f,0.043264f,0.058807f,0.037168f,0.022615f,0.020032f,-0.023114f,-0.021687f,-0.061072f,-0.0031883f,-0.027518f,-0.00156f,-0.010714f,-0.0026421f,-0.008667f,-0.00093303f,0.0024965f,0.051474f,-0.0036161f,0.0014857f,-0.028081f,-0.026304f,0.012858f,0.052697f,0.045043f,0.048617f,-0.037363f,-0.057626f,-0.039696f,-0.023563f,-0.016201f,-0.048061f,-0.034171f,0.0048874f,0.025192f,0.01376f,-0.034692f,-0.032254f,-0.048659f,-0.0015735f,-0.021636f,-0.040356f,-0.04813f,-0.067444f,-0.054107f,-0.024276f,-0.016378f,0.013995f,0.026496f,-0.0081219f,-0.11189f,-0.18588f},
    {0.78168f,-0.15405f,-0.089076f,-0.064619f,-0.0368f,-0.0010542f,0.0076596f,0.038435f,0.010278f,-0.028121f,-0.058479f,-0.06905f,-0.06326f,-0.064484f,-0.03423f,-0.0068834f,0.015317f,0.0077801f,-0.010645f,0.0014526f,-0.017479f,-0.011061f,-0.045818f,0.00083589f,-0.009073f,-0.016167f,-0.017203f,-0.0034955f,-0.0032446f,0.039917f,0.028278f,0.037385f,0.038346f,0.032696f,0.014611f,-0.0056419f,-0.069631f,-0.059116f,-0.044879f,-0.030153f,-0.034508f,0.0029597f,0.0020941f,0.017931f,0.003627f,0.026434f,-0.035456f,-0.0063187f,-0.019188f,-0.016957f,-0.0048145f,0.015231f,0.011791f,0.032664f,0.043299f,0.030321f,0.018718f,0.0023042f,-0.014798f,0.0095257f,-0.015868f,-0.0093614f,-0.039938f,-0.027709f,-0.023575f,-0.02741f,-0.052154f,-0.046388f,-0.079313f,-0.043115f,-0.012302f,0.020105f,0.0040514f,0.013133f,0.00034414f,-0.0027795f,0.0052582f,-0.033196f,-0.062917f,-0.014548f,0.0027389f,0.04388f,0.049245f,0.01326f,-0.0074732f,-0.0085204f,-0.01322f,0.031206f,-0.013646f,0.010232f,-0.011187f,0.0072647f,-0.026298f,-0.027361f,-0.045501f,-0.043768f,-0.055989f,-0.056203f,-0.028233f,0.016871f,0.029062f,0.038393f,0.019372f,-0.011168f,-0.020265f,-0.00072067f,-0.019419f,-0.014844f,-0.010198f,0.035814f,0.048228f,0.057709f,0.017576f,0.0021802f,-0.007759f,0.028738f,0.038368f,0.031003f,-0.0072056f,-0.039083f,-0.057388f,-0.033832f,0.009269f,0.047409f,0.020494f,0.012681f,-0.035595f,-0.040236f,-0.044991f,-0.019754f,-0.0073345f,-0.010917f,-0.014154f,-0.029434f,-0.012851f,0.034516f,0.030629f,0.02234f,-0.024917f,-0.041953f,-0.057071f,-0.020276f,0.0165f,0.042736f,0.028983f,0.012777f,0.0010096f,-0.0014728f,0.0030766f,0.00030962f,-0.018825f,-0.028884f,-0.030244f,-0.022473f,-0.058969f,-0.043182f,-0.032223f,0.0059723f,0.02886f,0.044107f,-0.0022635f,-0.029098f,-0.060139f,-0.051309f,-0.043013f,-0.027087f,0.0070015f,0.018867f,0.032737f,-0.0017854f,0.02134f,0.024211f,0.010638f,0.012882f,-0.034416f,-0.0091292f,0.018918f,0.047653f,0.07543f,0.054925f,0.0058275f,0.036839f,0.031673f,0.035364f,0.017499f,-0.0009155f,-0.0047424f,0.022673f,0.023039f,-0.010169f,-0.029987f,-0.035533f,-0.040857f,0.00066055f,0.017853f,0.016886f,0.010231f,0.0011446f,0.0085697f,0.0049733f,-0.024994f,-0.020746f,-0.052059f,-0.032275f,-0.025857f,-0.0044337f,0.01012f,0.041117f,0.022402f,0.030622f,0.051546f,0.013342f,0.0082567f,-0.010633f,-0.027775f,-0.012129f,0.028852f,0.061639f,0.063145f,0.09042f,0.083061f,0.048064f,0.043334f,0.067356f,0.057314f,0.059746f,0.021208f,-0.013867f,-0.012048f,-0.011248f,-0.021031f,-0.0083485f,-0.032322f,-0.022429f,0.011296f,0.0052319f,0.0040631f,0.013433f,0.019817f,0.025766f,-0.0012551f,0.02023f,0.0067211f,0.020574f,-0.02304f,-0.013111f,-0.030069f,-0.030549f,-0.072945f,-0.088933f,-0.084401f,-0.067157f,-0.065837f,-0.087402f,-0.18034f,-0.20342f},
    {0.81208f,-0.12805f,-0.063991f,-0.032353f,-0.039786f,-0.025332f,-0.053695f,-0.038351f,-0.049299f,-0.011178f,0.028547f,0.084796f,0.056876f,0.056903f,-0.01413f,-0.032161f,-0.022917f,0.032852f,0.023186f,-0.024535f,-0.0047217f,-0.00099205f,0.031555f,0.064337f,0.063012f,0.077193f,0.039827f,0.01443f,-0.024549f,-0.023475f,-0.0086893f,0.026703f,0.0099292f,0.02726f,-0.045299f,-0.043281f,-0.047499f,-0.013675f,-0.0059253f,0.014455f,0.02503f,0.030686f,0.0093f,0.019636f,0.0051832f,0.027548f,0.033772f,0.062123f,0.035088f,0.033806f,0.0089317f,0.0042965f,-0.036206f,-0.011912f,-0.03597f,-0.024445f,0.00035664f,0.032723f,-0.0024518f,0.0077816f,-0.015344f,-0.027212f,-0.050675f,-0.047643f,-0.04387f,0.0067384f,-0.018445f,0.010024f,-0.0072802f,-0.015409f,0.0096206f,0.03145f,0.004738f,0.018444f,0.015514f,0.040418f,0.044326f,0.0069866f,-0.02829f,-0.0054059f,-0.022701f,-0.038782f,-0.056895f,-0.079872f,-0.114f,-0.065247f,-0.044789f,0.0010234f,0.024203f,0.0013865f,-0.02242f,-0.02883f,-0.010542f,0.039301f,0.011636f,0.0046772f,-0.025106f,-0.02669f,-0.015016f,-0.0057703f,0.0049185f,-0.011133f,-0.03671f,-0.045011f,-0.037436f,-0.044384f,-0.021885f,-0.008195f,-0.0061131f,0.026834f,0.0075834f,0.039055f,0.0058757f,0.029404f,-0.002079f,0.0070104f,-0.0075937f,0.0077593f,-0.027125f,-0.058232f,-0.038445f,-0.03776f,-0.039621f,-0.0022493f,-0.012695f,0.0018136f,0.014156f,0.018611f,0.014002f,0.022953f,0.0086893f,0.032925f,0.03459f,0.043595f,0.062369f,0.067823f,0.042902f,0.066774f,0.018646f,-0.0030798f,0.0048747f,0.01581f,0.020853f,0.016675f,0.025748f,-0.0017248f,0.00083281f,-0.023075f,-0.021866f,-0.01927f,-0.03097f,-0.011217f,-0.05379f,-0.035352f,-0.063166f,-0.060785f,-0.08052f,-0.043273f,-0.044136f,-0.021612f,-0.036165f,-0.04562f,-0.050322f,-0.053146f,-0.020341f,-0.0049581f,-0.023681f,-0.025558f,-0.051471f,-0.02116f,-0.014954f,0.00044324f,0.020223f,0.018f,0.013974f,0.044104f,0.033225f,0.037884f,0.025972f,-0.021511f,-0.088284f,-0.061251f,-0.022498f,0.011375f,0.012552f,-0.0072083f,-0.012467f,0.020108f,-0.00047597f,-0.005359f,-0.0047111f,-0.03073f,-0.0054949f,0.0079696f,0.0028058f,0.012016f,0.0040974f,0.010501f,0.0013724f,-0.031967f,-0.041799f,-0.026908f,-0.035552f,-0.028868f,0.0086754f,0.02112f,0.018245f,0.011614f,-0.014635f,-0.043288f,-0.031911f,-0.001835f,0.015534f,0.027027f,0.012495f,-0.011646f,-0.045607f,-0.041005f,-0.047168f,-0.030764f,-0.0080659f,0.019305f,0.026885f,-0.0143f,-0.0018251f,-0.00027344f,0.014856f,0.049639f,0.045643f,0.041226f,0.0071527f,-0.013529f,-0.030077f,-0.040041f,-0.044709f,-0.013349f,-0.037688f,0.009951f,-0.045849f,-0.026552f,0.027279f,0.064176f,0.045613f,0.0074273f,-0.0022153f,-0.026782f,0.00048766f,0.028081f,0.024607f,0.0074522f,-0.0025354f,-0.010821f,-0.021668f,-0.068689f,-0.14692f,-0.16366f},
    {0.78195f,-0.15853f,-0.094882f,-0.0012374f,0.070278f,0.074902f,0.024608f,0.017898f,-0.016642f,-0.02833f,0.027419f,0.035392f,0.015127f,0.015781f,-0.026405f,0.011714f,0.03114f,0.074005f,0.03538f,0.022443f,-0.055615f,-0.054739f,-0.063247f,-0.026367f,0.016727f,0.061264f,0.043269f,0.0093594f,0.01395f,0.020024f,0.026592f,-0.010597f,-0.030239f,-0.047585f,-0.06052f,-0.02877f,-0.012038f,-0.014285f,-0.028901f,-0.028623f,-0.013269f,-0.0085029f,-0.044302f,-0.037653f,0.0044742f,0.034018f,0.051589f,0.046972f,-0.0057135f,-0.021252f,-0.041381f,-0.027309f,-0.042178f,-0.023126f,-0.046334f,-0.034625f,-0.022817f,0.0091435f,-0.0038588f,0.017863f,-0.0075051f,0.016485f,0.022422f,0.032571f,0.030395f,0.040892f,0.026642f,-0.0031407f,-0.0096811f,-0.030237f,-0.043957f,-0.034888f,-0.051804f,-0.027787f,-0.038054f,0.010156f,0.045283f,0.046959f,0.070162f,0.062326f,0.025892f,-0.0054864f,0.011215f,-0.0074162f,-0.02178f,0.024677f,0.0060255f,0.01239f,0.0047697f,-0.0089219f,-0.04317f,-0.028365f,0.019671f,0.054538f,0.057051f,0.059861f,0.042224f,0.0095414f,-0.0214f,0.0078016f,0.014841f,0.045571f,0.028178f,0.020107f,-0.031163f,-0.048669f,-0.048881f,-0.017489f,0.02035f,0.0076693f,-0.030358f,-0.078859f,-0.067016f,-0.04723f,-0.048719f,-0.016067f,-0.02158f,0.00351f,0.0091514f,0.01064f,0.017997f,0.035966f,0.059393f,0.057925f,0.014802f,0.013988f,-0.022655f,0.00064334f,-0.0078617f,0.013673f,0.039285f,0.043826f,0.039922f,0.0096291f,-0.027164f,-0.016585f,-0.033711f,-0.023236f,-0.041741f,-0.02971f,-0.026817f,-0.020383f,-0.0050476f,0.0061114f,0.011625f,0.044632f,0.0054916f,0.01554f,0.011986f,0.063973f,0.057027f,0.038596f,-0.0083143f,-0.042336f,-0.059853f,-0.047098f,-0.026943f,0.0023112f,0.027943f,0.027039f,-0.0012801f,0.01186f,-0.0069562f,-0.0086699f,-0.013315f,0.030443f,0.03166f,0.027242f,0.0054802f,0.018023f,0.0019681f,-0.011119f,0.014298f,0.025043f,-0.003579f,0.028824f,-0.026238f,-0.049329f,-0.066012f,-0.014636f,-0.0285f,-0.0070079f,-0.01542f,-0.045435f,-0.050038f,-0.042257f,-0.021226f,-0.0058578f,-0.028554f,-0.0078462f,-0.041138f,-0.034199f,-0.046714f,-0.040901f,-0.047206f,-0.024792f,-0.015872f,-0.031893f,-0.035835f,-0.05242f,-0.045549f,-0.016859f,-0.010454f,-0.010107f,-0.0032199f,0.0026406f,-0.013562f,-0.026309f,-0.05203f,-0.075121f,-0.072672f,-0.014743f,0.0029609f,0.05097f,0.019103f,-0.022601f,-0.059298f,-0.04073f,-0.056986f,-0.046562f,-0.042533f,0.016604f,0.054094f,0.067905f,0.037832f,0.02919f,-0.024374f,0.0034944f,-0.00028145f,0.030908f,-0.0071345f,-0.0065505f,0.016813f,0.052771f,0.046678f,0.032661f,0.019152f,-0.027657f,-0.034537f,-0.056091f,-0.037679f,-0.019933f,-0.0096505f,-0.032534f,-0.033938f,-0.027217f,-0.036689f,0.020647f,-0.015601f,-0.042946f,-0.045138f,-0.05645f,-0.081897f,-0.074032f,-0.12742f,-0.18107f},
    {0.8054f,-0.14808f,-0.0989f,-0.05787f,-0.035569f,0.0054747f,0.0010578f,0.009326f,0.0063448f,-0.02659f,-0.033045f,-0.066039f,-0.032805f,-0.033659f,-0.017375f,0.021339f,0.04462f,0.040605f,0.022552f,0.052306f,0.040153f,0.035486f,-0.0045585f,0.011669f,0.023518f,0.026006f,0.048972f,0.041396f,0.016578f,0.01987f,0.0011569f,0.0023468f,-0.0019198f,0.020336f,0.016112f,0.015421f,-0.022815f,-0.04573f,-0.077597f,-0.038928f,-0.020249f,0.0053616f,0.028053f,0.0028479f,-0.023584f,-0.03821f,-0.069169f,-0.04711f,-0.024296f,0.0041317f,0.013283f,0.010545f,-0.011399f,-0.025736f,-0.054564f,0.00096783f,-0.015805f,-0.019933f,-0.042282f,-0.032558f,0.0070783f,0.054566f,0.047545f,0.020921f,-0.024727f,-0.017691f,-0.022509f,0.015216f,0.030044f,0.058141f,0.041552f,0.042628f,0.041259f,0.061804f,0.0023505f,0.03271f,-0.01561f,0.0013756f,-0.0077659f,-0.01492f,0.0152f,0.0053845f,-0.018466f,0.026868f,0.010249f,-0.0099004f,-0.050843f,-0.059365f,-0.040517f,-0.0078577f,-0.027844f,-0.030514f,-0.0057151f,0.024199f,0.019075f,0.03283f,0.021296f,0.04933f,0.030666f,0.030718f,0.0033458f,-0.01755f,-0.038624f,-0.016826f,-0.08652f,-0.047287f,-0.068622f,-0.053004f,-0.03104f,-0.019542f,-0.022518f,-0.010752f,-0.019364f,0.0096527f,0.027477f,0.026543f,-0.0093301f,-0.0020619f,-0.040696f,-0.027483f,-0.0065841f,0.011882f,-0.013558f,0.004063f,-0.029866f,-0.047547f,-0.039456f,-0.0068826f,-0.015275f,-0.0074869f,0.017253f,0.058441f,0.045094f,0.044913f,0.041712f,0.010404f,-0.0070501f,0.014139f,0.016059f,0.013024f,0.022448f,0.016203f,0.00047533f,0.033904f,0.036642f,0.02853f,8.3096e-006f,-0.015632f,-0.044797f,-0.020974f,-0.012195f,-0.0091321f,0.027236f,-0.011162f,0.011931f,0.031653f,0.019038f,0.024451f,0.011348f,-0.001874f,-0.00086289f,-0.01506f,-0.019888f,-0.0014004f,0.032671f,0.078731f,0.062992f,0.052267f,0.064183f,0.090488f,0.062569f,0.02801f,-0.023225f,-0.037382f,-0.022299f,-0.029284f,-0.045696f,0.016551f,0.024698f,0.049357f,0.063007f,0.027576f,0.032001f,-0.022761f,-0.016177f,0.018796f,0.025579f,0.025253f,0.018254f,0.019283f,0.013224f,0.019567f,0.033262f,0.030885f,0.016367f,0.029617f,0.057145f,0.10131f,0.089886f,0.067799f,0.030027f,-0.00020649f,0.01877f,0.00056014f,-0.016867f,-0.01707f,-0.039549f,-0.033975f,-0.048276f,-0.021649f,-0.0097963f,-0.0087072f,-0.020518f,-0.021481f,-0.056506f,-0.014354f,0.00047817f,0.036474f,0.051487f,0.030595f,-0.0060386f,-0.027199f,-0.038023f,-0.01793f,-0.0034276f,0.012022f,-0.0093446f,-0.015274f,-0.039222f,-0.038727f,-0.045471f,0.0037715f,0.008083f,0.034362f,0.030237f,-9.116e-005f,-0.027302f,-0.015845f,0.0034944f,0.0046804f,-0.019682f,0.01109f,0.017132f,0.041026f,0.023995f,0.075016f,0.031528f,0.043821f,-0.012867f,-0.0026672f,-0.020221f,-0.025669f,-0.032248f,-0.064318f,-0.13384f,-0.1594f},
    {0.79174f,-0.20035f,-0.15839f,-0.083897f,-0.017319f,0.012596f,-0.042066f,-0.03412f,-0.066523f,-0.045697f,-0.031295f,0.00099028f,0.018768f,0.050696f,0.014993f,0.039057f,-0.0026739f,-0.025261f,-0.022196f,-0.014871f,-0.028108f,-0.0024255f,0.030795f,0.025252f,0.034132f,0.01965f,-0.010758f,-0.022521f,-0.024719f,-0.014496f,0.0065197f,0.0013951f,0.028592f,0.03176f,0.022165f,0.0029103f,0.009862f,0.018464f,0.0074827f,0.031719f,0.0073895f,0.017602f,0.00083025f,-0.0047421f,0.001672f,0.031843f,0.026468f,0.041279f,0.024745f,0.073471f,0.01253f,0.019127f,-0.025855f,-0.0031625f,0.010813f,0.022391f,0.0094218f,0.0051084f,0.0004692f,0.017111f,0.0070158f,0.025981f,-0.0033075f,0.042615f,0.026133f,0.013409f,-0.025963f,-0.043036f,-0.048462f,-0.03319f,-0.036965f,-0.037521f,-0.021365f,0.00067454f,0.015326f,0.014503f,0.021213f,0.00039371f,0.0083013f,0.02493f,0.010722f,-0.017976f,-0.057383f,-0.028468f,-0.039495f,-0.050969f,-0.03352f,-0.028644f,-0.031615f,-0.028976f,-0.012934f,-0.047923f,-0.025824f,-0.023421f,-0.041731f,-0.024849f,-0.010178f,-0.036991f,0.010497f,0.0073588f,0.0014928f,-0.021704f,-0.064603f,-0.049748f,-0.047389f,0.0060493f,0.028848f,0.03004f,0.012402f,-0.013811f,-0.054319f,-0.034035f,-0.054644f,-0.017382f,-0.019894f,0.0085278f,0.031243f,-0.0049168f,-0.029092f,-0.025413f,-0.038203f,-0.012239f,-0.013979f,-0.030453f,-0.0030706f,0.036729f,0.063575f,0.054078f,-0.00018055f,-0.038964f,-0.063173f,-0.027781f,-0.019838f,0.008106f,-0.023361f,0.010759f,0.03795f,0.051953f,0.02704f,-0.0051356f,-0.078427f,-0.048505f,-0.038205f,-0.017638f,0.0086724f,0.0059533f,0.0099119f,-0.020212f,-0.059584f,-0.062994f,-0.072003f,-0.023947f,0.0041517f,0.017853f,0.016752f,-0.0065156f,-0.023395f,0.0054112f,-0.0018037f,0.064588f,0.016267f,0.045349f,0.042374f,0.031485f,0.003625f,0.021295f,-0.048445f,-0.032323f,-0.048159f,-0.03213f,-0.037423f,-0.00038295f,-0.025744f,-0.020113f,0.005665f,0.0050946f,-0.013324f,0.011012f,0.025926f,0.051047f,0.015519f,0.032769f,0.015181f,0.043914f,0.047956f,0.080985f,0.062085f,0.063986f,0.046848f,0.04061f,-0.0024954f,-0.034968f,-0.067971f,-0.076286f,-0.045234f,-0.052655f,-0.039142f,-0.029656f,-0.015642f,0.0084037f,0.0044982f,0.017133f,0.0037724f,0.016296f,0.009917f,-0.02606f,-0.034964f,-0.078171f,-0.048829f,-0.054063f,-0.050669f,-0.023592f,0.006954f,0.042933f,0.035959f,0.044758f,0.048449f,0.035427f,0.036678f,0.034682f,0.025135f,0.047637f,0.018744f,0.020214f,0.0052526f,0.045764f,0.015092f,0.036974f,0.02083f,0.020651f,0.007908f,0.021166f,0.02309f,0.055488f,0.019407f,0.029047f,0.00093834f,-0.020265f,-0.038981f,-0.027105f,-0.051174f,-0.062632f,-0.028471f,-0.041729f,-0.03412f,-0.018344f,-0.018456f,0.018005f,0.028173f,0.02267f,0.0097259f,-0.038535f,-0.058573f,-0.050951f,-0.099214f,-0.1305f},
    {0.8248f,-0.11886f,-0.10238f,-0.005511f,-0.0094526f,-0.01252f,-0.062446f,-0.036317f,-0.031216f,-0.0079345f,-0.0010068f,-0.0040942f,-0.0176f,0.005108f,-0.035751f,-0.0056831f,-0.037478f,-0.022655f,0.0050572f,0.010501f,0.0082623f,0.033193f,0.012693f,-0.0069724f,-0.023495f,-0.00086056f,0.0085225f,-0.031627f,-0.0073421f,-0.03509f,-0.0075856f,-0.017438f,-0.0077023f,-0.0042726f,-0.011199f,0.0071414f,0.028707f,0.031531f,0.0073649f,0.0019415f,0.0092754f,0.029186f,0.034119f,0.04087f,0.035057f,0.034512f,0.033179f,-0.014076f,-0.033289f,-0.012938f,-0.017823f,0.034347f,0.030187f,0.056035f,0.035986f,0.025058f,0.056906f,0.050581f,0.031272f,0.012949f,-0.00076369f,-0.001846f,-0.0086961f,0.012649f,-0.027477f,-0.015001f,-0.031144f,-0.0021148f,-0.0047475f,0.00256f,-0.016367f,-0.013205f,-0.035126f,-0.020562f,-0.058489f,-0.045757f,-0.019222f,0.024529f,-0.010549f,-0.0074812f,0.010359f,0.017413f,0.037932f,0.058097f,0.021145f,-0.019163f,-0.035285f,-0.0070987f,-0.0033722f,0.053821f,0.038567f,0.051952f,0.017337f,0.0045746f,-0.0053641f,0.00051955f,0.0060025f,0.014688f,-0.012961f,-0.0050299f,-0.0091541f,0.0074566f,0.037859f,0.029004f,-0.015575f,-0.012296f,-0.023556f,0.011265f,0.0094842f,0.0041195f,0.0095201f,-0.0018381f,0.0115f,0.025584f,0.0079697f,0.017859f,0.013525f,-0.0035405f,-0.03626f,-0.025251f,-0.017277f,0.036914f,0.0086844f,-0.00047194f,-0.063631f,-0.10473f,-0.11883f,-0.078363f,-0.041525f,0.0080552f,0.043568f,0.059421f,0.018542f,-0.0055758f,-0.02049f,-0.031229f,0.0031087f,0.021922f,0.019166f,0.014351f,-0.01138f,-0.00044854f,-0.012111f,-0.022679f,-0.031519f,-0.0011904f,-0.025224f,-0.0051608f,-0.033804f,-0.021261f,-0.012398f,-0.011588f,-0.034554f,-0.072243f,-0.10318f,-0.056951f,-0.03448f,0.0090672f,0.025448f,0.020498f,-0.0038435f,-0.0088106f,-0.033225f,-0.022474f,-0.025481f,-0.016092f,0.0089888f,0.018382f,0.056999f,0.065807f,0.071237f,0.064941f,0.013599f,-0.00051338f,-0.0014235f,0.030284f,0.0081773f,0.027801f,0.0010925f,-0.045215f,-0.039036f,-0.0034844f,0.013596f,0.019923f,0.038827f,0.024951f,-0.019273f,-0.046275f,-0.056756f,-0.024736f,0.016176f,0.026559f,0.033344f,0.0036441f,0.0007727f,0.0058396f,-0.004176f,0.0054859f,-0.018141f,-0.021403f,-0.054173f,-0.018904f,-0.045141f,-0.036801f,-0.010112f,0.0082698f,0.036331f,0.039043f,0.031579f,0.033041f,-0.012112f,0.016146f,0.014773f,0.021506f,-0.0031089f,-0.011271f,-0.041344f,-0.045647f,-0.079094f,-0.087035f,-0.089378f,-0.047935f,-0.042366f,-0.033674f,-0.046932f,-0.016646f,-0.039341f,0.0028627f,-0.055314f,-0.0281f,-0.077421f,-0.033711f,0.0002056f,0.016002f,-0.011953f,-0.034973f,-0.032211f,-0.017837f,-0.019076f,0.027802f,0.029975f,0.011673f,0.030627f,0.0094022f,0.009884f,-0.0077068f,-0.053386f,-0.047077f,-0.052421f,-0.030892f,0.0011077f,-0.014211f,-0.034359f,-0.073278f,-0.08917f,-0.14666f},
    {0.77907f,-0.22648f,-0.13399f,-0.089746f,-0.034828f,-0.00010018f,-0.00022848f,0.0015276f,0.0092566f,0.025546f,0.0066588f,0.011542f,-0.029266f,-0.013164f,-0.026372f,-0.014381f,0.035196f,0.071871f,0.057802f,0.0083413f,-0.029341f,-0.017729f,-0.036098f,0.013303f,0.030921f,0.038532f,-0.0048751f,0.032757f,0.039158f,0.009631f,-0.0037194f,0.022041f,0.044233f,0.045709f,0.031798f,0.007144f,-0.042113f,-0.044332f,-0.032357f,0.018522f,0.029297f,0.026198f,-0.028416f,-0.043411f,-0.074782f,-0.020511f,-0.0136f,0.041223f,0.026521f,0.04395f,0.023334f,0.012737f,-0.0008268f,-0.016189f,-0.022315f,-0.010749f,-0.022737f,-0.0019663f,0.0045376f,0.0089637f,-0.016116f,0.0074777f,0.010687f,0.0083979f,-0.0065965f,-0.013151f,-0.020431f,-0.0021928f,-0.0039405f,-0.0067832f,-0.0070468f,0.00016348f,-0.0014811f,0.016351f,0.022653f,0.020161f,-0.0055458f,-0.026271f,-0.041057f,0.0092166f,-0.024828f,-0.02477f,-0.032142f,-0.0059682f,-0.016206f,-0.009314f,-0.023429f,-0.0036308f,0.027807f,-0.0052184f,-0.0037953f,-0.0048487f,0.003469f,-0.012268f,-0.0014258f,0.020101f,0.037732f,0.068704f,0.057278f,0.058962f,0.015698f,-0.0092044f,0.0019169f,0.045163f,0.033525f,0.045048f,0.0096381f,-0.03324f,-0.025948f,-0.0021249f,0.0014159f,-0.0034476f,-0.045016f,-0.042212f,-0.026349f,0.017515f,0.024543f,0.015294f,0.0089015f,-0.026407f,-0.029537f,0.008916f,-0.0054074f,0.040162f,0.029977f,0.01717f,0.010594f,0.0047683f,0.019181f,0.039433f,-0.0017365f,-0.0043561f,-0.026024f,-0.00072963f,0.032918f,0.020745f,0.030922f,0.031201f,-0.024621f,-0.0084194f,-0.033741f,-0.010957f,0.013211f,0.043216f,0.028328f,0.01011f,-0.0058281f,0.017139f,0.029148f,0.049926f,-0.0152f,-0.046808f,-0.064433f,-0.075217f,-0.059241f,-0.042135f,-0.088605f,-0.10311f,-0.099272f,-0.069428f,-0.021795f,0.032661f,0.049239f,0.070748f,0.030487f,0.038574f,0.015989f,0.02563f,-0.031605f,0.0057234f,-0.0035734f,-0.0055922f,-0.01685f,-0.0023412f,0.013418f,0.024411f,0.00057167f,-0.043674f,-0.029348f,-0.028704f,-0.030876f,0.0096617f,0.040087f,0.081863f,0.065142f,0.040718f,0.0084713f,0.015406f,-0.0018022f,0.0034508f,-0.0040952f,-0.006443f,-0.028377f,-0.01905f,-0.027696f,-0.0077144f,-0.0046634f,-0.011985f,-0.019975f,0.0076713f,0.0093351f,-0.00090625f,-0.020894f,-0.025488f,-0.065303f,-0.065574f,-0.084914f,-0.081847f,-0.08213f,-0.094523f,-0.056769f,-0.031683f,0.017858f,0.011853f,-0.011994f,-0.057397f,-0.079952f,-0.050287f,-0.00042882f,0.04827f,0.044286f,0.018304f,-0.028101f,-0.033383f,-0.051993f,-0.015574f,0.0035359f,0.019476f,-0.0068863f,0.025699f,0.03801f,-0.0040596f,-0.039803f,-0.031564f,-0.019744f,-0.037248f,-0.036371f,-0.046013f,-0.090392f,-0.053307f,-0.0089966f,0.064547f,0.058344f,0.053514f,0.048328f,0.027369f,0.023793f,0.0038995f,-0.017972f,-0.013325f,-0.017727f,0.011816f,0.019119f,0.013372f,-0.088259f,-0.11767f},
    {0.74305f,-0.23445f,-0.20726f,-0.1324f,-0.048898f,-0.02986f,-0.026302f,0.010254f,0.012821f,-0.0002575f,-0.046036f,-0.070234f,-0.029134f,0.010517f,0.028422f,0.0065973f,-0.012231f,-0.016396f,-0.00081824f,0.033765f,0.022149f,0.019492f,-0.0068527f,-0.027241f,-0.017843f,0.0074208f,0.030601f,0.027354f,-0.0034778f,-0.019862f,7.2379e-005f,0.019647f,0.029226f,0.024179f,-0.0027421f,0.027556f,0.036013f,0.020571f,0.01287f,0.019893f,0.013891f,0.037167f,0.053667f,0.059285f,0.013044f,0.072567f,0.054418f,0.035305f,0.01928f,0.015654f,0.020199f,0.032563f,0.039245f,0.067059f,0.028577f,0.0078689f,-0.0086552f,-0.036468f,-0.043591f,-0.039408f,-0.030205f,-0.0091845f,0.0015719f,-0.001398f,-0.00032411f,0.016746f,0.044768f,0.036519f,0.015187f,-0.031927f,-0.052255f,-0.00061479f,-0.0067575f,0.06227f,0.023115f,0.0081942f,-0.044912f,-0.056885f,-0.05244f,-0.028353f,-0.011799f,0.003665f,0.0047618f,0.022566f,0.014437f,0.045322f,0.0073587f,0.024244f,-0.010925f,-0.025957f,-0.032733f,-0.034116f,-0.023928f,-0.0090333f,-0.011411f,0.021224f,0.014973f,0.0065085f,-0.0099653f,-0.025348f,0.0010126f,0.0064429f,0.011266f,0.037897f,0.041437f,0.060839f,0.059444f,0.035565f,-0.0090017f,-0.015611f,-0.017419f,0.031695f,0.056892f,0.065299f,0.016828f,-0.0099602f,-0.053806f,0.0040507f,0.0083481f,0.00053708f,-0.021912f,-0.022195f,-0.026866f,-0.02121f,0.0087221f,-0.010571f,-0.03568f,-0.0064646f,-0.00037526f,0.015403f,0.021526f,-0.0013639f,-0.049803f,-0.0086232f,-0.0057799f,0.034777f,0.051881f,0.074231f,0.052884f,0.052776f,0.081433f,0.055077f,0.030249f,0.019195f,0.014123f,0.041741f,0.052869f,0.051587f,0.026206f,0.004487f,-0.032894f,-0.018951f,-0.010356f,0.015871f,0.027846f,0.037898f,0.0025433f,0.017706f,-0.023909f,-0.018073f,-0.021671f,-0.0017947f,0.010436f,0.0036362f,-0.025262f,-0.016945f,-0.028011f,-0.0017508f,-0.012694f,-0.014046f,-0.0025009f,-0.018989f,-0.0045144f,0.008787f,0.013744f,0.041577f,0.041252f,0.0572f,0.052434f,0.061203f,0.031989f,0.0067498f,-0.011095f,-0.027443f,0.015311f,0.0027961f,0.011858f,-0.016501f,-0.082736f,-0.082954f,-0.079589f,-0.0021174f,0.031917f,0.069287f,0.043068f,0.043397f,0.01802f,0.014938f,-0.0080982f,-0.0009772f,-0.037038f,-0.025524f,-0.040343f,-0.041832f,-0.003431f,0.035328f,0.03073f,0.028482f,-0.01515f,-0.031858f,-0.07038f,-0.075133f,-0.034701f,-0.070329f,-0.095589f,-0.063218f,-0.042553f,-0.005559f,0.021781f,0.069675f,0.064111f,0.083119f,0.07654f,0.035414f,-0.012156f,-0.0017394f,-0.013801f,0.010824f,-0.025202f,-0.039497f,-0.051245f,-0.0068026f,0.0074656f,0.035139f,0.0040271f,-0.0063418f,-0.025193f,-0.022735f,-0.0021628f,0.032877f,0.017907f,0.0030797f,-0.049664f,-0.034339f,-0.014914f,0.051808f,0.021017f,-0.0069023f,-0.051716f,-0.056627f,-0.036339f,-0.0061821f,-0.013358f,0.0053462f,-0.076609f,-0.17935f},
    {0.78504f,-0.1486f,-0.10061f,-0.0060171f,0.013128f,0.046119f,0.019664f,0.03116f,-0.030657f,-0.00793f,-0.034519f,-0.068765f,-0.055634f,-0.030629f,-0.017496f,0.0082891f,-0.0040471f,-0.0062302f,-0.017741f,-0.038789f,-0.031599f,-0.033078f,-0.0023468f,0.010455f,0.011775f,0.023984f,0.029345f,0.023526f,0.0066192f,-0.0027394f,-0.050366f,-0.028226f,-0.019194f,0.036521f,-0.00070911f,0.0039215f,-0.041985f,-0.0077968f,-0.0013644f,-0.004285f,-0.038436f,-0.018497f,0.0023538f,0.042758f,0.043771f,0.034377f,0.023161f,0.018933f,-0.0054571f,-0.0049134f,-0.055469f,-0.026959f,-0.046471f,-0.026154f,-0.05506f,-0.033832f,-0.016843f,-0.00165f,0.0051196f,-0.0046693f,-0.014525f,0.0017733f,0.031542f,0.032402f,0.0078213f,-0.005835f,-0.055886f,-0.062164f,-0.03613f,-0.041401f,-0.049217f,-0.056693f,-0.046302f,0.0072531f,0.020041f,0.012815f,-0.027329f,-0.044997f,-0.049723f,-0.026309f,-0.010257f,-0.016106f,-0.012991f,-0.024687f,0.014377f,0.034084f,0.040921f,0.049948f,0.028219f,0.026166f,-0.0043638f,-0.0038737f,-0.014186f,-0.0084155f,-0.019976f,0.0067537f,-0.0013496f,-0.0023289f,-0.0028587f,0.009695f,-0.0024375f,-0.011159f,-0.048731f,-0.088369f,-0.089656f,-0.050898f,-0.020755f,0.065916f,0.061179f,0.046102f,0.02707f,0.0027588f,0.025213f,0.036707f,0.015735f,-0.0064985f,-0.061909f,-0.064729f,-0.031319f,-0.035771f,-0.021588f,0.019091f,0.03146f,0.025294f,0.012313f,-0.00062448f,0.0059163f,-0.01346f,0.018124f,0.060152f,0.0092439f,0.015205f,-0.039251f,-0.056949f,-0.097416f,-0.070005f,-0.037285f,0.0023283f,-0.013691f,0.022737f,-0.01188f,-0.051378f,-0.065804f,-0.036995f,-0.0065591f,0.057873f,0.033028f,0.029715f,-0.012106f,-0.02422f,0.017728f,0.050064f,0.074401f,0.10836f,0.073897f,0.05609f,0.022284f,0.026136f,0.023367f,0.052391f,0.034236f,0.021078f,0.003099f,-0.012856f,-0.020706f,0.0078712f,0.01376f,0.0065983f,-0.012736f,-0.03023f,-0.052222f,-0.048043f,-0.050274f,0.0044755f,-0.0096803f,0.019157f,-0.00060181f,0.023297f,0.019674f,0.011853f,-0.028139f,-0.059706f,-0.074258f,-0.035981f,0.015239f,0.049053f,0.028657f,0.034348f,0.0078873f,0.028363f,-0.010196f,-0.04986f,-0.071619f,-0.057399f,-0.044241f,0.011846f,0.013493f,0.011323f,-0.026267f,-0.0097392f,-0.019377f,0.011148f,0.016348f,-0.0033631f,-0.0020001f,0.00031715f,0.031322f,0.02018f,0.029455f,0.029625f,0.007724f,0.0037735f,-0.035046f,-0.030367f,-0.02348f,6.1903e-005f,-0.0066887f,-0.030135f,-0.044732f,-0.019758f,0.01566f,-0.00071234f,-0.0070973f,-0.038729f,-0.0055767f,0.015956f,0.040533f,0.041157f,0.047316f,0.065123f,0.036858f,0.036696f,-0.0030652f,-0.0126f,-0.029759f,0.024345f,0.027499f,0.065874f,0.038119f,0.036038f,0.0085963f,0.005893f,0.017107f,0.049014f,0.062875f,0.080634f,0.034571f,0.020835f,0.029352f,-0.01356f,-0.0098109f,-0.012872f,-0.016274f,-0.060537f,-0.12159f,-0.1874f},
    {0.7997f,-0.12081f,-0.053996f,-0.022608f,-0.010412f,-0.0064406f,-0.033046f,0.00060715f,-0.0088388f,0.019027f,-0.01796f,-0.001739f,0.024302f,0.053628f,0.043863f,-0.010473f,-0.0093792f,0.02764f,0.011489f,0.020581f,-0.02627f,-0.029326f,-0.019533f,0.013378f,0.021809f,0.031093f,0.032589f,0.022103f,-0.021324f,0.0022537f,-0.03282f,-0.0031798f,0.014154f,0.060458f,0.078599f,0.054518f,0.012083f,0.044269f,0.022656f,0.047411f,0.075969f,0.044346f,0.0073504f,-0.01514f,-0.033963f,-0.062369f,-0.060356f,-0.066835f,-0.062142f,-0.058208f,-0.01675f,0.021998f,0.074213f,0.074298f,0.075453f,0.04087f,0.00020683f,-0.010535f,-0.018263f,-0.039428f,-0.029453f,-0.024596f,-0.021246f,0.010957f,0.0037951f,-0.013303f,-0.023928f,-0.0097664f,0.030548f,0.040961f,0.026606f,-0.010651f,-0.039442f,-0.05934f,-0.074404f,-0.031086f,-0.059704f,-0.047191f,-0.05289f,-0.029563f,-0.024929f,-0.03307f,-0.032098f,-0.030429f,-0.060214f,-0.060868f,-0.039018f,-0.036665f,-0.0086438f,0.011439f,0.0015078f,0.0099384f,-0.017176f,-0.030055f,0.0084964f,-0.017646f,-0.0064622f,0.027712f,0.02056f,0.0076477f,0.013861f,0.02132f,0.013797f,0.039664f,0.00070935f,0.025397f,-0.040571f,-0.036255f,-0.047493f,-0.027325f,-0.0049375f,0.011538f,0.001903f,0.022475f,-0.0049481f,-0.00071129f,0.043853f,0.023428f,0.01727f,0.0082728f,0.0024852f,0.012419f,0.0027447f,0.046531f,0.033796f,0.067344f,0.054583f,0.039661f,0.024598f,0.023037f,-0.012604f,-0.018926f,-0.0059033f,-0.0084461f,0.014453f,0.0050754f,-0.0074686f,-0.0071628f,-0.03663f,-0.035741f,-0.074454f,-0.015516f,-0.014642f,-0.021462f,-0.010632f,-0.011927f,-0.01298f,0.016228f,0.025303f,0.036372f,0.043374f,0.0044528f,0.013386f,-0.021996f,-0.030839f,-0.04368f,-0.068915f,-0.044642f,-0.061638f,-0.045003f,-0.016231f,0.022514f,-0.011288f,0.053985f,0.028375f,0.015319f,0.011275f,0.014912f,0.022487f,0.062681f,0.036936f,0.072853f,0.057379f,0.052024f,0.036634f,0.039977f,-0.0071964f,-0.011812f,-0.021928f,-0.013847f,-0.017195f,-0.022409f,0.017252f,0.027671f,0.0011002f,-0.0056798f,-0.057825f,-0.057196f,-0.06036f,0.004785f,0.01051f,0.031094f,0.0025109f,0.0019951f,0.0052143f,0.0083442f,-0.010783f,0.0042806f,-0.035088f,-0.022592f,-0.026728f,-0.013522f,-0.019606f,0.016929f,-0.0035499f,0.033627f,0.018379f,0.051132f,0.018939f,0.022788f,-0.01032f,0.0087717f,-0.013636f,0.01679f,0.011074f,0.0081707f,-0.024061f,0.031093f,0.046177f,0.04347f,0.057943f,0.017392f,-0.036835f,-0.0081822f,0.0090211f,0.033149f,0.038803f,0.014639f,0.029875f,0.014964f,-0.0086469f,-0.0050695f,-0.029661f,-0.028903f,-0.024147f,0.0060814f,0.00082022f,-0.0077607f,-0.00076569f,0.0072296f,0.031128f,0.043833f,-0.035473f,-0.044311f,-0.047345f,-0.012906f,0.0055091f,0.0092365f,0.039899f,0.062552f,0.076349f,0.048669f,-0.0073583f,-0.070834f,-0.17075f,-0.1886f},
    {0.81484f,-0.13325f,-0.039008f,-0.021934f,-0.0071875f,0.0063517f,0.0054291f,0.010645f,0.0039138f,-0.0047473f,-0.029787f,-0.022797f,-0.012797f,-0.018366f,-0.02067f,-0.039048f,-0.049391f,-0.00069028f,-0.00086527f,0.016987f,-0.0069105f,-0.0078739f,-0.010157f,-0.022384f,-0.0030869f,0.012569f,0.024916f,0.0025207f,0.025262f,0.022136f,0.027664f,0.021694f,0.025435f,0.038711f,0.045906f,0.0391f,0.015657f,-0.0055381f,-0.033947f,-0.0098887f,-0.013932f,0.030667f,-0.023346f,-0.026404f,-0.026493f,0.0067193f,-0.0014813f,0.03154f,0.043855f,0.036625f,-0.038019f,-0.047231f,-0.046215f,-0.039187f,-0.011919f,0.010929f,0.0010139f,-0.0038494f,-0.015814f,-0.0056305f,-0.036299f,-0.015448f,0.016274f,0.028996f,0.064635f,0.076886f,0.050637f,0.03392f,-0.013759f,-0.014342f,-0.04511f,-0.025428f,-0.018682f,0.013607f,0.035871f,0.048349f,0.023827f,0.041985f,0.032439f,0.041649f,0.012007f,-0.03341f,-0.025948f,-0.0089132f,-0.0075297f,0.024878f,-0.0051707f,0.01483f,-0.013649f,-0.038058f,-0.033666f,0.0101f,0.025935f,0.043601f,0.0069564f,-0.019232f,-0.057121f,-0.010152f,0.010974f,0.043072f,0.018084f,0.035379f,0.023096f,0.05468f,0.064293f,0.053348f,0.030308f,-0.0017243f,-0.0058783f,-0.01061f,-0.013168f,-0.0046187f,0.00063337f,0.0049872f,-0.029301f,-0.032941f,-0.030122f,-0.023145f,-0.00054652f,0.0089741f,0.0095158f,-0.02487f,-0.023654f,-0.034043f,-0.013891f,-0.014145f,-0.049054f,-0.026876f,-0.0050538f,0.031012f,0.032381f,0.012632f,0.0080837f,0.044075f,0.012879f,0.017808f,-0.047865f,-0.051252f,-0.072406f,-0.037836f,-0.034552f,0.013096f,0.032753f,0.041265f,0.045308f,0.063106f,0.052646f,0.054957f,0.031138f,0.054988f,0.025697f,0.034469f,0.013545f,0.028349f,0.012722f,-0.015301f,-0.037557f,-0.05889f,-0.049831f,-0.00090323f,0.034414f,0.024524f,-0.01051f,-0.02227f,-0.028611f,-0.0030018f,0.014035f,0.020978f,-0.033925f,-0.052248f,-0.037312f,-0.032071f,0.0088248f,0.028478f,0.01841f,0.011662f,-0.068471f,-0.071429f,-0.053735f,-0.023595f,-0.013689f,-0.033466f,-0.051507f,-0.034195f,-0.030695f,-0.016875f,-0.020402f,-0.045127f,-0.067281f,-0.086415f,-0.10029f,-0.093848f,-0.080432f,-0.023602f,-0.017293f,0.043635f,0.058947f,0.05038f,0.038687f,0.020774f,-0.0175f,-0.027862f,-0.027189f,0.01905f,0.019727f,0.054756f,0.044448f,0.0055765f,-0.015159f,-0.0074009f,-0.019013f,-0.028737f,-0.0086786f,0.018904f,0.0067971f,-0.029715f,-0.011054f,-0.0076887f,0.0090337f,0.010012f,-0.0015139f,0.011386f,0.0051486f,0.028848f,0.019126f,0.013433f,-0.030926f,-0.02826f,-0.05382f,-0.013966f,-0.042773f,0.010045f,0.031664f,0.077647f,0.048355f,0.038842f,0.00057889f,-0.015964f,-0.0061181f,0.024818f,0.07026f,0.066736f,0.019896f,0.0087986f,-0.02034f,0.0049433f,0.0040408f,0.0013826f,-0.028789f,-0.02183f,-0.020665f,0.0072022f,-0.0033038f,-0.033402f,-0.14741f,-0.1514f},
    {0.8277f,-0.11188f,-0.081963f,0.0085491f,0.017042f,0.027789f,-0.0090824f,-0.0032137f,-0.040197f,-0.041017f,-0.031373f,-0.0093639f,-0.015595f,0.014f,-0.0032016f,-0.01685f,-0.042115f,-0.062588f,-0.050887f,-0.02618f,-0.023156f,-0.0027388f,-0.033448f,-0.014984f,-0.017986f,-0.027727f,-0.043867f,-0.025943f,-0.0038396f,0.034546f,0.022112f,0.039251f,0.0056643f,0.023515f,-0.0019286f,0.045333f,0.041961f,0.0217f,-0.018185f,-0.027022f,-0.026852f,-0.0080794f,-0.0076089f,0.014386f,-0.010402f,0.022238f,-0.0030449f,0.0039044f,-0.028418f,-0.086714f,-0.10204f,-0.092465f,-0.11236f,-0.040992f,-0.027716f,0.01367f,0.034943f,0.018093f,-0.017428f,-0.042917f,-0.056954f,-0.035907f,-0.0067989f,-0.013252f,-0.015606f,0.0081923f,-0.011794f,0.005825f,-0.041509f,-0.02757f,-0.033781f,0.0032643f,-0.0083818f,0.0321f,0.012287f,0.0015818f,-0.024248f,0.0052023f,-0.0082866f,0.020262f,0.014231f,0.049369f,-0.011544f,0.0078887f,-0.01986f,-0.014748f,0.0046218f,0.0037547f,-0.0017573f,0.0031469f,0.0050929f,-0.015756f,-0.0064086f,-0.015139f,-0.018678f,0.0066256f,0.072758f,0.060488f,0.017885f,-0.0044382f,-0.022364f,-0.012749f,0.0055661f,0.018379f,0.019303f,0.010032f,-0.018727f,-0.033585f,-0.04816f,-0.0094869f,0.02484f,-0.00024726f,-0.017364f,-0.019867f,0.0074007f,0.035168f,0.037977f,0.048304f,0.011545f,0.018782f,0.021526f,0.018071f,0.014175f,-0.0022807f,-0.011414f,-0.0085295f,-0.0013637f,0.042806f,0.047209f,0.064913f,0.079208f,0.09265f,0.061666f,0.050074f,0.00086006f,8.2902e-005f,-0.023232f,0.0032173f,0.014787f,0.0063367f,-0.0043187f,-0.0059317f,-0.01913f,0.014742f,-0.00027495f,0.011347f,-0.0097168f,0.036377f,0.048684f,0.033111f,-0.00021187f,-0.017284f,-0.039567f,-0.036467f,-0.047828f,-0.015218f,-0.010685f,0.0058418f,-0.02278f,-0.059373f,-0.10197f,-0.026149f,-0.011268f,0.013107f,-0.0085896f,0.0252f,-0.0060466f,0.013637f,-0.005534f,-0.01442f,-0.050323f,-0.033679f,-0.047074f,-0.054406f,-0.034461f,-0.045217f,0.0093477f,0.041135f,0.053404f,0.031558f,0.014607f,-0.015518f,-0.039327f,-0.025429f,0.022453f,0.029958f,0.038229f,0.02594f,0.0098633f,-0.024115f,-0.007033f,0.0036471f,0.019245f,0.027899f,-0.0099651f,0.02593f,0.029095f,0.036885f,0.0088588f,-0.01615f,-0.053666f,-0.043304f,-0.039604f,-0.030411f,0.021223f,0.025459f,0.032449f,0.045469f,-0.016321f,-0.0183f,-0.01246f,-0.010269f,0.017042f,0.0081605f,0.019314f,0.03617f,0.0456f,0.061132f,0.025364f,-0.0077201f,-0.054046f,-0.047505f,-0.014582f,-0.020967f,-0.016993f,-0.042332f,-0.057396f,-0.06401f,-0.048821f,-0.011761f,-0.00090885f,0.016192f,0.0096809f,0.033312f,0.034308f,-0.00061374f,-0.023751f,-0.028371f,-0.025446f,0.010133f,-0.0031612f,0.007719f,-0.0081279f,-0.012935f,-0.019559f,-0.034938f,-0.053488f,-0.031315f,-0.039822f,-0.036336f,-0.021216f,-0.02089f,-0.037179f,-0.070973f,-0.10015f,-0.1496f}
  };

  memcpy(h_InteractConfig->diffuse_decorrFilters,decorrFilters,sizeof(decorrFilters));
  for (i = 0; i < MAE_MAX_NUM_SPEAKERS; i++)
  {
    for (j = 0; j < MAE_DIFFUSE_DECORR_LENGTH; j++)
    {
      h_InteractConfig->diffuse_decorrStates[i][j] = 0.0f;
    }
  }
  h_InteractConfig->diffuse_filterLength = MAE_DIFFUSE_DECORR_LENGTH;
  h_InteractConfig->diffuse_counter = 0;

  h_InteractConfig->diffuse_compensateDelay = 0;


  if (enhancedObjectMetadataConfig->hasDiffuseness)
  {
    diffuse_enableProcessing = 1;
  }
  h_InteractConfig->diffuse_enableProcessing = diffuse_enableProcessing;

  if (diffuse_enableProcessing == 1)
  {
    if (h_AudioConfig->decode_qmf)
    {
      error = wavIO_init(&h_AudioConfig->hWavIODiffuse_real, (unsigned int)h_AudioConfig->audio_blocksize, 0, 0);
      error = wavIO_init(&h_AudioConfig->hWavIODiffuse_imag, (unsigned int)h_AudioConfig->audio_blocksize, 0, 0);
      h_AudioConfig->fwavOut_diffuse_real = fopen(h_AudioConfig->wavIO_outpathDiffuse_real, "wb");
      h_AudioConfig->fwavOut_diffuse_imag = fopen(h_AudioConfig->wavIO_outpathDiffuse_imag, "wb");
      error = wavIO_openWrite(h_AudioConfig->hWavIODiffuse_real, h_AudioConfig->fwavOut_diffuse_real, h_AudioConfig->nSpeakers, h_AudioConfig->InSampleRate, h_AudioConfig->InBytedepth);
      error = wavIO_openWrite(h_AudioConfig->hWavIODiffuse_imag, h_AudioConfig->fwavOut_diffuse_imag, h_AudioConfig->nSpeakers, h_AudioConfig->InSampleRate, h_AudioConfig->InBytedepth);
    }
    if (h_AudioConfig->decode_wav)
    {
      error = wavIO_init(&h_AudioConfig->hWavIODiffuse, (unsigned int)h_AudioConfig->audio_blocksize, 0, 0);
      h_AudioConfig->fwavOut_diffuse = fopen(h_AudioConfig->wavIO_outpathDiffuse, "wb");
      error = wavIO_openWrite(h_AudioConfig->hWavIODiffuse, h_AudioConfig->fwavOut_diffuse, h_AudioConfig->nSpeakers, h_AudioConfig->InSampleRate, h_AudioConfig->InBytedepth);
    }

    h_InteractConfig->diffuse_compensateDelay += MAE_DIFFUSE_DECORR_LENGTH;


    if (decoderDomain == 1) /* if QMF domain processing: Get number of loudspeakers without LFE and initialize needed filter banks for synthesis and analysis */
    {
      int numLSout = 0;
      int numLSout_noLFE = 0;
      CICP2GEOMETRY_CHANNEL_GEOMETRY geoRep[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = {0};
      int nChannels;
      int nLFEs;

      if (localSetupConfig->rendering_type == 0)
      {
        numLSout = localSetupConfig->LoudspeakerRendering->numSpeakers;

        for (i = 0; i < numLSout; i++)
        {
          if ((localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 0) && (i == 0))
          {
            cicp2geometry_get_geometry_from_cicp(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
          }
          else if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 1)
          {
            cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[i],&geoRep[i]);
          }
          else if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
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
        if (geoRep[i].LFE == 0)
        {
          numLSout_noLFE ++;
        }
      }
      else
      {
        numLSout = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->nBrirPairs;
        for (i = 0; i < numLSout; i++)
        {
          if ((localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 0) && (i == 0))
          {
            cicp2geometry_get_geometry_from_cicp(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
          }
          else if (localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 1)
          {
            cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerIdx[i],&geoRep[i]);
          }
          else if (localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 2)
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
        if (geoRep[i].LFE == 0)
        {
          numLSout_noLFE++;
        }
      }

      h_InteractConfig->diffuse_QMFsynthesis = (QMFLIB_POLYPHASE_SYN_FILTERBANK **)calloc(numLSout_noLFE, sizeof (QMFLIB_POLYPHASE_SYN_FILTERBANK * ));

      if (NULL != h_InteractConfig->diffuse_QMFsynthesis) {
        QMFlib_InitSynFilterbank(QMFLIB_NUMBANDS, 0);
        for (i = 0; i < numLSout_noLFE; i++)
        {
          QMFlib_OpenSynFilterbank(&h_InteractConfig->diffuse_QMFsynthesis[i]);
        }
      } else {
        error = -1;
        fprintf(stderr, "Error during initialization of QMF synthesis filterbank.\n");
      }

      h_InteractConfig->diffuse_QMFanalysis = (QMFLIB_POLYPHASE_ANA_FILTERBANK**)calloc(numLSout_noLFE, sizeof (QMFLIB_POLYPHASE_ANA_FILTERBANK*));

      if (NULL != h_InteractConfig->diffuse_QMFanalysis) {
        QMFlib_InitAnaFilterbank(QMFLIB_NUMBANDS, 0);
        for (i = 0; i < numLSout_noLFE; i++)
        {
          QMFlib_OpenAnaFilterbank(&(h_InteractConfig->diffuse_QMFanalysis[i]));
        }
      } else {
        error = -2;
        fprintf(stderr, "Error during initialization of QMF analysis filterbank.\n");
      }

      h_InteractConfig->diffuse_compensateDelay += 320+257; /* QMF synthesis and analysis delay */
    }
  }
  return error;
}

static int MP_getObjectIndex(ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int maeID)
{
  int i, j;
  int ct = -1;
  int objIdx = -1;
  for (i = 0; i < (int)signalGroupConfig->numSignalGroups; i++)
  {
    for (j = 0; j < (int)signalGroupConfig->numberOfSignals[i]; j++)
    {
      ct++;
      if (signalGroupConfig->signalGroupType[i] == 1)
      {
        objIdx++;
      }
      if (ct == maeID)
      {
        return objIdx;
      }
    }
  }
  return objIdx;
}

int MP_createDiffusePart(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OAM_MP_CONFIG h_OamConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_AUDIO_MP_CONFIG h_AudioConfig, int isLastFrame, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame)
{
  int error = 0;
  int renderingType = localSetupConfig->rendering_type;
  int numSpeakers = 0;
  int numSpeakers_noLFE = 0;
  int i, j, k;
  float** diffuseBufferIn = NULL;
  float*** diffuseBufferIn_real = NULL;
  float*** diffuseBufferIn_imag = NULL;
  float** diffuseBufferOut = NULL;
  float*** diffuseBufferOut_real = NULL;
  float*** diffuseBufferOut_imag = NULL;
  float** diffuseBufferOut_decorr = NULL;
  float*** diffuseBufferOut_real_decorr = NULL;
  float*** diffuseBufferOut_imag_decorr = NULL;
  int ct = 0;
  int framesize = 0;
  int nTimeslots = 0;
  float OamAndDiffuseGain[MAE_MAX_NUM_ELEMENTS];
  int numObjects = 0;
  unsigned int nSamplesWrittenPerChannel = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoRep[CICP2GEOMETRY_MAX_LOUDSPEAKERS];

  int lastFrameSize = 0;
  int lastFrameNumTS = 0;

  if ((h_InteractConfig->diffuse_compensateDelay > 0) && (isLastFrame))
  {
    lastFrameSize = h_InteractConfig->diffuse_compensateDelay;
    lastFrameNumTS = lastFrameSize / QMFLIB_NUMBANDS;
  }

  if (renderingType == 0)
  {
    int nChannels;
    int nLFEs;
    numSpeakers = localSetupConfig->LoudspeakerRendering->numSpeakers;
    numSpeakers_noLFE = 0;

    for (i = 0; i < numSpeakers; i++)
    {
      if ((localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 0) && (i == 0))
      {
        cicp2geometry_get_geometry_from_cicp(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
      }
      else if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 1)
      {
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[i],&geoRep[i]);
      }
      else if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType == 2)
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
      if (geoRep[i].LFE == 0)
      {
        numSpeakers_noLFE ++;
      }
    }
  }
  else
  {
    int nChannels;
    int nLFEs;
    numSpeakers = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->nBrirPairs;
    for (i = 0; i < numSpeakers; i++)
    {
      if ((localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 0) && (i == 0))
      {
        cicp2geometry_get_geometry_from_cicp(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerLayoutIdx, geoRep, &nChannels, &nLFEs);
      }
      else if (localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 1)
      {
        cicp2geometry_get_geometry_from_cicp_loudspeaker_index(localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.CICPspeakerIdx[i],&geoRep[i]);
      }
      else if (localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d.speakerLayoutType == 2)
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
      if (geoRep[i].LFE == 0)
      {
        numSpeakers_noLFE++;
      }
    }
  }

  if (h_InteractConfig->diffuse_enableProcessing)
  {
    /* get numObjects */
    for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
    {
      if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1)
      {
        if (h_InteractConfig->divergence_ASImodified == 1)
        {
          ct += h_InteractConfig->origNumGroupMembers[i];
        }
        else
        {
          ct += audioSceneConfig->asi.groups[i].groupNumMembers;
        }
      }
    }
    numObjects = ct;

    h_AudioConfig->numTimeslots = h_AudioConfig->audio_blocksize / QMFLIB_NUMBANDS;
    if (!isLastFrame)
    {
      nTimeslots = (h_AudioConfig->samplesReadPerChannel / QMFLIB_NUMBANDS);
      framesize = h_AudioConfig->samplesReadPerChannel;
    }
    else
    {
      nTimeslots = (int)ceil(h_AudioConfig->samplesReadPerChannel * 1.0f / QMFLIB_NUMBANDS) + lastFrameNumTS;
      framesize = h_AudioConfig->samplesReadPerChannel + lastFrameSize;
    }

    /* init buffers */
    if (h_AudioConfig->decode_qmf == 1)
    {
      diffuseBufferIn_real = (float***)calloc(nTimeslots,sizeof(float*));
      diffuseBufferIn_imag = (float***)calloc(nTimeslots,sizeof(float*));
      for (i = 0; i < nTimeslots; i++)
      {
        diffuseBufferIn_real[i] = (float**)calloc(numObjects, sizeof(float*));
        diffuseBufferIn_imag[i] = (float**)calloc(numObjects, sizeof(float*));
        for (j = 0; j < numObjects; j++)
        {
          diffuseBufferIn_real[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
          diffuseBufferIn_imag[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
        }
      }
      diffuseBufferOut_real = (float***)calloc(nTimeslots,sizeof(float*));
      diffuseBufferOut_imag = (float***)calloc(nTimeslots,sizeof(float*));
      diffuseBufferOut_real_decorr = (float***)calloc(nTimeslots,sizeof(float*));
      diffuseBufferOut_imag_decorr = (float***)calloc(nTimeslots,sizeof(float*));
      for (i = 0; i < nTimeslots; i++)
      {
        diffuseBufferOut_real[i] = (float**)calloc(numSpeakers, sizeof(float*));
        diffuseBufferOut_imag[i] = (float**)calloc(numSpeakers, sizeof(float*));
        diffuseBufferOut_real_decorr[i] = (float**)calloc(numSpeakers, sizeof(float*));
        diffuseBufferOut_imag_decorr[i] = (float**)calloc(numSpeakers, sizeof(float*));
        for (j = 0; j < numSpeakers; j++)
        {
          diffuseBufferOut_real[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
          diffuseBufferOut_imag[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
          diffuseBufferOut_real_decorr[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
          diffuseBufferOut_imag_decorr[i][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
        }
      }
    }
    else if (h_AudioConfig->decode_wav == 1)
    {
      diffuseBufferIn = (float**)calloc(numObjects,sizeof(float*));
      for (i = 0; i < numObjects; i++)
      {
        diffuseBufferIn[i] = (float*)calloc(framesize, sizeof(float));
      }
    }

    diffuseBufferOut = (float**)calloc(numSpeakers,sizeof(float*));
    diffuseBufferOut_decorr = (float**)calloc(numSpeakers,sizeof(float*));
    for (i = 0; i < numSpeakers; i++)
    {
      diffuseBufferOut[i] = (float*)calloc(framesize, sizeof(float));
      diffuseBufferOut_decorr[i] = (float*)calloc(framesize, sizeof(float));
    }


    /* copy input signals, get oam gain and multiply with diffuse gain */
    ct = 0;
    for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
    {
      int numMembers = 0;
      if (h_InteractConfig->divergence_ASImodified == 1)
      {
        numMembers = h_InteractConfig->origNumGroupMembers[i];
      }
      else
      {
        numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      }
      if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1)
      {
        for (j = 0; j < numMembers; j++)
        {
          int maeID = -1;
          float gain = 0.0f;
          if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 1)
          {
            maeID = audioSceneConfig->asi.groups[i].startID + j;
          }
          else
          {
            maeID = audioSceneConfig->asi.groups[i].metaDataElementID[j];
          }
          for (k = 0; k < numObjects; k++)
          {
            if (maeID == h_InteractConfig->listOAM[k])
            {
              break;
            }
          }

          if (enhancedObjectMetadataConfig->hasCommonGroupDiffuseness)
          {
            gain = (float)sqrt(h_enhObjMdFrame->diffuseness[0]);
          }
          else
          {
            gain = (float)sqrt(h_enhObjMdFrame->diffuseness[MP_getObjectIndex(signalGroupConfig,maeID)]);
          }

          /* overall modified gain = oam modified gain * diffuse gain */
          OamAndDiffuseGain[ct] = ( h_InteractConfig->oamSampleModified->gain[k] * gain ) ;
          if (h_AudioConfig->decode_qmf == 1)
          {
            int ts;
            for (ts = 0; ts < nTimeslots; ts++)
            {
              for (k = 0; k < QMFLIB_NUMBANDS; k++)
              {
                if (ts < (int)(h_AudioConfig->samplesReadPerChannel / QMFLIB_NUMBANDS))
                {
                  diffuseBufferIn_real[ts][ct][k] = h_AudioConfig->audioQmfBuffer_real[ts][maeID][k];
                  diffuseBufferIn_imag[ts][ct][k] = h_AudioConfig->audioQmfBuffer_imag[ts][maeID][k];
                }
                else
                {
                  diffuseBufferIn_real[ts][ct][k] = 0.0f;
                  diffuseBufferIn_imag[ts][ct][k] = 0.0f;
                }
              }
            }
          }
          else if (h_AudioConfig->decode_wav == 1)
          {
            for (k = 0; k < framesize; k++)
            {
              if (k < (int)h_AudioConfig->samplesReadPerChannel)
              {
                diffuseBufferIn[ct][k] = h_AudioConfig->audioTdBuffer[maeID][k];
              }
              else
              {
                diffuseBufferIn[ct][k] = 0.0f;
              }
            }
          }
          ct++;
        }
      }
    }

    /* apply mofified OAM gain and diffuse gain */
    if (h_AudioConfig->decode_qmf == 1)
    {
      for (i = 0; i < nTimeslots; i++)
      {
        for (j = 0; j < numObjects; j++)
        {
          for (k = 0; k < QMFLIB_NUMBANDS; k++)
          {
            diffuseBufferIn_real[i][j][k] *= OamAndDiffuseGain[j];
            diffuseBufferIn_imag[i][j][k] *= OamAndDiffuseGain[j];
          }
        }
      }
    }
    else if (h_AudioConfig->decode_wav == 1)
    {
      for (i = 0; i < numObjects; i++)
      {
        for (k = 0; k < framesize; k++)
        {
            diffuseBufferIn[i][k] *= OamAndDiffuseGain[i];
        }
      }
    }

    /* send to speakers and decorrelate speaker signals */
    for (i = 0; i < numObjects; i++)
    {
      for (j = 0; j < numSpeakers; j++)
      {
        if (h_AudioConfig->decode_qmf == 1)
        {
          int ts;
          for (ts = 0; ts < nTimeslots; ts++)
          {
            for (k = 0; k < QMFLIB_NUMBANDS; k++)
            {
              if (geoRep[j].LFE == 0)
              {
                diffuseBufferOut_real[ts][j][k] += ( diffuseBufferIn_real[ts][i][k] * (float)(1.0f/sqrt((float)numSpeakers_noLFE)) );
                diffuseBufferOut_imag[ts][j][k] += ( diffuseBufferIn_imag[ts][i][k] * (float)(1.0f/sqrt((float)numSpeakers_noLFE)) );
              }
              else
              {
                diffuseBufferOut_real[ts][j][k] = 0.0f;
                diffuseBufferOut_imag[ts][j][k] = 0.0f;
              }
            }
          }
        }
        else if (h_AudioConfig->decode_wav == 1)
        {
          for (k = 0; k < framesize; k++)
          {
            if (geoRep[j].LFE == 0)
            {
              diffuseBufferOut[j][k] += ( diffuseBufferIn[i][k] * (float)(1.0f/sqrt((float)numSpeakers_noLFE)) );
            }
            else
            {
              diffuseBufferOut[j][k] = 0.0f;
            }

          }
        }
      }
    }
    /* decorrelate */
    for (j = 0; j < numSpeakers; j++)
    {
      if (h_AudioConfig->decode_qmf == 1)
      {
        int ts;
        int hopsize = QMFLIB_NUMBANDS;
        int m;
        float *temp = (float*)calloc(hopsize, sizeof(float));

        if (geoRep[j].LFE == 0)
        {
          for (ts = 0; ts < nTimeslots; ts++)
          {
            /* iqmf: from diffuseBufferOut_real and diffuseBufferOut_imag to diffuseBufferOut */
            QMFlib_CalculateSynFilterbank(h_InteractConfig->diffuse_QMFsynthesis[j],diffuseBufferOut_real[ts][j], diffuseBufferOut_imag[ts][j],temp, 0);
            for (m = 0; m < hopsize; m++)
	          {
		          diffuseBufferOut[j][ts*hopsize+m] = temp[m];
	          }
          }

          for (k = 0; k < framesize; k++)
          {
            diffuseBufferOut_decorr[j][k] = MP_getDiffuseDecorrOutputSample(diffuseBufferOut[j][k], j, h_InteractConfig);
          }

          for (ts = 0; ts < nTimeslots; ts++)
          {
            /* qmf: from diffuseBufferOut_decorr to diffuseBufferOut_real_decorr and diffuseBufferOut_imag_decorr*/
            for (m = 0; m < hopsize; m++)
	          {
		          temp[m] = diffuseBufferOut_decorr[j][ts*hopsize+m];
	          }
	          QMFlib_CalculateAnaFilterbank(h_InteractConfig->diffuse_QMFanalysis[j], temp, diffuseBufferOut_real_decorr[ts][j], diffuseBufferOut_imag_decorr[ts][j], 0);
          }
          free(temp);
        }
        else
        {
          for (ts = 0; ts < nTimeslots; ts++)
          {
            for (k = 0; k < QMFLIB_NUMBANDS; k++)
            {
              diffuseBufferOut_real_decorr[ts][j][k] = 0.0f;
              diffuseBufferOut_imag_decorr[ts][j][k] = 0.0f;
            }
          }
        }
      }
      else
      {
        for (k = 0; k < framesize; k++)
        {
          if (geoRep[j].LFE == 0)
          {
            diffuseBufferOut_decorr[j][k] = MP_getDiffuseDecorrOutputSample(diffuseBufferOut[j][k], j, h_InteractConfig);
          }
          else
          {
            diffuseBufferOut_decorr[j][k] = 0.0f;
          }
        }
      }
    }

    /* write output frame to file */
    if (h_AudioConfig->decode_qmf == 1)
    {
      int writeSamples = nTimeslots*QMFLIB_NUMBANDS;
      int start = 0;
      float** bufferTmp_real = (float**)calloc(numSpeakers,sizeof(float*));
      float** bufferTmp_imag = (float**)calloc(numSpeakers,sizeof(float*));
      for (j = 0; j < numSpeakers_noLFE; i++)
      {
        int cnt = 0;
        bufferTmp_real[j] = (float*)calloc(nTimeslots*QMFLIB_NUMBANDS, sizeof(float));
        bufferTmp_imag[j] = (float*)calloc(nTimeslots*QMFLIB_NUMBANDS, sizeof(float));
        for (k = 0; k < nTimeslots; k++)
        {
          for (i = 0; i < QMFLIB_NUMBANDS; i++)
          {
            bufferTmp_real[j][cnt] = diffuseBufferOut_real_decorr[k][j][i];
            bufferTmp_imag[j][cnt] = diffuseBufferOut_imag_decorr[k][j][i];
            cnt++;
          }
        }
      }

      if (nTimeslots*QMFLIB_NUMBANDS > h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS)
      {
        while (writeSamples > 0)
        {
          int writeSamples_tmp = writeSamples - h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS;
          if (writeSamples_tmp > 0)
          {
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse_real, bufferTmp_real, h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS, &nSamplesWrittenPerChannel, start);
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse_imag, bufferTmp_imag, h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS, &nSamplesWrittenPerChannel, start);
          }
          else
          {
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse_real, bufferTmp_real, writeSamples, &nSamplesWrittenPerChannel, start);
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse_imag, bufferTmp_imag, writeSamples, &nSamplesWrittenPerChannel, start);
          }
          writeSamples = writeSamples - h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS;
          start = start + nSamplesWrittenPerChannel;
        }
      }
      else
      {
        error = wavIO_writeFrame(h_AudioConfig->hWavIODiffuse_real, bufferTmp_real, h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS, &nSamplesWrittenPerChannel);
        error = wavIO_writeFrame(h_AudioConfig->hWavIODiffuse_imag, bufferTmp_imag, h_AudioConfig->numTimeslots*QMFLIB_NUMBANDS, &nSamplesWrittenPerChannel);
      }
      for (j = 0; j < numSpeakers; i++)
      {
        free(bufferTmp_real[j]); bufferTmp_real[j] = NULL;
        free(bufferTmp_imag[j]); bufferTmp_imag[j] = NULL;
      }
      free(bufferTmp_real); bufferTmp_real = NULL;
      free(bufferTmp_imag); bufferTmp_imag = NULL;
    }
    else if (h_AudioConfig->decode_wav == 1)
    {
      int writeSamples = framesize;
      int start = 0;
      if (framesize > h_AudioConfig->audio_blocksize)
      {
        while (writeSamples > 0)
        {
          int writeSamples_tmp = writeSamples - h_AudioConfig->audio_blocksize;
          if (writeSamples_tmp > 0)
          {
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse,diffuseBufferOut_decorr, h_AudioConfig->audio_blocksize, &nSamplesWrittenPerChannel, start);
          }
          else
          {
            error = wavIO_writeFrame_withOffset(h_AudioConfig->hWavIODiffuse,diffuseBufferOut_decorr, writeSamples, &nSamplesWrittenPerChannel, start);
          }
          writeSamples = writeSamples - h_AudioConfig->audio_blocksize;
          start = start + nSamplesWrittenPerChannel;
        }
      }
      else
      {
        error = wavIO_writeFrame(h_AudioConfig->hWavIODiffuse,diffuseBufferOut_decorr, framesize, &nSamplesWrittenPerChannel);
      }
    }

    /* free buffers */
    if (h_AudioConfig->decode_qmf == 1)
    {
      for (i = 0; i < nTimeslots; i++)
      {
        for (j = 0; j < numObjects; j++)
        {
          free(diffuseBufferIn_real[i][j]); diffuseBufferIn_real[i][j] = NULL;
	        free(diffuseBufferIn_imag[i][j]); diffuseBufferIn_imag[i][j] = NULL;
        }
        free(diffuseBufferIn_real[i]); diffuseBufferIn_real[i] = NULL;
        free(diffuseBufferIn_imag[i]); diffuseBufferIn_imag[i] = NULL;
      }
      free(diffuseBufferIn_real); diffuseBufferIn_real = NULL;
      free(diffuseBufferIn_imag); diffuseBufferIn_imag = NULL;

      for (i = 0; i < nTimeslots; i++)
      {
        for (j = 0; j < numSpeakers; j++)
        {
          free(diffuseBufferOut_real[i][j]); diffuseBufferOut_real[i][j] = NULL;
	        free(diffuseBufferOut_imag[i][j]); diffuseBufferOut_imag[i][j] = NULL;
          free(diffuseBufferOut_real_decorr[i][j]); diffuseBufferOut_real_decorr[i][j] = NULL;
	        free(diffuseBufferOut_imag_decorr[i][j]); diffuseBufferOut_imag_decorr[i][j] = NULL;
        }
        free(diffuseBufferOut_real[i]); diffuseBufferOut_real[i] = NULL;
        free(diffuseBufferOut_imag[i]); diffuseBufferOut_imag[i] = NULL;
        free(diffuseBufferOut_real_decorr[i]); diffuseBufferOut_real_decorr[i] = NULL;
        free(diffuseBufferOut_imag_decorr[i]); diffuseBufferOut_imag_decorr[i] = NULL;
      }
      free(diffuseBufferOut_real); diffuseBufferOut_real = NULL;
      free(diffuseBufferOut_imag); diffuseBufferOut_imag = NULL;
      free(diffuseBufferOut_real_decorr); diffuseBufferOut_real_decorr = NULL;
      free(diffuseBufferOut_imag_decorr); diffuseBufferOut_imag_decorr = NULL;
    }
    else if (h_AudioConfig->decode_wav == 1)
    {
      for (i = 0; i < numObjects; i++)
      {
        free(diffuseBufferIn[i]); diffuseBufferIn[i] = NULL;
        free(diffuseBufferOut[i]); diffuseBufferOut[i] = NULL;
        free(diffuseBufferOut_decorr[i]); diffuseBufferOut_decorr[i] = NULL;
      }
      free(diffuseBufferIn); diffuseBufferIn = NULL;
    }
    free(diffuseBufferOut); diffuseBufferOut = NULL;
    free(diffuseBufferOut_decorr); diffuseBufferOut_decorr = NULL;

    if (isLastFrame)
    {
      if (h_AudioConfig->decode_qmf == 1)
      {
        for (i = 0; i < numSpeakers; i++)
        {
          QMFlib_CloseAnaFilterbank(h_InteractConfig->diffuse_QMFanalysis[i]);
          QMFlib_CloseSynFilterbank(h_InteractConfig->diffuse_QMFsynthesis[i]);
        }
      }
    }
  }
  else
  {
    error = -2;
  }

  return error;
}

int MP_applyDirectGain(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int divergenceObjectsAdded)
{
  int error = 0;
  int i, j, k, ct, numObjects;

  /* get numObjects */
  ct = 0;
  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1)
    {
      if (h_InteractConfig->divergence_ASImodified == 1)
      {
        ct += h_InteractConfig->origNumGroupMembers[i];
      }
      else
      {
        ct += audioSceneConfig->asi.groups[i].groupNumMembers;
      }
    }
  }
  numObjects = ct;

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    int numMembers = 0;
    if (divergenceObjectsAdded > 0)
    {
      numMembers = h_InteractConfig->origNumGroupMembers[i];
    }
    else
    {
      numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
    }

    if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1)
    {
      for (j = 0; j < numMembers; j++)
      {
        int maeID = -1;
        float gain = 0.0f;
        float gain_temp = 0.0f;
        if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 1)
        {
          maeID = audioSceneConfig->asi.groups[i].startID + j;
        }
        else
        {
          maeID = audioSceneConfig->asi.groups[i].metaDataElementID[j];
        }
        for (k = 0; k < numObjects; k++)
        {
          if (maeID == h_InteractConfig->listOAM[k])
          {
            break;
          }
        }
        /* new modified gain = oam modified gain * direct gain */
        if (enhancedObjectMetadataConfig->hasCommonGroupDiffuseness)
        {
          gain_temp = (float)sqrt(h_enhObjMdFrame->diffuseness[0]);
        }
        else
        {
          gain_temp = (float)sqrt(h_enhObjMdFrame->diffuseness[MP_getObjectIndex(signalGroupConfig,maeID)]);
        }
        gain = (float)sqrt(1.0f - gain_temp);

        if (divergenceObjectsAdded > 0)
        {
          h_InteractConfig->oamSampleModified_Divergence->gain[k] *= gain;
        }
        else
        {
          h_InteractConfig->oamSampleModified->gain[k] *= gain;
        }
      }
    }
  }

  return error;
}

int MP_freeGroupSetupConfig(H_GROUP_SETUP_DATA groupSetupConfig)
{
  free(groupSetupConfig);
  return 0;
}

int MP_splitForExcludedSectors(char *inputAudio, char* inputOAM, H_GROUP_SETUP_DATA groupSetupConfig, H_AUDIO_MP_CONFIG h_AudioConfig, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_AUDIO_SCENE *audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG *signalGroupConfig)
{
  int numSeparateSetups = groupSetupConfig->numIndividualSetups;
  int i, j, k, m, n, p;
  int error = 0;
  int numElements = 0;
  int ctOut = 0;
  int frameNo = 0;

  unsigned int nInChannels = 0;
  unsigned int InSampleRate = 0;
  unsigned long nTotalSamplesPerChannel = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  unsigned int InBytedepth = 0;
  unsigned int nSamplesPerChannelFilled = 0;
  unsigned int nOutChannels = 0;
  unsigned int NumberSamplesWritten = 0;
  unsigned int isLastFrame = 0;

  FILE* fAudioIn;
  FILE* fAudioOut;
  WAVIO_HANDLE hWavIO = NULL;
  float **inBuffer = NULL;
  float **outBuffer = NULL;
  unsigned int wavio_blocklength = 4096;

  int **elementsPerSetup = (int**)calloc(numSeparateSetups,sizeof(int*));
  int *numElementsPerSetup = (int*)calloc(numSeparateSetups,sizeof(int));

  for (j = 0; j < audioSceneConfig->asi.numGroups; j++)
  {
    numElements += audioSceneConfig->asi.groups[j].groupNumMembers;
  }

  inputOAM[strlen(inputOAM) - 4]  = '\0';
  inputAudio[strlen(inputAudio) - 4]  = '\0';


  for (i = 0; i < numSeparateSetups; i++)
  {
    int setupID = i;
    int ct = 0;
    int maeID = 0;

    char oamInpath[FILENAME_MAX];
    char oamOutpath[FILENAME_MAX];
    char temp[FILENAME_MAX];
    char wavIO_inpath_real[FILENAME_MAX];
    char wavIO_inpath_imag[FILENAME_MAX];
    char wavIO_inpath[FILENAME_MAX];
    char wavIO_outpath_real[FILENAME_MAX];
    char wavIO_outpath_imag[FILENAME_MAX];
    char wavIO_outpath[FILENAME_MAX];

    strcpy(wavIO_inpath,inputAudio);
    strcpy(wavIO_inpath_real,inputAudio);
    strcpy(wavIO_inpath_imag,inputAudio);
    strcat(wavIO_inpath_real,"_real");
    strcat(wavIO_inpath_imag,"_imag");

    strcpy(oamInpath,inputOAM);

    sprintf (temp, "%s_setup_%d.oam", oamInpath, i);
    strcpy(oamOutpath,temp);
    strcat(oamInpath,".oam");

    if (h_AudioConfig->decode_qmf == 1)
    {
      sprintf (temp, "%s_setup_%d.qmf", wavIO_inpath_real, i);
      strcpy(wavIO_outpath_real,temp);
      sprintf (temp, "%s_setup_%d.qmf", wavIO_inpath_imag, i);
      strcpy(wavIO_outpath_imag,temp);
      strcat(wavIO_inpath_real,".qmf");
      strcat(wavIO_inpath_imag,".qmf");

    }
    else if (h_AudioConfig->decode_wav == 1)
    {
      sprintf (temp, "%s_setup_%d.wav", wavIO_inpath, i);
      strcpy(wavIO_outpath,temp);
      strcat(wavIO_inpath,".wav");
    }

    for (j = 0; j < (int)signalGroupConfig->numSignalGroups; j++)
    {
      if (groupSetupConfig->groupSetupID[j] == setupID)
      {
        numElementsPerSetup[i] += signalGroupConfig->numberOfSignals[j]; /*audioSceneConfig->asi.groups[j].groupNumMembers; */
      }
    }

    elementsPerSetup[i] = (int*)calloc(numElementsPerSetup[i],sizeof(int));

    for (j = 0; j < (int)signalGroupConfig->numSignalGroups; j++)
    {
      int numSignals = signalGroupConfig->numberOfSignals[j];

      if (groupSetupConfig->groupSetupID[j] == setupID)
      {
        for (k = 0; k < numSignals; k++)
        {
            elementsPerSetup[i][ct] = maeID;
            ct++;
            maeID++;
        }
      }
      else
      {
        maeID += numSignals;
      }
    }

    nOutChannels = numElementsPerSetup[i];

    if (h_AudioConfig->decode_wav == 1)
    {
      fAudioIn = fopen(wavIO_inpath,"rb");
      if (fAudioIn == NULL)
      {
        fprintf(stderr, "Excluded Sector Splitting: Could not open input file: %s.\n", wavIO_inpath);
        return -1;
      }
      fAudioOut = fopen(wavIO_outpath,"wb");
      if (fAudioOut == NULL)
      {
        fprintf(stderr, "Excluded Sector Splitting: Could not open output file: %s.\n", wavIO_outpath);
        return -1;
      }

      error = wavIO_init(&hWavIO,wavio_blocklength,0,0);
      error = wavIO_openRead(hWavIO,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,(int*)&nSamplesPerChannelFilled);
      error = wavIO_openWrite(hWavIO,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

      if (nInChannels)
      {
        inBuffer = (float**)calloc(nInChannels,sizeof(float*));
        for (m = 0; m < (int)nInChannels; m++) {
          inBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }
      }

      if (numElementsPerSetup[i])
      {
        outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
        for (m = 0; m < (int)nOutChannels; m++) {
          outBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }
      }

      frameNo = 0;
      isLastFrame = 0;

      do
      {
        unsigned int samplesReadPerChannel = 0;
        unsigned int samplesToWritePerChannel = 0;
        unsigned int samplesWrittenPerChannel = 0;
        unsigned int nZerosPaddedBeginning = 0;
        unsigned int nZerosPaddedEnd = 0;

        frameNo++;
        ctOut = 0;

        /* read frame if input file is available */
        if ( fAudioIn )
        {
          error = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
          for (n = 0; n < (int)nInChannels; n++)
          {
            for (p = 0; p < (int)nOutChannels; p++)
            {
              if (n == elementsPerSetup[i][p])
              {
                for (m = 0; m < (int)max((int)samplesReadPerChannel, wavio_blocklength); m++)
                {
                  outBuffer[ctOut][m] = inBuffer[n][m];
                }
                ctOut++;
              }
            }
          }

          /* Add up possible delay and actually read samples */
          samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
        }

        if ( fAudioOut )
        {
          /* write frame */
          error = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
          NumberSamplesWritten += samplesWrittenPerChannel;
        }
      }
      while (! isLastFrame);

      /* ----------------- EXIT WAV IO ----------------- */
      error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
      error = wavIO_close(hWavIO);
    }

    else if (h_AudioConfig->decode_qmf == 1)
    {
      FILE *fAudioInImag;
      FILE *fAudioOutImag;
      WAVIO_HANDLE hWavIOImag;

      fAudioIn = fopen(wavIO_inpath_real,"rb");
      fAudioInImag = fopen(wavIO_inpath_imag,"rb");

      if ((fAudioIn == NULL) || (fAudioInImag == NULL))
      {
        fprintf(stderr, "Excluded Sector Splitting: Could not open input file: %s.\n", wavIO_inpath);
        return -1;
      }

      fAudioOut = fopen(wavIO_outpath_real,"wb");
      fAudioOutImag = fopen(wavIO_outpath_imag,"wb");

      if ((fAudioOut == NULL) || (fAudioOutImag == NULL))
      {
        fprintf(stderr, "Excluded Sector Splitting: Could not open output file: %s.\n", wavIO_outpath);
        return -1;
      }

      error = wavIO_init(&hWavIO,wavio_blocklength,0,0);
      error = wavIO_init(&hWavIOImag,wavio_blocklength,0,0);

      error = wavIO_openRead(hWavIO,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,(int*)&nSamplesPerChannelFilled);
      error = wavIO_openWrite(hWavIO,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

      error = wavIO_openRead(hWavIOImag,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,(int*)&nSamplesPerChannelFilled);
      error = wavIO_openWrite(hWavIOImag,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

      if (nInChannels)
      {
        inBuffer = (float**)calloc(nInChannels,sizeof(float*));
        for (m = 0; m < (int)nInChannels; m++) {
          inBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }
      }

      if (numElementsPerSetup[i])
      {
        outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
        for (m = 0; m < (int)nOutChannels; m++) {
          outBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }
      }
      do
      {
        unsigned int samplesReadPerChannel = 0;
        unsigned int samplesToWritePerChannel = 0;
        unsigned int samplesWrittenPerChannel = 0;
        unsigned int nZerosPaddedBeginning = 0;
        unsigned int nZerosPaddedEnd = 0;

        frameNo++;
        ctOut = 0;

        /* read frame if input file is available */
        if ( fAudioIn )
        {
          error = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
          for (n = 0; n < (int)nInChannels; n++)
          {
            for (p = 0; p < (int)nOutChannels; p++)
            {
              if (n == elementsPerSetup[i][p])
              {
                for (m = 0; m < (int)max((int)samplesReadPerChannel, wavio_blocklength); m++)
                {
                  outBuffer[ctOut][m] = inBuffer[n][m];
                }
                ctOut++;
              }
            }
          }
          samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
        }

        if ( fAudioOut )
        {
          /* write frame */
          error = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
        }

        if ( fAudioInImag )
        {
          error = wavIO_readFrame(hWavIOImag,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
          for (n = 0; n < (int)nInChannels; n++)
          {
            for (p = 0; p < (int)nOutChannels; p++)
            {
              if (n == elementsPerSetup[i][p])
              {
                for (m = 0; m < (int)max((int)samplesReadPerChannel, wavio_blocklength); m++)
                {
                  outBuffer[ctOut][m] = inBuffer[n][m];
                }
                ctOut++;
              }
            }
          }
          samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
        }

        if ( fAudioOutImag )
        {
          /* write frame */
          error = wavIO_writeFrame(hWavIOImag,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);

        }
        NumberSamplesWritten += samplesWrittenPerChannel;

      }
      while (! isLastFrame);

      /* ----------------- EXIT WAV IO ----------------- */
      error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
      error = wavIO_close(hWavIO);

      error = wavIO_updateWavHeader(hWavIOImag, &nTotalSamplesWrittenPerChannel);
      error = wavIO_close(hWavIOImag);
    }

    for (k = 0; k < max((int)nOutChannels,(int)nInChannels); k++)
    {
      if ((k < (int)nInChannels) && (NULL != inBuffer[k]))
      {
        free(inBuffer[k]);
        inBuffer[k] = NULL;
      }
      if ((k < (int)nOutChannels) && (NULL != outBuffer[k]))
      {
        free(outBuffer[k]);
        outBuffer[k] = NULL;
      }
    }

    if (NULL != inBuffer) {
      free(inBuffer);
      inBuffer = NULL;
    }

    if (NULL != outBuffer) {
      free(outBuffer);
      outBuffer = NULL;
    }

    /* Split OAM File */
    {
      FILE *OAMIn = NULL;
      FILE *OAMOut = NULL;
      int needsOamFile = 0;
      int nOamIn = 0;
      int nOamOut = 0;

      for (k = 0; k < (int)signalGroupConfig->numSignalGroups; k++)
      {
        int setupIDtmp = groupSetupConfig->groupSetupID[k];
        if ((setupIDtmp == i) && (signalGroupConfig->signalGroupType[k] == 1))
        {
          needsOamFile = 1;
          break;
        }
      }

      if (needsOamFile == 1)
      {
        int* listOAM_temp;
        int oamCnt_tmp;
        int tmpi;
        int ctElements = 0;

        uint16_t hasDynObjectPrio;
        uint16_t hasUniformSpread = 1;
        uint16_t numObjects;
        uint16_t oam_version;

        StructOamMultidata* oamSample_in;
        StructOamMultidata* oamSample_out;

        listOAM_temp = (int*)calloc(MAE_MAX_NUM_OBJECTS, sizeof(int));
        oamCnt_tmp = 0;

        for (j = 0; j < (int)signalGroupConfig->numSignalGroups; j++)
        {
          int maeID_tmp = 0;
          if (signalGroupConfig->signalGroupType[j] == 1)
          {
            int numMembers = signalGroupConfig->numberOfSignals[j];
            for (k = 0; k < numMembers; k++)
            {
              listOAM_temp[oamCnt_tmp] = maeID_tmp;
              oamCnt_tmp++;
              maeID_tmp++;
            }
          }
          else
          {
            maeID_tmp += signalGroupConfig->numberOfSignals[j];
          }
        }

        for (k = 0; k < oamCnt_tmp; k++)
        {
          for (j = k + 1; j < oamCnt_tmp; j++)
          {
            if (listOAM_temp[k] > listOAM_temp[j])
            {
                tmpi =  listOAM_temp[k];
                listOAM_temp[k] = listOAM_temp[j];
                listOAM_temp[j] = tmpi;
            }
          }
        }

        /* initialization for OAM file reading */
        OAMIn = oam_read_open(oamInpath, &numObjects, &oam_version, &hasDynObjectPrio, &hasUniformSpread);
        nOamIn = numObjects;
        oamSample_in = oam_multidata_create(nOamIn, 1);

        /* initialization for OAM file writing */
        nOamOut = numElementsPerSetup[i];
        oamSample_out = oam_multidata_create(nOamOut,1);
        OAMOut = oam_write_open(oamOutpath, nOamOut, oam_version, hasDynObjectPrio, hasUniformSpread);

        do
        {
          MP_getOamSample(OAMIn, oamSample_in, oam_version, hasDynObjectPrio, hasUniformSpread);
          ctElements = 0;
          for (k = 0; k < nOamIn; k++)
          {
            int metadataId = listOAM_temp[k];
            for (j = 0; j < audioSceneConfig->asi.numGroups; j++)
            {
              int maeID_tmp;
              int setupIDtmp = groupSetupConfig->groupSetupID[j];
              if ((setupIDtmp == i) && (signalGroupConfig->signalGroupType[j] == 1))
              {
                int numMembers = signalGroupConfig->numberOfSignals[j];
                for (m = 0; m < numMembers; m++)
                {
                  int memberId = -1;
                  memberId = maeID_tmp;
                  maeID_tmp++;
                  if ((metadataId == memberId) && (ctElements < nOamOut))
                  {
                    oamSample_out->azimuth[ctElements] = oamSample_in->azimuth[k];
                    oamSample_out->dynamic_object_priority[ctElements] = oamSample_in->dynamic_object_priority[k];
                    oamSample_out->elevation[ctElements] = oamSample_in->elevation[k];
                    oamSample_out->gain[ctElements] = oamSample_in->gain[k];
                    oamSample_out->radius[ctElements] = oamSample_in->radius[k];
                    oamSample_out->sample[ctElements] = oamSample_in->sample[k];
                    oamSample_out->spread[ctElements] = oamSample_in->spread[k];
                    oamSample_out->spread_depth[ctElements] = oamSample_in->spread_depth[k];
                    oamSample_out->spread_height[ctElements] = oamSample_in->spread_height[k];
                    ctElements++;
                    oamSample_out->num_elements = ctElements;
                    oamSample_out->size1 = ctElements;
                  }
                }
              }
              else
              {
                maeID_tmp += signalGroupConfig->numberOfSignals[j];
              }
            }
          }
          oam_write_process(OAMOut, oamSample_out, oam_version, hasDynObjectPrio, hasUniformSpread);
        }
        while (oamSample_out->sample[ctElements-1] <= nTotalSamplesWrittenPerChannel);

        oam_read_close(OAMIn);
        oam_write_close(OAMOut);

        oam_multidata_destroy(oamSample_in); oamSample_in = NULL;
        oam_multidata_destroy(oamSample_out); oamSample_out = NULL;
        free(listOAM_temp); listOAM_temp = NULL;
      }
    }
  }

  free(numElementsPerSetup); numElementsPerSetup = NULL;
  for (i = 0; i < numSeparateSetups; i++)
  {
    free(elementsPerSetup[i]); elementsPerSetup[i] = NULL;
  }
  free(elementsPerSetup); elementsPerSetup = NULL;

  return error;
}

int MP_processFrame(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig, ELEMENT_INTERACTION_HANDLE h_EI_Dec, H_EI_BITSTREAM_DECODER h_bitstream_EI_Dec, SCENE_DISPLACEMENT_HANDLE h_SD_Dec, H_SD_BITSTREAM_DECODER h_bitstream_SD_Dec, H_GROUP_SETUP_DATA groupSetupConfig, int isLastFrame, int frameNo, int enableObjectOutputInterface, int chooseSGmember, int *selectedPresetID)
{
  /* read audio data */
  /* read OAM sample */
  /* read framewise metadata */
  /* read and apply interaction data */
  /* apply scene displacement data */
  /* write (modified) OAM sample */
  /* write (modified) audio data */

  unsigned long no_BitsRead;
  int i, k, j;
  int error;
  int selectedPreset = -1;
  int enhObjMdFramePresent = 0;
  int overallNumDivergenceObjectsAdded = 0;

  h_InteractConfig->localScreen = (H_LOCAL_SCREEN_INFO)calloc(1,sizeof(struct _LocalScreenInfo));

  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    h_InteractConfig->numObjects_in = h_OamConfig->oamSample_in->size1;
  }
  else
  {
    h_InteractConfig->numObjects_in = 0;
  }

  /**************** get framewise data (OAM, interaction)  *****************/

  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    /* get current OAM sample */
    MP_getOamSample(h_OamConfig->oamInFile, h_OamConfig->oamSample_in, h_OamConfig->oam_version, h_OamConfig->hasDynObjectPrio, h_OamConfig->hasUniformSpread);
  }

  /* get framewise metadata */
  error = MP_decodeEnhancedObjectMetadataFrame(h_enhObjMdFrame, h_OamConfig->num_objects_input, enhancedObjectMetadataConfig, frameNo, &enhObjMdFramePresent);

  /* get interaction data */
  h_InteractConfig->interactionDataToBeApplied = 0;
  if (h_bitstream_EI_Dec != NULL)
  {
    error = EI_readElementInteractionBitstream(h_EI_Dec, h_bitstream_EI_Dec, &no_BitsRead);
    if (error == 0)
    {
      error = EI_getElementInteractionData(h_EI_Dec, &h_InteractConfig->ElementInteractionData);
      h_InteractConfig->interactionDataToBeApplied = 1;

      h_InteractConfig->interactionType = h_InteractConfig->ElementInteractionData->interactionMode;

      if (h_InteractConfig->interactionType == 0)
      {
        if (audioSceneConfig->asi.numGroupPresets > 0)
        {
          fprintf(stderr, "Error: Advanced interaction mode is chosen, however the number of presets is bigger than 0. This is not allowed. Default preset is selected. \n");
          h_InteractConfig->interactionType = 1;
          /* ignore interaction data */
          h_InteractConfig->interactionDataToBeApplied = 0;
          /* use default preset */
          h_InteractConfig->defaultPresetMode = 1;
        }
        else
        {
          h_InteractConfig->defaultPresetMode = 0;
        }
      }
      else
      {
        if (audioSceneConfig->asi.numGroupPresets > 0)
        {
          h_InteractConfig->defaultPresetMode = 0;
        }
        else
        {
          h_InteractConfig->defaultPresetMode = 0;
          fprintf(stderr, "Error: Basic interaction mode is chosen, however the number of presets is not bigger than 0.\n");
          return -1;
        }
      }
    }
  }
  else
  {
    /* fprintf(stderr, "Information: No interaction data via element interaction interface available.\n"); */
    h_InteractConfig->interactionDataToBeApplied = 0;

    /* check for number of presets: if numPresets > 0 --> choose preset with ID=0 as default preset if no interaction data is available */
    if (selectedPresetID >=0) /* preset selection via commandline */
    {
      h_InteractConfig->defaultPresetMode = 0;
    }
    else
    {
      if (audioSceneConfig->asi.numGroupPresets > 0)
      {
        h_InteractConfig->defaultPresetMode = 1;
      }
      else
      {
        h_InteractConfig->defaultPresetMode = 0;
      }
    }
  }

  if ((h_InteractConfig->interactionDataToBeApplied == 1) && (h_InteractConfig->ElementInteractionData->numGroups != audioSceneConfig->asi.numGroups))
  {
    fprintf(stderr, "Error: Number of groups in ASI and interaction data is not the same.\n");
    return -1;
  }

  /*************** init of processing ********************/

  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    h_InteractConfig->oamSampleModified = oam_multidata_create(h_OamConfig->num_objects_input, 1);
    h_InteractConfig->oamSampleModified_Divergence = oam_multidata_create(h_OamConfig->num_objects_output, 1);
    h_InteractConfig->numObjects_out = h_OamConfig->num_objects_output;
  }

  h_InteractConfig->numDecodedGroups = audioSceneConfig->asi.numGroups;
  h_InteractConfig->numSwitchGroups = audioSceneConfig->asi.numSwitchGroups;
  h_InteractConfig->numElements_in = 0;

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    if (h_InteractConfig->divergence_ASImodified == 1)
    {
      h_InteractConfig->numElements_in += h_InteractConfig->origNumGroupMembers[i];
    }
    else
    {
      h_InteractConfig->numElements_in += audioSceneConfig->asi.groups[i].groupNumMembers;
    }
  }

  for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
  {
    h_InteractConfig->onOffStatusModified[i] = 0;
    h_InteractConfig->gainModifiedGroup[i] = 0.0f;
    h_InteractConfig->routeToWireID[i] = -1;
  }

  /****************** GET LOCAL SCREEN CONFIG **********************/
  h_InteractConfig->enableScreenRelatedProcessing = MP_getLocalScreenConfig(audioSceneConfig, localSetupConfig, h_InteractConfig->localScreen);

  /* define list of groups to process */
  h_InteractConfig->doProcess = (int*)calloc(h_InteractConfig->numDecodedGroups,sizeof(int));
  if (h_InteractConfig->interactionDataToBeApplied == 0)
  {
    if (h_InteractConfig->defaultPresetMode == 1)
    /* no interaction data to be applied and default preset mode is active */
    {
      /****************** APPLY ON-OFF INTERACTION **********************/
      error = MP_applyOnOffInteraction(h_InteractConfig, audioSceneConfig, localSetupConfig->dmxID, &selectedPreset);

      /****************** DEFINE GROUPS TO PROCESS **********************/
      for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
      {
        if (h_InteractConfig->onOffStatusModified[i] == 1)
        {
          h_InteractConfig->doProcess[i] = 1;
        }
      }
    }
    else
    /* no interaction data to be applied */
    {
      if (*selectedPresetID >= 0) /* preset selected via commandline or set in previous frame via interaction data */
      {
        selectedPreset = *selectedPresetID;
        /****************** APPLY ON-OFF INTERACTION **********************/
        error = MP_applyOnOffInteraction(h_InteractConfig, audioSceneConfig, localSetupConfig->dmxID, &selectedPreset);

        /****************** DEFINE GROUPS TO PROCESS **********************/
        for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
        {
          if (h_InteractConfig->onOffStatusModified[i] == 1)
          {
            h_InteractConfig->doProcess[i] = 1;
          }
        }
      }
      else /* no preset selected via commandline (and default preset mode not active) */
      {
        for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
        {
          h_InteractConfig->doProcess[i] = audioSceneConfig->asi.groups[i].defaultOnOff;
          h_InteractConfig->onOffStatusModified[i] = audioSceneConfig->asi.groups[i].defaultOnOff;
        }
        /****************** APPLY INITIAL SWITCH GROUP LOGIC **********************/
        error = MP_applySwitchGroupLogic(h_InteractConfig, audioSceneConfig);
      }
    }
  }
  else
  {
    if (h_InteractConfig->defaultPresetMode == 0)
    /* interaction data to be applied and default preset mode is not active */
    {
      /****************** APPLY ON-OFF INTERACTION **********************/
      error = MP_applyOnOffInteraction(h_InteractConfig, audioSceneConfig, localSetupConfig->dmxID, &selectedPreset);

      /****************** GET WIRE OUTPUT INFORMATION ***************************/
      error = MP_applyWireRouting(h_InteractConfig);

      /****************** DEFINE GROUPS TO PROCESS **********************/
      for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
      {
        if ((h_InteractConfig->routeToWireID[i] > -1) || (h_InteractConfig->onOffStatusModified[i] == 1))
        {
          h_InteractConfig->doProcess[i] = 1;
        }
      }

      /****************** GET LOUDNESS COMPENSATION GAIN **********************/
      h_InteractConfig->loudnessCompensationGain = MP_getLoudnessCompensationGain(audioSceneConfig,h_InteractConfig->ElementInteractionData);
    }
    else
    /* interaction data to be applied and default preset mode is active (should never occur, because h_InteractConfig->interactionDataToBeApplied is set to zero if defaultPresetMode is active ) */
    {

    }
  }

  *selectedPresetID = selectedPreset;

  if (chooseSGmember > 0)
  /********************** Switch group interaction via commandline **********************/
  {
    int m;
    int grpID;
    int grpIdx;

    /* assumtion: Only 1 Switch group in bitstream (interaction with 1st switch group) */
    if (audioSceneConfig->asi.numSwitchGroups == 1)
    {
      int SGnumMembers = audioSceneConfig->asi.switchGroups[0].switchGroupNumMembers;

      if ((chooseSGmember > SGnumMembers -1) || (chooseSGmember < 0))
      {
        /* error */
      }
      else
      {
        for (m = 0; m < SGnumMembers; m++)
        {
          grpID = audioSceneConfig->asi.switchGroups[0].switchGroupMemberID[m];
          grpIdx = MP_getGrpIdxAudioScene(audioSceneConfig, grpID);
          if (m == chooseSGmember)
          {
            audioSceneConfig->asi.groups[grpIdx].defaultOnOff = 1;
          }
          else
          {
            audioSceneConfig->asi.groups[grpIdx].defaultOnOff = 0;
          }
        }
      }
    }
  /********************** end of switch group interaction **********************/
  }

  /****************** COPY UN-MODIFIED OAM DATA FOR 1 FRAME ***********************/
  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    /****************** get sorted list of object-based elements with OAM data ***********************/
    error = MP_getOamList(h_InteractConfig, signalGroupConfig, audioSceneConfig);

    if (h_InteractConfig->oamCnt != h_InteractConfig->numObjects_in)
    {
      error = -1;
    }
    error = MP_copyOamFrame(h_InteractConfig->oamSampleModified, h_OamConfig->oamSample_in, h_InteractConfig->numObjects_in, 1);
  }

  /****************** GAIN INTERACTION AND APPLICATION OF LOUDNESS COMPENSATION GAIN ***********************/
  if (h_InteractConfig->interactionDataToBeApplied)
  {
    /* apply gain modification to OAM data and return modified gain for non-OAM data */
    error = MP_getModifiedGains(h_InteractConfig, audioSceneConfig, signalGroupConfig, h_OamConfig->oam_hasBeenDecoded, localSetupConfig->dmxID, overallNumDivergenceObjectsAdded);
  }

  /****************** DIFFUSENESS PROCESSING ***********************/
  if (h_InteractConfig->diffuse_enableProcessing == 1)
  {
    /* take objects + modified oam gains + diffuse gain and create diffuse output */
    /* directly apply modified gains + diffuse gains to object signals and create speaker signal */
    error = MP_createDiffusePart(h_InteractConfig, audioSceneConfig, signalGroupConfig, h_OamConfig, localSetupConfig, h_AudioConfig, isLastFrame, enhancedObjectMetadataConfig, h_enhObjMdFrame);

    /* take objects + modified oam gains + direct gain and process further */
    /* create new modified oam gains by multiplying with direct gain */
    error = MP_applyDirectGain(h_InteractConfig, audioSceneConfig, signalGroupConfig, h_OamConfig, enhancedObjectMetadataConfig, h_enhObjMdFrame, overallNumDivergenceObjectsAdded);
  }

  /****************** DIVERGENCE PROCESSING ******************/
  if ( enableObjectOutputInterface == 0 )
  {
    /* duplication of objects with divergence > 0 */
    error = MP_applyDivergenceProcessing(h_InteractConfig, h_AudioConfig, audioSceneConfig, &overallNumDivergenceObjectsAdded, enhancedObjectMetadataConfig, h_enhObjMdFrame, signalGroupConfig);
  }
  /************* SCENE DISPLACEMENT PROCESSING ***************/
  if ((h_SD_Dec != NULL) && (h_bitstream_SD_Dec != NULL))
  {
      SceneDisplacementData sceneDsplData = { 0 };
      error = SD_readSceneDisplacementBitstream(h_SD_Dec,h_bitstream_SD_Dec, &no_BitsRead);
      error = SD_getSceneDisplacementData(h_SD_Dec, &sceneDsplData);
      error = MP_applySceneDisplacement_CO(signalGroupInfo, h_InteractConfig, audioSceneConfig, signalGroupConfig, localSetupConfig, h_OamConfig->oam_hasBeenDecoded, &sceneDsplData, h_OamConfig, overallNumDivergenceObjectsAdded);
  }

  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    /****************** SCREEN-RELATED PROCESSING ***********************/
    error = MP_applyScreenRelatedRemappingAndZooming(h_InteractConfig, audioSceneConfig, signalGroupConfig, selectedPreset, overallNumDivergenceObjectsAdded);

    /****************** POSITION INTERACTION ***********************/
    if (h_InteractConfig->interactionDataToBeApplied)
    {
      error = MP_applyPositionInteractivity(h_InteractConfig, audioSceneConfig, signalGroupConfig, localSetupConfig->dmxID, overallNumDivergenceObjectsAdded);
    }

    /****************** CLOSEST SPEAKER PROCESSING ***********************/
    error = MP_applyClosestSpeakerProcessing(h_InteractConfig, audioSceneConfig, signalGroupConfig, localSetupConfig, groupSetupConfig, enhancedObjectMetadataConfig, h_enhObjMdFrame, overallNumDivergenceObjectsAdded);
  }

  /****************** OBJECT OUTPUT INTERFACE ***********************/
  h_InteractConfig->numElements_out = h_AudioConfig->nOutChannels;

  if ((enableObjectOutputInterface == 1) && (h_AudioConfig->decode_wav))
  {
    error = MP_writeObjectOutputData(signalGroupInfo, audioSceneConfig, h_InteractConfig, h_OamConfig, h_AudioConfig, signalGroupConfig, h_OoiConfig, frameNo, enhancedObjectMetadataConfig, h_enhObjMdFrame, overallNumDivergenceObjectsAdded);
  }

  /************* CREATION OF OUTPUT DATA *******************/

  h_InteractConfig->numElements_out = h_AudioConfig->nOutChannels;
  /* init output buffers */
  if (h_AudioConfig->decode_qmf)
  {
    h_AudioConfig->numTimeslots = h_AudioConfig->audio_blocksize/QMFLIB_NUMBANDS;
    h_InteractConfig->buffer_temp_real = (float***)calloc(h_InteractConfig->numElements_out, sizeof(float**));
    h_InteractConfig->buffer_temp_imag = (float***)calloc(h_InteractConfig->numElements_out, sizeof(float**));
    for (j = 0; j < h_InteractConfig->numElements_out; j++)
    {
      h_InteractConfig->buffer_temp_real[j] = (float**)calloc(h_AudioConfig->audio_blocksize, sizeof(float*));
      h_InteractConfig->buffer_temp_imag[j] = (float**)calloc(h_AudioConfig->audio_blocksize, sizeof(float*));
      for (k = 0; k < QMFLIB_NUMBANDS; k++)
      {
        h_InteractConfig->buffer_temp_real[j][k] = (float*)calloc(h_AudioConfig->numTimeslots, sizeof(float));
        h_InteractConfig->buffer_temp_imag[j][k] = (float*)calloc(h_AudioConfig->numTimeslots, sizeof(float));
      }
    }
  }
  else if (h_AudioConfig->decode_wav)
  {
    int tmp = max(h_InteractConfig->numElements_out, h_InteractConfig->numDecodedGroups);

    h_InteractConfig->buffer_temp = (float**)calloc(tmp, sizeof(float*));
    for (j = 0; j < tmp; j++)
    {
      h_InteractConfig->buffer_temp[j] = (float*)calloc(h_AudioConfig->audio_blocksize, sizeof(float));
    }
  }

  /****************** QMF-DOMAIN DATA ***********************/

  if (h_AudioConfig->decode_qmf)
  {
    /************* APPLICATION OF ON/OFF AND GAIN FOR NON-OAM ELEMENTS *******************/
    error = MP_applyGainInteractivity_QmfDomain(h_InteractConfig, h_AudioConfig, audioSceneConfig, signalGroupConfig, overallNumDivergenceObjectsAdded);

    h_AudioConfig->audioQmfBuffer_real_tmp = (float**)calloc(h_InteractConfig->numElements_out, sizeof(float*));
    h_AudioConfig->audioQmfBuffer_imag_tmp = (float**)calloc(h_InteractConfig->numElements_out, sizeof(float*));
    for (k = 0; k < h_InteractConfig->numElements_out; k++)
    {
      h_AudioConfig->audioQmfBuffer_real_tmp[k] = (float*)calloc(h_AudioConfig->audio_blocksize, sizeof(float));
      h_AudioConfig->audioQmfBuffer_imag_tmp[k] = (float*)calloc(h_AudioConfig->audio_blocksize, sizeof(float));
    }

    for (j = 0; j < h_InteractConfig->numElements_out; j++)
    {
      int ct = 0;
      for (k = 0; k < h_AudioConfig->numTimeslots; k++)
      {
        for (i = 0; i < QMFLIB_NUMBANDS; i++)
        {
          h_AudioConfig->audioQmfBuffer_real_tmp[j][ct] = h_InteractConfig->buffer_temp_real[j][i][k];
          h_AudioConfig->audioQmfBuffer_imag_tmp[j][ct] = h_InteractConfig->buffer_temp_imag[j][i][k];
          ct++;
        }
      }
    }

    error = wavIO_writeFrame(h_AudioConfig->hWavIO_real, h_AudioConfig->audioQmfBuffer_real_tmp, h_AudioConfig->samplesToWritePerChannel, &h_AudioConfig->samplesWrittenPerChannel);
    error = wavIO_writeFrame(h_AudioConfig->hWavIO_imag, h_AudioConfig->audioQmfBuffer_imag_tmp, h_AudioConfig->samplesToWritePerChannel, &h_AudioConfig->samplesWrittenPerChannel);

  }

  /******************  TIME-DOMAIN DATA ***********************/
  if (h_AudioConfig->decode_wav)
  {
    /************* APPLICATION OF ON/OFF AND GAIN FOR NON-OAM ELEMENTS *******************/
    error = MP_applyGainInteractivity_TimeDomain(h_InteractConfig, h_AudioConfig, audioSceneConfig, signalGroupConfig, overallNumDivergenceObjectsAdded);

    if (overallNumDivergenceObjectsAdded > 0)
    {
      error = wavIO_writeFrame(h_AudioConfig->hWavIO, h_InteractConfig->buffer_temp, h_AudioConfig->samplesToWritePerChannel, &h_AudioConfig->samplesWrittenPerChannel);
    }
    else
    {
      error = wavIO_writeFrame(h_AudioConfig->hWavIO, h_InteractConfig->buffer_temp, h_AudioConfig->samplesToWritePerChannel, &h_AudioConfig->samplesWrittenPerChannel);
    }
  }

  h_AudioConfig->nTotalSamplesWrittenPerChannel += h_AudioConfig->samplesWrittenPerChannel;

  /****************** WRITING OF OAM DATA ***********************/
  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    if (overallNumDivergenceObjectsAdded > 0)
    {
      oam_write_process(h_OamConfig->oamOutFile, h_InteractConfig->oamSampleModified_Divergence, h_OamConfig->oam_version, h_OamConfig->hasDynObjectPrio, h_OamConfig->hasUniformSpread);
    }
    else
    {
      oam_write_process(h_OamConfig->oamOutFile, h_InteractConfig->oamSampleModified, h_OamConfig->oam_version, h_OamConfig->hasDynObjectPrio, h_OamConfig->hasUniformSpread);
    }
  }

  /* finish */
  if (h_AudioConfig->decode_qmf)
  {
    for (k = 0; k < h_InteractConfig->numElements_out; k++)
    {
      free(h_AudioConfig->audioQmfBuffer_real_tmp[k]); h_AudioConfig->audioQmfBuffer_real_tmp[k] = NULL;
      free(h_AudioConfig->audioQmfBuffer_imag_tmp[k]); h_AudioConfig->audioQmfBuffer_imag_tmp[k] = NULL;
    }
    free(h_AudioConfig->audioQmfBuffer_real_tmp); h_AudioConfig->audioQmfBuffer_real_tmp = NULL;
    free(h_AudioConfig->audioQmfBuffer_imag_tmp); h_AudioConfig->audioQmfBuffer_imag_tmp = NULL;

    for (k = 0; k < h_InteractConfig->numElements_out; k++)
    {
      for (j = 0; j < QMFLIB_NUMBANDS; j++)
      {
        free(h_InteractConfig->buffer_temp_real[k][j]); h_InteractConfig->buffer_temp_real[k][j] = NULL;
        free(h_InteractConfig->buffer_temp_imag[k][j]); h_InteractConfig->buffer_temp_imag[k][j] = NULL;
      }
      free(h_InteractConfig->buffer_temp_real[k]); h_InteractConfig->buffer_temp_real[k] = NULL;
      free(h_InteractConfig->buffer_temp_imag[k]); h_InteractConfig->buffer_temp_imag[k] = NULL;
    }
    free(h_InteractConfig->buffer_temp_real); h_InteractConfig->buffer_temp_real = NULL;
    free(h_InteractConfig->buffer_temp_imag); h_InteractConfig->buffer_temp_imag = NULL;
  }

  if (h_AudioConfig->decode_wav)
  {
    int tmp = max(h_InteractConfig->numElements_out, h_InteractConfig->numDecodedGroups);

    for (j = 0; j < tmp; j++)
    {
      free(h_InteractConfig->buffer_temp[j]); h_InteractConfig->buffer_temp[j] = NULL;
    }
    free(h_InteractConfig->buffer_temp); h_InteractConfig->buffer_temp = NULL;
  }

  free(h_InteractConfig->listOAM); h_InteractConfig->listOAM = NULL;
  free(h_InteractConfig->localScreen); h_InteractConfig->localScreen = NULL;
  free(h_InteractConfig->doProcess); h_InteractConfig->doProcess = NULL;

  if (h_InteractConfig->enableScreenRelatedProcessing)
  {
    free(h_InteractConfig->screenRelatedObjects); h_InteractConfig->screenRelatedObjects = NULL;
  }

  return error;
}

int MP_readAudioFrame(H_AUDIO_MP_CONFIG h_AudioConfig)
{
  int error = 0;
  int k, j, i;

  /* DECODED WAV FILE */
  if (h_AudioConfig->decode_wav)
  {
    error = wavIO_readFrame(h_AudioConfig->hWavIO, h_AudioConfig->audioTdBuffer, &h_AudioConfig->samplesReadPerChannel, &h_AudioConfig->isLastFrame, &h_AudioConfig->nZerosPaddedBeginning, &h_AudioConfig->nZerosPaddedEnd);
    if (h_AudioConfig->isLastFrame) /* flush unused part of input buffer */
		{
			for (k = h_AudioConfig->samplesReadPerChannel; k < h_AudioConfig->audio_blocksize; k++)
			{
				for (j = 0; j < (int)h_AudioConfig->nInChannels; j++)
				{
					h_AudioConfig->audioTdBuffer[j][k] = 0.0f;
				}
			}
		}
		h_AudioConfig->samplesToWritePerChannel = h_AudioConfig->nZerosPaddedBeginning + h_AudioConfig->samplesReadPerChannel + h_AudioConfig->nZerosPaddedEnd;
  }

  /* DECODED QMF FILES */
  if (h_AudioConfig->decode_qmf)
  {
    error = wavIO_readFrame(h_AudioConfig->hWavIO_real, h_AudioConfig->audioTdBuffer, &h_AudioConfig->samplesReadPerChannel, &h_AudioConfig->isLastFrame, &h_AudioConfig->nZerosPaddedBeginning, &h_AudioConfig->nZerosPaddedEnd);
    if (h_AudioConfig->isLastFrame) /* flush unused part of input buffer */
		{
			for (k = h_AudioConfig->samplesReadPerChannel; k < h_AudioConfig->audio_blocksize; k++)
			{
				for (j = 0; j < (int)h_AudioConfig->nInChannels; j++)
				{
					h_AudioConfig->audioTdBuffer[j][k] = 0.0f;
				}
			}
		}
    /* transfer to real QMF buffer */
		for (i = 0; i < (int)h_AudioConfig->nInChannels; i++)
		{
			int ct = 0;
			for (k = 0; k < h_AudioConfig->numTimeslots; k++)
			{
				for (j = 0; j < QMFLIB_NUMBANDS; j++)
				{
					h_AudioConfig->audioQmfBuffer_real[k][i][j] = h_AudioConfig->audioTdBuffer[i][ct];
					ct++;
				}
			}
		}

    error = wavIO_readFrame(h_AudioConfig->hWavIO_imag, h_AudioConfig->audioTdBuffer, &h_AudioConfig->samplesReadPerChannel, &h_AudioConfig->isLastFrame, &h_AudioConfig->nZerosPaddedBeginning, &h_AudioConfig->nZerosPaddedEnd);
    if (h_AudioConfig->isLastFrame) /* flush unused part of input buffer */
		{
			for (k = h_AudioConfig->samplesReadPerChannel; k < h_AudioConfig->audio_blocksize; k++)
			{
				for (j = 0; j < (int)h_AudioConfig->nInChannels; j++)
				{
					h_AudioConfig->audioTdBuffer[j][k] = 0.0f;
				}
			}
		}
    /* transfer to imag QMF buffer */
		for (i = 0; i < (int)h_AudioConfig->nInChannels; i++)
		{
			int ct = 0;
			for (k = 0; k < h_AudioConfig->numTimeslots; k++)
			{
				for (j = 0; j < QMFLIB_NUMBANDS; j++)
				{
					h_AudioConfig->audioQmfBuffer_imag[k][i][j] = h_AudioConfig->audioTdBuffer[i][ct];
					ct++;
				}
			}
		}
    h_AudioConfig->samplesToWritePerChannel = h_AudioConfig->nZerosPaddedBeginning + h_AudioConfig->samplesReadPerChannel + h_AudioConfig->nZerosPaddedEnd;
  }

  return error;
}

int MP_applyGainInteractivity_QmfDomain(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int divergenceObjectsAdded)
{
  int i, j, k;
  int error = 0;
  int numBands = QMFLIB_NUMBANDS;
  int numTimeslots = h_AudioConfig->audio_blocksize/numBands;
  int m;

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    if (h_InteractConfig->onOffStatusModified[i] == 0)
    {
      /* set everything to zero */
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      int memberID = -1;

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

        for (j = 0; j < numTimeslots; j++)
        {
          for (m = 0; m < numBands; m++)
          {
            h_InteractConfig->buffer_temp_real[memberID][m][j] = 0.0f;
            h_InteractConfig->buffer_temp_imag[memberID][m][j] = 0.0f;
          }
        }
      }
    }
    if (h_InteractConfig->doProcess[i] == 1)
    {
      if (h_InteractConfig->interactionDataToBeApplied == 1)
      {
        if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 0) /* only apply gain for channel-based elements */
        {
          int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
          int memberID = -1;
          float currentGain_dB = h_InteractConfig->gainModifiedGroup[i];

          if (currentGain_dB == -1.0f)
          {
            /* error: The value of -1 is used to signal object-based element groups */
          }
          else
          {
            float lastGain_dB = h_InteractConfig->gainModified_LastFrame[i];

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

              /* interpolate between lastGain and currentGain over the interval of 1 frame */
              for (j = 0; j < numTimeslots; j++)
              {
                float gainDiff = 0.0f;
                float diffGain = 0.0f;
                float applyGain_factor = 1.0f;

                gainDiff = (float)pow(10.0f, currentGain_dB/20.0f)  - (float)pow(10.0f, lastGain_dB/20.0f);
                diffGain = gainDiff/((numTimeslots-1)*1.0f);

                applyGain_factor = (float)pow(10.0f, lastGain_dB/20.0f) + j*diffGain;
                for (m = 0; m < numBands; m++)
                {
                  if (divergenceObjectsAdded > 0)
                  {
                    h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_real[j][memberID][m] * applyGain_factor;
                    h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_imag[j][memberID][m] * applyGain_factor;
                  }
                  else
                  {
                    h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_real[j][memberID][m] * applyGain_factor;
                    h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_imag[j][memberID][m] * applyGain_factor;
                  }
                }
              }
            }
          }
        }
        else
        {
          /* if it is not a channel-based group -> copy audio without using any gain */
          int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
          int memberID = -1;

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
            for (j = 0; j < numTimeslots; j++)
            {
              for (m = 0; m < numBands; m++)
              {
                if (divergenceObjectsAdded > 0)
                {
                  h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_real[j][memberID][m];
                  h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_imag[j][memberID][m];
                }
                else
                {
                  h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_real[j][memberID][m];
                  h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_imag[j][memberID][m];
                }
              }
            }
          }
        }
      }
      else
      {
        int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
        int memberID = -1;

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
          for (j = 0; j < numTimeslots; j++)
          {
            for (m = 0; m < numBands; m++)
            {
              if (divergenceObjectsAdded > 0)
              {
                h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_real[j][memberID][m];
                h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_divergence_imag[j][memberID][m];
              }
              else
              {
                h_InteractConfig->buffer_temp_real[memberID][m][j] = h_AudioConfig->audioQmfBuffer_real[j][memberID][m];
                h_InteractConfig->buffer_temp_imag[memberID][m][j] = h_AudioConfig->audioQmfBuffer_imag[j][memberID][m];
              }
            }
          }
        }
      }
    }
    h_InteractConfig->gainModified_LastFrame[i] = h_InteractConfig->gainModifiedGroup[i];
  }
  return error;
}

int MP_applyGainInteractivity_TimeDomain(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int divergenceObjectsAdded)
{
  int i, j, k;
  int error = 0;
  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    if (h_InteractConfig->onOffStatusModified[i] == 0)
    {
      /* set everything to zero if group is switched off */
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      int memberID = -1;

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

        for (j = 0; j < h_AudioConfig->audio_blocksize; j++)
        {
          h_InteractConfig->buffer_temp[memberID][j] = 0.0f;
        }
      }
    }
    if ((h_InteractConfig->doProcess[i] == 1))
    {
      if (h_InteractConfig->interactionDataToBeApplied == 1)
      {
        if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 0) /* only apply gain for channel-based elements */
        {
          int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
          int memberID = -1;
          float currentGain_dB = h_InteractConfig->gainModifiedGroup[i];

          if (currentGain_dB == -1.0f)
          {
            /* error: The value of -1 is used to signal object-based element groups */
          }
          else
          {
            float lastGain_dB = h_InteractConfig->gainModified_LastFrame[i];

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

              /* interpolate between lastGain and currentGain over the interval of 1 frame */

              for (j = 0; j < h_AudioConfig->audio_blocksize; j++)
              {
                float gainDiff = 0.0f;
                float diffGain = 0.0f;
                float applyGain_factor = 1.0f;

                gainDiff = (float)pow(10.0f, currentGain_dB/20.0f)  - (float)pow(10.0f, lastGain_dB/20.0f);
                diffGain = gainDiff/((h_AudioConfig->audio_blocksize-1)*1.0f);

                applyGain_factor = (float)pow(10.0f, lastGain_dB/20.0f) + j*diffGain;
                if (divergenceObjectsAdded > 0)
                {
                  h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer_divergence[memberID][j] * (applyGain_factor);
                }
                else
                {
                  h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer[memberID][j] * (applyGain_factor);
                }
              }
            }
          }
        }
        else
        {
          /* if it is not a channel-based group -> copy audio without using any gain */
          int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
          int memberID = -1;

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
            for (j = 0; j < h_AudioConfig->audio_blocksize; j++)
            {
              if (divergenceObjectsAdded > 0)
              {
                h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer_divergence[memberID][j];
              }
              else
              {
                h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer[memberID][j];
              }
            }
          }
        }
      }
      else
      {
        int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
        int memberID = -1;

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

          for (j = 0; j < h_AudioConfig->audio_blocksize; j++)
          {
            if (divergenceObjectsAdded > 0)
            {
              h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer_divergence[memberID][j];
            }
            else
            {
              h_InteractConfig->buffer_temp[memberID][j] = h_AudioConfig->audioTdBuffer[memberID][j];
            }
          }
        }
      }
    }
    h_InteractConfig->gainModified_LastFrame[i] = h_InteractConfig->gainModifiedGroup[i];
  }
  return error;
}


int MP_applyWireRouting(H_INTERACT_MP_CONFIG h_interactConfig)
{
  int i;
  for (i = 0; i < h_interactConfig->numDecodedGroups; i++)
  {
    h_interactConfig->routeToWireID[i] = -1;
    if (h_interactConfig->ElementInteractionData->GroupInteraction[i]->routeToWireID > -1)
    {
      h_interactConfig->routeToWireID[i] = h_interactConfig->ElementInteractionData->GroupInteraction[i]->routeToWireID;
    }
  }
  return 0;
}


int MP_copyOamFrame(StructOamMultidata *Dest,  StructOamMultidata *Source, int numObj, int overwriteSize)
{
  int i;

  if (Source->size2 != 1)
  {
    fprintf(stderr, "Error: Size of OamMultidata exceeds one frame.\n");
    return -1;
  }

  for (i = 0; i < numObj; i++)
  {
    Dest->azimuth[i]          = Source->azimuth[i];
    Dest->elevation[i]        = Source->elevation[i];
    Dest->gain[i]             = Source->gain[i];
    Dest->radius[i]           = Source->radius[i];
    Dest->spread[i]           = Source->spread[i];
    Dest->spread_height[i]    = Source->spread_height[i];
    Dest->spread_depth[i]     = Source->spread_depth[i];
    Dest->dynamic_object_priority[i] = Source->dynamic_object_priority[i];
    Dest->sample[i] = Source->sample[i];
  }
  if (overwriteSize)
  {
    Dest->num_elements = Source->num_elements;
    Dest->size1 = Source->size1;
    Dest->size2 = Source->size2;
  }

  return 0;
}

int MP_applyClosestSpeakerProcessing(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_GROUP_SETUP_DATA groupSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int divergenceObjectsAdded)
{
  int error = 0;
  int i, j, k;
  int ct = 0;

  Setup_SpeakerConfig3d *pSpeakerConfig3d = (Setup_SpeakerConfig3d*)calloc(1, sizeof(Setup_SpeakerConfig3d));

  if (groupSetupConfig->doUseIndividualGroupSetups == 0)
  {

    /***************** GET LOCAL SETUP *************************************/
    if (localSetupConfig->LoudspeakerRendering != NULL) /* get local setup */
    {
      pSpeakerConfig3d->numSpeakers = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers;
      pSpeakerConfig3d->speakerLayoutType = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType;

      if (pSpeakerConfig3d->speakerLayoutType == 0)
      {
        pSpeakerConfig3d->CICPspeakerLayoutIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx;
      }
      else if (pSpeakerConfig3d->speakerLayoutType < 3)
      {
        pSpeakerConfig3d->numSpeakers =  localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers;
        if ( pSpeakerConfig3d->speakerLayoutType == 1 )
        {
          for ( i = 0; i < (int)pSpeakerConfig3d->numSpeakers; i++ )
          {
            pSpeakerConfig3d->CICPspeakerIdx[i] =  localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[i];
          }
        }
        if ( pSpeakerConfig3d->speakerLayoutType == 2 )
        {
          /* reset i for use in this scope */
          i = 0;

          pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->angularPrecision;

          while ( i < (int)pSpeakerConfig3d->numSpeakers )
          {
            pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isCICPspeakerIdx[i];

            if ( pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx )
            {
              pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->CICPspeakerIdx[i];
            }
            else
            {
              pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationClass = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationClass[i];
              if ( pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationClass == 3 )
              {
                pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationAngleIdx[i];
                pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationDirection = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->ElevationDirection[i];
              }

              pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthAngleIdx[i];
              pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->AzimuthDirection[i];
              pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isLFE = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[i];

            }
            i++;
          }
        }
      }
      /* Fill geometry, numChannels and numLFEs ... */
      get_geometry_from_speakerConfig3d(
      pSpeakerConfig3d,
      pSpeakerConfig3d->pGeometry,
      &pSpeakerConfig3d->numChannels,
      &pSpeakerConfig3d->numLFEs);

      /* ... and set numSpeakers */
      pSpeakerConfig3d->numSpeakers = pSpeakerConfig3d->numChannels + pSpeakerConfig3d->numLFEs;

      /* include known positions for speaker layout type 0 and 1 */
      if (( pSpeakerConfig3d->speakerLayoutType < 2) && (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL))
      {
        for (i = 0; i < (int)pSpeakerConfig3d->numSpeakers; i++)
        {
          if (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][i] == 1)
          {
            pSpeakerConfig3d->pGeometry[i].Az = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][i];
            pSpeakerConfig3d->pGeometry[i].El = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][i];
          }
        }
      }
    }
    else /* Get virtual speaker setup for binaural rendering */
    {
      /* Fill geometry, numChannels and numLFEs ... */
      get_geometry_from_speakerConfig3d(
      &localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->Setup_SpeakerConfig3d,
	    pSpeakerConfig3d->pGeometry,
	    &pSpeakerConfig3d->numChannels,
	    &pSpeakerConfig3d->numLFEs);

      /* ... and set numSpeakers */
      pSpeakerConfig3d->numSpeakers = pSpeakerConfig3d->numChannels + pSpeakerConfig3d->numLFEs;
    }
  }

  /********************* PROCESSING ************************/

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    int hasDivergence = 0;

    int totalNumMembers = 0;

    if ((MP_getSignalGroupType(audioSceneConfig, signalGroupConfig, i) == 1) && (h_InteractConfig->routeToWireID[i] == -1)) /* According to ISO/IEC 23008-3:2021, 18.2.5 */
    {
      int numMembers = 0;
      int* idx_min = NULL;

      if (h_InteractConfig->divergence_ASImodified == 1)
      {
        numMembers = h_InteractConfig->origNumGroupMembers[i];
      }
      else
      {
        numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      }

      idx_min = (int*)calloc(numMembers,sizeof(int));
      h_InteractConfig->azimuthModified = (float*)calloc(numMembers,sizeof(float));
      h_InteractConfig->elevationModified = (float*)calloc(numMembers,sizeof(float));

      for (k = 0; k < numMembers; k++)
      {
        hasDivergence = enhancedObjectMetadataConfig->hasDivergence[0];

        if ((hasDivergence == 0) &&
            (h_InteractConfig->oamSampleModified->spread[totalNumMembers+k] == 0) &&
            (h_InteractConfig->oamSampleModified->spread_height[totalNumMembers+k] == 0) &&
            (h_enhObjMdFrame->closestSpeakerPlayout[totalNumMembers+k])) /* only process if object has no divergence, no spread and the closestSpeakerPlayout flag active, according to ISO/IEC 23008-3:2021, 18.2.5 */
        {
          int memberID = -1;
          int oamIdx = -1;
          float LS_diff = 0.0f;
          int first = 0;
          int m;
          if (groupSetupConfig->doUseIndividualGroupSetups == 1)  /* copy individual setup */
          {
            memset(pSpeakerConfig3d,0,sizeof(groupSetupConfig->speakerConfig[i]));
            memcpy(pSpeakerConfig3d,&groupSetupConfig->speakerConfig[i],sizeof(groupSetupConfig->speakerConfig[i]));

            /* include known positions */
            if (localSetupConfig->LoudspeakerRendering != NULL)
            {
              for (m = 0; m < (int)pSpeakerConfig3d->numSpeakers; m++)
              {
                if (groupSetupConfig->hasKnownPos[i][m] == 1)
                {
                  pSpeakerConfig3d->pGeometry[m].Az = (int)groupSetupConfig->knownAzimuth[i][m];
                  pSpeakerConfig3d->pGeometry[m].El = (int)groupSetupConfig->knownElevation[i][m];
                }
              }
            }
          }
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
          for (j = 0; j < (int)pSpeakerConfig3d->numSpeakers; j++)
          {
            if (pSpeakerConfig3d->pGeometry[j].LFE == 0)
            {
              float temp_diff = 0.0f;
              float az_this = (float)pSpeakerConfig3d->pGeometry[j].Az;
              float el_this = (float)pSpeakerConfig3d->pGeometry[j].El;
              float min_az = 0.0f, min_el = 0.0f, max_az = 0.0f, max_el = 0.0f;
              float min_el1 = 0.0f, max_el1 = 0.0f, min_el2 = 0.0f, max_el2 = 0.0f, min_az1 = 0.0f, max_az1 = 0.0f, min_az2 = 0.0f, max_az2 = 0.0f, min_az3 = 0.0f, max_az3 = 0.0f;
              int condition = 0;

              if (enhancedObjectMetadataConfig->hasClosestSpeakerCondition == 1)
              {
                min_az = h_InteractConfig->azimuthModified[k] - enhancedObjectMetadataConfig->closestSpeakerThresholdAngle;
                max_az = h_InteractConfig->azimuthModified[k] + enhancedObjectMetadataConfig->closestSpeakerThresholdAngle;
                min_el = h_InteractConfig->elevationModified[k] - enhancedObjectMetadataConfig->closestSpeakerThresholdAngle;
                max_el = h_InteractConfig->elevationModified[k] + enhancedObjectMetadataConfig->closestSpeakerThresholdAngle;
              }
              else
              {
                min_az = -180.0f;
                max_az = +180.0f;
                min_el = -90.0f;
                max_el = +90.0f;
              }

              if (((max_el > 90.0f) || (min_el < -90.0f)) && ((min_az >= -180.0f) && (max_az <= 180.0f)))
              {
                if (max_el > 90.0f)
                {
                  min_el1 = min_el;
                  max_el1 = 90.0f;
                  min_el2 = 90.0f - (float)fabs(90.0f - max_el);
                  max_el2 = 90.0f;
                  min_az2 = min_az + 180.0f;
                  max_az2 = max_az + 180.0f;
                }
                else
                {
                  min_el1 = -90.0f;
                  max_el1 = max_el;
                  min_el2 = -90.0f;
                  max_el2 = 90.0f + (float)fabs(90.0f + min_el);
                  min_az2 = min_az + 180.0f;
                  max_az2 = max_az + 180.0f;
                }
                condition = ((((az_this >= min_az)&&(az_this <= max_az))&&((el_this >= min_el1)&&(el_this <= max_el1)))||(((az_this >= min_az2)&&(az_this <= max_az2))&&((el_this >= min_el2)&&(el_this <= max_el2))));
              }
              else if (((max_az > 180.0f) || (min_az < -180.0f)) && ((min_el >= -90.0f) &&(max_el <= 90.0f)))
              {
                if (min_az < 180.0f)
                {
                  min_az1 = -180.0;
                  max_az1 = max_az;
                  min_az2 = 180.0f - (min_az + 180.0f);
                  max_az2 = 180.0f;
                }
                else
                {
                  min_az1 = min_az;
                  max_az1 = 180.0f;
                  min_az2 = -180.0f;
                  max_az2 = -180.0f + (max_az - 180.0f);
                }
                condition = ((((az_this >= min_az1)&&(az_this <= max_az1))&&((el_this >= min_el)&&(el_this <= max_el)))||(((az_this >= min_az2)&&(az_this <= max_az2))&&((el_this >= min_el)&&(el_this <= max_el))));
              }
              else if (((max_az > 180.0f) || (min_az < -180.0f)) && ((min_el < -90.0f) &&(max_el > 90.0f)))
              {
                if (min_az < 180.0f)
                {
                  min_az1 = -180.0;
                  max_az1 = max_az;
                  min_az2 = 180.0f - (min_az + 180.0f);
                  max_az2 = 180.0f;
                }
                else
                {
                  min_az1 = min_az;
                  max_az1 = 180.0f;
                  min_az2 = -180.0f;
                  max_az2 = -180.0f + (max_az - 180.0f);
                }
                if (max_el > 90.0f)
                {
                  min_el1 = min_el;
                  max_el1 = 90.0f;
                  min_el2 = 90.0f - (float)fabs(90.0f - max_el);
                  max_el2 = 90.0f;
                  min_az3 = min_az + 180.0f;
                  max_az3 = max_az + 180.0f;
                }
                else
                {
                  min_el1 = -90.0f;
                  max_el1 = max_el;
                  min_el2 = -90.0f;
                  max_el2 = 90.0f + (float)fabs(90.0f + min_el);
                  min_az3 = min_az + 180.0f;
                  max_az3 = max_az + 180.0f;
                }
                condition = ((((az_this >= min_az1)&&(az_this <= max_az1)) && ((el_this >= min_el1)&&(el_this <= max_el1))) || (((az_this >= min_az2)&&(az_this <= max_az2)) && ((el_this >= min_el1)&&(el_this <= max_el1))) || (((az_this >= min_az3)&&(az_this <= max_az3)) && ((el_this >= min_el2)&&(el_this <= max_el2))));
              }
              else
              {
                condition = (((az_this >= min_az) && (az_this <= max_az)) && ((el_this >= min_el) && (el_this <= max_el)));
              }

              if (condition == 1)
              {
                if (first == 0)
                {
                  first = 1;
                }

                temp_diff = (float)(fabs(el_this - h_InteractConfig->elevationModified[k]) + fabs(az_this - h_InteractConfig->azimuthModified[k]));

                if (first == 1)
                {
                  LS_diff = temp_diff;
                  idx_min[k] = j;
                  first = -1;
                }
                else
                {
                  if (temp_diff < LS_diff)
                  /* Test for "smaller", not "smaller or equal" --> if there are several speakers with the same minimum distance, */
                  /* the first in the SpeakerConfig3D structure shall be taken as the closest speaker */
                  {
                    LS_diff = temp_diff;
                    idx_min[k] = j;
                  }
                }
              }
            }
          }

          if (first != 0)
          {
            h_InteractConfig->azimuthModified[k] = (float)pSpeakerConfig3d->pGeometry[idx_min[k]].Az;
            h_InteractConfig->elevationModified[k] = (float)pSpeakerConfig3d->pGeometry[idx_min[k]].El;

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
        }
        else
        {
          /* do nothing if divergence value of group is bigger than 0 */
        }
        ct++;
        totalNumMembers += numMembers;
      }
      free(idx_min); idx_min = NULL;
      free(h_InteractConfig->azimuthModified); h_InteractConfig->azimuthModified = NULL;
      free(h_InteractConfig->elevationModified); h_InteractConfig->elevationModified = NULL;
    }
  }
  free(pSpeakerConfig3d); pSpeakerConfig3d = NULL;
  return error;
}



int MP_applyPositionInteractivity(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int downmixId, int divergenceObjectsAdded)
{
  int i, k, j;
  int e;
  int error = 0;
  int chosenPreset = -1;
  int groupHasCondition = -1;
  int presetIdx = -1;
  int extensionIdx = -1;
  int listSgCondition[MAE_MAX_NUM_GROUPS][4];

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    groupHasCondition = -1;

    listSgCondition[i][0] = 0;
    listSgCondition[i][1] = -1;
    listSgCondition[i][2] = -1;
    listSgCondition[i][3] = -1;

    if (h_InteractConfig->doProcess[i])
    {
      if (h_InteractConfig->routeToWireID[i] == -1)
      {
        if (audioSceneConfig->asi.groups[i].allowPositionInteractivity)
        {
          if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1) /* at the moment not possible for SAOC */
          {
            int doApply = 0;
            int doApply_onlyPresetDependentValues = 0;
            if (h_InteractConfig->interactionType == 0)
            {
              doApply = 1;
            }
            else
            {
              chosenPreset = h_InteractConfig->ElementInteractionData->groupPresetID;
              for (k = 0; k < audioSceneConfig->asi.numGroupPresets; k++)
              {
                if (audioSceneConfig->asi.groupPresets[k].presetID == chosenPreset)
                {
                  presetIdx = k;
                  if (audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions > 0)
                  {
                    for (e = 0; e < audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions; e++)
                    {
                      if (audioSceneConfig->asi.groupPresetExtensions[k].downmixId[e] == downmixId)
                      {
                        extensionIdx = e;
                        break;
                      }
                    }
                  }
                  groupHasCondition = -1;

                  if (extensionIdx > -1)
                  {
                    for (j = 0; j < audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetNumConditions[extensionIdx]; j++)
                    {
                      if ((audioSceneConfig->asi.groups[i].groupID == audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetReferenceID[extensionIdx][j]) && (audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetIsSwitchGroupCondition[extensionIdx][j] == 0))
                      {
                        groupHasCondition = j;
                        if (audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetDisablePosition[groupHasCondition][j] == 0)
                        {
                          doApply = 1;
                        }
                        else
                        {
                          /* do not apply user position interaction, but "offset" values defined by preset */
                          doApply = 1;
                          doApply_onlyPresetDependentValues = 1;
                        }
                      }
                    }
                    /* if group has no condition, but is part of a SG with SGC, then the group has to be processed nevertheless with the values from the preset */
                    if (groupHasCondition == -1)
                    {
                      int isSgMember_tmp = -1;
                      int sgId_temp = -1;
                      int runTo = 0;
                      int r = 0;
                      int hasSgCondition = 0;

                      /* check if group is member of a switchGroup */
                      isSgMember_tmp = MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups, audioSceneConfig->asi.switchGroups, audioSceneConfig->asi.groups[i].groupID, &sgId_temp);
                      if ( isSgMember_tmp )
                      {
                        /* if yes -> check if a condition exists for this switchGroup */
                        if (extensionIdx > -1)
                        {
                          runTo = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetNumConditions[extensionIdx];
                        }
                        else
                        {
                          runTo = audioSceneConfig->asi.groupPresets[presetIdx].presetNumConditions;
                        }
                        for (r = 0; r < runTo; r++) /* run over all conditions of chosen preset (preset extension) */
                        {
                          int sgId = -1;
                          if (extensionIdx > -1)
                          {
                            int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetIsSwitchGroupCondition[extensionIdx][r];
                            if (isSgCond == 1)
                            {
                              sgId = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetReferenceID[extensionIdx][r];
                            }
                          }
                          else
                          {
                            int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].isSwitchGroupCondition[r];
                            if (isSgCond == 1)
                            {
                              sgId = audioSceneConfig->asi.groupPresets[presetIdx].presetGroupID[r];
                            }
                          }
                          if (sgId_temp == sgId)
                          {
                            hasSgCondition = 1;
                            listSgCondition[i][0] = 1; /* has switch group condition */
                            listSgCondition[i][2] = sgId; /* switch group id */
                            listSgCondition[i][3] = r; /* condition index */
                            listSgCondition[i][4] = extensionIdx; /* extension index */
                            break;
                          }
                        }
                        if (hasSgCondition)
                        {
                          /* do apply "offset" values defined by preset */
                          doApply = 1;
                          if (audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetDisablePosition[extensionIdx][r] == 1)
                          {
                            doApply_onlyPresetDependentValues = 1;
                          }
                        }
                      }
                    }
                  }
                  else
                  {
                    for (j = 0; j < audioSceneConfig->asi.groupPresets[k].presetNumConditions; j++)
                    {
                      if (audioSceneConfig->asi.groups[i].groupID == audioSceneConfig->asi.groupPresets[k].presetGroupID[j])
                      {
                        groupHasCondition = j;
                        if (audioSceneConfig->asi.groupPresets[k].groupPresetDisablePosition[groupHasCondition] == 0)
                        {
                          doApply = 1;
                        }
                        else
                        {
                          /* do not apply user position interaction, but "offset" values defined by preset */
                          doApply = 1;
                          doApply_onlyPresetDependentValues = 1;
                        }
                      }
                    }

                    /* if group has no condition, but is part of a SG with SGC, then the group has to be processed nevertheless with the values from the preset */
                    if (groupHasCondition == -1)
                    {
                      int isSgMember_tmp = -1;
                      int sgId_temp = -1;
                      int runTo = 0;
                      int r = 0;
                      int hasSgCondition = 0;

                      /* check if group is member of a switchGroup */
                      isSgMember_tmp = MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups, audioSceneConfig->asi.switchGroups, audioSceneConfig->asi.groups[i].groupID, &sgId_temp);
                      if ( isSgMember_tmp )
                      {
                        runTo = audioSceneConfig->asi.groupPresets[presetIdx].presetNumConditions;
                        for (r = 0; r < runTo; r++) /* run over all conditions of chosen preset (preset extension) */
                        {
                          int sgId = -1;

                          int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].isSwitchGroupCondition[r];
                          if (isSgCond == 1)
                          {
                            sgId = audioSceneConfig->asi.groupPresets[presetIdx].presetGroupID[r];
                          }

                          if (sgId_temp == sgId)
                          {
                            hasSgCondition = 1;
                            listSgCondition[i][0] = 1; /* has switch group condition */
                            listSgCondition[i][2] = sgId; /* switch group id */
                            listSgCondition[i][3] = r; /* condition index */
                            break;
                          }
                        }
                        if (hasSgCondition)
                        {
                          /* do apply "offset" values defined by preset */
                          doApply = 1;
                          if (audioSceneConfig->asi.groupPresets[presetIdx].groupPresetDisablePosition[r] == 1)
                          {
                            doApply_onlyPresetDependentValues = 1;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            if (doApply == 1)
            {
              int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
              int memberID = -1;
              int oamIdx = -1;

              float az_mod;
              float el_mod;
              float dist_mod;

              h_InteractConfig->azimuthModified = (float*)calloc(numMembers,sizeof(float));
              h_InteractConfig->elevationModified = (float*)calloc(numMembers,sizeof(float));
              h_InteractConfig->distanceModified = (float*)calloc(numMembers,sizeof(float));

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
                   }
                }
                if ((oamIdx == -1) || (h_InteractConfig->oamSampleModified == NULL))
                {
                  return -1;
                }
                /* copy unmodified values to interactConfig struct */
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

                /* copy modification values to variables */
                if (doApply_onlyPresetDependentValues == 0)
                {
                  /* copy user interaction */
                  az_mod = h_InteractConfig->ElementInteractionData->GroupInteraction[i]->azOffset;
                  el_mod = h_InteractConfig->ElementInteractionData->GroupInteraction[i]->elOffset;
                  dist_mod = h_InteractConfig->ElementInteractionData->GroupInteraction[i]->distFactor;
                }
                else
                {
                  /* do not copy user interaction */
                  az_mod = 0.0f;
                  el_mod = 0.0f;
                  dist_mod = 1.0f;
                }

                if (h_InteractConfig->interactionType == 1)
                {
                  if (groupHasCondition > -1)
                  {
                    if (audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetReferenceID[extensionIdx][groupHasCondition] == 1)
                    {
                      /* add preset extension modification values */
                      az_mod += audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetAz[extensionIdx][groupHasCondition];
                      el_mod += audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetEl[extensionIdx][groupHasCondition];
                      dist_mod *= audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetDist[extensionIdx][groupHasCondition];
                    }
                  }
                  else /* group has no condition, but is part of a switch group with a switch group condition */
                  {
                    if (listSgCondition[i][0] == 1)
                    {
                      if (listSgCondition[i][4] > -1)
                      {
                        az_mod += audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetAz[listSgCondition[i][4]][listSgCondition[i][3]];
                        el_mod += audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetEl[listSgCondition[i][4]][listSgCondition[i][3]];
                        dist_mod *= audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetDist[listSgCondition[i][4]][listSgCondition[i][3]];
                      }
                      else
                      {
                        az_mod += audioSceneConfig->asi.groupPresets[presetIdx].f_groupPresetAzOffset[listSgCondition[i][3]];
                        el_mod += audioSceneConfig->asi.groupPresets[presetIdx].f_groupPresetElOffset[listSgCondition[i][3]];
                        dist_mod *= audioSceneConfig->asi.groupPresets[presetIdx].f_groupPresetDist[listSgCondition[i][3]];
                      }
                    }
                  }
                }

                /* check for modification ranges */
                if (az_mod < audioSceneConfig->asi.groups[i].f_interactivityMinAzOffset)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  az_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMinAzOffset;
                }
                if (az_mod > audioSceneConfig->asi.groups[i].f_interactivityMaxAzOffset)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  az_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMaxAzOffset;
                }
                if (el_mod < audioSceneConfig->asi.groups[i].f_interactivityMinElOffset)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  el_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMinElOffset;
                }
                if (el_mod > audioSceneConfig->asi.groups[i].f_interactivityMaxElOffset)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  el_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMaxElOffset;
                }
                if (dist_mod < audioSceneConfig->asi.groups[i].f_interactivityMinDistFactor)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  dist_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMinDistFactor;
                }
                if (dist_mod > audioSceneConfig->asi.groups[i].f_interactivityMaxDistFactor)
                {
                  error = -1;
                  fprintf(stderr, "Warning: Violation of interactivity ranges.\n");
                  dist_mod = (float)audioSceneConfig->asi.groups[i].f_interactivityMaxDistFactor;
                }

                /* apply modification */
                if (az_mod != 0.0f)
                {
                  h_InteractConfig->azimuthModified[k] += az_mod;
                }
                if (el_mod != 0.0f)
                {
                  h_InteractConfig->elevationModified[k] += el_mod;
                }
                if (dist_mod != 1.0f)
                {
                  h_InteractConfig->distanceModified[k] *= dist_mod;
                }

                /* copy modified values to modifies OAM sample */

                if (divergenceObjectsAdded > 0)
                {
                  h_InteractConfig->oamSampleModified_Divergence->azimuth[oamIdx] = h_InteractConfig->azimuthModified[k];
                  h_InteractConfig->oamSampleModified_Divergence->elevation[oamIdx] = h_InteractConfig->elevationModified[k];
                  h_InteractConfig->oamSampleModified_Divergence->radius[oamIdx] = h_InteractConfig->distanceModified[k];
                }
                else
                {
                  h_InteractConfig->oamSampleModified->azimuth[oamIdx] = h_InteractConfig->azimuthModified[k];
                  h_InteractConfig->oamSampleModified->elevation[oamIdx] = h_InteractConfig->elevationModified[k];
                  h_InteractConfig->oamSampleModified->radius[oamIdx] = h_InteractConfig->distanceModified[k];
                }
              }
              free(h_InteractConfig->azimuthModified); h_InteractConfig->azimuthModified = NULL;
              free(h_InteractConfig->elevationModified); h_InteractConfig->elevationModified = NULL;
              free(h_InteractConfig->distanceModified); h_InteractConfig->distanceModified = NULL;
            }
          }
        }
      }
    }
  }
  return error;
}

int MP_getModifiedGains(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int oamHasBeenDecoded, int downmixId, int divergenceObjectsAdded)
{
  int i, k, j;
  int e;
  int error = 0;
  int groupHasCondition = -1;
  int chosenPreset = -1;
  int presetIdx = -1;
  int extensionIdx = -1;
  int listSgCondition[MAE_MAX_NUM_GROUPS][4];

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    listSgCondition[i][0] = 0;
    listSgCondition[i][1] = -1;
    listSgCondition[i][2] = -1;
    listSgCondition[i][3] = -1;

    if (h_InteractConfig->doProcess[i])
    {
      if (audioSceneConfig->asi.groups[i].allowGainInteractivity)
      {
        float interactionGain_dB = 0.0f;
        int doApply_onlyPresetDependentValues = 0;

        if (h_InteractConfig->interactionType == 1)
        {
          chosenPreset = h_InteractConfig->ElementInteractionData->groupPresetID;

          for (k = 0; k < audioSceneConfig->asi.numGroupPresets; k++)
          {
            if (audioSceneConfig->asi.groupPresets[k].presetID == chosenPreset)
            {
              presetIdx = k;
              groupHasCondition = -1;

              if (audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions > 0)
              {
                for (e = 0; e < audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions; e++)
                {
                  if (audioSceneConfig->asi.groupPresetExtensions[k].downmixId[e] == downmixId)
                  {
                    extensionIdx = e;
                    break;
                  }
                }
              }

              if (extensionIdx > -1)
              {
                for (j = 0; j < audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetNumConditions[extensionIdx]; j++)
                {
                  if ((audioSceneConfig->asi.groups[i].groupID == audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetReferenceID[extensionIdx][j]) && (audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetIsSwitchGroupCondition[extensionIdx][j] == 0))
                  {
                    groupHasCondition = j;
                    if (audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetDisableGain[extensionIdx][j] == 1)
                    {
                      doApply_onlyPresetDependentValues = 1;
                    }
                    if (audioSceneConfig->asi.groupPresetExtensions[k].downmixIdPresetGainFlag[extensionIdx][j] == 1)
                    {
                      interactionGain_dB += audioSceneConfig->asi.groupPresetExtensions[k].f_downmixIdPresetGain[extensionIdx][groupHasCondition];
                    }
                    break;
                  }
                }
                if (groupHasCondition == -1)
                {
                  int isSgMember = -1;
                  int sgId_temp = -1;
                  int runTo = 0;
                  int r = 0;
                  int hasSgCondition = 0;

                  /* check if group is member of a switchGroup */
                  isSgMember = MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups, audioSceneConfig->asi.switchGroups, audioSceneConfig->asi.groups[i].groupID, &sgId_temp);
                  if ( isSgMember )
                  {
                    /* if yes -> check if a condition exists for this switchGroup */
                    if (extensionIdx > -1)
                    {
                      runTo = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetNumConditions[extensionIdx];
                    }
                    else
                    {
                      runTo = audioSceneConfig->asi.groupPresets[presetIdx].presetNumConditions;
                    }
                    for (r = 0; r < runTo; r++) /* run over all conditions of chosen preset (preset extension) */
                    {
                      int sgId = -1;
                      if (extensionIdx > -1)
                      {
                        int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetIsSwitchGroupCondition[extensionIdx][r];
                        if (isSgCond == 1)
                        {
                          sgId = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetReferenceID[extensionIdx][r];
                        }
                      }
                      else
                      {
                        int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].isSwitchGroupCondition[r];
                        if (isSgCond == 1)
                        {
                          sgId = audioSceneConfig->asi.groupPresets[presetIdx].presetGroupID[r];
                        }
                      }
                      if (sgId_temp == sgId)
                      {
                        hasSgCondition = 1;
                        listSgCondition[i][0] = 1; /* has switch group condition */
                        listSgCondition[i][2] = sgId; /* switch group id */
                        listSgCondition[i][3] = r; /* condition index */
                        listSgCondition[i][4] = extensionIdx; /* extension index */
                        break;
                      }
                    }
                    if (hasSgCondition)
                    {
                      /* do apply "offset" values defined by preset */
                      if (audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetDisableGain[extensionIdx][r] == 1)
                      {
                        doApply_onlyPresetDependentValues = 1;
                      }
                      if (audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetGainFlag[extensionIdx][r] == 1)
                      {
                        interactionGain_dB += audioSceneConfig->asi.groupPresetExtensions[presetIdx].f_downmixIdPresetGain[extensionIdx][r];
                      }
                    }
                  }
                }
              }
              else
              {
                for (j = 0; j < audioSceneConfig->asi.groupPresets[k].presetNumConditions; j++)
                {
                  if (audioSceneConfig->asi.groups[i].groupID == audioSceneConfig->asi.groupPresets[k].presetGroupID[j])
                  {
                    groupHasCondition = j;
                    if (audioSceneConfig->asi.groupPresets[k].groupPresetDisableGain[groupHasCondition] == 1)
                    {
                      doApply_onlyPresetDependentValues = 1;
                    }
                    if (audioSceneConfig->asi.groupPresets[k].groupPresetGainFlag[groupHasCondition] == 1)
                    {
                      interactionGain_dB += audioSceneConfig->asi.groupPresets[k].f_groupPresetGain[groupHasCondition];
                    }
                    break;
                  }
                }
                if (groupHasCondition == -1)
                {
                  int isSgMember = -1;
                  int sgId_temp = -1;
                  int runTo = 0;
                  int r = 0;
                  int hasSgCondition = 0;

                  /* check if group is member of a switchGroup */
                  isSgMember = MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups, audioSceneConfig->asi.switchGroups, audioSceneConfig->asi.groups[i].groupID, &sgId_temp);
                  if ( isSgMember )
                  {
                    runTo = audioSceneConfig->asi.groupPresets[presetIdx].presetNumConditions;
                    for (r = 0; r < runTo; r++) /* run over all conditions of chosen preset (preset extension) */
                    {
                      int sgId = -1;

                      int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].isSwitchGroupCondition[r];
                      if (isSgCond == 1)
                      {
                        sgId = audioSceneConfig->asi.groupPresets[presetIdx].presetGroupID[r];
                      }

                      if (sgId_temp == sgId)
                      {
                        hasSgCondition = 1;
                        listSgCondition[i][0] = 1; /* has switch group condition */
                        listSgCondition[i][2] = sgId; /* switch group id */
                        listSgCondition[i][3] = r; /* condition index */
                        break;
                      }
                    }
                    if (hasSgCondition)
                    {
                      /* do apply "offset" values defined by preset */
                      if (audioSceneConfig->asi.groupPresets[presetIdx].groupPresetDisablePosition[r] == 1)
                      {
                        doApply_onlyPresetDependentValues = 1;
                      }
                      if (audioSceneConfig->asi.groupPresets[presetIdx].groupPresetGainFlag[r] == 1)
                      {
                        interactionGain_dB += audioSceneConfig->asi.groupPresets[presetIdx].f_groupPresetGain[r];
                      }
                    }
                  }
                }
              }
            }
          }
        }
        if (doApply_onlyPresetDependentValues == 0)
        {
          interactionGain_dB += (float)h_InteractConfig->ElementInteractionData->GroupInteraction[i]->gain_dB;
        }

        if (interactionGain_dB < audioSceneConfig->asi.groups[i].f_interactivityMinGain)
        {
          error = -1;
          fprintf(stderr, "Warning: Violation of minimum interactivity gain border.\n");
          interactionGain_dB = audioSceneConfig->asi.groups[i].f_interactivityMinGain;
        }
        if (interactionGain_dB > audioSceneConfig->asi.groups[i].f_interactivityMaxGain)
        {
          error = -1;
          fprintf(stderr, "Warning: Violation of maximum interactivity gain border.\n");
          interactionGain_dB = audioSceneConfig->asi.groups[i].f_interactivityMaxGain;
        }

        if ((MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1) && (oamHasBeenDecoded == 1)) /* at the moment not possible for SAOC */
        {
          int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
          int memberID = -1;
          int oamIdx = -1;
          h_InteractConfig->gainModifiedSingleObject = (float*)calloc(numMembers,sizeof(float));

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
                 if (divergenceObjectsAdded > 0)
                 {
                   h_InteractConfig->gainModifiedSingleObject[k] = h_InteractConfig->oamSampleModified_Divergence->gain[oamIdx];
                 }
                 else
                 {
                   h_InteractConfig->gainModifiedSingleObject[k] = h_InteractConfig->oamSampleModified->gain[oamIdx];
                 }
                 break;
               }
            }

            if ((oamIdx == -1) || (h_InteractConfig->oamSampleModified == NULL))
            {
              return -1;
            }

            if (h_InteractConfig->gainModifiedSingleObject[k] != 0.0f)
            {
              /* OAM gain to dB */
              h_InteractConfig->gainModifiedSingleObject[k] = 20.0f*(float)log(h_InteractConfig->gainModifiedSingleObject[k]);

              /* add interaction gain */
              h_InteractConfig->gainModifiedSingleObject[k] += interactionGain_dB;

              /* calculate factor from dB */
              h_InteractConfig->gainModifiedSingleObject[k] = (float)pow(10.0f, h_InteractConfig->gainModifiedSingleObject[k]/20.0f);
            }

            /* copy to OAM structure */
            if (divergenceObjectsAdded > 0)
            {
              h_InteractConfig->oamSampleModified_Divergence->gain[oamIdx] = h_InteractConfig->gainModifiedSingleObject[k];
            }
            else
            {
              h_InteractConfig->oamSampleModified->gain[oamIdx] = h_InteractConfig->gainModifiedSingleObject[k];
            }
          }
          h_InteractConfig->gainModifiedGroup[i] = -1.0f; /* object-based group */
          free(h_InteractConfig->gainModifiedSingleObject); h_InteractConfig->gainModifiedSingleObject = NULL;
        }
        else
        {
          h_InteractConfig->gainModifiedGroup[i] += interactionGain_dB;
        }
      }
      else
      {
        if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1)
        {
          h_InteractConfig->gainModifiedGroup[i] = -1.0f; /* object-based group */
        }
        else
        {
          h_InteractConfig->gainModifiedGroup[i] = 1.0f;
        }
      }
    }
    else
    {
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      int memberID = -1;
      int oamIdx = -1;

      if ((MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1) && (oamHasBeenDecoded == 1))
      {
        h_InteractConfig->gainModifiedGroup[i] = -1.0f; /* object-based group */
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
            }
            if ((oamIdx == -1) || (h_InteractConfig->oamSampleModified == NULL))
            {
              return -1;
            }
            if (divergenceObjectsAdded > 0)
            {
              h_InteractConfig->oamSampleModified_Divergence->gain[oamIdx] = 0.0f;
            }
            else
            {
              h_InteractConfig->oamSampleModified->gain[oamIdx] = 0.0f;
            }
            break;
          }
        }
      }
      else
      {
        h_InteractConfig->gainModifiedGroup[i] = 0.0f;
      }
    }

    /* add loudness compensation gain */
    if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1) /* object-based group */
    {
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      int memberID = -1;
      int oamIdx = -1;
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
      }
      /* get OAM data for the current group member */
      for (j = 0; j < h_InteractConfig->oamCnt; j++)
      {
        if (h_InteractConfig->listOAM[j] == memberID)
        {
          oamIdx = j;
        }
        if ((oamIdx == -1) || (h_InteractConfig->oamSampleModified == NULL))
        {
          return -1;
        }
        if (divergenceObjectsAdded > 0)
        {
          h_InteractConfig->oamSampleModified_Divergence->gain[oamIdx] *= (float)pow(10.0f, h_InteractConfig->loudnessCompensationGain/20.0f);
        }
        else
        {
          h_InteractConfig->oamSampleModified_Divergence->gain[oamIdx] *= (float)pow(10.0f, h_InteractConfig->loudnessCompensationGain/20.0f);
        }
        break;
      }
    }
    else
    {
      h_InteractConfig->gainModifiedGroup[i] *= (float)pow(10.0f, h_InteractConfig->loudnessCompensationGain/20.0f);
    }
  }
  return error;
}

int MP_getOamList(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig)
{
  int* listOAM_temp;
  int oamCnt_tmp;
  int i, k, j, temp;


  listOAM_temp = (int*)calloc(MAE_MAX_NUM_OBJECTS, sizeof(int));
  oamCnt_tmp = 0;

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig,i) == 1) /* at the moment not possible for SAOC */
    {
      int numMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      for (k = 0; k < numMembers; k++)
      {
        if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
        {
          listOAM_temp[oamCnt_tmp] = audioSceneConfig->asi.groups[i].startID + k;
          oamCnt_tmp++;
        }
        else
        {
          listOAM_temp[oamCnt_tmp] = audioSceneConfig->asi.groups[i].metaDataElementID[k];
          oamCnt_tmp++;
        }
      }
    }
  }

  for (k = 0; k < oamCnt_tmp; k++)
  {
    for (j = k + 1; j < oamCnt_tmp; j++)
    {
      if (listOAM_temp[k] > listOAM_temp[j])
      {
          temp =  listOAM_temp[k];
          listOAM_temp[k] = listOAM_temp[j];
          listOAM_temp[j] = temp;
      }
    }
  }
  h_InteractConfig->listOAM = listOAM_temp;
  h_InteractConfig->oamCnt = oamCnt_tmp;
  return 0;
}

int MP_applySwitchGroupLogic(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig)
{
  int i, k;
  int numSwitchGroups = audioSceneConfig->asi.numSwitchGroups;

  /* loop over all switch groups */
  for (i = 0; i < numSwitchGroups; i++)
  {
    int isValid = 1;
    int noNonZero = 0;
    int numMembers = audioSceneConfig->asi.switchGroups[i].switchGroupNumMembers;
    int defaultID = audioSceneConfig->asi.switchGroups[i].switchGroupDefaultGroupID;
    int idx = -1;
    int memberID = -1;
    int switchGroupAllowOnOff = -1;

    for (k = 0; k < numMembers; k++)
    {
      idx = MP_getGrpIdxAudioScene(audioSceneConfig,audioSceneConfig->asi.switchGroups[i].switchGroupMemberID[k]);
      if (h_InteractConfig->onOffStatusModified[idx] == 1)
      {
        noNonZero++;
      }
    }
    if (noNonZero > 1)
    {
      /* if more than one member is activated -> error */
      isValid = 0;
    }
    if (noNonZero == 0)
    {
      switchGroupAllowOnOff = MP_getSwitchGroupAllowOnOff(audioSceneConfig,audioSceneConfig->asi.switchGroups[i].switchGroupID);
      if (switchGroupAllowOnOff == 1)
      {
        /* if no member is activated and the switchGroup is allowed to be completely switched off -> deactivate all members */
        isValid = 1;
      }
      else
      {
        /* if no member is activated and the switchGroup is not allowed to be completely switched off -> set default as active */
        idx = MP_getGrpIdxAudioScene(audioSceneConfig, defaultID);
        h_InteractConfig->onOffStatusModified[idx] = 1;
      }
    }
    if (isValid == 0)
    {
      fprintf(stderr, "Error: Violation of SwitchGroup Logic, two SwitchGroup members were defined to be active simultaneously.\n");
      for (k = 0; k < numMembers; k++)
      {
        memberID = audioSceneConfig->asi.switchGroups[i].switchGroupMemberID[k];
        if (memberID == defaultID)
        {
          idx = MP_getGrpIdxAudioScene(audioSceneConfig, defaultID);
          h_InteractConfig->onOffStatusModified[idx] = 1;
        }
        else
        {
          idx = MP_getGrpIdxAudioScene(audioSceneConfig, memberID);
          h_InteractConfig->onOffStatusModified[idx] = 0;
        }
      }
      fprintf(stderr, "Default Switch Group Member is activated, all other members are switched off.\n");
    }
  }
  return 0;
}

int MP_applyOnOffInteraction(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, int downmixId, int* selectedPreset)
{
  int error = 0;
  int i, k;
  int numSwitchGroups = audioSceneConfig->asi.numSwitchGroups;
  int presetIdx = -1;
  int chosenPreset = -1;
  int isValidPresetID = -1;
  int runTo = 0;
  int extensionIdx = -1;
  int e;
  int j;

  isValidPresetID = 0;

  if (h_InteractConfig->defaultPresetMode == 1)
  {
    int minPresetID = MAE_MAX_NUM_GROUP_PRESETS;
    chosenPreset = 0; /* choose ID 0 as default preset */

    /* if ID 0 is not present -> choose minimim preset ID */
    for (i = 0; i < audioSceneConfig->asi.numGroupPresets; i++)
    {
      if (audioSceneConfig->asi.groupPresets[i].presetID < minPresetID)
      {
        minPresetID = audioSceneConfig->asi.groupPresets[i].presetID;
        if (minPresetID == 0) break;
      }
    }
    chosenPreset = minPresetID;
  }
  else
  {
    /* get chosen preset from interaction data */
    if (h_InteractConfig->interactionDataToBeApplied == 1)
    {
      chosenPreset = h_InteractConfig->ElementInteractionData->groupPresetID;
    }
    else
    {
      chosenPreset = *selectedPreset;
    }
  }

  for (i = 0; i < audioSceneConfig->asi.numGroupPresets; i++)
  {
    if (audioSceneConfig->asi.groupPresets[i].presetID == chosenPreset)
    {
      presetIdx = i;
      isValidPresetID = 1;
      break;
    }
  }

  if (isValidPresetID != 1)
  {
    fprintf(stderr, "Error: PresetID does not exist! Old preset is kept. If no old preset is selected, error is returned\n");
    if (*selectedPreset > -1)
    {
      chosenPreset = *selectedPreset;
    }
    else
    {
      return -1;
    }
  }
  else
  {
    *selectedPreset = chosenPreset; /* overwrite selected preset only if valid ID */
  }

  for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
  {
    h_InteractConfig->onOffStatusModified[i] = -1;
  }

  /* loop over all groups to determine their on-off status */
  if (chosenPreset > -1)
  {
    for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
    {
      int grpIdx = -1;
      int presetCnd = -1;
      int grpID = audioSceneConfig->asi.groups[i].groupID;

      extensionIdx = -1;
      presetCnd = MP_getPresetCnd(audioSceneConfig,chosenPreset,grpID, downmixId, &extensionIdx);

      if ((presetCnd == 1) && (h_InteractConfig->onOffStatusModified[i] == -1))
      {
        h_InteractConfig->onOffStatusModified[i] = 1;
        if (MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups,audioSceneConfig->asi.switchGroups,grpID, NULL))
        {
          error = MP_applySwitchGroupOff(audioSceneConfig, grpID, &h_InteractConfig->onOffStatusModified);
          if (error != 0)
          {
            fprintf(stderr, "Error: Violation of SwitchGroup Logic in the chosen preset, two SwitchGroup members were defined to be active simultaneously.\n");
          }
        }
      }
      else if ((presetCnd == 0) && (h_InteractConfig->onOffStatusModified[i] == -1))
      {
        h_InteractConfig->onOffStatusModified[i] = 0;
      }
      else if ( presetCnd == -1)
      {
        /* if current group onoff status is not defined in the preset conditions... */
        grpIdx = i;
        if (audioSceneConfig->asi.groups[i].allowOnOff == 1)
        {
          if (h_InteractConfig->interactionDataToBeApplied == 1)
          {
            /* ... the onoff status of the interaction data should be used, if allowOnOff == 1 and interaction data is available */
            h_InteractConfig->onOffStatusModified[i] = h_InteractConfig->ElementInteractionData->GroupInteraction[i]->onOff;
          }
          else
          {
            /* ... the default onoff status should be used, if allowOnOff == 1 and no interaction data is available */
            h_InteractConfig->onOffStatusModified[i] = audioSceneConfig->asi.groups[grpIdx].defaultOnOff;
          }
        }
        else
        {
          /* ... the default onoff status of the interaction data should be used, if allowOnOff == 0 */
          h_InteractConfig->onOffStatusModified[i] = audioSceneConfig->asi.groups[grpIdx].defaultOnOff;
        }
      }
    }

    /* check for switch group conditions */
    if (downmixId > -1)
    {
      for (e = 0; e < audioSceneConfig->asi.groupPresetExtensions[presetIdx].numDownmixIdPresetExtensions; e++)
      {
        if (audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixId[e] == downmixId)
        {
          extensionIdx = e;
          break;
        }
      }
    }
    if (extensionIdx > -1)
    {
      runTo = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetNumConditions[extensionIdx];
    }
    else
    {
      runTo = audioSceneConfig->asi.groupPresets[presetIdx].presetNumConditions;
    }

    for (i = 0; i < runTo; i++) /* run over all conditions of chosen preset (preset extension) */
    {
      int cond = -1;
      int sgId = -1;
      if (extensionIdx > -1)
      {
        int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetIsSwitchGroupCondition[extensionIdx][i];
        if (isSgCond == 1)
        {
          cond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetConditionOnOff[extensionIdx][i];
          sgId = audioSceneConfig->asi.groupPresetExtensions[presetIdx].downmixIdPresetReferenceID[extensionIdx][i];
        }
      }
      else
      {
        int isSgCond = audioSceneConfig->asi.groupPresetExtensions[presetIdx].isSwitchGroupCondition[i];
        if (isSgCond == 1)
        {
          cond = audioSceneConfig->asi.groupPresets[presetIdx].presetConditionOnOff[i];
          sgId = audioSceneConfig->asi.groupPresets[presetIdx].presetGroupID[i];
        }
      }

      if (cond == 1) /* one member has to be activated, if not: default is activated */
      {
        int sgFound = 0;
        for (k = 0; k < numSwitchGroups; k++)
        {
          if (audioSceneConfig->asi.switchGroups[k].switchGroupID == sgId)
          {
            int noNonZero = 0;
            int idx = -1;
            int defaultID = audioSceneConfig->asi.switchGroups[k].switchGroupDefaultGroupID;
            sgFound = 1;

            for (j = 0; j < audioSceneConfig->asi.switchGroups[k].switchGroupNumMembers; j++)
            {
              idx = MP_getGrpIdxAudioScene(audioSceneConfig,audioSceneConfig->asi.switchGroups[i].switchGroupMemberID[k]);
              if (h_InteractConfig->onOffStatusModified[idx] == 1)
              {
                noNonZero++;
              }
            }
            if (noNonZero == 0)
            {
              idx = MP_getGrpIdxAudioScene(audioSceneConfig, defaultID);
              h_InteractConfig->onOffStatusModified[idx] = 1;
            }
            else if (noNonZero > 1)
            {
              fprintf(stderr, "Error: Violation of SwitchGroup Logic, two SwitchGroup members were defined to be active simultaneously.\n");
              break;
            }
          }
        }
        if (sgFound == 0)
        {
          fprintf(stderr, "Error: There is no switch group defined with the ID referenced in the switch group condition of the chosen preset.\n");
        }
      }
      else if (cond == 0) /* all members have to be switched off, if SG is allowed to be switched off. If not, the preset definition is not correct */
      {
        int sgFound = 0;
        for (k = 0; k < numSwitchGroups; k++)
        {
          if (audioSceneConfig->asi.switchGroups[k].switchGroupID == sgId)
          {
            int idx = -1;
            int allowOnOff = audioSceneConfig->asi.switchGroups[k].allowOnOff;

            sgFound = 1;

            if (allowOnOff == 0)
            {
              fprintf(stderr, "Error: Violation of SwitchGroup Logic, is is not allowed to switch it off completely. Preset definition is invalid\n");
              break;
            }
            else
            {
              for (j = 0; j < audioSceneConfig->asi.switchGroups[k].switchGroupNumMembers; j++)
              {
                idx = MP_getGrpIdxAudioScene(audioSceneConfig,audioSceneConfig->asi.switchGroups[i].switchGroupMemberID[k]);
                if (h_InteractConfig->onOffStatusModified[idx] == 1)
                {
                  h_InteractConfig->onOffStatusModified[idx] = 0;
                }
              }
            }
          }
        }
        if (sgFound == 0)
        {
          fprintf(stderr, "Error: There is no switch group defined with the ID referenced in the switch group condition of the chosen preset.\n");
        }
      }
    }
    error = MP_applySwitchGroupLogic(h_InteractConfig, audioSceneConfig);
  }
  else
  {
    if (h_InteractConfig->interactionType == 0) /* no preset chosen and advanced interaction mode is active */
    {
      for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
      {
        h_InteractConfig->onOffStatusModified[i] = -1;
      }

      /* loop over all groups */
      for (i = 0; i < h_InteractConfig->numDecodedGroups; i++)
      {
        int grpIdx = -1;
        int grpID = audioSceneConfig->asi.groups[i].groupID;
        int allowOnOff = audioSceneConfig->asi.groups[i].allowOnOff;
        int temp = 0;

        if (allowOnOff)
        {
          grpIdx = MP_getGrpIdxElementInteraction(h_InteractConfig->ElementInteractionData,grpID);
          if (grpIdx > -1)
          {
            h_InteractConfig->onOffStatusModified[i] = h_InteractConfig->ElementInteractionData->GroupInteraction[grpIdx]->onOff;
            temp = MP_isSwitchGroupMember(audioSceneConfig->asi.numSwitchGroups,audioSceneConfig->asi.switchGroups,grpID, NULL);
            if ((h_InteractConfig->onOffStatusModified[i] == 1) && (temp == 1))
            {
              error = MP_applySwitchGroupOff(audioSceneConfig, grpID, &h_InteractConfig->onOffStatusModified);
              if (error != 0)
              {
                fprintf(stderr, "Warning: Violation of SwitchGroup Logic, two SwitchGroup members were defined to be active simultaneously.\n");
              }
            }
          }
          else
          {
            h_InteractConfig->onOffStatusModified[i] = audioSceneConfig->asi.groups[i].defaultOnOff;
          }
        }
        else
        {
          h_InteractConfig->onOffStatusModified[i] = audioSceneConfig->asi.groups[i].defaultOnOff;
        }
      }
      error = MP_applySwitchGroupLogic(h_InteractConfig,audioSceneConfig);
    }
  }
  return error;
}

int MP_decodeEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int numObjects, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhObjMdConfig, int frameNumber, int* isPresent)
{
  int error = 0;
  int i, j;
  char filename[FILENAME_MAX];
  FILE* fptr;
  int independencyFlag = 0;

  unsigned char* data_read = NULL;

  long	nBytes, nBits;
	robitbuf readbitBuffer;
	robitbufHandle hBitstream = &readbitBuffer;

  /* open file */
  sprintf(filename, "%s_%i.dat", h_enhObjMdFrame->path, frameNumber);
	fptr = fopen(filename, "rb");
	if (fptr == NULL) {
		/* file not existing */
    /* do not read new frame */
    *isPresent = 0;
    return -1;
	}
  else
  {
    *isPresent = 1;

    /* push current frame to the old frame structure */
    for (i = 0; i < numObjects; i++)
    {
      h_enhObjMdFrame->lastFrame_diffuseness[i] = h_enhObjMdFrame->diffuseness[i];
      h_enhObjMdFrame->lastFrame_divergence[i] = h_enhObjMdFrame->divergence[i];
      h_enhObjMdFrame->lastFrame_numExclusionSectors[i] = h_enhObjMdFrame->numExclusionSectors[i];

      for (j = 0; j < MAE_MAX_NUM_EXCLUDED_SECTORS; j++)
      {
        h_enhObjMdFrame->lastFrame_excludeSectorIndex[i][j] = h_enhObjMdFrame->excludeSectorIndex[i][j];
        h_enhObjMdFrame->lastFrame_excludeSectorMaxAzimuth[i][j] = h_enhObjMdFrame->excludeSectorMaxAzimuth[i][j];
        h_enhObjMdFrame->lastFrame_excludeSectorMinAzimuth[i][j] = h_enhObjMdFrame->excludeSectorMinAzimuth[i][j];
        h_enhObjMdFrame->lastFrame_excludeSectorMaxElevation[i][j] = h_enhObjMdFrame->excludeSectorMaxElevation[i][j];
        h_enhObjMdFrame->lastFrame_excludeSectorMinElevation[i][j] = h_enhObjMdFrame->excludeSectorMinElevation[i][j];
      }
    }

    /* read new frame */

    /* get file size */
	  fseek(fptr, 0L, SEEK_END);
	  nBytes = ftell(fptr);
	  fseek(fptr, 0L, SEEK_SET);
	  nBits = 8 * nBytes;

    /* init buffer with data from file */
	  data_read = (unsigned char*)calloc(nBytes, sizeof(unsigned char));
	  if( data_read == NULL ){
		  return -1;
	  }
    fread(data_read, nBytes, sizeof(char), fptr);
	  fclose(fptr);
    robitbuf_Init(hBitstream, data_read, nBits, 0);


    if ((enhObjMdConfig->hasDiffuseness) && (enhObjMdConfig->hasCommonGroupDiffuseness))
    {
      if (independencyFlag == 0)
      {
        h_enhObjMdFrame->keepDiffuseness[0] = robitbuf_ReadBits(hBitstream, 1);
      }
      else
      {
        h_enhObjMdFrame->keepDiffuseness[0] = 0;
      }
      if (h_enhObjMdFrame->keepDiffuseness[0] == 0)
      {
        h_enhObjMdFrame->diffuseness[0] = (robitbuf_ReadBits(hBitstream, 7))/127.0f;
      }
      else /* keepDiffuseness == 1*/
      {
        h_enhObjMdFrame->diffuseness[0] = h_enhObjMdFrame->lastFrame_diffuseness[0];
      }
    }

    if (enhObjMdConfig->hasCommonGroupExcludedSectors)
    {
      if (independencyFlag == 0)
      {
        h_enhObjMdFrame->keepExclusion[0] = robitbuf_ReadBits(hBitstream, 1);
      }
      else
      {
        h_enhObjMdFrame->keepExclusion[0] = 0;
      }
      if (h_enhObjMdFrame->keepExclusion[0] == 0)
      {
        h_enhObjMdFrame->numExclusionSectors[0] = robitbuf_ReadBits(hBitstream,4);
        if (enhObjMdConfig->useOnlyPredefinedSectors[0] == 1)
        {
          for (i = 0; i < h_enhObjMdFrame->numExclusionSectors[0]; i++)
          {
            h_enhObjMdFrame->excludeSectorIndex[0][i] = robitbuf_ReadBits(hBitstream,4);
          }
        }
        else
        {
          for (i = 0; i < h_enhObjMdFrame->numExclusionSectors[0]; i++)
          {
            h_enhObjMdFrame->usePredefinedSector[0][i] = robitbuf_ReadBits(hBitstream,1);
            if (h_enhObjMdFrame->usePredefinedSector[0][i] == 1)
            {
              h_enhObjMdFrame->excludeSectorIndex[0][i] = robitbuf_ReadBits(hBitstream,4);
            }
            else
            {
              int tmp = 0;
              tmp = robitbuf_ReadBits(hBitstream,7);
              h_enhObjMdFrame->excludeSectorMinAzimuth[0][i] = min(max((float)3.0f*(tmp-63.0f),-180.0f), 180.0f);
              tmp = robitbuf_ReadBits(hBitstream,7);
              h_enhObjMdFrame->excludeSectorMaxAzimuth[0][i] = min(max((float)3.0f*(tmp-63.0f),-180.0f), 180.0f);
              tmp = robitbuf_ReadBits(hBitstream,5);
              h_enhObjMdFrame->excludeSectorMinElevation[0][i] = min(max((float)6.0f*(tmp-15.0f),-90.0f), 90.0f);
              tmp = robitbuf_ReadBits(hBitstream,5);
              h_enhObjMdFrame->excludeSectorMaxElevation[0][i] = min(max((float)6.0f*(tmp-15.0f),-90.0f), 90.0f);
            }
          }
        }
      }
      else /* keepExclusion == 1 */
      {
        h_enhObjMdFrame->numExclusionSectors[0] = h_enhObjMdFrame->lastFrame_numExclusionSectors[0];
        for (i = 0; i < h_enhObjMdFrame->numExclusionSectors[0]; i++)
        {
          h_enhObjMdFrame->excludeSectorIndex[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorIndex[0][i];
          h_enhObjMdFrame->usePredefinedSector[0][i] = h_enhObjMdFrame->lastFrame_usePredefinedSector[0][i];
          h_enhObjMdFrame->excludeSectorIndex[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorIndex[0][i];
          h_enhObjMdFrame->excludeSectorMinAzimuth[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorMinAzimuth[0][i];
          h_enhObjMdFrame->excludeSectorMaxAzimuth[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorMaxAzimuth[0][i];
          h_enhObjMdFrame->excludeSectorMinElevation[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorMinElevation[0][i];
          h_enhObjMdFrame->excludeSectorMaxElevation[0][i] = h_enhObjMdFrame->lastFrame_excludeSectorMaxElevation[0][i];
        }
      }
    }

    for (i = 0; i < numObjects; i++)
    {
      h_enhObjMdFrame->closestSpeakerPlayout[i] =  robitbuf_ReadBits(hBitstream,1);

      if ((enhObjMdConfig->hasDiffuseness) && (enhObjMdConfig->hasCommonGroupDiffuseness == 0))
      {
        if (independencyFlag == 0)
        {
          h_enhObjMdFrame->keepDiffuseness[i] = robitbuf_ReadBits(hBitstream, 1);
        }
        else
        {
          h_enhObjMdFrame->keepDiffuseness[i] = 0;
        }
        if (h_enhObjMdFrame->keepDiffuseness[i] == 0)
        {
          h_enhObjMdFrame->diffuseness[i] = (robitbuf_ReadBits(hBitstream, 7))/127.0f;
        }
        else /* keepDiffuseness[i] == 1 */
        {
          h_enhObjMdFrame->diffuseness[i] = h_enhObjMdFrame->lastFrame_diffuseness[i];
        }
      }

      if (enhObjMdConfig->hasDivergence[i] == 1)
      {
        if (independencyFlag == 0)
        {
          h_enhObjMdFrame->keepDivergence[i] = robitbuf_ReadBits(hBitstream, 1);
        }
        else
        {
          h_enhObjMdFrame->keepDivergence[i] = 0;
        }
        if (h_enhObjMdFrame->keepDivergence[i] == 0)
        {
          h_enhObjMdFrame->divergence[i] = (robitbuf_ReadBits(hBitstream, 7))/127.0f;
        }
        else /* keepDivergence[i] == 1 */
        {
          h_enhObjMdFrame->divergence[i] = h_enhObjMdFrame->lastFrame_divergence[i];
        }
      }

      if (enhObjMdConfig->hasCommonGroupExcludedSectors == 0)
      {
        if (independencyFlag == 0)
        {
          h_enhObjMdFrame->keepExclusion[i] = robitbuf_ReadBits(hBitstream, 1);
        }
        else
        {
          h_enhObjMdFrame->keepExclusion[i] = 0;
        }
        if (h_enhObjMdFrame->keepExclusion[i] == 0)
        {
          h_enhObjMdFrame->numExclusionSectors[i] = robitbuf_ReadBits(hBitstream,4);
          if (enhObjMdConfig->useOnlyPredefinedSectors[i] == 1)
          {
            for (j = 0; j < h_enhObjMdFrame->numExclusionSectors[i]; j++)
            {
              h_enhObjMdFrame->excludeSectorIndex[i][j] = robitbuf_ReadBits(hBitstream,4);
            }
          }
          else
          {
            for (j = 0; j < h_enhObjMdFrame->numExclusionSectors[i]; j++)
            {
              h_enhObjMdFrame->usePredefinedSector[i][j] = robitbuf_ReadBits(hBitstream,1);
              if (h_enhObjMdFrame->usePredefinedSector[i][j] == 1)
              {
                h_enhObjMdFrame->excludeSectorIndex[i][j] = robitbuf_ReadBits(hBitstream,4);
              }
              else
              {
                int tmp = 0;
                tmp = robitbuf_ReadBits(hBitstream,7);
                h_enhObjMdFrame->excludeSectorMinAzimuth[i][j] = min(max((float)3.0f*(tmp-63.0f),-180.0f), 180.0f);
                tmp = robitbuf_ReadBits(hBitstream,7);
                h_enhObjMdFrame->excludeSectorMaxAzimuth[i][j] = min(max((float)3.0f*(tmp-63.0f),-180.0f), 180.0f);
                tmp = robitbuf_ReadBits(hBitstream,5);
                h_enhObjMdFrame->excludeSectorMinElevation[i][j] = min(max((float)6.0f*(tmp-15.0f),-90.0f), 90.0f);
                tmp = robitbuf_ReadBits(hBitstream,5);
                h_enhObjMdFrame->excludeSectorMaxElevation[i][j] = min(max((float)6.0f*(tmp-15.0f),-90.0f), 90.0f);
              }
            }
          }
        }
        else /* keepExclusion[i] == 1 */
        {
          h_enhObjMdFrame->numExclusionSectors[i] = h_enhObjMdFrame->lastFrame_numExclusionSectors[i];
          for (j = 0; j < h_enhObjMdFrame->numExclusionSectors[i]; j++)
          {
            h_enhObjMdFrame->excludeSectorIndex[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorIndex[i][j];
            h_enhObjMdFrame->usePredefinedSector[i][j] = h_enhObjMdFrame->lastFrame_usePredefinedSector[i][j];
            h_enhObjMdFrame->excludeSectorIndex[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorIndex[i][j];
            h_enhObjMdFrame->usePredefinedSector[i][j] = h_enhObjMdFrame->lastFrame_usePredefinedSector[i][j];
            h_enhObjMdFrame->excludeSectorIndex[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorIndex[i][j];
            h_enhObjMdFrame->excludeSectorMinAzimuth[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorMinAzimuth[i][j];
            h_enhObjMdFrame->excludeSectorMaxAzimuth[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorMaxAzimuth[i][j];
            h_enhObjMdFrame->excludeSectorMinElevation[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorMinElevation[i][j];
            h_enhObjMdFrame->excludeSectorMaxElevation[i][j] = h_enhObjMdFrame->lastFrame_excludeSectorMaxElevation[i][j];
          }
        }
      }
    }
  }

  free(data_read);
  return error;
}

int MP_getOamSample(FILE* oam_file, StructOamMultidata* oam_sample, unsigned int oam_version, unsigned int hasDynPrio, unsigned int hasUniformSpread)
{
  uint64_t sample_pos;
  int num_objects = oam_sample->size1;

  sample_pos = oam_sample->sample[0];

  oam_read_process(oam_file,
                    oam_sample,
                    &num_objects,
                    oam_version,
                    (DYNAMIC_OBJECT_PRIO)hasDynPrio,
                    hasUniformSpread);

  return (int)(oam_sample->sample[0] - sample_pos);
}

int MP_getGrpIdxElementInteraction(H_ELEMENT_INTERACTION_DATA ElementInteractionData, int grpID)
{
  int k;
  int grpIdx = -1;
  for (k = 0; k < ElementInteractionData->numGroups; k++)
  {
    if (ElementInteractionData->GroupInteraction[k]->groupID == grpID)
    {
      grpIdx = k;
      break;
    }
  }
  return grpIdx;
}

int MP_getGrpIdxAudioScene(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int grpID)
{
  int k;
  int grpIdx = -1;
  for (k = 0; k < audioSceneConfig->asi.numGroups; k++)
  {
    if (audioSceneConfig->asi.groups[k].groupID == grpID)
    {
      grpIdx = k;
      break;
    }
  }
  return grpIdx;
}

int MP_getSwitchGroupAllowOnOff(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int switchGroupId)
{
  int k;
  int sgIdx = -1;
  int cond = -1;
  for (k = 0; k < audioSceneConfig->asi.numSwitchGroups; k++)
  {
    if (audioSceneConfig->asi.switchGroups[k].switchGroupID == switchGroupId)
    {
      sgIdx = k;
      break;
    }
  }

  if (sgIdx > -1)
  {
    cond = audioSceneConfig->asi.switchGroups[sgIdx].allowOnOff;
  }

  return cond;
}

int MP_getPresetCnd(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int presetID, int grpID, int downmixID, int* extensionIdx)
{
  int k;
  int i;
  int pstIdx = -1;
  int grpIdx = -1;
  int cond = -1;
  *extensionIdx = -1;

  for (k = 0; k < audioSceneConfig->asi.numGroupPresets; k++)
  {
    if (audioSceneConfig->asi.groupPresets[k].presetID == presetID)
    {
      pstIdx = k;
      if (audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions > 0)
      {
        for (i = 0; i < audioSceneConfig->asi.groupPresetExtensions[k].numDownmixIdPresetExtensions; i++)
        {
          if (audioSceneConfig->asi.groupPresetExtensions[k].downmixId[i] == downmixID)
          {
            *extensionIdx = i;
            break;
          }
        }
      }
      break;
    }
  }

  if (pstIdx > -1)
  {
    if (*extensionIdx > -1)
    {
      for (k = 0; k < audioSceneConfig->asi.groupPresetExtensions[pstIdx].downmixIdPresetNumConditions[*extensionIdx]; k++)
      {
        if ((audioSceneConfig->asi.groupPresetExtensions[pstIdx].downmixIdPresetReferenceID[*extensionIdx][k] == grpID) && (audioSceneConfig->asi.groupPresetExtensions[pstIdx].downmixIdPresetIsSwitchGroupCondition[*extensionIdx][k] == 0))
        {
          grpIdx = k;
          break;
        }
      }
    }
    else
    {
      for (k = 0; k < audioSceneConfig->asi.groupPresets[pstIdx].presetNumConditions; k++)
      {
        if (audioSceneConfig->asi.groupPresets[pstIdx].presetGroupID[k] == grpID)
        {
          grpIdx = k;
          break;
        }
      }
    }
    if (grpIdx > -1)
    {
      if (*extensionIdx > -1)
      {
        cond = audioSceneConfig->asi.groupPresetExtensions[pstIdx].downmixIdPresetConditionOnOff[*extensionIdx][grpIdx];
      }
      else
      {
        cond = audioSceneConfig->asi.groupPresets[pstIdx].presetConditionOnOff[grpIdx];
      }
    }
  }
  return cond;
}

int MP_isSwitchGroupMember(int numSG, SwitchGroupDefinition* sgDef, int grpID, int* sgID)
{
  int i,k;
  int isMember = 0;

  for (k = 0; k < numSG; k++)
  {
    int numMembers = sgDef[k].switchGroupNumMembers;
    for (i = 0; i < numMembers; i++)
    {
      if (sgDef[k].switchGroupMemberID[i] == grpID)
      {
        isMember = 1;
        if (sgID != NULL)
        {
          *sgID = sgDef[k].switchGroupID;
        }
      }
    }
  }
  return isMember;
}

int MP_applySwitchGroupOff(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int grpID, int** onOffStatusModified)
{
  int i,k;
  int numSG = audioSceneConfig->asi.numSwitchGroups;
  int switchGroupIdx = 0;
  int numMembers;
  int grpID2;

  if (numSG > 1)
  {
    for (k = 0; k < numSG; k++)
    {
      numMembers = audioSceneConfig->asi.switchGroups[k].switchGroupNumMembers;
      for (i = 0; i < numMembers; i++)
      {
        if (audioSceneConfig->asi.switchGroups[k].switchGroupMemberID[i] == grpID)
        {
          switchGroupIdx = k;
          break;
        }
      }
      if (switchGroupIdx > 0)
      {
        break;
      }
    }
  }

  numMembers = audioSceneConfig->asi.switchGroups[switchGroupIdx].switchGroupNumMembers;
  for (k = 0; k < numMembers; k++)
  {
    int grpIdx = -1;

    grpID2 = audioSceneConfig->asi.switchGroups[switchGroupIdx].switchGroupMemberID[k];

    for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
    {
      if ( audioSceneConfig->asi.groups[i].groupID == grpID2)
      {
        grpIdx = i;
        break;
      }
    }

    if (grpID2 == grpID)
    {
      (*onOffStatusModified)[grpIdx] = 1;
    }
    else
    {
      if ((*onOffStatusModified)[grpIdx] == 1)
      {
        /* error, two switch group members were set to onoff=1*/
        return -1;
      }
      else
      {
        (*onOffStatusModified)[grpIdx] = 0;
      }
    }
  }
  return 0;
}

void MP_initOamConfig(H_OAM_MP_CONFIG* h_OamConfig, char* oamInpath, char* oamOutpath, int oam_mode, int decodedOAM, int hasDynOP, int hasUniformSpread)
{
  H_OAM_MP_CONFIG temp = (H_OAM_MP_CONFIG)calloc(1, sizeof(struct MP_OamConfigStruct));

  if (NULL != temp) {
    temp->oamInFile = NULL;
    temp->oamOutFile = NULL;

    strncpy(temp->oamInpath, oamInpath, sizeof(temp->oamInpath));
    strncpy(temp->oamOutpath, oamOutpath, sizeof(temp->oamOutpath));

    temp->num_objects_input = 0;
    temp->num_objects_output = 0;
    temp->oam_blocksize = 0;
    temp->oam_frames = 0;
    temp->oam_mode = oam_mode;

    temp->oam_hasBeenDecoded = decodedOAM;
    temp->hasDynObjectPrio = hasDynOP;
    temp->hasUniformSpread = hasUniformSpread;
  }

  *h_OamConfig = temp;
}

int MP_freeConfigStructs(H_AUDIO_MP_CONFIG h_AudioConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OAM_MP_CONFIG h_OamConfig)
{
  int error = 0;
  int k, j;


  for (k = 0; k < (int)h_AudioConfig->nInChannels; k++)
  {
    free(h_AudioConfig->audioTdBuffer[k]); h_AudioConfig->audioTdBuffer[k] = NULL;
  }
  free(h_AudioConfig->audioTdBuffer); h_AudioConfig->audioTdBuffer = NULL;


  if (h_AudioConfig->decode_qmf)
  {
    for(k = 0; k < h_AudioConfig->numTimeslots; k++ )
	  {
		  for (j = 0; j < (int)h_AudioConfig->nInChannels; j++)
		  {
			  free(h_AudioConfig->audioQmfBuffer_real[k][j]); h_AudioConfig->audioQmfBuffer_real[k][j] = NULL;
			  free(h_AudioConfig->audioQmfBuffer_imag[k][j]); h_AudioConfig->audioQmfBuffer_imag[k][j] = NULL;
		  }
      free(h_AudioConfig->audioQmfBuffer_real[k]); h_AudioConfig->audioQmfBuffer_real[k] = NULL;
		  free(h_AudioConfig->audioQmfBuffer_imag[k]); h_AudioConfig->audioQmfBuffer_imag[k] = NULL;
	  }
    free(h_AudioConfig->audioQmfBuffer_real); h_AudioConfig->audioQmfBuffer_real = NULL;
    free(h_AudioConfig->audioQmfBuffer_imag); h_AudioConfig->audioQmfBuffer_imag = NULL;
  }

  if (h_AudioConfig->nOutChannels > h_AudioConfig->nInChannels)
  {
    if (h_AudioConfig->decode_wav)
    {
      for (k = 0; k < (int)h_AudioConfig->nOutChannels; k++)
      {
        free(h_AudioConfig->audioTdBuffer_divergence[k]); h_AudioConfig->audioTdBuffer_divergence[k] = NULL;
      }
      free(h_AudioConfig->audioTdBuffer_divergence); h_AudioConfig->audioTdBuffer_divergence = NULL;
    }
    if (h_AudioConfig->decode_qmf)
    {
      for(k = 0; k < h_AudioConfig->numTimeslots; k++ )
      {
	      for (j = 0; j < (int)h_AudioConfig->nOutChannels; j++)
	      {
		      free(h_AudioConfig->audioQmfBuffer_divergence_real[k][j]); h_AudioConfig->audioQmfBuffer_divergence_real[k][j] = NULL;
		      free(h_AudioConfig->audioQmfBuffer_divergence_imag[k][j]); h_AudioConfig->audioQmfBuffer_divergence_imag[k][j] = NULL;
	      }
        free(h_AudioConfig->audioQmfBuffer_divergence_real[k]); h_AudioConfig->audioQmfBuffer_divergence_real[k] = NULL;
	      free(h_AudioConfig->audioQmfBuffer_divergence_imag[k]); h_AudioConfig->audioQmfBuffer_divergence_imag[k] = NULL;
      }
      free(h_AudioConfig->audioQmfBuffer_divergence_real); h_AudioConfig->audioQmfBuffer_divergence_real = NULL;
      free(h_AudioConfig->audioQmfBuffer_divergence_imag); h_AudioConfig->audioQmfBuffer_divergence_imag = NULL;
    }
  }

  if (h_InteractConfig->gainModified_LastFrame != NULL)
  {
    free(h_InteractConfig->gainModified_LastFrame);
    h_InteractConfig->gainModified_LastFrame = NULL;
  }

  if (h_InteractConfig->onOffStatusModified != NULL)
  {
    free(h_InteractConfig->onOffStatusModified);
    h_InteractConfig->onOffStatusModified = NULL;
  }

  if (h_InteractConfig->routeToWireID != NULL)
  {
    free(h_InteractConfig->routeToWireID);
    h_InteractConfig->routeToWireID = NULL;
  }

  if (h_InteractConfig->gainModifiedGroup != NULL)
  {
    free(h_InteractConfig->gainModifiedGroup);
    h_InteractConfig->gainModifiedGroup = NULL;
  }

  h_InteractConfig->oamSampleModified_Divergence = oam_multidata_destroy(h_InteractConfig->oamSampleModified_Divergence);
  h_InteractConfig->oamSampleModified_Divergence = NULL;
  if ( h_InteractConfig->oamSampleModified != NULL)
  {
    h_InteractConfig->oamSampleModified = oam_multidata_destroy(h_InteractConfig->oamSampleModified);
    h_InteractConfig->oamSampleModified = NULL;
  }

  free(h_AudioConfig);
  free(h_OamConfig);
  free(h_InteractConfig);

  return error;
}

static __inline int MP_isLittleEndian (void)
{
  short s = 0x01 ;

  return *((char *) &s) ? 1 : 0;
}

static void MP_convertFloatToInt(char *OutBuf, float *InBuf, unsigned int length, unsigned int nBytesPerSample)
{
  unsigned int j;

  if ( nBytesPerSample == 4 ) {
    memcpy(OutBuf, InBuf, length*sizeof(char));
  }
  else if (nBytesPerSample == sizeof(short))
  {
    union { signed short s; char c[sizeof(short)]; } u;
    int i;
    float fFactor   = (float)(1 << ( nBytesPerSample*8 - 1 ));
    u.s			    = 0;
    for (j=0; j < length / nBytesPerSample; j++)
    {
      float maxVal  =  32767.f;
      float minVal  = -32768.f;
      float tmpVal  = 0.f;

      int n = j * nBytesPerSample;

      tmpVal = InBuf[j] * fFactor;

      if ( tmpVal > maxVal ) {
        tmpVal = maxVal;
      }
      if ( tmpVal < minVal ) {
        tmpVal = minVal;
      }

      u.s = (signed short) tmpVal;

      if (MP_isLittleEndian())
      {
        for (i=0; i< (int)nBytesPerSample; i++)
        {
          OutBuf[n + i] = u.c[i];
        }
      }
      else
      {
        for (i = 0; i < (int)nBytesPerSample; i++)
        {
          OutBuf[n + nBytesPerSample - i - 1] = u.c[i + (sizeof(short) - (sizeof(short)-1))];
        }
      }
    }
  }
  else
  {
    union { signed long s; char c[sizeof(long)]; } u;
    int i;

    /* Calculate scaling factor for 24bit */
    float fFactor   = (float)(1 << ( nBytesPerSample*8 - 1 ));
    u.s			    = 0;
    for (j=0; j < length / nBytesPerSample; j++)
    {
      int maxVal = (int) fFactor - 1;
      int minVal = (int) -fFactor;

      int n = j * nBytesPerSample;

      u.s = (signed long) (InBuf[j] * fFactor);

      if ( u.s > maxVal ) {
        u.s = maxVal;
      }
      if ( u.s < minVal ) {
        u.s = minVal;
      }

      if (MP_isLittleEndian())
      {
        for (i=0;i< (int)nBytesPerSample; i++)
        {
          OutBuf[n + i] = u.c[i];
        }
      }
      else
      {
        for (i = 0; i < (int)nBytesPerSample; i++)
        {
          OutBuf[n + nBytesPerSample - i - 1] = u.c[i + (sizeof(long) - 3)];
        }
      }
    }
  }
}

int MP_writeObjectOutputData(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OAM_MP_CONFIG h_OamConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig, int frameNumber, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int divergenceObjectsAdded)
{
  int error = 0;

  unsigned char *bitbuf;
  int k, j, s, f;
  long	nBytes, nBits;
  unsigned char temp;
  int objIndex[MAE_MAX_NUM_ELEMENTS] = {0};
  int elementID = -1;
  int groupID = -1;
  int onOff = 0;
  int numOAMframes = 0;
  int objCount = 0;
  int groupIdx = -1;
  float temp_diff = 0.0f;
  float temp_div = 0.0f;
  float az_range = 0.0f;
  int ct = 0;

  wobitbuf writebitBuffer;
  wobitbufHandle hBitstream = &writebitBuffer;

  h_OoiConfig->framewise_numOutObjects = 0;

  /* write all bitstream file into the bitbuffer */
  nBytes = BITSTREAM_MAX_NUM_MEGA_BYTES * 1024 * 1024;
  nBits = 8 * nBytes;
  bitbuf = (unsigned char*)malloc(nBytes * sizeof(unsigned char));
  wobitbuf_Init(hBitstream, bitbuf, nBits, 0);

  /* write frame configuration */
  temp = h_OoiConfig->audioFrameLength >> 6;
  wobitbuf_WriteBits(hBitstream, temp, 6);
  wobitbuf_WriteBits(hBitstream, 0, 2); /* audioTruncation = 0*/

  /* write object metadata */
  for (k = 0; k < h_InteractConfig->numElements_out; k++)
  {
    elementID = k;
    groupID = MP_getGroupID(audioSceneConfig, elementID);
    for (j = 0; j < audioSceneConfig->asi.numGroups; j++)
    {
      if (audioSceneConfig->asi.groups[j].groupID == groupID)
      {
        onOff = h_InteractConfig->onOffStatusModified[j];
        break;
      }
    }
    if (MP_getSignalGroupType(audioSceneConfig,signalGroupConfig,j) == 1)
    {
      if (onOff == 1)
      {
        objIndex[h_OoiConfig->framewise_numOutObjects] = objCount;
        h_OoiConfig->framewise_numOutObjects++;
      }
      objCount++;
    }
  }
  wobitbuf_WriteBits(hBitstream, h_OoiConfig->framewise_numOutObjects, 9);

  /* loop over all objects */
  for (k = 0; k < h_InteractConfig->numElements_out; k++)
  {
    elementID = k;
    groupID = MP_getGroupID(audioSceneConfig, elementID);
    for (j = 0; j < audioSceneConfig->asi.numGroups; j++)
    {
      if (audioSceneConfig->asi.groups[j].groupID == groupID)
      {
        onOff = h_InteractConfig->onOffStatusModified[j];
        groupIdx = j;
        break;
      }
    }
    if (MP_getSignalGroupType(audioSceneConfig,signalGroupConfig,j) == 1)
    {
      if (onOff == 1)
      {
        ct++;

        wobitbuf_WriteBits(hBitstream, elementID, 9);
        wobitbuf_WriteBits(hBitstream, h_OamConfig->hasDynObjectPrio, 1);
        wobitbuf_WriteBits(hBitstream, h_OamConfig->hasUniformSpread, 1);

        numOAMframes = 1;
        wobitbuf_WriteBits(hBitstream,numOAMframes,8);

        for (f = 0; f < numOAMframes; f++)
        {
          wobitbuf_WriteBits(hBitstream, 1, 1); /* object metadata present = 1 */
          wobitbuf_WriteBits(hBitstream, (unsigned int)floor(h_InteractConfig->oamSampleModified->azimuth[objIndex[ct-1]]/1.5f + 0.5f),8);
          wobitbuf_WriteBits(hBitstream, (unsigned int)floor(h_InteractConfig->oamSampleModified->elevation[objIndex[ct-1]]/3.0f + 0.5f),6);
          wobitbuf_WriteBits(hBitstream, (unsigned int)floor(3.0f *(log(h_InteractConfig->oamSampleModified->radius[objIndex[ct-1]])/log(2.0f))+0.5f),4);
          wobitbuf_WriteBits(hBitstream, (unsigned int)floor(2.0f * 20.0f*log(h_InteractConfig->oamSampleModified->gain[objIndex[ct-1]])+96.0f +0.5f),4);

          wobitbuf_WriteBits(hBitstream, h_OamConfig->hasDynObjectPrio,1);
          if (h_OamConfig->hasDynObjectPrio)
          {
            wobitbuf_WriteBits(hBitstream, (unsigned int)h_InteractConfig->oamSampleModified->dynamic_object_priority[objIndex[ct-1]],3);
          }
          wobitbuf_WriteBits(hBitstream, h_OamConfig->hasUniformSpread,1);
          if (h_OamConfig->hasUniformSpread)
          {
            wobitbuf_WriteBits(hBitstream, (unsigned int)floor(h_InteractConfig->oamSampleModified->spread[objIndex[ct-1]] / 1.5f + 0.5f),7);
          }
          else
          {
            wobitbuf_WriteBits(hBitstream, (unsigned int)floor(h_InteractConfig->oamSampleModified->spread[objIndex[ct-1]] / 1.5f + 0.5f),7);
            wobitbuf_WriteBits(hBitstream, (unsigned int)floor(h_InteractConfig->oamSampleModified->spread_height[objIndex[ct-1]] / 3.0f + 0.5f),5);
            wobitbuf_WriteBits(hBitstream, (unsigned int)floor(3.0f *(log(h_InteractConfig->oamSampleModified->spread_depth[objIndex[ct-1]])/log(2.0f))+0.5f),4);
          }
        }

        wobitbuf_WriteBits(hBitstream, signalGroupInfo->fixedPosition[MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig, groupIdx, elementID)],1);
        wobitbuf_WriteBits(hBitstream, audioSceneConfig->asi.groups[groupIdx].groupPriority, 3);

        /* diffuseness */
        if ((enhancedObjectMetadataConfig->hasDiffuseness) && (enhancedObjectMetadataConfig->hasCommonGroupDiffuseness))
        {
          temp_diff = h_enhObjMdFrame->diffuseness[0];
        }
        else
        {
          if ((enhancedObjectMetadataConfig->hasDiffuseness) && (enhancedObjectMetadataConfig->hasCommonGroupDiffuseness == 0))
          {
            temp_diff = h_enhObjMdFrame->diffuseness[objCount];
          }
        }
        wobitbuf_WriteBits(hBitstream, (unsigned int)floor(temp_diff * 127.0f + 0.5f), 7);

        /* divergence */
        if (enhancedObjectMetadataConfig->hasDivergence[objCount])
        {
          temp_div = h_enhObjMdFrame->divergence[objCount];
          az_range = enhancedObjectMetadataConfig->divergenceAzimuthRange[objCount];
        }
        wobitbuf_WriteBits(hBitstream, (unsigned int)floor(temp_div * 127.0f + 0.5f), 7);
        wobitbuf_WriteBits(hBitstream, (unsigned int)floor(az_range/3.0f + 0.5f), 6);

        /* excluded sectors */
        {
          int numSectors = 0;
          int usePredefSect = 0;
          int sectIdx = 0;
          if (enhancedObjectMetadataConfig->hasExcludedSectors)
          {
            if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors)
            {
              numSectors = h_enhObjMdFrame->numExclusionSectors[0];
            }
            else
            {
              numSectors = h_enhObjMdFrame->numExclusionSectors[objCount];
            }
          }
          else
          {
            numSectors = 0;
          }
          wobitbuf_WriteBits(hBitstream,numSectors,4);
          if (numSectors > 0)
          {
            for (s = 0; s < numSectors; s++)
            {
              if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors)
              {
                usePredefSect = h_enhObjMdFrame->usePredefinedSector[0][s];
              }
              else
              {
                usePredefSect = h_enhObjMdFrame->usePredefinedSector[objCount][s];
              }
              wobitbuf_WriteBits(hBitstream,usePredefSect,1);
              if (usePredefSect == 1)
              {
                if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors)
                {
                  sectIdx = h_enhObjMdFrame->excludeSectorIndex[0][s];
                }
                else
                {
                  sectIdx = h_enhObjMdFrame->excludeSectorIndex[objCount][s];
                }
                wobitbuf_WriteBits(hBitstream,sectIdx,4);
              }
              else
              {
                if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors)
                {
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMinAzimuth[0][s]/3.0f)+63.0f +0.5f),7);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMaxAzimuth[0][s]/3.0f)+63.0f +0.5f),7);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMinElevation[0][s]/6.0f)+15.0f +0.5f),5);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMaxElevation[0][s]/6.0f)+15.0f +0.5f),5);
                }
                else
                {
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMinAzimuth[objCount][s]/3.0f)+63.0f +0.5f),7);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMaxAzimuth[objCount][s]/3.0f)+63.0f +0.5f),7);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMinElevation[objCount][s]/6.0f)+15.0f +0.5f),5);
                  wobitbuf_WriteBits(hBitstream,(unsigned int)floor((h_enhObjMdFrame->excludeSectorMaxElevation[objCount][s]/6.0f)+15.0f +0.5f),5);
                }
              }
            }
          }
        }
      }
    }
  }

  /* align bytes */
  wobitbuf_ByteAlign(hBitstream);

  /* write bitbuffer to file */
  nBits = wobitbuf_GetBitsWritten(hBitstream);
  nBytes = nBits / 8;
  fwrite(bitbuf, nBytes, 1, h_OoiConfig->outFile);

  free(bitbuf);

  return error;
}

int MP_getDrcParamData(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* asi, int* numGroupIdsRequested, int* groupIdRequested, int* groupPresetIdRequested, int selectedPresetID)
{
  int i;
  int error = 0;
  int temp = 0;

  if (h_InteractConfig->ElementInteractionData != NULL)
  {
    *groupPresetIdRequested = h_InteractConfig->ElementInteractionData->groupPresetID;
  }
  else if (selectedPresetID != -1) {
    *groupPresetIdRequested = selectedPresetID;
  }
  else if (asi->asi.numGroupPresets > 0) {
    /* choose ID 0 or minimim preset ID if not present */
    int minPresetID = MAE_MAX_NUM_GROUP_PRESETS;
    for (i = 0; i < asi->asi.numGroupPresets; i++)
    {
      if (asi->asi.groupPresets[i].presetID < minPresetID)
      {
        minPresetID = asi->asi.groupPresets[i].presetID;
        if (minPresetID == 0) break;
      }
    }
    *groupPresetIdRequested = minPresetID;
  }
  else
  {
    *groupPresetIdRequested = -1;
  }

  for (i = 0; i < asi->asi.numGroups; i++)
  {
    if (h_InteractConfig->onOffStatusModified[i] == 1)
    {
      groupIdRequested[temp] = asi->asi.groups[i].groupID;
      temp++;
    }
  }

  *numGroupIdsRequested = temp;

  return error;

}

int MP_closeEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME h_enhObjMdFrame)
{
  int error = 0;
  free(h_enhObjMdFrame);
  return error;
}

int MP_closeObjectOutputInterfaceWrite(H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig)
{
  int error = 0;

  fclose(h_OoiConfig->outFile);

  free(h_OoiConfig);
  return error;
}

int MP_initObjectOutputInterfaceWrite(H_OBJ_OUT_INTERFACE_CONFIG* h_OoiConfig, char* OoiOutpath, int audioFrameLength, int audioByteDepth, int audioSamplingRate)
{
  int error = 0;
  H_OBJ_OUT_INTERFACE_CONFIG temp = (H_OBJ_OUT_INTERFACE_CONFIG)calloc(1, sizeof(struct MP_ObjectOutputInterfaceConfig));

  if (NULL != temp) {
  strncpy(temp->outPath, OoiOutpath, sizeof(temp->outPath));
    temp->outFile = NULL;

    temp->audioByteDepth = audioByteDepth;
    temp->audioFrameLength = audioFrameLength;
    temp->audioSamplingRate = audioSamplingRate;

    /* open file */
    temp->outFile = fopen(temp->outPath, "wb");
    if ( NULL == temp->outFile )
    {
      error = -1;
      free(temp);
      fprintf(stderr, "Error during initialization of the object output interface.\n");
      return error;
    }
  } else {
    error = -2;
    fprintf(stderr, "Error during initialization of the object output interface.\n");
    return error;
  }

  *h_OoiConfig = temp;
  return error;
}

void MP_initAudioConfig(H_AUDIO_MP_CONFIG* h_AudioConfig, char* wavIO_inpath, char* wavIO_inpath_real, char* wavIO_inpath_imag, char* wavIO_outpath, char* wavIO_outpath_real, char* wavIO_outpath_imag, char* wavIO_outpathDiffuse_real, char* wavIO_outpathDiffuse_imag, char* wavIO_outpathDiffuse)
{
  H_AUDIO_MP_CONFIG temp = (H_AUDIO_MP_CONFIG)calloc(1, sizeof(struct MP_AudioConfigStruct));

  if (NULL != temp) {
    strncpy(temp->wavIO_inpath_real, wavIO_inpath_real, sizeof(temp->wavIO_inpath_real));
    strncpy(temp->wavIO_inpath_imag, wavIO_inpath_imag, sizeof(temp->wavIO_inpath_imag));
    strncpy(temp->wavIO_inpath, wavIO_inpath, sizeof(temp->wavIO_inpath));

    strncpy(temp->wavIO_outpath_real, wavIO_outpath_real, sizeof(temp->wavIO_outpath_real));
    strncpy(temp->wavIO_outpath_imag, wavIO_outpath_imag, sizeof(temp->wavIO_outpath_imag));
    strncpy(temp->wavIO_outpath, wavIO_outpath, sizeof(temp->wavIO_outpath));

    strncpy(temp->wavIO_outpathDiffuse_real, wavIO_outpathDiffuse_real, sizeof(temp->wavIO_outpathDiffuse_real));
    strncpy(temp->wavIO_outpathDiffuse_imag, wavIO_outpathDiffuse_imag, sizeof(temp->wavIO_outpathDiffuse_imag));
    strncpy(temp->wavIO_outpathDiffuse, wavIO_outpathDiffuse, sizeof(temp->wavIO_outpathDiffuse));
    temp->fwavOut_diffuse_real = NULL;
    temp->fwavOut_diffuse_imag = NULL;
    temp->fwavOut_diffuse = NULL;
    temp->hWavIODiffuse = NULL;

    temp->fwavIn_real = NULL;
    temp->fwavIn_imag = NULL;
    temp->fwavIn = NULL;

    temp->fwavOut_real = NULL;
    temp->fwavOut_imag = NULL;
    temp->fwavOut = NULL;

    temp->hWavIO_real = NULL;
    temp->hWavIO_imag = NULL;
    temp->hWavIO = NULL;

    temp->nInChannels = 0;
    temp->InSampleRate = 0;
    temp->InBytedepth = 0;
    temp->nTotalSamplesPerChannel = 0;
    temp->nSamplesPerChannelFilled = 0;

    temp->decode_wav = 0;
    temp->decode_qmf = 0;
    temp->write_qmf = 0;
    temp->write_wav = 0;

    temp->numTimeslots = 0;
    temp->audio_blocksize = 0;

    temp->samplesReadPerChannel = 0;
    temp->isLastFrame = 0;
    temp->nZerosPaddedBeginning = 0;
    temp->nZerosPaddedEnd = 0;
    temp->samplesToWritePerChannel = 0;

    temp->nTotalSamplesWrittenPerChannel = 0;
    temp->samplesWrittenPerChannel = 0;
  }

  *h_AudioConfig = temp;
}

int MP_getGroupSetupData(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_GROUP_SETUP_DATA* groupSetupConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int enhObjMdFramePresent )
{
  int numGroups = 0;
  int numObjGroups = 0;
  int numGroupsWithExclusion = 0;
  int numObjectsWithExclusion = 0;
  int i = 0, j = 0, k = 0;
  unsigned long jul = 0;
  int numObjects = 0;

  H_GROUP_SETUP_DATA temp = (H_GROUP_SETUP_DATA)calloc(1, sizeof(struct _GroupSetupData));
  Setup_SpeakerConfig3d *pSpeakerConfig3d = (Setup_SpeakerConfig3d*)calloc(1, sizeof(Setup_SpeakerConfig3d));

  /***************** GET IDEAL LOCAL SETUP *************************************/

  if (localSetupConfig->LoudspeakerRendering != NULL) /* get local setup */
  {
    pSpeakerConfig3d->numSpeakers = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->numSpeakers;
    pSpeakerConfig3d->speakerLayoutType = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->speakerLayoutType;

    if (pSpeakerConfig3d->speakerLayoutType == 0)
    {
      pSpeakerConfig3d->CICPspeakerLayoutIdx = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx;
    }
    else if (pSpeakerConfig3d->speakerLayoutType < 3)
    {
	    if ( pSpeakerConfig3d->speakerLayoutType == 1 )
	    {
		    for ( i = 0; i < (int)pSpeakerConfig3d->numSpeakers; i++ )
		    {
			    pSpeakerConfig3d->CICPspeakerIdx[i] =  localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->CICPspeakerIdx[i];
		    }
	    }
	    if ( pSpeakerConfig3d->speakerLayoutType == 2 )  /* restriction of excluded sectors to speakerLayoutSignaling Type 0 and 1 */
	    {
        temp->numIndividualSetups = 1;
        temp->doUseIndividualGroupSetups = 0;
        *groupSetupConfig = temp;
        return -1;
      }
    }

    /* Fill geometry, numChannels and numLFEs ... */
    get_geometry_from_speakerConfig3d(
	  pSpeakerConfig3d,
	  pSpeakerConfig3d->pGeometry,
	  &pSpeakerConfig3d->numChannels,
	  &pSpeakerConfig3d->numLFEs);

    /* ... and set numSpeakers */
    pSpeakerConfig3d->numSpeakers = pSpeakerConfig3d->numChannels + pSpeakerConfig3d->numLFEs;
  }
  
  if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors == 0)
  {
    
    enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors = 1;
  }

  if (enhancedObjectMetadataConfig->hasCommonGroupExcludedSectors == 1)
  {
    /***************** GET GROUP SETUP (PER SIGNAL GROUP) ****************************/

    numGroups = signalGroupConfig->numSignalGroups;

    for (i = 0; i < numGroups; i++)
    {
      if (signalGroupConfig->signalGroupType[i] == 1)
      {
        int numObjectsThis = 0;
        int numSectors = 0;
        numObjGroups++;
        numObjectsThis = signalGroupConfig->numberOfSignals[i];
        numObjects += numObjectsThis;
        if (enhObjMdFramePresent)
        {
          numSectors = h_enhObjMdFrame->numExclusionSectors[0];
        }

        if (numSectors > 0)
        {
          temp->hasExcludedSectors[i] = 1;
          numGroupsWithExclusion++;
          numObjectsWithExclusion += numObjectsThis;
        }
      }
    }

    /* define group-wise setups */
    if (numGroupsWithExclusion > 0)
    {
      temp->numObjectBasedGroups = numObjGroups;

      for (i = 0; i < numGroups; i++)
      {
        int numExcluded = 0;

        if (signalGroupConfig->signalGroupType[i] == 1)
        {
          if (temp->hasExcludedSectors[i] == 1)
          {
            int toBeExcluded[MAE_MAX_NUM_SPEAKERS] = { 0 };
            int numSectors = 0;


            /* init setup struct of group */
            temp->speakerConfig[i].speakerLayoutType = 2;
            temp->speakerConfig[i].numChannels = 0;
            temp->speakerConfig[i].numLFEs = 0;
            temp->speakerConfig[i].numSpeakers = 0;

            if (enhObjMdFramePresent)
            {
              numSectors = h_enhObjMdFrame->numExclusionSectors[0];
            }
            else
            {
              numSectors = 0;
            }

            /* loop over all speakers */
            for (jul = 0; jul < pSpeakerConfig3d->numSpeakers; jul++)
            {
              toBeExcluded[jul] = 0;
              /* loop over all sectors */
              for ( k = 0; k < numSectors; k++)
              {
                if (toBeExcluded[jul] == 0) /* no need to evalaute the speaker again if it has already been excluded by the use of another sector */
                {
                  /* predefined sectors */

                  if (enhancedObjectMetadataConfig->useOnlyPredefinedSectors[0])
                  {
                    int sectorIdx = h_enhObjMdFrame->excludeSectorIndex[0][k];
                    toBeExcluded[jul] = MP_checkSpeakerExclusion(sectorIdx, pSpeakerConfig3d->pGeometry[jul], localSetupConfig->LocalScreenInfo);
                  }
                  else
                  {
                    if (h_enhObjMdFrame->usePredefinedSector[0][k] == 1)
                    {
                      int sectorIdx = h_enhObjMdFrame->excludeSectorIndex[0][k];
                      toBeExcluded[jul] = MP_checkSpeakerExclusion(sectorIdx, pSpeakerConfig3d->pGeometry[jul], localSetupConfig->LocalScreenInfo);
                    }
                    else
                    {
                      toBeExcluded[jul] = MP_checkSpeakerExclusion_arbitrarySec(
                        (int)h_enhObjMdFrame->excludeSectorMaxAzimuth[0][k], (int)h_enhObjMdFrame->excludeSectorMinAzimuth[0][k],
                        (int)h_enhObjMdFrame->excludeSectorMaxElevation[0][k], (int)h_enhObjMdFrame->excludeSectorMinElevation[0][k],
                        pSpeakerConfig3d->pGeometry[jul]);
                    }
                  }
                }
              }
              if (toBeExcluded[jul] == 1)
              {
                numExcluded++;
              }

              if (toBeExcluded[jul] == 0)
              {
                if (pSpeakerConfig3d->pGeometry[j].LFE == 1)
                {
                  temp->speakerConfig[i].numLFEs = temp->speakerConfig[i].numLFEs + 1;
                }
                else
                {
                  temp->speakerConfig[i].numChannels = temp->speakerConfig[i].numChannels + 1;
                }

                temp->speakerConfig[i].numSpeakers = temp->speakerConfig[i].numSpeakers + 1;

                /* copy geometric information to setup struct */
                temp->speakerConfig[i].pGeometry[temp->speakerConfig[i].numSpeakers-1].Az = pSpeakerConfig3d->pGeometry[jul].Az;
                temp->speakerConfig[i].pGeometry[temp->speakerConfig[i].numSpeakers-1].El = pSpeakerConfig3d->pGeometry[jul].El;
                temp->speakerConfig[i].pGeometry[temp->speakerConfig[i].numSpeakers-1].loudspeakerType = pSpeakerConfig3d->pGeometry[jul].loudspeakerType;
                temp->speakerConfig[i].pGeometry[temp->speakerConfig[i].numSpeakers-1].screenRelative = pSpeakerConfig3d->pGeometry[jul].screenRelative;

                if ((localSetupConfig->LoudspeakerRendering != NULL) && (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL))
                {
                  /* copy known positions for loudspeaker rendering (to be used for closest speaker processing later on)*/
                  temp->hasKnownPos[i][temp->speakerConfig[i].numSpeakers-1] = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][jul];
                  if (temp->hasKnownPos[i][temp->speakerConfig[i].numSpeakers-1] == 1)
                  {
                    temp->knownAzimuth[i][temp->speakerConfig[i].numSpeakers-1] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][jul];
                    temp->knownElevation[i][temp->speakerConfig[i].numSpeakers-1] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][jul];
                  }
                }
              }
            }
          }
          else
          {
            /* standard setup */
            temp->speakerConfig[i].numSpeakers = pSpeakerConfig3d->numSpeakers;
            temp->speakerConfig[i].numChannels = pSpeakerConfig3d->numChannels;
            temp->speakerConfig[i].numLFEs = pSpeakerConfig3d->numLFEs;
            temp->speakerConfig[i].speakerLayoutType = pSpeakerConfig3d->speakerLayoutType;
            memcpy(temp->speakerConfig[i].pGeometry, pSpeakerConfig3d->pGeometry, sizeof(pSpeakerConfig3d->pGeometry));

            /* copy known positions for loudspeaker rendering (to be used for closest speaker processing later on) */
            if ((localSetupConfig->LoudspeakerRendering != NULL) && (pSpeakerConfig3d->speakerLayoutType < 2) && (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL))
            {
              for (j = 0; j < (int)temp->speakerConfig[i].numSpeakers; j++)
              {
                temp->hasKnownPos[i][j] = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][j];
                if ( temp->hasKnownPos[i][j] == 1)
                {
                  temp->knownAzimuth[i][j] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][j];
                  temp->knownElevation[i][j] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][j];
                }
              }
            }
          }
        }
        else
        {
          /* standard setup for all non-object-based groups */
          temp->speakerConfig[i].numSpeakers = pSpeakerConfig3d->numSpeakers;
          temp->speakerConfig[i].numChannels = pSpeakerConfig3d->numChannels;
          temp->speakerConfig[i].numLFEs = pSpeakerConfig3d->numLFEs;
          temp->speakerConfig[i].speakerLayoutType = pSpeakerConfig3d->speakerLayoutType;
          memcpy(temp->speakerConfig[i].pGeometry, pSpeakerConfig3d->pGeometry, sizeof(pSpeakerConfig3d->pGeometry));

          /* copy known positions for loudspeaker rendering (to be used for closest speaker processing later on) */
          if ((localSetupConfig->LoudspeakerRendering != NULL) && (pSpeakerConfig3d->speakerLayoutType < 2) && (localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions != NULL))
          {
            for (j = 0; j < (int)temp->speakerConfig[i].numSpeakers; j++)
            {
              temp->hasKnownPos[i][j] = (int)localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[0][j];
              if ( temp->hasKnownPos[i][j] == 1)
              {
                temp->knownAzimuth[i][j] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[1][j];
                temp->knownElevation[i][j] = localSetupConfig->LoudspeakerRendering->Setup_SpeakerConfig3D->knownPositions[2][j];
              }
            }
          }
        }
      }
      temp->numIndividualSetups = MP_getIndividualSetups(/*audioSceneConfig->asi.groupExtensions, */temp, audioSceneConfig->asi.numGroups, enhancedObjectMetadataConfig, h_enhObjMdFrame );
      if (temp->numIndividualSetups > 1)
      {
        temp->doUseIndividualGroupSetups = 1;
      }
      else
      {
        temp->doUseIndividualGroupSetups = 0;
      }
      *groupSetupConfig = temp;
      free(pSpeakerConfig3d);
      return 0;
    }
    else
    {
      temp->numIndividualSetups = 1;
      temp->doUseIndividualGroupSetups = 0;
      *groupSetupConfig = temp;
      free(pSpeakerConfig3d);
      return -1;
    }
  }
  return 0;
}

int MP_writeGroupSetupData(H_GROUP_SETUP_DATA groupSetupConfig, char* groupSetup_outpath)
{
  int error = 0;
  int i, j  = 0;
  int idx = 0;
  int setup_number = -1;
  char outpath_temp[FILENAME_MAX];
  int numChannelsTotal = 0;
  int ch = 0;

  if (groupSetupConfig->doUseIndividualGroupSetups == 1)
  {
    for (i = 0; i < groupSetupConfig->numIndividualSetups; i++)
    {
      FILE* fileHandle;
      setup_number = i;

      for (j = 0; j < MAE_MAX_NUM_GROUPS; j++)
      {
        if (groupSetupConfig->groupSetupID[j] == setup_number)
        {
          idx = j;
          break;
        }
      }

      numChannelsTotal = groupSetupConfig->speakerConfig[idx].numSpeakers;
      ch = 0;

      sprintf (outpath_temp, "%s_%d.txt", groupSetup_outpath, setup_number);

      /* open file */
      fileHandle = fopen(outpath_temp, "wb");

      if ( NULL == fileHandle )
      {
        error = -1;
        return error;
      }

      /* define number of entries */
      fprintf( fileHandle, "%i\n", numChannelsTotal );

      for ( ch = 0; ch < numChannelsTotal; ++ch )
      {
          /* write geometry info if necessary */
        fprintf( fileHandle, "g,%i,%i,%i,%i\n", groupSetupConfig->speakerConfig[idx].pGeometry[ch].Az, groupSetupConfig->speakerConfig[idx].pGeometry[ch].El, groupSetupConfig->speakerConfig[idx].pGeometry[ch].LFE, groupSetupConfig->speakerConfig[idx].pGeometry[ch].screenRelative);
      }
      /* close file */
      if ( fclose(fileHandle) )
      {
        error = -1;
        return error;
      }
    }
  }
  return error;
}


static int MP_compareSpeakerSetup(float listSpeakerAz1[], float listSpeakerAz2[], float listSpeakerEl1[], float listSpeakerEl2[], int listSpeakerLFE1[], int listSpeakerLFE2[], int numSpeakers)
{
  int i,j;
  int isEqual = 1;
  int* speakerEqual = (int*)calloc(numSpeakers,sizeof(int));

  for (i=0; i < numSpeakers; i++)
  {
    for (j=0; j < numSpeakers; j++)
    {
      if ((listSpeakerAz1[i] == listSpeakerAz2[j]) && (listSpeakerEl1[i] == listSpeakerEl2[j]) && (listSpeakerLFE1[i] == listSpeakerLFE2[j]))
      {
        speakerEqual[i] = 1;
      }
    }
  }
  for (i=0; i < numSpeakers; i++)
  {
    if (speakerEqual[i] == 0)
    {
      isEqual = 0;
      break;
    }
  }
  free(speakerEqual);
  return isEqual;
}

int MP_getIndividualSetups(H_GROUP_SETUP_DATA groupSetupConfig, int numGroups, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame)
{
  int numIndividualSetups = 1; /* standard setup */
  int i, j;
  int numExcludedGroups = 0;
  int isequal;

  float listSpeakerAz[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];
  float listSpeakerEl[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];
  int listSpeakerLFE[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];
  int groupSetupID[MAE_MAX_NUM_GROUPS];
  int doCompare[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_GROUPS];

  memset(listSpeakerAz,0,MAE_MAX_NUM_GROUPS*MAE_MAX_NUM_SPEAKERS);
  memset(listSpeakerEl,0,MAE_MAX_NUM_GROUPS*MAE_MAX_NUM_SPEAKERS);
  memset(listSpeakerLFE,0,MAE_MAX_NUM_GROUPS*MAE_MAX_NUM_SPEAKERS);

  /* assign standard setup to all groups */
  for (i=0; i< MAE_MAX_NUM_GROUPS; i++)
  {
    if (i < numGroups)
    {
      groupSetupID[i] = 0; /* group has standard layout */
    }
    else
    {
      groupSetupID[i] = -1;
    }
    for (j = 0; j < MAE_MAX_NUM_GROUPS; j++)
    {
      doCompare[i][j] = 1;
    }
  }

  /* mark groups with excluded sectors, copy speakers to lists for comparison */
  for (i=0; i < numGroups; i++)
  {
    if (groupSetupConfig->hasExcludedSectors[i] > 0)
    {
      groupSetupID[i] = -2; /* group completely disabled, no speakers left */
      if (groupSetupConfig->speakerConfig[i].numSpeakers > 0)
      {
        groupSetupID[i] = -3; /* group has a layout different then standard layout */
        numExcludedGroups++;
        for (j=0; j < (int)groupSetupConfig->speakerConfig[i].numSpeakers; j++)
        {
          listSpeakerAz[i][j] = (float)groupSetupConfig->speakerConfig[i].pGeometry[j].Az;
          listSpeakerEl[i][j] = (float)groupSetupConfig->speakerConfig[i].pGeometry[j].El;
          listSpeakerLFE[i][j] = groupSetupConfig->speakerConfig[i].pGeometry[j].LFE;
        }
      }
    }
  }

  /* compare the setups for all groups with sectors */
  for (i=0; i < numGroups; i++)
  {
    for (j=0; j < numGroups; j++)
    {
      isequal = -1;
      if ((groupSetupID[i] < -2) && (groupSetupID[j] < -2) && (doCompare[i][j] == 1))
      {
        isequal = MP_compareSpeakerSetup(listSpeakerAz[i], listSpeakerAz[j], listSpeakerEl[i], listSpeakerEl[j], listSpeakerLFE[i], listSpeakerLFE[j], groupSetupConfig->speakerConfig[i].numSpeakers);
        doCompare[i][j] = 0;
        doCompare[j][i] = 0;

        /* for groups with the same setup, a unique setup number is copied to the setup list for the corresponding groups */
        if (isequal == 1)
        {
          groupSetupID[i] = numIndividualSetups;
          groupSetupID[j] = numIndividualSetups;
        }
      }
    }
    /* if there is just one group with a specified setup, the number is copied to the setup list for the corresponding group */
    if (isequal == 0)
    {
      groupSetupID[i] = numIndividualSetups;
    }
    if (isequal > -1)
    {
      numIndividualSetups++;
    }
  }

  memcpy(groupSetupConfig->groupSetupID, groupSetupID, MAE_MAX_NUM_GROUPS*sizeof(int));
  return numIndividualSetups;
}

int MP_checkSpeakerExclusion_arbitrarySec(int secMaxAz, int  secMinAz, int secMaxEl, int secMinEl, CICP2GEOMETRY_CHANNEL_GEOMETRY speaker)
{
  int toBeExcluded = 0;

  if (((speaker.Az >= secMinAz) && (speaker.Az <= secMaxAz)) && ((speaker.El >= secMinEl) && (speaker.El <= secMaxEl)))
  {
    toBeExcluded = 1;
  }

  return toBeExcluded;
}


int MP_checkSpeakerExclusion(int sectorIdx, CICP2GEOMETRY_CHANNEL_GEOMETRY speaker, H_LOCAL_SCREEN_INFO localScreen)
{
  int toBeExcluded = 0;

  if (speaker.LFE == 0)
  {
    switch( sectorIdx )
    {
      case 0: /* no positive elevation */
      {
        if (speaker.El > 0.0f)
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 1: /* no negative elevation */
      {
        if (speaker.El < 0.0f)
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 2: /* no front */
      {
        if ((fabs((float)speaker.Az) < 90.0f) && (fabs((float)speaker.El) < 90.0f))
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 3: /* no rigth side */
      {
        if ((speaker.Az > -180.0f) && (speaker.Az < 0.0f))
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 4: /* no left side */
      {
        if ((speaker.Az > 0.0f) && (speaker.Az < 180.0f))
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 5: /* no surround */
      {
        if ((fabs((float)speaker.Az) >= 90.0f) || ((speaker.Az == 0.0f) && (speaker.El == 90.0f)))
        {
          toBeExcluded = 1;
        }
        break;
      }
      case 6: /* screen only */
      {
        if (((speaker.Az > localScreen->az_left) || (speaker.Az < localScreen->az_right)) && ((speaker.El > localScreen->el_top) || (speaker.El < localScreen->el_bottom)))
        {
          toBeExcluded = 1;
        }
        break;
      }
    }
  }

  return toBeExcluded;
}

void MP_initInteractConfig(H_INTERACT_MP_CONFIG* h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig)
{
  H_INTERACT_MP_CONFIG temp = (H_INTERACT_MP_CONFIG)calloc(1, sizeof(struct MP_InteractionConfigStruct));

  if (NULL != temp) {
    temp->azimuthModified = NULL;
    temp->elevationModified = NULL;
    temp->onOffStatusModified = NULL;
    temp->routeToWireID = NULL;
    temp->doProcess = NULL;
    temp->screenRelatedObjects = NULL;
    temp->oamSampleModified = NULL;
    temp->oamSampleModified_Divergence = NULL;
    temp->divergence_ASImodified = 0;
    temp->diffuse_enableProcessing = 0;

    temp->gainModifiedGroup = NULL;
    temp->gainModifiedSingleObject = NULL;
    temp->distanceModified = NULL;
    temp->gainModified_LastFrame = NULL;

    temp->listOAM = NULL;
    temp->oamCnt = 0;
    temp->numObjects_in = 0;
    temp->numObjects_out = 0;
    temp->numSpeakers = 0;

    temp->interactionDataToBeApplied = 0;

    temp->interactionType = 0;
    temp->numDecodedGroups = audioSceneConfig->asi.numGroups;
    temp->numSwitchGroups = audioSceneConfig->asi.numSwitchGroups;
    temp->enableScreenRelatedProcessing = 0;
    temp->hasScreenSize = 0;
    temp->numElements_in = 0;
    temp->numElements_out = 0;

    temp->buffer_temp = NULL;
    temp->buffer_temp_imag = NULL;
    temp->buffer_temp_real = NULL;

    temp->ElementInteractionData = NULL;
    temp->localScreen = NULL;
    temp->loudnessCompensationGain = 0.0f;
    if (audioSceneConfig->asi.numGroups > 0)
    {
      temp->gainModified_LastFrame = (float*)calloc(audioSceneConfig->asi.numGroups,sizeof(float));
      temp->onOffStatusModified = (int*)calloc(audioSceneConfig->asi.numGroups,sizeof(int));
      temp->gainModifiedGroup = (float*)calloc(audioSceneConfig->asi.numGroups, sizeof(float));
      temp->routeToWireID = (int*)calloc(audioSceneConfig->asi.numGroups, sizeof(int));
    }
  }

  *h_InteractConfig = temp;
}


int getNumAudioFrames(H_AUDIO_MP_CONFIG h_AudioConfig)
{
  return h_AudioConfig->nFrames;
}

int MP_getMaxMaeID(ASCPARSER_AUDIO_SCENE* audioSceneConfig)
{
  int numGroups = audioSceneConfig->asi.numGroups;
  int i, j;
  int maeID;
  int maeMaxID = -1;
  for (i = 0; i < numGroups; i++)
  {
    for (j = 0; j < audioSceneConfig->asi.groups[i].groupNumMembers; j++)
    {
      if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 1)
      {
        maeID = audioSceneConfig->asi.groups[i].startID + j;
      }
      else
      {
        maeID = audioSceneConfig->asi.groups[i].metaDataElementID[j];
      }

      if (maeID > maeMaxID)
      {
        maeMaxID = maeID;
      }
    }
  }
  return maeMaxID;
}

int MP_initEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME* h_enhObjMdFrame, char* path)
{
  int error = 0;
  H_ENH_OBJ_MD_FRAME tmp = (H_ENH_OBJ_MD_FRAME)calloc(1,sizeof(struct MP_enhObjMdFrameStruct));

  if (NULL != tmp) {
    strncpy(tmp->path, path, FILENAME_MAX);
  } else {
    error = -1;
    fprintf(stderr, "Error while initializing EnhancedObjectMetadataFrame. \n");
  }

  *h_enhObjMdFrame = tmp;

  return error;
}

int MP_initAudioAndOamReadWrite(H_AUDIO_MP_CONFIG AudioConfig, H_OAM_MP_CONFIG OamConfig, int oam_length, int decoder_domain, int audioFramelength, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhObjMdConfig, int enableObjectOutputInterface, const int profile)
{
  int j, k;
  int i;
  int ct;
  int error = 0;
  int add_objects = 0;
  int tmp_added_divergence_objects = 0;

  if (decoder_domain == 0)
  {
    AudioConfig->decode_qmf = 0;
    AudioConfig->decode_wav = 1;
  }
  else
  {
    AudioConfig->decode_qmf = 1;
    AudioConfig->decode_wav = 0;
  }

  AudioConfig->audio_blocksize = audioFramelength;
  add_objects = 0;

  if (OamConfig->oam_hasBeenDecoded == 1)
  {
    AudioConfig->audio_blocksize = oam_length;
    if (AudioConfig->audio_blocksize == 0)
    {
       AudioConfig->audio_blocksize = audioFramelength;
    }

    /* get number of additional objects used for divergence processing */
    if (!enableObjectOutputInterface)
    {
      ct = 0;
      for (i = 0; i < (int)signalGroupConfig->numSignalGroups; i++)
      {
        if (signalGroupConfig->signalGroupType[i] == 1)
        {
          for (j = 0; j < (int)signalGroupConfig->numberOfSignals[i]; j++)
          {
            if (enhObjMdConfig->hasDivergence[ct] == 1)
            {
              add_objects += 2;
            }
            ct++;
          }
        }
      }
    }
  }

  if (localSetupConfig->rendering_type == 0)
  {
    AudioConfig->nSpeakers = localSetupConfig->LoudspeakerRendering->numSpeakers;
  }
  else
  {
    AudioConfig->nSpeakers = localSetupConfig->BinauralRendering->pBR->ppBinauralRepresentation[0]->nBrirPairs;
  }

  if (AudioConfig->decode_qmf)
  {
    AudioConfig->write_qmf = 1;
    AudioConfig->write_wav = 0;

    AudioConfig->fwavIn_real = fopen(AudioConfig->wavIO_inpath_real, "rb");
    AudioConfig->fwavIn_imag = fopen(AudioConfig->wavIO_inpath_imag, "rb");
    if (!((AudioConfig->fwavIn_real != NULL) && (AudioConfig->fwavIn_imag != NULL)))
    {
      fprintf(stderr, "Error while reading decoded QMF file \n");
      return -1;
    }
    error = wavIO_init(&AudioConfig->hWavIO_real, (unsigned int)AudioConfig->audio_blocksize, 0, 0);
    error = wavIO_init(&AudioConfig->hWavIO_imag, (unsigned int)AudioConfig->audio_blocksize, 0, 0);
    if ((AudioConfig->fwavIn_real) && (AudioConfig->fwavIn_imag))
    {
	    error = wavIO_openRead(AudioConfig->hWavIO_real, AudioConfig->fwavIn_real, &AudioConfig->nInChannels, &AudioConfig->InSampleRate, &AudioConfig->InBytedepth, &AudioConfig->nTotalSamplesPerChannel, &AudioConfig->nSamplesPerChannelFilled);
	    error = wavIO_openRead(AudioConfig->hWavIO_imag, AudioConfig->fwavIn_imag, &AudioConfig->nInChannels, &AudioConfig->InSampleRate, &AudioConfig->InBytedepth, &AudioConfig->nTotalSamplesPerChannel, &AudioConfig->nSamplesPerChannelFilled);
    }

    AudioConfig->nOutChannels = AudioConfig->nInChannels + add_objects;

    AudioConfig->fwavOut_real = fopen(AudioConfig->wavIO_outpath_real, "wb");
    AudioConfig->fwavOut_imag = fopen(AudioConfig->wavIO_outpath_imag, "wb");
    error = wavIO_openWrite(AudioConfig->hWavIO_real, AudioConfig->fwavOut_real, AudioConfig->nOutChannels, AudioConfig->InSampleRate, AudioConfig->InBytedepth);
    error = wavIO_openWrite(AudioConfig->hWavIO_imag, AudioConfig->fwavOut_imag, AudioConfig->nOutChannels, AudioConfig->InSampleRate, AudioConfig->InBytedepth);
  }
  else if (AudioConfig->decode_wav)
  {
    AudioConfig->write_wav = 1;
    AudioConfig->write_qmf = 0;
    AudioConfig->fwavIn = fopen(AudioConfig->wavIO_inpath, "rb");
    if (!(AudioConfig->fwavIn != NULL))
	  {
		  fprintf(stderr, "Error while reading decoded wave file \n");
      return -1;
	  }
    error = wavIO_init(&AudioConfig->hWavIO, (unsigned int)AudioConfig->audio_blocksize, 0, 0);
    if (AudioConfig->fwavIn)
    {
	    error = wavIO_openRead(AudioConfig->hWavIO, AudioConfig->fwavIn, &AudioConfig->nInChannels, &AudioConfig->InSampleRate, &AudioConfig->InBytedepth, &AudioConfig->nTotalSamplesPerChannel, &AudioConfig->nSamplesPerChannelFilled);
    }

    AudioConfig->nOutChannels = AudioConfig->nInChannels + add_objects;

    AudioConfig->fwavOut = fopen(AudioConfig->wavIO_outpath, "wb");
    error = wavIO_openWrite(AudioConfig->hWavIO, AudioConfig->fwavOut, AudioConfig->nOutChannels, AudioConfig->InSampleRate, AudioConfig->InBytedepth);
  }

  if (OamConfig->oam_hasBeenDecoded == 1)
  {
    if (oam_length == 0) {
      fprintf(stderr, "Warning: OAM length is set to the audio blocksize \n");
      oam_length = AudioConfig->audio_blocksize;
    }
    OamConfig->oam_frames = (int)ceil(AudioConfig->nTotalSamplesPerChannel*1.0f / oam_length*1.0f);
    OamConfig->oam_blocksize = oam_length;
    if (AudioConfig->audio_blocksize != oam_length)
    {
      fprintf(stderr, "Warning: The audio blocksize and the oam length are different. The audio blocksize is set to the oam length \n");
      AudioConfig->audio_blocksize = oam_length;
    }
    AudioConfig->nFrames = OamConfig->oam_frames;
  }
  else
  {
    AudioConfig->nFrames = (int)ceil(AudioConfig->nTotalSamplesPerChannel*1.0f  / AudioConfig->audio_blocksize*1.0f);
  }

  /* init audio buffers */
  AudioConfig->audioTdBuffer = (float**)calloc(AudioConfig->nInChannels, sizeof(float*));
  for (j = 0; j < (int)AudioConfig->nInChannels; j++)
  {
    AudioConfig->audioTdBuffer[j] = (float*)calloc(AudioConfig->audio_blocksize, sizeof(float));
  }

  if (AudioConfig->decode_qmf)
  {
    AudioConfig->numTimeslots = AudioConfig->audio_blocksize / QMFLIB_NUMBANDS;
	  AudioConfig->audioQmfBuffer_real = (float***)calloc(AudioConfig->numTimeslots, sizeof (float**));
	  AudioConfig->audioQmfBuffer_imag = (float***)calloc(AudioConfig->numTimeslots, sizeof (float**));
	  for(k = 0; k < AudioConfig->numTimeslots; k++ )
	  {
		  AudioConfig->audioQmfBuffer_real[k] = (float**)calloc (AudioConfig->nInChannels, sizeof (float*));
		  AudioConfig->audioQmfBuffer_imag[k] = (float**)calloc (AudioConfig->nInChannels, sizeof (float*));

		  for (j = 0; j < (int)AudioConfig->nInChannels; j++)
		  {
			  AudioConfig->audioQmfBuffer_real[k][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
			  AudioConfig->audioQmfBuffer_imag[k][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
		  }
	  }
  }

  if (add_objects > 0)
  {
    if (AudioConfig->decode_wav)
    {
      AudioConfig->audioTdBuffer_divergence = (float**)calloc(AudioConfig->nOutChannels, sizeof(float*));
      for (j = 0; j < (int)AudioConfig->nOutChannels; j++)
      {
        AudioConfig->audioTdBuffer_divergence[j] = (float*)calloc(AudioConfig->audio_blocksize, sizeof(float));
      }
    }
    if (AudioConfig->decode_qmf)
    {
      AudioConfig->audioQmfBuffer_divergence_real = (float***)calloc(AudioConfig->numTimeslots, sizeof (float**));
      AudioConfig->audioQmfBuffer_divergence_imag = (float***)calloc(AudioConfig->numTimeslots, sizeof (float**));
      for(k = 0; k < AudioConfig->numTimeslots; k++ )
      {
	      AudioConfig->audioQmfBuffer_divergence_real[k] = (float**)calloc (AudioConfig->nOutChannels, sizeof (float*));
	      AudioConfig->audioQmfBuffer_divergence_imag[k] = (float**)calloc (AudioConfig->nOutChannels, sizeof (float*));

	      for (j = 0; j < (int)AudioConfig->nOutChannels; j++)
	      {
		      AudioConfig->audioQmfBuffer_divergence_real[k][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
		      AudioConfig->audioQmfBuffer_divergence_imag[k][j] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float));
	      }
      }
    }
  }

  if (OamConfig->oam_hasBeenDecoded == 1)
  {
    uint16_t tmpHasDynObjPrio;
    uint16_t tmpHasUniformSpread = 1;
    uint16_t tmpNumObjects;
    uint16_t tmpOamVersion;

    FILE* file_divergenceOAM = NULL;
    char filename_divergenceOAM[] = "tmpFile3Ddec_mdp_divergence.oam";
    /* unsigned int hasDivergence[255] = {0}; */

    unsigned int divSize;
    unsigned int divSizeEl;
    unsigned int divLength;

    /* initialization for OAM file reading */
    OamConfig->oamInFile = oam_read_open(OamConfig->oamInpath, &tmpNumObjects, &tmpOamVersion, &tmpHasDynObjPrio, &tmpHasUniformSpread);
    OamConfig->num_objects_input = (int)tmpNumObjects;
    OamConfig->oam_version = (unsigned int)tmpOamVersion;
    OamConfig->hasDynObjectPrio = (int)tmpHasDynObjPrio;
    OamConfig->hasUniformSpread = (int)tmpHasUniformSpread;

    OamConfig->oamSample_in = oam_multidata_create(OamConfig->num_objects_input, 1);

    /* initialization for OAM file writing */
    OamConfig->num_objects_output = OamConfig->num_objects_input + add_objects;

    /* adjust/expand signaling of divergence application for added objects */
    if (!enableObjectOutputInterface)
    {
      ct = 0;
      tmp_added_divergence_objects = 0;

      for (i = 0; i < (int)signalGroupConfig->numSignalGroups; i++)
      {
        if (signalGroupConfig->signalGroupType[i] == 1)
        {
          for (j = 0; j < (int)signalGroupConfig->numberOfSignals[i]; j++)
          {
            if (enhObjMdConfig->hasDivergence[ct] == 1)
            {
              enhObjMdConfig->hasDivergence[ OamConfig->num_objects_input - 1 + tmp_added_divergence_objects + 1 ] = 1;
              enhObjMdConfig->hasDivergence[ OamConfig->num_objects_input - 1 + tmp_added_divergence_objects + 2 ] = 1;
              tmp_added_divergence_objects += 2;
            }
            ct++;
          }
        }
      }
    }

    OamConfig->oamOutFile = oam_write_open(OamConfig->oamOutpath, OamConfig->num_objects_output, OamConfig->oam_version, OamConfig->hasDynObjectPrio, OamConfig->hasUniformSpread);

    /* Write profile and divergence-indication to dedicated .oam file */
    file_divergenceOAM = fopen(filename_divergenceOAM, "wb");

    if (file_divergenceOAM == NULL)
    {
      fprintf(stderr, "%s", "Error opening .oam-file in MP_initAudioAndOamReadWrite().");
      perror(filename_divergenceOAM);
    }

    divSize   = sizeof(enhObjMdConfig->hasDivergence);
    divSizeEl = sizeof(enhObjMdConfig->hasDivergence[0]);
    divLength = divSize / divSizeEl;

    fwrite( &profile, sizeof(profile), 1, file_divergenceOAM );
    fwrite( enhObjMdConfig->hasDivergence, divSizeEl, divLength, file_divergenceOAM );
    fclose( file_divergenceOAM );

    /* get initial OAM values */
    /* to be skipped because first OAM sample corresponds to end of first frame */
    /* MP_getOamSample(OamConfig->oamInFile, OamConfig->oamSample_in, OamConfig->oam_version, OamConfig->hasDynObjectPrio, OamConfig->hasUniformSpread); */

  }
  if (error != 0)
  {
    return -1;
  }
  else
  {
    return add_objects;
  }
}

#ifdef LOUDNESS_COMPENSATION
float MP_getLoudnessCompensationGain(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_ELEMENT_INTERACTION_DATA ElementInteractionData)
{
    int n = 0, c = 0, gp = 0;
    float tmp1 = 0.f, tmp2 = 0.f;
    float loudnessReference = 0.f;
    float loudnessAfterInteractivity = 0.f;
    float loudnessCompensationGainDb = 0.f;
    int groupLoudnessValueMissingFlag = 0;
    float minGainDb = -1000.f;
    float maxGainDb = 21.f;
    int numGroups = audioSceneConfig->asi.numGroups;
    int numGroupPresets = audioSceneConfig->asi.numGroupPresets;
    int selectedPreset = -1;
    int groupStateDefault[MAE_MAX_NUM_GROUPS] = {1};
    int groupStateInteractivity[MAE_MAX_NUM_GROUPS] = {1};
    float groupGainDefaultDb[MAE_MAX_NUM_GROUPS] = {0.f};
    float groupGainInteractivityDb[MAE_MAX_NUM_GROUPS] = {0};
    int includeGroup[MAE_MAX_NUM_GROUPS] = {1};
    float* groupLoudness;
    LoudnessCompensationData* loudnessCompensation = &audioSceneConfig->asi.loudnessCompensation;
    
    if (loudnessCompensation->loudnessCompensationDataPresent == 1) {
        
        for (n = 0; n < numGroups; n++) { 
            groupStateDefault[n] = audioSceneConfig->asi.groups[n].defaultOnOff;
            groupStateInteractivity[n] = ElementInteractionData->GroupInteraction[n]->onOff;
            groupGainInteractivityDb[n] = (float)ElementInteractionData->GroupInteraction[n]->gain_dB; 
        }
        
        if (ElementInteractionData->interactionMode == 1) {
            selectedPreset = ElementInteractionData->groupPresetID;
            
            for (gp = 0; gp < numGroupPresets ; gp++) {
                if (audioSceneConfig->asi.groupPresets[gp].presetID == selectedPreset) {
                    break;
                }
            }
            if (gp < numGroupPresets) {
                for (c = 0; c < audioSceneConfig->asi.groupPresets[gp].presetNumConditions; c++) {
                    for (n = 0; n < numGroups; n++) {
                        if (audioSceneConfig->asi.groupPresets[gp].presetGroupID[c] == audioSceneConfig->asi.groups[n].groupID) {
                            break;
                        }
                    }
                    if (n<numGroups) {
                        includeGroup[n] = audioSceneConfig->asi.groupPresets[gp].presetConditionOnOff[c];
                        if (audioSceneConfig->asi.groupPresets[gp].groupPresetGainFlag[c]) {
                            groupGainDefaultDb[n] = (float)audioSceneConfig->asi.groupPresets[gp].groupPresetGain[c];
                        }
                    }
                }
                if (loudnessCompensation->presetParamsPresent[gp] == 1) {
                    for (n = 0; n < numGroups; n++) {
                        includeGroup[n] = loudnessCompensation->presetIncludeGroup[gp][n];
                    }
                } else {
                    if (loudnessCompensation->defaultParamsPresent == 1) {
                        for (n = 0; n < numGroups; n++) {
                            includeGroup[n] = loudnessCompensation->defaultIncludeGroup[n];
                        }
                    }
                }
                if (loudnessCompensation->presetMinMaxGainPresent[gp] == 1) {
                    minGainDb = loudnessCompensation->presetMinGain[gp];
                    maxGainDb = loudnessCompensation->presetMaxGain[gp];
                } else {
                    if (loudnessCompensation->defaultMinMaxGainPresent == 1) {
                        minGainDb = loudnessCompensation->defaultMinGain;
                        maxGainDb = loudnessCompensation->defaultMaxGain;
                    }
                }
            } else {
                if (loudnessCompensation->defaultParamsPresent == 1) {
                    for (n = 0; n < numGroups; n++) {
                        includeGroup[n] = loudnessCompensation->defaultIncludeGroup[n];
                    }
                }
                if (loudnessCompensation->defaultMinMaxGainPresent == 1) {
                    minGainDb = loudnessCompensation->defaultMinGain;
                    maxGainDb = loudnessCompensation->defaultMaxGain;
                }
            }
        } else {
            if (loudnessCompensation->defaultParamsPresent == 1) {
                for (n = 0; n < numGroups; n++) {
                    includeGroup[n] = loudnessCompensation->defaultIncludeGroup[n];
                }
            }
            if (loudnessCompensation->defaultMinMaxGainPresent == 1) {
                minGainDb = loudnessCompensation->defaultMinGain;
                maxGainDb = loudnessCompensation->defaultMaxGain;
            }
        }
        
        if (loudnessCompensation->groupLoudnessPresent) {
            groupLoudness = loudnessCompensation->groupLoudness;
        } else {
            
            
            groupLoudnessValueMissingFlag = 1;
            
        }
        
        /* compute components of loudness compensation gain */
        for (n=0; n<numGroups; n++) {
            if (groupLoudnessValueMissingFlag == 0) {
                tmp1 = powf(10.0f, (groupGainDefaultDb[n] + groupLoudness[n]) / 10.0f);
                tmp2 = powf(10.0f, (groupGainInteractivityDb[n] + groupLoudness[n]) / 10.0f);
            } else { /* group loudness value missing for one or more groups */
                tmp1 = powf(10.0f, groupGainDefaultDb[n] / 10.0f);
                tmp2 = powf(10.0f, groupGainInteractivityDb[n] / 10.0f);
            }
            loudnessReference += includeGroup[n]*groupStateDefault[n] * tmp1;
            loudnessAfterInteractivity += includeGroup[n]*groupStateInteractivity[n] * tmp2;
        }

        /* loudness compensation gain in dB */
        loudnessCompensationGainDb = 10.0f * (float)log10(loudnessReference / loudnessAfterInteractivity);

        /* clip loudness compensation gain to min/max gain */
        if (loudnessCompensationGainDb < minGainDb) {
            loudnessCompensationGainDb = minGainDb;
        }
        if (loudnessCompensationGainDb > maxGainDb) {
            loudnessCompensationGainDb = maxGainDb;
        }
    }

    return loudnessCompensationGainDb;
}
#endif
