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
#include <stdlib.h>
#include <string.h>

#include "separatorlib.h"

int main(int argc, char *argv[])
{
  FILE *fIn = NULL;
  FILE *fQmfInFileReal = NULL;
  FILE *fQmfInFileImag = NULL;

  FILE *fOutChan = NULL;
  FILE *fQmfOutChanReal = NULL;
  FILE *fQmfOutChanImag = NULL;

  FILE *fOutObj[SEPARATOR_MAX_NUM_OBJECTS] = { 0 };
  FILE *fQmfOutObjReal[SEPARATOR_MAX_NUM_OBJECTS] = { 0 };
  FILE *fQmfOutObjImag[SEPARATOR_MAX_NUM_OBJECTS] = { 0 };


  char *inpath = NULL;
  char qmfInFileReal[200];
  char qmfInFileImag[200];

  char *outpath1 = NULL;
  char qmfOutChanReal[200];
  char qmfOutChanImag[200];

  char *outpath2 = NULL;
  char qmfOutObjReal[200];
  char qmfOutObjImag[200];

  char* wavExtension  = ".wav";

  int bQmfInput = 0;
  int bQmfOutChan = 0;
  int bQmfOutObj = 0;

  unsigned int nObjects = 0;
  int nChannelsOut = 0;
  int nChannelOffset = 0;

  unsigned int i = 0;
  int error = 0;
  unsigned int isLastFrame = 0;
  unsigned int isLastImagFrame = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  unsigned int frameCnt = 0;

  SEPARATOR_HANDLE hSeparator = NULL;
  SEPARATOR_HANDLE hImagSeparator = NULL;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                             Ch-Obj Separator Module                         *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
  
  /* Commandline Parsing */
  for ( i = 1; i < (unsigned int) argc; ++i )
  {
    int nWavExtensionChars = 0;

    if (!strcmp(argv[i],"-if"))
    {
      inpath = argv[i+1];
      nWavExtensionChars = 0;
      nWavExtensionChars = strspn(wavExtension,inpath);
      if ( nWavExtensionChars != 4 ) {
        bQmfInput = 1;
        strcpy(qmfInFileReal, inpath);
        strcpy(qmfInFileImag, inpath);
        strcat(qmfInFileReal, "_real.qmf");
        strcat(qmfInFileImag, "_imag.qmf");
      }
      i++;
    }
    
    if (!strcmp(argv[i],"-of"))
    {
      outpath1 = argv[i+1];
      nWavExtensionChars = 0;
      nWavExtensionChars = strspn(wavExtension,outpath1);
      if ( nWavExtensionChars != 4 ) {
        bQmfOutChan = 1;
        strcpy(qmfOutChanReal, outpath1);
        strcpy(qmfOutChanImag, outpath1);
        strcat(qmfOutChanReal, "_real.qmf");
        strcat(qmfOutChanImag, "_imag.qmf");
      }
      i++;
    }
    
    else if (!strcmp(argv[i],"-obj"))
    {
      outpath2 = argv[i+1];
      strcpy(qmfOutObjReal, outpath2);
      strcpy(qmfOutObjImag, outpath2);
      i++;
    } 
    else if (!strcmp(argv[i],"-nObj")) 
    {
      nObjects = atoi(argv[i+1]);
      i++;
    }     
    else if (!strcmp(argv[i],"-nCh")) 
    {
      nChannelsOut = atoi(argv[i+1]);
      i++;
    }     
    else if (!strcmp(argv[i],"-nOff")) 
    {
      nChannelOffset = atoi(argv[i+1]);
      i++;
    }     
  }


  if ( ( bQmfInput != bQmfOutChan ) ) {
      fprintf( stdout, "\nERROR: Either both input files and output file are FD or TD. Mixed domains are not allowed.\n\n");
      inpath = NULL;
      outpath1 = NULL;
  }
  else {
      bQmfOutObj = bQmfInput;
  }

  if ( (inpath == NULL) || (outpath1 == NULL) /*|| (outpath2 == NULL)*/ )
    {
      fprintf( stdout, "Invalid arguments, usage:\n");
      fprintf( stdout, "-if          <input.wav>     Path to time domain input.\n");
      fprintf( stdout, "-if          <input>         Path to 64-band QMF domain input.\n");
      fprintf( stdout, "                             Filename will be extended to <input>_real.qmf and <input>_imag.qmf.\n");
      fprintf( stdout, "-of          <output.wav>    Time domain multichannel output stripped from objects.\n"); 
      fprintf( stdout, "-of          <output>        64-band QMF domain output stripped from objects.\n");
      fprintf( stdout, "                             Filename will be extended to <output>_real.qmf and <output>_imag.qmf.\n");
      fprintf( stdout, "-obj         <basename>      path to mono output wave files, automatically numerated\n");
      fprintf( stdout, "                             Filename will be extended to <basename>_xxx_real.qmf and <output>_xxx_imag.qmf\n");
      fprintf( stdout, "                             in case of QMF input and output.\n");
      fprintf(stderr, "\n   NOTE: Mixed domains (TD/FD) are not allowed. Either all time or all frequency in-/ouput.\n\n");
      fprintf( stdout, "-nObj        <num_objects>   number of objects to separate in wave file\n");
      fprintf( stdout, "-nCh         <num_channels>  number of channels to separate in wave file\n"); 
      fprintf( stdout, "-nOff        <chan_offset>   offset of first channel to separate in wave file\n\n"); 
      fprintf( stdout, "Example: \n  Split 7 objects from 10 channel input file into separate mono output files\n");
      fprintf( stdout, "  Use -if input.wav -of channel.wav -obj object -nObj 7\n");
      fprintf( stdout, "  channel.wav contains the first 3 channels from input.wav\n");
      fprintf( stdout, "  object_000.wav object_001.wav up to object_006.wav contain the 7 single mono object tracks\n");

      return -1;
    }

 /* Open input file */
  if ( bQmfInput ) {
    fQmfInFileReal = fopen( qmfInFileReal, "rb" );
    fQmfInFileImag = fopen( qmfInFileImag, "rb" );
  }
  else {
    fIn = fopen(inpath, "rb");
  }

  if ( fIn != NULL) {
    fprintf(stdout, "Found input file: %s.\n", inpath );
  }
  else if ( fQmfInFileReal != 0 && fQmfInFileImag != 0 )  {
    fprintf(stdout, "Found input files: %s and %s.\n", qmfInFileReal, qmfInFileImag );
  }
  else {
    if ( bQmfInput ) {
      fprintf(stderr, "Could not open qmf input files: %s and/or %s.\n", qmfInFileReal, qmfInFileImag );
    }
    else {
      fprintf(stderr, "Could not open input file: %s.\n", inpath );
    }
    return -1;
  }

  if ( bQmfOutChan ) {
    fQmfOutChanReal = fopen( qmfOutChanReal, "wb" );
    fQmfOutChanImag = fopen( qmfOutChanImag, "wb" );
  }
  else {
    fOutChan = fopen(outpath1, "wb");
  }

  if ( fOutChan != NULL) {
    fprintf(stdout, "Found first output file: %s.\n", outpath1 );
  }
  else if ( fQmfOutChanReal != 0 && fQmfOutChanImag != 0 )  {
    fprintf(stdout, "Found input files: %s and %s.\n", qmfOutChanReal, qmfOutChanImag );
  }
  else {
    if ( bQmfOutChan )
      fprintf(stderr, "Could not open first output files: %s and/or %s.\n", qmfOutChanReal, qmfOutChanImag );
    else {
      fprintf(stderr, "Could not open first output file: %s.\n", outpath1 );
    }
    
    return -1;
  }

  if ( bQmfOutObj ) {
    for ( i = 0; i < nObjects; ++i )
    {
      char qmfOutFilenameReal[FILENAME_MAX];
      char qmfOutFilenameImag[FILENAME_MAX];
      sprintf(qmfOutFilenameReal, "%s_%03d_real.qmf", qmfOutObjReal, i );
      sprintf(qmfOutFilenameImag, "%s_%03d_imag.qmf", qmfOutObjImag, i );

      fQmfOutObjReal[i] = fopen(qmfOutFilenameReal, "wb");
      fQmfOutObjImag[i] = fopen(qmfOutFilenameImag, "wb");
      if ( !fQmfOutObjReal[i] || !fQmfOutObjImag[i] ) {
        fprintf(stderr, "Could not open qmf output files: %s and/or %s.\n", qmfOutFilenameReal, qmfOutFilenameImag );
        return -1;
      }
    }
  }
  else {
    for ( i = 0; i < nObjects; ++i )
    {
      char outFilename[FILENAME_MAX];
      sprintf(outFilename, "%s_%03d.wav", outpath2, i );

      fOutObj[i] = fopen(outFilename, "wb");
      if ( !fOutObj[i] ) {
        fprintf(stderr, "Could not open output file: %s.\n", outFilename );
        return -1;
      }
    }
  }

  if ( bQmfInput && bQmfOutChan && bQmfOutObj ) {
    error = separator_init(&hSeparator, fQmfInFileReal, fQmfOutChanReal, fQmfOutObjReal, nObjects, nChannelOffset, nChannelsOut);
    if ( !error ) error = separator_init(&hImagSeparator, fQmfInFileImag, fQmfOutChanImag, fQmfOutObjImag, nObjects, nChannelOffset, nChannelsOut);
  }
  else {
    error = separator_init(&hSeparator, fIn, fOutChan, fOutObj, nObjects, nChannelOffset, nChannelsOut);
  }

  if ( error )
    {
      fprintf(stderr, "Error initializing separator\n");
      return -1;
    }

  if ( bQmfInput && bQmfOutChan && bQmfOutObj ) {
    do 
    {
      separator_mix(hSeparator, &isLastFrame);
      separator_mix(hImagSeparator, &isLastImagFrame);
      if ( isLastFrame != isLastImagFrame ) {
        fprintf(stdout, "Filelength mismatch between real and imag data!\n");
        return -1;
      }
      fprintf(stdout, "Separating %d\r", frameCnt);
      frameCnt++;
    }
    while (!isLastFrame);
  }
  else {
    do 
    {
        if ( separator_mix(hSeparator, &isLastFrame) ) {
          return -1;
        }
      fprintf(stdout, "Separating %d\r", frameCnt);
      frameCnt++;
    }
    while (!isLastFrame);
  }

  fprintf(stderr, "Separating finished %d\n", frameCnt);

  separator_close(hSeparator, &nTotalSamplesWrittenPerChannel);

  if ( bQmfInput && bQmfOutChan && bQmfOutObj ) {
    separator_close(hImagSeparator, &nTotalSamplesWrittenPerChannel);
  }

  return 0;
}
