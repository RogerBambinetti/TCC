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

/* INCLUDES OF THIS PROJECT */
#include "mhasWriter.h"
#include "earconInfoWriter.h"
#include "pcmDataConfigWriter.h"
#include "pcmDataPayloadWriter.h"

/* OTHER INCLUDES */
#include "wavIO.h"


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define NUM_SAMPLES_PER_SIGNAL_FRAMESIZE (1024)
#define MAX_NUM_CHANNELS (1)

/* MHASPacketLabels - the same value is used for all earcon-related MHAS packets that belong together */
#define MHAS_PACKET_LABEL_EARCON (2049)
#define MHAS_PACKET_LABEL_PCMCONFIG (2049)
#define MHAS_PACKET_LABEL_PCMDATA (2049)

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

enum EARCON_ENCODER_RETURN {
  EARCON_ENCODER_RETURN_NO_ERROR = 0,
  EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS,
  EARCON_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS,
  EARCON_ENCODER_RETURN_MEMORY_ALLOCATION_FAILED,
  EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE,
  EARCON_ENCODER_RETURN_CANNOT_WRITE_MHAS_FILE,
  EARCON_ENCODER_RETURN_TOO_MANY_CHANNELS_IN_WAV_FILE,
  EARCON_ENCODER_RETURN_UNKNOWN_ERROR
};
/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct CMDL_PARAMS{
  char const * inputWavFileName;
  char const * outputMhasFileName;
  float        pcmLoudnessValue;
};

struct WAV_INFO {
  unsigned int  numChannels;
  unsigned int  sampleRate;
  unsigned int  numBitsPerSample;
  unsigned long numSamplesPerChannelTotal;
  int           numSamplesPerChannelFilled;
};

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum EARCON_ENCODER_RETURN retval){
  return (retval != EARCON_ENCODER_RETURN_NO_ERROR);
}

static const char * retValToString(enum EARCON_ENCODER_RETURN retval){
  switch(retval){
    case EARCON_ENCODER_RETURN_NO_ERROR:
      return "No error";
    case EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS:
      return "Invalid command line parameter(s)";
    case EARCON_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS:
      return "Insufficient number of command line parameters is present";
    case EARCON_ENCODER_RETURN_MEMORY_ALLOCATION_FAILED:
      return "Memory allocation failed";
    case EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE:
      return "Could not read to wav file";
    case EARCON_ENCODER_RETURN_CANNOT_WRITE_MHAS_FILE:
      return "Could not write to MHAS file";
    case EARCON_ENCODER_RETURN_TOO_MANY_CHANNELS_IN_WAV_FILE:
      return "Too many channels in input wav file";
    case EARCON_ENCODER_RETURN_UNKNOWN_ERROR:
      return "Unknown error";
    default:
      return "";
  }
}

static void printUsage(void){
  printf("Usage:\n");
  printf("earconEncoder -if <input wav file> -of <output mhas file> -loudness <PCM loudness value in range [%.2f;%.2f]dB>\n", MIN_VALUE_PCM_LOUDNESS_VALUE, MAX_VALUE_PCM_LOUDNESS_VALUE);
}

