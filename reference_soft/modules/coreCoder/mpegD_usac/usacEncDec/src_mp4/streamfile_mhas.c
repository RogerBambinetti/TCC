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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_descr.h"           /* structs */

#include "allHandles.h"
#include "common_m4a.h"          /* common module       */
#include "bitstream.h"           /* bit stream module   */
#include "flex_mux.h"            /* parse object descriptors */
#include "cmdline.h"             /* parse commandline options */

#include "mp4au.h"               /* frame work common declarations */


#include "streamfile.h"          /* HANDLE_STREAMFILE */
#include "streamfile_mhas.h"     /* public functions */
#include "streamfile_helper.h"    /* internal data */

#define MODULE_INFORMATION "StreamFile transport lib: MHAS module"

#define MAXBUFFER 128 
#define USE_MAGIC_FILE_PREFIX

#ifdef USE_MAGIC_FILE_PREFIX
static const char* bsopen_info_mhas = "\nMHAS";
#endif


#define CONFIG_WRITE_INTERVALL 8
#define SYNC_GAP_WRITE_INTERVALL 0


unsigned char MHAS_SyncWord       = 0xA5;
static const int  MHASdebugLevel  = 0;                  /* debug level */

int MHAS_HEADER_SIZES[3][3] = {
  { 3, 8, 8}, /* MHASPacketType */
  { 2, 8,32}, /* MHASPacketLabel */
  {11,24,24}  /* MHASPacketLength */
};

typedef enum {
  MHAS_PACTYP_FILLDATA          = 0,
  MHAS_PACTYP_MPEGH3DACFG       = 1,
  MHAS_PACTYP_MPEGH3DAFRAME     = 2,
  MHAS_PACTYP_SYNC              = 6,
  MHAS_PACTYP_SYNCGAP           = 7,
  MHAS_PACTYP_MARKER            = 8,
  MHAS_PACTYP_CRC16             = 9,
  MHAS_PACTYP_CRC32             = 10,
  MHAS_PACTYP_DESCRIPTOR        = 11,
  MHAS_PACTYP_USERINTERACTION   = 12,
  MHAS_PACTYP_LOUDNESS_DRC      = 13,
  MHAS_PACTYP_BUFFERINFO        = 14,
  MHAS_PACTYP_AUDIOTRUNCATION   = 17,
  MHAS_PACTYP_UNDEF             = 518
} mhas_packetType;


typedef struct {
  mhas_packetType PacType;
  unsigned int pacLabel;
  unsigned int pacLength;
} MHASPacketInfo;


struct tagStreamSpecificInfo {
  HANDLE_BSBITSTREAM bitStream;
  /* AUDIO_MUX_ELEMENT  audioMuxElement; */
  int first_frame;
};

struct tagProgSpecificInfo {
  /* Payload */
  FIFO_BUFFER   AUbuffer[MAXTRACK];
};


static int escapedValue(HANDLE_BSBITSTREAM bitStream, unsigned long *value, int nBits[3], int writeFlag) 
{
  unsigned long escVal[3] =
  {
    (1<<nBits[0])-1,
    (1<<nBits[1])-1,
    (1<<nBits[2])-1
  };
  unsigned long tmpVal;

  /* writeFlag - 0: read, 1:write */
  if ( writeFlag == 0 ) {
    int cnt=0;
    *value = 0;
    tmpVal = 0;
    do {
      BsRWBitWrapper(bitStream, &tmpVal, nBits[cnt], 0);
      *value += tmpVal;
      cnt++;
    } while (tmpVal == escVal[cnt-1]);
    
  } 
  else {
    int cnt=0;
    int interate=0;
    tmpVal = *value;
    do {
      if ( tmpVal < escVal[cnt] ) {
        BsRWBitWrapper(bitStream, &tmpVal, nBits[cnt], 1);
        tmpVal = 0;
        interate = 0;
      } else {
        BsRWBitWrapper(bitStream, &escVal[cnt], nBits[cnt], 1);
        tmpVal -= escVal[cnt];
        interate = 1;
      }
      cnt++;
    } while (interate && (cnt<=3));

    if (tmpVal) {
      CommonWarning("StreamFile:MHAS: Value too large for escapedValue! (%i)",*value);
      return -1;
    }
  }

  return 0;
}

