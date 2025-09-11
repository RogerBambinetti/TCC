#ifndef __DECODE_TYPES_H__
#define __DECODE_TYPES_H__


#define MPEG3DA_ERROR_FIRST -666

#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2 || _MSC_VER >= 1300
#define __func__ __FUNCTION__
#else
#define __func__ "<unknown>"
#endif
#else
#define __func__ "<unknown>"
#endif
#define MPEG3DA_ERROR_POSITION_INFO __FILE__,__func__,__LINE__


typedef enum _PROFILE
{
  PROFILE_UNDEFINED      = -1,
  PROFILE_RESERVED_ISO   =  0,
  PROFILE_MAIN           =  1,
  PROFILE_HIGH           =  2,
  PROFILE_LOW_COMPLEXITY =  3,
  PROFILE_BASELINE       =  4
} PROFILE;

typedef enum _LEVEL
{
  L_UNDEFINED     = -1,
  L_RESERVED_ISO  =  0,
  L1              =  1,
  L2              =  2,
  L3              =  3,
  L4              =  4,
  L5              =  5
} LEVEL;

typedef enum _MPEG3DA_OUTPUT_RATE
{
  MPEG3DA_44100   = 44100,
  MPEG3DA_48000   = 48000,
  MPEG3DA_96000   = 96000
} MPEG3DA_OUTPUT_RATE;

typedef enum _RETURN_CODE
{
  MPEG3DA_ERROR_DRC_INIT            = MPEG3DA_ERROR_FIRST,
  MPEG3DA_ERROR_DRC1_OBJPATH,
  MPEG3DA_ERROR_BINAURAL_INIT,
  MPEG3DA_ERROR_MIXER_INIT,
  MPEG3DA_ERROR_ASC_INIT,
  MPEG3DA_ERROR_ASC_PARSING,
  MPEG3DA_ERROR_ASCEXT_PARSING,
  MPEG3DA_ERROR_ASI_PARSING,
  MPEG3DA_ERROR_APR_INIT,
  MPEG3DA_ERROR_ATI_INIT,
  MPEG3DA_ERROR_ATI_PARSING,
  MPEG3DA_ERROR_ATI_GENERIC,
  MPEG3DA_ERROR_SGC_PARSING,
  MPEG3DA_ERROR_SGI_PARSING,
  MPEG3DA_ERROR_EOMC_PARSING,
  MPEG3DA_ERROR_LIS_PARSING,
  MPEG3DA_ERROR_HOAMTX_PARSING,
  MPEG3DA_ERROR_INPUTFORMAT,
  MPEG3DA_ERROR_PCT_INVALID_STREAM,
  MPEG3DA_ERROR_PCT_INVALID_BASELINE_STREAM,
  MPEG3DA_ERROR_CPLS_PARSING,
  MPEG3DA_ERROR_PCT_INVALID_LC_STREAM,
  MPEG3DA_ERROR_PCT_INVALID_LC_CONFIG,
  MPEG3DA_ERROR_DOMAINSW_EXEC,
  MPEG3DA_ERROR_DOMAINSW_NOSWITCH,
  MPEG3DA_ERROR_DOMAINSW_GENERIC,
  MPEG3DA_ERROR_WAVCUTTER_CLIPPING,
  MPEG3DA_ERROR_WAVCUTTER_OUTPUTFILE,
  MPEG3DA_ERROR_CORE_INVALID_STREAM,
  MPEG3DA_ERROR_CORE_CLIPPING,
  MPEG3DA_ERROR_CORE_GENERIC,
  MPEG3DA_ERROR_3DA_PLAYOUT,
  MPEG3DA_ERROR_3DA_EXEC_MODE2,
  MPEG3DA_ERROR_3DA_EXEC_MODE1,
  MPEG3DA_ERROR_3DA_MISSINGEXE,
  MPEG3DA_ERROR_3DA_INVALIDEXE,
  MPEG3DA_ERROR_3DA_INVALIDCMD,
  MPEG3DA_ERROR_3DA_BITDEPTH,
  MPEG3DA_ERROR_3DA_INIT,
  MPEG3DA_ERROR_EARCON_GENERIC,
  MPEG3DA_ERROR_EARCON_FILE_FORMAT_MP4,
  MPEG3DA_ERROR_3DA_GENERIC         = -1,
  MPEG3DA_OK                        = 0
} MPEG3DA_RETURN_CODE;

