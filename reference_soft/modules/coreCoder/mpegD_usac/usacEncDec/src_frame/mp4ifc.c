/**********************************************************************
MPEG-4 Audio VM

This software module was originally developed by

Heiko Purnhagen     (University of Hannover / ACTS-MoMuSys)

and edited by

Markus Werner       (SEED / Software Development Karlsruhe) 
Olaf Kaehler        (Fraunhofer IIS-A)

in the course of development of the MPEG-2 AAC/MPEG-4 Audio standard
ISO/IEC 13818-7, 14496-1,2 and 3. This software module is an
implementation of a part of one or more MPEG-2 AAC/MPEG-4 Audio tools
as specified by the MPEG-2 AAC/MPEG-4 Audio standard. ISO/IEC gives
users of the MPEG-2 AAC/MPEG-4 Audio standards free license to this
software module or modifications thereof for use in hardware or
software products claiming conformance to the MPEG-2 AAC/ MPEG-4 Audio
standards. Those intending to use this software module in hardware or
software products are advised that this use may infringe existing
patents. The original developer of this software module and his/her
company, the subsequent editors and their companies, and ISO/IEC have
no liability for use of this software module or modifications thereof
in an implementation. Copyright is not released for non MPEG-2
AAC/MPEG-4 Audio conforming products. The original developer retains
full right to use the code for his/her own purpose, assign or donate
the code to a third party and to inhibit third party from using the
code for non MPEG-2 AAC/MPEG-4 Audio conforming products. This
copyright notice must be included in all copies or derivative works.

Copyright (c) 2000.

$Id: mp4ifc.c,v 1.5 2012-03-13 11:33:37 frd Exp $
Decoder frame work
**********************************************************************/
#include <stdio.h>
#include <string.h>

#include "streamfile.h"
#include "ep_convert.h"

#include "nok_ltp_common.h"      /* structs */
#include "obj_descr.h"           /* structs */
#include "tf_mainStruct.h"       /* structs */
#include "hvxc_struct.h"	       /* structs */


#include "dec_tf.h"
#include "dec_lpc.h"
#include "dec_par.h"

#include "bitstream.h"
#include "common_m4a.h"
#include "mp4ifc.h"

#include "audio.h"      /* audio i/o module */
#include "mp4au.h"
#include "port.h"
#include "decifc.h"
#ifdef  DEBUGPLOT
#include "plotmtv.h"
#endif

#include "wavIO.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define MAX_AU 12
#define MAX_TRACKS_PER_LAYER 50

#ifndef DEFAULT_NUM_TIMESLOTS
#define DEFAULT_NUM_TIMESLOTS (64)
#endif

#ifndef DEFAULT_NUM_QMFBANDS
#define DEFAULT_NUM_QMFBANDS  (64)
#endif

extern FILE *qmfRealOutputFile;
extern FILE *qmfImagOutputFile;

extern WAVIO_HANDLE handleWavIO_real;
extern WAVIO_HANDLE handleWavIO_imag;

extern float *** qmfRealOutBuffer;
extern float *** qmfImagOutBuffer;

extern int qmfout; 

static unsigned int nSamplesWrittenPerChannel;

static void writeout_saoc_data(USAC_DATA* usacData, FILE* saocFileHandle)
{
  static int printInfo = 1;
  int i = 0;
 
   for ( i = 0; i < usacData->usacExtNumElements; ++i )
   {
    if ( usacData->usacExtElementComplete[i] == 1 )
     {
       if ( usacData->usacExtElementType[i] == USAC_EXT_TYPE_SAOC )
       {
          int j = 0;
          int size = usacData->usacExtElement[i].usacExtElementPayloadLength;
          
          if ( printInfo )
          {
            printf("SAOC container present\n");
            printInfo = 0;
          }

          for (j = 0; j < size; ++j ) 
          {
            char data = usacData->usacExtElement[i].usacExtElementSegmentData[j] & 0xff;
            fwrite(&data, 1, 1, saocFileHandle);   
          }
       }
    }
  }

}

