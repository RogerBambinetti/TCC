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

#include "wavIO.h"
#include "3DAudioCoreDeclib.h"

#define USAC_MAX_AUDIO_GROUPS 32
#define MAX_SIDE_INFO_INSTANCES 10

#ifndef INT_MAX
#define INT_MAX       2147483647    /* maximum (signed) int value */
#endif

typedef enum _debug_level {
  DEBUG_NORMAL,
  DEBUG_LVL1,
  DEBUG_LVL2
} DEBUG_LEVEL, *H_DEBUG_LEVEL;

/* External variables for mix matrix files */
extern char* g_mixMatrixFileName;
extern int   g_mixMatrixFileNameUsed;

static void writeout_contribution_data(char* extContributionFilename, char* cmdlName, int frameCntr, char* data, int size)
{

  FILE* extContributionFileHandle;
  char *pFilename;
  char nextExtContributionFilename[FILENAME_MAX];
  char suffix[5] = { '\0' };
  int fileExtLength = 0;

  /* strncpy(nextExtContributionFilename, extContributionFilename, FILENAME_MAX); */
  /* get suffix */
  pFilename = &(cmdlName[strlen(cmdlName)]);

  if( strstr(cmdlName, ".") != 0 ) {
    while ( pFilename[0] != '.' )
    {
      pFilename--;
      fileExtLength++;
    }
  }

  strncpy(suffix, pFilename, fileExtLength);
  /* copy wished name to front */
  strncpy(nextExtContributionFilename, cmdlName, strlen(cmdlName)-fileExtLength);
  /* add contribution info type,instance,frame and suffix */
  pFilename = &(nextExtContributionFilename[strlen(cmdlName)-fileExtLength]);
  strncpy(pFilename, extContributionFilename, strlen(extContributionFilename));
  pFilename = &(nextExtContributionFilename[strlen(cmdlName)-fileExtLength+strlen(extContributionFilename)]);


  sprintf(pFilename, "_%d%s", frameCntr, suffix);

  /* Open next object file */
  extContributionFileHandle = fopen(nextExtContributionFilename,"wb");

  fwrite(data, 1, size, extContributionFileHandle);   

  fclose(extContributionFileHandle);

}

static void writeout_obj_data(char* extObjMetadataFilename, int frameCntr, char* data, int size)
{

  FILE* extObjMetadataFileHandle;
  char *pFilename;
  char nextExtObjMetadataFilename[FILENAME_MAX];
  char suffix[5] = { '\0' };
  int fileExtLength = 0;

  strncpy(nextExtObjMetadataFilename, extObjMetadataFilename, FILENAME_MAX);
  pFilename = &(nextExtObjMetadataFilename[strlen(nextExtObjMetadataFilename)]);

  if( strstr(nextExtObjMetadataFilename, ".") == 0 ) {
    fprintf(stderr, "Warning: Object filename does not have a file ending (e.g. \".dat\"\n");
    return;
  }
  else {
    while ( pFilename[0] != '.' )
    {
      pFilename--;
      fileExtLength++;
    }
  }

  strncpy(suffix, pFilename, fileExtLength);

  while ( pFilename[0] != '_' )
  {
    pFilename--;
    if ( pFilename < nextExtObjMetadataFilename )
    {
      fprintf(stderr, "Warning: Object filename does not end with '_0'\n");
      return;
    }
  }

  sprintf(pFilename, "_%d%s", frameCntr, suffix);

  /* Open next object file */
  extObjMetadataFileHandle = fopen(nextExtObjMetadataFilename,"wb");

  fwrite(data, 1, size, extObjMetadataFileHandle);   

  fclose(extObjMetadataFileHandle);
}

static void writeout_productionmetadataframe_data(char* extProdMetadataFrameFilename, int frameCntr, char* data, int size)
{
  char *pFilename = NULL;
  char nextExtProductionMetadataFrameFilename[FILENAME_MAX];
  char suffix[5] = {'\0'};
  int fileExtLength = 0;

  strncpy(nextExtProductionMetadataFrameFilename, extProdMetadataFrameFilename, FILENAME_MAX);
  pFilename = &(nextExtProductionMetadataFrameFilename[strlen(nextExtProductionMetadataFrameFilename)]);

  if (strstr(nextExtProductionMetadataFrameFilename, ".") == 0) {
    fprintf(stderr, "Warning: Production metadata frame filename does not have a file ending (e.g. \".dat\"\n");
    return;
  } else {
    while (pFilename[0] != '.') {
      pFilename--;
      fileExtLength++;
    }
  }

  strncpy(suffix, pFilename, fileExtLength);
  sprintf(pFilename, "_%d.dat", frameCntr);

  /* Open next production metadata file */
  if (size > 0) {
    FILE* extProductionMetadataFrameFileHandle;
    extProductionMetadataFrameFileHandle = fopen(nextExtProductionMetadataFrameFilename,"wb");
    fwrite(data, 1, size, extProductionMetadataFrameFileHandle);
    fclose(extProductionMetadataFrameFileHandle);
  }
}

static int writeout_AudioPreRollInfo(
                    const char*                       apr_OutputFile,
                    const int                         nAudioPreRollSamplesWrittenPerChannel,
                    const int                         numPrerollAU
) {
  int error         = 0;
  FILE* aprInfoFile = NULL;

  if (apr_OutputFile[0] != '\0') {
    aprInfoFile = fopen(apr_OutputFile, "wb");

    if (NULL != aprInfoFile) {
      fwrite(&nAudioPreRollSamplesWrittenPerChannel, sizeof(int), 1, aprInfoFile);
      fwrite(&numPrerollAU, sizeof(int), 1, aprInfoFile);
      fclose(aprInfoFile);
    } else {
      error = 2;
    }
  } else {
    error = 1;
  }

  return error;
}

static int writeout_truncation_data(
                    MPEG_3DAUDIO_CORE_DECODER_HANDLE  hUsacDec,
                    FILE*                             pATI_OutputFile,
                    const char*                       ati_OutputFile
) {
  int error               = 0;
  unsigned int atiData[8] = {0};
  int size                = 0;

  if (ati_OutputFile[0] != '\0' && pATI_OutputFile) {
    error = MPEG_3DAudioCore_Declib_GetAudioTruncationInfoData(hUsacDec, atiData, &size);

    if ((0 == error) && (size != -1)) {
      fwrite(&size,   sizeof(int),          1,    pATI_OutputFile);
      fwrite(atiData, sizeof(unsigned int), size, pATI_OutputFile);
    }
  } else {
    error = 1;
  }

  return error;
}

