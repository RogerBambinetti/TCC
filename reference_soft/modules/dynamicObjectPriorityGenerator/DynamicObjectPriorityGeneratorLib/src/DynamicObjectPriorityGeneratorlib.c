/***********************************************************************************
 
 This software module was originally developed by 
 
 Sony Corporation and Fraunhofer IIS
 
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
  
 Sony Corporation and Fraunhofer IIS 
 retain full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "DynamicObjectPriorityGeneratorlib.h"
#include "wavIO.h"
#include "oam_common.h"
#include "oam_read.h"

#define SONY_MOD_20151006 /* support OAM file version 4 */

#define MAX_NUM_OBJECTS 32
#define RMS_THRESHOLD (1.0000000000000000e-012f) /* -120dB */
                       

struct _DYNAMIC_OBJECT_PRIORITY_GENERATOR
{
  WAVIO_HANDLE hWavIO_In[MAX_NUM_OBJECTS];
  FILE* fIn_oam;
  FILE* fOut_oam;
 
  float** inBuffer1;

  unsigned int nChannels;
  unsigned int blockLength;
  unsigned int nFilesIn;
  unsigned int oamVersionToWrite;
  unsigned int oamVersionToRead;
  unsigned int readDynamicObjectPriority;
  unsigned int readWriteUniformSpread;
  StructOamMultidata* chunk;

};

static void fatal_error(const char* msg, const char* parg)
{
	fprintf(stderr, "%s\n", msg);
	perror(parg);
	exit(-1);
}


static void syntax_error(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}

