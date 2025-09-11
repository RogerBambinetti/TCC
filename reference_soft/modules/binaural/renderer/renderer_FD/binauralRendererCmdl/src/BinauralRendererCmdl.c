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
#include <time.h>

#include "bitstreamBinaural.h" 
#include "Binauralizer.h"     
#include "BinauralProfileLevelConstraints.h"

#include "qmflib.h"
#include "ReverbGenerator.h"
#include "fftlib.h"
#include "wavIO.h"

/* Define statement for bug fixing */

#define BUGFIX_KCONV_KMAX_15JUNE

#define BUGFIX_BINAURAL_RENDERER

#ifdef BUGFIX_BINAURAL_RENDERER
#define BUGFIX_BINAURAL_INSERTION
#define BUGFIX_TDL_DELAY
#endif

#define BUGFIX_FRAMESIZE_1024

#define QMFLIB_MAX_CHANNELS 32
#ifdef _WIN32
#define FILE_SEPARATOR   '\\'
#else
#define FILE_SEPARATOR   '/'
#endif

void FdBinauralRendererParam2BRIRmetadata(BinauralRepresentation *pBinauralRepresentation, H_BRIRMETADATASTRUCT h_BRIRmetadata);

int main(int argc, char *argv[])
{
	/* ********************** DECLARATIONS ******************** */

	/* wavIO */
	WAVIO_HANDLE hWavIO = NULL; 
	WAVIO_HANDLE hWavIO_real = NULL; 
	WAVIO_HANDLE hWavIO_imag = NULL;
	char *wavIO_inpath = NULL;
	char *wavIO_outpath = NULL;

	/* read QMF domain data */
	char wavIO_inpath_real[1000]; 
	char wavIO_inpath_imag[1000];
	char wavIO_outpath_real[1000]; 
	char wavIO_outpath_imag[1000];
	int wavIO_error_init = 0; 
	int wavIO_error_read = 0; 
	int wavIO_error_write = 0; 
	int wavIO_error_close = 0; 

	unsigned long int NumberSamplesWritten = 0; 
	unsigned long int NumberSamplesRead = 0; 
	unsigned int isLastFrame = 0; 
	unsigned int nInChannels = 0; 
	unsigned int InSampleRate = 0; 
	unsigned long nTotalSamplesPerChannel = 0; 
	unsigned long nTotalSamplesWrittenPerChannel = 0; 
	unsigned int InBytedepth = 0; 
	int nSamplesPerChannelFilled = 0; 
	int bytedepth = 0; 
	int fs_out = 0; 
	int addSamples_written = 0;
	char* wavExtension  = ".wav";

	/* files */
	FILE *fBRIRmetadata = NULL; 
	FILE *fwavIn = NULL;  
	FILE *fwavOut = NULL; 

	/* read QMF domain data */
	FILE *fwavIn_real = NULL;
	FILE *fwavIn_imag = NULL;
	FILE *fwavOut_real = NULL;
	FILE *fwavOut_imag = NULL;

	/* buffer for reading audio */
	float **inBuffer = 0; 
	float **outBuffer = 0; 

	/* qmf analysis and synthesis */
	float ***input_audio_QMF_real  = NULL; 
	float ***input_audio_QMF_imag  = NULL; 
	float ***output_DE_QMF_real = NULL; 
	float ***output_DE_QMF_imag = NULL; 
	float ***output_dmx_QMF_real = NULL; 
	float ***output_dmx_QMF_imag = NULL; 
	float ***output_DE_LR_QMF_real = NULL; 
	float ***output_DE_LR_QMF_imag = NULL; 

	QMFLIB_POLYPHASE_ANA_FILTERBANK **pAnalysisQmf = NULL;  
	QMFLIB_POLYPHASE_SYN_FILTERBANK **pSynthesisQmf = NULL; 

	/* BRIR variables */
	char *BRIR_metadata_inpath = NULL;

	BRIRMETADATASTRUCT BRIRmetadata; 
	H_BRIRMETADATASTRUCT h_BRIRmetadata = &BRIRmetadata; 

	int *m_conv = NULL; 
	char *geoInpath = NULL;
	int nthRepresentation = 1;

	/* Audio variables */
	float *Audio_azimuth = NULL;
	float *Audio_elevation = NULL;
	int *LFE_channels = NULL;
	int num_LFE = 0;

	/* ReverbGenerator */
	REVERBSTRUCT reverb_struct;
	H_REVERBSTRUCT h_reverb_struct = &reverb_struct;
	int randSeed = 0;
	float DryWetDefault = 1.0f; 
	int delay_reverberator = 2*QMFLIB_NUMBANDS*10;

	/* FFT of BRIRs */
	HANDLE_FFTLIB h_FFT = NULL; 
	int *filterlength_fft = 0; 

	/* general parameters */
	int readQMF = 0, writeQMF = 0;
	int CICPConfIdx = -2; 
	int nWavExtensionChars = 0;
	int date_dependent_outputpath = 0;
	int read_matlab_metadata = 0;

	int f_max = 18000;
	int TDL = 1; 

	int binaural_numTimeslots = 0;
	int binaural_hopsize = QMFLIB_NUMBANDS; 
	int processingDelay = 0; 
	int additionalDelay = 0;
	int correctDelay = 0;
	int numband_conv = 0; 
	int numband_proc = 0;
	int numband_LRanalysis = 0;
	int k = 0, band = 0; 
	int error_brir = 0, error_brirqmf = 0; 
	int frameNo = -1; 
	unsigned int nInChannelsTest = 0; 
	float f_center_max = 0.0f;

	int do_compensate_delay = 1;
	int delay_to_compensate = 0;

	void *reverb = 0; 
	int *startReverb = NULL;
	int *startReverbQMF = NULL; 
	int max_negative = 0;
	int add_DE_delay = 0;
	int max_filterlength_fft = 0;
	int max_startReverbQMF = 0;
	int *DE_filter_length = NULL;
	int total_max_fft = 128;
	int max_numblocks = 1; 
	int max_numframes = 1;
	int max_filterlength = 0;

	float ****input_audio_FFT_real = NULL;
	float ****input_audio_FFT_imag = NULL;

	int chOut = 0, chIn = 0; 
	int idxRot = 0;
	int addSamples = 0;
	int addSamplesOrig = 0;
	int skipLastSamples = 0;

	float ***overlap_real = NULL;
	float ***overlap_imag = NULL;
	float ***temp_output_QMF_real = NULL;
	float ***temp_output_QMF_imag = NULL;

	float **dmxmtx = NULL; 
	int no_dmxchannels_left;
	int no_dmxchannels_right;

	int BRIRlength = 0;
	int just_DE = -1;
	int f_center = 0;
	int f_center_48 = 0;

	int ct = 0;
	int binaural_blocklength = 0;

  /* Profile level constraints termination */
  int profile = -1;
  int level = -1;

	/* variables for display progress */
	float percentage = 0.0f; 
	char  cDisplay[] = {'|', '/', '-', '\\', '|', '/', '-', '\\'}; 
	int testcondition = 0;
	int process = 0;

	/* error handling */
	int handle_error = 0;

	/* *************************** DE Buffer *************************** */
	float ***buf_DE_BLK_left_real = NULL, ***buf_DE_BLK_left_imag = NULL;
	float ***buf_DE_BLK_right_real = NULL, ***buf_DE_BLK_right_imag = NULL;
	float **buf_DE_FRM_left_real = NULL, **buf_DE_FRM_left_imag = NULL, **buf_DE_FRM_right_real = NULL, **buf_DE_FRM_right_imag = NULL;

	/* *************************** TDL Buffer ************************** */
	float ***buf_TDL_left_real = NULL, ***buf_TDL_left_imag = NULL;
	float ***buf_TDL_right_real = NULL, ***buf_TDL_right_imag = NULL;

	BinauralRendering *pBinauralRendering; 
	BinauralRepresentation *pBinauralRepresentation;
	FdBinauralRendererParam *pFdBinauralRendererParam; 

	/* **************************** INITIALIZATION ******************** */
	h_reverb_struct->buffer_rev_im = NULL;
	h_reverb_struct->buffer_rev_re = NULL;
	h_reverb_struct->scale = NULL;
	h_reverb_struct->scale_old = NULL;
	h_reverb_struct->startReverbQMF = NULL;

	h_BRIRmetadata->LateReverb_fc = NULL;
	h_BRIRmetadata->LateReverb_RT60 = NULL;
	h_BRIRmetadata->LateReverb_energy = NULL;
	h_BRIRmetadata->transition = NULL;
	h_BRIRmetadata->azimuth = NULL;
	h_BRIRmetadata->elevation = NULL;

	BRIR_metadata_inpath = NULL;

	fBRIRmetadata = NULL; 



	fprintf(stderr, "\n\n"); 

	fprintf(stdout, "\n");
	fprintf(stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
	fprintf(stdout, "*                                                                             *\n");
	fprintf(stdout, "*                         Binaural Renderer Module                            *\n");
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
		if (!strcmp(argv[k], "-if"))     /* Required */
		{
			int nWavExtensionChars = 0;
			wavIO_inpath = argv[k+1];
			nWavExtensionChars = (int)strspn(wavExtension,wavIO_inpath);
			if ( nWavExtensionChars != 4 ) 
			{
				readQMF = 1;
				strcpy(wavIO_inpath_real, wavIO_inpath);
				strcpy(wavIO_inpath_imag, wavIO_inpath);
				strcat(wavIO_inpath_real, "_real.qmf");
				strcat(wavIO_inpath_imag, "_imag.qmf"); 
			}
			k++; 
		}
		/* output path */
		else if (!strcmp(argv[k], "-of")) /* Required */
		{
			int nWavExtensionChars = 0;
			wavIO_outpath = argv[k+1];
			nWavExtensionChars = (int)strspn(wavExtension,wavIO_outpath);
			if ( nWavExtensionChars != 4 ) 
			{
				writeQMF = 1;
				strcpy(wavIO_outpath_real, wavIO_outpath);
				strcpy(wavIO_outpath_imag, wavIO_outpath);
				strcat(wavIO_outpath_real, "_real.qmf");
				strcat(wavIO_outpath_imag, "_imag.qmf"); 
			}
			k++; 
		} 
		else if (!strcmp(argv[k], "-fdbs")) /* Required */
		{
			BRIR_metadata_inpath = argv[k+1];
			k++; 
		}
		/* channel config */
		else if (!strcmp(argv[k], "-cicpIn")) /* Required */
		{
			CICPConfIdx = atoi(argv[k+1]); 
			k++;
		}
		/* TDL on or off (standard is on) */
		else if (!strcmp(argv[k], "-mode")) /* Optional */
		{
			TDL = atoi(argv[k+1]); 
			k++;
		}
		/* frame length for binauralization (standard is 4096 samples) */
		else if (!strcmp(argv[k], "-block")) /* Optional */
		{
			binaural_blocklength = atoi(argv[k+1]); 
			k++;
		}
		/* path to geometric information file if -cicpIn == -1 */
		else if (!strcmp(argv[k], "-inGeo")) /* Optional */
		{
			geoInpath = argv[k+1]; 
			k++;
		}
		/* the -nbs binaural representation in the bitstream can be used. */
		else if (!strcmp(argv[k], "-nbs")) /* Optional */
		{
			nthRepresentation = atoi(argv[k+1]); 
			k++;
		}
#ifdef BUGFIX_KCONV_KMAX_15JUNE
		/* the -nbs binaural representation in the bitstream can be used. */
		else if (!strcmp(argv[k], "-kconv")) /* Optional */
		{
			numband_conv = atoi(argv[k+1]); 
			k++;
		}
		else if (!strcmp(argv[k], "-kmax")) /* Optional */
		{
			numband_proc = atoi(argv[k+1]); 
			k++;
		}
#endif
    else if (!strcmp(argv[k], "-profile")) /* Optional */
		{
			profile = atoi(argv[k+1]); 
			k++;
		}
    else if (!strcmp(argv[k], "-level")) /* Optional */
		{
			level = atoi(argv[k+1]); 
			k++;
		}
    else if (!strcmp(argv[k], "-d")) /* Optional */
    {
      do_compensate_delay = atoi(argv[k+1]);;
      k++;
    }
	}

	if ((wavIO_inpath == NULL)|| (wavIO_outpath == NULL))
	{
		fprintf(stderr, "Invalid arguments, usage:\n"); 
		fprintf(stderr, "-if <input.wav>  path to input wave file or <input> qmf input base filename\n"); 
		fprintf(stderr, "-of <output.wav> path to output wave file or <output> qmf output base filename\n"); 
		fprintf(stderr, "-fdbs <bitstream.bs> path to FD binaural bitsream\n"); 
		fprintf(stderr, "-cicpIn <CICPinputIdx> CICP index of input layout:\n 5.1: 6\n 22.2: 13\n arbitrary (read from file): -1\n"); 
		fprintf(stderr, "-inGeo <file.txt> path to file for signaling an arbitrary input setup (just needed if cicpIn == -1)\n");
		fprintf(stderr, "(optional) -nbs <integer> the -nbs th binaural representation in the bitstream is used to render. (default = 1).\n"); 
		fprintf(stderr, "(optional) -mode <0/1> TDL processing is switched off/on.\n"); 
		fprintf(stderr, "(optional) -block framelength (samples) for binaural rendering. A standard framelength of 4096 is assumed if -block is not given.\n");
#ifdef BUGFIX_KCONV_KMAX_15JUNE
		fprintf(stderr, "(optional) -kconv <integer> the number of bands used for convolution. A default value of kconv is 32 if -kconv is not given.\n"); 
		fprintf(stderr, "(optional) -kmax  <integer> the number of processed bands via binaural renderer. A default value of kconv is 48 if -kmax is not given.\n"); 
#endif
    fprintf(stderr, "(optional) -d  <integer> (0, 1, 2) choose delay compensation mode (0 = no compensation, 1 = full compensation (default), 2 = 3DAudioDecoder delay mode).\n"); 
		handle_error = -1; 
		goto free_memory;
	}

	if (binaural_blocklength == 0)
	{
		binaural_blocklength = 4096;
	}
	binaural_numTimeslots =  binaural_blocklength / QMFLIB_NUMBANDS; 


	if ((CICPConfIdx == -1) && (geoInpath == NULL))
	{
		fprintf(stderr, "Error: Arbitrary setup defined, but no path to file with geometric information given.\n"); 
		handle_error = -1;
		goto free_memory;
	}
	if (CICPConfIdx != -1)
	{
		geoInpath = NULL;
	}


	/* *********************************** OPEN FILES ************************************* */

	if (readQMF)
	{
		fwavIn_real = fopen(wavIO_inpath_real, "rb"); 
		fwavIn_imag = fopen(wavIO_inpath_imag, "rb"); 
		if ((fwavIn_real != NULL) && (fwavIn_imag != NULL))
		{
			fprintf(stderr, "Found input qmf base filename: \n %s.\n", wavIO_inpath); 
		}
		else
		{
			fprintf(stderr, "Error: Invalid input qmf base filename: \n %s.\n", wavIO_inpath); 
			handle_error =  -2; 
			goto free_memory;
		}
	}
	else
	{
		fwavIn = fopen(wavIO_inpath, "rb"); 
		if (fwavIn != NULL)
		{
			fprintf(stderr, "Found input wave file: \n %s.\n", wavIO_inpath); 
		}
		else
		{
			fprintf(stderr, "Invalid input file: \n %s.\n", wavIO_inpath); 
			handle_error = -2;       
			goto free_memory;
		}
	}

	if (writeQMF)
	{
		fwavOut_real = fopen(wavIO_outpath_real, "wb"); 
		fwavOut_imag = fopen(wavIO_outpath_imag, "wb"); 
		if ((fwavOut_real != NULL) && (fwavOut_imag != NULL))
		{
			fprintf(stderr, "Write to output qmf base filename: \n %s.\n", wavIO_outpath); 
		}
		else
		{
			fprintf(stderr, "Invalid output qmf base filename: \n %s.\n", wavIO_outpath); 
			handle_error = -3; 
			goto free_memory;
		}
	}
	else
	{
		fwavOut = fopen(wavIO_outpath, "wb"); 
		if (fwavOut != NULL)
		{
			fprintf(stderr, "Write to output wave file: \n %s.\n", wavIO_outpath); 
		}
		else
		{
			fprintf(stderr, "Invalid output file: \n %s.\n", wavIO_outpath); 
			handle_error = -3; 
			goto free_memory;
		}
	}

	/* *********************************** INIT WAV IO ************************************* */

	/* init delay numbers */
	if (!readQMF && !writeQMF)
	{
		processingDelay = 320+257+33;
		addSamples = 0;
	}
	if (readQMF && writeQMF)
	{
		processingDelay = 0;
		addSamples = 33;
	}
	if (readQMF && !writeQMF)
	{
		processingDelay = 320+257+33;
		addSamples = 0;
	}
	if (!readQMF && writeQMF)
	{
		processingDelay = 0;
		addSamples = 320+33;
	}

	if (do_compensate_delay == 1)
	{
		delay_to_compensate = processingDelay;
	}
  else if (do_compensate_delay == 2)
	{ 
		delay_to_compensate = 33;
	}
  else
  {
    delay_to_compensate = 0;
  }

	/* read AND write QMF files */
	if (readQMF  && writeQMF)
	{
		wavIO_error_init = wavIO_init(&hWavIO_real, binaural_blocklength, 0, -1*delay_to_compensate); 
		wavIO_error_init = wavIO_init(&hWavIO_imag, binaural_blocklength, 0, -1*delay_to_compensate); 
	}
	/* read OR write QMF files */
	else if (readQMF || writeQMF)
	{
		wavIO_error_init = wavIO_init(&hWavIO_real, binaural_blocklength, 0, -1*delay_to_compensate); 
		wavIO_error_init = wavIO_init(&hWavIO_imag, binaural_blocklength, 0, -1*delay_to_compensate); 
		wavIO_error_init = wavIO_init(&hWavIO, binaural_blocklength, 0, -1*delay_to_compensate); 
	}

	if (readQMF)
	{
		if ((fwavIn_real) && (fwavIn_imag))
		{
			wavIO_error_init = wavIO_openRead(hWavIO_real, fwavIn_real, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled); 
			wavIO_error_init = wavIO_openRead(hWavIO_imag, fwavIn_imag, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled); 
		}
	}

	/* read AND write wav files */
	else
	{
		wavIO_error_init = wavIO_init(&hWavIO, binaural_blocklength, 0, -1*delay_to_compensate); 
		if (fwavIn)
		{
			wavIO_error_init = wavIO_openRead(hWavIO, fwavIn, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled); 
		}
	}
	fprintf(stderr, "\n");

	fs_out = InSampleRate; 

	if (writeQMF)
	{
		/* QMF files are always written with 32 bit per sample */
		if (fwavOut_real && fwavOut_imag  && (bytedepth == 0 ))
		{
			fprintf(stderr, "> Writing QMF data files with 32 bit.\n"); 
			bytedepth = 4; 
		}
		if (fwavOut_real && fwavOut_imag  && (CICPConfIdx == -2))
		{
			fprintf(stderr, "> No channel configuration specified! Assuming 22.2 channel configuration.\n"); 
		}
		else if (fwavOut_real && fwavOut_imag)
		{
			fprintf(stderr, "> Channel configuration index %i detected.\n\n", CICPConfIdx); 
		}

		wavIO_error_init = wavIO_openWrite(hWavIO_real, fwavOut_real, BINAURAL_NUM_OUTCHANNELS, fs_out, bytedepth); 
		wavIO_error_init = wavIO_openWrite(hWavIO_imag, fwavOut_imag, BINAURAL_NUM_OUTCHANNELS, fs_out, bytedepth); 
		if (0 != wavIO_error_init )
		{
			fprintf(stderr, "Error during wavIO initialization.\n"); 
			handle_error = -6; 
			goto free_memory;
		}
	}
	else
	{
		if (fwavOut && (bytedepth == 0 ))
		{
			if (readQMF)
			{
				/* wav files are written with 24 bit per sample when QMF files are read */
				fprintf(stderr, "> Writing time domain wave files with 24 bit.\n"); 
				bytedepth = 3; 
			}
			else
			{
				fprintf(stderr, "> No bit depth specified! Using input format (%i bits).\n", InBytedepth*8); 
				bytedepth = InBytedepth; 
			}
		}
		if (fwavOut && (CICPConfIdx == -2))
		{
			fprintf(stderr, "> No channel configuration specified! Assuming 22.2 channel configuration.\n"); 
			CICPConfIdx = 13; 
		}
		else if ((fwavOut) && (CICPConfIdx > -1))
		{
			fprintf(stderr, "> Channel configuration index %i detected.\n\n", CICPConfIdx); 
		}
		else if ((fwavOut) && (CICPConfIdx == -1))
		{
			fprintf(stderr, "> An arbitrary channel setup is detected.\n\n");
		}

		wavIO_error_init = wavIO_openWrite(hWavIO, fwavOut, BINAURAL_NUM_OUTCHANNELS, fs_out, bytedepth); 
		if (0 != wavIO_error_init )
		{
			fprintf(stderr, "Error: Error during wavIO initialization.\n"); 
			handle_error = -6; 
			goto free_memory;
		}
	}

	/* *********************************** INIT BINAURALIZER ************************************* */

	/* read Position data from channel setup */
	Audio_azimuth = (float*)calloc(nInChannels, sizeof(float)); 
	Audio_elevation = (float*)calloc(nInChannels, sizeof(float)); 
	error_brir = binaural_getChannelPosData(CICPConfIdx, geoInpath, nInChannels, Audio_azimuth, Audio_elevation, &LFE_channels, &num_LFE);

	/*read BRIRmetadata */
	/*binaural_readBRIRmetadata(fBRIRmetadata, h_BRIRmetadata); */

	/* Load binaural bitstream */
	pBinauralRendering = BinauralRendering_initAlloc();
	if (bitstreamRead(BRIR_metadata_inpath, pBinauralRendering) != 0)
	{
		fprintf(stderr, "\nError : can not read %s !\n", BRIR_metadata_inpath);
		/*system("PAUSE");*/
		return -1;
	}

	/* set low-level renderer parameters : pFdBinauralRendererParam */
	pFdBinauralRendererParam = NULL;
	pBinauralRepresentation = pBinauralRendering->ppBinauralRepresentation[nthRepresentation-1];
	if( pBinauralRepresentation == NULL)
	{
		fprintf(stderr, "\nError :%dth binaural representation does not exist!\n", nthRepresentation);
		return -1;
	}
	if (   ( pBinauralRepresentation->binauralDataFormatID == 1 ) ) /* FD low-level */
	{
		pFdBinauralRendererParam = pBinauralRepresentation->pFdBinauralRendererParam;
	}
	else
	{
		fprintf(stderr, "\nError :%dth binaural representation is not frequency-domain low-level param.!\n", nthRepresentation);		
		return -1;
	}
	FdBinauralRendererParam2BRIRmetadata(pBinauralRepresentation, h_BRIRmetadata);

	/* check for different sampling rates */
	if (h_BRIRmetadata->fs_BRIRs != InSampleRate)
	{
		fprintf(stderr, "Note: BRIRs have a different sampling rate than the audio material.\n");
		for (band = 0; band < h_BRIRmetadata->LateReverb_AnalysisBands; band++)
		{
			h_BRIRmetadata->LateReverb_fc[band] = h_BRIRmetadata->LateReverb_fc[band]/(h_BRIRmetadata->fs_BRIRs*1.0f)*(InSampleRate*1.0f);
		}
	}

#ifndef BUGFIX_TDL_DELAY
	for(band = 0; band < h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv; band++)
	{
		int chBrir;
		for(chBrir = 0; chBrir < h_BRIRmetadata->channels; chBrir++)
		{
			h_BRIRmetadata->TDL_delay_left[band][chBrir] += 1;
			h_BRIRmetadata->TDL_delay_right[band][chBrir] += 1;
		}
	}
#endif

	if (h_BRIRmetadata->LateReverb_AnalysisBands == 0)
		just_DE = 1;
	else
		just_DE = 0;

	/* maximum QMF band used for processing */
#ifndef BUGFIX_KCONV_KMAX_15JUNE
	numband_proc = min48, h_BRIRmetadata->noBands_transition);
