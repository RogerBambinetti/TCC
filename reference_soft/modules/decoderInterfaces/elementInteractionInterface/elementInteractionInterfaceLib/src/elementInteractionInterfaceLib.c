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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elementInteractionInterfaceLib.h"
#include "elementInteractionInterfaceLib_BitstreamIO.h"

int EI_getInteractionFromFile(char *filename, int **interactionMode, int **groupPresetID, H_ZOOM_AREA **h_ZoomArea_Enc, H_GROUP_INTERACTIVITY_STATUS ***h_GroupStatus_Enc, int **groupIDs, int* noGroups, int *noFrames, const int verboseLvl)
{
  /**
  *   Read interaction data from a dedicated file
  *   @param    filename            : path+filename of the file to be read
  *   @param    interactionMode     : output parameter, returns pointer to a an array that contains the interactionMode for each frame
  *   @param    groupPresetID       : output parameter, returns pointer to a an array that contains the chosen groupPresetID for each frame (contains -1 if interaction mode is zero (advanced interaction)
  *   @param    h_ZoomArea_Enc      : output parameter, returns pointer to a an array of zoom area structs (one for each frame)
  *   @param    h_GroupStatus_Enc   : output parameter, returns pointer to a an array of group status structs (one for each frame and group, dim: [frames][groups])
  *   @param    groupIDs            : output parameter, returns pointer to a an array that contains the groupIDs
  *   @param    noGroups            : output parameter, number of groups
  *   @param    noFrames            : output parameter, number of frames
  *   @param    verboseLvl          : input parameter,  verbose level
  *
  *   @return   ERROR_CODE
  *   
  *   The file (textfile) has to have the following form:
  *     line 1:   number of frames
  *     line 2:   number of groups : GroupIDs (comma separated)
  *     line 3 and ongoing:
  *       for each frame:
  *       line N:     interactionMode, groupPresetID (groupPresetID not needed if interactionMode == 0)
  *       line N+1:   hasZoomArea, Az, AzCenter, El, ElCenter (Az, AzCenter, El, ElCenter not needed if hasZoomArea == 0)
  *       line N+2:   onOffStatus for all groups (comma separated, order as groupIDs above, to be skipped in case interactionMode == 1)
  *       line N+3:   azOffset for all groups (comma separated, order as groupIDs above)
  *       line N+4:   elOffset for all groups (comma separated, order as groupIDs above)
  *       line N+5:   distFactor for all groups (comma separated, order as groupIDs above)
  *       line N+6:   gain_dB for all groups (comma separated, order as groupIDs above)
  *       line N+7:   routeToWireID for all groups (comma separated, order as groupIDs above, -1 if not routed to WIRE)
  */
  FILE *fileHandle;
  char line[512] = {0};
  int numGroups;
  int numFrames; 
  int i, k;
  int *groupID = NULL;
  int *interactMode = NULL;
  int *presetID = NULL;
  int m = 0;
  int n;
  int readLineType = 0;

  int readFrames = 0;
  int finish = 0;

  H_ZOOM_AREA *h_ZoomArea_Tmp = NULL;
  H_GROUP_INTERACTIVITY_STATUS **h_Interact_tmp = NULL;

  if ( filename != NULL ) 
  {
    fileHandle = fopen(filename, "r");    
    if ( !fileHandle )
    {
      if (verboseLvl >= 1) {
        fprintf(stderr,"Error: Unable to open interactivtiy file (%s).\n",filename);        
      }
      return -1;
    } 
    else 
    {
      if (verboseLvl >= 2) {
        fprintf(stdout, "\nInfo: Found interactivtiy file: %s.\n", filename );
      }
    }
    numGroups = 0;
    numFrames = 0;
    while ( fgets(line, 511, fileHandle) != NULL )
    {
      char* pChar = line;

      if (finish == 1)
      {
        break;
      }

      i = 0;
      /* Add newline at end of line (for eof line), terminate string after newline */
      line[strlen(line)+1] = '\0';
      line[strlen(line)] = '\n';      
      
      /* Replace all white spaces with new lines for easier parsing */
      while ( pChar[i] != '\0') {

        if ( pChar[i] == ' ' || pChar[i] == '\t') {
          pChar[i] = '\n';            
        }
        i++;
      }  
      /* Parse line */        
      pChar = line;
      while ( (*pChar) != '\0') 
      {
        while (  (*pChar) == '\n' || (*pChar) == '\r'  )
          pChar++;
        if (numFrames == 0)
        {
          numFrames = atoi(pChar);
          interactMode = (int*)calloc(numFrames, sizeof(int));
          presetID = (int*)calloc(numFrames, sizeof(int));
          h_ZoomArea_Tmp = (H_ZOOM_AREA*)calloc(numFrames, sizeof(H_ZOOM_AREA));
          for (k = 0; k < numFrames; k++)
          {
             h_ZoomArea_Tmp[k] = (H_ZOOM_AREA)calloc(1,sizeof(struct _ZoomArea));
          }
          if (numFrames > 0)
          {
            if (verboseLvl >= 2) {
              fprintf(stdout,"Reading element interaction data for %d frames.\n",numFrames);
            }
          }
          (*pChar) = '\0';
        }
        else
        {
          if (numGroups == 0)
          {
            while ( (*pChar) == '\n' || (*pChar) == '\r' )
              pChar++;
            numGroups = atoi(pChar);
            groupID = (int*)calloc(numGroups, sizeof(int));
            h_Interact_tmp = (H_GROUP_INTERACTIVITY_STATUS**)calloc(numFrames, sizeof(H_GROUP_INTERACTIVITY_STATUS*));
            for (k = 0; k < numFrames; k++)
            {
               h_Interact_tmp[k] = (H_GROUP_INTERACTIVITY_STATUS*)calloc(numGroups,sizeof(H_GROUP_INTERACTIVITY_STATUS));
               for (n = 0; n < numGroups; n++)
               {
                  h_Interact_tmp[k][n] = (H_GROUP_INTERACTIVITY_STATUS)calloc(1,sizeof(struct _GroupInteraction));
               }
            }
            (*pChar) = '\0';

            while ( *(pChar) != ':' )
              pChar++;

            pChar++;
            for (k = 0; k < numGroups; k++)
            {
              while ( (*pChar) == '\n' || (*pChar) == '\r' )
                pChar++;
              groupID[k] = atoi(pChar);
              if (k < numGroups - 1)
              {
                while ( *(pChar) != ',' )
                  pChar++;

                pChar++;
              }
              else
              {
                pChar++;
                (*pChar) = '\0';
              }
            }
          }
          else
          {
            if (readLineType == 0)
            {
              while ( (*pChar) == '\n' || (*pChar) == '\r' )
                pChar++;
              interactMode[m] = atoi(pChar);
              if (interactMode[m] == 1)
              {
                while ( *(pChar) != ',' )
                  pChar++;

                pChar++;

                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;
                presetID[m] = atoi(pChar);
              }
              else
              {
                presetID[m] = -1;
              }
              (*pChar) = '\0';
              readLineType = 1;
              break;
            }

            if (readLineType == 1)
            {
              while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;
              
              h_ZoomArea_Tmp[m]->hasZoomArea = atoi(pChar);

              if (h_ZoomArea_Tmp[m]->hasZoomArea == 1)
              {
                while ( *(pChar) != ',' )
                    pChar++;

                pChar++;
                
                h_ZoomArea_Tmp[m]->Az = (float)atof(pChar);
                while ( *(pChar) != ',' )
                    pChar++;

                pChar++;

                h_ZoomArea_Tmp[m]->AzCenter = (float)atof(pChar);
                while ( *(pChar) != ',' )
                    pChar++;

                pChar++;

                h_ZoomArea_Tmp[m]->El = (float)atof(pChar);
                while ( *(pChar) != ',' )
                    pChar++;

                pChar++;

                h_ZoomArea_Tmp[m]->ElCenter = (float)atof(pChar);
              }
              else
              {
                h_ZoomArea_Tmp[m]->Az = 0.0f;
                h_ZoomArea_Tmp[m]->AzCenter = 0.0f;
                h_ZoomArea_Tmp[m]->El = 0.0f;
                h_ZoomArea_Tmp[m]->ElCenter = 0.0f;
                
              }
              (*pChar) = '\0';
              readLineType = 2;
              break;
            }

            if (readLineType == 2)
            {
              if (interactMode[m] == 0) /* only for advanced interaction mode */
              {
                for (k = 0; k < numGroups; k++)
                {
                  while ( (*pChar) == '\n' || (*pChar) == '\r' )
                    pChar++;

                  h_Interact_tmp[m][k]->groupID = groupID[k];
                  h_Interact_tmp[m][k]->onOff = atoi(pChar);
                  if (k < numGroups - 1)
                  {
                    while ( *(pChar) != ',' )
                      pChar++;

                    pChar++;
                  }
                  else
                  {
                    pChar++;
                    (*pChar) = '\0';
                  }
                }
                readLineType = 3;
                break;
              }
              else
              {
                readLineType = 3;
              }
            }

            if (readLineType == 3)
            {
              for (k = 0; k < numGroups; k++)
              {
                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;

                h_Interact_tmp[m][k]->azOffset = (float)atof(pChar);
                if (k < numGroups - 1)
                {
                  while ( *(pChar) != ',' )
                    pChar++;

                  pChar++;
                }
                else
                {
                  pChar++;
                  (*pChar) = '\0';
                }
              }
              readLineType = 4;
              break;
            }

            if (readLineType == 4)
            {
              for (k = 0; k < numGroups; k++)
              {
                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;

                h_Interact_tmp[m][k]->elOffset = (float)atof(pChar);
                if (k < numGroups - 1)
                {
                  while ( *(pChar) != ',' )
                    pChar++;

                  pChar++;
                }
                else
                {
                  pChar++;
                  (*pChar) = '\0';
                }
              }
              readLineType = 5;
              break;
            }

            if (readLineType == 5)
            {
              for (k = 0; k < numGroups; k++)
              {
                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;

                h_Interact_tmp[m][k]->distFactor = (float)atof(pChar);
                if (k < numGroups - 1)
                {
                  while ( *(pChar) != ',' )
                    pChar++;

                  pChar++;
                }
                else
                {
                  pChar++;
                  (*pChar) = '\0';
                }
              }
              readLineType = 6;
              break;
            }

            if (readLineType == 6)
            {
              for (k = 0; k < numGroups; k++)
              {
                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;

                h_Interact_tmp[m][k]->gain_dB = atoi(pChar);
                if (k < numGroups - 1)
                {
                  while ( *(pChar) != ',' )
                    pChar++;

                  pChar++;
                }
                else
                {
                  pChar++;
                  (*pChar) = '\0';
                }
              }
              readLineType = 7;
              break;
            }

            if (readLineType == 7)
            {
              for (k = 0; k < numGroups; k++)
              {
                while ( (*pChar) == '\n' || (*pChar) == '\r' )
                  pChar++;

                h_Interact_tmp[m][k]->routeToWireID = atoi(pChar);
                if (k < numGroups - 1)
                {
                  while ( *(pChar) != ',' )
                    pChar++;

                  pChar++;
                }
                else
                {
                  pChar++;
                  (*pChar) = '\0';
                }
              }
              readLineType = 0;
              readFrames++;
              if (readFrames == numFrames)
              {
                finish = 1;
                break;
              }
              m++;
              break;
            }
          }
        }
      }
    }
  m = 1;
  *interactionMode = interactMode;
  *groupPresetID = presetID;
  *h_ZoomArea_Enc = h_ZoomArea_Tmp;
  *h_GroupStatus_Enc = h_Interact_tmp;
  *groupIDs = groupID;
  *noGroups = numGroups;
  *noFrames = numFrames;
  }
  else
  {
    return -1;
  }
  return 0;
}

