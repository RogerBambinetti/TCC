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

#include "oam_encode_rapV4.h"
#include "oam_encode_ld.h"
#include "oam_common.h"
#include "oam_read.h"
#include "oam_bitbuf_write.h"

#define OAM_ENC_BUFSIZE 1024

  /* oam options */
  uint16_t oam_version = 0;
  DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority = DYN_OBJ_PRIO_NOT_NEEDED;
  int hasUniformSpread = 1;

  /* data handles - low delay*/
  StructOamMultidata* oamInputData;
  StructOamMultidata* oamPreviousData;
  StructOamMultidata* oamDiffValues;
  int fixedValues[6];

  /* data handles - high eff*/
  StructOamPolChunk* pol_dframe = NULL;
  StructOamMultidata* chunk;
  StructOamMultidata* chunk_i;
  StructOamMultidata* chunk_d;
  StructOamMultidata* first;
  StructOamMultidata* last;
  /* bitstream and file handles */
  FILE* file;
  OamPBS pbs;
  OamPBS pbs_highrate;
  int frame_idx = 0;

typedef struct
{
  char filenameInput[OAM_ENC_BUFSIZE];
  char filenameOutput[OAM_ENC_BUFSIZE];
  int coreBlocksize;
  int oamBlocksize;
  int highRate;
  int iframe_period;
  int replace_radius;
  int dummy_frames;
  int lowDelay;
  char filenameOutput_hasDynamicObjectPriority[OAM_ENC_BUFSIZE];
} ConfigObjectMetadataEncoder;

