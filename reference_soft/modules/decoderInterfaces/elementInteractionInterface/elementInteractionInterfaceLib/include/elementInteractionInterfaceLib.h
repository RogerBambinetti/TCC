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

#ifndef MAX_LENGTH_SIGNATURE
  #define MAX_LENGTH_SIGNATURE 255
#endif

#include "elementInteractionInterfaceLib_API.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef struct _ZoomAreaQuant
{
  int AzCenter;
  int Az;
  int ElCenter;
  int El;

} *H_ZOOM_AREA_QUANT;

typedef struct _GroupInteractionQuant
{
  int groupID;
  int azOffset;
  int elOffset;
  int distFactor;
  int gain_dB;

} *H_GROUP_INTERACTIVITY_STATUS_QUANT;

struct _ElementInteractionHandle
{
  int signature_type;
  char *signature;
  int interactionMode;
  int numGroups;
  int numAllocatedGroups;
  int groupPresetID;
  int noBits;

  H_GROUP_INTERACTIVITY_STATUS *GroupInteraction;
  H_GROUP_INTERACTIVITY_STATUS_QUANT *GroupInteractionQuant;
  H_ZOOM_AREA ZoomArea;
  H_ZOOM_AREA_QUANT ZoomAreaQuant;

};

int EI_quantizeData(ELEMENT_INTERACTION_HANDLE ElementInteraction);
int EI_writeSignatureBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int EI_writeElementInteractionBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int EI_writeZoomAreaBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_ENCODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int EI_readElementInteractionBits(ELEMENT_INTERACTION_HANDLE ElementInteraction, H_EI_BITSTREAM_DECODER h_bitstream, unsigned long *noBitsWritten, unsigned long *noBitsWrittenTotal);
int EI_invquantizeData(ELEMENT_INTERACTION_HANDLE ElementInteraction);