static void writeout_obj_data(USAC_DATA* usacData, char* extObjMetadataFilename, int frameCntr)
{
  static int printInfo = 1;
  int i = 0;

  FILE* extObjMetadataFileHandle;
  char *pFilename;
  char nextExtObjMetadataFilename[FILENAME_MAX];
  char suffix[5] = { '\0' };

  strncpy(nextExtObjMetadataFilename, extObjMetadataFilename, FILENAME_MAX);
  pFilename = &(nextExtObjMetadataFilename[strlen(nextExtObjMetadataFilename)]);

  while ( pFilename[0] != '.' )
    {
      pFilename--;
    }
  
  strncpy(suffix, pFilename, 4);
  
  while ( pFilename[0] != '_' )
    {
      pFilename--;
      
    }
  
  sprintf(pFilename, "_%d%s", frameCntr, suffix);

 
  /* Open next object file */
  extObjMetadataFileHandle = fopen(nextExtObjMetadataFilename,"wb");

  for ( i = 0; i < usacData->usacExtNumElements; ++i )
   {
    if ( usacData->usacExtElementComplete[i] == 1 )
     {
       if ( usacData->usacExtElementType[i] == USAC_EXT_TYPE_OBJ)
        {
          int j = 0;
          int size = usacData->usacExtElement[i].usacExtElementPayloadLength;
          
          if ( printInfo )
          {
            printf("Object container present\n");
            printInfo = 0;
          } 
          
          for (j = 0; j < size; ++j ) 
          {
            char data = usacData->usacExtElement[i].usacExtElementSegmentData[j] & 0xff;
            fwrite(&data, 1, 1, extObjMetadataFileHandle);   
          }

       }
    }
  }

  fclose(extObjMetadataFileHandle);

}



static int MP4Audio_SetupDecoders (DEC_DATA         *decData,
                                   DEC_DEBUG_DATA   *decDebugData,
                                   int   streamCount,          /* in: number of Layers    */
                                   char* decPara ,          /* in: decoder parameter string */
                                   HANDLE_DECODER_GENERAL hFault,
                                   int   mainDebugLevel,
                                   int   audioDebugLevel,
                                   int   epDebugLevel,
                                   char* aacDebugString
#ifdef CT_SBR
                                   ,int HEaacProfileLevel
                                   ,int bUseHQtransposer
#endif
#ifdef HUAWEI_TFPP_DEC
                                   ,int actATFPP
#endif
                                   );

static int frameDataAddAccessUnit(DEC_DATA *decData,
                                  HANDLE_STREAM_AU au,
                                  int track);

static T_DECODERGENERAL faultData[MAX_TF_LAYER];
static HANDLE_DECODER_GENERAL hFault = &faultData[0];
static AudioFile *audioFile = NULL;


#ifdef DEBUGPLOT
extern  int framePlot;
#endif

int MP4Audio_ProbeFile (char *inFile)
{
  HANDLE_STREAMFILE file = NULL;
  file = StreamFileOpenRead ( inFile, FILETYPE_AUTO ) ; 
  if (file != NULL) 
  { 
    StreamFileClose (file) ; 
    return 1 ;
  } 
  else 
    return 0 ;
}

