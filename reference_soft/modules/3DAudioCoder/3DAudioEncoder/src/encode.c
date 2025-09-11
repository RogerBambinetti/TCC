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

#define IIS_INTERN_PACTYP_LOUDNESS_INTEGRATION_ENCODER

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef IAR
#define IAR 1
#endif

#define SUPPORTED_COMPATIBLEPROFILELEVEL_MIN (18)   /* 0x12 */
#define SUPPORTED_COMPATIBLEPROFILELEVEL_MAX (20)   /* 0x14 */

typedef enum _CONTENT_TYPE {
  INPUT_INVALID = -1,
  INPUT_CHANNEL,
  INPUT_OBJECT,
  INPUT_MIXED

} INPUT_TYPE;

/* MPEG3DA executable naming */
typedef enum _Binary_Naming
{
  MPEG3DA_DESCRIPTION,
  MPEG3DA_WIN_EXE,
  MPEG3DA_UNIX_EXE
} MPEG3DA_Binary_Naming;

#ifdef WIN32
#define MPEG3DA_EXE MPEG3DA_WIN_EXE
#else
#define MPEG3DA_EXE MPEG3DA_UNIX_EXE
#endif


char wav_InputFile[FILENAME_MAX];                  /* Input mp4 file */
char mp4_OutputFile[FILENAME_MAX];                 /* Wav Output file */
char cfg_InputFile[FILENAME_MAX];                  /* config file containing binary names */
int  bitrate;                                      /* Bitrate used for encoding */

char binary_CoreEncoder[FILENAME_MAX];             /* location: core encoder executable */
char binary_DynamicObjectPriorityGenerator[FILENAME_MAX]; /* location: dynamic object priority generator executable */

char binary_ObjectMetadataDecoder[FILENAME_MAX];   /* location: object metadata decoder */
char binary_ObjectMetadataEncoder[FILENAME_MAX];   /* location: object metadata encoder */
char binary_SaocEncoder[FILENAME_MAX];             /* location: saoc encoder executable */
char binary_HoaEncoder[FILENAME_MAX];              /* location: hoa encoder executable */
char binary_FormatConverter[FILENAME_MAX];         /* location: format converter executable */
char binary_Separator[FILENAME_MAX];               /* location: module separating wav files containing ch+obj */
char binary_Renderer[FILENAME_MAX];                /* location: renderer module */
char binary_Mixer[FILENAME_MAX];                   /* location: mixer module */
char binary_WavM2N[FILENAME_MAX];                  /* location: wavM2N module */
char binary_earconEncoder[FILENAME_MAX];           /* location: earcon encoder executable */
char binary_prodmetadataEncoder[FILENAME_MAX];     /* location: production metadata encoder executable */

char inputFile_CoreEncoder[FILENAME_MAX];

char  inputFile_SaocEncoder[FILENAME_MAX];
char outputFile_SaocEncoder[FILENAME_MAX];

char outputFile_HoaEncoder[FILENAME_MAX];
char ham_HoaEncoder[FILENAME_MAX];

char  inputFile_Renderer[FILENAME_MAX];
char outputFile_Renderer[FILENAME_MAX];

char  inputFile_FormatConverter[FILENAME_MAX];
char outputFile_FormatConverter[FILENAME_MAX];

char  inputFile_Separator[FILENAME_MAX];
char ch_outputFile_Separator[FILENAME_MAX];
char obj_outputFile_Separator[FILENAME_MAX];


char  inputFile_Renderer[FILENAME_MAX];
char outputFile_Renderer[FILENAME_MAX];

char  inputFile1_Mixer[FILENAME_MAX];
char  inputFile2_Mixer[FILENAME_MAX];
char outputFile_Mixer[FILENAME_MAX];

char  inputFile_WavM2N[FILENAME_MAX];
char outputFile_WavM2N[FILENAME_MAX];

#if IAR
char  iarFile[FILENAME_MAX];
#endif

char  pmcFile[FILENAME_MAX];                      /* Production Metadata Configuration file */
char  omfFile[FILENAME_MAX];                      /* Object Metadata Configuration file */
char  cocfgFile[FILENAME_MAX];                    /* Channel-Object Configuration file */

int hasDynamicObjectPriority = 0;
int generateDynamicObjectPriority = 0;

int cicpIndex = 13;  /* 22.2 */

int inputFormat = 0;
int numObjectFiles = 0;
char noSbr = 0;
int coreEncoderFrameLength = 1024;
int streamingElement = 1;
int streamingElementGiven = 0;

int useLcProfile = 0;
int useBlProfile = 0;
int compatibleProfileLevel = 0;
float earconPCMloudness = 0.0f;
int lowDelayMetadataCoding = 0;
int highResolutionOam      = 0;


char globalBinariesPath[3*FILENAME_MAX]                   = "";