static int mhasPacketHeader(HANDLE_BSBITSTREAM bitStream, MHASPacketInfo *pacInfo, int writeFlag)
{
  int err = 0;
  unsigned long localPacType   = pacInfo->PacType;
  unsigned long localPacLabel  = pacInfo->pacLabel;
  unsigned long localPacLength = pacInfo->pacLength;
  
  if ( BsEof ( bitStream, 8) ) return -2;

  err = (escapedValue(bitStream, &localPacType  , MHAS_HEADER_SIZES[0], writeFlag)!=0)||err?1:0;
  err = (escapedValue(bitStream, &localPacLabel , MHAS_HEADER_SIZES[1], writeFlag)!=0)||err?1:0;
  err = (escapedValue(bitStream, &localPacLength, MHAS_HEADER_SIZES[2], writeFlag)!=0)||err?1:0;

  if (writeFlag == 0) {
    pacInfo->PacType   = (mhas_packetType)localPacType;
    pacInfo->pacLabel  = localPacLabel;
    pacInfo->pacLength = localPacLength;
  }

  return err;
  
}

static int mhasAdvancePacket(HANDLE_BSBITSTREAM bitStream, const mhas_packetType pacType, const unsigned int paclabel, const unsigned long nBits, const unsigned char* data)
{
  int err = 0;
  unsigned long k;
  MHASPacketInfo pacInfo;

  pacInfo.PacType   = pacType;
  pacInfo.pacLabel  = paclabel;
  pacInfo.pacLength = ((nBits + 7) >> 3);

  if (NULL == data) {
    return -1;
  }

  err = mhasPacketHeader(bitStream, &pacInfo, 1);

  if (err == 0) {
    for (k = 0; k < pacInfo.pacLength; k++) {
      unsigned long tmp = (unsigned long)data[k];
      BsPutBit(bitStream, tmp, 8);
    }
  }

  return err;
}

static char* mapMhasPacketTypeToString(mhas_packetType  const MHASPacketType) {

  switch (MHASPacketType) {
    case MHAS_PACTYP_FILLDATA:
      return "MHAS_PACTYP_FILLDATA";
    case MHAS_PACTYP_SYNC:
      return "MHAS_PACTYP_SYNC";
    case MHAS_PACTYP_SYNCGAP:
      return "MHAS_PACTYP_SYNCGAP";
    case MHAS_PACTYP_DESCRIPTOR:
      return "MHAS_PACTYP_DESCRIPTOR";
    case MHAS_PACTYP_MPEGH3DACFG:
      return "MHAS_PACTYP_MPEGH3DACFG";
    case MHAS_PACTYP_MPEGH3DAFRAME:
      return "MHAS_PACTYP_MPEGH3DAFRAME";
    case MHAS_PACTYP_USERINTERACTION:
      return "MHAS_PACTYP_USERINTERACTION";
    case MHAS_PACTYP_LOUDNESS_DRC:
      return "MHAS_PACTYP_LOUDNESS_DRC";
    case MHAS_PACTYP_AUDIOTRUNCATION:
      return "MHAS_PACTYP_AUDIOTRUNCATION";
    case MHAS_PACTYP_CRC16:
      return "MHAS_PACTYP_CRC16";
    case MHAS_PACTYP_CRC32:
      return "MHAS_PACTYP_CRC32";
    case MHAS_PACTYP_MARKER:
      return "MHAS_PACTYP_MARKER";
    case MHAS_PACTYP_BUFFERINFO:
      return "MHAS_PACTYP_BUFFERINFO";
    case MHAS_PACTYP_UNDEF:
    default:
      return "MHAS_PACTYP_UNDEF";
  }
}

