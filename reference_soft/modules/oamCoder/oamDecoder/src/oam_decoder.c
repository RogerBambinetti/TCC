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
#include <string.h>

#include "oam_decode_rapV4.h"
#include "oam_decode_ldV1.h"
#include "oam_common.h"
#include "oam_bitbuf_read.h"


#define OAM_DEC_BUFSIZE 1024


typedef struct
{
  char filenameInput[OAM_MAX_NUM_OBJECTS][OAM_DEC_BUFSIZE+2];
  char filenameOutput[OAM_DEC_BUFSIZE];
  int num_objects;
  int oamBlocksize;
  int coreBlocksize;
  int lowdelay;
  int oam_version;
  int hasDynamicObjectPriority;
  int hasUniformSpread;
  int readIndependentConfigs;
} ConfigObjectMetadataDecoder;

static void printCmdlHelp(){

  fprintf( stdout, "\n"); 
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "OAM Decoder Module - Commandline Help:\n");
  fprintf( stdout, "                                                                     \n");
  fprintf( stdout, "Required:                                                            \n");
  fprintf( stdout, "-if filename      : Input file (.dat files)                \n");
  fprintf( stdout, "-of filename.oam  : Output file name (oam file)         \n");
  fprintf( stdout, "-nobj *           : number of objects                   \n");
  fprintf( stdout, "-oamVersion *     : oam version to use                  \n");
  fprintf( stdout, "                    1: az, el, radius, gain \n");
  fprintf( stdout, "                    2: az, el, radius, gain, spread \n");
  fprintf( stdout, "                    3: az, el, radius, gain, spread, dynamicObjectPriority \n");
  fprintf( stdout, "                    4: az, el, radius, gain, spreadWidth, \n");
  fprintf( stdout, "                       spreadHeight, spreadDepth, dynamicObjectPriority \n");
  fprintf( stdout, "-hasDynamicObjectPriority * : hasDynamicObjectPriority flag \n");
  fprintf( stdout, "                              Given by ObjectMetadataConfig().<default: 0> \n");
  fprintf( stdout, "-hasUniformSpread *         : hasUniformSpread flag  \n");
  fprintf( stdout, "                              1: write only uniformSpread data to oam file. \n");
  fprintf( stdout, "                              0: use width, height and depth of spread (only v4) \n");
  fprintf( stdout, "                              Given by ObjectMetadataConfig().<default: 1> \n");
  fprintf( stdout, "                                           \n");
  fprintf( stdout, "Optional:                                  \n");
  fprintf( stdout, "-ld *          : enable low dealy mode. Given by ObjectMetadataConfig() \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "-coreBs *      : core coder granularity in audio samples \n");
  fprintf( stdout, "                 default: 1024\n");
  fprintf( stdout, "-oamBs *       : oam granularity in audio samples \n");
  fprintf( stdout, "                 default: 1024\n");
  fprintf( stdout, "-indepConf     : read one set of .dat files per object (for bitstreams with independent object configs) \n");
  fprintf( stdout, "                 default: 0\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
}

static int oam_decoder_get_config(int argc, char* argv[], ConfigObjectMetadataDecoder* config)
{
  int i = 0;
  int ret = 0;

  /* default parameter */
  for (i = 0; i < OAM_MAX_NUM_OBJECTS; i++)
  {
    config->filenameInput[i][0]  = '\0';
  }
  config->filenameOutput[0] = '\0';
  config->num_objects       = -1;
  config->oamBlocksize         = 1024;
  config->coreBlocksize         = 1024;
  config->lowdelay          = 0;
  config->oam_version       = -1;
  config->hasDynamicObjectPriority = -1;
  config->hasUniformSpread         = -1;
  config->readIndependentConfigs = 0;

  for ( i = 1; i < argc-1; ++i )
  {
    if (!strcmp(argv[i], "-if"))    /* Required */
    {
      strncpy(config->filenameInput[0], argv[i+1], OAM_DEC_BUFSIZE) ;
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-of"))  /* Required */
    {
      strncpy(config->filenameOutput, argv[i+1], OAM_DEC_BUFSIZE) ;
      i++;
      continue;
    }
    else if (!strcmp(argv[i] ,"-nobj"))  /* Required */
    {
      config->num_objects = atoi(argv[i+1]);
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-oamVersion"))  /* Required */
    {
      config->oam_version = atoi(argv[i+1]);
      if ( config->oam_version < 1 || config->oam_version > 4 ) {
        fprintf(stderr, "Error: Unknow OAM version.\n");
        return -1;
      }
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-hasDynamicObjectPriority"))  /* Required */
    {
      config->hasDynamicObjectPriority = atoi(argv[i+1]);
      if ( config->hasDynamicObjectPriority < 0 || config->hasDynamicObjectPriority > 1 ) {
        fprintf(stderr, "Error: Illegal value for hasDynamicObjectPriority.\n");
        return -1;
      }
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-hasUniformSpread"))  /* Required */
    {
      config->hasUniformSpread = atoi(argv[i+1]);
      if ( config->hasUniformSpread < 0 || config->hasUniformSpread > 1 ) {
        fprintf(stderr, "Error: Illegal value for hasUniformSpread.\n");
        return -1;
      }
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-oamBs"))  /* Optional */
    {
      config->oamBlocksize = atoi(argv[i+1]);
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-coreBs"))  /* Optional */
    {
      config->coreBlocksize = atoi(argv[i+1]);
      i++;
      continue;
    }
    else if (!strcmp(argv[i], "-ld"))  /* Optional */
    {
      config->lowdelay = atoi(argv[i+1]);
      if ( config->lowdelay < 0 || config->lowdelay > 1 ) {
        fprintf(stderr, "Error: Illegal value for lowdelay.\n");
        return -1;
      }
      i++;
      continue;
    } 
    else if (!strcmp(argv[i], "-indepConf"))  /* Optional */
    {
      config->readIndependentConfigs = atoi(argv[i+1]);
      i++;
      continue;
    }
  }

  /* check parameter values */
  if (config->filenameInput[0][0] == '\0')
  {
    fprintf(stderr, "Missing or wrong command line parameter: input filename (-if FILE)\n\n");
    ret = -1;
  }
  if (config->filenameOutput[0] == '\0')
  {
    fprintf(stderr, "Missing or wrong command line parameter: output filename (-of FILE)\n\n");
    ret = -1;
  }
  if (config->num_objects < 0)
  {
    fprintf(stderr, "Missing or wrong command line parameter: object number (-nobj NUM)\n\n");
    ret = -1;
  }
  if (config->oam_version < 0)
  {
    fprintf(stderr, "Missing or wrong command line parameter: object number (-oam_version NUM)\n\n");
    ret = -1;
  }
  if (config->hasDynamicObjectPriority < 0)
  {
    fprintf(stderr, "Missing or wrong command line parameter: object number (-hasDynamicObjectPriority NUM)\n\n");
    ret = -1;
  }
  if (config->hasUniformSpread < 0)
  {
    fprintf(stderr, "Missing or wrong command line parameter: object number (-hasUniformSpread NUM)\n\n");
    ret = -1;
  }
  if (config->oamBlocksize > config->coreBlocksize || ( (config->oamBlocksize % 64) != 0 ) )
    {
      fprintf(stderr, "Invalid command line parameter: unsupported blocksize (-oamBs BLOCKSIZE)\n");
      ret = -1;
    }
  if (config->readIndependentConfigs == 1)
  {
    int k;
    for (k = 1; k < config->num_objects; k++) /* set file names for independent configs */
    {
      char filename[OAM_DEC_BUFSIZE];
      sprintf(filename,"%d_",k+1);
      strcat(filename,config->filenameInput[0]);
      strncpy(config->filenameInput[k], filename, OAM_DEC_BUFSIZE) ;
    }
  }
  return ret;
}

void getLastFrameInChunk( StructOamMultidata* chunk ){
  
  int obj;
  for (obj = 0; obj < chunk->size1; obj++){
    int kout = oam_mat2D(chunk->size1, obj+1, chunk->size2);
    chunk->sample[obj]                  = chunk->sample[kout];
    chunk->azimuth[obj]                 = chunk->azimuth[kout];
    chunk->elevation[obj]               = chunk->elevation[kout];
    chunk->radius[obj]                  = chunk->radius[kout];
    chunk->gain[obj]                    = chunk->gain[kout];
    chunk->spread[obj]                  = chunk->spread[kout];
    chunk->spread_height[obj]           = chunk->spread_height[kout];
    chunk->spread_depth[obj]            = chunk->spread_depth[kout];
    chunk->dynamic_object_priority[obj] = chunk->dynamic_object_priority[kout];
  }
  chunk->num_elements = chunk->size1;
  chunk->size2 = 1;
}

int main(int argc, char* argv[])
{
  ConfigObjectMetadataDecoder config;

/* both modes */
  OamBitbufRead bs[OAM_MAX_NUM_OBJECTS];
  StructOamMultidata* chunk[OAM_MAX_NUM_OBJECTS];
  StructOamPolChunk* pol_iframe[OAM_MAX_NUM_OBJECTS] = { NULL };
  StructOamPolChunk* pol_dframe[OAM_MAX_NUM_OBJECTS] = { NULL };
  StructOamMultidata* chunk_d[OAM_MAX_NUM_OBJECTS];
  FILE* file;
  int frameFactor;
  int object_metadata_present;
  int i, n, obj;

/* high eff */
  int last[OAM_MAX_NUM_OBJECTS][OAM_NUMBER_COMPONENTS * OAM_MAX_NUM_OBJECTS];
  int block_idx       = 1;
  uint64_t sample_counter = 0;

/* low delay */
  int x[OAM_MAX_NUM_OBJECTS][OAM_NUMBER_COMPONENTS];
  int fixed_val[OAM_MAX_NUM_OBJECTS][OAM_NUMBER_COMPONENTS];
  int frame_index = 0;

  fprintf( stdout, "\n"); 
  fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                               OAM Decoder Module                            *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*                                  %s                                *\n", __DATE__);
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
  fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
  fprintf( stdout, "*                                                                             *\n");
  fprintf( stdout, "*******************************************************************************\n"); 
  fprintf( stdout, "\n");
    
  /* parse command line parameters */
  if (oam_decoder_get_config(argc, argv, &config) < 0)
  {
     printCmdlHelp();
     return -1;
  }


  /* sanity check: number of objects */
  if (config.num_objects > OAM_MAX_NUM_OBJECTS)
  {
    fprintf(stderr, "Error: Too many objects (num_objects > %d).\n", OAM_MAX_NUM_OBJECTS);
    exit(-1);
  }

  /*********************************************/
  /* INIT decoder data structure */
  /*********************************************/

  /* sanity checks */
  if( config.oamBlocksize < 1 || config.coreBlocksize < 1 ){
    fprintf(stderr, "Error in main(): invalid blocksize \n");
    exit(-1);
  }
  if( config.coreBlocksize % config.oamBlocksize ){
    fprintf(stderr, "Error in main(): invalid blocksize factor \n");
    exit(-1);
  }
  /* calculate factor if high rate oam */
  frameFactor = config.coreBlocksize / config.oamBlocksize;

  if (config.lowdelay)
  {
    if (config.readIndependentConfigs == 1)
    {
      for (i = 0; i < config.num_objects; i++)
      {
        /* set "x" and "fixed_val" to 0 as initial values */
        memset(x[i], 0, OAM_NUMBER_COMPONENTS * sizeof(int));
        memset(fixed_val[i], 0, OAM_NUMBER_COMPONENTS * sizeof(int));
        /* initialization of oam_multidata structures */
        chunk[i] = oam_multidata_create(1, 1);
      }
    }
    else
    {
      /* set "x" and "fixed_val" to 0 as initial values */
      memset(x[0], 0, OAM_NUMBER_COMPONENTS * config.num_objects * sizeof(int));
      memset(fixed_val[0], 0, OAM_NUMBER_COMPONENTS * sizeof(int));

      /* initialization of oam_multidata structures */
      chunk[0] = oam_multidata_create(config.num_objects, 1);
    }
  }
  else
  {
    if (config.readIndependentConfigs == 1)
    {
      for (i = 0; i < config.num_objects; i++)
      {
        /* initialization of polygon data structure */ 
        pol_iframe[i] = create_pol_chunk(1);
        pol_dframe[i] = create_pol_chunk(1);

        /* initialization of oam_multidata structures */
        chunk[i]         = oam_multidata_create(1, 1);
        chunk_d[i]       = oam_multidata_create(1, 1);

        /* set "last" to 0 as initial value */
        memset(last[i], 0, OAM_NUMBER_COMPONENTS * 1 * sizeof(int));
      }
    }
    else
    {
      /* initialization of polygon data structure */ 
      pol_iframe[0] = create_pol_chunk(config.num_objects);
      pol_dframe[0] = create_pol_chunk(config.num_objects);

      /* initialization of oam_multidata structures */
      chunk[0]         = oam_multidata_create(config.num_objects, 1);
      chunk_d[0]       = oam_multidata_create(config.num_objects, 1);

      /* set "last" to 0 as initial value */
      memset(last[0], 0, OAM_NUMBER_COMPONENTS * config.num_objects * sizeof(int));
    }
  }

  /* open OAM file for writing and write OAM header */
  file = oam_write_open(config.filenameOutput, (uint16_t)config.num_objects, config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);


  /*********************************************/
  /* decoder loop over all frames */
  /*********************************************/
  if (config.readIndependentConfigs == 1)
  {
    int nObj;
    int read_open_error = 0;
    while(1)
    {
      /* open file reading for all independent configs */
      for (nObj = 0; nObj < config.num_objects; nObj++)
      {
        if (oam_bitbuf_read_open(&(bs[nObj]), config.filenameInput[nObj], block_idx) != 0)
        {
          read_open_error = 1;
          break; /* there are no more frames available -> leave loop */
        }

        /* skip zero padded input frames introduced by decoder delay */
        if (bs[nObj].num_bits == 0)
        {
          if (config.lowdelay){
            /* fprintf(stderr, "Error in oam_decode_ldV1(): empty OAM container.\n"); */
            /* exit(-1); */
            fprintf(stderr, "Warning in oam_decode_ldV1(): Empty OAM container. Should not happen.\n");
            block_idx += 1;
            continue;
          }else{
            block_idx += 1;
            continue;
          }
        }
      }
      if (read_open_error != 0)
      {
        break; /* there are no more frames available -> leave loop */
      }

      for( i = 0; i < frameFactor; i++){
        if( frameFactor > 1 ){
          for (nObj = 0; nObj < config.num_objects; nObj++)
          {
            if (oam_bitstream_read(&(bs[nObj]), &object_metadata_present, 1, 'u'))			/* object_metadata_present */
              fprintf(stderr, "error in main(): invalid bit stream syntax.\n");
          }
          if(!object_metadata_present){
            if (config.lowdelay){

              /* increase sampling position & repeat last value */
              for (nObj = 0; nObj < config.num_objects; nObj++){
                chunk[nObj]->sample[0] += config.oamBlocksize;
              }
              /* merge chunks and add data chunk to OAM file */
              {
                StructOamMultidata* chunk_merge;
                chunk_merge = oam_multidata_create(config.num_objects, 1);
                for (nObj = 0; nObj < config.num_objects; nObj++)
                {
                  chunk_merge->sample[nObj] = chunk[nObj]->sample[0];
                  chunk_merge->azimuth[nObj] = chunk[nObj]->azimuth[0];
                  chunk_merge->dynamic_object_priority[nObj] = chunk[nObj]->dynamic_object_priority[0];
                  chunk_merge->elevation[nObj] = chunk[nObj]->elevation[0];
                  chunk_merge->gain[nObj] = chunk[nObj]->gain[0];
                  chunk_merge->radius[nObj] = chunk[nObj]->radius[0];
                  chunk_merge->spread[nObj] = chunk[nObj]->spread[0];
                  chunk_merge->spread_depth[nObj] = chunk[nObj]->spread_depth[0];
                  chunk_merge->spread_height[nObj] = chunk[nObj]->spread_height[0];
                }
                oam_write_process(file, chunk_merge, config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
                oam_multidata_destroy(chunk_merge);
              }
            }
            else
            { /*high efficiency */
              if ( (block_idx  * frameFactor - 1*frameFactor /*first frame*/)  > sample_counter )
              { /* if new data expected but not available */
                /* increase sampling position & repeat last value */ 
                for (nObj = 0; nObj < config.num_objects; nObj++)
                {
                  int kout = oam_mat2D(config.num_objects, 1, chunk[nObj]->size2);
                  chunk[nObj]->sample[kout] = config.coreBlocksize + ((sample_counter + 1) * config.oamBlocksize);
                  sample_counter ++;
                  /*repeat last frame */
                  getLastFrameInChunk(chunk[nObj]);
                }
                /* merge chunks and add data chunk to OAM file */
                {
                  StructOamMultidata* chunk_merge;
                  chunk_merge = oam_multidata_create(config.num_objects, 1);
                  for (nObj = 0; nObj < config.num_objects; nObj++)
                  {
                    chunk_merge->sample[nObj] = chunk[nObj]->sample[0];
                    chunk_merge->azimuth[nObj] = chunk[nObj]->azimuth[0];
                    chunk_merge->dynamic_object_priority[nObj] = chunk[nObj]->dynamic_object_priority[0];
                    chunk_merge->elevation[nObj] = chunk[nObj]->elevation[0];
                    chunk_merge->gain[nObj] = chunk[nObj]->gain[0];
                    chunk_merge->radius[nObj] = chunk[nObj]->radius[0];
                    chunk_merge->spread[nObj] = chunk[nObj]->spread[0];
                    chunk_merge->spread_depth[nObj] = chunk[nObj]->spread_depth[0];
                    chunk_merge->spread_height[nObj] = chunk[nObj]->spread_height[0];
                  }
                  oam_write_process(file, chunk_merge, config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
                  oam_multidata_destroy(chunk_merge);
                }
              }

            }
            continue; /* skip decoding if no metadata present */
          }
        }

        for (nObj = 0; nObj < config.num_objects; nObj++)
        {
          if (config.lowdelay)
          {
            /* call decoder to decode frame from bs into chunk*/
            decode_ld_frame(1, config.oamBlocksize, config.hasDynamicObjectPriority, config.hasUniformSpread,
                            fixed_val[nObj], x[nObj], &(bs[nObj]), chunk[nObj]);
          }
          else
          {
            /* call decoder to decode frame from bs into chunk*/
            decode_higheff_frame(1, config.oamBlocksize, config.coreBlocksize, config.hasDynamicObjectPriority, config.hasUniformSpread,
                                 pol_iframe[nObj], pol_dframe[nObj], chunk_d[nObj], last[nObj], &(bs[nObj]), chunk[nObj], &sample_counter);
          }
        }
        /* merge chunks and add data chunk to OAM file */
        {
          int ins = 0;
          StructOamMultidata* chunk_merge;
          chunk_merge = oam_multidata_create(config.num_objects, 1);
          for (ins = 0; ins < config.num_objects; ins++)
          {
            chunk_merge->sample[ins] = chunk[ins]->sample[0];
            chunk_merge->azimuth[ins] = chunk[ins]->azimuth[0];
            chunk_merge->dynamic_object_priority[ins] = chunk[ins]->dynamic_object_priority[0];
            chunk_merge->elevation[ins] = chunk[ins]->elevation[0];
            chunk_merge->gain[ins] = chunk[ins]->gain[0];
            chunk_merge->radius[ins] = chunk[ins]->radius[0];
            chunk_merge->spread[ins] = chunk[ins]->spread[0];
            chunk_merge->spread_depth[ins] = chunk[ins]->spread_depth[0];
            chunk_merge->spread_height[ins] = chunk[ins]->spread_height[0];
          }
          /* add data chunk to OAM file */
          oam_write_process(file, chunk_merge, config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
          oam_multidata_destroy(chunk_merge);
        }
      }
      /* close frame */
      for (nObj = 0; nObj < config.num_objects; nObj++)
      {
        oam_bitbuf_read_close(&(bs[nObj]));
      }
      /* 
       * continue with next block 
       * increasing the block index by the iframe_period will not work
       * if object fragmentation is used and the obj data is smaller than the
       * fragmentation length !!!
       */
      block_idx = block_idx +  1;
    }
  }
  else
  {
  while(1)
  {
    int temp = oam_bitbuf_read_open(&(bs[0]), config.filenameInput[0], block_idx);
    if (temp != 0)
      break;  /* there are no more frames available -> leave loop */

    /* skip zero padded input frames introduced by decoder delay */
    if (bs[0].num_bits == 0)
    {
      if (config.lowdelay){
        /* fprintf(stderr, "Error in oam_decode_ldV1(): empty OAM container.\n"); */
        /* exit(-1); */
        fprintf(stderr, "Warning in oam_decode_ldV1(): Empty OAM container. Should not happen.\n");
        block_idx += 1;
        continue;
      }else{
        block_idx += 1;
        continue;
      }
    }

    for( i = 0; i < frameFactor; i++){

      if( frameFactor > 1 ){
        if (oam_bitstream_read(&(bs[0]), &object_metadata_present, 1, 'u'))			/* object_metadata_present */
          fprintf(stderr, "Error in main(): invalid bit stream syntax.\n");
        if(!object_metadata_present){
          if (config.lowdelay){

            /* increase sampling position & repeat last value */
            for (n = 0; n < config.num_objects; n++){
              chunk[0]->sample[n] += config.oamBlocksize;
            }
            /* add data chunk to OAM file */
            oam_write_process(file, chunk[0], config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
          }else{ /*high efficiency */
            if ( (block_idx  * frameFactor - 1*frameFactor/*first frame*/)  > sample_counter ){ /* if new data expected but not available */
              /* increase sampling position & repeat last value */ 
              for (obj = 1; obj <= config.num_objects; obj++){
                int kout = oam_mat2D(config.num_objects, obj, chunk[0]->size2);
                chunk[0]->sample[kout] = config.coreBlocksize + ((sample_counter + 1) * config.oamBlocksize);
              }
              sample_counter ++;
              /*repeat last frame */
              getLastFrameInChunk(chunk[0]);
              oam_write_process(file, chunk[0], config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
            }
          }
          continue; /* skip decoding if no metadata present */
        }
      }
      if (config.lowdelay)
      {
        /* call decoder to decode frame from bs into chunk*/
        decode_ld_frame(config.num_objects, config.oamBlocksize, config.hasDynamicObjectPriority, config.hasUniformSpread,
                        fixed_val[0], x[0], &(bs[0]), chunk[0]);
      }
      else
      {
        /* call decoder to decode frame from bs into chunk*/
        decode_higheff_frame(config.num_objects, config.oamBlocksize, config.coreBlocksize, config.hasDynamicObjectPriority, config.hasUniformSpread,
                             pol_iframe[0], pol_dframe[0], chunk_d[0], last[0], &(bs[0]), chunk[0], &sample_counter);
      }
      /* add data chunk to OAM file */
      oam_write_process(file, chunk[0], config.oam_version, config.hasDynamicObjectPriority, config.hasUniformSpread);
    }

    /* close frame */
    oam_bitbuf_read_close(&(bs[0]));

    /* 
     * continue with next block 
     * increasing the block index by the iframe_period will not work
     * if object fragmentation is used and the obj data is smaller than the
     * fragmentation length !!!
     */
    block_idx = block_idx +  1;
  }
  }

  /* close OAM file */
  oam_write_close(file);

  /*********************************************/
  /* DELETE decoder data structure */
  /*********************************************/
  if (config.lowdelay)
  {
    /* free multidata memory */
    if (config.readIndependentConfigs == 1)
    {
      for (i = 0; i < config.num_objects; i++)
      {
        chunk[i] = oam_multidata_destroy(chunk[i]);
      }
    }
    else
    {
      chunk[0] = oam_multidata_destroy(chunk[0]);
    }
  }
  else
  {
    if (config.readIndependentConfigs == 1)
    {
      for (i = 0; i < config.num_objects; i++)
      {
        /* free memory for polygon data */
        pol_iframe[i] = destroy_pol_chunk(pol_iframe[i]);
        pol_dframe[i] = destroy_pol_chunk(pol_dframe[i]);

        /* free multidata memory */
        chunk[i]   = oam_multidata_destroy(chunk[i]);
        chunk_d[i] = oam_multidata_destroy(chunk_d[i]);
      }
    }
    else
    {
      pol_iframe[0] = destroy_pol_chunk(pol_iframe[0]);
      pol_dframe[i] = destroy_pol_chunk(pol_dframe[0]);

      /* free multidata memory */
      chunk[0]   = oam_multidata_destroy(chunk[0]);
      chunk_d[0] = oam_multidata_destroy(chunk_d[0]);
    }
  }

  return 0;
}