int EI_setInteractionConfig(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int interactionMode, int groupPresetID, char *signature, int signatureType)
{
  /**
	*   Set the interaction mode, groupPresetID, signature
	*   @param    h_ElementInteractionHandle  : element interaction handle that contains the data to be written to the bitstream
	*   @param    interactionMode		          : interaction mode (0: advanced | 1: basic (presets))
  *   @param    groupPresetID		            : chosen preset if interactionMode == 1, values is ignored if interactionMode == 0
  *   @param    signature		                : signature of interaction (array of char)
  *   @param    signatureType		            : 0 for string-based signature (otherwise the signature is ignored)
	*
	*   @return   ERROR_CODE
	*/
  int k = 0; 

  if (signature != NULL)
  {
    h_ElementInteractionHandle->signature_type = signatureType;
    if (signatureType == 0)
    {
      for (k = 0; k < MAX_LENGTH_SIGNATURE; k++)
      {
        h_ElementInteractionHandle->signature[k] = signature[k];
        if (signature[k] == '\0')
        {
          break;
        }
      }
    }
  }
  else
  {
    h_ElementInteractionHandle->signature_type = -1;
  }
  h_ElementInteractionHandle->interactionMode = interactionMode;
  if (abs(interactionMode) == 1)
  {
    h_ElementInteractionHandle->groupPresetID = groupPresetID;
  }
  else
  {
    h_ElementInteractionHandle->groupPresetID = -1;
  }  

  return 0;
}

