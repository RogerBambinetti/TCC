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

#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h> 
#include <stdio.h> 

#include "saocDecoder.h"
#include "saoc_render.h"
#include "saoc_decor.h"
#include "wavIO.h"
#include "saoc_decode.h"

#include "qmflib.h"
#include "qmflib_hybfilter.h"
#include "saoc_spatial_filereader.h"
#include "saoc_bitstream.h"
#include "saocdeclib.h"
#include "error.h"
#include "cicp2geometry.h"
#include "saoc_drc_lfe.h"

#ifndef M_PI 
#define M_PI           3.14159265358979323846
#endif

#ifndef SAOC_CICP_INVALID
#define SAOC_CICP_INVALID  -10
#endif

#ifndef SAOC_CICP_FROM_GEO
#define SAOC_CICP_FROM_GEO = -1
#endif


static int saocGetGeometryInformation(int CICPIndex, char* FilenameChannelList,int *numChannels, int *numLFEs, CICP2GEOMETRY_CHANNEL_GEOMETRY* GeometryInfo);
static int saoc_get_oam_sample( FILE* oam_file, OAM_SAMPLE* currentPosition, StructOamMultidata* oam_sample, uint16_t oam_version, uint16_t hasDynamicObjectPrio, uint16_t hasUniformSpread);
static void SAOC_get_channel_mapping( CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo,
                                      int saocChannelMappingInfo[SAOC_MAX_RENDER_CHANNELS],
                                      int saocLfeMappingInfo[SAOC_MAX_LFE_CHANNELS],
                                      int nChannelsOut,
                                      int numOutLFEs);

