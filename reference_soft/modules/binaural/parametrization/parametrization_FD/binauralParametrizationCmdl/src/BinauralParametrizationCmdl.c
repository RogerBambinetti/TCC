/***********************************************************************************

This software module was originally developed by 

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute

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

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "BinauralFdParameterization.h"


#ifndef QMFLIB_MAX_CHANNELS
#define QMFLIB_MAX_CHANNELS 32
#endif

#define BINAURAL_NUM_OUTCHANNELS 2
#define QMFLIB_NUMBANDS 64

#define DEFAULT_FRAMESIZE 4096
/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	/* ********************** DECLARATIONS ******************** */
	/* files */
	FILE *fBRIRmetadata = NULL; 
	FILE *fbrirs_filelist = NULL;

	FILE *fbitstream_fir = NULL;
	FILE *fbitstream_fd = NULL; 

	/* BRIR variables */
	char *BRIR_base_inpath = NULL;
	char BRIR_filelist[1000] = {'\0'};

	char *Bitstream_inpath = NULL;
	char *Bitstream_outpath = NULL;
	char Bitstream_FIR[1000];		
	char Bitstream_FD[1000];	    

	int k;
	int handle_error = 0;
	int framesize = DEFAULT_FRAMESIZE;
	BinauralRendering *pBinauralRendering;


#ifdef  BUGFIX_KCONV_KMAX_15JUNE
	int kmax = DEFAULT_KMAX;
	int kconv = DEFAULT_KCONV;
#endif

	fprintf(stderr, "\n\n"); 
	fprintf(stdout, "\n");
	fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                     Binaural Parametrization Module                         *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                                  %s                                *\n", __DATE__);
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
	fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*******************************************************************************\n");
	fprintf(stdout, "\n");


	/* *********************************** COMMANDLINE PARSING ************************************* */
	for (k = 1; k < argc; ++k)
	{
		/* input path */
		if (!strcmp(argv[k], "-firbs")) /* Required */
		{
			Bitstream_inpath = argv[k+1];
			strcpy(Bitstream_FIR, Bitstream_inpath); 

			k++; 
		}
		/* output path */ 
		else if(!strcmp(argv[k], "-fdbs"))
		{
			Bitstream_outpath = argv[k+1];
			strcpy(Bitstream_FD, Bitstream_outpath); 

			k++;
		}
#ifdef BUGFIX_KCONV_KMAX_15JUNE
		/* Kconv */ 
		else if(!strcmp(argv[k], "-kconv"))
		{
			kconv = atoi(argv[k+1]);

			k++;
		}		
		/* Kmax */ 
		else if(!strcmp(argv[k], "-kmax"))
		{
			kmax = atoi(argv[k+1]);

			k++;
		}
		/* framesize */
		else if(!strcmp(argv[k], "-block"))
		{
			framesize = atoi(argv[k+1]);

			k++;
		}
#endif
	}
	
	/* Init BinauralRendering ptr */
	pBinauralRendering = BinauralRendering_initAlloc();
	if (pBinauralRendering == NULL)
	{
		printf("\n Error in BinauralRendering_init()");
		return -1;
	}

	/* load binaural bitstream */
	if (bitstreamRead(Bitstream_FIR, pBinauralRendering) != 0)
	{
		printf("\nError: could not load binaural bitstream ! \n");
		return -1;
	}

	/* convert each FIR binaural representation to FD renderer paramameters */
	for (k=0; k<pBinauralRendering->numBinauralRepresentation; k++)
	{
		if (pBinauralRendering->ppBinauralRepresentation[k]->binauralDataFormatID == 0)
		{
#ifdef  BUGFIX_KCONV_KMAX_15JUNE
            pBinauralRendering->ppBinauralRepresentation[k]->pFdBinauralRendererParam = (FdBinauralRendererParam*)calloc(1, sizeof (FdBinauralRendererParam));
            pBinauralRendering->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kConv = kconv;
            pBinauralRendering->ppBinauralRepresentation[k]->pFdBinauralRendererParam->kMax  = kmax;
#endif
			if( binauralParam_createFdBinauralParam(pBinauralRendering->ppBinauralRepresentation[k], framesize) )
			{
				printf("\nError: computing BRIR params failed ! \n");
				return -1;
			}
			pBinauralRendering->ppBinauralRepresentation[k]->binauralDataFormatID = 1;

			/* export BinauralRendering as a binaural bitstream */
			if (bitstreamWrite(Bitstream_outpath, 
				pBinauralRendering->fileSignature, 
				pBinauralRendering->fileVersion,
				pBinauralRendering) == 0)
			{
				printf("\nWrite BRIRs low-level params in %s\n", Bitstream_outpath);
			}
			else
			{
				printf("\n\nError: writing output bitsream failed %s\n", Bitstream_outpath);
				return -1;
			}
		}
	}

	/* ----------------- EXIT -------------------- */

	return handle_error; 
}