void oam_read_write_open(const char* filename_in, 
						  const char* filename_out,
						  FILE** fileIn,
						  FILE** fileOut,
						  uint16_t* numObjects,
						  unsigned int* oamVersionToRead,
#ifdef SONY_MOD_20151006
						  unsigned int* oamVersionToWrite,
#endif
						  unsigned int* readDynamicObjectPriority,
						  unsigned int* readWriteUniformSpread)
{
	char     id[4];
	uint16_t num_channels;
	uint16_t num_objects;
	char     description[32];
	char     obj_desc[64];
	char     chan_filename[64];
	int obj, chan;
	FILE* file_in = NULL;
	FILE* file_out = NULL;
#ifndef SONY_MOD_20151006
	uint16_t oamVersionToWrite = 3;
#endif
	uint16_t writeDynamicObjectPriority = 1;

	/* open input file */
	file_in = fopen(filename_in, "rb");
	if (file_in == NULL)
		fatal_error("Error in oam_read_write_open().", filename_in);
	
	/* open output file */
	file_out = fopen(filename_out, "wb");
	if (file_out == NULL)
		fatal_error("Error in oam_read_write_open().", filename_out);

	/* OAM Header: id */
	if (fread(&id, sizeof(char), 4, file_in) != 4)
		fatal_error("Error in oam_read_write_open().", "fread()");
	if (fwrite(&id, sizeof(char), 4, file_out) != 4)
		fatal_error("Error in oam_read_write_open().", "fwrite()");

	/* OAM Header: version */
	if (fread(oamVersionToRead, sizeof(uint16_t), 1, file_in) != 1)
		fatal_error("Error in oam_read_write_open().", "fread()");
	if (*oamVersionToRead > 4)
		syntax_error("Error in oam_read_open(): invalid OAM header.");
	if ( *oamVersionToRead > 2 ) {
		if (fread(readDynamicObjectPriority, sizeof(uint16_t), 1, file_in) != 1)
			fatal_error("Error in oam_read_write_open().", "fread()");
	}
	if (*oamVersionToRead > 3) {
		if (fread(readWriteUniformSpread, sizeof(uint16_t), 1, file_in) != 1) {
			syntax_error("Error in oam_read_open(): invalid OAM header.");
		}
	}
	else {
		*readWriteUniformSpread = 1;
	}

#ifdef SONY_MOD_20151006
	if (*oamVersionToRead > 3) {
		*oamVersionToWrite = *oamVersionToRead;
	} else {
		*oamVersionToWrite = 3;
	}
	if (fwrite(oamVersionToWrite, sizeof(uint16_t), 1, file_out) != 1)
		fatal_error("Error in oam_read_write_open().", "fwrite()");
	if ( *oamVersionToWrite > 2 ) {
		if (fwrite(&writeDynamicObjectPriority, sizeof(uint16_t), 1, file_out) != 1)
			fatal_error("Error in oam_read_write_open().", "fwrite()");
	}
	if ( *oamVersionToWrite > 3 ) {
		if (fwrite(readWriteUniformSpread, sizeof(uint16_t), 1, file_out) != 1)
			fatal_error("Error in oam_read_write_open().", "fwrite()");
	}
#else

	if (fwrite(&oamVersionToWrite, sizeof(uint16_t), 1, file_out) != 1)
		fatal_error("Error in oam_read_write_open().", "fwrite()");
	if ( oamVersionToWrite > 2 ) {
		if (fwrite(&writeDynamicObjectPriority, sizeof(uint16_t), 1, file_out) != 1)
			fatal_error("Error in oam_read_write_open().", "fwrite()");
	}
#endif

	/* OAM Header: num_channels */
	if (fread(&num_channels, sizeof(uint16_t), 1, file_in) != 1)
		fatal_error("Error in oam_read_write_open().", "fread()");
	if (fwrite(&num_channels, sizeof(uint16_t), 1, file_out) != 1)
		fatal_error("Error in oam_read_write_open().", "fwrite()");

	/* OAM Header: num_objects */
	if (fread(&num_objects, sizeof(uint16_t), 1, file_in) != 1)
		fatal_error("Error in oam_read_write_open().", "fread()");
	if (fwrite(&num_objects, sizeof(uint16_t), 1, file_out) != 1)
		fatal_error("Error in oam_read_write_open().", "fwrite()");

	/* OAM Header: description */
	if (fread(description, sizeof(char), 32, file_in) != 32)
		fatal_error("Error in oam_read_write_open().", "fread()");
	if (fwrite(description, sizeof(char), 32, file_out) != 32)
		fatal_error("Error in oam_read_write_open().", "fwrite()");

	/* OAM Header: channel filenames */
	for (chan = 0; chan < num_channels; chan++)
	{
		if (fread(chan_filename, sizeof(char), 64, file_in) != 64)
			fatal_error("Error in oam_read_write_open().", "fread()");
		if (fwrite(chan_filename, sizeof(char), 64, file_out) != 64)
		    fatal_error("Error in oam_read_write_open().", "fwrite()");
	}

	/* OAM Header: object describtions */
	for (obj = 0; obj < num_objects; obj++)
	{
		if (fread(obj_desc, sizeof(char), 64, file_in) != 64)
			fatal_error("Error in oam_read_write_open().", "fread()");
		if (fwrite(obj_desc, sizeof(char), 64, file_out) != 64)
			fatal_error("Error in oam_read_write_open().", "fwrite()");
	}

	*fileIn = file_in;
	*fileOut = file_out;
	*numObjects = num_objects;

	return ;
}

int DynamicObjectPriorityGenerator_init(DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE* hDynamicObjectPriorityGenerator, 
										char* inName_wav, 
										char* inName_oam,
										char* outName_oam,  
										unsigned int blockLength)
 
