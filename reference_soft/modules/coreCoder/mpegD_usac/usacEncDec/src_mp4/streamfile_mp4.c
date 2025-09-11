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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "obj_descr.h"           /* structs */

#include "allHandles.h"
#include "common_m4a.h"          /* common module       */
#include "bitstream.h"           /* bit stream module   */
#include "flex_mux.h"            /* parse object descriptors */
#include "cmdline.h"             /* parse commandline options */
#include "ISOMovies.h"           /* MP4-support  */
#include "MPEGHMovies.h"           /* MP4-support  */

/* defines missing in older ISOMovies.h */
#ifndef ISOOpenMovieNormal
#define ISOOpenMovieNormal MP4OpenMovieNormal
#define ISOAddMediaSamplesPad MP4AddMediaSamplesPad
#endif

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

#include "streamfile.h"          /* public functions */
#include "streamfile_mp4.h"
#include "streamfile_helper.h"

static const unsigned int roll = ('r' << 24) | ('o' << 16) | ('l' << 8) | ('l' << 0);

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


/* ---- mp4 specific structures ---- */
#define MODULE_INFORMATION "StreamFile transport lib: MP4ff module"

struct tagStreamSpecificInfo {
  ISOMovie             moov;
  int                  profileAndLevel;
};

struct tagProgSpecificInfo {
  ISOTrack           tracks[MAXTRACK];
  ISOMedia           media[MAXTRACK];
  ISOTrackReader     reader[MAXTRACK];
  ISOHandle          sampleEntryH[MAXTRACK];
  SIW_CFGTYPE        sampleDescriptionType[MAXTRACK];   /* cfgtype */
  /* read editlists */
  u32                sampleCount[MAXTRACK];
  u32                sampleIdx[MAXTRACK];
  double             startOffset[MAXTRACK];
  double             playTime[MAXTRACK];
  /* write editlists */
  int                bSampleGroupElst;
  s16                rollDistance[MAXTRACK];
  u32                sampleGroupIndex[MAXTRACK];
  int                nDelay;
  int                sampleRateOut;
  int                originalSampleCount;
  int                originalSamplingRate;
  int                sampleGroupStartOffset;
  int                sampleGroupLength;
};


/* ---- helper function ---- */

static const unsigned long initialObjectDescrTag = 0x02 ;


static HANDLE_BSBITSTREAM BsFromHandle (ISOHandle theHandle, int WriteFlag)
{
  u32    handleSize ;
  ISOErr err = ISOGetHandleSize (theHandle, &handleSize) ;
  if (!err)
  {
    HANDLE_BSBITBUFFER bb;

    bb = BsAllocPlainDirtyBuffer((unsigned char *) (*theHandle), handleSize * 8);
    if (WriteFlag == 0)
      return BsOpenBufferRead (bb) ;
    else
      return BsOpenBufferWrite (bb) ;
  }

  return NULL ;
}

static int commandline(HANDLE_STREAMFILE stream, int argc, char**argv)
{
  int profileAndLevel;

  CmdLineSwitch switchList[] = {
    {"pl",NULL,"%i","254",NULL,"audio profile and level"},
    {NULL,NULL,NULL,NULL,NULL,NULL}
  };
  CmdLinePara paraList[] = {
    {NULL,NULL,NULL}
  };

  switchList[0].argument = &profileAndLevel;

  if (stream!=NULL) {
    if (CmdLineEval(argc,argv,paraList,switchList,1,NULL,NULL)) return -10;

    stream->spec->profileAndLevel = profileAndLevel;
  } else {
    CmdLineHelp(NULL,paraList,switchList,stdout);
  }

  return 0;
}


static int MP4Audio_ReadUnknownElementFromStream ( HANDLE_BSBITSTREAM bs )
{
  unsigned long tmp;
  unsigned long sizeOfClass;

  /* read tag */
  BsGetBit (bs, &tmp, 8);

  /* read size */
  for (sizeOfClass = 0, tmp = 0x80 ; tmp &= 0x80 ;
       sizeOfClass = (sizeOfClass << 7) | (tmp & 0x7F))
    BsGetBit (bs, &tmp, 8) ;

  /* skip the element */
  DebugPrintf(3, "skipping %ld bytes of unknown element\n",sizeOfClass);
  while (sizeOfClass--) {
    BsGetBit (bs, &tmp, 8);
  }

  return 0;
}


