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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dmx_matrix_dec.h"
#include "readonlybitbuf.h"
#include "cicp2geometry.h"


typedef struct {
  char inputFilename[FILENAME_MAX];
  char outputFilenameMatrix[FILENAME_MAX];
  char outputFilenameEqualizer[FILENAME_MAX];
  int inputConfigIndex;
  char inputConfigGeometryFilename[FILENAME_MAX];
  int outputConfigIndex;
  char outputConfigGeometryFilename[FILENAME_MAX];
} ConfigDownmixMatrixDecoder;


static int parseCommandline(int argc, char* argv[], ConfigDownmixMatrixDecoder* config)
{
  int i;
  int ret = 0;

  config->inputFilename[0] = '\0';
  config->outputFilenameMatrix[0] = '\0';
  config->outputFilenameEqualizer[0] = '\0';
  config->inputConfigIndex = -2; /* invalid value */
  config->inputConfigGeometryFilename[0] = '\0';
  config->outputConfigIndex = -2; /* invalid value */
  config->outputConfigGeometryFilename[0] = '\0';

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-if")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputFilename, argv[i]);
    } else if (!strcmp(argv[i], "-of_mat")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputFilenameMatrix, argv[i]);
    } else if (!strcmp(argv[i], "-of_eq")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputFilenameEqualizer, argv[i]);
    } else if (!strcmp(argv[i], "-CICPin")) { /* Required */
      ++i; if (i >= argc) { ret = -1; break; }
      config->inputConfigIndex = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-CICPout")) { /* Required */
      ++i; if (i >= argc) { ret = -1; break; }
      config->outputConfigIndex = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-inGeo")) { /* Optional */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputConfigGeometryFilename, argv[i]);
    } else if (!strcmp(argv[i], "-outGeo")) { /* Optional */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputConfigGeometryFilename, argv[i]);
    } else {
      fprintf(stderr, "Unknown command line parameter: %s\n", argv[i]);
      ret = -1; break;
    }
  }

  /* check the validity of the parameters */
  if (config->inputFilename[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: input filename (-if FILE)\n");
    ret = -1;
  }
  if (config->outputFilenameMatrix[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: output filename for matrix (-of_mat FILE)\n");
    ret = -1;
  }
  if (config->outputFilenameEqualizer[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: output filename for equalizer (-of_eq FILE)\n");
    ret = -1;
  }
  if ((config->inputConfigIndex == -2) || (config->inputConfigIndex == 0) || (config->inputConfigIndex >= 64)) {
    fprintf(stderr, "Missing or wrong command line parameter: input CICP index (-CICPin CICP_INDEX)\n");
    ret = -1;
  }
  if ((config->inputConfigIndex == -1) && (config->inputConfigGeometryFilename[0] == '\0')) {
    fprintf(stderr, "Missing or wrong command line parameter: input geometry filename (-inGeo FILE)\n");
    ret = -1;
  }
  if ((config->outputConfigIndex == -2) || (config->outputConfigIndex == 0) || (config->outputConfigIndex >= 64)) {
    fprintf(stderr, "Missing or wrong command line parameter: output CICP index (-CICPout CICP_INDEX)\n");
    ret = -1;
  }
  if ((config->outputConfigIndex == -1) && (config->outputConfigGeometryFilename[0] == '\0')) {
    fprintf(stderr, "Missing or wrong command line parameter: output geometry filename (-outGeo FILE)\n");
    ret = -1;
  }

  return ret;
}


static int decoderWrapper(ConfigDownmixMatrixDecoder* config)
{
  int i, j;
  int inputCount;
  SpeakerInformation inputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int outputCount;
  SpeakerInformation outputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];

  robitbuf readbitBuffer;
  robitbufHandle hBitBuffer = &readbitBuffer;
  unsigned char bitstreamDataRead[DMX_MATRIX_BITBUFSIZE] = {0};

  FILE* fInput = NULL;
  FILE* fOutput = NULL;
  int inputIndex = -1;
  int outputIndex = -1;
  float flatDecodedMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT];
  eqConfigStruct eqConfig;
  int bytesRead = 0;
  int ret = 0;
  int bitsLeft = 0;
  int paddingBits = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY inputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_CHANNEL_GEOMETRY outputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_ERROR geometry_error = CICP2GEOMETRY_OK;
  int numLFEs = 0;


  inputIndex = config->inputConfigIndex;
  outputIndex = config->outputConfigIndex;

  fInput = fopen(config->inputFilename, "rb");
  if (fInput == NULL) {
    fprintf(stderr, "Error opening of input file %s\n", config->inputFilename);
    return -1;
  }
  bytesRead = fread(bitstreamDataRead, 1, DMX_MATRIX_BITBUFSIZE, fInput);
  robitbuf_Init(hBitBuffer, bitstreamDataRead, bytesRead << 3, 0);
  fclose(fInput);

  printf("CICP inputIndex %2d\n", inputIndex);
  if (inputIndex == -1) {
    printf("input geometry file %s\n", config->inputConfigGeometryFilename);
  }
  printf("CICP outputIndex %2d\n", outputIndex);
  if (outputIndex == -1) {
    printf("output geometry file %s\n", config->outputConfigGeometryFilename);
  }
  printf("\n");

  /* prepare the input config */
  if (inputIndex != -1) {
    geometry_error = cicp2geometry_get_geometry_from_cicp(inputIndex, inputGeometry, &inputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting CICP input index %d to geometry, error code %d\n", inputIndex, geometry_error);
      return -1;
    }
    inputCount += numLFEs;
  } else {
    geometry_error = cicp2geometry_get_geometry_from_file(config->inputConfigGeometryFilename, inputGeometry, &inputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting input geometry file %s to geometry, error code %d\n",
        config->inputConfigGeometryFilename, geometry_error);
      return -1;
    }
    inputCount += numLFEs;

    /* check whether the input geometry actually matches a CICP index and update inputIndex accordingly */
    geometry_error = cicp2geometry_get_cicpIndex_from_geometry(inputGeometry, inputCount - numLFEs, numLFEs, &inputIndex);
    if ((inputIndex == CICP2GEOMETRY_CICP_INVALID) || (geometry_error == CICP2GEOMETRY_INVALID_CICP_INDEX)) {
      inputIndex = -1; /* no matching CICP index was found for the input geometry */
      geometry_error = CICP2GEOMETRY_OK;
    }
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error checking if input geometry matches a CICP index, error code %d\n", geometry_error);
      return -1;
    }
  }
  /* copy the input geometry to inputConfig */
  for (i = 0; i < inputCount; ++i) {
    inputConfig[i].azimuth = (short) inputGeometry[i].Az;
    inputConfig[i].elevation = (short) inputGeometry[i].El;
    inputConfig[i].isLFE = (short) inputGeometry[i].LFE;
  }
  /* consistency check: no two identical speakers allowed */
  for (i = 0; i < inputCount - 1; ++i) {
    SpeakerInformation si = inputConfig[i];
    for (j = i + 1; j < inputCount; ++j) {
      if ((si.azimuth == inputConfig[j].azimuth) && (si.elevation == inputConfig[j].elevation)
        && (si.isLFE == inputConfig[j].isLFE)) {
          fprintf(stderr, "Error in input geometry, two identical speakers detected: %d and %d\n", i, j);
          return -1;
      }
    }
  }

  /* prepare output config */
  if (outputIndex != -1) {
    geometry_error = cicp2geometry_get_geometry_from_cicp(outputIndex, outputGeometry, &outputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting CICP output index %d to geometry, error code %d\n", outputIndex, geometry_error);
      return -1;
    }
    outputCount += numLFEs;
  } else {
    geometry_error = cicp2geometry_get_geometry_from_file(config->outputConfigGeometryFilename, outputGeometry, &outputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting CICP output geometry file %s to geometry, error code %d\n",
        config->outputConfigGeometryFilename, geometry_error);
      return -1;
    }
    outputCount += numLFEs;

    /* check whether the output geometry actually matches a CICP index and update outputIndex accordingly */
    geometry_error = cicp2geometry_get_cicpIndex_from_geometry(outputGeometry, outputCount - numLFEs, numLFEs, &outputIndex);
    if ((outputIndex == CICP2GEOMETRY_CICP_INVALID) || (geometry_error == CICP2GEOMETRY_INVALID_CICP_INDEX)) {
      outputIndex = -1; /* no matching CICP index was found for the output geometry */
      geometry_error = CICP2GEOMETRY_OK;
    }
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error checking if output geometry matches a CICP index, error code %d\n", geometry_error);
      return -1;
    }
  }
  /* copy the output geometry to outputConfig */
  for (i = 0; i < outputCount; ++i) {
    outputConfig[i].azimuth = (short) outputGeometry[i].Az;
    outputConfig[i].elevation = (short) outputGeometry[i].El;
    outputConfig[i].isLFE = (short) outputGeometry[i].LFE;
  }
  /* consistency check: no two identical speakers allowed */
  for (i = 0; i < outputCount - 1; ++i) {
    SpeakerInformation si = outputConfig[i];
    for (j = i + 1; j < outputCount; ++j) {
      if ((si.azimuth == outputConfig[j].azimuth) && (si.elevation == outputConfig[j].elevation)
        && (si.isLFE == outputConfig[j].isLFE)) {
          fprintf(stderr, "Error in output geometry, two identical speakers detected: %d and %d\n", i, j);
          return -1;
      }
    }
  }

  /* decode the data read */
  eqConfig.numEQs = 0;
  eqConfig.eqParams = NULL;
  eqConfig.eqMap = NULL;
  ret = DecodeDownmixMatrix(inputIndex, inputCount, inputConfig, outputIndex, outputCount, outputConfig,
    hBitBuffer, flatDecodedMatrix, &eqConfig);
  if (ret < 0) {
    fprintf(stderr, "Decoding error: DecodeDownmixMatrix got invalid bistream\n");
    return -1;
  }

  bitsLeft = robitbuf_GetBitsAvail(hBitBuffer);
  if ((bitsLeft < 0) || (bitsLeft >= 8)) {
    fprintf(stderr, "Decoding error: incorrect number of padding bits left\n");
    return -1;
  }
  paddingBits = robitbuf_ReadBits(hBitBuffer, bitsLeft);
  assert(robitbuf_GetBitsAvail(hBitBuffer) == 0);
  if (0 != paddingBits) {
    fprintf(stderr, "Decoding error: padding bits different from zero\n");
    return -1;
  }

  /* write decoded downmix matrix */
  fOutput = fopen(config->outputFilenameMatrix, "wt");
  if (fOutput == NULL) {
    fprintf(stderr, "Error opening of output matrix file %s\n", config->outputFilenameMatrix);
    return -1;
  }
  for (i = 0; i < inputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      fprintf(fOutput, " %7.5f", flatDecodedMatrix[i * outputCount + j]);
    }
    if (i != inputCount - 1) fprintf(fOutput, "\n");
  }
  fclose(fOutput);

  /* write decoded equalizer config */
  fOutput = fopen(config->outputFilenameEqualizer, "wt");
  if (fOutput == NULL) {
    fprintf(stderr, "Error opening of output equalizer file %s\n", config->outputFilenameEqualizer);
    return -1;
  }
  fprintf(fOutput, "%d\n", inputCount);
  fprintf(fOutput, "%d\n", eqConfig.numEQs);
  for (i = 0; i < eqConfig.numEQs; ++i) {
    fprintf(fOutput, "%f\n", eqConfig.eqParams[i].G);
    fprintf(fOutput, "%d\n", eqConfig.eqParams[i].nPkFilter);
    for (j = 0; j < eqConfig.eqParams[i].nPkFilter; ++j) {
      fprintf(fOutput, "%f\n", eqConfig.eqParams[i].pkFilterParams[j].f);
      fprintf(fOutput, "%f\n", eqConfig.eqParams[i].pkFilterParams[j].q);
      fprintf(fOutput, "%f\n", eqConfig.eqParams[i].pkFilterParams[j].g);
    }
    free(eqConfig.eqParams[i].pkFilterParams);
  }
  free(eqConfig.eqParams);
  for (i = 0; i < inputCount; ++i) {
    fprintf(fOutput, "%d\n", (eqConfig.numEQs != 0) ? eqConfig.eqMap[i] : 0);
  }
  free(eqConfig.eqMap);
  fclose(fOutput);

  printf("\n");
  printf("Decoding successful.\n");

  return 0;
}


int main(int argc, char* argv[])
{
  ConfigDownmixMatrixDecoder config;
  int ret = 0;

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                      Downmix Matrix Decoder Module                          *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                                  %s                                *\n", __DATE__);
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*******************************************************************************\n");
  fprintf(stdout, "\n");

  /* parse the command line parameters */
  if (parseCommandline(argc, argv, &config) < 0) {
    return -1;
  }

  ret = decoderWrapper(&config);

  return ret;
}
