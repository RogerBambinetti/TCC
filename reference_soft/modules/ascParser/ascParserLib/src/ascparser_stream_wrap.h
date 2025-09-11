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
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef __ASCPARSER_STREAM_WRAP_H__
#define __ASCPARSER_STREAM_WRAP_H__

#include <stdio.h>

#include "ascparser_asc.h"
#include "MPEGHMovies.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SIW_CFGTYPE_UNKNOWN = 0,
    SIW_CFGTYPE_MP4A,
    SIW_CFGTYPE_M4AE,
    SIW_CFGTYPE_MHA1,
    SIW_CFGTYPE_MHM1,
} SIW_CFGTYPE;

#define SIW_MP4A   (0x6d703461)
#define SIW_M4AE   (0x6d346165)
#define SIW_MHA1   (0x6d686131)
#define SIW_MHM1   (0x6d686d31)

enum { 
  SIW_UNKNOWN = +1000,
  SIW_AUTODETECT,
  SIW_MP4FILE,
  SIW_LATMFILE,
  SIW_ADTSFILE,
  SIW_ADIFFILE,
  SIW_MP3FILE,
  SIW_MHAS,
  SIW_MHAS2 /* MHAS without magic string */
};

enum {
  SIW_SUCCESS = 0,

  SIW_INVALID_HANDLE = -10000,
  SIW_INVALIDDATA,
  SIW_NOFILE,
  SIW_EOF,
  SIW_INVALID_FILEHANDLE,
  SIW_UNKNOWN_FILETYPE,
  SIW_COULDNOTREAD_ADTSHEADER,
  SIW_COULDNOTREAD_CRC,
  SIW_COULDNOTREAD_FRAME,
  SIW_COULDNOTREAD_MP4MEDIA,
  SIW_COULDNOTREAD_EDITLIST,
  SIW_COULDNOTREAD_CFG,
  SIW_COULDNOTREAD_ASI,
  SIW_COULDNOTREAD_LOUDNESS,
  SIW_COULDNOTREAD_EARCON,
  SIW_ASIEXISTSTWICE

};

typedef struct {
  FILE*          m_fh;
  int            m_detected_filetype;
  int            m_eof;

  ISOMovie       m_mp4Movie;
  UINT32         m_movietimescale;
  ISOTrackReader m_mp4TrackReader;
  ISOTrack       m_mp4Track;
  ISOHandle      m_mp4SampleH;
  unsigned int   m_mp4SampleH_size;
  ISOMedia       m_mp4Media;
  SIW_CFGTYPE    m_sampleDescriptionType;   /* cfgtype */
  UINT32         m_mediatimescale;
  UINT32         m_sampleNr;         /* current idx */
  UINT32         m_sampleCount;      /* nr of samples/frames in the track */

} ASCPARSER_BITSREAM;


ASCPARSER_BITSREAM* ascparserOpenInputStream(const char* filename);
void       ascparserCloseInputStream(ASCPARSER_BITSREAM* handle);

int ascparserReadConfig(ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc);
int ascParserCheckAudioSceneConfigExtension(const ascparser_ASCPtr asc, int* asi_config_present);
int ascParserReadAudioScenePacket(ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc, int* asi_mhas_present);
int ascparserGetNextAccessUnit(ASCPARSER_BITSREAM* h, unsigned char* pAccessUnit, unsigned int* AccessUnitSize);
int ascParserReadLoudnessPacket(ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc, int* loudnessMhasPresent);

#ifdef __cplusplus
}
#endif

#endif
