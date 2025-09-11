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

#include "fftlib.h"
#include "Binauralizer.h"       
#include "wavIO.h"                
#include "ReverbGenerator.h"
#include "cicp2geometry.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#define BUGFIX_FRAMESIZE_1024

static int binaural_ConvertIntToFloat(float *pfOutBuf, const char *pbInBuf, unsigned int nInBytes, unsigned int nBytesPerSample); 
static __inline int IsLittleEndian (void); 
static __inline short LittleEndian16 (short v); 
float ReverseFloat(const float inFloat ); 
static unsigned int BigEndian32 (char a, char b, char c, char d); 


/*----------PUBLIC-----------------*/


int binaural_getChannelPosData(int CICPIndex, char *filename, int nInChannels, float *Audio_azimuth, float *Audio_elevation, int **LFE_channels, int *num_LFE)
{

	CICP2GEOMETRY_CHANNEL_GEOMETRY myTable[32] = { 0 };
	int numChannels = 0;
	int numLFEs = 0;
	int error = 0;
	int cicpIndex = 0;
	int *LFE_temp = NULL;
	int ch = 0;
	int ct = 0;


	/* read geometry from file if file is given */
	if ( filename != '\0' ) 
	{
		error = cicp2geometry_get_geometry_from_file( filename, myTable, &numChannels, &numLFEs );
		if( error ) {
			cicpIndex = -1;
		}
		else {
			/* any setup*/
			cicpIndex = 0;
		}
	}
	else 
	{

		error = cicp2geometry_get_geometry_from_cicp( CICPIndex, myTable, &numChannels, &numLFEs);
		if (error) {
			fprintf(stderr, "ERROR in cicp2geometry_get_geometry_from_cicp() !\n");
		}
	}

	/* test channel number of input file and Channel config */
	if (nInChannels != (numChannels+numLFEs))
	{
		fprintf(stderr, "Error: Channel number of input data does not correspond to ChannelConfig.\n"); 
		error = -1;
	}

	LFE_temp = (int*)calloc(numLFEs,sizeof(int));
	ct = 0;

	if (error == 0)
	{
		for (ch = 0; ch < (numChannels+numLFEs); ch++)
		{
			Audio_azimuth[ch] = (float)myTable[ch].Az;
			Audio_elevation[ch] = (float)myTable[ch].El;
			if (myTable[ch].LFE == 1)
			{
				LFE_temp[ct] = ch;
				ct++;
			}
		}
		*num_LFE = numLFEs;
		*LFE_channels = LFE_temp;
		error = 0;
	}

	return error;
}


int binaural_getBRIRConfig(float *Audio_azimuth, float *Audio_elevation, H_BRIRMETADATASTRUCT h_BRIRmetadata, int *m_conv, int nInChannels, int num_LFE_audio, int *LFE_channels_audio)
{
	int ch,k,lfe;

	for (ch=0; ch < nInChannels; ch++)
	{
		float Aaz = Audio_azimuth[ch];
		float Ael = Audio_elevation[ch];
		int LFE_channel_audio = 0;
		m_conv[ch] = -2;

		/* check if Audio channel is LFE */
		if (num_LFE_audio > 0)
		{
			for (lfe = 0; lfe < num_LFE_audio; lfe++)
			{
				if (ch == LFE_channels_audio[lfe])
				{
					m_conv[ch] = -1;
					LFE_channel_audio = 1;
					break;
				}
			}
		}

		if (!LFE_channel_audio)
		{
			for (k = 0; k < h_BRIRmetadata->channels; k++)
			{
				float Baz = h_BRIRmetadata->azimuth[k];
				float Bel = h_BRIRmetadata->elevation[k];

				/* find corresponding BRIR */
				if ((Aaz == Baz) && (Ael == Bel) && (m_conv[ch] == -2))
				{
					m_conv[ch] = k;
				}
			}
		}
	}
	return 0;
}

int binaural_downmix_frame_qmfdomain(int timeslots, float ***input_real, float ***input_imag, float ***output_real, float ***output_imag, H_REVERBSTRUCT h_reverb_struct, int fs, float **dmxmtx, int nInChannels, int kmax)
{
	float P_in[QMFLIB_NUMBANDS] = { 0 }; 
	float P_out[QMFLIB_NUMBANDS] = { 0 }; 
	float factEQ[QMFLIB_NUMBANDS] = { 0 }; 
	int k, chOut, chIn, band;
	float scale_temp[2] = {0.0f}; 
	int inc_left, inc_right;
	float temp1, temp2;

	float smooth = 120*fs*1.0f;
	float smooth_fac = (float)exp(-1/(smooth/1000.0f/(QMFLIB_NUMBANDS*timeslots)));

	for (k = 0; k < timeslots; k++)
	{
		for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
		{
			memset(output_real[k][chOut], 0, QMFLIB_NUMBANDS*sizeof(float)); 
			memset(output_imag[k][chOut], 0, QMFLIB_NUMBANDS*sizeof(float)); 
		}
	}

	binaural_get_scaling(input_real, input_imag, dmxmtx, &inc_left, &inc_right, timeslots, nInChannels, scale_temp); 

	h_reverb_struct->scale[0] = (float)(smooth_fac*h_reverb_struct->scale_old[0] + (1-smooth_fac)*scale_temp[0]); 
	h_reverb_struct->scale[1] = (float)(smooth_fac*h_reverb_struct->scale_old[1] + (1-smooth_fac)*scale_temp[1]); 

	h_reverb_struct->scale_old[0] = h_reverb_struct->scale[0]; 
	h_reverb_struct->scale_old[1] = h_reverb_struct->scale[1]; 

	for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
	{
		memset(P_in, 0, QMFLIB_NUMBANDS*sizeof(float));
		memset(P_out, 0, QMFLIB_NUMBANDS*sizeof(float)); 
		memset(factEQ, 0, QMFLIB_NUMBANDS*sizeof(float)); 

		for (chIn = 0; chIn < nInChannels; chIn++)
		{
			for (band = 0; band < QMFLIB_NUMBANDS; band++)
			{
				for (k = 0; k < timeslots; k++)
				{
					temp1 = input_real[k][chIn][band] * dmxmtx[chOut][chIn];
					temp2 = input_imag[k][chIn][band] * dmxmtx[chOut][chIn];
					output_real[k][chOut][band] += temp1; 
					output_imag[k][chOut][band] += temp2; 
					P_in[band] += (float)(pow(sqrt(pow(temp1, 2)+pow(temp2, 2)), 2)); 
				}
			}
		}

		for (band = 0; band < QMFLIB_NUMBANDS; band++)
		{    
			for (k = 0; k < timeslots; k++)
			{
				P_out[band] += (float)(pow(sqrt(pow(output_real[k][chOut][band], 2) + pow(output_imag[k][chOut][band], 2)), 2)); 
			}

			factEQ[band] = (float)(sqrt(P_in[band]/(P_out[band]+pow(10.f, -20)))); 
			if ((factEQ[band] <= pow(10.f, -20)) || (P_in[band] <= pow(10.f, -20))|| (P_out[band] <= pow(10.f, -20)))
			{
				factEQ[band] = 1.0f; 
			}
			factEQ[band] = min(factEQ[band], 2.0f); 
			factEQ[band] = max(factEQ[band], 0.5f); 
			for (k = 0; k < timeslots; k++)
			{
				output_real[k][chOut][band] *= factEQ[band];
				output_imag[k][chOut][band] *= factEQ[band];
        if (band >= kmax)
        {
          output_real[k][chOut][band] = 0.0f;
				  output_imag[k][chOut][band] = 0.0f;
        }
			}
		}
	}

	return 0; 
}

