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
#include <limits.h>
#include <assert.h>


/* INCLUDES OF THIS PROJECT */
#include "pcmDataConfig.h"
#include "printHelper.h"

/* OTHER INCLUDES */



/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct PCM_SAMPLING_RATE_INDEX_INFO {
  unsigned int samplingRate;
  enum PCM_SAMPLING_RATE_INDEX pcmSamplingRateIndex;
};

/* ######################################################################*/
/* ############################## const data ############################*/
/* ######################################################################*/

const static struct PCM_SAMPLING_RATE_INDEX_INFO pcmSamplingRateIndexInfos[] = {
  {96000, PCM_SAMPLING_RATE_INDEX_96000},
  {88200, PCM_SAMPLING_RATE_INDEX_88200},
  {64000, PCM_SAMPLING_RATE_INDEX_64000},
  {58800, PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE},
  {48000, PCM_SAMPLING_RATE_INDEX_48000},
  {44100, PCM_SAMPLING_RATE_INDEX_44100},
  {32000, PCM_SAMPLING_RATE_INDEX_32000},
  {29400, PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE},
  {24000, PCM_SAMPLING_RATE_INDEX_24000},
  {22050, PCM_SAMPLING_RATE_INDEX_22050},
  {16000, PCM_SAMPLING_RATE_INDEX_16000},
  {14700, PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE}
};
#define NUM_PCM_SAMPLE_RATE_INDEX_INFOS ( sizeof(pcmSamplingRateIndexInfos) / sizeof(pcmSamplingRateIndexInfos[0]) )

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

static int isError(enum PCM_DATA_CONFIG_RETURN retval){
  return (retval != PCM_DATA_CONFIG_RETURN_NO_ERROR);
}

static void pcmDataConfig_Print_binary(
  FILE                          *       file,
  struct PCM_DATA_CONFIG  const * const pcmDataConfig
){
  unsigned int i;

  fwrite(&pcmDataConfig->numPcmSignals, sizeof(unsigned int), 1, file);

  fwrite(&pcmDataConfig->pcmAlignAudioFlag, sizeof(unsigned int), 1, file);

  fwrite(&pcmDataConfig->pcmSamplingRateIdx, sizeof(unsigned int), 1, file);
  if( pcmDataConfig->pcmSamplingRateIdx == PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE ){
    fwrite(&pcmDataConfig->pcmSamplingRate, sizeof(unsigned int), 1, file);
  }

  fwrite(&pcmDataConfig->pcmBitsPerSampleIdx, sizeof(unsigned int), 1, file);

  fwrite(&pcmDataConfig->pcmFrameSizeIndex, sizeof(unsigned int), 1, file);
  if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE ){
    fwrite(&pcmDataConfig->pcmFixFrameSize, sizeof(unsigned int), 1, file);
  }

  for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
    fwrite(&pcmDataConfig->pcmSignal_ID[i], sizeof(unsigned int), 1, file);
  } /* for i */

  fwrite(&pcmDataConfig->bsPcmLoudnessValue, sizeof(unsigned int), 1, file);

  fwrite(&pcmDataConfig->pcmHasAttenuationGain, sizeof(unsigned int), 1, file);
  if( pcmDataConfig->pcmHasAttenuationGain ){
    fwrite(&pcmDataConfig->bsPcmAttenuationGain, sizeof(unsigned int), 1, file);
  }
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! gets the pcmSamplingRateIndex for a given sampling Rate */
enum PCM_DATA_CONFIG_RETURN getPcmSamplingRateIndex(unsigned int const samplingRate, enum PCM_SAMPLING_RATE_INDEX * const samplingRateIdx){
  size_t n;

  if( ! samplingRateIdx ){
    return PCM_DATA_CONFIG_RETURN_UNKNOWN_ERROR;
  }

  for( n = 0; n < NUM_PCM_SAMPLE_RATE_INDEX_INFOS; n++){
    if( pcmSamplingRateIndexInfos[n].samplingRate == samplingRate ){
      *samplingRateIdx = pcmSamplingRateIndexInfos[n].pcmSamplingRateIndex;
      return PCM_DATA_CONFIG_RETURN_NO_ERROR;
    }
  } /* for n */

  return PCM_DATA_CONFIG_RETURN_UNKNOWN_ERROR;
}

