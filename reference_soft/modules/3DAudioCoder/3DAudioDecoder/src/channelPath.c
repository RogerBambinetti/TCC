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
#include "channelPath.h"
#include "formatConverter_api.h"

/* Set depending on platform type in main() */
extern int bUseWindowsCommands;
extern int bIarBinaryPresent;
extern MODULE_PROCESSING_DOMAINS moduleProcessingDomains;

typedef struct ChannelPathData_
{
  CICP_FORMAT         audioChannelLayout;
  CICP_FORMAT         reproductionLayout;
  int                 audioChannelLayoutNumChannels;
  int                 reproductionLayoutNumChannels;
  char*               binaryDmxMatrixDecoder;
  char*               binaryFormatConverter;
  char*               logfile;
  char*               inGeoFile;
  char*               outGeoFile;
  char*               outGeoDevFile;
  int                 delay;
  int                 passiveDownmixFlag;
  int                 downmixId;
  int                 numChannels;
  DRC_PARAMS          drcParams;
  DOMAIN_SWITCH       domainSwitchingDecisionsPostDrc1;
  PROCESSING_DOMAIN   moduleProcessingDomainsDrc1;
#if IAR
  char * binaryIarFormatConverter;
#endif
  DOMAIN_SWITCH domainSwitchPreFormatConverter;
  char* binaryDomainSwitcher;
  int phaseAlignStrength;
  int immersiveDownmixFlag;
  int matrixBitSize;
  char* downmixMatrixPayload;
} ChannelPathData;

int ChannelPathInit(ChannelPathDataHandle* ppData,
                    CICP_FORMAT audioChannelLayout,
                    CICP_FORMAT reproductionLayout,
                    char *inGeoFile,
                    char *outGeoFile,
                    char *outGeoDevFile,
                    char *logfile,
                    char* binaryFormatConverter,
                    int passiveDownmixFlag,
                    int numChannels,
                    DRC_PARAMS drcParams,
                    DOMAIN_SWITCH domainSwitchingDecisionsPostDrc1,
                    PROCESSING_DOMAIN moduleProcessingDomainsDrc1,
#if IAR
                    char * binaryIarFormatConverter,
#endif
                    DOMAIN_SWITCH domainSwitchPreFormatConverter,
                    char* binaryDomainSwitcher,
                    int phaseAlignStrength,
                    int immersiveDownmixFlag,
                    int downmixId,
                    int matrixBitSize,
                    char* downmixMatrix,
                    char* binaryDmxMatrixDecoder
) {
  int error = 0;
  int nChannelsAL = 0;
  int nChannelsRL = 0;

  error = getNumberOfCicpChannels(audioChannelLayout, &nChannelsAL);
  if (0 != error) {
    return error;
  }

  error = getNumberOfCicpChannels(reproductionLayout, &nChannelsRL);
  if (0 != error) {
    return error;
  }

  ( *ppData ) = ( ChannelPathDataHandle ) malloc ( sizeof ( ChannelPathData ) );
  ( *ppData )->audioChannelLayout = audioChannelLayout;
  ( *ppData )->reproductionLayout = reproductionLayout;
  ( *ppData )->audioChannelLayoutNumChannels = nChannelsAL;
  ( *ppData )->reproductionLayoutNumChannels = nChannelsRL;
  ( *ppData )->binaryFormatConverter = binaryFormatConverter;
  ( *ppData )->logfile = logfile;
  ( *ppData )->inGeoFile = inGeoFile;
  ( *ppData )->outGeoFile = outGeoFile;
  ( *ppData )->outGeoDevFile = outGeoDevFile;
  ( *ppData )->delay = 0;
  ( *ppData )->passiveDownmixFlag = passiveDownmixFlag;
  ( *ppData )->phaseAlignStrength = phaseAlignStrength;
  ( *ppData )->downmixId = downmixId;
  ( *ppData )->matrixBitSize = matrixBitSize;
  ( *ppData )->downmixMatrixPayload = downmixMatrix;
  ( *ppData )->immersiveDownmixFlag = immersiveDownmixFlag;
  ( *ppData )->numChannels = numChannels;
  ( *ppData )->drcParams = drcParams;
  ( *ppData )->domainSwitchingDecisionsPostDrc1 = domainSwitchingDecisionsPostDrc1;
  ( *ppData )->moduleProcessingDomainsDrc1 = moduleProcessingDomainsDrc1;
#if IAR
  ( *ppData )->binaryIarFormatConverter = binaryIarFormatConverter;
#endif
  ( *ppData )->binaryDomainSwitcher = binaryDomainSwitcher;
  ( *ppData )->domainSwitchPreFormatConverter = domainSwitchPreFormatConverter;
  ( *ppData )->binaryDmxMatrixDecoder = binaryDmxMatrixDecoder;

  return error;
}