int binaural_init_scaling(H_REVERBSTRUCT h_reverb_struct, float **dmxmtx, int nInChannels)
{

	int inc_left = 0, inc_right = 0; 
	int chIn;

	inc_left = 0; 
	inc_right = 0; 
	for (chIn = 0; chIn < nInChannels; chIn++)
	{
		if (dmxmtx[0][chIn] > 0)
		{
			inc_left++; 
		}
		if (dmxmtx[1][chIn] > 0)
		{
			inc_right++; 
		}
	}

	h_reverb_struct->scale_old[0] = (float)sqrt(inc_left); 
	h_reverb_struct->scale_old[1] = (float)sqrt(inc_right);

	return 0; 

}

int binaural_get_scaling(float*** input_real, float ***input_imag, float **dmxmtx, int *inc_left, int *inc_right, int timeslots, int channels, float *scale)
{
	float l_corr_mean = 0, r_corr_mean = 0; 
	int chIn, band, k; 
	float l_uncorrf, r_uncorrf, l_corrf, r_corrf, dist_l, dist_r; 
	float *max_data; 
	int left = 0, right = 0;
	float minv = 0.0f; 


	max_data = (float*)calloc(channels, sizeof(float)); 

	for (chIn = 0; chIn < channels; chIn++)
	{
		for (k = 0; k < timeslots; k++)
		{
			for (band = 0; band < QMFLIB_NUMBANDS; band++)
			{
				max_data[chIn] = max(max_data[chIn], (float)fabs(input_real[k][chIn][band])); 
				max_data[chIn] = max(max_data[chIn], (float)fabs(input_imag[k][chIn][band])); 
			}
		}
		if ((max_data[chIn] > minv) && (dmxmtx[0][chIn] > 0)) 
		{
			left++;
		}
		if ((max_data[chIn] > minv) && (dmxmtx[1][chIn] > 0)) 
		{
			right++; 
		}
	}

	l_corr_mean = 0.5f;
	r_corr_mean = 0.5f;

	l_uncorrf = (float)sqrt(left); 
	r_uncorrf = (float)sqrt(right);

	l_corrf = (float)left;
	r_corrf = (float)right;

	dist_l = l_corrf - l_uncorrf; 
	dist_r = r_corrf - r_uncorrf; 

	scale[0] = l_uncorrf + l_corr_mean*dist_l; 
	scale[1] = r_uncorrf + r_corr_mean*dist_r; 


	free(max_data); max_data = NULL; 

	*inc_left = left;
	*inc_right = right;

	return 0; 

}