static int mhasValidatePacketLabel(mhas_packetType  const MHASPacketType,
                                   unsigned int     const MHASPacketLabel
) {
  int err = 0;
  char* msg = NULL;

  switch (MHASPacketType) {
    case MHAS_PACTYP_FILLDATA:
    case MHAS_PACTYP_SYNC:
    case MHAS_PACTYP_SYNCGAP:
    case MHAS_PACTYP_DESCRIPTOR:
      /* required MHASPacketLabel == 0 */
      if (0 != MHASPacketLabel) {
        char* msg = mapMhasPacketTypeToString(MHASPacketType);
        fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        fprintf(stderr, "\n!!WARNING!!  MHASPacketLabel shall be 0 for MHASPacketType %s!", msg);
        fprintf(stderr, "\n!!           The current stream is NOT conforming to ISO/IEC 23008-3!");
        fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
        fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
        err = /*-1*/0;
      }
      break;
    case MHAS_PACTYP_MPEGH3DACFG:
    case MHAS_PACTYP_MPEGH3DAFRAME:
    case MHAS_PACTYP_USERINTERACTION:
    case MHAS_PACTYP_LOUDNESS_DRC:
    case MHAS_PACTYP_AUDIOTRUNCATION:
      /* not allowed MHASPacketLabel == 0 */
      if (0 == MHASPacketLabel) {
        char* msg = mapMhasPacketTypeToString(MHASPacketType);
        fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
        fprintf(stderr, "\n!!WARNING!!  MHASPacketLabel shall not be 0 for MHASPacketType %s!", msg);
        fprintf(stderr, "\n!!           The current stream does NOT obey MHASPacketLabel ranges mandated in Table 226 of ISO/IEC 23008-3!");
        fprintf(stderr, "\n!!           Decoders might refuse decoding this stream!");
        fprintf(stderr, "\n!!WARNING!!::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\n");
        err = /*-1*/0;
      }
      break;
    case MHAS_PACTYP_CRC16:
    case MHAS_PACTYP_CRC32:
    case MHAS_PACTYP_MARKER: 
    case MHAS_PACTYP_BUFFERINFO:  
    case MHAS_PACTYP_UNDEF:
    default:
      /* no restriction */
      err = 0;
  }

  return err;
}

