/**********************************************************************
MPEG-4 Audio VM

This software module was originally developed by

Heiko Purnhagen     (University of Hannover / ACTS-MoMuSys)

and edited by

Naoya Tanaka        (Matsushita Communication Industrial Co., Ltd.)
Ralph Sperschneider (Fraunhofer Gesellschaft IIS)
Ali Nowbakht-Irani  (Fraunhofer Gesellschaft IIS)
Markus Werner       (SEED / Software Development Karlsruhe) 

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

$Id: mp4audec.c,v 1.4 2011-05-28 14:25:18 mul Exp $
Decoder frame work


Source file: mp4dec.c


Required modules:
common.o                common module
cmdline.o               command line module
bitstream.o             bit stream module
audio.o                 audio i/o module
dec_par.o               decoder core (parametric)
dec_lpc.o               decoder core (CELP)
dec_tf.o                decoder core (T/F)

Authors:
HP    Heiko Purnhagen, Uni Hannover <purnhage@tnt.uni-hannover.de>
NT    Naoya Tanaka, Panasonic <natanaka@telecom.mci.mei.co.jp>
BT    Bodo Teichmann, FhG/IIS <tmn@iis.fhg.de>
CCETT N.N., CCETT <@ccett.fr>
OK    Olaf Kaehler, Fhg/IIS <kaehleof@iis.fhg.de>

Changes:
18-jun-96   HP    first version
19-jun-96   HP    added .wav format / using new ComposeFileName()
26-jun-96   HP    improved handling of switch -o
04-jul-96   HP    joined with t/f code by BG
09-aug-96   HP    adapted to new cmdline module, added mp4.h
16-aug-96   HP    adapted to new dec.h
                  added multichannel signal handling
                  added cmdline parameters for numChannel, fSample
26-aug-96   HP    CVS
03-sep-96   HP    added speed change & pitch change for parametric core
30-oct-96   HP    additional frame work options
15-nov-96   HP    adapted to new bitstream module
18-nov-96   HP    changed int to long where required
                  added bit stream header options
10-dec-96   HP    added variable bit rate
10-jan-97   HP    using BsGetSkip()
23-jan-97   HP    added audio i/o module
03-feb-97   HP    audio module bug fix
07-feb-97   NT    added PICOLA speed control
14-mar-97   HP    merged FhG AAC code
21-mar-97   BT    various changes (t/f, AAC)
04-apr-97   HP    new option -r for scalable decoder
26-mar-97   CCETT added G729 decoder
07-apr-97   HP    i/o filename handling improved / "-" supported
05-nov-97   HP    update by FhG/UER
30-mar-98   HP    added ts option
06-may-98   HP    aacEOF
07-jul-98   HP    improved bitrate statistics debug output
02-dec-98   HP    merged most FDIS contributions ...
20-jan-99   HP    due to the nature of some of the modifications merged
                  into this code, I disclaim any responsibility for this
		  code and/or its readability -- sorry ...
21-jan-99   HP	  trying to clean up a bit ...
22-jan-99   HP    stdin/stderr "-" support, -d 1 uses stderr
22-apr-99   HP    merging all contributions
19-nov-99   HP    added MOUSECHANGE
19-jun-01   OK    removed G723/G729 dummy calls
21-jul-03   AR/RJ Added Extension 2 parametric coding
28-jan-04   OK    Added program switch
**********************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_descr.h"           /* structs */
#include "common_m4a.h"          /* common module       */
#include "cmdline.h"             /* command line module */
#include "audio.h"               /* audio i/o module    */
#include "mp4au.h"               /* frame work common declarations */

#include "hvxc_struct.h"	      /* struct for HVXC(AI 990616) */
#include "dec_par.h"            /* decoder cores ... */
#include "dec_lpc.h"
#include "dec_tf.h"
#ifdef EXT2PAR
#include "SscDec.h"
#endif

#ifdef MPEG12
#include "dec_mpeg12.h"
#endif

#include "statistics.h"
#include "mp4ifc.h"