/*
Variable Order Filtering in Frequency domain
*/
void binaural_VOFF(float*** output_DE_QMF_real, float*** output_DE_QMF_imag, float**** input_audio_FFT_real, float**** input_audio_FFT_imag,
				   float*** buf_DE_BLK_left_real, float*** buf_DE_BLK_left_imag, float*** buf_DE_BLK_right_real, float*** buf_DE_BLK_right_imag, 
				   float** buf_DE_FRM_left_real, float** buf_DE_FRM_left_imag, float** buf_DE_FRM_right_real, float** buf_DE_FRM_right_imag, 
				   H_BRIRMETADATASTRUCT h_BRIRmetadata, int* m_conv, int binaural_numTimeslots, unsigned int nInChannels, int numband_conv)
{
	int band, k;

	for( band = 0; band < numband_conv; band++ ) /* Bandwise */
	{	
		/* Initialization */
		float* temp_zeros = NULL;
		int block = 0, max_block = h_BRIRmetadata->N_BLK[band];
		int idxbrir = 0;
		int frm = 0;
		int min_idx = 0;
		int chIn = 0;

		float *tmpPtr_left_real = NULL, *tmpPtr_left_imag = NULL, *tmpPtr_right_real = NULL, *tmpPtr_right_imag = NULL;
		float *tmpPtr_input_real = NULL, *tmpPtr_input_imag = NULL;
		float *temp_real_real_out = NULL, *temp_real_imag_out = NULL;
		float *temp_imag_real_out = NULL, *temp_imag_imag_out = NULL;
		int fftlen = h_BRIRmetadata->N_FFT[band];
		int framelength = (h_BRIRmetadata->N_FFT[band])>>1;
		int max_frame = h_BRIRmetadata->N_FRM[band];
		float *tmpOutput_left_real = NULL, *tmpOutput_left_imag = NULL, *tmpOutput_right_real = NULL, *tmpOutput_right_imag = NULL;
		HANDLE_FFTLIB h_IFFT = NULL;

		temp_real_real_out = (float*) calloc(fftlen, sizeof(float));		 
		temp_real_imag_out = (float*) calloc(fftlen, sizeof(float));	
		temp_imag_real_out = (float*) calloc(fftlen, sizeof(float));		  
		temp_imag_imag_out = (float*) calloc(fftlen, sizeof(float));

		tmpOutput_left_real = (float*) calloc(binaural_numTimeslots, sizeof(float));  
		tmpOutput_left_imag = (float*) calloc(binaural_numTimeslots, sizeof(float));  
		tmpOutput_right_real = (float*) calloc(binaural_numTimeslots, sizeof(float)); 
		tmpOutput_right_imag = (float*) calloc(binaural_numTimeslots, sizeof(float)); 
		
    /* FFT Initialization */
		FFTlib_Create(&h_IFFT, fftlen, -1); 

		for ( frm = 0; frm < max_frame; frm++) /* Frame */
		{
      if (max_block > 1)
      {
			  /* Buffer shifting for block-wise Fast convolution */
			  /* Flushing first block buffer  */
			  free(buf_DE_BLK_left_real[band][0]);	buf_DE_BLK_left_real[band][0] = NULL;	
			  free(buf_DE_BLK_left_imag[band][0]);	buf_DE_BLK_left_imag[band][0] = NULL;		
			  free(buf_DE_BLK_right_real[band][0]);	buf_DE_BLK_right_real[band][0] = NULL;			
			  free(buf_DE_BLK_right_imag[band][0]);	buf_DE_BLK_right_imag[band][0] = NULL;		
			  /* Buffer shift */
			  for ( block = 0; block < max_block-1; block++)
			  {
				  buf_DE_BLK_left_real[band][block] = (float*) &(buf_DE_BLK_left_real[band][block+1][0]);
				  buf_DE_BLK_left_imag[band][block] = (float*) &(buf_DE_BLK_left_imag[band][block+1][0]);
				  buf_DE_BLK_right_real[band][block] = (float*) &(buf_DE_BLK_right_real[band][block+1][0]);
				  buf_DE_BLK_right_imag[band][block] = (float*) &(buf_DE_BLK_right_imag[band][block+1][0]);
			  }
			  /* Initialization of last block buffer */
			  buf_DE_BLK_left_real[band][max_block-1] = (float*)calloc(fftlen, sizeof(float));
			  buf_DE_BLK_left_imag[band][max_block-1] = (float*)calloc(fftlen, sizeof(float));			
			  buf_DE_BLK_right_real[band][max_block-1] = (float*)calloc(fftlen, sizeof(float));
			  buf_DE_BLK_right_imag[band][max_block-1] = (float*)calloc(fftlen, sizeof(float));
      }
      else
      {
        memset(buf_DE_BLK_left_real[band][max_block-1],0,fftlen*sizeof(float));
        memset(buf_DE_BLK_left_imag[band][max_block-1],0,fftlen*sizeof(float));
        memset(buf_DE_BLK_right_real[band][max_block-1],0,fftlen*sizeof(float));
        memset(buf_DE_BLK_right_imag[band][max_block-1],0,fftlen*sizeof(float));
      }
			for( chIn = 0; chIn < (int)nInChannels; chIn++) /* Channel */
			{
				idxbrir = m_conv[chIn];
				if( idxbrir != -1 ) /* LFE Check! - Not consider LFE */
				{
					tmpPtr_input_real = (float*) &(input_audio_FFT_real[band][chIn][frm][0]); /* Note ) input_audio_FFT_real[band][chIn][block][fftbin] */
					tmpPtr_input_imag = (float*) &(input_audio_FFT_imag[band][chIn][frm][0]);
					for ( block = 0; block < max_block; block++) /* Block */
					{
						/* Multiplication and add */	
						/* Left */
						tmpPtr_left_real = (float*) &(h_BRIRmetadata->FFTdomain_left_real[band][block][idxbrir][0]); /* Note )  h_BRIRmetadata->FFTdomain_left_real[band][chOut][block][k] */
						tmpPtr_left_imag = (float*) &(h_BRIRmetadata->FFTdomain_left_imag[band][block][idxbrir][0]);
						for ( k = 0; k < fftlen; k++)
						{
							buf_DE_BLK_left_real[band][block][k] += tmpPtr_input_real[k] * tmpPtr_left_real[k] - tmpPtr_input_imag[k] * tmpPtr_left_imag[k]; 
							buf_DE_BLK_left_imag[band][block][k] += tmpPtr_input_real[k] * tmpPtr_left_imag[k] + tmpPtr_input_imag[k] * tmpPtr_left_real[k];
						}
						/* Right */
						tmpPtr_right_real = (float*) &(h_BRIRmetadata->FFTdomain_right_real[band][block][idxbrir][0]);
						tmpPtr_right_imag = (float*) &(h_BRIRmetadata->FFTdomain_right_imag[band][block][idxbrir][0]);
						for ( k = 0; k < fftlen; k++)
						{
							buf_DE_BLK_right_real[band][block][k] += tmpPtr_input_real[k] * tmpPtr_right_real[k] - tmpPtr_input_imag[k] * tmpPtr_right_imag[k];
							buf_DE_BLK_right_imag[band][block][k] += tmpPtr_input_real[k] * tmpPtr_right_imag[k] + tmpPtr_input_imag[k] * tmpPtr_right_real[k];
						}
					}
				}				
			}
			/* ----------------  iFFT transform of result ------------------ */
			/* 
			/* note that the inverse FFT needs normalization and the sign of the imaginary part needs to be flipped 
			/* output is written to output_DE_QMF_real and output_DE_QMF_imag of size [64 x 2 x bands] */
			temp_zeros = (float*) calloc(fftlen, sizeof(float)); 

			min_idx = (binaural_numTimeslots >= frm*framelength+fftlen) ? fftlen : frm*framelength+fftlen - binaural_numTimeslots;

			/* Inverse FFT */
			FFTlib_Apply(h_IFFT, buf_DE_BLK_left_real[band][0], temp_zeros, temp_real_real_out, temp_real_imag_out); /* IFFT for real */
			FFTlib_Apply(h_IFFT, buf_DE_BLK_left_imag[band][0], temp_zeros, temp_imag_real_out, temp_imag_imag_out); /* IFFT for imag */

			for(k = 0; k < min_idx; k++)
			{
				int idx = frm*framelength+k ;
				tmpOutput_left_real[idx] += (       temp_real_real_out[k] + 1.f * temp_imag_imag_out[k] ) / (float)fftlen; 
				tmpOutput_left_imag[idx] += (-1.f * temp_real_imag_out[k] +       temp_imag_real_out[k] ) / (float)fftlen; 
			}
			/* Store to buffer */
			for( k = min_idx; k < fftlen; k++)
			{
				int idx2 = frm*framelength+k - binaural_numTimeslots;
				tmpOutput_left_real[idx2] += buf_DE_FRM_left_real[band][idx2];
				tmpOutput_left_imag[idx2] += buf_DE_FRM_left_imag[band][idx2];
				buf_DE_FRM_left_real[band][idx2] =  (       temp_real_real_out[k] + 1.f * temp_imag_imag_out[k] ) / (float)fftlen; 
				buf_DE_FRM_left_imag[band][idx2] =  (-1.f * temp_real_imag_out[k] +       temp_imag_real_out[k] ) / (float)fftlen; 
			}

			/* Inverse FFT */
			FFTlib_Apply(h_IFFT, buf_DE_BLK_right_real[band][0], temp_zeros, temp_real_real_out, temp_real_imag_out); /* IFFT for real */
			FFTlib_Apply(h_IFFT, buf_DE_BLK_right_imag[band][0], temp_zeros, temp_imag_real_out, temp_imag_imag_out); /* IFFT for imag */
			
			for(k = 0; k < min_idx; k++)
			{
				int idx = frm*framelength+k;
				tmpOutput_right_real[idx] += (       temp_real_real_out[k] + 1.f * temp_imag_imag_out[k] ) / (float)fftlen; 
				tmpOutput_right_imag[idx] += (-1.f * temp_real_imag_out[k] +       temp_imag_real_out[k] ) / (float)fftlen; 
			}
			/* Store to buffer */
			for( k = min_idx; k < fftlen; k++)
			{
				int idx2 = frm*framelength+k - binaural_numTimeslots;
				tmpOutput_right_real[idx2] += buf_DE_FRM_right_real[band][idx2];
				tmpOutput_right_imag[idx2] += buf_DE_FRM_right_imag[band][idx2];
				buf_DE_FRM_right_real[band][idx2] =  (       temp_real_real_out[k] + 1.f * temp_imag_imag_out[k] ) / (float)fftlen; 
				buf_DE_FRM_right_imag[band][idx2] =  (-1.f * temp_real_imag_out[k] +       temp_imag_real_out[k] ) / (float)fftlen; 
			}
		}
		for( k = 0; k < binaural_numTimeslots; k++)
		{
			output_DE_QMF_real[k][0][band] = tmpOutput_left_real[k];
			output_DE_QMF_imag[k][0][band] = tmpOutput_left_imag[k];
			output_DE_QMF_real[k][1][band] = tmpOutput_right_real[k];
			output_DE_QMF_imag[k][1][band] = tmpOutput_right_imag[k];
		}

    FFTlib_Destroy(&h_IFFT); /* FFT Lib free */
		/* Memory Free */
		free(temp_zeros); temp_zeros = NULL;
		free(temp_real_real_out); temp_real_real_out = NULL;
		free(temp_real_imag_out); temp_real_imag_out = NULL;
		free(temp_imag_real_out); temp_imag_real_out = NULL;
		free(temp_imag_imag_out); temp_imag_imag_out = NULL;

		free(tmpOutput_left_real); tmpOutput_left_real = NULL;  
		free(tmpOutput_left_imag); tmpOutput_left_imag = NULL;  
		free(tmpOutput_right_real); tmpOutput_right_real = NULL;  
		free(tmpOutput_right_imag); tmpOutput_right_imag = NULL;  

		/* Pointer Free */
		tmpPtr_left_real = NULL, tmpPtr_left_imag = NULL, tmpPtr_right_real = NULL, tmpPtr_right_imag = NULL;
		tmpPtr_input_real = NULL, tmpPtr_input_imag = NULL;
	}
}