#ifndef CONTENT_TYPE_DEF  
  #define CONTENT_TYPE_DEF
  typedef enum _CONTENT_TYPE
  {
    INPUT_INVALID     =  0,         /* 0x00 = 0000                                              */
    INPUT_CHANNEL     =  1,         /* 0x01 = 0001                                              */
    INPUT_OBJECT      =  2,         /* 0x02 = 0010                                              */
    INPUT_CHOBJ,                    /*        0011 = INPUT_CHANNEL & INPUT_OBJECT               */
    INPUT_SAOC        =  4,         /* 0x04 = 0100                                              */
    INPUT_CHSAOC,                   /*        0101 = INPUT_CHANNEL & INPUT_SAOC                 */
    INPUT_OBJSAOC,                  /*        0110 = INPUT_OBJECT  & INPUT_SAOC                 */
    INPUT_CHOBJSAOC,                /*        0111 = INPUT_CHANNEL & INPUT_OBJECT & INPUT_SAOC  */
    INPUT_HOA         =  8,         /* 0x08 = 1000                                              */
    INPUT_DIFFUSE     =  9,         /* 0x09 = 1001                                              */
    INPUT_CHANNEL_SD  = 10,         /* 0x0a = 1010                                              */
    INPUT_EARCON      = 16          /* 0x10 = 0001 0000                                         */
  } INPUT_TYPE;
#endif

typedef enum _BIT_DEPTH
{
  BIT_DEPTH_DEFAULT = 0,
  BIT_DEPTH_16_BIT  = 16,
  BIT_DEPTH_24_BIT  = 24,
  BIT_DEPTH_32_BIT  = 32
} BIT_DEPTH;

typedef enum _RECYCLE_LEVEL
{
  RECYCLE_INACTIVE,
  RECYCLE_ACTIVE
} RECYCLE_LEVEL;

typedef enum _MPEG3DA_CONFPOINT
{
  MPEG3DA_DEFAULT_OUTPUT    = 0,    /* 0 - 127:   ISO use */
  MPEG3DA_CPO_1,
  MPEG3DA_CORE_OUTPUT       = 128,  /* 128 - ...: proprietary use */
  MPEG3DA_SKIP_EARCON,
  MPEG3DA_CONTRIBUTIONMODE
} MPEG3DA_CONFPOINT;

typedef enum _VERBOSE_LEVEL
{
  VERBOSE_NONE,
  VERBOSE_LVL1,
  VERBOSE_LVL2
} VERBOSE_LEVEL;

typedef enum _DEBUG_LEVEL
{
  DEBUG_NORMAL,
  DEBUG_LVL1,
  DEBUG_LVL2
} DEBUG_LEVEL;

#ifndef MDPREPROC_TYPE_DEF 
  #define MDPREPROC_TYPE_DEF
  typedef enum _MDPREPROC_TYPE
  {
    OBJECTS_WITH_DIVERGENCE     = 0,
    OBJECTS_NO_DIVERGENCE       = 1,
    DIFFUSE_FEED                = 2,
    CHANNELS_COMPLETE           = 3,
    CHANNELS_SCENE_DISPLACEMENT = 4,
    CHANNELS_NO_DISPLACEMENT    = 5

  } MDPREPROC_INPUT_TYPE;
#endif

