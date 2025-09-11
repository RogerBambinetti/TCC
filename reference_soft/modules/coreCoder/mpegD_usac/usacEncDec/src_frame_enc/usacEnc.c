/**********************************************************************
MPEG-4 Audio VM
Encoder frame work



This software module was originally developed by

Heiko Purnhagen (University of Hannover / ACTS-MoMuSys)

and edited by

Olaf Kaehler (Fraunhofer IIS-A)
Guillaume Fuchs (Fraunhofer IIS)

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

Copyright (c) 1997.



Source file: mp4enc.c


Required modules:
common.o                common module
cmdline.o               command line module
bitstream.o             bit stream module
audio.o                 audio i/o module
enc_par.o               encoder core (parametric)
enc_lpc.o               encoder core (CELP)
enc_tf.o                encoder core (T/F)

Authors:
HP    Heiko Purnhagen, Uni Hannover <purnhage@tnt.uni-hannover.de>
BG    Bernhard Grill, Uni Erlangen <grl@titan.lte.e-technik.uni-erlangen.de>
NI    Naoki Iwakami, NTT <iwakami@splab.hil.ntt.jp>
SE    Sebastien ETienne, Jean Bernard Rault CCETT <jbrault@ccett.fr>
OK    Olaf Kaehler, Fhg/IIS <kaehleof@iis.fhg.de>

Changes:
13-jun-96   HP    first version
14-jun-96   HP    added debug code
17-jun-96   HP    added bit reservoir control / switches -r -b
18-jun-96   HP    added delay compensation
19-jun-96   HP    using new ComposeFileName()
25-jun-96   HP    changed frameNumBit
26-jun-96   HP    improved handling of switch -o
04-jul-96   HP    joined with t/f code by BG (check "DISABLE_TF")
14-aug-96   HP    adapted to new cmdline module, added mp4.h
16-aug-96   HP    adapted to new enc.h
                  added multichannel signal handling
21-aug-96   HP    added AAC_RM4 switches by BG
26-aug-96   HP    CVS
28-aug-96   NI    moved "static CmdLineSwitch switchList[]" into main routine.
29-aug-96   HP    moved "switchlist[]" back again
                  NI stated, that it just was a locally required bug-fix:
                  This table is moved into the main routine,
                  because there was a run-time problem on a
                  SGI (IRIX-5.3) system (using g++).
30-oct-96   HP    additional frame work options
15-nov-96   HP    adapted to new bitstream module
18-nov-96   HP    changed int to long where required
                  added bit stream header options
                  removed AAC_RM4 switches (new -nh, -ms, -bi options)
10-dec-96   HP    new -ni, -vr options, added variable bit rate
23-jan-97   HP    added audio i/o module
07-apr-97   HP    i/o filename handling improved / "-" supported
08-apr-97   SE    added G729-based coder
15-may-97   HP    clean up
30-jun-97   HP    fixed totNumSample bug / calc encNumFrame
05-nov-97   HP    update by FhG/UER
12-nov-97   HP    added options -n & -s
02-dec-98   HP    merged most FDIS contributions ...
20-jan-99   HP    due to the nature of some of the modifications merged
                  into this code, I disclaim any responsibility for this
                  code and/or its readability -- sorry ...
21-jan-99   HP    trying to clean up a bit ...
22-jan-99   HP    stdin/stdout "-" support, -d 1 uses stderr
22-apr-99   HP    merging all contributions
19-jun-01   OK    removed G723/G729 dummy calls
22-jul-03   AR/RJ add extension 2 parametric coding
19-oct-03   OK    clean up for rewrite
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "allHandles.h"
#include "encoder.h"

#include "common_m4a.h" /* common module */
#include "bitstream.h"  /* bit stream module */
#include "audio.h"      /* audio i/o module */
#include "mp4au.h"      /* frame work declarations */
#include "enc_tf.h"
#include "enc_usac.h"
#include "fir_filt.h"   /* fir lowpass filter */
#include "plotmtv.h"

#include "flex_mux.h"

#include "streamfile.h"
#include "streamfile_helper.h"
#include "sac_dec_interface.h"
#include "sac_enc.h"
#include "sac_stream_enc.h"
#include "defines.h"

#include "signal_classifier.h"

/* ---------- declarations ---------- */

#define PROGVER "MPEG-4 Natural Audio Encoder (Edition 2001)   2003-08-27"
#define CVSID "$Id: mp4auenc.c,v 1.16 2011-09-30 09:13:13 mul Exp $"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* ---------- variables ---------- */
#ifdef DEBUGPLOT
extern int framePlot; /* global for debugging purposes */
extern int firstFrame;
#endif

int	    debug[256];
spatialDec* hSpatialDec = NULL;

typedef enum _VERBOSE_LEVEL
{
  VERBOSE_NONE,
  VERBOSE_LVL1,
  VERBOSE_LVL2
} VERBOSE_LEVEL;

static struct EncoderMode encoderCodecsList[] = {
  {MODE_TF,
   MODENAME_TF,
   EncTfInfo,
   EncTfInit,
   EncTfFrame,
   EncTfFree
  },
  {MODE_USAC,
   MODENAME_USAC,
   EncUsacInfo,
   EncUsacInit,
   EncUsacFrame,
   EncUsacFree
  },
  {-1,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
  }
};



int PARusedNumBit;

/* ---------- functions ---------- */