int EI_readElementInteractionBitstream(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_DECODER h_bitstream, unsigned long *no_BitsRead)
{
  unsigned long noBitsRead = 0;
  unsigned long noBitsReadTotal = 0;
  int error = 0;

  if (!feof(h_bitstream->bsFile))
  {
     /* read data */
    error = EI_readElementInteractionBits(ElementInteraction, h_bitstream, &noBitsRead, &noBitsReadTotal);
    
    if (error == 0)
    {
      /* inverse quantization of data */
      error = EI_invquantizeData(ElementInteraction);
      *no_BitsRead = noBitsReadTotal;
    }
    else
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }

  return 0;
}



int EI_initHandle(ELEMENT_INTERACTION_HANDLE *h_ElementInteractionHandle)
{
  /**
	*   Initialization of the element interaction handle that contains the data to be written to the bitstream/interface or read from the bitstream/interface
	*   @param    h_ElementInteractionHandle  : pointer to the element interaction handle, should point to NULL or to non-allocated memory for initialization
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  ELEMENT_INTERACTION_HANDLE temp = (ELEMENT_INTERACTION_HANDLE)calloc(1, sizeof(struct _ElementInteractionHandle));
  H_ZOOM_AREA temp_zoom = (H_ZOOM_AREA)calloc(1,sizeof(struct _ZoomArea));
  H_ZOOM_AREA_QUANT temp_zoom_quant = (H_ZOOM_AREA_QUANT)calloc(1,sizeof(struct _ZoomAreaQuant));

  H_GROUP_INTERACTIVITY_STATUS *groupInteraction = NULL;
  H_GROUP_INTERACTIVITY_STATUS_QUANT *groupInteractionQuant = NULL;

  groupInteraction = (H_GROUP_INTERACTIVITY_STATUS*)calloc(MAX_NUM_GROUPS,sizeof(H_GROUP_INTERACTIVITY_STATUS));
  groupInteractionQuant = (H_GROUP_INTERACTIVITY_STATUS_QUANT*)calloc(MAX_NUM_GROUPS,sizeof(H_GROUP_INTERACTIVITY_STATUS_QUANT));


  temp->numAllocatedGroups = MAX_NUM_GROUPS;
  temp->signature = (char*)calloc(MAX_LENGTH_SIGNATURE,sizeof(char));

  temp->numGroups = -1;

  temp->ZoomArea = temp_zoom;
  temp->ZoomArea->hasZoomArea = 0;
  temp->ZoomArea->AzCenter = 0.0f;
  temp->ZoomArea->Az = 0.0f;
  temp->ZoomArea->ElCenter = 0.0f;
  temp->ZoomArea->El = 0.0f;

  temp->ZoomAreaQuant = temp_zoom_quant;
  temp->ZoomAreaQuant->AzCenter = 0;
  temp->ZoomAreaQuant->Az = 0;
  temp->ZoomAreaQuant->ElCenter = 0;
  temp->ZoomAreaQuant->El = 0;

  for (k = 0; k < MAX_NUM_GROUPS; k++)
  {
    groupInteraction[k] = (H_GROUP_INTERACTIVITY_STATUS)calloc(1,sizeof(struct _GroupInteraction));
    groupInteraction[k]->groupID = -1;
    groupInteraction[k]->onOff = 1;
    groupInteraction[k]->routeToWireID = -1;
    groupInteraction[k]->azOffset = 0.0f;
    groupInteraction[k]->elOffset = 0.0f;
    groupInteraction[k]->distFactor = 1.0f;
    groupInteraction[k]->gain_dB = 0;

    groupInteractionQuant[k] = (H_GROUP_INTERACTIVITY_STATUS_QUANT)calloc(1,sizeof(struct _GroupInteractionQuant));
    groupInteractionQuant[k]->groupID = -1;
    groupInteractionQuant[k]->azOffset = 0;
    groupInteractionQuant[k]->elOffset = 0;
    groupInteractionQuant[k]->distFactor = 1;
    groupInteractionQuant[k]->gain_dB = 0;
  }

  temp->GroupInteraction = groupInteraction;
  temp->GroupInteractionQuant = groupInteractionQuant;

  *h_ElementInteractionHandle = temp;

  return 0;

}

int EI_closeHandle(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle)
{
  /**
	*   Closing of the element interaction handle that contains the data to be written to the bitstream/interface or read from the bitstream/interface
	*   @param    h_ElementInteractionHandle  : element interaction handle
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  for (k = 0; k < h_ElementInteractionHandle->numAllocatedGroups; k++)
  {
    free(h_ElementInteractionHandle->GroupInteraction[k]); h_ElementInteractionHandle->GroupInteraction[k] = NULL;
    free(h_ElementInteractionHandle->GroupInteractionQuant[k]); h_ElementInteractionHandle->GroupInteractionQuant[k] = NULL;
  }
  free(h_ElementInteractionHandle->GroupInteraction); h_ElementInteractionHandle->GroupInteraction = NULL;
  free(h_ElementInteractionHandle->GroupInteractionQuant); h_ElementInteractionHandle->GroupInteractionQuant = NULL;
  free(h_ElementInteractionHandle->ZoomArea); h_ElementInteractionHandle->ZoomArea = NULL;
  free(h_ElementInteractionHandle->ZoomAreaQuant); h_ElementInteractionHandle->ZoomAreaQuant = NULL;
  free(h_ElementInteractionHandle->signature); h_ElementInteractionHandle->signature = NULL;
  free(h_ElementInteractionHandle);

  return 0;
}

int EI_setInteractionGroupConfig(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int *groupIDs, int numGroups)
{
  /**
	*   Set the group configuration for the interaction (number of groups and groupIDs), can be called each time the number or IDs of groups change
	*   @param    h_ElementInteractionHandle  : element interaction handle 
	*   @param    interactionMode		          : interaction mode (0: advanced | 1: basic (presets))
  *   @param    groupIDs		                : array of groupIDs
  *   @param    numGroups		                : number of groups
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  if (numGroups > MAX_NUM_GROUPS)
  {
    fprintf( stderr, "Too many groups, only a maximum number of 128 is allowed!\n");
    return -1;
  }

  if (numGroups < h_ElementInteractionHandle->numGroups)
  {
    for (k = 0; k < h_ElementInteractionHandle->numGroups; k++)
    {
      h_ElementInteractionHandle->GroupInteraction[k]->groupID = -1;
      h_ElementInteractionHandle->GroupInteractionQuant[k]->groupID = -1;
    }
  }

  h_ElementInteractionHandle->numGroups = numGroups;
  for (k = 0; k < h_ElementInteractionHandle->numGroups; k++)
  {
    h_ElementInteractionHandle->GroupInteraction[k]->groupID = groupIDs[k];
    h_ElementInteractionHandle->GroupInteractionQuant[k]->groupID = groupIDs[k];
  }
  return 0;
}


int EI_setZoomArea(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, H_ZOOM_AREA ZoomArea)
{
  h_ElementInteractionHandle->ZoomArea->hasZoomArea = ZoomArea->hasZoomArea;
  if (ZoomArea->hasZoomArea)
  {
    h_ElementInteractionHandle->ZoomArea->AzCenter = ZoomArea->AzCenter;
    h_ElementInteractionHandle->ZoomArea->ElCenter = ZoomArea->ElCenter;
    h_ElementInteractionHandle->ZoomArea->Az = ZoomArea->Az;
    h_ElementInteractionHandle->ZoomArea->El = ZoomArea->El;
  }
  else
  {
    h_ElementInteractionHandle->ZoomArea->AzCenter = 0.0f;
    h_ElementInteractionHandle->ZoomArea->ElCenter = 0.0f;
    h_ElementInteractionHandle->ZoomArea->Az = 0.0f;
    h_ElementInteractionHandle->ZoomArea->El = 0.0f;
  }

  return 0;
}

int EI_freeElementInteractionData(H_ELEMENT_INTERACTION_DATA ElementInteractionData)
{ 
  /**
	*   Memory de-allocation of element interaction data
	*   @param    ElementInteractionData  : element interaction data struct
	*
	*   @return   ERROR_CODE
	*/
  
  int k = 0;

  free(ElementInteractionData->signature);

  for (k=0; k < ElementInteractionData->numGroups; k++)
  {
    free(ElementInteractionData->GroupInteraction[k]); ElementInteractionData->GroupInteraction[k] = NULL;
  }
  free(ElementInteractionData->GroupInteraction); ElementInteractionData->GroupInteraction = NULL;
  
  free(ElementInteractionData->ZoomArea); ElementInteractionData->ZoomArea = NULL;

  free(ElementInteractionData);
  ElementInteractionData = NULL;

  return 0;

}



