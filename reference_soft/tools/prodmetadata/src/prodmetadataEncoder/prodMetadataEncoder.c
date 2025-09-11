/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2019.

*************************************************************************/

/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/
/* SYSTEM INCLUDES */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/* INCLUDES OF THIS PROJECT */
#include "encPmcWriter.h"
#include "encPmfWriter.h"
#include "encPmfReader.h"
#include "omfReader.h"
#include "prodMetadataConfigWriter.h"
#include "prodMetadataFrameWriter.h"

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y)) 
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/
enum PRODMETADATA_ENCODER_RETURN {
  PRODMETADATA_ENCODER_RETURN_NO_ERROR = 0,
  PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS,
  PRODMETADATA_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS,
  PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMC_FILE,
  PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_FILE,
  PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_CONFIG,
  PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMC_CONFIG,
  PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMF_FILE,
  PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMF_FRAME,
  PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FILE,
  PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FRAME,
  PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMF_FRAME,
  PRODMETADATA_ENCODER_RETURN_CANNOT_READ_OMF_FRAME,
  PRODMETADATA_ENCODER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
struct CMDL_PARAMS {
  char const * inputPmcFileName;
  char const * inputObjFileName;
  char const * outputPmcFileName;
  unsigned int numChannelGroups;
  unsigned int numObjectGroups;
  unsigned int num_objects[MAX_AUDIO_GROUPS];
  unsigned int numFramesDelay;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/
static int isError(enum PRODMETADATA_ENCODER_RETURN retval) {
  return (retval != PRODMETADATA_ENCODER_RETURN_NO_ERROR);
}

static const char * retValToString(enum PRODMETADATA_ENCODER_RETURN retval) {
  switch (retval) {
    case PRODMETADATA_ENCODER_RETURN_NO_ERROR:
      return "No error";
    case PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS:
      return "Invalid command line parameter(s)";
    case PRODMETADATA_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS:
      return "Insufficient number of command line parameters is present";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_FILE:
      return "Could not write to PMC file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_CONFIG:
      return "Could not write prodMetadataConfig() to file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMC_CONFIG:
      return "Could not encode prodMetadataConfig()";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMF_FILE:
      return "Could not read to PMF file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMF_FRAME:
      return "Could not read prodMetadataFrame() from file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FILE:
      return "Could not write to PMF file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FRAME:
      return "Could not write prodMetadataFrame() to file";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMF_FRAME:
       return "Could not encode prodMetadataFrame()";
    case PRODMETADATA_ENCODER_RETURN_CANNOT_READ_OMF_FRAME:
      return "Could not read has_object_metadata from file";
    case PRODMETADATA_ENCODER_RETURN_UNKNOWN_ERROR:
      return "An unknown error occurred";
    default:
      return "";
  }
}

static void printUsage(void) {
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "prodmetadataEncoder -ipmc <PMCFILE> -iomf <OBJECTFILE> -opmc <PMCFILE> -ccfg <UINT> -ocfg <LIST> -ndly <UINT>\n\n");
  fprintf(stdout, "OPTIONS:\n\t-ipmc\tinput PMCFILE\n\t-iomf\tinput OBJECTFILE\n\t-opmc\toutput PMCFILE\n\t-ccfg\tchannel group config\n\t-ocfg\tobject group config\n\t-ndly\tnumber of frames to delay\n");
  fprintf(stdout, "PMCFILE:\n\tProduction Metadata Config file.\n\tProduction Metadata Frame file(s) will be derived on PMCFILE\n\tby removing the extension and adding \"_<frameNo>.dat\", e.g.:\n\t-ipmc pmc.txt -> pmc_1.dat, pmc_2.dat ...\n");
  fprintf(stdout, "OBJECTFILE:\n\tobject metadata file containing the flag\n\thas_object_metadata for each object of each group.\n");
  fprintf(stdout, "UINT:\n\tunsigned integer, max: %d\n", MAX_AUDIO_GROUPS);
  fprintf(stdout, "LIST:\n\tlist of unsigned integer values:\n\t");
  fprintf(stdout, "numObjectGroups:\n\t\tnumber of object goups, max: %d\n\tnum_objects:\n\t\tnumber of objects per ObjectGroup, max: %d\n\t", MAX_AUDIO_GROUPS, MAX_NUM_OBJECTS);
  fprintf(stdout, "e.g.: 1 2\n\t\tnumObjectGroups  = 1\n\t\tgrp1.num_objects = 2\n\t");
  fprintf(stdout, "e.g.: 2 2 1\n\t\tnumObjectGroups  = 2\n\t\tgrp1.num_objects = 2\n\t\tgrp2.num_objects = 1\n\n");
}

static enum PRODMETADATA_ENCODER_RETURN parseCmdl(
  int                  const         argc,
  char               * const * const argv,
  struct CMDL_PARAMS         * const cmdlParams
) {
  enum PRODMETADATA_ENCODER_RETURN retVal = PRODMETADATA_ENCODER_RETURN_NO_ERROR;
  int used_ipmc = 0;
  int used_iomf = 0;
  int used_opmc = 0;
  int used_ccfg = 0;
  int used_ocfg = 0;
  int used_ndly = 0;

  /* SANTIY CHECKS */
  if (!argv || !cmdlParams) {
    retVal = PRODMETADATA_ENCODER_RETURN_UNKNOWN_ERROR;
  } else if (argc < 5) {
    retVal = PRODMETADATA_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS;
  }

  /* PARSE CMDL PARAMS */
  if (!isError(retVal)) {
    char const * const string_ipmc = "-ipmc";
    char const * const string_iomf = "-iomf";
    char const * const string_opmc = "-opmc";
    char const * const string_ccfg = "-ccfg";
    char const * const string_ocfg = "-ocfg";
    char const * const string_ndly = "-ndly";
    int i;

    for (i = 1; i < argc; ++i) {
      if (!strncmp(argv[i], string_ipmc, strlen(string_ipmc)) && (used_ipmc == 0)) {
        if (++i < argc) {
          used_ipmc = 1;
          cmdlParams->inputPmcFileName = argv[i];
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if (!strncmp(argv[i], string_opmc, strlen(string_opmc)) && (used_opmc == 0)) {
        if (++i < argc) {
          used_opmc = 1;
          cmdlParams->outputPmcFileName = argv[i];
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if (!strncmp(argv[i], string_iomf, strlen(string_iomf)) && (used_iomf == 0)) {
        if (++i < argc) {
          used_iomf = 1;
          cmdlParams->inputObjFileName = argv[i];
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if (!strncmp(argv[i], string_ccfg, strlen(string_ccfg)) && (used_ccfg == 0)) {
        if (++i < argc) {
          used_ccfg = 1;
          cmdlParams->numChannelGroups = (unsigned int)atoi(argv[i]);
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if (!strncmp(argv[i], string_ocfg, strlen(string_ocfg)) && (used_ocfg == 0)) {
        if (++i < argc) {
          unsigned int grp = 0;

          used_ocfg = 1;
          cmdlParams->numObjectGroups = (unsigned int)atoi(argv[i]);
          if (cmdlParams->numObjectGroups < 0 || cmdlParams->numObjectGroups > MAX_AUDIO_GROUPS) {
            retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
            break;
          }

          for (grp = 0; grp < cmdlParams->numObjectGroups; grp++) {
            if (++i < argc) {
              cmdlParams->num_objects[grp] = (unsigned int)atoi(argv[i]);
              if (cmdlParams->num_objects[grp] < 0 || cmdlParams->num_objects[grp] > MAX_NUM_OBJECTS) {
                retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
                break;
              }
            } else {
              retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
              break;
            }
          }
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if (!strncmp(argv[i], string_ndly, strlen(string_ndly)) && (used_ndly == 0)) {
        if (++i < argc) {
          used_ndly = 1;
          cmdlParams->numFramesDelay = (unsigned int)atoi(argv[i]);
          continue;
        } else {
          retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else {
        retVal = PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
        break;
      }
    }
  }

  /* CHECK IF ALL REQUIRED PARAMS ARE AVAILABLE */
  if (!isError(retVal)) {
    if (!used_ipmc || !used_opmc || !used_ccfg || !used_ocfg /* || !used_iomf */) {
      retVal = PRODMETADATA_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS;
    }
  }

  return retVal;
}

/*! derive the PMF base file name from the PMC file name */
static enum PRODMETADATA_ENCODER_RETURN derivePmfBaseFile(
  char const * const inputPmcFileName,
  char       * const inputPmfBaseFileName
) {
  enum PRODMETADATA_ENCODER_RETURN retVal = PRODMETADATA_ENCODER_RETURN_NO_ERROR;
  char *pFilename = NULL;
  size_t strLength = strlen(inputPmcFileName);

  /* SANITY CHECKS */
  if ((strLength < 0) || (strLength > FILENAME_MAX) || !inputPmcFileName || !inputPmfBaseFileName) {
    retVal = PRODMETADATA_ENCODER_RETURN_UNKNOWN_ERROR;
  }

  /* GET PMF BASE FILE NAME */
  if (!isError(retVal)) {
    strncpy(inputPmfBaseFileName, inputPmcFileName, strLength);
    pFilename = &(inputPmfBaseFileName[strLength]);

    while ((*pFilename != '.') && (strLength > 0)) {
      pFilename--;
      strLength--;
    }

    *pFilename = '\0';
  }

  return retVal;
}

/*! encode reference_distance */
static unsigned int encodeReferenceDistance(
  float const distance
) {
  unsigned int bs_distance = 0;
  float f_distance;

  if (distance >= 0.5f) {
    f_distance = (float)log10(distance * 100.0f) / 0.014214299201363f - 119.0f; /* 0.014214299201363 = 0.0472188798661443*log10(2); */
    bs_distance = (unsigned int)(f_distance + 0.5);
  }

  return bs_distance;
}

/*! fill PMC struct */
static enum PRODMETADATA_ENCODER_RETURN fillPmcStruct(
  struct PRODMETADATACONFIG  * const prodMetadataConfig,
  char                 const * const inputPmcFileName,
  unsigned int                 const numObjectGroups,
  unsigned int                 const numChannelGroups
) {
  enum PRODMETADATA_ENCODER_RETURN retVal = PRODMETADATA_ENCODER_RETURN_NO_ERROR;
  FILE* fptr = NULL;
  int elementsRead = 0;
  int elementsToRead = 0;

  /* SANITY CHECKS */
  if (!prodMetadataConfig || !inputPmcFileName || numObjectGroups > MAX_AUDIO_GROUPS || numChannelGroups > MAX_AUDIO_GROUPS) {
    retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMC_FILE;
  }

  /* OPEN INPUT FILE */
  if (!isError(retVal)) {
    fptr = fopen(inputPmcFileName, "rb");
    if (NULL == fptr) {
      retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMC_FILE;
    }
  }

  /* READ DATA FROM FILE */
  if (!isError(retVal)) {
    unsigned int i;
    unsigned int bs_val;

    if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
    prodMetadataConfig->has_reference_distance = MIN(bs_val, 1);
    elementsToRead++;

    if (prodMetadataConfig->has_reference_distance) {
      float referenceDistance = 0.0f;

      if (!feof(fptr)) elementsRead += fscanf(fptr, "%f", &referenceDistance);
      elementsToRead++;

      prodMetadataConfig->bs_reference_distance = encodeReferenceDistance(referenceDistance);
    } else {
      prodMetadataConfig->bs_reference_distance = 57;
    }

    for (i = 0; i < numObjectGroups; i++) {
      if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
      prodMetadataConfig->has_object_distance[i] = MIN(bs_val, 1);
      elementsToRead++;
    }

    for (i = 0; i < numChannelGroups; i++) {
      if (!feof(fptr)) elementsRead += fscanf(fptr, "%d", &bs_val);
      prodMetadataConfig->directHeadphone[i] = MIN(bs_val, 1);
      elementsToRead++;
    }
  }

  if (!isError(retVal)) {
    if (!feof(fptr) || (elementsToRead != elementsRead)) {
      retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMC_FILE;
    }
  }

  /* CLOSE FILE */
  if (fptr) {
    fclose(fptr);
  }

  return retVal;
}

/*! map object data from omf file to OBJDATA */
static enum PRODMETADATA_ENCODER_RETURN map_objectData(
  struct OMF_PACKET  const * const omfPacket,
  unsigned int       const * const has_object_distance,
  struct OBJDATA           * const objData
) {
  enum PRODMETADATA_ENCODER_RETURN retVal = PRODMETADATA_ENCODER_RETURN_NO_ERROR;
  unsigned int grp;

  memset(objData, 0, sizeof(struct OBJDATA));

  objData->numObjectGroups = omfPacket->numObjectGroups;
  for (grp = 0; grp < omfPacket->numObjectGroups; grp++) {
    unsigned obj;

    objData->has_object_distance[grp] = has_object_distance[grp];
    objData->objectGroup[grp].num_objects = omfPacket->objectGroup[grp].num_objects;
    for (obj = 0; obj < omfPacket->objectGroup[grp].num_objects; obj++) {
      objData->objectGroup[grp].has_object_metadata[obj] = omfPacket->objectGroup[grp].has_object_metadata[obj];
    }
  }

  return retVal;
}

/*! maps from PMC_WRITER_RETURN to PRODMETADATA_ENCODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_PMC_WRITER_RETURN(enum PMC_WRITER_RETURN val) {
  if (val != PMC_WRITER_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_CONFIG;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/*! maps from PROD_METADATA_CONFIG_RETURN to PRODMETADATA_ENCODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_PROD_METADATA_CONFIG_RETURN(enum PROD_METADATA_CONFIG_RETURN val) {
  if (val != PROD_METADATA_CONFIG_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMC_CONFIG;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/*! maps from PMF_WRITER_RETURN to PRODMETADATA_ENCODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_PMF_WRITER_RETURN(enum PMF_WRITER_RETURN val) {
  if (val != PMF_WRITER_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FRAME;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/*! maps from PMF_READER_RETURN to PRODMETADATA_ENCODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_PMF_READER_RETURN(enum PMF_READER_RETURN val) {
  if (val != PMF_READER_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_READ_PMF_FRAME;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/*! maps from PROD_METADATA_FRAME_RETURN to PRODMETADATA_ENCODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_PROD_METADATA_FRAME_RETURN(enum PROD_METADATA_FRAME_RETURN val) {
  if (val != PROD_METADATA_FRAME_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_ENCODE_PMF_FRAME;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/*! maps from OMF_READER_RETURN to PRODMETADATA_DECODER_RETURN */
static enum PRODMETADATA_ENCODER_RETURN map_OMF_READER_RETURN(enum OMF_READER_RETURN val) {
  if (val != OMF_READER_RETURN_NO_ERROR ) {
    return PRODMETADATA_ENCODER_RETURN_CANNOT_READ_OMF_FRAME;
  }
  return PRODMETADATA_ENCODER_RETURN_NO_ERROR;
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
int main(int argc, char* argv[]) {
  enum PRODMETADATA_ENCODER_RETURN retVal = PRODMETADATA_ENCODER_RETURN_NO_ERROR;
  PMC_WRITER_HANDLE outputPmcFile = NULL;
  PMF_WRITER_HANDLE outputPmfFile = NULL;
  PMF_READER_HANDLE inputPmfFile = NULL;
  OMF_READER_HANDLE inputOmfFile = NULL;
  struct CMDL_PARAMS cmdlParams;
  struct PRODMETADATACONFIG prodMetadataConfig;
  struct PRODMETADATAFRAME prodMetadataFrame;
  unsigned int frameCnt = 0;
  unsigned int delay = 0;
  char inputPmfFileName[FILENAME_MAX] = {'\0'};
  char outputPmfFileName[FILENAME_MAX] = {'\0'};

  /* init structs */
  memset(&cmdlParams, 0, sizeof(cmdlParams));

  /* PARSE COMMAND LINE PARAMS */
  if (!isError(retVal)) {
    retVal = parseCmdl(argc,
                       argv,
                       &cmdlParams);
    if ((retVal == PRODMETADATA_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS) || (retVal == PRODMETADATA_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS)) {
      printUsage();
    }

    delay = cmdlParams.numFramesDelay;
  }

  /* DERIVE PMF BASE FILE NAME */
  if (!isError(retVal)) {
    retVal = derivePmfBaseFile(cmdlParams.inputPmcFileName, inputPmfFileName);
  }
  if (!isError(retVal)) {
    retVal = derivePmfBaseFile(cmdlParams.outputPmcFileName, outputPmfFileName);
  }

  /* PRINT OUT SOME INFORMATION */
  if (!isError(retVal)) {
    fprintf(stdout, "PMC input = %s\n", cmdlParams.inputPmcFileName);
    fprintf(stdout, "PMF input = %s\n", inputPmfFileName);
    fprintf(stdout, "OMF input = %s\n", cmdlParams.inputObjFileName);
    fprintf(stdout, "PMC output = %s\n", cmdlParams.outputPmcFileName);
    fprintf(stdout, "PMF output = %s\n", outputPmfFileName);
    fprintf(stdout, "\n");
    fprintf(stdout, "Starting encoding!\n\n");
  }

  /* OPEN PMC FILE */
  if (!isError(retVal)) {
    outputPmcFile = pmcWriter_Open(cmdlParams.outputPmcFileName);
    if (!outputPmcFile) {
      retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMC_FILE;
    }
  }

  /* OPEN OMF FILE */
  if (!isError(retVal)) {
    inputOmfFile = omfReader_Open(cmdlParams.inputObjFileName);
    if (!inputOmfFile) {
      /*retVal = PRODMETADATA_DECODER_RETURN_CANNOT_READ_OMF_FILE;*/
    }
  }

  /* FILL PMC STRUCT */
  if (!isError(retVal)) {
    memset(&prodMetadataConfig, 0, sizeof(prodMetadataConfig));
    retVal = fillPmcStruct(&prodMetadataConfig,
                           cmdlParams.inputPmcFileName,
                           cmdlParams.numObjectGroups,
                           cmdlParams.numChannelGroups);
  }

  /* WRITE prodMetadataConfig */
  if (!isError(retVal)) { 
    enum PMC_WRITER_RETURN pmcWriterReturn = PMC_WRITER_RETURN_NO_ERROR;
    enum PROD_METADATA_CONFIG_RETURN prodMetadataConfigReturn = PROD_METADATA_CONFIG_RETURN_NO_ERROR;
    struct PMC_PACKET pmcPacket;
    unsigned char prodMetadataConfigPayload[1024] = {0};
    size_t numBytesPmdConfig = 0;

    memset(&pmcPacket, 0, sizeof(pmcPacket));

    prodMetadataConfigReturn = prodMetadataConfig_WritePayload(&prodMetadataConfig,
                                                               prodMetadataConfigPayload,
                                                               sizeof(prodMetadataConfigPayload),
                                                               &numBytesPmdConfig,
                                                               cmdlParams.numObjectGroups,
                                                               cmdlParams.numChannelGroups);

    retVal = map_PROD_METADATA_CONFIG_RETURN(prodMetadataConfigReturn);

    if (!isError(retVal)) {
      pmcWriterReturn = pmcWriter_PackConfig(prodMetadataConfigPayload, numBytesPmdConfig, &pmcPacket);
      retVal = map_PMC_WRITER_RETURN(pmcWriterReturn);
    }

    if (!isError(retVal)) {
      pmcWriterReturn = pmcWriter_writeConfig(outputPmcFile, &pmcPacket);
      retVal = map_PMC_WRITER_RETURN(pmcWriterReturn);
    }

    if (!isError(retVal)) {
      fprintf(stdout, "Encoded prodMetadataConfig() payload:\n");
      prodMetadataConfigReturn = prodMetadataConfig_Print(stdout,
                                                          &prodMetadataConfig,
                                                          cmdlParams.numObjectGroups,
                                                          cmdlParams.numChannelGroups,
                                                          2);
    }
  }

  /* LOOP OVER ALL DELAY FRAMES */
  if (!isError(retVal)) {
    unsigned int frameCntDelay = 0;

    /* create just empty files for the encoder delay line */
    for (frameCntDelay = 1; frameCntDelay <= delay; frameCntDelay++){
      outputPmfFile = pmfWriter_Open(outputPmfFileName, frameCntDelay);
      if (!outputPmfFile) {
        retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FILE;
      }

      if (outputPmfFile) {
        pmfWriter_Close(&outputPmfFile);
      }
    }
  }

  /* LOOP OVER ALL EXISTING FRAMES */
  if (!isError(retVal)) {
    memset(&prodMetadataFrame, 0, sizeof(prodMetadataFrame));

    while (NULL != (inputPmfFile = pmfReader_Open(inputPmfFileName, ++frameCnt))) {
      struct OBJDATA objData;
      struct OMF_PACKET omfPacket;
      enum PMF_WRITER_RETURN pmfWriterReturn = PMF_WRITER_RETURN_NO_ERROR;
      enum PMF_READER_RETURN pmfReaderReturn = PMF_READER_RETURN_NO_ERROR;
      enum OMF_READER_RETURN omfReaderReturn = OMF_READER_RETURN_NO_ERROR;
      enum PROD_METADATA_FRAME_RETURN prodMetadataFrameReturn = PROD_METADATA_FRAME_RETURN_NO_ERROR;

      if (inputOmfFile) {
        omfReaderReturn = omfReader_readFrame(inputOmfFile, &omfPacket, cmdlParams.numObjectGroups, cmdlParams.num_objects);
        retVal = map_OMF_READER_RETURN(omfReaderReturn);
      } else {
        memset(&omfPacket, 0, sizeof(omfPacket));
      }

      if (!isError(retVal)) {
        retVal = map_objectData(&omfPacket, prodMetadataConfig.has_object_distance, &objData);
      }
      
      if (!isError(retVal)) {
        pmfReaderReturn = pmfReader_readFrame(inputPmfFile,
                                              &prodMetadataFrame,
                                              &prodMetadataConfig,
                                              &objData);
        retVal = map_PMF_READER_RETURN(pmfReaderReturn);
      }

      if (!isError(retVal)) {
        outputPmfFile = pmfWriter_Open(outputPmfFileName, frameCnt + delay);
        if (!outputPmfFile) {
          retVal = PRODMETADATA_ENCODER_RETURN_CANNOT_WRITE_PMF_FILE;
        }
      }

      if (!isError(retVal)) {
        struct PMF_PACKET pmfPacket;
        unsigned char prodMetadataFramePayload[4096] = {0};
        size_t numBytesPmdFrame = 0;

        memset(&pmfPacket, 0, sizeof(pmfPacket));

        prodMetadataFrameReturn = prodMetadataFrame_WritePayload(&prodMetadataFrame,
                                                                 prodMetadataFramePayload,
                                                                 sizeof(prodMetadataFramePayload),
                                                                 &numBytesPmdFrame,
                                                                 cmdlParams.numObjectGroups,
                                                                 cmdlParams.numChannelGroups);
        retVal = map_PROD_METADATA_FRAME_RETURN(prodMetadataFrameReturn);

        if (!isError(retVal)) {
          pmfWriterReturn = pmfWriter_PackConfig(prodMetadataFramePayload, numBytesPmdFrame, &pmfPacket);
          retVal = map_PMF_WRITER_RETURN(pmfWriterReturn);
        }

        if (!isError(retVal)) {
          pmfWriterReturn = pmfWriter_writeFrame(outputPmfFile, &pmfPacket);
          retVal = map_PMF_WRITER_RETURN(pmfWriterReturn);
        }

        if (!isError(retVal)) {
          fprintf(stdout, "\nEncoded prodMetadataFrame() payload (frame %d):\n", frameCnt);
          prodMetadataFrameReturn = prodMetadataFrame_Print(stdout,
                                                            &prodMetadataFrame,
                                                            cmdlParams.numObjectGroups,
                                                            2);
        }

        /* CLEANUP */
        if (inputPmfFile) {
          pmfReader_Close(&inputPmfFile);
        }
        if (outputPmfFile) {
          pmfWriter_Close(&outputPmfFile);
        }
      }

      if (isError(retVal)) {
        break;
      }
    }
  }

  /* CLEANUP */
  if (outputPmcFile) {
    pmcWriter_Close(&outputPmcFile);
  }
  if (inputOmfFile) {
    omfReader_Close(&inputOmfFile);
  }

  /* FINAL MESSAGE */
  if (!isError(retVal)) {
    fprintf(stdout, "\nDone without errors!\n");
  } else {
    fprintf(stderr, "\nATTENTION: An error occurred during encoding!\n");
    fprintf(stderr, "Error details: %s\n", retValToString(retVal) );
  }

  return retVal;
}