/* ###################################################################### */
/* ##                 MPEG USAC encoder static functions               ## */
/* ###################################################################### */
static int PrintCmdlineHelp(
            int                               argc,
            char                           ** argv
) {
  int bPrintCmdlHelp = 0;
  int i;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                               Core Encoder Module                           *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-h")) {
      bPrintCmdlHelp = 1;
      break;
    }
    if (!strcmp(argv[i], "-xh")) {
      bPrintCmdlHelp = 2;
      break;
    }
  }

  if (argc < 7 || bPrintCmdlHelp >= 1) {
    fprintf(stdout, "\n" );
    fprintf(stdout, "Usage: %s -if infile.wav -of outfile.mp4 -br <i> [options]\n\n", argv[0]);
    fprintf(stdout, "    -h                         Print help\n");
    fprintf(stdout, "    -xh                        Print extended help\n");
    fprintf(stdout, "    -if <s>                    Path to input wav\n");
    fprintf(stdout, "    -of <s>                    Path to output mp4\n");
    fprintf(stdout, "    -br <i>                    Bitrate in kbit/s\n");
    fprintf(stdout, "\n[options]:\n\n");
    fprintf(stdout, "    -numChannelOut <i>                 Set number of audio channels (0 = as input file) (default: 0)\n");
    fprintf(stdout, "    -fSampleOut <i>                    Set sampling frequency [Hz] for bit stream header (0 = as input file)\n");
    fprintf(stdout, "                                       (default: 0)\n");
    fprintf(stdout, "    -se <i>                            Write 'mha1' (1) or 'mha1' + 'btrt' (2) or other transport types in mp4 file\n");
    fprintf(stdout, "                                       (default: 1)\n");
    fprintf(stdout, "    -usac_switched                     USAC Switched Coding (default)\n");
    fprintf(stdout, "    -usac_fd                           USAC Frequency Domain Coding\n");
    fprintf(stdout, "    -usac_td                           USAC Temporal Domain Coding\n");
    fprintf(stdout, "    -noSbr                             Disable the usage of the SBR tool\n");
    fprintf(stdout, "    -sbrRatioIndex <i>                 Set SBR Ratio Index: 0, 1, 2, 3 (default)\n");
    fprintf(stdout, "    -mps_res                           Allow MPEG Surround residual (requires enabled SBR)\n");
    fprintf(stdout, "    -cplx_pred                         Complex prediction\n");
    fprintf(stdout, "    -mps_hybridResidualMode <i>        Enable hybrid residual coding\n");
    fprintf(stdout, "    -mps_qce                           Use quad channel element\n");
    fprintf(stdout, "    -noiseFillingMode <i>              Choose noise filling mode\n");
    fprintf(stdout, "    -enf                               Use enhanced noisefilling\n");
    fprintf(stdout, "    -splitTransform                    Use transform splitting\n");
#ifdef SPECIFY_INPUT_CONTENTTYPE
    fprintf(stdout, "    -contentType <i>                   Type of input content (default: 0), possible options:\n");
    fprintf(stdout, "                                       [0 = channels | 1 = objects | 2 = SAOC objects | 3 = HOA]\n");
#endif
    fprintf(stdout, "    -objFile <s>                       Filename of file with object data\n");
    fprintf(stdout, "    -saocFile <s>                      Filename of file with saoc data\n");
    fprintf(stdout, "    -saocBitrateFile <s>               Filename of file with saoc bitrate information\n");
    fprintf(stdout, "    -hoaFile <s>                       Filename of file with hoa data\n");
    fprintf(stdout, "    -hoaBitrateFile <s>                Filename of file with hoa bitrate information\n");
#if IAR
    fprintf(stdout, "    -iarFile <s>                       Filename of file with rendering3DType data for immersive audio rendering\n");
#endif
    fprintf(stdout, "    -pmcFile <s>                       Filename of file with production metadata configuration data\n");
    fprintf(stdout, "    -mctMode <i>                       Set Multichannel Coding Tool mode (default: -1), possible options:\n");                                   
    fprintf(stdout, "                                       [-1 = disable | 0 = prediction | 1 = rotation | 2 = pred+SF | 3 = rot+SF]\n");
    fprintf(stdout, "    -useLcProfile                      Enforce Low Complexity Profile compliant bitstream.\n");
    fprintf(stdout, "                                       [level 3 for <= 16 channels, 48kHz; level 5 above]\n");
    fprintf(stdout, "    -useBlProfile                      Enforce Baseline Profile compliant bitstream.\n");
    fprintf(stdout, "                                       [level 3 for <= 16 channels, 48kHz; level 5 above]\n");
    fprintf(stdout, "    -compatibleProfileLevel            Signal the Compatible Baseline Profile Level. Requires -useLcProfile to be set.\n");
    fprintf(stdout, "                                       compatibleProfileLevel in range [%d;%d]\n", SUPPORTED_COMPATIBLEPROFILELEVEL_MIN, SUPPORTED_COMPATIBLEPROFILELEVEL_MAX);
    fprintf(stdout, "    -cicpIn                            Specify the reference layout\n");
    fprintf(stdout, "    -hasDynamicObjectPriority <i>      hasDynamicObjectPriority (default: 0)\n");
    fprintf(stdout, "    -lowDelayMetadataCoding <i>        lowDelayMetadataCoding\n");
    fprintf(stdout, "                                       (default: 0, low delay: 1, low delay high resolution: 2)\n");
    fprintf(stdout, "    -bw <i>                            Bandwidth in Hz\n");
    fprintf(stdout, "    -hSBR                              Activate the usage of the harmonic patching for the SBR\n");
    fprintf(stdout, "    -ms <0,1,2>                        Set msMask instead of autodetect: 0, 1(default), 2\n");
    fprintf(stdout, "    -wshape                            Use window shape WS_DOLBY instead of WS_FHG (default)\n");
    fprintf(stdout, "    -deblvl <0,1>                      Activate debug level: 0 (default), 1\n");
    fprintf(stdout, "    -v                                 Activate verbose level 1\n");
    fprintf(stdout, "    -vv                                Activate verbose level 2\n");
    if( bPrintCmdlHelp == 2 ){
      fprintf(stdout, "\n[experimental options]:\n\n");
      fprintf(stdout, "    -noIPF                     Disable the embedding of IPF data\n");
      fprintf(stdout, "    -ep                        Create bitstreams with epConfig 1 syntax\n");
      fprintf(stdout, "    -tns                       Activate Temporal Noise Shaping (active by default)\n");
      fprintf(stdout, "    -tns_lc                    Activate Temporal Noise Shaping with low complexity (deactivates TNS -> not working)\n");
      fprintf(stdout, "    -usac_tw                   Activate the usage of the time-warped MDCT\n");
      fprintf(stdout, "    -noiseFilling              Activate Noise Filling\n");
      fprintf(stdout, "    -ipd                       Apply IPD coding in Mps212\n");
      fprintf(stdout, "    -aac_nosfacs               All AAC-scalefactors will be equal\n");
#ifdef USE_FILL_ELEMENT
      fprintf(stdout, "    -fillElem                  Write a FillElement or not.\n");
      fprintf(stdout, "                               Note: if no FillElement is written the buffer requirements may be violated (dflt: 1)\n");
#endif
      fprintf(stdout, "    -tsd <s>                   Activate applause coding\n");
    } else {
      fprintf(stdout, "\nUse -xh for extended help!\n");
    }
    fprintf(stdout, "\n[formats]:\n\n");
    fprintf(stdout, "    <s>                        string\n");
    fprintf(stdout, "    <i>                        integer number\n");
    fprintf(stdout, "    <f>                        floating point number\n");
    fprintf(stdout, "\n");
    return -1;
  }
  return 0;
}