#ifndef CICP_FORMAT_DEF
  #define CICP_FORMAT_DEF
  typedef enum _CICP_FORMAT
  {
    CICP_CONTRIBUTION = -3,
    CICP_INVALID      = -2,
    CICP_FROM_GEO     = -1,
    CICP_ANY          =  0,   /* loudspeaker layout index:  0       */
    CICP_1_0,                 /* loudspeaker layout index:  1       */
    CICP_2_0,                 /* loudspeaker layout index:  2       */
    CICP_3_0,                 /* loudspeaker layout index:  3       */
    CICP_4_0_FRONT,           /* loudspeaker layout index:  4       */
    CICP_5_0,                 /* loudspeaker layout index:  5       */
    CICP_5_1,                 /* loudspeaker layout index:  6       */
    CICP_7_1_FRONT,           /* loudspeaker layout index:  7       */
    CICP_1_p1,                /* loudspeaker layout index:  8       */
    CICP_3_0_REAR,            /* loudspeaker layout index:  9       */
    CICP_4_0,                 /* loudspeaker layout index: 10       */
    CICP_6_1,                 /* loudspeaker layout index: 11       */
    CICP_7_1_REAR,            /* loudspeaker layout index: 12       */
    CICP_22_2,                /* loudspeaker layout index: 13       */
    CICP_7_1_ELEV,            /* loudspeaker layout index: 14       */
    CICP_10_2,                /* loudspeaker layout index: 15       */
    CICP_5_1_p4,              /* loudspeaker layout index: 16       */
    CICP_5_1_p6,              /* loudspeaker layout index: 17       */
    CICP_7_1_p6,              /* loudspeaker layout index: 18       */
    CICP_7_1_p4,              /* loudspeaker layout index: 19       */
    CICP_9_1_p4,              /* loudspeaker layout index: 20       */
    CICP_RESERVED             /* loudspeaker layout index: 21 - 63  */
  } CICP_FORMAT;
#endif

/* enum for the processing domains */
typedef enum {
  NOT_DEFINED = -1,
  TIME_DOMAIN = 0,
  QMF_DOMAIN = 1,
  STFT_DOMAIN = 2
} PROCESSING_DOMAIN;

/* enums for the domain switching */
typedef enum {
  KEEP_DOMAIN   = 0,
  QMF_SYNTHESIS = 1,
  QMF_ANALYSIS  = 2,
  STFT_SYNTHESIS = 3,
  STFT_ANALYSIS  = 4
} DOMAIN_SWITCH;

/* enum for indicating DRC processing domains */
typedef enum {
  DISABLED   = -1,
  SINGLEBAND = 0,
  MULTIBAND  = 1
} DRC_PROC_MODE;

/* info structure to indicate which modules are active and in which domain */
typedef struct {

  PROCESSING_DOMAIN coreCoder;
  PROCESSING_DOMAIN drc1channels;
  PROCESSING_DOMAIN formatConverter;
  PROCESSING_DOMAIN drc1objects;
  PROCESSING_DOMAIN objectRenderer;
  PROCESSING_DOMAIN saocRenderer;
  PROCESSING_DOMAIN drc1hoa;
  PROCESSING_DOMAIN hoaRendererInput;
  PROCESSING_DOMAIN hoaRendererOutput;
  PROCESSING_DOMAIN mixer;
  PROCESSING_DOMAIN drc2channels;
  PROCESSING_DOMAIN drc2binaural;
  PROCESSING_DOMAIN tdBinaural;
  PROCESSING_DOMAIN fdBinaural;
  PROCESSING_DOMAIN hoaBinaural;
  PROCESSING_DOMAIN drc3channel;
  PROCESSING_DOMAIN loudnessNorm;
  PROCESSING_DOMAIN peakLimiter;

} MODULE_PROCESSING_DOMAINS;

typedef struct {
  int pathDelay;
  int delayToPayloadApplication;
} RENDERER_DELAY_INFO;


typedef struct {

  RENDERER_DELAY_INFO channelPath;
  RENDERER_DELAY_INFO objectPath;
  RENDERER_DELAY_INFO hoaPath;
  RENDERER_DELAY_INFO saocPath;

  /* Profile dependend delay which has to be filled up before mixer stage. */
  unsigned int     maxDelay;

} RENDERER_PATH_DELAYS;