static enum EARCON_ENCODER_RETURN parseCmdl(
  int                const         argc,
  char             * const * const argv,
  struct CMDL_PARAMS       * const cmdlParams
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;
  int used_if = 0;
  int used_of = 0;
  int used_loudness = 0;
  
  /* SANTIY CHECKS */
  if( ! argv || ! cmdlParams ){

    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  } else if ( argc < 4 ){

    retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
  }

   /* PARSE CMDL PARAMS */
  if( ! isError(retVal) ){
    char const * const string_if = "-if";
    char const * const string_of = "-of";
    char const * const string_loudness = "-loudness";
    int i;

    for ( i = 1; i < argc; ++i ) {

      if ( !strncmp(argv[i], string_if, strlen(string_if)) && ( used_if == 0 ) ) {
        if ( ++i < argc ) {
          used_if=1;
          cmdlParams->inputWavFileName = argv[i];
          continue;
        } else {
          retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }

      } else if ( !strncmp(argv[i], string_of, strlen(string_of)) && (used_of == 0) ) {

        if ( ++i < argc ) {

          used_of=1;
          cmdlParams->outputMhasFileName = argv[i];
          continue;
        } else {
          retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }

      } else if ( !strncmp(argv[i], string_loudness, strlen(string_loudness)) && (used_loudness == 0) ) {

        if ( ++i < argc ) {

          used_loudness=1;
          cmdlParams->pcmLoudnessValue = (float)atof(argv[i]);
          continue;
        } else {
          retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
          break;
        }

      } else {

        retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
        break;
      }    
    } /* for i */
  }

  /* CHECK IF ALL REQUIRED PARAMS ARE AVAILABLE */
  if( ! isError(retVal) ){
    if( ! used_if || ! used_of || ! used_loudness){
      retVal = EARCON_ENCODER_RETURN_INSUFFICIENT_COMMAND_LINE_PARAMS;
    } else if( cmdlParams->pcmLoudnessValue > MAX_VALUE_PCM_LOUDNESS_VALUE || cmdlParams->pcmLoudnessValue < MIN_VALUE_PCM_LOUDNESS_VALUE ){
      retVal = EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS;
    }
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN fill_EarconInfo(
  struct EARCON_INFO * const earconInfo
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! earconInfo ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  }

  /* FILL EARCON INFO */
  if( ! isError(retVal) ){
    memset(earconInfo, 0, sizeof(*earconInfo));
    earconInfo->numEarcons = 0; 

    earconInfo->earcons[0].isIndependent  = 1;
    earconInfo->earcons[0].hasTextLabel   = 0;
    earconInfo->earcons[0].type           = 5;
    earconInfo->earcons[0].active         = 1;
    earconInfo->earcons[0].positionType   = 0;

    earconInfo->earcons[0].CICPspeakerIdx = 1;

    earconInfo->earcons[0].hasGain        = 0;

    earconInfo->earcons[0].hasTextLabel   = 0;
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN write_MhasPacketEarcon(
  FILE * outputMhasFile,
  struct EARCON_INFO const * const earconInfo
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;


  unsigned char earconInfoPayload[1024] = {0};
  size_t numBytesEarconInfoPayload = 0;

  /* SANITY CHECKS */
  if( ! outputMhasFile || ! earconInfo ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  }



  /* WRITE earconInfo() */
  if( ! isError(retVal) ){
    enum EARCON_INFO_WRITER_RETURN tmpRet = earconInfoWriter_WritePayload(earconInfo, earconInfoPayload, sizeof(earconInfoPayload), &numBytesEarconInfoPayload);
    if( tmpRet != EARCON_INFO_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE MHAS PACKET */
  if( ! isError(retVal) ){
    enum MHAS_WRITER_RETURN tmpRet = writePacket(outputMhasFile, PACTYP_EARCON, MHAS_PACKET_LABEL_EARCON, numBytesEarconInfoPayload, earconInfoPayload);
    if( tmpRet != MHAS_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN fill_pcmDataConfig(
  struct PCM_DATA_CONFIG * const pcmDataConfig,
  unsigned int             const numSignals,
  unsigned int             const sampleRate,
  unsigned int             const frameSize,
  float                    const pcmLoudnessValue
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;
  enum PCM_SAMPLING_RATE_INDEX samplingRateIndex;

  /* SANITY CHECKS */
  if( ! pcmDataConfig ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  } else if ( numSignals < 1 || frameSize != 1024 ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  }

  /* RESET PCM DATA CONFIG */
  if( ! isError(retVal) ){
    memset(pcmDataConfig, 0, sizeof(*pcmDataConfig));
  }

  /* GET SAMPLING RATE INDEX */
  if( ! isError(retVal) ){
    enum PCM_DATA_CONFIG_RETURN tmpErr = getPcmSamplingRateIndex(sampleRate, &samplingRateIndex);
    if( tmpErr != PCM_DATA_CONFIG_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* FILL PCM DATA CONFIG */
  if( ! isError(retVal) ){
    pcmDataConfig->numPcmSignals     = numSignals - 1;
    pcmDataConfig->pcmAlignAudioFlag = 0;
    pcmDataConfig->pcmSamplingRateIdx = samplingRateIndex;
    if( pcmDataConfig->pcmSamplingRateIdx == PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE ){
      pcmDataConfig->pcmSamplingRate = sampleRate;
    }
    pcmDataConfig->pcmBitsPerSampleIdx = PCM_BITS_PER_SAMPLE_IDX_16_BITS;
    pcmDataConfig->pcmFrameSizeIndex = PCM_FRAME_SIZE_INDEX_1024;
    if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE ){
      pcmDataConfig->pcmFixFrameSize = frameSize;
    }

    {
      unsigned int i;
      for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
        pcmDataConfig->pcmSignal_ID[i] = i;
      } /* for i */ 
    }
    pcmDataConfig->bsPcmLoudnessValue = getBsPcmLoudnessValue(pcmLoudnessValue);
    pcmDataConfig->pcmHasAttenuationGain = 0;
    if( pcmDataConfig->pcmHasAttenuationGain ){
      pcmDataConfig->bsPcmAttenuationGain = 0;
    }
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN write_MhasPacketPcmConfig(
  FILE                   *       outputMhasFile,
  struct PCM_DATA_CONFIG * const pcmDataConfig
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;

  unsigned char pcmDataConfigPayload[1024] = {0};
  size_t numBytesPcmDataConfig = 0;

  /* SANITY CHECKS */
  if( ! outputMhasFile || ! pcmDataConfig ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE pcmDataConfig() */
  if( ! isError(retVal) ){
    enum PCM_DATA_CONFIG_WRITER_RETURN tmpRet = pcmDataConfigWriter_WritePayload(pcmDataConfig, pcmDataConfigPayload, sizeof(pcmDataConfigPayload), &numBytesPcmDataConfig);
    if( tmpRet != PCM_DATA_CONFIG_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE MHAS PACKET */
  if( ! isError(retVal) ){
    enum MHAS_WRITER_RETURN tmpRet = writePacket(outputMhasFile, PACTYP_PCMCONFIG, MHAS_PACKET_LABEL_PCMCONFIG, numBytesPcmDataConfig, pcmDataConfigPayload);
    if( tmpRet != MHAS_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN fill_pcmDataPayload(
  struct PCM_DATA_CONFIG          const * const pcmDataConfig,
  struct PCM_DATA_PAYLOAD               * const pcmDataPayload,
  float                   const * const * const sampleBuffer    /**< sample buffer to be index [channelIdx][sampleIdx] */
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! pcmDataConfig || ! pcmDataPayload || ! sampleBuffer ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  } else if( pcmDataConfig->pcmHasAttenuationGain || pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR; /* not implemented */
  }

  /* FILL PCM DATA PAYLOAD STRUCT */
  if( ! isError(retVal) ){
    unsigned int i;
    memset(pcmDataPayload, 0, sizeof(*pcmDataPayload));
    pcmDataPayload->numPcmSignalsInFrame = pcmDataConfig->numPcmSignals; /* always write all PCM signals */

    for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
      pcmDataPayload->pcmSignal_ID[i] = pcmDataConfig->pcmSignal_ID[i];
    } /* for i */

    for( i = 0; i < getPcmDataPayloadFrameSize(pcmDataPayload, pcmDataConfig); i++){
      unsigned int j;
      for( j = 0; j < pcmDataPayload->numPcmSignalsInFrame + 1; j++){
        pcmDataPayload->pcmSample[j][i] = sampleBuffer[j][i];
      } /* for j */
    } /* for i */
  }

  return retVal;
}

static enum EARCON_ENCODER_RETURN write_MhasPacketPcmData(
  FILE                          *       outputMhasFile,
  struct PCM_DATA_CONFIG  const * const pcmDataConfig,
  struct PCM_DATA_PAYLOAD const * const pcmDataPayload
){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;

  unsigned char pcmDataPayloadPayload[16348] = {0};
  size_t numBytesPcmDataPayload = 0;

  /* SANITY CHECKS */
  if( ! outputMhasFile || ! pcmDataConfig || ! pcmDataPayload ){
    retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
  }

  /* WRITE pcmDataConfig() */
  if( ! isError(retVal) ){
    enum PCM_DATA_PAYLOAD_WRITER_RETURN tmpRet = pcmDataPayloadWriter_WritePayload(pcmDataPayload, pcmDataConfig, pcmDataPayloadPayload, sizeof(pcmDataPayloadPayload), &numBytesPcmDataPayload);
    if( tmpRet != PCM_DATA_PAYLOAD_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }
  }

  /* WRITE MHAS PACKET */
  if( ! isError(retVal) ){
    enum MHAS_WRITER_RETURN tmpRet = writePacket(outputMhasFile, PACTYP_PCMDATA, MHAS_PACKET_LABEL_PCMDATA, numBytesPcmDataPayload, pcmDataPayloadPayload);
    if( tmpRet != MHAS_WRITER_RETURN_NO_ERROR ){
      retVal = EARCON_ENCODER_RETURN_UNKNOWN_ERROR;
    }

  }
  return retVal;
}
/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/
int main(int argc, char* argv[]){
  enum EARCON_ENCODER_RETURN retVal = EARCON_ENCODER_RETURN_NO_ERROR;

  
  FILE * outputMhasFile = NULL;

  WAVIO_HANDLE hWav = NULL;
  FILE * inputWavFile = NULL;
  struct WAV_INFO wavInfo;

  float * inputSampleBuffers[MAX_NUM_CHANNELS] = {NULL};

  struct PCM_DATA_CONFIG pcmDataConfig;

  struct CMDL_PARAMS cmdlParams;

  struct EARCON_INFO * earconInfo = NULL;
  struct PCM_DATA_PAYLOAD * pcmDataPayload = NULL;

  /* init structs */
  memset(&cmdlParams, 0, sizeof(cmdlParams));
  memset(&wavInfo, 0, sizeof(wavInfo));
  memset(&pcmDataConfig, 0, sizeof(pcmDataConfig));

  /* PARSE COMMAND LINE PARAMS */
  if( ! isError(retVal) ){
    retVal = parseCmdl(
      argc,
      argv,
      &cmdlParams
    );

    if( retVal == EARCON_ENCODER_RETURN_INVALID_COMMAND_LINE_PARAMS ){
      printUsage();
    }
  }

  /* SOME PRINTOUTS */
  if( ! isError(retVal) ){
    printf("Input = %s\n", cmdlParams.inputWavFileName);
    printf("PCM loudness value = %f\n", cmdlParams.pcmLoudnessValue);
    printf("Output = %s\n", cmdlParams.outputMhasFileName);
    printf("\n");
    printf("Starting encoding!\n");
  }

  /* OPEN WAV FILE */
  if( ! isError(retVal) ){

    /* open file handle */
    if( ! isError(retVal) ){
      inputWavFile = fopen( cmdlParams.inputWavFileName, "rb");
      if( ! inputWavFile ){
        retVal = EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE;
      }
    }

    /* init wav reader handle */
    if( ! isError(retVal) ){
      int ret = wavIO_init(&hWav, NUM_SAMPLES_PER_SIGNAL_FRAMESIZE, 0, 0);
      if( ret != 0 || ! hWav ){
        retVal = EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE;
      }
    }
    
    /* open wav reader handle */
    if( ! isError(retVal) ) {
     int ret = wavIO_openRead(hWav, inputWavFile, &wavInfo.numChannels, &wavInfo.sampleRate, &wavInfo.numBitsPerSample, &wavInfo.numSamplesPerChannelTotal, &wavInfo.numSamplesPerChannelFilled);
      if( ret != 0 ){
        retVal = EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE;
      } else if( wavInfo.numChannels > MAX_NUM_CHANNELS ){
        retVal = EARCON_ENCODER_RETURN_TOO_MANY_CHANNELS_IN_WAV_FILE;
      }
    }

    /* allocate input buffer */
    if( ! isError(retVal) ) {
      unsigned int ch;

      for( ch = 0; ch < wavInfo.numChannels; ch++ ){
        inputSampleBuffers[ch] = (float*) calloc( NUM_SAMPLES_PER_SIGNAL_FRAMESIZE, sizeof(*inputSampleBuffers[ch]));
        if( ! inputSampleBuffers[ch] ){
          retVal = EARCON_ENCODER_RETURN_MEMORY_ALLOCATION_FAILED;
          break;
        }
      } /* for ch */
    }

  }

  /* OPEN MHAS FILE */
  if( ! isError(retVal) ){
    outputMhasFile = fopen(cmdlParams.outputMhasFileName, "wb");
    if( ! outputMhasFile ){
      retVal = EARCON_ENCODER_RETURN_CANNOT_WRITE_MHAS_FILE;
    }
  }

  /* ALLOCATE EARCON INFO STRUCT */
  if( ! isError(retVal) ){
    earconInfo = (struct EARCON_INFO*)calloc( 1, sizeof(*earconInfo));
    if( ! earconInfo ){
      retVal = EARCON_ENCODER_RETURN_MEMORY_ALLOCATION_FAILED;
    }
  }

  /* FILL EARCON INFO */
  if( ! isError(retVal) ){
    retVal = fill_EarconInfo(earconInfo);
  }

  /* WRITE PACTYP_EARCON */
  if( ! isError(retVal) ){
    retVal = write_MhasPacketEarcon(outputMhasFile, earconInfo);
  }

  /* FILL PCM DATA CONFIG */
  if( ! isError(retVal) ){
    retVal = fill_pcmDataConfig(&pcmDataConfig,wavInfo.numChannels, wavInfo.sampleRate, NUM_SAMPLES_PER_SIGNAL_FRAMESIZE, cmdlParams.pcmLoudnessValue);
  }

  /* WRITE PACTYP_PCMCONFIG */
  if( ! isError(retVal) ){
    retVal = write_MhasPacketPcmConfig(outputMhasFile, &pcmDataConfig);
  }

  /* ALOCATE PCM DATA PAYLOAD STRUCT */
  if( ! isError(retVal) ){
    pcmDataPayload = (struct PCM_DATA_PAYLOAD*) calloc( 1, sizeof(*pcmDataPayload));
    if( ! pcmDataPayload ){
      retVal = EARCON_ENCODER_RETURN_MEMORY_ALLOCATION_FAILED;
    }
  }

  /* LOOP OVER ALL SAMPLES */
  if( ! isError(retVal) ){
    unsigned int isLastFrame = 0;

    unsigned long numSamplesPerChannelReadTotal = 0;

    printf("Progress: %.2f%%", 100.0f * (float)numSamplesPerChannelReadTotal / (float) wavInfo.numSamplesPerChannelTotal);

    while( ! isLastFrame ){
      
      unsigned int numSamplesPerChannelRead = 0;
      unsigned int nZerosPaddedBeginning = 0;
      unsigned int nZerosPaddedEnd = 0;

      /* reset inputSampleBuffers */
      if( ! isError(retVal) ){
        unsigned int ch;
        for( ch = 0; ch < wavInfo.numChannels; ch++ ){
          memset(inputSampleBuffers[ch], 0, NUM_SAMPLES_PER_SIGNAL_FRAMESIZE * sizeof(*inputSampleBuffers[ch]) );
        } /* for ch */
      }

      /* get samples from wav */
      if( ! isError(retVal) ){
        int ret = wavIO_readFrame(hWav, inputSampleBuffers, &numSamplesPerChannelRead, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
        if( ret != 0 ){
          retVal = EARCON_ENCODER_RETURN_CANNOT_READ_WAV_FILE;
        } else {
          numSamplesPerChannelReadTotal += numSamplesPerChannelRead;
        }
      }

      /* fill pcm data payload struct */
      if( ! isError(retVal) ){
        retVal = fill_pcmDataPayload(&pcmDataConfig, pcmDataPayload, inputSampleBuffers);
      }

      /* write pcm data payload mhas packet */
      if( ! isError(retVal) ){
        retVal = write_MhasPacketPcmData(outputMhasFile, &pcmDataConfig, pcmDataPayload);
      }

      printf("\rProgress: %.2f%%", 100.0f * (float)numSamplesPerChannelReadTotal / (float) wavInfo.numSamplesPerChannelTotal);

      if( isError(retVal) ) break;
    } /* while( ! isLastFrame ) */

    printf("\n");
  }




  /* CLEANUP */
  if( hWav ) wavIO_close(hWav);
  /*if( inputWavFile ) fclose(inputWavFile); */ /* inputWavFile is closed by wavIO_close */
  if( outputMhasFile ) fclose(outputMhasFile);
  {
    unsigned int ch;
    for( ch = 0; ch < MAX_NUM_CHANNELS; ch++ ){
      if( inputSampleBuffers[ch] ) free(inputSampleBuffers[ch]);
    } /* for ch */
  }
  if( earconInfo ) free(earconInfo);
  if( pcmDataPayload ) free(pcmDataPayload);
  
  /* FINAL MESSAGE */
  if( ! isError(retVal)){
    printf("Done without errors!\n");
  } else {
    printf("ATTENTION: An error occurred during encoding!\n");
    printf("Error details: %s\n", retValToString(retVal) );
  }

  return retVal;
}