#ifdef  DEBUGPLOT
#include "plotmtv.h"
#endif

#include "wavIO.h"


extern int bWriteIEEEFloat; /* defined in audio.c */

/* 
  command line module 
*/

static int   extendedHelp;
static int   maxLayer;
static int   programNr;
static char* decPara;
static char* outFileName;
static int   outFileNameUsed;
char*        g_mixMatrixFileName;
int          g_mixMatrixFileNameUsed;
static int   loopInFile;
static char* bitPath;
static char* decPath;
static int   bitPathUsed;
static int   decPathUsed;
static char* bitExt;
static int   int24flag;
static int   monoFilesOut;
static char* decExt;
static int   decExtUsed;
static int   mainDebugLevel;
static int   audioDebugLevel;
static int   bitDebugLevel;
static int   cmdDebugLevel;
static char* aacDebugString; /* Debug options for aac */
static int   epDebugLevel;
static int*  varArgIdx;
#ifdef CT_SBR
static int   HEaacProfileLevel;
static int   bUseHQtransposer;
#endif
static int   USACDecMode; 
static char* objFile  = NULL;
static char* saocFile = NULL;

int qmfout;
FILE *qmfRealOutputFile = NULL;
FILE *qmfImagOutputFile = NULL;


WAVIO_HANDLE handleWavIO_real;
WAVIO_HANDLE handleWavIO_imag;

float *** qmfRealOutBuffer;
float *** qmfImagOutBuffer;

#ifdef	HUAWEI_TFPP_DEC
static int   actATFPP;
#endif  

int gUseFractionalDelayDecor = 0;
int gUseFftHarmonicTransposer = 0; 



/*
  debug variables
*/

#define INF_FILE_LEN 1024
static int   epFlag;
static char  infoFileName[INF_FILE_LEN];/* for EP mode only */

#ifdef DEBUGPLOT
extern int firstFrame;  /* global for debugging purposes */
#endif

#ifndef DEFAULT_NUM_TIMESLOTS
#define DEFAULT_NUM_TIMESLOTS (64)
#endif

#ifndef DEFAULT_NUM_QMFBANDS
#define DEFAULT_NUM_QMFBANDS  (64)
#endif

#ifndef MAX_NUM_OUT_CHANNELS
#define MAX_NUM_OUT_CHANNELS (48)
#endif

static CmdLinePara paraList[] = {
  {&varArgIdx,NULL,"<bit stream file(s) (- = stdin)>"},
  {NULL,NULL,NULL}
};

