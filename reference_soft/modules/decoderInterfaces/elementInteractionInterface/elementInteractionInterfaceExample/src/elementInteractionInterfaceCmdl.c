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

#include "elementInteractionInterfaceLib_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
  int error = 0;
  int k = 0, j = 0;
  int frame_no = -1;

  /* encoder side */
  H_EI_BITSTREAM_ENCODER h_bitstream_EI_Enc = NULL;
  ELEMENT_INTERACTION_HANDLE h_EI_Enc = NULL;
  H_GROUP_INTERACTIVITY_STATUS **h_GroupStatus_Enc = NULL ;
  H_ZOOM_AREA *h_ZoomArea_Enc = NULL;

  /* decoder side */
  H_EI_BITSTREAM_DECODER h_bitstream_EI_Dec = NULL;
  ELEMENT_INTERACTION_HANDLE h_EI_Dec = NULL;
  H_ELEMENT_INTERACTION_DATA ElementInteractionData = NULL;
  
  /* Variables */
  int *groupIDlist;
  int signature_EI_Type = 0;
  int *interactionMode;
  int *groupPresetID;

  int numGroups = 0;
  int frames = 0;

  unsigned long no_BitsWritten, no_BitsRead;

  char *filename_EI = NULL;
  char *signature_EI = NULL;
  
  int required = 0;

  char *interactionfile = NULL;

  fprintf(stderr, "\n\n"); 

  fprintf(stdout, "\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                  Element Interaction Interface Module                       *\n");
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
      filename_EI = argv[k+1];
      k++; 
      required++;
    }

    /* input path for interaction file */
    if (!strcmp(argv[k], "-ei"))     /* Required */
    {
      interactionfile = argv[k+1];
      k++; 
      required++;
    }

    /* signature of interaction*/
    if (!strcmp(argv[k], "-sg"))     /* Required */
    {
      signature_EI = argv[k+1];
      signature_EI_Type = 0;
      k++; 
      required++;
    }
  }

  if (required < 3)
  {
    fprintf(stderr, "Invalid arguments, usage:\n"); 
		fprintf(stderr, "-of <output.bs> path to output 'bitstream' file with element interaction data\n"); 
		fprintf(stderr, "-ei <interaction.txt> path to text file containing element interaction data\n"); 
		fprintf(stderr, "-sg <string> signature string\n"); 
    return -1;
  }



  /***************** ELEMENT INTERACTION INTERFACE ************************/

  /* init handles + bitstream writers */
  interactionMode = NULL;
  groupPresetID = NULL;

  error = EI_initHandle(&h_EI_Enc);
  error = EI_DecInt_initBitstreamWriter(&h_bitstream_EI_Enc, filename_EI);

  /* get interaction data from file */
  error = EI_getInteractionFromFile(interactionfile, &interactionMode, &groupPresetID, &h_ZoomArea_Enc, &h_GroupStatus_Enc, &groupIDlist, &numGroups, &frames, 2);

  /* fill framewise element interaction data */
  for (j = 0; j < frames; j++)
  {
    frame_no ++;
    
    error = EI_setInteractionConfig(h_EI_Enc, interactionMode[j], groupPresetID[j], signature_EI, signature_EI_Type);
    error = EI_setInteractionGroupConfig(h_EI_Enc, groupIDlist, numGroups);
    if (error != 0) 
    {
      return error;
    }

    error = EI_setZoomArea(h_EI_Enc, h_ZoomArea_Enc[j]);
    
    for (k=0; k < numGroups; k++)
    {
      error = EI_setInteractionGroupStatus(h_EI_Enc, groupIDlist[k], h_GroupStatus_Enc[j][k]);
      if (error != 0)
      {
        return error;
      }

    }

    /* write data subsequently to file */
    error = EI_writeElementInteractionBitstream(h_EI_Enc, h_bitstream_EI_Enc, &no_BitsWritten);
  }
 
  /* finish writing */
  EI_DecInt_closeBitstreamWriter(h_bitstream_EI_Enc);
  error = EI_closeHandle(h_EI_Enc);
  
  /* free data from EI_getInteractionFromFile */
  for (j = 0; j < frames; j++)
  {
    free(h_ZoomArea_Enc[j]); h_ZoomArea_Enc[j] = NULL;
    for (k=0; k < numGroups; k++)
    {
      free(h_GroupStatus_Enc[j][k]); h_GroupStatus_Enc[j][k] = NULL;
    }
    free(h_GroupStatus_Enc[j]); h_GroupStatus_Enc[j] = NULL;
  }
  free(h_GroupStatus_Enc); 
  free(h_ZoomArea_Enc);
  free(interactionMode);
  free(groupPresetID);

  /* start Element Interaction reading */
  error = EI_initHandle(&h_EI_Dec);
  error = EI_DecInt_initBitstreamReader(&h_bitstream_EI_Dec, filename_EI);

  for (j = 0; j < frames; j++)
  {
    error = EI_readElementInteractionBitstream(h_EI_Dec, h_bitstream_EI_Dec, &no_BitsRead);
    error = EI_getElementInteractionData(h_EI_Dec, &ElementInteractionData);
  }

  /* finish reading */
  EI_DecInt_closeBitstreamReader(h_bitstream_EI_Dec);

  error = EI_closeHandle(h_EI_Dec);
  free(groupIDlist); groupIDlist = NULL;
  error = EI_freeElementInteractionData(ElementInteractionData);

  return 0;
}