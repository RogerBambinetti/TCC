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

#include "wavM2Nlib.h"
#include "wavIO.h"
#include "cicp2geometry.h"

#define WAVM2N_MAX_NUM_OBJECTS 32

struct _WAVM2N
{
  WAVIO_HANDLE hWavIO_In[WAVM2N_MAX_NUM_OBJECTS];
  WAVIO_HANDLE hWavIO_Chan;
  WAVIO_HANDLE hWavIO_Obj[WAVM2N_MAX_NUM_OBJECTS];
 
  CICP2GEOMETRY_CHANNEL_GEOMETRY cicp2geometry_channel_geometry[WAVM2N_MAX_NUM_OBJECTS]; 

  float** inBuffer1;

  unsigned int nChannels;
  unsigned int blocklength;
  unsigned int nFilesIn;
  unsigned int nFilesOut;
};

int wavM2N_init(WAVM2N_HANDLE* hWavM2N, char* inName, char* outName, char* inGeoPath, int direction, int cicpIndex, unsigned int nObjects)
 
{
  int error_init = 0;
  unsigned int i = 0;
  unsigned int nInChannels1  = 0;
  unsigned int InSampleRate1 = 0;
  unsigned int InBytedepth1 = 0;
  unsigned long nTotalSamplesPerChannel1 = 0;
  int numChannels = 0;
  int numLFEs = 0;
  int nSamplesPerChannelFilled1 = 0;

  int blocklength = 1024;
    
  FILE* fOut[WAVM2N_MAX_NUM_OBJECTS];
  FILE* fIn[WAVM2N_MAX_NUM_OBJECTS];

  char* baseName;

  unsigned int nFilesIn=0;
  unsigned int nFilesOut=0;

  WAVM2N_HANDLE hWavM2NTemp;

  hWavM2NTemp = (WAVM2N_HANDLE)calloc(1,sizeof(struct _WAVM2N));

  /* get channel info */
  {


    if(nObjects==0){
      /* get mapping index enum */
      CICP2GEOMETRY_ERROR error = CICP2GEOMETRY_OK;
      if ( cicpIndex == -1 ) {
        if ( inGeoPath == NULL ) {
          fprintf(stderr,"No geometry file (-inGeo) is given. Required when cicpIndex == -1!\n");
          return -1;
        }
        error = cicp2geometry_get_geometry_from_file( inGeoPath, hWavM2NTemp->cicp2geometry_channel_geometry, &numChannels, &numLFEs);  
      }
      else {
        error = cicp2geometry_get_geometry_from_cicp( cicpIndex, hWavM2NTemp->cicp2geometry_channel_geometry, &numChannels, &numLFEs);
      }
      nFilesIn = numChannels + numLFEs;

      if ( error != CICP2GEOMETRY_OK ) {
        fprintf(stderr,"Error in cicp2geometry_get_geometry_from_cicp() !\n");
        return -1;
      }
    }
    else{
      nFilesIn=nObjects;
    }

    nFilesOut=nFilesIn;

    if(direction==1){
      nFilesIn=1;
      baseName=outName;
    }
    else{
      nFilesOut=1;
      baseName=inName;
    }

    hWavM2NTemp->nFilesIn=nFilesIn;
    hWavM2NTemp->nFilesOut=nFilesOut;
  }

  {
    char tmpString[FILENAME_MAX];

    for(i=0;i<max(nFilesIn, nFilesOut);i++){

      if(nObjects==0) {
        if(hWavM2NTemp->cicp2geometry_channel_geometry[i].LFE > 0) {
          if ( hWavM2NTemp->cicp2geometry_channel_geometry[i].Az == -45 ) {
            sprintf(tmpString, "%s_LFE%i", baseName, 2);
          }
          else {
            sprintf(tmpString, "%s_LFE%i", baseName, 1);
          }
        }
        else
        {
          if(hWavM2NTemp->cicp2geometry_channel_geometry[i].Az>=0) {
            if(hWavM2NTemp->cicp2geometry_channel_geometry[i].El>=0) {
              sprintf(tmpString,"%s_A+%03i_E+%02i",baseName,hWavM2NTemp->cicp2geometry_channel_geometry[i].Az,hWavM2NTemp->cicp2geometry_channel_geometry[i].El);
            }
            else {
              sprintf(tmpString,"%s_A+%03i_E-%02i",baseName,abs(hWavM2NTemp->cicp2geometry_channel_geometry[i].Az),abs(hWavM2NTemp->cicp2geometry_channel_geometry[i].El));
            }
          }
          else 
          {
            if(hWavM2NTemp->cicp2geometry_channel_geometry[i].El>=0) {
              sprintf(tmpString,"%s_A-%03i_E+%02i",baseName,abs(hWavM2NTemp->cicp2geometry_channel_geometry[i].Az),hWavM2NTemp->cicp2geometry_channel_geometry[i].El);
            }
            else {
              sprintf(tmpString,"%s_A-%03i_E-%02i",baseName,abs(hWavM2NTemp->cicp2geometry_channel_geometry[i].Az),abs(hWavM2NTemp->cicp2geometry_channel_geometry[i].El));
            }
          }
        }
      }
      else{
        sprintf(tmpString,"%s_%03i",baseName,i);
      }

      sprintf(tmpString,"%s.wav",tmpString);
      
      if(nFilesOut==1) {
        fIn[i] = fopen(tmpString, "rb");
        if (!fIn[i])
          {
            fprintf(stderr,"Error opening input %s\n", tmpString);
            return -1;
          }

      }
      else{
        fOut[i] = fopen(tmpString, "wb");
        if (!fOut[i])
          {
            fprintf(stderr,"Error opening output %s\n", tmpString);
            return -1;
          }
      }
    }

    if(nFilesOut==1) {
      sprintf(tmpString,"%s.wav",outName);
      fOut[0] = fopen(tmpString, "wb");
      if (!fOut[0])
        {
          fprintf(stderr,"Error opening output %s\n", tmpString);
          return -1;
        }
    }
    else {
      sprintf(tmpString,"%s.wav",inName);
      fIn[0] = fopen(tmpString, "rb");
      if (!fIn[0])
        {
          fprintf(stderr,"Error opening output %s\n", tmpString);
          return -1;
        }
    }
  }

  for ( i = 0; i < nFilesIn; ++i )
    {
      error_init = wavIO_init(&(hWavM2NTemp->hWavIO_In[i]), blocklength, 0, 0);
      error_init = wavIO_openRead(hWavM2NTemp->hWavIO_In[i],
                                  fIn[i],
                                  &nInChannels1,
                                  &InSampleRate1, 
                                  &InBytedepth1,
                                  &nTotalSamplesPerChannel1, 
                                  &nSamplesPerChannelFilled1);
    }

  for ( i = 0; i < nFilesOut; ++i )
    {
      error_init = wavIO_init(&(hWavM2NTemp->hWavIO_Obj[i]), blocklength, 0, 0);
      error_init = wavIO_setTags(hWavM2NTemp->hWavIO_Obj[i], baseName, NULL, NULL, NULL, NULL);
      error_init = wavIO_openWrite(hWavM2NTemp->hWavIO_Obj[i],
                                   fOut[i],
                                   nFilesIn,
                                   InSampleRate1,
                                   InBytedepth1);
    }

  if ( 0 != error_init ) {
    return -1;
  }

  /* alloc local buffers */

  hWavM2NTemp->inBuffer1 = (float**)calloc(max(nInChannels1,max(nFilesIn, nFilesOut)),sizeof(float*));

  for (i = 0; i< max(nInChannels1,max(nFilesIn, nFilesOut)); i++) {
      hWavM2NTemp->inBuffer1[i]  = (float*)calloc(blocklength, sizeof(float));
  }

  hWavM2NTemp->blocklength = blocklength;
  hWavM2NTemp->nChannels   = max(nInChannels1,max(nFilesIn, nFilesOut));

  *hWavM2N = hWavM2NTemp;

  return 0;
}