const char mpeg3da_binaries[][3][FILENAME_MAX] =
{
/*
  module description                        Windows executable                            Unix executable
  MPEG3DA_DESCRIPTION                       MPEG3DA_WIN_EXE                               MPEG3DA_UNIX_EXE
*/
  {{"core encoder"},                        {"usacEnc.exe"},                              {"usacEnc"}},
  {{"object metadata encoder"},             {"oamEncoderCmdl.exe"},                       {"oamEncoderCmdl"}},
  {{"SAOC encoder"},                        {"saocEncoderCmdl.exe"},                      {"saocEncoderCmdl"}},
  {{"HOA encoder"},                         {"hoaReferenceEncoder.exe"},                  {"hoaReferenceEncoder"}},
  {{"wavM2N"},                              {"wavM2NCmdl.exe"},                           {"wavM2NCmdl"}},
  {{"Dynamic object priority generator"},   {"DynamicObjectPriorityGeneratorCmdl.exe"},   {"DynamicObjectPriorityGeneratorCmdl"}},
  {{"format converter"},                    {"formatConverterCmdl.exe"},                  {"formatConverterCmdl"}},
  {{"separator"},                           {"separatorCmdl.exe"},                        {"separatorCmdl"}},
  {{"mixer"},                               {"mixerCmdl.exe"},                            {"mixerCmdl"}},
  {{"earcon encoder"},                      {"earconEncoder.exe"},                        {"earconEncoder"}},
  {{"production metadata encoder"},         {"prodmetadataEncoder.exe"},                  {"prodmetadataEncoder"}}
};

static int setBinaries(char* globalBinariesPath);
static int setBinaries_module(const char* globalBinariesPath, const char* module_description, const char* module_name, char* module_binary);
static int checkBinaries_module(const char* module_description, char* module_binary);

static int checkExistencyAndRemoveNewlineChar(char* string);   /* Helper function */
static int checkIfFileExists(char* string);

static int execCoreEncoder(void);                 /* Call to core encoder */

static int execDynamicObjectPriorityGenerator(void); /* Call to dynamic object priority generator */

static int execMetadataEncoder(void);             /* Call to metadata encoder */
static int execSaocEncoder(void);                 /* Call to saoc encoder */
static int execHoaSpatialEncoder(void);
static int execEarconEncoder(char* inputFile, char* outputFile, float pcmLoudness);
static int execProdMetadataEncoder(char const * const pmdFile, char const * const omfFile, char const * const cocfgFile, const unsigned int delayEncFrames);
static int execWavM2NForObjects(void);
static int execWavM2NForChannels(void);
static int getNumObjects(char* string);

void resetFileNames(void)
{
  memset(wav_InputFile, 0, sizeof(wav_InputFile));
  memset(mp4_OutputFile, 0, sizeof(mp4_OutputFile));
  memset(cfg_InputFile, 0, sizeof(cfg_InputFile));
  memset(inputFile_CoreEncoder, 0, sizeof(inputFile_CoreEncoder));
  memset(inputFile_SaocEncoder, 0, sizeof(inputFile_SaocEncoder));
  memset(outputFile_SaocEncoder, 0, sizeof(outputFile_SaocEncoder));
  memset(outputFile_HoaEncoder, 0, sizeof(outputFile_HoaEncoder));
  memset(ham_HoaEncoder, 0, sizeof(ham_HoaEncoder));
  memset(inputFile_Renderer, 0, sizeof(inputFile_Renderer));
  memset(outputFile_Renderer, 0, sizeof(outputFile_Renderer));
  memset(inputFile_FormatConverter, 0, sizeof(inputFile_FormatConverter));
  memset(outputFile_FormatConverter, 0, sizeof(outputFile_FormatConverter));
  memset(inputFile_Separator, 0, sizeof(inputFile_Separator));
  memset(ch_outputFile_Separator, 0, sizeof(ch_outputFile_Separator));
  memset(obj_outputFile_Separator, 0, sizeof(obj_outputFile_Separator));
  memset(inputFile_Renderer, 0, sizeof(inputFile_Renderer));
  memset(outputFile_Renderer, 0, sizeof(outputFile_Renderer));
  memset(inputFile1_Mixer, 0, sizeof(inputFile1_Mixer));
  memset(inputFile2_Mixer, 0, sizeof(inputFile2_Mixer));
  memset(outputFile_Mixer, 0, sizeof(outputFile_Mixer));
  memset(inputFile_WavM2N, 0, sizeof(inputFile_WavM2N));
  memset(outputFile_WavM2N, 0, sizeof(outputFile_WavM2N));
#if IAR
  memset(iarFile, 0, sizeof(iarFile));
#endif
  memset(pmcFile, 0, sizeof(pmcFile));
  memset(omfFile, 0, sizeof(omfFile));
  memset(cocfgFile, 0, sizeof(cocfgFile));
}

