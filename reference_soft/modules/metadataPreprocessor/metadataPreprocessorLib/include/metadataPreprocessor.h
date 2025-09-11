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

Copyright (c) ISO/IEC 2015.

***********************************************************************************/

#include <stdio.h>
#include "elementInteractionInterfaceLib_API.h"
#include "localSetupInformationInterfaceLib_API.h"
#include "sceneDisplacementInterfaceLib_API.h"
#include "ascparser.h"
#include "oam_read.h"
#include "wavIO.h"
#include "qmflib.h"
#include "writeonlybitbuf.h"

#ifndef METADATA_PREPROCESSOR
  #define METADATA_PREPROCESSOR
  #define LOUDNESS_COMPENSATION

  #define MAE_DIFFUSE_DECORR_LENGTH 256

  #ifndef CONTENT_TYPE_DEF  
    #define CONTENT_TYPE_DEF
    typedef enum _CONTENT_TYPE
    {
      INPUT_INVALID     =  0,
      INPUT_CHANNEL     =  1,
      INPUT_OBJECT      =  2,
      INPUT_SAOC        =  4,
      INPUT_HOA         =  8,
      INPUT_DIFFUSE     =  9,
      INPUT_CHANNEL_SD  = 10
    } INPUT_TYPE;
  #endif

  #ifndef CICP_FORMAT_DEF
    #define CICP_FORMAT_DEF
    typedef enum _CICP_FORMAT
    {
      CICP_INVALID  = -2,
      CICP_FROM_GEO = -1,
      CICP_ANY      =  0,
      CICP_7_1_ELEV =  14,
      CICP_22_2     =  13,
      CICP_7_1_REAR =  12,
      CICP_7_1_FRONT=  7,
      CICP_5_1      =  6,
      CICP_2_0      =  2,
      CICP_1_0      =  1

    } CICP_FORMAT;
  #endif

  typedef struct MP_InteractionConfigStruct
  {
    float* gainModified_LastFrame;
    float* azimuthModified;
    float* elevationModified;
    int* onOffStatusModified;
    int* routeToWireID;
    int* doProcess; 
    int* screenRelatedObjects;

    StructOamMultidata* oamSampleModified;
    StructOamMultidata* oamSampleModified_Divergence;
    int origNumGroupMembers[MAE_MAX_NUM_GROUPS];
    int divergence_ASImodified;

    float* gainModifiedGroup;
    float* gainModifiedSingleObject;
    float* distanceModified;

    int diffuse_enableProcessing;
    float diffuse_decorrFilters[MAE_MAX_NUM_SPEAKERS][MAE_DIFFUSE_DECORR_LENGTH];
    float diffuse_decorrStates[MAE_MAX_NUM_SPEAKERS][MAE_DIFFUSE_DECORR_LENGTH];
    int diffuse_filterLength;
    int diffuse_counter;
    int diffuse_compensateDelay;
    QMFLIB_POLYPHASE_SYN_FILTERBANK ** diffuse_QMFsynthesis;
    QMFLIB_POLYPHASE_ANA_FILTERBANK ** diffuse_QMFanalysis;

    int* listOAM;
    int oamCnt;
    int numObjects_in;
    int numObjects_out;
    int numSpeakers;

    int interactionDataToBeApplied;

    int interactionType;
    int defaultPresetMode;
    int numDecodedGroups;
    int numSwitchGroups;
    int enableScreenRelatedProcessing;
    int hasScreenSize;
    int numElements_in;
    int numElements_out;
    float loudnessCompensationGain;
    float** buffer_temp;
    float*** buffer_temp_real;
    float*** buffer_temp_imag;

    H_ELEMENT_INTERACTION_DATA ElementInteractionData;
    H_LOCAL_SCREEN_INFO localScreen;


  } INTERACT_MP_CONFIG, *H_INTERACT_MP_CONFIG;

  typedef struct MP_ObjectOutputInterfaceConfig
  {
    FILE *outFile;
    char outPath[FILENAME_MAX];

    /* config */
    int audioByteDepth;
    int audioSamplingRate;
    int audioFrameLength;
    int oamFramesPerAudioFrame;

    /*frame-wise config */
    int framewise_frameNumber;
    int framewise_numOutObjects;

  } OBJ_OUT_INTERFACE_CONFIG, *H_OBJ_OUT_INTERFACE_CONFIG;

  typedef struct MP_AudioConfigStruct
  {
    char wavIO_inpath_real[FILENAME_MAX];
    char wavIO_inpath_imag[FILENAME_MAX];
    char wavIO_inpath[FILENAME_MAX];
    char wavIO_outpath_real[FILENAME_MAX];
    char wavIO_outpath_imag[FILENAME_MAX];
    char wavIO_outpath[FILENAME_MAX];

    FILE *fwavIn_real;
    FILE *fwavIn_imag;
    FILE *fwavIn;

    FILE *fwavOut_real;
    FILE *fwavOut_imag;
    FILE *fwavOut;

    WAVIO_HANDLE hWavIO_real; 
    WAVIO_HANDLE hWavIO_imag;
    WAVIO_HANDLE hWavIO;

    unsigned int nInChannels;
    unsigned int nOutChannels;
    unsigned int InSampleRate;
    unsigned int InBytedepth;
    unsigned long nTotalSamplesPerChannel;
    int nSamplesPerChannelFilled;

    int decode_wav;
    int decode_qmf;
    int write_qmf;
    int write_wav;

    float **audioTdBuffer; 
    float ***audioQmfBuffer_real;
    float ***audioQmfBuffer_imag;

    float **audioTdBuffer_divergence; 
    float ***audioQmfBuffer_divergence_real;
    float ***audioQmfBuffer_divergence_imag;

    float ** audioQmfBuffer_real_tmp;
    float ** audioQmfBuffer_imag_tmp;

    int numTimeslots;
    int audio_blocksize;

    unsigned int samplesReadPerChannel;
    unsigned int isLastFrame;
    unsigned int nZerosPaddedBeginning;
    unsigned int nZerosPaddedEnd;
    unsigned int samplesToWritePerChannel;
    unsigned long nTotalSamplesWrittenPerChannel;
    unsigned int samplesWrittenPerChannel;

    unsigned int nFrames;

    WAVIO_HANDLE hWavIODiffuse;
    WAVIO_HANDLE hWavIODiffuse_real;
    WAVIO_HANDLE hWavIODiffuse_imag;
    char wavIO_outpathDiffuse_real[FILENAME_MAX];
    char wavIO_outpathDiffuse_imag[FILENAME_MAX];
    char wavIO_outpathDiffuse[FILENAME_MAX];
    FILE* fwavOut_diffuse_real;
    FILE* fwavOut_diffuse_imag;
    FILE* fwavOut_diffuse;
    int nSpeakers;

  } AUDIO_MP_CONFIG, *H_AUDIO_MP_CONFIG;