static int MP4Audio_ReadInitialOD (ISOMovie theMovie, int ignoreProfiles)
{
  ISOHandle initialODH = NULL ;
  ISOErr err ;

  err = ISONewHandle( 0, &initialODH ); 
  err = MP4GetMovieInitialObjectDescriptor (theMovie, initialODH );

  if (!err)
  {
    unsigned long tag, tmp, tmp1 ;
    unsigned long sizeOfClass ;
    unsigned long startBitPos;
    unsigned long numESDs;

    unsigned long id ;
    unsigned long urlFlag ;
    unsigned long inlineFlag ;

    unsigned long odProfile = 0 ;
    unsigned long sceneProfile = 0 ;
    unsigned long audioProfile = 0 ;
    unsigned long visualProfile = 0 ;
    unsigned long graphicsProfile = 0 ;

    ES_DESCRIPTOR *esd = NULL;

    HANDLE_BSBITSTREAM bs = BsFromHandle (initialODH, 0) ;

    BsGetBit (bs, &tag, 8) ;
    if (tag != initialObjectDescrTag)
    {
      CommonWarning("Check MP4ff failed: IOD class tag == %ld \n", tag);
      err = ISOBadDataErr ; goto bail ;
    }

    for (sizeOfClass = 0, tmp = 0x80 ; tmp &= 0x80 ;
         sizeOfClass = (sizeOfClass << 7) | (tmp & 0x7F))
         BsGetBit (bs, &tmp, 8) ;

    startBitPos = BsCurrentBit(bs);

    BsGetBit (bs, &id, 10) ;
    DebugPrintf(3, "Initial Object Descriptor (id:%ld):", id);
    if ((id == 0) || (id == 1023)) {  /* 0 is forbidden and 1023 is reserved */
      CommonWarning("Check MP4ff failed: ID %ld invalid for IOD\n",id);
    }

    BsGetBit (bs, &urlFlag, 1) ;
    BsGetBit (bs, &inlineFlag, 1) ;
    BsGetBit (bs, &tmp, 4) ;

    if ( tmp != 0xf ) {
      CommonWarning("Check MP4ff failed: IOD reserved bits set to 0x%lx!\n", tmp);
    }

    if (!urlFlag)
    {
      BsGetBit (bs, &odProfile, 8) ;
      BsGetBit (bs, &sceneProfile, 8) ;
      BsGetBit (bs, &audioProfile, 8) ;
      BsGetBit (bs, &visualProfile, 8) ;
      BsGetBit (bs, &graphicsProfile, 8) ;
      DebugPrintf(3, "   odProfile       :%ld", odProfile);
      DebugPrintf(3, "   sceneProfile    :%ld", sceneProfile);
      DebugPrintf(3, "   audioProfile    :%ld", audioProfile);
      DebugPrintf(3, "   visualProfile   :%ld", visualProfile);
      DebugPrintf(3, "   graphicsProfile :%ld", graphicsProfile);
    } else {
      BsGetBit ( bs, &tmp1, 8); /* get URL length */
      DebugPrintf(3, "   URL string      :");
      for (;tmp1>0;tmp1--) {
        BsGetBit ( bs, &tmp, 8); /* get URL string */
        DebugPrintf(3, "%c",tmp);
      }
      DebugPrintf(3, "\n");
    }

    if (!ignoreProfiles) {
      if (odProfile != 0xFF) {
        CommonWarning("Check MP4ff: Can't handle OD profile %ld \n", tag);
        err = ISOBadDataErr ; goto bail ;
      }

      if (sceneProfile != 0xFF) {
        CommonWarning("Check MP4ff: Can't handle BIFS profile %ld \n", tag);
        err = ISOBadDataErr ; goto bail ;
      }

      if (visualProfile != 0xFF) {
        CommonWarning("Check MP4ff: Can't handle VIDEO profile %ld \n", tag);
        err = ISOBadDataErr ; goto bail ;
      }

      if (graphicsProfile != 0xFF) {
        CommonWarning("Check MP4ff: Can't handle GRAPHICS profile %ld \n", tag);
        err = ISOBadDataErr ; goto bail ;
      }
    }

    numESDs = 0;
    initESDescr(&esd);

    while ( (BsCurrentBit(bs) - startBitPos) < (sizeOfClass<<3) ) {
      if ( advanceESDescr (bs, esd, 0/*read*/, 1/*system-specific stuff*/) >= 0 ) {
        numESDs++;
      } else {
        MP4Audio_ReadUnknownElementFromStream( bs );
      }
    }


    closeESDescr(&esd);

    if ( numESDs == 0 ) {
      DebugPrintf (1, "IOD does not contain ES descriptors\n");
      goto bail;
    }

bail :
    BsFreePlainDirtyBuffer(bs);
    BsCloseRemove (bs, 0) ;
  }

  ISODisposeHandle (initialODH) ;



  return err ;
}



