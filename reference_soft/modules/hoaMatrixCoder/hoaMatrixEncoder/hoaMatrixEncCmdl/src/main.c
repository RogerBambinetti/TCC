/***********************************************************************************
 
 This software module was originally developed by 
 
 Qualcomm Techonologies, Inc. (QTI)
 
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
 
 Qualcomm Techonologies, Inc. (QTI) retains full right to modify and use the code 
 for its own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hoa_matrix_enc.h"
#include "writeonlybitbuf.h"
#include "cicp2geometry.h"

typedef struct {
  char inputFilenameMatrix[FILENAME_MAX];
  char outputFilename[FILENAME_MAX];
  int outputConfigIndex;
  char outputConfigGeometryFilename[FILENAME_MAX];
  int precisionLevel;
} ConfigHoaMatrixEncoder;


static int parseCommandline(int argc, char* argv[], ConfigHoaMatrixEncoder* config)
{
  int i;
  int ret = 0;

  config->inputFilenameMatrix[0] = '\0';
  config->outputFilename[0] = '\0';
  config->outputConfigIndex = -2; /* invalid value */
  config->outputConfigGeometryFilename[0] = '\0';
  config->precisionLevel = 1;

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-if")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputFilenameMatrix, argv[i]);
    } else if (!strcmp(argv[i], "-of")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputFilename, argv[i]);
    } else if (!strcmp(argv[i], "-CICP")) { /* Required */
      ++i; if (i >= argc) { ret = -1; break; }
      config->outputConfigIndex = atoi(argv[i]);
	} else if (!strcmp(argv[i], "-precision")) { /* Optional */
      ++i; if (i >= argc) { ret = -1; break; }
      config->precisionLevel = atoi(argv[i]);
    } else if (!strcmp(argv[i], "-geo")) { /* Optional */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputConfigGeometryFilename, argv[i]);
    } else {
      fprintf(stderr, "Unknown command line parameter: %s\n", argv[i]);
      ret = -1; break;
    }
  }

  /* check the validity of the parameters */
  if (config->inputFilenameMatrix[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: input filename for matrix (-if_mat FILE)\n");
    ret = -1;
  }
  if (config->outputFilename[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: output filename (-of FILE)\n");
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


/* ===================== PARSER CODE =============================== */

static int hoaMtxEncoderWrapper(ConfigHoaMatrixEncoder* config)
{  
  int i, j;
  int hoaMtxOutputCount, hoaMtxInputCount;
  int outputCount;
  SpeakerInformation outputConfig[HOA_MATRIX_MAX_SPEAKER_COUNT];
  int precisionLevel; 

  wobitbuf writebitbuffer;
  wobitbufHandle hBitBuffer = &writebitbuffer;
  unsigned char bitstreamDataWrite[HOA_MATRIX_BITBUFSIZE] = {0};

  FILE* fInput = NULL;
  FILE* fOutput = NULL;
  int outputIndex = -1;
  double flatHoaMatrix[HOA_MATRIX_MAX_HOA_COEF_COUNT * HOA_MATRIX_MAX_SPEAKER_COUNT];
  double* matrix = NULL;
  int error = 0;
  int byteCount = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY outputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_ERROR geometry_error = CICP2GEOMETRY_OK;
  int numLFEs = 0;

  precisionLevel = config->precisionLevel;
  outputIndex = config->outputConfigIndex;
  printf("CICP index %2d\n", outputIndex);
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


    /* read the hoa matrix from the input file */
  fInput = fopen(config->inputFilenameMatrix, "rt");
  if (fInput == NULL) {
    fprintf(stderr, "Error opening of matrix input file %s\n", config->inputFilenameMatrix);
    return -1;
  }


  fscanf(fInput, "%d", &hoaMtxOutputCount);
  if (hoaMtxOutputCount != outputCount) {
	  fprintf(stderr, "Number of output channels defined in rendering matrix (%d) differs from CICP channel count (%d)\n", hoaMtxOutputCount, outputCount);
    return -1;
  }
  if (hoaMtxOutputCount > HOA_MATRIX_MAX_SPEAKER_COUNT) {
	  fprintf(stderr, "Number of output channels defined in rendering matrix (%d) must be smaller than %d\n", hoaMtxOutputCount, HOA_MATRIX_MAX_SPEAKER_COUNT+1);
    return -1;
  }

  fscanf(fInput, " %d", &hoaMtxInputCount);
  printf("Number of HOA coefficients: %d\n",hoaMtxInputCount);
    if (hoaMtxInputCount > HOA_MATRIX_MAX_HOA_COEF_COUNT) {
	  fprintf(stderr, "Order of HOA Rendering Matrix (%d) must be smaller that %d \n", hoaMtxInputCount, HOA_MATRIX_MAX_HOA_ORDER+1);
    return -1;
  }
  for (i = 0; i < hoaMtxInputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      fscanf(fInput, " %lf", &(flatHoaMatrix[i * outputCount + j]));
	  fscanf(fInput, "\n");
    }
  }
  fclose(fInput);
  matrix = (double*) flatHoaMatrix;
  

    /* initialize bit buffer */
  wobitbuf_Init(hBitBuffer, bitstreamDataWrite, sizeof(bitstreamDataWrite) << 3, 0);
  
  error = EncodeHoaMatrix(hoaMtxInputCount, /*outputIndex, */
						   outputCount, outputConfig,
						   precisionLevel, hBitBuffer, matrix);
  if (error)
  {
    fprintf(stderr, "Error Encoding HOA Matrix\n");
    return -1;
  }



   /* write compressed data, the last incomplete byte is padded with zero bits */
  fOutput = fopen(config->outputFilename, "wb");
  if (fOutput == NULL) {
    fprintf(stderr, "Error opening of output file %s\n", config->outputFilename);
    return -1;
  }
  byteCount = (wobitbuf_GetBitsWritten(hBitBuffer) + 7) / 8;
  fwrite(bitstreamDataWrite, 1, byteCount, fOutput);
  fclose(fOutput);


	printf("\n");
	printf("Encoding successful.\n");
	return 0;
}

int main(int argc, char* argv[])
{
  ConfigHoaMatrixEncoder config;
  int ret = 0;

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                      HOA Rendering Matrix Encoder Module                    *\n");
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

  ret = hoaMtxEncoderWrapper(&config);

  return ret;
}
