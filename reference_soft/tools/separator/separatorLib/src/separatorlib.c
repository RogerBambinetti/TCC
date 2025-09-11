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

#include "separatorlib.h"
#include "wavIO.h"

struct _SEPARATOR
{
  WAVIO_HANDLE hWavIO_In;
  WAVIO_HANDLE hWavIO_Chan;
  WAVIO_HANDLE hWavIO_Obj[SEPARATOR_MAX_NUM_OBJECTS];

  float** inBuffer1;
  float** outBuffer1;
  float** outBuffer2;

  unsigned int blocklength;
  unsigned int nChannels;
  unsigned int nObjects;
  unsigned int nChannelOffset;  
  unsigned int nChannelsOut;  

};

int separator_init(SEPARATOR_HANDLE* hSeparator, FILE* fIn, FILE* fOutChan, FILE* fOutObj[SEPARATOR_MAX_NUM_OBJECTS], unsigned int nObjects, unsigned int nChannelOffset, unsigned int nChannelsOut)
{
  int error_init = 0;
  unsigned int i = 0;
  unsigned int nInChannels1  = 0;
  unsigned int InSampleRate1 = 0;
  unsigned int InBytedepth1 = 0;
  unsigned long nTotalSamplesPerChannel1 = 0;
  int nSamplesPerChannelFilled1 = 0;

  int blocklength = 1024;

  unsigned int nChans = 0;
     

  SEPARATOR_HANDLE hSeparatorTemp;

  if(nChannelsOut == 0) 
    {
      if (!fIn || !fOutChan || !fOutObj[0])
        return -1;
    }
  else
    {
      if (!fIn || !fOutChan)
        return -1;
    }

  if ( nChannelsOut!=0 ) nChans = nChannelsOut;

  hSeparatorTemp = (SEPARATOR_HANDLE)calloc(1,sizeof(struct _SEPARATOR));

  error_init = wavIO_init(&(hSeparatorTemp->hWavIO_In), blocklength, 0, 0);

  error_init = wavIO_openRead(hSeparatorTemp->hWavIO_In,
                              fIn,
                              &nInChannels1,
                              &InSampleRate1, 
                              &InBytedepth1,
                              &nTotalSamplesPerChannel1, 
                              &nSamplesPerChannelFilled1);


  if ( ((nObjects <= 0)&& (nChannelsOut == 0)) || (nObjects > nInChannels1) )
    return -1;

  if ( nChannelsOut != 0 ) 
    {
      nChans = nChannelsOut;
    }
  else 
    {
      nChans = nInChannels1 - nObjects;
    }

  if ( nInChannels1 > nObjects ) 
   {

     error_init = wavIO_init(&(hSeparatorTemp->hWavIO_Chan), blocklength, 0, 0);
     error_init = wavIO_openWrite(hSeparatorTemp->hWavIO_Chan,
                                  fOutChan,
                                  nChans,
                                  InSampleRate1,
                                  InBytedepth1);
     
     if ( 0 != error_init )
       return -1;
     
   }

  for ( i = 0; i < nObjects; ++i )
    {
      error_init = wavIO_init(&(hSeparatorTemp->hWavIO_Obj[i]), blocklength, 0, 0);
      error_init = wavIO_openWrite(hSeparatorTemp->hWavIO_Obj[i],
                                   fOutObj[i],
                                   1,
                                   InSampleRate1,
                                   InBytedepth1);
    }

  if ( 0 != error_init )
    return -1;

  /* alloc local buffers */

  hSeparatorTemp->inBuffer1  = (float**)calloc(nInChannels1,sizeof(float*));
  hSeparatorTemp->outBuffer1 = (float**)calloc(nChans,sizeof(float*));
  hSeparatorTemp->outBuffer2 = (float**)calloc(1, sizeof(float*));


  for (i = 0; i< nInChannels1; i++)
      hSeparatorTemp->inBuffer1[i]  = (float*)calloc(blocklength, sizeof(float));

  for (i = 0; i< nChans; i++)
      hSeparatorTemp->outBuffer1[i] = (float*)calloc(blocklength, sizeof(float));

   for (i = 0; i < 1; i++)
     hSeparatorTemp->outBuffer2[i] = (float*)calloc(blocklength, sizeof(float));

  hSeparatorTemp->blocklength = blocklength;

  hSeparatorTemp->nChannels   = nInChannels1;

  hSeparatorTemp->nObjects = nObjects;
  hSeparatorTemp->nChannelOffset = nChannelOffset;
  hSeparatorTemp->nChannelsOut   = nChannelsOut;

  *hSeparator = hSeparatorTemp;


  return 0;
}


