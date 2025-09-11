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

#include "sceneDisplacementInterfaceLib_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------------------------------*/

static char const *PSDI_ID = "azimuth,elevation,radius";

/*------------------------------------------------------------------------------------------------*/
/**
   @brief Function called for each new line
   @param ctx Context
   @param line Next line from the input
 */
typedef void (*ReadLineCallback)(void *ctx, char *line);

/*------------------------------------------------------------------------------------------------*/

/**
   @brief Read one CSV and interpret as float value
   @param string CSV string
   @param value The extracted value; not changed if no value could be read from #string
   @return 0 if a value could be read from #string, else != 0
*/
static int GetCsvFloat(char *string, float *value)
{
    char *tok = strtok(string, ",");
    if(!tok) return -1;

    if(value)
    {
        *value = (float)atof(tok);
    }

    return 0;
}

/**
   @brief Get a comma separated value with default value
   @param string CSV string
   @return CSV on success, default value (0.f) if no value could be extracted
 */
static float GetCsvFloatDefault(char *string)
{
    float value = 0.f;
    return GetCsvFloat(string, &value) == 0 ? value : 0.f;
}

/**
   @brief Check for the PSDI identifier #PSDI_ID
   @param context Must be valid SCENE_DISPLACEMENT_HANDLE
   @param string The string to search
 */
static void HasPsdiData(void *context, char *string)
{
    SCENE_DISPLACEMENT_HANDLE hSceneDspl = NULL;
    if(!context || !string)
    {
        return;
    }

    hSceneDspl = (SCENE_DISPLACEMENT_HANDLE)context;
    PSDI_enabled(hSceneDspl, (NULL != strstr(string, PSDI_ID)));
}

/**
   @brief Read next set of scene displacement parameters from input file

   Input file format (CSV): Each line contains comma speparated SD parameters:
   yaw,pitch,roll[,azimuth,elevation,radius]

   @param context #SCENE_DISPLACEMENT_HANDLE passed in from ReadLine(FILE*, ReadLineCallback, void*)
   @param line The current line to parse
*/
static void ReadParameterSet(void *context, char *line)
{
    SceneDisplacementData sceneDsplData = { 0 };
    RotSceneDisplacementData *rotSceneDsplData = &sceneDsplData.rotSceneDsplData;
    PosSceneDisplacementData *posSceneDsplData = &sceneDsplData.posSceneDsplData;

    SCENE_DISPLACEMENT_HANDLE hSceneDspl = NULL;
    if(!context || !line)
    {
        return;
    }

    hSceneDspl = (SCENE_DISPLACEMENT_HANDLE)context;

    rotSceneDsplData->yaw = GetCsvFloatDefault(line);
    rotSceneDsplData->pitch = GetCsvFloatDefault(NULL);
    rotSceneDsplData->roll = GetCsvFloatDefault(NULL);

    if(PSDI_isEnabled(hSceneDspl))
    {
        posSceneDsplData->sd_azimuth = GetCsvFloatDefault(NULL);
        posSceneDsplData->sd_elevation = GetCsvFloatDefault(NULL);
        posSceneDsplData->sd_radius = GetCsvFloatDefault(NULL);
    }

    SD_setSceneDisplacementData(hSceneDspl, &sceneDsplData);
}

/**
   @brief Read next line from input file

   Input file format (CSV): Each line contains comma speparated SD parameters:
   yaw,pitch,roll[,azimuth,elevation,radius]

   @param fSd Valid file handle to scene displacement text file
   @param callback ReadLineCallback function; called if a new line could be read from the file
   @param context Context passed to the callback function
   @return 0 on success, else < 0
 */
static int ReadLine(FILE *fSd, ReadLineCallback callback, void *context)
{
    char buf[512] = { 0 };
    char *line = NULL;

    if(!fSd || feof(fSd))
    {
        return -1;
    }

    line = fgets(buf, 512, fSd);
    if(!line)
    {
        return -1;
    }

    if(callback)
    {
        callback(context, line);
    }

    return 0;
}

/*------------------------------------------------------------------------------------------------*/

static int WriteInterfaceBitstream(char const *sceneDisplacementInputFile,
                                   SCENE_DISPLACEMENT_HANDLE hSceneDspl,
                                   H_SD_BITSTREAM_ENCODER hBitEnc)
{
    unsigned long no_BitsWritten = 0;

    FILE *fInput = fopen(sceneDisplacementInputFile, "r");
    if(!fInput)
    {
        fprintf(stderr, "[Error] Failed to open file: '%s'\n", sceneDisplacementInputFile);
        return -1;
    }

    /*Presence of PSDI data is indicated on the first line */
    ReadLine(fInput, &HasPsdiData, hSceneDspl);

    while(ReadLine(fInput, &ReadParameterSet, hSceneDspl) == 0)
    {
        SD_writeSceneDisplacementBitstream(hSceneDspl, hBitEnc, &no_BitsWritten);
    }

    fclose(fInput);

    return 0;
}

