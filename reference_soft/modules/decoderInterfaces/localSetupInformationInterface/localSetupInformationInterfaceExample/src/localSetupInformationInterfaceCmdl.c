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

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "localSetupInformationInterfaceLib_API.h"

/*

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsWav "C:\tmp\BRIR_Medium_IIS\IIS_BRIR_"  -geo "..\..\..\..\binaural\BRIRdata\geo_28_2.txt"

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsBitstream "..\..\..\..\binaural\BRIRdata\IIS_BRIR_FD_lowlevel.bs"

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -speakers "geo_CICPindex_knownPos.txt" -track" 

*/

int main(int argc, char *argv[]) 
{
  int error = 0;
  int k = 0, j = 0;

  /* encoder side */
  H_LS_BITSTREAM_ENCODER h_bitstream_LS_Enc = NULL;
  LOCAL_SETUP_HANDLE h_LS_Enc = NULL;
  H_LOCAL_SCREEN_INFO localScreenInfo = (H_LOCAL_SCREEN_INFO)calloc(1, sizeof(struct _LocalScreenInfo));
  

  /* decoder side */
  H_LS_BITSTREAM_DECODER h_bitstream_LS_Dec = NULL;
  LOCAL_SETUP_HANDLE h_LS_Dec = NULL;
  H_LOCAL_SETUP_DATA LocalSetupData = NULL;
  
  /* Variables */

  int *wireIDs;
  int *LSdistance = NULL;
  float *LScalibGain = NULL;
  int externalDistanceCompensation;
  int numSpeakers = -1;

  int Loudspeaker_Playback = 0;
  int useTrackingMode = 0;
 
  unsigned long no_BitsWritten, no_BitsRead;

  char *filename_LS = NULL;
  char *speakerfile = NULL; 
  char *brir_path = NULL; 
  char *brir_geo = NULL; 

  int hoaOrder = -1;
  int configType = 1;

  int numWireOutputs;

  int brirsFromWav = 0;
  int brirsFromBitstream = 0;

  fprintf(stderr, "\n\n"); 

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                  Local Setup Decoder Interface Module                       *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                                  %s                                *\n", __DATE__);
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*******************************************************************************\n");
  fprintf(stdout, "\n");

  /* *********************************** COMMANDLINE PARSING ************************************* */
  for (k = 1; k < argc; ++k)
  {
    /* output path for bitstream writer */
    if (!strcmp(argv[k], "-of"))     /* Required */
    {
      filename_LS = argv[k+1];
      k++; 
    }

    /* number of WIRE outputs */
    if (!strcmp(argv[k], "-wire"))     /* Required */
    {
      numWireOutputs = atoi(argv[k+1]);
      k++; 
      if (numWireOutputs > 0)
      {
        wireIDs = (int*)calloc(numWireOutputs, sizeof(int));
        for (j = 0; j < numWireOutputs; j++)
        {
          wireIDs[j] = atoi(argv[k+1]);
          k++;
        }
      }
    }

    /* local Screen Info */
    if (!strcmp(argv[k], "-screen"))     /* Required */
    {
      localScreenInfo->hasLocalScreenInfo = atoi(argv[k+1]);
      k++; 
      if (localScreenInfo->hasLocalScreenInfo == 1)
      {
        localScreenInfo->az_left = (float)atof(argv[k+1]);
        k++;
        localScreenInfo->az_right = (float)atof(argv[k+1]);
        k++;
        localScreenInfo->hasElevationInfo = atoi(argv[k+1]);
        k++;
        if (localScreenInfo->hasElevationInfo == 1)
        {
          localScreenInfo->el_bottom = (float)atof(argv[k+1]);
          k++;
          localScreenInfo->el_top = (float)atof(argv[k+1]);
          k++; 
        }
      }
    }

    { /* ONE OF THESE ARGUMENTS IS REQUIRED */
        if (!strcmp(argv[k], "-speakers"))     /* Optional */
        {
          Loudspeaker_Playback = 1;
          speakerfile = argv[k+1];
          k++; 
        }

        if (!strcmp(argv[k], "-brirsWav"))     /* Optional */
        {
          Loudspeaker_Playback = 0;
          brir_path = argv[k+1];
          brirsFromWav = 1;
          k++; 
        }

        if (!strcmp(argv[k], "-brirsBitstream"))     /* Optional */
        {
          Loudspeaker_Playback = 0;
          brir_path = argv[k+1];
          brirsFromBitstream = 1;
          k++; 
        }
    }

    { /* ONE OF THESE ARGUMENTS IS REQUIRED IN CASE OF -BRIRSWAV */
      if (!strcmp(argv[k], "-geo"))     /* Optional */
      {
        brir_geo = argv[k+1];
        k++; 
      }

      if (!strcmp(argv[k], "-hoa"))     /* Optional */
      {
        configType = 0;
        hoaOrder = atoi(argv[k+1]);
        k++; 
      }
    }

    { if (!strcmp(argv[k], "-track"))     /* Optional */
      {
        useTrackingMode = 1;
        k++; 
      }
    }
  }

  if (argc == 1)
  {
		fprintf(stderr, "Invalid arguments, usage:\n"); 
		fprintf(stderr, "-of <output.bs> path of the output bitstream file\n"); 
		fprintf(stderr, "-wire <number of defined wire outputs followed by a list of the wireIDs>\n"); 
		fprintf(stderr, "-screen <screensize> size of the local screen in the form:\n"); 
		fprintf(stderr, "\t hasLocalScreenInfo, (subsequent data only if hasLocalScreenInfo == 1)\n"); 
		fprintf(stderr, "\t az_left, az_right,\n"); 
		fprintf(stderr, "\t hasElevationInfo, (subsequent data only if hasElevationInfo == 1)\n"); 
		fprintf(stderr, "\t el_bottom, el_top\n\n"); 

		fprintf(stderr, "ONE OF THE FOLLOWING:\n"); 
		fprintf(stderr, "-speakers <speakers.txt> path to a text file that defines the speaker setup for loudspeaker playback\n");
		fprintf(stderr, "-brirsWav <brirfolder> path to a folder of BRIRs in the form of stereo wave files for binaural playback\n"); 
		fprintf(stderr, "-brirsBitstream <brirs.bs> path to a BRIR bitstream file (FIR data or low-level parameters) for binaural playback\n\n"); 

    fprintf(stderr, "ONE OF THE FOLLOWING ARGUMENTS IN CASE OF -BRIRSWAV:\n"); 
		fprintf(stderr, "-geo <geo.txt> path to a text file that contains the geometric information of the BRIR measurement positions\n");
		fprintf(stderr, "-hoa <order> hoa order of the BRIRs\n\n"); 

    fprintf(stderr, "-track switches useTrackingMode to 1 (ignored if binaural bitstream contains different useTrackingMode)\n"); 

    return -1;
  }

  if (localScreenInfo->az_left < localScreenInfo->az_right)
  {
    float tempAz;
    tempAz = localScreenInfo->az_left;
    localScreenInfo->az_left = localScreenInfo->az_right;
    localScreenInfo->az_right = tempAz;
    fprintf(stderr,"\nNote: Azimuth of left and right edge of screen were swapped.\n");
  }
  if (localScreenInfo->el_bottom > localScreenInfo->el_top)
  {
    float tempEl;
    tempEl = localScreenInfo->el_bottom;
    localScreenInfo->el_bottom = localScreenInfo->el_top;
    localScreenInfo->el_top = tempEl;
    fprintf(stderr,"\nNote: Elevation of lower and upper edge of screen were swapped.\n");
  }


  /***************** LOCAL SETUP INTERFACE ************************/

  /* init handle and bitstream writer */
  error = LS_initHandle(&h_LS_Enc);
  error = LS_DecInt_initBitstreamWriter(&h_bitstream_LS_Enc, filename_LS);

  /* set local screen data */
  error = LS_setLocalScreenSize(h_LS_Enc, localScreenInfo);
  /* init wire outputs */
  if (numWireOutputs > 0)
  {
    error = LS_initWireOutputs(h_LS_Enc, numWireOutputs, wireIDs);
  }
  else
  {
    numWireOutputs = 0;
    error = LS_initWireOutputs(h_LS_Enc, numWireOutputs, NULL);
  }
 

  if (Loudspeaker_Playback) /* read LS layout from txt file */
  {
    error = LS_initRenderingType(h_LS_Enc, 0);
    error = LS_setLoudspeakerLayout_fromFile(h_LS_Enc,speakerfile, &numSpeakers, &externalDistanceCompensation, &LSdistance, &LScalibGain);
    error = LS_setLoudspeakerRenderingConfig(h_LS_Enc, externalDistanceCompensation, numSpeakers, LSdistance, LScalibGain, useTrackingMode);
  }
  else if (brirsFromWav == 1) /* read BRIRs from wave files */
  {
	  if (configType == 0) /* HOA */
	  {
      error = LS_initRenderingType(h_LS_Enc, 1);

      if ( LS_setBRIRsFromWav_HOA(h_LS_Enc, brir_path, hoaOrder, useTrackingMode) != 0 )
		  {
			  printf("\nError -> could not load filters ! \n");
			  return -1;
		  }
	  }
	  else /* CICP or GEO */
	  {
      error = LS_initRenderingType(h_LS_Enc, 1);

      if ( LS_setBRIRsFromWav_CICP_GEO(h_LS_Enc, brir_path, -1, brir_geo, useTrackingMode) != 0 )
		  {
			  printf("\nError -> could not load filters ! \n");
			  return -1;
		  }
	  }
  }
  else if (brirsFromBitstream == 1)
  {
    error = LS_initRenderingType(h_LS_Enc, 1);

    if (LS_setBRIRsFromBitstream(h_LS_Enc, brir_path) != 0) /* useTrackingMode is ignored if BRIRs are read from bitstream file */
    {
		  printf("\nError -> could not load BRIRs ! \n");
		  return -1;
    }
  }
   

  /************ WRITING AND READING ************************/
  /* write Local Setup Data */
  error = LS_writeLocalSetupBitstream(h_LS_Enc, h_bitstream_LS_Enc, &no_BitsWritten, filename_LS);
  LS_DecInt_closeBitstreamWriter(h_bitstream_LS_Enc);

  /* start Local Setup reading */
  error = LS_initHandle(&h_LS_Dec);
  error = LS_DecInt_initBitstreamReader(&h_bitstream_LS_Dec, filename_LS);

  /* read Local Setup Data*/
  error = LS_readLocalSetupBitstream(h_LS_Dec, h_bitstream_LS_Dec, &no_BitsRead);
  error = LS_getLocalSetupData(h_LS_Dec, &LocalSetupData);

  /* close handles, free data */
  error = LS_closeHandle(h_LS_Enc);
  error = LS_closeHandle(h_LS_Dec);
  error = LS_freeLocalSetupData(LocalSetupData);
  LS_DecInt_closeBitstreamReader(h_bitstream_LS_Dec);
  

  /******************** free ********************************/
  
  free(localScreenInfo); localScreenInfo = NULL;
  if (numWireOutputs > 0)
  {
    free(wireIDs); wireIDs = NULL;
  }
  if (LSdistance != NULL)
  {
    free(LSdistance); LSdistance = NULL;
  }
  if (LScalibGain != NULL)
  {
    free(LScalibGain); LScalibGain = NULL;
  }
  
  return 0;
}