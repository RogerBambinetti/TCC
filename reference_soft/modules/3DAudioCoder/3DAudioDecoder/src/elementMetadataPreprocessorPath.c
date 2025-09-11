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

Copyright (c) ISO/IEC 2015.

***********************************************************************************/

#include "elementMetadataPreprocessorPath.h"

int MDPreproc_separateSDchannels(char* combined, char* sdChannelsPath, char* nonSDchannelsPath, int* SDchannelIdx, int numTotalChannels, int numSDchannels, int processingDomain)
{
  int error = 0;
  int i, m, n, p;
  unsigned int um, un;
  int numFiles = 2;

  unsigned int nInChannels = 0;
  unsigned int InSampleRate;
  unsigned long nTotalSamplesPerChannel;
  unsigned long nTotalSamplesWrittenPerChannel;
  unsigned int InBytedepth = 0;
  int nSamplesPerChannelFilled;
  int nOutChannels = 0;
  int NumberSamplesWritten = 0;
  unsigned int isLastFrame = 0;
  int frameNo = 0;
  int ctOut = 0;

  FILE* fAudioIn;
  FILE* fAudioOut;
  WAVIO_HANDLE hWavIO = NULL;
  float **inBuffer = NULL;
  float **outBuffer = NULL;
  unsigned int wavio_blocklength = 4096;

  for (i = 0; i < numFiles; i++)
  {
    char audioInPath[FILENAME_MAX];
    char audioInPath_real[FILENAME_MAX];
    char audioInPath_imag[FILENAME_MAX];
    char audioOutPath[FILENAME_MAX];
    char audioOutPath_real[FILENAME_MAX];
    char audioOutPath_imag[FILENAME_MAX];

    strcpy(audioInPath, combined);

    if (i == 0) /* sd channels */
    {
      strcpy(audioOutPath, sdChannelsPath);

      if (processingDomain == 1) /* QMF domain processing */
      {
        strcpy(audioInPath_real, combined);
        strcpy(audioInPath_imag, combined);
        strcat(audioInPath_real, "_real");
        strcat(audioInPath_imag, "_imag");
        strcat(audioInPath_real,  ".qmf");
        strcat(audioInPath_imag,  ".qmf");

        strcpy(audioOutPath_real, sdChannelsPath);
        strcpy(audioOutPath_imag, sdChannelsPath);
        strcat(audioOutPath_real, "_real");
        strcat(audioOutPath_imag, "_imag");
        strcat(audioOutPath_real,  ".qmf");
        strcat(audioOutPath_imag,  ".qmf");
      }

      nOutChannels = numSDchannels;
    }
    else
    {
      strcpy(audioOutPath, nonSDchannelsPath);

      if (processingDomain == 1) /* QMF doamin processing */
      {
        strcpy(audioInPath_real, combined);
        strcpy(audioInPath_imag, combined);
        strcat(audioInPath_real, "_real");
        strcat(audioInPath_imag, "_imag");
        strcat(audioInPath_real,  ".qmf");
        strcat(audioInPath_imag,  ".qmf");

        strcpy(audioOutPath_real,nonSDchannelsPath);
        strcpy(audioOutPath_imag,nonSDchannelsPath);
        strcat(audioOutPath_real,"_real");
        strcat(audioOutPath_imag,"_imag");
        strcat(audioOutPath_real,".qmf");
        strcat(audioOutPath_imag,".qmf");
      }

      nOutChannels = numTotalChannels - numSDchannels;
    }

    if (processingDomain == 0)
    {
      if (nOutChannels)
      {
        fAudioIn = fopen(audioInPath, "rb");
        if (!fAudioIn)
        {
          fprintf(stderr, "Error in splitting channel-based output: Could not open input file: %s.\n", audioInPath);
          return -1;
        }
        fAudioOut = fopen(audioOutPath, "wb");
        if (!fAudioOut)
        {
          fprintf(stderr, "Error in splitting channel-based outout: Could not open output file; %s.\n", audioOutPath);
        }
        error = wavIO_init(&hWavIO, wavio_blocklength, 0, 0);
        error = wavIO_openRead(hWavIO,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,&nSamplesPerChannelFilled);
        error = wavIO_openWrite(hWavIO,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

        if (nInChannels)
        {
          inBuffer = (float**)calloc(nInChannels,sizeof(float*));
          for (um = 0; um < nInChannels; um++) {
            inBuffer[um] = (float*)calloc(wavio_blocklength,sizeof(float));
          }
        }
        if (nOutChannels)
        {
          outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
          for (m = 0; m < nOutChannels; m++)
            outBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }
        frameNo = 0;
        isLastFrame = 0;

        do
        {
          unsigned int samplesReadPerChannel = 0;
          unsigned int samplesToWritePerChannel = 0;
          unsigned int samplesWrittenPerChannel = 0;
          unsigned int nZerosPaddedBeginning = 0;
          unsigned int nZerosPaddedEnd = 0;

          frameNo++;
          ctOut = 0;

          /* read frame if input file is available */
          if ( fAudioIn )
          {
            error = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
            if (i == 0) /* sd channels */
            {
              for (n = 0; n < nOutChannels; n++)
              {
                for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                {
                  outBuffer[n][um] = inBuffer[SDchannelIdx[n]][um];
                }
              }
            }
            else
            {
              for (un = 0; un < nInChannels; un++)
              {
                int isSDchannel = 0;
                for (p = 0; p < numSDchannels; p++)
                {
                  if (SDchannelIdx[p] == un)
                  {
                    isSDchannel = 1;
                    break;
                  }
                }
                if (isSDchannel == 0)
                {
                  for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                  {
                    outBuffer[ctOut][um] = inBuffer[un][um];
                  }
                  ctOut++;
                }
              }
            }

            /* Add up possible delay and actually read samples */
            samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
          }

          if ( fAudioOut )
          {
            /* write frame */
            error = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
            NumberSamplesWritten += samplesWrittenPerChannel;
          }
        }
        while (! isLastFrame);

        /* ----------------- EXIT WAV IO ----------------- */
        error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
        error = wavIO_close(hWavIO);
      }
    }

    else if (processingDomain == 1) /* QMF domain */
    {
      if (nOutChannels)
      {
        FILE *fAudioInImag;
        FILE *fAudioOutImag;
        WAVIO_HANDLE hWavIOImag;

        fAudioIn = fopen(audioInPath_real,"rb");
        fAudioInImag = fopen(audioInPath_imag,"rb");

        if ((fAudioIn == NULL) || (fAudioInImag == NULL))
        {
          fprintf(stderr, "Error in splitting channel-based output: Could not open input file: %s.\n", audioInPath);
          return -1;
        }

        fAudioOut = fopen(audioOutPath_real,"wb");
        fAudioOutImag = fopen(audioOutPath_imag,"wb");

        if ((fAudioOut == NULL) || (fAudioOutImag == NULL))
        {
          fprintf(stderr, "Error in splitting channel-based output: Could not open output file: %s.\n", audioOutPath);
          return -1;
        }

        error = wavIO_init(&hWavIO,wavio_blocklength,0,0);
        error = wavIO_init(&hWavIOImag,wavio_blocklength,0,0);

        error = wavIO_openRead(hWavIO,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,&nSamplesPerChannelFilled);
        error = wavIO_openWrite(hWavIO,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

        error = wavIO_openRead(hWavIOImag,fAudioIn,&nInChannels,&InSampleRate,&InBytedepth,&nTotalSamplesPerChannel,&nSamplesPerChannelFilled);
        error = wavIO_openWrite(hWavIOImag,fAudioOut,nOutChannels,InSampleRate,InBytedepth);

        if (nInChannels)
        {
          inBuffer = (float**)calloc(nInChannels,sizeof(float*));
          for (um = 0; um < nInChannels; um++) {
            inBuffer[um] = (float*)calloc(wavio_blocklength,sizeof(float));
          }
        }

        if (nOutChannels)
        {
          outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
          for (m = 0; m < nOutChannels; m++)
            outBuffer[m] = (float*)calloc(wavio_blocklength,sizeof(float));
        }

        do
        {
          unsigned int samplesReadPerChannel = 0;
          unsigned int samplesToWritePerChannel = 0;
          unsigned int samplesWrittenPerChannel = 0;
          unsigned int nZerosPaddedBeginning = 0;
          unsigned int nZerosPaddedEnd = 0;

          frameNo++;
          ctOut = 0;

          /* read frame if input file is available */
          if ( fAudioIn )
          {
            error = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
            if (i == 0) /* sd channels */
            {
              for (n = 0; n < nOutChannels; n++)
              {
                for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                {
                  outBuffer[n][um] = inBuffer[SDchannelIdx[n]][um];
                }
              }
            }
            else /* non sd channels */
            {
              for (un = 0; un < nInChannels; un++)
              {
                int isSDchannel = 0;
                for (p = 0; p < numSDchannels; p++)
                {
                  if (SDchannelIdx[p] == un)
                  {
                    isSDchannel = 1;
                    break;
                  }
                }
                if (isSDchannel == 0)
                {
                  for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                  {
                    outBuffer[ctOut][um] = inBuffer[un][um];
                  }
                  ctOut++;
                }
              }
            }
            samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
          }

          if ( fAudioOut )
          {
            /* write frame */
            error = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
          }

          if ( fAudioInImag )
          {
            error = wavIO_readFrame(hWavIOImag,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
            if (i == 0) /* sd channels */
            {
              for (n = 0; n < nOutChannels; n++)
              {
                for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                {
                  outBuffer[n][um] = inBuffer[SDchannelIdx[n]][um];
                }
              }
            }
            else /* non sd channels */
            {
              for (un = 0; un < nInChannels; un++)
              {
                int isSDchannel = 0;
                for (p = 0; p < numSDchannels; p++)
                {
                  if (SDchannelIdx[p] == un)
                  {
                    isSDchannel = 1;
                    break;
                  }
                }
                if (isSDchannel == 0)
                {
                  for (um = 0; um < max(samplesReadPerChannel, wavio_blocklength); um++)
                  {
                    outBuffer[ctOut][um] = inBuffer[un][um];
                  }
                  ctOut++;
                }
              }
            }
            samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
          }

          if ( fAudioOutImag )
          {
            /* write frame */
            error = wavIO_writeFrame(hWavIOImag,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);

          }
          NumberSamplesWritten += samplesWrittenPerChannel;

        }
        while (! isLastFrame);

        /* ----------------- EXIT WAV IO ----------------- */
        error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
        error = wavIO_close(hWavIO);

        error = wavIO_updateWavHeader(hWavIOImag, &nTotalSamplesWrittenPerChannel);
        error = wavIO_close(hWavIOImag);
      }
    }
  }
  return error;
}

int MDPreproc_getSDchannelIdx(char* sdChannels_indexListOutpath, int *SDchannelIdx, int *numSDchannels)
{
  int error = 0;
  FILE * idxList;
  char line[512] = {0};
  int n = 0;

  idxList = fopen(sdChannels_indexListOutpath,"r");
  if (!idxList)
  {
    return -1;
  }
  fgets(line, 511, idxList);
  {
    char* pChar = line;
    int i = 0;
    line[strlen(line)+1] = '\0';
    line[strlen(line)] = '\n';
    while ( pChar[i] != '\0')
    {
      if ( pChar[i] == ' ' || pChar[i] == '\t')
      {
        pChar[i] = '\n';
      }
      i++;
    }
    pChar = line;
    n = atoi(pChar);
    while ( *(pChar) != ':' )
            pChar++;
      pChar++;
    while (  (*pChar) == '\n' || (*pChar) == '\r'  )
        pChar++;

    for (i = 0; i < n; i++)
    {
      SDchannelIdx[i] = atoi(pChar);
      if (i < n-1)
      {
        while ( *(pChar) != ',' )
              pChar++;
        pChar++;
      }
    }
  }

  fclose(idxList);
  *numSDchannels = n;
  return error;
}

int MDPreproc_fillWithZeroChannels(char* audioFilePathIn, char* audioFilePathOut, char* audioFileSetup, char* targetSetup, int processingDomain)
{
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoSource[MAE_MAX_NUM_SPEAKERS];
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoTarget[MAE_MAX_NUM_SPEAKERS];
  int numChannelsSource, numChannelsTarget, numLFEsSource, numLFEsTarget;
  unsigned int numChannelsSourceTotal = 0, numChannelsTargetTotal = 0;
  int error = 0;

  unsigned int InSampleRate;
  unsigned long nTotalSamplesPerChannel;
  unsigned long nTotalSamplesWrittenPerChannel;
  int nSamplesPerChannelFilled;
  unsigned int bytedepth = 0;

  unsigned int i, j, k;
  unsigned int blocklength = 4096;
  int frameNo = 0;
  unsigned int isLastFrame = 0;
  int NumberSamplesWritten = 0;


  cicp2geometry_get_geometry_from_file( audioFileSetup, geoSource, &numChannelsSource, &numLFEsSource );
  cicp2geometry_get_geometry_from_file( targetSetup, geoTarget, &numChannelsTarget, &numLFEsTarget );

  numChannelsTargetTotal = numChannelsTarget + numLFEsTarget;

  if (processingDomain == 0)
  {
    WAVIO_HANDLE hWavIO_source = NULL;
    WAVIO_HANDLE hWavIO_dest = NULL;
    FILE* audioIn;
    FILE* audioOut;

    float **inBuffer = NULL;
    float **outBuffer = NULL;

    error = wavIO_init(&hWavIO_source,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_dest,blocklength, 0, 0);

    audioIn = fopen(audioFilePathIn, "rb");
    audioOut = fopen(audioFilePathOut, "wb");

    error = wavIO_openRead(hWavIO_source,audioIn,&numChannelsSourceTotal,&InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);

    inBuffer = (float**)calloc(numChannelsSourceTotal,sizeof(float*));
    for (i = 0; i < numChannelsSourceTotal; i++)
    {
      inBuffer[i] = (float*)calloc(blocklength,sizeof(float));
    }

    error = wavIO_openWrite(hWavIO_dest,audioOut,numChannelsTargetTotal,InSampleRate,bytedepth);
    outBuffer = (float**)calloc(numChannelsTargetTotal,sizeof(float*));
    for (i = 0; i < numChannelsTargetTotal; i++)
    {
      outBuffer[i] = (float*)calloc(blocklength,sizeof(float));
    }

    do
    {
      unsigned int samplesReadPerChannel = 0;
      unsigned int samplesToWritePerChannel = 0;
      unsigned int samplesWrittenPerChannel = 0;
      unsigned int nZerosPaddedBeginning = 0;
      unsigned int nZerosPaddedEnd = 0;

      frameNo++;

      /* read frame if input file is available */
      if ( audioIn )
      {
        error = wavIO_readFrame(hWavIO_source,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
        for (j = 0; j < numChannelsTargetTotal; j++)
        {
          int zeroChannel = 1;
          CICP2GEOMETRY_CHANNEL_GEOMETRY outChannel = geoTarget[j];
          for (k = 0; k < numChannelsSourceTotal; k++)
          {
            CICP2GEOMETRY_CHANNEL_GEOMETRY inChannel = geoSource[k];
            if ((outChannel.Az == inChannel.Az) && (outChannel.El == inChannel.El) && (outChannel.LFE == inChannel.LFE))
            {
              zeroChannel = 0;
              break;
            }
          }
          for (i = 0; i < max(samplesReadPerChannel, blocklength); i++)
          {
            if ( zeroChannel == 1 )
            {
              outBuffer[j][i] = 0.0f;
            }
            else
            {
              outBuffer[j][i] = inBuffer[j][i];
            }
          }
        }
        /* Add up possible delay and actually read samples */
        samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
      }

      if ( audioOut )
      {
        /* write frame */
        error = wavIO_writeFrame(hWavIO_dest,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
        NumberSamplesWritten += samplesWrittenPerChannel;
      }
    }
    while (! isLastFrame);

    error = wavIO_updateWavHeader(hWavIO_dest, &nTotalSamplesWrittenPerChannel);

    error = wavIO_close(hWavIO_source);
    error = wavIO_close(hWavIO_dest);


    for (i = 0; i < numChannelsSourceTotal; i++) {
      free(inBuffer[i]);
    }
    free(inBuffer);

    for (i = 0; i < numChannelsTargetTotal; i++) {
      free(outBuffer[i]);
    }
    free(outBuffer);
  }
  else
  {
    
  }

  return error;
}

int MDPreproc_getSplitInformationForExcludedSectors(ASCPARSER_AUDIO_SCENE *audioSceneConfig, H_GROUP_SETUP_DATA groupSetupConfig, ASCPARSER_SIGNAL_GROUP_CONFIG *signalGroupConfig, int *splittingInfoType, int *splittingInfoSetup)
{
  int i, j, mae_ID;

  memset(splittingInfoType,-1,MAE_MAX_NUM_ELEMENTS*sizeof(int));
  memset(splittingInfoSetup,-1,MAE_MAX_NUM_ELEMENTS*sizeof(int));

  for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
  {
    int type = signalGroupConfig->signalGroupType[i];
    int numElements = audioSceneConfig->asi.groups[i].groupNumMembers;
    int setup = groupSetupConfig->groupSetupID[i];

    for (j = 0; j < numElements; j++)
    {
      if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 1)
      {
        mae_ID = audioSceneConfig->asi.groups[i].startID + j;
      }
      else
      {
        mae_ID = audioSceneConfig->asi.groups[i].metaDataElementID[j];
      }
      /* list is sorted by increasing mae_ID (same as overall output wave file) */
      splittingInfoType[mae_ID] = type;
      splittingInfoSetup[mae_ID] = setup;
    }
  }

  return 0;
}

int MPPreproc_write_geometry_to_file_knownPositions( char *outputFilename, CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[CICP2GEOMETRY_MAX_LOUDSPEAKERS], CICP2GEOMETRY_CHANNEL_GEOMETRY knownPositions[CICP2GEOMETRY_MAX_LOUDSPEAKERS], int numChannels, int numLFEs, CICP_FORMAT outputCicp )
{
  int error = 0;
  FILE* fileHandle;
  int numChannelsTotal = numChannels + numLFEs;
  int ch = 0;
  /* sanity check */
  if ( NULL ==  outGeo ) {
    error = -1;
    return error;
  }
  /* open file */
  fileHandle = fopen(outputFilename, "wb");
  if ( NULL == fileHandle ) {
    error = -2;
    return error;
  }

  if (outputCicp == CICP_FROM_GEO)
  {
    /* define number of entries */
    fprintf( fileHandle, "%i, -1, 1\n", numChannelsTotal );
    /* assume external distance comensation */

    for ( ch = 0; ch < numChannelsTotal; ++ch ) {
      /* write cicp speaker index and known position */
      if( outGeo[ch].cicpLoudspeakerIndex != -1 ) {
        fprintf( fileHandle, "c, %i, %i , %i , ,\n", outGeo[ch].cicpLoudspeakerIndex, knownPositions[ch].Az, knownPositions[ch].El );
      }
      else {
        /* write geometry info if necessary */
        fprintf( fileHandle, "g, %i, %i, %i, ,\n", outGeo[ch].Az, outGeo[ch].El, outGeo[ch].LFE );
      }
    }
  }
  else /* write cicp speaker layout index and kown positions */
  {
    /* define number of entries */
    fprintf( fileHandle, "%i, %i, 1\n", numChannelsTotal, outputCicp );
    /* assume external distance comensation */

    for ( ch = 0; ch < numChannelsTotal; ++ch ) {
      fprintf( fileHandle, "%i, %i, ,\n", knownPositions[ch].Az, knownPositions[ch].El );
    }
  }

  /* close file */
  if ( fclose(fileHandle) ) {
    error = -2;
  }

  return error;
}

int MPPreproc_write_geometry_to_file( char *outputFilename, CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[CICP2GEOMETRY_MAX_LOUDSPEAKERS], int numChannels, int numLFEs, const VERBOSE_LEVEL verboseLvl)
{
  int error = 0;
  FILE* fileHandle;
  int numChannelsTotal = numChannels + numLFEs;
  int ch = 0;

  /* sanity check */
  if ( NULL ==  outGeo ) {
    error = -1;
    return error;
  }

  /* open file */
  fileHandle = fopen(outputFilename, "wb");

  if ( NULL == fileHandle ) {
    error = -2;
    return error;
  }

  /* define number of entries */
  fprintf( fileHandle, "%i, -1, 1\n", numChannelsTotal );
  /* assume external distance comensation */

  for ( ch = 0; ch < numChannelsTotal; ++ch ) {
    /* write cicp speaker index if available */
    if( outGeo[ch].cicpLoudspeakerIndex != -1 ) {
      fprintf( fileHandle, "c, %i, , , ,\n", outGeo[ch].cicpLoudspeakerIndex );
    }
    else {
      /* write geometry info if necessary */
      fprintf( fileHandle, "g, %i, %i, %i, ,\n", outGeo[ch].Az, outGeo[ch].El, outGeo[ch].LFE );
    }
  }

  /* close file */
  if ( fclose(fileHandle) ) {
    error = -2;
  }

  return error;
}

int MDPreproc_prepareLocalSetupBitstream(char* geometry_file, char** localSetup_OutputFile, char* BinReadBRIRsFromLocation, int fDBinaural, int tDBinaural, int readBRIRsType, char *binary_LocalSetupInformationInterface, const char* logfile, const VERBOSE_LEVEL verboseLvl)
{
  char tmpString[FILENAME_MAX];
  char callCmd[5 * FILENAME_MAX];
  char outfile[FILENAME_MAX] = "tmpFile3Ddec_localSetupInformationInterface.bs";

  strncpy(*localSetup_OutputFile, outfile, FILENAME_MAX);

  if ((fDBinaural == 0) && (tDBinaural == 0))
  {
    /* LS Playback:
    Call localSetupInformationInterfaceExample
    /* -of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -speakers "geo_CICPindex_knownPos.txt" */
    sprintf ( callCmd, "%s -of %s -wire 0 -screen 0 -speakers %s", binary_LocalSetupInformationInterface, outfile, geometry_file);
  }
  else
  {
    if (readBRIRsType == 0) /* read from wav */
    {
      /* Binaural Playback:
      Call localSetupInformationInterfaceExample
      /* -of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsWav "C:\tmp\BRIR_Medium_IIS\IIS_BRIR_"  -geo "..\..\..\..\binaural\BRIRdata\geo_28_2.txt" */
      sprintf ( callCmd, "%s -of %s -wire 0 -screen 0 -brirsWav %s -geo %s", binary_LocalSetupInformationInterface, outfile, BinReadBRIRsFromLocation, geometry_file);
    }
    else
    {
      /* Binaural Playback:
      Call localSetupInformationInterfaceExample
      /* -of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsBitstream "..\..\..\..\binaural\BRIRdata\IIS_BRIR_FD_lowlevel.bs" */
      sprintf ( callCmd, "%s -of %s -wire 0 -screen 0 -brirsBitstream %s -geo %s", binary_LocalSetupInformationInterface, outfile, BinReadBRIRsFromLocation, geometry_file);
    }
  }

  sprintf ( tmpString, " >> %s 2>&1", logfile);
  strcat ( callCmd, tmpString );

  if (0 != callBinary(callCmd, "LocalSetupBitstreamWriter", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}

/* int MDPreproc_getLocalSetup(H_LOCAL_SETUP_DATA* localSetupConfig, CICP2GEOMETRY_CHANNEL_GEOMETRY* geometry, int numSpeakers, int numLFE, CICP_FORMAT outCicp)
{
  int error = 0;

  int i = 0;
  int numSp;
  H_LOCAL_SETUP_DATA temp = NULL;
  int anyKnownPos = 0;
  H_LOUDSPEAKER_RENDERING LR = NULL;
  H_BINAURAL_RENDERING binauralRendering_temp = NULL;

  temp = (H_LOCAL_SETUP_DATA)calloc(1, sizeof(struct _LocalSetupData));

  temp->dmxID = -1;

  temp->rendering_type = 0;
  temp->numWireOutputs = 0;
  LR = (H_LOUDSPEAKER_RENDERING)calloc(1, sizeof(struct _LoudspeakerRendering));
  LR->numSpeakers =  numSpeakers + numLFE;
  LR->externalDistanceCompensation = 0;
  LR->loudspeakerCalibrationGain = NULL;
  LR->loudspeakerDistance = NULL;

  LR->Setup_SpeakerConfig3D = (H_LOUDSPEAKER_SETUP)calloc(1, sizeof(struct _LoudspeakerSetup));

  if (outCicp == CICP_FROM_GEO)
  {
    LR->Setup_SpeakerConfig3D->speakerLayoutType = 2;
    LR->Setup_SpeakerConfig3D->numSpeakers = LR->numSpeakers;
    LR->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = LR->numSpeakers;
    LR->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = -1;

    LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig = (H_FLEXIBLE_SPEAKER_CONFIG)calloc(1,sizeof(struct _FlexibleSpeakerConfig));
    LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth = (float*)calloc(numSp,sizeof(float));
    LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation = (float*)calloc(numSp,sizeof(float));;
    LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE = (int*)calloc(numSp,sizeof(int));

    for (i = 0; i < LR->numSpeakers; i++)
    {
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Azimuth[i] = geometry[i].Az;
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->Elevation[i] = geometry[i].El;
      LR->Setup_SpeakerConfig3D->FlexibleSpeakerConfig->isLFE[i] = geometry[i].LFE;
    }

  }
  else
  {
    LR->Setup_SpeakerConfig3D->speakerLayoutType = 0;
    LR->Setup_SpeakerConfig3D->numSpeakers = LR->numSpeakers;
    LR->Setup_SpeakerConfig3D->numExplicitelySignaledSpeakers = 0;
    LR->Setup_SpeakerConfig3D->CICPspeakerLayoutIdx = outCicp;
  }

  temp->LocalScreenInfo = (H_LOCAL_SCREEN_INFO)calloc(1, sizeof(struct _LocalScreenInfo));
  temp->LocalScreenInfo->hasLocalScreenInfo = 0;
  temp->LocalScreenInfo->az_left = 0;
  temp->LocalScreenInfo->az_right = 0;
  temp->LocalScreenInfo->hasElevationInfo = 0;
  temp->LocalScreenInfo->el_bottom = 0;
  temp->LocalScreenInfo->el_top = 0;

  temp->LoudspeakerRendering = LR;
  *localSetupConfig = temp;

  return error;
} */

int MPPreproc_write_CICP_to_file( char* outputFilename, int CICPIndex, int numChannels, int numLFEs )
{
  int error = 0;
  FILE *fileHandle;
  int numChannelsTotal = numChannels + numLFEs;

  if (numChannelsTotal == 0)
  {
    CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[MAE_MAX_NUM_SPEAKERS];
    error = cicp2geometry_get_geometry_from_cicp(CICPIndex, outGeo, &numChannels, &numLFEs);
    numChannelsTotal = numChannels + numLFEs;
  }

  fileHandle = fopen(outputFilename, "wb");
  if ( NULL == fileHandle ) {
    error = -1;
    return error;
  }

  /* define number of entries */
  fprintf( fileHandle, "%i, %i, 1\n", numChannelsTotal, CICPIndex );
  /* assumes external distance compensation */

  if ( fclose(fileHandle) ) {
    error = -1;
  }

  return error;
}

int MDPreprocPath_Execute(char* wavIO_inpath,
                          char* wavIO_outpath,
                          char* oamInpath,
                          char* oamOutpath,
                          ASCPARSERINFO* asc,
                          int decodedOAM,
                          ASCPARSER_AUDIO_SCENE* audioSceneConfig,
                          char* elementInteraction_InputFile,
                          char* localSetup_InputFile,
                          int moduleProcessingDomain,
                          int numOutChannels,
                          int numOutLFEs,
                          char* fileOutGeo,
                          int downmixId,
                          ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig,
                          int *numAddedObjects,
                          char* BinReadBRIRsFromLocation,
                          int fDBinaural,
                          int tDBinaural,
                          int readBRIRsType,
                          char* pathLocalSetupInformationInterface,
                          int *separateSetups,
                          int* splittingInfoType,
                          int* splittingInfoSetup,
                          int* enableDiffusenessRendering,
                          int sceneDsplInterfaceEnabled,
                          char* sceneDisplacement_InputFile,
                          char* sdChannels_oamOutpath,
                          char* sdChannels_indexListOutpath,
                          int* hasSDchannels,
                          ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo,
                          ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig,
                          int signalGroupInfoAvailable,
                          int writeToObjectOutputInterface,
                          int chooseSGmember,
                          int selectedPresetID,
                          DRC_PARAMS* drcParams,
                          const char* logfile,
                          const PROFILE profile,
                          const VERBOSE_LEVEL verboseLvl
) {

  H_AUDIO_MP_CONFIG h_AudioConfig = NULL;
  H_OAM_MP_CONFIG h_OamConfig = NULL;
  H_INTERACT_MP_CONFIG h_InteractConfig = NULL;
  H_EI_BITSTREAM_DECODER h_bitstream_EI_Dec = NULL;
  ELEMENT_INTERACTION_HANDLE h_EI_Dec = NULL;
  H_LS_BITSTREAM_DECODER h_bitstream_LS_Dec = NULL;
  LOCAL_SETUP_HANDLE h_LS_Dec = NULL;
  H_LOCAL_SETUP_DATA localSetupConfig = NULL;

  char wavIO_inpath_real[FILENAME_MAX];
  char wavIO_inpath_imag[FILENAME_MAX];
  char wavIO_outpath_real[FILENAME_MAX];
  char wavIO_outpath_imag[FILENAME_MAX];
  char wavIO_outpathDiffuse_real[FILENAME_MAX];
  char wavIO_outpathDiffuse_imag[FILENAME_MAX];
  char wavIO_outpathDiffuse[FILENAME_MAX];

  H_GROUP_SETUP_DATA groupSetupConfig = NULL;
  char groupSetup_outpath[FILENAME_MAX] = "tmpFile3Ddec_separate_output_setup";

  /* scene displacement information */
  H_SD_BITSTREAM_DECODER h_bitstream_SD_Dec = NULL;
  SCENE_DISPLACEMENT_HANDLE h_SD_Dec = NULL;

  unsigned long no_BitsRead = 0;
  int numFrames = 0;
  int f = 0;
  int error = 0;
  int temp = 0;

  int enableObjectOutputInterface = writeToObjectOutputInterface;
  char OoiOutpath[FILENAME_MAX] = "tmpFile3Ddec_object_output_interface_data";
  H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig = NULL;

  char enhancedObjectMetadataFramePath[FILENAME_MAX] = "tmpFile3Ddec_enhObjMdFrame";
  H_ENH_OBJ_MD_FRAME h_enhObjMdFrame = NULL;
  int enhObjMdFrameIsPresent = 0;

  sprintf(wavIO_outpathDiffuse,"%s_diffuse",wavIO_outpath);

  if (moduleProcessingDomain == 0)
  {
    strcat( wavIO_outpath, ".wav");
    strcat( wavIO_outpathDiffuse, ".wav");
  }
  else
  {
    strncpy(wavIO_inpath_real, wavIO_inpath, FILENAME_MAX);
    strcat(wavIO_inpath_real, "_real.qmf" );
    strncpy(wavIO_inpath_imag, wavIO_inpath, FILENAME_MAX);
    strcat(wavIO_inpath_imag, "_imag.qmf" );
    strncpy(wavIO_outpath_real, wavIO_outpath, FILENAME_MAX);
    strcat(wavIO_outpath_real, "_real.qmf" );
    strncpy(wavIO_outpath_imag, wavIO_outpath, FILENAME_MAX);
    strcat(wavIO_outpath_imag, "_imag.qmf" );

    strncpy(wavIO_outpathDiffuse_real, wavIO_outpathDiffuse, FILENAME_MAX);
    strcat(wavIO_outpathDiffuse_real, "_real.qmf" );
    strncpy(wavIO_outpathDiffuse_imag, wavIO_outpathDiffuse, FILENAME_MAX);
    strcat(wavIO_outpathDiffuse_imag, "_imag.qmf" );
  }

  /* INITIALIZATION */

  /********************** Re-assign elementIDs to fit to MDPreprocessor assumptions (1 ID per Core Coder output track) **********************/
  {
    int i;
    int m;
    int elementIdList_asASI[MAE_MAX_NUM_ELEMENTS];
    int elementIdList_MDpreproc[MAE_MAX_NUM_ELEMENTS];
    int ct = 0;


    /* assign number of HOA transport channels as members to HOA ASI group */
    for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
    {
      if (MP_getSignalGroupType(audioSceneConfig, signalGroupConfig, i) == ASCPARSER_SIGNAL_GROUP_TYPE_HOA)
      {
        /* assumption: complete HOA signal group as HOA mae_group */
        int signalGroupIndex = MP_getSignalGroupIndex(audioSceneConfig, signalGroupConfig, i, 0);
        audioSceneConfig->asi.groups[i].groupNumMembers = signalGroupConfig->numberOfSignals[signalGroupIndex];
        if (audioSceneConfig->asi.groups[i].hasConjunctMembers == 0)
        {
          for (m = 0; m < audioSceneConfig->asi.groups[i].groupNumMembers; m++)
          {
            audioSceneConfig->asi.groups[i].metaDataElementID[m] = audioSceneConfig->asi.groups[i].startID + m;
          }
        }
      }
    }

    /* avoid problems due to "gaps" in the numbering */
    for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
    {
      int nMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      for (m = 0; m < nMembers; m++)
      {
        int elementID = 0;
        if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
        {
          elementID = audioSceneConfig->asi.groups[i].startID + m;
        }
        else
        {
          elementID = audioSceneConfig->asi.groups[i].metaDataElementID[m];
        }
        elementIdList_asASI[ct] = elementID;
        ct++;
      }
    }

    for (i = 0; i < ct; i++)
    {
      elementIdList_MDpreproc[i] = i;
    }

    ct = 0;
    for (i = 0; i < audioSceneConfig->asi.numGroups; i++)
    {
      int nMembers = audioSceneConfig->asi.groups[i].groupNumMembers;
      for (m = 0; m < nMembers; m++)
      {
        int elementID = 0;
        if (audioSceneConfig->asi.groups[i].hasConjunctMembers)
        {
          if (m == 0)
          {
            audioSceneConfig->asi.groups[i].startID = elementIdList_MDpreproc[ct];
          }
        }
        else
        {
          audioSceneConfig->asi.groups[i].metaDataElementID[m] = elementIdList_MDpreproc[ct];
        }
        ct++;
      }
    }
  }
  /********************** end of ID re-assignment **********************/

  /* init audio config */
  MP_initAudioConfig(&h_AudioConfig, wavIO_inpath, wavIO_inpath_real, wavIO_inpath_real, wavIO_outpath, wavIO_outpath_real, wavIO_outpath_imag, wavIO_outpathDiffuse_real, wavIO_outpathDiffuse_imag, wavIO_outpathDiffuse);
  MP_initOamConfig(&h_OamConfig, oamInpath, oamOutpath, asc->oam_mode, decodedOAM, asc->oam_hasDynamicObjectPriority, asc->oam_hasUniformSpread);

  /* init metadata preprocessor interaction config */
  MP_initInteractConfig(&h_InteractConfig, audioSceneConfig);

  /* init element interaction interface */
  error = EI_initHandle(&h_EI_Dec);
  error = EI_DecInt_initBitstreamReader(&h_bitstream_EI_Dec, elementInteraction_InputFile);


  /***********************************************************************/
  /* READ LOCAL SETUP DATA */

  if (localSetup_InputFile[0] == '\0')
  {
    /* create interface data from given CICPindex / geo file */
    error = MDPreproc_prepareLocalSetupBitstream(fileOutGeo, &localSetup_InputFile,  BinReadBRIRsFromLocation, fDBinaural, tDBinaural, readBRIRsType, pathLocalSetupInformationInterface, logfile, verboseLvl);
    /* init local setup information interface */
    error = LS_initHandle(&h_LS_Dec);
    error = LS_DecInt_initBitstreamReader(&h_bitstream_LS_Dec, localSetup_InputFile);
  }
  else
  {
    /* init local setup information interface */
    error = LS_initHandle(&h_LS_Dec);
    error = LS_DecInt_initBitstreamReader(&h_bitstream_LS_Dec, localSetup_InputFile);
  }

  /* read local setup data */
  error = LS_readLocalSetupBitstream(h_LS_Dec, h_bitstream_LS_Dec, &no_BitsRead);
  error = LS_getLocalSetupData(h_LS_Dec, &localSetupConfig);

  localSetupConfig->dmxID = downmixId;

  /* init reading of .dat files for enhanced object metadata */
  error = MP_initEnhancedObjectMetadataFrame(&h_enhObjMdFrame, enhancedObjectMetadataFramePath);

  /* read first EnhancedObjectMetadataFrame() */
  error = MP_decodeEnhancedObjectMetadataFrame(h_enhObjMdFrame, asc->objects, enhancedObjectMetadataConfig, 0, &enhObjMdFrameIsPresent);

  /* init reading and writing of audio and OAM data */
  temp = MP_initAudioAndOamReadWrite(h_AudioConfig, h_OamConfig, asc->oam_length, moduleProcessingDomain, asc->outputFrameLength, audioSceneConfig, signalGroupConfig, localSetupConfig, enhancedObjectMetadataConfig, enableObjectOutputInterface, profile);
  if (temp > -1)
  {
    *numAddedObjects = temp;
  }
  else
  {
    error = temp;
  }

  /* init writing of object output interface data */
  if ((enableObjectOutputInterface == 1) && (moduleProcessingDomain == 0))
  {
    error = MP_initObjectOutputInterfaceWrite(&h_OoiConfig, OoiOutpath, h_AudioConfig->audio_blocksize, h_AudioConfig->InBytedepth, h_AudioConfig->InSampleRate);
  }

  /* init scene displacement interface + file writing */
  if ( sceneDsplInterfaceEnabled & eSceneDsplRotational )
  {
    error = SD_initHandle(&h_SD_Dec);
    error = SD_DecInt_initBitstreamReader(&h_bitstream_SD_Dec, sceneDisplacement_InputFile);

    /* Positional scene displacement interface */
    PSDI_enabled(h_SD_Dec, sceneDsplInterfaceEnabled & eSceneDsplPositional);

    error = MP_SD_initFileWriting(signalGroupInfo,
                                  h_OamConfig,
                                  h_AudioConfig,
                                  audioSceneConfig,
                                  signalGroupConfig,
                                  h_InteractConfig,
                                  sdChannels_oamOutpath,
                                  sdChannels_indexListOutpath,
                                  localSetupConfig);
  }

  /***********************************************************************/
  /* FRAMEWISE PROCESSING, READ AUDIO + OAM + ELEMENT INTERACTION DATA */

  numFrames = getNumAudioFrames(h_AudioConfig);

  if (VERBOSE_LVL1 <= verboseLvl) {
    fprintf ( stdout, "::-----------------------------------------------------------------------------\n::    " );
    fprintf ( stdout, "Metadata Preprocessing...\n");
    fprintf ( stdout, "::-----------------------------------------------------------------------------\n\n" );
  }

  for (f = 0; f < numFrames; f++)
  {
    int isLastFrame = !((numFrames - 1) - f);
    /* fprintf(stderr, "FrameNo %d\n", f); */

    if (f == 0)
    {
      /***********************************************************************/
      /*  WRITE INDIVIDUAL SETUP FILES (BASED ON EXCLUDED SECTORS) FOR OBJECT RENDERER */
      /* current assumption: exclude sectors are fixed over time (keep mechanism), therefore
      use enhancedObjectMetadataFrame() in the first audio frame (read before the loop over frames) to set the group-dependent target setups */
      if (localSetupConfig->rendering_type == 0)
      {
        error = MP_getGroupSetupData(audioSceneConfig, &groupSetupConfig, signalGroupConfig, localSetupConfig, enhancedObjectMetadataConfig, h_enhObjMdFrame, enhObjMdFrameIsPresent);
        if (groupSetupConfig->doUseIndividualGroupSetups == 1)
        {
          /* write txt files with geometric setup information (for signal groups with excluded sectors only) */
          error = MP_writeGroupSetupData(groupSetupConfig, groupSetup_outpath);
        }
      }
      /***********************************************************************/
      /*  INIT DIFFUSENESS RENDERING */
      /* get groupwise diffuseness gain values + init decorrelation filters */
      if (!enableObjectOutputInterface)
      {
        error = MP_initDiffusenessRendering(h_InteractConfig, h_AudioConfig, audioSceneConfig, signalGroupConfig, localSetupConfig, moduleProcessingDomain, enhancedObjectMetadataConfig);
      }
    }

    /* read audio data */
    error = MP_readAudioFrame(h_AudioConfig);

    /* process frame */
    error = MP_processFrame(signalGroupInfo,
                            h_AudioConfig,
                            enhancedObjectMetadataConfig,
                            h_enhObjMdFrame,
                            h_OamConfig,
                            audioSceneConfig,
                            signalGroupConfig,
                            localSetupConfig,
                            h_InteractConfig,
                            h_OoiConfig,
                            h_EI_Dec,
                            h_bitstream_EI_Dec,
                            h_SD_Dec,
                            h_bitstream_SD_Dec,
                            groupSetupConfig,
                            isLastFrame,
                            f,
                            enableObjectOutputInterface,
                            chooseSGmember,
                            &selectedPresetID);

    if (f == 0)
    {
      /***********************************************************************/
      /* write data needed for DRC processing to DRC params struct */
      int temp;
      int numGroupIdsRequested = 0;
      int groupIdRequested[MAE_MAX_NUM_GROUPS];
      int groupPresetIdRequested = -1;

      error = MP_getDrcParamData(h_InteractConfig, audioSceneConfig, &numGroupIdsRequested, groupIdRequested, &groupPresetIdRequested, selectedPresetID);

      if (error == 0)
      {
        drcParams->numGroupIdsRequested = numGroupIdsRequested;
        for (temp = 0; temp < numGroupIdsRequested; temp++)
        {
          drcParams->groupIdRequested[temp] = groupIdRequested[temp];
        }

        if (groupPresetIdRequested != -1) {
          drcParams->numGroupPresetIdsRequested = 1;
          drcParams->groupPresetIdRequested[0] = groupPresetIdRequested;
        }
        drcParams->groupPresetIdRequestedPreference = groupPresetIdRequested;
      }
    }

    if (h_InteractConfig->ElementInteractionData != NULL)
    {
      EI_freeElementInteractionData(h_InteractConfig->ElementInteractionData);
      h_InteractConfig->ElementInteractionData = NULL;
    }
  }

  /* close reading of enhanced metadata frame */
  error = MP_closeEnhancedObjectMetadataFrame(h_enhObjMdFrame);

  /* close object output interface */
  if ((enableObjectOutputInterface == 1) && (moduleProcessingDomain == 0))
  {
    error = MP_closeObjectOutputInterfaceWrite(h_OoiConfig);
  }

  /* close reading of decoded audio files and oam files */
  if (h_OamConfig->oam_hasBeenDecoded == 1)
  {
    oam_read_close(h_OamConfig->oamInFile);
    oam_write_close(h_OamConfig->oamOutFile);
    h_OamConfig->oamSample_in = oam_multidata_destroy(h_OamConfig->oamSample_in);
  }
  h_OamConfig->oamSample_in = NULL;

  if (h_OamConfig->num_SDchannels > 0)
  {
    oam_write_close(h_OamConfig->oamOutFile_SDchannels);
    h_OamConfig->oamSample_SDchannels_out = oam_multidata_destroy(h_OamConfig->oamSample_SDchannels_out);
    h_OamConfig->oamSample_SDchannels_out = NULL;
  }

  if (moduleProcessingDomain == 1)
  {
    error = wavIO_updateWavHeader(h_AudioConfig->hWavIO_real, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
    error = wavIO_updateWavHeader(h_AudioConfig->hWavIO_imag, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
    error = wavIO_close(h_AudioConfig->hWavIO_real);
    error = wavIO_close(h_AudioConfig->hWavIO_imag);
    if (h_InteractConfig->diffuse_enableProcessing == 1)
    {
      error = wavIO_updateWavHeader(h_AudioConfig->hWavIODiffuse_real, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
      error = wavIO_updateWavHeader(h_AudioConfig->hWavIODiffuse_imag, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
      error = wavIO_close(h_AudioConfig->hWavIODiffuse_real);
      error = wavIO_close(h_AudioConfig->hWavIODiffuse_imag);
    }
  }
  else if (moduleProcessingDomain == 0)
  {
    error = wavIO_updateWavHeader(h_AudioConfig->hWavIO, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
    error = wavIO_close(h_AudioConfig->hWavIO);
    if (h_InteractConfig->diffuse_enableProcessing == 1)
    {
      error = wavIO_updateWavHeader(h_AudioConfig->hWavIODiffuse, &h_AudioConfig->nTotalSamplesWrittenPerChannel);
      error = wavIO_close(h_AudioConfig->hWavIODiffuse);
    }
  }

  /***********************************************************************/
  /*  SPLIT UP WAVE FILE AND OAM FILES ACCORDING TO GROUP SETUPS */

  if (localSetupConfig->rendering_type == 0)
  {
    if (groupSetupConfig->doUseIndividualGroupSetups == 1)
    {
      int channelOffset = 0;

      /*  SPLIT UP WAVE FILE AND OAM FILES ACCORDING TO GROUP SETUPS */
      error = MP_splitForExcludedSectors(h_AudioConfig->wavIO_outpath, h_OamConfig->oamOutpath, groupSetupConfig, h_AudioConfig, h_OamConfig, audioSceneConfig, signalGroupConfig);

      /* determine splitting information for the different elements */
      error = MDPreproc_getSplitInformationForExcludedSectors(audioSceneConfig, groupSetupConfig, signalGroupConfig, splittingInfoType, splittingInfoSetup);

    }
    error = MP_freeGroupSetupConfig(groupSetupConfig);
  }

  /***********************************************************************/
  /*  CLOSING AND CLEANUP */

  /* close local setup interface */
  if (localSetup_InputFile != NULL)
  {
    error = LS_closeHandle(h_LS_Dec);
    LS_DecInt_closeBitstreamReader(h_bitstream_LS_Dec);
  }
  error = LS_freeLocalSetupData(localSetupConfig);

  /* close interaction interface */
  EI_DecInt_closeBitstreamReader(h_bitstream_EI_Dec);
  error = EI_closeHandle(h_EI_Dec);

  *enableDiffusenessRendering = h_InteractConfig->diffuse_enableProcessing;
  *hasSDchannels = h_OamConfig->num_SDchannels;

  error = MP_freeConfigStructs(h_AudioConfig, h_InteractConfig, h_OamConfig);

  return error;
}

int MDPreproc_combineWav(char* destination,
                         char *source,
                         char* addition,
                         int processingDomain
) {
  int error = 0;
  int nOutChannels;

  unsigned int nInChannels = 0;
  unsigned int nInChannels2 = 0;
  unsigned int InSampleRate;
  unsigned long nTotalSamplesPerChannel;
  unsigned long nTotalSamplesPerChannel2;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  int nSamplesPerChannelFilled;
  unsigned int bytedepth = 0;

  int i, j;
  unsigned int ui;
  unsigned int blocklength = 4096;
  int frameNo = 0;
  unsigned int isLastFrame = 0;
  int NumberSamplesWritten = 0;

  if (processingDomain == 0)
  {

    WAVIO_HANDLE hWavIO_dest = NULL;
    WAVIO_HANDLE hWavIO_source = NULL;
    WAVIO_HANDLE hWavIO_add = NULL;
    FILE *fIn_source = NULL;
    FILE *fIn_add = NULL;
    FILE *fOut_dest = NULL;
    float** inBuffer_source = NULL;
    float** inBuffer_add = NULL;
    float** outBuffer = NULL;

    error = wavIO_init(&hWavIO_source,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_add,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_dest,blocklength, 0, 0);

    fIn_source = fopen(source, "rb");
    fIn_add = fopen(addition, "rb");
    fOut_dest = fopen(destination, "wb");

    error = wavIO_openRead(hWavIO_source, fIn_source, &nInChannels, &InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    inBuffer_source = (float**)calloc(nInChannels,sizeof(float*));
    for (ui = 0; ui < nInChannels; ui++)
    {
      inBuffer_source[ui] = (float*)calloc(blocklength,sizeof(float));
    }

    error = wavIO_openRead(hWavIO_add, fIn_add, &nInChannels2, &InSampleRate, &bytedepth, &nTotalSamplesPerChannel2, &nSamplesPerChannelFilled);
    inBuffer_add = (float**)calloc(nInChannels2,sizeof(float*));
    for (ui = 0; ui < nInChannels2; ui++)
    {
      inBuffer_add[ui] = (float*)calloc(blocklength,sizeof(float));
    }

    nOutChannels = nInChannels + nInChannels2;

    error = wavIO_openWrite(hWavIO_dest, fOut_dest, nOutChannels, InSampleRate, bytedepth);
    outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
    for (i = 0; i < nOutChannels; i++)
    {
      outBuffer[i] = (float*)calloc(blocklength,sizeof(float));
    }

    do
    {
      unsigned int samplesReadPerChannel = 0;
      unsigned int samplesToWritePerChannel = 0;
      unsigned int samplesWrittenPerChannel = 0;
      unsigned int nZerosPaddedBeginning = 0;
      unsigned int nZerosPaddedEnd = 0;

      frameNo++;

      /* read frame if input file is available */
      if ( fIn_source && fIn_add )
      {
        error = wavIO_readFrame(hWavIO_source,inBuffer_source,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
        error = wavIO_readFrame(hWavIO_add,inBuffer_add,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);

        for (ui = 0; ui < max(samplesReadPerChannel, blocklength); ui++)
        {
          for (j = 0; j < nOutChannels; j++)
          {
            if ((unsigned int)j < nInChannels )
            {
              outBuffer[j][ui] = inBuffer_source[j][ui];
            }
            else
            {
              outBuffer[j][ui] = inBuffer_add[j-nInChannels][ui];
            }
          }
        }

        /* Add up possible delay and actually read samples */
        samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
      }

      if ( fOut_dest )
      {
        /* write frame */
        error = wavIO_writeFrame(hWavIO_dest,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
        NumberSamplesWritten += samplesWrittenPerChannel;
      }
    }
    while (! isLastFrame);

    error = wavIO_updateWavHeader(hWavIO_dest, &nTotalSamplesWrittenPerChannel);

    error = wavIO_close(hWavIO_source);
    error = wavIO_close(hWavIO_add);
    error = wavIO_close(hWavIO_dest);


    for (ui = 0; ui < nInChannels; ui++) {
      free(inBuffer_source[ui]);
      inBuffer_source[ui] = NULL;
    }
    free(inBuffer_source);
    inBuffer_source =  NULL;

    for (ui = 0; ui < nInChannels2; ui++) {
      free(inBuffer_add[ui]);
      inBuffer_add[ui] = NULL;
    }
    free(inBuffer_add);
    inBuffer_add = NULL;


    for (i = 0; i < nOutChannels; i++) {
      free(outBuffer[i]);
    }
    free(outBuffer);

  }
  else /* QMF domain */
  {

    WAVIO_HANDLE hWavIO_dest_real = NULL;
    WAVIO_HANDLE hWavIO_dest_imag = NULL;
    WAVIO_HANDLE hWavIO_source_real = NULL;
    WAVIO_HANDLE hWavIO_source_imag = NULL;
    WAVIO_HANDLE hWavIO_add_real = NULL;
    WAVIO_HANDLE hWavIO_add_imag = NULL;
    FILE *fIn_source_real = NULL;
    FILE *fIn_add_real = NULL;
    FILE *fOut_dest_real = NULL;
    FILE *fIn_source_imag = NULL;
    FILE *fIn_add_imag = NULL;
    FILE *fOut_dest_imag = NULL;

    float **inBuffer_source_real = NULL;
    float **inBuffer_add_real = NULL;
    float **outBuffer_real = NULL;
    float **inBuffer_source_imag = NULL;
    float **inBuffer_add_imag = NULL;
    float **outBuffer_imag = NULL;

    char path_dest_real[FILENAME_MAX];
    char path_dest_imag[FILENAME_MAX];
    char path_add_real[FILENAME_MAX];
    char path_add_imag[FILENAME_MAX];
    char path_source_real[FILENAME_MAX];
    char path_source_imag[FILENAME_MAX];

    error = wavIO_init(&hWavIO_dest_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_dest_imag,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_add_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_add_imag,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_source_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_source_imag,blocklength, 0, 0);

    strncpy(path_dest_real, destination, FILENAME_MAX);
    strncpy(path_dest_imag, destination, FILENAME_MAX);
    strncpy(path_add_real, addition, FILENAME_MAX);
    strncpy(path_add_imag, addition, FILENAME_MAX);
    strncpy(path_source_real, source, FILENAME_MAX);
    strncpy(path_source_imag, source, FILENAME_MAX);

    strcat(path_dest_real, "_real.qmf");
    strcpy(path_dest_imag, "_imag.qmf");
    strcpy(path_add_real, "_real.qmf");
    strcpy(path_add_imag, "_imag.qmf");
    strcpy(path_source_real, "_real.qmf");
    strcpy(path_source_imag, "_imag.qmf");

    error = wavIO_init(&hWavIO_source_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_add_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_dest_real,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_source_imag,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_add_imag,blocklength, 0, 0);
    error = wavIO_init(&hWavIO_dest_imag,blocklength, 0, 0);

    fIn_source_real = fopen(path_source_real, "rb");
    fIn_add_real = fopen(path_add_real, "rb");
    fOut_dest_real = fopen(path_dest_real, "wb");
    fIn_source_imag = fopen(path_source_imag, "rb");
    fIn_add_imag = fopen(path_add_imag, "rb");
    fOut_dest_imag = fopen(path_dest_imag, "wb");

    error = wavIO_openRead(hWavIO_source_real,fIn_source_real,&nInChannels,&InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    inBuffer_source_real = (float**)calloc(nInChannels,sizeof(float*));
    for (ui = 0; ui < nInChannels; ui++)
    {
      inBuffer_source_real[ui] = (float*)calloc(blocklength,sizeof(float));
    }
    error = wavIO_openRead(hWavIO_source_imag,fIn_source_imag,&nInChannels,&InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    inBuffer_source_imag = (float**)calloc(nInChannels,sizeof(float*));
    for (ui = 0; ui < nInChannels; ui++)
    {
      inBuffer_source_imag[ui] = (float*)calloc(blocklength,sizeof(float));
    }

    error = wavIO_openRead(hWavIO_add_real,fIn_add_real,&nInChannels2,&InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    inBuffer_add_real = (float**)calloc(nInChannels2,sizeof(float*));
    for (ui = 0; ui < nInChannels2; ui++)
    {
      inBuffer_add_real[ui] = (float*)calloc(blocklength,sizeof(float));
    }
    error = wavIO_openRead(hWavIO_add_imag,fIn_add_imag,&nInChannels2,&InSampleRate, &bytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    inBuffer_add_imag = (float**)calloc(nInChannels2,sizeof(float*));
    for (ui = 0; ui < nInChannels2; ui++)
    {
      inBuffer_add_imag[ui] = (float*)calloc(blocklength,sizeof(float));
    }

    nOutChannels = nInChannels + nInChannels2;

    error = wavIO_openWrite(hWavIO_dest_real,fOut_dest_real,nOutChannels,InSampleRate,bytedepth);
    outBuffer_real = (float**)calloc(nOutChannels,sizeof(float*));
    for (i=0; i< nOutChannels; i++)
    {
      outBuffer_real[i] = (float*)calloc(blocklength,sizeof(float));
    }
    error = wavIO_openWrite(hWavIO_dest_imag,fOut_dest_imag,nOutChannels,InSampleRate,bytedepth);
    outBuffer_imag = (float**)calloc(nOutChannels,sizeof(float*));
    for (i=0; i< nOutChannels; i++)
    {
      outBuffer_imag[i] = (float*)calloc(blocklength,sizeof(float));
    }

    do
    {
      unsigned int samplesReadPerChannel = 0;
      unsigned int samplesToWritePerChannel = 0;
      unsigned int samplesWrittenPerChannel = 0;
      unsigned int nZerosPaddedBeginning = 0;
      unsigned int nZerosPaddedEnd = 0;

      frameNo++;

      /* read frame if input file is available */
      if ( fIn_source_real && fIn_source_imag && fIn_add_real && fIn_add_imag )
      {
        error = wavIO_readFrame(hWavIO_source_real,inBuffer_source_real,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
        error = wavIO_readFrame(hWavIO_add_real,inBuffer_add_real,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
        error = wavIO_readFrame(hWavIO_source_imag,inBuffer_source_imag,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
        error = wavIO_readFrame(hWavIO_add_imag,inBuffer_add_imag,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);


        for (ui = 0; ui < max(samplesReadPerChannel, blocklength); ui++)
        {
          for (j = 0; j < nOutChannels; j++)
          {
            if ((unsigned int)j < nInChannels )
            {
              outBuffer_real[j][ui] = inBuffer_source_real[j][ui];
              outBuffer_imag[j][ui] = inBuffer_source_imag[j][ui];
            }
            else
            {
              outBuffer_real[j][ui] = inBuffer_add_real[j-nInChannels][ui];
              outBuffer_imag[j][ui] = inBuffer_add_imag[j-nInChannels][ui];
            }
          }
        }

        /* Add up possible delay and actually read samples */
        samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
      }

      if ( fOut_dest_real && fOut_dest_imag )
      {
        /* write frame */
        error = wavIO_writeFrame(hWavIO_dest_real,outBuffer_real,samplesToWritePerChannel,&samplesWrittenPerChannel);
        error = wavIO_writeFrame(hWavIO_dest_imag,outBuffer_imag,samplesToWritePerChannel,&samplesWrittenPerChannel);
        NumberSamplesWritten += samplesWrittenPerChannel;
      }
    }
    while (! isLastFrame);

    error = wavIO_updateWavHeader(hWavIO_dest_real, &nTotalSamplesWrittenPerChannel);
    error = wavIO_updateWavHeader(hWavIO_dest_imag, &nTotalSamplesWrittenPerChannel);

    error = wavIO_close(hWavIO_source_real);
    error = wavIO_close(hWavIO_add_real);
    error = wavIO_close(hWavIO_dest_real);
    error = wavIO_close(hWavIO_source_imag);
    error = wavIO_close(hWavIO_add_imag);
    error = wavIO_close(hWavIO_dest_imag);


    for (ui = 0; ui < nInChannels; ui++) {
      free(inBuffer_source_real[ui]);
      free(inBuffer_source_imag[ui]);
    }
    free(inBuffer_source_real);
    free(inBuffer_source_imag);

    for (ui = 0; ui < nInChannels2; ui++) {
      free(inBuffer_add_real[ui]);
      free(inBuffer_add_imag[ui]);
    }
    free(inBuffer_add_real);
    free(inBuffer_add_imag);

    for (i=0; i< nOutChannels; i++) {
      free(outBuffer_real[i]);
      free(outBuffer_imag[i]);
    }
    free(outBuffer_real);
    free(outBuffer_imag);
  }

  return error;
}