#else
  numband_proc = min(numband_proc, h_BRIRmetadata->noBands_transition);
  if (numband_proc == 0)
  {
    numband_proc = h_BRIRmetadata->noBands_transition;
  }
#endif

	f_center_max = (float)((numband_proc+0.5f) * ((fs_out*0.5f) / QMFLIB_NUMBANDS));

	/* get number of used LR analysis bands */
	for (band = 0; band < h_BRIRmetadata->LateReverb_AnalysisBands; band++)
	{
		if (h_BRIRmetadata->LateReverb_fc[band] > f_center_max)
		{
			numband_LRanalysis = band;
			break;
		}
	}
	if (numband_LRanalysis == 0)
	{
		numband_LRanalysis = band;
	}

	/* get maximum band for convolution */
	if (TDL == 0)
	{
		numband_conv = numband_proc;
	}
	else
	{
#ifndef BUGFIX_KCONV_KMAX_15JUNE
		numband_conv = min(32,h_BRIRmetadata->numband_conv);
#else
    numband_conv = min(numband_conv, h_BRIRmetadata->numband_conv);
    if (numband_conv == 0)
    {
      numband_conv = h_BRIRmetadata->numband_conv;
    }
#endif
	}

	/* calculate number of subframes */
	h_BRIRmetadata->N_FRM = (int*)calloc(h_BRIRmetadata->noBands_transition, sizeof(int));
	for (band = 0; band < h_BRIRmetadata->noBands_transition; band++)
	{
		h_BRIRmetadata->N_FRM[band] = (int)max(1,binaural_numTimeslots/(h_BRIRmetadata->N_FFT[band]/2));
	}

	/* test if enough transition values are given */
	if (h_BRIRmetadata->noBands_transition < numband_conv)
	{
		/* if not, fill up with transition from highest band */
		int *temp_transition = (int*)calloc(h_BRIRmetadata->noBands_transition,sizeof(int));
		memcpy(temp_transition,h_BRIRmetadata->transition,h_BRIRmetadata->noBands_transition*sizeof(int));

		free(h_BRIRmetadata->transition); h_BRIRmetadata->transition = NULL;
		h_BRIRmetadata->transition = (int*)calloc(numband_conv, sizeof(int));
		memcpy(h_BRIRmetadata->transition,temp_transition,h_BRIRmetadata->noBands_transition*sizeof(int));
		for (k = h_BRIRmetadata->noBands_transition; k < numband_conv; k++)
		{
			h_BRIRmetadata->transition[k] = h_BRIRmetadata->transition[h_BRIRmetadata->noBands_transition-1];
		}
		h_BRIRmetadata->noBands_transition = numband_conv;
		free(temp_transition); temp_transition = NULL;
	}

	startReverb = (int*)calloc(numband_conv,sizeof(int));
	startReverbQMF = (int*)calloc(numband_conv, sizeof(int));
	DE_filter_length = (int*)calloc(numband_conv, sizeof(int));

	for (band = 0; band < numband_conv; band ++)
	{
		/* calculate insertion point for reverberation */
		int temp1 = (int)(ceil(log(h_BRIRmetadata->N_FFT[band]/2)/log(2.0f)));
		DE_filter_length[band] = (int)(pow(2.0f,temp1));
		max_filterlength = max(max_filterlength, DE_filter_length[band]);

		if(!just_DE)
		{
#ifdef BUGFIX_BINAURAL_INSERTION
			startReverb[band] = DE_filter_length[band]*QMFLIB_NUMBANDS + h_BRIRmetadata->initDelay - delay_reverberator + 1; 
#else
			startReverb[band] = DE_filter_length[band]*QMFLIB_NUMBANDS - h_BRIRmetadata->initDelay - delay_reverberator + 1; 
#endif
		}
		else
		{
			startReverb[band] = DE_filter_length[band]*QMFLIB_NUMBANDS - h_BRIRmetadata->initDelay + 1; 
		}
		max_negative = min(max_negative, startReverb[band]);
	}

	for (band = 0; band < numband_conv; band++)
	{
		startReverb[band] = startReverb[band] + abs(max_negative) + 1;
		startReverbQMF[band] = (int)floor((float)startReverb[band]/QMFLIB_NUMBANDS + 0.5)+1;
		max_startReverbQMF = max(max_startReverbQMF, startReverbQMF[band]);
	}
	add_DE_delay = (int)floor(abs(max_negative)/QMFLIB_NUMBANDS+0.5f);

	/* update delay values */