void binaural_QTDL(float*** output_DE_QMF_real, float*** output_DE_QMF_imag, float*** input_audio_QMF_real, float*** input_audio_QMF_imag,
				   float*** buf_TDL_left_real, float*** buf_TDL_left_imag, float*** buf_TDL_right_real, float*** buf_TDL_right_imag, 
				   H_BRIRMETADATASTRUCT h_BRIRmetadata, int* m_conv, int binaural_numTimeslots, unsigned int nInChannels, int numband_conv, int numband_proc)
{
	int k, band;
	for (band = numband_conv ; band < numband_proc; band++)
	{
		int chIn;
		int band_TDL = band - numband_conv;
		float* tmp_TDL_left_real = (float*)calloc(binaural_numTimeslots, sizeof(float));
		float* tmp_TDL_left_imag = (float*)calloc(binaural_numTimeslots, sizeof(float));
		float* tmp_TDL_right_real = (float*)calloc(binaural_numTimeslots, sizeof(float));
		float* tmp_TDL_right_imag = (float*)calloc(binaural_numTimeslots, sizeof(float));
		for ( chIn = 0; chIn < (int)nInChannels; chIn++)
		{
			int idxbrir = m_conv[chIn];
			if ( idxbrir != -1)
			{
				int delay_left = h_BRIRmetadata->TDL_delay_left[band_TDL][idxbrir];
				float gain_left_real = h_BRIRmetadata->TDL_gain_left_real[band_TDL][idxbrir];
				float gain_left_imag = h_BRIRmetadata->TDL_gain_left_imag[band_TDL][idxbrir];
				int delay_right = h_BRIRmetadata->TDL_delay_right[band_TDL][idxbrir];
				float gain_right_real = h_BRIRmetadata->TDL_gain_right_real[band_TDL][idxbrir];
				float gain_right_imag = h_BRIRmetadata->TDL_gain_right_imag[band_TDL][idxbrir];
#ifdef BUGFIX_FRAMESIZE_1024
				for ( k = 0; k < binaural_numTimeslots; k++)
				{
					/* left */
					tmp_TDL_left_real[k] += buf_TDL_left_real[band_TDL][chIn][k];
					tmp_TDL_left_imag[k] += buf_TDL_left_imag[band_TDL][chIn][k];
					buf_TDL_left_real[band_TDL][chIn][delay_left+k] = gain_left_real * input_audio_QMF_real[k][chIn][band] - gain_left_imag * input_audio_QMF_imag[k][chIn][band];
					buf_TDL_left_imag[band_TDL][chIn][delay_left+k] = gain_left_real * input_audio_QMF_imag[k][chIn][band] + gain_left_imag * input_audio_QMF_real[k][chIn][band];
				}
				for ( k = 0; k < delay_left; k++)
				{
					buf_TDL_left_real[band_TDL][chIn][k] = buf_TDL_left_real[band_TDL][chIn][binaural_numTimeslots+k];
					buf_TDL_left_imag[band_TDL][chIn][k] = buf_TDL_left_imag[band_TDL][chIn][binaural_numTimeslots+k];
				}
				
				for ( k = 0; k < binaural_numTimeslots; k++)
				{
					/* right */
					tmp_TDL_right_real[k] += buf_TDL_right_real[band_TDL][chIn][k];
					tmp_TDL_right_imag[k] += buf_TDL_right_imag[band_TDL][chIn][k];
					buf_TDL_right_real[band_TDL][chIn][delay_right+k] = gain_right_real * input_audio_QMF_real[k][chIn][band] - gain_right_imag * input_audio_QMF_imag[k][chIn][band];
					buf_TDL_right_imag[band_TDL][chIn][delay_right+k] = gain_right_real * input_audio_QMF_imag[k][chIn][band] + gain_right_imag * input_audio_QMF_real[k][chIn][band];
				}
				for ( k = 0; k < delay_right; k++)
				{
					buf_TDL_right_real[band_TDL][chIn][k] = buf_TDL_right_real[band_TDL][chIn][binaural_numTimeslots+k];
					buf_TDL_right_imag[band_TDL][chIn][k] = buf_TDL_right_imag[band_TDL][chIn][binaural_numTimeslots+k];
				}
#else
        for ( k = 0; k < binaural_numTimeslots - delay_left; k++ )
				{
          tmp_TDL_left_real[delay_left+k] += gain_left_real * input_audio_QMF_real[k][chIn][band] - gain_left_imag * input_audio_QMF_imag[k][chIn][band];
          tmp_TDL_left_imag[delay_left+k] += gain_left_real * input_audio_QMF_imag[k][chIn][band] + gain_left_imag * input_audio_QMF_real[k][chIn][band];
        }
        for ( k = binaural_numTimeslots - delay_left; k < binaural_numTimeslots; k++)
				{
          int idx = k-binaural_numTimeslots+delay_left;
          /* left */
          tmp_TDL_left_real[idx] += buf_TDL_left_real[band_TDL][chIn][idx];
          tmp_TDL_left_imag[idx] += buf_TDL_left_imag[band_TDL][chIn][idx];
          buf_TDL_left_real[band_TDL][chIn][idx] = gain_left_real * input_audio_QMF_real[k][chIn][band] - gain_left_imag * input_audio_QMF_imag[k][chIn][band];
          buf_TDL_left_imag[band_TDL][chIn][idx] = gain_left_real * input_audio_QMF_imag[k][chIn][band] + gain_left_imag * input_audio_QMF_real[k][chIn][band];
        }
				                
        for ( k = 0; k < binaural_numTimeslots - delay_right; k++)
        {
            tmp_TDL_right_real[delay_right+k] += gain_right_real * input_audio_QMF_real[k][chIn][band] - gain_right_imag * input_audio_QMF_imag[k][chIn][band];
            tmp_TDL_right_imag[delay_right+k] += gain_right_real * input_audio_QMF_imag[k][chIn][band] + gain_right_imag * input_audio_QMF_real[k][chIn][band];
        }
        for ( k = binaural_numTimeslots - delay_right; k < binaural_numTimeslots; k++)
        {
            int idx = k-binaural_numTimeslots+delay_right;
            /* right */
            tmp_TDL_right_real[idx] += buf_TDL_right_real[band_TDL][chIn][idx];
            tmp_TDL_right_imag[idx] += buf_TDL_right_imag[band_TDL][chIn][idx];
            buf_TDL_right_real[band_TDL][chIn][idx] = gain_right_real * input_audio_QMF_real[k][chIn][band] - gain_right_imag * input_audio_QMF_imag[k][chIn][band];
            buf_TDL_right_imag[band_TDL][chIn][idx] = gain_right_real * input_audio_QMF_imag[k][chIn][band] + gain_right_imag * input_audio_QMF_real[k][chIn][band];
        }

#endif
			}
		}
		for ( k=0; k < binaural_numTimeslots; k++)
		{
			output_DE_QMF_real[k][0][band] = tmp_TDL_left_real[k];
			output_DE_QMF_imag[k][0][band] = tmp_TDL_left_imag[k];
			output_DE_QMF_real[k][1][band] = tmp_TDL_right_real[k];
			output_DE_QMF_imag[k][1][band] = tmp_TDL_right_imag[k];
		}
		free(tmp_TDL_left_real); tmp_TDL_left_real = NULL;
		free(tmp_TDL_left_imag); tmp_TDL_left_real = NULL;
		free(tmp_TDL_right_real); tmp_TDL_left_real = NULL;
		free(tmp_TDL_right_imag); tmp_TDL_left_real = NULL;
	}
}