static int mhasPacket(HANDLE_STREAMFILE         stream,
                      mhas_packetType          *pacType,
                      unsigned int             *pacLabel,
                      unsigned char            *data,
                      unsigned long            *nBits,
                      int                       writeFlag
) {
  int err = 0;
  HANDLE_BSBITSTREAM bitStream = stream->spec->bitStream;
  MHASPacketInfo tmpPacInfo;
  unsigned long k;

  if ((NULL == pacType) || (NULL == pacLabel)) {
    return -1;
  }

  if (writeFlag == 1) /* write */
  {
    size_t tmp = 0xf;
    int writeBitCount = 0;

    tmpPacInfo.PacType   = *pacType;
    tmpPacInfo.pacLength = 0;
    tmpPacInfo.pacLabel  = *pacLabel;

    switch (*pacType) {
      case MHAS_PACTYP_SYNC:
        err = mhasAdvancePacket(bitStream,
                                MHAS_PACTYP_SYNC,
                                tmpPacInfo.pacLabel,
                                8,
                                &MHAS_SyncWord);
        break;
      case MHAS_PACTYP_MPEGH3DACFG:
        writeBitCount = advanceMpeghDecoderConfigDescriptor(NULL,
                                                            &(stream->prog[0].decoderConfig[0]),
                                                            2,
                                                            0);

        tmpPacInfo.pacLength = ((writeBitCount + 7) >> 3);
        err = mhasPacketHeader(bitStream, &tmpPacInfo, 1);
        {
          int tmpLen          = BsCurrentBit(bitStream);
          unsigned long tmp   = 0;
     
          advanceMpeghDecoderConfigDescriptor(bitStream,
                                              &(stream->prog[0].decoderConfig[0]),
                                              writeFlag,
                                              0);

          /* byte align */
          tmpLen = BsCurrentBit(bitStream) - tmpLen;
          tmpLen = (tmpPacInfo.pacLength << 3) - tmpLen;
          BsPutBit(bitStream, tmp, tmpLen);
        }
        break;
      case MHAS_PACTYP_MPEGH3DAFRAME:
        if ((NULL == data) || (NULL == nBits)) {
          return -1;
        }

        err = mhasAdvancePacket(bitStream,
                                MHAS_PACTYP_MPEGH3DAFRAME,
                                tmpPacInfo.pacLabel,
                                *nBits,
                                data);
        break;
      case MHAS_PACTYP_AUDIOTRUNCATION:
        if (1 == stream->prog[0].audioTruncationinfo->isActive.value) {
          tmpPacInfo.pacLength = 2;
          err = mhasPacketHeader(bitStream, &tmpPacInfo, 1);

          writeBitCount = advanceMpeghAudioTruncationInfoDescriptor(bitStream,
                                                                    stream->prog[0].audioTruncationinfo,
                                                                    writeFlag);

          if (writeBitCount != (tmpPacInfo.pacLength << 3)) {
            CommonExit(-1, "StreamFile:MHAS: Error writing audioTruncationInfo.");
            err = -1;
          }
        }

        break;
      case MHAS_PACTYP_SYNCGAP:
      case MHAS_PACTYP_MARKER:
      case MHAS_PACTYP_UNDEF:
      default:
        err = -1;
        break;
    }
  } else { /* read */
    unsigned long tmp = 0;
    int len           = 0;

    err = mhasPacketHeader(bitStream, &tmpPacInfo, 0); /* read */
    if (err) {
      return err;
    }

    *pacType  = tmpPacInfo.PacType;
    *pacLabel = tmpPacInfo.pacLabel;

    if (nBits) {
      *nBits = tmpPacInfo.pacLength << 3;
    }

    switch (*pacType) {
      case MHAS_PACTYP_MPEGH3DACFG:
        setupDecConfigDescr(&stream->prog[0].decoderConfig[0]);
        presetDecConfigDescr(&stream->prog[0].decoderConfig[0]);
        {
          int tmpLen          = BsCurrentBit(bitStream);
          unsigned long tmp   = 0;
     
          advanceMpeghDecoderConfigDescriptor( bitStream, &(stream->prog[0].decoderConfig[0]), writeFlag, 0);

          {
            int numLFE = 0;
            static int initial_config = 1;
            static DEC_CONF_DESCRIPTOR initConf = {0};
            HDEC_CONF_DESCRIPTOR decConf = &stream->prog[0].decoderConfig[0];
            if (initial_config) {
              decConf->audioSpecificConfig.channelConfiguration.value = usac_get_channel_number(&decConf->audioSpecificConfig.specConf.usacConfig, &numLFE);
              memcpy(&initConf.audioSpecificConfig, &decConf->audioSpecificConfig, sizeof(AUDIO_SPECIFIC_CONFIG));
            }
            if (!initial_config && memcmp(&(decConf->audioSpecificConfig), &initConf.audioSpecificConfig, sizeof(AUDIO_SPECIFIC_CONFIG))) {
              CommonExit(1, "Explicit config change not supported by the Reference Software 3DAudioDecoder.\n.");
            }
            initial_config = 0;
          }

          /* byte align */
          tmpLen = BsCurrentBit(bitStream) - tmpLen;
          tmpLen = ( tmpPacInfo.pacLength << 3 ) - tmpLen;
          BsGetBit( bitStream, &tmp, tmpLen );
        }
        break;
      case MHAS_PACTYP_MPEGH3DAFRAME:
        {
          FIFO_BUFFER *pFifo = &stream->prog[0].programData->spec->AUbuffer[0];
          HANDLE_STREAM_AU au = StreamFileAllocateAU(tmpPacInfo.pacLength<<3);
          HANDLE_STREAM_AU tmpAU;
          DebugPrintf(5,"StreamFile:MHAS: reading");
          if (au) {
            int AUsLeft = FIFObufferLength(*pFifo);

            if (AUsLeft!=0) {
              CommonWarning("StreamFile:getAU(LATM): did not read %i AccessUnits. discarding due to read of new payload", AUsLeft);
            }
            while (AUsLeft--) {
              tmpAU = FIFObufferPop(*pFifo);
              StreamFileFreeAU(tmpAU);
            }
            for (k=0; k<tmpPacInfo.pacLength; k++) {
              if (BsGetBitChar(bitStream, &au->data[k], 8))
                err = -1;
            }
            if (FIFObufferPush(*pFifo, au)) {
              CommonExit(-1,"StreamFile:LATM: Error saving AU to buffer");
              err = -1;
            }
            if(err) return err;

            tmpAU = FIFObufferPop(*pFifo);
            if (tmpAU == NULL) {
              CommonExit(-1,"StreamFile:getAU(LATM): empty AU buffered???");
              return -1;
            }

            err = StreamFile_AUcopyResize((HANDLE_STREAM_AU)data,
                                          tmpAU->data,
                                          tmpAU->numBits);
            StreamFileFreeAU(tmpAU);
          }
        }
        break;
      case MHAS_PACTYP_AUDIOTRUNCATION:
        {
          int bitCnt = 0;
          bitCnt = advanceMpeghAudioTruncationInfoDescriptor(bitStream,
                                                             stream->prog[0].audioTruncationinfo,
                                                             writeFlag);
          if (bitCnt != (tmpPacInfo.pacLength << 3)) {
            CommonExit(-1, "StreamFile:MHAS: Error parsing audioTruncationInfo.");
            err = -1;
          }
          break;
        }
      default:
        /* skip payload */
        for (k = 0; k < tmpPacInfo.pacLength; k++) {
          BsGetBit(bitStream, &tmp, 8);
        }
        break;
    } 

    /* sanities */
    if (*pacType == MHAS_PACTYP_SYNC) {
      if (tmpPacInfo.pacLength != 1) err = -1; if (err) return err;
      if (tmp != MHAS_SyncWord)      err = -1; if (err) return err;
    }
  }

  if (0 == err) {
    err = mhasValidatePacketLabel(tmpPacInfo.PacType,
                                  tmpPacInfo.pacLabel);
  }

  return err;
}


