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

Copyright (c) ISO/IEC 2018.

*************************************************************************/


/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/
/* SYSTEM INCLUDES */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/* INCLUDES OF THIS PROJECT */
#include "mhasReader.h"
#include "earconInfo.h"
#include "pcmDataConfig.h"
#include "pcmDataPayload.h"
#include "earconInfoReader.h"
#include "pcmDataConfigReader.h"
#include "pcmDataPayloadReader.h"
#include "wavWriter.h"
#include "bsWriter.h"

/* OTHER INCLUDES */
#include "bitstreamReader.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define MAX_NUM_CHANNELS (16)
#define MP4_FTYPE_BYTE_OFFSET (4)
#define MP4_FTYPE_BYTE_SIZE (4)

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

enum EARCON_DECODER_RETURN {
  EARCON_DECODER_RETURN_NO_ERROR = 0,
  EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS,
  EARCON_DECODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS,
  EARCON_DECODER_RETURN_MEMORY_ALLOCATION_FAILED,
  EARCON_DECODER_RETURN_CANNOT_WRITE_BS_FILE,
  EARCON_DECODER_RETURN_CANNOT_WRITE_WAV_FILE,
  EARCON_DECODER_RETURN_CANNOT_READ_MHAS_FILE,
  EARCON_DECODER_RETURN_CANNOT_PARSE_MHAS,
  EARCON_DECODER_RETURN_CANNOT_PARSE_EARCON_INFO,
  EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_CONFIG,
  EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_PAYLOAD,
  EARCON_DECODER_RETURN_NO_PCM_DATA_CONFIG_PRESENT_BEFORE_PCM_DATA_PAYLOAD,
  EARCON_DECODER_RETURN_MORE_THAN_ONE_PCM_DATA_CONFIG_PRESENT_NOT_SUPPORTED,
  EARCON_DECODER_RETURN_PCM_DATA_CONFIG_VAR_FRAMESIZE_NOT_SUPPORTED,
  EARCON_DECODER_RETURN_FILE_FORMAT_IS_MP4,
  EARCON_DECODER_RETURN_BITSTREAM_READER_ERROR,
  EARCON_DECODER_RETURN_UNKNOWN_ERROR
};
/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct CMDL_PARAMS{
  char const * inputMhasFileName;
  char const * outputWavFileName;
  char const * outputBsFileName;
  int verbose;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum EARCON_DECODER_RETURN retval){
  return (retval != EARCON_DECODER_RETURN_NO_ERROR);
}

static const char * retValToString(enum EARCON_DECODER_RETURN retval){
  switch(retval){
    case EARCON_DECODER_RETURN_NO_ERROR:
      return "No error";
    case EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS:
      return "Invalid command line parameter(s)";
    case EARCON_DECODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS:
      return "Insufficient number of command line parameters is present";
    case EARCON_DECODER_RETURN_MEMORY_ALLOCATION_FAILED:
      return "Memory allocation failed";
    case EARCON_DECODER_RETURN_CANNOT_WRITE_WAV_FILE:
      return "Could not write to wav file";
    case EARCON_DECODER_RETURN_CANNOT_READ_MHAS_FILE:
      return "Could not read from MHAS file";
    case EARCON_DECODER_RETURN_CANNOT_PARSE_MHAS:
      return "Could not parse the MHAS file";
    case EARCON_DECODER_RETURN_CANNOT_PARSE_EARCON_INFO:
      return "Could not parse a earconInfo() payload";
    case EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_CONFIG:
      return "Could not parse a pcmDataConfig() payload";
    case EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_PAYLOAD:
      return "Could not parse a pcmDataPayload() payload";
    case EARCON_DECODER_RETURN_NO_PCM_DATA_CONFIG_PRESENT_BEFORE_PCM_DATA_PAYLOAD:
      return "Decoding a pcmDataPayload() payload required a pcmDataConfig() to be present";
    case EARCON_DECODER_RETURN_MORE_THAN_ONE_PCM_DATA_CONFIG_PRESENT_NOT_SUPPORTED:
      return "More than one pcmDataConfig() payload was found during decoding which is not supported";
    case EARCON_DECODER_RETURN_PCM_DATA_CONFIG_VAR_FRAMESIZE_NOT_SUPPORTED:
      return "A pcmDataPayload() contained the variable framesize mode which is not supported";
    case EARCON_DECODER_RETURN_UNKNOWN_ERROR:
      return "An unknown error occurred";
    default: return "";
  }
}

