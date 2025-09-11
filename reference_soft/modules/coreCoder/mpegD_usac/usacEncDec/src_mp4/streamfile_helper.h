/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
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

Copyright (c) ISO/IEC 2003.

*************************************************************************/

#ifndef _STREAMFILE_HELPER_INCLUDED_
#define _STREAMFILE_HELPER_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum { STATUS_INVALID=0, STATUS_READING, STATUS_WRITING, STATUS_PREPARE_WRITE, STATUS_PREPARE_READ } StreamStatus;


/* handle for a FIFO buffer */
typedef struct tagFIFObuffer* FIFO_BUFFER;


/* struct for a whole bitstream file with multiple programs */
struct tagStreamFile {
  char*                         fileName;
  StreamFileType                type;
  StreamStatus                  status;
  int                           providesIndependentReading;

  int                           progCount;
  struct StreamProgram          prog[MAXPROG];

  struct tagStreamSpecificInfo* spec; /* specific format dependent info */

  /* functions */
  int (*initProgram)(HANDLE_STREAMPROG prog);

  int (*openRead)(HANDLE_STREAMFILE stream);
  int (*openWrite)(HANDLE_STREAMFILE stream, int argc, char** argv);
  int (*headerWrite)(HANDLE_STREAMFILE stream);
  int (*close)(HANDLE_STREAMFILE stream);

  int (*getDependency)(HANDLE_STREAMFILE stream, int trackID);
  int (*openTrack)(HANDLE_STREAMFILE stream, int trackID, int prog, int index);

  int (*getAU)(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU auStream);
  int (*putAU)(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU auStream);
  int (*getEditlist)(HANDLE_STREAMPROG prog, int trackNr, double *startOffset, double *playTime);
  int (*setEditlist)(HANDLE_STREAMPROG stream, int trackNr, int bSampleGroupElst, int rollDistance);
  int (*setEditlistValues)(HANDLE_STREAMPROG stream, int nDelay, int sampleRateOut, int originalSampleCount, 
                           int originalSamplingRate, int sampleGroupStartOffset, int sampleGroupLength);
};

/* handle for buffer simulation */
typedef struct t_fullness_sim *HANDLE_BUFFER_SIMULATION;

/* struct for internal variables necessary for one program */
struct tagProgramData {
  StreamStatus                status;
  unsigned long               sampleDuration[MAXTRACK];
  unsigned long               timePerFrame;
  unsigned long               timeThisFrame[MAXTRACK];
  unsigned long               timePerAU[MAXTRACK];
  FIFO_BUFFER                 fifo_buffer[MAXTRACK];
  HANDLE_BUFFER_SIMULATION    buffer_sim;

  struct tagProgSpecificInfo* spec; /* specific format dependent info */
};


/* helpers for the whole stream */
void setStreamStatus(HANDLE_STREAMFILE stream, StreamStatus status);

/* helpers for programs */
HANDLE_STREAMPROG newStreamProg(HANDLE_STREAMFILE stream);
void closeProgram(HANDLE_STREAMPROG prog);
int openTrackInProg(HANDLE_STREAMFILE stream, int trackID, int prog);

/* helper for dependencies */
int genericOpenRead(HANDLE_STREAMFILE stream, unsigned long trackCount);

/* helper for AUs */
int StreamFile_AUcopyResize(HANDLE_STREAM_AU au, const unsigned char* src, const long numBits);
void StreamFile_AUfree(HANDLE_STREAM_AU au);


/* FIFO helper */

FIFO_BUFFER FIFObufferCreate(int size);
void FIFObufferFree(FIFO_BUFFER fifo);

int FIFObufferPush(FIFO_BUFFER fifo, void *item);
void* FIFObufferPop(FIFO_BUFFER fifo);

void* FIFObufferGet(FIFO_BUFFER fifo, int idx);
int FIFObufferLength(FIFO_BUFFER fifo);

#endif
