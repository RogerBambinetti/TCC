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

#include "dmx_matrix_enc.h"
#include "writeonlybitbuf.h"
#include "cicp2geometry.h"


typedef struct {
  char inputFilenameMatrix[FILENAME_MAX];
  char inputFilenameEqualizer[FILENAME_MAX];
  char outputFilename[FILENAME_MAX];
  int inputConfigIndex;
  char inputConfigGeometryFilename[FILENAME_MAX];
  int outputConfigIndex;
  char outputConfigGeometryFilename[FILENAME_MAX];
} ConfigDownmixMatrixEncoder;


static int parseCommandline(int argc, char* argv[], ConfigDownmixMatrixEncoder* config)
{
  int i;
  int ret = 0;

  config->inputFilenameMatrix[0] = '\0';
  config->inputFilenameEqualizer[0] = '\0';
  config->outputFilename[0] = '\0';
  config->inputConfigIndex = -2; /* invalid value */
  config->inputConfigGeometryFilename[0] = '\0';
  config->outputConfigIndex = -2; /* invalid value */
  config->outputConfigGeometryFilename[0] = '\0';

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-if_mat")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputFilenameMatrix, argv[i]);
    } else if (!strcmp(argv[i], "-if_eq")) { /* Optional */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->inputFilenameEqualizer, argv[i]);
    } else if (!strcmp(argv[i], "-of")) { /* Required */
      ++i; if ((i >= argc) || (strlen(argv[i]) >= FILENAME_MAX)) { ret = -1; break; }
      strcpy(config->outputFilename, argv[i]);
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
  if (config->inputFilenameMatrix[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: input filename for matrix (-if_mat FILE)\n");
    ret = -1;
  }
  if (config->outputFilename[0] == '\0') {
    fprintf(stderr, "Missing or wrong command line parameter: output filename (-of FILE)\n");
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


/* begin eq_params_parser_code */

/* ================================================================= */
/* ===================== PARSER CODE =============================== */

static int readEqParamsFromFile( char *filename, int numCh, int *numEQs, eqParamsStruct **eqParams, int **eqMap )
{
    FILE* fileHandle;
    char line[512] = {0};
    int numChEq, m, n;

    /* open file */
    fileHandle = fopen(filename, "r");
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open EQ parameters input file\n");
        return -1;
    } else {
        fprintf(stderr, "\nFound EQ parameters input file: %s.\n", filename );
    }

    fprintf(stderr, "Reading EQ parameters from file ... \n");

    /* read/check number of input channels */
    if (fgets(line, 511, fileHandle) == NULL ){
        fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
    } else {
        numChEq = atoi(line);
        if( numChEq != numCh ){
            fprintf(stderr, "ERROR: EQ parameters file defines settings for %d input channels but FormatConverter expects %d\n",numChEq,numCh);
            return -1;
        } else
            fprintf(stderr, "EQ parameters file defines settings for %d input channels\n",numChEq);
    }

    /* read number of EQ definitions */
    if (fgets(line, 511, fileHandle) == NULL ){
        fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
    } else {
        *numEQs = atoi(line);
        if( *numEQs > numCh ){
            fprintf(stderr, "ERROR: more EQs than input channels defined\n");
            return -1;
        } else
            fprintf(stderr, "EQ parameters file defines %d EQs\n",*numEQs);
    }

    /* allocate *numEQs eqParamsStructs */
    *eqParams = (eqParamsStruct*)calloc(*numEQs,sizeof(eqParamsStruct));

    /* read *numEQs EQ definitions */
    for(n=0; n<*numEQs; n++){
        /* read global gain for current EQ */
        if (fgets(line, 511, fileHandle) == NULL )
            fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
        else
            (*eqParams)[n].G = (float) atof(line);
        /* read number of cascaded peak filters for current EQ */
        if (fgets(line, 511, fileHandle) == NULL )
            fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
        else
            (*eqParams)[n].nPkFilter = atoi(line);
        /* allocate and read peak filters parameters for current EQ */
        (*eqParams)[n].pkFilterParams = (pkFilterParamsStruct*)calloc( (*eqParams)[n].nPkFilter, sizeof( pkFilterParamsStruct ) );
        for(m=0; m<(*eqParams)[n].nPkFilter; m++){
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].f = (float) atof(line);
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].q = (float) atof(line);
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].g = (float) atof(line);
        }
    }

    /* allocate and read eqMap defining mapping of EQs to input channels */
    *eqMap = (int*)calloc(numChEq,sizeof(int));
    for(n=0; n<numChEq; n++){
        if (fgets(line, 511, fileHandle) == NULL ){
            fprintf(stderr, "\nERROR reading EQ parameters input file: incomplete EqMap\n");
            return -1;
        } else {
            (*eqMap)[n] = atoi(line);
        }
    }

    /* print EQ parameters values ... */
    for(n=0; n<*numEQs; n++){
        fprintf(stderr,"\nEQ params %d of %d  : EQ gain    = %f dB\n",n+1,*numEQs,(*eqParams)[n].G);
        for(m=0; m<(*eqParams)[n].nPkFilter; m++){
            fprintf(stderr,"peak filter %d of %d: Pk_freq    = %f Hz\n",m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].f);
            fprintf(stderr,"peak filter %d of %d: Pk_qfactor = %f \n",  m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].q);
            fprintf(stderr,"peak filter %d of %d: Pk_gain    = %f dB\n",m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].g);
        }
    }
    fprintf(stderr,"\ninput channel -> EQ index map:\n");
    for(n=0; n<numChEq; n++)
        fprintf(stderr,"input ch %d: %d\n",n,(*eqMap)[n]);
    fprintf(stderr,"\n");

    fclose(fileHandle);
    return 0;
}