/* "present"-flags for the modules of the rendering domain */
typedef struct {

  unsigned int  bFormatConversionChannels;
  unsigned int  bObjectRendering;
  unsigned int  bSaocRendering;
  unsigned int  bHoaRendering;
  DRC_PROC_MODE modeDrc1Channels;
  DRC_PROC_MODE modeDrc1Objects;
  DRC_PROC_MODE modeDrc1Saoc;
  DRC_PROC_MODE modeDrc1Hoa;

} RENDERING_PROPERTIES;


/* "present"-flags for the modules of the post processing domain */
typedef struct {

  DRC_PROC_MODE modeDrc2Channels;
  DRC_PROC_MODE modeDrc3Channels;
  DRC_PROC_MODE modeDrc2Binaural;
  unsigned int  bFdBinauralizer;
  unsigned int  bTdBinauralizer;
  unsigned int  bHoaBinauralizer;

} POST_PROCESSING_PROPERTIES;


/* "present"-flags for the modules of the end of chain */
typedef struct {

  unsigned int bsHasDrc3;
  unsigned int bLoudnessNormalization;
  unsigned int bPeakLimiter;
  unsigned int decNeedsLSDistanceCompensation;

} END_OF_CHAIN_PROPERTIES;

/* info containing audioPreRoll*/
typedef struct {
  int nSamplesCoreCoderPreRoll;
  int numPrerollAU;
} AUDIOPREROLL_INFO, *HAUDIOPREROLL_INFO;

/* info containing audioTruncationInfo */
typedef struct {
  int isActive_left;
  int isActive_right;
  int nTruncSamples_left;
  int nTruncSamples_right;
} AUDIOTRUNC_INFO, *HAUDIOTRUNC_INFO;

/* info structure containing core decoder parameters */
typedef struct {
  AUDIOPREROLL_INFO apr;
  AUDIOTRUNC_INFO ati;
} CORE_DECODER_PROPERTIES, *HCORE_DECODER_PROPERTIES;

/* info structure containing all present flags of all paths */
typedef struct {

  unsigned int                 bCoreChannels;

  unsigned int                 bCoreHasSbr;

  RENDERING_PROPERTIES         renderingComponents;
  POST_PROCESSING_PROPERTIES   postProcessingComponents;

  END_OF_CHAIN_PROPERTIES    endOfChainComponents;

} DECODER_PROPERTIES;


/* info structure containing all switching decisions of the channel path */
typedef struct {

  DOMAIN_SWITCH     PostCore;
  DOMAIN_SWITCH     PreDrc1;
  DOMAIN_SWITCH     PostDrc1;
  DOMAIN_SWITCH     PreFormatConverter;
  DOMAIN_SWITCH     PostFormatConverter;

}  CHANNEL_DOMAIN_SWITCHES;


/* info structure containing all switching decisions of the object path */
typedef struct {

  DOMAIN_SWITCH     PreDrc1;
  DOMAIN_SWITCH     PostDrc1;
  DOMAIN_SWITCH     PreObjectRendering;
  DOMAIN_SWITCH     PostObjectRendering;

}  OBJECT_DOMAIN_SWITCHES;


/* info structure containing all switching decisions of the saoc path */
typedef struct {

  DOMAIN_SWITCH     PreSaocRendering;
  DOMAIN_SWITCH     PostSaocRendering;

} SAOC_DOMAIN_SWITCHES;


/* info structure containing all switching decisions of the hoa path */
typedef struct {

  DOMAIN_SWITCH     PreDrc1;
  DOMAIN_SWITCH     PostDrc1;
  DOMAIN_SWITCH     PreHoaRendering;
  DOMAIN_SWITCH     PostHoaRendering;

} HOA_DOMAIN_SWITCHES;


