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
#include "tdBinauralPath.h"
#include "cicp2geometry.h"

char TDrendererParams[] = "tmpFile3Ddec_BRIRs_TDrenderer.bs";
char tmpFileNameHoaOut[] = "tmpFile3Ddec_hoa_out_hoa.wav";       /* defined in HoaPathRun() */

typedef struct TdBinauralRenderingData_
{
  char* inPath;
  char* outPath;

  int	inputIsHoaOnly;
  int   configType; /* 1:CICP    2:Geo */
  int	cicpIdx;
  char* geo;
  
  char* brirBitstream;
  char* binaryTdBinauralRenderer;

  char* logfilename;

} TdBinauralRenderingData;

typedef struct TdBinauralParametrizationData_
{

  char* inputBitstream;
  char* binaryTdBinauralParametrization;
  char* logfilename;

} TdBinauralParametrizationData;


int TdBinauralParametrizationInit ( TdBinauralParametrizationDataHandle* ppData, char* brirBsPath, char* executablePath, char*  logfilename)
{
  ( *ppData ) = ( TdBinauralParametrizationDataHandle ) malloc ( sizeof ( TdBinauralParametrizationData ) );
  ( *ppData )->inputBitstream = brirBsPath;
  ( *ppData )->binaryTdBinauralParametrization = executablePath;
  ( *ppData )->logfilename =  logfilename;
  
  return 0;
}

int TdBinauralRendererInit ( TdBinauralRenderingDataHandle* ppData, char* infile, char* outfile, int inputType, int outputCicp, char *fileOutGeo, char* executablePath, char*  logfilename)
{
  ( *ppData ) = ( TdBinauralRenderingDataHandle ) malloc ( sizeof ( TdBinauralRenderingData ) );
  ( *ppData )->inPath = infile;
  ( *ppData )->outPath = outfile;

  ( *ppData )->cicpIdx = outputCicp;
  ( *ppData )->geo = fileOutGeo;
  ( *ppData )->binaryTdBinauralRenderer = executablePath;
  ( *ppData )->logfilename =  logfilename;

  ( *ppData )->brirBitstream = TDrendererParams;
  ( *ppData )->configType = -1;

  
    /* config is either Geo or CICP */
    if (outputCicp == CICP_INVALID)
	{
      fprintf(stderr, "Error invalid outputCicp \n");
      return -1;
    }
	else if (outputCicp == CICP_FROM_GEO)
	{
	  ( *ppData )->configType = 2;     /* Geo */
	}
	else
	{
	  ( *ppData )->configType = 1;     /* CICP */
	}

	/* is HOA only (H2B) flag */ 
	if ( ! (inputType & INPUT_CHANNEL) &&
       ! (inputType & INPUT_OBJECT) &&
       ! (inputType & INPUT_SAOC) &&
       ( inputType & INPUT_HOA ) )
	{
		( *ppData )->inputIsHoaOnly = 1;
	}
	else
	{
		( *ppData )->inputIsHoaOnly = 0;
	}

  return 0;
}

int TdBinauralRendererRun ( TdBinauralRenderingDataHandle pData, const VERBOSE_LEVEL verboseLvl )
{
  char callTdBinauralRenderer[4*FILENAME_MAX];
  char tmpString[FILENAME_MAX];

  sprintf ( tmpString, " >> %s 2>&1", pData->logfilename);

  /* if HOA only, then try the rendering in H2B mode */
  if (pData->inputIsHoaOnly)
  {
    sprintf( callTdBinauralRenderer, "%s -if %s -of %s -bs %s -h2b 1", pData->binaryTdBinauralRenderer, tmpFileNameHoaOut, pData->outPath, pData->brirBitstream);
    strcat ( callTdBinauralRenderer, tmpString );

    if (0 != callBinary(callTdBinauralRenderer, "TD binaural renderer (H2B mode)", verboseLvl, 0)) {
      fprintf ( stderr, "\nUse VL mode" );
    }
    else
    {
      return 0;
    }
  }

  /* VL mode */
  switch (pData->configType)
  {
  case 1: /* CICP */
    sprintf( callTdBinauralRenderer, "%s -if %s -of %s -bs %s -cicp %d", pData->binaryTdBinauralRenderer, pData->inPath, pData->outPath, pData->brirBitstream, pData->cicpIdx);
    strcat ( callTdBinauralRenderer, tmpString );
    break;
  case 2: /* Geo */
    sprintf( callTdBinauralRenderer, "%s -if %s -of %s -bs %s -geo %s", pData->binaryTdBinauralRenderer, pData->inPath, pData->outPath, pData->brirBitstream, pData->geo);
    strcat ( callTdBinauralRenderer, tmpString );
    break;
  default:
    fprintf(stderr, "Error unknown configType \n");
    return -1;
  }

  if (0 != callBinary(callTdBinauralRenderer, "TD binaural renderer (VL mode)", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}




int TdBinauralParametrizationRun (  TdBinauralParametrizationDataHandle pData, const VERBOSE_LEVEL verboseLvl )
{
  char callTdBinauralParam[4*FILENAME_MAX];
  char tmpString[FILENAME_MAX];

  sprintf( callTdBinauralParam, "%s -in %s -out %s ", pData->binaryTdBinauralParametrization, pData->inputBitstream, TDrendererParams);
  sprintf ( tmpString, " >> %s 2>&1", pData->logfilename);
  strcat ( callTdBinauralParam, tmpString );

  if (0 != callBinary(callTdBinauralParam, "TD binaural parametrization", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}




int TdBinauralParametrizationExit ( TdBinauralParametrizationDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

int TdBinauralRendererExit ( TdBinauralRenderingDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}