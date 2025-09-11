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

#include "mixerlib.h"


int main(int argc, char *argv[])
{
  FILE *fIn1 = NULL;
  FILE *fIn2 = NULL;
  FILE *fQmfInFileReal1 = NULL;
  FILE *fQmfInFileImag1 = NULL;
  FILE *fQmfInFileReal2 = NULL;
  FILE *fQmfInFileImag2 = NULL;

  FILE *fOut = NULL;
  FILE *fQmfOutFileReal = NULL;
  FILE *fQmfOutFileImag = NULL;

  char *inpath1 = NULL;
  char *inpath2 = NULL;
  char *outpath = NULL;

  char qmfInFileReal1[200];
  char qmfInFileImag1[200];
  char qmfInFileReal2[200];
  char qmfInFileImag2[200];

  char qmfOutFileReal[200];
  char qmfOutFileImag[200];

  char* wavExtension  = ".wav";

  int delay1 = 0;
  int delay2 = 0;
  int delay3 = 0;
  
  float gain1 = 1.0f;
  float gain2 = 1.0f;

  unsigned int i = 0;
  int error = 0;
  unsigned int isLastFrame = 0;
  unsigned int isLastImagFrame = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  unsigned int frameCnt = 0;

  int bQmfInput1 = 0;
  int bQmfInput2 = 0;
  int bQmfOutput = 0;

  MIXER_HANDLE hMixer = NULL;
  MIXER_HANDLE hImagMixer = NULL;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                   Mixer Module                              *\n");
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

    if (!strcmp(argv[i],"-if1"))
    {
      inpath1 = argv[i+1];
      nWavExtensionChars = 0;
      nWavExtensionChars = strspn(wavExtension,inpath1);
      if ( nWavExtensionChars != 4 ) {
        bQmfInput1 = 1;
        strcpy(qmfInFileReal1, inpath1);
        strcpy(qmfInFileImag1, inpath1);
        strcat(qmfInFileReal1, "_real.qmf");
        strcat(qmfInFileImag1, "_imag.qmf");
      }
      i++;
    }
    
    if (!strcmp(argv[i],"-if2"))
    {
      inpath2 = argv[i+1];
      nWavExtensionChars = strspn(wavExtension,inpath2);
      if ( nWavExtensionChars != 4 ) {
        bQmfInput2 = 1;
        strcpy(qmfInFileReal2, inpath2);
        strcpy(qmfInFileImag2, inpath2);
        strcat(qmfInFileReal2, "_real.qmf");
        strcat(qmfInFileImag2, "_imag.qmf");
      }
      i++;
    }
    
    else if (!strcmp(argv[i],"-of"))
    {
      outpath = argv[i+1];
      nWavExtensionChars = strspn(wavExtension,outpath);
      if ( nWavExtensionChars != 4 ) {
        bQmfOutput = 1;
        strcpy(qmfOutFileReal, outpath);
        strcpy(qmfOutFileImag, outpath);
        strcat(qmfOutFileReal, "_real.qmf");
        strcat(qmfOutFileImag, "_imag.qmf");
      }
      i++;
    } 
    else if (!strcmp(argv[i],"-d1")) /* Optional */
    {
      delay1 = atoi(argv[i+1]);
      i++;
    }     
    else if (!strcmp(argv[i],"-d2")) /* Optional */
    {
      delay2 = atoi(argv[i+1]);
      i++;
    }
     else if (!strcmp(argv[i],"-d3")) /* Optional */
    {
      delay3 = atoi(argv[i+1]);
      i++;
    }    
     else if (!strcmp(argv[i],"-g1")) /* Optional */
    {
      gain1 = (float)atof(argv[i+1]);
      i++;
    }
     else if (!strcmp(argv[i],"-g2")) /* Optional */
    {
      gain2 = (float)atof(argv[i+1]);
      i++;
    }
  }

  if ( ( bQmfInput1 != bQmfInput2 ) || ( bQmfInput2 != bQmfOutput ) ) {
      fprintf( stdout, "\nERROR: Either both input files and output file are FD or TD. Mixed domains are not allowed.\n\n");
      inpath1 = NULL;
      inpath2 = NULL;
  }

  if ( (inpath1 == NULL) || (inpath2 == NULL) || (outpath == NULL) )
    {
      fprintf( stdout, "Invalid arguments, usage:\n");
      fprintf(stderr, "-if1      <input.wav>          Path to first time domain input.\n");
      fprintf(stderr, "-if1      <input>              Path to first 64-band QMF domain input.\n");
      fprintf(stderr, "                                  Filename will be extended to <input>_real.qmf and <input>_imag.qmf.\n");
      fprintf(stderr, "-if2      <input.wav>          Path to second time domain input.\n");
      fprintf(stderr, "-if2      <input>              Path to second 64-band QMF domain input.\n");
      fprintf(stderr, "                                  Filename will be extended to <input>_real.qmf and <input>_imag.qmf.\n");
      fprintf(stderr, "-of       <output.wav>         Time domain output.\n"); 
      fprintf(stderr, "-of       <output>             64-band QMF domain output.\n");
      fprintf(stderr, "                                  Filename will be extended to <output>_real.qmf and <output>_imag.qmf.\n");
      fprintf(stderr, "\n   NOTE: Mixed domains (TD/FD) are not allowed. Either all time or all frequency in-/ouput.\n\n");
      fprintf( stdout, "-d1 <delay>       add delay samples at the beginning to the first input file, value must be positive\n");
      fprintf( stdout, "-d2 <delay>       add delay samples at the beginning to the second input file, value must be positive\n");
      fprintf( stdout, "-d3 <delay>       remove delay samples at the beginning from the output file, value must be negative\n");
      fprintf( stdout, "-g1 <gain>        gain applied to first input file\n");
      fprintf( stdout, "-g2 <gain>        gain applied to second input file\n\n");
      fprintf( stdout, "Example: \n  First input file has 1000 samples delay compared to the second input file, with zero delay.\n");
      fprintf( stdout, "  To get an output file with zero delay and different gain, use -d1 0 -d2 1000 -d3 -1000 -g1 1.0 -g2 0.7 \n"); 

      return -1;
    }

 /* Open input file */
  if ( bQmfInput1 ) {
    fQmfInFileReal1 = fopen( qmfInFileReal1, "rb" );
    fQmfInFileImag1 = fopen( qmfInFileImag1, "rb" );
  }
  else {
    fIn1 = fopen(inpath1, "rb");
  }

  if ( fIn1 != NULL ) {
    fprintf(stdout, "Found input file: %s.\n", inpath1 );
  }
  else if ( fQmfInFileReal1 != 0 && fQmfInFileImag1 != 0 )  {
    fprintf(stdout, "Found input files: %s and %s.\n", qmfInFileReal1, qmfInFileImag1 );
  }
  else {
    if ( bQmfInput1 ) {
      fprintf(stderr, "Could not open first qmf input files: %s and/or %s.\n", qmfInFileReal1, qmfInFileImag1 );
    }
    else {
      fprintf(stderr, "Could not open first input file: %s.\n", inpath1 );
    }
    return -1;
  }

  if ( bQmfInput2 ) {
    fQmfInFileReal2 = fopen( qmfInFileReal2, "rb" );
    fQmfInFileImag2 = fopen( qmfInFileImag2, "rb" );
  }
  else {
    fIn2 = fopen(inpath2, "rb");
  }

  if ( fIn2 != NULL) {
    fprintf(stdout, "Found input file: %s.\n", inpath2 );
  }
  else if ( fQmfInFileReal2 != 0 && fQmfInFileImag2 != 0 )  {
    fprintf(stdout, "Found input files: %s and %s.\n", qmfInFileReal2, qmfInFileImag2 );
  }
  else {
    if ( bQmfInput2 ) {
      fprintf(stderr, "Could not open second qmf input files: %s and/or %s.\n", qmfInFileReal2, qmfInFileImag2 );
    }
    else {
      fprintf(stderr, "Could not open second input file: %s.\n", inpath2 );
    }
    return -1;
  }


 /* Open output file */
  if ( bQmfOutput ) 
  {
    fQmfOutFileReal = fopen( qmfOutFileReal, "wb" );
    fQmfOutFileImag = fopen( qmfOutFileImag, "wb" );
  }
  else
  {
    fOut = fopen(outpath, "wb");
  }

  if ( fOut != NULL) {
    fprintf(stdout, "Write to output file: %s.\n", outpath);
  }
  else if ( fQmfOutFileReal != NULL && fQmfOutFileImag != NULL ) {
    fprintf(stdout, "Write to output  files: %s and %s.\n", qmfOutFileReal, qmfOutFileImag );
  }
  else{
    if ( bQmfOutput )
      fprintf(stderr, "Could not open output files: %s and/or %s.\n", qmfOutFileReal, qmfOutFileImag );
    else {
      fprintf(stderr, "Could not open output file: %s.\n", outpath );
    }
    return -1;
  }

  if ( bQmfOutput ) {
    error = mixer_init(&hMixer, fQmfInFileReal1, fQmfInFileReal2, fQmfOutFileReal, delay1, delay2, delay3, gain1, gain2 );
    if ( error ) return -1;
    error = mixer_init(&hImagMixer, fQmfInFileImag1, fQmfInFileImag2, fQmfOutFileImag, delay1, delay2, delay3, gain1, gain2 );
    if ( error ) return -1;
  }
  else {
    error = mixer_init(&hMixer, fIn1, fIn2, fOut, delay1, delay2, delay3, gain1, gain2 );
    if ( error ) return -1;
  }

  if ( bQmfOutput ) {
    do 
    {
      mixer_mix(hMixer, &isLastFrame);
      mixer_mix(hImagMixer, &isLastImagFrame);
      if ( isLastFrame != isLastImagFrame ) {
        fprintf(stdout, "Filelength mismatch between real and imag data!\n");
        return -1;
      }
      fprintf(stdout, "Mixing %d\r", frameCnt);
      frameCnt++;
    }
    while (!isLastFrame);
  }
  else {
    do 
    {
      mixer_mix(hMixer, &isLastFrame);
      fprintf(stdout, "Mixing %d\r", frameCnt);
      frameCnt++;
    }
    while (!isLastFrame);
  }
  fprintf(stderr, "Mixing finished %d\n", frameCnt);

  mixer_close(hMixer, &nTotalSamplesWrittenPerChannel);

  if ( bQmfOutput ) {
    mixer_close(hImagMixer, &nTotalSamplesWrittenPerChannel);
  }

  return 0;
}