typedef struct MP_OamConfigStruct
  {
    FILE *oamInFile;
    FILE *oamOutFile;
    int oam_blocksize;
    StructOamMultidata* oamSample_in;
    char oamInpath[FILENAME_MAX];
    char oamOutpath[FILENAME_MAX];
    int oam_frames;
    int oam_mode;
    unsigned int oam_version;
    int hasDynObjectPrio;
    int hasUniformSpread;
    int oam_hasBeenDecoded;

    uint16_t num_objects_input;
    uint16_t num_objects_output;

    char oamOutpath_SDchannels[FILENAME_MAX];
    FILE *oamOutFile_SDchannels;
    int num_SDchannels;
    StructOamMultidata* oamSample_SDchannels_out;

  } OAM_MP_CONFIG, *H_OAM_MP_CONFIG;

typedef struct MP_enhObjMdFrameStruct
{
  char path[FILENAME_MAX];

  int keepDiffuseness[MAE_MAX_NUM_OBJECTS];
  int keepExclusion[MAE_MAX_NUM_OBJECTS];
  int keepDivergence[MAE_MAX_NUM_OBJECTS];

  int closestSpeakerPlayout[MAE_MAX_NUM_OBJECTS];
  float diffuseness[MAE_MAX_NUM_OBJECTS];
  float divergence[MAE_MAX_NUM_OBJECTS];
  int numExclusionSectors[MAE_MAX_NUM_OBJECTS];
  int usePredefinedSector[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  int excludeSectorIndex[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float excludeSectorMinAzimuth[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float excludeSectorMaxAzimuth[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float excludeSectorMinElevation[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float excludeSectorMaxElevation[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];

  int   lastFrame_hasDivergence[MAE_MAX_NUM_OBJECTS];
  float lastFrame_diffuseness[MAE_MAX_NUM_OBJECTS];
  float lastFrame_divergence[MAE_MAX_NUM_OBJECTS];
  int   lastFrame_numExclusionSectors[MAE_MAX_NUM_OBJECTS];
  int   lastFrame_usePredefinedSector[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  int   lastFrame_excludeSectorIndex[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float lastFrame_excludeSectorMinAzimuth[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float lastFrame_excludeSectorMaxAzimuth[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float lastFrame_excludeSectorMinElevation[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];
  float lastFrame_excludeSectorMaxElevation[MAE_MAX_NUM_OBJECTS][MAE_MAX_NUM_EXCLUDED_SECTORS];

} ENH_OBJ_MD_FRAME, *H_ENH_OBJ_MD_FRAME;

typedef struct _GroupSetupData
{
  int doUseIndividualGroupSetups;
  int numIndividualSetups;

  int numObjectBasedGroups;
  int hasExcludedSectors[MAE_MAX_NUM_GROUPS];

  Setup_SpeakerConfig3d speakerConfig[MAE_MAX_NUM_GROUPS];
  int hasKnownPos[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];
  float knownAzimuth[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];
  float knownElevation[MAE_MAX_NUM_GROUPS][MAE_MAX_NUM_SPEAKERS];

  int groupSetupID[MAE_MAX_NUM_GROUPS];

} GROUP_SETUP_DATA, *H_GROUP_SETUP_DATA;


  #ifndef M_PI
    #define M_PI    3.14159265358979323846
  #endif

  #ifndef QMFLIB_NUMBANDS
    #define QMFLIB_NUMBANDS 64
  #endif

/* init */
int MP_initAudioAndOamReadWrite(H_AUDIO_MP_CONFIG AudioConfig, H_OAM_MP_CONFIG OamConfig, int oam_length, int decoder_domain, int audioFramelength, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhObjMdConfig, int enableObjectOutputInterface, const int profile);
void MP_initAudioConfig(H_AUDIO_MP_CONFIG* h_AudioConfig, char* wavIO_inpath, char* wavIO_inpath_real, char* wavIO_inpath_imag, char* wavIO_outpath, char* wavIO_outpath_real, char* wavIO_outpath_imag, char* wavIO_outpathDiffuse_real, char* wavIO_outpathDiffuse_imag, char* wavIO_outpathDiffuse);
void MP_initOamConfig(H_OAM_MP_CONFIG* h_OamConfig, char* oamInpath, char* oamOutpath, int oam_mode, int decodedOAM, int hasDynOP, int hasUniformSpread);
void MP_initInteractConfig(H_INTERACT_MP_CONFIG* h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig);
int MP_initEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME* h_enhObjMdFrame, char* path);
int getNumAudioFrames(H_AUDIO_MP_CONFIG h_AudioConfig);

/* framewise processing */
int MP_processFrame(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig, ELEMENT_INTERACTION_HANDLE h_EI_Dec, H_EI_BITSTREAM_DECODER h_bitstream_EI_Dec, SCENE_DISPLACEMENT_HANDLE h_SD_Dec, H_SD_BITSTREAM_DECODER h_bitstream_SD_Dec, H_GROUP_SETUP_DATA groupSetupConfig, int isLastFrame, int frameNo, int enableObjectOutputInterface, int chooseSGmember, int *selectedPresetID);
int MP_readAudioFrame(H_AUDIO_MP_CONFIG h_AudioConfig);
int MP_decodeEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int numObjects, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhObjMdConfig, int frameNumber, int* isPresent);

/* wire routing */
int MP_applyWireRouting(H_INTERACT_MP_CONFIG h_interactConfig);

/* closest speaker playout processing */
int MP_applyClosestSpeakerProcessing(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_GROUP_SETUP_DATA groupSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame , int divergenceObjectsAdded);

/* divergence processing */
int MP_applyDivergenceProcessing(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, int* overallNumDivergenceObjectsAdded, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig);

/* sector exclusion */
int MP_checkSpeakerExclusion(int sectorIdx, CICP2GEOMETRY_CHANNEL_GEOMETRY speaker, H_LOCAL_SCREEN_INFO localScreen);
int MP_checkSpeakerExclusion_arbitrarySec(int secMaxAz, int  secMinAz, int secMaxEl, int secMinEl, CICP2GEOMETRY_CHANNEL_GEOMETRY speaker);
int MP_getIndividualSetups(H_GROUP_SETUP_DATA groupSetupConfig, int numGroups, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame);
int MP_writeGroupSetupData(H_GROUP_SETUP_DATA groupSetupConfig, char* groupSetup_outpath);
int MP_splitForExcludedSectors(char *inputAudio, char* inputOAM, H_GROUP_SETUP_DATA groupSetupConfig, H_AUDIO_MP_CONFIG h_AudioConfig, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_AUDIO_SCENE *audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG *signalGroupConfig);
int MP_freeGroupSetupConfig(H_GROUP_SETUP_DATA groupSetupConfig);

/* diffuseness processing */
int MP_initDiffusenessRendering(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, int decoderDomain, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig);
int MP_createDiffusePart(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OAM_MP_CONFIG h_OamConfig, H_LOCAL_SETUP_DATA localSetupConfig, H_AUDIO_MP_CONFIG h_AudioConfig, int isLastFrame, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame);
int MP_applyDirectGain(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OAM_MP_CONFIG h_OamConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int divergenceObjectsAdded);
float MP_getDiffuseDecorrOutputSample(float inputSample, int speakerIndex, H_INTERACT_MP_CONFIG h_InteractConfig);

/* on-off interaction */
int MP_applyOnOffInteraction(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, int downmixId, int* selectedPreset);
int MP_applySwitchGroupOff(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int grpID, int** onOffStatusModified);

/* switch group logic */
int MP_applySwitchGroupLogic(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig);

/* position interaction */
int MP_applyPositionInteractivity(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int downmixId, int divergenceObjectsAdded);

 /* gain interactivity */
int MP_applyGainInteractivity_TimeDomain(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int divergenceObjectsAdded);
int MP_applyGainInteractivity_QmfDomain(H_INTERACT_MP_CONFIG h_InteractConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int divergenceObjectsAdded);
int MP_getModifiedGains(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int oamHasBeenDecoded, int downmixId, int divergenceObjectsAdded);

/* object output interface */
int MP_initObjectOutputInterfaceWrite(H_OBJ_OUT_INTERFACE_CONFIG* h_OoiConfig, char* OoiOutpath, int audioFrameLength, int audioByteDepth, int audioSamplingRate);
int MP_writeObjectOutputData(ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo, ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OAM_MP_CONFIG h_OamConfig, H_AUDIO_MP_CONFIG h_AudioConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig, int frameNumber, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int divergenceObjectsAdded);
int MP_closeObjectOutputInterfaceWrite(H_OBJ_OUT_INTERFACE_CONFIG h_OoiConfig);

/* close */
int MP_closeEnhancedObjectMetadataFrame(H_ENH_OBJ_MD_FRAME h_enhObjMdFrame);
int MP_freeConfigStructs(H_AUDIO_MP_CONFIG h_AudioConfig, H_INTERACT_MP_CONFIG h_InteractConfig, H_OAM_MP_CONFIG h_OamConfig);

/* helper functions */
int MP_getGroupSetupData(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_GROUP_SETUP_DATA* groupSetupConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, H_LOCAL_SETUP_DATA localSetupConfig, ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig, H_ENH_OBJ_MD_FRAME h_enhObjMdFrame, int enhObjMdFramePresent );
int MP_getPresetCnd(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int presetID, int grpID, int downmixID, int* extensionIdx);
int MP_getSignalGroupIndex(ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int groupIndex, int memberIndex);
int MP_getSignalGroupType(ASCPARSER_AUDIO_SCENE* audioSceneConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, int groupIndex);
int MP_getGroupID(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int elementID);
int MP_getOamList(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig, ASCPARSER_AUDIO_SCENE* audioSceneConfig);
int MP_getOamSample(FILE* oam_file, StructOamMultidata* oam_sample, unsigned int oam_version, unsigned int hasDynPrio, unsigned int hasUniformSpread);
int MP_getMaxMaeID(ASCPARSER_AUDIO_SCENE* audioSceneConfig);
int MP_getGrpIdxAudioScene(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int grpID);
int MP_getGrpIdxElementInteraction(H_ELEMENT_INTERACTION_DATA ElementInteractionData, int grpID);
int MP_copyOamFrame(StructOamMultidata* Dest, StructOamMultidata* Source, int numObj, int overwriteSize);
int MP_getSwitchGroupAllowOnOff(ASCPARSER_AUDIO_SCENE* audioSceneConfig, int presetID);
int MP_isSwitchGroupMember(int numSG, SwitchGroupDefinition* sgDef, int grpID, int* sgID);
int MP_getDrcParamData(H_INTERACT_MP_CONFIG h_InteractConfig, ASCPARSER_AUDIO_SCENE* asi, int* numGroupIdsRequested, int* groupIdRequested, int* groupPresetIdRequested, int selectedPresetID);

#ifdef LOUDNESS_COMPENSATION
float MP_getLoudnessCompensationGain(ASCPARSER_AUDIO_SCENE* audioSceneConfig, H_ELEMENT_INTERACTION_DATA ElementInteractionData);
#endif
#endif