int EI_setInteractionGroupStatus(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int groupID, H_GROUP_INTERACTIVITY_STATUS GroupStatus)
{
  /**
	*   Set the interaction status for a specific group
	*   @param    h_ElementInteractionHandle  : element interaction handle 
	*   @param    groupID		                  : groupID of the group that should be modified
  *   @param    GroupStatus		              : group status handle that contains the interaction data for the chosen group
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  int index = -1;
  for (k = 0; k< h_ElementInteractionHandle->numGroups; k++)
  {
    if (h_ElementInteractionHandle->GroupInteraction[k]->groupID == groupID)
    {
      index = k;
      break;
    }
  }
  if (index == -1)
  {
    fprintf( stderr, "GroupID not found in list of groups, interaction ignored!\n");
    return -1;
  }
  else
  {
    h_ElementInteractionHandle->GroupInteraction[index]->onOff = (GroupStatus->onOff != h_ElementInteractionHandle->GroupInteraction[index]->onOff) ? GroupStatus->onOff : h_ElementInteractionHandle->GroupInteraction[index]->onOff;
    h_ElementInteractionHandle->GroupInteraction[index]->azOffset = (GroupStatus->azOffset != h_ElementInteractionHandle->GroupInteraction[index]->azOffset) ? GroupStatus->azOffset : h_ElementInteractionHandle->GroupInteraction[index]->azOffset;
    h_ElementInteractionHandle->GroupInteraction[index]->elOffset = (GroupStatus->elOffset != h_ElementInteractionHandle->GroupInteraction[index]->elOffset) ? GroupStatus->elOffset : h_ElementInteractionHandle->GroupInteraction[index]->elOffset;
    h_ElementInteractionHandle->GroupInteraction[index]->distFactor = (GroupStatus->distFactor != h_ElementInteractionHandle->GroupInteraction[index]->distFactor) ? GroupStatus->distFactor : h_ElementInteractionHandle->GroupInteraction[index]->distFactor;
    h_ElementInteractionHandle->GroupInteraction[index]->gain_dB = (GroupStatus->gain_dB != h_ElementInteractionHandle->GroupInteraction[index]->gain_dB) ? GroupStatus->gain_dB : h_ElementInteractionHandle->GroupInteraction[index]->gain_dB;
    
    return 0;
  }
}



int EI_readElementInteractionBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsRead, unsigned long *noBitsReadTotal)
{
  /**
	*   Read the element interaction (mpeg3daElementInteraction() syntax) once from the bitstream (for 1 frame)
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*   @param    h_bitstream		        : bitstream handle
  *   @param    noBitsRead		        : output parameter, number of read bits
  *   @param    noBitsReadTotal		    : output parameter, adds the number of read bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned int length_byte_signature = 0;
  char *signature = NULL;
  int k = 0;
  int change_position = 0, change_gain = 0;
  int route_to_wire = 0;
  unsigned int temp = 0;
  int temp2;

  *noBitsRead = 0;

  temp2 = EI_DecInt_readBits(h_bitstream,&length_byte_signature,8);
  if (temp2 == -1)
  {
    return -1;
  }
  else
  {
    *noBitsRead += temp2;
  }

  signature = (char*)calloc(MAX_LENGTH_SIGNATURE,sizeof(char));
  if (length_byte_signature != 0)
  {
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,8);
    ElementInteraction->signature_type = (int)temp;
    for (k = 0; k < (int)length_byte_signature; k++)
    {
      *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,8);
      signature[k] = (char)temp;
    }   
  }   
  ElementInteraction->signature = signature;
  *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 1);
  ElementInteraction->interactionMode = (int)temp;
  *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 7);
  ElementInteraction->numGroups = (int)temp;
  if (ElementInteraction->interactionMode == 1)
  {
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 5);
    ElementInteraction->groupPresetID = (int)temp;
  }
  else
  {
    ElementInteraction->groupPresetID = -1;
  }

  for (k = 0; k < ElementInteraction->numGroups; k++)
  {
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 7);
    ElementInteraction->GroupInteractionQuant[k]->groupID = (int)temp;
    ElementInteraction->GroupInteraction[k]->groupID = ElementInteraction->GroupInteractionQuant[k]->groupID;
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 1);
    ElementInteraction->GroupInteraction[k]->onOff = (int)temp;
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 1);
    route_to_wire = (int)temp;
    if (route_to_wire == 1)
    {
      *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp, 16);
      ElementInteraction->GroupInteraction[k]->routeToWireID = (int)temp;
    }
    else
    {
      ElementInteraction->GroupInteraction[k]->routeToWireID = -1;
    }
    if (ElementInteraction->GroupInteraction[k]->onOff == 1)
    {
      *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,1);
      change_position = (int)temp;
      if (change_position == 1)
      {
        *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,8);
        ElementInteraction->GroupInteractionQuant[k]->azOffset = (int)temp;
        *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,6);
        ElementInteraction->GroupInteractionQuant[k]->elOffset = (int)temp;
        *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,4);
        ElementInteraction->GroupInteractionQuant[k]->distFactor = (int)temp;
      }
      else
      {
        ElementInteraction->GroupInteractionQuant[k]->azOffset = 128;
        ElementInteraction->GroupInteractionQuant[k]->elOffset = 32;
        ElementInteraction->GroupInteractionQuant[k]->distFactor = 12;
      }
      *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,1);
      change_gain = (int)temp;
      if (change_gain == 1)
      {
        *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,7);
        ElementInteraction->GroupInteractionQuant[k]->gain_dB = (int)temp;
      }
      else
      {
        ElementInteraction->GroupInteractionQuant[k]->gain_dB = 64;
      }
    }
  }
  *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,1);
  ElementInteraction->ZoomArea->hasZoomArea = (int)temp;
  if (ElementInteraction->ZoomArea->hasZoomArea == 1)
  {
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,10);
    ElementInteraction->ZoomAreaQuant->AzCenter = EI_DecInt_convertIfNegative(temp,10);
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,10);
    ElementInteraction->ZoomAreaQuant->Az = (int)temp;
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,9);
    ElementInteraction->ZoomAreaQuant->ElCenter = EI_DecInt_convertIfNegative(temp,9);
    *noBitsRead += EI_DecInt_readBits(h_bitstream,&temp,10);
    ElementInteraction->ZoomAreaQuant->El = (int)temp;
  }

  *noBitsReadTotal += *noBitsRead;
  ElementInteraction->noBits = *noBitsRead;
  return 0;
}



int EI_getElementInteractionData(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_ELEMENT_INTERACTION_DATA *ElementInteractionData)
{
	/**
	*   Get ElementInteractionData that was read from the bitstream (1 frame)
	*   @param    ElementInteraction	      : element interaction handle that contains the data read from the bitstream/interface
	*   @param    ElementInteractionData		: output parameter, returns pointer to a public struct for the data from the bitstream/interface,
  *                                         has to point to NULL for initialization
	*
	*   @return   ERROR_CODE
	*/
  int k = 0;
  H_ELEMENT_INTERACTION_DATA temp = NULL;
  H_ZOOM_AREA temp_zoom = NULL;
  H_GROUP_INTERACTIVITY_STATUS *groupInteraction = NULL;
  int error;

  if (*ElementInteractionData != NULL)
  {
    error = EI_freeElementInteractionData(*ElementInteractionData);
  }

  temp = (H_ELEMENT_INTERACTION_DATA)calloc(1, sizeof(struct _ElementInteractionData));
  temp_zoom = (H_ZOOM_AREA)calloc(1,sizeof(struct _ZoomArea));

  temp->signature = (char*)calloc(MAX_LENGTH_SIGNATURE,sizeof(char));
  
  for (k = 0; k < MAX_LENGTH_SIGNATURE; k++)
  {
    temp->signature[k] = ElementInteraction->signature[k];
    if (ElementInteraction->signature[k] == '\0')
    {
      break;
    }
  }
   
  temp->signature_type = ElementInteraction->signature_type;
  temp->interactionMode = ElementInteraction->interactionMode;
  temp->numGroups = ElementInteraction->numGroups;
  groupInteraction = (H_GROUP_INTERACTIVITY_STATUS*)calloc(temp->numGroups,sizeof(H_GROUP_INTERACTIVITY_STATUS));
  
  temp->groupPresetID = ElementInteraction->groupPresetID;
  for (k=0; k < temp->numGroups; k++)
  {
    groupInteraction[k] = (H_GROUP_INTERACTIVITY_STATUS)calloc(1,sizeof(struct _GroupInteraction));
    groupInteraction[k]->groupID = ElementInteraction->GroupInteraction[k]->groupID;
    groupInteraction[k]->onOff = ElementInteraction->GroupInteraction[k]->onOff;
    groupInteraction[k]->routeToWireID = ElementInteraction->GroupInteraction[k]->routeToWireID;
    groupInteraction[k]->azOffset = ElementInteraction->GroupInteraction[k]->azOffset;
    groupInteraction[k]->elOffset = ElementInteraction->GroupInteraction[k]->elOffset;
    groupInteraction[k]->distFactor = ElementInteraction->GroupInteraction[k]->distFactor;
    groupInteraction[k]->gain_dB = ElementInteraction->GroupInteraction[k]->gain_dB;
  }
  temp->GroupInteraction = groupInteraction;

  temp_zoom->hasZoomArea = ElementInteraction->ZoomArea->hasZoomArea;
  temp_zoom->Az = ElementInteraction->ZoomArea->Az;
  temp_zoom->AzCenter = ElementInteraction->ZoomArea->AzCenter;
  temp_zoom->El = ElementInteraction->ZoomArea->El;
  temp_zoom->ElCenter = ElementInteraction->ZoomArea->ElCenter;

  temp->ZoomArea = temp_zoom;

  *ElementInteractionData = temp;
  return 0;
}