/* --------------------------------------------------- */
/* ---- MHAS                                      ---- */
/* --------------------------------------------------- */

static int MHASinitProgram(HANDLE_STREAMPROG prog)
{
  if ((prog->programData->spec = (struct tagProgSpecificInfo*)malloc(sizeof(struct tagProgSpecificInfo))) == NULL) {
    CommonExit(-1,"StreamFile:initProgram: error in malloc");
    return -1;
  }
  memset(prog->programData->spec, 0, sizeof(struct tagProgSpecificInfo));
  return 0;
}


static int MHASopenRead(HANDLE_STREAMFILE stream)
{
  int err = 0;
  unsigned int pacLabel = 0;
  mhas_packetType pacTyp;
  AUDIO_SPECIFIC_CONFIG *asc;
  HANDLE_STREAMPROG prog;

  char *info=NULL;
  stream->spec->bitStream = BsOpenFileRead(stream->fileName,MP4_MAGIC,&info);
  if (stream->spec->bitStream==NULL) {
    if (MHASdebugLevel >= 1) {
      CommonWarning("StreamFile:openRead(MHAS): no MHAS bitstream with magic string available.");
    }
    return -1;
  }
  if (strcmp(info,bsopen_info_mhas)) {
    if (MHASdebugLevel >= 1) {
      CommonWarning("StreamFile:openRead(MHAS): bitstream info does not indicate MHAS");
    }
    return -1;
  }

  /* stream->spec->first_frame = 1; */ /* useByteAligment ??? */

  asc = &(stream->prog[0].decoderConfig->audioSpecificConfig);
  asc->audioDecoderType.value = USAC; /* fixed for MHAS */

  /* init progCount and prog */
  prog = newStreamProg(stream);
  prog->trackCount = 1;
  prog->programData->spec->AUbuffer[0] = FIFObufferCreate(MAXBUFFER);
  if (prog->programData->spec->AUbuffer[0] == NULL) {
    CommonWarning("StreamFile:LATM: could not create buffer");
  }

  do {
    err = mhasPacket(stream,
                     &pacTyp,
                     &pacLabel,
                     (unsigned char *)asc,
                     NULL,
                     0);

    if (0 != err) {
      CommonExit(-1,"StreamFile:openRead(MHAS): bitstream parsing error");
      return -1;
    }
  } while (pacTyp != MHAS_PACTYP_MPEGH3DACFG);

  
  stream->prog[0].allTracks          = 1;
  stream->prog[0].dependencies[0]    = -1;
  stream->providesIndependentReading = 0;

  return 0;
}