void binaural_close(H_BRIRMETADATASTRUCT h_BRIRmetadata, int max_numblocks, int handle_error)
{
	if (handle_error == 0)
	{
		if (h_BRIRmetadata->LateReverb_fc != NULL)
		{
			free(h_BRIRmetadata->LateReverb_fc); h_BRIRmetadata->LateReverb_fc = NULL;
			free(h_BRIRmetadata->LateReverb_RT60); h_BRIRmetadata->LateReverb_RT60 = NULL;
			free(h_BRIRmetadata->LateReverb_energy); h_BRIRmetadata->LateReverb_energy = NULL;
			free(h_BRIRmetadata->transition); h_BRIRmetadata->transition = NULL;

			free(h_BRIRmetadata->azimuth); h_BRIRmetadata->azimuth = NULL;
			free(h_BRIRmetadata->elevation); h_BRIRmetadata->elevation = NULL;
		}
		if (h_BRIRmetadata->numLFE > 0)
		{
			free(h_BRIRmetadata->LFEchannels); h_BRIRmetadata->LFEchannels = NULL;
		}
		if (h_BRIRmetadata->filterlength_fft != 0)
		{
			free(h_BRIRmetadata->filterlength_fft); h_BRIRmetadata->filterlength_fft = NULL;
		}
		if (h_BRIRmetadata->noBands_transition >0)
		{
			int band;			
			for ( band = 0; band < h_BRIRmetadata->noBands_transition ; band ++)
			{
				int blk;
				for ( blk = 0; blk < h_BRIRmetadata->N_BLK[band]; blk++)
				{
					int chIn;
					for ( chIn = 0; chIn < h_BRIRmetadata->channels; chIn++)
					{
						free(h_BRIRmetadata->FFTdomain_left_real[band][blk][chIn]); h_BRIRmetadata->FFTdomain_left_real[band][blk][chIn]= NULL;
						free(h_BRIRmetadata->FFTdomain_left_imag[band][blk][chIn]); h_BRIRmetadata->FFTdomain_left_imag[band][blk][chIn]= NULL;
						free(h_BRIRmetadata->FFTdomain_right_real[band][blk][chIn]); h_BRIRmetadata->FFTdomain_right_real[band][blk][chIn]= NULL;
						free(h_BRIRmetadata->FFTdomain_right_imag[band][blk][chIn]); h_BRIRmetadata->FFTdomain_right_imag[band][blk][chIn]= NULL;
					}
					free(h_BRIRmetadata->FFTdomain_left_real[band][blk]); h_BRIRmetadata->FFTdomain_left_real[band][blk] = NULL;
					free(h_BRIRmetadata->FFTdomain_left_imag[band][blk]); h_BRIRmetadata->FFTdomain_left_imag[band][blk] = NULL;
					free(h_BRIRmetadata->FFTdomain_right_real[band][blk]); h_BRIRmetadata->FFTdomain_right_real[band][blk] = NULL;
					free(h_BRIRmetadata->FFTdomain_right_imag[band][blk]); h_BRIRmetadata->FFTdomain_right_imag[band][blk] = NULL;
				}
				free(h_BRIRmetadata->FFTdomain_left_real[band]); h_BRIRmetadata->FFTdomain_left_real[band] = NULL;
				free(h_BRIRmetadata->FFTdomain_left_imag[band]); h_BRIRmetadata->FFTdomain_left_imag[band] = NULL;
				free(h_BRIRmetadata->FFTdomain_right_real[band]); h_BRIRmetadata->FFTdomain_right_real[band] = NULL;
				free(h_BRIRmetadata->FFTdomain_right_imag[band]); h_BRIRmetadata->FFTdomain_right_imag[band] = NULL;
			}
			free(h_BRIRmetadata->FFTdomain_left_real); h_BRIRmetadata->FFTdomain_left_real = NULL;
			free(h_BRIRmetadata->FFTdomain_left_imag); h_BRIRmetadata->FFTdomain_left_imag = NULL;
			free(h_BRIRmetadata->FFTdomain_right_real); h_BRIRmetadata->FFTdomain_right_real = NULL;
			free(h_BRIRmetadata->FFTdomain_right_imag); h_BRIRmetadata->FFTdomain_right_imag = NULL;
		}
		if (h_BRIRmetadata->noBands_transition >0)
		{
			int band;
			for ( band = 0; band < h_BRIRmetadata->noBands_transition - h_BRIRmetadata->numband_conv; band ++)
			{
				free(h_BRIRmetadata->TDL_gain_left_real[band]); h_BRIRmetadata->TDL_gain_left_real[band] = NULL;
				free(h_BRIRmetadata->TDL_gain_left_imag[band]); h_BRIRmetadata->TDL_gain_left_imag[band] = NULL;
				free(h_BRIRmetadata->TDL_gain_right_real[band]); h_BRIRmetadata->TDL_gain_right_real[band] = NULL;
				free(h_BRIRmetadata->TDL_gain_right_imag[band]); h_BRIRmetadata->TDL_gain_right_imag[band] = NULL;
				free(h_BRIRmetadata->TDL_delay_left[band]); h_BRIRmetadata->TDL_delay_left[band] = NULL;
				free(h_BRIRmetadata->TDL_delay_right[band]); h_BRIRmetadata->TDL_delay_right[band] = NULL;
			}
			free(h_BRIRmetadata->TDL_gain_left_real); h_BRIRmetadata->TDL_gain_left_real = NULL;
			free(h_BRIRmetadata->TDL_gain_left_imag); h_BRIRmetadata->TDL_gain_left_imag = NULL;
			free(h_BRIRmetadata->TDL_gain_right_real); h_BRIRmetadata->TDL_gain_right_real = NULL;
			free(h_BRIRmetadata->TDL_gain_right_imag); h_BRIRmetadata->TDL_gain_right_imag = NULL;
			free(h_BRIRmetadata->TDL_delay_left); h_BRIRmetadata->TDL_delay_left = NULL;
			free(h_BRIRmetadata->TDL_delay_right); h_BRIRmetadata->TDL_delay_right = NULL;
		}
		if (h_BRIRmetadata->noBands_transition >0)
		{
			free(h_BRIRmetadata->N_FFT); h_BRIRmetadata->N_FFT = NULL;
			free(h_BRIRmetadata->N_FRM); h_BRIRmetadata->N_FRM = NULL;
			free(h_BRIRmetadata->N_BLK); h_BRIRmetadata->N_BLK = NULL;
		}
	}

}


