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

#include <math.h> /* for powf() and sqrtf() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "binauralization_FilterDesign.h"
#define FFT_VERSION2
#ifdef FFT_VERSION2
#include "fftlib.h" 
#else
#include "uhd_fft.h" 
#endif





/*------------------------------  Debug utilities ----------------------------------------------*/
void printBuf(float *buf, int size);
void printBuf(float *buf, int size)
{
	int ii;
	for (ii = 0; ii < size; ++ii)
	{
		printf("%4.2f ", buf[ii]);
	}
}



/*------------------------------------ Binaural Filter Design Functions ----------------------------------------*/


int computeFilterParams(
	TdBinauralRendererParam *pParam, 
	BinauralFirData *pFirData, 
	int nBrirPairs)
{
	/* Parameters */
	int limitHRTFshort = 1024;
	float BeginEnergyThld = -50;

	float DirectEnergyThld = -15;   /* original MPEG value  */

	float DiffuseEnergyThld = -19;
	float DiffuseFcThld = -20;

	/* Vars for loop indices etc. */
	int i_ear, i_chan, i_tap;

	int indBegin, indBeginMini, indDirect, indDirectMaxi, lenDirect, lenDiffus, indDiffuse,
		indDiffuseMaxi, diffuseMinLength, numDiffuseBlock, FFTsize, indStart, i_block,
		indNyquist, indFreq, indFreqMaxi, cpyLen, lenWin, numChannelWithZeroEnergy;

	float energy, energyMaxi, threshold, CSEinst, CSEinstInv, CSEinstInvFreq;

#ifdef _BUGFIX_DIRECT_FC_
	float DirectFcThld = -20;
	float pDirectBlock[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDirectBlockSpectrum_r[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDirectBlockSpectrum_i[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDirectBlockSpectrum_Mag[MAX_LENGTH_DIRECT_FILTER] = {0};
#endif

	float pDiffuseBlock[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDiffuseBlockSpectrum_r[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDiffuseBlockSpectrum_i[MAX_LENGTH_DIRECT_FILTER] = {0};
	float pDiffuseBlockSpectrum_Mag[MAX_LENGTH_DIRECT_FILTER] = {0};
#ifndef _BUGFIX_DIFFUSE_FC_
	float   *pDiffuseFc, *pDiffuseFilter;
#endif

	float *pDiffuseFilter;				/* diffuse filter (containing all blocks) ( pDiffuseFilter[tap] ) */
	float **ppDiffuseFilterWMC;			/* diffuse filters WMC (containing all blocks) ( ppDiffuseFilterWMC[ear][tap] ) */
	float ppEnergyDiffuseFilter[2][MAX_NUM_BRIR_PAIRS] = {{0}};   /* diffuse filter energies ( ppEnergyDiffuseFilter[ear][channel] ) */
	float ppDiffuseWeight[2][MAX_NUM_BRIR_PAIRS] = {{0}};         /* diffuse weights ( ppDiffuseWeight[ear][channel] ) */

#ifdef FFT_VERSION2
		/** handle to FFT */
		HANDLE_FFTLIB hFFT;
#endif

	/************************************************************************/
	/* Case HRIR (len <= 1024) -> fill FilterParam struct and quit function */
	/************************************************************************/
	if (pFirData->Ntaps <= limitHRTFshort)
	{
		/* initial BRIR size */
		/* removed in new binaural interface : pParam->lenBRIR = pFirData->Ntaps; */

		/* number of channels */
		pParam->numChannel = nBrirPairs;

		/* set beginDelay to 0 */
		pParam->beginDelay = 0; 

		/* direct filter length has to be a power of 2 */
		lenDirect = 1; 
		while (lenDirect < pFirData->Ntaps)
		{
			lenDirect = lenDirect * 2;
		}
		pParam->lenDirect = lenDirect;

#ifdef _BUGFIX_DIRECT_FC_
		/* directFc : set to 1  */
		for (i_ear = 0; i_ear < 2; ++i_ear)
		{
			for (i_chan = 0; i_chan < pParam->numChannel; ++i_chan)
			{
				pParam->ppDirectFc[i_ear][i_chan] = 1;
			}
		}
#endif
		/* direct filters : copy (zero-padding if pFirData->Ntaps was not a power of 2) */
		for (i_ear = 0; i_ear < 2; ++i_ear)
		{
			for (i_chan = 0; i_chan < pParam->numChannel; ++i_chan)
			{
				memset(pParam->pppTaps_direct[i_ear][i_chan], 0, pParam->lenDirect * sizeof(float));
				memcpy(pParam->pppTaps_direct[i_ear][i_chan],
					pFirData->pppTaps[i_ear][i_chan], /* no beginDelay */
					pFirData->Ntaps * sizeof(float));
			}
		}
		/* diffuse filters : none */
		pParam->numDiffuseBlock = 0;
#ifndef _BUGFIX_DIFFUSE_FC_
		pParam->pDiffuseFc = NULL;
#else
		memset(pParam->ppDiffuseFc, 0, 2*MAX_NUM_DIFFUSE_BLOCKS*sizeof(float));
#endif
		/* set pInvDiffuseWeight to 0 */
		memset(pParam->pInvDiffuseWeight, 0, pParam->numChannel * sizeof(float));
		memset(pParam->pppTaps_diffuse, 0, 2*MAX_NUM_DIFFUSE_BLOCKS*MAX_LENGTH_DIRECT_FILTER*sizeof(float));
		return 0;
	}

	/***********************************************************************/
	/* Case BRIR (len > 1024) -> design direct and diffuse filters         */
	/***********************************************************************/
	/* Find maximum energy */
	energyMaxi = 0;
	numChannelWithZeroEnergy = 0;
	for (i_ear = 0; i_ear < 2; ++i_ear)
	{
		for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
		{
			energy = 0;
			for (i_tap = 0; i_tap < pFirData->Ntaps; ++i_tap)
			{
				energy += (float) pow (pFirData->pppTaps[i_ear][i_chan][i_tap], 2);
			}
			if (energy > energyMaxi)
				energyMaxi = energy;
			if (energy == 0)
				numChannelWithZeroEnergy += 0.5;
		}
	}
	if (numChannelWithZeroEnergy == nBrirPairs)
	{
		fprintf(stderr, "\nError : all filters have zero energy !\n");
		return -1;
	}
	/* Find minimum indBegin */
	indBeginMini = pFirData->Ntaps - 1;
	threshold = (float) pow(10., BeginEnergyThld / 10);
	for (i_ear = 0; i_ear < 2; ++i_ear)
	{
		for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
		{
			indBegin = 0;
			CSEinst = 0;
			while (indBegin < pFirData->Ntaps)
			{
				CSEinst += (float) pow(pFirData->pppTaps[i_ear][i_chan][indBegin], 2);
				if (CSEinst / energyMaxi > threshold)
					break;
				++indBegin;
			}
			if (indBegin < indBeginMini)
				indBeginMini = indBegin;
		}
	}
	/* Find end of direct part */
	indDirectMaxi = 0;
	for (i_ear = 0; i_ear < 2; ++i_ear)
	{
		for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
		{
			/* Recompute filter energy from indBeginMini */
			energy = 0;
			for (i_tap = indBeginMini; i_tap < pFirData->Ntaps; ++i_tap)
			{
				energy += (float) pow(pFirData->pppTaps[i_ear][i_chan][i_tap], 2);
			}
			if (energy > 0)
			{
				/* set threshold  */
				threshold = (float) pow(10., DirectEnergyThld / 10.) * energy;

				indDirect = pFirData->Ntaps - 1;
				CSEinstInv = 0;
				while (indDirect >= 0)
				{
					CSEinstInv += (float) pow(pFirData->pppTaps[i_ear][i_chan][indDirect], 2);
					if (CSEinstInv >= threshold)
						break;
					--indDirect;
				}
			}
			else
			{
				indDirect = 0;
			}
			if (indDirect > indDirectMaxi)
				indDirectMaxi = indDirect;
		}
	}
	/* remove indBeginMini */
	indDirectMaxi -= indBeginMini;
	/* Direct length calculation */
	lenDirect = 1;
	while (lenDirect < indDirectMaxi)
	{
		lenDirect = lenDirect * 2; 
	}
	/*  max MAX_LENGTH_DIRECT_FILTER  */
	if (lenDirect > MAX_LENGTH_DIRECT_FILTER)
	{
		lenDirect = MAX_LENGTH_DIRECT_FILTER;
		fprintf(stderr, "\nWarning : max filterlength reached in Binaural Parametrization. \n");
	}

#ifdef FFT_VERSION2
	/* init fft handle */
	FFTlib_Create(&hFFT, lenDirect, 1);
#endif

#ifdef _BUGFIX_DIRECT_FC_
	/* Direct Fc calculation */
	FFTsize = lenDirect; /* Analysis stage : it is the size of Direct blocks */
	indNyquist = FFTsize / 2; /* -> N/2+1 points */

	/*For each chan, find the index from which cumulative spectral power (end to begin) is under the threshold */
	indFreqMaxi = 0;
	for (i_ear = 0; i_ear < 2; ++i_ear)
	{
		for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
		{
			/* Get block */
			indStart = indBeginMini;
			if (indStart + FFTsize - 1 < pFirData->Ntaps) /* full block is available */
			{
				memcpy(pDirectBlock, &pFirData->pppTaps[i_ear][i_chan][indStart], FFTsize * sizeof(float));
			}
			else /* exceed BRIR size -> zero-pad the block */
			{
				memset(pDirectBlock, 0, FFTsize * sizeof(float));
				memcpy(pDirectBlock, &pFirData->pppTaps[i_ear][i_chan][indStart], (pFirData->Ntaps - indStart) * sizeof(float));
			}
			/* Compute block spectrum and take magnitude */
			memcpy(pDirectBlockSpectrum_r, pDirectBlock, FFTsize * sizeof(float)); /* init real part to pDirectBlock (inplace fft) */
			memset(pDirectBlockSpectrum_i, 0, FFTsize * sizeof(float));          /* init imaginary part to zeros (inplace fft) */
#ifndef FFT_VERSION2
			UHD_fft(pDirectBlockSpectrum_r, pDirectBlockSpectrum_i, FFTsize);
#else
			FFTlib_Apply(hFFT, pDirectBlockSpectrum_r, pDirectBlockSpectrum_i, pDirectBlockSpectrum_r, pDirectBlockSpectrum_i);
#endif
			FFT_Complex_to_Mag(pDirectBlockSpectrum_r, pDirectBlockSpectrum_i, pDirectBlockSpectrum_Mag, FFTsize); /* compute Magnitude */
			/* Recompute filter energy (to Nyquist freq only) */
			energy = 0;
			for (i_tap = 0; i_tap <= indNyquist; ++i_tap)
			{
				energy += (float) pow(pDirectBlockSpectrum_Mag[i_tap], 2);
			}
			if (energy > 0)
			{
				/* set threshold  */
				threshold = (float) pow(10., DirectFcThld / 10.) * energy;
				indFreq = indNyquist;
				CSEinstInvFreq = 0;
				while (indFreq >= 0)
				{
					CSEinstInvFreq += (float)pow(pDirectBlockSpectrum_Mag[indFreq], 2);
					if (CSEinstInvFreq >= threshold)
						break;
					--indFreq;
				}
			}
			else
			{
				indFreq = 0;
			}
			/* Set the frequency value(in Hz) in the output parameter "directFc" */
			pParam->ppDirectFc[i_ear][i_chan] = (float)indFreq / indNyquist;
			/* Possible cutoff frequencies Fc = [0 1/6 1/3 1/2 2/3 5/6 1]; */
			pParam->ppDirectFc[i_ear][i_chan] = ceil(pParam->ppDirectFc[i_ear][i_chan] * 6.0f) / 6.0f;
		} /* loop filter */
	}  /* loop ear */
#endif /* _BUGFIX_DIRECT_FC_ */

	/* Diffuse parameters calculation */
	if (lenDirect < pFirData->Ntaps)
	{
		/* Find end of diffuse part */
		indDiffuseMaxi = 0;
		for (i_ear = 0; i_ear < 2; ++i_ear)
		{
			for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
			{
				/* Recompute filter energy from indBeginMini */
				energy = 0;
				for (i_tap = indBeginMini; i_tap < pFirData->Ntaps; ++i_tap)
				{
					energy += (float)pow(pFirData->pppTaps[i_ear][i_chan][i_tap], 2);
				}
				if (energy == 0)
				{
					indDiffuse = 0;
				}
				else
				{
					/* set threshold  */
					threshold = (float)pow(10., DiffuseEnergyThld / 10.) * energy;

					indDiffuse = pFirData->Ntaps - 1;
					CSEinstInv = 0;
					while (indDiffuse >= 0)
					{
						CSEinstInv += (float)pow(pFirData->pppTaps[i_ear][i_chan][indDiffuse], 2);
						if (CSEinstInv >= threshold)
							break;
						--indDiffuse;
					}
				}
				if (indDiffuse > indDiffuseMaxi)
					indDiffuseMaxi = indDiffuse;
			}
		}
		/* Calculate number of diffuse blocks */
		diffuseMinLength = indDiffuseMaxi - (indBeginMini + lenDirect - 1);
		numDiffuseBlock = 0;
		while (numDiffuseBlock * lenDirect < diffuseMinLength)
			++numDiffuseBlock;

		/*  max MAX_NUM_DIFFUSE_BLOCKS  */
		if (numDiffuseBlock > MAX_NUM_DIFFUSE_BLOCKS)
		{
			numDiffuseBlock = MAX_NUM_DIFFUSE_BLOCKS;
			fprintf(stderr, "\nWarning : max number of diffuse blocks reached in Binaural Parametrization. \n");
		}

		/**************************/
		/* Diffuse Fc calculation */
		if (numDiffuseBlock > 0)
		{
			FFTsize = lenDirect; /* Analysis stage : it's the size of diffuse blocks */
			indNyquist = FFTsize / 2; /* -> N/2+1 points */
			/*For each block, find the index from which cumulative spectral power (end to begin) is under the threshold */
			indFreqMaxi = 0;
			for (i_block = 0; i_block < numDiffuseBlock; ++i_block)
			{
				/* reset indFreqMaxi */
				indFreqMaxi = 0;
				for (i_ear = 0; i_ear < 2; ++i_ear)
				{
					for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
					{
						/* Get block */
						indStart = indBeginMini + (i_block + 1)*FFTsize;
						if (indStart + FFTsize - 1 < pFirData->Ntaps) /* full block is available */
						{
							memcpy(pDiffuseBlock, &pFirData->pppTaps[i_ear][i_chan][indStart], FFTsize * sizeof(float));
						}
						else /* exceed BRIR size -> zero-pad the block */
						{
							memset(pDiffuseBlock, 0, FFTsize * sizeof(float));
							memcpy(pDiffuseBlock, &pFirData->pppTaps[i_ear][i_chan][indStart], (pFirData->Ntaps - indStart) * sizeof(float));
						}
						/* Compute block spectrum and take magnitude */
						memcpy(pDiffuseBlockSpectrum_r, pDiffuseBlock, FFTsize * sizeof(float)); /* init real part to pDiffuseBlock (inplace fft) */
						memset(pDiffuseBlockSpectrum_i, 0, FFTsize * sizeof(float));          /* init imaginary part to zeros (inplace fft) */		
#ifndef FFT_VERSION2
						UHD_fft(pDiffuseBlockSpectrum_r, pDiffuseBlockSpectrum_i, FFTsize);
#else
						FFTlib_Apply(hFFT, pDiffuseBlockSpectrum_r, pDiffuseBlockSpectrum_i, pDiffuseBlockSpectrum_r, pDiffuseBlockSpectrum_i);
#endif
						FFT_Complex_to_Mag(pDiffuseBlockSpectrum_r, pDiffuseBlockSpectrum_i, pDiffuseBlockSpectrum_Mag, FFTsize); /* compute Magnitude */
						/* Recompute filter energy (to Nyquist freq only) */
						energy = 0;
						for (i_tap = 0; i_tap <= indNyquist; ++i_tap)
						{
							energy += (float)pow(pDiffuseBlockSpectrum_Mag[i_tap], 2);
						}
						if (energy > 0)
						{
							/* set threshold */
							threshold = powf(10.0f, DiffuseFcThld / 10.0f) * energy;
							indFreq = indNyquist;
							CSEinstInvFreq = 0;
							while (indFreq >= 0)
							{
								CSEinstInvFreq += (float)pow(pDiffuseBlockSpectrum_Mag[indFreq], 2);
								if (CSEinstInvFreq >= threshold)
									break;
								--indFreq;
							}
						}
						else
						{
							indFreq = 0;
						}

						if (indFreq > indFreqMaxi)
							indFreqMaxi = indFreq;
					} /* loop filter */
#ifdef _BUGFIX_DIFFUSE_FC_
					/* Set the frequency value(in Hz) in the output parameter "diffuseFc" */
					pParam->ppDiffuseFc[i_ear][i_block] = (float)indFreqMaxi / indNyquist;
					/* Possible cutoff frequencies Fc = [1/6 1/3 1/2 2/3 5/6 1]; */
					if (pParam->ppDiffuseFc[i_ear][i_block] == 0)
						pParam->ppDiffuseFc[i_ear][i_block] = 1.0f / 6.0f;
					else
						pParam->ppDiffuseFc[i_ear][i_block] = ceil(pParam->ppDiffuseFc[i_ear][i_block] * 6.0f) / 6.0f;
					/* reset indFreqMaxi */
					indFreqMaxi = 0;
#endif
				}  /* loop ear */

#ifndef _BUGFIX_DIFFUSE_FC_
				/* Set the frequency value(in Hz) in the output parameter "diffuseFc" */
				pDiffuseFc[i_block] = (float)indFreqMaxi / indNyquist;

				/* Possible cutoff frequencies Fc = [1/6 1/3 1/2 2/3 5/6 1]; */
				if (pDiffuseFc[i_block] == 0)
					pDiffuseFc[i_block] = 1. / 6.;
				else
					pDiffuseFc[i_block] = ceil(pDiffuseFc[i_block] * 6.) / 6.;
#endif
			}  /* loop i_block */
		} /* if numDiffuseBlock > 0 */

		/********************************/
		/* Diffuse Weights calculation  */
		if (numDiffuseBlock > 0)
		{
			/* size of diffuse filters (containing all blocks) */
			lenDiffus = numDiffuseBlock * lenDirect;

			/* mallocs */
			pDiffuseFilter = (float*)malloc(lenDiffus * sizeof(float)); /* memset not necessary for pDiffuseFilter */
			ppDiffuseFilterWMC = (float **)malloc(2*sizeof(float *));
            for (i_ear = 0; i_ear < 2; ++i_ear)
            {
                ppDiffuseFilterWMC[i_ear] = (float*)malloc(lenDiffus * sizeof(float));
                memset(ppDiffuseFilterWMC[i_ear], 0, lenDiffus * sizeof(float)); /* memset necessary for ppDiffuseFilterWMC */
            }
			
			/* compute diffusFilterWMC */
			for (i_ear = 0; i_ear < 2; ++i_ear)
			{
				for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
				{
					/* Get diffuse filter */
					indStart = indBeginMini + lenDirect;
					if (indStart + lenDiffus - 1 < pFirData->Ntaps) /* full diffuse filter is available */
					{
						memcpy(pDiffuseFilter, &pFirData->pppTaps[i_ear][i_chan][indStart], lenDiffus * sizeof(float));
					}
					else /* exceed BRIR size -> zero-pad the diffuse filter */
					{
						memset(pDiffuseFilter, 0, lenDiffus * sizeof(float));
						memcpy(pDiffuseFilter, &pFirData->pppTaps[i_ear][i_chan][indStart], (pFirData->Ntaps - indStart) * sizeof(float));
					}
					/* Apply cos window to last 512 samples of the diffuse filter */
					lenWin = 512;
					if (lenDiffus >= lenWin)  /* WARNING WARNING what if lenDiffus < lenWin ? impossible, lenDiffus is > 1024 */
					{
						for (i_tap = 0; i_tap < lenWin; ++i_tap)
						{
							pDiffuseFilter[lenDiffus - 1 - i_tap] *= (float)((cos(3.141592653589793 * (1 - ((float)i_tap + 1.) / lenWin)) + 1) / 2.0);
						}
					}
					/* Compute diffuse filter energy and store it */
					energy = 0;
					for (i_tap = 0; i_tap < lenDiffus; ++i_tap)
					{
						energy += (float)pow(pDiffuseFilter[i_tap], 2);
					}
					ppEnergyDiffuseFilter[i_ear][i_chan] = energy;

					if (ppEnergyDiffuseFilter[i_ear][i_chan] > 0)
					{
						/* compute diffusFilterWMC */
						for (i_tap = 0; i_tap < lenDiffus; ++i_tap)
						{
							ppDiffuseFilterWMC[i_ear][i_tap] += pDiffuseFilter[i_tap] / sqrtf(energy) / (float)(nBrirPairs - numChannelWithZeroEnergy);
						}
					}

				} /* loop filter */

			}  /* loop ear */

			for (i_ear = 0; i_ear < 2; ++i_ear)
			{
				/* Compute diffuse ppDiffuseFilterWMC energy */
				energy = 0;
				for (i_tap = 0; i_tap < lenDiffus; ++i_tap)
				{
					energy += (float)pow(ppDiffuseFilterWMC[i_ear][i_tap], 2);
				}

				for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
				{
					if (ppEnergyDiffuseFilter[i_ear][i_chan] > 0)
					{
						ppDiffuseWeight[i_ear][i_chan] = sqrtf(energy / ppEnergyDiffuseFilter[i_ear][i_chan]);
					}
					else
					{
						ppDiffuseWeight[i_ear][i_chan] = -1; /* avoid div by 0 when ppEnergyDiffuseFilter[i_ear][i_chan]=0 */ 
					}
				}
			}

			/* c6 USE global attenuation : apply -6dB to diffuse filters */
			for (i_ear = 0; i_ear < 2; ++i_ear)
			{
				for (i_tap = 0; i_tap < lenDiffus; ++i_tap)
				{
					ppDiffuseFilterWMC[i_ear][i_tap] *= 0.501187233627272f;
				}
			}

		} /* if numDiffuseBlock > 0 */

	} /* if (lenDirect < pFirData->Ntaps) */
	else
	{
		/* no diffuse block */
		numDiffuseBlock = 0;
	}

	/***************************/
	/* Fill FilterParam struct */

	/* initial BRIR size */
	/* removed in new binaural interface : pParam->lenBRIR = pFirData->Ntaps; */

	/* number of channels */
	pParam->numChannel = nBrirPairs;

	/* set beginDelay */
	pParam->beginDelay = indBeginMini;

	/* direct filter length */
	pParam->lenDirect = lenDirect;

	/* number of diffuse blocks */
	pParam->numDiffuseBlock = numDiffuseBlock;

	/* directFc : ok, nothing to do */

	/* direct filters : copy (zero-padding if pParam->lenDirect > pFirData->Ntaps) */
	cpyLen = pFirData->Ntaps < pParam->lenDirect ? pFirData->Ntaps : pParam->lenDirect; /* take min of the 2 len */
	for (i_ear = 0; i_ear < 2; ++i_ear)
	{
		for (i_chan = 0; i_chan < pParam->numChannel; ++i_chan)
		{
			memset(pParam->pppTaps_direct[i_ear][i_chan], 0, pParam->lenDirect * sizeof(float));
			memcpy(pParam->pppTaps_direct[i_ear][i_chan],
				&pFirData->pppTaps[i_ear][i_chan][pParam->beginDelay],
				cpyLen * sizeof(float));
		}
	}

	/* diffuse filters params */
	if (numDiffuseBlock > 0)
	{
		/* ppDiffuseFc : ok, nothing to do */

		/* set pInvDiffuseWeight (inverse of mean weights per channel) */
		for (i_chan = 0; i_chan < pParam->numChannel; ++i_chan)
		{
			if (ppDiffuseWeight[0][i_chan] != -1 && ppDiffuseWeight[1][i_chan] != -1)
			{
				pParam->pInvDiffuseWeight[i_chan] = 2.0f / (ppDiffuseWeight[0][i_chan] + ppDiffuseWeight[1][i_chan]);
			}
			else
			{
				pParam->pInvDiffuseWeight[i_chan] = 0.0f; /* pathological case ppEnergyDiffuseFilter[i_ear][i_chan]=0 */ 
			} 
		}

		/* set diffuse block filters */
		for (i_ear = 0; i_ear < 2; ++i_ear)
		{
			for (i_block = 0; i_block < pParam->numDiffuseBlock; ++i_block)
			{
				indStart = i_block * pParam->lenDirect;
				memcpy(pParam->pppTaps_diffuse[i_block][i_ear], &ppDiffuseFilterWMC[i_ear][indStart], pParam->lenDirect * sizeof(float));
			}
		}
	}
	else /* no diffuse block */
	{
		memset(pParam->ppDiffuseFc, 0, 2*MAX_NUM_DIFFUSE_BLOCKS*sizeof(float));
		/* set pInvDiffuseWeight to 0 */
		memset(pParam->pInvDiffuseWeight, 0, pParam->numChannel * sizeof(float));
		memset(pParam->pppTaps_diffuse, 0, 2*MAX_NUM_DIFFUSE_BLOCKS*MAX_LENGTH_DIRECT_FILTER*sizeof(float));

	}

	if (numDiffuseBlock > 0)
    {
        free(pDiffuseFilter);
        for (i_ear = 0; i_ear < 2; ++i_ear)
        { 
            free(ppDiffuseFilterWMC[i_ear]);
        }
        free(ppDiffuseFilterWMC);
    }

#ifdef FFT_VERSION2
	/* destroy fft handle */
	FFTlib_Destroy(&hFFT);
#endif

	return 0;

}


/*----------------------------------------------------------------------------*/
void printFilterParams(TdBinauralRendererParam *pParam)
{
	printf("\nFilter Params");
	printf("\nNum Channels : %d", pParam->numChannel);
	printf("\nBegin delay : %d", pParam->beginDelay);
	/* removed in new binaural interface : printf("\nLength BRIR : %d", pParam->lenBRIR); */
	printf("\nLength Direct : %d", pParam->lenDirect);
	printf("\nNum Diffuse Blocks : %d", pParam->numDiffuseBlock);

#ifdef _BUGFIX_DIRECT_FC_
	printf("\n");
	printf("\nDirect Fc Left  : ");
	printBuf(pParam->ppDirectFc[0], pParam->numChannel);
	printf("\n");
	printf("\nDirect Fc Right : ");
	printBuf(pParam->ppDirectFc[1], pParam->numChannel);
	printf("\n");
#endif

	/* diffuse Weights */
	printf("\nInverse Diffuse Weights :");
	printBuf(pParam->pInvDiffuseWeight, pParam->numChannel);
	printf("\n");

	if (pParam->numDiffuseBlock > 0)
	{
		/* diffuse Fc */
#ifndef _BUGFIX_DIFFUSE_FC_
		printf("\nDiffuse Fc : ");
		printBuf(pParam->pDiffuseFc, pParam->numDiffuseBlock);

#else
		printf("\nDiffuse Fc Left  : ");
		printBuf(pParam->ppDiffuseFc[0], pParam->numDiffuseBlock);
		printf("\n");
		printf("\nDiffuse Fc Right : ");
		printBuf(pParam->ppDiffuseFc[1], pParam->numDiffuseBlock);
#endif

	}
	else
	{
		printf("\nDiffuse Fc : None");
	}

	printf("\n");
}






/*--------------------------------  FFT Utils   --------------------------------------------*/
void FFT_Complex_to_Mag(
	float				*pXr_32f,
	float				*pXi_32f,
	float				*pMag_32f,
	int					FFTsize_32s)
{
	int ind_32s;

	if (!pMag_32f)
	{
		pMag_32f = (float*)malloc(FFTsize_32s*sizeof(float));
	}

	for (ind_32s = 0; ind_32s < FFTsize_32s; ++ind_32s)
	{
		pMag_32f[ind_32s] = (float)sqrt(pow(pXr_32f[ind_32s], 2) + pow(pXi_32f[ind_32s], 2));
	}
}













