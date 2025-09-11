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
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <saoc_encoder.h>
#include "saoc_enc.h"
#include "wavIO.h"
#include "qmflib.h"
#include "qmflib_hybfilter.h"
#include "cicp2geometry.h"

/**********************************************************************************************/
void printDisclaimer( void );

void InitDownmixMat(char* pInName, float pMat[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS],int nInChannels,int nOutChannels)
{
  FILE* fptr;
  float fTmp;
  int i,j, nCnt=0;

  if(pInName)  {
    fptr=fopen(pInName,"r");
    while(fscanf(fptr,"%f",&fTmp)!=EOF) {
      pMat[nCnt/nInChannels][nCnt%nInChannels]=fTmp;
      nCnt++;
    }
    fclose(fptr);
  } else {
    memset(pMat,0,sizeof(int)*MAX_NUM_OUTPUTCHANNELS*MAX_NUM_INPUTCHANNELS);
    for(i=0;i<nOutChannels;i++) {
      for(j=0;j<nInChannels;j++) {
        pMat[i][j]=1.f/(float)sqrt(nInChannels) + 1.f/(float)sqrt(nOutChannels);
      }
    }
  }
}

/**********************************************************************************************/

void InitRelatedToMat(char* pInName, int pMat[MAX_OBJECTS][MAX_OBJECTS])
{
  FILE* fptr;
  unsigned int i=0;
  char pTmpChar[100];
  int nIdxFrom=0;
  int nIdxTo=-1;

  memset(pMat,0,sizeof(int)*MAX_OBJECTS*MAX_OBJECTS);

  for(i=0;i<MAX_OBJECTS;i++) {
    pMat[i][i]=1;
  }

  if(pInName) {
    fptr=fopen(pInName,"r");
    fscanf(fptr,"%s",pTmpChar);
    for(i=0;i<strlen(pTmpChar);i++)  {
      nIdxTo=-1;
      if(pTmpChar[i]==91)  {
        if(pTmpChar[i+1]>=48 && pTmpChar[i+1]<=57) {
          nIdxTo=pTmpChar[i+1]-48-1;
        }
        if(pTmpChar[i+2]>=48 && pTmpChar[i+2]<=57) {
          nIdxTo=(pTmpChar[i+1]-48)*10+pTmpChar[i+2]-48-1;
        }

        if(nIdxTo>=0) {
          pMat[nIdxFrom][nIdxTo]=1;
        }
        nIdxFrom++;
      }
    }
    fclose(fptr);
  }
}

/**********************************************************************************************/