int MP4Audio_DecodeFile (char* inFile,          /* in: bitstream input file name */
                         char* audioFileName,   /* in: audio file name */
                         char* audioFileFormat, /* in: audio file format (au, wav, aif, raw) */
                         int   int24flag,
                         int   monoFilesOut,
                         int   maxLayer,        /* in  max layers */
                         char* decPara,         /* in: decoder parameter string */
                         int   mainDebugLevel,
                         int   audioDebugLevel,
                         int   epDebugLevel,
                         int   bitDebugLevel,
                         char* aacDebugString,
#ifdef CT_SBR
                         int   HEaacProfileLevel,
                         int   bUseHQtransposer,
#endif
#ifdef HUAWEI_TFPP_DEC
                         int   actATFPP,
#endif
                         int   programNr,
                         char* objFileName,
                         char* saocFileName
                        )
{

  int err = 0;
  unsigned long framesDone;
  int fileOpened = 0;
  int numFC = 0;
  int fCenter = 0;
  int numSC = 0; 
  int numBC = 0;
  int bCenter = 0;
  int numLFE = 0;
  int nUsacMainChannels = 0;
  int channelNum = 0, tmp;
  AUDIO_SPECIFIC_CONFIG* asc;
  DEC_DATA           *decData = NULL;
  DEC_DEBUG_DATA     *decDebugData = NULL;
  HANDLE_STREAMFILE stream = NULL;
  HANDLE_STREAMPROG prog = NULL;
  int  suitableTracks = 0;
  int  trackIdx;
  float **outSamples;
  long  numOutSamples;
  char buffer_empty;
  int numChannelOut;
  float fSampleOut=0;
  int firstDecdoeFrame = 1; /* SAMSUNG_2005-09-30 */
  int progStart, progStop, progCnt;
  int useEditlist[MAX_TRACKS_PER_LAYER] = {0};
  double startOffset[MAX_TRACKS_PER_LAYER] = {0.0};
  double durationTotal[MAX_TRACKS_PER_LAYER] = {0.0};
  long startOffsetInSamples[MAX_TRACKS_PER_LAYER] = {0};
  long playTimeInSamples[MAX_TRACKS_PER_LAYER] = {0};

  static int qmfFileOpen = 0;

  FILE* fSAOC = NULL;

  /* initialize hFault */
  memset(hFault,0,MAX_TF_LAYER*sizeof(T_DECODERGENERAL));

  BsInit (0,bitDebugLevel,0);

  stream = StreamFileOpenRead( inFile, FILETYPE_AUTO ); if (stream==NULL) goto bail;
   
  if ( programNr != -1 ) {      /* specific program selected */
    progStart = programNr;
    progStop = programNr+1;
    DebugPrintf(1,"Selected program: %d",progStart);
  } else {                      /* no specific program selected -> loop over all programs */
    progStart = 0;
    progStop = StreamFileGetProgramCount( stream );
    if ( progStop > 1 ) {       /* more than one program in stream */
        DebugPrintf(1,"No specific program selected -> looping over all programs");
    }
    DebugPrintf(1,"Number of programs: %d",progStop);
  }
  
  for ( progCnt = progStart; progCnt < progStop; progCnt++ ) {
 
  programNr = progCnt;
  DebugPrintf(1,"Decoding program: %d",programNr);
     
  prog = StreamFileGetProgram( stream, programNr ); if (prog==NULL) goto bail;
  suitableTracks = prog->trackCount;
  
  
  DebugPrintf(1,"\nFound MP4 file with  %d suitable Tracks  ",suitableTracks);


  if (!suitableTracks)
    return -1 ;
  
  asc = &(prog->decoderConfig[0].audioSpecificConfig);
  switch (asc->audioDecoderType.value) {
#ifdef AAC_ELD
   case ER_AAC_ELD:
    channelNum = asc->channelConfiguration.value;
    break;
#endif
   default:
   if (asc->channelConfiguration.value == 0) {
     if(asc->audioDecoderType.value != 42){
       select_prog_config(asc->specConf.TFSpecificConfig.progConfig,
                         (asc->specConf.TFSpecificConfig.frameLength.value ? 960:1024),
                         hFault->hResilience,
                         hFault->hEscInstanceData,
                         aacDebugString['v']);
     }
   }
   if(asc->audioDecoderType.value == 42){
     channelNum = usac_get_channel_number(&asc->specConf.usacConfig, &numLFE);
     nUsacMainChannels = channelNum - numLFE;
   } else {
     channelNum = get_channel_number(asc->channelConfiguration.value,
                                     asc->specConf.TFSpecificConfig.progConfig,
                                     &numFC, &fCenter, &numSC, &numBC, &bCenter, &numLFE, NULL);
   }

   break;
  }

  /*
    instantiate proper decoder for decoderconfig
  */

  decData = (DEC_DATA *)calloc(1,sizeof(DEC_DATA));
  decDebugData=(DEC_DEBUG_DATA *)calloc(1,sizeof(DEC_DEBUG_DATA));

  /* suitableTracks was initally total no of tracks
     this function stores total no of tracks in decData and
     returns no of tracks needed for decoding up to mayLayer */
  tmp = asc->channelConfiguration.value;
  asc->channelConfiguration.value = channelNum;
  
  
  suitableTracks=frameDataInit(prog->allTracks,
                               suitableTracks,
                               maxLayer,
                               prog->decoderConfig,
                               decData);
  asc->channelConfiguration.value = tmp;

  if (suitableTracks<=0) {
    CommonWarning("Error during framework initialisation");
    goto bail;
  }

  /* get editlist info */
  for(trackIdx = 0; trackIdx < suitableTracks; trackIdx++){
    double tmpStart, tmpDuration;

    useEditlist[trackIdx] = 0;
    if(0 == StreamFileGetEditlist(prog, trackIdx, &tmpStart, &tmpDuration)){
      startOffset[trackIdx]   = tmpStart;
      durationTotal[trackIdx] = tmpDuration;
      if(tmpDuration > -1){
        useEditlist[trackIdx]   = 1;
      }
    }
  }

  for (framesDone = 0, buffer_empty=0; buffer_empty==0; framesDone++)  {
    int tracksForDecoder;
#ifdef DEBUGPLOT
    plotInit(framePlot,audioFileName,0);
#endif

    {
      HANDLE_STREAM_AU inputAUs[MAX_TRACKS_PER_LAYER];
      HANDLE_STREAM_AU decoderAUs[MAX_TRACKS_PER_LAYER];
      int layer, track;
      int firstTrackInLayer, resultTracksInLayer;
      int err = 0;
      for (track=0; track<MAX_TRACKS_PER_LAYER; track++) {
        inputAUs[track]=StreamFileAllocateAU(0);
        decoderAUs[track]=StreamFileAllocateAU(0);
      }
      firstTrackInLayer = tracksForDecoder = 0;
      /* go through each decoded layer */
      for (layer=0; layer<(signed)decData->frameData->scalOutSelect+1; layer++) {
        /* go through all frames in the superframe */
        resultTracksInLayer = EPconvert_expectedOutputClasses(decData->frameData->ep_converter[layer]);
        while ((signed)decData->frameData->layer[tracksForDecoder].NoAUInBuffer<StreamAUsPerFrame(prog, firstTrackInLayer)) {
          int epconv_input_tracks;
          if (EPconvert_numFramesBuffered(decData->frameData->ep_converter[layer])) {
            epconv_input_tracks=0;
          } else {
            epconv_input_tracks=decData->frameData->tracksInLayer[layer];
          }

          /* go through all tracks in this layer */
          for (track=0; track<epconv_input_tracks; track++) {
            err = StreamGetAccessUnit( prog, firstTrackInLayer+track, inputAUs[track] );
            if ( err ) break;
          }
          if ( err ) break;

          /* decode with epTool if necessary */
          err = EPconvert_processAUs(decData->frameData->ep_converter[layer],
                                     inputAUs, epconv_input_tracks,
                                     decoderAUs, MAX_TRACKS_PER_LAYER);
          if (resultTracksInLayer!=err) {
            CommonWarning("Expected %i AUs after ep-conversion, but got %i AUs", resultTracksInLayer, err);
            /*resultTracksInLayer=err;*/
          }
          err=0;

          /* save the tracks */
          for (track=0; track<resultTracksInLayer; track++) {
            frameDataAddAccessUnit(decData, decoderAUs[track], tracksForDecoder+track);
          }
		  
        }
        if (err) break;
        firstTrackInLayer += decData->frameData->tracksInLayer[layer];
        tracksForDecoder += resultTracksInLayer;
      }
      for (track=0; track<MAX_TRACKS_PER_LAYER; track++) {
        if (inputAUs[track]) StreamFileFreeAU(inputAUs[track]);
        if (decoderAUs[track]) StreamFileFreeAU(decoderAUs[track]);
      }
      buffer_empty=1;
      for (track=0; track<(signed)decData->frameData->od->streamCount.value; track++) {
        if (decData->frameData->layer[track].NoAUInBuffer>0) buffer_empty=0;
      }
      if (buffer_empty) 
        break;
    }
    if (framesDone==0) {
      int delay, track;
      
      delay = MP4Audio_SetupDecoders (decData,
                                      decDebugData,
                                      tracksForDecoder,
                                      decPara,
                                      hFault,
                                      mainDebugLevel,
                                      audioDebugLevel,
                                      epDebugLevel,
                                      aacDebugString
#ifdef CT_SBR
                                     ,HEaacProfileLevel
                                     ,bUseHQtransposer
#endif
#ifdef HUAWEI_TFPP_DEC
                                      ,actATFPP
#endif
                                      );
      /* decoder initialisation will update streamCount in decData and
         print an error if decoder initialisation does not agree with track count
         determined earlier by frameDataInit() and stored in suitableTracks */

      fSampleOut = (float) decData->frameData->scalOutSamplingFrequency;
      numChannelOut = decData->frameData->scalOutNumChannels ;
      
      for (track=0; track<(signed)decData->frameData->od->streamCount.value; track++) {
        if(useEditlist[track]){
#ifndef ALIGN_PRECISION_NOFIX
          double tmpStartOffset = startOffset[track] * fSampleOut + 0.5;
          double tmpPlayTime = durationTotal[track] * fSampleOut + 0.5;
          startOffsetInSamples[track] = (long)(tmpStartOffset);
          playTimeInSamples[track]    = (long)(tmpPlayTime);
#else
          startOffsetInSamples[track] = (long)(startOffset[track] * fSampleOut + 0.5);
          playTimeInSamples[track]    = (long)(durationTotal[track] * fSampleOut + 0.5);          
#endif
        }
      }
      

      if ( qmfFileOpen == 0 ) {
        int error_init = 0;

        /* always 32 bits for qmf values */
        int bytedepth = 4;

        if ( qmfRealOutputFile ) {
          error_init = wavIO_openWrite(handleWavIO_real,
                                       qmfRealOutputFile,
                                       numChannelOut,
                                       fSampleOut,
                                       bytedepth);
        }

        if ( qmfImagOutputFile ) {
          error_init = wavIO_openWrite(handleWavIO_imag,
                                       qmfImagOutputFile,
                                       numChannelOut,
                                       fSampleOut,
                                       bytedepth);
        }
        if ( error_init == 0 ) {
          qmfFileOpen = 1;
        } else {
          return -1;
        }
      }

      if (!qmfout) {

      /* open saoc file */
      if ( saocFileName )
        fSAOC = fopen(saocFileName, "wb");
     
      /* open audio file */
      /* (sample format: 16 bit twos complement, uniform quantisation) */
      if (fileOpened == 0) {
      
      if (monoFilesOut==0) {
        audioFile = AudioOpenWrite(audioFileName,
                                   audioFileFormat,
                                   numChannelOut,
                                   fSampleOut,
                                   int24flag);
      } else {

        audioFile = AudioOpenWriteUsacMC(audioFileName,
                                         audioFileFormat,
                                         fSampleOut,
                                         int24flag,
                                         (nUsacMainChannels + numLFE),
                                         ((asc->specConf.usacConfig).numObjects).value,
                                         (int*)&asc->specConf.usacConfig.usacChannelConfig.outputChannelPos[0],
                                         ((asc->specConf.usacConfig).channelConfigurationIndex).value);
      }
      
      if (audioFile==NULL)
        CommonExit(1,"Decode: error opening audio file %s "
                   "(maybe unknown format \"%s\")",
                   audioFileName,audioFileFormat);

      /* seek to beginning of first frame (with delay compensation) */
      AudioSeek(audioFile,-delay);

      fileOpened=1;
      }
    }

    } /* qmf out */

    /*
      send AU to decoder
    */
    if ((mainDebugLevel == 1) && (framesDone % 20 == 0)) {
      fprintf(stderr,"\rFrame [%ld] ",framesDone);
      fflush(stderr);
    }
    if (mainDebugLevel >= 2 && mainDebugLevel <= 3) {
      printf("\rFrame [%ld] ",framesDone);
      fflush(stdout);
    }
    if (mainDebugLevel > 3)
      printf("Frame [%ld]\n",framesDone);


    audioDecFrame(decData,hFault,&outSamples,&numOutSamples, &numChannelOut);  /* SAMSUNG_2005-09-30 : added numChannelOut */
/* SAMSUNG_2005-09-30 : In BSAC MC, final out channel number is determined after 1 frame decoding.  */
    if(numChannelOut != decData->frameData->scalOutNumChannels && firstDecdoeFrame)
      {
        AudioClose(audioFile);
        /*  fixing 7.1 channel error */
        if(decData->frameData->scalOutObjectType == ER_BSAC)
        {
          channelNum = get_channel_number_BSAC(numChannelOut, /* SAMSUNG_2005-09-30 : Warnning ! it may not be same with asc->channelConfiguration.value. */
                                               asc->specConf.TFSpecificConfig.progConfig,
                                               &numFC, &fCenter, &numSC, &numBC, &bCenter, &numLFE, NULL);

			
        }
        else
        {
         switch (asc->audioDecoderType.value) {
#ifdef AAC_ELD
          case ER_AAC_ELD:
           channelNum = asc->channelConfiguration.value;
           break;
#endif
          default:
          channelNum = get_channel_number(numChannelOut, /* SAMSUNG_2005-09-30 : Warnning ! it may not be same with asc->channelConfiguration.value. */
                                          asc->specConf.TFSpecificConfig.progConfig,
                                          &numFC, &fCenter, &numSC, &numBC, &bCenter, &numLFE, NULL);
	    break;
	  }
         }

      if (!qmfout) {

        if (channelNum > 2) {
          CommonWarning("No standards for putting %ld Channels into 1 File -> Using multiple files\n", channelNum);
          monoFilesOut = 1;
        }
			
        if (monoFilesOut==0) {
          audioFile = AudioOpenWrite(audioFileName,
                                     audioFileFormat,
                                     numChannelOut,
                                     fSampleOut,
                                     int24flag);
        } else {
          /* SAMSUNG 2005-10-18 */
          if(decData->frameData->scalOutObjectType == ER_BSAC)
            audioFile = AudioOpenWriteMC_BSAC(audioFileName,
                                              audioFileFormat,
                                              fSampleOut,
                                              int24flag,
                                              numFC,
                                              fCenter,
                                              numSC,
                                              bCenter,
                                              numBC,
                                              numLFE);
          else
            audioFile = AudioOpenWriteMC(audioFileName,
                                         audioFileFormat,
                                         fSampleOut,
                                         int24flag,
                                         numFC,
                                         fCenter,
                                         numSC,
                                         bCenter,
                                         numBC,
                                         numLFE);
        }
        firstDecdoeFrame = 0;

      }

      } /* qmfout */

/* ~SAMSUNG_2005-09-30 */

#ifdef CT_SBR
    /* The implicit signalling....*/

    if (decData->tfData != NULL 
         && !qmfout 
      ) {
      static int reInitAudioOutput = 1;

      if (decData->tfData->runSbr == 1 &&             /* If zero, we know that no implicit signalling was detected*/
          decData->tfData->sbrPresentFlag != 0 &&     /* If zero, we know no SBR is present.*/
          decData->tfData->sbrPresentFlag != 1 &&     /* If one , we know for sure SBR is present, and already set up the output correctly.*/
          reInitAudioOutput ) {

        /* check sampling frequency just for AAC_LC and AAC_SCAL ( other aot are not supported at the moment ) */
		/* 20070326 BSAC Ext.*/
        if ( (decData->frameData->scalOutObjectType == AAC_LC) || (decData->frameData->scalOutObjectType == AAC_SCAL) || (decData->frameData->scalOutObjectType == ER_BSAC) ) {
		/* 20070326 BSAC Ext.*/

          /* get current output sampling frequency */
          float outputSamplingFreq = AudioGetSamplingFreq( audioFile );

          /* calculate favoured output sbr frequency */ 
          float sbrSamplingFreq;

          if(decData->tfData->bDownSampleSbr){
            sbrSamplingFreq = (float) prog->decoderConfig[suitableTracks-1].audioSpecificConfig.samplingFrequency.value;
          }
          else{
            sbrSamplingFreq = (float) prog->decoderConfig[suitableTracks-1].audioSpecificConfig.samplingFrequency.value * 2;
          }


          if ( outputSamplingFreq != sbrSamplingFreq ) {

            reInitAudioOutput = 0;

            /* reinit audio writeout */
            AudioClose(audioFile);
            if (monoFilesOut==0) {
              audioFile = AudioOpenWrite(audioFileName,
                                         audioFileFormat,
                                         channelNum,
                                         sbrSamplingFreq,
                                         int24flag);
            } else if(decData->frameData->scalOutObjectType == ER_BSAC) {
              audioFile = AudioOpenWriteMC_BSAC(audioFileName,
                                                audioFileFormat,
                                                sbrSamplingFreq,
                                                int24flag,
                                                numFC,
                                                fCenter,
                                                numSC,
                                                bCenter,
                                                numBC,
                                                numLFE);
            } else {
              audioFile = AudioOpenWriteMC(audioFileName,
                                           audioFileFormat,
                                           sbrSamplingFreq,
                                           int24flag,
                                           numFC,
                                           fCenter,
                                           numSC,
                                           bCenter,
                                           numBC,
                                           numLFE);
            }
          }
        }
      }
    }
#endif
    
    {
      int writeoutOn = 0;

#ifdef I2R_LOSSLESS
      if ((decData->frameData->scalOutObjectType == SLS) || (decData->frameData->scalOutObjectType == SLS_NCORE)) {
        if (framesDone > 1) {
          writeoutOn = 1;      /* for SLS skip 1st 2 frames (to chk why?) */
        }
      } else
#endif
        {
          writeoutOn = 1;      /* normally write out all frames */
        }


      if (writeoutOn == 1 
          && !qmfout
       ) {
        long skipSamples = 0;

        if(useEditlist[0]){
          if(numOutSamples < startOffsetInSamples[0]){
            skipSamples = numOutSamples;
          } else {
            skipSamples = startOffsetInSamples[0];

            if(numOutSamples > playTimeInSamples[0]){
              numOutSamples = playTimeInSamples[0];
            }
          }
          startOffsetInSamples[0] -= skipSamples;
          playTimeInSamples[0]    -= (numOutSamples - skipSamples);
        }

        if(decData->frameData->scalOutObjectType == USAC){
          if(monoFilesOut){
            AudioWriteDataTruncat(audioFile, outSamples, numOutSamples, skipSamples,-1);
          }
          else{
            AudioWriteDataTruncat(audioFile, outSamples, numOutSamples, skipSamples,((asc->specConf.usacConfig).channelConfigurationIndex).value);          
          }
        } else {
          AudioWriteData(audioFile, outSamples, numOutSamples, skipSamples);
        }

       if ( objFileName )
          writeout_obj_data(decData->usacData, objFileName, framesDone + 1);

       if ( fSAOC )
          writeout_saoc_data(decData->usacData, fSAOC);
      }

      else {
        int ts = 0;
        for ( ts = 0; ts < DEFAULT_NUM_TIMESLOTS; ++ts ) {
          wavIO_writeFrame(handleWavIO_real, qmfRealOutBuffer[ts], DEFAULT_NUM_QMFBANDS, &nSamplesWrittenPerChannel);
          wavIO_writeFrame(handleWavIO_imag, qmfImagOutBuffer[ts], DEFAULT_NUM_QMFBANDS, &nSamplesWrittenPerChannel);
        }
      }

    }
   

#ifdef DEBUGPLOT    
    framePlot++;
    plotDisplay(0);
#endif

  }
  if (mainDebugLevel >= 1)
    fprintf(stdout,"\n");

  
  audioDecFree(decData,hFault);

  free(decData);
  free(decDebugData);

  }     /* program loop closed */
  
  bail:
 
  if ( fSAOC )
    fclose(fSAOC);

  if (audioFile)
    AudioClose(audioFile);
  if (stream)
    StreamFileClose(stream);
 
  return err;
}