/* --------------------------------------------------- */
/* ---- MP4ff (MP4)                               ---- */
/* --------------------------------------------------- */


static int MP4initProgram(HANDLE_STREAMPROG prog)
{
  if ((prog->programData->spec = (struct tagProgSpecificInfo*)malloc(sizeof(struct tagProgSpecificInfo))) == NULL) {
    CommonWarning("StreamFile:initProgram: error in malloc");
    return -1;
  }
  memset(prog->programData->spec, 0, sizeof(struct tagProgSpecificInfo));
  return 0;
}


static int MP4openRead(HANDLE_STREAMFILE stream)
{
  ISOErr err = ISONoErr;
  u32 trackCount;

  err = ISOOpenMovieFile(&stream->spec->moov, stream->fileName, ISOOpenMovieNormal);
  if (err) {
    DebugPrintf(3,"StreamFile:openRead(MP4): file is not MP4ff");
    return -1;
  }

  if (MP4Audio_ReadInitialOD (stream->spec->moov, 0)) {
    DebugPrintf(3,"StreamFile:read initial OD(MP4): no OD found");
  }

  err = ISOGetMovieTrackCount(stream->spec->moov, &trackCount);if (err) return -2;

  err = genericOpenRead(stream, trackCount); /* open all tracks, handle dependencies */
  if (err) return -3;
  stream->providesIndependentReading = 0 /*1... 0 needed for access unit diagnosis*/;

  return trackCount;
}


static int MP4getDependency(HANDLE_STREAMFILE stream, int trackID)
{
  ISOErr err = ISONoErr;
  ISOTrack track;
  ISOTrack referencedTrack;
  u32 tmp;

  /* NOTE: trackID's range starting from 0 to trackCount-1, mp4 tracks are numbered from 1 to trackCount */
  err = ISOGetMovieIndTrack(stream->spec->moov, trackID+1, &track); if (err) return -3;
  err = ISOGetTrackEnabled(track, &tmp); if (err) return -3;
  if (!tmp) return -2;
  err = ISOGetTrackReferenceCount(track, MP4StreamDependencyReferenceType, &tmp); if (err) return -3;
  if (!tmp) return -1; /* non-dependant track */
  
  /* track depends on something */
  err = ISOGetTrackReference(track, MP4StreamDependencyReferenceType, 1, &referencedTrack); if (err) return -3;
  err = ISOGetTrackID(referencedTrack, &tmp); if (err) return -3;
  return tmp-1;
}

static int MP4extractMhasPacketsFromMhm(HANDLE_STREAMFILE stream, int prog, int index)
{
  ISOErr err = ISONoErr;
  HANDLE_STREAM_AU au = StreamFileAllocateAU(0);
  FILE* mhasFile;
  char mhasFilename[FILENAME_MAX];
  u32 tmp = stream->prog[prog].programData->spec->sampleIdx[index];

  /* Prepare extraction of MHAS packets */
  fprintf(stderr, "\n\nTrying to decode mhm. Using temporary mhas-file.\n\n");
  sprintf(mhasFilename, "tmpFile3Ddec_prog%d_track%d.mhas", prog, index);
  mhasFile = fopen(mhasFilename, "wb");
  if (mhasFile) {
    FILE* mhasFilenameInfoFile = NULL;
    mhasFilenameInfoFile = fopen("tmpFile3Ddec_mhm_mhas_filename.txt", "w+");
    if (mhasFilenameInfoFile) {
      fprintf(mhasFilenameInfoFile, "%s\n", mhasFilename);
      fclose(mhasFilenameInfoFile);
    }
    fprintf(stderr, "\n\nWarning: Could not decode mhm-mp4. Please decode the dumped mhas-stream: %s.\n\n", mhasFilename);
    err = ISONoErr;
  }
  else {
    fprintf(stderr, "\n\nError: Could not open %s.\n\n", mhasFilename);
    err = ISOFileNotFoundErr;
  }

  /* Extract MHAS packets and create temporary MHAS stream */
  while (ISONoErr == err) {
    unsigned int i;
    int error = stream->getAU(&(stream->prog[prog]), index, au);
    if (error == -2) {
      break;
    }
    if (error) {
      err = ISOFileNotFoundErr;
      break;
    }

    for (i = 0; i < (au->numBits + 7) >> 3; i++) {
      fprintf(mhasFile, "%c", au->data[i]);
    }
  }

  if (ISONoErr == err) {
    stream->prog[prog].programData->spec->sampleIdx[index] = tmp;
    StreamFileFreeAU(au);
    fclose(mhasFile);
  }

  return err;
}

