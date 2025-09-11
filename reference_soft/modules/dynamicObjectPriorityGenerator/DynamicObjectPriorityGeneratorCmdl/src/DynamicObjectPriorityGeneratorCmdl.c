/***********************************************************************************
 
 This software module was originally developed by 
 
 Sony Corporation
 
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
  
 Sony Corporation retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DynamicObjectPriorityGeneratorlib.h"


int main(int argc, char *argv[])
{

  char *inpath_wav = NULL;
  char *inpath_oam = NULL;
  char *outpath_oam = NULL;

  int blockLength = 1024;

  unsigned int i = 0;
  int error = 0;
  unsigned int isLastFrame = 0;
  unsigned int frameCnt = 0;

  DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE hDynamicObjectPriorityGenerator = NULL;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                Dynamic Object Priority Generator Module                     *\n");
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
    if (!strcmp(argv[i],"-if_wav"))
    {
      inpath_wav = argv[i+1];
      i++;
    }
	if (!strcmp(argv[i],"-if_oam"))
    {
      inpath_oam = argv[i+1];
      i++;
    }    
    if (!strcmp(argv[i],"-of_oam"))
    {
      outpath_oam = argv[i+1];
      i++;
    }
	else if (!strcmp(argv[i],"-bs")) 
    {
      blockLength = atoi(argv[i+1]);
      i++;
    } 
  }

  if(argc<7)
    {
      fprintf( stdout, "Invalid arguments, usage:\n");
      fprintf( stdout, "-if_wav      <input>       path to input object wave file  (w/o wav extension)\n");
	  fprintf( stdout, "-if_oam      <input>       path to input oam file\n");
      fprintf( stdout, "-of_oam      <output>      path to output oam file\n");
      fprintf( stdout, "-bs          <block_size>  blocksize for dynamic object priority calculation\n");
      fprintf( stdout, "Example: \n");
      fprintf( stdout, "  Use -if_wav input -if_oam input.oam -of_oam output.oam -bs 2048\n");

      return -1;
    }

  error = DynamicObjectPriorityGenerator_init(&hDynamicObjectPriorityGenerator, inpath_wav, inpath_oam, outpath_oam, blockLength);
  if ( error )
    {
      fprintf(stderr, "Error initializing DynamicObjectPriorityGenerator\n");
      return -1;
    }

  do 
    {
      DynamicObjectPriorityGenerator_proc(hDynamicObjectPriorityGenerator, &isLastFrame);
      fprintf(stdout, "Processing %d\r", frameCnt);
      frameCnt++;
    }
  while (!isLastFrame);

  fprintf(stderr, "Processing finished %d\n", frameCnt);

  DynamicObjectPriorityGenerator_close(hDynamicObjectPriorityGenerator);

  return 0;
}
