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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sceneDisplacementInterfaceLib.h"
#include "sceneDisplacementInterfaceLib_BitstreamIO.h"


/*------------------------------------------------------------------------------------------------*/

static void CopySceneDisplacementData(SceneDisplacementData *dest, SceneDisplacementData const *src)
{
    if(!src || !dest) return;

    memcpy(dest, src, sizeof(SceneDisplacementData));
}

/*------------------------------------------------------------------------------------------------*/

void SD_setSceneDisplacementData(SCENE_DISPLACEMENT_HANDLE hSceneDspl,
                                 SceneDisplacementData const *sceneDsplData)
{
    if(!hSceneDspl) return;
    CopySceneDisplacementData(&hSceneDspl->sceneDsplData, sceneDsplData);
}

int SD_readSceneDisplacementBitstream(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_DECODER h_bitstream, unsigned long *noBits)
{
  unsigned long noBitsRead = 0;
  unsigned long noBitsReadTotal = 0;
  int error = 0;

  if(!SceneDisplacement || !h_bitstream || !noBits)
  {
      return -1;
  }

  if (feof(h_bitstream->bsFile))
  {
      return -1;
  }

  /* read data */
  error = SD_readSceneDisplacementBits(SceneDisplacement, h_bitstream, &noBitsRead, &noBitsReadTotal);
  if(error != 0)
  {
      return -1;
  }

  /* inverse quantization of data */
  error = SD_invquantizeData(SceneDisplacement);
  if(error != 0)
  {
      return -1;
  }

  /* Parse PSDI data if enabled */
  if(PSDI_isEnabled(SceneDisplacement))
  {
      int const ret = PSDI_parse(SceneDisplacement, h_bitstream);
      if(ret < 0)
      {
          fprintf(stderr,"Error: PSDI_parse() returned code %i\n", ret);
          return -2;
      }

      noBitsReadTotal += ret;
      (void)PSDI_invQuantization(&SceneDisplacement->sceneDsplData.posSceneDsplData);
  }

  *noBits = noBitsReadTotal;
  return 0;
}



int SD_initHandle(SCENE_DISPLACEMENT_HANDLE *h_SceneDisplacementHandle)
{
  SCENE_DISPLACEMENT_HANDLE temp = (SCENE_DISPLACEMENT_HANDLE)calloc(1, sizeof(struct _SceneDisplacementHandle));

  RotSceneDisplacementData *rotSceneDsplData = &temp->sceneDsplData.rotSceneDsplData;

  /**
     PSDI is disabled per default
     Use PSDI_enabled(SCENE_DISPLACEMENT_HANDLE, unsigned int) to enable
  */
  temp->bPosSceneDsplEnabled = 0;

  rotSceneDsplData->roll = 0.0f;
  rotSceneDsplData->pitch = 0.0f;
  rotSceneDsplData->yaw = 0.0f;

  rotSceneDsplData->roll_q = 256;
  rotSceneDsplData->pitch_q = 256;
  rotSceneDsplData->yaw_q = 256;

  *h_SceneDisplacementHandle = temp;
  return 0;
}

void SD_closeHandle(SCENE_DISPLACEMENT_HANDLE h_SceneDisplacementHandle)
{
  free(h_SceneDisplacementHandle);
}


int SD_readSceneDisplacementBits(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal)
{
  unsigned int roll;
  unsigned int pitch;
  unsigned int yaw;
  int temp2;

  *noBitsRead = 0;

  temp2 = SD_DecInt_readBits(h_bitstream,&yaw,9);
  if (temp2 == -1)
  {
    return -1;
  }
  else
  {
    *noBitsRead += temp2;
  }
  temp2 = SD_DecInt_readBits(h_bitstream,&pitch,9);
  if (temp2 == -1)
  {
    return -1;
  }
  else
  {
    *noBitsRead += temp2;
  }
  temp2 = SD_DecInt_readBits(h_bitstream,&roll,9);
  if (temp2 == -1)
  {
    return -1;
  }
  else
  {
    *noBitsRead += temp2;
  }

  *noBitsReadTotal += *noBitsRead;

  SceneDisplacement->sceneDsplData.rotSceneDsplData.roll_q  = (int)roll;
  SceneDisplacement->sceneDsplData.rotSceneDsplData.pitch_q = (int)pitch;
  SceneDisplacement->sceneDsplData.rotSceneDsplData.yaw_q   = (int)yaw;
  SceneDisplacement->noBits  = *noBitsRead;
  return 0;
}