static int MP4openTrack(HANDLE_STREAMFILE stream, int trackID, int prog, int index)
{
  ISOErr err = ISONoErr;
  ISOTrack track;
  ISOHandle decoderConfigH = 0;
  HANDLE_BSBITSTREAM bs = NULL;
  u32 elstEntryCount;
  u64 trackDuration;
  s64 startOffset;
  u32 mediaTimeScale, movieTimeScale;
  u32 sampleDescriptionType = 0;

  /* get dependencies */
  /*stream->prog[prog].dependencies[index] = stream->getDependency(stream, trackID);*/
  /*stream->prog[prog].programData->trackID = trackID+1;*/
  stream->prog[prog].dependencies[index] = index-1;

  /* open track and media, get track reader */
  /* NOTE: trackID's range starting from 0 to trackCount-1, mp4 tracks are numbered from 1 to trackCount */
  err = ISOGetMovieIndTrack(stream->spec->moov, trackID+1, &track); if (err) return -1;
  err = ISOGetTrackMedia(track, &stream->prog[prog].programData->spec->media[index]); if (err) return -1;
  err = ISOCreateTrackReader(track, &stream->prog[prog].programData->spec->reader[index]); if (err) return -1;

  /* correctly set sample idx, sample count: needed to get samples via ISOGetIndMediaSample() */
  err = ISOGetMediaSampleCount(stream->prog[prog].programData->spec->media[index], &stream->prog[prog].programData->spec->sampleCount[index]);
  stream->prog[prog].programData->spec->sampleIdx[index] = 1;

  /* data needed for editlits support */
  err = ISOGetMediaTimeScale(stream->prog[prog].programData->spec->media[index], &mediaTimeScale);
  err = ISOGetMovieTimeScale(stream->spec->moov, &movieTimeScale);
  err = ISOGetTrackEditlistEntryCount(track, &elstEntryCount);
  err = ISOGetTrackEditlist(track, &trackDuration, &startOffset, 1);

  if(err != ISONoErr){
    stream->prog[prog].programData->spec->startOffset[index] =  0.0;
    stream->prog[prog].programData->spec->playTime[index]    = -1.0;
  } else {
    stream->prog[prog].programData->spec->startOffset[index] = (double) startOffset / (double) mediaTimeScale;
    stream->prog[prog].programData->spec->playTime[index]    = (double) trackDuration / (double) movieTimeScale;
  }


  /* check sample description type */
  {
    ISOHandle tmpSampleDescriptionH;
    u32 dummy = 0;

    err = ISONewHandle( 0, &tmpSampleDescriptionH );
    if (err) return -1;

    /* returns the atom below stsd: mp4a or mha1 */
    err = MP4GetMediaSampleDescription( stream->prog[prog].programData->spec->media[index], 1, tmpSampleDescriptionH, &dummy);
    if (err) return -1;

    err = ISOGetSampleDescriptionType(tmpSampleDescriptionH, &sampleDescriptionType);
    if (err) return -1;

    ISODisposeHandle(tmpSampleDescriptionH);

    err = ISONewHandle(0, &decoderConfigH); if (err) return -1;

    /* get decoder configuration  and set sample description type */
    switch (sampleDescriptionType)
    {
     case SIW_MP4A:
       stream->prog[prog].programData->spec->sampleDescriptionType[index] = SIW_CFGTYPE_MP4A;
       err = MP4TrackReaderGetCurrentDecoderConfig(stream->prog[prog].programData->spec->reader[index], decoderConfigH);
       if (err) goto bail;
       break;

     case SIW_M4AE:
       stream->prog[prog].programData->spec->sampleDescriptionType[index] = SIW_CFGTYPE_M4AE;
       err = MP4TrackReaderGetCurrentDecoderConfig(stream->prog[prog].programData->spec->reader[index], decoderConfigH);
       if (err) goto bail;
       break;

     case SIW_MHA1:
       stream->prog[prog].programData->spec->sampleDescriptionType[index] = SIW_CFGTYPE_MHA1;
       err = MHAGetMediaDecoderConfig(stream->prog[prog].programData->spec->reader[index], decoderConfigH);
       if (err) goto bail;
       break;

     case SIW_MHM1:
       stream->prog[prog].programData->spec->sampleDescriptionType[index] = SIW_CFGTYPE_MHM1;
       err = MHAGetMediaDecoderConfig(stream->prog[prog].programData->spec->reader[index], decoderConfigH);
       if (err) goto bail;
       break;

     default:
       return -1;
    }
  }

  bs = BsFromHandle(decoderConfigH, 0);
  setupDecConfigDescr(&stream->prog[prog].decoderConfig[index]);

  switch (sampleDescriptionType) {
     case SIW_MP4A:
     case SIW_M4AE:
       err = advanceDecoderConfigDescriptor(bs, &stream->prog[prog].decoderConfig[index], 0/*read*/, 1/*do systems specific stuff*/
#ifndef CORRIGENDUM1
         ,1
#endif
         );
       if (err) goto bail;
       break;

     case SIW_MHA1:
       err = advanceMpeghDecoderConfigDescriptor(bs, &stream->prog[prog].decoderConfig[index], 0 /*read*/, 1);
       if (err) goto bail;
       break;
     case SIW_MHM1:
       err = advanceMpeghDecoderConfigDescriptor(bs, &stream->prog[prog].decoderConfig[index], 0 /*read*/, 1);
       if (err) goto bail;
       err = MP4extractMhasPacketsFromMhm(stream, prog, index);
       if (err) goto bail;
       break;

     default:
       err = -1;
       goto bail;
       break;
  }

 bail:
  ISODisposeHandle(decoderConfigH);
  if (bs) {
    BsFreePlainDirtyBuffer(bs);
    BsClose(bs);
  }
  return err;
}
  