int main(int argc, char *argv[]) {

  int i,k;
  unsigned int uj;
  unsigned int ui;
  char *relatedToFilename=NULL;
  char *downmixMatFilename = NULL, *bitFilename = NULL, *bitrateFilename = NULL;

  int trAbsNrg=0;
  int timeSlots     = 32;
  int coreTimeSlots = 32;
  int numQMFBands = 64;
  int frameSize   = 2048;
  int inCICPIndex = -2; /* no CICP used */

  CICP2GEOMETRY_CHANNEL_GEOMETRY inGeometryInfo[32] = { {0} }; /*[AZ EL isLFE] */
  int cicpNumChannelsIn, cicpNumLFEs, errorFlag; 

  WAVIO_HANDLE hWavIO = 0;
  WAVIO_HANDLE hWavIO_real = 0;
  WAVIO_HANDLE hWavIO_imag = 0;

  char* inFile        = 0;
  char qmfInFileReal[200];
  char qmfInFileImag[200];

  char* outFile       = 0;
  char qmfOutFileReal[200];
  char qmfOutFileImag[200];
  char* wavExtension  = ".wav";
  char* inFilenameInputChannelList = NULL;

  FILE *pInFileName   = 0;
  FILE *pQmfInRealFile  = 0;
  FILE *pQmfInImagFile  = 0;

  FILE *pOutFileName    = 0;
  FILE *pQmfOutRealFile = 0;
  FILE *pQmfOutImagFile  = 0;

  FILE *pBitrateFile = 0;
  int error           = 0;

  unsigned int nInChannels  = 0;
  unsigned int nOutChannels  = 0;

  unsigned int n3DAChannels    = 0;
  unsigned int n3DAObjects     = 0;
  unsigned int n3DALFEs        = 0;
  unsigned int n3DAdmxChannels = 0;
  unsigned int n3DAdmxObjects  = 0;

  unsigned int InSampleRate = 0;
  unsigned int InBytedepth  = 0;
  unsigned long nTotalSamplesPerChannel = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  int nSamplesPerChannelFilled = 0;
  float **inBuffer  = 0;
  float **outBuffer = 0;
  float **qmfInRealBuffer = 0;
  float **qmfInImagBuffer = 0;
  float **qmfOutRealBuffer = 0;
  float **qmfOutImagBuffer = 0;

  unsigned long nTotalSamplesReadPerChannel = 0;
  unsigned int isLastFrame = 0;


  int bQmfInput = 0;
  int bQmfOutput = 0;
  int LdMode = 0;
  
  unsigned int nChans = 0;

  int flushingSamplesPerChannel = 0;
  int additionalDelay = 0;
  int outputBitdepth = 0;
  int outputBytedepth = 3;
  int sample = 0;
  int channel = 0;

  unsigned int  nSamplesReadPerChannel = 0;
  unsigned int  nSamplesToWritePerChannel = 0;
  unsigned int  nSamplesWrittenPerChannel = 0;
  unsigned int  nSamplesToWritePerChannel_real = 0;
  unsigned int  nSamplesWrittenPerChannel_real = 0;
  unsigned int  nSamplesToWritePerChannel_imag = 0;
  unsigned int  nSamplesWrittenPerChannel_imag = 0;
  unsigned int  nZerosPaddedBeginning = 0;
  unsigned int  nZerosPaddedEnd = 0;
  int nWavExtensionChars = 0;

  int bufferSize = 0;
  int offset = 0;
  int frameCounter = 0;

  QMFLIB_HYBRID_FILTER_MODE hybridMode = QMFLIB_HYBRID_THREE_TO_TEN;

  Stream bitstream;

  float pDmxMatrix[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS];
  float deqDmxMatrix[MAX_NUM_OUTPUTCHANNELS][MAX_NUM_INPUTCHANNELS];
  int   bsRelatedTo[MAX_OBJECTS][MAX_OBJECTS];

  int saocBitsWritten = 0;

  tSpatialEnc *enc = NULL;

  /* print disclaimer info */
  printDisclaimer();

  if(argc < 6 || !strcmp(argv[1],"-h") || !strcmp(argv[1],"--help")) {
    fprintf(stderr, "\nCommand line switches:\n\n");
    fprintf(stderr, "  -if <input.wav> or <input> \n the latter will be extended to <input>_real.qmf and <input>_imag.qmf\n");
    fprintf(stderr, "  -of <output.wav> or <output> \n the latter will be extended to <output>_real.qmf and <output>_imag.qmf\n");
    fprintf(stderr, "  -bs <filename>   filename of the bitstream output file\n");
    fprintf(stderr, "  -br <filename>   filename of the output file containing the number of SAOC bits written per frame\n"); 
    fprintf(stderr, "  -dm <filename>   downmix matrix file\n");
    fprintf(stderr, "  -rt <filename>   relatedTo file (optional)\n");
    fprintf(stderr, "  -numChan <number> number of 3DA input channels \n");
    fprintf(stderr, "  -numObj <number> number of 3DA input objects \n");
    fprintf(stderr, "  -numLFE <number> number of 3DA input LFEs \n");
    fprintf(stderr, "  -numDmxChan <number> number of 3DA downmix channels \n");
    fprintf(stderr, "  -numDmxObj <number> number of 3DA downmix objects \n");
    fprintf(stderr, "  -ts <number>     times slots, 16 or 32 [32 is the default]\n");
    fprintf(stderr, "  -core_ts <number>     number of core coder times slots, 16 or 32 [32 is the default]\n");
    fprintf(stderr, "  -cicpIn   <inputConfigIdx>   input config idx\n");
    fprintf(stderr, "       ( -1: LIST_OF_CHANNELS, 0: GENERIC, >0: CICP_index )\n");
    fprintf(stderr, "   LIST_OF_CHANNELS signaling of setups will read from file (use -inGeo <inconf.txt>).\n");
    /* Channel path not active in Reference Software SAOC 3D encoder */
    fprintf(stderr, "   Channel path not active in Reference Software SAOC 3D encoder! .\n");
    return -1;
  }

  for(i = 1; i < argc; i++) {
    char *arg = argv[i];
    if(!strcmp(arg, "-if")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -if\n\n");
        return -1;
      }
      nWavExtensionChars = 0;
      inFile = argv[i+1];
      nWavExtensionChars = strspn(wavExtension,inFile);
      if ( nWavExtensionChars != 4 ) {
        bQmfInput = 1;
        strcpy(qmfInFileReal, inFile);
        strcpy(qmfInFileImag, inFile);
        strcat(qmfInFileReal, "_real.qmf");
        strcat(qmfInFileImag, "_imag.qmf");
      }
      i++;
    } else if(!strcmp(arg, "-of")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -of\n\n");
        return -1;
      }
      nWavExtensionChars = 0;
      outFile = argv[i+1];
      nWavExtensionChars = strspn(wavExtension,outFile);
      if ( nWavExtensionChars != 4 ) {
        bQmfOutput = 1;
        strcpy(qmfOutFileReal, outFile);
        strcpy(qmfOutFileImag, outFile);
        strcat(qmfOutFileReal, "_real.qmf");
        strcat(qmfOutFileImag, "_imag.qmf");
      }
      i++;
    } else if(!strcmp(arg, "-rt")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -rt\n\n");
        return -1;
      }
      relatedToFilename = argv[i+1];
      i++;
    } else if(!strcmp(arg, "-dm")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -dm\n\n");
        return -1;
      }
      downmixMatFilename = argv[i+1];
      i++;
    } else if(!strcmp(arg, "-bs")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -bs\n\n");
        return -1;
      }
      bitFilename = argv[i+1];
      i++;
    } else if(!strcmp(arg, "-br")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -br\n\n");
        return -1;
      }
      bitrateFilename = argv[i+1];
      i++;
    } else if(!strcmp(arg, "-numChan")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -numChan\n\n");
        return -1;
      }
      n3DAChannels = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(arg, "-numObj")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -numObj\n\n");
        return -1;
      }
      n3DAObjects = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(arg, "-numLFE")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -numLFE\n\n");
        return -1;
      }
      n3DALFEs  = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(arg, "-numDmxChan")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -numDmxChan\n\n");
        return -1;
      }
      n3DAdmxChannels  = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(arg, "-numDmxObj")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -numDmxObj\n\n");
        return -1;
      }
      n3DAdmxObjects  = atoi(argv[i+1]);
      i++;
    } else if(!strcmp(arg, "-ts")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -ts\n\n");
        return -1;
      }
      timeSlots = atoi(argv[i+1]);
      i++;

    } else if(!strcmp(arg, "-core_ts")) {
      if(i == argc-1) {
        fprintf(stderr, "\nArgument expected for -core_ts\n\n");
        return -1;
      }
      coreTimeSlots = atoi(argv[i+1]);
      i++;

    } else if(!strcmp(arg, "-cicpIn")) {  /* Required */
		  if(i == argc-1) {
			  fprintf(stderr, "\nArgument expected for -cicpIn\n\n");
			  return -1;
		  }
		  inCICPIndex = atoi(argv[i+1]);
		  i++;
	  } else if (!strcmp(arg,"-inGeo")) { /* needed if cicpIn==-1 */           
      
        if(i == argc-1) {
          fprintf(stderr, "\nArgument expected for -inGeo\n\n");
          return -1;
        }
        inFilenameInputChannelList = argv[i+1];                              
        i++;

    } else {
      fprintf(stderr, "\nUnknown command line switch %s\n\n", arg);
      /* return -1; */
    }
  }


  if(inFile == NULL) {
    fprintf(stderr, "\nNo input file specified\n\n");
    return -1;
  }

  if(outFile == NULL) {
    fprintf(stderr, "\nNo downmix file specified\n\n");
    return -1;
  }

  if(bitFilename == NULL) {
    fprintf(stderr, "\nNo bitstream file specified\n\n");
    return -1;
  }

  if(bitrateFilename == NULL) {
    fprintf(stderr, "\nNo bitrate file specified\n\n");
    return -1;
  }

  if (inCICPIndex != -2) {
    /* Get Azimuth, Elevations from CICP */
    if ( -1 == inCICPIndex )
    {
      /* Read configuration from txt file */
      if(inFilenameInputChannelList==NULL){
        fprintf(stderr,"Error: Missing filename for output setup geometry file. Use -inGeo <file.txt> switch.\n");
        return -1;
      }
      errorFlag = cicp2geometry_get_geometry_from_file(inFilenameInputChannelList, inGeometryInfo, &cicpNumChannelsIn, &cicpNumLFEs);
    }
    else {/* setup from CICP setup index */
      errorFlag = cicp2geometry_get_geometry_from_cicp(inCICPIndex, inGeometryInfo, &cicpNumChannelsIn, &cicpNumLFEs); 
    }
    
    if ( 0 != errorFlag ) {
      fprintf(stderr, "Error during getting output setup geometry.\n");
      return -1;
    }    
  }

  pBitrateFile = fopen(bitrateFilename, "w");


  nInChannels  = n3DAChannels + n3DAObjects;
  nOutChannels = n3DAdmxChannels + n3DAdmxObjects;

  frameSize = timeSlots * numQMFBands;
  LdMode = 0;

  InitDownmixMat(downmixMatFilename, pDmxMatrix, nInChannels, nOutChannels);
  InitRelatedToMat(relatedToFilename,bsRelatedTo);

  InitStream(&bitstream, bitFilename, STREAM_WRITE);


  if ( LdMode ) {
    flushingSamplesPerChannel = numQMFBands * 2.5 + /* analysis */
                                numQMFBands * 1.5 ; /* synthesis */
  } else {
    flushingSamplesPerChannel = numQMFBands * 5 +     /* analysis */
                                numQMFBands * 4 + 1 ; /* synthesis */
  }

  if ( QMFLIB_HYBRID_OFF != hybridMode ) {
    flushingSamplesPerChannel += 6 * numQMFBands;
  }

  flushingSamplesPerChannel += additionalDelay;

  if ( bQmfInput ) {
    pQmfInRealFile  = fopen(qmfInFileReal, "rb");
    pQmfInImagFile  = fopen(qmfInFileImag, "rb");
  } else {
    pInFileName  = fopen(inFile, "rb");
  }

  if ( bQmfOutput ) {
    pQmfOutRealFile = fopen(qmfOutFileReal, "wb");
    pQmfOutImagFile = fopen(qmfOutFileImag, "wb");
  } else {
    pOutFileName = fopen(outFile, "wb");
  }

  /* wavIO for wav file */
  error = wavIO_init(&hWavIO, frameSize, 0, -flushingSamplesPerChannel);
  if ( 0 != error ) {
    fprintf(stderr, "Error during initialization.\n");
    return -1;
  }

  /* wavIO for qmf file */
  if ( bQmfInput || bQmfOutput ) {
    error = wavIO_init(&hWavIO_real, frameSize, 0, 0);
    if ( 0 != error ) {
      fprintf(stderr, "Error during initialization.\n");
      return -1;
    }
    error = wavIO_init(&hWavIO_imag, frameSize, 0, 0);
    if ( 0 != error ) {
      fprintf(stderr, "Error during initialization.\n");
      return -1;
    } 
  }

  if ( bQmfInput ) {
    error = wavIO_openRead(hWavIO_real, pQmfInRealFile, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening input qmf real file\n");
      return -1;
    }
    error = wavIO_openRead(hWavIO_imag, pQmfInImagFile, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening input qmf imag file\n");
      return -1;
    }
  }

  /* wav input */
  if( !bQmfInput ) {
    error = wavIO_openRead(hWavIO, pInFileName, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening input wav\n");
      return -1;
    }
    outputBytedepth = InBytedepth;
  }

  /* wav output */
  if( !bQmfOutput ) {
    error = wavIO_openWrite(hWavIO, pOutFileName, nOutChannels, InSampleRate, outputBytedepth);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening output wav\n");
      return -1;
    }
  }

  if ( bQmfOutput ) {
    error = wavIO_openWrite(hWavIO_real, pQmfOutRealFile, nOutChannels, InSampleRate, QMF_BYTE_DEPTH);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening output qmf\n");
      return -1;
    }
    error = wavIO_openWrite(hWavIO_imag, pQmfOutImagFile, nOutChannels, InSampleRate, QMF_BYTE_DEPTH);
    if ( 0 != error ) {
      fprintf(stderr, "Error opening output qmf\n");
      return -1;
    }
  }

  if( bQmfInput ) {
    /* allocate buffer for qmf input */
    qmfInRealBuffer = (float**) calloc(nInChannels, sizeof(float*));
    qmfInImagBuffer = (float**) calloc(nInChannels, sizeof(float*));

    for (nChans = 0; nChans < nInChannels; ++nChans ) {
      qmfInRealBuffer[nChans] = calloc(frameSize, sizeof(float));
      qmfInImagBuffer[nChans] = calloc(frameSize, sizeof(float));
    }
  } else {
    inBuffer  = (float**) calloc(nInChannels, sizeof(float*));
    for (nChans = 0; nChans < nInChannels; ++nChans ) {
      inBuffer[nChans] = calloc(frameSize, sizeof(float));
      inBuffer[nChans] = calloc(frameSize, sizeof(float));
    }
  }

  if( bQmfInput ) {
    /* allocate buffer for qmf output */
    qmfOutRealBuffer = (float**) calloc(nOutChannels, sizeof(float*));
    qmfOutImagBuffer = (float**) calloc(nOutChannels, sizeof(float*));

    for (nChans = 0; nChans < nOutChannels; ++nChans ) {
      qmfOutRealBuffer[nChans] = calloc(frameSize, sizeof(float));
      qmfOutImagBuffer[nChans] = calloc(frameSize, sizeof(float));
    }
  } else {
    outBuffer  = (float**) calloc(nOutChannels, sizeof(float*));
    for (nChans = 0; nChans < nOutChannels; ++nChans ) {
      outBuffer[nChans] = calloc(frameSize, sizeof(float));
      outBuffer[nChans] = calloc(frameSize, sizeof(float));
    }
  }

  enc = SaocEncOpen(nInChannels,
                    bsRelatedTo,
                    nOutChannels,
                    timeSlots,
                    coreTimeSlots,
                    trAbsNrg,
                    &bufferSize,
                    &bitstream, 
                    hybridMode,
                    LdMode,
                    numQMFBands,
                    bQmfInput,
                    n3DAChannels,
                    n3DAObjects,
                    n3DALFEs,
                    n3DAdmxChannels,
                    n3DAdmxObjects);