/*------------------------------------------------------------------------------------------------*/

static void ReadInterfaceBitstream(SCENE_DISPLACEMENT_HANDLE hSceneDspl, H_SD_BITSTREAM_DECODER hBitDec)
{
    SceneDisplacementData sceneDisplacementData = { 0 };
    int frame = 0;
    unsigned long nBitsRead = 0;

    while(0 == SD_readSceneDisplacementBitstream(hSceneDspl, hBitDec, &nBitsRead))
    {
        SD_getSceneDisplacementData(hSceneDspl, &sceneDisplacementData);
        fprintf(stdout
                , "[frame %.4d] yaw=%.2f; pitch=%.2f; roll=%.2f"
                , frame
                , sceneDisplacementData.rotSceneDsplData.yaw
                , sceneDisplacementData.rotSceneDsplData.pitch
                , sceneDisplacementData.rotSceneDsplData.roll);

        if(PSDI_isEnabled(hSceneDspl))
        {
            fprintf(stdout
                    , "; azimuth=%.2f; elevation=%.2f; radius=%.2f\n"
                    , sceneDisplacementData.posSceneDsplData.sd_azimuth
                    , sceneDisplacementData.posSceneDsplData.sd_elevation
                    , sceneDisplacementData.posSceneDsplData.sd_radius);
        }
        else
        {
            fprintf(stdout, "\n");
        }

        ++frame;
    }
}

/*------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  int error = 0;
  int k = 0;

  /* encoder side */
  H_SD_BITSTREAM_ENCODER h_bitstream_SD_Enc = NULL;
  SCENE_DISPLACEMENT_HANDLE h_SD_Enc = NULL;


  /* decoder side */
  H_SD_BITSTREAM_DECODER h_bitstream_SD_Dec = NULL;
  SCENE_DISPLACEMENT_HANDLE h_SD_Dec = NULL;

  char *sceneDisplacementBitstreamFile = NULL;
  char *sceneDisplacementInputFile = NULL;

  unsigned int psdiEnabled = 0;
  int required = 0;

  fprintf(stderr, "\n\n");

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                   Scene Displacement Interface Module                       *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                                  %s                                *\n", __DATE__);
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*******************************************************************************\n");
  fprintf(stdout, "\n");

  /* *********************************** COMMANDLINE PARSING ************************************* */

  for (k = 1; k < argc; ++k)
  {
    /* output path for bitstream writer */
    if (!strcmp(argv[k], "-of"))     /* Required */
    {
      sceneDisplacementBitstreamFile = argv[k+1];
      k++;
      required++;
    }

    /* input path for scene displacement angles file */
    if (!strcmp(argv[k], "-sd"))     /* Required */
    {
      sceneDisplacementInputFile = argv[k+1];
      k++;
      required++;
    }
  }

  if (required < 2)
  {
    fprintf(stderr, "Invalid arguments, usage:\n");
    fprintf(stderr, "-of <output.bs> path to output 'bitstream' file with scene displacement data\n");
    fprintf(stderr, "-sd <sceneDisplacement.txt> path to text file containing scene displacement angle data\n");
    return -1;
  }



  /***************** SCENE DISPLACEMENT INTERFACE ************************/

  /* init handles + bitstream writers */
  error = SD_initHandle(&h_SD_Enc);
  if(error != 0)
  {
      return -1;
  }
  error = SD_DecInt_initBitstreamWriter(&h_bitstream_SD_Enc, sceneDisplacementBitstreamFile);
  if(error != 0)
  {
      SD_closeHandle(h_SD_Enc);
      return -2;
  }

  error = WriteInterfaceBitstream(sceneDisplacementInputFile, h_SD_Enc, h_bitstream_SD_Enc);
  psdiEnabled = PSDI_isEnabled(h_SD_Enc);

  /* finish writing */
  SD_DecInt_closeBitstreamWriter(h_bitstream_SD_Enc);
  SD_closeHandle(h_SD_Enc);

  if(error != 0)
  {
      return -3;
  }

  /* start scene displacement reading */
  error = SD_initHandle(&h_SD_Dec);
  if(error != 0)
  {
      return -4;
  }

  error = SD_DecInt_initBitstreamReader(&h_bitstream_SD_Dec, sceneDisplacementBitstreamFile);
  if(error != 0)
  {
      SD_closeHandle(h_SD_Dec);
      return -5;
  }

  PSDI_enabled(h_SD_Dec, psdiEnabled);
  ReadInterfaceBitstream(h_SD_Dec, h_bitstream_SD_Dec);

  /* finish reading */
  SD_DecInt_closeBitstreamReader(h_bitstream_SD_Dec);
  SD_closeHandle(h_SD_Dec);

  return 0;
}