static int MP4getAccessUnit(HANDLE_STREAMPROG stream, int trackNr, HANDLE_STREAM_AU au)
{
  ISOErr err;
#ifdef CORRECTED_INIT_NO_FIX
  u32 unitSize;
#else
  u32 unitSize=0;
#endif

  u32 sampleFlags;
  u64 duration = 0;
  u64 outDecodingTime = 0;
  s32 outCTSOffset = 0;
  u32 outSampleDescIndex = 0;
  u8 pad = 0x7f;
  ISOHandle sampleDataH;

  stream->stssSignalsSyncSample[trackNr] = STSS_SYNCSAMPLE_NOTSET;

  err = ISONewHandle(0, &sampleDataH); if (err) return -1;

  if(stream->programData->spec->sampleIdx[trackNr] > stream->programData->spec->sampleCount[trackNr]){
    err = ISOEOF;
  } else {
    err = ISOGetIndMediaSampleWithPad(stream->programData->spec->media[trackNr], 
                                      stream->programData->spec->sampleIdx[trackNr], 
                                      sampleDataH,
                                      &unitSize, 
                                      &outDecodingTime,
                                      &outCTSOffset,
                                      &duration,
                                      &sampleFlags,
                                      &outSampleDescIndex,
                                      &pad);

    stream->programData->spec->sampleIdx[trackNr]++;
    stream->stssSignalsSyncSample[trackNr] = (!(sampleFlags & 0x1))? STSS_SYNCSAMPLE_SET : STSS_SYNCSAMPLE_NOTSET;
  }

  if (pad > 7) pad = 0; /* padding unknown */

  DebugPrintf(6,"StreamFile:getAU(MP4): unitSize: %ld, padding bits: %ld",unitSize, pad);

  if (err) {
    if (err == ISOEOF) return -2;
    return -1;
  }

  StreamFile_AUcopyResize(au, (u8*)*sampleDataH, (unitSize<<3)-pad);

  ISODisposeHandle(sampleDataH);

  return 0;
}


static int MP4openWrite(HANDLE_STREAMFILE stream, int argc, char** argv)
{
  ISOErr err = ISONoErr;
  u32 initialObjectDescriptorID = 1;
  u8 OD_profileAndLevel = 0xff; 	/* none required */
  u8 scene_profileAndLevel = 0xff;	/* none required */
  u8 audio_profileAndLevel = 0xfe;
  u8 visual_profileAndLevel = 0xff; 	/* none required */
  u8 graphics_profileAndLevel = 0xff; 	/* none required */

  int result;

  /* - options parsing */

  result = commandline(stream, argc, argv);
  if (result) return result;

  audio_profileAndLevel = stream->spec->profileAndLevel;

  err = MP4NewMovie(&(stream->spec->moov),
		    initialObjectDescriptorID, 
		    OD_profileAndLevel,
		    scene_profileAndLevel,
		    audio_profileAndLevel,
		    visual_profileAndLevel,
		    graphics_profileAndLevel);

  stream->providesIndependentReading = 0 /*1... 0 needed for access unit diagnosis*/;

  return err;
}


