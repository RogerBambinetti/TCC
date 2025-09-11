/***********************************************************************************

This software module was originally developed by 

Orange Labs

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

Orange Labs retains full right to modify 
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


#include "binauralization_FilterDesign.h"



static void helpCmd()
{
	printf("binauralParametrizationCmdl.exe\n");
	printf("Purpose:\nAutomatic filter design for TD binaural renderer\n");
	printf("Load an input binaural bitstream, convert all FIR representations to TD low-level renderer parameters\n");
	printf("and save the results in the output bitstream.\n");
	printf("Args:\n");
	printf("  -in   <in  : path to input binaural bitstream (FIR)>          (required)\n");
	printf("  -out  <out : path to output binaural bitstream (low-level)>   (required)\n");
	printf("\n");
	printf("Usage ex: binauralParametrizationCmdl.exe -in in.bs -out out.bs");
}


/*---------------------------------------------------------------------------*/
static int parseCmdParameter(	int argc,
							 char *argv[],			
							 char *sIn,
							 char *sOut)
{
	unsigned int nArgs = 0;
	int	i;

	/* loop over all parameters */
	for (i=1; i<argc; ++i )
	{
		if (!strcmp(argv[i],"-in"))      /* Required */
		{
			if (++i < argc )
			{
				strcpy(sIn, argv[i]);
				nArgs++;
				continue;
			}
		}

		else if (!strcmp(argv[i],"-out"))      /* Required */
		{
			if (++i < argc )
			{
				strcpy(sOut, argv[i]);
				nArgs++;
				continue;
			}
		}	

		else 
		{
			fprintf(stderr,"Error : Wrong argument!\n\n");
			helpCmd();
			return -1;
		}  
	}

	/* check number of parameters found */
	if(nArgs < 2)
	{
		helpCmd();
		return -1;
	}
	else
		return 0;
};

/*---------------------------------------------------------------------------*/
static int replaceSlash( char *pPathFile)
{
	char	*pTmp = NULL;

	/* replace '\' by '/' */
	pTmp = pPathFile;

	while( *pTmp != '\0' )
	{
		if ( *pTmp == '\\' )
		{
			*pTmp = '/';
		}
		pTmp++;
	};
	return 0;
}

/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{

	int i;
	BinauralRendering *pBinauralRendering;
	char sInputBitstream[256] = ("");
	char sOutputBitstream[256] = ("");

	fprintf(stderr, "\n\n"); 
	fprintf(stdout, "\n");
	fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                  TD Binaural Parametrization Module                         *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                                  %s                                *\n", __DATE__);
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
	fprintf(stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*******************************************************************************\n");
	fprintf(stdout, "\n");

	/* Init BinauralRendering ptr */
	pBinauralRendering = BinauralRendering_initAlloc();
	if (pBinauralRendering == NULL)
	{
		fprintf(stderr,"\n Error in BinauralRendering_init()");
		return -1;
	}

	/* Parse input parameters */
	if (parseCmdParameter(argc, argv, sInputBitstream, sOutputBitstream) != 0)
	{
		fprintf(stderr,"\n");
		return -1;
	}
	replaceSlash(sInputBitstream);
	replaceSlash(sOutputBitstream);

	/* load binaural bitstream */
	if (bitstreamRead(sInputBitstream, pBinauralRendering) != 0)
	{
		fprintf(stderr,"\nError: could not load binaural bitstream ! \n");
		return -1;
	}

	/* convert each FIR binaural representation to TD renderer paramameters */
	for (i=0; i<pBinauralRendering->numBinauralRepresentation; i++)
	{
		if (pBinauralRendering->ppBinauralRepresentation[i]->binauralDataFormatID == 0)
		{
			/* compute TD params */
			if (computeFilterParams(
				pBinauralRendering->ppBinauralRepresentation[i]->pTdBinauralRendererParam, 
				pBinauralRendering->ppBinauralRepresentation[i]->pBinauralFirData,
				pBinauralRendering->ppBinauralRepresentation[i]->nBrirPairs) != 0)
			{
				fprintf(stderr,"\nError: computing BRIR params failed ! \n");
				return -1;
			}
			else
			{
				printFilterParams(
					pBinauralRendering->ppBinauralRepresentation[i]->pTdBinauralRendererParam);
				/* change binauralDataFormatID to TD renderer param */
				pBinauralRendering->ppBinauralRepresentation[i]->binauralDataFormatID = 2;
			}
		}
	}

	/* export BinauralRendering as a binaural bitstream */
	if (bitstreamWrite(sOutputBitstream, 
		pBinauralRendering->fileSignature, 
		pBinauralRendering->fileVersion,
		pBinauralRendering) == 0)
	{
		fprintf(stderr,"\nWrite BRIRs low-level params in %s\n", sOutputBitstream);
	}
	else
	{
		fprintf(stderr,"\n\nError: writing output bitsream failed %s\n", sOutputBitstream);
		return -1;
	}

	return 0;

}