static void printUsage(void){
  printf("Usage:\n");
  printf("earconDecoder -if <input mhas file> -of <output wav file> -bs <output bs file>\n");
}

static enum EARCON_DECODER_RETURN parseCmdl(
  int                const         argc,
  char             * const * const argv,
  struct CMDL_PARAMS       * const cmdlParams
){
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;
  int used_if = 0;
  int used_of = 0;
  int used_bs = 0;

  /* SANTIY CHECKS */
  if( ! argv || ! cmdlParams ){

    retVal = EARCON_DECODER_RETURN_UNKNOWN_ERROR;
  } else if ( argc < 3 ){

    retVal = EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
  }

  /* PARSE CMDL PARAMS */
  if( ! isError(retVal) ){
    char const * const string_if = "-if";
    char const * const string_of = "-of";
    char const * const string_bs  = "-bs";
    int i;

    cmdlParams->verbose = 1;

    for ( i = 1; i < argc; ++i ) {

      if ( !strncmp(argv[i], string_if, strlen(string_if)) && ( used_if == 0 ) ) {
        if ( ++i < argc ) {
          used_if=1;
          cmdlParams->inputMhasFileName = argv[i];
          continue;
        } else {
          retVal = EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }

      } else if ( !strncmp(argv[i], string_of, strlen(string_of)) && (used_of == 0) ) {

        if ( ++i < argc ) {

          used_of=1;
          cmdlParams->outputWavFileName = argv[i];
          continue;
        } else {
          retVal = EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else if ( !strncmp(argv[i], string_bs, strlen(string_bs)) && (used_bs == 0) ) {

        if ( ++i < argc ) {

          used_bs=1;
          cmdlParams->outputBsFileName = argv[i];
          cmdlParams->verbose = 0;
          continue;
        } else {
          retVal = EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }
      } else {

        retVal = EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
        break;
      }    
    } /* for i */
  }

  /* CHECK IF ALL REQUIRED PARAMS ARE AVAILABLE */
  if( ! isError(retVal) ){
    if( ! used_if || ! used_of ){
      retVal = EARCON_DECODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS;
    }
  }

  return retVal;
}

/*! maps from EARCON_INFO_READER_RETURN to EARCON_DECODER_RETURN */
static enum EARCON_DECODER_RETURN map_PCM_DEARCON_INFO_READER_RETURN( enum EARCON_INFO_READER_RETURN val){
  if( val != EARCON_INFO_READER_RETURN_NO_ERROR ){
    return EARCON_DECODER_RETURN_CANNOT_PARSE_EARCON_INFO;
  }
  return EARCON_DECODER_RETURN_NO_ERROR;
}

/*! maps from PCM_DATA_CONFIG_READER_RETURN to EARCON_DECODER_RETURN */
static enum EARCON_DECODER_RETURN map_PCM_DATA_CONFIG_READER_RETURN( enum PCM_DATA_CONFIG_READER_RETURN val){
  if( val != PCM_DATA_CONFIG_READER_RETURN_NO_ERROR ){
    return EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_CONFIG;
  }
  return EARCON_DECODER_RETURN_NO_ERROR;
}

/*! maps from PCM_DATA_PAYLOAD_READER_RETURN to EARCON_DECODER_RETURN */
static enum EARCON_DECODER_RETURN map_PCM_DATA_PAYLOAD_READER_RETURN( enum PCM_DATA_PAYLOAD_READER_RETURN val){
  if( val != PCM_DATA_PAYLOAD_READER_RETURN_NO_ERROR ){
    return EARCON_DECODER_RETURN_CANNOT_PARSE_PCM_DATA_PAYLOAD;
  }
  return EARCON_DECODER_RETURN_NO_ERROR;
}

/*! handles the decoding of MHAS PACTYP_EARCON */
static enum EARCON_DECODER_RETURN decode_PACTYP_EARCON(struct MHAS_PACKET const * const mhasPacket, struct EARCON_INFO * const earconInfo){
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if( ! earconInfo || ! mhasPacket ){
    retVal = EARCON_DECODER_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PACKET */
  if( ! isError(retVal) ) {
    enum EARCON_INFO_READER_RETURN parserErr = earconInfoReader_ParsePayload( mhasPacket->mhasPacketPayload, mhasPacket->mhasPacketLength, earconInfo);
    retVal = map_PCM_DEARCON_INFO_READER_RETURN(parserErr);
  }

  return retVal;
}

/*! handles the decoding of MHAS PACTYP_PCMCONFIG */
static enum EARCON_DECODER_RETURN decode_PACTYP_PCMCONFIG(struct MHAS_PACKET const * const mhasPacket, struct PCM_DATA_CONFIG * const pcmDataConfig){
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if( ! pcmDataConfig || ! mhasPacket ){
    retVal = EARCON_DECODER_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PACKET */
  if( ! isError(retVal) ) {
    enum PCM_DATA_CONFIG_READER_RETURN parserErr = pcmDataConfigReader_ParsePayload( mhasPacket->mhasPacketPayload, mhasPacket->mhasPacketLength, pcmDataConfig);
    retVal = map_PCM_DATA_CONFIG_READER_RETURN(parserErr);
  }

  return retVal;
}

/*! handles the decoding of MHAS PACTYP_PCMDATA */
static enum EARCON_DECODER_RETURN decode_PACTYP_PCMDATA(struct MHAS_PACKET const * const mhasPacket, struct PCM_DATA_CONFIG const * const pcmDataConfig, struct PCM_DATA_PAYLOAD * const pcmDataPayload){
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;

  /* SANITY CHECK */
  if( ! pcmDataConfig || ! mhasPacket ){
    retVal = EARCON_DECODER_RETURN_UNKNOWN_ERROR;
  }

  /* PARSE PACKET */
  if( ! isError(retVal) ) {
    enum PCM_DATA_PAYLOAD_READER_RETURN parserErr = pcmDataPayloadReader_ParsePayload( mhasPacket->mhasPacketPayload, mhasPacket->mhasPacketLength, pcmDataConfig, pcmDataPayload);
    retVal = map_PCM_DATA_PAYLOAD_READER_RETURN(parserErr);
  }

  return retVal;
}

/*! checks whether the input file format is mp4 */
static enum EARCON_DECODER_RETURN checkWhetherFileFormatIsMp4(char const * const inputFileName) {
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;
  BITSTREAM_READER_HANDLE bitstreamReader = NULL;
  unsigned char mp4_ftype[MP4_FTYPE_BYTE_SIZE] = { 0 };

  /* OPEN FILE */
  bitstreamReader = bitstreamReader_Open(inputFileName);
  if (!bitstreamReader) {
    bitstreamReader_Close(&bitstreamReader);
    retVal = EARCON_DECODER_RETURN_BITSTREAM_READER_ERROR;
  }

  /* READ THE FIRST BYTES TO CHECK IF THE FILE FORMAT IS MP4 */
  if (!isError(retVal)) {
    unsigned int byteCounter;

    for (byteCounter = 0; byteCounter < (MP4_FTYPE_BYTE_SIZE + MP4_FTYPE_BYTE_OFFSET); byteCounter++) {
      unsigned int tmp = 0;
      enum BITSTREAM_READER_RETURN bsReadRet = bitstreamReader_ReadBits(
        bitstreamReader,
        8,
        &tmp
      );

      if (bsReadRet != BITSTREAM_READER_RETURN_NO_ERROR) {
        retVal = EARCON_DECODER_RETURN_BITSTREAM_READER_ERROR;
        break;
      }

      if (byteCounter >= MP4_FTYPE_BYTE_OFFSET) {
        mp4_ftype[byteCounter - MP4_FTYPE_BYTE_OFFSET] = (unsigned char)tmp;
      }
    }
  }

  /* CLOSE FILE */
  bitstreamReader_Close(&bitstreamReader);


  if (!isError(retVal)) {
    if (memcmp(mp4_ftype, "ftyp", MP4_FTYPE_BYTE_SIZE) == 0) {
      retVal = EARCON_DECODER_RETURN_FILE_FORMAT_IS_MP4;
    }
  }

  return retVal;
}


/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
int main(int argc, char* argv[]){
  enum EARCON_DECODER_RETURN retVal = EARCON_DECODER_RETURN_NO_ERROR;

  MHAS_READER_HANDLE inputMhasFile = NULL;
  WAV_WRITER_HANDLE outputWavFile = NULL;
  BS_WRITER_HANDLE outputBsFile = NULL;
  FILE * outputBsFile_fptr = NULL;

  struct PCM_DATA_CONFIG pcmDataConfig;

  struct CMDL_PARAMS cmdlParams;

  struct EARCON_INFO * earconInfo = NULL;
  struct PCM_DATA_PAYLOAD * pcmDataPayload = NULL;

  /* init structs */
  memset(&cmdlParams, 0, sizeof(cmdlParams));
  memset(&pcmDataConfig, 0, sizeof(pcmDataConfig));

  /* PARSE COMMAND LINE PARAMS */
  if( ! isError(retVal) ){
    retVal = parseCmdl(
      argc,
      argv,
      &cmdlParams
    );

    if( retVal == EARCON_DECODER_RETURN_INVALID_COMMAND_LINE_PARAMS ){
      printUsage();
    }
  }

  /* PRINT OUT SOME INFORMATION */
  if( ! isError(retVal) ){
    printf("Input = %s\n", cmdlParams.inputMhasFileName);
    printf("Output = %s\n", cmdlParams.outputWavFileName);
    if (NULL != cmdlParams.outputBsFileName) {
      printf("Payload = %s\n", cmdlParams.outputBsFileName);
    }
    printf("\n");
    printf("Starting decoding!\n");
  }

  /* OPEN MHAS FILE */
  if( ! isError(retVal) ){
    inputMhasFile = mhasReader_Open(cmdlParams.inputMhasFileName);
    if( ! inputMhasFile ){
      retVal = EARCON_DECODER_RETURN_CANNOT_READ_MHAS_FILE;
    }
  }

  /* OPEN RAW BITSTREAM FILE */
  if( ! isError(retVal) ){
    outputBsFile = bsWriter_Open(cmdlParams.outputBsFileName);
    if( ! outputBsFile ){
      retVal = EARCON_DECODER_RETURN_CANNOT_WRITE_BS_FILE;
    }
    outputBsFile_fptr = bsWriter_file(outputBsFile);
  }

  /* ALLOCATE EARCON INFO STRUCT */
  if( ! isError(retVal) ){
    earconInfo = (struct EARCON_INFO*)calloc( 1, sizeof(*earconInfo));
    if( ! earconInfo ){
      retVal = EARCON_DECODER_RETURN_MEMORY_ALLOCATION_FAILED;
    }
  }

  /* ALOCATE PCM DATA PAYLOAD STRUCT */
  if( ! isError(retVal) ){
    pcmDataPayload = (struct PCM_DATA_PAYLOAD*) calloc( 1, sizeof(*pcmDataPayload));
    if( ! pcmDataPayload ){
      retVal = EARCON_DECODER_RETURN_MEMORY_ALLOCATION_FAILED;
    }
  }

  /* CHECK IF THE INPUT FILE FORMAT IS MP4 */
  if ( ! isError(retVal) ) {
    retVal = checkWhetherFileFormatIsMp4(cmdlParams.inputMhasFileName);
  }

  /* LOOP OVER ALL SAMPLES */
  if( ! isError(retVal) ){
    static struct MHAS_PACKET mhasPacket;
    static struct EARCON_INFO earconInfo;
    static struct PCM_DATA_CONFIG pcmDataConfig;
    static struct PCM_DATA_PAYLOAD pcmDataPayload;

    int isValid_pcmDataConfig = 0;
    int isFirstPcmDataPacket = 1;
    enum MHAS_READER_RETURN mhasReaderReturn = MHAS_READER_RETURN_NO_ERROR;

    while( MHAS_READER_RETURN_NO_ERROR == (mhasReaderReturn = readPacket(inputMhasFile, &mhasPacket)) ){
      if (1 == cmdlParams.verbose) {
        printf("Packet Type = %d\n", mhasPacket.mhasPacketType);
        printf("Packet Label = %u\n", mhasPacket.mhasPacketLabel);
        printf("Packet Length = %u\n", mhasPacket.mhasPacketLength);
      }

      switch( mhasPacket.mhasPacketType ){

        /* HANDLE EARCON PACKETS */
        case PACTYP_EARCON:
          retVal = decode_PACTYP_EARCON(&mhasPacket, &earconInfo);
          if( ! isError(retVal) ){
            if (1 == cmdlParams.verbose) {
              fprintf(stdout, "Decoded payload:\n");
            }
            earconInfo_Print(outputBsFile_fptr, &earconInfo, 2);
          }
          break;

        /* HANDLE PCMCONFIG PACKETS */
        case PACTYP_PCMCONFIG:
          if( isValid_pcmDataConfig ){
            retVal = EARCON_DECODER_RETURN_MORE_THAN_ONE_PCM_DATA_CONFIG_PRESENT_NOT_SUPPORTED;
          }
          if( ! isError(retVal) ){
            retVal = decode_PACTYP_PCMCONFIG(&mhasPacket, &pcmDataConfig);
          }
          if( ! isError(retVal) ){
            if( pcmDataConfig.pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE ){
              retVal = EARCON_DECODER_RETURN_PCM_DATA_CONFIG_VAR_FRAMESIZE_NOT_SUPPORTED;
            }
          }
          if( ! isError(retVal) ){
            if (1 == cmdlParams.verbose) {
              fprintf(stdout, "Decoded payload:\n");
            }
            pcmDataConfig_Print(outputBsFile_fptr, &pcmDataConfig, 2);
          }
          if( ! isError(retVal) ){
            isValid_pcmDataConfig = 1;
          }
          break;

        /* HANDLE PCMDATA PACKETS */
        case PACTYP_PCMDATA:
          {
            if( ! isValid_pcmDataConfig ){
              retVal = EARCON_DECODER_RETURN_NO_PCM_DATA_CONFIG_PRESENT_BEFORE_PCM_DATA_PAYLOAD;
            }
            /* decode mhas packet */
            if( ! isError(retVal) ) {
              retVal = decode_PACTYP_PCMDATA(&mhasPacket, &pcmDataConfig, &pcmDataPayload);
            }
            /* open wav file */
            if( ! isError(retVal) ) {
              if( isFirstPcmDataPacket ){
                outputWavFile = wavWriter_Open(
                  cmdlParams.outputWavFileName,
                  pcmDataConfig.numPcmSignals+1,
                  getSamplingRate(&pcmDataConfig),
                  getNumBitsForPcmBitsPerSampleIdx(pcmDataConfig.pcmBitsPerSampleIdx),
                  getPcmDataPayloadFrameSize(&pcmDataPayload, &pcmDataConfig)
                );
                if( ! outputWavFile ){
                  retVal = EARCON_DECODER_RETURN_CANNOT_WRITE_WAV_FILE;
                }
              }
            }
            /* write samples */
            if( ! isError(retVal) ) {
              float *samples[16] = {NULL};
              unsigned int n;
              enum WAV_WRITER_RETURN wavWriteErr = WAV_WRITER_RETURN_NO_ERROR;
              for( n = 0; n < pcmDataConfig.numPcmSignals+1; n++) samples[n] = pcmDataPayload.pcmSample[n];
              wavWriteErr = wavWriter_WriteSamples(outputWavFile, samples);
              if( wavWriteErr != WAV_WRITER_RETURN_NO_ERROR ){
                retVal = EARCON_DECODER_RETURN_CANNOT_WRITE_WAV_FILE;
              }
            }
            if( ! isError(retVal) ){
              if (1 == cmdlParams.verbose) {
                fprintf(stdout, "Decoded payload:\n");
              }
              pcmDataPayload_Print(outputBsFile_fptr, &pcmDataConfig, &pcmDataPayload, 2);
            }
            if( ! isError(retVal) ) {
              isFirstPcmDataPacket = 0;
            }
          }
          break;

        default:
          if (1 == cmdlParams.verbose) {
            printf("--> Packet is ignored\n");
          }
      } /* switch( mhasPacket.mhasPacketType ) */

      if (1 == cmdlParams.verbose) {
        printf("-----------------------\n");
      }
        
      if( isError(retVal) ) break;
    } /* while */

    if( ! isError(retVal) ) {
      if( mhasReaderReturn != MHAS_READER_RETURN_EOF ){
       retVal = EARCON_DECODER_RETURN_CANNOT_PARSE_MHAS;
      }
    }

    if (1 == cmdlParams.verbose) {
      printf("\n");
    }
  }

  /* CLEANUP */
  if( outputWavFile ) wavWriter_Close(&outputWavFile);
  if( inputMhasFile ) mhasReader_Close(&inputMhasFile);
  if( outputBsFile )  bsWriter_Close(&outputBsFile);

  if( earconInfo ) free(earconInfo);
  if( pcmDataPayload ) free(pcmDataPayload);


  /* FINAL MESSAGE */
  if( ! isError(retVal)){
    printf("Done without errors!\n");
  } else if (retVal == EARCON_DECODER_RETURN_FILE_FORMAT_IS_MP4) {
    printf("ATTENTION: File format is mp4. Earcon decoding was skipped!\n");
    printf("Details: %s\n", retValToString(retVal) );
  } else {
    printf("ATTENTION: An error occurred during decoding!\n");
    printf("Error details: %s\n", retValToString(retVal) );
  }
  
  return retVal;
}