int binaural_calculate_reverb(void *reverb, H_REVERBSTRUCT h_reverb_struct, float ***dmxOutputQMF_real, float ***dmxOutputQMF_imag, float ***output_real, float ***output_imag, int maxband, int blocklength)
{
	int k, chOut, band, idxRot;
	int timeslots = 0;
	int m;

	timeslots = blocklength / QMFLIB_NUMBANDS;

	for (k = 0; k < timeslots; k++)
	{
		/* reverb */
		ReverbGenerator_process_sample(reverb, dmxOutputQMF_real[k], dmxOutputQMF_imag[k], h_reverb_struct->buffer_rev_re, h_reverb_struct->buffer_rev_im, h_reverb_struct); 

		for (band = 0; band < maxband; band++)
		{    
			/* get actual reverb_left sample from reverb_struct, add up outputs */

			output_real[band][0][k] += (h_reverb_struct->buffer_rev_re[0][0][band] * h_reverb_struct->scale[0]);
			output_imag[band][0][k] += (h_reverb_struct->buffer_rev_im[0][0][band] * h_reverb_struct->scale[0]); 
			output_real[band][1][k] += (h_reverb_struct->buffer_rev_re[0][1][band] * h_reverb_struct->scale[1]); 
			output_imag[band][1][k] += (h_reverb_struct->buffer_rev_im[0][1][band] * h_reverb_struct->scale[1]); 
		}


		/* rotate reverb_left delay buffer after every timeslot*/
		for (idxRot = 0; idxRot < h_reverb_struct->max_startReverbQMF-1; idxRot++)
		{
			for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
			{  
				memcpy(h_reverb_struct->buffer_rev_re[idxRot][chOut],h_reverb_struct->buffer_rev_re[idxRot+1][chOut], maxband*sizeof(float));
				memcpy(h_reverb_struct->buffer_rev_im[idxRot][chOut],h_reverb_struct->buffer_rev_im[idxRot+1][chOut], maxband*sizeof(float));
			}
		}


		for (band = 0; band < maxband; band++)
		{  
			for (m = h_reverb_struct->startReverbQMF[band]-1; m < h_reverb_struct->max_startReverbQMF; m++)
			{
				for (chOut = 0; chOut < BINAURAL_NUM_OUTCHANNELS; chOut++)
				{  
					h_reverb_struct->buffer_rev_re[m][chOut][band] = 0.f; 
					h_reverb_struct->buffer_rev_im[m][chOut][band] = 0.f; 
				}
			}
		}
	}


	return 0;
}