int SD_getSceneDisplacementData(SCENE_DISPLACEMENT_HANDLE const SceneDisplacement,
                                SceneDisplacementData *sceneDisplacementData)
{
    if(!SceneDisplacement)
    {
        return -1;
    }

    CopySceneDisplacementData(sceneDisplacementData, &SceneDisplacement->sceneDsplData);
    return 0;
}

int SD_invquantizeData(SCENE_DISPLACEMENT_HANDLE SceneDisplacement)
{

  float tempfloat = 0.0f;
  int tempint = 0;
  RotSceneDisplacementData *rotSceneDsplData = &SceneDisplacement->sceneDsplData.rotSceneDsplData;


  tempint = rotSceneDsplData->roll_q;
  tempfloat = (float)(180.0f * (tempint/256.0f - 1.0f));
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  rotSceneDsplData->roll = tempfloat;

  tempint = rotSceneDsplData->pitch_q;
  tempfloat = (float)(180.0f * (tempint/256.0f - 1.0f));
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  rotSceneDsplData->pitch = tempfloat;

  tempint = rotSceneDsplData->yaw_q;
  tempfloat = (float)(180.0f * (tempint/256.0f - 1.0f));
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  rotSceneDsplData->yaw = tempfloat;

  return 0;
}


int SD_quantizeData(SCENE_DISPLACEMENT_HANDLE SceneDisplacement)
{

  float tempfloat = 0.0f;
  int tempint = 0;
  RotSceneDisplacementData *rotSceneDsplData = &SceneDisplacement->sceneDsplData.rotSceneDsplData;

  tempfloat = rotSceneDsplData->roll;
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  tempint = (int)floor(((tempfloat/180.0f + 1.0f) * 256.0f) + 0.5f);
  rotSceneDsplData->roll_q = tempint;

  tempfloat = rotSceneDsplData->pitch;
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  tempint = (int)floor(((tempfloat/180.0f + 1.0f) * 256.0f) + 0.5f);
  rotSceneDsplData->pitch_q = tempint;

  tempfloat = rotSceneDsplData->yaw;
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  tempint = (int)floor(((tempfloat/180.0f + 1.0f) * 256.0f) + 0.5f);
  rotSceneDsplData->yaw_q = tempint;

  return 0;
}

int SD_writeSceneDisplacementBitstream(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_ENCODER h_bitstream, unsigned long *no_BitsWritten)
{
  unsigned long noBitsWritten = 0;
  unsigned long noBitsWrittenTotal = 0;

  /* quantize data */
  (void)SD_quantizeData(SceneDisplacement);

  /* write data */
  (void)SD_writeSceneDisplacementBits(SceneDisplacement,h_bitstream, &noBitsWritten,&noBitsWrittenTotal);

  if(PSDI_isEnabled(SceneDisplacement))
  {
      int ret = 0;
      (void)PSDI_quantization(&SceneDisplacement->sceneDsplData.posSceneDsplData);

      ret = PSDI_write(SceneDisplacement, h_bitstream);
      if(ret < 0)
      {
          fprintf(stderr, "Error: Failed to write PSDI bitstream");
          return -1;
      }

      noBitsWrittenTotal += ret;
  }

  *no_BitsWritten = noBitsWrittenTotal;
  SceneDisplacement->noBits = noBitsWrittenTotal;

  return 0;
}

int SD_writeSceneDisplacementBits(SCENE_DISPLACEMENT_HANDLE SceneDisplacement, H_SD_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal)
{
  unsigned long no_BitsWritten = 0;
  RotSceneDisplacementData *rotSceneDsplData = &SceneDisplacement->sceneDsplData.rotSceneDsplData;

  no_BitsWritten += SD_DecInt_writeBits(h_bitstream, rotSceneDsplData->yaw_q, 9);
  no_BitsWritten += SD_DecInt_writeBits(h_bitstream, rotSceneDsplData->pitch_q, 9);
  no_BitsWritten += SD_DecInt_writeBits(h_bitstream, rotSceneDsplData->roll_q, 9);

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += no_BitsWritten;

  return 0;
}

/*------------------------------------------------------------------------------------------------*/
/* Positional scene displacement interface (PSDI)                                                 */
/*------------------------------------------------------------------------------------------------*/

void PSDI_enabled(SCENE_DISPLACEMENT_HANDLE hSceneDspl, unsigned int enabled)
{
    if(!hSceneDspl) return;
    hSceneDspl->bPosSceneDsplEnabled = (enabled != 0);
}

/*------------------------------------------------------------------------------------------------*/

int PSDI_isEnabled(SCENE_DISPLACEMENT_HANDLE hSceneDspl)
{
    return !hSceneDspl ? 0 : hSceneDspl->bPosSceneDsplEnabled;
}