static int writeout_extension_data(
                    MPEG_3DAUDIO_CORE_DECODER_HANDLE  hUsacDec,
                    FILE**                            pSAOCFileName,
                    FILE**                            pHOAFileName,
                    FILE*                             pDRCPayloadFile,
                    FILE*                             pFMT_CnvrtrFile,
                    char*                             saoc_OutputFile,
                    char*                             hoa_OutputFile,
                    char*                             drc_OutputFile,
                    char*                             fmt_cnvrtrFile,
                    char*                             obj_OutputFile,
                    char*                             enhObjMetadataFrame_OutputFile,
                    char*                             pmc_OutputFile,
                    char*                             contr_OutputFile,
                    const int                         frameCntTotal,
                    const int                         AudioPreRollExisting
) {
  if (saoc_OutputFile[0] != '\0') {
    char extData[2048];
    int size = 0;
    int instance = 1;

    while (MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_SAOC_3D, instance, extData, &size)) {
      if (size != - 1) {
        fwrite(extData, 1, size, pSAOCFileName[instance]);
      }

      instance++;
    }
  }

  if (hoa_OutputFile[0] != '\0') {
    char extData[2048];
    int size = 0;
    int instance = 1;

    while (MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_HOA, instance, extData, &size)) {
      if (size != - 1) {
        fwrite(extData, 1, size, pHOAFileName[instance]);
      }

      instance++;
    }
  }

  if (drc_OutputFile[0] != '\0') {
    char extData[2048];
    int size = 0;
    int instance = 1;

    MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_DRC, instance, extData, &size);

    if (size != - 1) {
      fwrite(extData, 1, size, pDRCPayloadFile);
    }
  }

#if IAR
  if (fmt_cnvrtrFile[0] != '\0') {
    char extData[2048];
    int size = 0;
    int instance = 1;

    MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_FMC, instance, extData, &size);

    if (size != - 1) {
      fprintf(pFMT_CnvrtrFile, "%d\n", extData[0]);
    }
  }
#endif

  if (pmc_OutputFile[0] != '\0') {
    char extData[2048];
    int size = 0;
    int instance = 1;

    MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_PRODUCTION_METADATA, instance, extData, &size);
    if (size != - 1) {
      /* Write production metadata frame .dat */
      writeout_productionmetadataframe_data(pmc_OutputFile, frameCntTotal + 1, extData, size);
    }
  }

  if (obj_OutputFile[0] != '\0') {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    int instance = 1;

    while (MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_OBJ, instance, extData, &size)) {
      if (size != - 1) {
        if(instance == 1) {
          sprintf(tmpOutputFile, "%s", obj_OutputFile);
        } else {
          sprintf(tmpOutputFile, "%i_%s", instance, obj_OutputFile);
        }

        writeout_obj_data(tmpOutputFile, frameCntTotal + 1, extData, size);
      }

      instance++;
    }
  }

  if (enhObjMetadataFrame_OutputFile[0] != '\0') {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    int instance = 1;

    while (MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_ENHANCED_OBJ_METADATA, instance, extData, &size)) {
      if (size != - 1) {
        if (instance == 1) {
          sprintf(tmpOutputFile, "%s", obj_OutputFile);
        } else {
          sprintf(tmpOutputFile, "%i_%s", instance, enhObjMetadataFrame_OutputFile);
        }

        writeout_obj_data(tmpOutputFile, frameCntTotal + 1, extData, size);
      }

      instance++;
    }
  }

  if (contr_OutputFile[0] != '\0') {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    int instance = 1;
    int extType = -1;

    while (MPEG_3DAudioCore_Declib_GetExtensionData(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_UNKNOWN, instance, extData, &size)) {
      if (size != - 1) {
        extType = MPEG_3DAudioCore_Declib_GetExtensionType(hUsacDec,instance-1);

        if (extType != 0) { /*leave out fill bits */
          sprintf(tmpOutputFile, "_type_%i_inst_%i_frame",extType, instance); /* add payload type name to filename */
          writeout_contribution_data(tmpOutputFile, contr_OutputFile, frameCntTotal + 1, extData, size);
        }
      }

      instance++;
    }
  }

  return 0;
}

static int writeout_audio_data(
          const MPEG_3DAUDIO_CORE_DECODER_HANDLE    hUsacDec,
          WAVIO_HANDLE                              hWavIO_OutTime,
          WAVIO_HANDLE                              hWavIO_OutQmfReal,
          WAVIO_HANDLE                              hWavIO_OutQmfImag,
          const CORE_OUTPUT_MODE                    coreOutputMode,
          const int                                 nOutChannels,
          const int                                 maxFrameLength,
          const int                                 maxNumQmfBands,
          const int                                 bIsChannelSignalGroupOnly,
          const int                                 discardAudioTruncationInfo,
          const int                                 useEditlist,
          const unsigned long                       editListDurationTotal,
          unsigned long                            *nTotalSamplesWrittenPerChannel,
          float                                   **outBuffer,
#ifdef RM6_INTERNAL_CHANNEL
          float                                   **outBufferIC,
#endif
          float                                  ***qmfReal_outBuffer,
          float                                  ***qmfImag_outBuffer
#ifdef RM6_INTERNAL_CHANNEL
         ,float                                  ***qmfReal_outBufferIC,
          float                                  ***qmfImag_outBufferIC
#endif
) {
  int error           = 0;
  unsigned long nSamplesWritten = *nTotalSamplesWrittenPerChannel;

  if (CORE_OUTPUT_TD == coreOutputMode) {
    int numOutSamples                      = 0;
    unsigned int nSamplesWrittenPerChannel = 0;
    int nTruncSamples                      = 0;

    error = MPEG_3DAudioCore_Declib_GetDecodedSamples_TimeDomain(hUsacDec,
                                                                 outBuffer,
                                                                 &numOutSamples,
                                                                 &nTruncSamples);
    if (error) {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetDecodedSamples_TimeDomain returned %d\n", error);
      return error;
    }

    if (0 == discardAudioTruncationInfo) {
      if (nTruncSamples < 0) {
        /* left truncation */
        wavIO_setDelay(hWavIO_OutTime, nTruncSamples);
      } else {
        /* right truncation */
        numOutSamples -= nTruncSamples;
      }
    }

    if (useEditlist && (editListDurationTotal != 0) && ((nSamplesWritten + numOutSamples) > editListDurationTotal)) {
      numOutSamples = editListDurationTotal - nSamplesWritten;
    }

    if (numOutSamples) {
#ifdef RM6_INTERNAL_CHANNEL
      if ( ICConfig.isStereoOut && bIsChannelSignalGroupOnly ) {
        int i, j;

        for (i = 0, j = 0; i < nOutChannels; i++) {
          if (ICConfig.isOutIC[i] == 1) {
            memcpy(outBufferIC[j++], outBuffer[i], sizeof(float) * maxFrameLength);
          }
        }  
        error = wavIO_writeFrame(hWavIO_OutTime,
                                 outBufferIC,
                                 numOutSamples,
                                 &nSamplesWrittenPerChannel);
        if (error) {
          fprintf(stderr, "wavIO_writeFrame returned %d\n", error);
          return error;
        }
      } else
#endif
      {
        error = wavIO_writeFrame(hWavIO_OutTime,
                                 outBuffer,
                                 numOutSamples,
                                 &nSamplesWrittenPerChannel);
        if (error) {
          fprintf(stderr, "wavIO_writeFrame returned %d\n", error);
          return error;
        }
      }

      nSamplesWritten += nSamplesWrittenPerChannel;
    }
  } else {
    int ts                                 = 0;
    int numTimeSlots                       = 0;
    int numQmfBands                        = 0;
    unsigned int nSamplesWrittenPerChannel = 0;

    error = MPEG_3DAudioCore_Declib_GetDecodedSamples_QmfDomain(hUsacDec,
                                                                qmfReal_outBuffer,
                                                                qmfImag_outBuffer,
                                                                &numTimeSlots,
                                                                &numQmfBands);
    if (error) {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetDecodedSamples_QmfDomain returned %d\n", error);
      return error;
    }


#ifdef RM6_INTERNAL_CHANNEL
    if (ICConfig.isStereoOut) {
      int i, j;

      for (ts = 0; ts < numTimeSlots; ++ts)  {
        for (i = 0, j = 0; i < nOutChannels; i++) {
          if (ICConfig.isOutIC[i] == 1) {
            memcpy(qmfReal_outBufferIC[ts][j],   qmfReal_outBuffer[ts][i], sizeof(float) * maxNumQmfBands);
            memcpy(qmfImag_outBufferIC[ts][j++], qmfImag_outBuffer[ts][i], sizeof(float) * maxNumQmfBands);
          }
        }  
      }

      for (ts = 0; ts < numTimeSlots; ++ts) {
        error |= wavIO_writeFrame(hWavIO_OutQmfReal, qmfReal_outBufferIC[ts], numQmfBands, &nSamplesWrittenPerChannel);
        error |= wavIO_writeFrame(hWavIO_OutQmfImag, qmfImag_outBufferIC[ts], numQmfBands, &nSamplesWrittenPerChannel);
      }

      if (error) {
        fprintf(stderr, "wavIO_writeFrame returned %d\n", error);
        return error;
      }
    }
    else
#endif
    {
      for (ts = 0; ts < numTimeSlots; ++ts) {
        error |= wavIO_writeFrame(hWavIO_OutQmfReal, qmfReal_outBuffer[ts], numQmfBands, &nSamplesWrittenPerChannel);
        error |= wavIO_writeFrame(hWavIO_OutQmfImag, qmfImag_outBuffer[ts], numQmfBands, &nSamplesWrittenPerChannel);
      }

      if (error) {
        fprintf(stderr, "wavIO_writeFrame returned %d\n", error);
        return error;
      }
    }

    nSamplesWritten += nSamplesWrittenPerChannel;
  }


  *nTotalSamplesWrittenPerChannel = nSamplesWritten;
  return error;
}

