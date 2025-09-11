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
#include "objectPath.h"

typedef struct ObjectPathData_
{
  int   reproductionLayout;
  char* logfile;
  char* binaryName;
  char* outGeoFile;
  int   numberObjects;
  DRC_PARAMS drcParams;
  DOMAIN_SWITCH domainSwitchingDecisionsPostDrc1;
  PROCESSING_DOMAIN  moduleProcessingDomainsDrc1;
  char* binaryDomainSwitcher;
} ObjectPathData;

int ObjectPathInit(ObjectPathDataHandle* ppData,
                   int numberObjects,
                   int reproductionLayout,
                   char* outGeoFile,
                   char *logfile,
                   char* binaryName,
                   DRC_PARAMS drcParams,
		               DOMAIN_SWITCH domainSwitchingDecisionsPostDrc1,
                   PROCESSING_DOMAIN moduleProcessingDomainsDrc1,
		               char* binaryDomainSwitcher
                   )
{
  ( *ppData ) = ( ObjectPathDataHandle ) malloc ( sizeof ( ObjectPathData ) );
  ( *ppData )->reproductionLayout = reproductionLayout;
  ( *ppData )->numberObjects = numberObjects;
  ( *ppData )->outGeoFile = outGeoFile;
  ( *ppData )->logfile = logfile;
  ( *ppData )->binaryName = binaryName;
  ( *ppData )->drcParams = drcParams;
  ( *ppData )->domainSwitchingDecisionsPostDrc1 = domainSwitchingDecisionsPostDrc1;
  ( *ppData )->moduleProcessingDomainsDrc1 = moduleProcessingDomainsDrc1;
  ( *ppData )->binaryDomainSwitcher = binaryDomainSwitcher;

  return 0;
}

int ObjectPathExit ( ObjectPathDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

MPEG3DA_RETURN_CODE ObjectPathRun ( ObjectPathDataHandle pData, char* infile, char* outfile, int mdp_run, int setupNumber, int sdChannels, const VERBOSE_LEVEL verboseLvl )
{
  char tmpString[3 * FILENAME_MAX];
  char tmpDrc1OutputFile[3 * FILENAME_MAX];
  char callRenderer[3 * FILENAME_MAX];
  DRC_PARAMS* drcParams = &pData->drcParams;
  int error = 0;

  /**********************************/
  /* DRC-1 Decoder                  */
  /**********************************/
  if (drcParams->drcDecoderState) {
    char callDrc1[3 * FILENAME_MAX];
        
    if (pData->moduleProcessingDomainsDrc1 != NOT_DEFINED)
    {
      if(pData->moduleProcessingDomainsDrc1 == QMF_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ObjOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetObjectPath, pData->numberObjects, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 1 ");
        strcat(  callDrc1, "-gd 32 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == STFT_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ObjOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetObjectPath, pData->numberObjects, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 2 ");
        strcat(  callDrc1, "-gd 128 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == TIME_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ObjOut.wav" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetObjectPath, pData->numberObjects, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 0 ");
      }
      sprintf( tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", drcParams->bsMpegh3daUniDrcConfig, drcParams->bsMpegh3daLoudnessInfoSet, drcParams->bsUnDrcGain, drcParams->txtDrcSelectionTransferData);
      strcat(  callDrc1, tmpString);
      sprintf( tmpString, "-dms %s ", drcParams->txtDownmixMatrixSet);
      strcat(  callDrc1, tmpString);      

      if (0 != callBinary(callDrc1, "DRC-1 decoder on object path", verboseLvl, 0)) {
        return MPEG3DA_ERROR_DRC1_OBJPATH;
      }
    }
    else
    {
      strcpy(tmpDrc1OutputFile,infile);
    }

    if (pData->domainSwitchingDecisionsPostDrc1 == QMF_SYNTHESIS)
    {
      transformF2T(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 0);
    }
    else if (pData->domainSwitchingDecisionsPostDrc1 == STFT_SYNTHESIS)
    {
      transformF2T(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 1);
    }
      
    
  } else {
    strcpy(tmpDrc1OutputFile,infile);
  }

  if (mdp_run == 1)
  {
    if (setupNumber > -1)
    {
      sprintf ( callRenderer, "%s -cicpOut %i -if %s -of %s -oamFile tmpFile3Ddec_mdp_object_setup_%i.oam",
              pData->binaryName, pData->reproductionLayout, tmpDrc1OutputFile, outfile, setupNumber);
    }
    else
    {
      if (sdChannels > 0)
      {
        sprintf ( callRenderer, "%s -cicpOut %i -if %s -of %s -oamFile tmpFile3Ddec_mdp_sdchannel.oam",
                pData->binaryName, pData->reproductionLayout, tmpDrc1OutputFile, outfile);
      }
      else
      {
        sprintf ( callRenderer, "%s -cicpOut %i -if %s -of %s -oamFile tmpFile3Ddec_mdp_object.oam -oamDivFile tmpFile3Ddec_mdp_divergence.oam",
                pData->binaryName, pData->reproductionLayout, tmpDrc1OutputFile, outfile);
      }
    }
  }
  else
  {
    sprintf ( callRenderer, "%s -cicpOut %i -if %s -of %s -oamFile tmpFile3Ddec_separator_object.oam",
            pData->binaryName, pData->reproductionLayout, tmpDrc1OutputFile, outfile);
  }

  if ( pData->reproductionLayout == CICP_FROM_GEO ) 
  {
    sprintf ( tmpString, " -outGeo %s ", pData->outGeoFile );
    strcat ( callRenderer, tmpString );
  }

  sprintf ( tmpString, " >> %s 2>&1", pData->logfile);
  strcat ( callRenderer, tmpString );

  error = callBinary(callRenderer, "Object Renderer", verboseLvl, 0);
  if (0 != error) {
    switch (error) {
      case -665:          /* -665 = MPEG3DAGVBAP_ERROR_INVALID_STREAM */
        return MPEG3DA_ERROR_CORE_INVALID_STREAM;
        break;
      default:
        return MPEG3DA_ERROR_CORE_GENERIC;
    }
  }

  return MPEG3DA_OK;
}
