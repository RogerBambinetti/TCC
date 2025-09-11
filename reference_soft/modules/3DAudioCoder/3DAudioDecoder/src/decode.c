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
#  include <errno.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

/**********************************************************************************************************************************************************************************/

#include "decode_types.h"
#include "ascparser.h"
#include "formatConverter_api.h"
#include "saoc_getinfo.h"  /* To get the number of SAOC output channels */
#include "initRenderPaths.h"
#include "channelPath.h"
#include "objectPath.h"
#include "saocPath.h"
#include "hoaPath.h"
#include "fdBinauralPath.h"
#include "tdBinauralPath.h"
#include "elementMetadataPreprocessorPath.h"
#include "profileLevelHandling.h"

/* #define RM6_INTERNAL_CHANNEL */

#define MPEG3DA_USAC_VERSION_NUMBER    "04.00.10"
#define MPEG3DA_USAC_MODULE_NAME       "mpegh3dars"

/* Delay number */
#define FORMAT_CONVERTER_FD_DELAY (2432 + 2048)
#define FORMAT_CONVERTER_TD_DELAY (4033 + 1024)
#define BINAURAL_FD_DELAY         (33)
#define SAOC_ROUNDING_DELAY       (820)
#define MAX_SBR_MPS_OFFSET_DELAY  (384 + 321 + 384)
#define MIN_SBR_MPS_OFFSET_DELAY  (384 + 321)
#define CORE_QMF_SYNTHESIS_DELAY  (257)

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

typedef struct
{
  int idx;
  int Az;
  int El;
  int AzStart;
  int AzEnd;
  int ElStart;
  int ElEnd;
  int isLfe;
  int posRelative;
} CICP_TOLERANCE_INDEX;

const CICP_TOLERANCE_INDEX CICP_TOLERANCEINDICES[] =
{/* idx  az    el lfe screenRel   loudspeaker type             informative label (merely descriptive) */
  { 0,   30,     0,    15,    45,   -15,    15,     0,   0},
  { 1,  -30,     0,   -45,   -15,   -15,    15,     0,   0},
  { 2,    0,     0,   -10,    10,   -15,    15,     0,   0},
  { 3,    0, -1337, -1337, -1337, -1337, -1337,     1,   0},
  { 4,  110,     0,    90,   130,   -15,    15,     0,   0},
  { 5, -110,     0,  -130,   -90,   -15,    15,     0,   0},
  { 6,   22,     0,     7,    37,   -15,    15,     0,   0},
  { 7,  -22,     0,   -37,    -7,   -15,    15,     0,   0},
  { 8,  135,     0,   120,   150,   -15,    15,     0,   0},
  { 9, -135,     0,  -150,  -120,   -15,    15,     0,   0},
  {10,  180,     0,   170,   190,   -15,    15,     0,   0},
  {11,   -1,    -1, -1000, -1000, -1000, -1000,     0,   0},
  {12,   -1,    -1, -1000, -1000, -1000, -1000,     0,   0},
  {13,   90,     0,    70,   110,   -15,    15,     0,   0},
  {14,  -90,     0,  -110,   -70,   -15,    15,     0,   0},
  {15,   60,     0,    40,    80,   -15,    15,     0,   0},
  {16,  -60,     0,   -80,   -40,   -15,    15,     0,   0},
  {17,   30,    35,    15,    45,    15,    55,     0,   0},
  {18,  -30,    35,   -45,   -15,    15,    55,     0,   0},
  {19,    0,    35,   -15,    15,    15,    55,     0,   0},
  {20,  135,    35,   115,   155,    15,    55,     0,   0},
  {21, -135,    35,  -155,  -115,    15,    55,     0,   0},
  {22,  180,    35,   165,   195,    15,    55,     0,   0},
  {23,   90,    35,    70,   110,    15,    55,     0,   0},
  {24,  -90,    35,  -110,   -70,    15,    55,     0,   0},
  {25,    0,    90,  -180,   180,    60,    90,     0,   0},
  {26,   45,   -15, -1000, -1000, -1000, -1000,     1,   0},
  {27,   45,   -15,    25,    65,   -40,     0,     0,   0},
  {28,  -45,   -15,   -65,   -25,   -40,     0,     0,   0},
  {29,    0,   -15,   -15,    15,   -40,     0,     0,   0},
  {30,  110,    35,    90,   130,    15,    55,     0,   0},
  {31, -110,    35,  -130,   -90,    15,    55,     0,   0},
  {32,   45,    35,    30,    60,    15,    55,     0,   0},
  {33,  -45,    35,   -60,   -30,    15,    55,     0,   0},
  {34,   45,     0,    30,    60,   -15,    15,     0,   0},
  {35,  -45,     0,   -60,   -30,   -15,    15,     0,   0},
  {36,  -45,   -15, -1000, -1000, -1000, -1000,     1,   0},
  {37,   60,     0,    15,    80,   -15,    15,     0,   1},
  {38,  -60,     0,   -80,   -15,   -15,    15,     0,   1},
  {39,   30,     0,     7,    40,   -15,    15,     0,   1},
  {40,  -30,     0,   -40,    -7,   -15,    15,     0,   1},
  {41,  150,     0,   135,   165,   -15,    15,     0,   0},
  {42, -150,     0,  -165,  -135,   -15,    15,     0,   0}
};

const unsigned int CICP_TOLERANCEINDICES_SIZE = 43;

typedef struct _Cicp_ChannelConfiguration
{
  CICP_FORMAT cicpIdx;
  int nChannels;
} CICP_CHANNELCONFIG;

const CICP_CHANNELCONFIG cicp_channeConfigTable[CICP_RESERVED] =
{
  {CICP_ANY,       -1},
  {CICP_1_0,        1},
  {CICP_2_0,        2},
  {CICP_3_0,        3},
  {CICP_4_0_FRONT,  4},
  {CICP_5_0,        5},
  {CICP_5_1,        6},
  {CICP_7_1_FRONT,  8},
  {CICP_1_p1,       2},
  {CICP_3_0_REAR,   3},
  {CICP_4_0,        4},
  {CICP_6_1,        7},
  {CICP_7_1_REAR,   8},
  {CICP_22_2,      24},
  {CICP_7_1_ELEV,   8},
  {CICP_10_2,      12},
  {CICP_5_1_p4,    10},
  {CICP_5_1_p6,    12},
  {CICP_7_1_p6,    14},
  {CICP_7_1_p4,    12},
  {CICP_9_1_p4,    14}
};

/**********************************************************************************************************************************************************************************/

char mp4_InputFile[FILENAME_MAX];                                                                       /* Input mp4 file */
char wav_OutputFile[FILENAME_MAX];                                                                      /* Wav Output file */
char cfg_InputFile[FILENAME_MAX];                                                                       /* Config Input file */
char earconMetaData_OutputFile[FILENAME_MAX] = { 0 };                                                   /* Ouput file for earcon metadata */
char  omfFile[FILENAME_MAX];                                                                            /* Object Metadata Configuration file */
char  cocfgFile[FILENAME_MAX];                                                                          /* Channel-Object Configuration file */
char BinReadBRIRsFromLocation[FILENAME_MAX];
char* BinReadBRIRsFromLocationPtr = NULL;
char BinReadBRIRsGeometryFromLocation[FILENAME_MAX];
char* BinReadBRIRsGeometryFromLocationPtr = NULL;
char WriteBRIRsBitstreamPath[FILENAME_MAX];
char *WriteBRIRsBitstreamPathPtr = NULL;
char WriteBRIRsLowLevelBitstreamPath[FILENAME_MAX];
char *WriteBRIRsLowLevelBitstreamPathPtr = NULL;
char binary_CoreDecoder[FILENAME_MAX];                                                                  /* location: core decoder executable */
char binary_ObjectMetadataDecoder[FILENAME_MAX];                                                        /* location: object metadata decoder */
char binary_SaocDecoder[FILENAME_MAX];                                                                  /* location: saoc decoder executable */
char binary_HoaDecoder[FILENAME_MAX];                                                                   /* location: hoa decoder executable */
char binary_earconDecoder[FILENAME_MAX];                                                                /* location: earcon decoder executable */
char binary_prodmetadataDecoder[FILENAME_MAX];                                                          /* location: production metadata decoder executable */
char binary_HoaMatrixDecoder[FILENAME_MAX];                                                             /* location: hoa matrix decoder executable */
char binary_FormatConverter[FILENAME_MAX];                                                              /* location: format converter executable */
#if IAR
char binary_IarFormatConverter[FILENAME_MAX];                                                              /* location: format converter executable */
#endif
char binary_Separator[FILENAME_MAX];                                                                    /* location: module separating wav files containing ch+obj */
char binary_Renderer[FILENAME_MAX];                                                                     /* location: renderer module */
char binary_Mixer[FILENAME_MAX];                                                                        /* location: mixer module */
char binary_FdBinauralBitstreamWriter[FILENAME_MAX];
char binary_FdBinauralRenderer[FILENAME_MAX];                                                           /* location: frequency domain binauralizer module */
char binary_FdBinauralParam[FILENAME_MAX];                                                              /* location: frequency domain binaural parametrization module */
char binary_TdBinauralRenderer[FILENAME_MAX];                                                           /* location: time domain binauralizer module */
char binary_TdBinauralParam[FILENAME_MAX];                                                              /* location: time domain binaural parametrization module */
char binary_WavM2N[FILENAME_MAX];                                                                       /* location: wavM2N module */
char binary_DrcSelection[FILENAME_MAX];                                                                 /* location: Drc Set Selection module */
char binary_DrcDecoder[FILENAME_MAX];                                                                   /* location: Drc Decoder module */
char binary_LoudnessNormalizer[FILENAME_MAX];                                                           /* location: Loudness Normalizer module */
char binary_PeakLimiter[FILENAME_MAX];                                                                  /* location: Peak Limiter module */
char binary_DomainSwitcher[FILENAME_MAX];                                                               /* location: domainSwitcher module */
char binary_WavCutter[FILENAME_MAX];                                                                    /* location: wavCutter module */
char binary_DmxMatrixDecoder[FILENAME_MAX];                                                             /* location: dmxMatrixCoder */
char binary_ResampAudio[FILENAME_MAX];                                                                  /* location: dmxMatrixCoder */
RECYCLE_LEVEL     recycleLevel;                                                                         /* defines the clean-up behavior */
VERBOSE_LEVEL     verboseLevel;                                                                         /* defines verbose level */
DEBUG_LEVEL       debugLevel;                                                                           /* defines debuging level */
int               inputType;
BIT_DEPTH         mpeg3daBitDepth = BIT_DEPTH_32_BIT;                                                   /* bitdepth of the MPEG-H 3DA processing chain */
BIT_DEPTH         outputBitDepth;                                                                       /* bitdepth of output file */
MPEG3DA_CONFPOINT mpeg3daCpoType = MPEG3DA_DEFAULT_OUTPUT;                                              /* output mode type of the MPEG-H 3DA decoder */
CICP_FORMAT       inputCicpReferenceLayout;
CICP_FORMAT       inputCicpAudioChannelLayout;
CICP_FORMAT       outputCicp = CICP_INVALID;
int               passiveDownmixFlag;                                                                   /* Is read from dmx. matrix config extension element in bitstream */
int               phaseAlignStrength;                                                                   /* Read from downmixConfig */
int               immersiveDownmixFlag;                                                                 /* Read from downmixConfig */
int               downmixConfigAvailable;
ASCPARSER_DOWNMIX_CONFIG  downmixConfig;
int               matrixBitSize;
char*             downmixMatrix;
int               downmixId;
int               hoaMatrixConfigAvailable;
ASCPARSER_HOA_CONFIG  hoaMatrixConfig;
int               hoaMatrixBitSize;
int               HoaRenderingMatrixId;
char*             hoaMatrix;
DRC_PARAMS        drcParams;
int               monoWavOut;
int               fDBinaural;                                                                           /* Apply freq. domain binauralization */
int               BinReadBRIRType;
int               tDBinaural;                                                                           /* Apply time domain binauralization */
int               bFormatConverter;
int               bHoaBinaryPresent = 0;
int               bHoaMatrixBinaryPresent = 0;
int               bIarBinaryPresent = 0;
int               bEarconDecoderBinaryPresent = 0;
int               bProdmetadataDecoderBinaryPresent = 0;
int               nSamplesCoreCoderDelay;
int               nSamplesPerChannel;
int               bDelayAlignMpegTestpoints;
int               bRemoveConstantDecoderDelay;
int               targetSamplingRate;
int               disableResampling;
PROCESSING_DOMAIN mixerDomain;
int               bResamplerActive;
float             resamplingRatio;
PROFILE           profile;
LEVEL             level;
ASCPARSERINFO     asc;
ASCPARSER_AUDIO_SCENE asi;
int hasASI;
int payloadOffsetDelay;
int bUseWindowsCommands;                                                                                /* Windows or Linux commands defined in decode_types.h */
int decodedOAM;
int hasASI = 0;
int separateSetups = 0;
int splittingInfoType[MAE_MAX_NUM_ELEMENTS];
int splittingInfoSetup[MAE_MAX_NUM_ELEMENTS];
int numAddedObjects = 0;
int hasDiffuseness = 0;
int totalNumObjAfterMDP = 0;
char oamInpath[FILENAME_MAX] = "tmpFile3Ddec_separator_object.oam";
char oamOutpath[FILENAME_MAX] = "tmpFile3Ddec_mdp_object.oam";
char outputFile_MDPreprocessor[FILENAME_MAX] = "tmpFile3Ddec_mdp_output";
char oamOutpath_sceneDisplacementChannels[FILENAME_MAX] = "tmpFile3Ddec_mdp_sceneDisplacement_channels.oam";
char sceneDisplacementChannels_indexListOutpath[FILENAME_MAX] = "tmpFile3Ddec_mdp_sceneDisplacement_channels_idx.txt";
char fileOutGeoMDPreproc[FILENAME_MAX];
char binary_LocalSetupInformationInterface[FILENAME_MAX];
int sceneDsplInterfaceEnabled = eSceneDsplOff;
int hasSceneDisplacementChannels = 0;
int numSceneDisplacementChannels = 0;
int chooseSGmember = -1;
int selectedPresetID = -1;
int writeToObjectOutputInterface = 0;
int disableFormatConverter = 0;
ASCPARSER_SIGNAL_GROUP_CONFIG signalGroupConfig;
char elementInteraction_InputFile[FILENAME_MAX];
char localSetup_InputFile[FILENAME_MAX];
char sceneDisplacement_InputFile[FILENAME_MAX];
ASCPARSER_SIGNAL_GROUP_INFO signalGroupInfo;
ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG enhancedObjectMetadataConfig;
ASCPARSER_PRODUCTION_METADATA_CONFIG productionMetadataConfig;
int signalGroupInfoAvailable;
int hasElementInteraction = 0;
CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[MAE_MAX_NUM_SPEAKERS];
CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo_misplaced[MAE_MAX_NUM_SPEAKERS];
int numChannelsTmp;
int numLFEsTmp;

char tempFileNames_MDP_tempForSplitting[][1][FILENAME_MAX] =
{
  {"tmpFile3Ddec_mdp_objects_withDivergence_temp_output"},
  {"tmpFile3Ddec_mdp_objects_noDivergence_temp_output"},
  {"tmpFile3Ddec_mdp_diffuse_temp_output"},
  {"tmpFile3Ddec_mdp_channels_temp_output"},
  {"tmpFile3Ddec_mdp_channels_sceneDisplacement_temp_output"},
  {"tmpFile3Ddec_mdp_channels_noDisplacement_temp_output"},
};

char tempFileNames_MDP_afterSplitting[][3][FILENAME_MAX] =
{
  {{"out of MDPreprocessor"},                             {"after rendering"},                                             {"resampled"}},
  {{"tmpFile3Ddec_mdp_channels_output"},                  {"tmpFile3Ddec_mdp_channels_output_rendered"},                   {"tmpFile3Ddec_mdp_channels_output_resampled"}},
  {{"tmpFile3Ddec_mdp_objects_output"},                   {"tmpFile3Ddec_mdp_objects_output_rendered"},                    {"tmpFile3Ddec_mdp_objects_output_resampled"}},
  {{"dummy"},                                             {"rendered"},                                                    {"resampled"}},
  {{"tmpFile3Ddec_mdp_saoc_output"},                      {"tmpFile3Ddec_mdp_saoc_output_rendered"},                       {"tmpFile3Ddec_mdp_saoc_output_resampled"}},
  {{"dummy"},                                             {"rendered"},                                                    {"resampled"}},
  {{"dummy"},                                             {"rendered"},                                                    {"resampled"}},
  {{"dummy"},                                             {"rendered"},                                                    {"resampled"}},
  {{"tmpFile3Ddec_mdp_hoa_output"},                       {"tmpFile3Ddec_mdp_hoa_output_rendered"},                        {"tmpFile3Ddec_mdp_hoa_output_resampled"}},
  {{"tmpFile3Ddec_mdp_diffuse_output"},                   {"tmpFile3Ddec_mdp_diffuse_output_rendered"},                    {"tmpFile3Ddec_mdp_diffuse_output_resampled"}},
  {{"tmpFile3Ddec_mdp_channels_sceneDisplacement_output"},{"tmpFile3Ddec_mdp_channels_sceneDisplacement_output_rendered"}, {"tmpFile3Ddec_mdp_channels_sceneDisplacement_output_resampled"}},
};

const char mpeg3da_binaries[][3][FILENAME_MAX] =
{
/*
  module description                      Windows executable                      Unix executable
  MPEG3DA_DESCRIPTION                     MPEG3DA_WIN_EXE                         MPEG3DA_UNIX_EXE
*/
  {{"core decoder"},                      {"3DAudioCoreDecoder.exe"},             {"3DAudioCoreDecoder"}},
  {{"object metadata decoder"},           {"oamDecoder.exe"},                     {"oamDecoder"}},
  {{"SAOC decoder"},                      {"saocDecoderCmdl.exe"},                {"saocDecoderCmdl"}},
  {{"HOA decoder"},                       {"hoaReferenceDecoder.exe"},            {"hoaReferenceDecoder"}},
  {{"format converter"},                  {"formatConverterCmdl.exe"},            {"formatConverterCmdl"}},
  {{"IarFormatConverter"},                {"iarFormatConverterCmdl.exe"},         {"iarFormatConverterCmdl"}},
  {{"separator"},                         {"separatorCmdl.exe"},                  {"separatorCmdl"}},
  {{"gVBAP renderer"},                    {"gVBAPRendererCmdl.exe"},              {"gVBAPRendererCmdl"}},
  {{"mixer"},                             {"mixerCmdl.exe"},                      {"mixerCmdl"}},
  {{"wavM2N"},                            {"wavM2NCmdl.exe"},                     {"wavM2NCmdl"}},
  {{"FD BinauralRenderer"},               {"binauralRendererFdCmdl.exe"},         {"binauralRendererFdCmdl"}},
  {{"TD BinauralRenderer"},               {"BinauralRendererTdCmdl.exe"},         {"binauralRendererTdCmdl"}},
  {{"FD BinauralParametrization"},        {"binauralParametrizationFdCmdl.exe"},  {"binauralParametrizationFdCmdl"}},
  {{"TD BinauralParametrization"},        {"binauralParametrizationTdCmdl.exe"},  {"binauralParametrizationTdCmdl"}},
  {{"Binaural Bitstream Writer"},         {"bitstreamWriterCmdl.exe"},            {"bitstreamWriterCmdl"}},
  {{"DomainSwitcher"},                    {"domainSwitcherCmdl.exe"},             {"domainSwitcherCmdl"}},
  {{"wavCutter"},                         {"wavCutterCmdl.exe"},                  {"wavCutterCmdl"}},
  {{"DmxMatrixDecoder"},                  {"dmxMatrixDecoderCmdl.exe"},           {"dmxMatrixDecoderCmdl"}},
  {{"ResampAudio"},                       {"ResampAudio.exe"},                    {"ResampAudio"}},
  {{"DRC Set Selection"},                 {"uniDrcSelectionProcessCmdl.exe"},     {"uniDrcSelectionProcessCmdl"}},
  {{"DRC decoder"},                       {"uniDrcGainDecoderCmdl.exe"},          {"uniDrcGainDecoderCmdl"}},
  {{"Loudness Normalizer"},               {"loudnessNormalizerCmdl.exe"},         {"loudnessNormalizerCmdl"}},
  {{"Local Setup Information Interface"}, {"localSetupInterfaceExample.exe"},     {"localSetupInformationInterfaceExample"}},
  {{"Peak Limiter"},                      {"peakLimiterCmdl.exe"},                {"peakLimiterCmdl"}},
  {{"HOA matrix decoder"},                {"hoaMatrixDecoderCmdl.exe"},           {"hoaMatrixDecoderCmdl"}},
  {{"earcon decoder"},                    {"earconDecoder.exe"},                  {"earconDecoder"}},
  {{"production metadata decoder"},       {"prodmetadataDecoder.exe"},            {"prodmetadataDecoder"}}
};

/**********************************************************************************************************************************************************************************/

DECODER_PROPERTIES     decoderProperties;
DOMAIN_SWITCHING_INFO  domainSwitchingDecisions;
MODULE_PROCESSING_DOMAINS moduleProcessingDomains;
RENDERER_PATH_DELAYS   rendererPathDelays;

/**********************************************************************************************************************************************************************************/

char globalBinariesPath[3*FILENAME_MAX]                   = "";
char fileOutGeo[FILENAME_MAX];
char fileOutGeoDeviation[FILENAME_MAX];
char* fileOutGeoDeviationPtr                              = NULL;
int speakerMisplacement                                   = 0;
char fileOutGeo_misplaced[FILENAME_MAX];
char outputFile_CoreDecoder[FILENAME_MAX]                 = "tmpFile3Ddec_core_output";
char outputFile_Mixer[FILENAME_MAX]                       = "tmpFile3Ddec_mixer_output";
char outputFile_inGeoRefLayout[FILENAME_MAX]              = "tmpFile3Ddec_inGeoRefLayout.txt";
char outputFile_inGeoAudioChannelLayout[FILENAME_MAX]     = "tmpFile3Ddec_inGeoAudioChannelLayout.txt";
#ifdef SUPPORT_SAOC_DMX_LAYOUT
char outputFile_inGeoSaocDmxLayout[FILENAME_MAX]          = "tmpFile3Ddec_inGeoSaocDmxLayout.txt";
CICP_FORMAT       inputCicpSaocDmxChannelLayout;
#endif
char outputFile_Binaural[]                                = "tmpFile3Ddec_bin_output";
char inputFile_uniDrcInterface[FILENAME_MAX];
char outputFile_drcSelection_mpegh3daParams[FILENAME_MAX] = "tmpFile3Ddec_drcSelection_mpegh3daParams_output.txt";
char outputFile_drcDecoder_downmixMatrixSet[FILENAME_MAX] = "tmpFile3Ddec_drcDecoder_downmixMatrixSet_output.txt";
char outputFile_drcSelectionTransferData[FILENAME_MAX]    = "tmpFile3Ddec_drcSelectionTransferData_output.txt";
char outputFile_multiBandDrcPresent[FILENAME_MAX]         = "tmpFile3Ddec_multiBandDrcPresent_output.txt";
char outputFile_mpegh3daUniDrcConfig[FILENAME_MAX]        = "config_tmpFile3Ddec_bitstreamDrc_output.bit";
char outputFile_mpegh3daLoudnessInfoSet[FILENAME_MAX]     = "tmpFile3Ddec_mpegh3daLoudnessInfoSet_output.bit";
char outputFile_unDrcGain[FILENAME_MAX]                   = "tmpFile3Ddec_bitstreamDrc_output.bit";
char outputFile_Drc2[FILENAME_MAX]                        = "tmpFile3Ddec_drc2_output.wav";
char outputFile_Drc3[FILENAME_MAX]                        = "tmpFile3Ddec_drc3_output.wav";         /* always time domain */
char outputFile_LoudnessNorm[FILENAME_MAX]                = "tmpFile3Ddec_LNorm_output";
char outputFile_PeakLimiter[FILENAME_MAX]                 = "tmpFile3Ddec_PeakLim_output.wav";      /* always time domain */
char outputFile_earconDecoder[FILENAME_MAX]               = "tmpFile3Ddec_earcon_output";
  char pmcFilename[FILENAME_MAX]                          = "tmpFile3Ddec_pmc.dat";
char mhasFilename[FILENAME_MAX]                           = "";

char tempFileNames[][4][FILENAME_MAX] =
{
  {{"in"},                                {"out"},                          {"resampled"}},
  {{"tmpFile3Ddec_channel_core_output"},  {"tmpFile3Ddec_channel_output"},  {"tmpFile3Ddec_channels_resampled"},  {"tmpFile3Ddec_channels_mixer_ready"}},
  {{"tmpFile3Ddec_object_core_output"},   {"tmpFile3Ddec_object_output"},   {"tmpFile3Ddec_objects_resampled"},   {"tmpFile3Ddec_objects_mixer_ready"}},
  {{"in"},                                {"out"},                          {"resampled"}},
  {{"tmpFile3Ddec_saoc_core_output"},     {"tmpFile3Ddec_saoc_output"},     {"tmpFile3Ddec_saoc_resampled"},      {"tmpFile3Ddec_saoc_mixer_ready"}},
  {{"in"},                                {"out"},                          {"resampled"}},
  {{"in"},                                {"out"},                          {"resampled"}},
  {{"in"},                                {"out"},                          {"resampled"}},
  {{"tmpFile3Ddec_hoa_core_output"},      {"tmpFile3Ddec_hoa_output"},      {"tmpFile3Ddec_hoa_resampled"},       {"tmpFile3Ddec_hoa_mixer_ready"}}
};

char logFileNames[][FILENAME_MAX] =
{
  {"framework.log"},
  {"channel_path.log"},
  {"object_path.log"},
  {"in"},
  {"saoc_path.log"},
  {"in"},
  {"in"},
  {"in"},
  {"hoa_path.log"},
  {"binaural.log"}
};