static int ParseEncoderOptions(
            int                               argc,
            char                           ** argv,
            HANDLE_ENCPARA                    encPara,
            int                             * mainDebugLevel
) {
  int error_usac              = 0;
  int i                       = 0;
  int required                = 0;
  int codecModeSet            = 0;
  int tnsLcSet                = 0;

  /* ========================== */
  /* Initialize Encoder Options */
  /* ========================== */

  encPara->debugLevel               = *mainDebugLevel;
  encPara->bw_limit                 = 0;
  encPara->codecMode                = USAC_ONLY_FD;
  encPara->tns_select               = NO_TNS;
#ifdef SPECIFY_INPUT_CONTENTTYPE                                           
  encPara->contentType              = CONTENT_3D_CHANNELS;
#endif
  encPara->ms_select                = 1;
  encPara->ep_config                = -1;
  encPara->wshape_flag              = 0;
#ifdef USE_FILL_ELEMENT
  encPara->bUseFillElement          = 1;
#endif
  encPara->sbrenable                = 1;
  encPara->flag_960                 = 0;
  encPara->flag_768                 = 0;
  encPara->flag_twMdct              = 0;
  encPara->flag_harmonicBE          = 0;
  encPara->flag_noiseFilling        = 0;
  encPara->flag_ipd                 = 0;
  encPara->flag_mpsres              = 0;
  encPara->hybridResidualMode       = 0;
  encPara->flag_mpsqce              = 0;
  encPara->noiseFillingMode         = 0;
  encPara->g_useSplitTransform      = 0;
  encPara->enhancedNoiseFilling     = 0;
  encPara->flag_cplxPred            = 0;
  encPara->mctMode                  = -1;
  encPara->useLcProfile             = 0;
  encPara->useBlProfile             = 0;
  encPara->compatibleProfileLevel   = 0;
  encPara->hasDynamicObjectPriority = 0;
  encPara->lowDelayMetadataCoding   = 0;
  encPara->OAMFrameLength           = -1;
  encPara->cicpIn                   = 13;
  memset(encPara->tsdInputFileName, 0, sizeof(encPara->tsdInputFileName));
  memset(encPara->objFile,          0, sizeof(encPara->objFile));      
  memset(encPara->saocFile,         0, sizeof(encPara->saocFile)); 
  memset(encPara->saocBitrateFile,  0, sizeof(encPara->saocBitrateFile)); 
  memset(encPara->hoaFile,          0, sizeof(encPara->hoaFile));
  memset(encPara->hoaBitrateFile,   0, sizeof(encPara->hoaBitrateFile));
  memset(encPara->iarFile,          0, sizeof(encPara->iarFile));
  memset(encPara->pmcFile,          0, sizeof(encPara->pmcFile));

  /* get selected encoder mode */
  for ( i = 0 ; i < 2 ; i++ ) {
    encPara->prev_coreMode[i] = CORE_MODE_FD;
    encPara->coreMode[i]      = CORE_MODE_FD;
  }

  /* ===================== */
  /* Parse Encoder Options */
  /* ===================== */
  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-br")) {                        /* Required */
      encPara->bitrate = atoi(argv[++i]);
      required++;
      continue;
    }
    else if (!strcmp(argv[i], "-bw")) {                   /* Optional */
      encPara->bw_limit = atoi(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-usac_switched")) {        /* Optional */
      if (codecModeSet == 1) {
        CommonWarning("main: codecMode has already been set. Choose only one of the following options: -usac_switched (default), -usac_fd, -usac_td.");
        error_usac = 1;
        break;
      } else {
        encPara->codecMode = USAC_SWITCHED;
        codecModeSet = 1;
        continue;
      }
    }
    else if (!strcmp(argv[i], "-usac_fd")) {              /* Optional */
      if (codecModeSet == 1) {
        CommonWarning("main: codecMode has already been set. Choose only one of the following options: -usac_switched (default), -usac_fd, -usac_td.");
        error_usac = 1;
        break;
      } else {
        encPara->codecMode = USAC_ONLY_FD;
        codecModeSet = 1;
        continue;
      }
    }
    else if (!strcmp(argv[i], "-usac_td")) {              /* Optional */
      if (codecModeSet == 1) {
        CommonWarning("main: codecMode has already been set. Choose only one of the following options: -usac_switched (default), -usac_fd, -usac_td.");
        error_usac = 1;
        break;
      } else {
        encPara->codecMode = USAC_ONLY_TD;
        codecModeSet = 1;
        continue;
      }
    }
    else if (!strcmp(argv[i], "-tns")) {                  /* Optional */
      encPara->tns_select = TNS_USAC;
      continue;
    }
    else if (!strcmp(argv[i], "-ms")) {                   /* Optional */
      encPara->ms_select = atoi(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-ep")) {                   /* Optional */
      encPara->ep_config = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-wshape")) {               /* Optional */
      encPara->wshape_flag = 1;
      continue;
    }
#ifdef USE_FILL_ELEMENT
    else if (!strcmp(argv[i], "-fillElem")) {             
      encPara->bUseFillElement = atoi(argv[++i]);
      continue;
    }
#endif
    else if (!strcmp(argv[i], "-noSbr")) {                /* Optional */
      encPara->sbrenable = 0;
      continue;
    }
    else if (!strcmp(argv[i], "-usac_tw")) {              /* Optional */
      encPara->flag_twMdct = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-hSBR")) {                 /* Optional */
      encPara->flag_harmonicBE = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-noiseFilling")) {         /* Optional */
      encPara->flag_noiseFilling = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-ipd")) {                  /* Optional */
      encPara->flag_ipd = 1;
      continue;
    }                                                        
    else if (!strcmp(argv[i], "-cplx_pred")) {            /* Optional */
      encPara->flag_cplxPred = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-mps_qce")) {              /* Optional */
      encPara->flag_mpsqce = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-noiseFillingMode")) {     /* Optional */
      encPara->noiseFillingMode = atoi(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-splitTransform")) {       /* Optional */
      encPara->g_useSplitTransform = 1;
      encPara->flag_noiseFilling = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-enf")) {                  /* Optional */
      encPara->enhancedNoiseFilling = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-tsd")) {                                 /* Optional */
      strncpy ( encPara->tsdInputFileName, argv[++i], FILENAME_MAX );
      continue;
    }
    else if (!strcmp(argv[i], "-objFile")) {                             /* Optional */
      strncpy ( encPara->objFile, argv[++i], FILENAME_MAX );
      continue;
    }
    else if (!strcmp(argv[i], "-saocFile")) {                            /* Optional */
      strncpy ( encPara->saocFile, argv[++i], FILENAME_MAX );
      continue;
    }
    else if (!strcmp(argv[i], "-saocBitrateFile")) {                     /* Optional */
      strncpy ( encPara->saocBitrateFile, argv[++i], FILENAME_MAX );
      continue;
    }
#ifdef SPECIFY_INPUT_CONTENTTYPE
    else if (!strcmp(argv[i], "-contentType")) {                         /* Optional */
      /* encoder content type */
      encPara->contentType = (INPUT_CONTENT_TYPE_3D) atoi(argv[++i]);   
      continue;
    }
#endif
    else if (!strcmp(argv[i], "-hoaFile")) {                             /* Optional */
      strncpy ( encPara->hoaFile, argv[++i], FILENAME_MAX );
      continue;
    }
    else if (!strcmp(argv[i], "-hoaBitrateFile")) {                      /* Optional */
      strncpy ( encPara->hoaBitrateFile, argv[++i], FILENAME_MAX );
      continue;
    }
#if IAR
    else if (!strcmp(argv[i], "-iarFile")) {                             /* Optional */
      strncpy ( encPara->iarFile, argv[++i], FILENAME_MAX );
      continue;
    }
#endif
    else if (!strcmp(argv[i], "-pmcFile")) {                             /* Optional */
      strncpy ( encPara->pmcFile, argv[++i], FILENAME_MAX );
      continue;
    }
    else if (!strcmp(argv[i], "-mctMode")) {                             /* Optional */
      /* encoder content type */
      encPara->mctMode = atoi(argv[++i]);
      if (encPara->mctMode > 3) {
        CommonWarning("EncUsacInit: invalid value for -mctMode found");
        return -1;
      } else {
        continue;
      }
    }
    else if (!strcmp(argv[i], "-useLcProfile")) {                        /* Optional */
      encPara->useLcProfile = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-useBlProfile")) {                        /* Optional */
      encPara->useBlProfile = 1;
      continue;
    }
    else if (!strcmp(argv[i], "-compatibleProfileLevel")) {              /* Optional */
      const int compatibleProfileLevel = atoi(argv[++i]);
      if (compatibleProfileLevel < SUPPORTED_COMPATIBLEPROFILELEVEL_MIN || compatibleProfileLevel > SUPPORTED_COMPATIBLEPROFILELEVEL_MAX) {
        CommonWarning("EncUsacInit: invalid value for -compatibleProfileLevel found");
        return -1;
      } else {
        encPara->compatibleProfileLevel = (unsigned int)compatibleProfileLevel;
        continue;
      }
    }
    else if (!strcmp(argv[i], "-hasDynamicObjectPriority")) {            /* Optional */
      encPara->hasDynamicObjectPriority = atoi(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-lowDelayMetadataCoding")) {              /* Optional */
      encPara->lowDelayMetadataCoding = atoi(argv[++i]);
      if ((encPara->lowDelayMetadataCoding < 0) || (encPara->lowDelayMetadataCoding > 2)) {
        CommonWarning("EncUsacInit: illegal value of parameter lowDelayMetadataCoding, must be [0|1]>");
        return -1;
      }
      if ( encPara->lowDelayMetadataCoding == 2 ) {
        encPara->lowDelayMetadataCoding = 1;
        encPara->OAMFrameLength = 256;
      }
      else {
        encPara->OAMFrameLength = -1;
      }
      continue;
    }
    else if (!strcmp(argv[i], "-cicpIn")) {               /* Optional */
      encPara->cicpIn = atoi(argv[++i]);
      continue;
    }
  }

  /* get sbrRatioIndex */
  encPara->sbrRatioIndex = (encPara->sbrenable) ? 3 : 0; /* set to 2:1 SBR by default if sbr is enabled*/
  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-sbrRatioIndex")) {             /* Optional */
      encPara->sbrRatioIndex = atoi(argv[++i]);
      if(encPara->sbrRatioIndex == 0){
        encPara->sbrenable = 0;
      }
      if(encPara->sbrRatioIndex == 2){
        encPara->flag_768 = 1;
      }
      break;
    }
  }

  if (encPara->sbrenable == 1) {
    for (i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-mps_res")) {                 /* Optional */
        encPara->flag_mpsres = encPara->sbrenable;
        break;
      }
    }
  }

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-mps_hybridResidualMode")) {    /* Optional */
      encPara->hybridResidualMode = atoi(argv[++i]);
      if (encPara->hybridResidualMode > 0) {
        encPara->flag_mpsres = encPara->sbrenable;
      }
      break;
    }
  }

  /* read AAC common command line */
  if ((encPara->codecMode == USAC_SWITCHED) || (encPara->codecMode == USAC_ONLY_FD))
  {
    encPara->aacAllowScalefacs = 1;
    for (i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-aac_nosfacs")) {             /* Optional */
        encPara->aacAllowScalefacs = 0;
        break;
      }
    }
  }

  /* read more specific AAC command line */
  if (encPara->ep_config == -1) {
    if (encPara->tns_select != NO_TNS){
      encPara->tns_select = TNS_USAC;
    }
    for (i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-tns_lc")) {
        tnsLcSet = 1;
      }
    }
    if (tnsLcSet != 1) {
      encPara->tns_select = NO_TNS; /* TNS_USAC */ /* TNS not yet implemented for 3DA*/
    }
  }

  /* validate command line arguments */
  if (required != 1) {
    error_usac = 1;
  }

  if (0 != encPara->useLcProfile && 0 != encPara->useBlProfile) {
    CommonWarning("EncUsacInit:-useLcProfile and -useBlProfile must not be enabled simultaneously");
    return -1;
  }
  
  return error_usac;
}

static int GetCmdline(
            int                               argc,
            char                           ** argv,
            char                            * wav_InputFile,
            char                            * mp4_OutputFile,
            int                             * numChannelOut,
            float                           * fSampleOut,
            int                             * sampleEntryFormat,
            HANDLE_ENCPARA                    encPara,
            int                             * mainDebugLevel,
            VERBOSE_LEVEL                   * verboseLevel
) {
  int error_usac = 0;
  int i                       = 0;
  int required                = 0;

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-if")) {                                /* Required */
      strncpy ( wav_InputFile, argv[++i], FILENAME_MAX );
      required++;
      continue;
    }
    else if (!strcmp(argv[i], "-of")) {                           /* Required */
      strncpy ( mp4_OutputFile, argv[++i], FILENAME_MAX );
      required++;
      continue;
    }
    else if (!strcmp(argv[i], "-numChannelOut")) {                /* Optional */
      *numChannelOut = atoi(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-fSampleOut")) {                   /* Optional */
      *fSampleOut = (float) atof(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-se")) {                           /* Optional */
      *sampleEntryFormat = (int) atof(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i], "-v")) {                            /* Optional */
      *verboseLevel = (*verboseLevel > VERBOSE_LVL1) ? *verboseLevel : VERBOSE_LVL1;
      continue;
    }
    else if (!strcmp(argv[i], "-vv")) {                           /* Optional */
      *verboseLevel = VERBOSE_LVL2;
      continue;
    }
    else if (!strcmp(argv[i], "-deblvl")) {                       /* Optional */
      *mainDebugLevel = atoi(argv[++i]);
      continue;
    }
  }

  /* validate command line arguments */
  if (required != 2) {
    error_usac = 1;
  }
  
  error_usac = ParseEncoderOptions( argc, argv, encPara, mainDebugLevel );

  return error_usac;
}