/* info structure containing all switching decisions of the binaural path */
typedef struct {

  DOMAIN_SWITCH     PreFdBinaural;
  DOMAIN_SWITCH     PostFdBinaural;
  DOMAIN_SWITCH     PreTdBinaural;
  DOMAIN_SWITCH     PostTdBinaural;
  DOMAIN_SWITCH     PreHoaBinaural;
  DOMAIN_SWITCH     PostHoaBinaural;
  DOMAIN_SWITCH     PostBinauralDrc2;

} BINAURAL_DOMAIN_SWITCHES;


/* info structure containing all switching decisions */
typedef struct {

  /* RENDERING */
  CHANNEL_DOMAIN_SWITCHES  channelDomains;
  OBJECT_DOMAIN_SWITCHES   objectDomains;
  SAOC_DOMAIN_SWITCHES     saocDomains;
  HOA_DOMAIN_SWITCHES      hoaDomains;

  /* POST-PROCESSING*/
  DOMAIN_SWITCH            PostMixer;
  DOMAIN_SWITCH            PreDrc2;
  DOMAIN_SWITCH            PostDrc2;

  BINAURAL_DOMAIN_SWITCHES binauralDomains; 

} DOMAIN_SWITCHING_INFO;

/* info structure containing DRC decoder parameters */
typedef struct {
    
  int drcDecoderState;
  int lnState;
    
  int loudnessInfoAvailable;
  int drcDataAvailable;
    
  int uniDrcInterfacePresent;
    
  int targetLoudnessLevelPresent;
  float targetLoudnessLevel;
    
  int drcEffectTypeRequestPresent;
  unsigned long drcEffectTypeRequest;
    
  int downmixIdPresent;
  int downmixId;

  int downmixConfigAvailable;
  int numDownmixIdsDownmixConfig;
  int downmixIdDownmixConfig[15];
  int targetChannelCountForDownmixId[15];
    
  int numGroupIdsRequested;
  int groupIdRequested[15];
  int numGroupPresetIdsRequested;
  int groupPresetIdRequested[15];
  int numMembersGroupPresetIdsRequested[15];
  int groupPresetIdRequestedPreference;
    
  char* binaryDrcDecoder;
  char* txtMpegh3daParams;
  char* txtDownmixMatrixSet;
  char* txtDrcSelectionTransferData;
  char* bsMpegh3daUniDrcConfig;
  char* bsMpegh3daLoudnessInfoSet;
  char* bsUnDrcGain;
    
  int framesize;
    
  int drcChannelOffsetChannelPath;
  int drcChannelOffsetObjectPath;
  int drcChannelOffsetSaocPath;
  int drcChannelOffsetHoaPath;
    
  PROFILE profile;

} DRC_PARAMS;

/* print 3da error code */
MPEG3DA_RETURN_CODE PrintErrorCode( const MPEG3DA_RETURN_CODE mpeg3da_code, int* error_depth, const char* error_msg, const char* error_hlp, const VERBOSE_LEVEL verboseLvl, const char* file, const char* func, const int line );

/* get number of channels corresponding to a CICP ChannelConfiguration */
int getNumberOfCicpChannels( const CICP_FORMAT cicpIdx, int* nChannels );

/* Apply QMF_ANALYSIS */
MPEG3DA_RETURN_CODE transformT2F( char* inputFilenameTimeWithoutExtension, char* binary_DomainSwitcher, int stftFlag );

/* Apply QMF_SYNTHESIS */
MPEG3DA_RETURN_CODE transformF2T( char* inputFilenameFreqWithoutExtension, char* binary_DomainSwitcher, int stftFlag );

/* checks if filename contains a .wav suffix */
int checkIfWavExtension( char* filename );

/* removes .wav suffix from filename */
int removeWavExtension( char* filename );

/* call binary - wrapper for system() */
int callBinary( const char* exe_2call, const char* exe_description, const VERBOSE_LEVEL verboseLvl, const unsigned int hideErrorMsg);

/* get delay of the core decoder in constant delay mode */
int GetCoreCoderConstantDelay ( void );

#endif