#ifdef REPRODUCE_CE_SUBMISSION_DELAY
	if (abs(max_negative) > 0)
	{ 
		correctDelay = 1;
	}
	else
	{
		correctDelay = 0;
	}

	if (!readQMF && !writeQMF)
	{
		processingDelay += (abs(max_negative) - h_BRIRmetadata->initDelay + correctDelay);
		addSamples = processingDelay;
	}
	if (readQMF && !writeQMF)
	{
		processingDelay += (abs(max_negative) - h_BRIRmetadata->initDelay + correctDelay);
		if ((nTotalSamplesPerChannel - processingDelay) < (nTotalSamplesPerChannel - 320)) /* - forward QMF */
		{
			addSamples += (processingDelay - 320); /* add samples to get right file length */
		}
		if ((nTotalSamplesPerChannel - processingDelay) > (nTotalSamplesPerChannel - 320)) /* - forward QMF */
		{
			skipLastSamples += abs(processingDelay - 320); /* skip samples to get right file length */
		}
	}
	if (!readQMF && writeQMF)
	{
		addSamples += (abs(max_negative) - h_BRIRmetadata->initDelay + correctDelay);
	}

	if (do_compensate_delay == 1)
	{
		delay_to_compensate = processingDelay;
	}
	else
	{ 
		delay_to_compensate = 0;
	}
#else
	if (abs(max_negative) > 0)
	{ 
		correctDelay = 1;
	}
	else
	{
		correctDelay = 0;
	}

	if (!readQMF && !writeQMF)
	{
		processingDelay += (abs(max_negative) + correctDelay);
		addSamples = processingDelay;
	}
	if (readQMF && !writeQMF)
	{
		processingDelay += (abs(max_negative) + correctDelay);
		if ((nTotalSamplesPerChannel - processingDelay) < (nTotalSamplesPerChannel - (320+257))) /* - forward QMF */
		{
			addSamples += (processingDelay - (320+257)); /* add samples to get right file length */
		}
		if ((nTotalSamplesPerChannel - processingDelay) > (nTotalSamplesPerChannel - (320+257))) /* - forward QMF */
		{
			skipLastSamples += abs(processingDelay - (320+257)); /* skip samples to get right file length */
		}
	}
	if (!readQMF && writeQMF)
	{
		addSamples += (abs(max_negative) + correctDelay + 257);
	}

	if (do_compensate_delay == 1)
	{
		delay_to_compensate = processingDelay;
	}
  else if (do_compensate_delay == 2)
	{ 
		delay_to_compensate = 33;
	}
	else
	{ 
		delay_to_compensate = 0;
	}