static HANDLE_ENCODER newEncoder()
{
  HANDLE_ENCODER ret = (HANDLE_ENCODER)malloc(sizeof(struct tagEncoderData));
  ret->data = NULL;
  ret->getNumChannels = NULL;
  ret->getReconstructedTimeSignal = NULL;
  ret->getEncoderMode = NULL;
  ret->getSbrEnable = NULL;
  ret->getSbrRatioIndex = NULL;
  ret->getBitrate = NULL;
  return ret;
}

/* 20060107 */
#ifdef USE_AFSP
#include <math.h>
#include "libtsp.h"
static double
FIxKaiser (double x, double alpha)

{
  static double alpha_prev = 0.0;
  static double Ia = 1.0;
  double beta, win;

  /* Avoid recalculating the denominator Bessel function */
  if (alpha != alpha_prev) {
    Ia = FNbessI0 (alpha);
    alpha_prev = alpha;
  }

  if (x < -1.0 || x > 1.0)
    win = 0.0;
  else {
    beta = alpha * sqrt (1.0 - x * x);
    win = FNbessI0 (beta) / Ia;
  }

  return win;
}

static void
RSKaiserLPF (float h[], int N, double Fc, double alpha)
{
  int i, k;
  double t, u, T, Ga;

  /* Multiply the Kaiser window by the sin(x)/x response */
  /* Use symmetry to reduce computations */
  Ga = 2.0 * Fc;
  T = 0.5 * (N-1);
  for (i = 0, k = N-1; i <= k; ++i, --k) {
    t = i - T;
    u = k - T;
    h[i] = (float) (Ga * FNsinc (2.0 * Fc * t) * FIxKaiser (t / T, alpha));
    if (-t == u)
      h[k] = h[i];
    else
      h[k] = (float) (Ga * FNsinc (2.0 * Fc * u) * FIxKaiser (u / T, alpha));
  }
}


static void
FIdownsample(const float* input, float* out, int NOut, const float*win, int Ncof, int downSampleRatio)
{
  int i, j, k, l;
  double sum;

  k = Ncof-1;
  for (i=0; i<NOut; i++) {
    sum = 0.0;
    l = k;
    for (j=0; j<Ncof; j++) {
      sum += win[j]*input[l--];
    }
    out[i] = sum;
    k+=downSampleRatio;
  }
}

static void
FIupsample(const float* input, float* out, int NOut, const float*win, int Ncof, int upSampleRatio)
{
  int i, j, k, l;
  double sum;

  for (i=0; i<NOut+Ncof; i++){
    out[i] = 0.f;
  }

  for (i=0; i<(NOut+Ncof)/upSampleRatio; i++){
    out[i*upSampleRatio] = input[i];
  }

  k = Ncof-1;
  for (i=0; i<NOut; i++) {
    sum = 0.0;
    l = k;
    for (j=0; j<Ncof; j++) {
      sum += win[j]*out[l--];
    }
    out[i] = sum;
    k++;
  }
}
#endif

/* modulo buffer for delay compensation */
typedef struct {
  int bufferSize;
  int readIndex;
  int writeIndex;
  int samplesInBuffer;
  float *buffer;
} DELAY_BUFFER;

static void usac212_delayBufferInit(DELAY_BUFFER *delayBuffer, int bufSize, int delay) {
  if (delayBuffer == NULL) CommonExit(1,"Encode: memory allocation error");

  delayBuffer->bufferSize = bufSize+delay;
  delayBuffer->writeIndex = delay;
  delayBuffer->readIndex = 0;
  delayBuffer->samplesInBuffer = delay;

  delayBuffer->buffer = (float*)calloc(delayBuffer->bufferSize, sizeof(float));
  if (delayBuffer->buffer == NULL) CommonExit(1,"Encode: memory allocation error");

  return;
}
  
static void usac212_delayBufferWriteSamples(DELAY_BUFFER *delayBuffer, float *samples, int numSamples) {
  if (delayBuffer == NULL) CommonExit(1,"Encode: delay buffer error");
  if (delayBuffer->samplesInBuffer+numSamples <= delayBuffer->bufferSize) {
    int i;
    for (i = 0; i < numSamples; i++){
      delayBuffer->buffer[delayBuffer->writeIndex] = samples[i];
      delayBuffer->writeIndex++;
      delayBuffer->writeIndex %= delayBuffer->bufferSize;
      delayBuffer->samplesInBuffer++;
    }
  } 
  else {
    CommonExit(1,"Encode: tried to write to full delay buffer!");
  }
}

static void usac212_delayBufferReadSamples(DELAY_BUFFER *delayBuffer, float* samples, int numSamples) {
  if (delayBuffer == NULL) CommonExit(1,"Encode: delay buffer error");
  if (delayBuffer->samplesInBuffer >= numSamples) {
    int i;
    for (i = 0; i < numSamples; i++){
      samples[i] = delayBuffer->buffer[delayBuffer->readIndex];
      delayBuffer->readIndex++;
      delayBuffer->readIndex %= delayBuffer->bufferSize;
      delayBuffer->samplesInBuffer--;
    }
  } 
  else {
    CommonExit(1,"Encode: tried to read from empty delay buffer!");
  }
}

static void usac212_delayBufferClose(DELAY_BUFFER *delayBuffer) {
  if (delayBuffer == NULL) return;

  delayBuffer->bufferSize = 0;
  delayBuffer->writeIndex = 0;
  delayBuffer->readIndex = 0;
  delayBuffer->samplesInBuffer = 0;

  free(delayBuffer->buffer);
  delayBuffer->buffer = NULL;

  return;
}

/* 20060107 */
/* Encode() */
/* Encode audio file and generate bit stream. */
/* (This function evaluates the global xxxDebugLevel variables !!!) */

static int Encode (
                   char *audioFileName,          /* in: audio file name */
                   char *bitFileName,            /* in: bit stream file name */
                   char *codecMode,              /* in: codec mode string(s) */
                   HANDLE_ENCPARA encPara,       /* in: encoder parameter struct */
                   float regionStart,            /* in: start time of region */
                   float regionDurat,            /* in: duration of region */
                   int numChannelOut,            /* in: number of channels (0 = as input) */
                   float fSampleOut,             /* in: sampling frequency (0 = as input) */
                   int mainDebugLevel,
                   int verboseLevel,
                   int sampleEntryFormat         /* in: 'mha1' or 'mha1' + 'btrb', or other transport formats */
                   )