int saoc_compare_geometry(CICP2GEOMETRY_CHANNEL_GEOMETRY geoOne[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                          unsigned int numChannelsOne,
                          CICP2GEOMETRY_CHANNEL_GEOMETRY geoTwo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                          unsigned int numChannelsTwo,
                          unsigned int tolerance,
                          int mappingMatrix[SAOC_MAX_RENDER_CHANNELS]);


static INVERSION_PARAMETERS initInversionParameters(INVERSION_PARAMETERS inversionParam)
{
	INVERSION_PARAMETERS self;

	self.inversion_reltol = 1e-2f;
	self.inversion_abstol = 0.0f;
	self.omiteigvecs      = 1; 

	return self;
}


extern jmp_buf g_JmpBuf;

float pRenderMat_ch[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
float pRenderMat_lfe[SAOC_MAX_LFE_CHANNELS][SAOC_MAX_LFE_CHANNELS] = {{0}};

float frameRenderMat[SAOC_MAX_TIME_SLOTS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
float framePreMixMat[SAOC_MAX_TIME_SLOTS][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
int framePossOAM[SAOC_MAX_TIME_SLOTS] = {{0}};
int numOamPerFrame = 1;

int mappingMatrix[SAOC_MAX_RENDER_CHANNELS] = {0};

float pPreMixMatPre[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
float pPreMixMatCur[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};

QMFLIB_HYBRID_FILTER_MODE hybridMode = QMFLIB_HYBRID_THREE_TO_TEN; /* QMFLIB_HYBRID_THREE_TO_SIXTEEN; */
INVERSION_PARAMETERS inversionParam;


int main(int argc, char* argv[])
{

	tSaocDec    *decoder = NULL;
	tSaocDrcLfe *drcLfe  = NULL; 
	unsigned int uCh, j;
	int ch, ts, n, i, sj;
  unsigned int uch;

	int saocInputCh_Mapping[SAOC_MAX_RENDER_CHANNELS] = {0};
	int saocInputLFE_Mapping[SAOC_MAX_LFE_CHANNELS ]  = {0};

	CICP2GEOMETRY_CHANNEL_GEOMETRY saocOutputGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = { {0} }; /*[AZ EL isLFE] */
	int saocOutputCh_Mapping[SAOC_MAX_RENDER_CHANNELS] = {0};
	int saocOutputLFE_Mapping[SAOC_MAX_LFE_CHANNELS ]  = {0};
	int outCICPIndex = SAOC_CICP_INVALID;

	CICP2GEOMETRY_CHANNEL_GEOMETRY saocReferenceGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = { {0} }; /*[AZ EL isLFE] */
	int saocReferenceCh_Mapping[SAOC_MAX_RENDER_CHANNELS] = {0};
	int saocReferenceLFE_Mapping[SAOC_MAX_LFE_CHANNELS ]  = {0};
	int refCICPIndex = SAOC_CICP_INVALID;

  CICP2GEOMETRY_CHANNEL_GEOMETRY saocDmxGeometryInfo[CICP2GEOMETRY_MAX_LOUDSPEAKERS] = { {0} }; /*[AZ EL isLFE] */
	int saocDmxCh_Mapping[SAOC_MAX_RENDER_CHANNELS] = {0};
	int saocDmxLFE_Mapping[SAOC_MAX_LFE_CHANNELS ]  = {0};
  int dmxCICPIndex = SAOC_CICP_INVALID;

  int hasDmxLayout   = 0;
  int playoutDownmix = 0;

	int sacHeaderLen;
	int coreOutputFrameLength = 32; /* default value */

	int DecodingEnd = 0;
	int flushSaocDecoder = 0;

	int nInputSaocSignals = -1;
	int nChannelsOut      = -1;
	int numOutLFEs        = -1; 
  int nDmxChannels      = -1;
	int numDmxLFEs        = -1; 
	int numPremixChannels = -1;
	int numPremixLFEs     = -1;
	int frameLength;

	/* DRC imformation */
	int drcFlag  = 0;
	char* inDrcFilename   = 0;
	float ***drcMatrix     = 0;
	WAVIO_HANDLE hWavIO_Drc = 0;
	FILE *pDrcFile  = NULL;
	unsigned int nDrcChannels  = 0;
	unsigned int DrcSampleRate = 0;
	unsigned int DrcBytedepth  = 0;
	unsigned long nTotalDrcSamplesPerChannel = 0;
	int nDrcSamplesPerChannelFilled = 0;
	unsigned int isDrcLastFrame = 0;

	char* inFilenameOutputChannelList = 0;
	char* inFilenameReferenceChannelList = 0;
  char* inFilenameDmxChannelList = 0;

	char* indmxFilename = 0;
	char* inbsFilename  = 0;
	char* inrmFilename  = 0;
	char* outFilename   = 0;
	char* oamFilename   = 0;

	char qmfInFileReal[200];
	char qmfInFileImag[200];
	char qmfOutFileReal[200];
	char qmfOutFileImag[200];

	int isInQmf  = 0, isOutQmf = 0;
	int error_init  = 0, error_read  = 0, error_write = 0, error_close = 0;

	int frameNo     = 0;
	uint64_t currentParameterPosition = 0;

	unsigned long nTotalSamplesReadPerChannel = 0;
	unsigned int isLastFrame = 0;

	WAVIO_HANDLE hWavIO = NULL, hWavIO_real = NULL, hWavIO_imag = NULL;

	int bFillLastFrame = 0;
	int nSamplesPerChannelFilled;

	FILE *fIn = NULL, *fOut = NULL;
	FILE *pQmfInRealFile  = NULL, *pQmfInImagFile  = NULL, *pQmfOutRealFile = NULL, *pQmfOutImagFile = NULL;

	unsigned int nIn_WAV_IO_Channels = 0;
	unsigned int nIn_WAV_IO_Channels_tmp = 0;
	unsigned int nOut_WAV_IO_Channels = 24;

	unsigned int InSampleRate;
	unsigned long nTotalSamplesPerChannel;
	unsigned long nTotalSamplesWrittenPerChannel = 0;
	unsigned int InBytedepth = 0;

	int hybridHFAlign = 1; /* 0: Align hybrid HF part with LF part, 1: no delay alignment for HF part */ 

	int isFirstFrame = 1;

	int use_ren_mat_files = 0;

	float *time_instances;
	int num_time_instances;
	float ***pRenderMats;

	int coder_offset;

	int OFFSET_TD = SAOC_QMF_DELAY_ANA + SAOC_QMF_DELAY_HYB;  /* 704 samples */
	int OFFSET_FD = SAOC_QMF_DELAY_HYB;  /* 384 samples */

	/* OAM data for rendering*/
	FILE* fOAMFile = NULL;
	uint16_t numObjects = 0;
	uint16_t oamVersion = 2;
	uint16_t hasDynamicObjectPrio = 0;
	uint16_t hasUniformSpread = 1;

	StructOamMultidata* oamSample;
	OAM_SAMPLE* currentPosition = NULL;

	uint64_t oam_sample_pos = 0;
	int errorMessage = 0;
	int current_blocksize;

	HANDLE_GVBAPRENDERER hgVBAPRenderer;
	float** gainMatrix;

	HANDLE_GVBAPRENDERER hgVBAPRenderer_PreMix;
	float** gainPreMixMatrix;

	float invSqrt2;

	inversionParam = initInversionParameters(inversionParam);
	invSqrt2 = (float) (1.0/sqrt(2.0));

	/* ************************************************************************* */
	/* print disclaimer info */
	saoc_printDisclaimer();

	/* parse command line */
	error_init = saoc_parseCmdl(argc,
		&argv,
		&outCICPIndex,
		&refCICPIndex,
    &dmxCICPIndex,
		&inFilenameOutputChannelList,
		&inFilenameReferenceChannelList,
    &inFilenameDmxChannelList,
		&indmxFilename,
		&inbsFilename,
		&inrmFilename,
		&outFilename,
		&oamFilename,
		&inDrcFilename,
		&drcFlag,
		qmfInFileReal,
		qmfInFileImag,
		qmfOutFileReal,
		qmfOutFileImag,
		&coreOutputFrameLength,
		&isInQmf,
		&isOutQmf
		,&sacHeaderLen);

	coder_offset = 1;


	if (error_init) 
	{
		fprintf(stderr, "\nError during Command Line parsing! Program exits.\n");
		return -1;
	}

	if(setjmp(g_JmpBuf) == 0) 
	{
		error_init = saocGetGeometryInformation(outCICPIndex, inFilenameOutputChannelList,&nChannelsOut, &numOutLFEs, saocOutputGeometryInfo);

		if (error_init) 
		{
			fprintf(stderr,"Error parsing output Layout Index! Program exits.\n");
			return -1;
		}

    SAOC_get_channel_mapping(saocOutputGeometryInfo, saocOutputCh_Mapping, saocOutputLFE_Mapping, nChannelsOut, numOutLFEs);
    
    if (dmxCICPIndex != SAOC_CICP_INVALID)
    {
      hasDmxLayout = 1;
		  error_init = saocGetGeometryInformation(dmxCICPIndex, inFilenameDmxChannelList,&nDmxChannels, &numDmxLFEs, saocDmxGeometryInfo);

		  if (error_init) 
		  {
			  fprintf(stderr,"Error parsing output Layout Index! Program exits.\n");
			  return -1;
		  }
    
      SAOC_get_channel_mapping(saocDmxGeometryInfo, saocDmxCh_Mapping, saocDmxLFE_Mapping, nDmxChannels, numDmxLFEs);

      playoutDownmix = saoc_compare_geometry( saocDmxGeometryInfo, nDmxChannels + numDmxLFEs,
                                              saocOutputGeometryInfo, nChannelsOut + numOutLFEs,
                                              0,
                                              mappingMatrix);

		}

    

		if (refCICPIndex != -10)
		{
			error_init = saocGetGeometryInformation(refCICPIndex, inFilenameReferenceChannelList, &numPremixChannels, &numPremixLFEs, saocReferenceGeometryInfo);
			if (error_init)
			{
				fprintf(stderr,"Error parsing reference Layout Index! Program exits.\n");
				return -1;
			}
		}

		decoder = saoc_DecoderOpen(isInQmf, isOutQmf, nChannelsOut, numOutLFEs, hybridMode, hybridHFAlign, &nIn_WAV_IO_Channels, &nOut_WAV_IO_Channels, inbsFilename,
			sacHeaderLen,
			outCICPIndex, saocOutputGeometryInfo,
			numPremixChannels,
			coreOutputFrameLength,
			&error_init);  
		if (error_init)
		{
			saoc_DecoderClose(decoder, nIn_WAV_IO_Channels, nOut_WAV_IO_Channels);
			fprintf(stderr,"saocdeclib.c:  Error during initialisation of the decorrelators! Program exits.\n");
			return -1;
		}

		if (drcFlag == 1 && decoder->numInLFEs > 0)
		{
			drcLfe = saoc_DrcLfeInterfaceOpen(decoder->isInQmf,
				decoder->isOutQmf,
				decoder->numInLFEs,
				decoder->numOutLFEs,
				decoder->nQmfBands);
		} 

		nInputSaocSignals = decoder->nSaocObjects + decoder->nSaocChannels + decoder->numInLFEs;

		if (use_ren_mat_files)
		{
			time_instances = SAOC_InitTimeInstances(inrmFilename,&num_time_instances);
			pRenderMats    = SAOC_InitRenderMats(inrmFilename,num_time_instances,time_instances,decoder->nChannelsOut); 
		} 
		else
		{
			if (decoder->nSaocChannels > 0)
			{
				SAOC_get_channel_mapping(decoder->saocInputGeometryInfo, saocInputCh_Mapping, saocInputLFE_Mapping, decoder->nSaocChannels, decoder->numInLFEs);

				saoc_get_RenMat_ChannelPath(decoder->inSaocCICPIndex, outCICPIndex, saocInputCh_Mapping, saocInputLFE_Mapping, decoder->nSaocChannels, decoder->numInLFEs,
					saocOutputCh_Mapping, saocOutputLFE_Mapping, nChannelsOut, numOutLFEs, inrmFilename, pRenderMat_ch, pRenderMat_lfe);
			}

			if (decoder->nSaocObjects > 0) 
			{
				if (decoder->nNumPreMixedChannels)
				{
					SAOC_get_channel_mapping(saocReferenceGeometryInfo, saocReferenceCh_Mapping, saocReferenceLFE_Mapping, decoder->nNumPreMixedChannels, numPremixLFEs);
				}

				/* open OAM file for reading and get numObjects */
				fOAMFile = oam_read_open(oamFilename, &numObjects, &oamVersion, &hasDynamicObjectPrio, &hasUniformSpread);

				/* initialization of oam_multidata structure */
				oamSample = oam_multidata_create(numObjects, 1);

				/* allocate memory for position data */
				currentPosition = (OAM_SAMPLE*)calloc(numObjects, sizeof(OAM_SAMPLE));

				gainMatrix = (float**)calloc(numObjects, sizeof(float*));

				for (ch=0; ch<numObjects; ch++)
				{
					gainMatrix[ch] = (float*)calloc(nChannelsOut + numOutLFEs, sizeof(float));
				}

				errorMessage = gVBAPRenderer_Open(&hgVBAPRenderer, numObjects, 1, outCICPIndex, saocOutputGeometryInfo, nChannelsOut + numOutLFEs);

				if ( errorMessage )
				{
					fprintf(stderr, "gVBAPRenderer_Open returned %d\n", errorMessage);
					return -1 ;
				}

				if (decoder->nNumPreMixedChannels)
				{
					gainPreMixMatrix = (float**)calloc(numObjects, sizeof(float*));

					for (ch=0; ch<numObjects; ch++)
					{
						gainPreMixMatrix[ch] = (float*)calloc(decoder->nNumPreMixedChannels + numPremixLFEs, sizeof(float));
					}

					errorMessage = gVBAPRenderer_Open(&hgVBAPRenderer_PreMix, numObjects, 1, refCICPIndex, saocReferenceGeometryInfo, decoder->nNumPreMixedChannels + numPremixLFEs);

					if ( errorMessage )
					{
						fprintf(stderr, "gVBAPRenderer_Open returned %d\n", errorMessage);
						return 0 ;
					}
				}
			}
		}
	} 
	else
	{
		printf("\n\nFinishing of Program, cleaning up resources\n");

		if (use_ren_mat_files) 
		{
			SAOC_deleteTimeInstances(time_instances);
			SAOC_deleteRenderMats(num_time_instances,pRenderMats);
		} 
		else 
		{
			if (decoder->nSaocObjects > 0)
			{
				/* close OAM file */
				oam_read_close(fOAMFile);

				if ( currentPosition )
					free(currentPosition);

				for (ch=0; ch<numObjects; ch++)
				{
					free(gainMatrix[ch]);
				}
				free(gainMatrix);

				gVBAPRenderer_Close(hgVBAPRenderer);

				if (decoder->nNumPreMixedChannels)
				{
					for (ch=0; ch<numObjects; ch++)
					{
						free(gainPreMixMatrix[ch]);
					}
					free(gainPreMixMatrix);

					gVBAPRenderer_Close(hgVBAPRenderer_PreMix);
				}
			}
		}

		if (drcFlag == 1 && decoder->numInLFEs > 0)
		{
			saoc_DrcLfeInterfaceClose(drcLfe,
				decoder->isInQmf,
				decoder->isOutQmf,
				decoder->numInLFEs,
				decoder->numOutLFEs);
		}

		saoc_DecoderClose(decoder, nIn_WAV_IO_Channels, nOut_WAV_IO_Channels);

		return(0);    
	}

	/* ************************************************************************* */
	if(setjmp(g_JmpBuf)==0) 
	{ 
		if(isInQmf)
		{
			pQmfInRealFile  = fopen(qmfInFileReal, "rb");
			pQmfInImagFile  = fopen(qmfInFileImag, "rb");
		} 
		else 
		{
			/* Open input file */
			fIn = fopen(indmxFilename, "rb");

			if ( fIn != NULL) 
			{
				fprintf(stderr, "SAOC Decoder input file: %s.\n", indmxFilename);
			}
			else 
			{
				myexit("Could not open input file\n");
			}
		}   

		fprintf(stderr, "SAOC bitsream file: %s.\n", inbsFilename);

		if(isOutQmf) 
		{
			pQmfOutRealFile = fopen(qmfOutFileReal, "wb");
			pQmfOutImagFile = fopen(qmfOutFileImag, "wb");
		} 
		else 
		{
			/* Open output file */
			fOut = fopen(outFilename, "wb");

			if ( fOut != NULL) 
			{
				fprintf(stderr, "SAOC Decoder output file: %s.\n", outFilename);
			}
			else 
			{
				myexit("Could not open output file\n"); 
			}
		}

		if (drcFlag == 1)
		{ 
			drcMatrix  = (float***) calloc(decoder->nTimeSlots, sizeof(float**));
			for (ts = 0; ts < decoder->nTimeSlots; ts++ ) 
			{
				drcMatrix[ts] = (float**) calloc(nInputSaocSignals, sizeof(float*));
				for (ch = 0; ch < nInputSaocSignals; ch++ ) 
				{
					drcMatrix[ts][ch] = (float*)calloc(decoder->numParamBands, sizeof(float));
				}
			}

			error_init = wavIO_init(&hWavIO_Drc, decoder->numParamBands, 0, -decoder->moduleDelay);
			if ( 0 != error_init )  
			{
				fprintf(stderr, "Error during DRC interface initialization  .\n");
				return -1;
			}

			/* Open DRC file for reading */
			pDrcFile = fopen(inDrcFilename, "rb");
			error_init = wavIO_openRead(hWavIO_Drc, pDrcFile, &nDrcChannels, &DrcSampleRate, &DrcBytedepth, &nTotalDrcSamplesPerChannel, &nDrcSamplesPerChannelFilled);
			if ( 0 != error_init )
			{
				fprintf(stderr, "Error opening input DRC file\n");
				return -1;
			}
		}

		/* wavIO for wav file */
		error_init = wavIO_init(&hWavIO, decoder->frameSize, bFillLastFrame, -decoder->moduleDelay);
		if ( 0 != error_init ) 
		{
			fprintf(stderr, "Error during initialization.\n");
			return -1;
		}

		/* wavIO for qmf file */
		if(isInQmf || isOutQmf) 
		{
			error_init = wavIO_init(&hWavIO_real, decoder->frameSize, 0, -decoder->moduleDelay); 
			if ( 0 != error_init )
			{
				fprintf(stderr, "Error during initialization.\n");
				return -1;
			}
			error_init = wavIO_init(&hWavIO_imag, decoder->frameSize, 0, -decoder->moduleDelay); 
			if ( 0 != error_init )
			{
				fprintf(stderr, "Error during initialization.\n");
				return -1;
			}
		}

		if(isInQmf) /* wavIO for qmf file */
		{ 
			error_init = wavIO_openRead(hWavIO_real, pQmfInRealFile, &nIn_WAV_IO_Channels_tmp, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
			if ( 0 != error_init ) 
			{
				fprintf(stderr, "Error opening input qmf real file\n");
				return -1;
			}

			error_init = wavIO_openRead(hWavIO_imag, pQmfInImagFile, &nIn_WAV_IO_Channels_tmp, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
			if ( 0 != error_init ) 
			{
				fprintf(stderr, "Error opening input qmf imag file\n");
				return -1;
			}
		} 
		else /* wavIO for wav file */
		{  
			error_init = wavIO_openRead(hWavIO, fIn, &nIn_WAV_IO_Channels_tmp, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);

			if (nIn_WAV_IO_Channels != nIn_WAV_IO_Channels_tmp) 
			{
				fprintf(stderr, "Number of input channels different from the value signaled in the bitstream .\n");
				return -1;
			} 
			else
			{
				fprintf(stderr, "Number of input channels: %d.\n", nIn_WAV_IO_Channels);
			}

			fprintf(stderr, "Number of output channels: %d.\n", nOut_WAV_IO_Channels);

			if (InSampleRate != decoder->sampleFreq)
			{
				fprintf(stderr, "Sampling Frequency different from the value signaled in the bitstream .\n");
				return -1;
			} 
			else 
			{
				fprintf(stderr, "Input Sampling Frequency: %d.\n", InSampleRate);
			}
		}

		if (isOutQmf) /* wavIO for qmf file */
		{ 
			error_init = wavIO_openWrite(hWavIO_real, pQmfOutRealFile, nOut_WAV_IO_Channels , InSampleRate, QMF_BYTE_DEPTH);
			if ( 0 != error_init )
			{
				fprintf(stderr, "Error opening output qmf\n");
				return -1;
			}

			error_init = wavIO_openWrite(hWavIO_imag, pQmfOutImagFile, nOut_WAV_IO_Channels , InSampleRate, QMF_BYTE_DEPTH);
			if ( 0 != error_init ) 
			{
				fprintf(stderr, "Error opening output qmf\n");
				return -1;
			}
		} 
		else /* wavIO for wav file */
		{ 
			error_init = wavIO_openWrite(hWavIO, fOut, nOut_WAV_IO_Channels , InSampleRate, InBytedepth);
		}
	}
	else 
	{
		/* ========================= EXIT ============================================= */
		printf("\n\nFinishing of Program, cleaning up resources\n");

		if (use_ren_mat_files) 
		{
			SAOC_deleteTimeInstances(time_instances);
			SAOC_deleteRenderMats(num_time_instances,pRenderMats);
		} 
		else 
		{
			if (decoder->nSaocObjects > 0) 
			{
				/* close OAM file */
				oam_read_close(fOAMFile);

				if ( currentPosition )
					free(currentPosition);

				for (ch=0; ch<numObjects; ch++)
				{
					free(gainMatrix[ch]);
				}
				free(gainMatrix);

				gVBAPRenderer_Close(hgVBAPRenderer);

				if (decoder->nNumPreMixedChannels)
				{
					for (ch=0; ch<numObjects; ch++)
					{
						free(gainPreMixMatrix[ch]);
					}
					free(gainPreMixMatrix);

					gVBAPRenderer_Close(hgVBAPRenderer_PreMix);
				}
			}
		}

		if (drcFlag == 1)
		{ 
			for (ts = 0; ts < decoder->nTimeSlots; ts++ )
			{
				for (ch = 0; ch < nInputSaocSignals; ch++ )
				{
					free(drcMatrix[ts][ch]);
				}
				free(drcMatrix[ts]);
			}
			free(drcMatrix);

			error_close = wavIO_close(hWavIO_Drc);

			if ( error_close != 0 )
			{
				fprintf(stderr, "\n Error during DRC reading from file %s.\n",  outFilename);
			}

			if (decoder->numInLFEs > 0)
			{
				saoc_DrcLfeInterfaceClose(drcLfe,
					decoder->isInQmf,
					decoder->isOutQmf,
					decoder->numInLFEs,
					decoder->numOutLFEs);
			}
		}

		saoc_DecoderClose(decoder, nIn_WAV_IO_Channels, nOut_WAV_IO_Channels);

		if ( (hWavIO != NULL) || (hWavIO_real != NULL))
		{
			if (isOutQmf)
			{
				error_close = wavIO_updateWavHeader(hWavIO_real, &nTotalSamplesWrittenPerChannel);
				error_close = wavIO_updateWavHeader(hWavIO_imag, &nTotalSamplesWrittenPerChannel);
			} 
			else 
			{
				error_close = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
				/* assert(nTotalSamplesWrittenPerChannel==nTotalSamplesReadPerChannel); */
			} 

			if(isInQmf || isOutQmf) 
			{
				wavIO_close(hWavIO_real);
				wavIO_close(hWavIO_imag);
			} 

			error_close = wavIO_close(hWavIO);

			if ( error_close != 0 ) 
			{
				fprintf(stderr, "\n Error during writing to output %s.\n",  outFilename);
			}
		}
		return(0);
		/* ========================= END of EXIT ======================================= */
	}

	currentParameterPosition = (coder_offset + decoder->nTimeSlots-1) * decoder->nQmfBands;
	isFirstFrame = 1;
	fprintf(stderr, "\n====SAOC Decoding started====\n");

	flushSaocDecoder = decoder->moduleDelay;

	/* ========================= FRAME BASED PROCESSING ============================== */
	while(DecodingEnd != SAOC_END_DECODING) 
	{
		unsigned int nSamplesReadPerChannel = 0;
		unsigned int nSamplesToWritePerChannel = 0;
		unsigned int nSamplesWrittenPerChannel = 0;
		unsigned int nZerosPaddedBeginning = 0;
		unsigned int nZerosPaddedEnd = 0;
		unsigned int nDrcSamplesReadPerChannel = 0;
		unsigned int nDrcZerosPaddedBeginning = 0;
		unsigned int nDrcZerosPaddedEnd = 0;

		frameNo++;
		fprintf(stderr, "Processing Frame No: %d\r",frameNo);

		/* read input DRC frame   */
		if (drcFlag == 1) 
		{
			ts = 0;
			while(isDrcLastFrame == 0 && ts <  decoder->nTimeSlots) 
			{  
				wavIO_readFrame(hWavIO_Drc, drcMatrix[ts], &nDrcSamplesReadPerChannel, &isDrcLastFrame, &nDrcZerosPaddedBeginning, &nDrcZerosPaddedEnd);
				ts++;
			}

			/* Add dummy DRC values for SAOC 3D decoder flushing */
			if (ts < decoder->nTimeSlots)
			{
				for (sj = ts; sj < decoder->nTimeSlots; sj++)
				{
					for (ch = 0; ch < nInputSaocSignals; ch++)
					{
						for (n = 0; n < decoder->numParamBands; n++)
						{
							drcMatrix[sj][ch][n] = 1.0f;
						}
					}
				}
			}
		}
    
    /* empty buffers */
    if(!isInQmf)
	  {
  	  for (uCh = 0; uCh < nIn_WAV_IO_Channels; uCh++)
	    {
		   memset(decoder->inBuffer[uCh], 0, decoder->frameSize * sizeof(decoder->inBuffer[uCh][0]));
		  }
    } else {
      for (uCh = 0; uCh < nIn_WAV_IO_Channels; uCh++)
			{
        memset(decoder->qmfInRealBuffer[uCh], 0, decoder->frameSize * sizeof(decoder->qmfInRealBuffer[uCh][0]));
			  memset(decoder->qmfInImagBuffer[uCh], 0, decoder->frameSize * sizeof(decoder->qmfInImagBuffer[uCh][0]));
			}
    }

    /* read input wav/qmf frame   */
    if (!isLastFrame)
    {
      if(!isInQmf)
	    {
        error_read = wavIO_readFrame(hWavIO,decoder->inBuffer,&nSamplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning, &nZerosPaddedEnd);
      } else {
        error_read = wavIO_readFrame(hWavIO_real, decoder->qmfInRealBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
		    error_read = wavIO_readFrame(hWavIO_imag, decoder->qmfInImagBuffer, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
      }
    } else {
		  if (flushSaocDecoder < decoder->frameSize)
			{
			  DecodingEnd = SAOC_END_DECODING;
				nSamplesReadPerChannel = flushSaocDecoder;
		  } 
			else 
			{
			  DecodingEnd = SAOC_FLUSH_DECODER;
			  nSamplesReadPerChannel = decoder->frameSize;
			  flushSaocDecoder -= decoder->frameSize;
      }
    }
    nSamplesToWritePerChannel = nZerosPaddedBeginning + nSamplesReadPerChannel + nZerosPaddedEnd;

    if (playoutDownmix)
    {
      if(!isInQmf)
		  {
			  /*  Add SAOC delay to downmix channels */
			  for (uch = 0; uch < nIn_WAV_IO_Channels; uch++)
			  {
				  for (n = 0; n < decoder->moduleDelay; n++) 
				  {
            decoder->outBuffer[ mappingMatrix[uch] ][n] = decoder->tmpInBuffer[uch][n];
					  decoder->tmpInBuffer[uch][n]                = decoder->inBuffer[uch][decoder->frameSize - decoder->moduleDelay + n];
				  }
				  for (n = decoder->moduleDelay; n < nSamplesReadPerChannel ; n++) 
				  {
					  decoder->outBuffer[ mappingMatrix[uch] ][n] = decoder->inBuffer[uch][n - decoder->moduleDelay];
				  }
			  }
		  } 
      else 
      { 
        for (uch = 0; uch < nIn_WAV_IO_Channels; uch++)
		    {
			    for (n = 0; n < decoder->moduleDelay; n++) 
			    {
            decoder->qmfOutRealBuffer[ mappingMatrix[uch] ][n] = decoder->tmpInRealQmfBuffer[uch][n];
						decoder->qmfOutImagBuffer[ mappingMatrix[uch] ][n] = decoder->tmpInImagQmfBuffer[uch][n];

				    decoder->tmpInRealQmfBuffer[uch][n] = decoder->qmfInRealBuffer[uch][decoder->frameSize - decoder->moduleDelay + n];
				    decoder->tmpInImagQmfBuffer[uch][n] = decoder->qmfInImagBuffer[uch][decoder->frameSize - decoder->moduleDelay + n];
			    }
          for (n = decoder->moduleDelay; n < decoder->frameSize ; n++) 
				  {
					  decoder->qmfOutRealBuffer[ mappingMatrix[uch] ][n] = decoder->qmfInRealBuffer[uch][n - decoder->moduleDelay];
            decoder->qmfOutImagBuffer[ mappingMatrix[uch] ][n] = decoder->qmfInImagBuffer[uch][n - decoder->moduleDelay];
				  }
		    }
      }
    }
    else
    {
		  if(!isInQmf)
		  {
			  /*  Add delay for bypassing the LFEs channels */
			  for (ch = 0; ch < decoder->numInLFEs; ch++) 
			  {
				  for (n = 0; n < decoder->moduleDelay; n++) 
				  {
					  decoder->tmpInBuffer[ch][n] = decoder->tmpInBuffer[ch][decoder->frameSize + n];
				  }
				  for (j = 0; j < nSamplesReadPerChannel ; j++) 
				  {
					  decoder->tmpInBuffer[ch][decoder->moduleDelay + j] = decoder->inBuffer[ch][j];
				  }
			  }
		  } 
      else 
      {     
        /*  Add delay for bypassing the LFEs channels */
		    for (ch = 0; ch < decoder->numInLFEs; ch++) 
		    {
			    for (n = 0; n < decoder->moduleDelay; n++) 
			    {
				    decoder->tmpInRealQmfBuffer[ch][n] = decoder->tmpInRealQmfBuffer[ch][decoder->frameSize + n];
				    decoder->tmpInImagQmfBuffer[ch][n] = decoder->tmpInImagQmfBuffer[ch][decoder->frameSize + n];
			    }
			    for (j = 0; j < nSamplesReadPerChannel ; j++) 
			    {
				    decoder->tmpInRealQmfBuffer[ch][decoder->moduleDelay + j] = decoder->qmfInRealBuffer[ch][j];
				    decoder->tmpInImagQmfBuffer[ch][decoder->moduleDelay + j] = decoder->qmfInImagBuffer[ch][j];
			    }
		    } 
		  }	/* end of if(!isInQmf) { */     

		  if (!use_ren_mat_files && decoder->nSaocObjects > 0)
		  { 
			  for(ch=0;ch<SAOC_MAX_RENDER_CHANNELS;ch++)
			  {
				  for(j=0;j<SAOC_MAX_OBJECTS;j++)
				  {
					  frameRenderMat[0][ch][j] = frameRenderMat[numOamPerFrame-1][ch][j];
					  framePreMixMat[0][ch][j] = framePreMixMat[numOamPerFrame-1][ch][j];
				  }
			  }
			  framePossOAM[0] = framePossOAM[numOamPerFrame-1];
			  numOamPerFrame = 1;

			  while ( oam_sample_pos < currentParameterPosition )
			  {
				  current_blocksize = saoc_get_oam_sample(fOAMFile, currentPosition, oamSample, oamVersion, hasDynamicObjectPrio, hasUniformSpread);

				  errorMessage = gVBAPRenderer_GetGains(hgVBAPRenderer, currentPosition, gainMatrix, hasUniformSpread);
				  if (errorMessage ) 
				  {
					  myexit("\n Error during VBAP gains computation.\n");
				  }

				  numOamPerFrame++;
				  framePossOAM[numOamPerFrame-1] = *oamSample->sample / 64;

				  for(ch=0;ch<nChannelsOut; ch++)
				  {
					  for(j=0;j<numObjects;j++)
					  {
						  frameRenderMat[numOamPerFrame-1][ch][j] = gainMatrix[j][saocOutputCh_Mapping[ch]];
					  }
				  }

				  if (decoder->nNumPreMixedChannels)
				  {
					  errorMessage = gVBAPRenderer_GetGains(hgVBAPRenderer_PreMix, currentPosition, gainPreMixMatrix, hasUniformSpread);

					  if (errorMessage ) 
					  {
						  myexit("\n Error during VBAP gains computation.\n");
					  }

					  for(ch=0;ch<decoder->nNumPreMixedChannels; ch++) 
					  {
						  for(j=0;j<numObjects;j++)
						  {
							  framePreMixMat[numOamPerFrame-1][ch][j] = gainPreMixMatrix[j][saocReferenceCh_Mapping[ch]];
						  }
					  }
				  }  

				  oam_sample_pos = *oamSample->sample; 

				  if (current_blocksize == 0 && frameNo > 1)
				  {
					  break;
				  }
			  }
		  }

      saoc_DecoderApply( decoder,           
			  DecodingEnd,
			  numOamPerFrame,
			  framePossOAM,
			  frameRenderMat,
			  framePreMixMat,
			  coder_offset,
			  use_ren_mat_files,
			  pRenderMats,
			  time_instances,
			  num_time_instances,
			  pRenderMat_ch,
			  inversionParam,
			  saocInputCh_Mapping,
			  saocOutputCh_Mapping,
			  drcFlag,
			  drcMatrix,
			  &frameLength);

		  coder_offset += frameLength;
		  currentParameterPosition += decoder->frameSize;

		  /*=================== Process LFE channels: ================================ */
		  if (drcFlag == 1 && decoder->numInLFEs > 0)
		  {
			  saoc_DrcLfeInterfaceApply( decoder, 
				  drcLfe,
				  saocInputLFE_Mapping,
				  saocOutputLFE_Mapping,
				  pRenderMat_lfe, 
				  drcMatrix
				  );
		  } 
		  else
		  {
			  if(!decoder->isOutQmf)
			  {
          for (ch = 0; ch < decoder->numOutLFEs; ch++) 
					{
            memset(decoder->outBuffer[ saocOutputLFE_Mapping[ch] ], 0, decoder->frameSize * sizeof(decoder->outBuffer[ saocOutputLFE_Mapping[ch] ][0]));
            for (i = 0; i < decoder->numInLFEs; i++)  
				    {
						  for (n= 0; n < decoder->frameSize; n++)
						  {
							  decoder->outBuffer[ saocOutputLFE_Mapping[ch] ][n] += pRenderMat_lfe[ch][i] * decoder->tmpInBuffer[i][n];
						  }
					  }
				  }
			  } 
			  else 
			  {
          for (ch = 0; ch < decoder->numOutLFEs; ch++) 
					{
            memset(decoder->qmfOutRealBuffer[ saocOutputLFE_Mapping[ch] ], 0, decoder->frameSize * sizeof(decoder->qmfOutRealBuffer[ saocOutputLFE_Mapping[ch] ][0]));
            memset(decoder->qmfOutImagBuffer[ saocOutputLFE_Mapping[ch] ], 0, decoder->frameSize * sizeof(decoder->qmfOutImagBuffer[ saocOutputLFE_Mapping[ch] ][0]));
            for (i = 0; i < decoder->numInLFEs; i++)  
				    {
						  for (n= 0; n < decoder->frameSize; n++) 
						  {
							  decoder->qmfOutRealBuffer[ saocOutputLFE_Mapping[ch] ][n] += pRenderMat_lfe[ch][i] * decoder->tmpInRealQmfBuffer[i][n];
							  decoder->qmfOutImagBuffer[ saocOutputLFE_Mapping[ch] ][n] += pRenderMat_lfe[ch][i] * decoder->tmpInImagQmfBuffer[i][n];
						  }
					  }
				  }
			  }
		  }
		  /* =================================================== */

		  if (isLastFrame && DecodingEnd != SAOC_END_DECODING && nSamplesReadPerChannel + flushSaocDecoder < decoder->frameSize) 
		  {
			  nSamplesToWritePerChannel += flushSaocDecoder;
			  DecodingEnd = SAOC_END_DECODING;
		  }
    }
      
    /* write frame */
	  if(isOutQmf)
	  {
		  error_write = wavIO_writeFrame(hWavIO_real, decoder->qmfOutRealBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
		  error_write = wavIO_writeFrame(hWavIO_imag, decoder->qmfOutImagBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
	  } 
	  else 
	  {
		  error_write = wavIO_writeFrame(hWavIO,decoder->outBuffer, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
	  }
    nTotalSamplesReadPerChannel += nSamplesWrittenPerChannel;

	}

	if (DecodingEnd == SAOC_END_DECODING)
	{
		myexit("saocDecoder.c: End of WAV file");
	}

	/* ========================= end FRAME BASED PROCESSING ========================== */
	return 0;
}

int saoc_parseCmdl(int argc,
              char *** pargv,
              int *outCICPIndex,
              int *refCICPIndex,
              int *dmxCICPIndex,
              char** inFilenameOutputChannelList,
              char** inFilenameReferenceChannelList,
              char** inFilenameDmxChannelList,
              char** indmxFilename,
              char** inbsFilename,
              char** inrmFilename,
              char** outFilename,
              char** oamFilename,
              char** inDrcFilename,
              int *drcFlag,
              char qmfInFileReal[200],
              char qmfInFileImag[200],
              char qmfOutFileReal[200],
              char qmfOutFileImag[200],
              int *coreOutputFrameLength,
              int *isInQmf,
              int *isOutQmf,
              int *sacHeaderLen)
{
	int i, nWavExtensionChars = 0;
	char ** argv = *pargv;
	char* wavExtension  = ".wav";

	*drcFlag = 0;

	if(argc < 4)
	{
		saoc_printHelp();
		return(-1);
	}

	for(i = 1; i < argc; i++)
	{
		char *arg = argv[i];

		if(!strcmp(arg, "-cicpOut")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -cicpOut\n\n");
				return -1;
			}
			*outCICPIndex = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp(arg,"-outGeo")) { /* needed if cicpOut==-1 */           
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -outGeo\n\n");
				return -1;
			}
			*inFilenameOutputChannelList = argv[i+1];                              
			i++;
		}

		else if(!strcmp(arg, "-refLayout")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for refLayout\n\n");
				return -1;
			}
			*refCICPIndex = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp(arg,"-refGeo")) { /* needed if refLayout==-1 */           
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -outGeo\n\n");
				return -1;
			}
			*inFilenameReferenceChannelList = argv[i+1];                              
			i++;
		}

		else if(!strcmp(arg, "-dmxLayout")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for dmxLayout\n\n");
				return -1;
			}
			*dmxCICPIndex = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp(arg,"-dmxGeo")) { /* needed if refLayout==-1 */           
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -dmxGeo\n\n");
				return -1;
			}
			*inFilenameDmxChannelList = argv[i+1];                              
			i++;
		}

		else if(!strcmp(arg, "-if"))  {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -in\n\n");
				return -1;
			}
			*indmxFilename = argv[i+1];
			nWavExtensionChars = strspn(wavExtension,*indmxFilename);
			if ( nWavExtensionChars != 4 ) {
				*isInQmf = 1;	
				strcpy(qmfInFileReal, *indmxFilename);
				strcpy(qmfInFileImag, *indmxFilename);
				strcat(qmfInFileReal, "_real.qmf");
				strcat(qmfInFileImag, "_imag.qmf");
			}
			i++;
		} 
		else if(!strcmp(arg, "-bs")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -bs\n\n");
				return -1;
			}
			*inbsFilename = argv[i+1];
			i++;
		} 
		else if(!strcmp(arg, "-oam")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -oam\n\n");
				return -1;
			}
			*oamFilename = argv[i+1];
			i++;
		} 
		else if(!strcmp(arg, "-coreOutputFrameLength")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for refLayout\n\n");
				return -1;
			}
			*coreOutputFrameLength = atoi(argv[i+1]);
			i++;
		}
		else if(!strcmp(arg, "-ren")) { /* Required if Channel Path is active */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -ren\n\n");
				return -1;
			}
			*inrmFilename = argv[i+1];
			i++;
		} 
		else if(!strcmp(arg, "-of")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -out\n\n");
				return -1;
			}
			*outFilename = argv[i+1];
			nWavExtensionChars = strspn(wavExtension,*outFilename);
			if ( nWavExtensionChars != 4 ) {
				*isOutQmf = 1;	
				strcpy(qmfOutFileReal, *outFilename);
				strcpy(qmfOutFileImag, *outFilename);
				strcat(qmfOutFileReal, "_real.qmf");
				strcat(qmfOutFileImag, "_imag.qmf");
			}
			i++;


		} else if(!strcmp(arg, "-configLen")) {  /* Required */
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -ld\n\n");
				return -1;
			}
			*sacHeaderLen = atoi(argv[i+1]);
			i++;

		}
		else if(!strcmp(arg, "-drc")) {
			if(i == argc-1) {
				fprintf(stderr, "\nArgument expected for -drc\n\n");
				return -1;
			}
			*drcFlag = 1;
			*inDrcFilename = argv[i+1];
			i++;

		} else {
			fprintf(stderr, "\nWarning: unknown command line switch \"%s\"\n\n", arg);
		}
	}

	if ( (*outCICPIndex == -1) && (*inFilenameOutputChannelList == NULL)) {
		fprintf(stderr,"Error: Missing filename for output setup geometry file. Use -outGeo <file.txt> switch.\n");
		return -1;
	}
	if ( (*refCICPIndex == -1) && (*inFilenameReferenceChannelList == NULL)) {
		fprintf(stderr,"Error: Missing filename for reference layout geometry file. Use -refGeo <file.txt> switch.\n");
		return -1;
	}

	return 0;
}