/* mapping bs_reference_distance and reference distance [m] */
static float bs_reference_distance_to_reference_distance_table[128] = {
  0.491475196f, 0.507827136f, 0.524723124f, 0.542181260f, 0.560220248f, 0.578859414f, 0.598118727f, 0.618018818f,
  0.638581009f, 0.659827327f, 0.681780534f, 0.704464149f, 0.727902474f, 0.752120619f, 0.777144529f, 0.803001013f,
  0.829717771f, 0.857323427f, 0.885847555f, 0.915320712f, 0.945774475f, 0.977241470f, 1.009755407f, 1.043351120f,
  1.078064601f, 1.113933038f, 1.150994860f, 1.189289771f, 1.228858796f, 1.269744329f, 1.311990170f, 1.355641578f,
  1.400745318f, 1.447349711f, 1.495504685f, 1.545261830f, 1.596674452f, 1.649797630f, 1.704688277f, 1.761405199f,
  1.820009157f, 1.880562935f, 1.943131407f, 2.007781603f, 2.074582785f, 2.143606519f, 2.214926750f, 2.288619887f,
  2.364764879f, 2.443443301f, 2.524739444f, 2.608740402f, 2.695536168f, 2.785219728f, 2.877887162f, 2.973637747f,
  3.072574064f, 3.174802104f, 3.280431388f, 3.389575078f, 3.502350103f, 3.618877282f, 3.739281453f, 3.863691608f,
  3.992241031f, 4.125067439f, 4.262313133f, 4.404125148f, 4.550655411f, 4.702060902f, 4.858503826f, 5.020151785f,
  5.187177955f, 5.359761276f, 5.538086642f, 5.722345095f, 5.912734037f, 6.109457436f, 6.312726047f, 6.522757636f,
  6.739777215f, 6.964017283f, 7.195718074f, 7.435127814f, 7.682502989f, 7.938108617f, 8.202218537f, 8.475115694f,
  8.757092451f, 9.048450895f, 9.349503168f, 9.660571792f, 9.981990024f, 10.31410221f, 10.65726414f, 11.01184346f,
  11.37822004f, 11.75678639f, 12.14794807f, 12.55212414f, 12.96974762f, 13.40126590f, 13.84714129f, 14.30785146f,
  14.78388998f, 15.27576685f, 15.78400902f, 16.30916098f, 16.85178535f, 17.41246345f, 17.99179595f, 18.59040351f,
  19.20892742f, 19.84803032f, 20.50839691f, 21.19073464f, 21.89577453f, 22.62427190f, 23.37700721f, 24.15478687f,
  24.95844416f, 25.78884004f, 26.64686413f, 27.53343567f, 28.44950445f, 29.39605188f, 30.37409202f, 31.38467267f
};