static int MP4Audio_SetupDecoders (DEC_DATA         *decData,
                                   DEC_DEBUG_DATA   *decDebugData,
                                   int   streamCount,     /* in: number of streams decoded */
                                   char* decPara,         /* in: decoder parameter string */
                                   HANDLE_DECODER_GENERAL hFault,
                                   int   mainDebugLevel,
                                   int   audioDebugLevel,
                                   int   epDebugLevel,
                                   char* aacDebugString
#ifdef CT_SBR
                                   ,int HEaacProfileLevel
                                   ,int bUseHQtransposer
#endif
#ifdef HUAWEI_TFPP_DEC
                                   ,int actATFPP
#endif
                                   )
                            
{
  int    delayNumSample;

  AudioInit(NULL, audioDebugLevel);

  /*
    init decoders
  */

  decDebugData->aacDebugString = aacDebugString;
  decDebugData->decPara = decPara;
  decDebugData->infoFileName=NULL;
  decDebugData->mainDebugLevel=mainDebugLevel;
  decDebugData->epDebugLevel=epDebugLevel;
  
  delayNumSample=audioDecInit(hFault,
                              decData,
                              decDebugData,
#ifdef CT_SBR
                              HEaacProfileLevel,
                              bUseHQtransposer,
#endif
                              streamCount
#ifdef HUAWEI_TFPP_DEC
                              ,actATFPP
#endif
                              );
  

  return delayNumSample ;
}