static int MP4headerWrite(HANDLE_STREAMFILE stream)
{
  ISOErr err = ISONoErr;
  u32 x;
  u8 i;
  int progIdx;

  for (progIdx=0; progIdx<stream->progCount; progIdx++) {
    HANDLE_STREAMPROG prog = &stream->prog[progIdx];
    int maxSamplingRate = 0;
    for (x=0; x<prog->trackCount; x++) {
      u32 tmp;

      err = ISONewMovieTrack(stream->spec->moov, ISONewTrackIsAudio, &prog->programData->spec->tracks[x]);if (err) goto bail;
      err = MP4AddTrackToMovieIOD( prog->programData->spec->tracks[x] ); if (err) goto bail;
      err = ISOSetTrackEnabled(prog->programData->spec->tracks[x], 1); if (err) goto bail;
      if ( prog->dependencies[x]!=-1 ) {
        err = ISOAddTrackReference(prog->programData->spec->tracks[x],
                                   prog->programData->spec->tracks[prog->dependencies[x]],
                                   MP4StreamDependencyReferenceType,
                                   &tmp/*outRefNum*/); if (err) goto bail;
      }

      err = ISONewTrackMedia(prog->programData->spec->tracks[x],
                             &(prog->programData->spec->media[x]),
                             ISOAudioHandlerType,
                             prog->decoderConfig[x].audioSpecificConfig.samplingFrequency.value,
                             NULL); if (err) goto bail;
      err = ISOBeginMediaEdits(prog->programData->spec->media[x]); if (err) goto bail;
      err = ISOSetMediaLanguage(prog->programData->spec->media[x], "und"); if (err) goto bail;

      /* write pre-roll, needed for editlist support */
      if(prog->programData->spec->bSampleGroupElst && (prog->programData->spec->rollDistance[x] != 0) ){
        unsigned char* SGD_buff;
        MP4Handle sampleGroupDescriptionH = NULL;
        err = MP4NewHandle(2, &sampleGroupDescriptionH);
			
        SGD_buff = (unsigned char *) *sampleGroupDescriptionH;
        SGD_buff[0] = (unsigned char) (0xff & (prog->programData->spec->rollDistance[x] >> 8));
        SGD_buff[1] = (unsigned char) (0xff & (prog->programData->spec->rollDistance[x]));
      
        ISOAddGroupDescription(prog->programData->spec->media[x], roll, sampleGroupDescriptionH, &(prog->programData->spec->sampleGroupIndex[x]));
      
        err = MP4DisposeHandle(sampleGroupDescriptionH);
      }

      {
        HANDLE_BSBITSTREAM mpegh3daConfigStream;
        ISOHandle mpegh3daConfig;
        u8  speakerLayoutType = 0;
        u8  referenceChannelLayout = 0;
        u8* compatibleProfileLevelSetIndications;
        u8  numProfileAndLevelCompatibleSets = 0;

        tmp = advanceMpeghDecoderConfigDescriptor(NULL,
                                                  &prog->decoderConfig[x],
                                                  2, 1);

        tmp = (tmp+7)/8;

        /* create output handle and bitstream with correct size */
        err = ISONewHandle(tmp, &mpegh3daConfig);if (err) goto bail;
        mpegh3daConfigStream = BsFromHandle ( mpegh3daConfig, 1 );

        advanceMpeghDecoderConfigDescriptor(mpegh3daConfigStream,
                                            &prog->decoderConfig[x],
                                            1, 1);

        err = ISONewHandle(0, &prog->programData->spec->sampleEntryH[x]);

        /* Get reference channel Layout */
        speakerLayoutType =  prog->decoderConfig[x].audioSpecificConfig.specConf.usacConfig.referenceLayout.speakerLayoutType.value;
        
        /* If reference layout is not signalized as CICP-Index ( speakerLayoutType == 0 ) referenceChannelLayout = 0 is signalized */
        if ( 0 == speakerLayoutType )
          referenceChannelLayout =  prog->decoderConfig[x].audioSpecificConfig.specConf.usacConfig.referenceLayout.CICPspeakerLayoutIdx.value;
        else
          referenceChannelLayout = 0;

        numProfileAndLevelCompatibleSets = prog->compatibleProfileLevelSet[x].numCompatibleSets.value;
        compatibleProfileLevelSetIndications = (u8*)calloc(numProfileAndLevelCompatibleSets, 1);
        for (i = 0; i < numProfileAndLevelCompatibleSets; i++) {
          memcpy(compatibleProfileLevelSetIndications + i, (u8*)(&(prog->compatibleProfileLevelSet[x].CompatibleSetIndication[i].value)), 1);
        }

        err = MP4NewMHAConfiguration(prog->programData->spec->tracks[x],
                                     prog->programData->spec->sampleEntryH[x],
                                     1,
                                     prog->decoderConfig[x].bufferSizeDB.value,
                                     prog->decoderConfig[x].maxBitrate.value,
                                     prog->decoderConfig[x].avgBitrate.value,
                                     prog->decoderConfig[x].MPEGHAudioProfileLevel.value,
                                     prog->decoderConfig[x].transportMask,
                                     referenceChannelLayout,
                                     numProfileAndLevelCompatibleSets,
                                     compatibleProfileLevelSetIndications,
                                     mpegh3daConfig
                                     ); if (err) goto bail;

        if ( BsFreePlainDirtyBuffer( mpegh3daConfigStream ) ) {
          err = MP4BadParamErr;
        }

        BsClose( mpegh3daConfigStream );
      }

      maxSamplingRate = max((int)prog->decoderConfig[x].audioSpecificConfig.samplingFrequency.value, maxSamplingRate);
    }


    if(prog->programData->spec->bSampleGroupElst){
      ISOSetMovieTimeScale(stream->spec->moov, maxSamplingRate);
    }

  }
 bail:
  return err;
}