static void writeout_productionmetadataconfig_txt(char* extProdMetadataConfigFilename, char* data, int size, MPEG_3DAUDIO_CORE_DECODER_INFO* info)
{
  char *pFilename = NULL;
  char extProdMetadataConfigFilenametxt[FILENAME_MAX];
  char extData[2048] = {'\0'};
  unsigned int numObjectGroups = 0;
  unsigned int numChannelGroups = 0;

  strncpy(extProdMetadataConfigFilenametxt, extProdMetadataConfigFilename, FILENAME_MAX);
  pFilename = &(extProdMetadataConfigFilenametxt[strlen(extProdMetadataConfigFilenametxt)]);

  while (pFilename[0] != '.') {
    pFilename--;
  }
  sprintf(pFilename, ".txt");

  if (size <= 0) {
    fprintf(stderr, "Warning: No Production metadata config found\n");
  } else {
    FILE *pPMCFile = NULL;
    unsigned char temp = '\0';
    int has_reference_distance = 0;
    int bs_reference_distance = 0;
    int i = 0;
    int configbitlength = 0;
    unsigned int hasobjectdistance[USAC_MAX_AUDIO_GROUPS] = {0};
    unsigned int directHeadphone[USAC_MAX_AUDIO_GROUPS] = {0};
    float reference_distance = 0.0f;

    /* open PMC text file */
    pPMCFile = fopen(extProdMetadataConfigFilenametxt, "wb");
    if (pPMCFile == 0) {
     fprintf(stderr, "Error: Could not open production metadata configuration file %s!\n", extProdMetadataConfigFilenametxt);
      return;
    }

    /* has_reference_distance */
    temp = ( (unsigned char)data[configbitlength / 8] >> (7 - configbitlength % 8) ) & 0x01;
    has_reference_distance = (int)temp;
    configbitlength++;

    /* bs_reference_distance */
    if (has_reference_distance == 0) {
      bs_reference_distance = 57;
    } else {
      temp = (unsigned char)data[0] & 0x7F;
      bs_reference_distance = (int)temp;
      configbitlength = configbitlength + 7;
    }

    /* bs_reference_distance to distance */
    reference_distance = bs_reference_distance_to_reference_distance_table[bs_reference_distance];

    /* write reference distance */
    fprintf(pPMCFile, "has_reference_distance : %d", has_reference_distance);
    fprintf(pPMCFile, "\nreference_distance : %f [m]", reference_distance);

    /* write hasobjectdistance */
    fprintf(pPMCFile, "\n# of ObjectGroups : %d", info->numObjectGroups);
    for (i = 0; i < info->numObjectGroups; i++) {
      temp = ((unsigned char)data[configbitlength / 8] >> (7 - configbitlength % 8)) & 0x01;
      fprintf(pPMCFile, "\n%d", (int)temp);
      configbitlength++;
    }

    /* write direct to headphone */
    fprintf(pPMCFile, "\n# of ChannelGroups : %d", info->numChannelGroups);
    for (i = 0; i < info->numChannelGroups; i++){
      temp = ((unsigned char)data[configbitlength / 8] >> (7 - configbitlength % 8)) & 0x01;
      fprintf(pPMCFile, "\n%d", (int)temp);
      configbitlength++;
    }

    fclose(pPMCFile);
  }
}

