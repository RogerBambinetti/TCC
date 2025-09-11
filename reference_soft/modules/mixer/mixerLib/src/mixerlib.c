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
#include <assert.h>

#include "mixerlib.h"
#include "wavIO.h"


struct _MIXER
{
  WAVIO_HANDLE hWavIO1;
  WAVIO_HANDLE hWavIO2;
  WAVIO_HANDLE hWavIO3;

  float** inBuffer1;
  float** inBuffer2;
  float** outBuffer;

  unsigned int blocklength;
  unsigned int nChannels;
  
  float gain1;
  float gain2;

};


int mixer_init(MIXER_HANDLE* hMixer, FILE* fIn1, FILE* fIn2, FILE* fOut, int delay1, int delay2, int delay3, float gain1, float gain2)
{
  int error_init = 0;
  unsigned int i = 0;
  unsigned int nInChannels1  = 0;
  unsigned int InSampleRate1 = 0;
  unsigned int InBytedepth1 = 0;
  unsigned long nTotalSamplesPerChannel1 = 0;
  int nSamplesPerChannelFilled1 = 0;

  unsigned int nInChannels2  = 0;
  unsigned int InSampleRate2 = 0;
  unsigned int InBytedepth2  = 0;
  unsigned long nTotalSamplesPerChannel2 = 0;
  int nSamplesPerChannelFilled2 = 0;

  int blocklength = 1024;

  MIXER_HANDLE hMixerTemp;

  if (!fIn1 || !fIn2 || !fOut)
    return -1;

  hMixerTemp = (MIXER_HANDLE)calloc(1,sizeof(struct _MIXER));

  error_init = wavIO_init(&(hMixerTemp->hWavIO1), blocklength, 0, delay1);
  error_init = wavIO_init(&(hMixerTemp->hWavIO2), blocklength, 0, delay2);
  error_init = wavIO_init(&(hMixerTemp->hWavIO3), blocklength, 0, delay3);

  error_init = wavIO_openRead(hMixerTemp->hWavIO1,
                              fIn1,
                              &nInChannels1,
                              &InSampleRate1, 
                              &InBytedepth1,
                              &nTotalSamplesPerChannel1, 
                              &nSamplesPerChannelFilled1);

  if ( 0 != error_init )
    return -1;

  error_init = wavIO_openRead(hMixerTemp->hWavIO2,
                              fIn2,
                              &nInChannels2,
                              &InSampleRate2, 
                              &InBytedepth2,
                              &nTotalSamplesPerChannel2, 
                              &nSamplesPerChannelFilled2);

  if ( 0 != error_init )
    return -1;

  if ( nInChannels1 !=  nInChannels2 )
    return -1;

  if ( InSampleRate1 != InSampleRate2 )
    return -1;

  if ( InBytedepth1 != InBytedepth2 )
    return -1;

  if ( (nTotalSamplesPerChannel1 + delay1) != (nTotalSamplesPerChannel2 + delay2) )
    return -1;

  error_init = wavIO_openWrite(hMixerTemp->hWavIO3,
                               fOut,
                               nInChannels1,
                               InSampleRate1,
                               InBytedepth1);
  
  if ( 0 != error_init )
    return -1;


  /* alloc local buffers */

  hMixerTemp->inBuffer1 = (float**)calloc(nInChannels1,sizeof(float*));
  hMixerTemp->inBuffer2 = (float**)calloc(nInChannels1,sizeof(float*));
  hMixerTemp->outBuffer = (float**)calloc(nInChannels1,sizeof(float*));
  
  for (i = 0; i< nInChannels1; i++)
    {
      hMixerTemp->inBuffer1[i] = (float*)calloc(blocklength, sizeof(float));
      hMixerTemp->inBuffer2[i] = (float*)calloc(blocklength, sizeof(float));
      hMixerTemp->outBuffer[i] = (float*)calloc(blocklength, sizeof(float));
    }

  hMixerTemp->blocklength = blocklength;
  hMixerTemp->nChannels   = nInChannels1;
  hMixerTemp->gain1       = gain1;
  hMixerTemp->gain2       = gain2;


  *hMixer = hMixerTemp;

  return 0;

}

int mixer_mix(MIXER_HANDLE hMixer, unsigned int* lastFrame)
{
  unsigned int i = 0, j = 0;
  unsigned int isLastFrame = 0;
  unsigned int nZerosPaddedBeginning = 0;
  unsigned int nZerosPaddedEnd = 0;
  unsigned int samplesReadPerChannel = 0;
  unsigned int samplesWrittenPerChannel = 0;
  unsigned int samplesToWritePerChannel = 0;

  wavIO_readFrame(hMixer->hWavIO1,hMixer->inBuffer1,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);

  wavIO_readFrame(hMixer->hWavIO2,hMixer->inBuffer2,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);

  for (i = 0; i < max(samplesReadPerChannel, hMixer->blocklength); i++)
    {
      for (j = 0; j < hMixer->nChannels; j++)
        {
          hMixer->outBuffer[j][i] = hMixer->gain1 * hMixer->inBuffer1[j][i] + hMixer->gain2 * hMixer->inBuffer2[j][i];
        }
    }

  /* Add up possible delay and actually read samples */
  samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;


  wavIO_writeFrame(hMixer->hWavIO3, hMixer->outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);

  *lastFrame = isLastFrame;


  return 0;
}


int mixer_close(MIXER_HANDLE hMixer, unsigned long* nTotalSamplesWrittenPerChannel)
{
  unsigned int i = 0;
  if ( hMixer )
    {
      wavIO_updateWavHeader(hMixer->hWavIO3, nTotalSamplesWrittenPerChannel);

      if (hMixer->hWavIO1)
        wavIO_close(hMixer->hWavIO1);
      if (hMixer->hWavIO2)
        wavIO_close(hMixer->hWavIO2);
      if (hMixer->hWavIO3)
        wavIO_close(hMixer->hWavIO3);

      if (hMixer->inBuffer1)
        {
          for (i=0; i< hMixer->nChannels; i++) {
            free(hMixer->inBuffer1[i]);
          }

          free(hMixer->inBuffer1);
        }
      if (hMixer->inBuffer2)
        {
          for (i=0; i< hMixer->nChannels; i++) {
            free(hMixer->inBuffer2[i]);
          }

          free(hMixer->inBuffer2);
        }

      if (hMixer->outBuffer)
        {
          for (i=0; i< hMixer->nChannels; i++) {
            free(hMixer->outBuffer[i]);
          }

          free(hMixer->outBuffer);
        }
    }

  free(hMixer);

  return 0;
}