#ifdef RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
    /* write number of SAOC bits written */
    {
      int saocBitsToWrite = bitstream.bitsInBuffer - saocBitsWritten;
      fprintf(pBitrateFile,"%d\n", saocBitsToWrite);
      saocBitsWritten = bitstream.bitsInBuffer;
    }
#endif

  if(enc == NULL) {
    fprintf(stderr, "\nFailed to open spatial encoder\n\n");
    return -1;
  }

  do /*loop over all frames*/ 
  {

    nSamplesReadPerChannel = 0;
    nSamplesToWritePerChannel = 0;
    nSamplesWrittenPerChannel = 0;
    nSamplesToWritePerChannel_real = 0;
    nSamplesWrittenPerChannel_real = 0;
    nSamplesToWritePerChannel_imag = 0;
    nSamplesWrittenPerChannel_imag = 0;
    nZerosPaddedBeginning = 0;
    nZerosPaddedEnd = 0;
    uj = 0;

    if ( !bQmfInput ) {

      /* read frame */
      if ( !isLastFrame ) {
        wavIO_readFrame(hWavIO, inBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
        nSamplesToWritePerChannel      = nSamplesReadPerChannel;
        nTotalSamplesReadPerChannel   += nSamplesReadPerChannel;
      }

      /* Flush the qmf transformation / Calculate flushing samples */
      if ( nSamplesReadPerChannel < (unsigned int) frameSize ) {

        int samplesToAdd = 0;

        if ( flushingSamplesPerChannel < frameSize ) {
          if ( flushingSamplesPerChannel == 0 ) {
            flushingSamplesPerChannel = -1;
          } 
          else if ( flushingSamplesPerChannel < 0 ) {
            samplesToAdd = flushingSamplesPerChannel + frameSize;
            nSamplesToWritePerChannel = samplesToAdd;
          }
          else {
            samplesToAdd = flushingSamplesPerChannel;
            flushingSamplesPerChannel = -1;
            nSamplesToWritePerChannel = samplesToAdd;
          }
        }
        else {
          samplesToAdd = frameSize;
          nSamplesToWritePerChannel = samplesToAdd;
        }

        for ( channel = 0; channel < (int) nInChannels; ++channel ) {
          for ( sample = nSamplesReadPerChannel; sample < frameSize; ++sample ) {
            inBuffer[channel][sample] = 0.0f;
          }
        }
      }
    } else {
      /* read qmf frame */
      wavIO_readFrame(hWavIO_real, qmfInRealBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
      wavIO_readFrame(hWavIO_imag, qmfInImagBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);

      nSamplesToWritePerChannel      = nSamplesReadPerChannel;
      nSamplesToWritePerChannel_real = nSamplesReadPerChannel;
      nSamplesToWritePerChannel_imag = nSamplesReadPerChannel;
      nTotalSamplesReadPerChannel   += nSamplesReadPerChannel;

    }

    SaocEncApply(enc,
                 inBuffer,
                 qmfInRealBuffer,
                 qmfInImagBuffer,
                 nInChannels,
                 &bitstream,
                 nOutChannels,
                 pDmxMatrix,
                 deqDmxMatrix,
                 bQmfInput,
                 numQMFBands,
                 hybridMode,
                 LdMode);
  

    /* downmix */
    if (!bQmfInput) {
      for (uj = 0; uj < nOutChannels; uj++) {
        for (k = 0; k < bufferSize; k++) {
          outBuffer[uj][k] = 0.0f;
          for (ui = 0; ui < nInChannels; ui++) {
            outBuffer[uj][k] += deqDmxMatrix[uj][ui] * inBuffer[ui][k];
          }
        }
      }
    } else {
      for (uj = 0; uj < nOutChannels; uj++) {
        for (k = 0; k < bufferSize; k++) {
          qmfOutRealBuffer[uj][k] = 0.0f;
          qmfOutRealBuffer[uj][k] = 0.0f;
          for (ui = 0; ui < nInChannels; ui++) {
            qmfOutRealBuffer[uj][k] += qmfInRealBuffer[ui][k] * deqDmxMatrix[uj][ui];
            qmfOutRealBuffer[uj][k] += qmfInRealBuffer[ui][k] * deqDmxMatrix[uj][ui];
          }
        }
      }
    }

    /* write frame - wav file */
    if ( !bQmfOutput ) {
      wavIO_writeFrame(hWavIO, outBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
    } else {
      wavIO_writeFrame(hWavIO_real, qmfOutRealBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel_real);
      wavIO_writeFrame(hWavIO_imag, qmfOutImagBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel_imag);
    }


    /* write number of SAOC bits written */
    {
      int saocBitsToWrite = bitstream.bitsInBuffer - saocBitsWritten;
      fprintf(pBitrateFile,"%d\n", saocBitsToWrite);
      saocBitsWritten = bitstream.bitsInBuffer;
    }


    flushingSamplesPerChannel -= ( frameSize - nSamplesReadPerChannel );

    offset += bufferSize;

    printf("Processing frame No. %d\r", frameCounter);

    fflush(stdout);

    frameCounter++;

  }  while ( !isLastFrame || flushingSamplesPerChannel >= 0 );

  SaocEncClose(enc, nInChannels);

  CloseStream(&bitstream);

  if ( bQmfOutput ) {
    wavIO_updateWavHeader(hWavIO_real, &nTotalSamplesWrittenPerChannel);
    wavIO_updateWavHeader(hWavIO_imag, &nTotalSamplesWrittenPerChannel);
  } else {
    wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
  }

  if ( bQmfInput || bQmfOutput ) {
    wavIO_close(hWavIO_real);
    wavIO_close(hWavIO_imag);
  }
  wavIO_close(hWavIO);

  for (ui = 0; ui < nInChannels; ui++) {
    if (!bQmfInput) {
      free(inBuffer[ui]);
    } else {
      free(qmfInRealBuffer[ui]);
      free(qmfInImagBuffer[ui]);
    }
  }

  for (ui = 0; ui < nOutChannels; ui++) {
    if (!bQmfOutput) {
      free(outBuffer[ui]);
    } else {
      free(qmfOutRealBuffer[ui]);
      free(qmfOutImagBuffer[ui]);
    }
  }

  free(inBuffer);
  free(outBuffer);
  free(qmfInRealBuffer);
  free(qmfInImagBuffer);
  free(qmfOutRealBuffer);
  free(qmfOutImagBuffer);

  fclose(pBitrateFile);

  return 0;
}

void printDisclaimer( void ) {
  fprintf(stdout, "\n\n");
  fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                           SAOC 3D Encoder Module                            *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*                                %s                                  *\n", __DATE__);
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf(stdout, "*                                                                             *\n");
  fprintf(stdout, "*******************************************************************************\n\n");
}
