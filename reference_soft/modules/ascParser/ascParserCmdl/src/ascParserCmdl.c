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

#include "ascparser.h"


static void printSpeakerConfig3d(ASCPARSER_SPEAKER_CONFIG_3D speakerConfig);

static void writeBinaryDataToFile(unsigned char* dmxMatrixBuffer, unsigned int numBits, const char* filename);


int main(int argc, char *argv[])
{
  HANDLE_ASCPARSER hAscParser;
  ASCPARSERINFO asc;
  ASCPARSER_DOWNMIX_CONFIG downmixConfig;
  ASCPARSER_HOA_CONFIG hoaConfig;
  ASCPARSER_SIGNAL_GROUP_INFO signalGroupInfo;
  ASCPARSER_ENHANCED_OBJECT_METADATA_CONFIG enhancedObjectMetadataConfig;
  ASCPARSER_PRODUCTION_METADATA_CONFIG productionMetadataConfig;
  
  unsigned char loudnessInfoSet[2048];
  int loudnessInfoAvailable;
  int loudnessInfoBitSize;

  int downmixConfigAvailable = 0;
  int hoaConfigAvailable = 0;
  int signalGroupInfoAvailable = 0;

  int err = 0;

  unsigned char* inputFile;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                ASC Parser Module                            *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
  

  if ( argc < 2 )  {
    printf("invalid arguments:\n");
    printf("Usage: %s infile.mp4\n", argv[0]);
    return -1;
  }

  inputFile  = (unsigned char*) argv[1];

  ASCPARSER_Init( &hAscParser, inputFile);

  ASCPARSER_GetASC(hAscParser, &asc);


  fprintf(stderr, "\nAudioSpecificConfig:\n");

  fprintf(stderr,"AOT:             %d\n", asc.aot);
  fprintf(stderr,"fs:              %d\n", asc.fs);
  fprintf(stderr,"extension_fs:    %d\n", asc.ext_fs);
  fprintf(stderr,"channelConfig:   %d\n", asc.chConfig);
  fprintf(stderr,"channels:        %d\n", asc.channels);
  fprintf(stderr,"objects:         %d\n", asc.objects);
  fprintf(stderr,"SAOC Trans. ch.: %d\n", asc.SAOCTransportChannels);
#ifdef SUPPORT_HOA
  fprintf(stderr,"HOA  Trans. ch.: %d\n", asc.HOATransportChannels);
#endif

  fprintf(stderr,"OAM mode:        %d\n", asc.oam_mode);
  fprintf(stderr,"OAM length:      %d\n", asc.oam_length);
  fprintf(stderr,"SAOC Header Len: %d\n", asc.saocHeaderLength);
  fprintf(stderr,"has Ref. Dist. : %d\n", asc.has_reference_distance);
  fprintf(stderr,"Ref. Disf. bit : %d\n", asc.bs_reference_distance);

#ifndef WD1_REFERENCE_LAYOUT_3D
  fprintf(stderr,"channelConfigurationIndex:  %d\n", asc.channelConfigurationIndex);
#else
  fprintf(stderr,"Reference Layout:\n");
  printSpeakerConfig3d(asc.referenceLayout);

  if ( asc.differsFromReferenceLayout )
  {
     fprintf(stderr,"Audio Channel Layout:\n");
     printSpeakerConfig3d(asc.audioChannelLayout);
  }
#endif
  fprintf(stderr, "Config extensions: %d\n\n", asc.numConfigExtensions);


 

  err = ASCPARSER_GetDownmixConfig(hAscParser, &downmixConfig, &downmixConfigAvailable);
  if ( err ) goto cleanup;
  if (downmixConfigAvailable != 0) 
  {
    unsigned int j = 0;
    unsigned int k = 0;
    unsigned int l = 0;
    fprintf(stderr, "Downmix Configuration found\n");
    fprintf(stderr, "Downmix config type    : %d\n", downmixConfig.downmixConfigType);
    fprintf(stderr, "Passive downmix flag   : %d\n", downmixConfig.passiveDownmixFlag);
    fprintf(stderr, "Immersive downmix flag : %d\n", downmixConfig.immersiveDownmixFlag);

    fprintf(stderr, "Config contains %d downmix settings\n", downmixConfig.downmixIdCount);
    for ( j = 0; j < downmixConfig.downmixIdCount; j++)
    {
      char outFile[FILENAME_MAX] = { '\0' };
      fprintf(stderr, "\nMatrixSet   : %d\n", j);
      fprintf(stderr, " -ID           : %d\n", downmixConfig.downmixMatrix[j].DMX_ID);
      fprintf(stderr, " -CICP Layout  : %d\n", downmixConfig.downmixMatrix[j].CICPspeakerLayoutIdx);
      fprintf(stderr, " -num matrices : %d\n", downmixConfig.downmixMatrix[j].downmixMatrixCount);
      for( k = 0; k < downmixConfig.downmixMatrix[j].downmixMatrixCount; k++ )
      {
        fprintf(stderr, " -Size: %d bits\n", downmixConfig.downmixMatrix[j].DmxMatrixLenBits[k]);
        for( l = 0; l < downmixConfig.downmixMatrix[j].numAssignedGroupIDs[k]; l++ )
        {
          fprintf(stderr, " -Assigned signal group #%d : %d\n", l, downmixConfig.downmixMatrix[j].signal_groupID[k][l] );
        }
        sprintf(outFile, "dmxMatrix_%d_%d.dat", j, k);
        writeBinaryDataToFile(downmixConfig.downmixMatrix[j].DownmixMatrix[k], downmixConfig.downmixMatrix[j].DmxMatrixLenBits[k], outFile);
      }
      
    }
  }
  else
  {
    fprintf(stderr, "No downmix configuration found.\n");
  }

  err = ASCPARSER_GetLoudnessInfoSet(hAscParser, loudnessInfoSet, &loudnessInfoAvailable, &loudnessInfoBitSize);
  if ( err ) goto cleanup;
  if( loudnessInfoAvailable != 0 )
  {
    fprintf(stderr, "\nLoudness info set found.\n");
    writeBinaryDataToFile(loudnessInfoSet, loudnessInfoBitSize, "loudnessInfo.dat");
  }
  else
  {
    fprintf(stderr, "\nNo loudness info set found.\n");
  }

  err = ASCPARSER_GetSignalGroupInfo(hAscParser, &signalGroupInfo, &signalGroupInfoAvailable);
  if (err) goto cleanup;

  err = ASCPARSER_GetEnhancedObjectConfig(hAscParser, &enhancedObjectMetadataConfig);
  if (err) goto cleanup;

  err = ASCPARSER_GetHoaConfig(hAscParser, &hoaConfig, &hoaConfigAvailable);
  if ( err ) goto cleanup;
  if (hoaConfigAvailable != 0) 
  {
    unsigned int j = 0;
    fprintf(stderr, "HOA Rendering Configuration found\n");
    
    fprintf(stderr, "Config contains %d matrices\n", hoaConfig.numHoaMatrices);
    for ( j = 0; j < hoaConfig.numHoaMatrices; j++)
    {
      char outFile[FILENAME_MAX] = { '\0' };
      fprintf(stderr, "\nMatrix: %d\n", j);
      fprintf(stderr, " -ID: %d\n", hoaConfig.hoaMatrix[j].HOA_ID);
      fprintf(stderr, " -CICP Layout: %d\n", hoaConfig.hoaMatrix[j].CICPspeakerLayoutIdx);
      fprintf(stderr, " -Size: %d bits\n", hoaConfig.hoaMatrix[j].HoaMatrixLenBits);
      sprintf(outFile, "hoaMatrix_%d_cicp_%d.dat", j, hoaConfig.hoaMatrix[j].CICPspeakerLayoutIdx);
      writeBinaryDataToFile(hoaConfig.hoaMatrix[j].HoaMatrix, hoaConfig.hoaMatrix[j].HoaMatrixLenBits, outFile);
    }
  }
  else
  {
    fprintf(stderr, "No HOA rendering configuration found.\n");
  }


cleanup:
  ASCPARSER_Close(hAscParser);
  return 0;
}


static void writeBinaryDataToFile(unsigned char* dataBuffer, unsigned int numBits, const char* filename)
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


static void printSpeakerConfig3d(ASCPARSER_SPEAKER_CONFIG_3D speakerConfig)
{
  int i = 0;
  if ( speakerConfig.speakerLayoutID == 0)
  {
    fprintf(stderr," -CICP Layout:    %d\n", speakerConfig.CICPspeakerLayoutIdx);
  }
  else  if ( speakerConfig.speakerLayoutID == 1)
  {
    fprintf(stderr," -CICP Loudspeakers:\n");
  }
  else  if ( speakerConfig.speakerLayoutID == 2)
  {
    fprintf(stderr," -Explicit Signaling:\n");
  }

  for ( i = 0; i < speakerConfig.numLoudspeakers; ++i )
  {
    fprintf(stderr, "   %d Az.: %4d, El.: %4d, isLFE.: %2d\n", i, speakerConfig.geometryInfo[i].Az, speakerConfig.geometryInfo[i].El, (speakerConfig.geometryInfo[i].LFE) > 0 ? 1 : 0 );
  }

  fprintf(stderr,"\n");
}