static int MHAS2openRead(HANDLE_STREAMFILE stream)
{
  int err = 0;
  unsigned int pacLabel = 0;
  mhas_packetType pacTyp;
  AUDIO_SPECIFIC_CONFIG *asc;
  HANDLE_STREAMPROG prog;

  stream->spec->bitStream = BsOpenFileRead(stream->fileName, NULL, NULL);
  if (stream->spec->bitStream==NULL) {
    if (MHASdebugLevel >= 1) {
      CommonWarning("StreamFile:openRead(MHAS): no MHAS bitstream without magic string available.");
    }
    return -1;
  }
  /* stream->spec->first_frame = 1; */ /* useByteAligment ??? */

  asc = &(stream->prog[0].decoderConfig->audioSpecificConfig);
  asc->audioDecoderType.value = USAC; /* fixed for MHAS */

  /* init progCount and prog */
  prog = newStreamProg(stream);
  prog->trackCount = 1;
  prog->programData->spec->AUbuffer[0] = FIFObufferCreate(MAXBUFFER);
  if (prog->programData->spec->AUbuffer[0] == NULL)
    CommonWarning("StreamFile:LATM: could not create buffer");

  do {
    err = mhasPacket(stream,
                     &pacTyp,
                     &pacLabel,
                     (unsigned char *)asc,
                     NULL,
                     0);

    if (0 != err) {
      CommonExit(-1,"StreamFile:openRead(MHAS): bitstream parsing error");
      return -1;
    }
  } while (pacTyp != MHAS_PACTYP_MPEGH3DACFG);

  
  stream->prog[0].allTracks          = 1;
  stream->prog[0].dependencies[0]    = -1;
  stream->providesIndependentReading = 0;
  return 0;
}

static int MHASgetAccessUnit(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU au)
{
  int err = -1;
  unsigned int pacLabel = 0;
  mhas_packetType pacTyp;
  HANDLE_STREAMFILE stream = prog->fileData;

  if (trackNr != 0) {
    CommonExit(-1, "StreamFile:getAccessUnit(MHAS): only one track supported");
    return -1;
  }
  /* if (!(au->data)) return -1; */

  /* preset audioTruncationInfo */
  setupMpeghAudioTruncationInfoDescr(&(stream->prog[0].audioTruncationinfo[0]));
  presetMpeghAudioTruncationInfoDescr(&(stream->prog[0].audioTruncationinfo[0]));

  do {
    err = mhasPacket(stream,
                     &pacTyp,
                     &pacLabel,
                     (unsigned char *)au,
                     NULL,
                     0);

    if (err == -1) {
      CommonExit(-1,"StreamFile:getAccessUnit(MHAS): bitstream parsing error");
    }
    if (err) {
      return err;
    }
  } while (pacTyp != MHAS_PACTYP_MPEGH3DAFRAME);
  err = 0;

  return err;
}

static int MHASopenWrite(HANDLE_STREAMFILE stream, int argc, char** argv)
{
  char *info = NULL;

  info = (char*)bsopen_info_mhas;

  stream->spec->bitStream = BsOpenFileWrite(stream->fileName,MP4_MAGIC, info);

  if (stream->spec->bitStream==NULL) {
    CommonExit(-1,"StreamFile:openWrite(MHAS): error opening bit stream for writing");
    return -1;
  }
  stream->spec->first_frame = 1;

  stream->providesIndependentReading = 0; 

  return 0;
}

static int MHAS2openWrite(HANDLE_STREAMFILE stream, int argc, char** argv)
{
  stream->spec->bitStream = BsOpenFileWrite(stream->fileName,NULL, NULL);

  if (stream->spec->bitStream==NULL) {
    CommonExit(-1,"StreamFile:openWrite(MHAS): error opening bit stream for writing");
    return -1;
  }
  stream->spec->first_frame = 1;

  stream->providesIndependentReading = 0; 

  return 0;
}


