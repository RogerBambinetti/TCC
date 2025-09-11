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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hoaPath.h"
#include "cicp2geometry.h"

typedef struct HoaPathData_
{

  int HOATransportChannels;
  int reproductionLayout;
  char* outGeoFile;
  char* logfile;
  char* binaryName_HoaReferenceDecoder;
  int coreFrameLength;
  int NumOfHoaCoeffs; /* from HoaConfig() */
  unsigned int hoaOrder;
  DRC_PARAMS drcParams;
  DOMAIN_SWITCH     domainSwitchingDecisionsPostDrc1;
  PROCESSING_DOMAIN  moduleProcessingDomainsDrc1;
  char* binaryDomainSwitcher;
  char* binaryWavecutter;
  char* binaryName_HoaMatrixDecoder;
  int hoaMatrixId; 
  int hoaMatrixBitSize;
  char* hoaMatrix;
} HoaPathData;

int HoaPathInit(HoaPathDataHandle* ppData, int HOATransportChannels, int reproductionLayout, char *outGeoFile, char* logfile, char* binaryName_HoaReferenceDecoder, int coreFrameLength, char* binaryName_HoaMatrixDecoder, char* hoaMatrix, int hoaMatrixId, int hoaMatrixBitSize, DRC_PARAMS drcParams, DOMAIN_SWITCH domainSwitchingDecisionsPostDrc1, PROCESSING_DOMAIN moduleProcessingDomainsDrc1, char* binaryDomainSwitcher, char* binaryWavecutter)
{
  FILE* pOutFileHoaSideband = NULL;	
  unsigned char tmpVal      = 0;
  ( *ppData ) = ( HoaPathDataHandle ) malloc ( sizeof ( HoaPathData ) );
  ( *ppData )->logfile = logfile;
  ( *ppData )->HOATransportChannels = HOATransportChannels;
  ( *ppData )->reproductionLayout = reproductionLayout;
  ( *ppData )->outGeoFile = outGeoFile;
  ( *ppData )->binaryName_HoaReferenceDecoder = binaryName_HoaReferenceDecoder;
  ( *ppData )->coreFrameLength = coreFrameLength;
  ( *ppData )->drcParams = drcParams;
  ( *ppData )->domainSwitchingDecisionsPostDrc1 = domainSwitchingDecisionsPostDrc1;
  ( *ppData )->moduleProcessingDomainsDrc1 = moduleProcessingDomainsDrc1;
  ( *ppData )->binaryDomainSwitcher = binaryDomainSwitcher;
  ( *ppData )->binaryWavecutter = binaryWavecutter;
  ( *ppData )->binaryName_HoaMatrixDecoder = binaryName_HoaMatrixDecoder;
  ( *ppData )->hoaMatrixId = hoaMatrixId; 
  ( *ppData )->hoaMatrixBitSize = hoaMatrixBitSize; 
  ( *ppData )->hoaMatrix = hoaMatrix;

  /* read out HOA order from hoaConfig() */
  pOutFileHoaSideband = fopen("tmpFile3Ddec_hoaData.bs", "r");
  fread(&tmpVal, sizeof(char), 1, pOutFileHoaSideband);
  ( *ppData )->hoaOrder = tmpVal >> 5;

  if (( *ppData )->hoaOrder == 7) {
    ( *ppData )->hoaOrder += tmpVal & 0x1f;
  }

  fclose(pOutFileHoaSideband);
  ( *ppData )->NumOfHoaCoeffs = (( *ppData )->hoaOrder + 1) * (( *ppData )->hoaOrder + 1);

  return 0;
}

int HoaPathExit ( HoaPathDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}