static void printCmdlHelp(){

  fprintf( stdout, "\n"); 
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "OAM Encoder Module - Commandline Help:\n");
  fprintf( stdout, "                                                                     \n");
  fprintf( stdout, "Required:                                                            \n");
  fprintf( stdout, "-if filename.oam : Input file (OAM-file)                             \n");
  fprintf( stdout, "-of filename     : Output file name (without file extension)         \n");
  fprintf( stdout, "                                                                     \n");
  fprintf( stdout, "Optional:                                                            \n");
  fprintf( stdout, "-ld *          : 1 enables low delay mode \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "-dummyFrames * : Number of delay frames (in core coder frames) \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "-coreBs *      : core coder granularity in audio samples \n");
  fprintf( stdout, "                 default: 1024\n");
  fprintf( stdout, "-oamBs *       : oam granularity in audio samples \n");
  fprintf( stdout, "                 default: 1024\n");
  fprintf( stdout, "-highRate *    : activate high rate mode (if coreBs = x*oamBs) \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "-ip *          : i-frame period (in core coder frames) \n");
  fprintf( stdout, "                 default: 24\n");
  fprintf( stdout, "-rr *          : set replace radius flag \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "-of_hasDynamicObjectPriority name : create hasDynamicObjectPriority outputfile \n");
  fprintf( stdout, "                 default: none\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
}

static int oam_encoder_get_config(int argc, char* argv[], ConfigObjectMetadataEncoder* config)
{
  int i = 0;
  int ret = 0;

  /* default parameter */
  config->filenameInput[0]  = '\0';
  config->filenameOutput[0] = '\0';
  config->coreBlocksize     = 1024;
  config->oamBlocksize      = 1024;
  config->highRate          = 0;
  config->iframe_period     = -1;
  config->replace_radius    = 0;
  config->dummy_frames      = 0;
  config->lowDelay          = 0;
  config->filenameOutput_hasDynamicObjectPriority[0] = '\0';

  for ( i = 1; i < argc-1; ++i )
    {
      if (!strcmp(argv[i], "-if"))    /* Required */
        {
          strncpy(config->filenameInput, argv[i+1], OAM_ENC_BUFSIZE) ;
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-of"))  /* Required */
        {
          strncpy(config->filenameOutput, argv[i+1], OAM_ENC_BUFSIZE) ;
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-dummyFrames"))  /* Optional */
        {
          config->dummy_frames = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-coreBs"))  /* Optional */
        {
          config->coreBlocksize = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-oamBs"))  /* Optional */
        {
          config->oamBlocksize = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-highRate"))  /* Optional */
        {
          config->highRate = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-ip"))  /* Optional */
        {
          config->iframe_period = atoi(argv[i+1]);
          i++;
          continue;
        } 
      else if (!strcmp(argv[i], "-rr"))  /* Optional */
        {
          config->replace_radius = atoi(argv[i+1]);
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-of_hasDynamicObjectPriority"))  /* Optional */
        {
          strncpy(config->filenameOutput_hasDynamicObjectPriority, argv[i+1], OAM_ENC_BUFSIZE) ;
          i++;
          continue;
        }
      else if (!strcmp(argv[i], "-ld"))  /* Optional */
        {
          config->lowDelay = atoi(argv[i+1]);
          i++;
          continue;
        }
    }

  /* check parameter values */
  if (config->filenameInput[0] == '\0')
    {
      fprintf(stderr, "Missing or wrong command line parameter: input filename (-if FILE)\n");
      ret = -1;
    }
  if (config->filenameOutput[0] == '\0')
    {
      fprintf(stderr, "Missing or wrong command line parameter: output filename (-of FILE)\n");
      ret = -1;
    }
  if (config->coreBlocksize != 1024 &&
      config->coreBlocksize != 2048 &&
      config->coreBlocksize != 4096)
    {
      fprintf(stderr, "Invalid command line parameter: unsupported blocksize (-coreBs BLOCKSIZE)\n");
      ret = -1;
    }
  if (config->oamBlocksize > config->coreBlocksize || ( (config->oamBlocksize % 64) != 0 ) )
    {
      fprintf(stderr, "Invalid command line parameter: unsupported blocksize (-oamBs BLOCKSIZE)\n");
      ret = -1;
    }

  return ret;

}

int initEncoder(const char* oam_filename, const char* hdop_filename, int iframe_period, int sub_sample, int lowDelay){

  uint16_t num_objects = 0;
  uint16_t tmpHasDynamicObjectPriority = 0;
  uint16_t tmpHasUniformSpread = 1;

  /* open OAM file for reading and read OAM header */
  file = oam_read_open(oam_filename, &num_objects, &oam_version, &tmpHasDynamicObjectPriority, &tmpHasUniformSpread);
  if ( tmpHasDynamicObjectPriority ) {
    hasDynamicObjectPriority = DYN_OBJ_PRIO_AVAILABLE;
  }
  if ( !tmpHasUniformSpread ) {
    hasUniformSpread = 0;
  }
  if ( ADD_NONUNIFORMSPREAD ) {
    hasUniformSpread = 0;
  }
  if ( hdop_filename[0] != 0 )
  {
    FILE* file_hasDynamicObjectPriority;
    int  buf_hasDynamicObjectPriority;

    if (hasDynamicObjectPriority == DYN_OBJ_PRIO_NOT_NEEDED)
    {
      buf_hasDynamicObjectPriority = 0;
    }
    else
    {
      buf_hasDynamicObjectPriority = 1;
    }
    
    /* open file */
    file_hasDynamicObjectPriority = fopen(hdop_filename, "wb");
    if (file_hasDynamicObjectPriority == NULL)
    {
      fprintf(stderr, "Fatal error in initEncoder: can't open hasDynamicObjectPriotiy file");
      exit(-1);
    }

    /* write hasDynamicObjectPriority */
    if (fwrite(&buf_hasDynamicObjectPriority, sizeof(int), 1, file_hasDynamicObjectPriority) != 1)
    {
      fprintf(stderr, "Fatal error in initEncoder: can't write hasDynamicObjectPriotiy");
      exit(-1);
    }

    /* close file */
    fclose(file_hasDynamicObjectPriority);
  }

  /* sanity check: number of objects */
  if (num_objects > OAM_MAX_NUM_OBJECTS)
  {
    fprintf(stderr, "Error: Too many objects (num_objects > %d).\n", OAM_MAX_NUM_OBJECTS);
    exit(-1);
  }

  if ( lowDelay ){

    /* input */
    oamInputData =  oam_multidata_create(num_objects, 1 * sub_sample);
    /* int diff */
    oamDiffValues = oam_multidata_create(num_objects, 1 * sub_sample);
    /* int previous */
    oamPreviousData = oam_multidata_create(num_objects, 1 * sub_sample);

  }else{

    /* initialization of polygon data structure */
    pol_dframe = create_pol_chunk(num_objects);
    if (realloc_pol_chunk(pol_dframe, iframe_period + 1))
    {
      fprintf(stderr, "Fatal error in initEncoder: re-allocation of memory for D-Frame failed.\n");
      exit(-1);
    }

    chunk   = oam_multidata_create(num_objects, iframe_period * sub_sample);  /* reserve memory according according to iframe_period/subsampling */
    chunk_i = oam_multidata_create(num_objects, iframe_period);
    chunk_d = oam_multidata_create(num_objects, iframe_period);
    first   = oam_multidata_create(num_objects, 1);
    last    = oam_multidata_create(num_objects, 1);

    /* adjust matrix size according to sub-sampling factor */
    chunk->num_elements /= sub_sample;
    chunk->size2        /= sub_sample;

  }

  return 0;
}

void deleteEncoder( int lowDelay ){
  /* close OAM file */
  oam_read_close(file);
  if ( lowDelay ){
    /* free multidata memory */
    oamInputData    = oam_multidata_destroy(oamInputData);
    oamDiffValues   = oam_multidata_destroy(oamDiffValues);
    oamPreviousData = oam_multidata_destroy(oamPreviousData);
  }else{
    /* free memory for polygon data */
    pol_dframe = destroy_pol_chunk(pol_dframe);

    /* free multidata memory */
    chunk   = oam_multidata_destroy(chunk);
    chunk_i = oam_multidata_destroy(chunk_i);
    chunk_d = oam_multidata_destroy(chunk_d);
    first   = oam_multidata_destroy(first);
    last    = oam_multidata_destroy(last);
  }

}

int getFirstFrame(StructOamMultidata* data, int coreBlocksize, int frameFactor, int sub_sample,int replace_radius, int lowDelay, const char* prefix){

  int num_samples;
  int object_metadata_present;
  int i;
  /* discard OAM information for sample index 0 and get Frame #0 chunk */
  while ( data->sample[0] < (coreBlocksize-1) ) 
    num_samples = get_scaled_chunk(data, file, 1, replace_radius, 1 /*sub_sample*/, oam_version, hasDynamicObjectPriority, hasUniformSpread);

  /* problem with the bit stream */
  if ( num_samples != 1 ) {
    fprintf(stderr, "Fatal error in oam_encode_rapV4(): Could not read OAM samples from file.\n");
    exit(-1);
  }

  /* clear bitstream for new core frame */
  pbs_highrate.num = 0;

  /* clear no. of available bits */
  pbs.num = 0;
  /* write the very first OAM sample to make the first AU decodable */
  if(lowDelay){
    encode_frame_ld(data, oamPreviousData, oamDiffValues, fixedValues, 1, hasDynamicObjectPriority, hasUniformSpread, &pbs);
  }else{
    encode_chunk(&pbs, data, chunk_i, chunk_d, &first, &last, pol_dframe, 1, hasDynamicObjectPriority, hasUniformSpread);
  }

  /* if subsampling get rid of unwanted sampling points */
  if( sub_sample != 1 ){
    num_samples = get_scaled_chunk(data, file, 1, replace_radius, (sub_sample-1), oam_version, hasDynamicObjectPriority, hasUniformSpread);
  }
  /* problem with the bit stream */
  if ( num_samples != 1 ) {
    fprintf(stderr, "Fatal error in getFirstFrame(): Could not read OAM samples from file.\n");
    exit(-1);
  }

  /*write first frame */
  if( frameFactor > 1 ){
    /* object metadata present flag*/
    if(pbs.num > 0){
      object_metadata_present = 1;
    }else{
      object_metadata_present = 0;
    }
    oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
  }
  oam_bitbuf_concat(&pbs_highrate, &pbs);

  for( i = 1; i < frameFactor; i++){
    /* object metadata present flag*/
    object_metadata_present = 0;
    oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
  }

  oam_bitbuf_write(prefix, ++frame_idx, &pbs_highrate);

  return 0;
}

int handleDelayHighEff(int dummy_frames, int iframe_period, int coreBlocksize, int frameFactor, int sub_sample,int replace_radius, const char* prefix){
  /* special treatment for Frame #0 */
  int i;
  int object_metadata_present;
  unsigned int first_real_iframe =  1;
  unsigned int first_iframe_period = 1;

  if ( dummy_frames > 1 ) {
    unsigned int ll = 0;
    unsigned int dummy_iframe_period = dummy_frames - 1;

    unsigned int dummy_iframe_periodHR = (dummy_frames * frameFactor)-1;
    
    /* Keep value for several frames in case of a dummy_iframe_period */
    repeat_chunk(chunk, dummy_iframe_periodHR, iframe_period, last);

    /* clear bitstream for new core frame */
    pbs_highrate.num = 0;

    /* clear bitstream bit counter */
    pbs.num = 0;
    encode_chunk(&pbs, chunk, chunk_i, chunk_d, &first, &last, pol_dframe, dummy_iframe_periodHR, hasDynamicObjectPriority, hasUniformSpread);

    for( i = 0; i < frameFactor; i++){
      if( frameFactor > 1 ){
        /* object metadata present flag*/
        if(pbs.num > 0){
          object_metadata_present = 1;
        }else{
          object_metadata_present = 0;
        }
        oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
      }
      oam_bitbuf_concat(&pbs_highrate, &pbs);
      pbs.num = 0;
    }

    oam_bitbuf_write(prefix, ++frame_idx, &pbs_highrate);

    for (ll =0; ll< dummy_iframe_period-1; ll++) {
      /* clear bitstream for new core frame */
      pbs_highrate.num = 0;
      for( i = 0; i < frameFactor; i++){
        object_metadata_present = 0;
        if( frameFactor > 1 ){
          oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
        }
      }
      /* write empty frames */
      oam_bitbuf_write(prefix, ++frame_idx, &pbs_highrate);
    }
  }

  if ( dummy_frames >= 1 ) {
   /* clear bitstream for new core frame */
    pbs_highrate.num = 0;

    /* clear bitstream bit counter */
    pbs.num = 0;
    /* write the first independent OAM sample */
    encode_chunk(&pbs, chunk, chunk_i, chunk_d, &first, &last, pol_dframe, first_real_iframe, hasDynamicObjectPriority, hasUniformSpread);

    if( frameFactor > 1 ){
      /* object metadata present flag*/
      if(pbs.num > 0){
        object_metadata_present = 1;
      }else{
        object_metadata_present = 0;
      }
      oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
    }

    oam_bitbuf_concat(&pbs_highrate, &pbs);

    for( i = 1; i < frameFactor; i++){
      /* object metadata present flag*/
      object_metadata_present = 0;
      oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
    }

    oam_bitbuf_write(prefix, ++frame_idx, &pbs_highrate);
  }

  return 0;
}

int handleDelayLd(int dummy_frames, int iframe_period, int coreBlocksize, int frameFactor, int sub_sample,int replace_radius, const char* prefix){
  
  int n, i;
  int iFrame;
  int object_metadata_present;

  /* clear bitstream for new core frame */
  pbs_highrate.num = 0;
  /* clear no. of available bits */
  pbs.num = 0;

  if( frameFactor == 1 ){
    for ( n = 0; n < dummy_frames; n++){
      if(n == (dummy_frames-1)){ /* write the first independent OAM sample */
        iFrame = 1;
      }else{
        iFrame = 0;
      }
      /* encode ld frame */
      encode_frame_ld(oamInputData, oamPreviousData, oamDiffValues, fixedValues, iFrame, hasDynamicObjectPriority, hasUniformSpread, &pbs);
      /*write frame*/
      oam_bitbuf_write(prefix, ++frame_idx, &pbs);
      /* clear bitstream bit counter */
      pbs.num = 0;
    }
  }
  else if( frameFactor > 1 ){
    /* Keep value for several frames in case of a dummy_frames */
    for ( n = 0; n < dummy_frames; n++){
      for( i = 0; i < frameFactor; i++){
        if( i == (frameFactor-1) ){ /* write an OAM sample every last position in the frame*/
          /* encode ld frame */
          encode_frame_ld(oamInputData, oamPreviousData, oamDiffValues, fixedValues, 1, hasDynamicObjectPriority, hasUniformSpread, &pbs);
          object_metadata_present = 1;
        }else{
          object_metadata_present = 0;
        }
        oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
        /* concatenate bitstreams */
        oam_bitbuf_concat(&pbs_highrate, &pbs);
        /* clear bitstream bit counter */
        pbs.num = 0;
      }
      /*write frame*/
      oam_bitbuf_write(prefix, ++frame_idx, &pbs_highrate);
      /* clear bitstream for new core frame */
      pbs_highrate.num = 0;
    }
  }
  else {
    fprintf(stderr, "Fatal error in handleDelayLd(): Invalid OAM / Core framelength ratio.\n");
    exit(-1);
  }

  return 0;
}


int main(int argc, char* argv[])
{
  ConfigObjectMetadataEncoder config;
  int object_metadata_present;
  int frameFactor;
  int num_samples;
  int sub_sample;
  unsigned int first_real_iframe =  1;
  int last_frame = 0;
  int is_iframe = 0;
  int error = 0;
  int i = 0;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                               OAM Encoder Module                            *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
  
  /* parse command line parameters */
  if (oam_encoder_get_config(argc, argv, &config) < 0){
    printCmdlHelp();
    return -1;
  }

  /* initialization of oam_multidata structures */
  /* sanity checks */
  if( config.oamBlocksize < 1 || config.coreBlocksize < 1 ){
    fprintf(stderr, "Fatal error: invalid blocksize \n");
    exit(-1);
  }
  if( config.coreBlocksize % config.oamBlocksize ){
    fprintf(stderr, "Fatal error: unsupported subsampling \n");
    exit(-1);
  }
  /* calculate factor for high rate / subsampling*/
  if( config.highRate == 1 ){
    sub_sample = 1;
    frameFactor = (config.coreBlocksize / config.oamBlocksize);
  }else{
    frameFactor = 1;
    sub_sample = (config.coreBlocksize / config.oamBlocksize);
  }


  /*adjust iframe period*/
  if(config.iframe_period < 0){ 
    /* no iframe period given in cmdl -> use 24*oam frames */
    config.iframe_period = 24;
  }else{ 
    /* iframe period given in core frames per cmdl -> calculate iFrameperiod for oam frames*/
    config.iframe_period = config.iframe_period * frameFactor;
  }

  if( !((config.dummy_frames + 1) < config.iframe_period) ){
    fprintf(stderr, "Unsupported dummy_frames \n");
    exit(-1);
  }
  if( config.iframe_period > OAM_MAX_IFRAME_PERIOD ){
    fprintf(stderr, "iframe_period too large -> (coreBs/oamBs)*ip = max 64 \n");
    exit(-1);
  }
  /* init encoder handles and memory*/
  error = initEncoder(config.filenameInput, config.filenameOutput_hasDynamicObjectPriority, config.iframe_period, sub_sample, config.lowDelay);

  if( config.lowDelay ){
    /* first frame and dummy frames*/
    error = getFirstFrame( oamInputData, config.coreBlocksize, frameFactor, sub_sample, config.replace_radius, config.lowDelay, config.filenameOutput);
    error = handleDelayLd(config.dummy_frames, config.iframe_period, config.coreBlocksize, frameFactor, sub_sample, config.replace_radius, config.filenameOutput);
  }else{
    /* first frame and dummy frames*/
    error = getFirstFrame( chunk, config.coreBlocksize, frameFactor, sub_sample, config.replace_radius, config.lowDelay, config.filenameOutput);
    error = handleDelayHighEff(config.dummy_frames, config.iframe_period, config.coreBlocksize, frameFactor, sub_sample, config.replace_radius, config.filenameOutput);
  }

  if(error){
    fprintf(stderr, "Error while handling first frame and delay! \n");
    exit(-1);
  }

  /* main loop */
  while( !last_frame )
  {
    /* clear bitstream for new core frame */
    pbs_highrate.num = 0;
    /* clear bitstream bit counter */
    pbs.num = 0;

    if( config.lowDelay ){
      /* if highrate add object_metadata_present flag */
      for( i = 0; i < frameFactor; i++){
        /* Check if iframe */
        if( (frame_idx % config.iframe_period) == 0 ){
          is_iframe = 1;
        }
        else{
          is_iframe = 0;
        }

        /* get next oam samples */
        num_samples = get_scaled_chunk(oamInputData, file, 1 /*alway one frame*/, config.replace_radius, sub_sample, oam_version, hasDynamicObjectPriority, hasUniformSpread);
        if( num_samples < 1 ){
          last_frame = 1;
        }

        /* encode ld frame */
        encode_frame_ld(oamInputData, oamPreviousData, oamDiffValues, fixedValues, is_iframe, hasDynamicObjectPriority, hasUniformSpread, &pbs);

        if( frameFactor > 1 ){
          /* object metadata present flag */
          if(pbs.num > 0){
            object_metadata_present = 1;
          }else{
            object_metadata_present = 0;
          }
          oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
        }
        /* concatenate bitstreams */
        oam_bitbuf_concat(&pbs_highrate, &pbs);

        /* clear bitstream bit counter */
        pbs.num = 0;
      }
    }else{ /* high eff */
      /* handle ordinary frames */
      if ( ((config.iframe_period/frameFactor) == 1) || ((frame_idx % (config.iframe_period/frameFactor)) - ( config.dummy_frames + first_real_iframe) == 0) )
      {
        num_samples = get_scaled_chunk(chunk, file, config.iframe_period, config.replace_radius, sub_sample, oam_version, hasDynamicObjectPriority, hasUniformSpread);

        if ( config.iframe_period > num_samples ) {
         config.iframe_period = num_samples;
         last_frame    = 1;
        }

        encode_chunk(&pbs, chunk, chunk_i, chunk_d, &first, &last, pol_dframe, config.iframe_period, hasDynamicObjectPriority, hasUniformSpread);
      }

      /* if highrate add object_metadata_present flag */
      for( i = 0; i < frameFactor; i++){
        if( frameFactor > 1 ){
          /* object metadata present flag */
          if(pbs.num > 0){
            object_metadata_present = 1;
          }else{
            object_metadata_present = 0;
          }
          oam_bitbuf_add(&pbs_highrate, object_metadata_present, 1);
        }
        /* concatenate bitstreams */
        oam_bitbuf_concat(&pbs_highrate, &pbs);
        /* clear bitstream bit counter */
        pbs.num = 0;
      }
    }

    /* write frame */
    oam_bitbuf_write(config.filenameOutput, frame_idx + 1, &pbs_highrate);

    /* continue with next frame */
    frame_idx = frame_idx + 1;
  }

  /* free memory */
  deleteEncoder(config.lowDelay);

  fprintf(stderr, "Oam encoding finished!\n");
  return 0;
}