/*! gets the sampling rate in HZ for a given PCM data config */
unsigned int getSamplingRate(struct PCM_DATA_CONFIG const * const pcmDataConfig){
  size_t n;

  assert(pcmDataConfig);

  if( PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE == pcmDataConfig->pcmSamplingRateIdx ){
    return pcmDataConfig->pcmSamplingRate;
  }

  for( n = 0; n < NUM_PCM_SAMPLE_RATE_INDEX_INFOS; n++){
    if( pcmSamplingRateIndexInfos[n].pcmSamplingRateIndex == pcmDataConfig->pcmSamplingRateIdx ){
      return  pcmSamplingRateIndexInfos[n].samplingRate;
    }
  } /* for n */

  assert(0);

  return 0;
}
unsigned int getNumBitsForPcmBitsPerSampleIdx( enum PCM_BITS_PER_SAMPLE_IDX pcmBitsPerSampleIdx){
  switch(pcmBitsPerSampleIdx){
    case PCM_BITS_PER_SAMPLE_IDX_16_BITS:
      return 16;

    case PCM_BITS_PER_SAMPLE_IDX_24_BITS:
      return 24;

    case PCM_BITS_PER_SAMPLE_IDX_32_BITS:
      return 32;

    default:
      assert(0);
      return 0;
  }
}

/* converts the actual loudness level to bsPcmLoudnessValue */
unsigned int getBsPcmLoudnessValue(float const pcmLoudnessValue){
  float clippedLoudnessLevel;
  unsigned int bsPcmLoudnessValue;

  if( pcmLoudnessValue > MAX_VALUE_PCM_LOUDNESS_VALUE ){
    clippedLoudnessLevel = MAX_VALUE_PCM_LOUDNESS_VALUE;
  } else if( pcmLoudnessValue < MIN_VALUE_PCM_LOUDNESS_VALUE ){
    clippedLoudnessLevel = MIN_VALUE_PCM_LOUDNESS_VALUE;
  } else {
    clippedLoudnessLevel = pcmLoudnessValue;
  }
  
  bsPcmLoudnessValue = (unsigned int)((clippedLoudnessLevel - MIN_VALUE_PCM_LOUDNESS_VALUE)*4.0f + 0.5f);
  assert(bsPcmLoudnessValue <= (1<<(8)) );
  return bsPcmLoudnessValue;
}

/*! prints out content of the PCM data config struct */
enum PCM_DATA_CONFIG_RETURN pcmDataConfig_Print(
  FILE                          *       file,
  struct PCM_DATA_CONFIG  const * const pcmDataConfig,
  unsigned int                    const numSpacesIndentation
){
   enum PCM_DATA_CONFIG_RETURN retVal = PCM_DATA_CONFIG_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! file || ! pcmDataConfig ){
    retVal = PCM_DATA_CONFIG_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT PCM DATA CONFIG */
  if( ! isError(retVal) ){
    if (stdout == file) {
      unsigned int i;

      fprintf_indented(file, numSpacesIndentation, "numPcmSignals = %u\n", pcmDataConfig->numPcmSignals);

      fprintf_indented(file, numSpacesIndentation, "pcmAlignAudioFlag = %u\n", pcmDataConfig->pcmAlignAudioFlag);

      fprintf_indented(file, numSpacesIndentation, "pcmSamplingRateIndex = %d\n", (int)pcmDataConfig->pcmSamplingRateIdx);
      if( pcmDataConfig->pcmSamplingRateIdx == PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE ){
        fprintf_indented(file, numSpacesIndentation, "pcmSamplingRate = %u\n", pcmDataConfig->pcmSamplingRate);
      }

      fprintf_indented(file, numSpacesIndentation, "pcmBitsPerSampleIndex = %u\n", pcmDataConfig->pcmBitsPerSampleIdx);

      fprintf_indented(file, numSpacesIndentation, "pcmFrameSizeIndex = %d\n", (int)pcmDataConfig->pcmFrameSizeIndex);
      if( pcmDataConfig->pcmFrameSizeIndex == PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE ){
        fprintf_indented(file, numSpacesIndentation, "pcmFixFrameSize = %u\n", pcmDataConfig->pcmFixFrameSize);
      }

      for( i = 0; i < pcmDataConfig->numPcmSignals + 1; i++){
        fprintf_indented(file, numSpacesIndentation, "pcmSignal_ID[%u] = %u\n", i, pcmDataConfig->pcmSignal_ID[i]);
      } /* for i */

      fprintf_indented(file, numSpacesIndentation, "bsPcmLoudnessValue = %u\n", pcmDataConfig->bsPcmLoudnessValue);

      fprintf_indented(file, numSpacesIndentation, "pcmHasAttenuationGain = %u\n", pcmDataConfig->pcmHasAttenuationGain);
      if( pcmDataConfig->pcmHasAttenuationGain ){
        fprintf_indented(file, numSpacesIndentation, "bsPcmAttenuationGain = %u\n", pcmDataConfig->bsPcmAttenuationGain);
      }
    } else {
      pcmDataConfig_Print_binary(file, pcmDataConfig);
    }
  }

  return retVal;
}
