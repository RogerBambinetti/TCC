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
#include "oam_bitbuf_write.h"
#include "writeonlybitbuf.h"


static void fatal_error(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}


static void fatal_perror(const char* msg, const char* parg)
{
	fprintf(stderr, "%s\n", msg);
	perror(parg);
	exit(-1);
}


void oam_bitbuf_add(OamPBS* pbs, int value, int wordsize)
{
	if (pbs == NULL)
		fatal_error("Error in oam_bitbuf_add(): invalid pointer");
	if (pbs->num >= OAM_MAX_PBS_LEN)
		fatal_error("Error in oam_bitbuf_add(): PBS buffer overflow");

	pbs->data[pbs->num] = value;
	pbs->size[pbs->num] = wordsize;
	pbs->num += 1;
}

void oam_bitbuf_concat(OamPBS* pbs, OamPBS* pbs2)
{
  int i = 0;
	if (pbs == NULL || pbs2 == NULL)
		fatal_error("Error in oam_bitbuf_concat(): invalid pointer");
	if (pbs->num >= OAM_MAX_PBS_LEN|| pbs2->num >= OAM_MAX_PBS_LEN )
		fatal_error("Error in oam_bitbuf_concat(): PBS buffer overflow");
  for( i = 0; i < pbs2->num; i++){
		pbs->data[pbs->num] = pbs2->data[i];
		pbs->size[pbs->num] = pbs2->size[i];
		pbs->num += 1;
  }
}


void oam_bitbuf_write(const char* prefix, int frame_idx, const OamPBS* pbs)
{
	char            filename[OAMBITBUFWRITE_MAX_FILENAME_LENGTH];
	unsigned char   bitstreamDataWrite[OAM_MAX_PBS_LEN] = {0};
	wobitbuf        writebitbuffer;
	FILE* file;
    int n, size;

	/* buffer overflow protection */
	sprintf(filename, "_%i.dat", frame_idx);
	if (strlen(prefix) + strlen(filename) >= OAMBITBUFWRITE_MAX_FILENAME_LENGTH)
		fatal_error("Error in oam_bitbuf_write(): filename too long");
	sprintf(filename, "%s_%i.dat", prefix, frame_idx);

	/* initialize bit buffer */
	wobitbuf_Init(&writebitbuffer, bitstreamDataWrite, (sizeof(bitstreamDataWrite)<<3), 0);

	/* generate bitstream */
	for (n = 0; n < pbs->num; n++) {
		if (wobitbuf_WriteBits(&writebitbuffer, pbs->data[n], pbs->size[n]) != 0 )
			fatal_error("Error in oam_bitbuf_write(): bit buffer size exceeded");
	}

	/* open file for writing */
	file = fopen(filename, "wb");
	if (file == NULL)
		fatal_perror("Error in oam_bitbuf_write():", filename);

	/* write data to file */
	size = (wobitbuf_GetBitsWritten(&writebitbuffer) + 7) / 8;
	if (fwrite(bitstreamDataWrite, sizeof(char), size, file) != size)
		fatal_perror("Error in oam_bitbuf_write(): can't write bitstream", filename);

	/* close file */
	fclose(file);
}