#endif

	if (readQMF  && writeQMF)
	{
		wavIO_setDelay(hWavIO_real, -1*delay_to_compensate); 
		wavIO_setDelay(hWavIO_imag, -1*delay_to_compensate); 
	}
	/* read OR write QMF files */
	else if (readQMF || writeQMF)
	{
		wavIO_setDelay(hWavIO_real, -1*delay_to_compensate); 
		wavIO_setDelay(hWavIO_imag, -1*delay_to_compensate); 
		wavIO_setDelay(hWavIO, -1*delay_to_compensate); 
	}
	else
	{
		wavIO_setDelay(hWavIO, -1*delay_to_compensate); 
	}
	addSamplesOrig = addSamples;


	/* init buffers for wav read and write */
	inBuffer = (float**)calloc(nInChannels, sizeof(float*)); 
	outBuffer = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
	for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
	{
		outBuffer[chOut] = (float*)calloc(binaural_blocklength, sizeof(float)); 
	}
	for (chIn = 0; chIn < (int)nInChannels; chIn++)
	{
		inBuffer[chIn] = (float*)calloc(binaural_blocklength, sizeof(float)); 
	}

  
	/* ************************* PROFILE LEVEL CONSTRAINTS TERMINATION ***************************** */
  
  /* binaural profile level constraints */
  if (profile == PROFILE_LOW_COMPLEXITY)
  {
  
    /* bsBinauralDataformatID shall be equal 1 */
    if ( pBinauralRepresentation->binauralDataFormatID != 1 )
    {
      fprintf(stderr, "\nError: binauralDataFormatID shall be set to 1 when the FD binaural renderer is used (see %s)!\n\n", DAM3_TITLE);
    }

    /* Number of BRIR sets shall be smaller of equal to 3 */
    if ( pBinauralRendering->numBinauralRepresentation > 3 )
    {
      fprintf(stderr, "\nError: numBinauralRepresentation shall be less or equal than 3 (see %s)!\n\n", DAM3_TITLE);
    }

    /* Check number of BRIR pairs */
    {
      int nPairs = pBinauralRepresentation->nBrirPairs;
      if ( checkMaxNumBrirPairs( nPairs, level ) == 0 )
      {
        fprintf(stderr, "\nError: The number of BRIR pairs (%i) is too high for Low Complexity Profile (see %s)!\n\n", nPairs, DAM3_TITLE);
      }
    }

    /* Check k_max */
    if ( numband_proc > 48 )
    {
      fprintf(stderr, "\nError: k_max shall be less or equal than 48 (see %s)!\n\n", DAM3_TITLE);
    }

    /* Check k_conv */
    if ( numband_conv > 32 )
    {
      fprintf(stderr, "\nError: k_conv shall be less or equal than 32 (see %s)!\n\n", DAM3_TITLE);
    }

    /* Check rt60 */
    for (band = 0; band < h_BRIRmetadata->LateReverb_AnalysisBands; band++)
    {
      if ( h_BRIRmetadata->LateReverb_RT60[band] > 1.0f )
      {
        fprintf(stderr, "\nError: rt60 shall not exceed 1.0 sec (see %s)!\n\n", DAM3_TITLE);
        break;
      }
    }

    /* Check nFilter average value */
    {
      int average = 0;
      for (band = 0; band < numband_conv; band++)
      {
        average += DE_filter_length[band];
      }
      average /= numband_conv;
      if ( average > 64 )
      {
        fprintf(stderr, "\nError: The average of nFilter[k] shall not exceed 64 (see %s)!\n\n", DAM3_TITLE);
      }
    }

    /* Check nFilter */
    for (band = 0; band < numband_conv; band++)
    {
      if ( DE_filter_length[band] > 256 )
      {
        fprintf(stderr, "\nError: nFilter[k] shall not exceed 256 (see %s)!\n\n", DAM3_TITLE);
        break;
      }
    }
  }




	/* ************************* INIT CONVOLUTION OF D+E ***************************** */

	/* get list of BRIRs that are used for the convolution */

	m_conv = (int*)calloc(nInChannels, sizeof(int)); 
	error_brir = binaural_getBRIRConfig(Audio_azimuth, Audio_elevation, h_BRIRmetadata, m_conv, nInChannels, num_LFE, LFE_channels); 

	/* ************************* fallback if some BRIRs are not available ****************************** */
	{
		float az_thresh = 20.0f;
		float az_should = 0.0f;
		float el_should = 0.0f;
		float az_is = 0.0f;
		float el_is = 0.0f;
		int is_LFE = 0;
		int i = 0;
		int found_same_layer = 0;
		int num_resetBRIR = 0;

		/* check for availability of current channel config */
		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			/* found a non-supported audio channel */
			if (m_conv[chIn] == -2)
			{
				az_should = Audio_azimuth[chIn];
				el_should = Audio_elevation[chIn];

				/* search on same layer first with given azimuth threshold */
				for (k = 0; k < h_BRIRmetadata->channels; k++)
				{
					is_LFE = 0;
					az_is = h_BRIRmetadata->azimuth[k];
					el_is = h_BRIRmetadata->elevation[k];
					for (i = 0; i < h_BRIRmetadata->numLFE; i++)
					{
						if (h_BRIRmetadata->LFEchannels[i] == k)
						{
							is_LFE = 1;
						}
					}

					if (!is_LFE)
					{
						if (el_should == el_is)
						{
							if (az_should >= 0)
							{
								if ((az_is >= (az_should - az_thresh)) && (az_is < (az_should + az_thresh)))
								{
									m_conv[chIn] = k;
									num_resetBRIR++;
									found_same_layer = 1;
									break;
								}
							}
							else
							{
								if ((az_is <= (az_should + az_thresh)) && (az_is > (az_should - az_thresh)))
								{
									m_conv[chIn] = k;
									num_resetBRIR++;
									found_same_layer = 1;
									break;

								}
							}
						}
					}
				}

				if (found_same_layer == 0)
					/* search next BRIR with minimum geometric distance difference on all layers */
				{
					float diff = 0.0;
					float mindiff = 1000.0f;
					int mindiff_idx = -1;

					for (k = 0; k < h_BRIRmetadata->channels; k++)
					{
						is_LFE = 0;
						az_is = h_BRIRmetadata->azimuth[k];
						el_is = h_BRIRmetadata->elevation[k];
						for (i = 0; i < h_BRIRmetadata->numLFE; i++)
						{
							if (h_BRIRmetadata->LFEchannels[i] == k)
							{
								is_LFE = 1;
							}
						}
						if (!is_LFE)
						{
							diff = (float)fabs(az_is - az_should) + (float)fabs(el_is - el_should);
							/* put in weighting of az and el if wanted */
							if (diff <= mindiff)
							{
								mindiff_idx = k;
							}
							mindiff = min(mindiff, diff);
						}
					}
					m_conv[chIn] = mindiff_idx;
					num_resetBRIR++;
				}

			}
		}
	}

	/* ********************* INIT DMX MATRIX FOR LATE REVERB GENERATOR ************************* */

	/* calculate stereo dmx matrix */
	if (!just_DE)
	{ 
		int left = 0;
		int right = 0;
		dmxmtx = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			dmxmtx[chOut] = (float*)calloc(nInChannels, sizeof(float)); 
		}
		ct = 0;

		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			float az = Audio_azimuth[chIn];
			if (((az == 0) || (az == 180)) && (m_conv[chIn] > -1))
			{
				dmxmtx[0][ct] = 0.7071f;
				dmxmtx[1][ct] = 0.7071f;
				left++;
				right++;
				ct++;
			}
			else if ((az > 0) && (m_conv[chIn] > -1))
			{
				dmxmtx[0][ct] = 1.0f;
				left++;
				ct++;
			}
			else if (m_conv[chIn] <= -1)
			{
				ct++;
			}
			else if ((az < 0) && (m_conv[chIn] > -1))
			{ 
				dmxmtx[1][ct] = 1.0f;
				right++;
				ct++;
			}
			no_dmxchannels_left = left;
			no_dmxchannels_right = right;
		}
	}



	/* ******************************** INIT FRAME OVERLAP *********************************** */
	temp_output_QMF_real = (float***)calloc(numband_proc,sizeof(float**));
	temp_output_QMF_imag = (float***)calloc(numband_proc,sizeof(float**));
	for (band = 0; band < numband_proc; band++)
	{
		temp_output_QMF_real[band] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS,sizeof(float*));
		temp_output_QMF_imag[band] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS,sizeof(float*));
		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			temp_output_QMF_real[band][chOut] = (float*)calloc(binaural_numTimeslots,sizeof(float));
			temp_output_QMF_imag[band][chOut] = (float*)calloc(binaural_numTimeslots,sizeof(float));
		}
	}

	if (add_DE_delay > 0)
	{
		overlap_real = (float***)calloc(numband_proc,sizeof(float**));
		overlap_imag = (float***)calloc(numband_proc,sizeof(float**));
		for (band = 0; band < numband_proc; band++)
		{
			overlap_real[band] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS,sizeof(float*));
			overlap_imag[band] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS,sizeof(float*));
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				overlap_real[band][chOut] = (float*)calloc(add_DE_delay,sizeof(float));
				overlap_imag[band][chOut] = (float*)calloc(add_DE_delay,sizeof(float));
			}
		}
	}

	/* *************************** INIT FFT TRANSFORM OF AUDIO FRAME ***************************** */

	/* 4D Buffer [bands x channels x max_numframes x 128] */
	input_audio_FFT_real = (float****)calloc(numband_conv,sizeof(float***));
	input_audio_FFT_imag = (float****)calloc(numband_conv,sizeof(float***));

	for (band = 0; band < numband_conv; band++)
	{
		input_audio_FFT_real[band] = (float***)calloc(nInChannels,sizeof(float**));
		input_audio_FFT_imag[band] = (float***)calloc(nInChannels,sizeof(float**));

		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			input_audio_FFT_real[band][chIn] = (float**)calloc(h_BRIRmetadata->N_FRM[band],sizeof(float*));
			input_audio_FFT_imag[band][chIn] = (float**)calloc(h_BRIRmetadata->N_FRM[band],sizeof(float*));

			for (k=0; k< h_BRIRmetadata->N_FRM[band]; k++) 
			{
				input_audio_FFT_real[band][chIn][k] = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
				input_audio_FFT_imag[band][chIn][k] = (float*)calloc(h_BRIRmetadata->N_FFT[band],sizeof(float));
			}
		}
	}

	/* ************************* INIT D&E BUFFERS ****************************** */	
	buf_DE_BLK_left_real = (float***)calloc(numband_conv, sizeof(float**)); 
	buf_DE_BLK_left_imag = (float***)calloc(numband_conv, sizeof(float**)); 
	buf_DE_BLK_right_real = (float***)calloc(numband_conv, sizeof(float**)); 
	buf_DE_BLK_right_imag = (float***)calloc(numband_conv, sizeof(float**)); 
	for( band = 0; band < numband_conv; band++ )
	{
		int blk;
		int num_block = h_BRIRmetadata->N_BLK[band];
		buf_DE_BLK_left_real[band] = (float**)calloc(num_block, sizeof(float*));
		buf_DE_BLK_left_imag[band] = (float**)calloc(num_block, sizeof(float*));
		buf_DE_BLK_right_real[band] = (float**)calloc(num_block, sizeof(float*));
		buf_DE_BLK_right_imag[band] = (float**)calloc(num_block, sizeof(float*));		  
		for ( blk = 0; blk < num_block; blk++)
		{
			int num_FFT = h_BRIRmetadata->N_FFT[band];
			buf_DE_BLK_left_real[band][blk] = (float*)calloc(num_FFT, sizeof(float));
			buf_DE_BLK_left_imag[band][blk] = (float*)calloc(num_FFT, sizeof(float));
			buf_DE_BLK_right_real[band][blk] = (float*)calloc(num_FFT, sizeof(float));
			buf_DE_BLK_right_imag[band][blk] = (float*)calloc(num_FFT, sizeof(float));
		}
	}
	buf_DE_FRM_left_real = (float**)calloc(numband_conv, sizeof(float*));
	buf_DE_FRM_left_imag = (float**)calloc(numband_conv, sizeof(float*));
	buf_DE_FRM_right_real = (float**)calloc(numband_conv, sizeof(float*));
	buf_DE_FRM_right_imag = (float**)calloc(numband_conv, sizeof(float*));

	for( band = 0; band < numband_conv; band++)
	{
		/*int bufferlength = h_BRIRmetadata->N_FFT[band]/2; */
		int bufferlength = max(h_BRIRmetadata->N_FFT[band]/2, h_BRIRmetadata->N_FFT[band]-binaural_numTimeslots); 
		buf_DE_FRM_left_real[band] = (float*)calloc(bufferlength, sizeof(float));
		buf_DE_FRM_left_imag[band] = (float*)calloc(bufferlength, sizeof(float));
		buf_DE_FRM_right_real[band] = (float*)calloc(bufferlength, sizeof(float));
		buf_DE_FRM_right_imag[band] = (float*)calloc(bufferlength, sizeof(float));

		memset(buf_DE_FRM_left_real[band], 0, bufferlength*sizeof(float));
		memset(buf_DE_FRM_left_imag[band], 0, bufferlength*sizeof(float));
		memset(buf_DE_FRM_right_real[band], 0, bufferlength*sizeof(float));
		memset(buf_DE_FRM_right_imag[band], 0, bufferlength*sizeof(float));
	}

	/* ************************* INIT DOWNMIX BUFFERS ***************************** */
	if (!just_DE)
	{
		output_dmx_QMF_real = (float***)calloc(binaural_blocklength/QMFLIB_NUMBANDS, sizeof(float**)); 
		output_dmx_QMF_imag = (float***)calloc(binaural_blocklength/QMFLIB_NUMBANDS, sizeof(float**)); 
		for (k = 0; k < binaural_blocklength/QMFLIB_NUMBANDS; k++)
		{
			output_dmx_QMF_real[k] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
			output_dmx_QMF_imag[k] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				output_dmx_QMF_real[k][chOut] = (float*)calloc(QMFLIB_NUMBANDS, sizeof(float)); 
				output_dmx_QMF_imag[k][chOut] = (float*)calloc(QMFLIB_NUMBANDS, sizeof(float)); 
			}
		}
	}

	/* **************************** INIT ReverbGenerator ****************************** */
	if (!just_DE)
	{
		ReverbGenerator_Init(&reverb, 0, BINAURAL_NUM_OUTCHANNELS, BINAURAL_NUM_OUTCHANNELS, QMFLIB_NUMBANDS); 
		ReverbGenerator_set_reverb_times(reverb, (float)InSampleRate, numband_LRanalysis, h_BRIRmetadata->LateReverb_fc, h_BRIRmetadata->LateReverb_RT60, h_BRIRmetadata->LateReverb_energy, randSeed); 
		ReverbGenerator_set_correctCoh(reverb, 1); 
		ReverbGenerator_set_predelay(reverb, 10); 
		ReverbGenerator_set_drywet(reverb, DryWetDefault); 

		h_reverb_struct->max_startReverbQMF = max_startReverbQMF;

		/* init reverb structure and buffers for delay of reverberation */
		h_reverb_struct->buffer_rev_re = (float***)calloc(max_startReverbQMF, sizeof(float**)); 
		h_reverb_struct->buffer_rev_im = (float***)calloc(max_startReverbQMF, sizeof(float**)); 
		for (k = 0; k < max_startReverbQMF; k++)
		{
			h_reverb_struct->buffer_rev_re[k] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
			h_reverb_struct->buffer_rev_im[k] = (float**)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float*)); 
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				h_reverb_struct->buffer_rev_re[k][chOut] = (float*)calloc(QMFLIB_NUMBANDS, sizeof(float)); 
				h_reverb_struct->buffer_rev_im[k][chOut] = (float*)calloc(QMFLIB_NUMBANDS, sizeof(float)); 
			}
		}
		h_reverb_struct->scale_old = (float*)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float));
		h_reverb_struct->scale = (float*)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof(float));

		h_reverb_struct->startReverbQMF = (int*)calloc(QMFLIB_NUMBANDS,sizeof(int));
		for (band = 0; band < QMFLIB_NUMBANDS; band++)
		{
			if (band < numband_conv)
			{
				h_reverb_struct->startReverbQMF[band] = startReverbQMF[band] ;
			}
			else
			{
				h_reverb_struct->startReverbQMF[band] = 1;
			}
		}
	}
	/* **************************** INIT TDL ****************************** */

	if (TDL == 1)
	{
		/* Buffer */
		buf_TDL_left_real = (float***) calloc(numband_proc-numband_conv, sizeof(float**));
		buf_TDL_left_imag = (float***) calloc(numband_proc-numband_conv, sizeof(float**));
		buf_TDL_right_real = (float***) calloc(numband_proc-numband_conv, sizeof(float**));
		buf_TDL_right_imag = (float***) calloc(numband_proc-numband_conv, sizeof(float**));
		for (band = 0; band < numband_proc-numband_conv; band++ )
		{
			buf_TDL_left_real[band] = (float**) calloc(nInChannels, sizeof(float*));
			buf_TDL_left_imag[band] = (float**) calloc(nInChannels, sizeof(float*));
			buf_TDL_right_real[band] = (float**) calloc(nInChannels, sizeof(float*));
			buf_TDL_right_imag[band] = (float**) calloc(nInChannels, sizeof(float*));

			for (chIn = 0; chIn < (int)nInChannels; chIn++)
			{
				int idxbrir = m_conv[chIn];
				if( idxbrir != -1)
				{
#ifdef BUGFIX_FRAMESIZE_1024
					buf_TDL_left_real[band][chIn] = (float*) calloc(binaural_numTimeslots+h_BRIRmetadata->TDL_delay_left[band][idxbrir], sizeof(float));
					buf_TDL_left_imag[band][chIn] = (float*) calloc(binaural_numTimeslots+h_BRIRmetadata->TDL_delay_left[band][idxbrir], sizeof(float));
					buf_TDL_right_real[band][chIn] = (float*) calloc(binaural_numTimeslots+h_BRIRmetadata->TDL_delay_right[band][idxbrir], sizeof(float));
					buf_TDL_right_imag[band][chIn] = (float*) calloc(binaural_numTimeslots+h_BRIRmetadata->TDL_delay_right[band][idxbrir], sizeof(float));
#else
                    buf_TDL_left_real[band][chIn] = (float*) calloc(h_BRIRmetadata->TDL_delay_left[band][idxbrir], sizeof(float));
                    buf_TDL_left_imag[band][chIn] = (float*) calloc(h_BRIRmetadata->TDL_delay_left[band][idxbrir], sizeof(float));
                    buf_TDL_right_real[band][chIn] = (float*) calloc(h_BRIRmetadata->TDL_delay_right[band][idxbrir], sizeof(float));
                    buf_TDL_right_imag[band][chIn] = (float*) calloc(h_BRIRmetadata->TDL_delay_right[band][idxbrir], sizeof(float));
#endif
				}
			}
		}
	}
	/* **************************** INIT QMF ****************************** */

	/* input QMF [timeslots x channels x 64] */
	input_audio_QMF_real = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	input_audio_QMF_imag = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	for(k = 0; k < binaural_numTimeslots; k++ )
	{
		input_audio_QMF_real[k] = (float**)calloc (nInChannels, sizeof (float*)); 
		input_audio_QMF_imag[k] = (float**)calloc (nInChannels, sizeof (float*)); 

		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			input_audio_QMF_real[k][chIn] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
			input_audio_QMF_imag[k][chIn] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
		}
	}
	/* output QMF [timeslots x 2 x 64] */
	output_DE_QMF_real = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	output_DE_QMF_imag = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	for(k = 0; k < binaural_numTimeslots; k++ )
	{
		output_DE_QMF_real[k] = (float**)calloc (BINAURAL_NUM_OUTCHANNELS, sizeof (float*)); 
		output_DE_QMF_imag[k] = (float**)calloc (BINAURAL_NUM_OUTCHANNELS, sizeof (float*)); 

		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			output_DE_QMF_real[k][chOut] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
			output_DE_QMF_imag[k][chOut] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
		}
	}

	/* output QMF [timeslots x 2 x 64] */
	output_DE_LR_QMF_real = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	output_DE_LR_QMF_imag = (float***)calloc(binaural_numTimeslots, sizeof (float**)); 	
	for(k = 0; k < binaural_numTimeslots; k++ )
	{
		output_DE_LR_QMF_real[k] = (float**)calloc (BINAURAL_NUM_OUTCHANNELS, sizeof (float*)); 
		output_DE_LR_QMF_imag[k] = (float**)calloc (BINAURAL_NUM_OUTCHANNELS, sizeof (float*)); 

		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			output_DE_LR_QMF_real[k][chOut] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
			output_DE_LR_QMF_imag[k][chOut] = (float*)calloc (QMFLIB_NUMBANDS, sizeof (float)); 
		}
	}

	/* **************************** init multi-channel QMF **************************** */
	if (!writeQMF)
	{
		pSynthesisQmf = (QMFLIB_POLYPHASE_SYN_FILTERBANK **)calloc(BINAURAL_NUM_OUTCHANNELS, sizeof (QMFLIB_POLYPHASE_SYN_FILTERBANK * )); 
		/* init synthesis */     
		QMFlib_InitSynFilterbank(QMFLIB_NUMBANDS, 0); 
		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			QMFlib_OpenSynFilterbank(&pSynthesisQmf[chOut]); 
		}
	}

	if (!readQMF)
	{
		pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK **)calloc(nInChannels, sizeof (QMFLIB_POLYPHASE_ANA_FILTERBANK * ));
		/* init analysis */
		QMFlib_InitAnaFilterbank(QMFLIB_NUMBANDS, 0); 
		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			QMFlib_OpenAnaFilterbank(&pAnalysisQmf[chIn]); 
		}
	}		

	/* ************************************************************************************* */
	/* **************************** START BINAURAL PROCESSING ****************************** */
	/* ************************************************************************************* */
	if (handle_error != 0)
	{
		goto free_memory;
	}

	fprintf(stderr, "\nProcessing loop started.\n"); 

	do /*loop over all frames*/
	{
		unsigned int samplesReadPerChannel = 0; 
		unsigned int samplesToWritePerChannel = 0; 
		unsigned int samplesWrittenPerChannel = 0; 
		unsigned int nZerosPaddedBeginning = 0; 
		unsigned int nZerosPaddedEnd = 0; 

		frameNo++; 

		/* ----------------------------- read input data frame ------------------------------ */

		/* ------------------ read QMF frame ------------------ */
		if (readQMF)
		{
			if (fwavIn_real && fwavIn_imag)
			{
				/* activate binaural processing code */
				process = 1;

				/* ------ read real parts of QMF values ----- */

				wavIO_error_read = wavIO_readFrame(hWavIO_real, inBuffer, &samplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd); 
				samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
				NumberSamplesRead += samplesReadPerChannel;
				/* last frame: flush unused part of input buffer*/
				if (isLastFrame)
				{
					for (chIn = 0; chIn < (int)nInChannels; chIn++)
					{
						for (k = samplesReadPerChannel; k < binaural_blocklength; k++)
						{
							inBuffer[chIn][k] = 0.0f;
						}
					}
				}
				/* transfer to real QMF buffer */
				for (chIn = 0; chIn < (int)nInChannels; chIn++)
				{
					int ct = 0;
					for (k = 0; k < binaural_numTimeslots; k++)
					{
						for (band = 0; band < QMFLIB_NUMBANDS; band++)
						{
							input_audio_QMF_real[k][chIn][band] = inBuffer[chIn][ct];
							ct++;
						}
					}
				}

				/* ------ read imaginary parts of QMF values ----- */

				wavIO_error_read = wavIO_readFrame(hWavIO_imag, inBuffer, &samplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd); 
				if (isLastFrame)
				{
					for (chIn = 0; chIn < (int)nInChannels; chIn++)
					{
						for (k = samplesReadPerChannel; k < binaural_blocklength; k++)
						{
							inBuffer[chIn][k] = 0.0f;
						}
					}
				}
				/* transfer to imag QMF buffer */
				for (chIn = 0; chIn < (int)nInChannels; chIn++)
				{
					int ct = 0;
					for (k = 0; k < binaural_numTimeslots; k++)
					{
						for (band = 0; band < QMFLIB_NUMBANDS; band++)
						{
							input_audio_QMF_imag[k][chIn][band] = inBuffer[chIn][ct];
							ct++;
						}
					}
				}
			}
			else
			{
				/* deactivate binaural processing code */
				process = 0;
			}
		}
		/* ------------------ read time domain frame ------------------ */
		else
		{
			if (fwavIn)
			{
				/* activate binaural processing code */
				process = 1;
				wavIO_error_read = wavIO_readFrame(hWavIO, inBuffer, &samplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd); 
				samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd; 
				NumberSamplesRead += samplesReadPerChannel; 
				if (isLastFrame)
					/* flush unused part of input buffer */
				{
					for (k = samplesReadPerChannel; k < binaural_blocklength; k++)
					{
						for (chIn = 0; chIn < (int)nInChannels; chIn++)
						{
							inBuffer[chIn][k] = 0.0f; 
						}
					}
				}

				/* QMF transform of read frame */
				binaural_qmftransform_frame(inBuffer, binaural_numTimeslots, binaural_hopsize, nInChannels, pAnalysisQmf, 0, input_audio_QMF_real, input_audio_QMF_imag); 
			}
			else
			{
				/* deactivate binaural processing code */
				process = 0;
			}
		}

		/* init late reverb scaling in first frame */
		if ((frameNo == 0) && (!just_DE))
		{
			binaural_init_scaling(h_reverb_struct, dmxmtx, nInChannels); 
		}

		/* -------------------  binauralization processing -------------------- */
		if (process == 1)
		{
			/* ----------------  1. convolution with Early Reflections  ------------------*/

			/* ----------------  FFT transform of frame ------------------*/
			binaural_FFTtransform_QMFframe(input_audio_QMF_real, input_audio_QMF_imag, input_audio_FFT_real, input_audio_FFT_imag, numband_conv, h_BRIRmetadata->N_FFT, h_BRIRmetadata->N_FRM, binaural_numTimeslots, nInChannels);

			/* ----------------  Variable Order Filtering in Frequency domain ---------------------*/ 			
			binaural_VOFF(output_DE_QMF_real, output_DE_QMF_imag, input_audio_FFT_real, input_audio_FFT_imag, 
				buf_DE_BLK_left_real, buf_DE_BLK_left_imag, buf_DE_BLK_right_real, buf_DE_BLK_right_imag, 
				buf_DE_FRM_left_real, buf_DE_FRM_left_imag, buf_DE_FRM_right_real, buf_DE_FRM_right_imag, 
				h_BRIRmetadata, m_conv, binaural_numTimeslots, nInChannels, numband_conv);

			/* ----------------  QTDL Processing ---------------------*/ 	
			binaural_QTDL(output_DE_QMF_real, output_DE_QMF_imag, input_audio_QMF_real, input_audio_QMF_imag, 
				buf_TDL_left_real, buf_TDL_left_imag, buf_TDL_right_real, buf_TDL_right_imag,
				h_BRIRmetadata, m_conv, binaural_numTimeslots, nInChannels, numband_conv, numband_proc);

			/* ---------------- delay of D+E result ---------------- */
			for (band = 0; band < numband_proc; band++)
			{
				for (k=0;k<binaural_numTimeslots; k++)
				{
					if ( k < add_DE_delay)
					{
						temp_output_QMF_real[band][0][k] = overlap_real[band][0][k];
						temp_output_QMF_imag[band][0][k] = overlap_imag[band][0][k];
						temp_output_QMF_real[band][1][k] = overlap_real[band][1][k];
						temp_output_QMF_imag[band][1][k] = overlap_imag[band][1][k];

						overlap_real[band][0][k] = output_DE_QMF_real[binaural_numTimeslots-add_DE_delay+k][0][band];
						overlap_imag[band][0][k] = output_DE_QMF_imag[binaural_numTimeslots-add_DE_delay+k][0][band];
						overlap_real[band][1][k] = output_DE_QMF_real[binaural_numTimeslots-add_DE_delay+k][1][band];
						overlap_imag[band][1][k] = output_DE_QMF_imag[binaural_numTimeslots-add_DE_delay+k][1][band];
					}
					else
					{
						temp_output_QMF_real[band][0][k] = output_DE_QMF_real[k-add_DE_delay][0][band];
						temp_output_QMF_imag[band][0][k] = output_DE_QMF_imag[k-add_DE_delay][0][band];
						temp_output_QMF_real[band][1][k] = output_DE_QMF_real[k-add_DE_delay][1][band];
						temp_output_QMF_imag[band][1][k] = output_DE_QMF_imag[k-add_DE_delay][1][band];
					}
				}
			}

			if (!just_DE)
			{
				/* ----------------  2. downmix to stereo and get scaling factors  ------------------*/
				binaural_downmix_frame_qmfdomain(binaural_numTimeslots, input_audio_QMF_real, input_audio_QMF_imag, output_dmx_QMF_real, output_dmx_QMF_imag, h_reverb_struct, InSampleRate, dmxmtx, nInChannels, numband_conv); 
				/* ---------------- 3. Scale downmix and calculate reverb from downmix ------------------*/
				/* result is added to temp_output_QMF_real, temp_output_QMF_imag */
				binaural_calculate_reverb(reverb, h_reverb_struct, output_dmx_QMF_real, output_dmx_QMF_imag, temp_output_QMF_real, temp_output_QMF_imag, numband_conv, binaural_blocklength);
			}
		}


		/* -------------------  write data -------------------- */
		if (isLastFrame)
		{
			/* ----------------  write last frame + additional data (delay) with maximum total size of  binaural_blocklength----------------- */
			int temp = min((int)samplesToWritePerChannel + addSamples,binaural_blocklength);
			if (skipLastSamples > 0)
			{
				temp -= skipLastSamples;
			}
			if ((int)samplesToWritePerChannel + addSamples > binaural_blocklength)
			{
				/* more additional data needs to be written */
				addSamples = samplesToWritePerChannel + addSamples - binaural_blocklength;
			}
			else
			{
				/*writing is finished */
				addSamples = 0;
				addSamples_written = 1;
			}
			samplesToWritePerChannel = temp;
		}

		/* ---------- write QMF domain data ----------- */
		if (writeQMF)
		{
			if (fwavOut_real && fwavOut_imag)
			{
				/* resort temp_output_QMF */
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					int ct = 0;
					for (k = 0; k < binaural_numTimeslots; k++)
					{
						for (band = 0; band < QMFLIB_NUMBANDS; band++)
						{
							if (band < numband_proc)
							{
								outBuffer[chOut][ct] = temp_output_QMF_real[band][chOut][k];
							}
							else
							{
								outBuffer[chOut][ct] = 0.0f;
							}
							ct++;
						}
					}
				}
				/* write real frame */
				wavIO_error_write = wavIO_writeFrame(hWavIO_real, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);

				/* resort temp_output_QMF */
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					int ct = 0;
					for (k = 0; k < binaural_numTimeslots; k++)
					{
						for (band = 0; band < QMFLIB_NUMBANDS; band++)
						{
							if (band < numband_proc)
							{
								outBuffer[chOut][ct] = temp_output_QMF_imag[band][chOut][k];
							}
							else
							{
								outBuffer[chOut][ct] = 0.0f;
							}
							ct++;
						}
					}
				}
				/* write imag frame */
				wavIO_error_write = wavIO_writeFrame(hWavIO_imag, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);
			}
		}
		else
		{
			/* ---------- write time domain data ----------- */
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				for (k = 0; k < binaural_numTimeslots; k++)
				{
					for (band = 0; band < numband_proc; band++)
					{
						output_DE_LR_QMF_real[k][chOut][band] = temp_output_QMF_real[band][chOut][k];
						output_DE_LR_QMF_imag[k][chOut][band] = temp_output_QMF_imag[band][chOut][k];
					}
				}
			}

			/* inverse QMF transform */
			binaural_iqmftransform_frame(outBuffer, binaural_numTimeslots, binaural_hopsize, BINAURAL_NUM_OUTCHANNELS, pSynthesisQmf, 0, output_DE_LR_QMF_real, output_DE_LR_QMF_imag); 
			if (fwavOut)
			{
				wavIO_error_write = wavIO_writeFrame(hWavIO, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);
			}
		}
		NumberSamplesWritten += samplesWrittenPerChannel; 
		/* update progress */
		percentage = (float)NumberSamplesWritten / (float)(nTotalSamplesPerChannel);  



		/*----------------- end of file behavior, add delay data after last frame ------------ */
		/* calculate needed additional output with zero input */

		if ((isLastFrame) && (addSamples > 0))
		{

			int addFramesToWrite = (int)(floor(addSamples / binaural_blocklength)+1);
			int addFrame;

			for (addFrame = 0; addFrame < addFramesToWrite; addFrame++)
			{

				samplesToWritePerChannel = min(addSamples,binaural_blocklength);
				addSamples = addSamples - samplesToWritePerChannel;

				/* set output of D+E frame to zero */
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					for (band = 0; band < numband_proc; band++)
					{
						for(k = 0; k < binaural_numTimeslots; k++)
						{
							output_DE_QMF_real[k][chOut][band] = 0.0f; 
							output_DE_QMF_imag[k][chOut][band] = 0.0f; 
						}
					}
				}

				/* delay + overlap of D+E result */
				for (band = 0; band < numband_proc; band++)
				{
					for (k=0;k<binaural_numTimeslots; k++)
					{
						if ( k < add_DE_delay)
						{
							temp_output_QMF_real[band][0][k] = overlap_real[band][0][k];
							temp_output_QMF_imag[band][0][k] = overlap_imag[band][0][k];
							temp_output_QMF_real[band][1][k] = overlap_real[band][1][k];
							temp_output_QMF_imag[band][1][k] = overlap_imag[band][1][k];

							overlap_real[band][0][k] = output_DE_QMF_real[binaural_numTimeslots-add_DE_delay+k][0][band];
							overlap_imag[band][0][k] = output_DE_QMF_imag[binaural_numTimeslots-add_DE_delay+k][0][band];
							overlap_real[band][1][k] = output_DE_QMF_real[binaural_numTimeslots-add_DE_delay+k][1][band];
							overlap_imag[band][1][k] = output_DE_QMF_imag[binaural_numTimeslots-add_DE_delay+k][1][band];
						}
						else
						{
							temp_output_QMF_real[band][0][k] = output_DE_QMF_real[k-add_DE_delay][0][band];
							temp_output_QMF_imag[band][0][k] = output_DE_QMF_imag[k-add_DE_delay][0][band];
							temp_output_QMF_real[band][1][k] = output_DE_QMF_real[k-add_DE_delay][1][band];
							temp_output_QMF_imag[band][1][k] = output_DE_QMF_imag[k-add_DE_delay][1][band];
						}
					}
				}

				/* calculate reverb if needed */
				if (!just_DE)
				{
					binaural_downmix_frame_qmfdomain(binaural_numTimeslots, input_audio_QMF_real, input_audio_QMF_imag, output_dmx_QMF_real, output_dmx_QMF_imag, h_reverb_struct, InSampleRate, dmxmtx, nInChannels, numband_conv); 
          binaural_calculate_reverb(reverb, h_reverb_struct, output_dmx_QMF_real, output_dmx_QMF_imag, temp_output_QMF_real, temp_output_QMF_imag, numband_conv, binaural_blocklength);
				}

				/* write data */
				if (writeQMF)
				{
					if (fwavOut_real && fwavOut_imag)
					{
						/* resort temp_output_QMF */
						for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
						{
							int ct = 0;
							for (k = 0; k < binaural_numTimeslots; k++)
							{
								for (band = 0; band < QMFLIB_NUMBANDS; band++)
								{
									if (band < numband_proc)
									{
										outBuffer[chOut][ct] = temp_output_QMF_real[band][chOut][k];
									}
									else
									{
										outBuffer[chOut][ct] = 0.0f;
									}
									ct++;
								}
							}
						}
						/* write real frame */
						wavIO_error_write = wavIO_writeFrame(hWavIO_real, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);

						/* resort temp_output_QMF */
						for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
						{
							int ct = 0;
							for (k = 0; k < binaural_numTimeslots; k++)
							{
								for (band = 0; band < QMFLIB_NUMBANDS; band++)
								{
									if (band < numband_proc)
									{
										outBuffer[chOut][ct] = temp_output_QMF_imag[band][chOut][k];
									}
									else
									{
										outBuffer[chOut][ct] = 0.0f;
									}
									ct++;
								}
							}
						}
						/* write imag frame */
						wavIO_error_write = wavIO_writeFrame(hWavIO_imag, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);
					}
				}
				else
				{
					/* ---------- write time domain data ----------- */
					for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
					{
						for (k = 0; k < binaural_numTimeslots; k++)
						{
							for (band = 0; band < numband_proc; band++)
							{
								output_DE_LR_QMF_real[k][chOut][band] = temp_output_QMF_real[band][chOut][k];
								output_DE_LR_QMF_imag[k][chOut][band] = temp_output_QMF_imag[band][chOut][k];
							}
						}
					}

					/* inverse QMF transform */
					binaural_iqmftransform_frame(outBuffer, binaural_numTimeslots, binaural_hopsize, BINAURAL_NUM_OUTCHANNELS, pSynthesisQmf, 0, output_DE_LR_QMF_real, output_DE_LR_QMF_imag); 
					if (fwavOut)
					{
						wavIO_error_write = wavIO_writeFrame(hWavIO, outBuffer, samplesToWritePerChannel, &samplesWrittenPerChannel);
					}
				}

				/* update number of written samples */
				NumberSamplesWritten += samplesWrittenPerChannel; 
				percentage = (float)NumberSamplesWritten / (float)(nTotalSamplesPerChannel);
			}

			addSamples_written = 1;

			/*----------------------- end of end of file behavior -------------- */

		}

		/* display progress */
		printf("Progress:\t[%c][%3d%%]\r", cDisplay[(frameNo/2)%8], (int)(100.0f*percentage)); 

		/* test if finished */
		if (writeQMF)
		{
			testcondition = !addSamples_written;
		}
		else if (readQMF && !writeQMF)
		{
			testcondition = !(NumberSamplesWritten >= nTotalSamplesPerChannel - delay_to_compensate + addSamplesOrig - skipLastSamples);
		}
		else
		{
			testcondition = !(NumberSamplesWritten >= nTotalSamplesPerChannel);
		}

	} 
	/* loop while testcondition is fulfilled */
	while (testcondition == 1);



	/* after loop: test assertion */
	if (do_compensate_delay == 1)
	{
		if (!writeQMF && !readQMF)
		{
			assert(nTotalSamplesPerChannel == NumberSamplesWritten); 
		}
		else
		{
			assert(nTotalSamplesPerChannel - processingDelay + addSamplesOrig - skipLastSamples == NumberSamplesWritten); 
		} 
	}

	/* ****************************** FREE MEMORY ******************************** */