static CmdLineSwitch switchList[] = {
  {"h",NULL,NULL,NULL,NULL,"print help",0},
  {"xh",&extendedHelp,NULL,NULL,NULL,"print extended help",0},

  {"qmfout",&qmfout,NULL,NULL,&qmfout,"write qmf output files",0}, 

  {"maxl",&maxLayer,"%d","-1",NULL,"number of enhancement layers "
   "(-1=all layers)",1},
  
  {"ep",&epFlag,NULL,NULL,NULL,
   "enables error protection (requires error resilient syntax)",1},
  {"c",&decPara,"%s","",NULL,
   "decoder parameter string",1},
  {"o",&outFileName,"%s",NULL,&outFileNameUsed,
   "output file name (- = stderr)",0},
  {"mixMatrix",&g_mixMatrixFileName,"%s",NULL,&g_mixMatrixFileNameUsed,
   "output file name (- = stderr)",0},
  {"p",&programNr,"%d","-1",NULL,"program to decode (in multi-program streams)",1},
  {"li",&loopInFile,"%d","1",NULL,
   "loop input files n times (use with -o -)",1},
  {"pb",&bitPath,"%s",NULL,&bitPathUsed,
   "default path bit stream files (dflt: $" MP4_BIT_PATH_ENV ")",1},
  {"pd",&decPath,"%s",NULL,&decPathUsed,
   "default path decoded audio files (dflt: $" MP4_DEC_PATH_ENV ")",1},
  {"eb",&bitExt,"%s",MP4_BIT_EXT,NULL,"bit stream file extension",1},
  {"ed",&decExt,"%s",NULL,&decExtUsed,
   "decoded audio file extension\n"
#ifdef USE_AFSP
   "supported file formats: .au, .snd, .wav, .aif, .raw\n"
   "(dflt: $" MP4_DEC_FMT_ENV " / " MP4_DEC_EXT ")\n"
   "(raw format: integer16, native)",1},
  {"int24",&int24flag,NULL,NULL,NULL,
  "output is: two's-complement 24-bit integer data\n"
  "(dflt: two's-complement 16-bit integer data)"
#else
   "supported file formats: .au, .snd\n"
   "(dflt: $" MP4_DEC_FMT_ENV " / " MP4_DEC_EXT ")\n"
   "(raw format: integer16, native)"
#endif /*USE_AFSP*/
  ,0},
   {"float",&bWriteIEEEFloat,NULL,NULL,NULL,
    "output is: IEEE float\n"
    "(dflt: PCM)",1},
#ifdef I2R_LOSSLESS 
  /* {"osf",&osf,"%d","1",NULL,"oversampling factor for lossless enhancement (dflt: 1, options: 1,2,4)"}, */
#endif
  {"m",&monoFilesOut,NULL,NULL,NULL,"output each channel in a individual mono file",1},
  {"d",&mainDebugLevel,"%i","0",NULL,"main debug level",1},
#ifdef DEBUGPLOT
  {"ps",&firstFrame,"%i","9999",NULL,"start plot at frame%",1},
#endif
  {"da",&audioDebugLevel,"%i","0",NULL,"audio file debug level",1},
  {"db",&bitDebugLevel,"%i","0",NULL,"bit stream debug level",1},
  {"dc",&cmdDebugLevel,"%i","0",NULL,"cmd line debug level",1},
  {"daac",&aacDebugString,"%s","",NULL,"[a-Z] Debug Options for aac-Decoder",1},
  {"dep",&epDebugLevel,"%i","0",NULL,"error protection debug level",1},
#ifdef CT_SBR
  {"HEAacProfileLevel",&HEaacProfileLevel,"%i","5",NULL,"Level of the High Efficiency AAC Profile",1},
  {"HbeUseHQ",&bUseHQtransposer,NULL,NULL,NULL,"Use HQ (FFT) transposer",1},
#endif
#ifdef HUAWEI_TFPP_DEC
  {"ATFPP",&actATFPP,NULL,NULL,NULL,"Use Adaptive T/F Post-processing",1},
#endif

  {"USACDecMode",&USACDecMode,"%i","0",NULL,
   "USAC Decoder modes\n"
   "1: Fractional Delay Decorrelator,\n"
   "2: FFT based Harmonic Transposer,\n"
   "3: FFT based Harmonic Transposer and Fractional Delay Decorrelator\n",1},

   {"objFile",&objFile,"%s",NULL,NULL,
   "output object file name, must end with '_0.dat' ",0},

   {"saocFile",&saocFile,"%s",NULL,NULL,
   "output saoc file name",0},


  {NULL,NULL,NULL,NULL,NULL,NULL,NULL}
};

static void MakeInfoFileName(char *bitFileName, char *infoFileName)
{
  char *writepos;
  short len;

  assert( INF_FILE_LEN > (int) strlen(bitFileName) );
  strcpy(infoFileName, bitFileName);
  len = strlen(bitFileName);
  writepos = infoFileName + len;
  
  if ((len >= 4) && (!strcmp(writepos - 4, ".bit") || !strcmp(writepos - 4, MP4_BIT_EXT)))
    writepos -= 4;
  sprintf(writepos,".inf");
}