int separator_mix(SEPARATOR_HANDLE hSeparator, unsigned int* lastFrame)
{
  unsigned int i = 0, j = 0, k = 0;
  unsigned int isLastFrame = 0;
  unsigned int nZerosPaddedBeginning = 0;
  unsigned int nZerosPaddedEnd = 0;
  unsigned int samplesReadPerChannel = 0;
  unsigned int samplesWrittenPerChannel = 0;
  unsigned int offset = hSeparator->nChannelOffset;
  unsigned int nChans = hSeparator->nChannels - hSeparator->nObjects;

  if ( hSeparator->nChannelsOut != 0 ) nChans = hSeparator->nChannelsOut;

  wavIO_readFrame(hSeparator->hWavIO_In,hSeparator->inBuffer1,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);

  if ( hSeparator->nChannels < offset + nChans ) {
    fprintf(stderr, "Invalid number of channels and/or offset given. Possible read out of bounds.\n");
    return -1;
  }

  if ( hSeparator->hWavIO_Chan )
    {
      for (i = 0; i < samplesReadPerChannel; i++)
        {
          for (j = 0; j < nChans; j++)
            {
              hSeparator->outBuffer1[j][i] = hSeparator->inBuffer1[j + offset][i];
            }
        }

      wavIO_writeFrame(hSeparator->hWavIO_Chan, hSeparator->outBuffer1,samplesReadPerChannel,&samplesWrittenPerChannel);
    }
     
  offset = hSeparator->nChannels - hSeparator->nObjects;

  if ( hSeparator->nChannels < offset + hSeparator->nObjects ) {
    fprintf(stderr, "Invalid number of channels and/or offset given. Possible read out of bounds.\n");
    return -1;
  }

  for (k = 0; k < hSeparator->nObjects; k++)
    {
      for (i = 0; i < samplesReadPerChannel; i++)
        {
          hSeparator->outBuffer2[0][i] = hSeparator->inBuffer1[offset + k][i];
        }
      wavIO_writeFrame(hSeparator->hWavIO_Obj[k], hSeparator->outBuffer2,samplesReadPerChannel,&samplesWrittenPerChannel);

    }

  *lastFrame = isLastFrame;

  return 0;

}


int separator_close(SEPARATOR_HANDLE hSeparator, unsigned long* nTotalSamplesWrittenPerChannel)
{
  unsigned int i = 0;
  if ( hSeparator )
    {
      if ( hSeparator->hWavIO_Chan )
        {   
          wavIO_updateWavHeader(hSeparator->hWavIO_Chan, nTotalSamplesWrittenPerChannel);
          wavIO_close(hSeparator->hWavIO_Chan);
        }

      for ( i = 0; i < hSeparator->nObjects; ++i )
        {
          wavIO_updateWavHeader(hSeparator->hWavIO_Obj[i], nTotalSamplesWrittenPerChannel);
          wavIO_close(hSeparator->hWavIO_Obj[i]);
        }

      if (hSeparator->hWavIO_In)
        wavIO_close(hSeparator->hWavIO_In);

      if (hSeparator->inBuffer1)
        {
          for (i=0; i< hSeparator->nChannels; i++) {
            free(hSeparator->inBuffer1[i]);
          }

          free(hSeparator->inBuffer1);
        }
      if (hSeparator->outBuffer1)
        {
          unsigned int nChans = hSeparator->nChannels - hSeparator->nObjects;

          if ( hSeparator->nChannelsOut != 0 ) nChans = hSeparator->nChannelsOut;

          for (i=0; i< nChans; i++) {
            free(hSeparator->outBuffer1[i]);
          }

          free(hSeparator->outBuffer1);
        }

      if (hSeparator->outBuffer2)
        {
          for (i=0; i< 1; i++) {
            free(hSeparator->outBuffer2[i]);
          }

          free(hSeparator->outBuffer2);
        }
    }

  free(hSeparator);

  return 0;
}
