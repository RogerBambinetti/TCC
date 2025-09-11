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

#ifndef _streamfile_h_
#define _streamfile_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_descr.h"           /* structs */

#define MAXPROG (8)
#define MAXTRACK (50)

/* ---- enum & consts ---- */
typedef enum { FILETYPE_AUTO=0, FILETYPE_MP4, FILETYPE_LATM, FILETYPE_MHAS, FILETYPE_MHAS2, FILETYPE_FL4, FILETYPE_RAW, FILETYPE_UNKNOWN } StreamFileType;

typedef enum { STSS_SYNCSAMPLE_NOTSET, STSS_SYNCSAMPLE_SET, STSS_SYNCSAMPLE_UNDEFINED } StreamFileSyncSampleState;

/* ---- Handles ---- */
typedef struct tagStreamFile*           HANDLE_STREAMFILE;
typedef struct StreamProgram*           HANDLE_STREAMPROG;
typedef struct StreamAccessUnit*        HANDLE_STREAM_AU;

/* ---- Structs ---- */
struct StreamProgram {
  unsigned long                 trackCount;
  unsigned long                 allTracks;
  long                          dependencies[MAXTRACK];
  DEC_CONF_DESCRIPTOR           decoderConfig[MAXTRACK];
  AUDIO_TRUNCATION_INFO         audioTruncationinfo[MAXTRACK];
  StreamFileSyncSampleState     stssSignalsSyncSample[MAXTRACK];
  COMPATIBLE_PROFILE_LEVEL_SET  compatibleProfileLevelSet[MAXTRACK];

  /* internal information */
  struct tagProgramData*      programData;
  HANDLE_STREAMFILE           fileData;
};

struct StreamAccessUnit {
  unsigned long               numBits;
  unsigned char*              data;
};


/* ---- functions ---- */

/* StreamFileShowHelp()
     Show information about the known file formats and their options
*/
void StreamFileShowHelp( void );


/* ---- HANDLE_STREAMFILE related ---- */

/* StreamFileOpenRead(), StreamFileOpenWrite() 
     filename: prepare and open a stream at this path
     type    : one of the above; force a type or try to detect automatically
     options : string containing requested low-level options, NULL for defaults
     return a valid HANDLE_STREAMFILE
*/
HANDLE_STREAMFILE StreamFileOpenRead(char *filename, StreamFileType type);
HANDLE_STREAMFILE StreamFileOpenWrite(char *filename, StreamFileType type, char *options);

/* StreamFileClose()
     stream: a valid handle
     return 0 on success
*/
int StreamFileClose(HANDLE_STREAMFILE stream);


/* ---- HANDLE_STREAMPROG related ---- */

/* StreamFileAddProgram()
     stream: a valid handle
     return a reference to the new program for editing
*/
HANDLE_STREAMPROG StreamFileAddProgram(HANDLE_STREAMFILE stream);

/* StreamFileGetProgram()
     stream: a valid handle
     progNr: number of the desired program (starting at 0)
     return the specified program for further use
*/
HANDLE_STREAMPROG StreamFileGetProgram(HANDLE_STREAMFILE stream, int progNr);

/* StreamFileGetProgramCount()
     stream: a valid handle
     return the number of programs in the stream
*/
int StreamFileGetProgramCount(HANDLE_STREAMFILE stream);

/* StreamFileFixProgram()
     prog: a valid handle
     fix some known values in the program (dependsOnCoreCoder flags and similar)
     return the number of fixed values
*/
int StreamFileFixProgram(HANDLE_STREAMPROG prog);


/* ---- track related ---- */
int StreamFileAddTrackToProg(HANDLE_STREAMPROG prog);
int StreamFileSetupTrack(HANDLE_STREAMPROG prog, int track, int aot, int bitrate, int avgBitrate, int samplerate, int channels);

/* ---- HANDLE_STREAM_AU related ---- */

/* StreamFileAllocateAU()
     numBits: initial size of the access unit
     return a new and valid access unit
*/
HANDLE_STREAM_AU StreamFileAllocateAU(unsigned long numBits);

/* StreamFile_AUresize()
     au: access unit to resize
     numBits: new size of the access unit
     return 1 on success, 0 if an error occured
*/
int StreamFile_AUresize(HANDLE_STREAM_AU au, const long numBits);

/* StreamFileFreeAU()
     free the memory of a access unit
*/
void StreamFileFreeAU(HANDLE_STREAM_AU au);

/* StreamGetAccessUnit()
     get next access unit from program
     prog   : a valid handle where to get the AU from
     trackNr: specify track inside the program
     au     : valid access unit; au->data will be realloc()ed
     return 0: OK; -1: error; -2: EOF
*/
int StreamGetAccessUnit(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU au);

/* StreamGetAudioTruncationInfo()
     get next audioTruncationInfo from program (Only applicable for MHAS streams)
     prog   : a valid handle where to get the AU from
     trackNr: specify track inside the program
     ati    : valid audioTruncationInfo
     return 0: OK; -1: error;
*/
int StreamGetAudioTruncationInfo(HANDLE_STREAMPROG stream, int trackNr, HAUDIO_TRUNCATION_INFO ati);

/* StreamPutAccessUnit()
     append access unit in program
     prog   : a valid handle where to put the AU
     trackNr: specify track inside the program
     au     : a valid access unit containing the data
     return error code or 0
*/
int StreamPutAccessUnit(HANDLE_STREAMPROG stream, int trackNr, HANDLE_STREAM_AU au);

/* StreamAUsPerFrame()
     get number of access units to complete a full frame
     prog   : a valid handle where to query info from
     trackNr: specify track inside the program
     return error code or the number of access units to complete a full frame
*/
int StreamAUsPerFrame(HANDLE_STREAMPROG prog, int track);

/* StreamFileGetEditlist()
     get editlistdata from MP4FF
     return error code or 0
*/
int StreamFileGetEditlist(HANDLE_STREAMPROG stream, int trackNr, double *startOffset, double *playTime);

int StreamFileSetEditlist(HANDLE_STREAMPROG stream, int trackNr, int bSampleGroupElst, int rollDistance);

int StreamFileSetEditlistValues(HANDLE_STREAMPROG stream, int nDelay, int sampleRateOut, int originalSampleCount, 
                                int originalSamplingRate, int sampleGroupStartOffset, int sampleGroupLength);

#endif