int wavM2N_mix(WAVM2N_HANDLE hWavM2N, unsigned int* lastFrame)
{
  unsigned int k = 0;
  unsigned int isLastFrame = 0;
  unsigned int nZerosPaddedBeginning = 0;
  unsigned int nZerosPaddedEnd = 0;
  unsigned int samplesReadPerChannel = 0;
  unsigned int samplesWrittenPerChannel = 0;

  for (k = 0; k < hWavM2N->nFilesIn; k++) {
    wavIO_readFrame(hWavM2N->hWavIO_In[k],&hWavM2N->inBuffer1[k],&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
  }

  for (k = 0; k < hWavM2N->nFilesOut; k++) {
    wavIO_writeFrame(hWavM2N->hWavIO_Obj[k], &hWavM2N->inBuffer1[k],samplesReadPerChannel,&samplesWrittenPerChannel);
  }

  *lastFrame = isLastFrame;

  return 0;

}


int wavM2N_close(WAVM2N_HANDLE hWavM2N, unsigned long* nTotalSamplesWrittenPerChannel)
{
  unsigned int i = 0;
  if ( hWavM2N )
    {
      if ( hWavM2N->hWavIO_Chan )
        {   
          wavIO_updateWavHeader(hWavM2N->hWavIO_Chan, nTotalSamplesWrittenPerChannel);
          wavIO_close(hWavM2N->hWavIO_Chan);
        }

      for ( i = 0; i < hWavM2N->nFilesOut; ++i )
        {
          wavIO_updateWavHeader(hWavM2N->hWavIO_Obj[i], nTotalSamplesWrittenPerChannel);
          wavIO_close(hWavM2N->hWavIO_Obj[i]);
        }

       for ( i = 0; i < hWavM2N->nFilesIn; ++i )
        {
       if (hWavM2N->hWavIO_In[i])
        wavIO_close(hWavM2N->hWavIO_In[i]);
      }

  
      if (hWavM2N->inBuffer1)
        {
          for (i=0; i< hWavM2N->nChannels; i++) {
            free(hWavM2N->inBuffer1[i]);
          }

          free(hWavM2N->inBuffer1);
        }
    }

  free(hWavM2N);

  return 0;
}