static int saocGetGeometryInformation(int CICPIndex, char* FilenameChannelList,int *numChannels, int *numLFEs, CICP2GEOMETRY_CHANNEL_GEOMETRY* GeometryInfo) {

	int errorFlag = 0;
	/* Get Azimuth, Elevations from CICP */
	if ( CICPIndex == -1) /* Read configuration from txt file */
	{
		errorFlag = cicp2geometry_get_geometry_from_file(FilenameChannelList, GeometryInfo, numChannels, numLFEs);
	}
	else /* Read configuration from CICP setup index */
	{ 
		errorFlag = cicp2geometry_get_geometry_from_cicp(CICPIndex, GeometryInfo, numChannels, numLFEs); 
	}

	if ( 0 != errorFlag ) 
	{
		fprintf(stderr, "Error during getting output setup geometry.\n");
		return -1;
	}    
	return 0;
}

static int saoc_get_oam_sample( FILE* oam_file,
                                OAM_SAMPLE* currentPosition, 
                                StructOamMultidata* oam_sample,
                                uint16_t oam_version,
                                uint16_t hasDynamicObjectPriority,
                                uint16_t hasUniformSpread)
{
	uint64_t sample_pos;
	int num_objects = oam_sample->size1;
	int n;

	DYNAMIC_OBJECT_PRIO hasDynObjPrio = DYN_OBJ_PRIO_NOT_NEEDED;
	if ( hasDynamicObjectPriority ) 
	{
		hasDynObjPrio = DYN_OBJ_PRIO_AVAILABLE;
	}
	else
	{
		hasDynObjPrio = DYN_OBJ_PRIO_NOT_NEEDED;
	}

	sample_pos = oam_sample->sample[0];

	oam_read_process(oam_file, oam_sample, &num_objects, oam_version, hasDynObjPrio, hasUniformSpread);

	for (n = 0; n < num_objects; n++)
	{
		currentPosition[n].azimuth   = oam_sample->azimuth[n]   * (float)(M_PI / 180.0);
		currentPosition[n].elevation = oam_sample->elevation[n] * (float)(M_PI / 180.0);
		currentPosition[n].radius    = oam_sample->radius[n];
		currentPosition[n].gain      = oam_sample->gain[n];
	}

	return (int)(oam_sample->sample[0] - sample_pos);
}

