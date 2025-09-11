/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2019.

*************************************************************************/
#ifndef PMC_READER_H
#define PMC_READER_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#ifndef MAX_AUDIO_GROUPS
#define MAX_AUDIO_GROUPS (32)
#endif
#define MAX_NUM_BYTES_PMC_PACKET_HEADER 8   /* contains numObjectGroups and numChannelGroups as int */
#define MAX_NUM_BYTES_SUPPORTED_PMC_PACKET (MAX_NUM_BYTES_PMC_PACKET_HEADER + ((7 + 8 + MAX_AUDIO_GROUPS + MAX_AUDIO_GROUPS) >> 3))

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for PMC reader */
enum PMC_READER_RETURN {
  PMC_READER_RETURN_NO_ERROR = 0,
  PMC_READER_RETURN_EOF,
  PMC_READER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
typedef struct PMC_READER * PMC_READER_HANDLE;

struct PMC_PACKET {
  size_t pmcPacketLength;
  unsigned char pmcPacketPayload[MAX_NUM_BYTES_SUPPORTED_PMC_PACKET];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens PMC reader */
PMC_READER_HANDLE pmcReader_Open(
  char const * const filename
);

/*! closes PMC reader */
void pmcReader_Close(
  PMC_READER_HANDLE * const pmcReaderHandle
);

  /*! read PMC data from file */
enum PMC_READER_RETURN pmcReader_readConfig(
  PMC_READER_HANDLE   const pmcReader,          /**< PMC reader handle */
  struct PMC_PACKET * const pmcPacket           /**< PMC packet struct to read to */
);

#endif /* ifndef PMC_READER_H */
