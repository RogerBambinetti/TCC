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

#include "hoa_matrix_dec.h"
#include "readonlybitbuf.h"
#include "cicp2geometry.h"

typedef struct {
  char inputFilename[FILENAME_MAX];
  char outputFilenameMatrix[FILENAME_MAX];
  /*int inputConfigIndex;*/
  int inputNumHoaCoeff;
  int outputConfigIndex;
  char outputConfigGeometryFilename[FILENAME_MAX];
} ConfigDownmixMatrixDecoder;


static int parseCommandline(int argc, char* argv[], ConfigDownmixMatrixDecoder* config)
{
  int i;
  int ret = 0;

  config->inputFilename[0] = '\0';
  config->outputFilenameMatrix[0] = '\0';
  /*config->inputConfigIndex = -2;*/ /* invalid value */
  config->inputNumHoaCoeff = 0;
  config->outputConfigIndex = -2; /* invalid value */
  config->outputConfigGeometryFilename[0] = '\0';

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-if")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputFilename, argv[i]);
    } else if (!strcmp(argv[i], "-of_mat")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputFilenameMatrix, argv[i]);
      } else if (!strcmp(argv[i], "-order")) { /* Required */
      ++i; if (i >= argc) { ret = -1; break; }
      config->inputNumHoaCoeff = atoi(argv[i])+1;
      config->inputNumHoaCoeff = (config->inputNumHoaCoeff)*(config->inputNumHoaCoeff);
    } else if (!strcmp(argv[i], "-CICP")) { /* Required */
      ++i; if (i >= argc) { ret = -1; break; }
      config->outputConfigIndex = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-geo")) { /* Optional */
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
  if (config->inputNumHoaCoeff == 0) {
    fprintf(stderr, "Missing or wrong command line parameter: HOA order (-order HOA_ORDER)\n");
    ret = -1;
  }
  if (config->outputFilenameMatrix[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: output filename for matrix (-of_mat FILE)\n");
    ret = -1;
  }
  if ((config->outputConfigIndex == -2) || (config->outputConfigIndex == 0) || (config->outputConfigIndex >= 64)) {
    fprintf(stderr, "Missing or wrong command line parameter: CICP index (-CICP CICP_INDEX)\n");
    ret = -1;
  }
  if ((config->outputConfigIndex == -1) && (config->outputConfigGeometryFilename[0] == '\0')) {
    fprintf(stderr, "Missing or wrong command line parameter: geometry filename (-geo FILE)\n");
    ret = -1;
  }

  return ret;
}

static int hoaDecoderWrapper(ConfigDownmixMatrixDecoder* config)
{
  int i, j;
  int outputCount;
  SpeakerInformation outputConfig[HOA_MATRIX_MAX_SPEAKER_COUNT];

  robitbuf readbitBuffer;
  robitbufHandle hBitBuffer = &readbitBuffer;
  unsigned char bitstreamDataRead[HOA_MATRIX_BITBUFSIZE] = {0};

  FILE* fInput = NULL;
  FILE* fOutput = NULL;
  int outputIndex = -1;
  double flatDecodedMatrix[HOA_MATRIX_MAX_HOA_COEF_COUNT * HOA_MATRIX_MAX_SPEAKER_COUNT];
  int bytesRead = 0;
  int ret = 0;
  int bitsLeft = 0;
  int paddingBits = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY outputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_ERROR geometry_error = CICP2GEOMETRY_OK;
  int numLFEs = 0;

  outputIndex = config->outputConfigIndex;

  fInput = fopen(config->inputFilename, "rb");
  if (fInput == NULL) {
    fprintf(stderr, "Error opening of input file %s\n", config->inputFilename);
    return -1;
  }
  bytesRead = fread(bitstreamDataRead, 1, HOA_MATRIX_BITBUFSIZE, fInput);
  robitbuf_Init(hBitBuffer, bitstreamDataRead, bytesRead << 3, 0);
  fclose(fInput);
   
  printf("CICP Index %2d\n", outputIndex);
  if (outputIndex == -1) {
    printf("output geometry file %s\n", config->outputConfigGeometryFilename);
  }
  printf("\n");

  /* prepare output config */
  if (outputIndex != -1) {
    geometry_error = cicp2geometry_get_geometry_from_cicp(outputIndex, outputGeometry, &outputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting CICP index %d to geometry, error code %d\n", outputIndex, geometry_error);
      return -1;
    }
    outputCount += numLFEs;
  } else {
    geometry_error = cicp2geometry_get_geometry_from_file(config->outputConfigGeometryFilename, outputGeometry, &outputCount, &numLFEs);
    if (geometry_error != CICP2GEOMETRY_OK) {
      fprintf(stderr, "Error converting CICP geometry file %s to geometry, error code %d\n",
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
  ret = DecodeHoaMatrix(config->inputNumHoaCoeff, /*outputIndex, */
	  outputCount, outputConfig,
    hBitBuffer, flatDecodedMatrix);
  if (ret < 0) {
    fprintf(stderr, "Decoding error: DecodeHoaMatrix got invalid bistream\n");
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

  /* write decoded rendering matrix */
  fOutput = fopen(config->outputFilenameMatrix, "wt");
  if (fOutput == NULL) {
    fprintf(stderr, "Error opening of output matrix file %s\n", config->outputFilenameMatrix);
    return -1;
  }
  fprintf(fOutput, " # Number of Loudspeaker\n");
  fprintf(fOutput, " %d\n", outputCount);	
  fprintf(fOutput, " # Number of HOA Coefficients\n");
  fprintf(fOutput, " %d\n", config->inputNumHoaCoeff);	 
  fprintf(fOutput, " # CICP Loudspeaker Config\n");
  for (i = 0; i < outputCount; ++i) {
	  if (outputGeometry[i].LFE){
		   fprintf(fOutput, "LFE_");
	  }
	  fprintf(fOutput, "R%04d_A%+04d_E%+03d\n", 200, outputGeometry[i].Az, outputGeometry[i].El); /*need to add radius once this information becomes available */
  }
  fprintf(fOutput, " # HOA Matrix Values\n");
  for (i = 0; i < config->inputNumHoaCoeff; ++i) {
    for (j = 0; j < outputCount; ++j) {      
	  fprintf(fOutput, " %.21g\n", flatDecodedMatrix[i * outputCount + j]);	
    }    
  }
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
  fprintf(stdout, "*                   HOA Rendering Matrix Decoder Module                       *\n");
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

  /*ret = decoderWrapper(&config);*/
  ret = hoaDecoderWrapper(&config);

  return ret;
}
