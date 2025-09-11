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
#include <string.h>
#include "oam_bitbuf_read.h"


int oam_bitbuf_read_open(OamBitbufRead* bitstream, const char* prefix, int frame_idx)
{
	char filename[OAMBITBUFREAD_MAX_FILENAME_LENGTH];
	int fileSize;

	/* buffer overflow protection */
	sprintf(filename, "_%i.dat", frame_idx);
	if (strlen(prefix) + strlen(filename) >= OAMBITBUFREAD_MAX_FILENAME_LENGTH)
	{
		fprintf(stderr, "Error: filename too long");
		return -1;
	}

	/* open file for reading */
	sprintf(filename, "%s_%i.dat", prefix, frame_idx);
	bitstream->fptr = fopen(filename, "rb");
	if (bitstream->fptr == NULL) {
		perror(filename);
		return -1;
	}

	/* get file size */
	fseek(bitstream->fptr, 0L, SEEK_END);
	fileSize = ftell(bitstream->fptr);
	fseek(bitstream->fptr, 0L, SEEK_SET);
	bitstream->num_bits = 8 * fileSize;

	/* init buffer with data from file */
	bitstream->data_read = (unsigned char*)calloc(fileSize, sizeof(unsigned char));
	if( bitstream->data_read == NULL ){
		exit(-1);
	}

	fread(bitstream->data_read, fileSize, sizeof(char), bitstream->fptr);

	/* close file */
	fclose(bitstream->fptr);

	/* set read position to the beginning of the file */
	bitstream->offset = 0;

	return 0;
}


int oam_bitbuf_read_process(OamBitbufRead* bitstream, int *val, int wordsize)
{
	int ret = 0;
	robitbuf readbitbuffer;

	if (bitstream->offset + wordsize > bitstream->num_bits)
		return -1;
		
	robitbuf_Init(&readbitbuffer, bitstream->data_read, bitstream->num_bits, bitstream->offset);
	*val = robitbuf_ReadBits(&readbitbuffer, wordsize);
	bitstream->offset += wordsize;

	return ret;
}


void oam_bitbuf_read_close(OamBitbufRead* bitstream)
{
	free(bitstream->data_read);
	bitstream->fptr      = NULL;
	bitstream->num_bits  = 0;
	bitstream->offset    = 0;
	bitstream->data_read = 0;
}


int oam_bitstream_read(OamBitbufRead* bitstream, int *val, int wordsize, char sig)
{
	int ret = oam_bitbuf_read_process(bitstream, val, wordsize);

	/* reconstruct the sign */
	if (sig == 's' && *val >= (1<<(wordsize-1)))
		*val = *val - (1<<wordsize);

	return ret;
}
