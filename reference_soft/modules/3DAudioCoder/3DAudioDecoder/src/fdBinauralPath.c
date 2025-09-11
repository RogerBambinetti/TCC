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
#include "fdBinauralPath.h"


typedef struct FdBinauralRenderingData_
{

  char* inPath;
  char* outPath;
  int   outputFormat;
  int   readBrirMetadataType;
  char* readBrirPath;
  char* binaryFdBinauralRenderer;
  char* parametrizationPath;
  char* outGeo;
  char* logfilename;
  int profile;
  int level;

} FdBinauralRenderingData;

typedef struct FdBinauralParametrizationData_
{

  char* brirBitstreamFileIn;
  char* brirBitstreamFileOut;
  char* binaryFdBinauralParametrization;
  char* logfilename;

} FdBinauralParametrizationData;

typedef struct FdBinauralBitstreamWriterData_
{

  char* brirInFolder;
  char* brirBitstreamFileOut;
  char* geometryInPath;
  char* binaryFdBinauralBitstreamWriter;
  char* logfilename;

} FdBinauralBitstreamWriterData;


int FdBinauralParametrizationInit ( FdBinauralParametrizationDataHandle* ppData, char* brirBitstreamPath, char* executablePath, char*  logfilename, char *bitstreamOutPath )
{
  ( *ppData ) = ( FdBinauralParametrizationDataHandle ) malloc ( sizeof ( FdBinauralParametrizationData ) );
  ( *ppData )->brirBitstreamFileIn = brirBitstreamPath;
  ( *ppData )->binaryFdBinauralParametrization = executablePath;
  ( *ppData )->logfilename =  logfilename;
  ( *ppData )->brirBitstreamFileOut = bitstreamOutPath;

  return 0;
}

int FdBinauralBitstreamWriterInit ( FdBinauralBitstreamWriterDataHandle* ppData, char* brirFolderPath, char *executablePath, char* logfilename, char* bitstreamOutPath, char* geoInPath  )
{
  ( *ppData ) = ( FdBinauralBitstreamWriterDataHandle ) malloc ( sizeof ( FdBinauralBitstreamWriterDataHandle ) );
  ( *ppData )->brirInFolder = brirFolderPath;
  ( *ppData )->binaryFdBinauralBitstreamWriter = executablePath;
  ( *ppData )->logfilename =  logfilename;
  ( *ppData )->brirBitstreamFileOut = bitstreamOutPath;
  ( *ppData )->geometryInPath = geoInPath;

  return 0;
}

int FdBinauralRendererInit ( FdBinauralRenderingDataHandle* ppData, char* infile, char* outfile, int outputFormat, int readBrirMetadataType, char *readBrirLocation, char* executablePath, char* outGeoPath, char*  logfilename, int profile, int level)
{
  ( *ppData ) = ( FdBinauralRenderingDataHandle ) malloc ( sizeof ( FdBinauralRenderingData ) );
  ( *ppData )->inPath = infile;
  ( *ppData )->outPath = outfile;
  ( *ppData )->outputFormat = outputFormat;
  ( *ppData )->readBrirMetadataType = readBrirMetadataType;
  ( *ppData )->readBrirPath = readBrirLocation;
  ( *ppData )->binaryFdBinauralRenderer = executablePath;
  ( *ppData )->outGeo = outGeoPath;
  ( *ppData )->logfilename =  logfilename;
  ( *ppData )->profile = profile;
  ( *ppData )->level = level;

  return 0;
}

int FdBinauralRendererRun ( FdBinauralRenderingDataHandle pData, const VERBOSE_LEVEL verboseLvl)
{
  char callFdBinauralRenderer[4*FILENAME_MAX];
  char readBRIRparams[FILENAME_MAX];
  char tmpString[FILENAME_MAX];

  strcpy(readBRIRparams,pData->readBrirPath);

  if ( pData->outputFormat == CICP_FROM_GEO ) /* input format given with geo file */
  {
    sprintf( callFdBinauralRenderer, "%s -if %s -of %s -fdbs %s -cicpIn %d -inGeo %s -profile %d -level %d", pData->binaryFdBinauralRenderer, pData->inPath, pData->outPath, readBRIRparams, pData->outputFormat, pData->outGeo, pData->profile, pData->level);
    sprintf ( tmpString, " >> %s 2>&1", pData->logfilename);
    strcat ( callFdBinauralRenderer, tmpString );
  }
  else /* input format given with CICP index */
  {
    sprintf( callFdBinauralRenderer, "%s -if %s -of %s -fdbs %s -cicpIn %d", pData->binaryFdBinauralRenderer, pData->inPath, pData->outPath, readBRIRparams, pData->outputFormat );
    sprintf ( tmpString, " >> %s 2>&1", pData->logfilename);
    strcat ( callFdBinauralRenderer, tmpString );
  }

  if (0 != callBinary(callFdBinauralRenderer, "FD binaural renderer", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}




int FdBinauralParametrizationRun (  FdBinauralParametrizationDataHandle pData, const VERBOSE_LEVEL verboseLvl )
{
  char callFdBinauralParam[4*FILENAME_MAX];
  char tmpString[FILENAME_MAX];

  sprintf( callFdBinauralParam, "%s -firbs %s -fdbs %s", pData->binaryFdBinauralParametrization, pData->brirBitstreamFileIn, pData->brirBitstreamFileOut);
  sprintf( tmpString, " >> %s 2>&1", pData->logfilename);
  strcat ( callFdBinauralParam, tmpString );
  if (0 != callBinary(callFdBinauralParam, "FD binaural parametrization", verboseLvl, 0)) {
    return -1;
  }


  return 0;
}

int FdBinauralBitstreamWriterRun ( FdBinauralBitstreamWriterDataHandle pData, const VERBOSE_LEVEL verboseLvl )
{
  char callFdBinauralBitstreamWriter[4*FILENAME_MAX];
  char tmpString[FILENAME_MAX];

  sprintf( callFdBinauralBitstreamWriter, "%s -wavprefix %s -bs %s -geo %s", 
    pData->binaryFdBinauralBitstreamWriter, pData->brirInFolder, pData->brirBitstreamFileOut, pData->geometryInPath);
  sprintf ( tmpString, " >> %s 2>&1", pData->logfilename);
  strcat ( callFdBinauralBitstreamWriter, tmpString );

  if (0 != callBinary(callFdBinauralBitstreamWriter, "FD binaural bitstream writer", verboseLvl, 0)) {
    return -1;
  }

  return 0;
}




int FdBinauralParametrizationExit ( FdBinauralParametrizationDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

int FdBinauralRendererExit ( FdBinauralRenderingDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

int FdBinauralBitstreamWriterExit ( FdBinauralBitstreamWriterDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}