static int MP4putAccessUnit(HANDLE_STREAMPROG prog, int trackNr, HANDLE_STREAM_AU au)
{ 
  unsigned long i;
  ISOErr err = ISONoErr;
  ISOHandle sampleDataH;
  ISOHandle sampleSizeH;
  ISOHandle sampleDurationH;
  ISOHandle samplePaddingH;
  ISOHandle syncSamplesH;
  u32 length = (au->numBits+7) >> 3;

  err = ISONewHandle(sizeof(u32),&sampleSizeH); if (err) goto bail;
  err = ISONewHandle(sizeof(u32),&sampleDurationH); if (err) goto bail;
  err = ISONewHandle(sizeof(u8),&samplePaddingH); if (err) goto bail;
  err = ISONewHandle(length, &sampleDataH); if (err) goto bail;

  if (STSS_SYNCSAMPLE_SET == prog->stssSignalsSyncSample[trackNr]) {
    err = ISONewHandle(sizeof(u32) * 1, &syncSamplesH); if (err) goto bail;
    *(u32*)* syncSamplesH = 1;
  } else {
    err = ISONewHandle(sizeof(u32) * 0, &syncSamplesH); if (err) goto bail;
  }

  for (i=0; i<length; i++) {
    *(unsigned char*)(*sampleDataH+i) = au->data[i];
  }

  err = ISOGetHandleSize(sampleDataH, (u32*) *sampleSizeH); if (err) goto bail;
  *(u32*)*sampleDurationH = prog->programData->sampleDuration[trackNr];
  *(u8*)*samplePaddingH = (8-(au->numBits & 7))&7;

  DebugPrintf(6,"StreamFile:putAU(MP4): sampleSizeH: %ld, padding %i, sampleDurationH: %ld",*(u32*) *sampleSizeH, *(u8*)*samplePaddingH, *(u32*) *sampleDurationH);

  err = ISOAddMediaSamplesPad(prog->programData->spec->media[trackNr], 
                              sampleDataH, 
                              1, 
                              sampleDurationH, 
                              sampleSizeH,
                              prog->programData->spec->sampleEntryH[trackNr], 
                              NULL,
                              syncSamplesH,
                              samplePaddingH); if (err) goto bail;

  if (prog->programData->spec->sampleEntryH[trackNr]) {
    ISODisposeHandle(prog->programData->spec->sampleEntryH[trackNr]);
    prog->programData->spec->sampleEntryH[trackNr] = NULL;
  }
  ISODisposeHandle(sampleDataH);
  ISODisposeHandle(sampleSizeH);
  ISODisposeHandle(sampleDurationH);
  ISODisposeHandle(samplePaddingH);

 bail:
  return err;
}


static int MP4getEditlist(HANDLE_STREAMPROG stream, int trackNr, double *startOffset, double *playTime){

  int err = 0;

  if(!err){
    if( (startOffset == NULL) ||
        (playTime    == NULL) ){
      err = -1;
    }
  }

  if(!err){
    *startOffset = stream->programData->spec->startOffset[trackNr];
    *playTime    = stream->programData->spec->playTime[trackNr];
  }

  return err;
}

static int MP4setEditlist(HANDLE_STREAMPROG stream, int trackNr, int bSampleGroupElst, int rollDistance){

  int err = 0;

  if(!err){
    stream->programData->spec->bSampleGroupElst = bSampleGroupElst;
    stream->programData->spec->rollDistance[trackNr] = rollDistance;
  }

  return err;
}