{
  int error_init = 0;
  unsigned int i = 0;
  unsigned int nInChannels1  = 0;
  unsigned int InSampleRate1 = 0;
  unsigned int InBytedepth1 = 0;
  unsigned long nTotalSamplesPerChannel1 = 0;
  int nSamplesPerChannelFilled1 = 0;
  uint16_t nObjects;
  unsigned int oamVersionToRead = 0;
  unsigned int oamVersionToWrite = 3;
  unsigned int readDynamicObjectPriority = 0;
  unsigned int readWriteUniformSpread = 0;
    
  FILE* fIn_wav[MAX_NUM_OBJECTS];  
  FILE* fIn_oam;
  FILE* fOut_oam;

  char* baseName;

  unsigned int nFilesIn=0;

  DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE hDynamicObjectPriorityGeneratorTemp;

  hDynamicObjectPriorityGeneratorTemp = (DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE)calloc(1,sizeof(struct _DYNAMIC_OBJECT_PRIORITY_GENERATOR));

  /* open OAM file for reading/writing and copy OAM header */
#ifdef SONY_MOD_20151006
  oam_read_write_open(inName_oam, outName_oam, &fIn_oam, &fOut_oam, &nObjects, &oamVersionToRead, &oamVersionToWrite, &readDynamicObjectPriority, &readWriteUniformSpread);
#else
  oam_read_write_open(inName_oam, outName_oam, &fIn_oam, &fOut_oam, &nObjects, &oamVersionToRead, &readDynamicObjectPriority, &readWriteUniformSpread);
#endif
  
  /* for oam processing */
  hDynamicObjectPriorityGeneratorTemp->chunk = oam_multidata_create(nObjects, 1);
  
  /* get channel info */
  {

    if(nObjects==0){
    }
    else{
      nFilesIn=nObjects;
    }
    
	baseName=inName_wav;

    hDynamicObjectPriorityGeneratorTemp->nFilesIn=nFilesIn;

  }

  {
    char tmpString[FILENAME_MAX];

    for(i=0;i<nFilesIn;i++){

      if(nObjects==0) {
      }
      else{
        sprintf(tmpString,"%s_%03i",baseName,i);
      }

      sprintf(tmpString,"%s.wav",tmpString);

	  fIn_wav[i] = fopen(tmpString, "rb");
      if (!fIn_wav[i])
        {
          fprintf(stderr,"Error opening input %s\n", tmpString);
          return -1;
        }
  
    }

  }

  for ( i = 0; i < nFilesIn; ++i )
    {
      error_init = wavIO_init(&(hDynamicObjectPriorityGeneratorTemp->hWavIO_In[i]), blockLength, 0, 0);
      error_init = wavIO_openRead(hDynamicObjectPriorityGeneratorTemp->hWavIO_In[i],
                                  fIn_wav[i],
                                  &nInChannels1,
                                  &InSampleRate1, 
                                  &InBytedepth1,
                                  &nTotalSamplesPerChannel1, 
                                  &nSamplesPerChannelFilled1);
    }

  if ( 0 != error_init ) {
    return -1;
  }

  /* alloc local buffers */
  hDynamicObjectPriorityGeneratorTemp->inBuffer1 = (float**)calloc(nFilesIn, sizeof(float*));

  for (i = 0; i < nFilesIn; i++) {
      hDynamicObjectPriorityGeneratorTemp->inBuffer1[i]  = (float*)calloc(blockLength, sizeof(float));
  }

  hDynamicObjectPriorityGeneratorTemp->oamVersionToWrite = oamVersionToWrite;
  hDynamicObjectPriorityGeneratorTemp->oamVersionToRead  = oamVersionToRead;
  hDynamicObjectPriorityGeneratorTemp->readDynamicObjectPriority = readDynamicObjectPriority;
  hDynamicObjectPriorityGeneratorTemp->readWriteUniformSpread = readWriteUniformSpread;
  hDynamicObjectPriorityGeneratorTemp->blockLength = blockLength;
  hDynamicObjectPriorityGeneratorTemp->nChannels   = nFilesIn;
  hDynamicObjectPriorityGeneratorTemp->fIn_oam = fIn_oam;
  hDynamicObjectPriorityGeneratorTemp->fOut_oam = fOut_oam;

  *hDynamicObjectPriorityGenerator = hDynamicObjectPriorityGeneratorTemp;

  return 0;
}