/* end eq_params_parser_code */


static int encoderWrapper(ConfigDownmixMatrixEncoder* config)
{
  int i, j;
  int inputCount;
  SpeakerInformation inputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int outputCount;
  SpeakerInformation outputConfig[DMX_MATRIX_MAX_SPEAKER_COUNT];
  int precisionLevel = 1;

  wobitbuf writebitbuffer;
  wobitbufHandle hBitBuffer = &writebitbuffer;
  unsigned char bitstreamDataWrite[DMX_MATRIX_BITBUFSIZE] = {0};

  FILE* fInput = NULL;
  FILE* fOutput = NULL;
  int inputIndex = -1;
  int outputIndex = -1;
  float flatDownmixMatrix[DMX_MATRIX_MAX_SPEAKER_COUNT * DMX_MATRIX_MAX_SPEAKER_COUNT];
  float* matrix = NULL;
  eqConfigStruct eqConfig;
  int eqPrecisionLevel = 1;
  int error = 0;
  int byteCount = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY inputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_CHANNEL_GEOMETRY outputGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  CICP2GEOMETRY_ERROR geometry_error = CICP2GEOMETRY_OK;
  int numLFEs = 0;


  inputIndex = config->inputConfigIndex;
  outputIndex = config->outputConfigIndex;
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

  /* read the downmix matrix from the input file */
  fInput = fopen(config->inputFilenameMatrix, "rt");
  if (fInput == NULL) {
    fprintf(stderr, "Error opening of matrix input file %s\n", config->inputFilenameMatrix);
    return -1;
  }
  for (i = 0; i < inputCount; ++i) {
    for (j = 0; j < outputCount; ++j) {
      fscanf(fInput, " %f", &(flatDownmixMatrix[i * outputCount + j]));
    }
    if (i != inputCount - 1) fscanf(fInput, "\n");
  }
  fclose(fInput);
  matrix = (float*) flatDownmixMatrix;

  /* if specified, read the equalizer from the input file */
  eqConfig.numEQs = 0;
  eqConfig.eqParams = NULL;
  eqConfig.eqMap = NULL;
  if (config->inputFilenameEqualizer[0] != '\0') {
    int errorFlag = readEqParamsFromFile(config->inputFilenameEqualizer, inputCount,
      &eqConfig.numEQs, &eqConfig.eqParams, &eqConfig.eqMap);
    if (0 != errorFlag) {
      fprintf(stderr, "Error during readEqParamsFromFile for file %s\n",
        config->inputFilenameEqualizer);
      return -1;
    }
  }

  /* initialize bit buffer */
  wobitbuf_Init(hBitBuffer, bitstreamDataWrite, sizeof(bitstreamDataWrite) << 3, 0);

  error = EncodeDownmixMatrix(inputIndex, inputCount, inputConfig, outputIndex, outputCount, outputConfig,
    precisionLevel, hBitBuffer, matrix, eqPrecisionLevel, &eqConfig);
  if (error)
  {
    fprintf(stderr, "Error Encoding Downmix Matrix\n");
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
  ConfigDownmixMatrixEncoder config;
  int ret = 0;

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                      Downmix Matrix Encoder Module                          *\n");
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

  ret = encoderWrapper(&config);

  return ret;
}