int binaural_qmftransform_frame(float **inBuffer, int binaural_numTimeslots, int binaural_hopsize, int nInChannels, QMFLIB_POLYPHASE_ANA_FILTERBANK **pAnalysisQmf, int ldMode, float ***inputBufferFreqQMF_real, float ***inputBufferFreqQMF_imag)
{
	int chIn, k, m; 
	float *blocktimeframe = (float*)calloc(binaural_hopsize, sizeof(float)); 

	for (chIn = 0; chIn < nInChannels; chIn++)
	{
		for (k = 0; k < binaural_numTimeslots; k++)
		{
			for (m = 0; m < binaural_hopsize; m++)
			{
				blocktimeframe[m] = inBuffer[chIn][k*binaural_hopsize+m]; 
			}
			QMFlib_CalculateAnaFilterbank(pAnalysisQmf[chIn], blocktimeframe, inputBufferFreqQMF_real[k][chIn], inputBufferFreqQMF_imag[k][chIn], ldMode); 
		}
	}

	free(blocktimeframe); blocktimeframe = NULL;
	return 0; 
}


int binaural_iqmftransform_frame(float **outBuffer, int binaural_numTimeslots, int binaural_hopsize, int nOutChannels, QMFLIB_POLYPHASE_SYN_FILTERBANK **pSynthesisQmf, int ldMode, float ***outputBufferFreqQMF_real, float ***outputBufferFreqQMF_imag)
{
	int chOut, k, m; 
	float *blocktimeframe = (float*)calloc(binaural_hopsize, sizeof(float)); 

	for (chOut = 0; chOut < nOutChannels; chOut++)
	{
		for (k = 0; k < binaural_numTimeslots; k++)
		{
			QMFlib_CalculateSynFilterbank(pSynthesisQmf[chOut], outputBufferFreqQMF_real[k][chOut], outputBufferFreqQMF_imag[k][chOut], blocktimeframe, ldMode); 

			for (m = 0; m < binaural_hopsize; m++)
			{
				outBuffer[chOut][k*binaural_hopsize+m] = blocktimeframe[m]; 
			}
		}
	}

	free(blocktimeframe); blocktimeframe = NULL;
	return 0; 
}