int DynamicObjectPriorityGenerator_proc(DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE hDynamicObjectPriorityGenerator, unsigned int* lastFrame)
{
  unsigned int i = 0;
  unsigned int k = 0;
  unsigned int isLastFrame = 0;
  unsigned int nZerosPaddedBeginning = 0;
  unsigned int nZerosPaddedEnd = 0;
  unsigned int samplesReadPerChannel = 0;
  unsigned int hasDynamicObjectPriority = 0;
  unsigned int writeDynamicObjectPriority = 1;
  int num;

  for (k = 0; k < hDynamicObjectPriorityGenerator->nFilesIn; k++) {
    wavIO_readFrame(hDynamicObjectPriorityGenerator->hWavIO_In[k],&hDynamicObjectPriorityGenerator->inBuffer1[k],&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);
  }

  /* read OAM data chunk */
  oam_read_process(hDynamicObjectPriorityGenerator->fIn_oam, hDynamicObjectPriorityGenerator->chunk, &num, hDynamicObjectPriorityGenerator->oamVersionToRead, hDynamicObjectPriorityGenerator->readDynamicObjectPriority, hDynamicObjectPriorityGenerator->readWriteUniformSpread);

  /* Generation of object priority */
  for (k = 0; k < hDynamicObjectPriorityGenerator->nFilesIn; k++) {
    float rms = 0.0f;
	for (i = 0; i < samplesReadPerChannel; i++) {
	  rms += (hDynamicObjectPriorityGenerator->inBuffer1[k][i] * hDynamicObjectPriorityGenerator->inBuffer1[k][i]);
	}
	if (samplesReadPerChannel) {
	  rms /= samplesReadPerChannel;
	} else {
	  rms = 0.0f;
	}
	if (rms <= RMS_THRESHOLD) {
	  hDynamicObjectPriorityGenerator->chunk->dynamic_object_priority[k] = 0; /* minimum priority */
	} else {
	  hDynamicObjectPriorityGenerator->chunk->dynamic_object_priority[k] = 7; /* maximum priority */
	}
  }

  /* add data chunk to OAM file */
  oam_write_process(hDynamicObjectPriorityGenerator->fOut_oam, hDynamicObjectPriorityGenerator->chunk, hDynamicObjectPriorityGenerator->oamVersionToWrite, writeDynamicObjectPriority, hDynamicObjectPriorityGenerator->readWriteUniformSpread);

  *lastFrame = isLastFrame;

  return 0;

}


int DynamicObjectPriorityGenerator_close(DYNAMIC_OBJECT_PRIORITY_GENERATOR_HANDLE hDynamicObjectPriorityGenerator)
{
  unsigned int i = 0;
  if ( hDynamicObjectPriorityGenerator )
    {
      for ( i = 0; i < hDynamicObjectPriorityGenerator->nFilesIn; ++i )
        {
          if (hDynamicObjectPriorityGenerator->hWavIO_In[i])
            wavIO_close(hDynamicObjectPriorityGenerator->hWavIO_In[i]);
        }
  
      if (hDynamicObjectPriorityGenerator->inBuffer1)
        {
          for (i=0; i< hDynamicObjectPriorityGenerator->nChannels; i++) {
            free(hDynamicObjectPriorityGenerator->inBuffer1[i]);
          }

          free(hDynamicObjectPriorityGenerator->inBuffer1);
        }
    }

  /* close input OAM file */
  oam_read_close(hDynamicObjectPriorityGenerator->fIn_oam);

  /* close output OAM file */
  oam_write_close(hDynamicObjectPriorityGenerator->fOut_oam);

  /* free multidata memory */
  hDynamicObjectPriorityGenerator->chunk = oam_multidata_destroy(hDynamicObjectPriorityGenerator->chunk);
  
  free(hDynamicObjectPriorityGenerator);

  return 0;
}