static int MP4setEditlistValues(HANDLE_STREAMPROG stream, int nDelay, int sampleRateOut, int originalSampleCount, 
                                int originalSamplingRate, int sampleGroupStartOffset, int sampleGroupLength){

  int err = 0;

  if(!err){
    stream->programData->spec->nDelay                 = nDelay;
    stream->programData->spec->sampleRateOut          = sampleRateOut;
    stream->programData->spec->originalSampleCount    = originalSampleCount;
    stream->programData->spec->originalSamplingRate   = originalSamplingRate;
    stream->programData->spec->sampleGroupStartOffset = sampleGroupStartOffset;
    stream->programData->spec->sampleGroupLength      = sampleGroupLength;
  }

  return err;
}


static int MP4close(HANDLE_STREAMFILE stream)
{
  ISOErr err = ISONoErr;
  u32 x;
  int progIdx;

  if (stream->status == STATUS_WRITING) {

    /* close writing MP4 --- */
    for (progIdx=0; progIdx<stream->progCount; progIdx++) {
      HANDLE_STREAMPROG prog = &stream->prog[progIdx];
      for (x=0;x<prog->trackCount;x++) {
        u64 mediaDuration;
        u32 mediaTimeScale;
        u32 editBegin = 0;     /* in movie ticks */
        u64 editDuration = 0;  /* in media ticks */

        err = ISOEndMediaEdits(prog->programData->spec->media[x]); if (err) goto bail;
        err = ISOGetMediaDuration(prog->programData->spec->media[x], &mediaDuration); if (err) goto bail;
        err = ISOGetMediaTimeScale(prog->programData->spec->media[x], &mediaTimeScale ); if (err) goto bail;

        editBegin = 0;
        editDuration = mediaDuration;

        if ((mediaDuration!=0) && prog->programData->spec->bSampleGroupElst == 1) {
          if( (prog->programData->spec->originalSamplingRate <= 0) || (prog->programData->spec->originalSampleCount <=0 ) || (prog->programData->spec->nDelay <=-1 ) ) {
            editBegin = 0;
            editDuration = mediaDuration;
          }
          else {
            /* edit list duration is in movie time scale, but ISOSamplesToGroup() uses mediaTimeScale for all params */
            editBegin = (u32)((float)prog->programData->spec->nDelay * ((float)mediaTimeScale / (float)prog->programData->spec->sampleRateOut));
            editDuration = (u64)ceil((float)prog->programData->spec->originalSampleCount * ((float)mediaTimeScale / (float)prog->programData->spec->originalSamplingRate));
          }
          ISOMapSamplestoGroup(prog->programData->spec->media[x], roll, prog->programData->spec->sampleGroupIndex[x], prog->programData->spec->sampleGroupStartOffset, prog->programData->spec->sampleGroupLength);
        }

        err = ISOInsertMediaIntoTrack(prog->programData->spec->tracks[x], 0, editBegin, editDuration, 1); if (err) goto bail;
      }
#ifdef I2R_LOSSLESS
#endif
    }
    err = ISOWriteMovieToFile(stream->spec->moov, stream->fileName);if (err) goto bail;

  } else {

    /* close reading MP4 --- */
    for (progIdx=0; progIdx<stream->progCount; progIdx++) {
      HANDLE_STREAMPROG prog = &stream->prog[progIdx];
      for (x=0;x<prog->trackCount;x++) {
        if (prog->programData->spec->reader[x])
          ISODisposeTrackReader(prog->programData->spec->reader[x]);
      }
    }
  }

  err = ISODisposeMovie(stream->spec->moov);
 bail:
  return err;
}


/* --------------------------------------------------- */
/* ---- Constructor                               ---- */
/* --------------------------------------------------- */

int MP4initStream(HANDLE_STREAMFILE stream)
{
  /*init and malloc*/
  stream->initProgram=MP4initProgram;
  /*open read*/
  stream->openRead=MP4openRead;
  stream->getDependency=MP4getDependency;
  stream->openTrack=MP4openTrack;
  /*open write*/
  stream->openWrite=MP4openWrite;
  stream->headerWrite=MP4headerWrite;
  /*close*/
  stream->close=MP4close;
  /*getAU, putAU*/
  stream->getAU=MP4getAccessUnit;
  stream->putAU=MP4putAccessUnit;
  stream->getEditlist = MP4getEditlist;
  stream->setEditlist = MP4setEditlist;
  stream->setEditlistValues = MP4setEditlistValues;

  if ((stream->spec = (struct tagStreamSpecificInfo*)malloc(sizeof(struct tagStreamSpecificInfo))) == NULL) {
    CommonWarning("StreamFile:initStream: error in malloc");
    return -1;
  }
  memset(stream->spec, 0, sizeof(struct tagStreamSpecificInfo));
  return 0;
}


void MP4showHelp( void )
{
  printf(MODULE_INFORMATION);
  commandline(NULL,0,NULL);
}