/* returns: 0=OK  1=error */
{
  int err = 0;
  AudioFile *audioFile;
  float **sampleBuf;
  float **reSampleBuf;
  float **encSampleBuf;
  float fSample=0;
  long fileNumSample;
  long totNumSample;
  int numSample;
  int numChannel;
  int frameNumSample;
  long fSampleLong;
  HANDLE_STREAMFILE outfile;
  HANDLE_STREAMPROG outprog;
  HANDLE_BSBITBUFFER *au_buffers;
  HANDLE_BSBITBUFFER *asc_buffers;
  int nextIsIPF;
  int currentIsIPF;
  int startupNumFrame,startupFrame;
  int ch, i;
  long startSample;
  long encNumSample;
  int numChannelBS = 0;
  long fSampleLongBS;
  ENC_FRAME_DATA* frameData;
  int track_count_total;
  int delayNumSample = 0;

  FIR_FILT *lowpassFilt = NULL;

  /* 20060107 */
  int sbrenable = 0;
  int sbrRatioIndex = 0;
  int usac212enable = 0;
  int downSampleRatio = 1;
  int upSampleRatio   = 1;
  int bitRate;
  int NcoefRS = 0;
  int NcoefUS = 0;
  int delayTotal = 0;
  int delay212 = 0;
  int addlCoreDelay = 0;
  int rightTruncation = 0;
  int nSamplesProcessed = 0;
  int delayRSfilt = 0;
  int delayEncoder = 0;
  float *hRS = NULL;
  float *hUS = NULL;
  float **sampleBufRS = NULL;
  float **sampleBufRSold = NULL;
  float **tmpBufRS = NULL;
  float **sampleBufUS = NULL;
  float **sampleBufUSold = NULL;
  float **tmpBufUS = NULL;
  float lowPassFreq = 0;
  float* inputBuffer = NULL;
  float* downmixBuffer = NULL;

  float **sampleBufRSfiltDelay = NULL;

  int   bQcePresent = 0;
  int   maxChannelDelay = 0;

  DELAY_BUFFER *delayBuffer = NULL;

  int     totalFrames              = 0;
  int     processedSamplesPerFrame = 0;
  int     numDelayFrames           = 0;
  int     resampledInput           = 0;
  float   resamplingRatio          = 0.0;

  /* 20060107 */
  int frameMaxNumBit; /* SAMSUNG_2005-09-30 : maximum number of bits per frame */

  const int         bitBufSize                    = 0;
  const int         bitDebugLevel                 = 0;
  const int         audioDebugLevel               = 0;
  
  int               frame                         = 0;
  int               osf                           = 1;



  frameData = (ENC_FRAME_DATA*)calloc(1, sizeof(ENC_FRAME_DATA));
  if (NULL == frameData) {
    CommonExit(1, "Encode: memory allocation failed!");
  }

  /* ---- init ---- */
  if (verboseLevel >= VERBOSE_LVL1) {
    fprintf(stdout,"Encode:\n");
    fprintf(stdout,"audioFileName=\"%s\"\n",audioFileName);
    fprintf(stdout,"bitFileName=\"%s\"\n",bitFileName);
  }
  if (verboseLevel == VERBOSE_LVL2) {
  fprintf(stdout,"codecMode=\"%s\"\n",codecMode);
  }

  SetDebugLevel(mainDebugLevel);
  BsInit(bitBufSize,bitDebugLevel,0);
  AudioInit(getenv(MP4_ORI_RAW_ENV),       /* headerless file format */
            audioDebugLevel);

  /* ---- open audio file ---- */
  audioFile = AudioOpenRead(audioFileName,&numChannel,&fSample,
                            &fileNumSample);
  if (audioFile==NULL)
    CommonExit(1,"Encode: error opening audio file %s "
               "(maybe unknown format)",
               audioFileName);


  startSample = (long)(regionStart*fSample+0.5);

  if (regionDurat >= 0)
    encNumSample = (long)(regionDurat*fSample+0.5);
  else
    if (fileNumSample < 0) /* unkown */
      encNumSample = -1;
    else
      encNumSample = max(0,fileNumSample-startSample);


  
  /* ---- init encoder ---- */
  fSampleLong = (long)(fSample+0.5);
    numChannelBS = (numChannelOut==0) ? numChannel : numChannelOut;
  if (numChannelBS > numChannel) {
    CommonWarning("Desired number of channels too large, using %i instead of %i\n",numChannel,numChannelBS);
    numChannelBS = numChannel;
  }
  fSampleLongBS = (fSampleOut==0) ? fSampleLong : (long)(fSampleOut+0.5);

  /* ---- get selected codec ---- */
  track_count_total = 0;
  frameData->trackCount = (int*)malloc(sizeof(int));
  frameData->enc_mode = (struct EncoderMode*)malloc(sizeof(EncoderMode));
  frameData->enc_data = (HANDLE_ENCODER*)malloc(sizeof(HANDLE_ENCODER));

  for (i=0; i<MAX_TRACKS; i++) {
    setupDecConfigDescr(&frameData->dec_conf[i]);
    frameData->dec_conf[i].streamType.value = 0x05;
    frameData->dec_conf[i].upsteam.value = 0;

    /* MPEG-H Specific */
    frameData->dec_conf[i].transportMask                   = sampleEntryFormat;
    frameData->dec_conf[i].MPEGHAudioProfileLevel.value    = 0x0A;
    frameData->dec_conf[i].profileAndLevelIndication.value = frameData->dec_conf[i].MPEGHAudioProfileLevel.value;
  }

  {
    struct EncoderMode *mode;
    HANDLE_BSBITBUFFER asc_buffer = BsAllocBuffer(USAC_MAX_CONFIG_LEN << 3);
    i = 0;

    do {
      mode = &encoderCodecsList[i++];
    } while ((mode->mode != -1) && strcmp(codecMode,mode->modename));
    if (mode->mode == -1)
      CommonExit(1,"Encode: unknown codec mode %s",codecMode);

    *(frameData->enc_mode) = *mode;

    /*varBitRate*/
    *frameData->enc_data = newEncoder();
    err = frameData->enc_mode->EncInit(numChannelBS,
                                      fSample,
                                      encPara,
                                      &frameNumSample,
                                      &frameMaxNumBit,   /* SAMSUNG_2005-09-30 */
                                      &(frameData->dec_conf[track_count_total]),
                                      frameData->trackCount,
                                      &asc_buffer,
                                      NULL,
                                      *frameData->enc_data
                                      ); 
    if (0 != err) {
      CommonExit(1, "EncInit returned %i", err);
    }

    DebugPrintf(1,"initialized encoder: %i tracks",frameData->trackCount);

    for (i = 0; i < *frameData->trackCount; i++) {
      DebugPrintf(2,"                   track %i: AOT %i",i,frameData->dec_conf[track_count_total+i].audioSpecificConfig.audioDecoderType.value);
    }
    track_count_total += *frameData->trackCount;

    BsFreeBuffer(asc_buffer);
  }


  /* 20060107 */
  sbrenable = (*frameData->enc_data)->getSbrEnable(*frameData->enc_data, &bitRate);
  if((*frameData->enc_data)->getSbrRatioIndex){
    sbrRatioIndex = (*frameData->enc_data)->getSbrRatioIndex(*frameData->enc_data);
  }

  if(sbrenable && (!sbrRatioIndex) ){
    downSampleRatio = 2;
  } else {
    switch(sbrRatioIndex){
    case 0:
      downSampleRatio = 1;
      break;
    case 1:
      downSampleRatio = 4;
      break;
    case 2:
      downSampleRatio = 8;
      upSampleRatio   = 3;
      break;
    case 3:
      downSampleRatio = 2;
      break;
    default:
      assert(0);
      break;
    }
  }

  if (sbrenable || (sbrRatioIndex > 0)) {
    fSample = (upSampleRatio*fSample)/downSampleRatio;

    lowPassFreq = 5000.0f;
    if (numChannelBS==1) {
      if (bitRate==24000 && fSample==22050)
        lowPassFreq = 5513.0f; /* startFreq 4 @ 22.05kHz */
      if (bitRate==24000 && fSample==24000)
        lowPassFreq = 5625.0f; /* startFreq 4 @ 24kHz */
    }
    if (numChannelBS==2) {
      if (bitRate==48000 && fSample==22050)
        lowPassFreq = 7924.0f; /* startFreq 9 @ 22.05kHz */
      if (bitRate==48000 && fSample==24000)
        lowPassFreq = 8250.0f; /* startFreq 9 @ 24kHz */
    }
    lowPassFreq = fSample/2.0f;

    {
      /* init resampler and buffers for downsampling by 2 */
      float Fc = lowPassFreq / (fSample * downSampleRatio);
      float dF = 0.1f * Fc; /* 10% transition bw */
      float alpha = 7.865f; /* 80 dB attenuation */
      float D = 5.015f; /* transition width ? */
      NcoefRS = (int)ceil(D/dF) + 1;

      delayRSfilt = (int)ceil((float)NcoefRS/2);

      delayTotal = addlCoreDelay = (EncUsac_getUsacDelay(*frameData->enc_data) * downSampleRatio/upSampleRatio);
      usac212enable = EncUsac_getusac212enable(*frameData->enc_data);
      if(usac212enable){
        inputBuffer   = (float*)calloc(2 * MAX_BUFFER_SIZE, sizeof(float));
        downmixBuffer = (float*)calloc(2 * MAX_BUFFER_SIZE, sizeof(float));

        /* get mps212 delay, assuming first element is a CPE */
        delay212 = EncUsac_getSpatialOutputBufferDelay(*frameData->enc_data);
        delayTotal += delay212;
      }

      /* downsampling filter coefficients */
      if ((hRS = (float *)calloc(NcoefRS, sizeof(float)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
#ifdef USE_AFSP
      RSKaiserLPF (hRS, NcoefRS, (double)Fc, (double)alpha);
#else
      CommonExit(1,"AFSP not available.");
#endif

      Fc = 1.047198f; /* = pi/upsampleRatio = pi/3 */
      dF = 0.1f * Fc;
      NcoefUS = (int)ceil(D/dF) + 1;
      NcoefUS = ((NcoefUS+upSampleRatio-1)/upSampleRatio)*upSampleRatio;

      /* downsampling filter coefficients */
      if ((hUS = (float *)calloc(NcoefUS, sizeof(float)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
#ifdef USE_AFSP
      RSKaiserLPF (hUS, NcoefUS, (double)Fc, (double)alpha);
#else
      CommonExit(1,"AFSP not available.");
#endif

      /* input buffers */
      if ((sampleBufRS=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
      if ((sampleBufRSfiltDelay=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
      if ((sampleBufRSold=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
      if ((tmpBufRS=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");

      if ((sampleBufUS=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
      if ((sampleBufUSold=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");
      if ((tmpBufUS=(float**)malloc(numChannel*sizeof(float*)))==NULL)
        CommonExit(1,"Encode: memory allocation error");

      /* usac212 channel delay alignment */
      if (usac212enable) {
        if ((delayBuffer=(DELAY_BUFFER*)calloc(numChannel,sizeof(DELAY_BUFFER)))==NULL)
          CommonExit(1,"Encode: memory allocation error");
        
        /* check if QCE elements present */
        for (ch = 0; ch < numChannel; ch++) {
          int elemIdx = EncUsac_getChannelElementIndex(*frameData->enc_data, ch);
          if (EncUsac_getIsQceElement(*frameData->enc_data, elemIdx)) {
            bQcePresent = 1;
            break;
          }
        }
      }


      for (ch=0; ch<numChannel; ch++) {
        /* down sampling */
        if ((sampleBufRS[ch]=(float*)calloc(downSampleRatio*frameNumSample+NcoefUS,sizeof(float)))== NULL)
          CommonExit(1,"Encode: memory allocation error");
        if ((sampleBufRSfiltDelay[ch]=(float*)calloc(delayRSfilt+delayTotal,sizeof(float)))==NULL)
          CommonExit(1,"Encode: memory allocation error");
        if ((sampleBufRSold[ch]=(float*)calloc(downSampleRatio*frameNumSample,sizeof(float)))==NULL)
          CommonExit(1,"Encode: memory allocation error");
        if ((tmpBufRS[ch]=(float*)calloc(delayRSfilt+delayTotal+2*downSampleRatio*frameNumSample,sizeof(float)))==NULL)
          CommonExit(1,"Encode: memory allocation error");

        /* up sampling */
        if ((sampleBufUS[ch]=(float*)calloc(downSampleRatio*frameNumSample/upSampleRatio,sizeof(float)))== NULL)
          CommonExit(1,"Encode: memory allocation error");
        if ((sampleBufUSold[ch]=(float*)calloc(NcoefUS,sizeof(float)))==NULL)
          CommonExit(1,"Encode: memory allocation error");
        if ((tmpBufUS[ch]=(float*)calloc(NcoefUS+downSampleRatio*frameNumSample/upSampleRatio,sizeof(float)))==NULL)
          CommonExit(1,"Encode: memory allocation error");

        /* usac212 channel delay alignment */
        if (usac212enable) {
          int channelDelay = 0;
          int elemIdx = EncUsac_getChannelElementIndex(*frameData->enc_data, ch);
          int channelElementType = EncUsac_getChannelElementType(*frameData->enc_data, ch);

          if ((channelElementType == USAC_CHANNEL_ELEMENTTYPE_SCE ) || (channelElementType == USAC_CHANNEL_ELEMENTTYPE_LFE )) {
            channelDelay += delay212+384;
            if (bQcePresent || (EncUsac_getFlagMpsRes(*frameData->enc_data) == 1)) {
              /* channelDelay += 384;*/;
            }
          }
          else {          
            if (bQcePresent && (EncUsac_getIsQceElement(*frameData->enc_data, elemIdx) == 0)) {
              /* channelDelay += 384; */ ;
            }
          }

          usac212_delayBufferInit(&delayBuffer[ch], downSampleRatio*frameNumSample/upSampleRatio, channelDelay);

          if (channelDelay > maxChannelDelay) maxChannelDelay = channelDelay;
        }
      }
    }
  }
  /* 20060107 */

  if (verboseLevel == VERBOSE_LVL2) {
    fprintf(stdout,"fSample=%.3f Hz  (int=%ld)\n",fSample,fSampleLong);
    fprintf(stdout,"frameNumSample=%d  (%.6f sec/frame)\n",
           frameNumSample,frameNumSample/fSample);
    fprintf(stdout,"fileNumSample=%ld  (%.3f sec  %.3f frames)\n",
           fileNumSample,fileNumSample/fSample,
           fileNumSample/(float)frameNumSample);
    fprintf(stdout,"startSample=%ld\n",startSample);
    fprintf(stdout,"encNumSample=%ld  (%.3f sec  %.3f frames)\n",
           encNumSample,encNumSample/fSample,
           encNumSample/(float)frameNumSample);
  }

  /* allocate buffers */
  {
    au_buffers = (HANDLE_BSBITBUFFER*)malloc(track_count_total*sizeof(HANDLE_BSBITBUFFER));
    for (i = 0; i < track_count_total; i++) {
      au_buffers[i] = BsAllocBuffer(frameMaxNumBit);
    }

    asc_buffers = (HANDLE_BSBITBUFFER*)malloc(track_count_total*sizeof(HANDLE_BSBITBUFFER));
    for (i = 0; i < track_count_total; i++) {
      asc_buffers[i] = BsAllocBuffer(1024);
    }
  }

  if ((sampleBuf=(float**)malloc(numChannel*sizeof(float*)))==NULL)
    CommonExit(1,"Encode: memory allocation error");
  for (ch=0; ch<numChannel; ch++)
    if ((sampleBuf[ch]=(float*)malloc(MAX_OSF*frameNumSample*sizeof(float)))==NULL)
      CommonExit(1,"Encode: memory allocation error");

  if ((reSampleBuf=(float**)malloc(MAX_OSF*numChannel*sizeof(float*)))==NULL)
    CommonExit(1,"Encode: memory allocation error");
  for (ch=0; ch<numChannel; ch++)
    if ((reSampleBuf[ch]=(float*)malloc(frameNumSample*sizeof(float)))==NULL)
      CommonExit(1,"Encode: memory allocation error");

  /* ---- open bit stream file ---- */
  outfile = StreamFileOpenWrite(bitFileName, FILETYPE_AUTO, NULL);

  if (outfile==NULL)
    CommonExit(1,"Encode: error opening bit stream file %s",bitFileName);

  outprog = StreamFileAddProgram(outfile);
  outprog->trackCount = track_count_total;

  for (i=0; i<track_count_total; i++) {
    outprog->decoderConfig[i] = frameData->dec_conf[i];
    outprog->dependencies[i] = i-1;
    outprog->stssSignalsSyncSample[i] = STSS_SYNCSAMPLE_UNDEFINED;
    outprog->compatibleProfileLevelSet[i] = EncUsac_getCompatibleProfileLevelSet(*frameData->enc_data);
    DebugPrintf(2," track %i: AOT %i", i, outprog->decoderConfig[i].audioSpecificConfig.audioDecoderType.value);
  }

  /* Specify number of samples used for the encoder delay.
  This number represents the length of the buffer "twoFrame_DTimeSigBuf" in EncUsacFrame().
  Make sure to adjust "delayEncoder" accordingly if the amount of frames for the buffersize changes,
  to ensure the original file length functionality. */
  delayEncoder = EncUsac_getUsacEncoderDelay(*frameData->enc_data);

  DebugPrintf(2,"total track count: %i\n",(int)outprog->trackCount);

  /* num frames to start up encoder due to delay compensation */
  startupNumFrame = (delayNumSample+frameNumSample-1)/frameNumSample;

  /* seek to beginning of first (startup) frame (with delay compensation) */
  AudioSeek(audioFile,
            startSample+delayNumSample-startupNumFrame*frameNumSample);

  if (verboseLevel == VERBOSE_LVL2) {
    fprintf(stdout,"startupNumFrame=%d\n",startupNumFrame);
  }

  /* process audio file frame by frame */
  frame = -startupNumFrame;
  totNumSample = 0;

  for (i=0; i<track_count_total; i++) {
    DebugPrintf(2,"                   track %i: AOT %i",i,frameData->dec_conf[i].audioSpecificConfig.audioDecoderType.value);
  }

  /* **************************************************************** */
  /* MAIN ENCODING LOOP */
  /* **************************************************************** */
  do {
#ifdef FHG_DEBUGPLOT
    /*plotInit(framePlot,BsGetFileName(bitStream),1);*/
#endif

    if ((verboseLevel <= VERBOSE_LVL1) && (frame%20 == 0)) {
      fprintf(stdout,"\rframe %4d",frame);
      fflush(stdout);
    }
    if (verboseLevel == VERBOSE_LVL2) {
      fprintf(stdout,"\rframe %4d\n",frame);
      fflush(stdout);
    }

    /* check for startup frame */
    startupFrame = frame < 0;

    /* number of samples processed after this frame */
    nSamplesProcessed = (frame + 1) * (long)frameNumSample * downSampleRatio / upSampleRatio;

    /* 20060107 */
    if(sbrenable){
      /* save last (delayRSfilt+downSampleRatio*frameNumSample) samples */
      for (ch=0; ch<numChannel; ch++){
        for (i=0; i<delayRSfilt+delayTotal; i++){
          sampleBufRSfiltDelay[ch][i] = sampleBufRSold[ch][downSampleRatio*frameNumSample-(delayRSfilt+delayTotal)+i];
        }
        for (i=0; i<downSampleRatio*frameNumSample; i++){
          sampleBufRSold[ch][i] = sampleBufRS[ch][i];
        }
      }

      for (ch=0; ch<numChannel; ch++)
        for (i=0; i<NcoefUS/upSampleRatio; i++)
          sampleBufUSold[ch][i] = sampleBufUS[ch][downSampleRatio*frameNumSample/upSampleRatio-(NcoefUS/upSampleRatio)+i];
    }
    /* 20060107 */
    /* read audio file */
    /* 20060107 */
    if(sbrenable)
      numSample = AudioReadData(audioFile,sampleBufRS,frameNumSample*downSampleRatio/upSampleRatio);
    /* read audio file */
    else {
      numSample = AudioReadData(audioFile,sampleBuf,osf*frameNumSample);
      numSample /= osf;
    }
    totNumSample += numSample;
    if (numSample != frameNumSample && encNumSample == -1)
      encNumSample = totNumSample;

    /* get input signals for classification, process each frame with length 1024 samples */
    if (classfyData.isSwitchMode) {
      int di;
      for (di = 0; di < numSample; di++) {
        if (0 == sbrenable)
          classfyData.input_samples[classfyData.n_buffer_samples + di] = sampleBuf[0][di];
        else
          classfyData.input_samples[classfyData.n_buffer_samples + di] = sampleBufRS[0][di];
      }
      classfyData.n_buffer_samples += numSample;
      classification(&classfyData);
    }

    if (sbrenable){
      int nChannelsResamp = numChannel;

      if(usac212enable){
        int i_ch;        
        for (i_ch = 0; i_ch < numChannel; i_ch++) {
          int channelElementType = EncUsac_getChannelElementType(*frameData->enc_data, i_ch);
          if ((channelElementType == USAC_CHANNEL_ELEMENTTYPE_CPE_L ) || (channelElementType == USAC_CHANNEL_ELEMENTTYPE_CPE_DMX )) {
            int elemIdx = EncUsac_getChannelElementIndex(*frameData->enc_data, i_ch);
            Stream bitstream;
            HANDLE_SPATIAL_ENC enc = EncUsac_getSpatialEnc(*frameData->enc_data, elemIdx);
            int indepFlagOffset = enc->nBitstreamDelayBuffer - 1;
            unsigned char *databuf;
            int i, j;
            int size;
            int nchDmx = enc->outputChannels;
            int qceElement = EncUsac_getIsQceElement(*frameData->enc_data, elemIdx);
            const float inv_sqrt2 = (float)(1.0/sqrt(2.0));
            
            InitStream(&bitstream, NULL, STREAM_WRITE);
            
            
            for(i = 0; i < downSampleRatio*frameNumSample/upSampleRatio; i++)
              for(j = 0; j < 2; j++)
                inputBuffer[i*2+j] = sampleBufRS[i_ch+j][i];
                
            
            SpatialEncApply(enc, inputBuffer, downmixBuffer, &bitstream, EncUsac_getIndependencyFlag( *frameData->enc_data, indepFlagOffset));
            
            size = GetBitsInStream(&bitstream);
            
            ByteAlignWrite(&bitstream);
            
            databuf = bitstream.buffer;
            
            EncUsac_setSpatialData(*frameData->enc_data, databuf, size, elemIdx);

            /* pseudo LR downmix signal */
            if (nchDmx == 2) {
              if (!qceElement) {                
                for (i = 0; i < 2*frameNumSample; i++) {
                  float tmp = inv_sqrt2*(downmixBuffer[2*i] - downmixBuffer[2*i+1]);
                  downmixBuffer[2*i]   = inv_sqrt2*(downmixBuffer[2*i] + downmixBuffer[2*i+1]);
                  downmixBuffer[2*i+1] = tmp;
                }
              }
            }
            else {
              /* zero residual channel */
              for (i = 0; i < 2*frameNumSample; i++) 
                downmixBuffer[2*i]   = inv_sqrt2*(downmixBuffer[2*i] + downmixBuffer[2*i+1]);
                downmixBuffer[2*i+1] = 0.0f;
            }

            
            for (i=0; i<downSampleRatio*frameNumSample/upSampleRatio; i++)
              for (ch=0; ch<nchDmx; ch++)
                sampleBufRS[i_ch+ch][i] = downmixBuffer[nchDmx*i+ch];

            /* resort channels for QCE */
            if (qceElement == 2) {              
              float tmp;
              assert( i_ch > 1);
              for (i=0; i<downSampleRatio*frameNumSample/upSampleRatio; i++){               
                tmp =  sampleBufRS[i_ch][i];
                sampleBufRS[i_ch][i] = sampleBufRS[i_ch-1][i];
                sampleBufRS[i_ch-1][i] = tmp;
              }
            }

          }

          /* delay align channels */
          usac212_delayBufferWriteSamples(&delayBuffer[i_ch], sampleBufRS[i_ch], downSampleRatio*frameNumSample/upSampleRatio);
          usac212_delayBufferReadSamples( &delayBuffer[i_ch], sampleBufRS[i_ch], downSampleRatio*frameNumSample/upSampleRatio);
        }
      } 


      if(upSampleRatio > 1){
        for (i=0; i<downSampleRatio*frameNumSample/upSampleRatio; i++)
          for (ch=0; ch<nChannelsResamp; ch++)
            sampleBufUS[ch][i] = sampleBufRS[ch][i];

        for (i=0; i<NcoefUS/upSampleRatio; i++)
          for (ch=0; ch<nChannelsResamp; ch++)
            tmpBufUS[ch][i] = sampleBufUSold[ch][i];
        
        for (i=NcoefUS/upSampleRatio; i<downSampleRatio*frameNumSample/upSampleRatio+(NcoefUS/upSampleRatio); i++)
          for (ch=0; ch<nChannelsResamp; ch++)
            tmpBufUS[ch][i] = sampleBufUS[ch][i-(NcoefUS/upSampleRatio)];

        for (ch=0; ch<nChannelsResamp; ch++)
          FIupsample(tmpBufUS[ch], sampleBufRS[ch], downSampleRatio*frameNumSample, hUS, NcoefUS, upSampleRatio);

      }

      for (ch=0; ch<nChannelsResamp; ch++){
        for (i=0; i<(delayRSfilt+delayTotal); i++)
          tmpBufRS[ch][i] = sampleBufRSfiltDelay[ch][i];

        for (i=0; i<(downSampleRatio*frameNumSample); i++)
          tmpBufRS[ch][delayRSfilt+delayTotal+i] = sampleBufRSold[ch][i];

        /* delay input signal sampleBufRS */
        for (i=0; i<(downSampleRatio*frameNumSample); i++)
          tmpBufRS[ch][(delayRSfilt+delayTotal)+(downSampleRatio*frameNumSample)+i] = sampleBufRS[ch][i];
      }
      
#ifdef USE_AFSP
      /* Low-pass filtering and downsampling */
      for (ch=0; ch<nChannelsResamp; ch++)
        FIdownsample(tmpBufRS[ch]+addlCoreDelay, sampleBuf[ch], frameNumSample, hRS, NcoefRS, downSampleRatio);
#else
      CommonExit(1,"AFSP not available.");
#endif

    }

    if (0 == sbrenable) {
      if (DELAY_ENC_FRAMES-1 == frame) {
        nextIsIPF = 1;
      } else {
        nextIsIPF = 0;
      }
      if (DELAY_ENC_FRAMES == frame) {
        currentIsIPF = 1;
      } else {
        currentIsIPF = 0;
      }
    } else {
      nextIsIPF = 0;
      currentIsIPF = 0;
    }

    /* encode one frame */
    track_count_total = 0;
    /* mix down stereo to mono */
    encSampleBuf = sampleBuf;
    if ((numChannelBS == 1) && (numChannel == 2)) {
      for (i = 0; i < frameNumSample; i++) {
        reSampleBuf[0][i] = (sampleBuf[0][i] + sampleBuf[1][i]) / 2;
      }
      encSampleBuf = reSampleBuf;
    }

    err = frameData->enc_mode->EncFrame(encSampleBuf,
                                        &au_buffers[track_count_total],
                                        nextIsIPF,
                                        *frameData->enc_data,
                                        sbrenable ? ((upSampleRatio > 1) ? tmpBufUS : tmpBufRS) : NULL);
    if (0 != err) {
      CommonExit(1, "EncFrame returned %i", err);
    }

    track_count_total += *frameData->trackCount;

    /* Allowed resampling ratios based on sampling rates */
    if (fSample == 14700|| fSample == 16000) {
      resamplingRatio = 3.0;
    } else if (fSample == 22050 || fSample == 24000){
        resamplingRatio = 2.0;
    } else if (fSample == 29400 || fSample == 32000){
        resamplingRatio = 1.5;
    } else if (fSample == 44100 || fSample == 48000){
        resamplingRatio = 1.0;
    } else if (fSample == 58800 || fSample == 64000){
        resamplingRatio = 1.5;
    } else if (fSample == 88200 || fSample == 96000){
        resamplingRatio = 1.0;
    } else {
        CommonWarning("unsupported sampling rate!");
    }

    if (0 == sbrenable) {
      /* preset AudioTruncationInfo */
      presetMpeghAudioTruncationInfoDescr(&outprog->audioTruncationinfo[0]);

      rightTruncation = nSamplesProcessed - (encNumSample + delayEncoder);

      /* truncate the last frame if needed */
      if (rightTruncation > 0) {
        /* Adapt the right truncation to the decoder output samplerate i.e. 48kHz */
        processedSamplesPerFrame = (int)(frameNumSample/resamplingRatio);
        numDelayFrames = delayEncoder/processedSamplesPerFrame;
        resampledInput = (int)(resamplingRatio * encNumSample);
        totalFrames = (int)((resampledInput / processedSamplesPerFrame) + numDelayFrames);

        rightTruncation = (totalFrames+1)*processedSamplesPerFrame - (resampledInput + delayEncoder);
        setMpeghAudioTruncationInfoDescr(&outprog->audioTruncationinfo[0],
                                          0,
                                          (UINT32)rightTruncation);
      }
    }

    if ((frame >= DELAY_ENC_FRAMES) || (1 == sbrenable)) {
      for (i = 0; i < track_count_total; i++) {
        HANDLE_STREAM_AU au = StreamFileAllocateAU(0);
        au->numBits = BsBufferNumBit(au_buffers[i]);
        au->data = BsBufferGetDataBegin(au_buffers[i]);

        if (currentIsIPF) {
          outprog->stssSignalsSyncSample[i] = STSS_SYNCSAMPLE_SET;
        } else {
          outprog->stssSignalsSyncSample[i] = STSS_SYNCSAMPLE_NOTSET;
        }

        StreamPutAccessUnit(outprog, i, au);
        BsClearBuffer(au_buffers[i]);
        free(au);
      }
    }

    frame++;

#ifdef DEBUGPLOT
    framePlot++;
    plotDisplay(0);
#endif

#ifdef _DEBUG
    assert(nSamplesProcessed == frame*(long)frameNumSample*downSampleRatio/upSampleRatio);
#endif
  } while (encNumSample < 0 || nSamplesProcessed < encNumSample + delayEncoder
           );

  if (verboseLevel <= VERBOSE_LVL1) {
    fprintf(stdout," \n");
  }
  if (verboseLevel == VERBOSE_LVL2) {
    fprintf(stdout,"totNumFrame=%d\n", frame);
    fprintf(stdout,"encNumSample=%ld  (%.3f sec  %.3f frames)\n",
           encNumSample,encNumSample/fSample,
           encNumSample/(float)frameNumSample);
    fprintf(stdout,"totNumSample=%ld\n",totNumSample);
  }

  /* free encoder memory */
  if(frameData->enc_data && frameData->enc_mode){
      frameData->enc_mode->EncFree(*frameData->enc_data);
      if(frameData->enc_data) {
        free(frameData->enc_data);
      }
    free(frameData->enc_mode);
  }
  if(frameData->trackCount){
    free(frameData->trackCount);
  }

  free(frameData);

  /* close audio file */
  AudioClose(audioFile);

  /* close bit stream file */
  if (StreamFileClose(outfile))
    CommonExit(1,"Encode: error closing bit stream file");

  /* free buffers */
  for (ch=0; ch<numChannel; ch++) {
    free(sampleBuf[ch]);
    free(reSampleBuf[ch]);
  }
  free(sampleBuf);
  free(reSampleBuf);
  if (au_buffers) {
    for (i = 0; i < track_count_total; i++) {
      BsFreeBuffer(au_buffers[i]);
    }
    free(au_buffers);
  }

  if (asc_buffers) {
    for (i = 0; i < track_count_total; i++) {
      BsFreeBuffer(asc_buffers[i]);
    }
    free(asc_buffers);
  }
   

  /* free up-/downsampling buffers */
  if(hRS){
    free(hRS);
  }
   
  if(hUS){
    free(hUS);
  }

  if(sampleBufRS){
    for (ch=0; ch<numChannel; ch++) {
      if(sampleBufRS[ch]){
        free(sampleBufRS[ch]);
      }
    }
    free(sampleBufRS);
  }

  if(sampleBufUS){
    for (ch=0; ch<numChannel; ch++) {
      if(sampleBufUS[ch]){
        free(sampleBufUS[ch]);
      }
    }
    free(sampleBufUS);
  }

  if(sampleBufRSold){
    for (ch=0; ch<numChannel; ch++) {
      if(sampleBufRSold[ch]){
        free(sampleBufRSold[ch]);
      }
    }
    free(sampleBufRSold);
  }

  if(sampleBufUSold){
    for (ch=0; ch<numChannel; ch++) {
      if(sampleBufUSold[ch]){
        free(sampleBufUSold[ch]);
      }
    }
    free(sampleBufUSold);
  }

  if(tmpBufRS){
    for (ch=0; ch<numChannel; ch++) {
      if(tmpBufRS[ch]){
        free(tmpBufRS[ch]);
      }
    }
    free(tmpBufRS);
  }

  if(tmpBufUS){
    for (ch=0; ch<numChannel; ch++) {
      if(tmpBufUS[ch]){
        free(tmpBufUS[ch]);
      }
    }
    free(tmpBufUS);
  }

  if (delayBuffer) {
    for (ch=0; ch<numChannel; ch++) {
      usac212_delayBufferClose(&delayBuffer[ch]);
    }
    free(delayBuffer);
  }

  if (inputBuffer )
    free( inputBuffer );

  if ( downmixBuffer )
    free( downmixBuffer );


  /* 20060107 */
  return 0;
}


/* ---------- main ---------- */

int main (int argc, char *argv[])
{
  char *progName = "<no program name>";
  int              error                                   = 0;
  char             wav_InputFile[FILENAME_MAX]             = {0};
  char             mp4_OutputFile[FILENAME_MAX]            = {0};
  char           * codecMode;
  int              numChannelOut                           = 0;
  float            fSampleOut                              = 0.0f;
  int              sampleEntryFormat                       = 1;
  int              mainDebugLevel                          = 0;
  int              regionDuratUsed                         = 0;
  float            regionDurat                             = -1;
  float            regionStart                             = 0;
  HANDLE_ENCPARA   encPara = (HANDLE_ENCPARA)calloc(1, sizeof(ENC_PARA));

  VERBOSE_LEVEL    verboseLevel  = VERBOSE_NONE;  /* defines verbose level */

  codecMode = MODENAME_USAC;
  
  /* parse command line parameters */
  error = PrintCmdlineHelp(argc, argv);
  if (0 != error) {
    exit(0);
  }

  error = GetCmdline(argc,
                     argv,
                     wav_InputFile,
                     mp4_OutputFile,
                     &numChannelOut,
                     &fSampleOut,
                     &sampleEntryFormat,
                     encPara,
                     &mainDebugLevel,
                     &verboseLevel);


  if (0 != error) {
    CommonExit(1, "Error initializing MPEG USAC Audio decoder. Invalid command line parameters.");
  }
    
  Encode(wav_InputFile,
         mp4_OutputFile,
         codecMode,
         encPara,
         regionStart,
         regionDurat,
         numChannelOut,
         fSampleOut,
         mainDebugLevel,
         verboseLevel,
         sampleEntryFormat);

  if( verboseLevel >= VERBOSE_LVL1 ){
    fprintf(stdout,"%s: finished\n",progName);
  }

  return 0;
}

/* end of mp4enc.c */