static void SAOC_get_channel_mapping(CICP2GEOMETRY_CHANNEL_GEOMETRY* outGeometryInfo,
                              int saocChannelMappingInfo[SAOC_MAX_RENDER_CHANNELS],
                              int saocLfeMappingInfo[SAOC_MAX_LFE_CHANNELS],
                              int nChannelsOut,
                              int numOutLFEs)
{
	int i, idxCh, idxLFE;

	idxCh = idxLFE = 0;
	for (i = 0; i < nChannelsOut+numOutLFEs; i++)
	{
		if (outGeometryInfo[i].loudspeakerType  == CICP2GEOMETRY_LOUDSPEAKER_KNOWN)
		{
			if (outGeometryInfo[i].LFE ==0)
			{
				saocChannelMappingInfo[idxCh] = i;
				idxCh++;
			} 
			else
			{
				saocLfeMappingInfo[idxLFE] = i;
				idxLFE++;
			}
		}
	}
}

int saoc_compare_geometry(CICP2GEOMETRY_CHANNEL_GEOMETRY geoOne[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                          unsigned int numChannelsOne,
                          CICP2GEOMETRY_CHANNEL_GEOMETRY geoTwo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                          unsigned int numChannelsTwo,
                          unsigned int tolerance,
                          int mappingMatrix[SAOC_MAX_RENDER_CHANNELS])
{
  unsigned int i, j, counter = -1;

  if ( numChannelsOne != numChannelsTwo )
    return -1;

  for(i = 0; i < numChannelsOne; i++)
  {
    for(j = 0; j < numChannelsTwo; j++)
    {
      if (( abs(geoOne[i].Az - geoTwo[j].Az ) <= (int)tolerance ) && ( abs(geoOne[i].El - geoTwo[j].El ) <= (int)tolerance ) && ( geoOne[i].LFE == geoTwo[j].LFE ) && ( geoOne[i].screenRelative == geoTwo[j].screenRelative ) )
      {
        counter++;
        mappingMatrix[i] = j;
        break;
      }
    }
    if( counter != i)
    {
      return -1;
    }
  }

  return 1;
}


void saoc_printHelp( void ) {
	fprintf(stdout, "\nCommand line switches:\n\n");
	fprintf(stdout, " -if         <input.wav> or <input> \n the latter will be extended to <input>_real.qmf and <input>_imag.qmf\n");
	fprintf(stdout, " -of         <output.wav> or <output> \n the latter will be extended to <output>_real.qmf and <output>_imag.qmf\n");
	fprintf(stdout, " -ren        <filename> Rendering Matrix for channel based content/static objects \n");
	fprintf(stdout, " -bs         <filename> \n");
	fprintf(stdout, " -oam        <filename> \n");
	fprintf(stdout, " -configLen  <int>     SAOC Header Length in bytes\n");
	fprintf(stdout, " -coreOutputFrameLength  <int> core coder output frame length ( number of time slots ) \n");
	fprintf(stderr, " -cicpOut   <outputConfigIdx>   output config idx\n");
	fprintf(stderr, "      ( -1: LIST_OF_CHANNELS, 0: GENERIC, >0: CICP_index )\n");
	fprintf(stderr, "  LIST_OF_CHANNELS signaling of setups will read from file (use -outGeo <outConf.txt>).\n");
	fprintf(stderr, " -refLayout   <referenceConfigIdx>   reference config idx\n");
	fprintf(stderr, "      ( -1: LIST_OF_CHANNELS, 0: GENERIC, >0: CICP_index )\n");
	fprintf(stderr, "  LIST_OF_CHANNELS signaling of setups will read from file (use -refGeo <refConf.txt>).\n");
	fprintf(stdout, " -drc        <filename> DRC matrix \n");
}

void saoc_printDisclaimer( void ) {
	fprintf(stdout, "\n\n");
	fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*                           SAOC 3D Decoder Module                            *\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*                                %s                                  *\n", __DATE__);
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
	fprintf(stderr, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
	fprintf(stderr, "*                                                                             *\n");
	fprintf(stderr, "*******************************************************************************\n\n");
}