int prepareWavIO( WAVIO_HANDLE * handleWavIO_real, WAVIO_HANDLE * handleWavIO_imag) {
  int error_init = 0;
  int bFillLastFrame = 0;
  int moduleDelay = 0;
  int channel = 0;
  int ts = 0;
  
  qmfRealOutBuffer = (float ***) calloc( DEFAULT_NUM_TIMESLOTS, sizeof(float**) );
  qmfImagOutBuffer = (float ***) calloc( DEFAULT_NUM_TIMESLOTS, sizeof(float**) );

  for ( ts = 0; ts < DEFAULT_NUM_TIMESLOTS; ++ts ) {

    qmfRealOutBuffer[ts] = (float **) calloc( MAX_NUM_OUT_CHANNELS, sizeof(float*) );
    qmfImagOutBuffer[ts] = (float **) calloc( MAX_NUM_OUT_CHANNELS, sizeof(float*) );
    
    for ( channel = 0; channel < MAX_NUM_OUT_CHANNELS; ++channel ) {
      qmfRealOutBuffer[ts][channel] = (float *) calloc( DEFAULT_NUM_QMFBANDS, sizeof(float) );
      qmfImagOutBuffer[ts][channel] = (float *) calloc( DEFAULT_NUM_QMFBANDS, sizeof(float) );
    }
  }




  if (qmfout){
    error_init = wavIO_init(handleWavIO_real,
                            DEFAULT_NUM_QMFBANDS,
                            bFillLastFrame,
                            moduleDelay);

    error_init = wavIO_init(handleWavIO_imag,
                            DEFAULT_NUM_QMFBANDS,
                            bFillLastFrame,
                            moduleDelay);
  }


  return error_init;

}


/* 
  main
*/

