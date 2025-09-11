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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ascparser_stream_wrap.h"
#include "ascparser_parse.h"
#include "ascparser_mhas.h"

static const int ChannelConfig_to_Channels[9]   = { 0, 1, 2, 3, 4, 5, 6, 8, 0 };
static const int FrequencyIndex_to_FrequencyAAC[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350 };
static const int FrequencyIndex_to_FrequencyMP3[2][4] = { { 22050, 24000, 16000, -1 }, { 44100, 48000, 32000, -1 } };
static const int BitrateIndexTable[16] = { -1, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 1920000, 224000, 256000, 320000, -1};

static int detect_input_filetype(const char* filename);

#define BYTES_TO_READ 1024*4

#ifndef MIN
#define MIN(_x,_y) ( (_x) < (_y) ? (_x) : (_y) )
#endif

#ifndef MINMAX
#define MINMAX(_val, _min,_max) ( (_val) < (_max) ? ( (_val) > (_min) ? (_val) : (_min) ) : (_max) )
#endif


ASCPARSER_BITSREAM* ascparserOpenInputStream(const char* filename)
{

  


  ASCPARSER_BITSREAM* tmp_handle = NULL;
  int file_opened = 0;

  tmp_handle = (ASCPARSER_BITSREAM*)calloc(sizeof(ASCPARSER_BITSREAM), 1);
  if (!tmp_handle) {
    printf("\n out of memory\n");
    return NULL;
  }

  /* check for input file type (simple auto-detection) */
  tmp_handle->m_detected_filetype = detect_input_filetype(filename);

  /* try to open it */
  switch (tmp_handle->m_detected_filetype) {

  case SIW_MP4FILE:
    {
      ISOErr err;
      unsigned int outType = 0;
      unsigned int tmp_track_count = 0;

      err = ISOOpenMovieFile( &tmp_handle->m_mp4Movie, filename, MP4OpenMovieNormal );     if (err) break;
      err = ISOGetMovieTrackCount( tmp_handle->m_mp4Movie, &tmp_track_count);              if (err) break;
      if (tmp_track_count > 1) {
        fprintf(stderr, "\n!!ERROR!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        fprintf(stderr, "\n!!ERROR!!  Input MP4 file contains more than one track!");
        fprintf(stderr, "\n!!         Please make sure the input MP4 contains only one track with audio and no video.");
        fprintf(stderr, "\n!!ERROR!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
        break;
      }
      err = ISOGetMovieTimeScale( tmp_handle->m_mp4Movie, &tmp_handle->m_movietimescale);  if (err) break;
      err = ISOGetMovieIndTrack( tmp_handle->m_mp4Movie, 1, &tmp_handle->m_mp4Track );     if (err) break;
      err = ISOGetTrackMedia( tmp_handle->m_mp4Track, &tmp_handle->m_mp4Media);            if (err) break;
      err = ISOGetMediaHandlerDescription( tmp_handle->m_mp4Media, &outType, NULL );       if (err) break;
      err = ISOCreateTrackReader( tmp_handle->m_mp4Track, &tmp_handle->m_mp4TrackReader ); if (err) break;
      err = ISONewHandle( 0, &tmp_handle->m_mp4SampleH );                                  if (err) break;

      tmp_handle->m_sampleNr = 1;   /* sample numbering in libisomedia starts with 1 */
      err = ISOGetMediaSampleCount(
                                   tmp_handle->m_mp4Media,
                                   &tmp_handle->m_sampleCount
                                   );

      err = ISOGetMediaTimeScale( 
                                 tmp_handle->m_mp4Media, 
                                 &tmp_handle->m_mediatimescale
                                 );

      /* check sample description type */
        {
          ISOHandle tmpSampleDescriptionH;
          u32 sampleDescriptionType = 0;
          u32 dummy = 0;

          err = ISONewHandle( 0, &tmpSampleDescriptionH );
          if (err) break;

          /* returns the atom below stsd: mp4a or mha1 */
          err = MP4GetMediaSampleDescription( tmp_handle->m_mp4Media, 1, tmpSampleDescriptionH, &dummy);
          if (err) break;
 
          err = ISOGetSampleDescriptionType(tmpSampleDescriptionH, &sampleDescriptionType);
          if (err) break;

          switch (sampleDescriptionType) {

          case SIW_MP4A:
            tmp_handle->m_sampleDescriptionType = SIW_CFGTYPE_MP4A;
            file_opened = 1;
            break;

          case SIW_M4AE:
            tmp_handle->m_sampleDescriptionType = SIW_CFGTYPE_M4AE;
            file_opened = 1;
            break;

          case SIW_MHA1:
            tmp_handle->m_sampleDescriptionType = SIW_CFGTYPE_MHA1;
            file_opened = 1;
            break;

          case SIW_MHM1:
            tmp_handle->m_sampleDescriptionType = SIW_CFGTYPE_MHM1;
            file_opened = 1;
            break;

          default:
            err = SIW_COULDNOTREAD_MP4MEDIA;
            break;
          }
        }
    }
    break;

  case SIW_MHAS:
  case SIW_MHAS2:
    tmp_handle->m_fh = fopen(filename, "rb");
    file_opened = 1;
    break;

  case SIW_NOFILE:
    printf("\ninput file <%s> ?\n", filename);
    return NULL;
    break;

  case SIW_UNKNOWN:
  default:
    printf("\nunsupported file type\n");
    return NULL;
    break;
  }

  /* sanity chk (if we could not open the file) */
  if (!file_opened) {
    printf("\n file open failed!\n");
    ascparserCloseInputStream(tmp_handle);
    return NULL;
  }

  return tmp_handle;
}


void ascparserCloseInputStream(ASCPARSER_BITSREAM* handle)
{
  if (handle) {

    if (handle->m_fh)
      fclose(handle->m_fh);
    handle->m_fh = 0;


    if (handle->m_mp4SampleH)
      ISODisposeHandle(handle->m_mp4SampleH);
    handle->m_mp4SampleH = 0;

    if (handle->m_mp4TrackReader)
      ISODisposeTrackReader(handle->m_mp4TrackReader);
    handle->m_mp4TrackReader = 0;

    if (handle->m_mp4Movie)
      ISODisposeMovie(handle->m_mp4Movie);
    handle->m_mp4Movie = 0;

    handle->m_detected_filetype = SIW_UNKNOWN;
    free(handle);
  }
}

int ascParserCheckAudioSceneConfigExtension(const ascparser_ASCPtr asc, int* asi_config_present)
{
  int err = SIW_SUCCESS;
  unsigned int i;
  int asi_present = 0;

  for ( i = 0; i < asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtNumConfExt; ++i )
  {
    if (asc->m_usacConfig.m_usacConfigExtension.m_usacConfigExtType[i] == ASCPARSER_CONFIG_EXT_AUDIOSCENE_INFO)
    {
      asi_present = 1;
      break;
    }
  }

  if (NULL != asi_config_present) {
    *asi_config_present = asi_present;
  }

  return err;
}

/* read asi as mhas packet */
int ascParserReadAudioScenePacket(ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc, int* asi_mhas_present)
{
  int err = SIW_SUCCESS;

  if (!hInput) return SIW_INVALID_HANDLE;
  if (!p_asc)  return SIW_INVALID_HANDLE;

  switch(hInput->m_detected_filetype) {
  case SIW_MHAS:
  case SIW_MHAS2:
    {
      unsigned char tmp_buf[8192] = {0};
      size_t bytesRead = fread(tmp_buf, 1, sizeof(tmp_buf), hInput->m_fh);

      err = ascparser_mhas_getAudioSceneInfo(tmp_buf, sizeof(tmp_buf), p_asc, (SIW_MHAS == hInput->m_detected_filetype) ? 1 : 0, asi_mhas_present);

      fseek(hInput->m_fh, -bytesRead, SEEK_CUR);
    }

    break;

  case SIW_MP4FILE:
    break;

  default:
    fprintf(stderr, "unknown file type in ascParserReadAudioScenePacket()\n");
    return SIW_UNKNOWN_FILETYPE;
  }

  return err;
}

int ascparserReadConfig( ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc)
{
  int err = SIW_SUCCESS;

  if (!hInput) return SIW_INVALID_HANDLE;
  if (!p_asc)  return SIW_INVALID_HANDLE;


  switch(hInput->m_detected_filetype) {
  case SIW_MP4FILE:
    {
      ISOErr err;
      ISOMedia media;
      ISOHandle decoderConfigH;
      ISOHandle dummyH;
      UINT32 ASClen = 0;
      UINT32 dummy;
      UINT32 avgBitrate = 0;


      err = ISOGetTrackMedia( hInput->m_mp4Track, &media ); if (err) return SIW_COULDNOTREAD_MP4MEDIA;
      err = ISONewHandle( 0, &decoderConfigH );             if (err) return SIW_COULDNOTREAD_MP4MEDIA;
      err = ISONewHandle(0, &dummyH);                       if (err) return SIW_COULDNOTREAD_MP4MEDIA;

      switch (hInput->m_sampleDescriptionType)
      {
      case SIW_CFGTYPE_MP4A:
      case SIW_CFGTYPE_M4AE:
        err = MP4GetMediaDecoderInformation( media,
                                             1,
                                             &dummy,                        /* object type */
                                             &dummy,                        /* stream type */
                                             &dummy,                        /* buffer size */
                                             &dummy,                        /* upstream flag */
                                             &dummy,                        /* max bitrate */
                                             &avgBitrate,                   /* avg bitrate */
                                             decoderConfigH
                                             ); if (err) return SIW_COULDNOTREAD_MP4MEDIA;

        err = ISOGetHandleSize(decoderConfigH, &ASClen);
        if (err) {
          return SIW_COULDNOTREAD_MP4MEDIA;
        }

        err = ascparser_ascParse( decoderConfigH[0], ASClen, p_asc);
        if (err) {
          return SIW_COULDNOTREAD_MP4MEDIA;
        }

        break;

      case SIW_CFGTYPE_MHA1:
      case SIW_CFGTYPE_MHM1:
        {
          UINT32 cfgversion = 0;
          UINT32 mp4ProfileLevelIndication = 0;
          ISOHandle profileLevelCompatibleSetsH;
          UINT8 numProfileLevelCompatibleSets = 0;
          int j = 0;
          ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET compatibleProfileLevelSet;
          int compatibleProfileLevelSetAvailable;

          err = ISONewHandle(0, &profileLevelCompatibleSetsH);             if (err) return SIW_COULDNOTREAD_MP4MEDIA;
          memset(&compatibleProfileLevelSet, 0, sizeof(ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET));

          err = MHAGetMediaDecoderInformation(
                                               hInput->m_mp4Media,          /* media handle */
                                               1,                           /* sample descriptor index */
                                               &cfgversion,
                                               &mp4ProfileLevelIndication,  /* mpegh profileLevel */
                                               &dummy,                      /* channelLayout */
                                               &dummy,                      /* buffer size */
                                               &dummy,                      /* max bitrate */
                                               &dummy,                      /* avg bitrate */
                                               &numProfileLevelCompatibleSets,/* num profile and level compatible sets */
                                               profileLevelCompatibleSetsH,   /* profile and level compatible sets */
                                               decoderConfigH
                                              ); if (err) return SIW_COULDNOTREAD_MP4MEDIA;

          err = ISOGetHandleSize(decoderConfigH, &ASClen);
          if (err) {
            return SIW_COULDNOTREAD_MP4MEDIA;
          }

          if (hInput->m_sampleDescriptionType == SIW_CFGTYPE_MHM1 && ASClen == 0) {
            p_asc->m_compatibleProfileLevelSet.numCompatibleSets = numProfileLevelCompatibleSets;

            for (j = 0; j < numProfileLevelCompatibleSets; j++) {
                p_asc->m_compatibleProfileLevelSet.CompatibleSetIndication[j] = *(profileLevelCompatibleSetsH)[j];
            }
            return err;
          }

          err = ascparser_mpeghConfigParse( decoderConfigH[0], ASClen, p_asc);
          if (err) {
            return SIW_COULDNOTREAD_MP4MEDIA;
          }

          if ( p_asc->m_usacConfig.m_mpegh3daProfileLevelIndication != mp4ProfileLevelIndication ) {
            fprintf(stderr,"\n\nWarning: ProfileLevelIndication mismatch!\n\tmp4ProfileLevelIndication\t0x%X\n\tmpegh3daProfileLevelIndication\t0x%X!\n\n", mp4ProfileLevelIndication,p_asc->m_usacConfig.m_mpegh3daProfileLevelIndication);
          }

          err = ascparser_ASC_FillCompatibleProfileLevelSet(p_asc, &compatibleProfileLevelSet, &compatibleProfileLevelSetAvailable);
          if (compatibleProfileLevelSetAvailable == 1) {
              if (numProfileLevelCompatibleSets == 0) {
                  fprintf(stderr, "\n\nWarning: Compatible Profile Level Set is only present in bitstream!\n\n");
        }
              else {
                  for (j = 0; j < compatibleProfileLevelSet.numCompatibleSets; j++) {
                      if (compatibleProfileLevelSet.CompatibleSetIndication[j] != *(profileLevelCompatibleSetsH)[j]) {
                          fprintf(stderr, "\n\nWarning: compatibleSetIndication mismatch!\n\tmp4CompatibleSetIndication\t0x%X\n\tmpegh3daCompatibleSetIndication\t0x%X!\n\n", *(profileLevelCompatibleSetsH)[j], compatibleProfileLevelSet.CompatibleSetIndication[j]);
                      }
                  }
              }
          }
          else if (numProfileLevelCompatibleSets > 0 && !compatibleProfileLevelSetAvailable) {
              return SIW_COULDNOTREAD_MP4MEDIA;
          }

          err = ISODisposeHandle(profileLevelCompatibleSetsH); if (err) return SIW_COULDNOTREAD_MP4MEDIA;
        }
        break;
      }

      err = ISODisposeHandle(decoderConfigH); if (err) return SIW_COULDNOTREAD_MP4MEDIA;
      err = ISODisposeHandle(dummyH); if (err) return SIW_COULDNOTREAD_MP4MEDIA;
    }
    break;

  case SIW_MHAS:
  case SIW_MHAS2:
    {
      unsigned char tmp_buf[8192];
      size_t bytesRead = fread(tmp_buf, 1, sizeof(tmp_buf), hInput->m_fh);

      err = ascparser_mhas_getConfig(tmp_buf, sizeof(tmp_buf), p_asc, (SIW_MHAS == hInput->m_detected_filetype) ? 1 : 0);

      fseek(hInput->m_fh, -bytesRead, SEEK_CUR);
    }
    break;

  default:
    fprintf(stderr, "unknown file type in ascparserReadConfig()\n");
    return SIW_UNKNOWN_FILETYPE;
  }

  return err;
}

int ascParserReadLoudnessPacket(ASCPARSER_BITSREAM* hInput, ascparser_ASC* p_asc, int* loudnessMhasPresent)
{
  int err = SIW_SUCCESS;

  if (!hInput) return SIW_INVALID_HANDLE;
  if (!p_asc)  return SIW_INVALID_HANDLE;

  switch(hInput->m_detected_filetype) {
  case SIW_MHAS:
  case SIW_MHAS2:
    {
      unsigned char tmp_buf[8192];
      size_t bytesRead = fread(tmp_buf, 1, sizeof(tmp_buf), hInput->m_fh);

      err = ascparser_mhas_getLoudnessInfoSet(tmp_buf, sizeof(tmp_buf), p_asc, (SIW_MHAS == hInput->m_detected_filetype) ? 1 : 0, loudnessMhasPresent);

      fseek(hInput->m_fh, -bytesRead, SEEK_CUR);
    }

    break;

  case SIW_MP4FILE:
    break;

  default:
    fprintf(stderr, "unknown file type in ascParserReadLoudnessInfoSetPacket()\n");
    return SIW_UNKNOWN_FILETYPE;
  }

  return err;
}

int ascparserGetNextAccessUnit( ASCPARSER_BITSREAM* hInput, unsigned char* pAccessUnit, unsigned int* pAccessUnitSize)
{
  if (!hInput)          return SIW_INVALID_HANDLE;
  if (!pAccessUnit)     return SIW_INVALID_HANDLE;
  if (!pAccessUnitSize) return SIW_INVALID_HANDLE;
  *pAccessUnitSize = 0;

  switch(hInput->m_detected_filetype) {
  case SIW_MP4FILE:
    {
      /*
        MP4TrackReaderascparserGetNextAccessUnit() does not correctly consider pre-roll; 
        Use ISOGetIndMediaSample() which outputs all AUs, and do the cutting in 
        time-domain when writing the samples
      */
      ISOErr err;
      s32 cts = 0;
      u64 dts = 0;
      u32 sampleFlags = 0;
      u64 duration = 0;
      u32 outSampleDescIndex = 0;

      if (hInput->m_sampleNr > hInput->m_sampleCount) {
        hInput->m_eof = 1;
        return SIW_EOF;
      }

      err = ISOGetIndMediaSample(
                                 hInput->m_mp4Media,
                                 hInput->m_sampleNr,
                                 hInput->m_mp4SampleH,
                                 &hInput->m_mp4SampleH_size,
                                 &dts,
                                 &cts,
                                 &duration,
                                 &sampleFlags,
                                 &outSampleDescIndex
                                 );
      if (err) {

        /* return in case of any non-eof error*/
        if (err != MP4EOF) {
          fprintf(stderr, "\nlibisomedia error %d\n", err);
          return err;
        }

        /* return eof for this track */
        return err;
      }

      hInput->m_sampleNr++;

      memcpy(pAccessUnit, *hInput->m_mp4SampleH, hInput->m_mp4SampleH_size);
      *pAccessUnitSize = hInput->m_mp4SampleH_size;

    }
    break;

  default:
    fprintf(stderr, "unknown file type in GetNextAU()\n");
    return SIW_UNKNOWN_FILETYPE;
  }

  return SIW_SUCCESS;
}


static int detect_input_filetype(const char* filename)
{
  FILE* fh;
  ISOMovie theMovie;
  unsigned char tmp_buf[8192] = {0};

  fh = fopen(filename, "rb");

  if (!fh) {
    return SIW_NOFILE;
  }

  fread(tmp_buf, 1, sizeof(tmp_buf), fh);      /* don't mind, if we could not read complete buffer */

  /* mp4 */
  if (ISONoErr == ISOOpenMovieFile(&theMovie, filename, MP4OpenMovieNormal)) {
    ISODisposeMovie(theMovie);
    printf("input file seems to be MP4\n");
    fclose(fh);
    return SIW_MP4FILE;
  }
  
  /* mhas */
  if (0 == ascparser_mhas_searchSync(tmp_buf, sizeof(tmp_buf), 1)) {
    fclose(fh);
    return SIW_MHAS;
  }

  /* mhas wo magic string */
  if (0 == ascparser_mhas_searchSync(tmp_buf, sizeof(tmp_buf), 0)) {
    fclose(fh);
    return SIW_MHAS2;
  }

  fclose(fh);

  return SIW_UNKNOWN;
}
