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
#ifndef PCM_DATA_CONFIG_H
#define PCM_DATA_CONFIG_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define MAX_NUM_PCM_SIGNALS (2<<7)
#define MAX_NUM_PCM_SIGNAL_IDS (MAX_NUM_PCM_SIGNALS)

#define MAX_VALUE_PCM_LOUDNESS_VALUE (6.0f)
#define MIN_VALUE_PCM_LOUDNESS_VALUE (-57.75f)
/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

enum PCM_DATA_CONFIG_RETURN {
  PCM_DATA_CONFIG_RETURN_NO_ERROR = 0,
  PCM_DATA_CONFIG_RETURN_UNKNOWN_ERROR
};

/*! values of pcmSamplingRateIndex */
enum PCM_SAMPLING_RATE_INDEX {
  PCM_SAMPLING_RATE_INDEX_96000 = 0x00,
  PCM_SAMPLING_RATE_INDEX_88200 = 0x01,
  PCM_SAMPLING_RATE_INDEX_64000 = 0x02,
  PCM_SAMPLING_RATE_INDEX_48000 = 0x03,
  PCM_SAMPLING_RATE_INDEX_44100 = 0x04,
  PCM_SAMPLING_RATE_INDEX_32000 = 0x05,
  PCM_SAMPLING_RATE_INDEX_24000 = 0x06,
  PCM_SAMPLING_RATE_INDEX_22050 = 0x07,
  PCM_SAMPLING_RATE_INDEX_16000 = 0x08,
  PCM_SAMPLING_RATE_INDEX_ESCAPE_VALUE = 0x1f,
};

/*! values of pcmBitsPerSampleIndex */
enum PCM_BITS_PER_SAMPLE_IDX {
  PCM_BITS_PER_SAMPLE_IDX_16_BITS = 0,
  PCM_BITS_PER_SAMPLE_IDX_24_BITS = 1,
  PCM_BITS_PER_SAMPLE_IDX_32_BITS = 2
  /* values 3 to 7 are reserved */
};

enum PCM_FRAME_SIZE_INDEX {
  PCM_FRAME_SIZE_INDEX_AS_DEF_IN_ISOIEC_230003_3 = 0,
  PCM_FRAME_SIZE_INDEX_768                       = 1,
  PCM_FRAME_SIZE_INDEX_1024                      = 2,
  PCM_FRAME_SIZE_INDEX_2048                      = 3,
  PCM_FRAME_SIZE_INDEX_4096                      = 4,
  PCM_FRAME_SIZE_INDEX_FIX_FRAME_SIZE            = 5,
  PCM_FRAME_SIZE_INDEX_VAR_FRAME_SIZE            = 6
  /* index 7 is reserved */
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct PCM_DATA_CONFIG {
  unsigned int numPcmSignals;          /**< ATTENTION: actual number of PCM signals is numPcmSignals + 1 */
  unsigned int pcmAlignAudioFlag;
  enum PCM_SAMPLING_RATE_INDEX pcmSamplingRateIdx;
  unsigned int pcmSamplingRate;        /**< only used if pcmSamplingRateIdx == 0x1f */
  enum PCM_BITS_PER_SAMPLE_IDX pcmBitsPerSampleIdx;
  enum PCM_FRAME_SIZE_INDEX pcmFrameSizeIndex;
  unsigned int pcmFixFrameSize;        /**< only used if pcmFrameSizeIndex == 5 */

  unsigned int pcmSignal_ID[MAX_NUM_PCM_SIGNAL_IDS];

  unsigned int bsPcmLoudnessValue;

  unsigned int pcmHasAttenuationGain;
  unsigned int bsPcmAttenuationGain;   /**< only used if pcmHasAttenuationGain == 1 */
};
/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! gets the pcmSamplingRateIndex for a given sampling Rate */
enum PCM_DATA_CONFIG_RETURN getPcmSamplingRateIndex(unsigned int const samplingRate, enum PCM_SAMPLING_RATE_INDEX * const samplingRateIdx);

/*! gets the sampling rate in HZ for a given PCM data config */
unsigned int getSamplingRate(struct PCM_DATA_CONFIG const * const pcmDataConfig);

/*! gets number of bits for PCM_BITS_PER_SAMPLE_IDX */
unsigned int getNumBitsForPcmBitsPerSampleIdx( enum PCM_BITS_PER_SAMPLE_IDX pcmBitsPerSampleIdx);

/* converts the actual loudness level to bsPcmLoudnessValue */
unsigned int getBsPcmLoudnessValue(float const pcmLoudnessValue);

/*! prints out content of the PCM data config struct */
enum PCM_DATA_CONFIG_RETURN pcmDataConfig_Print(
  FILE                          *       file,
  struct PCM_DATA_CONFIG  const * const pcmDataConfig,
  unsigned int                    const numSpacesIndentation
);

#endif /* ifndef PCM_DATA_CONFIG_H */