int main (int argc, char *argv[])
{
  char *progName = "<no program name>";
  int  result;
  char bitFileName[STRLEN];
  char decFileName[STRLEN];
  char decFileNameImag[STRLEN];
  char *decFormat;
  int  fileIdx;
  unsigned long writtenSamples = 0;
  /* evaluate command line  */
  
  CmdLineInit(0);
  
  fprintf( stderr, "\n");
  fprintf( stderr, "This binary is outdated, use 3DAudioCoreDecoder instead\n");
  fprintf( stderr, "*******************************************************************************\n");
 
  fprintf( stderr, "\n");

  
  
  result = CmdLineEval(argc,argv,paraList,switchList,1,&progName,&extendedHelp);
  
  if (result) {
    if (result==1) {
      CmdLineHelp(progName,paraList,switchList,stderr);
      DecParInfo(stderr,extendedHelp);
      DecLpcInfo(stderr,extendedHelp);
      DecTfInfo(stderr,extendedHelp);
#ifdef EXT2PAR
      DecSSCInfo(stderr);
#endif
#ifdef MPEG12
      DecMPEG12Info(stderr);
#endif
      exit (1);
    }
    else
      CommonExit(1,"command line error (\"-h\" for help)");
  }

  SetDebugLevel (mainDebugLevel) ;

  if (mainDebugLevel >= 2) {
    printf("%s\n",DecParInfo(NULL,0));
    printf("%s\n",DecLpcInfo(NULL,0));
    printf("%s\n",DecTfInfo(NULL,0));
#ifdef EXT2PAR
    printf("%s\n",DecSSCInfo(NULL));
#endif
  }

  CmdLineInit(cmdDebugLevel);

  /* calc variable default values */
  if (!bitPathUsed)
    bitPath = getenv(MP4_BIT_PATH_ENV);
  if (!decPathUsed)
    decPath = getenv(MP4_DEC_PATH_ENV);
  if (!decExtUsed)
    decExt = getenv(MP4_DEC_FMT_ENV);
  if (decExt==NULL)
    decExt = MP4_DEC_EXT;

  /* check command line options */
  if (outFileNameUsed && varArgIdx[0]>=0 && varArgIdx[1]>=0 &&
      strcmp(outFileName,"-")!=0)
    CommonExit(1,"only one input file allowed when using -o");
  if (varArgIdx[0]<0)
    CommonExit(1,"no input file specified");
  
  /* process all files on command line */
  fileIdx = 0;

  /* Open output file */
  if ( outFileName && qmfout)
  {
    char outFileNameReal[200];
    char outFileNameImag[200];
    strcpy(outFileNameReal, outFileName);
    strcat(outFileNameReal, "_real.qmf");
    qmfRealOutputFile = fopen(outFileNameReal, "wb");
    if ( qmfRealOutputFile != NULL) {
      fprintf(stderr, "Write to output file: %s.\n", qmfRealOutputFile);
    }
    else {
      fprintf(stderr, "Could not open output file: %s.\n", qmfRealOutputFile );
      return -1;
    }

    strcpy(outFileNameImag, outFileName);
    strcat(outFileNameImag, "_imag.qmf");
    qmfImagOutputFile = fopen(outFileNameImag, "wb");
    if ( qmfImagOutputFile != NULL) {
      fprintf(stderr, "Write to output file: %s.\n", qmfImagOutputFile);
    }
    else {
      fprintf(stderr, "Could not open output file: %s.\n", qmfImagOutputFile );
      return -1;
    }

  }

  prepareWavIO(&handleWavIO_real, &handleWavIO_imag);

  while (loopInFile) {
    /* compose file names */
    if (ComposeFileName(argv[varArgIdx[fileIdx]],0,bitPath,bitExt,bitFileName,
                        STRLEN))
      CommonExit(1,"composed file name too long");
    if ( epFlag )
      MakeInfoFileName(bitFileName, infoFileName);
    if ( outFileNameUsed ) {
      if (ComposeFileName(outFileName,0,decPath,decExt,decFileName,STRLEN))
        CommonExit(1,"composed file name too long");
    }
    else
      if (ComposeFileName(argv[varArgIdx[fileIdx]],1,decPath,decExt,
                          decFileName,STRLEN))
        CommonExit(1,"composed file name too long");

    /* extract file format (extension) */
    if (strcmp(decFileName,"-") == 0)
      decFormat = decExt;
    else {
      decFormat = decFileName+strlen(decFileName);
      do
        decFormat--;
      while (decFormat>decFileName && *decFormat!='.');
    }

    /* decode file */
    if (mainDebugLevel == 1)
      fprintf(stderr,"decoding %s -> %s\n",bitFileName,decFileName);
    if (mainDebugLevel >= 2)
      printf("decoding %s -> %s\n",bitFileName,decFileName);

  
    if ( epFlag ) {
      if (mainDebugLevel == 1) 
        fprintf(stderr,"crc information - %s\n",infoFileName);
      if (mainDebugLevel >= 2)
        printf("crc information - %s\n",infoFileName);
    }
    
    /* Set USAC Decoder Mode */
    gUseFractionalDelayDecor = (USACDecMode & 1);
    gUseFftHarmonicTransposer =  ((USACDecMode >> 1) & 1);
    
    if (MP4Audio_ProbeFile (bitFileName)){
        MP4Audio_DecodeFile (bitFileName,
                             decFileName,
                             decFormat,
                             int24flag,
                             monoFilesOut,
                             maxLayer,
                             decPara,
                             mainDebugLevel,
                             audioDebugLevel,
                             epDebugLevel,
                             bitDebugLevel,
                             aacDebugString,
#ifdef CT_SBR
                             HEaacProfileLevel,
                             bUseHQtransposer,
#endif
#ifdef HUAWEI_TFPP_DEC
                             actATFPP,
#endif
                             programNr,
                             objFile,
                             saocFile
                             );
    }
    else{
      CommonWarning("error decoding audio file %s",bitFileName);
    }

    fileIdx++;
    if (varArgIdx[fileIdx] < 0) {
      fileIdx = 0;
      loopInFile--;
    }
  } /* end while(loopInFile) */

  if ( qmfout ) {
    wavIO_updateWavHeader( handleWavIO_real, &writtenSamples);
    wavIO_updateWavHeader( handleWavIO_imag, &writtenSamples);
  }


  CmdLineEvalFree(paraList);

  StatisticsPrint ( epFlag ) ;
  
  if (mainDebugLevel == 1)
    fprintf(stderr,"%s: finished\n",progName);
  if (mainDebugLevel >= 2)
    printf("%s: finished\n",progName);

  return 0;
}

/* end of mp4dec.c */