int ChannelPathExit ( ChannelPathDataHandle* ppData )
{
  if ( *ppData )
  {
    free ( *ppData );
    *ppData = NULL;
  }

  return 0;
}

#if IAR
int ChannelPathRun ( ChannelPathDataHandle pData, char* infile, char* outfile, char* r3tfile, const PROFILE profile, const VERBOSE_LEVEL verboseLvl )
#else
int ChannelPathRun ( ChannelPathDataHandle pData, char* infile, char* outfile, const PROFILE profile, const VERBOSE_LEVEL verboseLvl )
#endif
{
  char callDmxMatrixDecoder[ 3 * FILENAME_MAX];
  char callFormatConverter[3 * FILENAME_MAX];
  char callFileCopy[3 * FILENAME_MAX];
  char tmpString[3 * FILENAME_MAX];
  /* Bitstream format of the downmix matrix. Has to be saved temporarily to hand over to the matrix decoder. */
  char* tmpDmxMatrixInputFile = "tmpFile3Ddec_dmxMatrixBS.dat";
  /* Output files of the downmix matrix decoder */
  char* matrixOutputFile      = "tmpFile3Ddec_dmxMatrix.dat";
  char* eqOutputFile          = "tmpFile3Ddec_dmxEq.dat";
#ifdef RM6_INTERNAL_CHANNEL
  char icLayoutFilename[256]  = "tmpFile3Ddec_icLayout.txt";
#endif
#ifdef IAR	
  char iarFilename[256]       = "tmpFile3Ddec_fmt.txt";
#endif
  CICP_FORMAT cicpIn          = pData->audioChannelLayout;
  CICP_FORMAT cicpOut         = pData->reproductionLayout;
  /* Output file of DRC 1 decoder */
  char tmpDrc1OutputFile[3 * FILENAME_MAX];
  DRC_PARAMS* drcParams       = &pData->drcParams;

  /**********************************/
  /* DRC-1 Decoder                  */
  /**********************************/
    
  if (drcParams->drcDecoderState) {
    char callDrc1[3 * FILENAME_MAX];
        
    if (pData->moduleProcessingDomainsDrc1 != NOT_DEFINED)
    {
      if(pData->moduleProcessingDomainsDrc1 == QMF_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ChannelOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetChannelPath, pData->numChannels, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 1 ");
        strcat(  callDrc1, "-gd 32 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == STFT_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ChannelOut" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetChannelPath, pData->numChannels, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 2 ");
        strcat(  callDrc1, "-gd 128 ");
      }
      else if(pData->moduleProcessingDomainsDrc1 == TIME_DOMAIN)
      {
        sprintf( tmpDrc1OutputFile, "tmpFile3Ddec_Drc1ChannelOut.wav" );
        sprintf( callDrc1, "%s -if %s -of %s -aco %i -acp %i -afs %i ", drcParams->binaryDrcDecoder, infile, tmpDrc1OutputFile, drcParams->drcChannelOffsetChannelPath, pData->numChannels, drcParams->framesize);
        strcat(  callDrc1, "-decId 0 -gdt 0 ");
      }
      sprintf( tmpString, "-ic %s -il %s -ig %s -is %s -v 0 ", drcParams->bsMpegh3daUniDrcConfig, drcParams->bsMpegh3daLoudnessInfoSet, drcParams->bsUnDrcGain, drcParams->txtDrcSelectionTransferData);
      strcat(  callDrc1, tmpString);
      sprintf( tmpString, "-dms %s ", drcParams->txtDownmixMatrixSet);
      strcat(  callDrc1, tmpString);

      if (0 != callBinary(callDrc1, "DRC-1 decoder on channel path", verboseLvl, 0)) {
        return -1;
      }
    }
    else
    {
      strcpy(tmpDrc1OutputFile,infile);
    }
        
    if (pData->domainSwitchingDecisionsPostDrc1 == QMF_ANALYSIS)
    {
      removeWavExtension(tmpDrc1OutputFile);
      transformT2F(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 0);
    }
    else if (pData->domainSwitchingDecisionsPostDrc1 == QMF_SYNTHESIS)
    {
      transformF2T(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 0);
    }
    else if (pData->domainSwitchingDecisionsPostDrc1 == STFT_ANALYSIS)
    {
      transformT2F(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 1);
    }
    else if (pData->domainSwitchingDecisionsPostDrc1 == STFT_SYNTHESIS)
    {
      transformF2T(tmpDrc1OutputFile, pData->binaryDomainSwitcher, 1);
    }
  } else {
    strcpy(tmpDrc1OutputFile,infile);
  }

  if ( pData->domainSwitchPreFormatConverter == QMF_ANALYSIS ) {
    removeWavExtension( tmpDrc1OutputFile );
    transformT2F( tmpDrc1OutputFile, pData->binaryDomainSwitcher, 0);
  }

  /* do format conversion if output geometry is different to input geometry */
  if ( CompareInOutGeometry( pData->audioChannelLayout, pData->reproductionLayout, pData->inGeoFile, pData->outGeoFile ) )
  {
    int delayFormatConverter;

    int pas, aes, dmx;

    /* Get delay */
    FORMAT_CONVERTER_MODE mode = FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN; 
    delayFormatConverter =  formatConverterGetDelaySamples ( mode );
    delayFormatConverter += 961; /* non-ld QMF (577) + hybrid fb (384)*/
    pData->delay = delayFormatConverter;

    if ( moduleProcessingDomains.formatConverter == STFT_DOMAIN || moduleProcessingDomains.formatConverter == TIME_DOMAIN ) {
      dmx = 6;
    }
    else {
      dmx = 4;
    }

    if( pData->passiveDownmixFlag != 0 )
    {
      pas = 0;
      aes = 0;
    } else {
      if( pData->phaseAlignStrength >= 0 && pData->phaseAlignStrength <= 7 )
      {
        pas = pData->phaseAlignStrength;
      }
      else
      {
        pas = 3;
      }
      aes = 7;
    }

#if IAR
    if ( pData->immersiveDownmixFlag )
    {
      if ( bIarBinaryPresent == 0 ) {
        fprintf(stderr, "Error: No IAR FormatConverter binary available but IAR FormatConverter is triggered!\n\n");
        return -1;
      }

      if ( pData->reproductionLayout == CICP2GEOMETRY_CICP_3_2_0 || pData->reproductionLayout == CICP2GEOMETRY_CICP_3_2_1 )
      {
        sprintf ( callFormatConverter, "\"%s\" -if %s -of %s -cicpIn %d -cicpOut %d -d -dmx %i -aes %d -3DType ", pData->binaryIarFormatConverter, tmpDrc1OutputFile, outfile, cicpIn, cicpOut, dmx, aes );
        strcat ( callFormatConverter, iarFilename );
      }
      else
      {
        if (4 == dmx) {
          sprintf ( callFormatConverter, "\"%s\" -if %s -of %s -d -dmx %i -aes %d -pas %d ", pData->binaryFormatConverter, tmpDrc1OutputFile, outfile, dmx, aes, pas );
        } else {
          sprintf ( callFormatConverter, "\"%s\" -if %s -of %s -d -dmx %i -aes %d ", pData->binaryFormatConverter, tmpDrc1OutputFile, outfile, dmx, aes );
        }
      }
    }
    else
    {
      if (4 == dmx) {
        sprintf ( callFormatConverter, "\"%s\" -if %s -of %s -d -dmx %i -aes %d -pas %d ", pData->binaryFormatConverter, tmpDrc1OutputFile, outfile, dmx, aes, pas );
      } else {
        sprintf ( callFormatConverter, "\"%s\" -if %s -of %s -d -dmx %i -aes %d ", pData->binaryFormatConverter, tmpDrc1OutputFile, outfile, dmx, aes );
      }
    }
    if( pData->downmixId >= 0 )
    {
      fprintf(stderr, "Downmix matrix with ID %d should be applied.\n", pData->downmixId);
    }
#else
    if(pData->immersiveDownmixFlag && ((pData->reproductionLayout == 5) || (pData->reproductionLayout == 6)))
    {
      fprintf(stderr, "Immersive downmix not yet provided, will perform casual downmix.\n");
    }
    if( pData->downmixId >= 0 )
    {
      fprintf(stderr, "Downmix matrix with ID %d should be applied.\n", pData->downmixId);
    }

    sprintf ( callFormatConverter, "%s -if %s -of %s -d -dmx 4 -aes %d -pas %d ", pData->binaryFormatConverter, tmpDrc1OutputFile, outfile, aes, pas );
#endif

    if (pData->audioChannelLayout == CICP_FROM_GEO ) {
      sprintf ( tmpString, " -inGeo %s ", pData->inGeoFile );
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

    /* If a downmix matrix payload is given the dmxMatrix decoder 
       is called and the output is appended to the call to the 
       format converter */
    if( pData->matrixBitSize > 0 )
    {
      FILE *binMatrixFile = fopen(tmpDmxMatrixInputFile, "w");
      int res = fputs(pData->downmixMatrixPayload, binMatrixFile);
      if( res == EOF )
      {
        fprintf( stderr, "\n\n Error creating temporary downmix matrix file.\n");
        return -1;
      }
      fclose(binMatrixFile);
      sprintf( callDmxMatrixDecoder, "\"%s\" -if %s -of_mat %s -of_eq %s -CICPin %d -CICPout %d >> %s 2>&1", pData->binaryDmxMatrixDecoder, tmpDmxMatrixInputFile, matrixOutputFile, eqOutputFile, pData->audioChannelLayout, pData->reproductionLayout, pData->logfile);

      if (0 != callBinary(callDmxMatrixDecoder, "Downmix Matrix decoder", verboseLvl, 0)) {
        return -1;
      }
      cicpIn  = CICP_ANY;
      cicpOut = CICP_ANY;
      sprintf( tmpString, " -inNch %d -outNch %d", pData->audioChannelLayoutNumChannels, pData->reproductionLayoutNumChannels);
      strcat ( callFormatConverter, tmpString );

      sprintf( tmpString, " -dmxMtx %s -extEq %s", matrixOutputFile, eqOutputFile);
      strcat ( callFormatConverter, tmpString );
    }

#ifdef RM6_INTERNAL_CHANNEL
    if ( pData->audioChannelLayout != CICP_FROM_GEO && pData->reproductionLayout == 2 && profile != PROFILE_LOW_COMPLEXITY ) {
      sprintf( tmpString, " -cicpIn -1 -cicpOut %d -inGeo %s", cicpOut, icLayoutFilename);
    } else {
      sprintf( tmpString, " -cicpIn %d -cicpOut %d", cicpIn, cicpOut);
    }
    strcat ( callFormatConverter, tmpString );
#else
    sprintf( tmpString, " -cicpIn %d -cicpOut %d", cicpIn, cicpOut);
    strcat ( callFormatConverter, tmpString );
#endif

    sprintf ( tmpString, " >> %s 2>&1", pData->logfile);
    strcat ( callFormatConverter, tmpString );

    if (0 != callBinary(callFormatConverter, "Format Converter", verboseLvl, 0)) {
      return -1;
    }
  }
  else
  {
    int tmpStringHasWav = checkIfWavExtension(tmpDrc1OutputFile);
    int outfileHasWav = checkIfWavExtension(outfile);
    PROCESSING_DOMAIN bCoreCoder = TIME_DOMAIN;

    if ( tmpStringHasWav != outfileHasWav ) {
      fprintf ( stderr, "\n\n Error copying file in channel path: mixed domains!\n" );
      return -1;
    }

     if ( !tmpStringHasWav && !outfileHasWav ) {
       bCoreCoder = QMF_DOMAIN;
    }

    pData->delay = 0;


    /* make copy */
    if( bUseWindowsCommands ) {
      if ( bCoreCoder ==  TIME_DOMAIN ) {
      /*  time domain */
        sprintf ( callFileCopy, "copy %s %s > NUL", tmpDrc1OutputFile, outfile );
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
      }
      else {
        /*  frequency domain - copy real and imag files */
        sprintf ( callFileCopy, "copy %s_real.qmf %s_real.qmf > NUL", tmpDrc1OutputFile, outfile );
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
        sprintf ( callFileCopy, "copy %s_imag.qmf %s_imag.qmf > NUL", tmpDrc1OutputFile, outfile );
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
      }
    }
    else {
      if ( bCoreCoder ==  TIME_DOMAIN ) {
        /*  time domain */
        sprintf ( callFileCopy, "cp %s %s ", tmpDrc1OutputFile, outfile );       
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
      }
      else {
        /*  frequency domain - copy real and imag files */
        sprintf ( callFileCopy, "cp %s_real.qmf %s_real.qmf", tmpDrc1OutputFile, outfile );
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
        sprintf ( callFileCopy, "cp %s_imag.qmf %s_imag.qmf", tmpDrc1OutputFile, outfile );
        if ( system ( callFileCopy ) ) {
          fprintf ( stderr, "\n\n Error copying file in channel path.\n" );
          return -1;
        }
      }
    }
  }

  return 0;
}

int ChannelPathGetInfo ( ChannelPathDataHandle pData, int* pDelay )
{
  if ( pData && pDelay )
  {
    *pDelay = pData->delay;
  }

  return 0;
}

int CompareInOutGeometry(int cicpIn, int cicpOut, char *inGeoFile, char *outGeoFile)
{

  CICP2GEOMETRY_CHANNEL_GEOMETRY geoIn[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numChannelsIn;
  int numLFEsIn;
  CICP2GEOMETRY_CHANNEL_GEOMETRY geoOut[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numChannelsOut;
  int numLFEsOut;
    
  if ( cicpIn == CICP_FROM_GEO ) 
  {
    cicp2geometry_get_geometry_from_file( inGeoFile, geoIn, &numChannelsIn, &numLFEsIn );
  }
  else
  {
    cicp2geometry_get_geometry_from_cicp( cicpIn, geoIn, &numChannelsIn, &numLFEsIn );
  }

  if ( cicpOut == CICP_FROM_GEO ) 
  {
    cicp2geometry_get_geometry_from_file( outGeoFile, geoOut, &numChannelsOut, &numLFEsOut );
  }
  else
  {
    cicp2geometry_get_geometry_from_cicp( cicpOut, geoOut, &numChannelsOut, &numLFEsOut );
  }

  return cicp2geometry_compare_geometry( geoIn, (unsigned int)(numChannelsIn + numLFEsIn), geoOut, (unsigned int)(numChannelsOut + numLFEsOut), 0 );

}

int getNumberOfOutputChannels(int cicpOut, char *outGeoFile, const int numChannelsAsc)
{

  CICP2GEOMETRY_CHANNEL_GEOMETRY geoOut[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int numChannelsOut = 0;
  int numLFEsOut = 0;


  if (cicpOut == CICP_FROM_GEO) {
    cicp2geometry_get_geometry_from_file( outGeoFile, geoOut, &numChannelsOut, &numLFEsOut );
  } else if (cicpOut == CICP_CONTRIBUTION) {
    numChannelsOut = numChannelsAsc;
    numLFEsOut = 0;
  } else {
    cicp2geometry_get_geometry_from_cicp(cicpOut, geoOut, &numChannelsOut, &numLFEsOut);
  }

  return numChannelsOut + numLFEsOut;

}