/*------------------------------------------------------------------------------------------------*/

int PSDI_parse(SCENE_DISPLACEMENT_HANDLE hSceneDspl, H_SD_BITSTREAM_DECODER hBitstream)
{
    int ret = 0;
    unsigned int bits = 0;
    PosSceneDisplacementData *posSceneDsplData = NULL;

    if(!hSceneDspl || !hBitstream )
    {
        return -1;
    }

    posSceneDsplData = &hSceneDspl->sceneDsplData.posSceneDsplData;


    ret = SD_DecInt_readBits(hBitstream, &posSceneDsplData->bs_sd_azimuth, PSDI_AZIMUTH_BITS);
    bits += PSDI_AZIMUTH_BITS;
    if(ret != PSDI_AZIMUTH_BITS)
    {
        return -2;
    }

    ret = SD_DecInt_readBits(hBitstream, &posSceneDsplData->bs_sd_elevation, PSDI_ELEVATION_BITS);
    bits += PSDI_ELEVATION_BITS;
    if(ret != PSDI_ELEVATION_BITS)
    {
        return -2;
    }

    ret = SD_DecInt_readBits(hBitstream, &posSceneDsplData->bs_sd_radius, PSDI_RADIUS_BITS);
    bits += PSDI_RADIUS_BITS;
    if(ret != PSDI_RADIUS_BITS)
    {
        return -2;
    }

    return bits;
}

/*------------------------------------------------------------------------------------------------*/

int PSDI_write(SCENE_DISPLACEMENT_HANDLE hSceneDspl, H_SD_BITSTREAM_ENCODER hBitstream)
{
    int bits = 0;
    PosSceneDisplacementData *posSceneDsplData = NULL;

    if(!hSceneDspl || !hBitstream)
    {
        return -1;
    }

    posSceneDsplData = &hSceneDspl->sceneDsplData.posSceneDsplData;
    bits += SD_DecInt_writeBits(hBitstream, posSceneDsplData->bs_sd_azimuth, PSDI_AZIMUTH_BITS);
    bits += SD_DecInt_writeBits(hBitstream, posSceneDsplData->bs_sd_elevation, PSDI_ELEVATION_BITS);
    bits += SD_DecInt_writeBits(hBitstream, posSceneDsplData->bs_sd_radius, PSDI_RADIUS_BITS);

    return bits;
}

/*------------------------------------------------------------------------------------------------*/

int PSDI_quantization(PosSceneDisplacementData *pPosSceneDsplData)
{
    if(!pPosSceneDsplData)
    {
        return -1;
    }

    /* Limit to -180..180 */
    pPosSceneDsplData->sd_azimuth = min(max(pPosSceneDsplData->sd_azimuth, -180.f), 180.f);
    pPosSceneDsplData->bs_sd_azimuth =
        (int)(((pPosSceneDsplData->sd_azimuth + 192.f) * 255.f / (192.f + 190.5f)) + .5f);

    /* Limit to -90..90 */
    pPosSceneDsplData->sd_elevation = min(max(pPosSceneDsplData->sd_elevation, -90.f), 90.f);
    pPosSceneDsplData->bs_sd_elevation = (int)(((pPosSceneDsplData->sd_elevation + 96.f) * 63.f / (96.f + 93.f)) + .5f);

    /* Limit to 0..0.25 */
    pPosSceneDsplData->sd_radius = min(max(pPosSceneDsplData->sd_radius, 0.f), .25f);
    pPosSceneDsplData->bs_sd_radius = (int)((pPosSceneDsplData->sd_radius * 15.f / .25f) + .5f);

    return 0;
};

/*------------------------------------------------------------------------------------------------*/

int PSDI_invQuantization(PosSceneDisplacementData *pPosSceneDsplData)
{
    if(!pPosSceneDsplData)
    {
        return -1;
    }

    pPosSceneDsplData->sd_azimuth = ((signed)pPosSceneDsplData->bs_sd_azimuth - 128) * 1.5f;
    pPosSceneDsplData->sd_azimuth = min(max(pPosSceneDsplData->sd_azimuth, -180.f), 180.f);

    pPosSceneDsplData->sd_elevation = ((signed)pPosSceneDsplData->bs_sd_elevation - 32) * 3.f;
    pPosSceneDsplData->sd_elevation = min(max(pPosSceneDsplData->sd_elevation, -90.f), 90.f);

    pPosSceneDsplData->sd_radius = pPosSceneDsplData->bs_sd_radius / 60.f;

    return 0;
}

/*------------------------------------------------------------------------------------------------*/