int HoaPathRun ( HoaPathDataHandle pData, char* infile, char* outfile, const BIT_DEPTH bitdepth, const VERBOSE_LEVEL verboseLvl )
{
  char callHoaDecoder[3*FILENAME_MAX];
  char callHoaMatrixDecoder[ 3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];
  char tmpDrc1OutputFile[3 * FILENAME_MAX];
  char tmpFileNameHoaOut[]="tmpFile3Ddec_hoa_out_hoa.wav";
  /* Bitstream format of the HOA matrix. Has to be saved temporarily to hand over to the HOA matrix decoder. */
  char* tmpHoaMatrixInputFile = "tmpFile3Ddec_hoaMatrixBS.dat";
  /* Output files of the HOA matrix decoder */
  char* matrixOutputFile      = "tmpFile3Ddec_hoaMatrix.txt";
  CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numChannels = 0;
  int numLFEs = 0;
  FILE *spkFile  = NULL;
  int channel = 0;
  DRC_PARAMS* drcParams = &pData->drcParams;

  /**********************************/
  /* DRC-1 Decoder                  */
  /**********************************/
 
  if (drcParams->drcDecoderState) {
    char callDrc1[3 * FILENAME_MAX];
    char callWavcutter[3 * FILENAME_MAX];
      
    if (drcParams->profile == PROFILE_LOW_COMPLEXITY) {
      if (pData->moduleProcessingDomainsDrc1 != NOT_DEFINED)
      {
        if(pData->moduleProcessingDomainsDrc1 == STFT_DOMAIN)
        {       
          sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut" );
          sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetHoaPath, pData->HOATransportChannels, drcParams->framesize);
          strcat(  callDrc1, "-decId 0 -gdt 2 ");
          strcat(  callDrc1, "-gd 128 ");
        }
        else if(pData->moduleProcessingDomainsDrc1 == TIME_DOMAIN)
        {
          sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut.wav" );
          sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetHoaPath, pData->HOATransportChannels, drcParams->framesize);
          strcat(  callDrc1, "-decId 0 -gdt 0 ");
        }      
        sprintf( tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", drcParams->bsMpegh3daUniDrcConfig, drcParams->bsMpegh3daLoudnessInfoSet, drcParams->bsUnDrcGain, drcParams->txtDrcSelectionTransferData);
        strcat(  callDrc1, tmpString);
        sprintf( tmpString, "-dms %s ", drcParams->txtDownmixMatrixSet);
        strcat(  callDrc1, tmpString);      
      
        if (0 != callBinary(callDrc1, "DRC-1 decoder on HOA", verboseLvl, 0)) {
          return -1;
        }
      }
      else
      {
        strcpy(tmpDrc1OutputFile,infile);
      }

      if (pData->domainSwitchingDecisionsPostDrc1 == STFT_SYNTHESIS)
      {
        transformF2T(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 1);
         
        
        sprintf ( callWavcutter, "%s -if %s -of %s -delay %i ", pData->binaryWavecutter, tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut_delayComp.wav", 256 );
        if (0 != callBinary(callWavcutter, "Wavcutter for STFT delay in HOA path", verboseLvl, 0)) {
          return -1;
        }
        strcpy(tmpDrc1OutputFile,"tmpFile3Ddec_Drc1HoaOut_delayComp.wav");
      }
    } else {
      fprintf ( stderr, "\n\n Error DRC-1 gains applied to HOA coefficients aka inside HOA decoder currently not available\n" );
      return -1;
          
      /*if (pData->NumOfHoaCoeffs == 0) {
        fprintf ( stderr, "\n\n Error running DRC-1 decoder. NumOfHoaCoeffs == 0 not permitted. TODO: NumOfHoaCoeffs needs to be initialized.\n" );
        return -1;
      }
          
      if (pData->moduleProcessingDomainsDrc1 != NOT_DEFINED)
      {
        if(pData->moduleProcessingDomainsDrc1 == QMF_DOMAIN)
        {
          sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut" );
          sprintf( callDrc1, "%s -if %s -of %s -co %i -cn %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetHoaPath, pData->NumOfHoaCoeffs, drcParams->framesize);
                strcat(  callDrc1, "-decId 0 -gdt 4 ");
        }
        else if(pData->moduleProcessingDomainsDrc1 == STFT_DOMAIN)
        {
          sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut" );
          sprintf( callDrc1, "%s -if %s -of %s -co %i -cn %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetHoaPath, pData->NumOfHoaCoeffs, drcParams->framesize);
          strcat(  callDrc1, "-decId 0 -gdt 5 ");
        }
        else if(pData->moduleProcessingDomainsDrc1 == TIME_DOMAIN)
        {
          sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1HoaOut" );
          sprintf( callDrc1, "%s -if %s -of %s -co %i -cn %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetHoaPath, pData->NumOfHoaCoeffs, drcParams->framesize);
          strcat(  callDrc1, "-decId 0 -gdt 3 ");
        }
        sprintf( tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", drcParams->bsMpegh3daUniDrcConfig, drcParams->bsMpegh3daLoudnessInfoSet, drcParams->bsUnDrcGain, drcParams->txtDrcSelectionTransferData);
        strcat(  callDrc1, tmpString);
              
        fprintf ( stderr, "Calling DRC-1 decoder on HOA path\n %s\n\n", callDrc1 );
              
        if ( system ( callDrc1 ) )
        {
          fprintf ( stderr, "\n\n Error running DRC-1 decoder on HOA path\n" );
          return -1;
        }
      }*/
          
      
    }
  } else {
    strcpy(tmpDrc1OutputFile,infile);
  }

  /* If a matching HOA Rendering matrix payload is given the hoaMatrixDecoder    */
  /* is called and the output is appended to the call to the hoaReferenceDecoder */
  if (pData->hoaMatrixId >= 0)
  {
    FILE *binMatrixFile = NULL;	 
    binMatrixFile = fopen(tmpHoaMatrixInputFile, "wb");

    if(binMatrixFile == NULL)
    {
      fprintf( stderr, "\n\n Error creating temporary HOA matrix file.\n");
      return -1;
    }

    fwrite(pData->hoaMatrix, (pData->hoaMatrixBitSize + 7)/8, sizeof(unsigned char), binMatrixFile);
    fclose(binMatrixFile);
    sprintf( callHoaMatrixDecoder, "\"%s\" -if %s -of_mat %s -CICP %d -order %d >> %s 2>&1", pData->binaryName_HoaMatrixDecoder, tmpHoaMatrixInputFile, matrixOutputFile, pData->reproductionLayout, pData->hoaOrder, pData->logfile);

    if (0 != callBinary(callHoaMatrixDecoder, "HOA Matrix decoder", verboseLvl, 0)) {
      return -1;
    }
  }

  /* create speaker position tmp file */
  if(pData->reproductionLayout == CICP_FROM_GEO)
  {
    CICP2GEOMETRY_ERROR error = cicp2geometry_get_geometry_from_file(pData->outGeoFile, AzElLfe, &numChannels, &numLFEs);
    if(error != CICP2GEOMETRY_OK)
    {
      printf("Error reading GEO file\n");
      return -1;
    }
  }
  else
  {
    CICP2GEOMETRY_ERROR error = cicp2geometry_get_geometry_from_cicp(pData->reproductionLayout, AzElLfe, &numChannels, &numLFEs);
    if(error != CICP2GEOMETRY_OK)
    {
      printf("Error getting speaker positions from reproduction layout index\n");
      return -1;
    }
  }  
  spkFile = fopen("tmpFile3Ddec_speakerTemp.txt", "w");
  for(channel = 0; channel < (numChannels+numLFEs); channel++)
  {
    char sLine[6];
    if(AzElLfe[channel].LFE)
    {
      strcpy(sLine, "LFE_");
    }
    else
      sLine[0] = 0;
    fprintf(spkFile, "%sR0200_A%s%03i_E%s%02i\n", sLine, AzElLfe[channel].Az >= 0 ? "+" : "-", abs(AzElLfe[channel].Az), 
                                           AzElLfe[channel].El >= 0 ? "+" : "-", abs(AzElLfe[channel].El));
  }
  fclose(spkFile);

  /* Decode and render HOA representation */
  if (pData->hoaMatrixId >= 0) {
    sprintf(callHoaDecoder, "%s -ifpcm %s -ifside tmpFile3Ddec_hoaData.bs -ofpcm %s -spk %s -ofhoa %s -bitDepth %d -coreFrameLength %i -mtx %s >> %s 2>&1", 
            pData->binaryName_HoaReferenceDecoder, tmpDrc1OutputFile, outfile, "tmpFile3Ddec_speakerTemp.txt", tmpFileNameHoaOut, (int)bitdepth, pData->coreFrameLength, matrixOutputFile, pData->logfile);
  }else {
    sprintf(callHoaDecoder, "%s -ifpcm %s -ifside tmpFile3Ddec_hoaData.bs -ofpcm %s -spk %s -ofhoa %s -bitDepth %d -coreFrameLength %i >> %s 2>&1", 
            pData->binaryName_HoaReferenceDecoder, tmpDrc1OutputFile, outfile, "tmpFile3Ddec_speakerTemp.txt", tmpFileNameHoaOut, (int)bitdepth, pData->coreFrameLength, pData->logfile);
  }

  if (0 != callBinary(callHoaDecoder, "HOA-Decoder and Renderer", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}
