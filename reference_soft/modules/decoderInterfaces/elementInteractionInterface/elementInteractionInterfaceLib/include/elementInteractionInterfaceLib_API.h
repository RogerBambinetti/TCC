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


#ifndef EI_API
#define EI_API

#ifndef NULL
  #define NULL 0
#endif

#define MAX_NUM_GROUPS 128

typedef struct _GroupInteraction
{
  int groupID;
  int onOff;
  int routeToWireID;
  float azOffset;
  float elOffset;
  float distFactor;
  int gain_dB;

} *H_GROUP_INTERACTIVITY_STATUS;

typedef struct _ZoomArea
{
  int hasZoomArea;
  float AzCenter;
  float Az;
  float ElCenter;
  float El;

} *H_ZOOM_AREA;

typedef struct _ElementInteractionData
{
  int signature_type;
  char *signature;
  int interactionMode;
  int numGroups;
  int groupPresetID;

  H_GROUP_INTERACTIVITY_STATUS *GroupInteraction;
  H_ZOOM_AREA ZoomArea;
} *H_ELEMENT_INTERACTION_DATA;


typedef struct _ElementInteractionHandle *ELEMENT_INTERACTION_HANDLE;
/* typedef struct _ElementInteractionData *H_ELEMENT_INTERACTION_DATA; */

typedef struct _eiBitstreamEncoder *H_EI_BITSTREAM_ENCODER;
typedef struct _eiBitstreamDecoder *H_EI_BITSTREAM_DECODER;

int EI_initHandle(ELEMENT_INTERACTION_HANDLE *h_ElementInteractionHandle);
int EI_closeHandle(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle);
int EI_setInteractionGroupConfig(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int *groupIDs, int numGroups);

int EI_setInteractionConfig(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int interactionMode, int groupPresetID, char *signature, int signatureType);
int EI_setZoomArea(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, H_ZOOM_AREA ZoomArea);
int EI_setInteractionGroupStatus(ELEMENT_INTERACTION_HANDLE h_ElementInteractionHandle, int groupID, H_GROUP_INTERACTIVITY_STATUS GroupStatus);

int EI_getElementInteractionData(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_ELEMENT_INTERACTION_DATA *ElementInteractionData);
int EI_freeElementInteractionData(H_ELEMENT_INTERACTION_DATA ElementInteractionData);

int EI_writeElementInteractionBitstream(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten);
int EI_readElementInteractionBitstream(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsRead);

int EI_getInteractionFromFile(char *filename, int **interactionMode, int **groupPresetID, H_ZOOM_AREA **h_ZoomArea_Enc, H_GROUP_INTERACTIVITY_STATUS ***h_GroupStatus_Enc, int **groupIDs, int* noGroups, int *noFrames, const int verboseLvl);

int EI_DecInt_initBitstreamWriter(H_EI_BITSTREAM_ENCODER *h_bitstream, char *file);
void EI_DecInt_closeBitstreamWriter(H_EI_BITSTREAM_ENCODER h_bitstream);
int EI_DecInt_initBitstreamReader(H_EI_BITSTREAM_DECODER *h_bitstream, char *file);
void EI_DecInt_closeBitstreamReader(H_EI_BITSTREAM_DECODER h_bitstream);

#endif