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

#include "saoc_getinfo.h"

#include "saocPath.h"
#include "formatConverter_api.h"

typedef struct SaocPathData_
{

  int reproductionLayout;
  int referenceLayout;
  char* inGeoFile;
  int saocHeaderLength;
  int coreOutputFrameLength;
  char* outGeoFile;
  char* outGeoDevFile;
  char* logfile;
  char* binaryFormatConverter;
  char* binaryName;

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  int hasDmxLayout;
  int dmxLayout;
  char* dmxGeoFile;
#endif

  int   NumInputSignals; /* from SAOC3DSpecificConfig() */
  DRC_PARAMS drcParams;
  PROCESSING_DOMAIN  moduleProcessingDomainsDrc1;

} SaocPathData;


#ifdef SUPPORT_SAOC_DMX_LAYOUT
int SaocPathInit ( SaocPathDataHandle* ppData, int saocHeaderLength, int coreOutputFrameLength, int referenceLayout, char *inGeoFile, int hasDmxLayout, int dmxLayout, char *dmxGeoFile, int reproductionLayout, char *outGeoFile, char *outGeoDevFile, char* logfile, char* binaryName, char* binaryFormatConverter
#else
int SaocPathInit ( SaocPathDataHandle* ppData, int saocHeaderLength, int coreOutputFrameLength, int referenceLayout, char *inGeoFile, int reproductionLayout, char *outGeoFile, char *outGeoDevFile, char* logfile, char* binaryName, char* binaryFormatConverter
#endif
                   , DRC_PARAMS drcParams, PROCESSING_DOMAIN moduleProcessingDomainsDrc1)
{
  ( *ppData ) = ( SaocPathDataHandle ) malloc ( sizeof ( SaocPathData ) );
  ( *ppData )->referenceLayout = referenceLayout;
  ( *ppData )->inGeoFile = inGeoFile;
#ifdef SUPPORT_SAOC_DMX_LAYOUT
  ( *ppData )->hasDmxLayout = hasDmxLayout;
  ( *ppData )->dmxLayout = dmxLayout;
  ( *ppData )->dmxGeoFile = dmxGeoFile;
#endif
  ( *ppData )->reproductionLayout = reproductionLayout;
  ( *ppData )->outGeoFile = outGeoFile;
  ( *ppData )->outGeoDevFile = outGeoDevFile;
  ( *ppData )->logfile = logfile;
  ( *ppData )->binaryName = binaryName;
  ( *ppData )->binaryFormatConverter = binaryFormatConverter;
  ( *ppData )->saocHeaderLength = saocHeaderLength;
  ( *ppData )->coreOutputFrameLength = coreOutputFrameLength/64; /* number of time slots */
  ( *ppData )->drcParams = drcParams;
  ( *ppData )->moduleProcessingDomainsDrc1 = moduleProcessingDomainsDrc1;

  return 0;
}

int SaocPathExit ( SaocPathDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

int SaocPathRun ( SaocPathDataHandle pData, char* infile, char* outfile, int mdp_run, const VERBOSE_LEVEL verboseLvl )
{
  /* do SAOC decoding */
  char tmpString[3 * FILENAME_MAX];
  char tmpDrc1OutputFile[3 * FILENAME_MAX];
  char callSaocDecoder[3 * FILENAME_MAX];
  DRC_PARAMS* drcParams = &pData->drcParams;

  /**********************************/
  /* DRC-1 Decoder                  */
  /**********************************/
    
  if (drcParams->drcDecoderState) {
    char callDrc1[3 * FILENAME_MAX];
        
    if (pData->NumInputSignals == 0) {
      fprintf ( stderr, "\n\n Error running DRC-1 decoder. NumInputSignals == 0 not permitted. TODO: NumInputSignals needs to be initialized. \n" );
      return -1;
    }
        
    if (pData->moduleProcessingDomainsDrc1 != NOT_DEFINED)
    {
      if(pData->moduleProcessingDomainsDrc1 == QMF_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1SaocOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetSaocPath, pData->NumInputSignals, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 4 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == STFT_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1SaocOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetSaocPath, pData->NumInputSignals, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 5 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == TIME_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1SaocOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetSaocPath, pData->NumInputSignals, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 3 ");
      }
      sprintf( tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", drcParams->bsMpegh3daUniDrcConfig, drcParams->bsMpegh3daLoudnessInfoSet, drcParams->bsUnDrcGain, drcParams->txtDrcSelectionTransferData);
      strcat(  callDrc1, tmpString);
      sprintf( tmpString, "-dms %s ", drcParams->txtDownmixMatrixSet);
      strcat(  callDrc1, tmpString);      

      if (0 != callBinary(callDrc1, "DRC-1 decoder on SAOC path", verboseLvl, 0)) {
        return -1;
      }
    }
    
  }

  if (mdp_run)
  {
    sprintf ( callSaocDecoder, "%s -cicpOut %i -if %s -of %s -bs tmpFile3Ddec_saocData.bs -oam tmpFile3Ddec_mdp_object.oam -configLen %d -coreOutputFrameLength %d",
              pData->binaryName, pData->reproductionLayout, infile, outfile, pData->saocHeaderLength, pData->coreOutputFrameLength );
  }
  else
  {
    sprintf ( callSaocDecoder, "%s -cicpOut %i -if %s -of %s -bs tmpFile3Ddec_saocData.bs -oam tmpFile3Ddec_separator_object.oam -configLen %d -coreOutputFrameLength %d",
              pData->binaryName, pData->reproductionLayout, infile, outfile, pData->saocHeaderLength, pData->coreOutputFrameLength );
  }

  if (pData->reproductionLayout == CICP_FROM_GEO ) {
    sprintf ( tmpString, " -outGeo %s ", pData->outGeoFile );
    strcat ( callSaocDecoder, tmpString );
  }

  /* get downmix matrix from format converter for SAOC channel based */
  {
    int  inSaocCICPIndex;
    char inSaocGeoFilename[255];
    int  saocChannelPathFlag = 0;
    char callFormatConverter[3 * FILENAME_MAX];

    saoc_GetSaocInputLayout( "tmpFile3Ddec_saocData.bs", pData->saocHeaderLength, &inSaocCICPIndex, inSaocGeoFilename, &saocChannelPathFlag);

    if (saocChannelPathFlag) {

      if ( ((inSaocCICPIndex != -1) && (inSaocCICPIndex != pData->reproductionLayout)) ||  /* not needed if cicp of in/out are the same */
        (inSaocCICPIndex == -1) || (pData->reproductionLayout == -1) )                /* but always for explicit geometry info */
      {
        sprintf ( callFormatConverter, "%s -cicpIn %d -cicpOut %d -dmxMtxOutput %s", pData->binaryFormatConverter, inSaocCICPIndex,  pData->reproductionLayout, "tmpFile3Ddec_dmxMtx.txt" );

        if (inSaocCICPIndex == CICP_FROM_GEO ) {
          sprintf ( tmpString, " -inGeo %s ", inSaocGeoFilename );
          strcat ( callFormatConverter, tmpString );
        }
        if (pData->reproductionLayout == CICP_FROM_GEO ) {
          sprintf ( tmpString, " -outGeo %s ", pData->outGeoFile );
          strcat ( callFormatConverter, tmpString );
        }
        if (pData->outGeoDevFile != NULL ) {
          sprintf ( tmpString, " -angleDev %s ", pData->outGeoDevFile );
          strcat ( callFormatConverter, tmpString );
        }

        sprintf ( tmpString, " >> %s 2>&1", pData->logfile);
        strcat ( callFormatConverter, tmpString );

        if (0 != callBinary(callFormatConverter, "Format Converter", verboseLvl, 0)) {
          return -1;
        }

        sprintf ( tmpString, " -ren %s ", "tmpFile3Ddec_dmxMtx.txt" );
        strcat ( callSaocDecoder, tmpString );
      }
    }
  }

  /* add reference layout */

  sprintf ( tmpString, " -refLayout %d ", pData->referenceLayout );
  strcat ( callSaocDecoder, tmpString );
  if (pData->referenceLayout == CICP_FROM_GEO ) {
    sprintf ( tmpString, " -refGeo %s ", pData->inGeoFile );
    strcat ( callSaocDecoder, tmpString );
  }

#ifdef SUPPORT_SAOC_DMX_LAYOUT
  /* add downmix layout */
  if (pData->hasDmxLayout) {
    sprintf ( tmpString, " -dmxLayout %d ", pData->dmxLayout );
    strcat ( callSaocDecoder, tmpString );
    if (pData->dmxLayout == CICP_FROM_GEO ) {
      sprintf ( tmpString, " -dmxGeo %s ", pData->dmxGeoFile );
      strcat ( callSaocDecoder, tmpString );
    }
  }
#endif

  sprintf ( tmpString, " >> %s 2>&1", pData->logfile);
  strcat ( callSaocDecoder, tmpString );

  if (0 != callBinary(callSaocDecoder, "SAOC-Decoder and Renderer", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}