free_memory:

	if (handle_error == 0)
	{
		for (band = 0; band < numband_proc; band++)
		{    
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				free(temp_output_QMF_real[band][chOut]); temp_output_QMF_real[band][chOut] = NULL;
				free(temp_output_QMF_imag[band][chOut]); temp_output_QMF_imag[band][chOut] = NULL;
			}
			free(temp_output_QMF_real[band]); temp_output_QMF_real[band] = NULL;
			free(temp_output_QMF_imag[band]); temp_output_QMF_imag[band] = NULL;
		}
		free(temp_output_QMF_real); temp_output_QMF_real = NULL;
		free(temp_output_QMF_imag); temp_output_QMF_imag = NULL;


		if (add_DE_delay > 0)
		{
			for (band = 0; band < numband_proc; band++)
			{  
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					free(overlap_real[band][chOut]); overlap_real[band][chOut] = NULL;
					free(overlap_imag[band][chOut]); overlap_imag[band][chOut] = NULL;
				}
				free(overlap_real[band]); overlap_real[band] = NULL;
				free(overlap_imag[band]); overlap_imag[band] = NULL;
			}
			free(overlap_real); overlap_real = NULL;
			free(overlap_imag); overlap_imag = NULL;
		}

		/* destroy buffers for VOFF(D&E) */
		for (band = 0; band < numband_conv; band++)
		{
			int blk;
			int num_block = h_BRIRmetadata->N_BLK[band];
			for ( blk = 0; blk < num_block; blk++)
			{
				free(buf_DE_BLK_left_real[band][blk]); buf_DE_BLK_left_real[band][blk] = NULL;
				free(buf_DE_BLK_left_imag[band][blk]); buf_DE_BLK_left_imag[band][blk] = NULL;
				free(buf_DE_BLK_right_real[band][blk]); buf_DE_BLK_right_real[band][blk] = NULL;
				free(buf_DE_BLK_right_imag[band][blk]); buf_DE_BLK_right_imag[band][blk] = NULL;
			}
			free(buf_DE_BLK_left_real[band]); buf_DE_BLK_left_real[band] = NULL;
			free(buf_DE_BLK_left_imag[band]); buf_DE_BLK_left_imag[band] = NULL;
			free(buf_DE_BLK_right_real[band]); buf_DE_BLK_right_real[band] = NULL;
			free(buf_DE_BLK_right_imag[band]); buf_DE_BLK_right_imag[band] = NULL;
		}
		free(buf_DE_BLK_left_real); buf_DE_BLK_left_real = NULL;
		free(buf_DE_BLK_left_imag); buf_DE_BLK_left_imag = NULL;
		free(buf_DE_BLK_right_real); buf_DE_BLK_right_real = NULL;
		free(buf_DE_BLK_right_imag); buf_DE_BLK_right_imag = NULL;
		for (band = 0; band < numband_conv; band++)
		{
			free(buf_DE_FRM_left_real[band]); buf_DE_FRM_left_real[band] = NULL;
			free(buf_DE_FRM_left_imag[band]); buf_DE_FRM_left_imag[band] = NULL;
			free(buf_DE_FRM_right_real[band]); buf_DE_FRM_right_real[band] = NULL;
			free(buf_DE_FRM_right_imag[band]); buf_DE_FRM_right_imag[band] = NULL;
		}
		free(buf_DE_FRM_left_real); buf_DE_FRM_left_real = NULL;
		free(buf_DE_FRM_left_imag); buf_DE_FRM_left_imag = NULL;
		free(buf_DE_FRM_right_real); buf_DE_FRM_right_real = NULL;
		free(buf_DE_FRM_right_imag); buf_DE_FRM_right_imag = NULL;

		/* Destroy TDL buffer */
		if (TDL == 1)
		{
			for(band=0; band< numband_proc - numband_conv; band++)
			{
				for (chIn = 0; chIn < (int)nInChannels; chIn++)
				{
					free(buf_TDL_left_real[band][chIn]); buf_TDL_left_real[band][chIn] = NULL;
					free(buf_TDL_left_imag[band][chIn]); buf_TDL_left_imag[band][chIn] = NULL;
					free(buf_TDL_right_real[band][chIn]); buf_TDL_right_real[band][chIn] = NULL;
					free(buf_TDL_right_imag[band][chIn]); buf_TDL_right_imag[band][chIn] = NULL;
				}
				free(buf_TDL_left_real[band]); buf_TDL_left_real[band] = NULL;
				free(buf_TDL_left_imag[band]); buf_TDL_left_imag[band] = NULL;
				free(buf_TDL_right_real[band]); buf_TDL_right_real[band] = NULL;
				free(buf_TDL_right_imag[band]); buf_TDL_right_imag[band] = NULL;
			}
			free(buf_TDL_left_real); buf_TDL_left_real = NULL;
			free(buf_TDL_left_imag); buf_TDL_left_imag = NULL;
			free(buf_TDL_right_real); buf_TDL_right_real = NULL;
			free(buf_TDL_right_imag); buf_TDL_right_imag = NULL;
		}

		/* destroy dmx matrix */
		if (!just_DE)
		{
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				free(dmxmtx[chOut]); dmxmtx[chOut] = NULL; 
			}
			free(dmxmtx); dmxmtx = NULL; 
		}

		/* **************************** DESTROY FFT BUFFERS ******************************** */
		for (band = 0; band < numband_conv; band++)
		{    
			for (chIn = 0; chIn < (int)nInChannels; chIn++)
			{      
				for (k=0; k< h_BRIRmetadata->N_FRM[band]; k++) 
				{
					free(input_audio_FFT_real[band][chIn][k]); input_audio_FFT_real[band][chIn][k] = NULL;
					free(input_audio_FFT_imag[band][chIn][k]); input_audio_FFT_imag[band][chIn][k] = NULL;
				}
				free(input_audio_FFT_real[band][chIn]); input_audio_FFT_real[band][chIn] = NULL;
				free(input_audio_FFT_imag[band][chIn]); input_audio_FFT_imag[band][chIn] = NULL;
			}
			free(input_audio_FFT_real[band]); input_audio_FFT_real[band] = NULL;
			free(input_audio_FFT_imag[band]); input_audio_FFT_imag[band] = NULL;
		}
		free(input_audio_FFT_real); input_audio_FFT_real = NULL;
		free(input_audio_FFT_imag); input_audio_FFT_imag = NULL;

		if (!just_DE)
		{
			/* destroy downmix buffer */
			for(k = 0; k < binaural_numTimeslots; k++ )
			{
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					free(output_dmx_QMF_real[k][chOut]); output_dmx_QMF_real[k][chOut] = NULL; 		
					free(output_dmx_QMF_imag[k][chOut]); output_dmx_QMF_imag[k][chOut] = NULL; 		
				}
				free(output_dmx_QMF_real[k]); output_dmx_QMF_real[k] = NULL; 		
				free(output_dmx_QMF_imag[k]); output_dmx_QMF_imag[k] = NULL; 		
			}
			free(output_dmx_QMF_real ); output_dmx_QMF_real = NULL; 
			free(output_dmx_QMF_imag ); output_dmx_QMF_imag = NULL; 	
		}

		/* ****************************** DESTROY REVERB BUFFERS ******************************** */
		if (!just_DE)
		{
			/* destroy reverb buffer */
			for (k = 0; k < max_startReverbQMF; k++)
			{    
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{
					free(h_reverb_struct->buffer_rev_re[k][chOut]); h_reverb_struct->buffer_rev_re[k][chOut] = NULL; 
					free(h_reverb_struct->buffer_rev_im[k][chOut]); h_reverb_struct->buffer_rev_im[k][chOut] = NULL; 
				}
				free(h_reverb_struct->buffer_rev_re[k]); h_reverb_struct->buffer_rev_re[k] = NULL; 
				free(h_reverb_struct->buffer_rev_im[k]); h_reverb_struct->buffer_rev_im[k] = NULL; 
			}

			free(h_reverb_struct->buffer_rev_im); h_reverb_struct->buffer_rev_im = NULL; 
			free(h_reverb_struct->buffer_rev_re); h_reverb_struct->buffer_rev_re = NULL; 

			free(h_reverb_struct->scale_old); h_reverb_struct->scale_old = NULL;
			free(h_reverb_struct->scale); h_reverb_struct->scale = NULL;

			free(h_reverb_struct->startReverbQMF); h_reverb_struct->startReverbQMF = NULL;
		}

		/* ****************************** EXIT QMF ******************************** */
		/* input QMF */
		for(k = 0; k <  binaural_numTimeslots; k++ )
		{
			for (chIn = 0; chIn < (int)nInChannels; chIn++)
			{
				free(input_audio_QMF_real[k][chIn]); input_audio_QMF_real[k][chIn] = NULL; 		
				free(input_audio_QMF_imag[k][chIn]); input_audio_QMF_imag[k][chIn] = NULL; 		
			}
			free(input_audio_QMF_real[k]); input_audio_QMF_real[k] = NULL; 		
			free(input_audio_QMF_imag[k]); input_audio_QMF_imag[k] = NULL; 		
		}
		free(input_audio_QMF_real ); input_audio_QMF_real = NULL; 
		free(input_audio_QMF_imag); input_audio_QMF_imag = NULL; 		

		/* output QMF */
		for(k = 0; k < binaural_numTimeslots; k++ )
		{
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				free(output_DE_QMF_real[k][chOut]); output_DE_QMF_real[k][chOut] = NULL; 		
				free(output_DE_QMF_imag[k][chOut]); output_DE_QMF_imag[k][chOut] = NULL; 
				free(output_DE_LR_QMF_real[k][chOut]); output_DE_LR_QMF_real[k][chOut] = NULL; 		
				free(output_DE_LR_QMF_imag[k][chOut]); output_DE_LR_QMF_imag[k][chOut] = NULL; 	
			}
			free(output_DE_QMF_real[k]); output_DE_QMF_real[k] = NULL; 		
			free(output_DE_QMF_imag[k]); output_DE_QMF_imag[k] = NULL; 
			free(output_DE_LR_QMF_real[k]); output_DE_LR_QMF_real[k] = NULL; 		
			free(output_DE_LR_QMF_imag[k]); output_DE_LR_QMF_imag[k] = NULL; 		
		}
		free(output_DE_QMF_real ); output_DE_QMF_real = NULL; 
		free(output_DE_QMF_imag ); output_DE_QMF_imag = NULL; 	
		free(output_DE_LR_QMF_real ); output_DE_LR_QMF_real = NULL; 
		free(output_DE_LR_QMF_imag ); output_DE_LR_QMF_imag = NULL; 	

		/* destroy analysis */
		if (!readQMF)
		{
			for (chIn = 0; chIn < (int)nInChannels; chIn++)
			{
				QMFlib_CloseAnaFilterbank(pAnalysisQmf[chIn]); 
			}
		}
		/* destroy synthesis */
		if (!writeQMF)
		{
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{
				QMFlib_CloseSynFilterbank(pSynthesisQmf[chOut]); 
			}
		}
		free(pAnalysisQmf ); pAnalysisQmf = NULL; 
		free(pSynthesisQmf ); pSynthesisQmf = NULL;    

		/* ****************************** EXIT ReverbGenerator ******************************** */
		if (!just_DE)
		{
			ReverbGenerator_close(&reverb); 
		}

	}

	if ((abs(handle_error) > 9) || (handle_error == 0))
	{
		free(m_conv); m_conv = NULL; 
		free(startReverb); startReverb = NULL;
		free(startReverbQMF); startReverbQMF = NULL;
		free(DE_filter_length); DE_filter_length = NULL;
	}

	if ((abs(handle_error) > 7) || (handle_error == 0))
	{
		binaural_close(h_BRIRmetadata, max_numblocks, handle_error); 

		/* destroy wavIO buffers */
		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			free(outBuffer[chOut]); outBuffer[chOut] = NULL; 
		}
		free(outBuffer); outBuffer = NULL; 

		for (chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			free(inBuffer[chIn]); inBuffer[chIn] = NULL; 
		}
		free(inBuffer); inBuffer = NULL; 

		if (LFE_channels != NULL)
		{
			free(LFE_channels); LFE_channels = NULL;
		}
		free(Audio_elevation); Audio_elevation = NULL;
		free(Audio_azimuth); Audio_azimuth = NULL;

	}

	if ((abs(handle_error) > 5) || (handle_error == 0))
	{
		/* ****************************** EXIT WAV IO ******************************** */
		if (writeQMF)
		{
			wavIO_error_close = wavIO_updateWavHeader(hWavIO_real, &nTotalSamplesWrittenPerChannel); 
			wavIO_error_close = wavIO_updateWavHeader(hWavIO_imag, &nTotalSamplesWrittenPerChannel);
			if (wavIO_error_close == 0)
			{
				fprintf(stderr, "\nOutput qmf files with base filename %s are written.\n", wavIO_outpath); 
			}
			wavIO_error_close = wavIO_close(hWavIO_real);
			wavIO_error_close = wavIO_close(hWavIO_imag);
			if (!readQMF)
			{
				wavIO_error_close = wavIO_close(hWavIO); 
			}
		}
		else
		{
			wavIO_error_close = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel); 
			if (wavIO_error_close == 0)
			{
				fprintf(stderr, "\nOutput file %s is written.\n", wavIO_outpath); 
			}
			if (readQMF)
			{
				wavIO_error_close = wavIO_close(hWavIO_real);
				wavIO_error_close = wavIO_close(hWavIO_imag);
			}
			wavIO_error_close = wavIO_close(hWavIO); 
		}
	}
	/* ----------------- EXIT -------------------- */
	return handle_error; 

}
void FdBinauralRendererParam2BRIRmetadata(BinauralRepresentation *pBinauralRepresentation, H_BRIRMETADATASTRUCT h_BRIRmetadata){

	int ch, band, idx=0;
	FdBinauralRendererParam *pFdBinauralRendererParam = pBinauralRepresentation->pFdBinauralRendererParam;
	h_BRIRmetadata->numband_conv = pFdBinauralRendererParam->kConv;
	h_BRIRmetadata->noBands_transition = pFdBinauralRendererParam->kMax;
	h_BRIRmetadata->initDelay = pFdBinauralRendererParam->dInit;
	h_BRIRmetadata->numLFE = pBinauralRepresentation->Setup_SpeakerConfig3d.numLFEs;
	h_BRIRmetadata->fs_BRIRs = pBinauralRepresentation->brirSamplingFrequency;

	h_BRIRmetadata->channels = pBinauralRepresentation->Setup_SpeakerConfig3d.numChannels;
	h_BRIRmetadata->azimuth = (float*) calloc(h_BRIRmetadata->channels, sizeof(float));
	h_BRIRmetadata->elevation = (float*) calloc(h_BRIRmetadata->channels, sizeof(float));
	h_BRIRmetadata->LFEchannels = (int*) calloc(h_BRIRmetadata->channels, sizeof(int));

	for(ch = 0; ch<pBinauralRepresentation->nBrirPairs; ch++)
	{
		if(pBinauralRepresentation->Setup_SpeakerConfig3d.pGeometry[ch].LFE == 0)
		{
			h_BRIRmetadata->azimuth[idx] = pBinauralRepresentation->Setup_SpeakerConfig3d.pGeometry[ch].Az;
			h_BRIRmetadata->elevation[idx] = pBinauralRepresentation->Setup_SpeakerConfig3d.pGeometry[ch].El;
			h_BRIRmetadata->LFEchannels[idx] = pBinauralRepresentation->Setup_SpeakerConfig3d.pGeometry[ch].LFE;
			idx++;

		}
	}
	
	h_BRIRmetadata->filterlength_fft = (int*) calloc(h_BRIRmetadata->noBands_transition, sizeof(int));
	h_BRIRmetadata->N_FFT = (int*) calloc(h_BRIRmetadata->noBands_transition, sizeof(int));
	h_BRIRmetadata->N_BLK = (int*) calloc(h_BRIRmetadata->noBands_transition, sizeof(int));
	h_BRIRmetadata->FFTdomain_left_real = (float****)calloc(h_BRIRmetadata->noBands_transition, sizeof(float***));
	h_BRIRmetadata->FFTdomain_left_imag = (float****)calloc(h_BRIRmetadata->noBands_transition, sizeof(float***));
	h_BRIRmetadata->FFTdomain_right_real = (float****)calloc(h_BRIRmetadata->noBands_transition, sizeof(float***));
	h_BRIRmetadata->FFTdomain_right_imag = (float****)calloc(h_BRIRmetadata->noBands_transition, sizeof(float***));

	for(band = 0; band<h_BRIRmetadata->noBands_transition; band++)
	{
		int blk;
		int num_block;
		h_BRIRmetadata->filterlength_fft[band] = pFdBinauralRendererParam->nFilter[band];
		h_BRIRmetadata->N_FFT[band] = (int) pow(2.0f, pFdBinauralRendererParam->nFft[band]);
		h_BRIRmetadata->N_BLK[band] = pFdBinauralRendererParam->nBlk[band];
		num_block = h_BRIRmetadata->N_BLK[band];
		h_BRIRmetadata->FFTdomain_left_real[band] = (float***)calloc(num_block, sizeof(float**));
		h_BRIRmetadata->FFTdomain_left_imag[band] = (float***)calloc(num_block, sizeof(float**));
		h_BRIRmetadata->FFTdomain_right_real[band] = (float***)calloc(num_block, sizeof(float**));
		h_BRIRmetadata->FFTdomain_right_imag[band] = (float***)calloc(num_block, sizeof(float**));
		for ( blk = 0; blk < num_block; blk++)
		{
			int chIn;
			h_BRIRmetadata->FFTdomain_left_real[band][blk] = (float**)calloc(h_BRIRmetadata->channels, sizeof(float*));
			h_BRIRmetadata->FFTdomain_left_imag[band][blk] = (float**)calloc(h_BRIRmetadata->channels, sizeof(float*));
			h_BRIRmetadata->FFTdomain_right_real[band][blk] = (float**)calloc(h_BRIRmetadata->channels, sizeof(float*));
			h_BRIRmetadata->FFTdomain_right_imag[band][blk] = (float**)calloc(h_BRIRmetadata->channels, sizeof(float*));
			for (chIn = 0; chIn < h_BRIRmetadata->channels; chIn ++)
			{
				int k;
				h_BRIRmetadata->FFTdomain_left_real[band][blk][chIn] = (float*)calloc(h_BRIRmetadata->N_FFT[band], sizeof(float));
				h_BRIRmetadata->FFTdomain_left_imag[band][blk][chIn] = (float*)calloc(h_BRIRmetadata->N_FFT[band], sizeof(float));
				h_BRIRmetadata->FFTdomain_right_real[band][blk][chIn] = (float*)calloc(h_BRIRmetadata->N_FFT[band], sizeof(float));
				h_BRIRmetadata->FFTdomain_right_imag[band][blk][chIn] = (float*)calloc(h_BRIRmetadata->N_FFT[band], sizeof(float));
				for (k = 0; k < h_BRIRmetadata->N_FFT[band]; k++)
				{
					h_BRIRmetadata->FFTdomain_left_real[band][blk][chIn][k] = pFdBinauralRendererParam->ppppVoffCoeffLeftReal[band][blk][chIn][k];
					h_BRIRmetadata->FFTdomain_left_imag[band][blk][chIn][k] = pFdBinauralRendererParam->ppppVoffCoeffLeftImag[band][blk][chIn][k];
					h_BRIRmetadata->FFTdomain_right_real[band][blk][chIn][k] = pFdBinauralRendererParam->ppppVoffCoeffRightReal[band][blk][chIn][k];
					h_BRIRmetadata->FFTdomain_right_imag[band][blk][chIn][k] = pFdBinauralRendererParam->ppppVoffCoeffRightImag[band][blk][chIn][k];
				}
			}			  
		}
	}

	h_BRIRmetadata->LateReverb_AnalysisBands = pFdBinauralRendererParam->kAna;

	h_BRIRmetadata->LateReverb_fc = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands, sizeof(float));
	h_BRIRmetadata->LateReverb_RT60 = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands, sizeof(float));
	h_BRIRmetadata->LateReverb_energy = (float*)calloc(h_BRIRmetadata->LateReverb_AnalysisBands, sizeof(float));
	for( band = 0; band < h_BRIRmetadata->LateReverb_AnalysisBands; band++)
	{
		h_BRIRmetadata->LateReverb_fc[band] = pFdBinauralRendererParam->fcAna[band];
		h_BRIRmetadata->LateReverb_RT60[band] = pFdBinauralRendererParam->rt60[band];
		h_BRIRmetadata->LateReverb_energy[band] = pFdBinauralRendererParam->nrgLr[band];
	}

	h_BRIRmetadata->TDL_gain_left_real = (float**)calloc(h_BRIRmetadata->noBands_transition, sizeof(float*));
	h_BRIRmetadata->TDL_gain_left_imag = (float**)calloc(h_BRIRmetadata->noBands_transition, sizeof(float*));
	h_BRIRmetadata->TDL_gain_right_real = (float**)calloc(h_BRIRmetadata->noBands_transition, sizeof(float*));
	h_BRIRmetadata->TDL_gain_right_imag = (float**)calloc(h_BRIRmetadata->noBands_transition, sizeof(float*));
	h_BRIRmetadata->TDL_delay_left = (int**)calloc(h_BRIRmetadata->noBands_transition, sizeof(int*));
	h_BRIRmetadata->TDL_delay_right = (int**)calloc(h_BRIRmetadata->noBands_transition, sizeof(int*));

	for(band = 0; band<h_BRIRmetadata->noBands_transition-h_BRIRmetadata->numband_conv; band++)
	{
		h_BRIRmetadata->TDL_gain_left_real[band] = (float*)calloc(h_BRIRmetadata->channels, sizeof(float));
		h_BRIRmetadata->TDL_gain_left_imag[band] = (float*)calloc(h_BRIRmetadata->channels, sizeof(float));
		h_BRIRmetadata->TDL_gain_right_real[band] = (float*)calloc(h_BRIRmetadata->channels, sizeof(float));
		h_BRIRmetadata->TDL_gain_right_imag[band] = (float*)calloc(h_BRIRmetadata->channels, sizeof(float));
		h_BRIRmetadata->TDL_delay_left[band] = (int*)calloc(h_BRIRmetadata->channels, sizeof(int));
		h_BRIRmetadata->TDL_delay_right[band] = (int*)calloc(h_BRIRmetadata->channels, sizeof(int));
		for(ch = 0; ch<h_BRIRmetadata->channels; ch++)
		{
			h_BRIRmetadata->TDL_gain_left_real[band][ch]  = pFdBinauralRendererParam->ppQtdlGainLeftReal[band][ch];
			h_BRIRmetadata->TDL_gain_left_imag[band][ch]  = pFdBinauralRendererParam->ppQtdlGainLeftImag[band][ch];
			h_BRIRmetadata->TDL_gain_right_real[band][ch] = pFdBinauralRendererParam->ppQtdlGainRightReal[band][ch];
			h_BRIRmetadata->TDL_gain_right_imag[band][ch] = pFdBinauralRendererParam->ppQtdlGainRightImag[band][ch];
			h_BRIRmetadata->TDL_delay_left[band][ch]	  = pFdBinauralRendererParam->ppQtdlLagLeft[band][ch];
			h_BRIRmetadata->TDL_delay_right[band][ch]	  = pFdBinauralRendererParam->ppQtdlLagRight[band][ch];
		}
	}


}