int EI_invquantizeData(ELEMENT_INTERACTION_HANDLE ElementInteraction)
{
  /**
	*   Inverse quantization of the data read from the bitstream
	*   @param    ElementInteraction        : element interaction handle that contains the data read from the bitstream/interface
  *
	*   @return   ERROR_CODE
	*/
  float tempfloat = 0.0f;
  int tempint = 0;
  int k = 0;

  for (k = 0; k < ElementInteraction->numGroups; k++)
  {
    ElementInteraction->GroupInteraction[k]->groupID = ElementInteraction->GroupInteractionQuant[k]->groupID;
    
    /* AzOffset */
    tempint = ElementInteraction->GroupInteractionQuant[k]->azOffset;
    tempfloat = (float)(tempint - 128)*1.5f;
    tempfloat = max(min(tempfloat,180.0f),-180.0f);
    ElementInteraction->GroupInteraction[k]->azOffset = tempfloat;

    /* ElOffset */
    tempint = ElementInteraction->GroupInteractionQuant[k]->elOffset;
    tempfloat = (float)(tempint -32)*3.0f;
    tempfloat = max(min(tempfloat,90.0f),-90.0f);
    ElementInteraction->GroupInteraction[k]->elOffset = tempfloat;

    /* DistFact */
    tempint = ElementInteraction->GroupInteractionQuant[k]->distFactor;
    tempfloat = (float)pow(2,(float)(tempint - 12));
    tempfloat = max(min(tempfloat,8.0f),0.00025f);
    ElementInteraction->GroupInteraction[k]->distFactor = tempfloat;

    /* Gain */
    tempint = ElementInteraction->GroupInteractionQuant[k]->gain_dB;
    if (tempint == 0)
    {
      ElementInteraction->GroupInteraction[k]->gain_dB = -1000;
    }
    else
    {
      tempint = tempint - 64;
      tempint = max(min(tempint,31),-63);
      ElementInteraction->GroupInteraction[k]->gain_dB = tempint;
    }
  }

  /* ZoomAzCenter */ 
  tempint = ElementInteraction->ZoomAreaQuant->AzCenter;
  tempfloat = (float)((int)tempint)*0.5f;
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  ElementInteraction->ZoomArea->AzCenter = tempfloat;

  /* ZoomAz */
  tempint = ElementInteraction->ZoomAreaQuant->Az;
  tempfloat = (float)(tempint)*0.125f;
  tempfloat = max(min(tempfloat,90.0f),0.0f);
  ElementInteraction->ZoomArea->Az = tempfloat;

  /* ZoomElCenter */ 
  tempint = ElementInteraction->ZoomAreaQuant->ElCenter;
  tempfloat = (float)((int)tempint)*0.5f;
  tempfloat = max(min(tempfloat,90.0f),-90.0f);
  ElementInteraction->ZoomArea->ElCenter = tempfloat;

  /* ZoomEl */
  tempint = ElementInteraction->ZoomAreaQuant->El;
  tempfloat = (float)(tempint)*0.125f;
  tempfloat = max(min(tempfloat,90.0f),0.0f);
  ElementInteraction->ZoomArea->El = tempfloat;

  return 0;
}




