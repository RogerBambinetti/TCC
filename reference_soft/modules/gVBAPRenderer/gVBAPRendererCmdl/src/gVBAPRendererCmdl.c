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

#include "gVBAPRenderer.h"

#include "oam_read.h"
#include "wavIO.h"

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

/* This function is used to read a single data vector from an OAM file. */
static int get_oam_sample(FILE* oam_file,OAM_SAMPLE* startPosition, OAM_SAMPLE* stopPosition, StructOamMultidata* oam_sample, uint16_t oam_version, uint16_t hasDynamicObjectPriority, uint16_t hasUniformSpread);

int main(int argc, char* argv[])
{
  HANDLE_GVBAPRENDERER hgVBAPRenderer = NULL;
  HANDLE_GVBAPRENDERER hgVBAPRendererReal = NULL;
  HANDLE_GVBAPRENDERER hgVBAPRendererImag = NULL;
  int error = 0;
  char inFile [FILENAME_MAX] = {'\0'};
  char qmfInFileReal[200];
  char qmfInFileImag[200];

  char outFile[FILENAME_MAX] = {'\0'};
  char qmfOutFileReal[200];
  char qmfOutFileImag[200];

  char oamFile[FILENAME_MAX] = {'\0'};
  char outGeometryFile[FILENAME_MAX] = {'\0'};

  FILE* fIn  = NULL;
  FILE *fQmfInFileReal = NULL;
  FILE *fQmfInFileImag = NULL;

  FILE* fOut = NULL;
  FILE *fQmfOutFileReal = NULL;
  FILE *fQmfOutFileImag = NULL;

  FILE* fOAMFile = NULL;

  char* wavExtension  = ".wav";

  int bQmfInput = 0;
  int bQmfOut = 0;
  int bQmfRendering = 0;
  
  int numChannels = 0;
  int numLFE = 0;
  int frameLength = 0;
  int numQmfBands = 64;

  int i = 0;
  int argCount = 0;
  int cicpIndex = -1;

  uint16_t numObjects = 0;
  uint16_t oamVersion = 2;
  uint16_t hasDynamicObjectPriority = 0;
  uint16_t hasUniformSpread = 0;

  StructOamMultidata* oamSample;
 
  OAM_SAMPLE* startPosition = NULL;
  OAM_SAMPLE* stopPosition  = NULL;
 
  float** inBuffer  = NULL;
  float** outBuffer = NULL;

  float** inImagBuffer  = NULL;
  float** outImagBuffer = NULL;

 
  WAVIO_HANDLE hWavIO = NULL;
  WAVIO_HANDLE hWavIOReal = NULL;
  WAVIO_HANDLE hWavIOImag = NULL;
  unsigned int nInChannels = 0;
  unsigned int InSampleRate;
  unsigned long nTotalSamplesPerChannel;
  unsigned long nTotalSamplesWrittenPerChannel;
  unsigned int InBytedepth = 0;
  int nSamplesPerChannelFilled;
  unsigned int isLastFrame = 0;
  unsigned int isLastImagFrame = 0;

  FILE* file_divergenceOAM = NULL;
  char filename_divergenceOAM[FILENAME_MAX] = {'\0'};
  int divergenceOAMfile_set = 0;
  unsigned int hasDivergence[255] = {0};
  int profile = 3;

  MPEG3DAGVBAP_RETURN_CODE error_code = MPEG3DAGVBAP_OK;

  CICP2GEOMETRY_CHANNEL_GEOMETRY geometryInfo[32] = { {0} }; /*[cicpLoudspeakerIndex AZ EL isLFE] */

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                    Generalized VBAP Renderer Module                         *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
  
  for ( i = 1; i < argc; ++i )
  {
    int nWavExtensionChars = 0;

    if (!strcmp(argv[i], "-if"))	/* Required */
    {
      i++;
      argCount++;
      strncpy(inFile, argv[i], FILENAME_MAX) ;
      nWavExtensionChars = 0;
      nWavExtensionChars = strspn(wavExtension, inFile);
      if ( nWavExtensionChars != 4 ) {
        bQmfInput = 1;
        strcpy(qmfInFileReal, inFile);
        strcpy(qmfInFileImag, inFile);
        strcat(qmfInFileReal, "_real.qmf");
        strcat(qmfInFileImag, "_imag.qmf");
      }
      continue;
    }
    else if (!strcmp(argv[i], "-of"))	/* Required */
    {
      i++;
      argCount++;
      strncpy(outFile, argv[i], FILENAME_MAX) ;
      nWavExtensionChars = 0;
      nWavExtensionChars = strspn(wavExtension, outFile);
      if ( nWavExtensionChars != 4 ) {
        bQmfOut = 1;
        strcpy(qmfOutFileReal, outFile);
        strcpy(qmfOutFileImag, outFile);
        strcat(qmfOutFileReal, "_real.qmf");
        strcat(qmfOutFileImag, "_imag.qmf");
      }
      continue;
    }
    else if (!strcmp(argv[i], "-oamFile"))		/* Required */
    {
      i++;
      argCount++;
      strncpy(oamFile, argv[i], FILENAME_MAX) ;
      continue;
    }
    else if (!strcmp(argv[i], "-cicpOut"))		/* Required */
    {
      i++;
      argCount++;
      cicpIndex = atoi(argv[i]);
      continue;
    }
    else if (!strcmp(argv[i], "-outGeo"))		/* Required */
    {
      i++;
      argCount++;
      strncpy(outGeometryFile, argv[i], FILENAME_MAX);
      continue;
    }
    else if (!strcmp(argv[i], "-oamDivFile"))		/* Optional */
    {
      i++;
      argCount++;
      strncpy(filename_divergenceOAM, argv[i], FILENAME_MAX);

      divergenceOAMfile_set = 1;
      continue;
    }
   }

  if ( ( bQmfInput != bQmfOut ) ) {
    fprintf( stdout, "\nERROR: Either both input files and output file are FD or TD. Mixed domains are not allowed.\n\n");
    strcpy(inFile, "\0");
    strcpy(outFile, "\0");
  }
  else {
    bQmfRendering = bQmfInput;
  }

  if ( argCount < 4 || inFile[0] == '\0' || outFile[0] == '\0' )
  {
    fprintf(stderr, "invalid arguments:\n"); 
    fprintf(stderr, "Usage: %s -if <input.wav> -of <output.wav> -oamFile <in.oam> -cicpOut <CICPIdx> -outGeo <geometry.txt>\n", argv[0]);
    
    return -1;
  }

  /* open OAM file for reading and get num_objects */
  fOAMFile = oam_read_open(oamFile, &numObjects, &oamVersion, &hasDynamicObjectPriority, &hasUniformSpread);

  /* initialization of oam_multidata structure */
  oamSample = oam_multidata_create(numObjects, 1);

  /* allocate memory for position data */
  startPosition = (OAM_SAMPLE*)calloc(numObjects, sizeof(OAM_SAMPLE));
  stopPosition  = (OAM_SAMPLE*)calloc(numObjects, sizeof(OAM_SAMPLE));
 

  /* get initial OAM values */
  if ( frameLength = get_oam_sample(fOAMFile, startPosition, stopPosition, oamSample, oamVersion, hasDynamicObjectPriority, hasUniformSpread) ) {
    int obj = 0;
    /* oam sample for sample pos 0 is not available in real-world applications / per OAM definition 
        -> sample and hold the OAM value valid for the end of the very first audio frame  */
    for ( obj = 0; obj < numObjects; obj++ ) {
      startPosition[obj].azimuth             = stopPosition[obj].azimuth;
      startPosition[obj].elevation           = stopPosition[obj].elevation;
      startPosition[obj].gain                = stopPosition[obj].gain;
      startPosition[obj].radius              = stopPosition[obj].radius;
      startPosition[obj].spread_angle        = stopPosition[obj].spread_angle;
      startPosition[obj].spread_angle_height = stopPosition[obj].spread_angle_height;
      startPosition[obj].spread_depth        = stopPosition[obj].spread_depth;
    }
  }
  else {
    /* get first valid OAM start+ stop samples and frame length */
    frameLength = get_oam_sample(fOAMFile, startPosition, stopPosition, oamSample, oamVersion, hasDynamicObjectPriority, hasUniformSpread);
  }


  /* open .oam-file containing information about profile and divergence */
  if (divergenceOAMfile_set == 1)
  {
    file_divergenceOAM = fopen(filename_divergenceOAM, "rb");
    if (file_divergenceOAM == NULL)
    {
      fprintf(stderr, "Error opening %s in main().", filename_divergenceOAM);
      goto cleanup;
    }

    if (fread( &profile, sizeof(int), 1, file_divergenceOAM ) != 1)
    {
      fprintf(stderr, "Error in main(). Error reading profile information from %s.\n", filename_divergenceOAM);
      goto cleanup;
    }
    if (fread( hasDivergence, sizeof(unsigned int), numObjects, file_divergenceOAM ) != numObjects)
    {
      fprintf(stderr, "Error in main(). Error reading divergence information from %s.\n", filename_divergenceOAM);
      goto cleanup;
    }
    fclose(file_divergenceOAM);
  }

  /* sanity check to avoid endless loops */
  if ( frameLength == 0 ) {
    fprintf(stderr, "Error: frameLength must never be 0!\n");
    return -1;
  }

  /* Get Azimuth, Elevations from CICP */
  if ( -1 == cicpIndex )
  {
    /* Read configuration from txt file */
    if ( outGeometryFile[0] == '\0' )
    {
      fprintf(stderr, "Error: Required txt file containing geometry info not found\n");
      goto cleanup;
    }
    else
    {
      /* Read in geometry data from txt file */
       cicp2geometry_get_geometry_from_file(outGeometryFile, geometryInfo, &numChannels, &numLFE); 
    }
  }
  else
  {
    cicp2geometry_get_geometry_from_cicp(cicpIndex, geometryInfo, &numChannels, &numLFE); 
  }


   /* Open input file */
  if ( bQmfInput ) {
    fQmfInFileReal = fopen( qmfInFileReal, "rb" );
    fQmfInFileImag = fopen( qmfInFileImag, "rb" );
  }
  else {
    fIn = fopen(inFile, "rb");
  }

  if ( fIn != NULL) {
    fprintf(stdout, "Found input file: %s.\n", inFile );
  }
  else if ( fQmfInFileReal != 0 && fQmfInFileImag != 0 )  {
    fprintf(stdout, "Found input files: %s and %s.\n", qmfInFileReal, qmfInFileImag );
  }
  else {
    if ( bQmfInput ) {
      fprintf(stderr, "Could not open qmf input files: %s and/or %s.\n", qmfInFileReal, qmfInFileImag );
    }
    else {
      fprintf(stderr, "Could not open input file: %s.\n", inFile );
    }
    goto cleanup;
  }

  /* Open output file */
  if ( bQmfOut ) {
    fQmfOutFileReal = fopen( qmfOutFileReal, "wb" );
    fQmfOutFileImag = fopen( qmfOutFileImag, "wb" );
  }
  else {
    fOut = fopen(outFile, "wb");
  }

  if ( fOut != NULL) {
    fprintf(stdout, "Found output file: %s.\n", outFile );
  }
  else if ( fQmfOutFileReal != 0 && fQmfOutFileImag != 0 )  {
    fprintf(stdout, "Found output files: %s and %s.\n", qmfOutFileReal, qmfOutFileImag );
  }
  else {
    if ( bQmfOut )
      fprintf(stderr, "Could not open output files: %s and/or %s.\n", qmfOutFileReal, qmfOutFileImag );
    else {
      fprintf(stderr, "Could not open output file: %s.\n", outFile );
    }
    
    goto cleanup;
  }

  /* Init wavIO */
  if ( bQmfRendering ) {
    error = wavIO_init(&hWavIOReal, frameLength, 0, 0);
    if ( !error ) error = wavIO_init(&hWavIOImag, frameLength, 0, 0);
  }
  else {
    error = wavIO_init(&hWavIO, frameLength, 0, 0);
  }

  if ( 0 != error )
  {
    fprintf(stderr, "Error during initialization.\n");
    goto cleanup;
  }

  if ( bQmfRendering ) {
    unsigned int tmpImagInChannels = 0;
    unsigned int tmpImagInSampleRate = 0;
    unsigned int tmpImagInBytedepth = 0;
    unsigned long tmpImagnTotalSamplesPerChannel = 0;
    int tmpImagNSamplesPerChannelFilled = 0;

    error = wavIO_openRead(hWavIOReal, fQmfInFileReal, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
    if ( !error ) 
      error = wavIO_openRead(hWavIOImag, fQmfInFileImag, &tmpImagInChannels, &tmpImagInSampleRate, &tmpImagInBytedepth, &tmpImagnTotalSamplesPerChannel, &tmpImagNSamplesPerChannelFilled);

    /* sanity check whether *_real.qmf and *_imag.qmf headers are consistent */
    if ( tmpImagInChannels   != nInChannels  ||
         tmpImagInSampleRate != InSampleRate ||
         tmpImagInBytedepth  != InBytedepth  ||
         tmpImagnTotalSamplesPerChannel != nTotalSamplesPerChannel ||
         tmpImagNSamplesPerChannelFilled != nSamplesPerChannelFilled ) {
      fprintf(stderr, "Error: qmf real & imag file do not match / RIFF header missmatch.\n");
      goto cleanup;      
    }
  }
  else {
    error = wavIO_openRead(hWavIO, fIn, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
  }
  if ( nInChannels != numObjects )
  {
    fprintf(stderr, "Object count missmatch between oam and wav file\n");
    goto cleanup;
  }

  if ( bQmfRendering ) {
    error = wavIO_openWrite(hWavIOReal,fQmfOutFileReal, numChannels + numLFE, InSampleRate, InBytedepth);
    if (!error) error = wavIO_openWrite(hWavIOImag,fQmfOutFileImag, numChannels + numLFE, InSampleRate, InBytedepth);
  }
  else {
    error = wavIO_openWrite(hWavIO,fOut, numChannels + numLFE, InSampleRate, InBytedepth);
  }

  if ( 0 != error )
  {
    fprintf(stderr, "Error opening the output file.\n");
    goto cleanup;
  }

  /* alloc local buffers */
  inBuffer = (float**)calloc(nInChannels,sizeof(float*));
  for (i = 0; i < (int) nInChannels; i++)
    inBuffer[i] = (float*)calloc(frameLength,sizeof(float));

  outBuffer = (float**)calloc(numChannels + numLFE, sizeof(float*));
    for (i = 0; i< numChannels + numLFE; i++)
      outBuffer[i] = (float*)calloc(frameLength,sizeof(float));

  if ( bQmfRendering ) {
    inImagBuffer = (float**)calloc(nInChannels,sizeof(float*));
    for (i = 0; i < (int) nInChannels; i++)
	  inImagBuffer[i] = (float*)calloc(frameLength,sizeof(float));

    outImagBuffer = (float**)calloc(numChannels + numLFE, sizeof(float*));
	for (i = 0; i< numChannels + numLFE; i++)
	  outImagBuffer[i] = (float*)calloc(frameLength,sizeof(float));
  }

  if ( bQmfRendering ) {
    error = gVBAPRenderer_Open(&hgVBAPRendererReal, numObjects, frameLength, cicpIndex, 
                                geometryInfo, numChannels + numLFE);
    if (!error) error = gVBAPRenderer_Open(&hgVBAPRendererImag, numObjects, frameLength, cicpIndex, 
                                geometryInfo, numChannels + numLFE);
  }
  else {
    error = gVBAPRenderer_Open(&hgVBAPRenderer, numObjects, frameLength, cicpIndex, 
                                geometryInfo, numChannels + numLFE);
  }

  if ( error )
  {
    fprintf(stderr, "gVBAPRenderer_Open returned %d\n", error);
    goto cleanup;
  }

  /* Renderer loop */
  
  while ( 1 )
  {
    static int counter = 0;
    unsigned int nZerosPaddedBeginning = 0;
    unsigned int nZerosPaddedEnd = 0;
    unsigned int samplesReadPerChannel = 0;
    unsigned int samplesWrittenPerChannel = 0;


    int obj = 0;

    /* check if divergence and spread are applied simultaneously on the same object in LC mode */
    for ( obj = 0; obj < numObjects; obj++ )
    {
      if ( profile == 3 && startPosition[obj].spread_angle && hasDivergence[obj] )
      {
        fprintf ( stderr, "Error: divergence and spread can not be applied simultaneously on one object in the Low Complexity profile!\n");
        error_code = MPEG3DAGVBAP_ERROR_INVALID_STREAM;
        goto cleanup;
      }
    }


    if ( bQmfRendering ) {
      error = wavIO_readFrame(hWavIOReal,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
      if (!error) error = wavIO_readFrame(hWavIOImag,inImagBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
    }
    else {
      error = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
    }

    if ( error ) {
      fprintf ( stderr, "Error reading from file!");
      goto cleanup;
    }
 
    if ( bQmfRendering ) {
      gVBAPRenderer_RenderFrame_Frequency(hgVBAPRendererReal, inBuffer, outBuffer, startPosition, stopPosition, numQmfBands, hasUniformSpread);
      gVBAPRenderer_RenderFrame_Frequency(hgVBAPRendererImag, inImagBuffer, outImagBuffer, startPosition, stopPosition, numQmfBands, hasUniformSpread);
    }
    else {
      gVBAPRenderer_RenderFrame_Time(hgVBAPRenderer, inBuffer, outBuffer, startPosition, stopPosition, hasUniformSpread);
    }

    if ( bQmfRendering ) {
      error = wavIO_writeFrame(hWavIOReal,outBuffer, samplesReadPerChannel, &samplesWrittenPerChannel);
      if (!error) error = wavIO_writeFrame(hWavIOImag,outImagBuffer, samplesReadPerChannel, &samplesWrittenPerChannel);
    }
    else {
      error = wavIO_writeFrame(hWavIO,outBuffer, samplesReadPerChannel, &samplesWrittenPerChannel);
    }

    if ( error ) {
      fprintf ( stderr, "Error reading from file!");
      goto cleanup;
    }

    /* Get next positions */
    frameLength = get_oam_sample(fOAMFile, startPosition, stopPosition, oamSample, oamVersion, hasDynamicObjectPriority, hasUniformSpread);

    fprintf(stderr,"Rendering Frame %d\r", counter);
    counter++;

    if ( isLastFrame )
      break; 
  }

  if ( bQmfOut ) {
    error = wavIO_updateWavHeader(hWavIOReal, &nTotalSamplesWrittenPerChannel);
    error = wavIO_updateWavHeader(hWavIOImag, &nTotalSamplesWrittenPerChannel);
  }
  else {
    error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
  }


cleanup:

    oam_multidata_destroy(oamSample);

    /* close OAM file */
    oam_read_close(fOAMFile);

    if ( bQmfRendering ) {
      if (hWavIOReal) wavIO_close(hWavIOReal);
      if (hWavIOImag) wavIO_close(hWavIOImag);
    }
    else {
      if (hWavIO) wavIO_close(hWavIO);
    }

    if (inBuffer ) {
      for (i = 0; i < (int) nInChannels; i++)
        if ( inBuffer[i] ) free(inBuffer[i]);
      free(inBuffer);
    }

    if ( outBuffer ) {
      for (i = 0; i< numChannels + numLFE; i++)
        if ( outBuffer[i] )free(outBuffer[i]);
      free(outBuffer);
    }

    if ( bQmfRendering ) {
      if ( inImagBuffer ) {
        for (i = 0; i < (int) nInChannels; i++)
          if ( inImagBuffer[i] ) free(inImagBuffer[i]);
        free(inImagBuffer);
      }

      if ( outImagBuffer ) {
        for (i = 0; i< numChannels + numLFE; i++)
          if ( outImagBuffer[i] ) free(outImagBuffer[i]);
        free(outImagBuffer);
      }
    } 



    if ( stopPosition )
      free(stopPosition);

    if ( startPosition )
      free(startPosition);

    if ( bQmfRendering ) {
      if ( hgVBAPRendererReal ) gVBAPRenderer_Close(hgVBAPRendererReal);
      if ( hgVBAPRendererImag ) gVBAPRenderer_Close(hgVBAPRendererImag);
    }
    else {
      if ( hgVBAPRenderer ) gVBAPRenderer_Close(hgVBAPRenderer);
    }

    return error_code;
  }



static int get_oam_sample(FILE* oam_file,
                          OAM_SAMPLE* startPosition, OAM_SAMPLE* stopPosition,
                          StructOamMultidata* oam_sample,
                          uint16_t oam_version,
                          uint16_t hasDynamicObjectPriority,
                          uint16_t hasUniformSpread)
{
    uint64_t sample_pos;
    int num_objects = oam_sample->size1;
    int n;

    DYNAMIC_OBJECT_PRIO hasDynObjPrio = DYN_OBJ_PRIO_NOT_NEEDED;
    if ( hasDynamicObjectPriority ) {
      hasDynObjPrio = DYN_OBJ_PRIO_AVAILABLE;
    }
    else{
      hasDynObjPrio = DYN_OBJ_PRIO_NOT_NEEDED;
    }

    sample_pos = oam_sample->sample[0];

    oam_read_process(oam_file, oam_sample, &num_objects, oam_version, hasDynObjPrio, hasUniformSpread);

    for (n = 0; n < num_objects; n++)
    {
      startPosition[n].azimuth              = stopPosition[n].azimuth;
      startPosition[n].elevation            = stopPosition[n].elevation;
      startPosition[n].radius               = stopPosition[n].radius;
      startPosition[n].gain                 = stopPosition[n].gain;
      startPosition[n].spread_angle         = stopPosition[n].spread_angle;
      startPosition[n].spread_angle_height  = stopPosition[n].spread_angle_height;
      startPosition[n].spread_depth         = stopPosition[n].spread_depth;


      stopPosition[n].azimuth               = oam_sample->azimuth[n]   * (float)(M_PI / 180.0);
      stopPosition[n].elevation             = oam_sample->elevation[n] * (float)(M_PI / 180.0);
      stopPosition[n].radius                = oam_sample->radius[n];
      stopPosition[n].gain                  = oam_sample->gain[n];
      stopPosition[n].spread_angle          = oam_sample->spread[n] * (float)(M_PI / 180.0);
      stopPosition[n].spread_angle_height   = oam_sample->spread_height[n] * (float)(M_PI / 180.0);
      stopPosition[n].spread_depth          = oam_sample->spread_depth[n];
    }

    return (int)(oam_sample->sample[0] - sample_pos);
}