static int MHASheaderWrite(HANDLE_STREAMFILE stream)
{
  int err = 0;
  unsigned int pacLabel = 0;
  mhas_packetType pacTyp;
  HANDLE_STREAMPROG prog = &stream->prog[0];

  /* sanity */
  if (stream->progCount != 1) {
    CommonExit(-1,"StreamFile:headerWrite(MHAS): only one program possible at this time");
    return -1;
  }

  /* write SYNC */
  pacTyp   = MHAS_PACTYP_SYNC;
  pacLabel = 0;
  err = mhasPacket(stream,
                   &pacTyp,
                   &pacLabel,
                   NULL,
                   NULL,
                   1);

  /* write MPEG-H Config */
  pacTyp   = MHAS_PACTYP_MPEGH3DACFG;
  pacLabel = 1;
  err = mhasPacket(stream,
                   &pacTyp,
                   &pacLabel,
                   NULL,
                   NULL,
                   1);

  return err;
}


static int MHASputAccessUnit(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU au)
{
  int err = 0;
  long length = (au->numBits+7) >> 3;
  unsigned int pacLabel = 0;
  mhas_packetType pacTyp;
  HANDLE_STREAMFILE stream=prog->fileData;

  /* write AudioTruncationInfo */
  setupMpeghAudioTruncationInfoDescr(&(stream->prog[0].audioTruncationinfo[0]));

  pacTyp   = MHAS_PACTYP_AUDIOTRUNCATION;
  pacLabel = 1;
  err = mhasPacket(stream,
                   &pacTyp,
                   &pacLabel,
                   NULL,
                   NULL,
                   1);

  /* write AU */
  pacTyp   = MHAS_PACTYP_MPEGH3DAFRAME;
  pacLabel = 1;
  err = mhasPacket(stream,
                   &pacTyp,
                   &pacLabel,
                   au->data,
                   &(au->numBits),
                   1);

  
  return err;
}

static int MHASclose(HANDLE_STREAMFILE stream)
{
  int err;

  err = BsClose(stream->spec->bitStream);
  return err;
}


/* --------------------------------------------------- */
/* ---- Constructor                               ---- */
/* --------------------------------------------------- */

int MHASinitStream(HANDLE_STREAMFILE stream)
{
  /*init and malloc*/
  stream->initProgram=MHASinitProgram;
  /*open read*/
  stream->openRead=MHASopenRead;
  /*open write*/
  stream->openWrite=MHAS2openWrite;
  stream->headerWrite=MHASheaderWrite;
  /*close*/
  stream->close=MHASclose;
  /*getAU, putAU*/
  stream->getAU=MHASgetAccessUnit;
  stream->putAU=MHASputAccessUnit;

  stream->getEditlist=NULL;

  if ((stream->spec = (struct tagStreamSpecificInfo*)malloc(sizeof(struct tagStreamSpecificInfo))) == NULL) {
    CommonWarning("StreamFile:initStream: error in malloc");
    return -1;
  }
  memset(stream->spec, 0, sizeof(struct tagStreamSpecificInfo));
  return 0;
}

/* --------------------------------------------------- */
/* ---- Constructor MHAS2                         ---- */
/* --------------------------------------------------- */

int MHAS2initStream(HANDLE_STREAMFILE stream)
{
  /*init and malloc*/
  stream->initProgram=MHASinitProgram;
  /*open read*/
  stream->openRead=MHAS2openRead;
  /*open write*/
  stream->openWrite=MHAS2openWrite;
  stream->headerWrite=MHASheaderWrite;
  /*close*/
  stream->close=MHASclose;
  /*getAU, putAU*/
  stream->getAU=MHASgetAccessUnit;
  stream->putAU=MHASputAccessUnit;

  stream->getEditlist=NULL;

  if ((stream->spec = (struct tagStreamSpecificInfo*)malloc(sizeof(struct tagStreamSpecificInfo))) == NULL) {
    CommonWarning("StreamFile:initStream: error in malloc");
    return -1;
  }
  memset(stream->spec, 0, sizeof(struct tagStreamSpecificInfo));
  return 0;
}

void MHASshowHelp( void )
{
  printf(MODULE_INFORMATION);
}

void MHAS2showHelp( void )
{
  printf(MODULE_INFORMATION);
}