int EI_writeSignatureBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal)
{
  /**
	*   Writing of the signature to the bitstream
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*   @param    h_bitstream		        : bitstream handle
  *   @param    noBitsWritten		      : output parameter, number of written bits
  *   @param    noBitsWrittenTotal		: output parameter, adds the number of written bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned long no_BitsWritten = 0;
  int c = 0;
  unsigned int length_in_byte = 0; 

  if (ElementInteraction->signature != NULL)
  {
    int k = 0;
    for (k = 0; k < MAX_LENGTH_SIGNATURE; k++)
    {
      if (ElementInteraction->signature[k] == '\0')
      {
        length_in_byte = k+1;
        break;
      }
    }   
  }   
  
  no_BitsWritten += EI_DecInt_writeBits(h_bitstream, length_in_byte,8);
  if (length_in_byte > 0)
  {
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,0,8);
    for (c = 0; c < (int)length_in_byte; c++)
    {
      no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->signature[c],8);
    }
  }

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += no_BitsWritten;

  return 0;
}

int EI_writeElementInteractionBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal)
{
  /**
	*   Writing of the element interaction to the bitstream (ElementInteractionData() syntax)
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*   @param    h_bitstream		        : bitstream handle
  *   @param    noBitsWritten		      : output parameter, number of written bits
  *   @param    noBitsWrittenTotal		: output parameter, adds the number of written bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned long no_BitsWritten = 0;
  int k = 0;

  no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->interactionMode,1);
  no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->numGroups,7);
  if (ElementInteraction->interactionMode == 1)
  {
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->groupPresetID,5);
  }
  for (k = 0; k < ElementInteraction->numGroups; k++)
  {
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteraction[k]->groupID,7);
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteraction[k]->onOff,1);
    if (ElementInteraction->GroupInteraction[k]->routeToWireID > -1)
    {
      no_BitsWritten += EI_DecInt_writeBits(h_bitstream,1,1);
      no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteraction[k]->routeToWireID,16);
    }
    else
    {
      no_BitsWritten += EI_DecInt_writeBits(h_bitstream,0,1);
    }
    if (ElementInteraction->GroupInteraction[k]->onOff == 1)
    {
      if ((ElementInteraction->GroupInteraction[k]->azOffset != 0) || (ElementInteraction->GroupInteraction[k]->elOffset != 0) || (ElementInteraction->GroupInteraction[k]->distFactor != 1))
      {
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,1,1);
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteractionQuant[k]->azOffset,8);
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteractionQuant[k]->elOffset,6);
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteractionQuant[k]->distFactor,4);
      }
      else
      {
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,0,1);
      }
      if (ElementInteraction->GroupInteraction[k]->gain_dB != 0)
      {
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,1,1);
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->GroupInteractionQuant[k]->gain_dB, 7);
      }
      else
      {
        no_BitsWritten += EI_DecInt_writeBits(h_bitstream,0,1);
      }
    }
  }

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += *noBitsWritten;

  return 0;
}

int EI_writeZoomAreaBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal)
{
  /**
	*   Writing of the zoom area to the bitstream (hasLocalZoomAreaSize + LocalZoomAreaSize() syntax)
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*   @param    h_bitstream		        : bitstream handle
  *   @param    noBitsWritten		      : output parameter, number of written bits
  *   @param    noBitsWrittenTotal		: output parameter, adds the number of written bits to the previous value of the variable
	*
	*   @return   ERROR_CODE
	*/
  unsigned long no_BitsWritten = 0;
  int k = 0;

  if (ElementInteraction->ZoomArea->hasZoomArea == 1)
  {
    unsigned int temp;
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,1,1);
    temp = (unsigned int)ElementInteraction->ZoomAreaQuant->AzCenter;
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,temp,10);
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->ZoomAreaQuant->Az,10);
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->ZoomAreaQuant->ElCenter,9);
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream,(unsigned int)ElementInteraction->ZoomAreaQuant->El,10);
  }
  else
  {
    no_BitsWritten += EI_DecInt_writeBits(h_bitstream, 0,1);
  }

  *noBitsWritten = no_BitsWritten;
  *noBitsWrittenTotal += no_BitsWritten;

  return 0;
}

