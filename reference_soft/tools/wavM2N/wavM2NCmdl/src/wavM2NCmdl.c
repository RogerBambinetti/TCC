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

#include "wavM2Nlib.h"


int main(int argc, char *argv[])
{

  char *inpath = NULL;
  char *outpath = NULL;
  char *inGeoPath = NULL;

  int nObjects = 0;
  int chConfig = 0;
  int direction = 0;


  unsigned int i = 0;
  int error = 0;
  unsigned int isLastFrame = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  unsigned int frameCnt = 0;

  WAVM2N_HANDLE hWavM2N = NULL;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                           Split/Combine WavM2N Module                       *\n");
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
    if (!strcmp(argv[i],"-if"))
    {
      inpath = argv[i+1];
      i++;
    }
    
    if (!strcmp(argv[i],"-of"))
    {
      outpath = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i],"-dir"))
    {
      direction = atoi(argv[i+1]);
      i++;
    } 
    else if (!strcmp(argv[i],"-cicpIn"))
    {
      chConfig = atoi(argv[i+1]);
      i++;
    } 
    else if (!strcmp(argv[i],"-inGeo"))
    {
      inGeoPath = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i],"-nObj")) 
    {
      nObjects = atoi(argv[i+1]);
      i++;
    }     
  }

  if(argc<9)
    {
      fprintf( stdout, "Invalid arguments, usage:\n");
      fprintf( stdout, "-if          <input>       path to first input wave file  (w/o wav extension)\n");
      fprintf( stdout, "-of          <output>      path to first output wave file (w/o wav extension)\n");
      fprintf( stdout, "-cicpIn      <config>      cicp index\n");
      fprintf( stdout, "-inGeo       <geoFile>     path to geometry file (when cicpIn == -1).\n");
      fprintf( stdout, "-dir         <direction>   0: combine mono wave into MC wave; 1: split MC wave to mono waves\n"); 
      fprintf( stdout, "-nObj        <num_objects> number of objects to separate/merge in wave file, use cicpIn=31 for object files\n"); 
      fprintf( stdout, "Example: \n");
      fprintf( stdout, "  Use -if input -of output -nObj 7 -dir 0\n");
      fprintf( stdout, "  Use -if input -of output -cicpIn 13 -dir 1\n");

      return -1;
    }

  error = wavM2N_init(&hWavM2N, inpath, outpath, inGeoPath, direction, chConfig, nObjects);
  if ( error )
    {
      fprintf(stderr, "Error initializing wavM2N\n");
      return -1;
    }

  do 
    {
      wavM2N_mix(hWavM2N, &isLastFrame);
      fprintf(stdout, "Processing %d\r", frameCnt);
      frameCnt++;
    }
  while (!isLastFrame);

  fprintf(stderr, "Processing finished %d\n", frameCnt);

  wavM2N_close(hWavM2N, &nTotalSamplesWrittenPerChannel);

  return 0;
}