int main(int argc, char *argv[])
{
  int error = 0;
  int i = 0;
  char* basename = NULL;
  
  bitrate = 512000;
  ham_HoaEncoder[0] = 0;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 (FDIS) ****************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                               3D-Encoder Module                             *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n");
  fprintf( stdout, "\n");

  if ( argc < 5 ) {
    fprintf(stderr, "invalid arguments:\n");
#if IAR
    fprintf(stderr, "Usage: %s -if infile -of outfile.<mp4/mhas> -inConf <output_format> -ham <hoainfo> -iar <rendering3DType>\n", argv[0]);
#else
    fprintf(stderr, "Usage: %s -if infile -of outfile.<mp4/mhas> -inConf <output_format> -ham <hoainfo>\n", argv[0]);
#endif
    fprintf(stderr, "-if\n");
    fprintf(stderr, "\t Path to input wav (either with .wav or without _A+xxx_E+xx.wav for\n");
    fprintf(stderr, "\t channels or without _000.wav ending for objects!)\n");
    fprintf(stderr, "\t For channel based encoding, multiple mono wave files with 24 tracks\n");
    fprintf(stderr, "\t using the naming convention from the CfP, as well as multichannel wave\n");
    fprintf(stderr, "\t files (with specified .wav file extension) are supported.\n");
    fprintf(stderr, "\t For encoding objects make sure the corresponding object metadata file\n");
    fprintf(stderr, "\t is located in the same directory as the mono wave tracks.\n");
    fprintf(stderr, "\t For HOA encoding give the path to first HOA input wav\n");
    fprintf(stderr, "\t (including order and _00+.wav)\n");
    fprintf(stderr, "-of\n");
    fprintf(stderr, "\t Path to output file\n");
    fprintf(stderr, "-se\n");
    fprintf(stderr, "\t Streaming Element: only relevant for *.mp4 output files.\n");
    fprintf(stderr, "\t 0: 'mp4a'\n");
    fprintf(stderr, "\t 1: 'mha1' (default)\n");
    fprintf(stderr, "\t 2: 'mha1' + 'btrb'\n\n");
    fprintf(stderr, "-inConf\n");
    fprintf(stderr, "\t 0: Channel based encoding, 22.2 only (default)\n");
    fprintf(stderr, "\t 1: Discrete object encoding\n");
    fprintf(stderr, "\t 2: SAOC based object encoding\n");
    fprintf(stderr, "\t 3: HOA based encoding\n");
    fprintf(stderr, "\t 4: earcon encoding, requires -loudness\n");
    fprintf(stderr, "-br\n");
    fprintf(stderr, "\t Encoding bitrate (default: 512 kbps)\n");
    fprintf(stderr, "-ham\n");
    fprintf(stderr, "\t For HOA encoding the path to the HAM-file with HOA side-information.\n");
#if IAR
    fprintf(stderr, "-iar\n");
    fprintf(stderr, "\t rendering3DType flag file for immersive audio rendering.\n" );
#endif
    fprintf(stderr, "-pmc\n");
    fprintf(stderr, "\t Production metadata configuration file\n" );
    fprintf(stderr, "\t -pmc requires also -omf and -cocfg\n" );
    fprintf(stderr, "-omf\n");
    fprintf(stderr, "\t Object Metadata file\n" );
    fprintf(stderr, "\t -omf requires also -pmc and -cocfg\n" );
    fprintf(stderr, "-cocfg\n");
    fprintf(stderr, "\t Channel-Object Configuration file\n" );
    fprintf(stderr, "\t -cocfg requires also -pmc and -omf\n" );
    fprintf(stderr, "-generateDynamicObjectPriority\n");
    fprintf(stderr, "\t Switch for enabling dynamic object priority generation (default: 0)\n");
    fprintf(stderr, "\t Channel Based: -if CO_01_Church -of church.mp4 -inConf 0\n");
    fprintf(stderr, "\t Object Based:  -if CO_12_mechanism_4 -of mechanism.mp4 -inConf 1\n");
    fprintf(stderr, "-cicpIn\n");
    fprintf(stderr, "\t reference layout (cicp index).\n" );
    fprintf(stderr, "-useLcProfile\n");
    fprintf(stderr, "\t Generate a Low Complexity Profile, Level 3 bit stream.\n" );
    fprintf(stderr, "-useBlProfile\n");
    fprintf(stderr, "\t Generate a Baseline Profile, Level 3 bit stream.\n");
    fprintf(stderr, "-compatibleProfileLevel\n");
    fprintf(stderr, "\t Signal the Compatible Baseline Profile Level. Requires -useLcProfile to be set.\n");
    fprintf(stderr, "\t compatibleProfileLevel in range [%d;%d]\n",SUPPORTED_COMPATIBLEPROFILELEVEL_MIN, SUPPORTED_COMPATIBLEPROFILELEVEL_MAX);
    fprintf(stderr, "-lowDelayMetadataCoding <int>\n");
    fprintf(stderr, "\t 0:\t efficient object metadata decoding\n" );
    fprintf(stderr, "\t 1:\t object metadata decoding with low delay\n" );
    fprintf(stderr, "\t 2:\t object metadata decoding with low delay and high resolution\n" );
    fprintf(stderr, "\t <default: 0> \n" );
    fprintf(stderr, "-loudness\n");
    fprintf(stderr, "\t PCM loudness value in range [%.2f;%.2f]dB.\n", 6.0f, -57.75f );
    return(-1);
  }

  resetFileNames();

  for ( i = 1; i < argc; ++i )
    {
      if (!strcmp(argv[i], "-if"))		/* Required */
        {
          strncpy(wav_InputFile, argv[i+1], FILENAME_MAX) ;
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-of"))	/* Required */
        {
          strncpy(mp4_OutputFile, argv[i+1], FILENAME_MAX) ;
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-inConf"))	/* Optional */
        {
          inputFormat = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-br"))	/* Optional */
        {
          bitrate = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-ham"))	/* Required */
        {
          strncpy(ham_HoaEncoder, argv[i+1], FILENAME_MAX);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-se"))	/* Optional */
        {
          streamingElementGiven = 1;
          streamingElement = atoi(argv[i+1]);
          i++;
          continue;
        } 
#if IAR
      else if (!strcmp(argv[i], "-iar"))	/* Optional */
        {
          strncpy(iarFile, argv[i+1], FILENAME_MAX);
          i++;
          continue;
        }
#endif
      else if (!strcmp(argv[i], "-pmc"))	/* Optional */
        {
          strncpy(pmcFile, argv[i+1], FILENAME_MAX);
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-omf"))	/* Optional */
        {
          strncpy(omfFile, argv[i+1], FILENAME_MAX);
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-cocfg"))	/* Optional */
        {
          strncpy(cocfgFile, argv[i+1], FILENAME_MAX);
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-generateDynamicObjectPriority"))	/* Optional */
        {
          generateDynamicObjectPriority = atoi(argv[i+1]);
          i++;
          continue;
        }
     else if (!strcmp(argv[i], "-useLcProfile"))
       {
         useLcProfile = 1;
         noSbr = 1;
         continue;
       }
     else if (!strcmp(argv[i], "-useBlProfile"))	/* Optional */
      {
        useBlProfile = 1;
        noSbr = 1;
        continue;
      }
     else if (!strcmp(argv[i], "-compatibleProfileLevel"))	/* Optional */
      {
        compatibleProfileLevel = atoi(argv[i + 1]);
        if (compatibleProfileLevel < SUPPORTED_COMPATIBLEPROFILELEVEL_MIN || compatibleProfileLevel > SUPPORTED_COMPATIBLEPROFILELEVEL_MAX) {
          fprintf(stderr, "Error: compatibleProfileLevel shall be in the range of [%d, %d].\n", SUPPORTED_COMPATIBLEPROFILELEVEL_MIN, SUPPORTED_COMPATIBLEPROFILELEVEL_MAX);
          return -1;
        }
        continue;
      }
     else if (!strcmp(argv[i], "-lowDelayMetadataCoding"))
       {
         lowDelayMetadataCoding = atoi(argv[i+1]);
         if ( lowDelayMetadataCoding != 0 && lowDelayMetadataCoding != 1 && lowDelayMetadataCoding != 2 ) {
           fprintf(stderr, "Error: lowDelayMetadataCoding can only be 0, 1 or 2.\n");
           return -1;
         }
         if ( lowDelayMetadataCoding == 2 ) {
           highResolutionOam = 1;
         }
         i++;
         continue;
       }
      else if (!strcmp(argv[i], "-cicpIn"))	/* Optional */
        {
          cicpIndex = atoi(argv[i+1]);
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-loudness"))	/* Optional */
        {
          earconPCMloudness = (float)atof(argv[i+1]);
          i++;
          continue;
        }
    }

  if (0 == useLcProfile || 0 == useBlProfile || 0 == noSbr ){
    fprintf( stderr, "Warning: The original file length (OFL) can only be guaranteed when using the LC/BL profile and with disabled SBR! " );
    fprintf( stderr, "Please make sure to consider a correct OFL handling when implementing other profile settings.\n\n" );
  }

  if (0 == useLcProfile && compatibleProfileLevel >= SUPPORTED_COMPATIBLEPROFILELEVEL_MIN && compatibleProfileLevel <= SUPPORTED_COMPATIBLEPROFILELEVEL_MAX) {
    fprintf(stderr, "Error: When signaling a Baseline Profile compatible stream, -useLcProfile must be set! ");
    return -1;
  }

  if (0 != useLcProfile && 0 != useBlProfile) {
    fprintf(stderr, "Error: -useLcProfile and -useBlProfile must not be enabled simultaneously! ");
    return -1;
  }

  if (1 == useBlProfile && 2 == inputFormat) {
    fprintf(stderr, "Error: SAOC not supported in Baseline profile! ");
    return -1;
  }

  if (1 == useBlProfile && (3 == inputFormat || '\0' != ham_HoaEncoder[0])) {
    fprintf(stderr, "Error: HOA not supported in Baseline profile! ");
    return -1;
  }

  /* Get the binary names */
  fprintf (stdout, "Collecting executables...\n");

#ifdef _WIN32
    GetModuleFileName(NULL, globalBinariesPath, 3 * FILENAME_MAX);
#else
#ifdef __linux__
    readlink("/proc/self/exe", globalBinariesPath, 3 * FILENAME_MAX);
#endif

#ifdef __APPLE__ 
    uint32_t tmpStringLength = 3 * FILENAME_MAX;
    if ( _NSGetExecutablePath( globalBinariesPath, &tmpStringLength ) ) {
      return -1;
    }
#endif
#endif

#ifdef _WIN32
    basename = strstr(globalBinariesPath, "3DAudioEncoder.exe");
#else
    basename = strstr(globalBinariesPath, "3DAudioEncoder");
#endif

    if ( basename == 0 ) {
      return -1;
    }
    else {
      strcpy(basename, "\0");
    }

    error = setBinaries ( globalBinariesPath );
    if (error) return -1; 


  switch (inputFormat) {
    case 0:   /* channels */
      if (strstr( wav_InputFile, ".wav" ))
      {
        strcpy(inputFile_CoreEncoder, wav_InputFile);
      } else {
        execWavM2NForChannels();
        strcpy(inputFile_CoreEncoder, "output.wav");
      }
      break;

    case 1:   /* discrete objects */
      numObjectFiles = getNumObjects(wav_InputFile);
      execWavM2NForObjects();

      if (generateDynamicObjectPriority == 1)
      {
        error = execDynamicObjectPriorityGenerator();  
      }

      if ( highResolutionOam ) {
        noSbr = 1;
      }

      error = execMetadataEncoder();
      strcpy(inputFile_CoreEncoder, "output.wav");
      break;
  
    case 2:   /* SAOC objects */ 
      numObjectFiles = getNumObjects(wav_InputFile);
      execWavM2NForObjects();

      if (generateDynamicObjectPriority == 1)
      {
        error = execDynamicObjectPriorityGenerator();  
      }

      error = execMetadataEncoder();
      strcpy(outputFile_SaocEncoder, "saoc_output.wav");
      strcpy(inputFile_CoreEncoder, "saoc_output.wav");

      /* Call SAOC encoder */
      execSaocEncoder();
      break;

    case 3: /* HOA */
      /* enable sbr for bit rates greater or equal to 512000 */
      if(bitrate > 256000)
      {
        noSbr = 1;
        coreEncoderFrameLength = 1024;
      }
      else
      {
        noSbr = 0;
        coreEncoderFrameLength = 2048;
      }

      strcpy(outputFile_HoaEncoder, "hoa_output.wav");
      strcpy(inputFile_CoreEncoder, "hoa_output.wav");
      execHoaSpatialEncoder();
      break;
    case 4: /* earcon */
      execEarconEncoder(wav_InputFile, mp4_OutputFile, earconPCMloudness);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown input configuration, must be between 0 and 4.");
      break;
  }

  if (4 != inputFormat) {
    /* Call prodmetadata encoder */
    error = execProdMetadataEncoder(pmcFile, omfFile, cocfgFile, 1);
    if ( error ) return -1;

    /* Call core encoder */
    error = execCoreEncoder();
    if ( error ) return -1;
  }

  return 0;
}

static int execProdMetadataEncoder(char const * const pmdFile, char const * const omfFile, char const * const cocfgFile, const unsigned int delayEncFrames)
{
  if ( pmdFile[0] && omfFile[0] && cocfgFile[0] ) {
    int ccfg = 0;
    int idx = 0;
    char callPmdEncoder[3*FILENAME_MAX];
    char ocfg[FILENAME_MAX] = {'\0'};
    char  pmdOutputFile[] = "tmpFile3Ddec_pmd.dat";
    FILE* fptr = fopen(cocfgFile, "rb");

    if (!fptr) {
      return -1;
    }

    fscanf(fptr, "%d", &ccfg);
    while (!feof(fptr) && idx < sizeof(ocfg)) {
      char tmp = '\0';
      fscanf(fptr, "%c", &tmp);

      if (tmp >= '0' && tmp <= '9' || tmp == ' ') {
        ocfg[idx++] = tmp;
      }
    }

    sprintf(callPmdEncoder, "%s -ipmc %s -iomf %s -opmc %s -ccfg %d -ocfg %s -ndly %d", binary_prodmetadataEncoder, pmdFile, omfFile, pmdOutputFile, ccfg, ocfg, delayEncFrames);

    fprintf(stderr,"Calling Production Metadata Encoder\n %s\n", callPmdEncoder);

    if ( system(callPmdEncoder) )
    {   
      fprintf(stderr," Error running Production Metadata encoder\n");
      return -1;
    }
  }

  return 0;
}

static int execCoreEncoder(void)
{
  char callCoreEncoder[3*FILENAME_MAX];
  char extra_switches[2*FILENAME_MAX] = "\0";
  char  pmdOutputFile[] = "tmpFile3Ddec_pmd.dat";
  char tmpString[3 * FILENAME_MAX];

  /* basic command line */
  if ( streamingElementGiven ) {
    sprintf(callCoreEncoder, "%s -if %s -of %s -se %i", binary_CoreEncoder, inputFile_CoreEncoder, mp4_OutputFile, streamingElement);
  }
  else {
    sprintf(callCoreEncoder, "%s -if %s -of %s", binary_CoreEncoder, inputFile_CoreEncoder, mp4_OutputFile);
  }

  if ( cicpIndex != 13 ) {
    sprintf(tmpString, " -cicpIn %d", cicpIndex);
    strcat(extra_switches, tmpString);
  }

  if ( useLcProfile ) {
    strcat(extra_switches, " -useLcProfile");
  }

  if (useBlProfile) {
    strcat(extra_switches, " -useBlProfile");
  }

  if (compatibleProfileLevel >= SUPPORTED_COMPATIBLEPROFILELEVEL_MIN && compatibleProfileLevel <= SUPPORTED_COMPATIBLEPROFILELEVEL_MAX) {
    sprintf(tmpString, " -compatibleProfileLevel %d", compatibleProfileLevel);
    strcat(extra_switches, tmpString);
  }

  if(noSbr) { /* disable SBR */
    strcat(extra_switches, " -noSbr");
  }
  if ( pmcFile[0] && omfFile[0] && cocfgFile[0] ) {
    sprintf(tmpString, " -pmcFile %s", pmdOutputFile);
    strcat(extra_switches, tmpString);
  }

  if ( lowDelayMetadataCoding ) {
    if ( inputFormat != 1 ) {
      fprintf(stderr,"Error: lowDelayMetadataCoding only works with object content (\"-inConf 1\").\n");
      return -1;
    }
    sprintf(tmpString, " -lowDelayMetadataCoding %d", lowDelayMetadataCoding);
    strcat(extra_switches, tmpString);
  }


  /* create commandline */
  switch (inputFormat) {

    case 0: /* channels */
#if IAR
      if (iarFile[0]) {
        sprintf(tmpString, " -iarFile %s -br %d -contentType 0 %s", iarFile, bitrate, extra_switches);
      }
      else {
        sprintf(tmpString, " -br %d -contentType 0 %s", bitrate, extra_switches);
      }
#else
      sprintf(tmpString, " -br %d -contentType 0 %s", bitrate, extra_switches);
#endif
      strcat(callCoreEncoder, tmpString);
      break;
    case 1: /* discrete objects */
      sprintf(tmpString, " -br %d -objFile objData_1.dat -contentType 1 -hasDynamicObjectPriority %d %s", bitrate, hasDynamicObjectPriority, extra_switches);
      strcat(callCoreEncoder, tmpString);
      break;
    case 2: /* SAOC objects */
      sprintf(tmpString, " -br %d -objFile objData_1.dat -saocFile saocData.bs -saocBitrateFile saocBitrate.txt -contentType 2 -hasDynamicObjectPriority %d %s", bitrate, hasDynamicObjectPriority, extra_switches);
      strcat(callCoreEncoder, tmpString);
      break;
    case 3: /* HOA */
      sprintf(tmpString, " -br %d -hoaFile hoaData.bs -hoaBitrateFile hoaBitrate.txt -contentType 3 %s", bitrate, extra_switches);
      strcat(callCoreEncoder, tmpString);
      break;
    default:
      break;
  }

  fprintf(stderr,"Calling Core Encoder\n %s\n", callCoreEncoder);

  if ( system(callCoreEncoder) )
  {   
    fprintf(stderr," Error running core encoder\n");
    return -1;
  }

  return 0;
}

static int execDynamicObjectPriorityGenerator(void)
{
  char callDynamicObjectPriorityGenerator[3*FILENAME_MAX];
  
   if ( numObjectFiles <= 0 )
   {
     fprintf(stderr, "Error: No object files were found\n");
     return -1;
   }

  sprintf(callDynamicObjectPriorityGenerator, "%s -if_wav %s -if_oam \"%s.oam\" -of_oam \"generator_output.oam\" -bs 2048 ", 
          binary_DynamicObjectPriorityGenerator, wav_InputFile, wav_InputFile);
  
  fprintf(stderr, "Calling Dynamic Object Priority Generator\n %s\n", callDynamicObjectPriorityGenerator);

  if ( system (callDynamicObjectPriorityGenerator) )
  {
    fprintf(stderr," Error calling dynamic object priority generator\n");
    return -1;
  }

  return 0;
}

static int execMetadataEncoder(void)
{
  char callObjectMetadataEncoder[3*FILENAME_MAX];
  
   if ( numObjectFiles <= 0 )
   {
     fprintf(stderr, "Error: No object files were found\n");
     return -1;
   }

   if ( generateDynamicObjectPriority == 1 )
   {
     sprintf(callObjectMetadataEncoder, "%s -if \"generator_output.oam\" -of objData -of_hasDynamicObjectPriority \"hasDynamicObjectPriority\" -dummyFrames 2 ", 
             binary_ObjectMetadataEncoder);
   }
   else
   {
     sprintf(callObjectMetadataEncoder, "%s -if \"%s.oam\" -of objData -of_hasDynamicObjectPriority \"hasDynamicObjectPriority\" -dummyFrames 2 ", 
             binary_ObjectMetadataEncoder, wav_InputFile);
   }

   if ( lowDelayMetadataCoding ) {
     strcat(callObjectMetadataEncoder, " -ld 1 ");

     if (highResolutionOam) {
       strcat(callObjectMetadataEncoder, " -oamBs 256 -highRate 1 ");
     }

   }

  fprintf(stderr, "Calling Metadata Encoder\n %s\n", callObjectMetadataEncoder);

  if ( system (callObjectMetadataEncoder) )
  {
    fprintf(stderr," Error calling object metadata encoder\n");
    return -1;
  }

  {
    FILE* f = fopen("hasDynamicObjectPriority", "rb");
    if (!f)
    { 
      fprintf(stderr, "Unable to open hasDynamicObjectPriority file containing binary\n");
      return -1;
    }
	if (fread(&hasDynamicObjectPriority, sizeof(int), 1, f) != 1)
	{
	  fprintf(stderr, "Error reading hasDynamicObjectPriority\n");
      return -1;
	}
	fclose(f);
  }

  return 0;
}

static int execSaocEncoder()
{
  char callSaocEncoder[3*FILENAME_MAX];
  sprintf(callSaocEncoder, "%s -if output.wav -of %s -bs saocData.bs -br saocBitrate.txt -numChan 0 -numObj %i -numDmxChan 0 -numDmxObj %i", 
          binary_SaocEncoder, outputFile_SaocEncoder, numObjectFiles, (int)((float)(numObjectFiles)/6.f+0.5f));
  
  fprintf(stderr,"Calling SAOC Encoder\n %s\n", callSaocEncoder);

  if ( system (callSaocEncoder) )
    {
      fprintf(stderr," Error calling saoc encoder\n");
      return -1;
    }

  return 0;
}

static int execHoaSpatialEncoder(void)
{
  char callHoaEncoder[3*FILENAME_MAX];
  sprintf(callHoaEncoder, "%s -if \"%s\" -ofpcm \"%s\" -ofside hoaData.bs -ofsize hoaBitrate.txt -br %i -coreFrameLength %i ", 
          binary_HoaEncoder, wav_InputFile, outputFile_HoaEncoder, bitrate, coreEncoderFrameLength);
  
  if(noSbr == 0)
    strcat(callHoaEncoder, "-sbr ");

  if(strlen(ham_HoaEncoder) > 0)
  {
    strcat(callHoaEncoder, "-ham \"");
    strcat(callHoaEncoder, ham_HoaEncoder);
    strcat(callHoaEncoder, "\"");
  }

  fprintf(stderr,"Calling HOA Encoder\n %s\n", callHoaEncoder);

  if ( system (callHoaEncoder) )
    {
      fprintf(stderr," Error calling hoa encoder\n");
      return -1;
    }

  return 0;
}

static int execEarconEncoder(char* inputFile, char* outputFile, float pcmLoudness)
{
  char callEarconEncoder[3*FILENAME_MAX];

  /* basic command line */
  sprintf(callEarconEncoder, "%s -if %s -of %s -loudness %f", binary_earconEncoder, inputFile, outputFile, pcmLoudness);

  fprintf(stderr,"Calling Earcon Encoder\n %s\n", callEarconEncoder);

  if ( system(callEarconEncoder) )
  {   
    fprintf(stderr," Error running earcon encoder\n");
    return -1;
  }

  return 0;
}

static int execWavM2NForObjects(void)
{
  char callWavM2N[FILENAME_MAX];  

  sprintf(callWavM2N, "%s -if \"%s\" -of output -nObj %d  -dir 0 ", binary_WavM2N, wav_InputFile, numObjectFiles);

  strcat(callWavM2N, " > wavM2NForObjects.log");
  
  fprintf(stderr,"Calling wavM2N to combine object files\n %s\n", callWavM2N);
  
  if ( system (callWavM2N) )
    {
      fprintf(stderr," Error calling wavM2N\n");
      return -1;
    }

  return 0;

}

static int execWavM2NForChannels(void)
{
  char callWavM2N[FILENAME_MAX];

  sprintf(callWavM2N, "%s -if %s -of output -cicpIn %d -dir 0 ", binary_WavM2N, 
          wav_InputFile, cicpIndex);

  strcat(callWavM2N, " > wavM2NForChannels.log");
   
  fprintf(stderr,"Calling wavM2N to create MC-wave files\n %s\n", callWavM2N);
  
  if ( system (callWavM2N) )
    {
      fprintf(stderr," Error calling wavM2N\n");
      return -1;
    }
  
  return 0;

}


static int checkBinaries_module( const char* module_description, char* module_binary )
{
  int retError = 0;

  if ( checkExistencyAndRemoveNewlineChar ( module_binary ) ) {
      fprintf ( stderr, "\nWarning: No %s support.\n", module_description);
    retError = -1;
  } else {
      fprintf ( stderr, "\nUsing %s binary:\n %s\n", module_description, module_binary );
  }

  return retError;
}

static int setBinaries_module ( const char* globalBinariesPath, const char* module_description, const char* module_name, char* module_binary )
{
  int retError = 0;

  strcpy( module_binary, globalBinariesPath );
  strcat( module_binary, module_name );

  retError = checkBinaries_module(module_description, module_binary);

  return retError;
}

static int setBinaries ( char* globalBinariesPath )
{
  int retError = 0;
  int nBinaries_tmp = 0;

  /***********************************************
               binary_CoreEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[0][MPEG3DA_DESCRIPTION], mpeg3da_binaries[0][MPEG3DA_EXE], binary_CoreEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_ObjectMetadataEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[1][MPEG3DA_DESCRIPTION], mpeg3da_binaries[1][MPEG3DA_EXE], binary_ObjectMetadataEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_SaocEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[2][MPEG3DA_DESCRIPTION], mpeg3da_binaries[2][MPEG3DA_EXE], binary_SaocEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_HoaEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[3][MPEG3DA_DESCRIPTION], mpeg3da_binaries[3][MPEG3DA_EXE], binary_HoaEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_WavM2N
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[4][MPEG3DA_DESCRIPTION], mpeg3da_binaries[4][MPEG3DA_EXE], binary_WavM2N);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
       binary_DynamicObjectPriorityGenerator
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[5][MPEG3DA_DESCRIPTION], mpeg3da_binaries[5][MPEG3DA_EXE], binary_DynamicObjectPriorityGenerator);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_FormatConverter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[6][MPEG3DA_DESCRIPTION], mpeg3da_binaries[6][MPEG3DA_EXE], binary_FormatConverter);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_Separator
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[7][MPEG3DA_DESCRIPTION], mpeg3da_binaries[7][MPEG3DA_EXE], binary_Separator);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_Mixer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[8][MPEG3DA_DESCRIPTION], mpeg3da_binaries[8][MPEG3DA_EXE], binary_Mixer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_Renderer
  ***********************************************/
  /* retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[9][MPEG3DA_DESCRIPTION], mpeg3da_binaries[9][MPEG3DA_EXE], binary_Renderer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++; */

  /***********************************************
               binary_earconEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[9][MPEG3DA_DESCRIPTION], mpeg3da_binaries[9][MPEG3DA_EXE], binary_earconEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_prodmetadataEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[10][MPEG3DA_DESCRIPTION], mpeg3da_binaries[10][MPEG3DA_EXE], binary_prodmetadataEncoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  return retError;
}

static int checkExistencyAndRemoveNewlineChar(char* string)
{
  FILE* fExists;
  int len;

  len = strlen(string);
  if ( string[len - 1] == '\n')
    string[len - 1] = '\0';


  fExists = fopen(string, "r");
  if (!fExists)
    {
      fprintf(stderr, "Error binary %s not present.\nCheck the corresponding config file entry.\n", string);
      return -1;
    } 

  fclose(fExists);

  return 0;
}

static int checkIfFileExists(char* string)
{
  FILE* fExists;

  fExists = fopen(string, "r");
  if (!fExists) 
    return 0;

  fclose(fExists);
  return 1;
}

static int getNumObjects(char* string) {
  int numObjects = 0;
  char objectFile[FILENAME_MAX];

  sprintf(objectFile, "%s_%03d.wav", string, numObjects);

  while ( checkIfFileExists(objectFile) ) {
    numObjects++;
    sprintf(objectFile, "%s_%03d.wav", string, numObjects);
  }

  return numObjects;
}
  



    