/* ###################################################################### */
/* ##                 MPEG 3DA decoder static functions                ## */
/* ###################################################################### */
static int PrintCmdlineHelp(
            int                               argc,
            char                             *argv0,
            const int                         versionMajor,
            const int                         versionMinor
) {
  fprintf ( stdout, "\n" );
  fprintf ( stdout, "******************** MPEG-H 3D Audio Coder - Edition %d.%d (FDIS) ****************\n", versionMajor, versionMinor);
  fprintf ( stdout, "*                                                                             *\n" );
  fprintf ( stdout, "*                                3D-Decoder Module                            *\n" );
  fprintf ( stdout, "*                                                                             *\n" );
  fprintf ( stdout, "*                                  %s                                *\n", __DATE__ );
  fprintf ( stdout, "*                                                                             *\n" );
  fprintf ( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n" );
  fprintf ( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n" );
  fprintf ( stdout, "*                                                                             *\n" );
  fprintf ( stdout, "*******************************************************************************\n" );

  if ( argc < 5 )
  {
    fprintf ( stdout, "\n" );
    fprintf ( stdout, "Usage: %s -if infile.mp4 -of outfile.wav [options]\n\n", argv0 );
    fprintf ( stdout, "-if <s>                    Path to input mp4/mhas\n" );
    fprintf ( stdout, "-of <s>                    Path to output wav (WITH .wav extension!)\n" );
    fprintf ( stdout, "\n[options]:\n\n" );
    fprintf ( stdout, "-bitdepth <i>              Quantization depth of the decoder output in bits.\n" );
    fprintf ( stdout, "-use24bitChain             Reduce the internal processing chain to 24bit.\n" );
    fprintf ( stdout, "                           <default: N/A>, 32bit chain is used by default.\n" );
    fprintf ( stdout, "-targetSamplingRate <i>    Sampling rate of the decoder output in Hz.\n" );
    fprintf ( stdout, "                           Only resampling factors of 1.5, 2, 3 are allowed.\n" );
    fprintf ( stdout, "                           <default: 48000>\n" );
    fprintf ( stdout, "-disableResampling         Use core coder sampling rate for decoder output.\n" );
    fprintf ( stdout, "-mixerDomain <i>           mixerDomain options are:\n" );
    fprintf ( stdout, "                           0:\ttime domain\n" );
    fprintf ( stdout, "                           1:\tqmf  domain\n" );
    fprintf ( stdout, "                           <default: 0>\n" );
    fprintf ( stdout, "-cicpOut <i>               Output configuration options are:\n" );
    fprintf ( stdout, "                           -1:\tread geometry from file\n" );
    fprintf ( stdout, "                           0: \tinput channels = output channels\n" );
    fprintf ( stdout, "                           <index>: cicp index, e.g. 13 for 22.2, 6 for 5.1\n" );
    fprintf ( stdout, "                           <default: 0>\n" );
    fprintf ( stdout, "-outGeo <s>                text file specifying output geometry\n" );
    fprintf ( stdout, "                           (only applicable if \"-cicpOut -1\" was used)\n" );
    fprintf ( stdout, "-outGeoDev <s>             path to text file specifying the output geometry\n" );
    fprintf ( stdout, "-fDBinaural                Apply frequency domain binauralization \n" );
    fprintf ( stdout, "-tDBinaural                Apply time domain binauralization \n" );
    fprintf ( stdout, "-Bin <i>                   Binaural options:\n" );
    fprintf ( stdout, "                           0:\tread BRIR as wave files from folder\n" );
    fprintf ( stdout, "                           1:\tread BRIR from low-level parameter .bs file\n" );
    fprintf ( stdout, "                           <default: N/A>\n" );
    fprintf ( stdout, "-BRIRgeo <s>               path to text file specifying BRIR geometry \n" );
    fprintf ( stdout, "                           (only applicable if -Bin == 0)\n" );
    fprintf ( stdout, "-BRIRbsH <s>               path to output BRIR highlevel bitstream \n" );
    fprintf ( stdout, "                           (only applicable if -Bin == 0)\n" );
    fprintf ( stdout, "-BRIRbsL <s>               path to output BRIR lowlevel bitstream \n" );
    fprintf ( stdout, "                           (only applicable if -Bin == 0)\n" );
    fprintf ( stdout, "-Brir <s>                  path to BRIR representation\n" );
    fprintf ( stdout, "                           (folder including prefix/path to parameter .bs file)\n" );
    fprintf ( stdout, "-mono                      if set: single mono wav files are written\n" );
    fprintf ( stdout, "-delayAlignMpegTestpoints  Apply delay alignment for Test1-1, Test1-3, Test1-4.\n" );
    fprintf ( stdout, "                           Might not work correctly for other input\n" );
    fprintf ( stdout, "-drcInterface <s>          Path to uniDrcInterface bitstream. If this parameter\n" );
    fprintf ( stdout, "                           is present, the internal DRC module will be enabled.\n" );
    fprintf ( stdout, "-targetLoudnessLevel <f>   target loudness level of decoder output in [LKFS].\n" );
    fprintf ( stdout, "                           Note that the -drcInterface parameter has priority\n" );
    fprintf ( stdout, "                           if it signals the targetLoudness parameter.\n" );
    fprintf ( stdout, "                           <default: not present>\n" );
    fprintf ( stdout, "-drcEffectTypeRequest <i>  DRC effect request index according to Table 11 of\n" );
    fprintf ( stdout, "                           ISO/IEC 23003-4:2015 in hexadecimal format.\n" );
    fprintf ( stdout, "                           Note that the -drcInterface parameter has priority\n" );
    fprintf ( stdout, "                           if it signals the drcEffectTypeRequest parameter.\n" );
    fprintf ( stdout, "                           <default: not present>\n" );
    fprintf ( stdout, "-ei <s>                    path to Element Interaction 'bitstream' file\n" );
    fprintf ( stdout, "-ls <s>                    path to Local Setup Information 'bitstream' file\n" );
    fprintf ( stdout, "-sd <s>                    path to scene displacement 'bitstream' file, i.e.:\n" );
    fprintf ( stdout, "                             `mpegh3daSceneDisplacementData'\n" );
    fprintf ( stdout, "-sd2 <s>                   path to scene displacement 'bitstream' file, i.e.:\n" );
    fprintf ( stdout, "                             `mpegh3daSceneDisplacementData', `mpeg3daPositionalSceneDisplacementData'\n" );
    fprintf ( stdout, "-omf                       Object Metadata file\n" );
    fprintf ( stdout, "                           -omf requires also -cocfg\n" );
    fprintf ( stdout, "-cocfg                     Channel-Object Configuration file\n" );
    fprintf ( stdout, "                           -cocfg requires also -omf\n" );
    fprintf ( stdout, "-ooi                       write object data to object output interface\n" );
    fprintf ( stdout, "-emd <s>                   write earcon metadata to file\n" );
    fprintf ( stdout, "-Pr <i>                    select presetID\n" );
    fprintf ( stdout, "-sgMem <i>                 choose switch group member to render [0,N]\n" );
    fprintf ( stdout, "                           <default: not present>\n" );
    fprintf ( stdout, "-nogc                      Disable the garbage collector\n" );
    fprintf ( stdout, "-cpo <i>                   conformance point output type\n" );
    fprintf ( stdout, "                           0:\tfull 3DA decoder output\n" );
    fprintf ( stdout, "                           1:\tCpo-1 as defined in ISO/IEC 23008-9\n" );
    fprintf ( stdout, "                           <default: 0>\n" );
    fprintf ( stdout, "-deblvl <0,1>              Activate debug level: 0 (default), 1\n");
    fprintf ( stdout, "-v                         Activate verbose level 1.\n" );
    fprintf ( stdout, "-vv                        Activate verbose level 2.\n" );
    fprintf ( stdout, "\n[formats]:\n\n" );
    fprintf ( stdout, "<s>                        string\n" );
    fprintf ( stdout, "<i>                        integer number\n" );
    fprintf ( stdout, "<f>                        floating point number\n" );
    fprintf ( stdout, "\n" );
    return -1;
  }

  return 0;
}

static int GetCmdline ( int argc, char** argv )
{
  int i;
  int required = 0;
  int setTargetLayoutByLocalSetupInterface = 0;

  monoWavOut   =  0;
  BinReadBRIRType = -1;

  for ( i = 1; i < argc; ++i )
  {
    if ( !strcmp ( argv[i], "-if" ) )                             /* Required */
    {
      strncpy ( mp4_InputFile, argv[++i], FILENAME_MAX ) ;
      required++;
      continue;
    }
    else if ( !strcmp ( argv[i], "-of" ) )                        /* Required */
    {
      strncpy ( wav_OutputFile, argv[++i], FILENAME_MAX ) ;
      required++;
      continue;
    }
    else if ( !strcmp ( argv[i], "-bitdepth" ) )                  /* Optional */
    {
      int tmp = atoi ( argv[++i] );
      switch (tmp) {
        case 16:
        case 24:
        case 32:
          outputBitDepth = (BIT_DEPTH)tmp;
          break;
        default:
          outputBitDepth = BIT_DEPTH_DEFAULT;
      }
      continue;
    }
    else if ( !strcmp ( argv[i], "-use24bitChain" ) )             /* Optional */
    {
      mpeg3daBitDepth = BIT_DEPTH_24_BIT;
      continue;
    }
    else if ( !strcmp ( argv[i], "-v" ) )                         /* Optional */
    {
      verboseLevel = (verboseLevel > VERBOSE_LVL1) ? verboseLevel : VERBOSE_LVL1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-vv" ) )                        /* Optional */
    {
      verboseLevel = VERBOSE_LVL2;
      continue;
    }
    else if ( !strcmp ( argv[i], "-deblvl" ) )                    /* Optional */
    {
      int tmp = atoi ( argv[++i] );
      switch (tmp) {
        case 1:
        case 2:
          debugLevel = (DEBUG_LEVEL)tmp;
          break;
        default:
          debugLevel = DEBUG_NORMAL;
      }
      continue;
    }
    else if ( !strcmp (argv[i], "-targetSamplingRate" ) )         /* Optional */
    {
      targetSamplingRate = atoi ( argv[++i] );
      continue;
    }
    else if ( !strcmp (argv[i], "-disableResampling" ) )          /* Optional */
    {
      disableResampling = 1;
      continue;
    }
    else if ( !strcmp (argv[i], "-mixerDomain" ) )                /* Optional */
    {
      int tmp = 0;
      tmp = atoi ( argv[++i] );
      if ( tmp == 0 ) {
        mixerDomain = TIME_DOMAIN;
      }
      else if ( tmp == 1 ) {
        mixerDomain = QMF_DOMAIN;
      }
      else {
        mixerDomain = NOT_DEFINED;
      }
      continue;
    }
    else if ( !strcmp ( argv[i], "-cicpOut" ) )                   /* Optional */
    {
      outputCicp = (CICP_FORMAT)atoi ( argv[++i] );
      continue;
    }
    else if ( !strcmp ( argv[i], "-outGeo" ) )                    /* Optional */
    {
      strncpy ( fileOutGeo, argv[++i], FILENAME_MAX ) ;
      continue;
    }
    else if ( !strcmp ( argv[i], "-outGeoDev" ) )                 /* Optional */
    {
      strncpy ( fileOutGeoDeviation, argv[++i], FILENAME_MAX ) ;
      speakerMisplacement = 1;
      fileOutGeoDeviationPtr = fileOutGeoDeviation;
      continue;
    }
    else if ( !strcmp (argv[i], "-fDBinaural" ) )                 /* Optional */
    {
      fDBinaural = 1;
      continue;
    }
    else if ( !strcmp (argv[i], "-Bin" ) )                        /* Optional */
    {
      BinReadBRIRType = atoi ( argv[++i] );
      continue;
    }
    else if ( !strcmp (argv[i], "-BRIRgeo" ) )                    /* Optional, only if -Bin == 0 */
    {
      strncpy ( BinReadBRIRsGeometryFromLocation, argv[++i], FILENAME_MAX );
      BinReadBRIRsGeometryFromLocationPtr = BinReadBRIRsGeometryFromLocation;
    }
    else if ( !strcmp (argv[i], "-BRIRbsH" ) )                    /* Optional, only if -Bin == 0 */
    {
      strncpy ( WriteBRIRsBitstreamPath, argv[++i], FILENAME_MAX );
      WriteBRIRsBitstreamPathPtr = WriteBRIRsBitstreamPath;
    }
    else if ( !strcmp (argv[i], "-BRIRbsL" ) )                    /* Optional, only if -Bin == 0 */
    {
      strncpy ( WriteBRIRsLowLevelBitstreamPath, argv[++i], FILENAME_MAX );
      WriteBRIRsLowLevelBitstreamPathPtr = WriteBRIRsLowLevelBitstreamPath;
    }
    else if ( !strcmp (argv[i], "-Brir" ) )                       /* Optional */
    {
      strncpy ( BinReadBRIRsFromLocation, argv[++i], FILENAME_MAX );
      BinReadBRIRsFromLocationPtr = BinReadBRIRsFromLocation;
    }
    else if ( !strcmp (argv[i], "-tDBinaural" ) )                 /* Optional */
    {
      tDBinaural = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-mono" ) )                      /* Optional */
    {
      monoWavOut = 1;
      continue;
    }
    else if ( !strcmp (argv[i], "-delayAlignMpegTestpoints" ) )   /* Optional */
    {
      bDelayAlignMpegTestpoints = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-drcInterface" ) )              /* Optional */
    {
      strncpy ( inputFile_uniDrcInterface, argv[++i], FILENAME_MAX );
      drcParams.uniDrcInterfacePresent = 1;
      continue;
    }
    else if ( !strcmp (argv[i], "-targetLoudnessLevel" ) )        /* Optional */
    {
      drcParams.targetLoudnessLevel = (float) atof ( argv[++i] );
      if (drcParams.targetLoudnessLevel > 0.0f) {
        return -1;
      }
      drcParams.targetLoudnessLevelPresent = 1;
      continue;
    }
    else if ( !strcmp (argv[i], "-drcEffectTypeRequest" ) )       /* Optional */
    {
      drcParams.drcEffectTypeRequest = (unsigned long)strtol(argv[++i],NULL,16);
      drcParams.drcEffectTypeRequestPresent = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-ei" ) )                        /* Optional */
    {
      strncpy ( elementInteraction_InputFile, argv[++i], FILENAME_MAX ) ;
      hasElementInteraction = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-ls" ) )                        /* Optional */
    {
      strncpy ( localSetup_InputFile, argv[++i], FILENAME_MAX ) ;
      setTargetLayoutByLocalSetupInterface = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-sd" ) )                        /* Optional */
    {
      strncpy ( sceneDisplacement_InputFile, argv[++i], FILENAME_MAX ) ;
      sceneDsplInterfaceEnabled = eSceneDsplRotational;
      continue;
    }
    else if ( !strcmp ( argv[i], "-sd2" ) )                       /* Optional */
    {
        strncpy (sceneDisplacement_InputFile, argv[++i], FILENAME_MAX);
        sceneDsplInterfaceEnabled = ( eSceneDsplRotational | eSceneDsplPositional);
        continue;
    }
    else if (!strcmp(argv[i], "-omf"))	/* Optional */
    {
      strncpy ( omfFile, argv[++i], FILENAME_MAX ) ;
      continue;
    }
    else if (!strcmp(argv[i], "-cocfg"))	/* Optional */
    {
      strncpy(cocfgFile, argv[++i], FILENAME_MAX);
      continue;
    }
    else if ( !strcmp ( argv[i], "-ooi" ) )                       /* Optional */
    {
      writeToObjectOutputInterface = 1;
      continue;
    }
    else if ( !strcmp ( argv[i], "-emd" ) )                       /* Optional */
    {
        strncpy(earconMetaData_OutputFile, argv[++i], FILENAME_MAX);
        continue;
    }
    else if ( !strcmp ( argv[i], "-sgMem" ) )                     /* Optional */
    {
      chooseSGmember = atoi ( argv[++i] );
      continue;
    }
    else if ( !strcmp ( argv[i], "-Pr" ) )                        /* Optional */
    {
      selectedPresetID = atoi ( argv[++i] );
      continue;
    }
    else if ( !strcmp ( argv[i], "-nogc" ) )                      /* Optional */
    {
      recycleLevel = RECYCLE_INACTIVE;
    }
    else if ( !strcmp ( argv[i], "-cpo" ) )                       /* Optional */
    {
      int tmp = atoi ( argv[++i] );

      switch (tmp) {
      case 1:
        mpeg3daCpoType = MPEG3DA_CPO_1;
        break;
      case 128:
        mpeg3daCpoType = MPEG3DA_CORE_OUTPUT;
        break;
      case 129:
        mpeg3daCpoType = MPEG3DA_SKIP_EARCON;
        break;
      default:
        mpeg3daCpoType = MPEG3DA_DEFAULT_OUTPUT;
      }
    }
    else if ( !strcmp ( argv[i], "-outMode" ) )                   /* Optional, deprecated */
    {
      int tmp = atoi ( argv[++i] );

      switch (tmp) {
      case 1:
        mpeg3daCpoType = MPEG3DA_CORE_OUTPUT;
        break;
      default:
        mpeg3daCpoType = MPEG3DA_DEFAULT_OUTPUT;
      }
    }
  }

  if ( outputCicp == CICP_FROM_GEO ) {
    if ( fileOutGeo[0] == '\0' ) {
      fprintf( stderr, "Error: No output geometry file given!\n");
      return -1;
    }
  }

  if (setTargetLayoutByLocalSetupInterface == 0) {
    localSetup_InputFile[0] = '\0';
  }
  if (hasElementInteraction == 0) {
    elementInteraction_InputFile[0] = '\0';
  }
  if (sceneDsplInterfaceEnabled == eSceneDsplOff) {
    sceneDisplacement_InputFile[0] = '\0';
  }

  if (required != 2) {
    return -1;
  } else {
    return 0;
  }
}

/* evaluate version string */
static int getVersion_evalString(
            const char*                       versionString,
            char*                             versionSubString,
            const int                         maxSubStringLength,
            const int                         offset,
            int*                              versionNumber
) {
  int i = 0;
  int j = 0;

  for (i = 0; i < maxSubStringLength; i++) {
    versionSubString[i] = versionString[offset + j++];

    if ((versionSubString[i] < '0') || (versionSubString[i] > '9')) {
      return 1;
    }
  }

  *versionNumber = atoi(versionSubString);

  return 0;
}

/* get software version number from string */
static int getVersion(
            const char*                       versionString,
            int*                              versionMajor,
            int*                              versionMinor,
            int*                              versionRMbuild
) {
  int error         = 0;
  int i             = 0;
  int j             = 0;
  char subString[2] = {'\0'};

  error = getVersion_evalString(versionString,
                                subString,
                                2,
                                0,
                                versionMajor);
  if (0 != error) {
    return error;
  }

  error = getVersion_evalString(versionString,
                                subString,
                                2,
                                3,
                                versionMinor);
  if (0 != error) {
    return error;
  }

  error = getVersion_evalString(versionString,
                                subString,
                                2,
                                6,
                                versionRMbuild);


  return error;
}

static int getWav_wavIOwrapper( const char* pathAudioFile, unsigned int* nChannels, unsigned int* fs, unsigned int* InBytedepth, unsigned long* nTotalSamplesPerChannel ) {
  int error = 0;
  int nSamplesPerChannelFilled = 0;
  int pathLen = 0;
  char path[FILENAME_MAX+5] = {0};
  WAVIO_HANDLE hWavIO = NULL;
  FILE* fptr = NULL;

  *nChannels               = 0;
  *fs                      = 0;
  *InBytedepth             = 0;
  *nTotalSamplesPerChannel = 0;

  /* add .wav extension if missing */
  strcpy(path, pathAudioFile);
  pathLen = strlen(path);
  if (0 != strcmp(path + pathLen - 4, ".wav")) {
    strcat(path, ".wav");
  }

  /* open .wav file */
  fptr = fopen(path, "rb");

  if (NULL != fptr) {
    /* open wavIO */
    error = wavIO_init(&hWavIO, 1024, 0, 0);
    if (0 != error) {
      fprintf ( stderr, "\nError opening audio file %s\n", path );
      fclose(fptr);
      return -1;
    }

    /* read .wav file header */
    error = wavIO_openRead(hWavIO, fptr, nChannels, fs, InBytedepth, nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    if (0 != error) {
      fclose(fptr);
      return -1;
    }

    /* close wavIO again */
    error = wavIO_close(hWavIO);
    if (0 != error) {
      fprintf ( stderr, "\nError closing wavIO.\n" );
    }
  } else {
    return -1;
  }

  return error;
}

static int getWavLength( const char* pathAudioFile, unsigned long* wavlength ) {
  int error = 0;
  unsigned int nChannels = 0;
  unsigned int fs = 0;
  unsigned int InBytedepth = 0;
  unsigned long nTotalSamplesPerChannel = 0;

  *wavlength = 0;

  error = getWav_wavIOwrapper(pathAudioFile, &nChannels, &fs, &InBytedepth, &nTotalSamplesPerChannel);

  if (0 == error) {
    *wavlength = nTotalSamplesPerChannel;
  }

  return error;
}

static int getWavBitsPerSample( const char* pathAudioFile, BIT_DEPTH* bitdepth) {
  int error = 0;
  unsigned int nChannels = 0;
  unsigned int fs = 0;
  unsigned int InBytedepth = 0;
  unsigned long nTotalSamplesPerChannel = 0;

  *bitdepth = BIT_DEPTH_DEFAULT;

  error = getWav_wavIOwrapper(pathAudioFile, &nChannels, &fs, &InBytedepth, &nTotalSamplesPerChannel);

  if (0 == error) {
    switch (InBytedepth) {
      case 2:
        *bitdepth = BIT_DEPTH_16_BIT;
        break;
      case 3:
        *bitdepth = BIT_DEPTH_24_BIT;
        break;
      case 4:
        *bitdepth = BIT_DEPTH_32_BIT;
        break;
      default:
        *bitdepth = BIT_DEPTH_DEFAULT;
    }
  }

  return error;
}

static int checkExistencyAndRemoveNewlineChar ( char* string )
{
  FILE* fExists;
  int len;
  len = (int)strlen ( string );

  if ( string[len - 1] == '\n' )
  {
    string[len - 1] = '\0';
  }

  fExists = fopen ( string, "r" );

  if ( !fExists )
  {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf ( stderr, "\nBinary %s not present.\nCheck the corresponding config file entry or binary location.\n", string );
    }
    return -1;
  }

  fclose ( fExists );
  return 0;
}

static int checkBinaries_module( const char* module_description, char* module_binary )
{
  int retError = 0;

  if ( checkExistencyAndRemoveNewlineChar ( module_binary ) ) {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf ( stderr, "\nWarning: No %s support.\n", module_description);
    }
    retError = -1;
  } else {
    if (VERBOSE_LVL2 <= verboseLevel) {
      fprintf ( stderr, "\nUsing %s binary:\n %s\n", module_description, module_binary );
    }
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


static void writeBinaryDataToFile( unsigned char* dataBuffer, unsigned int numBits, const char* filename )
{
  FILE* fOut = NULL;
  fOut = fopen(filename, "wb");
  if(fOut == NULL)
  {
    fprintf(stderr, "Could not write to file %s.\n", filename);
    return;
  }
  fwrite(dataBuffer, (numBits + 7)/8, sizeof(unsigned char), fOut);
  fclose(fOut);
}

static MPEG3DA_RETURN_CODE Get_Config_StaticMetadata(char* infile,
                                                     ASCPARSERINFO* asc,
                                                     ASCPARSER_AUDIO_SCENE* audioSceneConfig,
                                                     ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET* compatibleProfileLevelSet,
                                                     int* compatibleProfileLevelSetAvailable,
                                                     ascparserUsacDecoderConfig* usacDecoderConfig,
                                                     int* audioSceneInfoAvailable,
                                                     ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig,
                                                     ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo,
                                                     int* signalGroupInfoAvailable,
                                                     ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig,
                                                     ASCPARSER_PRODUCTION_METADATA_CONFIG* productionMetadataConfig,
                                                     VERBOSE_LEVEL verboseLvl,
                                                     int* error_depth

) {
  /* Parse ASC */
  HANDLE_ASCPARSER  hAscParser;
  int error;
  int asi_available = 0;
  unsigned char loudnessInfoSet[2048];
  int loudnessInfoBitSize;

  if (NULL != audioSceneInfoAvailable) {
    *audioSceneInfoAvailable = 0;
  }

  error = ASCPARSER_Init(&hAscParser, infile);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_ASC_INIT, error_depth, "Error initializing ASC parser.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetASC(hAscParser, asc);
  if (0 != error) {
    /* return here, try decoding earcon only stream befor throwing this error */
    ASCPARSER_Close(hAscParser);
    return MPEG3DA_OK;
  }

  error = ASCPARSER_GetASI(hAscParser, asc, audioSceneConfig, &asi_available);
  if (0 != error) {
    switch (error) {
      case -9987:     /* -9987 == SIW_ASIEXISTSTWICE as defined in ascparser_stream_wrap.h */
        return PrintErrorCode(MPEG3DA_ERROR_ASI_PARSING, error_depth, "Error parsing ASI.", "ASI must not exist in PACTYP_MPEGH3DACFG and PACTYP_AUDIOSCENEINFO.", verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
        break;
      default:
        return PrintErrorCode(MPEG3DA_ERROR_ASI_PARSING, error_depth, "Error parsing ASI.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  error = ASCPARSER_GetSignalGroupConfig(hAscParser, signalGroupConfig);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_SGC_PARSING, error_depth, "Error parsing signal group config.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetSignalGroupInfo(hAscParser, signalGroupInfo, signalGroupInfoAvailable);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_SGI_PARSING, error_depth, "Error parsing signal group info.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetCompatibleProfileLevelSet(hAscParser, compatibleProfileLevelSet, compatibleProfileLevelSetAvailable);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_CPLS_PARSING, error_depth, "Error parsing compatible profile level set.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetEnhancedObjectConfig(hAscParser, enhancedObjectMetadataConfig);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_EOMC_PARSING, error_depth, "Error parsing enhanced object metadata config.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetProductionMetadataConfig(hAscParser, productionMetadataConfig);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_EOMC_PARSING, error_depth, "Error parsing enhanced object metadata config.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetDownmixConfig(hAscParser, &downmixConfig, &downmixConfigAvailable);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_ASCEXT_PARSING, error_depth, "Error parsing ConfigExtension.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (downmixConfigAvailable) {
    passiveDownmixFlag = downmixConfig.passiveDownmixFlag;

    if (passiveDownmixFlag != 0) {
      phaseAlignStrength = 0;
    } else {
      phaseAlignStrength = downmixConfig.phaseAlignStrength;
    }
    immersiveDownmixFlag = downmixConfig.immersiveDownmixFlag;
  } else {
    phaseAlignStrength = 3;
  }

  error = ASCPARSER_GetHoaConfig(hAscParser, &hoaMatrixConfig, &hoaMatrixConfigAvailable);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_HOAMTX_PARSING, error_depth, "Error parsing HOA Matrix ConfigExtension.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = ASCPARSER_GetLoudnessInfoSet(hAscParser, loudnessInfoSet, &drcParams.loudnessInfoAvailable, &loudnessInfoBitSize);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_LIS_PARSING, error_depth, "Error parsing loudness info set.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (drcParams.loudnessInfoAvailable != 0) {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf(stderr, "Loudness info set found.\n");
    }
    writeBinaryDataToFile(loudnessInfoSet, loudnessInfoBitSize, outputFile_mpegh3daLoudnessInfoSet);
  } else {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf(stderr, "No loudness info set found.\n");
    }
  }

  error = ASCPARSER_GetMaxStereoConfigIndex(hAscParser, asc);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_3DA_GENERIC, error_depth, "Error determining max stereo config index.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
  error = ASCPARSER_GetDecoderConfig(hAscParser, usacDecoderConfig);
   if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_3DA_GENERIC, error_depth, "Error getting decoder config.", NULL, verboseLvl, MPEG3DA_ERROR_POSITION_INFO);
  }
  ASCPARSER_Close(hAscParser);

  if (NULL != audioSceneInfoAvailable) {
    *audioSceneInfoAvailable = asi_available;
  }

  return MPEG3DA_OK;
}

static int checkIfFileExists ( char* string, char* suffix )
{
  char  name[FILENAME_MAX];
  FILE* fExists;

  if ( suffix != NULL )
  {
    sprintf ( name, "%s.%s", string, suffix );
  }
  else
  {
    sprintf ( name, "%s", string );
  }

  fExists = fopen ( name, "r" );

  if ( !fExists )
  {
    return 0;
  }

  fclose ( fExists );
  return 1;
}

/* checks if imag and real qmf files exist */
static int checkIfFDFilesExist(char* filename, int stftFlag){

  char tmpStringQmfReal[3 * FILENAME_MAX];
  char tmpStringQmfImag[3 * FILENAME_MAX];

  strcpy( tmpStringQmfReal, filename );
  strcat( tmpStringQmfReal, "_real");

  strcpy( tmpStringQmfImag, filename );
  strcat( tmpStringQmfImag, "_imag");

  if (stftFlag)
  {
    return ( ( checkIfFileExists ( tmpStringQmfImag, "stft" ) ) && ( checkIfFileExists ( tmpStringQmfReal, "stft" ) ) );
  }
  else
  {
    return ( ( checkIfFileExists ( tmpStringQmfImag, "qmf" ) ) && ( checkIfFileExists ( tmpStringQmfReal, "qmf" ) ) );
  }

}

static int checkInputFormat ( void )
{

  if ( asc.referenceLayout.speakerLayoutID == 0 ) {
    inputCicpReferenceLayout = (CICP_FORMAT)asc.referenceLayout.CICPspeakerLayoutIdx;
  }
  else if( asc.referenceLayout.speakerLayoutID != 3)
  {
    inputCicpReferenceLayout = CICP_FROM_GEO;
    cicp2geometry_write_geometry_to_file( outputFile_inGeoRefLayout,
                                          (asc.referenceLayout).geometryInfo,
                                          (asc.referenceLayout).numLoudspeakers, 0);
  }

  if(asc.differsFromReferenceLayout)
  {
    if ( asc.audioChannelLayout.speakerLayoutID == 0 ) {
      inputCicpAudioChannelLayout = (CICP_FORMAT)asc.audioChannelLayout.CICPspeakerLayoutIdx;
    }
    else
    {
      inputCicpAudioChannelLayout = CICP_FROM_GEO;
      cicp2geometry_write_geometry_to_file( outputFile_inGeoAudioChannelLayout,
                                            (asc.audioChannelLayout).geometryInfo,
                                            (asc.audioChannelLayout).numLoudspeakers, 0);
    }
  }
  else if( asc.referenceLayout.speakerLayoutID != 3)/* always write audioChannelLayout, unless contribution mode */
  {
    inputCicpAudioChannelLayout = inputCicpReferenceLayout;

    if ( asc.referenceLayout.speakerLayoutID != 0 ) {
      cicp2geometry_write_geometry_to_file( outputFile_inGeoAudioChannelLayout,
                                            (asc.referenceLayout).geometryInfo,
                                            (asc.referenceLayout).numLoudspeakers, 0);
    }
  }

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  if (asc.saocDmxLayoutPresent) {
    if ( asc.saocDmxChannelLayout.speakerLayoutID == 0 ) {
      inputCicpSaocDmxChannelLayout = (CICP_FORMAT)asc.saocDmxChannelLayout.CICPspeakerLayoutIdx;
    }
    else if( asc.referenceLayout.speakerLayoutID != 3)
    {
      inputCicpSaocDmxChannelLayout = CICP_FROM_GEO;
      cicp2geometry_write_geometry_to_file( outputFile_inGeoSaocDmxLayout,
                                            (asc.saocDmxChannelLayout).geometryInfo,
                                            (asc.saocDmxChannelLayout).numLoudspeakers, 0);
    }
  }
#endif

  if (asc.asc_isValid == 1 && asc.usacSamplingFreq != targetSamplingRate && disableResampling == 0) {
    int validResampling = asc.usacSamplingFreq * 1.5 == targetSamplingRate ||
                          asc.usacSamplingFreq * 2   == targetSamplingRate ||
                          asc.usacSamplingFreq * 3   == targetSamplingRate;

    if (!validResampling) {
      fprintf ( stderr, "Error: Invalid sampling rate ratio of coreCoder (%i Hz) mixer (%i Hz). Allowed are ratios of 1.5, 2, and 3. \n\n", asc.usacSamplingFreq, targetSamplingRate );
      return -1;
    }
    else {
      if (asc.usacSamplingFreq * 1.5 == targetSamplingRate) {
        resamplingRatio = 1.5f;
      }
      if (asc.usacSamplingFreq * 2   == targetSamplingRate) {
        resamplingRatio = 2.0f;
      }
      if (asc.usacSamplingFreq * 3   == targetSamplingRate) {
        resamplingRatio = 3.0f;
      }

      bResamplerActive = 1;

      if (VERBOSE_LVL1 <= verboseLevel) {
        fprintf ( stderr, "\nInfo: coreCoder sampling rate (%i Hz) differs from mixer (%i Hz).\nResampling activated!\n", asc.usacSamplingFreq, targetSamplingRate );
      }
    }

  }
  else {
    bResamplerActive = 0;
    resamplingRatio = 1.f;
  }

  return 0;
}

static int checkInputType ( void )
{
  inputType = INPUT_INVALID;

  if ( asc.channels > 0 )
  {
    inputType |= INPUT_CHANNEL;
  }

  if ( asc.objects > 0 )
  {
    inputType |= INPUT_OBJECT;
  }

  if ( asc.SAOCTransportChannels > 0 )
  {
    inputType |= INPUT_SAOC;
  }

  if ( asc.HOATransportChannels > 0 )
  {
    inputType |= INPUT_HOA;
  }

  if ( asc.earcon > 0 )
  {
    inputType |= INPUT_EARCON;
  }

  if ( inputType == INPUT_INVALID )
  {
    fprintf ( stderr, " Error checking input type\n" );
    fprintf ( stderr, " Trying to decode mhm\n" );
    return -1;
  }

  return 0;
}

static int createMixMatrix( VERBOSE_LEVEL verboseLvl )
{
  char tmpString[3 * FILENAME_MAX];
  char callFormatConverter[3 * FILENAME_MAX];
  int pas, aes;
  int dmxMode = 4;

  if ( profile == PROFILE_LOW_COMPLEXITY || profile == PROFILE_BASELINE ) {
    dmxMode = 6;
  }

  if( passiveDownmixFlag != 0 )
  {
    pas = 0;
    aes = 0;
  } else {
    pas = phaseAlignStrength;
    aes = 7;
  }
#if IAR
  if (VERBOSE_LVL1 <= verboseLvl) {
    if ( immersiveDownmixFlag == 1 )
    {
      fprintf (stderr, "Immersive format converter chosen\n" );
    }
    else
    {
      fprintf (stderr, "Generic format converter chosen\n" );
    }
  }
#else
  if( immersiveDownmixFlag == 1 && ( (outputCicp == 5) || (outputCicp == 6) ) )
  {
    fprintf(stderr, "Immersive downmix not yet provided, will perform casual downmix.\n");
  }
#endif

  if (4 == dmxMode) {
    sprintf ( callFormatConverter, "%s -mixMtxOutput %s -cicpIn %d -cicpOut %d -dmx %d -aes %d -pas %d ", binary_FormatConverter, "tmpFile3Ddec_mix_matrix_out.txt", inputCicpAudioChannelLayout, outputCicp, dmxMode, aes, pas);
  } else {
    sprintf ( callFormatConverter, "%s -mixMtxOutput %s -cicpIn %d -cicpOut %d -dmx %d -aes %d ", binary_FormatConverter, "tmpFile3Ddec_mix_matrix_out.txt", inputCicpAudioChannelLayout, outputCicp, dmxMode, aes);
  }

  if (inputCicpAudioChannelLayout == CICP_FROM_GEO ) {
    sprintf ( tmpString, " -inGeo %s ", outputFile_inGeoAudioChannelLayout);
    strcat ( callFormatConverter, tmpString );
  }
  if (outputCicp == CICP_FROM_GEO ) {
    sprintf ( tmpString, " -outGeo %s ", fileOutGeo );
    strcat ( callFormatConverter, tmpString );
  }

  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callFormatConverter, tmpString );

  if (0 != callBinary(callFormatConverter, "Format Converter", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static MPEG3DA_RETURN_CODE getAudioPreRollInfo(const char* audioPreRollFilename, HAUDIOPREROLL_INFO audioPreRollInfo)
{
  MPEG3DA_RETURN_CODE error = MPEG3DA_OK;
  FILE* aprInfoFile         = NULL;

  audioPreRollInfo->nSamplesCoreCoderPreRoll = 0;
  audioPreRollInfo->numPrerollAU             = 0;

  aprInfoFile = fopen(audioPreRollFilename, "rb");

  if (NULL != aprInfoFile) {
    fread(&audioPreRollInfo->nSamplesCoreCoderPreRoll, sizeof(int), 1, aprInfoFile);
    fread(&audioPreRollInfo->numPrerollAU,             sizeof(int), 1, aprInfoFile);
    fclose(aprInfoFile);
  } else {
    error = MPEG3DA_ERROR_APR_INIT;
  }

  return error;
}

static MPEG3DA_RETURN_CODE getAudioTruncationInfo(const char* audioTruncationInfoFilename, HAUDIOTRUNC_INFO audioTruncationInfo)
{
  MPEG3DA_RETURN_CODE error = MPEG3DA_OK;
  int nATI                  = 0;
  int nValues               = 0;
  int data[4]               = {0};
  FILE* atiInfoFile         = NULL;

  audioTruncationInfo->isActive_left       = 0;
  audioTruncationInfo->isActive_right      = 0;
  audioTruncationInfo->nTruncSamples_left  = 0;
  audioTruncationInfo->nTruncSamples_right = 0;

  atiInfoFile = fopen(audioTruncationInfoFilename, "rb");

  if (NULL != atiInfoFile) {
    while (fread(&nValues, sizeof(int), 1, atiInfoFile)) {
      if (4 == nValues) {
        fread(data, sizeof(int), nValues, atiInfoFile);

        /* data[0] = isActive       */
        /* data[1] = reserved       */
        /* data[2] = truncFromBegin */
        /* data[3] = nTruncSamples  */

        if (1 == data[0]) {
          if (1 == data[2]) {
            audioTruncationInfo->isActive_left      = data[0];
            audioTruncationInfo->nTruncSamples_left = data[3];
          } else {
            audioTruncationInfo->isActive_right      = data[0];
            audioTruncationInfo->nTruncSamples_right = data[3];
          }
        }

        nATI++;
      } else {
        error = MPEG3DA_ERROR_ATI_PARSING;
      }
    }

    if (nATI > 2) {
      /* only two truncation points are supported simultaneously: one in the first frame and one in the last frame */
      error = MPEG3DA_ERROR_ATI_GENERIC;
    }

    fclose(atiInfoFile);
  } else {
    error = MPEG3DA_ERROR_ATI_INIT;
  }

  return error;
}

static MPEG3DA_RETURN_CODE execEarconDecoder(int* earconPresent, const MPEG3DA_CONFPOINT mpeg3daCpoType) {
  MPEG3DA_RETURN_CODE mpeg3da_error      = MPEG3DA_OK;
  int error                              = 0;
  
  *earconPresent                         = 0;

  if ((1 == bEarconDecoderBinaryPresent) && (mpeg3daCpoType == MPEG3DA_DEFAULT_OUTPUT)) {
    char callEarconDecoder[3 * FILENAME_MAX] = {0};
    char tmpString[3 * FILENAME_MAX]         = {0};
    char earconDataFilename[FILENAME_MAX]    = "tmpFile3Ddec_earconData.bit";
    char const *earconMetaOutFile = strlen(earconMetaData_OutputFile) ? earconMetaData_OutputFile : earconDataFilename;

    strcat(outputFile_earconDecoder, ".wav");

    sprintf ( callEarconDecoder, "%s -if %s -of %s -bs %s ", binary_earconDecoder, mp4_InputFile, outputFile_earconDecoder, earconMetaOutFile );

    sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
    strcat ( callEarconDecoder, tmpString );

    error = callBinary(callEarconDecoder, "earcon decoder", verboseLevel, 0);
    if (0 != error) {
      if (14 == error) {
        return MPEG3DA_ERROR_EARCON_FILE_FORMAT_MP4;
      } else
      if (6 != error) {
        return MPEG3DA_ERROR_EARCON_GENERIC;
      } else {
        return MPEG3DA_OK;
      }
    } else {
      FILE* fptr = fopen(outputFile_earconDecoder, "rb");
      if (fptr) {
        *earconPresent = 1;
        fclose(fptr);
      }
    }
  }

  return mpeg3da_error;
}

static MPEG3DA_RETURN_CODE execProdMetadataDecoder(void)
{
  MPEG3DA_RETURN_CODE mpeg3da_error     = MPEG3DA_OK;
  FILE* tmpFile = fopen(pmcFilename, "rb");

  if (!tmpFile) {
    return MPEG3DA_OK;
  } else {
    fclose(tmpFile);
  }

  if ( omfFile[0] && cocfgFile[0] ) {
    int error                             = 0;
    int ccfg = 0;
    int idx = 0;
    char callPmdDecoder[3 * FILENAME_MAX] = {0};
    char tmpString[3 * FILENAME_MAX]      = {0};
    char ocfg[FILENAME_MAX] = {'\0'};
    FILE* fptr = fopen(cocfgFile, "rb");

    if (!fptr) {
      return MPEG3DA_ERROR_CORE_GENERIC;
    }

    fscanf(fptr, "%d", &ccfg);
    while (!feof(fptr) && idx < sizeof(ocfg)) {
      char tmp = '\0';
      fscanf(fptr, "%c", &tmp);

      if (tmp >= '0' && tmp <= '9' || tmp == ' ') {
        ocfg[idx++] = tmp;
      }
    }

    sprintf(callPmdDecoder, "%s -ipmc %s -iomf %s -ccfg %d -ocfg %s", binary_prodmetadataDecoder, pmcFilename, omfFile, ccfg, ocfg);

    sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
    strcat ( callPmdDecoder, tmpString );

    error = callBinary(callPmdDecoder, "production metadata parser", verboseLevel, 0);
    if (0 != error) {
      mpeg3da_error = MPEG3DA_ERROR_CORE_GENERIC;
    }
  }

  return mpeg3da_error;
}

static MPEG3DA_RETURN_CODE execCoreDecoder(HCORE_DECODER_PROPERTIES coreDecoderProperties, const MPEG3DA_CONFPOINT mpeg3daCpoType)
{
  MPEG3DA_RETURN_CODE mpeg3da_error      = MPEG3DA_OK;
  int error                              = 0;
  char callCoreDecoder[3 * FILENAME_MAX] = {0};
  char tmpString[3 * FILENAME_MAX]       = {0};
  int drcDataFlag                        = 0;
  int discardAudioPreRoll                = (MPEG3DA_CORE_OUTPUT == mpeg3daCpoType) ? 1 : 0;
  int discardAudioTruncationInfo         = (MPEG3DA_CORE_OUTPUT == mpeg3daCpoType) ? 1 : 0;
  BIT_DEPTH CoreBitDepth                 = BIT_DEPTH_DEFAULT;
  char aprFilename[FILENAME_MAX]         = "tmpFile3Ddec_apr.bit";
  char atiFilename[FILENAME_MAX]         = "tmpFile3Ddec_ati.bit";
#if IAR
  char iarFilename[FILENAME_MAX]         = "tmpFile3Ddec_fmt.txt";
#endif
#ifdef RM6_INTERNAL_CHANNEL
  char icLayoutFilename[FILENAME_MAX]    = "tmpFile3Ddec_icLayout.txt";

  if ( inputCicpAudioChannelLayout != CICP_FROM_GEO && outputCicp == 2 && profile != PROFILE_LOW_COMPLEXITY )
  {
    sprintf ( callCoreDecoder, "%s -if %s -of %s -bitDepth %d -qmfauto -writeElstToFile -noElst -icFile %s -outStereo 1 ", binary_CoreDecoder, mp4_InputFile, outputFile_CoreDecoder, (int)mpeg3daBitDepth, icLayoutFilename );
  }
  else
  {
    sprintf ( callCoreDecoder, "%s -if %s -of %s -bitDepth %d -qmfauto -writeElstToFile -noElst -icFile %s -outStereo 0 ", binary_CoreDecoder, mp4_InputFile, outputFile_CoreDecoder, (int)mpeg3daBitDepth, icLayoutFilename );
  }
#else
  {
    sprintf ( callCoreDecoder, "%s -if %s -of %s -bitDepth %d -qmfauto -writeElstToFile -noElst -deblvl %d ", binary_CoreDecoder, mp4_InputFile, outputFile_CoreDecoder, (int)mpeg3daBitDepth, (int)debugLevel );
  }
#endif

  /* Append object metadata output file */
  if ( ( asc.objects > 0 ) || ( asc.SAOCTransportChannels > 0 ) )
  {
    strcat ( callCoreDecoder, "-objFile tmpFile3Ddec_obj_0.dat " );
  }

  /* Append saoc metadata output file */
  if ( asc.saocHeaderLength )
  {
    strcat ( callCoreDecoder, "-saocFile tmpFile3Ddec_saocData.bs " );
  }

  if ( asc.HOATransportChannels > 0 )
  {
    strcat ( callCoreDecoder, "-hoaFile tmpFile3Ddec_hoaData.bs " );
  }

  if ( inputType & INPUT_CHANNEL )
  {
    if( asc.referenceLayout.speakerLayoutID != 3 ) /* not contribution mode */
    {
      if ( CompareInOutGeometry( inputCicpAudioChannelLayout, outputCicp, outputFile_inGeoAudioChannelLayout, fileOutGeo ) )
      {
        strcat ( callCoreDecoder, "-mixMatrix tmpFile3Ddec_mix_matrix_out.txt " );
      }
    }
#if IAR
    if ( bFormatConverter && immersiveDownmixFlag ) {
      char tmpString[3 * FILENAME_MAX];
      sprintf(tmpString, " -fcFile %s ", iarFilename);
      strcat(callCoreDecoder, tmpString);
    }
#endif
  }

  if (0 == discardAudioPreRoll) {
    sprintf ( tmpString, "-aprFile %s ", aprFilename);
    strcat ( callCoreDecoder, tmpString );
  }

  if (0 == discardAudioTruncationInfo) {
    sprintf ( tmpString, "-atiFile %s ", atiFilename);
    strcat ( callCoreDecoder, tmpString );
  }

  sprintf ( tmpString, "-pmcFile %s ", pmcFilename);
  strcat ( callCoreDecoder, tmpString );

  sprintf ( tmpString, "-drcFile %s ", outputFile_unDrcGain);
  strcat ( callCoreDecoder, tmpString );

  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callCoreDecoder, tmpString );

  error = callBinary(callCoreDecoder, "core decoder", verboseLevel, 0);
  if (0 != error) {
    switch (error) {
      case -665:          /* -665 = MPEG3DACORE_ERROR_INVALID_STREAM */
        return MPEG3DA_ERROR_CORE_INVALID_STREAM;
        break;
      case -41:           /* -41 = MPEG3DACORE_ERROR_WAVIO_CLIPPED */
        return MPEG3DA_ERROR_CORE_CLIPPING;
        break;
      default:
        return MPEG3DA_ERROR_CORE_GENERIC;
    }
  }

  {
    char grepString[3 * FILENAME_MAX] = {'\0'};
    int grepResult = 0;
    if (bUseWindowsCommands) {
      sprintf(grepString, "%s %s %s", "findstr", "\!\!WARNING\!\! ", logFileNames[0]);
    } else {
      sprintf(grepString, "%s %s %s", "grep", "\!\!WARNING\!\! ", logFileNames[0]);
    }
    grepResult = system(grepString);

    if (0 == grepResult) {
      fprintf(stderr, "\n!!WARNINGS where discovered while calling:\n!!  %s\n!!\n!!  Please see %s for more details (requires -v option)!\n\n", callCoreDecoder, logFileNames[0]);
    } else {
      if (grepResult == -1) {
        fprintf(stderr, "\n!Error starting command %s: %s", grepString, strerror(errno));
      } else {
#  if defined(_POSIX_C_SOURCE) || (defined(__APPLE__) && defined(__MACH__))
        grepResult = WEXITSTATUS(grepResult);
#  endif /* system() is waitpid(2) based */
        if (grepResult != 1) {
          fprintf(stderr, "\n!Error running command %s: %s", grepString, strerror(errno));
        }
      }
    }
  }

  if ( asc.sbrRatioIndex == 0 ) {
    /* time domain output: determine core decoder bit depth */
    if (0 != getWavBitsPerSample(outputFile_CoreDecoder, &CoreBitDepth)) {
      return MPEG3DA_ERROR_3DA_GENERIC;
    }
    if (CoreBitDepth != mpeg3daBitDepth) {
      /* input bit depth differs from output bit depth */
      return MPEG3DA_ERROR_3DA_BITDEPTH;
    }
  }

  if (NULL != coreDecoderProperties) {
    if (0 == discardAudioPreRoll) {
      mpeg3da_error = getAudioPreRollInfo(aprFilename, &coreDecoderProperties->apr);

      if (MPEG3DA_OK != mpeg3da_error) {
        return mpeg3da_error;
      }
    }

    if (0 == discardAudioTruncationInfo) {
      mpeg3da_error = getAudioTruncationInfo(atiFilename, &coreDecoderProperties->ati);

      if (MPEG3DA_OK != mpeg3da_error) {
        return mpeg3da_error;
      }
    }
  }

  drcDataFlag = checkIfFileExists(outputFile_unDrcGain, NULL);
  if (drcDataFlag == 1) {
    drcParams.drcDataAvailable = 1;
  } else {
    unsigned char bitBuf[4];
    FILE* pOutFileDummyDrcConfig;
    pOutFileDummyDrcConfig = fopen(outputFile_mpegh3daUniDrcConfig, "wb");
    bitBuf[0] = 0; bitBuf[1] = 0; bitBuf[2] = 0; bitBuf[3] = 0;
    fwrite(bitBuf, sizeof(unsigned char), 4, pOutFileDummyDrcConfig);
    fclose(pOutFileDummyDrcConfig);
  }

  return MPEG3DA_OK;
}

static int execSeparator ( char* infile, char* outfile, int offs, int numChannels )
{
  char callSeparator[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];
#ifdef RM6_INTERNAL_CHANNEL
  char icLayoutFilename[256]	= "tmpFile3Ddec_icLayout.txt";

  int icFileExists = checkIfFileExists("tmpFile3Ddec_icLayout", "txt");
  int bOnlyChannelContent = asc.channels               > 0 &&
                            asc.HOATransportChannels  == 0 &&
                            asc.SAOCTransportChannels == 0 &&
                            asc.objects               == 0;

  if ( inputCicpAudioChannelLayout != CICP_FROM_GEO && outputCicp == 2 && icFileExists && bOnlyChannelContent )
  {
    FILE * fp;
    char bufCh[256];
    int tempNumChannels = 0;

    fp  = fopen ( icLayoutFilename, "rt" );
    if ( fp != 0 ) {
      fgets ( bufCh, 255, fp );
      tempNumChannels = atoi(bufCh);
      fclose (fp);
    }
    if (numChannels <= tempNumChannels)
    {
      numChannels = tempNumChannels;
    }
  }
#endif

  sprintf ( callSeparator, "%s -if %s -of %s -nOff %d -nCh %d", binary_Separator, infile, outfile, offs, numChannels );
  sprintf ( tmpString, " >> %s 2>&1",  logFileNames[0]);
  strcat ( callSeparator, tmpString );

  if (0 != callBinary(callSeparator, "Channel and Object Separator", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int execMetadataDecoder ( int hasIndepConf )
{
  char callObjectMetadataDecoder[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  if ( asc.saocHeaderLength )
  {
    int saocDmxChannels = 0; /* Number of SAOC Downmix Channels */
    int saocDmxObjects = 0;  /* Number of SAOC Downmix Objects */

    /* Get number of objects from the SAOC bitstream */
    asc.objects = saoc_GetNumber_of_OAM_Objects ( "tmpFile3Ddec_saocData.bs", &saocDmxChannels, &saocDmxObjects, asc.saocHeaderLength );

    if ( asc.objects <= 0 )
    {
      if (saocDmxChannels == 0)
      {
        /* no SAOC Objects and no SAOC Channels are present */
        fprintf ( stderr, "Error parsing SAOC bitstream file\n" );
        return -1;
      }

      /* no SAOC Objects are present, only SAOC Channels */
      fprintf ( stderr, "SAOC bitstream file contains only Channel based Content\n" );
      return 0;
    }
  }

  if (hasIndepConf == 1)
  {
    sprintf ( callObjectMetadataDecoder, "%s -if tmpFile3Ddec_obj -of tmpFile3Ddec_separator_object.oam -nobj %d -oamBs %d -coreBs %d -ld %d -oamVersion 4 -hasDynamicObjectPriority %d -hasUniformSpread %d -indepConf 1",
              binary_ObjectMetadataDecoder, asc.objects, asc.oam_length, asc.outputFrameLength, asc.oam_mode, asc.oam_hasDynamicObjectPriority, asc.oam_hasUniformSpread );
  }
  else
  {
    sprintf ( callObjectMetadataDecoder, "%s -if tmpFile3Ddec_obj -of tmpFile3Ddec_separator_object.oam -nobj %d -oamBs %d -coreBs %d -ld %d -oamVersion 4 -hasDynamicObjectPriority %d -hasUniformSpread %d",
          binary_ObjectMetadataDecoder, asc.objects, asc.oam_length, asc.outputFrameLength, asc.oam_mode, asc.oam_hasDynamicObjectPriority, asc.oam_hasUniformSpread );

  }
  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callObjectMetadataDecoder, tmpString );

  if (0 != callBinary(callObjectMetadataDecoder, "Object Metadata decoder", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int changeDelayOfFile_End( char* inputFilename, const int nSamplesOfDelay )
{
  /* cut delay at end of file */
  char* wavExtension  = ".wav";
  int nWavExtensionChars = strspn(wavExtension,inputFilename);
  int bQmfInput = 0;
  unsigned int newFilelength = 0;
  char outputFilename[] = "tmpFile3Ddec_delay_aligned_end.wav";
  FILE *pInFileName = 0;
  FILE *pOutFileName = 0;
  WAVIO_HANDLE hWavIO_in  = 0;
  WAVIO_HANDLE hWavIO_out = 0;
  unsigned int i = 0;
  char temp_char[3*FILENAME_MAX];

  float** inBuffer = NULL;

  int error = 0;
  unsigned int nInChannels  = 0;
  unsigned int InSampleRate = 0;
  unsigned int InBytedepth  = 0;
  unsigned long nTotalSamplesPerChannel = 0;
  unsigned long nTotalSamplesWrittenPerChannel = 0;
  int nSamplesPerChannelFilled = 0;

  unsigned long nTotalSamplesReadPerChannel = 0;
  unsigned int isLastFrame = 0;

  int nSamplesFramelength = 2048;

  int sample = 0;
  int channel = 0;
  int frame = 0;


  /* qmf domain check */
  if ( nWavExtensionChars != 4 ) {
    bQmfInput = 1;
  }

  pInFileName  = fopen(inputFilename, "rb");
  pOutFileName = fopen(outputFilename, "wb");

  if ( pInFileName == NULL) {
    fprintf(stderr, "Error opening input file.\n");
    return -1;
  }

  if ( pOutFileName == NULL) {
    fprintf(stderr, "Error opening output file.\n");
    return -1;
  }

  error = wavIO_init(&hWavIO_in, nSamplesFramelength, 0, 0);
  if ( 0 != error ) {
    fprintf(stderr, "Error during initialization of wavIO.\n");
    return -1;
  }

  error = wavIO_init(&hWavIO_out, nSamplesFramelength, 0, 0);
  if ( 0 != error ) {
    fprintf(stderr, "Error during initialization of wavIO.\n");
    return -1;
  }

  error = wavIO_openRead(hWavIO_in, pInFileName, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
  if ( 0 != error ) {
   fprintf(stderr, "Error opening input qmf imag file\n");
   return -1;
  }
  if ((bQmfInput == 1) && (nTotalSamplesPerChannel % 64))
  {
    wavIO_close(hWavIO_in);
    wavIO_close(hWavIO_out);
    fprintf(stderr, "Error: You can only have file lengths that are multiples of the number of qmfbands (i.e. n*64).\n");
    return -1;
  }


  error = wavIO_openWrite(hWavIO_out, pOutFileName, nInChannels, InSampleRate, InBytedepth);
  if ( 0 != error ) {
    fprintf(stderr, "Error opening output qmf\n");
    return -1;
  }

  /* new file length */
  newFilelength = nTotalSamplesPerChannel - nSamplesOfDelay;

  /* allocate buffers */
  inBuffer  = (float**) calloc(nInChannels, sizeof(float*));

  for (i = 0; i < nInChannels; ++i ) {
    inBuffer[i]  = (float*)calloc( nSamplesFramelength, sizeof(float));
  }

  do  /*loop over all frames*/
  {
    unsigned int  nSamplesReadPerChannel = 0;
    unsigned int  nSamplesToWritePerChannel = 0;
    unsigned int  nSamplesWrittenPerChannel = 0;
    unsigned int  nZerosPaddedBeginning = 0;
    unsigned int  nZerosPaddedEnd = 0;
    unsigned int  nSamplesTemp = 0;
    /* read frame */
    if ( !isLastFrame ) {
      wavIO_readFrame(hWavIO_in, inBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
      nSamplesToWritePerChannel      = nSamplesReadPerChannel + nZerosPaddedBeginning;
      nTotalSamplesReadPerChannel   += nSamplesReadPerChannel;
    }
    nSamplesTemp = nTotalSamplesWrittenPerChannel + nSamplesToWritePerChannel;

    /* write frame if new file length is not exceeded */
    if ( nSamplesTemp > newFilelength )
    {
      nSamplesToWritePerChannel = newFilelength - nTotalSamplesWrittenPerChannel;
    }

    if (nSamplesToWritePerChannel > 0)
    {
      wavIO_writeFrame(hWavIO_out, inBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
      nTotalSamplesWrittenPerChannel += nSamplesWrittenPerChannel;
    }
  }  while ( !isLastFrame );

  wavIO_updateWavHeader(hWavIO_out, &nTotalSamplesWrittenPerChannel);
  wavIO_close(hWavIO_in);
  wavIO_close(hWavIO_out);

  for (i=0; i< nInChannels; i++)
  {
    free(inBuffer[i]);
  }
  free(inBuffer);


  /* delete input file */
  if( bUseWindowsCommands ) {
    sprintf(temp_char, "del %s 2> NUL", inputFilename);
    system ( temp_char );
  }
  else {
    sprintf(temp_char, "rm -f %s", inputFilename);
    system ( temp_char );
  }

  /* rename created output file to input filename */
  if( bUseWindowsCommands ) {
    sprintf ( temp_char, "move %s %s > NUL", outputFilename, inputFilename );
    system ( temp_char );
  }
  else {
    sprintf ( temp_char, "mv %s %s", outputFilename, inputFilename );
    system ( temp_char );
  }

  return 0;

}

static int useWindowsCommands( void )
{
  int retVal = 1;

#if defined __linux__ || defined __APPLE__
  retVal = 0;
#else
  FILE *fp;
  char path[1035];

  /* Open the command for reading. */
  fp = _popen("uname -o >> win_commands.txt 2>&1", "r");
  if (! (fp == NULL)) {
    if(fgets(path, sizeof(path)-1, fp) != NULL ) {
      if(!strncmp(path,"Cygwin",6)){
        retVal = 0;
      }
    }
  }
  _pclose(fp);

  if (retVal) {
    system("del win_commands.txt 2>  NUL");
  }
  else {
    system("rm -f win_commands.txt");
  }
#endif

  return retVal;
}

/* positive nSamplesOfDelay = add delay, negative nSamplesOfDelay = cut delay from wave file */
static int changeDelayOfFile( char* inputFilename, char* outputFilename, const int nSamplesOfDelay )
{
  char callWavcutter[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  if ( inputFilename == NULL || outputFilename == NULL ) {
    fprintf ( stderr, "Error in changeDelayOfFile: no valid input/outputFilename given!\n\n");
    return -1;
  }

  if (VERBOSE_LVL1 <= verboseLevel) {
    if ( nSamplesOfDelay == 0 ) {
      fprintf ( stderr, "Warning: No delay to remove from file %s. Copying to output file %s. \n\n", inputFilename, outputFilename );
    }
  }

  sprintf ( callWavcutter, "%s -if %s -of %s -delay %i ", binary_WavCutter, inputFilename, outputFilename, (-1)*(nSamplesOfDelay) );
  sprintf(tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat(callWavcutter, tmpString);

  if (0 != callBinary(callWavcutter, "WavCutter", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int execSampleRateConverter ( char * input, char * output )
{
  int retError = 0;
  char callResampler[3 * FILENAME_MAX];
  char tmpFilename[FILENAME_MAX] = "tmpFile3Ddec_undelayedResampledFile.wav";
  unsigned int nSamplesResamplerDelay = 256;
  char tmpString[3 * FILENAME_MAX];

  sprintf ( callResampler, "%s -s %i %s %s", binary_ResampAudio, targetSamplingRate, input, tmpFilename);
  sprintf(tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat(callResampler, tmpString);

  retError = callBinary(callResampler, "Resampler", verboseLevel, 0);
  if (0 != retError) {
    return retError;
  }

  retError = changeDelayOfFile( tmpFilename, output, nSamplesResamplerDelay );

  return retError;
}

static int execMixer ( char* in1, char* in2, char* out, int delayIn1, int delayIn2, int delayOut )
{
  char callMixer[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  int in1WavExt = checkIfWavExtension( in1 );
  int in2WavExt = checkIfWavExtension( in2 );
  int outWavExt = checkIfWavExtension( out );

  PROCESSING_DOMAIN bCoreCoder = TIME_DOMAIN;

  if ( in1WavExt != in2WavExt  || in1WavExt != outWavExt ) {
    fprintf ( stderr, "\n\n Error at mixer: mixed domains!\n" );
    return -1;
  }

  if ( !in1WavExt && !in2WavExt && !outWavExt){
    bCoreCoder = QMF_DOMAIN;
  }

  if ( bCoreCoder == QMF_DOMAIN ) {

    if ( ( !checkIfFDFilesExist ( in1, 0 ) ) ){
      fprintf ( stderr, "\n\nError in execMixer. File: %s does not exist\n", in1 );
      return -1;
    }

    if ( !checkIfFDFilesExist ( in2, 0 ) ) {  /* if second file doesn't exist create it ! */
      sprintf ( callMixer, "%s -if1 %s -if2 %s -of %s -d1 %d -d2 %d -d3 %d -g2 0", binary_Mixer, in1, in1, in2, delayIn1, delayIn1 , delayOut ); 
    }
    else {
      
      if ( !strcmp ( in2, out ) )
      {
        sprintf ( tmpString, "%s_2_", in2 );
      
        if( bUseWindowsCommands ) {
          sprintf ( callMixer, "move %s_real.qmf %s_real.qmf > NUL", in2, tmpString );
          if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
            return -1;
          }

          sprintf ( callMixer, "move %s_imag.qmf %s_imag.qmf > NUL", in2, tmpString );
          if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
            return -1;
          }
        }
        else {
          sprintf ( callMixer, "mv %s_real.qmf %s_real.qmf", in2, tmpString );
          if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
            return -1;
          }

          sprintf ( callMixer, "mv %s_imag.qmf %s_imag.qmf", in2, tmpString );
          if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
            return -1;
          }
        }
        in2 = tmpString;
      }

      sprintf ( callMixer, "%s -if1 %s -if2 %s -of %s -d1 %d -d2 %d -d3 %d ", binary_Mixer, in1, in2, out, delayIn1, delayIn2 , delayOut );
    }

  }
  else {

    if ( ( !checkIfFileExists ( in1, NULL ) ) ) {
      return -1;
    }

    if ( !checkIfFileExists ( in2, NULL ) ) { /* if second file doesn't exist create it ! */
      sprintf ( callMixer, "%s -if1 %s -if2 %s -of %s -d1 %d -d2 %d -d3 %d -g2 0", binary_Mixer, in1, in1, in2, delayIn1, delayIn1 , delayOut ); 
    }
    else {
      
      if ( !strcmp ( in2, out ) )
      {
        sprintf ( tmpString, "%s_", in2 );
      
        if( bUseWindowsCommands ) {
          sprintf ( callMixer, "move %s %s > NUL", in2, tmpString );
        }
        else {
          sprintf ( callMixer, "mv %s %s", in2, tmpString );
        }

        if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
          return -1;
        }

        in2 = tmpString;
      }

      sprintf ( callMixer, "%s -if1 %s -if2 %s -of %s -d1 %d -d2 %d -d3 %d ", binary_Mixer, in1, in2, out, delayIn1, delayIn2 , delayOut );
    }
  }

  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callMixer, tmpString );

  if (0 != callBinary(callMixer, "Mixer", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}


static int setBinaries ( char* globalBinariesPath, int* nBinaries )
{
  int retError = 0;
  int nBinaries_tmp = 0;

  /***********************************************
               binary_CoreDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[0][MPEG3DA_DESCRIPTION], mpeg3da_binaries[0][MPEG3DA_EXE], binary_CoreDecoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_ObjectMetadataDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[1][MPEG3DA_DESCRIPTION], mpeg3da_binaries[1][MPEG3DA_EXE], binary_ObjectMetadataDecoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_SaocDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[2][MPEG3DA_DESCRIPTION], mpeg3da_binaries[2][MPEG3DA_EXE], binary_SaocDecoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_HoaDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[3][MPEG3DA_DESCRIPTION], mpeg3da_binaries[3][MPEG3DA_EXE], binary_HoaDecoder);
  if (0 != retError) {
    if (VERBOSE_LVL2 <= verboseLevel) {
#ifdef _WIN32
      fprintf ( stderr, "Warning: HOA only available for VS2012!\n" );
#else
      fprintf ( stderr, "Warning: HOA only available for c++11!\n" );
#endif
    }
    retError = 0;
  } else {
    bHoaBinaryPresent = 1;
    nBinaries_tmp++;
  }

  /***********************************************
              binary_HoaMatrixDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[24][MPEG3DA_DESCRIPTION], mpeg3da_binaries[24][MPEG3DA_EXE], binary_HoaMatrixDecoder);
  if (0 != retError) {
    if (VERBOSE_LVL2 <= verboseLevel) {
#ifdef _WIN32
      fprintf ( stderr, "Warning: HOA only available for VS2012!\n" );
#else
      fprintf ( stderr, "Warning: HOA only available for c++11!\n" );
#endif
    }
    retError = 0;
  } else {
    bHoaMatrixBinaryPresent = 1;
    nBinaries_tmp++;
  }

  /***********************************************
               binary_FormatConverter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[4][MPEG3DA_DESCRIPTION], mpeg3da_binaries[4][MPEG3DA_EXE], binary_FormatConverter);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_IarFormatConverter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[5][MPEG3DA_DESCRIPTION], mpeg3da_binaries[5][MPEG3DA_EXE], binary_IarFormatConverter);
  if (0 != retError) {
    if (VERBOSE_LVL2 <= verboseLevel) {
#ifdef _WIN32
      fprintf ( stderr, "Warning: IarFormatConverter only available for VS2012!\n" );
#else
      fprintf ( stderr, "Warning: IarFormatConverter only available for c++11!\n" );
#endif
    }
    retError = 0;
  } else {
    bIarBinaryPresent = 1;
    nBinaries_tmp++;
  }

  /***********************************************
               binary_Separator
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[6][MPEG3DA_DESCRIPTION], mpeg3da_binaries[6][MPEG3DA_EXE], binary_Separator);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_Renderer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[7][MPEG3DA_DESCRIPTION], mpeg3da_binaries[7][MPEG3DA_EXE], binary_Renderer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_Mixer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[8][MPEG3DA_DESCRIPTION], mpeg3da_binaries[8][MPEG3DA_EXE], binary_Mixer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_WavM2N
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[9][MPEG3DA_DESCRIPTION], mpeg3da_binaries[9][MPEG3DA_EXE], binary_WavM2N);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_FdBinauralRenderer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[10][MPEG3DA_DESCRIPTION], mpeg3da_binaries[10][MPEG3DA_EXE], binary_FdBinauralRenderer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_TdBinauralRenderer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[11][MPEG3DA_DESCRIPTION], mpeg3da_binaries[11][MPEG3DA_EXE], binary_TdBinauralRenderer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_FdBinauralParam
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[12][MPEG3DA_DESCRIPTION], mpeg3da_binaries[12][MPEG3DA_EXE], binary_FdBinauralParam);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_TdBinauralParam
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[13][MPEG3DA_DESCRIPTION], mpeg3da_binaries[13][MPEG3DA_EXE], binary_TdBinauralParam);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_FdBinauralBitstreamWriter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[14][MPEG3DA_DESCRIPTION], mpeg3da_binaries[14][MPEG3DA_EXE], binary_FdBinauralBitstreamWriter);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_DomainSwitcher
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[15][MPEG3DA_DESCRIPTION], mpeg3da_binaries[15][MPEG3DA_EXE], binary_DomainSwitcher);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_WavCutter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[16][MPEG3DA_DESCRIPTION], mpeg3da_binaries[16][MPEG3DA_EXE], binary_WavCutter);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_DmxMatrixDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[17][MPEG3DA_DESCRIPTION], mpeg3da_binaries[17][MPEG3DA_EXE], binary_DmxMatrixDecoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_ResampAudio
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[18][MPEG3DA_DESCRIPTION], mpeg3da_binaries[18][MPEG3DA_EXE], binary_ResampAudio);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_DrcSelection
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[19][MPEG3DA_DESCRIPTION], mpeg3da_binaries[19][MPEG3DA_EXE], binary_DrcSelection);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_DrcDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[20][MPEG3DA_DESCRIPTION], mpeg3da_binaries[20][MPEG3DA_EXE], binary_DrcDecoder);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_LoudnessNormalizer
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[21][MPEG3DA_DESCRIPTION], mpeg3da_binaries[21][MPEG3DA_EXE], binary_LoudnessNormalizer);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
     binary_localSetupInformationInterface
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[22][MPEG3DA_DESCRIPTION], mpeg3da_binaries[22][MPEG3DA_EXE], binary_LocalSetupInformationInterface);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_PeakLimiter
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[23][MPEG3DA_DESCRIPTION], mpeg3da_binaries[23][MPEG3DA_EXE], binary_PeakLimiter);
  if (0 != retError) { return retError; }
  nBinaries_tmp++;

  /***********************************************
               binary_earconDecoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[25][MPEG3DA_DESCRIPTION], mpeg3da_binaries[25][MPEG3DA_EXE], binary_earconDecoder);
  if (0 != retError) {
    if (VERBOSE_LVL2 <= verboseLevel) {
#ifdef _WIN32
      fprintf ( stderr, "Warning: earcon decoder only available for VS2012!\n" );
#endif
    }
    retError = 0;
  } else {
    bEarconDecoderBinaryPresent = 1;
    nBinaries_tmp++;
  }
  /***********************************************
               binary_prodmetadataEncoder
  ***********************************************/
  retError = setBinaries_module(globalBinariesPath, mpeg3da_binaries[26][MPEG3DA_DESCRIPTION], mpeg3da_binaries[26][MPEG3DA_EXE], binary_prodmetadataDecoder);
  if (0 != retError) {
    if (VERBOSE_LVL2 <= verboseLevel) {
#ifdef _WIN32
      fprintf ( stderr, "Warning: production metadata decoder only available for VS2012!\n" );
#endif
    }
    retError = 0;
  } else {
    bProdmetadataDecoderBinaryPresent = 1;
    nBinaries_tmp++;
  }

  *nBinaries = nBinaries_tmp;

  return retError;
}

static void removeTempFiles ( RECYCLE_LEVEL removeFiles )
{
  if (RECYCLE_ACTIVE == removeFiles) {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf ( stdout, "Deleting intermediate files...\n" );
    }

    if (VERBOSE_NONE == verboseLevel) {
      char delString[3 * FILENAME_MAX] = {'\0'};;
      if (bUseWindowsCommands) {
        sprintf(delString, "%s %s %s", "del ", logFileNames[0], " 2>  NUL");
      } else {
        sprintf(delString, "%s %s %s", "rm -f ", logFileNames[0], " ");
      }
      system(delString);
    }

    if (bUseWindowsCommands) {
      system ( "del tmpFile3Ddec*.* 2>  NUL" );
      system ( "del *_tmpFile3Ddec*.* 2>  NUL" );
      system ( "del config_tmpFile3Ddec*.* 2>  NUL" );
    } else {
      system ( "rm -f tmpFile3Ddec*.*" );
      system ( "rm -f *_tmpFile3Ddec*.*" );
      system ( "rm -f config_tmpFile3Ddec*.*" );
    }
  } else {
    if (VERBOSE_LVL1 <= verboseLevel) {
      fprintf ( stdout, "Keeping intermediate files!\n" );
    }
  }
}

static MPEG3DA_RETURN_CODE copyToOutputFile(char* inputFileName, char* outputFileName, const int forceCopy, const MPEG3DA_CONFPOINT mpeg3daCpoType, const BIT_DEPTH bitdepth, const int delay, const int truncation)
{
  int error                        = 0;
  int truncatedSamples             = truncation;
  char callCmd[3 * FILENAME_MAX]   = {0};
  char tmpString[3 * FILENAME_MAX] = {0};

  /* generate single mono files with geometry naming */
  if ( monoWavOut && !forceCopy ) {

    removeWavExtension(inputFileName);
    removeWavExtension(outputFileName);

    if ( outputCicp == -1 ) {

      /* geometry from file */
      if ( fileOutGeo[0] != '\0' ) {
        sprintf ( callCmd, "%s -if %s -of %s -cicpIn -1 -inGeo %s -dir 1", binary_WavM2N, inputFileName, outputFileName, fileOutGeo );
      }
      else {
        sprintf ( callCmd, "%s -if %s -of %s -cicpIn -1 -inGeo %s -dir 1", binary_WavM2N, inputFileName, outputFileName, outputFile_inGeoRefLayout );
      }
    }
    else {
      /* geometry from cicp index */
      sprintf ( callCmd, "%s -if %s -of %s -cicpIn %d -dir 1", binary_WavM2N, inputFileName, outputFileName, outputCicp );
    }

    sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
    strcat ( callCmd, tmpString );

    if (0 != callBinary(callCmd, "WavM2N", verboseLevel, 0)) {
      return MPEG3DA_ERROR_3DA_GENERIC;
    }
  }
  else {
    /* AudioTruncatioInfo() can only be applied if the core sampling frequency equals 48000 Hz */
    if ((targetSamplingRate != asc.usacSamplingFreq) && (1 == disableResampling)) {
      if (VERBOSE_NONE < verboseLevel) {
        fprintf ( stdout, "Warning: Audio Truncation is only applied for 48000 Hz output samplerate.\n\n" );
      } else {
        fprintf ( stdout, "\nWarning: Audio Truncation is only applied for 48000 Hz output samplerate.\n\n" );
      }
      truncatedSamples = 0;
    }

    if ((BIT_DEPTH_DEFAULT == bitdepth) || (MPEG3DA_DEFAULT_OUTPUT == mpeg3daCpoType)) {
      sprintf ( callCmd, "%s -if %s -of %s -delay %d -truncate %d -deblvl %d", binary_WavCutter, inputFileName, outputFileName, delay, truncatedSamples, (int)debugLevel );
    } else {
      sprintf ( callCmd, "%s -if %s -of %s -delay %d -truncate %d -bitdepth %d -deblvl %d", binary_WavCutter, inputFileName, outputFileName, delay, truncatedSamples, (int)bitdepth, (int)debugLevel );
    }

    sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
    strcat ( callCmd, tmpString );

    error = callBinary(callCmd, "WavCutter", verboseLevel, 0);
    if (0 != error) {
      switch (error) {
        case -2:            /* can not open output file */
          return MPEG3DA_ERROR_WAVCUTTER_OUTPUTFILE;
          break;
        case -41:           /* -41 = WAVIO_ERROR_CLIPPED */
          return MPEG3DA_ERROR_WAVCUTTER_CLIPPING;
          break;
        default:
          return MPEG3DA_ERROR_CORE_GENERIC;
      }
    }
  }

  return MPEG3DA_OK;
}

static int MakeLogFileNames(
            char                             *wav_OutputFile,
            const int                         bUseWindowsCommands,
            const VERBOSE_LEVEL               verboseLevel
) {
  char tmp[3 * FILENAME_MAX]  = {"\0"};
  char tmp2[3 * FILENAME_MAX] = {"\0"};

  if (VERBOSE_LVL1 <= verboseLevel) {
    strncpy(tmp2, wav_OutputFile, strlen(wav_OutputFile) - 4);
    tmp2[strlen(wav_OutputFile) - 4] = '\0';

    strcpy(tmp, logFileNames[0]);
    sprintf(logFileNames[0], "%s_%s", tmp2, tmp);
    strcpy(tmp, logFileNames[INPUT_CHANNEL]);
    sprintf(logFileNames[INPUT_CHANNEL], "%s_%s", tmp2, tmp);
    strcpy(tmp, logFileNames[INPUT_OBJECT]);
    sprintf(logFileNames[INPUT_OBJECT], "%s_%s", tmp2, tmp);
    strcpy(tmp, logFileNames[INPUT_SAOC]);
    sprintf(logFileNames[INPUT_SAOC], "%s_%s", tmp2, tmp);
    strcpy(tmp, logFileNames[INPUT_HOA]);
    sprintf(logFileNames[INPUT_HOA], "%s_%s", tmp2, tmp);
  } else {
    if (1 == bUseWindowsCommands) {
      strncpy(tmp2, wav_OutputFile, strlen(wav_OutputFile) - 4);
      tmp2[strlen(wav_OutputFile) - 4] = '\0';

      strcpy(tmp, logFileNames[0]);
      sprintf(logFileNames[0], "%s_%s", tmp2, tmp);
      sprintf(logFileNames[INPUT_CHANNEL], "nul");
      sprintf(logFileNames[INPUT_OBJECT], "nul");
      sprintf(logFileNames[INPUT_SAOC], "nul");
      sprintf(logFileNames[INPUT_HOA], "nul");
    } else {
      strncpy(tmp2, wav_OutputFile, strlen(wav_OutputFile) - 4);
      tmp2[strlen(wav_OutputFile) - 4] = '\0';

      strcpy(tmp, logFileNames[0]);
      sprintf(logFileNames[0], "%s_%s", tmp2, tmp);
      sprintf(logFileNames[INPUT_CHANNEL], "/dev/null");
      sprintf(logFileNames[INPUT_OBJECT], "/dev/null");
      sprintf(logFileNames[INPUT_SAOC], "/dev/null");
      sprintf(logFileNames[INPUT_HOA], "/dev/null");
    }
  }

  return 0;
}

static int setDrcParams( int mode )
{
  unsigned int i;
  int error = 0;
  CICP2GEOMETRY_CHANNEL_GEOMETRY dummy[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numLFEs;

  switch (mode) {
    case 1:
      /* set */
      drcParams.profile                     = profile;
      drcParams.binaryDrcDecoder            = binary_DrcDecoder;
      drcParams.bsMpegh3daUniDrcConfig      = outputFile_mpegh3daUniDrcConfig;
      drcParams.bsMpegh3daLoudnessInfoSet   = outputFile_mpegh3daLoudnessInfoSet;
      drcParams.bsUnDrcGain                 = outputFile_unDrcGain;
      drcParams.txtDrcSelectionTransferData = outputFile_drcSelectionTransferData;
      drcParams.txtMpegh3daParams           = outputFile_drcSelection_mpegh3daParams;
      drcParams.txtDownmixMatrixSet         = outputFile_drcDecoder_downmixMatrixSet;
      drcParams.drcChannelOffsetChannelPath = 0;
      drcParams.drcChannelOffsetObjectPath  = asc.channels;
      drcParams.drcChannelOffsetSaocPath    = asc.channels + asc.objects;
      drcParams.drcChannelOffsetHoaPath     = asc.channels + asc.objects + 0; 
      drcParams.framesize                   = asc.outputFrameLength;
      drcParams.downmixId                   = downmixId;
      if (downmixId != -1) {
        drcParams.downmixIdPresent = 1;
      } else {
        drcParams.downmixIdPresent = 0;
      }
      drcParams.downmixConfigAvailable      = downmixConfigAvailable;
      if (downmixConfigAvailable) {
        drcParams.numDownmixIdsDownmixConfig = downmixConfig.downmixIdCount;
        for (i=0; i<downmixConfig.downmixIdCount; i++) {
          drcParams.downmixIdDownmixConfig[i] = downmixConfig.downmixMatrix[i].DMX_ID;
          error = cicp2geometry_get_geometry_from_cicp( downmixConfig.downmixMatrix[i].CICPspeakerLayoutIdx, dummy, &drcParams.targetChannelCountForDownmixId[i], &numLFEs);
          drcParams.targetChannelCountForDownmixId[i] += numLFEs;
          if ( error ) { return -1; }
        }
      }
      
      /*drcParams.numGroupIdsRequested = 0;
      drcParams.groupIdRequested[0] = xxx;
      drcParams.numGroupPresetIdsRequested = 0;
      drcParams.groupPresetIdRequested[0] = xxx;
      drcParams.groupPresetIdRequestedPreference = -1;*/
      break;
    case 0:
    default:
      drcParams.loudnessInfoAvailable = 0;
      drcParams.drcDataAvailable = 0;
      drcParams.drcDecoderState = 0;
      drcParams.uniDrcInterfacePresent = 0;
      drcParams.targetLoudnessLevelPresent = 0;
      drcParams.drcEffectTypeRequestPresent = 0;
      drcParams.drcEffectTypeRequest = 0;
      drcParams.downmixIdPresent = 0;
      drcParams.downmixId = -1;
      drcParams.downmixConfigAvailable = 0;
      drcParams.numDownmixIdsDownmixConfig = 0;
      drcParams.numGroupIdsRequested = 0;
      drcParams.numGroupPresetIdsRequested = 0;
      drcParams.groupPresetIdRequestedPreference = -1;
      break;
  }
  return 0;
}

static int writeDrcSelectionMpegh3daParams( void )
{
  int i;
  FILE *pOutFileDrcSelectionMpegh3daParams = NULL;

  /* open file */
  pOutFileDrcSelectionMpegh3daParams = fopen(outputFile_drcSelection_mpegh3daParams, "wb");

  /* groupIds */
  fprintf(pOutFileDrcSelectionMpegh3daParams,"%d\n", drcParams.numGroupIdsRequested);
  for( i =0 ; i < drcParams.numGroupIdsRequested; i++)
  {
    fprintf(pOutFileDrcSelectionMpegh3daParams,"%d\n", drcParams.groupIdRequested[i]);
  }

  /* groupPresetIds */
  fprintf(pOutFileDrcSelectionMpegh3daParams,"%d\n", drcParams.numGroupPresetIdsRequested);
  for( i =0 ; i < drcParams.numGroupPresetIdsRequested; i++)
  {
   fprintf(pOutFileDrcSelectionMpegh3daParams,"%d %d\n", drcParams.groupPresetIdRequested[i], drcParams.numMembersGroupPresetIdsRequested[i]);
  }
  /* groupPresetIdRequestedPreference */
  fprintf(pOutFileDrcSelectionMpegh3daParams,"%d\n", drcParams.groupPresetIdRequestedPreference);

  /* close file */
  fclose(pOutFileDrcSelectionMpegh3daParams);

  return 0;
}

static int writeDrcDecoderDownmixMatrixSet( void )
{
  int i;
  FILE *pOutFileDrcDecoderDownmixMatrixSet = NULL;

  /* open file */
  pOutFileDrcDecoderDownmixMatrixSet = fopen(outputFile_drcDecoder_downmixMatrixSet, "wb");

  /* downmixIds and targetChannelCounts */
  fprintf(pOutFileDrcDecoderDownmixMatrixSet,"%d\n", drcParams.numDownmixIdsDownmixConfig);
  for( i =0 ; i < drcParams.numDownmixIdsDownmixConfig; i++)
  {
    fprintf(pOutFileDrcDecoderDownmixMatrixSet,"%d %d\n", drcParams.downmixIdDownmixConfig[i], drcParams.targetChannelCountForDownmixId[i]);
  }

  /* close file */
  fclose(pOutFileDrcDecoderDownmixMatrixSet);

  return 0;
}

static int execDrcSelection( void )
{
  int error = 0, i = 0, numDrcEffectTypeRequests = 0, drcEffectTypeRequested = 0;
  int drcEffectTypeRequest[8] = {0};
  unsigned long tmp;
  char callSelection[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  error = writeDrcDecoderDownmixMatrixSet();
  if ( error ) { return -1; }
  error = writeDrcSelectionMpegh3daParams();
  if ( error ) { return -1; }

  sprintf ( callSelection, "%s -if %s -il %s -of %s -multiBandDrcPresent %s -v 0 ", binary_DrcSelection, outputFile_mpegh3daUniDrcConfig, outputFile_mpegh3daLoudnessInfoSet, outputFile_drcSelectionTransferData, outputFile_multiBandDrcPresent);

  if (drcParams.downmixIdPresent) {
    sprintf( tmpString, "-id %d ", drcParams.downmixId);
    strcat(  callSelection, tmpString);
  }

  sprintf( tmpString, "-mpegh3daParams %s ", outputFile_drcSelection_mpegh3daParams);
  strcat(  callSelection, tmpString);

  sprintf( tmpString, "-dms %s ", outputFile_drcDecoder_downmixMatrixSet);
  strcat(  callSelection, tmpString);

  if (drcParams.uniDrcInterfacePresent) {
    sprintf( tmpString, "-ii %s ", inputFile_uniDrcInterface);
    strcat(  callSelection, tmpString);
  }

  if (drcParams.targetLoudnessLevelPresent) {
    sprintf( tmpString, "-tl %f ", drcParams.targetLoudnessLevel);
    strcat(  callSelection, tmpString);
  }

  if (drcParams.drcEffectTypeRequestPresent) {
    sprintf( tmpString, "-drcEffect ");
    strcat(  callSelection, tmpString);
    tmp = drcParams.drcEffectTypeRequest;
    for (i=0; i<8; i++) {
      drcEffectTypeRequested = tmp & 15;
      if (i == 0 || drcEffectTypeRequested != 0) {
        numDrcEffectTypeRequests += 1;
        drcEffectTypeRequest[i] = drcEffectTypeRequested;
      } else {
        break;
      }
      tmp >>= 4;
    }

    /* recommended DRC fallback effect according to ISO/IEC 23003-4:2015, Annex E.2.2 */
    if (numDrcEffectTypeRequests == 1 && drcEffectTypeRequest[0] > 0 && drcEffectTypeRequest[0] < 7) {
      numDrcEffectTypeRequests = 6;
      switch (drcEffectTypeRequest[0]) {
        case 1:
          drcEffectTypeRequest[1] = 6;
          drcEffectTypeRequest[2] = 2;
          drcEffectTypeRequest[3] = 3;
          drcEffectTypeRequest[4] = 4;
          drcEffectTypeRequest[5] = 5;
          break;
        case 2:
          drcEffectTypeRequest[1] = 6;
          drcEffectTypeRequest[2] = 1;
          drcEffectTypeRequest[3] = 3;
          drcEffectTypeRequest[4] = 4;
          drcEffectTypeRequest[5] = 5;
          break;
        case 3:
          drcEffectTypeRequest[1] = 6;
          drcEffectTypeRequest[2] = 1;
          drcEffectTypeRequest[3] = 2;
          drcEffectTypeRequest[4] = 4;
          drcEffectTypeRequest[5] = 5;
          break;
        case 4:
          drcEffectTypeRequest[1] = 6;
          drcEffectTypeRequest[2] = 2;
          drcEffectTypeRequest[3] = 1;
          drcEffectTypeRequest[4] = 3;
          drcEffectTypeRequest[5] = 5;
          break;
        case 5:
          drcEffectTypeRequest[1] = 6;
          drcEffectTypeRequest[2] = 1;
          drcEffectTypeRequest[3] = 2;
          drcEffectTypeRequest[4] = 3;
          drcEffectTypeRequest[5] = 4;
          break;
        case 6:
          drcEffectTypeRequest[1] = 1;
          drcEffectTypeRequest[2] = 2;
          drcEffectTypeRequest[3] = 3;
          drcEffectTypeRequest[4] = 4;
          drcEffectTypeRequest[5] = 5;
          break;
      }
    }

    for (i=numDrcEffectTypeRequests; i>0; i--) {
      sprintf( tmpString, "%d",drcEffectTypeRequest[i-1]);
      strcat(  callSelection, tmpString);
    }
    sprintf( tmpString, " ");
    strcat(  callSelection, tmpString);
  } else {
    sprintf( tmpString, "-drcEffect 0 ");
    strcat(  callSelection, tmpString);
  }

  if (0 != callBinary(callSelection, "DRC Set Selection", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int execDrc2( int framesize, char* audioIn, char* audioOut, PROCESSING_DOMAIN processingDomain )
{
  char callDrc2[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];


  if(processingDomain == QMF_DOMAIN)
  {
    sprintf( callDrc2, "%s -if %s -of %s -afs %i ", binary_DrcDecoder, audioIn, audioOut, framesize);
    strcat(  callDrc2, "-decId 2 -gdt 1 ");
  }
  if(processingDomain == STFT_DOMAIN)
  {
    sprintf( callDrc2, "%s -if %s -of %s -afs %i ", binary_DrcDecoder, audioIn, audioOut, framesize);
    strcat(  callDrc2, "-decId 2 -gdt 2 ");
  }
  else if(processingDomain == TIME_DOMAIN)
  {
    sprintf( callDrc2, "%s -if %s -of %s -afs %i ", binary_DrcDecoder, audioIn, audioOut, framesize);
    strcat(  callDrc2, "-decId 2 -gdt 0 ");
  }

  sprintf(tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", outputFile_mpegh3daUniDrcConfig, outputFile_mpegh3daLoudnessInfoSet, outputFile_unDrcGain, outputFile_drcSelectionTransferData);

  strcat(callDrc2, tmpString);
  if (drcParams.profile == PROFILE_LOW_COMPLEXITY || drcParams.profile == PROFILE_BASELINE) {
    sprintf( tmpString, "-gd %d ",rendererPathDelays.maxDelay);
    strcat(  callDrc2, tmpString);
  } else {
    
  }
  sprintf( tmpString, "-dms %s ", outputFile_drcDecoder_downmixMatrixSet);
  strcat(  callDrc2, tmpString);

  if (0 != callBinary(callDrc2, "DRC-2 decoder", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int execDrc3( int framesize, char* audioIn, char* audioOut )
{
  char callDrc3[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];
    
  
    
  sprintf(callDrc3, "%s -if %s -of %s -afs %i ", binary_DrcDecoder, audioIn, audioOut, framesize);
  sprintf(tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", outputFile_mpegh3daUniDrcConfig, outputFile_mpegh3daLoudnessInfoSet, outputFile_unDrcGain, outputFile_drcSelectionTransferData);

  strcat(callDrc3, tmpString);
  strcat(callDrc3, "-decId 3 -gdt 0 ");
  if (drcParams.profile == PROFILE_LOW_COMPLEXITY || drcParams.profile == PROFILE_BASELINE) {
    sprintf( tmpString, "-gd %d ",rendererPathDelays.maxDelay);
    strcat(  callDrc3, tmpString);
  } else {
    
  }
  sprintf( tmpString, "-dms %s ", outputFile_drcDecoder_downmixMatrixSet);
  strcat(  callDrc3, tmpString);

  if (0 != callBinary(callDrc3, "DRC-3 decoder", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static int execLoudnessNorm( char* audioIn, char* audioOut, PROCESSING_DOMAIN processingDomain )
{
  char callLoudnessNorm[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  if (processingDomain == TIME_DOMAIN) {
    sprintf ( callLoudnessNorm, "%s -if %s -of %s -selProcData %s ", binary_LoudnessNormalizer, audioIn, audioOut, outputFile_drcSelectionTransferData);
  } else if (processingDomain == QMF_DOMAIN) {
    sprintf ( callLoudnessNorm, "%s -if %s_real.qmf -of %s_real.qmf -selProcData %s ", binary_LoudnessNormalizer, audioIn, audioOut, outputFile_drcSelectionTransferData);
    sprintf ( callLoudnessNorm, "%s -if %s_imag.qmf -of %s_imag.qmf -selProcData %s ", binary_LoudnessNormalizer, audioIn, audioOut, outputFile_drcSelectionTransferData);
  } else if (processingDomain == STFT_DOMAIN) {
    sprintf ( callLoudnessNorm, "%s -if %s_real.stft -of %s_real.stft -selProcData %s ", binary_LoudnessNormalizer, audioIn, audioOut, outputFile_drcSelectionTransferData);
    sprintf ( callLoudnessNorm, "%s -if %s_imag.stft -of %s_imag.stft -selProcData %s ", binary_LoudnessNormalizer, audioIn, audioOut, outputFile_drcSelectionTransferData);
  }
  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callLoudnessNorm, tmpString );

  if (0 != callBinary(callLoudnessNorm, "Loudness Normalizer", verboseLevel, 0)) {
    return -1;
  }

  return 0;
}

static MPEG3DA_RETURN_CODE execPeakLimiter( char* audioIn, char* audioOut, const BIT_DEPTH bitdepth )
{
  char callPeakLimiter[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];
  BIT_DEPTH PrePeakLimiterBitDepth = BIT_DEPTH_DEFAULT;

  if (0 != getWavBitsPerSample(audioIn, &PrePeakLimiterBitDepth)) {
    return MPEG3DA_ERROR_3DA_GENERIC;
  }
  if (PrePeakLimiterBitDepth != mpeg3daBitDepth) {
    /* input bit depth differs from processing chain bit depth */
    return MPEG3DA_ERROR_3DA_BITDEPTH;
  }

  if (BIT_DEPTH_DEFAULT == bitdepth) {
    sprintf ( callPeakLimiter, "%s -if %s -of %s -bitdepth %d -d ", binary_PeakLimiter, audioIn, audioOut, 24);             
  } else {
    sprintf ( callPeakLimiter, "%s -if %s -of %s -bitdepth %d -d ", binary_PeakLimiter, audioIn, audioOut, (int)bitdepth);  
  }

  sprintf ( tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat ( callPeakLimiter, tmpString );

  if (0 != callBinary(callPeakLimiter, "Peak Limiter", verboseLevel, 0)) {
    return MPEG3DA_ERROR_3DA_GENERIC;
  }

  return MPEG3DA_OK;
}

static int readDrcMultiBandPresentFromFile( int* drcSetIsPresent, int* drcIsMultiBand )
{
    FILE* fileHandle;
    char line[512] = {0};
    int col = 0;
    int lineCount;
    char* filename = outputFile_multiBandDrcPresent;

    /* open file */
    fileHandle = fopen(filename, "r");
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open multiband present file\n");
        return -1;
    } else {
        fprintf(stderr, "\nFound multiband present file: %s.\n", filename );
    }

    /* Get new line */
    lineCount = 0;
    fprintf(stderr, "Reading multiband present  file ... \n");
    while ( fgets(line, 511, fileHandle) != NULL )
    {
        int i = 0;
        char* pChar = line;

        /* Add newline at end of line (for eof line), terminate string after newline */
        line[strlen(line)+1] = '\0';
        line[strlen(line)] = '\n';

        /* Replace all white spaces with new lines for easier parsing */
        while ( pChar[i] != '\0')
        {
            if ( pChar[i] == ' ' || pChar[i] == '\t')
                pChar[i] = '\n';
            i++;
        }

        pChar = line;

        col = 0;

        /* Parse line */
        while ( (*pChar) != '\0')
        {
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

            if (lineCount == 1){
                if(col == 0)
                {
                    drcSetIsPresent[0] = atoi(pChar);
                }
                else if(col == 1)
                {
                    drcIsMultiBand[0] = atoi(pChar);
                }
            }
            else if (lineCount == 2){
                if(col == 0)
                {
                    drcSetIsPresent[1] = atoi(pChar);
                }
                else if(col == 1)
                {
                    drcIsMultiBand[1] = atoi(pChar);
                }
            }
            else if (lineCount == 3){
                if(col == 0)
                {
                    drcSetIsPresent[2] = atoi(pChar);
                }
                else if(col == 1)
                {
                    drcIsMultiBand[2] = atoi(pChar);
                }
            }
            /* Jump over parsed float value */
            while (  (*pChar) != '\n' )
                pChar++;

            /* Jump over new lines */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

            col++;
        }
        lineCount++;
    }

    fclose(fileHandle);
    return 0;
}

static int getDecoderProperties( void ) {

  int error = 0;

  int drcSetIsPresent[3] = { 0, 0, 0 };
  int drcIsMultiBand[3]  = { 0, 0, 0 };

  if (drcParams.drcDecoderState) {
    readDrcMultiBandPresentFromFile(drcSetIsPresent, drcIsMultiBand);
    if (drcParams.profile == PROFILE_LOW_COMPLEXITY || drcParams.profile == PROFILE_BASELINE) {
      drcIsMultiBand[1] = 0;
    }
  }

  if ( asc.channels > 0 ) {
    decoderProperties.bCoreChannels = 1;
  }
  else {
    decoderProperties.bCoreChannels = 0;
  }


  if ( asc.sbrRatioIndex == 0 ) {
    decoderProperties.bCoreHasSbr = 0;
  }
  else {
    decoderProperties.bCoreHasSbr = 1;
  }

  if ( asc.SAOCTransportChannels > 0 ) {
    decoderProperties.renderingComponents.bSaocRendering = 1;
  }
  else {
    decoderProperties.renderingComponents.bSaocRendering = 0;
  }

  if ( asc.objects > 0 ) {
    decoderProperties.renderingComponents.bObjectRendering = 1;
  }
  else {
    decoderProperties.renderingComponents.bObjectRendering = 0;
  }

  if ( asc.HOATransportChannels > 0 ) {
    decoderProperties.renderingComponents.bHoaRendering = 1;
  }
  else {
    decoderProperties.renderingComponents.bHoaRendering = 0;
  }

  if ( drcSetIsPresent[0] > 0 ) {
    if ( drcIsMultiBand[0] > 0 ) {
      decoderProperties.renderingComponents.modeDrc1Channels = MULTIBAND;
      decoderProperties.renderingComponents.modeDrc1Objects = MULTIBAND;
      decoderProperties.renderingComponents.modeDrc1Saoc = DISABLED; 
      decoderProperties.renderingComponents.modeDrc1Hoa = MULTIBAND;
    }
    else{
      decoderProperties.renderingComponents.modeDrc1Channels = SINGLEBAND;
      decoderProperties.renderingComponents.modeDrc1Objects = SINGLEBAND;
      decoderProperties.renderingComponents.modeDrc1Saoc = DISABLED; 
      decoderProperties.renderingComponents.modeDrc1Hoa = SINGLEBAND;
    }
  }
  else {
    decoderProperties.renderingComponents.modeDrc1Channels = DISABLED;
    decoderProperties.renderingComponents.modeDrc1Objects = DISABLED;
    decoderProperties.renderingComponents.modeDrc1Saoc = DISABLED;
    decoderProperties.renderingComponents.modeDrc1Hoa = DISABLED;
  }

  if ( bFormatConverter ) {
    decoderProperties.renderingComponents.bFormatConversionChannels = 1;
  }
  else {
    decoderProperties.renderingComponents.bFormatConversionChannels = 0;
  }

  if ( drcSetIsPresent[1] > 0 ) {
    if ( drcIsMultiBand[1] > 0 ) {
      decoderProperties.postProcessingComponents.modeDrc2Channels = MULTIBAND;
    }
    else{
      decoderProperties.postProcessingComponents.modeDrc2Channels = SINGLEBAND;
    }
  }
  else {
    decoderProperties.postProcessingComponents.modeDrc2Channels = DISABLED;
  }

  if ( fDBinaural ) {
    decoderProperties.postProcessingComponents.bFdBinauralizer = 1;
    /* set DRC mode */
    if ( drcSetIsPresent[1] > 0 ) {
      if ( drcIsMultiBand[1] > 0 ) {
        decoderProperties.postProcessingComponents.modeDrc2Binaural = MULTIBAND;
      }
      else{
        decoderProperties.postProcessingComponents.modeDrc2Binaural = SINGLEBAND;
      }
    }
    else{
      decoderProperties.postProcessingComponents.modeDrc2Binaural = DISABLED;
    }
  }
  else {
    decoderProperties.postProcessingComponents.bFdBinauralizer = 0;
    decoderProperties.postProcessingComponents.modeDrc2Binaural = DISABLED;
  }

  if ( tDBinaural ) {
    decoderProperties.postProcessingComponents.bTdBinauralizer = 1;
    /* set DRC mode */
    if ( drcSetIsPresent[1] > 0 ) {
      if ( drcIsMultiBand[1] > 0 ) {
        decoderProperties.postProcessingComponents.modeDrc2Binaural = MULTIBAND;
      }
      else{
        decoderProperties.postProcessingComponents.modeDrc2Binaural = SINGLEBAND;
      }
    }
    else{
      decoderProperties.postProcessingComponents.modeDrc2Binaural = DISABLED;
    }
  }
  else {
    decoderProperties.postProcessingComponents.bTdBinauralizer = 0;
    decoderProperties.postProcessingComponents.modeDrc2Binaural = DISABLED;
  }


  if ( ! (inputType & INPUT_CHANNEL) &&
       ! (inputType & INPUT_OBJECT) &&
       ! (inputType & INPUT_SAOC) &&
       ( inputType & INPUT_HOA ) &&
       tDBinaural ) {
    decoderProperties.postProcessingComponents.bHoaBinauralizer = 1;
  }
  else {
    decoderProperties.postProcessingComponents.bHoaBinauralizer = 0;
  }
  
    
  /* DRC 3 */
  if ( drcSetIsPresent[2] > 0 ) {
    decoderProperties.endOfChainComponents.bsHasDrc3 = 1;
  }
  else {
    decoderProperties.endOfChainComponents.bsHasDrc3 = 0;
  }

  /* loudness normalizer */
  if ( drcParams.lnState || drcParams.drcDecoderState ) {
    decoderProperties.endOfChainComponents.bLoudnessNormalization = 1;
  }
  else {
    decoderProperties.endOfChainComponents.bLoudnessNormalization = 0;
  }

  /* peak limiter */
  decoderProperties.endOfChainComponents.bPeakLimiter = 1; /* peak limiter enabled by default in RM7 */

  /* distance compensation */
  decoderProperties.endOfChainComponents.decNeedsLSDistanceCompensation = 0;


  return error;

}

static int getProcessingDomains( void ) {

  return initRenderPaths(
                         &decoderProperties,
                         &domainSwitchingDecisions,
                         &moduleProcessingDomains,
                         &rendererPathDelays,
                         asc.receiverDelayCompensation
                        );
}

static int compensateDelay( char* inputNameToDelayAlign, char * outputNameDelayAligned, const int nSamplesCoreCoderPreRoll) {
  char inputBasename[FILENAME_MAX];
  char callCmd[3*FILENAME_MAX];
  int overallDelay = 0;
  char tmpString[3 * FILENAME_MAX];

  if ( inputNameToDelayAlign == outputNameDelayAligned ) {
    return -1;
  }

  if ( inputNameToDelayAlign == NULL || outputNameDelayAligned == NULL ) {
    return -1;
  }


  if ( nSamplesPerChannel == -1 ) {
    /* in case of mhas, no edit list info is available / can be read from file */
    if ( moduleProcessingDomains.coreCoder == QMF_DOMAIN ) {
      if ( asc.receiverDelayCompensation ) {
        nSamplesCoreCoderDelay = -1346; /* stay core coder delay agnostic for now */
      }
      else {
        nSamplesCoreCoderDelay = -( GetCoreCoderConstantDelay () + CORE_QMF_SYNTHESIS_DELAY );
      }
    }
    else {
      nSamplesCoreCoderDelay = 0;
    }
  }

  /* discard Audio Pre-Roll (outSamplesFrame) from Core Coder */
  if (1 == bResamplerActive) {
    nSamplesCoreCoderDelay += (int)(resamplingRatio * (float)nSamplesCoreCoderPreRoll);
  } else {
    nSamplesCoreCoderDelay += nSamplesCoreCoderPreRoll;
  }

  if ( moduleProcessingDomains.mixer == QMF_DOMAIN ) {
    if ( asc.receiverDelayCompensation ) {
      overallDelay = nSamplesCoreCoderDelay + 255; /* qmf synthesis + most likely off by two samples   */
    }
    else {
      overallDelay = nSamplesCoreCoderDelay + 257; /* qmf synthesis  */
    }
  }
  else {
    overallDelay = nSamplesCoreCoderDelay;
  }


  strcpy( inputBasename, inputNameToDelayAlign );

  removeWavExtension( inputBasename );

  strcpy ( outputNameDelayAligned, inputBasename );
  strcat ( outputNameDelayAligned, "_delayAligned.wav");

  overallDelay += rendererPathDelays.maxDelay;

  if ( moduleProcessingDomains.saocRenderer == QMF_DOMAIN ) {
    /* remove delay padded by the SAOC encoder */
    overallDelay       += SAOC_ROUNDING_DELAY;
    nSamplesPerChannel -= SAOC_ROUNDING_DELAY;
  }

  if ( moduleProcessingDomains.fdBinaural == QMF_DOMAIN ) {
    overallDelay += BINAURAL_FD_DELAY;
  }

  if (1 == bResamplerActive) {
    nSamplesPerChannel = (int)(resamplingRatio * (float)nSamplesPerChannel);
  }

  sprintf ( callCmd, "%s -if %s -of %s -delay %i -nSamplesPerChannel %i ", binary_WavCutter, inputNameToDelayAlign, outputNameDelayAligned, overallDelay, nSamplesPerChannel );
  sprintf(tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat(callCmd, tmpString);

  if (0 != callBinary(callCmd, "WavCutter", verboseLevel, 0)) {
    return -1;
  }

  return 0;

}

static int transformAndResample( char* renderingPathOutput, char* preMixerFilename, INPUT_TYPE inputType, const MPEG3DA_CONFPOINT mpeg3daCpoType ) {
  if ((TIME_DOMAIN == mixerDomain) || (MPEG3DA_CPO_1 == mpeg3daCpoType)) {
    if ( moduleProcessingDomains.mixer == QMF_DOMAIN ) {
      moduleProcessingDomains.mixer = TIME_DOMAIN;
      transformF2T(renderingPathOutput, binary_DomainSwitcher, 0 );
      if (strcmp(outputFile_Mixer + strlen(outputFile_Mixer) - 4,".wav"))
      {
        strcat( outputFile_Mixer, ".wav" );
      }
      strcat( tempFileNames_MDP_afterSplitting[inputType][2], ".wav" );
    }
    else {
      int l = strlen(outputFile_Mixer);
      if (strcmp(outputFile_Mixer + l - 4,".wav"))
      {
        strcat( outputFile_Mixer, ".wav" );
      }
      strcat( tempFileNames_MDP_afterSplitting[inputType][2], ".wav" );
    }
  }

  if ((1 == bResamplerActive) && !(MPEG3DA_CPO_1 == mpeg3daCpoType)) {
    char tempFilename[FILENAME_MAX];
    strcpy(tempFilename, tempFileNames_MDP_afterSplitting[inputType][2]);
    if (!(strcmp(tempFilename + strlen(tempFilename) - 4,".wav")))
    {
      /* remove wav extension*/
      tempFilename[strlen(tempFilename) - 4] = '\0';
    }
      strcat(tempFilename, "_temp");
      if ( moduleProcessingDomains.mixer == TIME_DOMAIN )
      {
        strcat(tempFilename, ".wav");
      }

    execSampleRateConverter(renderingPathOutput, tempFilename);
    strcpy( preMixerFilename, tempFilename );
  }
  else {
    strcpy( preMixerFilename, renderingPathOutput );
  }

  return 0;

}

static int setDownmixIdFromRuleSet(CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeo, unsigned int numChannels, unsigned int numLFEs, ASCPARSER_DOWNMIX_CONFIG downmixConfig, int* downmixId)
{
  unsigned int i, j, k = 0;
  int  cicpLabelsInToleranceInterval[32][5]; /* first dimension speaker (out), second: fitting cicp indices */
  int amountOfFittingCicpSpeakers = 0;

  CICP2GEOMETRY_CHANNEL_GEOMETRY geoWithMatchingChannelAmounts[32];

  int channelCompare;
  int lfeCompare;

  for(i = 0; i < 32; i++)
    for( j = 0; j < 5; j++ )
       cicpLabelsInToleranceInterval[i][j] = -1;

  /* look for speakers in tolerance interval */
  /* save their cicp indices to  cicpLabelsInToleranceInterval[] */
  for( i = 0; i < numChannels + numLFEs; i++ )
  {
    if(outGeo[i].LFE)
      continue;
    for( j = 0; j < CICP_TOLERANCEINDICES_SIZE; j++ )
    {
      if(outGeo[i].Az > CICP_TOLERANCEINDICES[j].AzStart && outGeo[i].Az < CICP_TOLERANCEINDICES[j].AzEnd
        && outGeo[i].El > CICP_TOLERANCEINDICES[j].ElStart && outGeo[i].El < CICP_TOLERANCEINDICES[j].ElEnd)
      {
        k = 0; /* step further until unused slot in array, to not overwrite already fitting cicp indices */
        while(k < 5 &&  cicpLabelsInToleranceInterval[i][k] != -1)
          k++;

        if(k >= 5);
          return -1;

        /* if cicp index j is in tolerance interval: j is a candidate for this output speaker */
        cicpLabelsInToleranceInterval[i][k] = j;
        if (k == 0)
          amountOfFittingCicpSpeakers++;
      }
    }
  }


  {
    unsigned int maxCicpLayouts = CICP_TOLERANCEINDICES_SIZE;
    int distanceToCICPLayout[43];
    int chOut, chCompare, candidates;

    int lowestDistanceTotal = 10000;
    int cicpLayoutIdxWithLowestDistance = -1;

    for( i = 0; i < maxCicpLayouts; i++ )
    {
      distanceToCICPLayout[i] = -1;
    }

     /* now check all layouts (1) if they fit,
        if yes: (2) their distance to the outspeakers. */
    for(i = 0; i < maxCicpLayouts ; i++)
    {
      int currentCICPSpeakerWithLowestDistance = -1;
      int cicpSpeakersAlreadyInUse[32];
      int j;
      int dmxForCurrentLayoutIndexAvailable = -1;

    /* Look if a downmix matrix for the current cicp index
       is available. If not continue with the next index.*/
      for( j = 0; j < (int) downmixConfig.downmixIdCount; j++ )
      {
        if( downmixConfig.downmixMatrix[j].CICPspeakerLayoutIdx == i )
        {
          dmxForCurrentLayoutIndexAvailable = downmixConfig.downmixMatrix[j].DMX_ID;
        }
      }
      /* No dmx matrix available */
      if( dmxForCurrentLayoutIndexAvailable < 0 )
        continue;


      for( j = 0; j < 32; j++)
        cicpSpeakersAlreadyInUse[j] = -1;

      if( 0 != cicp2geometry_get_geometry_from_cicp(i, geoWithMatchingChannelAmounts, &channelCompare, &lfeCompare) )
      {
        return -1;
      }
      /* Continue with the next cicp layout index if speaker amount is not fitting */
      if( channelCompare != numChannels
        || lfeCompare != numLFEs )
      {
        distanceToCICPLayout[i] = -1;
        continue;
      }
      else
      {
        /* geoWithMatchingChannel Amounts is a candidate */
        for(chCompare = 0; chCompare < amountOfFittingCicpSpeakers + (int) numLFEs; chCompare++ )
        {
          int minDistance = 1000;

          /* Don't compare geometry of LFEs - only the amount is relevant. */
          if( geoWithMatchingChannelAmounts[chCompare].LFE )
          {
            continue;
          }
          /* Assumption: the speaker with the lowest (not equal) distance to a cicp index
             can not be nearer to another cicp index geometry. */
          for(chOut = 0; chOut < amountOfFittingCicpSpeakers + (int)numLFEs; chOut++ )
          {
            if(outGeo[chOut].LFE)
              continue;

            for( candidates = 0; candidates < 5; candidates++ )
            {
              int r;
              int alreadyInUse = 0;
              if(  cicpLabelsInToleranceInterval[chOut][candidates] == -1 && minDistance == 1000)
              {
                /* No fitting speaker found. This case should be already covered above. */
                *downmixId = -1;
                return 0;
              }
              /* check if */
              for( r = 0; r < 32; r++ )
              {
                if(  cicpLabelsInToleranceInterval[chOut][candidates] == cicpSpeakersAlreadyInUse[r] )
                  alreadyInUse = 1;
              }
              if( alreadyInUse )
                continue;

              /* Speaker was previously elected as fitting, now calculate distance */
              if (  cicpLabelsInToleranceInterval[chOut][candidates] == geoWithMatchingChannelAmounts[chCompare].cicpLoudspeakerIndex )
              {
                int currDistance;
                currDistance  = abs(outGeo[chOut].Az - CICP_TOLERANCEINDICES[geoWithMatchingChannelAmounts[chCompare].cicpLoudspeakerIndex].Az);
                currDistance += abs(outGeo[chOut].El - CICP_TOLERANCEINDICES[geoWithMatchingChannelAmounts[chCompare].cicpLoudspeakerIndex].El);
                if( currDistance <= minDistance)
                {
                  minDistance = currDistance;
                  currentCICPSpeakerWithLowestDistance = geoWithMatchingChannelAmounts[chCompare].cicpLoudspeakerIndex;
                }
              }
            }
          }

          cicpSpeakersAlreadyInUse[chCompare] = currentCICPSpeakerWithLowestDistance;
          distanceToCICPLayout[i] += minDistance;

        }
      }

    }

    /* compare the distances and select the lowest */
    for( i = 0; i < CICP_TOLERANCEINDICES_SIZE; i++)
    {
      if(distanceToCICPLayout[i] >= 0
        && distanceToCICPLayout[i] < lowestDistanceTotal)
      {  /* The '<' assures that the layout with lowest index is chosen
            if there are multiple layouts with the same distance. */
        cicpLayoutIdxWithLowestDistance = i;
        lowestDistanceTotal = distanceToCICPLayout[i];
      }
    }

    for( i = 0; i < downmixConfig.downmixIdCount; i++ )
    {
      if( downmixConfig.downmixMatrix[i].CICPspeakerLayoutIdx == cicpLayoutIdxWithLowestDistance )
      {
        *downmixId = downmixConfig.downmixMatrix[i].DMX_ID;
      }
    }

  }

  return 0;
}

static PROCESSING_DOMAIN getCurrentProcessingDomain( char * filename ) {

  char * wavExtension = strstr(filename, ".wav" );

  if ( filename == NULL ) return NOT_DEFINED;

  if ( wavExtension != NULL ) {
    return TIME_DOMAIN;
  }
  else {
    return QMF_DOMAIN;
  }

}

static int MisplacedSpeakers_ReadAngleDeviationsFromFile( char* filename, float *azimuthDeviation, float *elevationDeviation, int numOutputChans )
{
    FILE* fileHandle;
    char line[512] = {0};
    int col = 0;
    int outputChannelIdx = 0;
    int i;

    /* open file */
    fileHandle = fopen(filename, "r");
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open angle deviation input file\n");
        return -1;
    } else {
        fprintf(stderr, "\nFound angle deviation input file: %s.\n", filename );
    }

    /* Get new line */
    fprintf(stderr, "Reading angle deviation input file ... \n");
    while ( fgets(line, 511, fileHandle) != NULL )
    {
        int i = 0;
        char* pChar = line;

        /* Add newline at end of line (for eof line), terminate string after newline */
        line[strlen(line)+1] = '\0';
        line[strlen(line)] = '\n';

        /* Replace all white spaces with new lines for easier parsing */
        while ( pChar[i] != '\0')
        {
            if ( pChar[i] == ' ' || pChar[i] == '\t')
                pChar[i] = '\n';
            i++;
        }

        pChar = line;
        col = 0;

        /* Parse line */
        while ( (*pChar) != '\0')
        {
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

            if ( col == 2 ) {
                fprintf(stderr,"Angle deviation read from file has wrong format (too many columns)!\n");
                return -1;
            }
            if ( outputChannelIdx == numOutputChans ) {
                fprintf(stderr,"Angle deviation read from file has wrong format (too many output channels)!\n");
                return -1;
            }

            if (col == 0) {
                azimuthDeviation[outputChannelIdx]   = (float) atof(pChar);
            } else if (col == 1) {
                elevationDeviation[outputChannelIdx] = (float) atof(pChar);
            }

            /* Jump over parsed float value */
            while (  (*pChar) != '\n' )
                pChar++;

            /* Jump over new lines */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;

            col++;
        }
        if (col != 2) {
            fprintf(stderr,"Angle deviation read from file has wrong format (too little columns)!\n");
            return -1;
        }
        outputChannelIdx++;
    }

    if (outputChannelIdx != numOutputChans) {
        fprintf(stderr,"Angle deviation read from file has wrong format (too little rows)!\n");
        return -1;
    }

    /* print values */
    fprintf(stderr,"\nangleDeviation [deg] = [\n");
    for (i = 0; i < numOutputChans; i++) {
        fprintf(stderr,"%2.4f %2.4f\n",azimuthDeviation[i],elevationDeviation[i]);
    }
    fprintf(stderr,"]\n");

    fclose(fileHandle);
    return 0;
}

static int GetPayloadOffsetDelay ( void ) {

  if ( moduleProcessingDomains.coreCoder == QMF_DOMAIN ) {
    payloadOffsetDelay = GetCoreCoderConstantDelay();
    payloadOffsetDelay += CORE_QMF_SYNTHESIS_DELAY;
  }
  else {
    payloadOffsetDelay = 0;
  }
  return payloadOffsetDelay;

}

static MPEG3DA_RETURN_CODE execContributionMode(int* error_depth, const VERBOSE_LEVEL verboseLevel)
{
  const MPEG3DA_RETURN_CODE default_error = MPEG3DA_ERROR_3DA_EXEC_MODE2;
  MPEG3DA_RETURN_CODE error_3da = MPEG3DA_OK;
  int error = 0;

  /**********************************************/
  /* Call core decoder                          */
  /**********************************************/#
  error_3da = execCoreDecoder(NULL, MPEG3DA_CORE_OUTPUT);
  if (MPEG3DA_OK != error_3da) {
    return PrintErrorCode(error_3da, error_depth, "Error executing MPEG 3D Audio core decoder.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /* copy to output */
  if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
    strcat( outputFile_CoreDecoder, ".wav" );
  }

  error_3da = copyToOutputFile(outputFile_CoreDecoder,
                               wav_OutputFile,
                               0,
                               MPEG3DA_CONTRIBUTIONMODE,
                               BIT_DEPTH_24_BIT,
                               0,
                               0);
  if (MPEG3DA_OK != error_3da) {
    return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  return MPEG3DA_OK;

}

static MPEG3DA_RETURN_CODE playOutCpo(const MPEG3DA_CONFPOINT mpeg3daCpoType, const int delay, const int truncation, char* inputFileName, char* outputFileName, char* signalPathID) {
  MPEG3DA_RETURN_CODE error_3da = MPEG3DA_OK;
  int error                     = 0;
  char tmpName[FILENAME_MAX]    = {0};
  char tmpExt[15]               = {0};

  if (MPEG3DA_DEFAULT_OUTPUT != mpeg3daCpoType) {
    if (NULL != signalPathID) {
      strcpy(tmpName, outputFileName);
      removeWavExtension(tmpName);

      sprintf(tmpExt, "_%s.wav", signalPathID);
      strcat(tmpName, tmpExt);
    } else {
      strcpy(tmpName, outputFileName);
    }

    error_3da = copyToOutputFile(inputFileName,
                                 tmpName,
                                 1,
                                 mpeg3daCpoType,
                                 BIT_DEPTH_24_BIT,
                                 delay,
                                 truncation);
    if (MPEG3DA_OK != error_3da) {
      return error_3da;
    }
  }

  return error_3da;
}

static MPEG3DA_RETURN_CODE execFullDecode(int* error_depth, const VERBOSE_LEVEL verboseLevel)
{
  const MPEG3DA_RETURN_CODE default_error = MPEG3DA_ERROR_3DA_EXEC_MODE1;
  MPEG3DA_RETURN_CODE error_3da           = MPEG3DA_OK;
  int error                               = 0;
  int channelOffset                       = 0;
  int hasIndepConf                        = 0;
  int earconPresent                       = 0;
  CORE_DECODER_PROPERTIES coreDecoderProp = {0};

  ChannelPathDataHandle channelDataHandle                     = NULL;
  ObjectPathDataHandle  sceneDisplacementChannelDataHandle    = NULL;
  ObjectPathDataHandle  objectDataHandle[MAE_MAX_NUM_GROUPS]  = {NULL};
  SaocPathDataHandle    saocDataHandle                        = NULL;
  HoaPathDataHandle     hoaDataHandle                         = NULL;

  FdBinauralRenderingDataHandle       FdBinRendDataHandle     = NULL;
  FdBinauralParametrizationDataHandle FdBinParamDataHandle    = NULL;
  FdBinauralBitstreamWriterDataHandle  FdBinBswDataHandle     = NULL;

  TdBinauralRenderingDataHandle       TdBinRendDataHandle     = NULL;
  TdBinauralParametrizationDataHandle TdBinParamDataHandle    = NULL;

  char preMixerFilename[FILENAME_MAX];

  /* Create mix matrix for given output setup */
  if ( inputType & INPUT_CHANNEL )
  {
    if ( CompareInOutGeometry( inputCicpAudioChannelLayout, outputCicp, outputFile_inGeoAudioChannelLayout, fileOutGeo ) )
    {
      bFormatConverter = 1;
      error = createMixMatrix(verboseLevel);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  /* if there are multiple channel groups, assume format converter is needed */
  if (signalGroupInfo.numSignalGroups > 1) {
    unsigned int i;
    int numChannelGroups = 0;
    for (i = 0; i < signalGroupConfig.numSignalGroups; i++) {
      if (signalGroupConfig.signalGroupType[i] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS) {
        numChannelGroups++;
      }
    }
    if (numChannelGroups > 1) {
      bFormatConverter = 1;
    }
  }

  if ( asc.asc_isValid == 1 && asc.receiverDelayCompensation == 1 ) {
    fprintf(stdout, "\nStart decoding with constant delay.\n");
  }
  else {
    fprintf(stdout, "\nStart decoding without constant delay.\n");
  }

  if ( MPEG3DA_DEFAULT_OUTPUT != mpeg3daCpoType ) {
    fprintf(stderr, "\nWarning: Decoder output is set to Cpo-%d.\n         The decoder will NOT render the default 3DA output!\n", mpeg3daCpoType);
  }

  if (VERBOSE_LVL1 <= verboseLevel) {
    fprintf(stdout, "Initializing %d bit processing chain.\n\n", (int)mpeg3daBitDepth);
  } else {
    fprintf(stdout, "\n");
  }

  /**********************************************/
  /* Call earcon decoder                        */
  /**********************************************/
  error_3da = execEarconDecoder(&earconPresent, mpeg3daCpoType);

  /* Display warning message in case of earcon file format error */
  if (error_3da == MPEG3DA_ERROR_EARCON_FILE_FORMAT_MP4) {
    printf("Warning: Earcon decoding was skipped due to file format being mp4. Decoding will be continued.\n\n");
    error_3da = MPEG3DA_OK;
  }

  if (MPEG3DA_OK != error_3da) {
    return PrintErrorCode(error_3da, error_depth, "Error executing MPEG 3D Audio earcon decoder.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  } else {
    if (1 == earconPresent) {
      char earconOutputFilename[FILENAME_MAX]  = {0};
      strcat(earconOutputFilename, wav_OutputFile);

      if (checkIfWavExtension(earconOutputFilename)) {
        removeWavExtension(earconOutputFilename);
      }
      strcat(earconOutputFilename, "_earcon.wav");
      error_3da = copyToOutputFile(outputFile_earconDecoder,
                                   earconOutputFilename,
                                   1,
                                   MPEG3DA_DEFAULT_OUTPUT,
                                   BIT_DEPTH_DEFAULT,
                                   0,
                                   0);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  if ( 0 == asc.asc_isValid) {
    if (1 == earconPresent) {
      return MPEG3DA_OK;
    } else {
      /* now throw ASC error as the earcon was not decoded */
      return PrintErrorCode(MPEG3DA_ERROR_ASC_INIT, error_depth, "Error initializing ASC parser.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /**********************************************/
  /* Call core decoder                          */
  /**********************************************/
  error_3da = execCoreDecoder(&coreDecoderProp, mpeg3daCpoType);
  if (MPEG3DA_OK != error_3da) {
    return PrintErrorCode(error_3da, error_depth, "Error executing MPEG 3D Audio core decoder.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (MPEG3DA_CORE_OUTPUT == mpeg3daCpoType) {
    if (checkIfFileExists(outputFile_CoreDecoder, "wav")) {
      strcat(outputFile_CoreDecoder, ".wav");
    } else if (checkIfFDFilesExist(outputFile_CoreDecoder, 0)) {
      transformF2T(outputFile_CoreDecoder, binary_DomainSwitcher, 0);
    }

    error_3da = playOutCpo(mpeg3daCpoType,
                           0,
                           0,
                           outputFile_CoreDecoder,
                           wav_OutputFile,
                           NULL);
    return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /**********************************************/
  /* Call prodmetadataDecoder decoder           */
  /**********************************************/
  error_3da = execProdMetadataDecoder();
  if (MPEG3DA_OK != error_3da) {
    return PrintErrorCode(error_3da, error_depth, "Error decoding Production Metadata.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /**********************************/
  /* MPEG-D DRC config              */
  /**********************************/
  if (drcParams.loudnessInfoAvailable && (drcParams.uniDrcInterfacePresent || drcParams.targetLoudnessLevelPresent )) {
    drcParams.lnState = 1;
  } else {
    drcParams.lnState = 0;
  }

  if (drcParams.drcDataAvailable && (drcParams.uniDrcInterfacePresent || drcParams.targetLoudnessLevelPresent || drcParams.drcEffectTypeRequestPresent)) {
    drcParams.drcDecoderState = 1;
  } else {
    drcParams.drcDecoderState = 0;
  }

  if ( drcParams.lnState || drcParams.drcDecoderState ) {
    error = execDrcSelection();
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /**********************************/
  /* post-core config               */
  /**********************************/
  error = getDecoderProperties();
  if (error) {
    return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = getProcessingDomains();
  if (error) {
    return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
    strcat( outputFile_CoreDecoder, ".wav" );
  }

  GetPayloadOffsetDelay ();


  /* read elst information from core decoder */
  if ( bDelayAlignMpegTestpoints ) {
    char coreDelayFilename[120] = "tmpFile3Ddec_coreDelay.txt";
    char line[50];

    /* open delay info file */
    FILE* fCoreDelay = fopen( coreDelayFilename, "r");

    if ( fCoreDelay != NULL ) {

      /* read core coder delay */
      if ( fgets(line, 511, fCoreDelay) ) {
        nSamplesCoreCoderDelay = atoi(line);
        if ( nSamplesCoreCoderDelay == 0 ) {
          nSamplesCoreCoderDelay = -1;
        }
      }
      else {
        nSamplesCoreCoderDelay = -1;
      }

      /* read file length */
      if ( fgets(line, 511, fCoreDelay) ) {
        nSamplesPerChannel = atoi(line);
        if ( nSamplesPerChannel == 0 ) {
          nSamplesPerChannel = -1;
        }
      }
      else {
        nSamplesPerChannel = -1;
      }

      fclose( fCoreDelay );

    }
    else {
      fprintf(stderr, "No core coder delay information available!\n\n");
    }

  }
  else {
    nSamplesCoreCoderDelay = -1;
    nSamplesPerChannel     = -1;
  }

  /**********************************************/
  /* decode object metadata if available */
  /**********************************************/

  if ( ( inputType & INPUT_OBJECT ) || ( inputType & INPUT_SAOC ) )
  {
    char filename[FILENAME_MAX];
    FILE *temp;
    strcpy(filename,"2_tmpFile3Ddec_obj_1.dat");
    if (temp = fopen(filename,"r"))
    {
      fclose(temp);
      hasIndepConf = 1;
    }
    error = execMetadataDecoder( hasIndepConf );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
    if (error == 0) {
      decodedOAM = 1;
    } else {
      decodedOAM = 0;
    }
  }

  if (speakerMisplacement == 1)
  {
    /* prepare local setup information for subsequent modules */

    /* metadata preprocessor: geometry file including 'known Positions' as defined in the localSetupInformationInterface */
    /* Format Converter:      CICP index + angle deviations */
    /* gVBAP renderer:        geometry file with reproduction speaker positions */

    int i = 0;
    float *azimuthDev = NULL;
    float *elevationDev = NULL;

    char fileOutGeoTmp[FILENAME_MAX] = "tmpFile3Ddec_geoFile_MDPreproc.txt";


    /* create geometry structure including misplacement angles */

    /* read non-deviated loudspeaker geometry */
    if (outputCicp == CICP_FROM_GEO)
    {

      cicp2geometry_get_geometry_from_file( fileOutGeo, outGeo, &numChannelsTmp, &numLFEsTmp );
    }
    else
    {
      cicp2geometry_get_geometry_from_cicp( outputCicp, outGeo, &numChannelsTmp, &numLFEsTmp );
    }

    strcpy(fileOutGeoMDPreproc,fileOutGeoTmp);
    strcpy(fileOutGeo_misplaced, "tmpFile3Ddec_geoFile_misplaced.txt");

    azimuthDev   = (float*)calloc(numChannelsTmp+numLFEsTmp,sizeof(float));
    elevationDev = (float*)calloc(numChannelsTmp+numLFEsTmp,sizeof(float));

    /* read misplacement angles */
    MisplacedSpeakers_ReadAngleDeviationsFromFile(fileOutGeoDeviation, azimuthDev, elevationDev,numChannelsTmp+numLFEsTmp );

    /* determine output speaker setup including misplaced loudspeaker geometry */
    for (i = 0; i < (numChannelsTmp+numLFEsTmp); i++)
    {
      outGeo_misplaced[i].cicpLoudspeakerIndex = outGeo[i].cicpLoudspeakerIndex;
      outGeo_misplaced[i].LFE = outGeo[i].LFE;
      outGeo_misplaced[i].loudspeakerType = outGeo[i].loudspeakerType;
      outGeo_misplaced[i].screenRelative = outGeo[i].screenRelative;

      outGeo_misplaced[i].Az = outGeo[i].Az + azimuthDev[i];
      outGeo_misplaced[i].El = outGeo[i].El + elevationDev[i];
    }

    /* write to geometry file --> used by VBAP renderer  */
    MPPreproc_write_geometry_to_file( fileOutGeo_misplaced, outGeo_misplaced, numChannelsTmp, numLFEsTmp, verboseLevel );

    /* write file for metadata preprocessor */
    if (outputCicp == CICP_FROM_GEO)
    {
      int allCicpSpeakerIdx = 1;
      /* check if cicp loudspeaker indices are given for all loudspeakers */
      for (i = 0; i < (numChannelsTmp+numLFEsTmp); i++)
      {
        if (outGeo[i].cicpLoudspeakerIndex == -1)
        {
          allCicpSpeakerIdx = 0;
        }
        if (allCicpSpeakerIdx == 0)
        {
          break;
        }
      }

      if (allCicpSpeakerIdx == 1)
      {
        /* write to geometry file (cicp speaker indices + "known Positions")*/
        MPPreproc_write_geometry_to_file_knownPositions( fileOutGeoTmp, outGeo, outGeo_misplaced, numChannelsTmp, numLFEsTmp, outputCicp );
      }

      /* if not: write misplaced speaker positions to geometry file */
      MPPreproc_write_geometry_to_file( fileOutGeoTmp, outGeo_misplaced, numChannelsTmp, numLFEsTmp, verboseLevel );

    }
    else
    {
      /* write to geometry file (cicp speaker layout index + "known Positions")*/
      MPPreproc_write_geometry_to_file_knownPositions( fileOutGeoTmp, outGeo, outGeo_misplaced, numChannelsTmp, numLFEsTmp, outputCicp );
    }


    /* free offset angles memory. */
    if (azimuthDev != NULL) {
        free(azimuthDev);
        azimuthDev = NULL;
    }
    if (elevationDev != NULL) {
        free(elevationDev);
        elevationDev = NULL;
    }

  }
  else if (outputCicp == CICP_FROM_GEO)
  {
    char fileOutGeoTmp[FILENAME_MAX] = "tmpFile3Ddec_geoFile_MDPreproc.txt";
    cicp2geometry_get_geometry_from_file( fileOutGeo, outGeo, &numChannelsTmp, &numLFEsTmp );
    MPPreproc_write_geometry_to_file( fileOutGeoTmp, outGeo, numChannelsTmp, numLFEsTmp, verboseLevel );
    strcpy(fileOutGeoMDPreproc,fileOutGeoTmp);
  }
  else
  {
    char fileOutGeoTmp[FILENAME_MAX] = "tmpFile3Ddec_geoFile_MDPreproc.txt";
    cicp2geometry_get_geometry_from_cicp( outputCicp, outGeo, &numChannelsTmp, &numLFEsTmp );
    MPPreproc_write_geometry_to_file( fileOutGeoTmp, outGeo, numChannelsTmp, numLFEsTmp, verboseLevel );
    strcpy(fileOutGeoMDPreproc,fileOutGeoTmp);
  }

  downmixId            = -1;
  HoaRenderingMatrixId = -1;

  if (downmixConfigAvailable)
  {
    if (outputCicp == CICP_FROM_GEO)
    {
      setDownmixIdFromRuleSet(outGeo, numChannelsTmp, numLFEsTmp, downmixConfig, &downmixId); /* use non-misplaced speakers for format converter */
    }
    else if (outputCicp > 0)
    {
      unsigned int i;
      for (i = 0; i < downmixConfig.downmixIdCount; i++)
      {
        if (downmixConfig.downmixMatrix[i].CICPspeakerLayoutIdx == outputCicp)
        {
          downmixId = downmixConfig.downmixMatrix[i].DMX_ID;
        }
      }
    }

    /* HOA Rendering Matrix */
    if (downmixId >= 0 && (inputType & INPUT_HOA)){
      unsigned int i;

      for (i = 0; i < hoaMatrixConfig.numHoaMatrices; i++)
      {
        if (downmixId == hoaMatrixConfig.hoaMatrix[i].HOA_ID)
        {
          HoaRenderingMatrixId = downmixId;
          hoaMatrix            = (char*)hoaMatrixConfig.hoaMatrix[i].HoaMatrix;
          hoaMatrixBitSize     = hoaMatrixConfig.hoaMatrix[i].HoaMatrixLenBits;
        }
      }
    }
  }

  if (hasASI == 1) {
    error =  MDPreprocPath_Execute(outputFile_CoreDecoder,
                                   outputFile_MDPreprocessor,
                                   oamInpath,
                                   oamOutpath,
                                   &asc,
                                   decodedOAM,
                                   &asi,
                                   elementInteraction_InputFile,
                                   localSetup_InputFile,
                                   moduleProcessingDomains.coreCoder,
                                   numChannelsTmp,
                                   numLFEsTmp,
                                   fileOutGeoMDPreproc,
                                   downmixId,
                                   &signalGroupConfig,
                                   &numAddedObjects,
                                   BinReadBRIRsFromLocation,
                                   fDBinaural,
                                   tDBinaural,
                                   BinReadBRIRType,
                                   binary_LocalSetupInformationInterface,
                                   &separateSetups,
                                   splittingInfoType,
                                   splittingInfoSetup,
                                   &hasDiffuseness,
                                   sceneDsplInterfaceEnabled,
                                   sceneDisplacement_InputFile,
                                   oamOutpath_sceneDisplacementChannels,
                                   sceneDisplacementChannels_indexListOutpath,
                                   &hasSceneDisplacementChannels,
                                   &signalGroupInfo,
                                   &enhancedObjectMetadataConfig,
                                   signalGroupInfoAvailable,
                                   writeToObjectOutputInterface,
                                   chooseSGmember,
                                   selectedPresetID,
                                   &drcParams,
                                   logFileNames[0],
                                   profile,
                                   verboseLevel);
  } else {
    strncpy(outputFile_MDPreprocessor, outputFile_CoreDecoder, FILENAME_MAX);
  }

  /* MD preprocessor generates the following ouput:

    - tmpFile3Ddec_mdp_output.wav                       modified wave file (Number of channels may be bigger due to divergence (new N channels in the last N channels))
    - tmpFile3Ddec_mdp_object.oam                       modified OAM file (Number of objects may be bigger due to divergence (new N objects as the last N objects))
    - tmpFile3Ddec_mdp_sceneDisplacement_channels.oam   pseudo-OAM file for channels that are influenced by scene-displacement processing

    - tmpFile3Ddec_localSetupInformationInterface.bs   Generated .bs file for the reproduction setup, processed internally by the metadata preprocessor (obsolete afterwards)

    If there are excluded sectors in the bitstream:
      - For setups 0 to N
        * tmpFile3Ddec_separate_output_setup_#.txt              separate layout files for group-dependent setups (defined by use of excluded sectors)
        * tmpFile3Ddec_mdp_output_setup_#.wav                   separate setup-dependent wave files
        * tmpFile3Ddec_mdp_object_setup_#.oam                   separate setup-dependent OAM files (only if setup includes objects)
      - structs and file for splitting the wave files for the rendering modules:
        * struct "splittingInfoSetup"                           struct containing the setup # for each mae_ID
        * struct "splittingInfoType"                            struct containing the signal type for each mae_ID
        * tmpFile3Ddec_mdp_sceneDisplacement_channels_idx.txt   indices of scene-displacement channels for splitting

    - wave file for 'diffuse' loudspeaker feeds
  */

  /**********************************************/
  /* execute DRC selection                      */
  /**********************************************/
  if ( drcParams.lnState || drcParams.drcDecoderState ) {
    error = execDrcSelection();
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /**********************************************/
  /* split core coder wave for separate modules */
  /**********************************************/
  if (separateSetups > 1)
  {
    unsigned int i;
    unsigned int j;
    int start = 0;
    int separateDivergenceObjectsSetup0 = 0;
    int numAddedObjSetup0 = 0;
    int numObjectsPerSetup0 = 0;

    /*  SPLIT UP WAVE FILES FOR SEPARATE RENDERING MODULES */

    /* channels are all located in setup_0 (at the beginning), so just split them off the setup_0 file */
    if ( inputType & INPUT_CHANNEL )
    {
      char tmpChannelFilename[FILENAME_MAX];
      char outputFile_MDPreprocessorChannels[FILENAME_MAX];
      char temp[FILENAME_MAX];
      strcpy(temp,outputFile_MDPreprocessor);
      temp[strlen(temp)-4]='\0';
      if (moduleProcessingDomains.coreCoder == TIME_DOMAIN)
      {
        sprintf (outputFile_MDPreprocessorChannels, "%s_setup_0.wav", temp);
      }
      else
      {
        sprintf (outputFile_MDPreprocessorChannels, "%s_setup_0", outputFile_MDPreprocessor);
      }
      if (hasSceneDisplacementChannels > 0)
      {
        strcpy( tmpChannelFilename, tempFileNames_MDP_tempForSplitting[CHANNELS_COMPLETE][0] );
      }
      else
      {
        strcpy( tmpChannelFilename, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0] );
      }
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
      {
        strcat( tmpChannelFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessorChannels, tmpChannelFilename, channelOffset, asc.channels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
      channelOffset += asc.channels;
    }

    /* create separate files for the objects */
    /* setup_0 may have objects */
    /* all other setups > 0 are objects only */
    if ( inputType & INPUT_OBJECT )
    {
      for ( i = 0; i < (unsigned int)separateSetups; i++)
      {
        char tmpObjectFilename[FILENAME_MAX];
        char outputFile_MDPreprocessorObjects[FILENAME_MAX];
        char temp[FILENAME_MAX];
        int ct = 0;
        int numElementsPerSetup0 = 0;
        int numObjectsPerSetup = 0;

        /* count objects to be split off */
        if (i > 0)
        {
          for ( j = 0; j < MAE_MAX_NUM_ELEMENTS; j++)
          {
            if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i))
            {
              ct++;
            }
          }
          numObjectsPerSetup = ct;
        }
        else
        {
          for ( j = 0; j < MAE_MAX_NUM_ELEMENTS; j++)
          {
            if (splittingInfoSetup[j] == i)
            {
              numElementsPerSetup0++;
            }
          }
          for ( j = 0; j < MAE_MAX_NUM_ELEMENTS; j++)
          {
            if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i))
            {
              start = j;
              break;
            }
          }
          for ( j = start; j < MAE_MAX_NUM_ELEMENTS; j++)
          {
            if (splittingInfoSetup[j] == i)
            {
              if (splittingInfoType[j] == 1)
              {
                ct++;
              }
              if (splittingInfoType[j] != 1)
              {
                break;
              }
            }
          }
          numObjectsPerSetup = ct;
          numObjectsPerSetup0 = ct;
        }

        if (numObjectsPerSetup > 0)
        {
          strcpy(temp,outputFile_MDPreprocessor);
          temp[strlen(temp)-4]='\0';

          sprintf(tmpObjectFilename, "%s_setup_%i", tempFileNames_MDP_afterSplitting[INPUT_OBJECT], i);

          if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
          {
            strcat( tmpObjectFilename, ".wav" );
          }

          if (moduleProcessingDomains.coreCoder == TIME_DOMAIN)
          {
            sprintf(outputFile_MDPreprocessorObjects, "%s_setup_%i.wav", temp, i);
          }
          else
          {
            sprintf (outputFile_MDPreprocessorObjects, "%s_setup_%i", outputFile_MDPreprocessor, i);
          }

          if (i == 0) /* objects from setup 0 (divergence objects not included if SAOC or HOA signals exist, because they are located at the very end of the output file) */
          {
            if (( inputType & INPUT_SAOC ) || ( inputType & INPUT_HOA ))
            {
              if ((numObjectsPerSetup0 + asc.HOATransportChannels + asc.SAOCTransportChannels) < numElementsPerSetup0)
              {
                separateDivergenceObjectsSetup0 = 1;
                /* overwrite filename if setup 0 contains divergence object copies */
                sprintf(tmpObjectFilename, "%s_setup_%i", tempFileNames_MDP_tempForSplitting[OBJECTS_NO_DIVERGENCE], i);
                if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
                {
                  strcat( tmpObjectFilename, ".wav" );
                }
              }
            }
            error = execSeparator ( outputFile_MDPreprocessorObjects, tmpObjectFilename, channelOffset, numObjectsPerSetup );
            channelOffset += numObjectsPerSetup;
          }
          else /* objects from all other setups */
          {
            error = execSeparator ( outputFile_MDPreprocessorObjects, tmpObjectFilename, 0, numObjectsPerSetup );
          }
          if (error) {
            return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
          }
        }
      }
    }

    /* SOAC transport channels are all located in setup_0, so just split them off the setup_0 file */
    if ( inputType & INPUT_SAOC )
    {
      char tmpSaocFilename[FILENAME_MAX];
      char outputFile_MDPreprocessorSAOC[FILENAME_MAX];
      char temp[FILENAME_MAX];
      strcpy(temp,outputFile_MDPreprocessor);
      temp[strlen(temp)-4]='\0';
      if (moduleProcessingDomains.coreCoder == TIME_DOMAIN)
      {
        sprintf (outputFile_MDPreprocessorSAOC, "%s_setup_0.wav", temp);
      }
      else
      {
        sprintf (outputFile_MDPreprocessorSAOC, "%s_setup_0", outputFile_MDPreprocessor);
      }
      strcpy( tmpSaocFilename, tempFileNames[INPUT_CHANNEL][0] );
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
      {
        strcat( tmpSaocFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessorSAOC, tmpSaocFilename, channelOffset, asc.SAOCTransportChannels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
      channelOffset += asc.SAOCTransportChannels;
    }

    /* HOA transport channels are all located in setup_0, so just split them off the setup_0 file */
    if ( inputType & INPUT_HOA )
    {
      char tmpHoaFilename[FILENAME_MAX];
      char outputFile_MDPreprocessorHOA[FILENAME_MAX];
      char temp[FILENAME_MAX];
      strcpy(temp,outputFile_MDPreprocessor);
      temp[strlen(temp)-4]='\0';
      if (moduleProcessingDomains.coreCoder == TIME_DOMAIN)
      {
        sprintf (outputFile_MDPreprocessorHOA, "%s_setup_0.wav", temp);
      }
      else
      {
        sprintf (outputFile_MDPreprocessorHOA, "%s_setup_0", outputFile_MDPreprocessor);
      }
      strcpy( tmpHoaFilename, tempFileNames[INPUT_CHANNEL][0] );
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
      {
        strcat( tmpHoaFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessorHOA, tmpHoaFilename, channelOffset, asc.HOATransportChannels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
      channelOffset += asc.HOATransportChannels;
    }

    if ( separateDivergenceObjectsSetup0 == 1)
    {
      /* split off additional objects (divergence processing) of setup 0 if necessary */
      /* then combine them with original object wave file of setup 0 */
      int end = 0;
      int start2 = 0;
      int ct = 0;

      for (j = 0; j < MAE_MAX_NUM_ELEMENTS; j++) {
        if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i)) {
          start = j;
          break;
        }
      }

      for (j = start; j < MAE_MAX_NUM_ELEMENTS; j++) {
        if (splittingInfoSetup[j] == i) {
          if (splittingInfoType[j] == 1) {
            ct++;
          }

          if (splittingInfoType[j] != 1) {
            end = j;
            break;
          }
        }
      }

      for ( j = end; j < MAE_MAX_NUM_ELEMENTS; j++) {
        if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i)) {
          start2 = j;
          break;
        }
      }

      ct = 0;
      for ( j = start2; j < MAE_MAX_NUM_ELEMENTS; j++) {
        if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i)) {
          ct++;
        }

        if (splittingInfoType[j] != 1) {
          end = j;
          break;
        }
      }
      numAddedObjSetup0 = ct;
    }

    if ( numAddedObjSetup0 > 0 ) {
      char tmpDivFilename[FILENAME_MAX];
      char tmpCombObjectFilename[FILENAME_MAX];
      char tmpObjectFilename[FILENAME_MAX];
      char outputFile_MDPreprocessorDivergence[FILENAME_MAX];
      char temp[FILENAME_MAX];
      strcpy(temp,outputFile_MDPreprocessor);
      temp[strlen(temp)-4]='\0';

      sprintf(tmpDivFilename, "%s_setup_0", tempFileNames_MDP_tempForSplitting[OBJECTS_WITH_DIVERGENCE][0]);
      sprintf(tmpCombObjectFilename, "%s_setup_0", tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0]);
      sprintf(tmpObjectFilename, "%s_setup_0", tempFileNames_MDP_tempForSplitting[OBJECTS_NO_DIVERGENCE][0]);

      if (moduleProcessingDomains.coreCoder == TIME_DOMAIN)
      {
        sprintf(outputFile_MDPreprocessorDivergence, "%s_setup_0.wav", temp);
      }
      else
      {
        sprintf(outputFile_MDPreprocessorDivergence, "%s_setup_0", temp);
      }
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN )
      {
        strcat( tmpDivFilename, ".wav" );
        strcat( tmpCombObjectFilename, ".wav" );
      }
      /* split off divergence objects */
      error = execSeparator ( outputFile_MDPreprocessorDivergence, tmpDivFilename, channelOffset, numAddedObjSetup0 );
      /* combine with no-divergence file */
      error = MDPreproc_combineWav(tmpCombObjectFilename,
                                   tmpObjectFilename,
                                   tmpDivFilename,
                                   moduleProcessingDomains.coreCoder);
    }

    totalNumObjAfterMDP = asc.objects + numAddedObjects;
  }
  else /* no separate setups = no excluded sectors used */
  {
    if ( inputType & INPUT_CHANNEL )
    {
      char tmpChannelFilename[FILENAME_MAX];

      if (hasSceneDisplacementChannels > 0)
      {
        strcpy( tmpChannelFilename, tempFileNames_MDP_tempForSplitting[CHANNELS_COMPLETE][0] );
      }
      else
      {
        strcpy( tmpChannelFilename, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0] );
      }

      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpChannelFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessor, tmpChannelFilename, channelOffset, asc.channels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      channelOffset += asc.channels;
    }

    if ( inputType & INPUT_OBJECT )
    {
      char tmpObjectFilename[FILENAME_MAX];

      if (numAddedObjects > 0)
      {
        strcpy( tmpObjectFilename, tempFileNames_MDP_tempForSplitting[OBJECTS_NO_DIVERGENCE][0] );
      }
      else
      {
        strcpy( tmpObjectFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0] );
      }

      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpObjectFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessor, tmpObjectFilename, channelOffset, asc.objects );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      channelOffset += asc.objects;
    }

    if ( inputType & INPUT_SAOC )
    {
      char tmpSaocFilename[FILENAME_MAX];

      strcpy( tmpSaocFilename, tempFileNames_MDP_afterSplitting[INPUT_SAOC][0] );

      /* TD CORE */
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpSaocFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessor, tmpSaocFilename, channelOffset, asc.SAOCTransportChannels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      channelOffset += asc.SAOCTransportChannels;
    }

    if ( inputType & INPUT_HOA )
    {
      char tmpHoaFilename[FILENAME_MAX];
      if (bHoaBinaryPresent == 0) {
        return PrintErrorCode(default_error, error_depth, "Error: No HOA binary available but HOA path is triggered!", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      strcpy( tmpHoaFilename, tempFileNames_MDP_afterSplitting[INPUT_HOA][0] );

      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpHoaFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessor, tmpHoaFilename, channelOffset, asc.HOATransportChannels );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      /* TD CORE - nothing to do, stay in TD for hoa rendering */

      /* FD CORE */   /* moved to next HOA dec section, we do not need to do it twice */
      /*if ( domainSwitchingDecisions.hoaDomains.PreHoaRendering == QMF_SYNTHESIS ) {
        transformF2T( tempFileNames[INPUT_HOA][0], binary_DomainSwitcher );
      }*/

      channelOffset += asc.HOATransportChannels;
    }

    /* split off additional objects (divergence processing) */
    /* then combine with original object wave file */

    if ( numAddedObjects > 0 )
    {
      char tmpObjDivergenceFilename[FILENAME_MAX];
      char tmpObjectFilename[FILENAME_MAX];
      char tmpCombObjectFilename[FILENAME_MAX];

      strcpy( tmpObjDivergenceFilename, tempFileNames_MDP_tempForSplitting[OBJECTS_WITH_DIVERGENCE][0] );
      strcpy( tmpObjectFilename, tempFileNames_MDP_tempForSplitting[OBJECTS_NO_DIVERGENCE][0] );
      strcpy( tmpCombObjectFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0] );

      /* TD CORE */
      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpObjDivergenceFilename, ".wav" );
      }

      error = execSeparator ( outputFile_MDPreprocessor, tmpObjDivergenceFilename, channelOffset, numAddedObjects );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      channelOffset += numAddedObjects;

      if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpObjectFilename, ".wav" );
        strcat( tmpCombObjectFilename, ".wav" );
      }

      error = MDPreproc_combineWav(tmpCombObjectFilename,
                                   tmpObjectFilename,
                                   tmpObjDivergenceFilename,
                                   moduleProcessingDomains.coreCoder);
    }
    totalNumObjAfterMDP = asc.objects + numAddedObjects;
  }

  /* split off channel-based content of scene-displacement channels from channel file*/
  if ( hasSceneDisplacementChannels > 0 )
  {
    char tmpChannelFilename[FILENAME_MAX];
    char tmpChannelSDfilename[FILENAME_MAX];
    char tmpChannelNonSDfilename[FILENAME_MAX];

    int SDchannelIdx[MAE_MAX_NUM_ELEMENTS];

    strcpy(tmpChannelFilename,tempFileNames_MDP_tempForSplitting[CHANNELS_COMPLETE][0]);
    strcpy(tmpChannelSDfilename,tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][0]);
    strcpy(tmpChannelNonSDfilename,tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0]);
    if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tmpChannelFilename, ".wav" );
        strcat( tmpChannelSDfilename, ".wav" );
        strcat( tmpChannelNonSDfilename, ".wav" );
      }

    error = MDPreproc_getSDchannelIdx(sceneDisplacementChannels_indexListOutpath, SDchannelIdx, &numSceneDisplacementChannels);

    /* separate SD channels and non-SD channels */
    error = MDPreproc_separateSDchannels(tmpChannelFilename, tmpChannelSDfilename, tmpChannelNonSDfilename, SDchannelIdx, asc.channels, numSceneDisplacementChannels, moduleProcessingDomains.coreCoder);

    if (asc.channels-numSceneDisplacementChannels == 0)
    {
      /* all channels are tracked -> no channel-based content to render with FC */
      disableFormatConverter = 1;
      if( bUseWindowsCommands ) {
        system ( "del tmpFile3Ddec_mdp_channel_output.* 2>  NUL" );
      }
      else {
        system ( "rm -f tmpFile3Ddec_mdp_channel_output.*" );
      }
    }
  }
  else
  {
    /* rename channel-based file to "nonSDchannels" */
    /* char tmpChannelFilename[FILENAME_MAX];
    char tmpChannelNonSDfilename[FILENAME_MAX];

    strcpy(tmpChannelFilename,tempFileNames[INPUT_CHANNEL][0]);
    strcpy(tmpChannelNonSDfilename,tempFileNames[INPUT_CHANNEL_NON_SD][0]);
    if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
      strcat( tmpChannelFilename, ".wav" );
      strcat( tmpChannelNonSDfilename, ".wav" );
    }
    error = execSeparator(tmpChannelFilename,tmpChannelNonSDfilename,0,asc.channels); */
  }

  /**********************************/
  /* run separate modules           */
  /**********************************/

  /* init */

  if (inputType & INPUT_CHANNEL) {
    unsigned int ui;
    unsigned int n;
    int i;

    /* if a downmixConfig is available look for a fitting downmix matrix */
    if (downmixConfigAvailable) {
      /* Check if there is a downmix matrix for the calculated downmix id */
      for (ui = 0; ui < downmixConfig.downmixIdCount; ui++) {
        if (downmixId == downmixConfig.downmixMatrix[ui].DMX_ID) {
          if (downmixConfig.downmixMatrix[ui].downmixType == 1) {
            matrixBitSize = downmixConfig.downmixMatrix[ui].DmxMatrixLenBits[0];
            downmixMatrix = (char*)downmixConfig.downmixMatrix[ui].DownmixMatrix[0];
          }
        }
      }
    }

    if (disableFormatConverter == 0) {
      int numChannelBasedElements = 0;
      int numChannelBasedGroups = 0;
      int elementID = -1;

      /* count number of channel-based signal elements, where scene displacement processing is disabled */
      for (n = 0; n < signalGroupConfig.numSignalGroups; n++) {
        int ctMembers = 0;
        for (ui = 0; ui < signalGroupConfig.numberOfSignals[n]; ui++) {
          elementID++;
          if (signalGroupConfig.signalGroupType[n] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS) {
            if (signalGroupInfo.fixedPosition[MP_getSignalGroupIndex(&asi, &signalGroupConfig, n, elementID)] == 1) {
              numChannelBasedElements++;
              ctMembers++;
            }
          }
        }
        if (ctMembers == signalGroupConfig.numberOfSignals[n]) {
          numChannelBasedGroups++;
        }
      }

      /* concatenate channel-based group setups to form overall format converter input layout */
      /* currently only supported if downmix matrix is not transmitted in the bitstream */
      if ((numChannelBasedGroups > 1) && (downmixId == -1)){
        CICP2GEOMETRY_CHANNEL_GEOMETRY geoFC[MAE_MAX_NUM_ELEMENTS];
        int numCh = 0;
        int numLFE = 0;
        int numOverallChannels = asc.channels;
        unsigned int k;
        int ct = 0;

        for (n = 0; n < signalGroupConfig.numSignalGroups; n++)
        {
          int numChTmp, numLFETmp;
          CICP2GEOMETRY_CHANNEL_GEOMETRY thisGroup[MAE_MAX_NUM_ELEMENTS];
          memset(thisGroup,0,sizeof(thisGroup));

          if (signalGroupConfig.signalGroupType[n] == ASCPARSER_SIGNAL_GROUP_TYPE_CHANNELS)
          {
            if (signalGroupConfig.signalGroupChannelLayout[n].speakerLayoutID == 0)
            {
              cicp2geometry_get_geometry_from_cicp(signalGroupConfig.signalGroupChannelLayout[n].CICPspeakerLayoutIdx, thisGroup, &numChTmp, &numLFETmp);
            }
            else if (signalGroupConfig.signalGroupChannelLayout[n].speakerLayoutID == 1)
            {
              int numSpeakers = signalGroupConfig.signalGroupChannelLayout[n].numSpeakers;
              for (i = 0; i < numSpeakers; i++)
              {
                cicp2geometry_get_geometry_from_cicp_loudspeaker_index(signalGroupConfig.signalGroupChannelLayout[n].CICPspeakerIdx[i], &thisGroup[i]);
              }

            }
            else if (signalGroupConfig.signalGroupChannelLayout[n].speakerLayoutID == 2)
            {
              int numSpeakers = signalGroupConfig.signalGroupChannelLayout[n].numSpeakers;
              for (i = 0; i < numSpeakers; i++)
              {
                if ((signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx > 0) && (signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx > 0))
                {
                  cicp2geometry_get_geometry_from_cicp_loudspeaker_index(signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx, &thisGroup[i]);
                }
                else
                {
                  thisGroup[i].Az = ascparser_AzimuthAngleIdxToDegrees(signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx,signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection,signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.angularPrecision);
                  thisGroup[i].El = ascparser_ElevationAngleIdxToDegrees(signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx,signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].ElevationDirection,signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.angularPrecision);
                  thisGroup[i].cicpLoudspeakerIndex = -1;
                  thisGroup[i].LFE = signalGroupConfig.signalGroupChannelLayout[n].flexibleSpeakerConfig.speakerDescription[i].isLFE;
                  thisGroup[i].screenRelative = 0;
                }
              }
            }

            for (k = 0; k < signalGroupConfig.numberOfSignals[n]; k++)
            {
              if (signalGroupInfo.fixedPosition[n] == 1)
              {
                geoFC[ct] = thisGroup[k];
                if (geoFC[ct].LFE == 1)
                {
                  numLFE++;
                }
                else
                {
                  numCh++;
                }
                ct++;
              }
            }
          }
        }
        cicp2geometry_write_geometry_to_file(outputFile_inGeoAudioChannelLayout,geoFC, numCh, numLFE);
        inputCicpAudioChannelLayout = CICP_FROM_GEO;
      }
      else if ((numChannelBasedGroups > 1) && (downmixId != -1)) {
        return PrintErrorCode(default_error, error_depth, "Framework only supports the use of transmitted downmix matrixes for files with only one channel-based group.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = ChannelPathInit(&channelDataHandle,
                              inputCicpAudioChannelLayout,
                              outputCicp,
                              outputFile_inGeoAudioChannelLayout,
                              fileOutGeo,
                              fileOutGeoDeviationPtr,
                              logFileNames[INPUT_CHANNEL],
                              binary_FormatConverter,
                              passiveDownmixFlag,
                              asc.channels,
                              drcParams,
                              domainSwitchingDecisions.channelDomains.PostDrc1,
                              moduleProcessingDomains.drc1channels,
#if IAR
                              binary_IarFormatConverter,
#endif
                              domainSwitchingDecisions.channelDomains.PreFormatConverter,
                              binary_DomainSwitcher,
                              phaseAlignStrength,
                              immersiveDownmixFlag,
                              downmixId,
                              matrixBitSize,
                              downmixMatrix,
                              binary_DmxMatrixDecoder );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }

    /* init object renderer for Scene Displacement channels */
    if ((hasSceneDisplacementChannels > 0) && (hasASI)) {
      error = ObjectPathInit(&sceneDisplacementChannelDataHandle,
                             numSceneDisplacementChannels,
                             -1,
                             fileOutGeo_misplaced,
                             logFileNames[INPUT_CHANNEL],
                             binary_Renderer,
                             drcParams,
                             domainSwitchingDecisions.objectDomains.PostDrc1,
                             moduleProcessingDomains.drc1objects,
                             binary_DomainSwitcher);
    }

    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }


  if (inputType & INPUT_OBJECT) {
    if (separateSetups > 1) {
      char tempGeo[FILENAME_MAX];
      int i, j;

      for ( i = 0; i < separateSetups; i++) {
        int isObjectBasedSetup = 0;
        int numObjects = 0;

        for (j = 0; j < MAE_MAX_NUM_ELEMENTS; j++) {
          if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i)) {
            isObjectBasedSetup = 1;
            numObjects++;
          }
        }

        if (isObjectBasedSetup == 1) {
          sprintf (tempGeo, "tmpFile3Ddec_separate_output_setup_%i.txt", i);
          error = ObjectPathInit(&(objectDataHandle[i]),
                                 numObjects,
                                 -1,
                                 tempGeo,
                                 logFileNames[INPUT_OBJECT],
                                 binary_Renderer,
                                 drcParams,
                                 domainSwitchingDecisions.objectDomains.PostDrc1,
                                 moduleProcessingDomains.drc1objects,
                                 binary_DomainSwitcher);
        }
      }
    } else {
      if (speakerMisplacement == 1)
      {
                error = ObjectPathInit(&(objectDataHandle[0]),
                               totalNumObjAfterMDP,
                               -1,
                               fileOutGeo_misplaced,
                               logFileNames[INPUT_OBJECT],
                               binary_Renderer,
                               drcParams,
                               domainSwitchingDecisions.objectDomains.PostDrc1,
                               moduleProcessingDomains.drc1objects,
                               binary_DomainSwitcher);
      }
      else
      {
        error = ObjectPathInit(&(objectDataHandle[0]),
                               totalNumObjAfterMDP,
                               outputCicp,
                               fileOutGeo,
                               logFileNames[INPUT_OBJECT],
                               binary_Renderer,
                               drcParams,
                               domainSwitchingDecisions.objectDomains.PostDrc1,
                               moduleProcessingDomains.drc1objects,
                               binary_DomainSwitcher);
      }
    }
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  if ( inputType & INPUT_SAOC )
  {
#ifdef SUPPORT_SAOC_DMX_LAYOUT
    error = SaocPathInit ( &saocDataHandle, asc.saocHeaderLength, asc.outputFrameLength, inputCicpReferenceLayout, outputFile_inGeoRefLayout, asc.saocDmxLayoutPresent, inputCicpSaocDmxChannelLayout, outputFile_inGeoSaocDmxLayout, outputCicp, fileOutGeo, fileOutGeoDeviationPtr, logFileNames[INPUT_SAOC], binary_SaocDecoder, binary_FormatConverter
#else
    error = SaocPathInit ( &saocDataHandle, asc.saocHeaderLength, asc.outputFrameLength, inputCicpReferenceLayout, outputFile_inGeoRefLayout, outputCicp, fileOutGeo, fileOutGeoDeviationPtr, logFileNames[INPUT_SAOC], binary_SaocDecoder, binary_FormatConverter
#endif
                          , drcParams, TIME_DOMAIN );
   if (error) {
     return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
   }
  }

  if ( inputType & INPUT_HOA )
  {
    if (speakerMisplacement == 1)
    {
      error = HoaPathInit(&hoaDataHandle, asc.HOATransportChannels, -1, fileOutGeo_misplaced, logFileNames[INPUT_HOA], binary_HoaDecoder, asc.outputFrameLength, binary_HoaMatrixDecoder, hoaMatrix, HoaRenderingMatrixId, hoaMatrixBitSize
                         , drcParams, domainSwitchingDecisions.hoaDomains.PostDrc1, moduleProcessingDomains.drc1hoa, binary_DomainSwitcher, binary_WavCutter);
    }
    else
    {
      error = HoaPathInit(&hoaDataHandle, asc.HOATransportChannels, outputCicp, fileOutGeo, logFileNames[INPUT_HOA], binary_HoaDecoder, asc.outputFrameLength, binary_HoaMatrixDecoder, hoaMatrix, HoaRenderingMatrixId, hoaMatrixBitSize
                          , drcParams, domainSwitchingDecisions.hoaDomains.PostDrc1, moduleProcessingDomains.drc1hoa, binary_DomainSwitcher, binary_WavCutter);
    }
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /* exec */

  if ( inputType & INPUT_CHANNEL )
  {
    if (hasSceneDisplacementChannels > 0)
    {
      char tmpObjectPathOutFilename[FILENAME_MAX];

      strcpy( tmpObjectPathOutFilename, tempFileNames[INPUT_CHANNEL_SD][1] );

      /* TD CORE */
      if ( domainSwitchingDecisions.objectDomains.PreDrc1 == QMF_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == QMF_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][0], binary_DomainSwitcher, 0);
      } else if ( domainSwitchingDecisions.objectDomains.PreDrc1 == STFT_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == STFT_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][0], binary_DomainSwitcher, 1);
      }
      else if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][0], ".wav" );
      }

      /* FD CORE - nothing to do, stay in FD for object rendering */

      if ( moduleProcessingDomains.objectRenderer == TIME_DOMAIN ) {
        strcat( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], ".wav" );
      }

      if ( moduleProcessingDomains.objectRenderer == QMF_DOMAIN ) {
        strcpy(tmpObjectPathOutFilename, "tmpFile3Ddec_tmp_obj");
        changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][0], tmpObjectPathOutFilename, -(payloadOffsetDelay - 257) );
      }
      else {
        strcpy(tmpObjectPathOutFilename, tempFileNames[INPUT_CHANNEL_SD][0]);
      }

      error_3da = ObjectPathRun(sceneDisplacementChannelDataHandle,
                                tmpObjectPathOutFilename,
                                tempFileNames[INPUT_CHANNEL_SD][1],
                                hasASI,
                                -1,
                                1,
                                verboseLevel);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error running object path.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], binary_DomainSwitcher, 0 );
        strcpy( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], tmpObjectPathOutFilename );
      }
      else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == STFT_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], binary_DomainSwitcher, 1 );
        strcpy( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], tmpObjectPathOutFilename );
      }
      else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_SYNTHESIS ) {
        transformF2T(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], binary_DomainSwitcher, 0 );
        strcpy( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1], tmpObjectPathOutFilename );
      }
    }

    if (disableFormatConverter == 0)
    {
      /* TD CORE */
      if ( domainSwitchingDecisions.channelDomains.PostCore == QMF_ANALYSIS || domainSwitchingDecisions.channelDomains.PreDrc1 == QMF_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], binary_DomainSwitcher, 0);
      } else if ( domainSwitchingDecisions.channelDomains.PostCore == STFT_ANALYSIS || domainSwitchingDecisions.channelDomains.PreDrc1 == STFT_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], binary_DomainSwitcher, 1);
      } else if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], ".wav" );
      }
        
      /* FD CORE */
      if ( domainSwitchingDecisions.channelDomains.PostCore == QMF_SYNTHESIS ) { 
        transformF2T(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], binary_DomainSwitcher, 0);
      }

      {
        int bTimeCore = moduleProcessingDomains.coreCoder == TIME_DOMAIN;
        int bDrcNeutral = (moduleProcessingDomains.drc1channels == NOT_DEFINED || moduleProcessingDomains.drc1channels == TIME_DOMAIN);
        int bNoFormatConverter = moduleProcessingDomains.formatConverter == NOT_DEFINED;
        int bTimeDomainMixer = mixerDomain == TIME_DOMAIN;
        int bNoQmfFormatConverter = !(moduleProcessingDomains.formatConverter == QMF_DOMAIN);
        if ( bTimeCore && ( bDrcNeutral || bNoFormatConverter ) || ( bTimeDomainMixer && bNoQmfFormatConverter ) ) {
          strcat( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1], ".wav" );
        }
      }

#if IAR
      error = ChannelPathRun ( channelDataHandle, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1], "tmp3D.txt", profile, verboseLevel );
#else
      error = ChannelPathRun ( channelDataHandle, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][0], tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1], profile, verboseLevel );
#endif

      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      /* DEBUG  asc.receiverDelay = 0; */

      if ( bFormatConverter == 0 && asc.receiverDelayCompensation == 1 ) {

        PROCESSING_DOMAIN currentProcessingDomain = getCurrentProcessingDomain( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1] );

        /* error = receiverDelayCompensation ( currentProcessingDomain ); */
        if (error) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      }

      if ( domainSwitchingDecisions.channelDomains.PostFormatConverter == QMF_SYNTHESIS ) {
        transformF2T(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1], binary_DomainSwitcher, 0);
      }
    }
  }

  if ( inputType & INPUT_OBJECT ) {
    if (separateSetups > 1) {
      int i, j;

      for (i = 0; i < separateSetups; i++) {
        int isObjectBasedSetup = 0;
        int numObjects = 0;

        for (j = 0; j < MAE_MAX_NUM_ELEMENTS; j++) {
          if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i)) {
            isObjectBasedSetup = 1;
            numObjects++;
          }
        }

        if (isObjectBasedSetup == 1) {
          char tmpObjectPathOutFilename[FILENAME_MAX];
          char tmpObjectPathInFilename[FILENAME_MAX];
          sprintf(tmpObjectPathOutFilename,"%s_setup_%i",tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1],i);
          sprintf(tmpObjectPathInFilename,"%s_setup_%i",tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0],i);

          /* TD CORE */
          if ( domainSwitchingDecisions.objectDomains.PreDrc1 == QMF_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == QMF_ANALYSIS ) {
            transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], binary_DomainSwitcher, 0);
          } else if ( domainSwitchingDecisions.objectDomains.PreDrc1 == STFT_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == STFT_ANALYSIS ) {
            transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], binary_DomainSwitcher, 1);
          }
          else if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
            strcat( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], ".wav" );
          }

          /* FD CORE - nothing to do, stay in FD for object rendering */

          if ( moduleProcessingDomains.objectRenderer == TIME_DOMAIN ) {
            strcat( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], ".wav" );
          }

          if ( moduleProcessingDomains.objectRenderer == QMF_DOMAIN ) {
            strcpy(tmpObjectPathOutFilename, "tmpFile3Ddec_tmp_obj");
            changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], tmpObjectPathOutFilename, -(payloadOffsetDelay - 257) );
          }
          else {
            strcpy(tmpObjectPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0]);
          }

          error = ObjectPathRun(objectDataHandle[i],
                                tmpObjectPathInFilename,
                                tmpObjectPathOutFilename,
                                hasASI,
                                i,
                                0,
                                verboseLevel);
          if (MPEG3DA_OK != error_3da) {
            return PrintErrorCode(error_3da, error_depth, "Error running onject path.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
          }

          if ( moduleProcessingDomains.objectRenderer == QMF_DOMAIN ) {
            strcpy(tmpObjectPathOutFilename, "tmpFile3Ddec_tmp_obj2");
            changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], tmpObjectPathOutFilename, (payloadOffsetDelay - 257) );
            changeDelayOfFile( tmpObjectPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], 0);
          }

          if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_ANALYSIS ) {
            transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 0);
            strcpy( tempFileNames[INPUT_OBJECT][1], tmpObjectPathOutFilename );
          }
          else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_SYNTHESIS ) {
            transformF2T(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 0);
          }
          else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == STFT_SYNTHESIS ) {
            transformF2T(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 1);
          }
        }
      }
    }
    else
    {
      char tmpObjectPathOutFilename[FILENAME_MAX];

      strcpy( tmpObjectPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1] );

      /* TD CORE */
      if ( domainSwitchingDecisions.objectDomains.PreDrc1 == QMF_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == QMF_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], binary_DomainSwitcher, 0);
      } else if ( domainSwitchingDecisions.objectDomains.PreDrc1 == STFT_ANALYSIS || domainSwitchingDecisions.objectDomains.PreObjectRendering  == STFT_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], binary_DomainSwitcher, 1);
      }
      else if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
        strcat( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], ".wav" );
      }

      /* FD CORE - nothing to do, stay in FD for object rendering */
      if ( moduleProcessingDomains.objectRenderer == TIME_DOMAIN ) {
        strcat( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], ".wav" );
      }

      if ( moduleProcessingDomains.objectRenderer == QMF_DOMAIN ) {
        strcpy(tmpObjectPathOutFilename, "tmpFile3Ddec_tmp_obj");
        changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0], tmpObjectPathOutFilename, -(payloadOffsetDelay - 257) );
      }
      else {
        strcpy(tmpObjectPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][0]);
      }

      error = ObjectPathRun(objectDataHandle[0],
                            tmpObjectPathOutFilename,
                            tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1],
                            hasASI,
                            -1,
                            0,
                            verboseLevel);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error running object path.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      if ( moduleProcessingDomains.objectRenderer == QMF_DOMAIN ) {
        strcpy(tmpObjectPathOutFilename, "tmpFile3Ddec_tmp_obj2");
        changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], tmpObjectPathOutFilename, (payloadOffsetDelay - 257) );
        changeDelayOfFile( tmpObjectPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], 0);
      }

      if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_ANALYSIS ) {
        transformT2F( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 0);
        strcpy( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], tmpObjectPathOutFilename );
      }
      else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == QMF_SYNTHESIS ) {
        transformF2T(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 0);
      }
      else if ( domainSwitchingDecisions.objectDomains.PostObjectRendering == STFT_SYNTHESIS ) {
        transformF2T(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], binary_DomainSwitcher, 1);
      }
    }
  }

  if ( inputType & INPUT_SAOC )
  {
    char tmpSaocPathOutFilename[FILENAME_MAX];

    if ( domainSwitchingDecisions.saocDomains.PreSaocRendering == QMF_ANALYSIS ) {
      transformT2F( tempFileNames_MDP_afterSplitting[INPUT_SAOC][0], binary_DomainSwitcher, 0 );
    }

    /* FD CORE - nothing to do, stay in FD for saoc rendering */

    if (  moduleProcessingDomains.coreCoder == QMF_DOMAIN ) {
      strcpy(tmpSaocPathOutFilename, "tmpFile3Ddec_tmp_saoc");
      changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_SAOC][0], tmpSaocPathOutFilename, - (payloadOffsetDelay - 384 + 257) );
    }
    else {
      strcpy(tmpSaocPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_SAOC][0]);
    }

    error = SaocPathRun ( saocDataHandle, tmpSaocPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_SAOC][1], hasASI, verboseLevel );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    strcpy(tmpSaocPathOutFilename, "tmpFile3Ddec_tmp_saoc2");
    changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_SAOC][1], tmpSaocPathOutFilename, (payloadOffsetDelay - 384 + 257) );
    changeDelayOfFile( tmpSaocPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_SAOC][1], 0);

    if ( domainSwitchingDecisions.saocDomains.PostSaocRendering == QMF_SYNTHESIS ) {
      transformF2T(tempFileNames_MDP_afterSplitting[INPUT_SAOC][1], binary_DomainSwitcher, 0 );
    }
  }

  if ( inputType & INPUT_HOA )
  {
    char tmpHoaPathOutFilename[FILENAME_MAX];
    char tmpHoaFilename[FILENAME_MAX];

    strcpy( tmpHoaPathOutFilename, tempFileNames_MDP_afterSplitting[INPUT_HOA][1] );
    strcat( tempFileNames_MDP_afterSplitting[INPUT_HOA][1], ".wav" );


    if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
      strcat( tempFileNames_MDP_afterSplitting[INPUT_HOA][0], ".wav" );
    }
    else                               /* ( domainSwitchingDecisions.hoaDomains.PreHoaRendering == QMF_SYNTHESIS )?? */
    {
      transformF2T(tempFileNames_MDP_afterSplitting[INPUT_HOA][0], binary_DomainSwitcher, 0 );
    }

    strcpy(tmpHoaFilename, "tmpFile3Ddec_tmp_hoa.wav");
    changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_HOA][0], tmpHoaFilename, - (payloadOffsetDelay) );

    if ( domainSwitchingDecisions.hoaDomains.PreDrc1 == STFT_ANALYSIS ) {
      strcpy(tmpHoaFilename, "tmpFile3Ddec_tmp_hoa");
      transformT2F( tmpHoaFilename, binary_DomainSwitcher, 1);
    }

    error = HoaPathRun ( hoaDataHandle, tmpHoaFilename, tempFileNames_MDP_afterSplitting[INPUT_HOA][1], mpeg3daBitDepth, verboseLevel );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    changeDelayOfFile( tempFileNames_MDP_afterSplitting[INPUT_HOA][1], tmpHoaFilename, (payloadOffsetDelay) );
    changeDelayOfFile( tmpHoaFilename, tempFileNames_MDP_afterSplitting[INPUT_HOA][1], 0);

    if ( domainSwitchingDecisions.hoaDomains.PostHoaRendering == QMF_ANALYSIS ) {
      transformT2F(  strcat(tempFileNames_MDP_afterSplitting[INPUT_HOA][1], ".wav" ) , binary_DomainSwitcher, 0 );
      strcpy( tempFileNames_MDP_afterSplitting[INPUT_HOA][1], tmpHoaPathOutFilename );
    }
  }

  /* exit */

  ChannelPathExit ( &channelDataHandle );
  ObjectPathExit(&sceneDisplacementChannelDataHandle);
  {
    int i;
    for (i = 0; i < MAE_MAX_NUM_GROUPS; i++) {
      ObjectPathExit(&objectDataHandle[i]);
    }
  }
  SaocPathExit(&saocDataHandle);
  HoaPathExit(&hoaDataHandle);

  /**********************************/
  /* apply resampler and mixer      */
  /**********************************/

  if ( inputType & INPUT_CHANNEL )
  {
    int nSamplesOfDelay = 0;
    int formatConverterImplementationDelay = 0;

    if (hasSceneDisplacementChannels > 0)
    {
      transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][1],
                           preMixerFilename,
                           INPUT_CHANNEL_SD,
                           mpeg3daCpoType);

      if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
        error_3da = playOutCpo(mpeg3daCpoType,
                               coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                               coreDecoderProp.ati.nTruncSamples_right,
                               preMixerFilename,
                               wav_OutputFile,
                               "chPath");
        if (MPEG3DA_OK != error_3da) {
          return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      } else {

        nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.objectPath.pathDelay ;

        if (nSamplesOfDelay < 0) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (( moduleProcessingDomains.mixer == TIME_DOMAIN ) && (strcmp(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][2] + strlen(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][2]) - 4,".wav"))) {
          strcat(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][2], ".wav");
        }

        if ( bResamplerActive ) {
          nSamplesOfDelay -= 256;
        }

        changeDelayOfFile(preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][2], nSamplesOfDelay );
        error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL_SD][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
        if (error) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
    }
    if (disableFormatConverter == 0)
    {
      transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][1],
                           preMixerFilename,
                           INPUT_CHANNEL,
                           mpeg3daCpoType);

      if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
        error_3da = playOutCpo(mpeg3daCpoType,
                               coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                               coreDecoderProp.ati.nTruncSamples_right,
                               preMixerFilename,
                               wav_OutputFile,
                               (INPUT_CHANNEL != inputType) ? "chPath" : NULL);
        if (MPEG3DA_OK != error_3da) {
          return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      } else {

        if ( moduleProcessingDomains.formatConverter == QMF_DOMAIN ) {
          formatConverterImplementationDelay = FORMAT_CONVERTER_FD_DELAY - getFormatConverterDelay( profile, &moduleProcessingDomains);
        }
        else if ( moduleProcessingDomains.formatConverter == TIME_DOMAIN ) {
          formatConverterImplementationDelay = FORMAT_CONVERTER_TD_DELAY - getFormatConverterDelay( profile, &moduleProcessingDomains);
        }
        else if ( moduleProcessingDomains.formatConverter == STFT_DOMAIN ) {
          formatConverterImplementationDelay = -256;
          if ( moduleProcessingDomains.drc1channels == STFT_DOMAIN ) {
            formatConverterImplementationDelay = 0;
          }
        }

        if ( (int)rendererPathDelays.maxDelay < rendererPathDelays.channelPath.pathDelay + (int)( formatConverterImplementationDelay * resamplingRatio) ) {
          /* work around for format converter implementation delay */
          rendererPathDelays.maxDelay += rendererPathDelays.maxDelay - (rendererPathDelays.channelPath.pathDelay + (int)( formatConverterImplementationDelay * resamplingRatio));
          nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.channelPath.pathDelay - (int)( formatConverterImplementationDelay * resamplingRatio);
        }
        else {
          nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.channelPath.pathDelay - (int)( formatConverterImplementationDelay * resamplingRatio);
        }

        if (nSamplesOfDelay < 0) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }

        if (( moduleProcessingDomains.mixer == TIME_DOMAIN ) && (strcmp(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][2] + strlen(tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][2]) - 4,".wav"))) {
          strcat( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][2], ".wav" );
        }
        if ( bResamplerActive ) {
          nSamplesOfDelay -= 256;
        }

        changeDelayOfFile( preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][2], nSamplesOfDelay );

        error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_CHANNEL][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
        if (error) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
    }
  }

  if ( inputType & INPUT_OBJECT )
  {
    if (separateSetups > 1)
    {
      char tempAudioIn[FILENAME_MAX];
      char tempAudioOut[FILENAME_MAX];
      char tempGeo[FILENAME_MAX];
      char tempTarget[FILENAME_MAX];
      int i, j;
      for ( i = 0; i < separateSetups; i++)
      {
        int isObjectBasedSetup = 0;
        for (j = 0; j < MAE_MAX_NUM_ELEMENTS; j++)
        {
          if ((splittingInfoType[j] == 1) && (splittingInfoSetup[j] == i))
          {
            isObjectBasedSetup = 1;
          }
        }
        if (isObjectBasedSetup == 1)
        {
          int nSamplesOfDelay = 0;

          sprintf(tempAudioIn,"%s_setup_%i", tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], i);
          sprintf(tempAudioOut,"%s_setup_%i_filled", tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1], i);
          if ( moduleProcessingDomains.mixer == TIME_DOMAIN )
          {
            strcat( tempAudioIn, ".wav" );
            strcat( tempAudioOut, ".wav" );
          }

          /* add zero channels to fill up to setup_0 (target setup without sector exclusion -> maximum number of speakers) */
          sprintf (tempGeo, "tmpFile3Ddec_separate_output_setup_%i.txt", i);
          sprintf (tempTarget, "tmpFile3Ddec_separate_output_setup_0.txt");
          MDPreproc_fillWithZeroChannels(tempAudioIn, tempAudioOut, tempGeo, tempTarget, moduleProcessingDomains.mixer);

          transformAndResample(tempAudioOut,
                               preMixerFilename,
                               INPUT_OBJECT,
                               mpeg3daCpoType);

          if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
            error_3da = playOutCpo(mpeg3daCpoType,
                                   coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                                   coreDecoderProp.ati.nTruncSamples_right,
                                   preMixerFilename,
                                   wav_OutputFile,
                                   (INPUT_OBJECT != inputType) ? "objPath" : NULL);
            if (MPEG3DA_OK != error_3da) {
              return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
            }
          } else {

            nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.objectPath.pathDelay ;

            if (nSamplesOfDelay < 0) {
              return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
            }

            if ( getCurrentProcessingDomain(preMixerFilename) == TIME_DOMAIN ) {
              strcat(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], ".wav");
            }

            if ( bResamplerActive ) {
              nSamplesOfDelay -= 256;
            }

            changeDelayOfFile( preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], nSamplesOfDelay );

            error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
            if (error) {
              return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
            }
          }
        }
      }
    }
    else
    {
      int nSamplesOfDelay = 0;
      transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][1],
                           preMixerFilename,
                           INPUT_OBJECT,
                           mpeg3daCpoType);

      if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
        error_3da = playOutCpo(mpeg3daCpoType,
                               coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                               coreDecoderProp.ati.nTruncSamples_right,
                               preMixerFilename,
                               wav_OutputFile,
                               (INPUT_OBJECT != inputType) ? "objPath" : NULL);
        if (MPEG3DA_OK != error_3da) {
          return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      } else {

        nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.objectPath.pathDelay ;

        if (nSamplesOfDelay < 0) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }

        if ( getCurrentProcessingDomain(preMixerFilename) == TIME_DOMAIN ) {
          strcat(tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], ".wav");
        }

        if ( bResamplerActive ) {
          nSamplesOfDelay -= 256;
        }

        changeDelayOfFile( preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], nSamplesOfDelay );

        error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_OBJECT][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
        if (error) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
    }

    if ( hasDiffuseness == 1)
    {
      char inputDiffuse[FILENAME_MAX];
      int nSamplesOfDelay = 0;

      strcpy(inputDiffuse, tempFileNames_MDP_afterSplitting[INPUT_DIFFUSE][0]);

      transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_DIFFUSE][0],
                           preMixerFilename,
                           INPUT_DIFFUSE,
                           mpeg3daCpoType);

      if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
        error_3da = playOutCpo(mpeg3daCpoType,
                               coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                               coreDecoderProp.ati.nTruncSamples_right,
                               preMixerFilename,
                               wav_OutputFile,
                               (INPUT_CHANNEL != inputType) ? "objPath" : NULL);
        if (MPEG3DA_OK != error_3da) {
          return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      } else {

        nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.objectPath.pathDelay;

        if (nSamplesOfDelay < 0) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }

        if ( getCurrentProcessingDomain(preMixerFilename) == TIME_DOMAIN ) {
          strcat( tempFileNames_MDP_afterSplitting[INPUT_DIFFUSE][2], ".wav" );
          strcat( inputDiffuse, ".wav" );
        }
        if ( bResamplerActive ) {
          nSamplesOfDelay -= 256;
        }

        changeDelayOfFile( inputDiffuse, tempFileNames_MDP_afterSplitting[INPUT_DIFFUSE][2], nSamplesOfDelay );
        error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_DIFFUSE][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
        if (error) {
          return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
        }
      }
    }
  }

  if ( inputType & INPUT_SAOC )
  {
    int nSamplesOfDelay = 0;

    transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_SAOC][1],
                         preMixerFilename,
                         INPUT_SAOC,
                         mpeg3daCpoType);

    if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
      error_3da = playOutCpo(mpeg3daCpoType,
                             coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                             coreDecoderProp.ati.nTruncSamples_right,
                             preMixerFilename,
                             wav_OutputFile,
                             (INPUT_SAOC != inputType) ? "saocPath" : NULL);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {

      nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.saocPath.pathDelay;

      if (nSamplesOfDelay < 0) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      if ( getCurrentProcessingDomain(preMixerFilename) == TIME_DOMAIN ) {
        strcat(tempFileNames_MDP_afterSplitting[INPUT_SAOC][2], ".wav");
      }

      if ( bResamplerActive ) {
        nSamplesOfDelay -= 256;
      }

      changeDelayOfFile( preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_SAOC][2], nSamplesOfDelay );

      error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_SAOC][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );

      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  if ( inputType & INPUT_HOA )
  {
    int nSamplesOfDelay = 0;

    transformAndResample(tempFileNames_MDP_afterSplitting[INPUT_HOA][1],
                         preMixerFilename,
                         INPUT_HOA,
                         mpeg3daCpoType);

    if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
      error_3da = playOutCpo(mpeg3daCpoType,
                             coreDecoderProp.apr.nSamplesCoreCoderPreRoll + coreDecoderProp.ati.nTruncSamples_left,
                             coreDecoderProp.ati.nTruncSamples_right,
                             preMixerFilename,
                             wav_OutputFile,
                             (INPUT_HOA != inputType) ? "hoaPath" : NULL);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {

      nSamplesOfDelay = rendererPathDelays.maxDelay - rendererPathDelays.hoaPath.pathDelay;

      if (nSamplesOfDelay < 0) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      if ( getCurrentProcessingDomain(preMixerFilename) == TIME_DOMAIN ) {
        if (strcmp(tempFileNames_MDP_afterSplitting[INPUT_HOA][2] + strlen(tempFileNames_MDP_afterSplitting[INPUT_HOA][2]) - 4,".wav"))
        {
          strcat(tempFileNames_MDP_afterSplitting[INPUT_HOA][2], ".wav");
        }
      }

      if ( bResamplerActive ) {
        nSamplesOfDelay -= 256;
      }

      changeDelayOfFile( preMixerFilename, tempFileNames_MDP_afterSplitting[INPUT_HOA][2], nSamplesOfDelay );

      {
        int wavIOstate1 = 0;
        int wavIOstate2 = 0;
        int delay = 0;
        unsigned long length_in1 = 0;
        unsigned long length_in2 = 0;
        char temp1[FILENAME_MAX];
        char temp2[FILENAME_MAX];

        strcpy(temp1,tempFileNames_MDP_afterSplitting[INPUT_HOA][2]);
        strcpy(temp2, outputFile_Mixer);

        if ( getCurrentProcessingDomain(preMixerFilename) == QMF_DOMAIN )
        {
          strcat(temp1, "_real.qmf");
          strcat(temp2, "_real.qmf");
        }

        wavIOstate1 = getWavLength(temp1, &length_in1);
        wavIOstate2 = getWavLength(temp2, &length_in2);

        if ((0 != wavIOstate1) || (0 != wavIOstate2)) {
          length_in1 = length_in2 = 0;
        }

        if (length_in1 > length_in2) /* hoa file is longer than mixer output */
        {
          /* cut delay at end of hoa file(s) */
          delay = (length_in1 - length_in2);
          if ( getCurrentProcessingDomain(preMixerFilename) == QMF_DOMAIN )
          {
            strcpy(temp2,tempFileNames_MDP_afterSplitting[INPUT_HOA][2]);
            strcat(temp2, "_imag.qmf");

            error = changeDelayOfFile_End ( temp1, delay);
            error = changeDelayOfFile_End ( temp2, delay);
          }
          else
          {
            error = changeDelayOfFile_End ( temp1, delay);
          }
        }

        /* do mixing */
        error = execMixer ( tempFileNames_MDP_afterSplitting[INPUT_HOA][2], outputFile_Mixer, outputFile_Mixer, 0, 0, 0 );
      }
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }
  }

  /* stop processing if Cpo-1 is used */
  if (MPEG3DA_CPO_1 == mpeg3daCpoType) {
    return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /* QMF */
  if ( domainSwitchingDecisions.PostMixer == QMF_ANALYSIS ) {
    removeWavExtension( outputFile_Mixer );
    transformT2F( outputFile_Mixer, binary_DomainSwitcher, 0 );
  }
  else if ( domainSwitchingDecisions.PostMixer == QMF_SYNTHESIS && mixerDomain != TIME_DOMAIN ) {
    transformF2T( outputFile_Mixer, binary_DomainSwitcher, 0 );
  }
  else if ( domainSwitchingDecisions.PostMixer == STFT_SYNTHESIS && mixerDomain != TIME_DOMAIN ) {
    transformF2T( outputFile_Mixer, binary_DomainSwitcher, 1);
  }

  /**********************************/
  /* Binaural                       */
  /**********************************/
  if ( fDBinaural )
  {

    char outputFile_Binaural_delayAligned[FILENAME_MAX];

    /********************************************/
    /* FD binaural parametrization + rendering  */
    /********************************************/

    if (-1 == BinReadBRIRType) {
      return PrintErrorCode(default_error, error_depth, "Error switch \"-Bin\" not set.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    if (BinReadBRIRType == 0) /* read BRIRs from wave files */
    {

      /* read wave files, write high-level bitstream */

      error = FdBinauralBitstreamWriterInit ( &FdBinBswDataHandle, BinReadBRIRsFromLocation, binary_FdBinauralBitstreamWriter, logFileNames[9], WriteBRIRsBitstreamPath, BinReadBRIRsGeometryFromLocation);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralBitstreamWriterRun( FdBinBswDataHandle, verboseLevel );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      /* read high-level bitstream, call parametrization, write low-level parameter bitstream */

      strcpy(BinReadBRIRsFromLocation,WriteBRIRsBitstreamPath);
      error = FdBinauralParametrizationInit ( &FdBinParamDataHandle, BinReadBRIRsFromLocation, binary_FdBinauralParam, logFileNames[9], WriteBRIRsLowLevelBitstreamPath );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralParametrizationRun ( FdBinParamDataHandle, verboseLevel );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralParametrizationExit ( &FdBinParamDataHandle );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      if ( domainSwitchingDecisions.binauralDomains.PreFdBinaural == QMF_ANALYSIS ) {
        removeWavExtension( outputFile_Mixer );
        transformT2F( outputFile_Mixer, binary_DomainSwitcher, 0 );
      }

      /* read low-level parameter bitstream, call binaural renderer */

      strcpy(BinReadBRIRsFromLocation,WriteBRIRsLowLevelBitstreamPath);
      error = FdBinauralRendererInit( &FdBinRendDataHandle, outputFile_Mixer, outputFile_Binaural, outputCicp, BinReadBRIRType, BinReadBRIRsFromLocation, binary_FdBinauralRenderer, outputFile_inGeoRefLayout, logFileNames[9], profile, level );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralRendererRun( FdBinRendDataHandle, verboseLevel );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralRendererExit ( &FdBinRendDataHandle );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }

    else
    {
      if ( domainSwitchingDecisions.binauralDomains.PreFdBinaural == QMF_ANALYSIS ) {
        removeWavExtension( outputFile_Mixer );
        transformT2F( outputFile_Mixer, binary_DomainSwitcher, 0 );
      }

      error = FdBinauralRendererInit( &FdBinRendDataHandle, outputFile_Mixer, outputFile_Binaural, outputCicp, BinReadBRIRType, BinReadBRIRsFromLocation, binary_FdBinauralRenderer, outputFile_inGeoRefLayout, logFileNames[9], profile, level );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralRendererRun( FdBinRendDataHandle, verboseLevel );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = FdBinauralRendererExit ( &FdBinRendDataHandle );
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    }

    if ( domainSwitchingDecisions.binauralDomains.PostFdBinaural == QMF_SYNTHESIS ) {
      transformF2T( outputFile_Binaural, binary_DomainSwitcher, 0 );
    }

    /**********************************/
    /* DRC-2 Decoder                  */
    /**********************************/

    if (decoderProperties.postProcessingComponents.modeDrc2Channels != DISABLED) {
      error = execDrc2(asc.outputFrameLength, outputFile_Binaural, outputFile_Drc2, moduleProcessingDomains.drc2binaural);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_Drc2, outputFile_Binaural);
    }

    if ( domainSwitchingDecisions.binauralDomains.PostBinauralDrc2 == QMF_SYNTHESIS ) {
      transformF2T( outputFile_Drc2, binary_DomainSwitcher, 0);
    }

    /**********************************/
    /* Loudness Normalizer            */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bLoudnessNormalization ) {
      if (moduleProcessingDomains.loudnessNorm == TIME_DOMAIN) {
        strcat( outputFile_LoudnessNorm, ".wav" );
      } else {
        return PrintErrorCode(default_error, error_depth, "Time Domain required. This should not happen.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
    error = execLoudnessNorm(outputFile_Drc2, outputFile_LoudnessNorm, moduleProcessingDomains.loudnessNorm);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_LoudnessNorm, outputFile_Drc2);
    }

    /**********************************/
    /* Peak Limiter                   */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bPeakLimiter ) {
      error_3da = execPeakLimiter(outputFile_LoudnessNorm, outputFile_PeakLimiter, outputBitDepth);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error executing peak limiter.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_PeakLimiter, outputFile_LoudnessNorm);
    }

    error = compensateDelay(outputFile_PeakLimiter,
                            outputFile_Binaural_delayAligned,
                            coreDecoderProp.apr.nSamplesCoreCoderPreRoll);
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    /* copy to output */
    error_3da = copyToOutputFile(outputFile_Binaural_delayAligned,
                                 wav_OutputFile,
                                 0,
                                 mpeg3daCpoType,
                                 BIT_DEPTH_DEFAULT,
                                 coreDecoderProp.ati.nTruncSamples_left,
                                 coreDecoderProp.ati.nTruncSamples_right);
    if (MPEG3DA_OK != error_3da) {
      return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

  }
  else if (tDBinaural )
  {
    /********************************************/
    /* TD binaural parametrization + rendering  */
    /********************************************/

    if (BinReadBRIRType != -1) {
      return PrintErrorCode(default_error, error_depth, "Error switch \"-Bin\" deprecated.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    error = TdBinauralParametrizationInit ( &TdBinParamDataHandle, BinReadBRIRsFromLocation, binary_TdBinauralParam, logFileNames[9] );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    error = TdBinauralParametrizationRun ( TdBinParamDataHandle, verboseLevel  );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    error = TdBinauralParametrizationExit ( &TdBinParamDataHandle );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    if ( domainSwitchingDecisions.binauralDomains.PreTdBinaural == QMF_SYNTHESIS ) {
      transformF2T( outputFile_Mixer, binary_DomainSwitcher, 0 );
    }

    strcat( outputFile_Binaural, ".wav" );
    error = TdBinauralRendererInit( &TdBinRendDataHandle, outputFile_Mixer, outputFile_Binaural, inputType, outputCicp, fileOutGeo, binary_TdBinauralRenderer, logFileNames[9]);
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    error = TdBinauralRendererRun( TdBinRendDataHandle, verboseLevel );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    error = TdBinauralRendererExit ( &TdBinRendDataHandle );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    if ( domainSwitchingDecisions.binauralDomains.PostTdBinaural == QMF_ANALYSIS ) {
      transformT2F( outputFile_Binaural, binary_DomainSwitcher, 0);
    } else if ( domainSwitchingDecisions.binauralDomains.PostTdBinaural == STFT_ANALYSIS ) {
      transformT2F( outputFile_Binaural, binary_DomainSwitcher, 1);
    }

    /**********************************/
    /* DRC-2 Decoder                  */
    /**********************************/

    if (decoderProperties.postProcessingComponents.modeDrc2Channels != DISABLED) {
      error = execDrc2(asc.outputFrameLength, outputFile_Binaural, outputFile_Drc2, moduleProcessingDomains.drc2binaural);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_Drc2, outputFile_Binaural);
    }

    if ( domainSwitchingDecisions.binauralDomains.PostBinauralDrc2 == QMF_SYNTHESIS ) {
      transformF2T( outputFile_Drc2, binary_DomainSwitcher, 0);
    } else if ( domainSwitchingDecisions.binauralDomains.PostBinauralDrc2 == STFT_SYNTHESIS ) {
      transformF2T( outputFile_Drc2, binary_DomainSwitcher, 1);
    }

    /**********************************/
    /* Loudness Normalizer            */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bLoudnessNormalization ) {
      if (moduleProcessingDomains.loudnessNorm == TIME_DOMAIN) {
        strcat( outputFile_LoudnessNorm, ".wav" );
      } else {
        return PrintErrorCode(default_error, error_depth, "Time Domain required. This should not happen.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = execLoudnessNorm(outputFile_Drc2, outputFile_LoudnessNorm, moduleProcessingDomains.loudnessNorm);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_LoudnessNorm, outputFile_Drc2);
    }

    /**********************************/
    /* Peak Limiter                   */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bPeakLimiter ) {
      error_3da = execPeakLimiter(outputFile_LoudnessNorm, outputFile_PeakLimiter, outputBitDepth);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error executing peak limiter.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_PeakLimiter, outputFile_LoudnessNorm);
    }

    /* copy to output */
    error_3da = copyToOutputFile(outputFile_PeakLimiter,
                                 wav_OutputFile,
                                 0,
                                 mpeg3daCpoType,
                                 BIT_DEPTH_DEFAULT,
                                 coreDecoderProp.ati.nTruncSamples_left,
                                 coreDecoderProp.ati.nTruncSamples_right);
    if (MPEG3DA_OK != error_3da) {
      return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }
  else
  {
    char outputFile_Mixer_delayAligned[FILENAME_MAX];

    /**********************************/
    /* DRC-2 Decoder                  */
    /**********************************/

    if (domainSwitchingDecisions.PreDrc2 == QMF_ANALYSIS) {
      removeWavExtension(outputFile_Mixer);
      transformT2F(outputFile_Mixer, binary_DomainSwitcher, 0);
    } else if (domainSwitchingDecisions.PreDrc2 == STFT_ANALYSIS) {
      removeWavExtension(outputFile_Mixer);
      transformT2F(outputFile_Mixer, binary_DomainSwitcher, 1);
    }

    if (decoderProperties.postProcessingComponents.modeDrc2Channels != DISABLED) {
      error = execDrc2(asc.outputFrameLength, outputFile_Mixer, outputFile_Drc2, moduleProcessingDomains.drc2channels);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_Drc2, outputFile_Mixer);
    }

    if (domainSwitchingDecisions.PostDrc2 == QMF_SYNTHESIS) {
      transformF2T(outputFile_Drc2, binary_DomainSwitcher, 0);
    } else if (domainSwitchingDecisions.PostDrc2 == STFT_SYNTHESIS) {
      transformF2T(outputFile_Drc2, binary_DomainSwitcher, 1);
    }

    /**********************************/
    /* DRC-3 Decoder                  */
    /**********************************/

    if (decoderProperties.endOfChainComponents.bsHasDrc3) {
      error = execDrc3(asc.outputFrameLength, outputFile_Drc2, outputFile_Drc3);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_Drc3, outputFile_Drc2);
    }

    /**********************************/
    /* Loudness Normalizer            */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bLoudnessNormalization ) {
      if (moduleProcessingDomains.loudnessNorm == TIME_DOMAIN) {
        strcat( outputFile_LoudnessNorm, ".wav" );
      } else {
        return PrintErrorCode(default_error, error_depth, "Time Domain required. This should not happen.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }

      error = execLoudnessNorm(outputFile_Drc3, outputFile_LoudnessNorm, moduleProcessingDomains.loudnessNorm);
      if (error) {
        return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_LoudnessNorm, outputFile_Drc3);
    }

    /**********************************/
    /* Peak Limiter                   */
    /**********************************/

    if ( decoderProperties.endOfChainComponents.bPeakLimiter ) {
      error_3da = execPeakLimiter(outputFile_LoudnessNorm, outputFile_PeakLimiter, outputBitDepth);
      if (MPEG3DA_OK != error_3da) {
        return PrintErrorCode(error_3da, error_depth, "Error executing peak limiter.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
      }
    } else {
      strcpy(outputFile_PeakLimiter, outputFile_LoudnessNorm);
    }

    /* compensate delay */
    error = compensateDelay(outputFile_PeakLimiter,
                            outputFile_Mixer_delayAligned,
                            coreDecoderProp.apr.nSamplesCoreCoderPreRoll );
    if (error) {
      return PrintErrorCode(default_error, error_depth, "decoding error.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

    /* copy to output */
    error_3da = copyToOutputFile(outputFile_Mixer_delayAligned,
                                 wav_OutputFile,
                                 0,
                                 mpeg3daCpoType,
                                 BIT_DEPTH_DEFAULT,
                                 coreDecoderProp.ati.nTruncSamples_left,
                                 coreDecoderProp.ati.nTruncSamples_right);
    if (MPEG3DA_OK != error_3da) {
      return PrintErrorCode(error_3da, error_depth, "Error in reproduction chain.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /**********************************/
  /* clean up                       */
  /**********************************/
  removeTempFiles(recycleLevel);

  return MPEG3DA_OK;
}

static MPEG3DA_RETURN_CODE setup3daDecoder(char* globalPath, int* error_depth, VERBOSE_LEVEL verboseLvl)
{
  int error      = 0;
  int nBinaries  = 0;
  char* basename = NULL;

  fprintf ( stdout, "\n" );
  fprintf ( stdout, "::-----------------------------------------------------------------------------\n::    " );
  fprintf ( stdout, "Setup decoder...\n");
  fprintf ( stdout, "::-----------------------------------------------------------------------------\n" );
  fprintf ( stdout, "\n" );

#ifdef _WIN32
    GetModuleFileName(NULL, globalPath, 3 * FILENAME_MAX);
#else
#ifdef __linux__
    readlink("/proc/self/exe", globalPath, 3 * FILENAME_MAX);
#endif

#ifdef __APPLE__
    uint32_t tmpStringLength = 3 * FILENAME_MAX;
    if ( _NSGetExecutablePath( globalPath, &tmpStringLength ) ) {
      return PrintErrorCode(MPEG3DA_ERROR_3DA_INIT, error_depth, "Error retrieving path(s).", "Please use cfg-Files for your binary paths.", verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
#endif
#endif

    basename = strstr(globalPath, "3DAudioDecoder");

    if ( basename == 0 ) {
      return PrintErrorCode(MPEG3DA_ERROR_3DA_INVALIDEXE, error_depth, "Error initializing MPEG 3D Audio Decoder.", "Binary must be named 3DAudioDecoder[.exe].", verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
    else {
      strcpy(basename, "\0");
    }

    /* set the binary names */
    fprintf (stdout, "Collecting executables...\n");
    error = setBinaries ( globalPath, &nBinaries );
    if ( error ) {
      return PrintErrorCode(MPEG3DA_ERROR_3DA_MISSINGEXE, error_depth, "Error locating 3D Audio decoder binaries.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }

  if (VERBOSE_LVL1 <= verboseLvl) {
    fprintf ( stdout, "\n>>    %d executables loaded!\n", nBinaries);
  }

  return MPEG3DA_OK;
}

/* ###################################################################### */
/* ##                 MPEG 3DA decoder public functions                ## */
/* ###################################################################### */
/* print 3da error code */
MPEG3DA_RETURN_CODE PrintErrorCode( const MPEG3DA_RETURN_CODE mpeg3da_code, int* error_depth, const char* error_msg, const char* error_hlp, const VERBOSE_LEVEL verboseLvl, const char* file, const char* func, const int line )
{
  MPEG3DA_RETURN_CODE error = mpeg3da_code;
  int error_hlp_present     = 0;

  if (NULL != error_hlp) {
    if (strncmp("\0", error_hlp, 1)) {
      error_hlp_present = 1;
    }
  }

  if (NULL != error_msg && NULL != error_depth && MPEG3DA_OK != mpeg3da_code) {
    /* only print deepest error */
    if (*error_depth == 0) {
      if (0 == error_hlp_present) {
        /* print error message: */
        fprintf ( stderr, "\n%s (Code: %d)\n\tTry -v for more help.\n\n", error_msg, error);
      } else {
        /* print error message + extended help: */
        fprintf ( stderr, "\n%s (Code: %d)\n\t%s\n\n", error_msg, error, error_hlp);
      }
      switch (verboseLvl) {
        case VERBOSE_LVL1:
          /* add some more information */
          fprintf ( stderr, "Error information:\n\tCode: %d\n\tfunc: %s()\n\tline: %d\n\n", error, func, line);
          (*error_depth)++;
          break;
        case VERBOSE_LVL2:
          /* add even more information */
          fprintf ( stderr, "Error information:\n\tCode: %d\n\tfile: %s\n\tfunc: %s()\n\tline: %d\n\n", error, file, func, line);
          break;
        default:
          (*error_depth)++;
      }
    }
  }

  return error;
}

/* get number of channels corresponding to a CICP ChannelConfiguration */
int getNumberOfCicpChannels( const CICP_FORMAT cicpIdx, int* nChannels )
{
  int error = 0;
  int nChannels_local = -1;

  if (cicpIdx >= CICP_ANY && cicpIdx < CICP_RESERVED) {
    if (cicpIdx == cicp_channeConfigTable[cicpIdx].cicpIdx) {
      nChannels_local = cicp_channeConfigTable[cicpIdx].nChannels;
    } else {
      error = -1;
    }
  }

  if (0 == error && NULL != nChannels) {
    *nChannels = nChannels_local;
  } else {
    error = -1;
  }

  return error;
}

/* checks if filename contains a .wav suffix */
int checkIfWavExtension( char* filename )
{
  char * wavSuffix = strstr( filename, ".wav" );

  if ( wavSuffix == NULL ) {
    return 0;
  }
  else {
    return -1;
  }
}

/* removes .wav suffix from filename */
int removeWavExtension( char* filename )
{
  char * wavSuffix = strstr(filename, ".wav" );

  if ( wavSuffix != NULL ) {
    /* remove the file extension */
    strcpy( wavSuffix, "\0" );
  }
  else {
    return -1;
  }

  return 0;
}

/* call binary - wrapper for system() */
int callBinary(const char* exe_2call, const char* exe_description, const VERBOSE_LEVEL verboseLvl, const unsigned int hideErrorMsg) {
  int retError = 0;

  fprintf(stdout, "Calling %s...\n", exe_description);

  if (VERBOSE_LVL1 <= verboseLvl) {
    fprintf(stdout, ">>     command:\n%s\n\n", exe_2call);
  }

  retError = system(exe_2call);
  if (retError == -1) {
    fprintf(stderr, "\nError starting %s: %s\n", exe_2call, strerror(errno));
  } 
#  if defined(_POSIX_C_SOURCE) || (defined(__APPLE__) && defined(__MACH__))
    else {
    retError = WEXITSTATUS(retError);
  }
#  endif /* system() is waitpid(2) based */

  if (retError && !hideErrorMsg) {
    fprintf(stderr, "\nError running %s\n", exe_description);
  }

  return retError;
}

/* get delay of the core decoder in constant delay mode */
int GetCoreCoderConstantDelay ( void )
{
  int coreCoderConstantDelay = 0;

  if ( profile == PROFILE_LOW_COMPLEXITY || profile == PROFILE_BASELINE ) {
    return coreCoderConstantDelay;
  }


  if ( asc.receiverDelayCompensation == 0 ) {

    if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
      coreCoderConstantDelay = 0;
      return coreCoderConstantDelay;
    }

    switch ( asc.maxStereoConfigIndex ) {

      case -1:
      case  0:
      case  1:
      case  2:
        coreCoderConstantDelay = MIN_SBR_MPS_OFFSET_DELAY;
        break;

      case 3:
        coreCoderConstantDelay = MAX_SBR_MPS_OFFSET_DELAY;
        break;

      default:
        return -1;
    }
  }
  else {
    coreCoderConstantDelay = MAX_SBR_MPS_OFFSET_DELAY;
  }

  if ( moduleProcessingDomains.coreCoder == TIME_DOMAIN ) {
    coreCoderConstantDelay += CORE_QMF_SYNTHESIS_DELAY;
  }

  return coreCoderConstantDelay;
}

/* Apply QMF_ANALYSIS */
MPEG3DA_RETURN_CODE transformT2F( char* inputFilenameTimeWithoutExtension, char* binary_DomainSwitcher, int stftFlag ){

  char callCmd[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  /* check if there is a file to transform */
  if ( !( checkIfFileExists ( inputFilenameTimeWithoutExtension, "wav" ) ) ){
    fprintf ( stderr, "\n\nError applying DomainSwitcher: %s.%s does not exist.\n", inputFilenameTimeWithoutExtension, ".wav" );
    return MPEG3DA_ERROR_DOMAINSW_GENERIC;
  }

  /* check if there already was a transform */
  if ( ( checkIfFDFilesExist ( inputFilenameTimeWithoutExtension, stftFlag ) ) ){
    fprintf ( stderr, "\n\nError applying DomainSwitcher: %s already exists.\n", inputFilenameTimeWithoutExtension );
    return MPEG3DA_ERROR_DOMAINSW_NOSWITCH;
  }

  sprintf ( callCmd, "%s -if %s.wav -of %s ", binary_DomainSwitcher, inputFilenameTimeWithoutExtension, inputFilenameTimeWithoutExtension );
  if (stftFlag) {
    strcat(callCmd, " -stft");
  }
  sprintf(tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat(callCmd, tmpString);

  if (0 != callBinary(callCmd, "DomainSwitcher in transformT2F", verboseLevel, 0)) {
    return MPEG3DA_ERROR_DOMAINSW_EXEC;
  }

  return MPEG3DA_OK;
}

/* Apply QMF_SYNTHESIS */
MPEG3DA_RETURN_CODE transformF2T( char* inputFilenameFreqWithoutExtension, char* binary_DomainSwitcher, int stftFlag ){

  char callCmd[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];

  /* check if there are qmf files to transform */
  if ( !( checkIfFDFilesExist ( inputFilenameFreqWithoutExtension, stftFlag ) ) ){
    if(stftFlag){
      fprintf ( stderr, "\n\nError applying DomainSwitcher: %s_real.stft and/or %s_imag.stft do not exist.\n", inputFilenameFreqWithoutExtension, inputFilenameFreqWithoutExtension);
    }
    else {
      fprintf ( stderr, "\n\nError applying DomainSwitcher: %s_real.qmf and/or %s_imag.qmf do not exist.\n", inputFilenameFreqWithoutExtension, inputFilenameFreqWithoutExtension);
    }
    return MPEG3DA_ERROR_DOMAINSW_GENERIC;
  }

  /*check if there already was a transform*/
  if( checkIfFileExists( inputFilenameFreqWithoutExtension, "wav" ) ){
    fprintf ( stderr, "\n\nError applying DomainSwitcher: %s already exists\n", inputFilenameFreqWithoutExtension );
    return MPEG3DA_ERROR_DOMAINSW_NOSWITCH;
  }

  sprintf ( callCmd, "%s -if %s -of %s.wav -outputBitdepth %d ", binary_DomainSwitcher, inputFilenameFreqWithoutExtension, inputFilenameFreqWithoutExtension, mpeg3daBitDepth );
  if (stftFlag) {
    strcat(callCmd, " -stft");
  }
  sprintf(tmpString, " >> %s 2>&1", logFileNames[0]);
  strcat(callCmd, tmpString);

  if (0 != callBinary(callCmd, "DomainSwitcher in transformF2T", verboseLevel, 0)) {
    return MPEG3DA_ERROR_DOMAINSW_EXEC;
  }

  strcat ( inputFilenameFreqWithoutExtension, ".wav");

  return MPEG3DA_OK;
}

/* ###################################################################### */
/* ##                  MPEG 3DA decoder main function                  ## */
/* ###################################################################### */
int main ( int argc, char* argv[] )
{
  MPEG3DA_RETURN_CODE error_3da = MPEG3DA_OK;
  int error                     = 0;
  int error_depth               = 0;
  int versionMajor              = 0;
  int versionMinor              = 0;
  int versionRMbuild            = 0;
  int bTriedMhmDecoding         = 0;
  ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET compatibleProfileLevelSet;
  ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET mp4LevelCompatibleProfileLevelSet;
  int compatibleProfileLevelSetAvailable = 0;
  ascparserUsacDecoderConfig  usacDecoderConfig;

  recycleLevel                  = RECYCLE_ACTIVE;
  verboseLevel                  = VERBOSE_NONE;
  debugLevel                    = DEBUG_NORMAL;
  bDelayAlignMpegTestpoints     = 0;
  bRemoveConstantDecoderDelay   = 1;

  /* sampling rate at mixer is 48 kHz per default */
  targetSamplingRate            = MPEG3DA_48000;
  disableResampling             = 0;
  mixerDomain                   = TIME_DOMAIN;

  /* bitdepth of the decoder output audio file */
  outputBitDepth                = BIT_DEPTH_DEFAULT;
  mpeg3daBitDepth               = BIT_DEPTH_32_BIT;

  /* output mode type */
  mpeg3daCpoType                = MPEG3DA_DEFAULT_OUTPUT;

  fDBinaural                    = 0;
  tDBinaural                    = 0;

  speakerMisplacement           = 0;
  downmixConfigAvailable        = 0;
  passiveDownmixFlag            = 0;
  phaseAlignStrength            = 0;
#if IAR
  immersiveDownmixFlag          = 0;
#endif
  matrixBitSize                 = 0;
  downmixId                     = -1;

  memset(&compatibleProfileLevelSet, 0, sizeof(ASCPARSER_COMPATIBLE_PROFILE_LEVEL_SET));
  memset(&usacDecoderConfig, 0, sizeof(ascparserUsacDecoderConfig));
  memset(omfFile, 0, sizeof(omfFile));
  memset(cocfgFile, 0, sizeof(cocfgFile));

  /* get version number */
  error = getVersion(MPEG3DA_USAC_VERSION_NUMBER,
                     &versionMajor,
                     &versionMinor,
                     &versionRMbuild);
  if (0 != error) {
    return PrintErrorCode(MPEG3DA_ERROR_3DA_GENERIC, &error_depth, "Error intializing decoder.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /* parse command line parameters */
  error = PrintCmdlineHelp(argc,
                           argv[0],
                           versionMajor,
                           versionMinor);
  if ( error ) {
    return PrintErrorCode(MPEG3DA_OK, &error_depth, NULL, NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /* init DRC params */
  error = setDrcParams(0);
  if ( error ) {
    return PrintErrorCode(MPEG3DA_ERROR_DRC_INIT, &error_depth, "Error initializing DRC parameters.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  error = GetCmdline ( argc, argv );
  if ( error ) {
    return PrintErrorCode(MPEG3DA_ERROR_3DA_INVALIDCMD, &error_depth, "Error initializing MPEG 3D Audio decoder.", "Invalid command line parameters.", verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }


  /********************** Switch group interaction via commandline switch **********************/
  if ((chooseSGmember >= 0) && (hasElementInteraction > 0)) /* element interaction takes priority over switch group interaction via commandline */
  {
    chooseSGmember = -1;
  }

  if (chooseSGmember >= 0)
  {
    char SgMem[FILENAME_MAX];
    sprintf ( SgMem, "_sgmember%d.wav", chooseSGmember );
    removeWavExtension(wav_OutputFile);
    strcat(wav_OutputFile,SgMem);
  }
  /********************** end of switch group interaction **********************/

  /********************** Preset interaction via commandline switch **********************/
  if ((selectedPresetID >= 0) && (hasElementInteraction > 0)) /* element interaction takes priority over preset interaction via commandline */
  {
    selectedPresetID = -1;
  }
  /********************** end of preset interaction **********************/

  /* Check if both binaural renderers are active */
  if ( (fDBinaural == 1) & (tDBinaural == 1) )
  {
    return PrintErrorCode(MPEG3DA_ERROR_BINAURAL_INIT, &error_depth, "Error initializing binaural renderer.", "Please use either -tDBinaural or -fDBinaural parameter.", verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  if (VERBOSE_LVL1 <= verboseLevel) {
    fprintf ( stdout, "\n" );
    fprintf ( stdout, "::-----------------------------------------------------------------------------\n::    " );
    fprintf ( stdout, "Initializing I/O files...\n");
    fprintf ( stdout, "::-----------------------------------------------------------------------------\n" );
    fprintf ( stdout, "\n" );
    fprintf ( stdout, "Input bitstream:\n      %s\n\n", mp4_InputFile);
    fprintf ( stdout, "Output audio file:\n      %s\n", wav_OutputFile);

    if (checkIfFileExists(wav_OutputFile, NULL) && !monoWavOut) {
      fprintf ( stderr, "\nWarning: Output File already exists: %s\n", wav_OutputFile );
    }
    fprintf ( stdout, "\n" );
  }

  if ( mixerDomain == NOT_DEFINED ) {
    return PrintErrorCode(MPEG3DA_ERROR_MIXER_INIT, &error_depth, "Error initializing mixer module.", "Invalid value for mixerDomain.", verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  /* Check if windows or linux commands should be used */
  bUseWindowsCommands = useWindowsCommands();

  MakeLogFileNames(wav_OutputFile,
                   bUseWindowsCommands,
                   verboseLevel);

  /* Remove all old temp files */
  removeTempFiles(RECYCLE_ACTIVE);

  redo_decoding_with_mhas_file:

  /* Parse ASC and ASI */
  error_3da = Get_Config_StaticMetadata(mp4_InputFile,
                                        &asc,
                                        &asi,
                                        &compatibleProfileLevelSet,
                                        &compatibleProfileLevelSetAvailable,
                                        &usacDecoderConfig,
                                        &hasASI,
                                        &signalGroupConfig,
                                        &signalGroupInfo,
                                        &signalGroupInfoAvailable,
                                        &enhancedObjectMetadataConfig, 
                                        &productionMetadataConfig, 
                                        verboseLevel,
                                        &error_depth);
  if ( MPEG3DA_OK != error_3da ) {
    return error_3da;
  }

  error_3da = setup3daDecoder(globalBinariesPath, &error_depth, verboseLevel);
  if ( MPEG3DA_OK != error_3da ) {
    return error_3da;
  }

  /* if no config is found in iso-ff, it can be a mhm file */
  if (asc.usacSamplingFreq == 0 && asc.asc_isValid == 1 && bTriedMhmDecoding == 0) {
    FILE* mhasFilenameTxt = NULL;
    char callMhmCoreDecoder[3 * FILENAME_MAX] = {0};

    bTriedMhmDecoding = 1;

    fprintf(stdout, "\n::-----------------------------------------------------------------------------\n::    ");
    fprintf(stdout, "Extracting mhas file from mp4-mhm file and proceed with mhas file...\n");
    fprintf(stdout, "::-----------------------------------------------------------------------------\n\n");

    sprintf(callMhmCoreDecoder, "%s -if %s -of tmpFile3Ddec_core_output_tmp.wav >> %s 2>&1", binary_CoreDecoder, mp4_InputFile, logFileNames[0]);
    callBinary(callMhmCoreDecoder, "core decoder", verboseLevel, 1);

    mhasFilenameTxt = fopen("tmpFile3Ddec_mhm_mhas_filename.txt", "r");
    if (mhasFilenameTxt) {
      fscanf(mhasFilenameTxt, "%s", mhasFilename);
      fclose(mhasFilenameTxt);
      strncpy(mp4_InputFile, mhasFilename, FILENAME_MAX);

      /* Store Compatible Profile Levels on MP4 level for later comparison with values in bitstream */
      mp4LevelCompatibleProfileLevelSet = asc.compatibleProfileLevelSet;

      goto redo_decoding_with_mhas_file;
    }
  }

  /* Compare for mhm files the Compatible Profile Level values in mp4 file and bitstream */
  if (bTriedMhmDecoding) {
    if (compatibleProfileLevelSetAvailable == 1) {
      if (mp4LevelCompatibleProfileLevelSet.numCompatibleSets == 0) {
        fprintf(stderr, "\n\nWarning: Compatible Profile Level Set is only present in bitstream!\n\n");
      }
      else {
        unsigned int j;
        for (j = 0; j < compatibleProfileLevelSet.numCompatibleSets; j++) {
          if (compatibleProfileLevelSet.CompatibleSetIndication[j] != mp4LevelCompatibleProfileLevelSet.CompatibleSetIndication[j]) {
            fprintf(stderr, "\n\nWarning: compatibleSetIndication mismatch!\n\tmp4CompatibleSetIndication\t0x%X\n\tmpegh3daCompatibleSetIndication\t0x%X!\n\n", mp4LevelCompatibleProfileLevelSet.CompatibleSetIndication[j], compatibleProfileLevelSet.CompatibleSetIndication[j]);
          }
        }
      }
    }
    else if (mp4LevelCompatibleProfileLevelSet.numCompatibleSets > 0 && !compatibleProfileLevelSetAvailable) {
        return MPEG3DA_ERROR_PCT_INVALID_STREAM;
    }
  }

  /* Disable resampling if cpo output is used */
  if ((MPEG3DA_DEFAULT_OUTPUT != mpeg3daCpoType) && (MPEG3DA_SKIP_EARCON != mpeg3daCpoType)) {
    disableResampling = 1;
  }

  /* Check input type */
  error = checkInputType();

  /* Check input channel format*/
  error = checkInputFormat();
  if ( error ) {
    return PrintErrorCode(MPEG3DA_ERROR_INPUTFORMAT, &error_depth, "Error detected in input format.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
  }

  if ((outputCicp == CICP_INVALID) || (outputCicp ==  CICP_ANY)) {
    /* no cicp set from cmdline */
    outputCicp = inputCicpReferenceLayout;

    if (outputCicp == CICP_FROM_GEO) {
      strncpy(fileOutGeo, outputFile_inGeoRefLayout, strlen(outputFile_inGeoRefLayout) + 1);
    }
  }
  if (asc.referenceLayout.speakerLayoutID == 3) {
    outputCicp = CICP_CONTRIBUTION;
  }

  /* if a downmixConfig is available look for a fitting downmix matrix */
  if( downmixConfigAvailable )
  {
    if( outputCicp == CICP_FROM_GEO )
    {
      int numChannelsTmp;
      int numLFEsTmp;
      CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[32];

      cicp2geometry_get_geometry_from_file( fileOutGeo, outGeo, &numChannelsTmp, &numLFEsTmp );
      setDownmixIdFromRuleSet(outGeo, numChannelsTmp, numLFEsTmp, downmixConfig, &downmixId);

    } else if( outputCicp > 0 )
    {
      unsigned int i;
      for( i = 0; i < downmixConfig.downmixIdCount; i++ )
      {
        if( downmixConfig.downmixMatrix[i].CICPspeakerLayoutIdx == outputCicp )
        {
          downmixId = downmixConfig.downmixMatrix[i].DMX_ID;
        }
      }
    }
    /* Check if there is a downmix matrix for the calculated downmix id */
    {
      unsigned int i;
      for( i = 0; i < downmixConfig.downmixIdCount; i++ )
      {
        if( downmixId == downmixConfig.downmixMatrix[i].DMX_ID )
        {
          if( downmixConfig.downmixMatrix[i].downmixType == 1 )
          {
            matrixBitSize = downmixConfig.downmixMatrix[i].DmxMatrixLenBits[0];
            downmixMatrix = (char*)downmixConfig.downmixMatrix[i].DownmixMatrix[0];
          }
        }
      }
    }
  }


  if (1 == asc.asc_isValid) {
    /* check whether profile constraints are fulfilled. Exit if not. */
    error_3da = profileConstraintsTerminator(targetSamplingRate,
                                             &profile,
                                             &level,
                                             &compatibleProfileLevelSet,
                                             compatibleProfileLevelSetAvailable,
                                             &downmixConfig,
                                             &usacDecoderConfig,
                                             &error_depth,
                                             verboseLevel);
    if ( MPEG3DA_OK != error_3da ) {
      return error_3da;
    }

    /* set DRC params */
    error = setDrcParams(1);
    if ( error ) {
      return PrintErrorCode(MPEG3DA_ERROR_DRC_INIT, &error_depth, "Error initializing DRC parameters.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  if (asc.asc_isValid == 1 && asc.referenceLayout.speakerLayoutID == 3){
    /* contribution mode */
    error_3da = execContributionMode(&error_depth, verboseLevel);
    if ( MPEG3DA_OK != error_3da ) {
      return PrintErrorCode(error_3da, &error_depth, "Error executing MPEG 3D Audio decoder (contribution mode).", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  } else {
    /* full decode */
    error_3da = execFullDecode(&error_depth, verboseLevel);
    if ( MPEG3DA_OK != error_3da ) {
      return PrintErrorCode(error_3da, &error_depth, "Error executing MPEG 3D Audio decoder.", NULL, verboseLevel, MPEG3DA_ERROR_POSITION_INFO);
    }
  }

  /**********************************/
  /* clean up                       */
  /**********************************/
  removeTempFiles(recycleLevel);

  fprintf ( stdout, "\n>>    Decoding successfully completed!\n\n" );

  return MPEG3DA_OK;
}