int binaural_FFTtransform_QMFframe(float ***input_audio_QMF_real, float ***input_audio_QMF_imag, float ****input_audio_FFT_real, float ****input_audio_FFT_imag, int numband_conv, int *N_FFT, int *N_FRM, int binaural_numTimeslots, int nInChannels)
{
	int band, frame, k = 0;
	int chIn = 0;
	HANDLE_FFTLIB h_FFT = NULL; 

	for (band = 0; band < numband_conv; band++)
	{
		int fftlength = N_FFT[band];
		int numframes = N_FRM[band];
		int framelength = binaural_numTimeslots/numframes;

		FFTlib_Create(&h_FFT, fftlength, -1);

		for (frame = 0; frame < numframes; frame++)
		{
			for (chIn = 0; chIn < nInChannels; chIn++)
			{
				float *temp_real_in, *temp_imag_in, *temp_real_out, *temp_imag_out;

				temp_real_in = (float*)calloc(fftlength, sizeof(float)); 
				temp_imag_in = (float*)calloc(fftlength, sizeof(float)); 
				temp_real_out = (float*)calloc(fftlength, sizeof(float)); 
				temp_imag_out = (float*)calloc(fftlength, sizeof(float)); 

				memset(input_audio_FFT_real[band][chIn][frame],0,fftlength*sizeof(float));
				memset(input_audio_FFT_imag[band][chIn][frame],0,fftlength*sizeof(float));

				for(k = 0; k < framelength; k++)
				{
					temp_real_in[k] = input_audio_QMF_real[frame*framelength+k][chIn][band]; 
					temp_imag_in[k] = input_audio_QMF_imag[frame*framelength+k][chIn][band]; 
				}
				FFTlib_Apply(h_FFT, temp_real_in, temp_imag_in, temp_real_out, temp_imag_out);
				for(k = 0; k < fftlength; k++)
				{
					input_audio_FFT_real[band][chIn][frame][k] = temp_real_out[k]; 
					input_audio_FFT_imag[band][chIn][frame][k] = temp_imag_out[k]; 
				}

				free(temp_real_in); temp_real_in = NULL;
				free(temp_imag_in); temp_imag_in = NULL;
				free(temp_real_out); temp_real_out = NULL;
				free(temp_imag_out); temp_imag_out = NULL;
			}
		}
		FFTlib_Destroy(&h_FFT);
	}
	return 0;
}




/*----------PRIVATE----------------*/


static int binaural_ConvertIntToFloat(float *pfOutBuf, const char *pbInBuf, unsigned int nInBytes, unsigned int nBytesPerSample)
{
	unsigned int i, j, nSamples, nOffset; 

	if ( nBytesPerSample == 4 ) {
		memcpy(pfOutBuf, pbInBuf, nInBytes*sizeof(char));
		return 0;
	}

	if (nBytesPerSample == sizeof(short))
	{
		short* shortBuf = (short*)pbInBuf; 
		for(j = 0; j < nInBytes/nBytesPerSample; j++)
			pfOutBuf[j] = ((float)LittleEndian16(shortBuf[j]))/32768.f; 
	}
	else if (nBytesPerSample > 2 )
	{

		union { signed long s; char c[sizeof(long)]; } u; 
		float fFactor = (float)(1 << (nBytesPerSample*8 - 1 )); 
		fFactor = 1.0f / fFactor; 

		u.s = 0; 

		nOffset = (sizeof(long)- nBytesPerSample)* 8; 

		nSamples = nInBytes / nBytesPerSample; 

		for(j = 0; j < nSamples; j++)
		{
			int n = j * nBytesPerSample; 

			/* convert chars to 32 bit */
			if (IsLittleEndian())
			{
				for (i = 0; i < nBytesPerSample; i++)
				{
					u.c[sizeof(long)- 1 - i] = pbInBuf[n + nBytesPerSample - i - 1]; 
				}
			} 
			else
			{
				for (i = 0; i < nBytesPerSample; i++)
				{
					u.c[i] = pbInBuf[n + nBytesPerSample - i - 1]; 
				}

			} 

			u.s = u.s >> nOffset; 

			pfOutBuf[j] = ((float)u.s)* fFactor; 

		}
	}
	return 0; 
}

static __inline int IsLittleEndian (void)
{
	short s = 0x01 ; 

	return *((char *)&s)? 1 : 0; 
}


static __inline short LittleEndian16 (short v)
{ /* get signed little endian (2-compl.)and returns in native format, signed */
	if (IsLittleEndian ())return v ; 

	else return ((v << 8)& 0xFF00)| ((v >> 8)& 0x00FF); 
}

float ReverseFloat(const float inFloat )
{
	float retVal; 
	char *floatToConvert = (char* )& inFloat; 
	char *returnFloat = (char* )& retVal; 

	/* swap the bytes into a temporary buffer */
	returnFloat[0] = floatToConvert[3]; 
	returnFloat[1] = floatToConvert[2]; 
	returnFloat[2] = floatToConvert[1]; 
	returnFloat[3] = floatToConvert[0]; 

	return retVal; 
}


static unsigned int BigEndian32 (char a, char b, char c, char d)
{
	if (IsLittleEndian ())
	{
		return (unsigned int)d << 24 |
			(unsigned int)c << 16 |
			(unsigned int)b <<  8 |
			(unsigned int)a ; 
	}
	else
	{
		return (unsigned int)a << 24 |
			(unsigned int)b << 16 |
			(unsigned int)c <<  8 |
			(unsigned int)d ; 
	}
}
