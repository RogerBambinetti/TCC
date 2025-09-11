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
#ifndef OMF_READER_H
#define OMF_READER_H
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
#ifndef MAX_NUM_OBJECTS
#define MAX_NUM_OBJECTS (28)
#endif

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/*! return values for OMC reader */
enum OMF_READER_RETURN {
  OMF_READER_RETURN_NO_ERROR = 0,
  OMF_READER_RETURN_EOF,
  OMF_READER_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/
typedef struct OMF_READER * OMF_READER_HANDLE;

struct OMF_OBJECT {
  unsigned int num_objects;
  unsigned int has_object_metadata[MAX_NUM_OBJECTS];
};

struct OMF_PACKET {
  unsigned int numObjectGroups;
  struct OMF_OBJECT objectGroup[MAX_AUDIO_GROUPS];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! opens OMF reader */
OMF_READER_HANDLE omfReader_Open(
  char const *   const filename
);

/*! closes OMF reader */
void omfReader_Close(
  OMF_READER_HANDLE * const omfReaderHandle
);

  /*! read OMF data from file */
enum OMF_READER_RETURN omfReader_readFrame(
  OMF_READER_HANDLE    const omfReader,         /**< OMF reader handle */
  struct OMF_PACKET  * const omfPacket,         /**< OMF packet struct to read to */
  unsigned int         const numObjectGroups,
  unsigned int const * const num_objects
);

#endif /* ifndef OMC_READER_H */
