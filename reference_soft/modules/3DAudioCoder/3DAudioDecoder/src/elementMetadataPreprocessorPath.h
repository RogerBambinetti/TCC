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

#include <string.h>
#include "metadataPreprocessor.h"
#include "elementInteractionInterfaceLib_API.h"
#include "localSetupInformationInterfaceLib_API.h"
#include "sceneDisplacementProcessing.h"
#include "oam_read.h"
#include "decode_types.h"

int MDPreprocPath_Execute(char* wavIO_inpath,
                          char* wavIO_outpath,
                          char* oamInpath,
                          char* oamOutpath,
                          ASCPARSERINFO* asc,
                          int decodedOAM,
                          ASCPARSER_AUDIO_SCENE* audioSceneConfig,
                          char* elementInteraction_InputFile,
                          char* localSetup_InputFile,
                          int moduleProcessingDomain,
                          int numOutChannels,
                          int numOutLFEs,
                          char* fileOutGeo,
                          int downmixId,
                          ASCPARSER_SIGNAL_GROUP_CONFIG* signalGroupConfig,
                          int *numAddedObjects,
                          char* BinReadBRIRsFromLocation,
                          int fDBinaural,
                          int tDBinaural,
                          int readBRIRsType,
                          char* pathLocalSetupInformationInterface,
                          int *separateSetups,
                          int splittingInfoType[MAE_MAX_NUM_ELEMENTS],
                          int splittingInfoSetup[MAE_MAX_NUM_ELEMENTS],
                          int* enableDiffusenessRendering,
                          int sceneDsplInterfaceEnabled,
                          char* sceneDisplacement_InputFile,
                          char* sdChannels_oamOutpath,
                          char* sdChannels_indexListOutpath,
                          int* hasSDchannels,
                          ASCPARSER_SIGNAL_GROUP_INFO* signalGroupInfo,
                          ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG* enhancedObjectMetadataConfig,
                          int signalGroupInfoAvailable,
                          int writeToObjectOutputInterface,
                          int chooseSGmember,
                          int selectedPresetID,
                          DRC_PARAMS* drcParams,
                          const char* logfile,
                          const PROFILE profile,
                          const VERBOSE_LEVEL verboseLvl
);

int MDPreproc_prepareLocalSetupBitstream(char* geometry_file,
                                         char** localSetup_OutputFile,
                                         char* BinReadBRIRsFromLocation,
                                         int fDBinaural,
                                         int tDBinaural,
                                         int readBRIRsType,
                                         char *binary_LocalSetupInformationInterface,
                                         const char* logfile,
                                         const VERBOSE_LEVEL verboseLvl
);

int MDPreproc_combineWav(char* destination,
                         char *source,
                         char* addition,
                         int processingDomain
);

/* int MDPreproc_getLocalSetup(H_LOCAL_SETUP_DATA* localSetupConfig, CICP2GEOMETRY_CHANNEL_GEOMETRY* geometry, int numSpeakers, int numLFE, CICP_FORMAT outCicp); */
int MPPreproc_write_CICP_to_file(char *outputFilename,
                                 int CICPIndex,
                                 int numChannels,
                                 int numLFEs
);

int MPPreproc_write_geometry_to_file(char *outputFilename,
                                     CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                     int numChannels,
                                     int numLFEs,
                                     const VERBOSE_LEVEL verboseLvl
);

int MPPreproc_write_geometry_to_file_knownPositions( char *outputFilename,
                                                         CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                                         CICP2GEOMETRY_CHANNEL_GEOMETRY outGeo_misplaced[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                                         int numChannels,
                                                         int numLFEs,
                                                         CICP_FORMAT outputCicp
);

int MDPreproc_getSplitInformationForExcludedSectors(ASCPARSER_AUDIO_SCENE *audioSceneConfig,
                                                    H_GROUP_SETUP_DATA groupSetupConfig,
                                                    ASCPARSER_SIGNAL_GROUP_CONFIG *signalGroupConfig,
                                                    int *splittingInfoType,
                                                    int *splittingInfoSetup
);

int MDPreproc_fillWithZeroChannels(char* audioFilePathIn,
                                   char* audioFilePathOut,
                                   char* audioFileSetup,
                                   char* targetSetup,
                                   int processingDomain
);

int MDPreproc_getSDchannelIdx(char* sdChannels_indexListOutpath,
                              int *SDchannelIdx,
                              int *numSDchannels
);

int MDPreproc_separateSDchannels(char* combined,
                                 char* sdChannelsPath,
                                 char* nonSDchannelsPath,
                                 int* SDchannelIdx,
                                 int numTotalChannels,
                                 int numSDchannels,
                                 int processingDomain
);
