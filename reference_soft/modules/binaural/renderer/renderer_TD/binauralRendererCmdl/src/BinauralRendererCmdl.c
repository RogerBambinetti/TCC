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

Orange Labs retain full right to modify and use the code for its own purpose, 
assign or donate the code to a third party and to inhibit third parties 
from using the code for products that do not conform to MPEG-related 
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#define USE_WAVIO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "binauralization.h"
#include "bitstreamBinaural.h"
#include "wavIO.h"

/*---------------------------------------------------------------------------*/

#define BINAURAL_TEST_LGR_SIGNAL           8192


static void helpCmd()
{
	fprintf(stderr, "BinauralRendererCmdl.exe\n");
	fprintf(stderr, "Purpose:\nBinauralize N-channel input file using TD renderer low-level parameters \n");
	fprintf(stderr, "Args:\n");
	fprintf(stderr, "  -if         <in  : input file>                                  (required)\n");
	fprintf(stderr, "  -of         <out : output binaural soundfile>                   (required)\n");
	fprintf(stderr, "  -bs         <in  : binaural bitstream (BRIR parameters)>        (required)\n");
	fprintf(stderr, "  -h2b        <in  : use H2B filters>                             (optional) OR\n");
	fprintf(stderr, "  -cicp       <in  : input CICP index> e.g., 5.1->6  22.2->13     (optional) OR\n");
	fprintf(stderr, "  -geo        <in  : path to geometric information file>          (optional)\n");
	fprintf(stderr, "  -delay      <in  : apply BRIR begin delay, 1:yes, 0:no>         (optional, 0 by default)\n");
	fprintf(stderr, "  -bitDepth   <in  : wave output file format: 16, 24 or 32        (optional)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage ex1: -if in24chan.wav -cicp 13 -of outBinau.wav -bs BRIR_vl.bs -delay 1");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage ex2: -if inHoa.wav -h2b 1 -of outBinau.wav -bs BRIR_h2b.bs");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage ex3: -if in.wav -geo geofile.txt -of outBinau.wav -bs BRIR_vl.bs");
}