int main (int argc, char *argv[])
{
  MPEG_3DAUDIO_CORE_DECODER_HANDLE hUsacDec = NULL;
  MPEG_3DAUDIO_CORE_DECODER_INFO info;

  char mp4_InputFile[FILENAME_MAX]                  = {'\0' };   /* Input mp4 file */
  char wav_OutputFile[FILENAME_MAX]                 = {'\0' };   /* Wav Output file */
  char qmfreal_OutputFile[FILENAME_MAX]             = {'\0' };   /* QMF Real Output file */
  char qmfimag_OutputFile[FILENAME_MAX]             = {'\0' };   /* QMF Imag Output file */
  char apr_OutputFile[FILENAME_MAX]                 = {'\0' };   /* Audio Pre-Roll Output file */
  char ati_OutputFile[FILENAME_MAX]                 = {'\0' };   /* audioTruncationInfo Output file */
  char saoc_OutputFile[FILENAME_MAX]                = {'\0' };   /* SAOC Output file */
  char obj_OutputFile[FILENAME_MAX]                 = {'\0' };   /* OBJ Output file */
  char enhObjMetadataFrame_OutputFile[FILENAME_MAX] = {'\0' };   /* Enhanced Object Metadata Frame Output file */
  char contr_OutputFile[FILENAME_MAX]               = {'\0' };   /* contribution Output file */
  char hoa_OutputFile[FILENAME_MAX]                 = {'\0' };   /* HOA Output file */
  char drc_OutputFile[FILENAME_MAX]                 = {'\0' };   /* DRC Output file for DRC payload, the config is written into a separate file by adding */
                                                                 /* the prefix 'config_' to the given drc_OutputFile */
#if IAR
  char fmt_cnvrtrFile[FILENAME_MAX]                 = {'\0'};
  FILE *pFMT_CnvrtrFile                             = NULL;
#endif
  char pmcFile[FILENAME_MAX]                        = {'\0'};
  FILE *pHOAFileName[MAX_SIDE_INFO_INSTANCES]       = {NULL};
  FILE *pDRCPayloadFile                             = NULL;
  FILE *pDRCConfigFile                              = NULL;
  FILE *pOutFileNameTime                            = NULL;
  FILE *pOutFileNameQmfReal                         = NULL;
  FILE *pOutFileNameQmfImag                         = NULL;
  FILE *pSAOCFileName[MAX_SIDE_INFO_INSTANCES]      = {NULL};
  FILE *pATI_OutputFile                             = NULL;

  WAVIO_HANDLE hWavIO_OutTime     = NULL; 
  WAVIO_HANDLE hWavIO_OutQmfReal  = NULL; 
  WAVIO_HANDLE hWavIO_OutQmfImag  = NULL; 

  DEBUG_LEVEL debugLvl = DEBUG_NORMAL;

  unsigned int bytesPerSample = 4;
  int i = 0;
  unsigned int ui = 0;
  int error = 0;
  int isLastFrame = 0;
  int AudioPreRollExisting = 0;
  int discardAudioPreRoll = 1;            /* in stand-alone operation mode the AudioPreRoll frames will be discarded */
  int discardAudioTruncationInfo = 0;     /* in stand-alone operation mode the audioTruncationInfo will be applied in the 3DAudioCoreDecoder */
  int numPrerollAU = 0;
  float **outBuffer = 0;
  float *** qmfReal_outBuffer = 0;
  float *** qmfImag_outBuffer = 0;
#ifdef RM6_INTERNAL_CHANNEL
  float **outBufferIC = 0;
  float *** qmfReal_outBufferIC = 0;
  float *** qmfImag_outBufferIC = 0;
#endif
  unsigned long nAudioPreRollSamplesWrittenPerChannel = 0;
  unsigned long nTotalTimeSamplesWrittenPerChannel = 0;
  unsigned long nTotalQmfSamplesWrittenPerChannel = 0;

  int useEditlist = 0;
  int writeEditlistInfo = 0;
  int outBitDepth = 0;
  unsigned long editListStartOffset = 0;
  unsigned long editListDurationTotal = 0;
  int delay = 0;
  int frameCnt = 0;
  int frameCntAudioPreRoll = 0;
  CORE_OUTPUT_MODE coreOutputMode = CORE_OUTPUT_TD;

  fprintf( stderr, "\n");
  fprintf( stderr, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stderr, "*                                                                             *\n");
  fprintf( stderr, "*                                  Core Decoder                               *\n");
  fprintf( stderr, "*                                                                             *\n");
  fprintf( stderr, "*                                  %s                                *\n", __DATE__);
  fprintf( stderr, "*                                                                             *\n");
  fprintf( stderr, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stderr, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stderr, "*                                                                             *\n");
  fprintf( stderr, "*******************************************************************************\n");
  fprintf( stderr, "\n");

  if ( argc < 3 )
  {
    fprintf(stderr," Usage: %s -if <input.mp4> -of <output.wav> OPTIONS\n", argv[0]);
    fprintf(stderr, "-if <input.mp4>            Input mp4 file\n");
    fprintf(stderr, "-of <output.wav>           Output wav file, for qmf output see switch -qmfout\n");
    fprintf(stderr, "-mixMatrix <mixMatrix.txt> Mix Matrix Input file to use from format converter module\n");
    fprintf(stderr, "-aprFile   <aprFile>       Output Audio Pre-Roll file name\n");
    fprintf(stderr, "                           If present the output will NOT be delay compensated!\n");
    fprintf(stderr, "-atiFile   <atiFile>       Output Audio Trunctaion Info file name\n");
    fprintf(stderr, "                           If present the output will NOT be truncated!\n");
    fprintf(stderr, "-saocFile  <saocFile>      Output SAOC file name\n");
    fprintf(stderr, "-hoaFile   <hoaFile>       Output HOA file name\n");
    fprintf(stderr, "-objFile   <objFile>       Output object file name, must end with '_0.dat'\n");
    fprintf(stderr, "-enhObjFile   <enhObjFile> Output enhanced object metadata file name, must end with '_0.dat'\n");
    fprintf(stderr, "-drcFile   <drcFile>       Output DRC file name for DRC Payload. The config is written into\n");
    fprintf(stderr, "                           a separate file named 'config_<drcFile>'\n");
    fprintf(stderr, "-qmfout                    Switch for QMF output, use together with -of <output> without .wav ending\n");
    fprintf(stderr, "-qmfauto                   Decoder outputs QMF or wav depending on the content (no file extension needed for <output>)\n");
    fprintf(stderr, "-elst                      Enable Edit List support for time domain output, default \"-noElst\" is active\n");
    fprintf(stderr, "-noElst                    Disable Edit List support for time domain output, is active as default\n");
    fprintf(stderr, "-writeElstToFile           Write Edit List info to file\n");
    fprintf(stderr, "-fcFile   <fcFile>         output file name for the format conversion decisions\n");
    fprintf(stderr, "-pmcFile  <fcFile>         output file name for the production metadata configuration data\n");
    fprintf(stderr, "-bitDepth <16,24,32>       wave output file format: 16, 24 LPCM or 32bit IEEE Float (default)\n");
#ifdef RM6_INTERNAL_CHANNEL
    fprintf(stderr, "-icFile   <icFile>         Internal channel speaker position output file name for the format conversion\n");
    fprintf(stderr, "-outStereo [1 or 0]        Indicate whether the output channel layout is stereo for internal channel\n" );
#endif
    fprintf(stderr, "-deblvl <0,1>              Set the debug level: 0 (default), 1\n");
    return -1;
  }

  for ( i = 1; i < argc; ++i )
  {
    if (!strcmp(argv[i], "-if"))		  /* Required */
    {
      strncpy(mp4_InputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-of"))	/* Required */
    {
      strncpy(wav_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-bitDepth") )
    {
      outBitDepth  = atoi ( argv[i+1] );
      switch(outBitDepth) {
        case 16:
        case 24:
        case 32:
          bytesPerSample = outBitDepth>>3; /* bits to bytes */
          break;
        default:
          error = -1;
          fprintf(stderr, "Unsupported wave output format. Please choose -bitDepth <16,24,32>, meaning 16, 24 bits LPCM or 32-bit IEE Float wave output.\n");
          goto cleanup;
      }
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-objFile"))
    {
      strncpy(obj_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-enhObjFile"))
    {
      strncpy(enhObjMetadataFrame_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-contributionFile"))
    {
      strncpy(contr_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-aprFile"))
    {
      strncpy(apr_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      discardAudioPreRoll = 0;
      continue;
    }
    else if ( !strcmp(argv[i], "-atiFile"))
    {
      strncpy(ati_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      discardAudioTruncationInfo = 1;
      continue;
    }
    else if ( !strcmp(argv[i], "-saocFile"))
    {
      strncpy(saoc_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-hoaFile"))
    {
      strncpy(hoa_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-drcFile"))
    {
      strncpy(drc_OutputFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
#if IAR
    else if ( !strcmp(argv[i], "-fcFile") )
    {
      strncpy(fmt_cnvrtrFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
#endif
    else if ( !strcmp(argv[i], "-pmcFile"))
    {
      strncpy(pmcFile, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-qmfout"))
    {
      coreOutputMode = CORE_OUTPUT_QMF;
      continue;
    }
    else if ( !strcmp(argv[i], "-qmfauto"))
    {
      coreOutputMode = CORE_OUTPUT_AUTO;
      continue;
    }
    else if ( !strcmp(argv[i], "-noElst"))
    {
      useEditlist = 0;
      continue;
    }
    else if ( !strcmp(argv[i], "-elst"))
    {
      useEditlist = 1;
      continue;
    }
    else if ( !strcmp(argv[i], "-writeElstToFile"))
    {
      writeEditlistInfo = 1;
      continue;
    }
    else if ( !strcmp(argv[i], "-mixMatrix"))
    {
      /* mix matrix variables are global, we can simply use them */
      strncpy(g_mixMatrixFileName, argv[i+1], FILENAME_MAX);
      g_mixMatrixFileNameUsed = 1;
      i++;
      continue;
    }
#ifdef RM6_INTERNAL_CHANNEL
    else if ( !strcmp(argv[i], "-icFile") )
    {
      strncpy(ICConfig.spkrFileName, argv[i+1], FILENAME_MAX);
      i++;
      continue;
    }
    else if ( !strcmp(argv[i], "-outStereo") )
    {
      ICConfig.isStereoOut  = atoi ( argv[i+1] );
      i++;
      continue;
    }
#endif
    else if ( !strcmp(argv[i], "-deblvl"))
    {
      int tmp = atoi ( argv[i+1] );
      switch (tmp) {
        case 1:
        case 2:
          debugLvl = (DEBUG_LEVEL)tmp;
          break;
        default:
          debugLvl = DEBUG_NORMAL;
      }
      i++;
      continue;
    }
  }

  error = MPEG_3DAudioCore_Declib_Open(&hUsacDec, coreOutputMode);
  if ( error )
  {
    fprintf(stderr, "MPEG_3DAudioCore_Declib_Open returned %d\n", error);
    goto cleanup;
  }

  error = MPEG_3DAudioCore_Declib_Init(hUsacDec, mp4_InputFile);
  if ( error )
  {
    fprintf(stderr, "MPEG_3DAudioCore_Declib_Init returned %d\n", error);
    goto cleanup;
  }

  MPEG_3DAudioCore_Declib_GetInfo(hUsacDec, &info);

  /* AudioTruncatioInfo() can only be applied if the core sampling frequency equals 48000 Hz */
  if ((0 == discardAudioTruncationInfo) && (48000 != info.sampleRate) && (44100 != info.sampleRate)) {
    fprintf(stderr, "Warning: Audio Truncation is only applied for 48000/44100 Hz core samplerate.\n");
    discardAudioTruncationInfo = 1;
  }

  coreOutputMode = info.coreOutputMode;

  if ( info.hasEditlist ) {
    editListStartOffset   = info.editListStartOffset;
    editListDurationTotal = info.editListDurationTotal;
  }
  else {
    editListStartOffset   = 0;
    editListDurationTotal = 0;
  }

  if ( useEditlist )
  {
    delay = 0 - editListStartOffset;
  }

  if ( writeEditlistInfo ) {
    char coreDelayFilename[120] = "tmpFile3Ddec_coreDelay.txt";
    FILE* fCoreDelay = fopen( coreDelayFilename, "wb" );
    if (fCoreDelay != NULL ) {
      fprintf( fCoreDelay, "%i\n%i", editListStartOffset, editListDurationTotal );
      fclose( fCoreDelay );
    }
    else {
      fprintf ( stderr, "\nCan not open %s for writing!\n\n", coreDelayFilename);
    }
  }

  if (CORE_OUTPUT_TD == coreOutputMode)
  {

    char * wavExtensionPtr = strstr(wav_OutputFile, ".wav" );

    error = wavIO_init( &hWavIO_OutTime, info.maxFrameLength, 0, delay);
    if (error) {
      fprintf(stderr, "wavIO_init returned %d\n", error);
      goto cleanup;
    }

    if ( wavExtensionPtr == NULL ) {
      /* add the file extension */
      strcat( wav_OutputFile, ".wav" );
    }

    pOutFileNameTime = fopen(wav_OutputFile, "wb");

    /* set tags for the output file */
    wavIO_setTags(hWavIO_OutTime, wav_OutputFile, NULL, NULL, NULL, NULL);

#ifdef RM6_INTERNAL_CHANNEL
    error = wavIO_openWrite(hWavIO_OutTime, pOutFileNameTime, info.nOutChannelsIC, info.sampleRate, bytesPerSample);
    outBufferIC = (float**) calloc(info.nOutChannelsIC, sizeof(float*));
    for (i = 0; i < info.nOutChannelsIC; ++i )
      outBufferIC[i] = (float*) calloc(info.maxFrameLength, sizeof(float));
#else
    error = wavIO_openWrite(hWavIO_OutTime, pOutFileNameTime, info.nOutChannels, info.sampleRate, bytesPerSample);
#endif

    if ( error )
    {
      fprintf(stderr, "wavIO_openWrite returned %d\n", error);
      goto cleanup;
    }

    outBuffer = (float**) calloc(info.nOutChannels, sizeof(float*));
    for (i = 0; i < info.nOutChannels; ++i )
      outBuffer[i] = (float*) calloc(info.maxFrameLength, sizeof(float));
  }
  else
  {
    int ts, channel; 
    error = wavIO_init( &hWavIO_OutQmfReal, 64, 0, 0);
    if ( error )
    {
      fprintf(stderr, "wavIO_init returned %d\n", error);
      goto cleanup;
    }
    error = wavIO_init( &hWavIO_OutQmfImag, 64, 0, 0);

    if ( error )
    {
      fprintf(stderr, "wavIO_init returned %d\n", error);
      goto cleanup;
    }

    strcpy(qmfreal_OutputFile, wav_OutputFile);
    strcat(qmfreal_OutputFile, "_real.qmf");
    strcpy(qmfimag_OutputFile, wav_OutputFile);
    strcat(qmfimag_OutputFile, "_imag.qmf");

    pOutFileNameQmfReal = fopen(qmfreal_OutputFile, "wb");
    pOutFileNameQmfImag = fopen(qmfimag_OutputFile, "wb");

    /* always 4 byte for qmf values */
#ifdef RM6_INTERNAL_CHANNEL
    error = wavIO_openWrite(hWavIO_OutQmfReal, pOutFileNameQmfReal, info.nOutChannelsIC, info.sampleRate, 4);
    error = wavIO_openWrite(hWavIO_OutQmfImag, pOutFileNameQmfImag, info.nOutChannelsIC, info.sampleRate, 4);

    qmfReal_outBufferIC = (float ***) calloc( info.maxNumTimeSlots, sizeof(float**) );
    qmfImag_outBufferIC = (float ***) calloc( info.maxNumTimeSlots, sizeof(float**) );

    for ( ts = 0; ts < info.maxNumTimeSlots; ++ts ) {

    qmfReal_outBufferIC[ts] = (float **) calloc( info.nOutChannelsIC, sizeof(float*) );
    qmfImag_outBufferIC[ts] = (float **) calloc( info.nOutChannelsIC, sizeof(float*) );

    for ( channel = 0; channel < info.nOutChannelsIC; ++channel ) {
      qmfReal_outBufferIC[ts][channel] = (float *) calloc( info.maxNumQmfBands, sizeof(float) );
        qmfImag_outBufferIC[ts][channel] = (float *) calloc( info.maxNumQmfBands, sizeof(float) );
      }
    }
#else
    error = wavIO_openWrite(hWavIO_OutQmfReal, pOutFileNameQmfReal, info.nOutChannels, info.sampleRate, 4);
    error = wavIO_openWrite(hWavIO_OutQmfImag, pOutFileNameQmfImag, info.nOutChannels, info.sampleRate, 4);
#endif

    qmfReal_outBuffer = (float ***) calloc( info.maxNumTimeSlots, sizeof(float**) );
    qmfImag_outBuffer = (float ***) calloc( info.maxNumTimeSlots, sizeof(float**) );

    for ( ts = 0; ts < info.maxNumTimeSlots; ++ts ) {

      qmfReal_outBuffer[ts] = (float **) calloc( info.nOutChannels, sizeof(float*) );
      qmfImag_outBuffer[ts] = (float **) calloc( info.nOutChannels, sizeof(float*) );

      for ( channel = 0; channel < info.nOutChannels; ++channel ) {
        qmfReal_outBuffer[ts][channel] = (float *) calloc( info.maxNumQmfBands, sizeof(float) );
        qmfImag_outBuffer[ts][channel] = (float *) calloc( info.maxNumQmfBands, sizeof(float) );
      }
    }
  }

  /* set debug level for modules */
  if (DEBUG_LVL1 <= debugLvl) {
    wavIO_setErrorLvl(hWavIO_OutTime,
                      WAVIO_ERR_LVL_STRICT);
  }

  /* Write out saoc configuration header */
  if ( saoc_OutputFile[0] != '\0' )
  {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;

    int instance = 1;

    while( MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_SAOC_3D, instance, extData, &size) ) {
      
     if(instance == 1) {
        sprintf(tmpOutputFile, "%s", saoc_OutputFile);
      }
      else {
        sprintf(tmpOutputFile, "%i_%s", instance, saoc_OutputFile);
      }
      pSAOCFileName[instance] = fopen(tmpOutputFile, "wb");

      if ( size <= 0 ) {
        fprintf(stderr, "Warning: No SAOC config header found\n");
      }
      else {
        fwrite(extData, 1, size, pSAOCFileName[instance]);
      }
      
      instance++;
    }

    if ( instance == 1 )
    {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetExtensionConfig returned %d: no saoc found.\n", error);
      goto cleanup;
    }

  }

  /* Write out hoa configuration header */
  if ( hoa_OutputFile[0] != '\0' )
  {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    
    int instance = 1;

    while( MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_HOA, instance, extData, &size) ) {
      
      if(instance == 1) {
        sprintf(tmpOutputFile, "%s", hoa_OutputFile);
      }
      else {
        sprintf(tmpOutputFile, "%i_%s", instance, hoa_OutputFile);
      }
      pHOAFileName[instance] = fopen(tmpOutputFile, "wb");

      if ( size <= 0 ) {
        fprintf(stderr, "Warning: No HOA config header found\n");
      }
      else {
        fwrite(extData, 1, size, pHOAFileName[instance]);
      }
      
      instance++;
    }

    if ( instance == 1 )
    {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetExtensionConfig returned %d: no hoa found.\n", error);
      goto cleanup;
    }
  }

  /* Write out DRC configuration */
  if ( drc_OutputFile[0] != '\0' )
  { 
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    int instance = 1;

    sprintf(tmpOutputFile, "config_%s", drc_OutputFile);

    pDRCConfigFile = fopen(tmpOutputFile, "wb");

    /* Only one instance for DRC data */
    MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_DRC, instance, extData, &size);

    if ( size <= 0 ) {
      fprintf(stderr, "Warning: No DRC config found\n");
    }
    else 
    {
      fwrite(extData, 1, size, pDRCConfigFile);
      fclose(pDRCConfigFile);

      /* Open payload file */
      pDRCPayloadFile = fopen(drc_OutputFile, "wb"); 
    } 
  }

#if IAR
  if ( fmt_cnvrtrFile[0] != '\0' )
  {
    /* char extData[2048]; */
    int size = 0;
    
    int instance = 1;

    /*MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_FMC, instance, extData, &size);*/
    pFMT_CnvrtrFile = fopen (fmt_cnvrtrFile, "wt");
    if ( pFMT_CnvrtrFile == 0 ) {
      fprintf(stderr, "Error: Could not open file %s!\n", fmt_cnvrtrFile);
      goto cleanup;
    }
  }
#endif

  if ( pmcFile[0] != '\0' ) {
    char tmpOutputFile[FILENAME_MAX] = {'\0'};
    char extData[2048] = {'\0'};
    int size = 0;
    int instance = 1;
    unsigned int numObjectGroups = 0;
    unsigned int numChannelGroups = 0;

    /* Only one instance for Production metadata configuration data */
    MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_PRODUCTION_METADATA, instance, extData, &size);

    if ( size <= 0 ) {
      fprintf(stderr, "Warning: No Production metadata config found\n");
    } else {
      FILE *pPMCFile = NULL;

      /* open PMC file */
      pPMCFile = fopen(pmcFile, "wb");
      if ( pPMCFile == NULL ) {
        fprintf(stderr, "Error: Could not open file %s!\n", pmcFile);
        goto cleanup;
      }

      /* write dat */
      fwrite(extData, 1, size, pPMCFile);
      fclose(pPMCFile);

      /* write txt */
      writeout_productionmetadataconfig_txt(pmcFile, extData, size, &info);
    }
  }

  if ( ati_OutputFile[0] != '\0' ) {
    pATI_OutputFile = fopen(ati_OutputFile, "wb");
  }

  /* write out all configs in contribution mode */
  if ( contr_OutputFile[0] != '\0' )
  {
    char tmpOutputFile[FILENAME_MAX];
    char extData[2048];
    int size = 0;
    int configType = -1;

    int instance = 1;

    while(MPEG_3DAudioCore_Declib_GetExtensionConfig(hUsacDec, MPEG_3DAUDIO_CORE_EXT_DATA_UNKNOWN, instance, extData, &size)) {
      if ( size != - 1 ) {
        configType = MPEG_3DAudioCore_Declib_GetExtensionConfigType(hUsacDec,instance-1);
        if( configType != 0 ){ /*leave out fill bits */
          sprintf(tmpOutputFile, "_config_type_%i_inst_%i_frame",configType , instance); /* add payload type name to filename */
          writeout_contribution_data(tmpOutputFile,contr_OutputFile , frameCnt+1, extData, size);
        }
       instance++;
      }
    }
    if ( instance == 1 )
    {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetExtensionConfig returned %d: no configs found.\n", error);
      goto cleanup;
    }
  }


  fprintf(stderr, "\nDecoding ...\n");

  /* decoding loop */
  while (!isLastFrame) {
    error = MPEG_3DAudioCore_Declib_GetAccessUnit(hUsacDec, &isLastFrame);
    if (error) {
      fprintf(stderr, "MPEG_3DAudioCore_Declib_GetAccessUnit returned %d\n", error);
      goto cleanup;
    }

    if (0 == isLastFrame) {
      error = MPEG_3DAudioCore_Declib_ParseAudioPreRoll(hUsacDec, &AudioPreRollExisting, &numPrerollAU);
      if (error) {
        fprintf(stderr, "MPEG_3DAudioCore_Declib_ParseAudioPreRoll returned %d\n", error);
        goto cleanup;
      }

      for (ui = 0; ui < (unsigned int)numPrerollAU; ++ui) {
        error = MPEG_3DAudioCore_Declib_DecodeAudioPreRoll(hUsacDec, AudioPreRollExisting, ui);
        if (error) {
          fprintf(stderr, "MPEG_3DAudioCore_Declib_DecodeAudioPreRoll returned %d\n", error);
          goto cleanup;
        }

        if ((1 == AudioPreRollExisting) && (0 == discardAudioPreRoll)) {
          error = writeout_audio_data(hUsacDec,
                                      hWavIO_OutTime,
                                      hWavIO_OutQmfReal,
                                      hWavIO_OutQmfImag,
                                      coreOutputMode,
                                      info.nOutChannels,
                                      info.maxFrameLength,
                                      info.maxNumQmfBands,
                                      info.bIsChannelSignalGroupOnly,
                                      discardAudioTruncationInfo,
                                      useEditlist,
                                      editListDurationTotal,
                                      &nAudioPreRollSamplesWrittenPerChannel,
                                      outBuffer,
#ifdef RM6_INTERNAL_CHANNEL
                                      outBufferIC,
#endif
                                      qmfReal_outBuffer,
                                      qmfImag_outBuffer
#ifdef RM6_INTERNAL_CHANNEL
                                     ,qmfReal_outBufferIC,
                                      qmfImag_outBufferIC
#endif
                                      );
          if (error) {
            fprintf(stderr, "writeout_audio_data returned %d\n", error);
            goto cleanup;
          }

          writeout_extension_data(hUsacDec,
                                  pSAOCFileName,
                                  pHOAFileName,
                                  pDRCPayloadFile,
                                  pFMT_CnvrtrFile,
                                  saoc_OutputFile,
                                  hoa_OutputFile,
                                  drc_OutputFile,
                                  fmt_cnvrtrFile,
                                  obj_OutputFile,
                                  enhObjMetadataFrame_OutputFile,
                                  pmcFile,
                                  contr_OutputFile,
                                  frameCnt + frameCntAudioPreRoll,
                                  AudioPreRollExisting);
          frameCntAudioPreRoll++;
        }
      }

      if (0 == discardAudioPreRoll) {
        int nAprSamples = 0;

        if (CORE_OUTPUT_TD == coreOutputMode) {
          nTotalTimeSamplesWrittenPerChannel += nAudioPreRollSamplesWrittenPerChannel;
        } else {
          nTotalQmfSamplesWrittenPerChannel += nAudioPreRollSamplesWrittenPerChannel;
        }

        if (nAudioPreRollSamplesWrittenPerChannel < INT_MAX) {
          nAprSamples = (int)nAudioPreRollSamplesWrittenPerChannel;
        } else {
          error = 1;
          goto cleanup;
        }

        writeout_AudioPreRollInfo(apr_OutputFile, nAprSamples, numPrerollAU);
      }

      error = MPEG_3DAudioCore_Declib_DecodeFrame(hUsacDec, &isLastFrame);
      if (error) {
        fprintf(stderr, "MPEG_3DAudioCore_Declib_DecodeFrame returned %d\n", error);
        goto cleanup;
      }
    }

    error = writeout_audio_data(hUsacDec,
                                hWavIO_OutTime,
                                hWavIO_OutQmfReal,
                                hWavIO_OutQmfImag,
                                coreOutputMode,
                                info.nOutChannels,
                                info.maxFrameLength,
                                info.maxNumQmfBands,
                                info.bIsChannelSignalGroupOnly,
                                discardAudioTruncationInfo,
                                useEditlist,
                                editListDurationTotal,
                                (CORE_OUTPUT_TD == coreOutputMode) ? &nTotalTimeSamplesWrittenPerChannel : &nTotalQmfSamplesWrittenPerChannel,
                                outBuffer,
#ifdef RM6_INTERNAL_CHANNEL
                                outBufferIC,
#endif
                                qmfReal_outBuffer,
                                qmfImag_outBuffer
#ifdef RM6_INTERNAL_CHANNEL
                               ,qmfReal_outBufferIC,
                                qmfImag_outBufferIC
#endif
                                );
    if (error) {
      fprintf(stderr, "writeout_audio_data returned %d\n", error);
      goto cleanup;
    }

    if (1 == discardAudioTruncationInfo) {
      writeout_truncation_data(hUsacDec,
                               pATI_OutputFile,
                               ati_OutputFile);
    }

    writeout_extension_data(hUsacDec,
                            pSAOCFileName,
                            pHOAFileName,
                            pDRCPayloadFile,
                            pFMT_CnvrtrFile,
                            saoc_OutputFile,
                            hoa_OutputFile,
                            drc_OutputFile,
                            fmt_cnvrtrFile,
                            obj_OutputFile,
                            enhObjMetadataFrame_OutputFile,
                            pmcFile,
                            contr_OutputFile,
                            frameCnt + frameCntAudioPreRoll,
                            0);

    fprintf(stderr, "Decoding frame %3d\r", frameCnt++);
  }

  if (hWavIO_OutTime)
  {
    unsigned long nTotalSamplesWrittenPerChannel = 0;
    wavIO_updateWavHeader(hWavIO_OutTime, &nTotalSamplesWrittenPerChannel);
  }

  if (CORE_OUTPUT_QMF == coreOutputMode)
  {
    unsigned long nTotalSamplesWrittenPerChannelReal = 0;
    unsigned long  nTotalSamplesWrittenPerChannelImag = 0;

    wavIO_updateWavHeader( hWavIO_OutQmfReal, &nTotalSamplesWrittenPerChannelReal);
    wavIO_updateWavHeader( hWavIO_OutQmfImag, &nTotalSamplesWrittenPerChannelImag);
  }

  if ((frameCntAudioPreRoll > 0) && (0 == discardAudioPreRoll)) {
    fprintf(stderr, "Finished - (%d frames + %d Audio Pre-Roll frames)\n", frameCnt++, frameCntAudioPreRoll++);
  } else {
    fprintf(stderr, "Finished - (%d frames)\n", frameCnt++);
  }

cleanup:

  if ( hUsacDec )
    MPEG_3DAudioCore_Declib_Close(hUsacDec);

  if ( hWavIO_OutTime )
    wavIO_close(hWavIO_OutTime);

  if ( hWavIO_OutQmfReal )
    wavIO_close(hWavIO_OutQmfReal);

  if ( hWavIO_OutQmfImag )
    wavIO_close(hWavIO_OutQmfImag);

  i=0;
  while ( pSAOCFileName[i] ) {
    fclose(pSAOCFileName[i]);
    i++;
  }

  i=0;
  while ( pHOAFileName[i] ) {
    fclose(pHOAFileName[i]);
    i++;
  }

  if ( pDRCPayloadFile )
  {
    fclose(pDRCPayloadFile);
  }

  if ( pATI_OutputFile ) {
    fclose(pATI_OutputFile);
  }

#if IAR
  if ( pFMT_CnvrtrFile )
  {
    fclose (pFMT_CnvrtrFile);
  }
#endif

  if ( outBuffer )
  {
    for (i = 0; i < info.nOutChannels; ++i )
    {
      if (outBuffer[i])
        free(outBuffer[i]);
    }
    free(outBuffer);
  }

  if ( qmfReal_outBuffer )
  {
    int ts, channel;
    for ( ts = 0; ts < info.maxNumTimeSlots; ++ts ) 
    {
      for ( channel = 0; channel < info.nOutChannels; ++channel ) 
        free( qmfReal_outBuffer[ts][channel] );

      free ( qmfReal_outBuffer[ts] );
    }
    free ( qmfReal_outBuffer );
  }

  if ( qmfImag_outBuffer )
  {
    int ts, channel;
    for ( ts = 0; ts < info.maxNumTimeSlots; ++ts ) 
    {
      for ( channel = 0; channel < info.nOutChannels; ++channel ) 
        free( qmfImag_outBuffer[ts][channel] );

      free ( qmfImag_outBuffer[ts] );
    }
    free ( qmfImag_outBuffer );
  }

#ifdef RM6_INTERNAL_CHANNEL
  if ( outBufferIC )
  {
    for (i = 0; i < info.nOutChannelsIC; ++i )
    {
      if (outBufferIC[i])
        free(outBufferIC[i]);
    }
    free(outBufferIC);
  }

  if ( qmfReal_outBufferIC )
  {
    int ts, channel;
    for ( ts = 0; ts < info.maxNumTimeSlots; ++ts ) 
    {
      for ( channel = 0; channel < info.nOutChannelsIC; ++channel ) 
        free( qmfReal_outBufferIC[ts][channel] );

      free ( qmfReal_outBufferIC[ts] );
    }
    free ( qmfReal_outBufferIC );
  }
#endif

  return error;
}