int EI_quantizeData(ELEMENT_INTERACTION_HANDLE ElementInteraction)
{
  /**
	*   Quantize the data for writing to the bitstream (conversion to the bitstream syntax)
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*
	*   @return   ERROR_CODE
	*/
  float tempfloat = 0.0f;
  int tempint = 0;
  int k = 0;

  for (k = 0; k < ElementInteraction->numGroups; k++)
  {
    ElementInteraction->GroupInteractionQuant[k]->groupID = ElementInteraction->GroupInteraction[k]->groupID;
    
    /* AzOffset */
    tempfloat = ElementInteraction->GroupInteraction[k]->azOffset;
    tempfloat = max(min(tempfloat,180.0f),-180.0f);
    tempint = (int)( floor(tempfloat/1.5f + 0.5f) + 128.0f);
    ElementInteraction->GroupInteractionQuant[k]->azOffset = tempint;

    /* ElOffset */
    tempfloat = ElementInteraction->GroupInteraction[k]->elOffset;
    tempfloat = max(min(tempfloat,90.0f),-90.0f);
    tempint = (int)( floor(tempfloat/3.0f + 0.5f) + 32.0f);
    ElementInteraction->GroupInteractionQuant[k]->elOffset = tempint;

    /* DistFact */
    tempfloat = ElementInteraction->GroupInteraction[k]->distFactor;
    tempfloat = max(min(tempfloat,8.0f),0.00025f);
    tempint = (int)( floor((log(tempfloat) / log(2.0f)) + 0.5) + 12.0f);
    ElementInteraction->GroupInteractionQuant[k]->distFactor = tempint;

    /* Gain */
    tempint = ElementInteraction->GroupInteraction[k]->gain_dB;
    tempint = max(min(tempint,31),-64);
    tempint = (int)( tempint + 64);
    ElementInteraction->GroupInteractionQuant[k]->gain_dB = tempint;

  }

  /* ZoomAzCenter */ 
  tempfloat = ElementInteraction->ZoomArea->AzCenter;
  tempfloat = max(min(tempfloat,180.0f),-180.0f);
  tempint = (int)((floor(tempfloat*2.0f + 0.5f) / 2.0f) * 2.0); /* + 360.0f); */
  ElementInteraction->ZoomAreaQuant->AzCenter = tempint;

  /* ZoomAz */
  tempfloat = ElementInteraction->ZoomArea->Az;
  tempfloat = max(min(tempfloat,90.0f),0.0f);
  tempint = (int)((floor(tempfloat*8.0f + 0.5f) / 8.0f) * 8.0);
  ElementInteraction->ZoomAreaQuant->Az = tempint;

  /* ZoomElCenter */ 
  tempfloat = ElementInteraction->ZoomArea->ElCenter;
  tempfloat = max(min(tempfloat,90.0f),-90.0f);
  tempint = (int)((floor(tempfloat*2.0f + 0.5f) / 2.0f) * 2.0); /* + 180.0f); */
  ElementInteraction->ZoomAreaQuant->ElCenter = tempint;

  /* ZoomEl */
  tempfloat = ElementInteraction->ZoomArea->El;
  tempfloat = max(min(tempfloat,90.0f),0.0f);
  tempint = (int)((floor(tempfloat*8.0f + 0.5f) / 8.0f) * 8.0);
  ElementInteraction->ZoomAreaQuant->El = tempint;

  return 0;
}

int EI_writeElementInteractionBitstream(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *no_BitsWritten)
{
  /**
	*   Writing of the element interaction (mpeg3daElementInteraction() syntax) once to the bitstream (for 1 frame)
	*   @param    ElementInteraction    : element interaction handle that contains the data to be written to the bitstream
	*   @param    h_bitstream		        : bitstream handle
  *   @param    noBitsWritten		      : output parameter, number of written bits
	*
	*   @return   ERROR_CODE
	*/
  unsigned long noBitsWritten = 0;
  unsigned long noBitsWrittenTotal = 0;
  int error = 0;

  /* quantize data */
  error = EI_quantizeData(ElementInteraction);

  /* write data */
  error = EI_writeSignatureBits(ElementInteraction,h_bitstream, &noBitsWritten,&noBitsWrittenTotal);
  error = EI_writeElementInteractionBits(ElementInteraction,h_bitstream,&noBitsWritten,&noBitsWrittenTotal);
  error = EI_writeZoomAreaBits(ElementInteraction,h_bitstream,&noBitsWritten,&noBitsWrittenTotal);
  
  *no_BitsWritten = noBitsWrittenTotal;
  ElementInteraction->noBits = noBitsWrittenTotal;

  return 0;
}