/*---------------------------------------------------------------------------*/
static int parseCmdParameter(int   argc,
                             char *argv[],
                             char *sInputFile,
                             int  *configType,
                             int  *h2b,
                             int  *cicp,
                             char *sGeoPath,
                             char *sBSpath,
                             int  *beginDelay,
                             int  *bytesPerSample,
                             char *sOutputFile)
{
	unsigned int nArgs = 0;
	int	i;
	int	outBitDepth = 0;

	*configType = -1; /* 0:HOA  1:CICP 2:GEO */

	/* loop over all parameters */
	for (i=1; i<argc; ++i )
	{
		if (!strcmp(argv[i],"-if"))               /* Required */
		{
			if (++i < argc )
			{
				strcpy(sInputFile, argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-h2b"))         /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument!  -h2b, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 0;
			if (++i < argc )
			{
				*h2b = atoi(argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-cicp"))        /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument! -h2b, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 1;
			if (++i < argc )
			{
				*cicp = atoi(argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-geo"))         /* Optional */
		{
			if (*configType != -1)
			{
				fprintf(stderr,"Error : Wrong argument! -h2b, -cicp and -geo are not compatible \n\n");
				return -1;
			}
			*configType = 2;
			if (++i < argc )
			{
				strcpy(sGeoPath, argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-of"))          /* Required */
		{
			if (++i < argc )
			{
				strcpy(sOutputFile, argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-bs"))          /* Required */
		{
			if (++i < argc )
			{
				strcpy(sBSpath, argv[i]);
				nArgs++;
				continue;
			}
		}
		else if (!strcmp(argv[i],"-delay"))       /* Optional */
		{
			if (++i < argc )
			{
				*beginDelay = atoi(argv[i]);
				continue;
			}
		}
		else if (!strcmp(argv[i],"-bitDepth"))    /* Optional */
		{
			if (++i < argc )
			{
				outBitDepth = atoi(argv[i]);
				switch (outBitDepth) {
					case 16:
					case 24:
					case 32:
						*bytesPerSample = outBitDepth>>3; /* bits to bytes */
					break;
					default:
						fprintf(stderr,"Error : Unsupported wave output format. Please choose -bitDepth <16,24,32>, meaning 16, 24 bits LPCM or 32-bit IEE Float wave output.\n\n");
						helpCmd();
						return -1;
				}
				continue;
			}
		}
		else 
		{
			fprintf(stderr,"Error : Wrong argument! Require at least 4 arguments : -if, -of, -bs, and (-h2b OR -cicp OR -geo)\n\n");
			helpCmd();
			return -1;
		}  
	}

	/* check number of parameters found */
	if(nArgs < 4)
	{
		fprintf(stderr,"Error : Wrong argument! Require at least 4 arguments : -if, -of, -bs, and (-h2b OR -cicp OR -geo)\n\n");
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
static int FileExist(char *pPath)
{
	FILE * fid = fopen(pPath, "r");

	if(fid != NULL)
	{
		fclose(fid);
		return 1;
	}
	else
	{
		return 0;
	}
}

/*---------------------------------------------------------------------------*/
int main(int argc, char *pArgv[] )
{
	int configType = -1; /* input config type  0:HOA  1:CICP 2:GEO */
	int cicp = -1;
	int h2b = -1;
	int hoaOrder = 0;
	char sGeoPath[256] = ("");

	char	sPathInFile[256] = ("");
	char	sPathOutFile[256] = ("");
	char	sPathBitstream[256] = ("");
	int		ind,indN;
	int		nbSamples = 0;
	int		i, j, k;
	float  **pDataIn_32f = NULL;
	float  **pDataOut_32f = NULL;
	float	**ppSignalInRenderer = NULL;
	float	**ppSignalOutRenderer = NULL;
	float	*pWork_32f = NULL;
	int		returnedCode_8s;
	float	*buf_32f=NULL;
	FILE *pfIn = NULL;
	FILE *pfOut = NULL;
	unsigned int numChannelsInput, fsInput, bytedepth;
	unsigned long sizeInterleavedInput;
	unsigned long nTotalSamplesWrittenPerChannel = 0; 
	int nSamplesPerChannelFilled;
	unsigned int sampleOffset = 0;
	unsigned int isLastFrame = 0;
	CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometryExpected[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
	int numSpeakerExpected=0, numChannelsExpected=0, numLFEsExpected=0;
	int channelMap[MAX_NUM_BRIR_PAIRS] = {0};
	int wavIO_error = 0;
	WAVIO_HANDLE hWavIO = NULL;
	BinauralStruct_32f		*pBinaural = NULL;
	BinauralRendering		*pBinauralRendering = NULL;
	BinauralRepresentation	*pBinauralRepresentation = NULL;
	TdBinauralRendererParam *pTdBinauralRendererParam = NULL;
	float ***pppTaps_direct, ***pppTaps_diffuse, **ppDirectFc, **ppDiffuseFc;
	int nbSamplesProcessed = 0;
	int beginDelay = 0;
	int bytesPerSample = 0;

	fprintf(stderr, "\n\n"); 
	fprintf(stderr, "\n");
	fprintf(stderr, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*                      TD Binaural Rendering Module                           *\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*                                  %s                                *\n", __DATE__);
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
	fprintf(stderr, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*******************************************************************************\n");
	fprintf(stderr, "\n");


	/* Parse input parameters */
	if( parseCmdParameter(	argc,
		pArgv,
		sPathInFile,
		&configType,
		&h2b,
		&cicp,
		sGeoPath,
		sPathBitstream,
		&beginDelay,
		&bytesPerSample,
		sPathOutFile) != 0)
	{
		return -1;
	}
	replaceSlash(sPathBitstream);
	replaceSlash(sPathInFile);
	replaceSlash(sPathOutFile);

	/* Load input wavfile */
	if ( (FileExist(sPathInFile) != 1) || (sPathInFile[0] == '\0') )
	{
		fprintf(stderr, "Cannot open input file\n");
		return -1;
	}

	/* open .wav files */
	pfIn = fopen(sPathInFile, "rb"); 
	pfOut = fopen(sPathOutFile, "wb"); 

  if ((NULL != pfIn) && (NULL != pfOut)) {
		/* open wavIO */
		wavIO_error = wavIO_init(&hWavIO, BINAURAL_TEST_LGR_SIGNAL, 0, 0);
		if (0 != wavIO_error) {
			fprintf(stderr, "Error during wavIO initialization.\n");
			fclose(pfIn);
			fclose(pfOut);
			return -1;
		}

		wavIO_error = wavIO_openRead(hWavIO, pfIn, &numChannelsInput, &fsInput, &bytedepth, &sizeInterleavedInput, &nSamplesPerChannelFilled);
		sizeInterleavedInput *= numChannelsInput;
		if (0 != wavIO_error) {
			fprintf(stderr, "Cannot open input file.\n");
			fclose(pfIn);
			fclose(pfOut);
			return -1;
		}

		/* set output byte depth to input byte depth if -bitDepth was not given */
		if (bytesPerSample == 0)
		{
			bytesPerSample = bytedepth;
		}

		wavIO_error = wavIO_openWrite(hWavIO, pfOut, BINAURAL_NB_OUTPUT, fsInput, bytesPerSample);
		if (0 != wavIO_error) {
			fprintf(stderr, "Cannot open output file.\n");
			fclose(pfIn);
			fclose(pfOut);
			return -1;
		}
	} else {
		fprintf(stderr, "Cannot open input/output files\n");
		return -1;
	}

	/* Load geometry and check number of input channels */
	switch (configType)
	{
	case 0: /* HOA */
		{
			hoaOrder = sqrt( (float) numChannelsInput) - 1;
			fprintf(stderr,"Input wavfile HOA order : %d \n", hoaOrder);
			numChannelsExpected = (hoaOrder+1)*(hoaOrder+1);
			numLFEsExpected     = 0;
			numSpeakerExpected  = numChannelsExpected + numLFEsExpected;
			if ( numChannelsInput !=  numSpeakerExpected)
			{
				fprintf(stderr,"Error : HOA input wavfile should have (%d+1)^2 channels, while it has %d !\n",
					hoaOrder,
					numChannelsInput);
				return -1;
			}
			break;
		}

	case 1: /* CICP */
		{
			cicp2geometry_get_geometry_from_cicp(cicp, pGeometryExpected, &numChannelsExpected, &numLFEsExpected);
			numSpeakerExpected = numChannelsExpected + numLFEsExpected;
			if ( numChannelsInput != numSpeakerExpected )
			{
				fprintf(stderr,"Error : CICP input wavfile should have %d channels, while it has %d !\n",
					numSpeakerExpected,
					numChannelsInput);
				return -1;		
			}
			break;
		}

	case 2: /* GEO */
		{
			cicp2geometry_get_geometry_from_file(sGeoPath, pGeometryExpected, &numChannelsExpected, &numLFEsExpected);
			numSpeakerExpected = numChannelsExpected + numLFEsExpected;
			if ( numChannelsInput !=  numSpeakerExpected )
			{
				fprintf(stderr,"Error : GEO input wavfile should have %d channels, while it has %d !\n",
					numSpeakerExpected,
					numChannelsInput);
				return -1;		
			}
			break;
		}
	}
	

	/* Load binaural bitstream */
	pBinauralRendering = BinauralRendering_initAlloc();
	if (bitstreamRead(sPathBitstream, pBinauralRendering) != 0)
	{
		fprintf(stderr, "\nError : can not read %s !\n", sPathBitstream);
		/*system("PAUSE");*/
		return -1;
	}

	/* set low-level renderer parameters : pTdBinauralRendererParam */
	pTdBinauralRendererParam = NULL;
	if (configType == 0)  /* HOA */
	{
		for (i=0; 
			i < pBinauralRendering->numBinauralRepresentation;
			i++)
		{
			pBinauralRepresentation = pBinauralRendering->ppBinauralRepresentation[i];
			if (   ( pBinauralRepresentation->binauralDataFormatID == 2 ) /* TD low-level */
				&& ( pBinauralRepresentation->isHoaData )
				&& ( pBinauralRepresentation->brirSamplingFrequency == fsInput )
				&& ( pBinauralRepresentation->hoaOrderBinaural >= hoaOrder )  )
			{
				pTdBinauralRendererParam = pBinauralRepresentation->pTdBinauralRendererParam;
				/* restrict renderer to the fisrt numChannelsInput HOA channels */
				pTdBinauralRendererParam->numChannel = numChannelsInput;
				for (j=0; j<numSpeakerExpected; j++)
				{
					channelMap[j] = j;
				}
				break;
			}
		}
	}
	else /* CICP or GEO */
	{
		for (i=0; 
			i < pBinauralRendering->numBinauralRepresentation;
			i++)
		{
			pBinauralRepresentation = pBinauralRendering->ppBinauralRepresentation[i];
			
			if (   ( pBinauralRepresentation->binauralDataFormatID == 2 ) /* TD low-level */
				&& ( pBinauralRepresentation->isHoaData == 0 )
				&& ( pBinauralRepresentation->brirSamplingFrequency == fsInput )   )
			{

				/* search each speaker of the target layout in the BRIR layout */
				for (j=0; j<numSpeakerExpected; j++)
				{
					ind = -1;
					find_geometry_in_speakerConfig3d(
						&pGeometryExpected[j],
						&pBinauralRepresentation->Setup_SpeakerConfig3d,
						&ind);
					if (ind == -1)
					{
						/* could not find the appropriate virtual speaker */
						pTdBinauralRendererParam = NULL;
						break;
					}
					else
					{
						/* pour l'instant tout va bien */
						pTdBinauralRendererParam = pBinauralRepresentation->pTdBinauralRendererParam;
						channelMap[j] = ind;
					}
				}

				if (pTdBinauralRendererParam != NULL)
				{
					/* found the approprate representation, and corresponding channelMap, so break the main loop ! */
					break;
				}
			}
		}

	}

	if ( pTdBinauralRendererParam == NULL )
	{
		if (configType == 0) /* H2B */
		{
			fprintf(stderr, "\nDid not find appropriate H2B filters in %s\n", sPathBitstream);
			return -1;
		}
		else /* CICP or GEO */
		{
			fprintf(stderr, "\nError : could not find appropriate filters (expect low-level parameters) in %s !\n", sPathBitstream);
			fprintf(stderr, "          either fs is different, or the spatial layout, or both... \n");
			return -1;
		}
	}
	
	/* numChannelsInput may be less than pTdBinauralRendererParam->numChannel, but not the contrary */
	if (numChannelsInput > pTdBinauralRendererParam->numChannel)
	{
		fprintf(stderr,"Error : input wavfile has %d channels while binaural bitstream has %d channels\n",
			numChannelsInput,
			pTdBinauralRendererParam->numChannel); 

		return -1;
	}

	/* Buffers for interleaved input/output samples */
	pDataIn_32f = (float **)calloc(
		numChannelsInput,
		sizeof(float*) );
	for(ind = 0; ind < numChannelsInput; ind++)
	{
		pDataIn_32f[ind] = (float *)calloc(
			BINAURAL_TEST_LGR_SIGNAL,
			sizeof(float) );
	}

	pDataOut_32f = (float **)calloc(
		BINAURAL_NB_OUTPUT,
		sizeof(float) );
	for(ind = 0; ind < BINAURAL_NB_OUTPUT; ind++)
	{
		pDataOut_32f[ind] = (float *)calloc(
			pTdBinauralRendererParam->lenDirect * (1 + BINAURAL_TEST_LGR_SIGNAL / pTdBinauralRendererParam->lenDirect),
			sizeof(float) );
	}

	/* Buffers for NON-interleaved input/output samples */
	ppSignalInRenderer = (float **)malloc( pTdBinauralRendererParam->numChannel * sizeof(float*) );
	ppSignalOutRenderer = (float **)malloc( BINAURAL_NB_OUTPUT * sizeof(float*) );

	for(ind = 0; ind < pTdBinauralRendererParam->numChannel; ind++)
	{
		ppSignalInRenderer[ind] = (float*)malloc(
			sizeof(float ) *
			BINAURAL_TEST_LGR_SIGNAL );

		Set_32f(
			0,
			ppSignalInRenderer[ind],
			BINAURAL_TEST_LGR_SIGNAL);
	}

	for(ind = 0; ind < BINAURAL_NB_OUTPUT; ind++)
	{
		ppSignalOutRenderer[ind] = (float*)malloc(
			sizeof(float ) *
			pTdBinauralRendererParam->lenDirect * 
			(1 + BINAURAL_TEST_LGR_SIGNAL / pTdBinauralRendererParam->lenDirect) );

		Set_32f(
			0,
			ppSignalOutRenderer[ind],
			pTdBinauralRendererParam->lenDirect * 
			(1 + BINAURAL_TEST_LGR_SIGNAL / pTdBinauralRendererParam->lenDirect ));
	}

	/* Init Binaural Struct */
	returnedCode_8s = BinauralStruct_initAlloc_32f(
		0,
		pTdBinauralRendererParam->lenDirect,
		pTdBinauralRendererParam->lenDirect,
		pTdBinauralRendererParam->numDiffuseBlock,
		pTdBinauralRendererParam->numChannel,
		BINAURAL_NB_OUTPUT,
		&pBinaural );

	if ( returnedCode_8s < 0 )
	{
		fprintf(stderr, "Allocation problem.\n" );
		return -6;
	}

	/* convert arrays to pointers */
	pppTaps_direct = (float ***) malloc(BINAURAL_NB_OUTPUT*sizeof(float **));
	for (i=0; 
		i<BINAURAL_NB_OUTPUT; 
		i++)
	{
		pppTaps_direct[i] = 
			(float **) malloc(pTdBinauralRendererParam->numChannel*sizeof(float *));
		for (j=0; 
			j<pTdBinauralRendererParam->numChannel; 
			j++)
		{
			pppTaps_direct[i][j] = 
				(float *) malloc(pTdBinauralRendererParam->lenDirect *sizeof(float));
			for (k=0;
				k<pTdBinauralRendererParam->lenDirect;
				k++)
			{
				pppTaps_direct[i][j][k] =
					pTdBinauralRendererParam->pppTaps_direct[i][j][k];
			}
		}
	}

	pppTaps_diffuse = 
		(float ***) malloc(pTdBinauralRendererParam->numDiffuseBlock*sizeof(float **));
	for (i=0; 
		i<pTdBinauralRendererParam->numDiffuseBlock; 
		i++)
	{
		pppTaps_diffuse[i] = 
			(float **) malloc(BINAURAL_NB_OUTPUT*sizeof(float *));
		for (j=0; 
			j<BINAURAL_NB_OUTPUT; 
			j++)
		{
			pppTaps_diffuse[i][j] = 
				(float *) malloc(pTdBinauralRendererParam->lenDirect *sizeof(float));
			for (k=0;
				k<pTdBinauralRendererParam->lenDirect;
				k++)
			{
				pppTaps_diffuse[i][j][k] =
					pTdBinauralRendererParam->pppTaps_diffuse[i][j][k];
			}
		}
	}

	ppDirectFc = 
		(float **) malloc(BINAURAL_NB_OUTPUT*sizeof(float *));
	for (i=0; 
		i<BINAURAL_NB_OUTPUT; 
		i++)
	{
		ppDirectFc[i] = (float *) malloc(pTdBinauralRendererParam->numChannel*sizeof(float));
		for (j=0; 
			j<pTdBinauralRendererParam->numChannel; 
			j++)
		{
			ppDirectFc[i][j] = pTdBinauralRendererParam->ppDirectFc[i][j];
		}
	}

	
	ppDiffuseFc = 
		(float **) malloc(BINAURAL_NB_OUTPUT*sizeof(float *));
	for (i=0; 
		i<BINAURAL_NB_OUTPUT; 
		i++)
	{
		ppDiffuseFc[i] = (float *) malloc(pTdBinauralRendererParam->numDiffuseBlock*sizeof(float));
		for (j=0; 
			j<pTdBinauralRendererParam->numDiffuseBlock; 
			j++)
		{
			ppDiffuseFc[i][j] = pTdBinauralRendererParam->ppDiffuseFc[i][j];
		}
	}


	BinauralStruct_set_32f(
		pppTaps_direct,
		pTdBinauralRendererParam->lenDirect,
		pppTaps_diffuse,
		pTdBinauralRendererParam->lenDirect,
		pTdBinauralRendererParam->numDiffuseBlock,
		pTdBinauralRendererParam->pInvDiffuseWeight,
		ppDirectFc, 
		ppDiffuseFc, 
		pBinaural );

	pWork_32f = (float *)malloc(
		sizeof(float ) *
		( pBinaural->m_filterSize_16s ) );

	

	/* add bsDelay zeros */
	if (beginDelay)
	{
		sampleOffset = -pTdBinauralRendererParam->beginDelay * numChannelsInput;
	}
	else
	{
		sampleOffset = 0;
	}

	wavIO_setDelay(hWavIO, sampleOffset);

	/* Loop process */
	nbSamples = sizeInterleavedInput / numChannelsInput;
	while ( nbSamples > 0 )
	{
		unsigned int samplesReadPerChannel = 0; 
		unsigned int samplesToWritePerChannel = 0; 
		unsigned int samplesWrittenPerChannel = 0; 
		unsigned int nZerosPaddedBeginning = 0; 
		unsigned int nZerosPaddedEnd = 0;

		if (NULL != hWavIO) {
			wavIO_error = wavIO_readFrame(hWavIO, pDataIn_32f, &sampleOffset, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);

			if (isLastFrame)
			{
        /* clear memory in the last frame */
				for ( i = 0 ; i < numChannelsInput ; i++ )
				{
					memset(pDataIn_32f[i] + sampleOffset, 0, sizeof(float) * (BINAURAL_TEST_LGR_SIGNAL - sampleOffset));
				}
			}
		}

		/* apply channelMap to input signal, to fit renderer channel order */
		for ( j = 0 ; j < BINAURAL_TEST_LGR_SIGNAL ; j++ )
		{
			for ( i = 0 ; i < numChannelsInput ; i++ )
			{
				ppSignalInRenderer[channelMap[i]][j] = pDataIn_32f[i][j] ;
			}
		}

		nbSamplesProcessed = BinauralStruct_processLowDelayOLA_32f(
			ppSignalInRenderer,
			ppSignalOutRenderer,
			BINAURAL_TEST_LGR_SIGNAL,
			pWork_32f,
			pBinaural );

		nbSamples -= nbSamplesProcessed;

		if (nbSamples < 0)
		{
			nbSamplesProcessed += nbSamples;
			nbSamples = 0;
		}

		if (nbSamplesProcessed > 0 )
		{
			for ( j = 0 ; j < nbSamplesProcessed ; j++ )
			{
				for ( i = 0 ; i < BINAURAL_NB_OUTPUT ; i++ )
				{
					pDataOut_32f[i][j] = ppSignalOutRenderer[i][j];
				}
			}

			if (NULL != hWavIO) {
				wavIO_error = wavIO_writeFrame(hWavIO, pDataOut_32f, nbSamplesProcessed, &samplesWrittenPerChannel);
				nTotalSamplesWrittenPerChannel += samplesWrittenPerChannel;
			}
		}

		fprintf(stderr,"Proccessed sample: %2.1f (%%) \r" ,
			100.f - numChannelsInput * 100.f *  nbSamples / sizeInterleavedInput);
	}


	/* close files and exit */
	fprintf(stderr, "\n" );

	if (NULL != hWavIO) {
		wavIO_error = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
		if (0 == wavIO_error)
		{
			fprintf(stderr, "\nOutput file %s is written.\n", sPathOutFile);
		}

		wavIO_error = wavIO_close(hWavIO);
		if (0 != wavIO_error) {
			fprintf(stderr, "Error during wavIO deinitialization.\n");
		}
	}

	for( indN = 0; indN < numChannelsInput; indN++)
	{
		free( ppSignalInRenderer[indN] );
	}
	for( indN = 0; indN<BINAURAL_NB_OUTPUT; indN++)
	{
		free( ppSignalOutRenderer[indN] );
	}

	free( ppSignalInRenderer );
	free( ppSignalOutRenderer );

	for(indN = 0; indN < numChannelsInput; indN++)
	{
		free( pDataIn_32f[indN] );
	}
	for(indN = 0; indN < BINAURAL_NB_OUTPUT; indN++)
	{
		free( pDataOut_32f[indN] );
	}

	free( pDataIn_32f );
	free( pDataOut_32f );

	BinauralStruct_freeAlloc_32f( &pBinaural );

	return 0;
}