static int frameDataAddAccessUnit(DEC_DATA *decData,
                                  HANDLE_STREAM_AU au,
                                  int track)
{
  unsigned long unitSize, unitPad;
  HANDLE_BSBITBUFFER bb;
  HANDLE_BSBITSTREAM bs;
  HANDLE_BSBITBUFFER AUBuffer  = NULL;
  int idx = track;

  unitSize = (au->numBits+7)/8;
  unitPad = (unitSize*8) - au->numBits;

  bb = BsAllocPlainDirtyBuffer(au->data, /*au->numBits*/ unitSize<<3);
  bs = BsOpenBufferRead(bb);

  AUBuffer = decData->frameData->layer[idx].bitBuf ;
  if (AUBuffer!=0 ) {
    if ((BsBufferFreeBits(AUBuffer)) > (long)unitSize*8 ) {
      int auNumber = decData->frameData->layer[idx].NoAUInBuffer;
      BsGetBufferAppend(bs, AUBuffer, 1, unitSize*8);
      decData->frameData->layer[idx].AULength[auNumber]= unitSize*8;
      decData->frameData->layer[idx].AUPaddingBits[auNumber]= unitPad;
      decData->frameData->layer[idx].NoAUInBuffer++;/* each decoder must decrease this by the number of decoded AU */       
            /* each decoder must remove the first element in the list  by shifting the elements down */
      DebugPrintf(6,"AUbuffer %2i: %i AUs, last size %i, lastPad %i\n",idx, decData->frameData->layer[idx].NoAUInBuffer, decData->frameData->layer[idx].AULength[auNumber], decData->frameData->layer[idx].AUPaddingBits[auNumber]);
    } else {
      BsGetSkip(bs,unitSize*8);
      CommonWarning (" input buffer overflow for layer %d ; skipping next AU",idx);
    }
  } else {
    BsGetSkip(bs,unitSize*8);
  }
  BsCloseRemove (bs, 0);
  free(bb);
  return 0;
}


