/***********************************************************************************

This software module was originally developed by 

Orange Labs, ETRI, Yonsei University, WILUS Institute

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

Orange Labs, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/


#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "bitstreamBinaural.h"
#include "cicp2geometry.h"
#include "libtsp.h"
#include <math.h>

static const int brirSamplingFrequencyTable[32] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 
	16000, 12000, 11025, 8000, 7350, -1, -1, 57600,
	51200, 40000, 38400, 34150, 28800, 25600, 20000, 19200,
	17075, 14400, 12800, 9600, -1, -1, -1, 0
};

static int getBrirSamplingFrequencyIdx(int fs)
{
	short ii;
	for (ii=0; ii<32; ++ii)
	{
		if ( brirSamplingFrequencyTable[ii] == fs )
			return ii;
	}

	return 0x1F; /* escape code */
}


/*--------------------- Utilities : complete ROBITBUFAPI/WOBITBUFAPI to read/write IEEE float 32    ---------------------*/

static float robitbuf_ReadFloat( robitbufHandle self )
{
	float val = 0;
	unsigned long temp_uL = 0;

	temp_uL = robitbuf_ReadBits(self, 32);
	*((unsigned long *)(&val)) = temp_uL;

	return val;
}

static int wobitbuf_WriteFloat( wobitbufHandle self, float data )
{
	int error = 0;
	unsigned long temp_uL = 0;

	*((float *)(&temp_uL)) = data;
	error = wobitbuf_WriteBits(self, temp_uL, 32);

	return error;
}



/*--------------------------  BinauralFirData  ------------------------------*/

int readBinauralFirData(BinauralFirData *pBinauralFirData, 
						short nBrirPairs,
						robitbufHandle hBitstream)
{

	int i, j;

	pBinauralFirData->Ntaps = robitbuf_ReadBits(hBitstream, 24); /* bsNumCoefs */

	if (pBinauralFirData->Ntaps > MAX_BRIR_SIZE)
	{
		fprintf(stderr,"\nError : BRIR size should be <= %d", MAX_BRIR_SIZE);
		return -1;
	}

	for (i=0; i<nBrirPairs; i++)
	{
		for(j=0; j<pBinauralFirData->Ntaps; j++)
		{
			pBinauralFirData->pppTaps[0][i][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFirCoefLeft[pos][i] */
			pBinauralFirData->pppTaps[1][i][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFirCoefRight[pos][i] */
		}
	}

	pBinauralFirData->allCutFreq = robitbuf_ReadBits(hBitstream, 32); /* bsAllCutFreq */

	if (pBinauralFirData->allCutFreq == 0)
	{
		for (i=0; i<nBrirPairs; i++)
		{
			pBinauralFirData->ppCutFreq[0][i] =
				robitbuf_ReadFloat(hBitstream); /* bsCutFreqLeft[pos] */
			pBinauralFirData->ppCutFreq[1][i] =
				robitbuf_ReadFloat(hBitstream); /* bsCutFreqRight[pos] */
		}
	}
	else
	{
		for (i=0; i<nBrirPairs; i++)
		{
			pBinauralFirData->ppCutFreq[0][i] = (float)pBinauralFirData->allCutFreq;
			pBinauralFirData->ppCutFreq[1][i] = (float)pBinauralFirData->allCutFreq;
		}
	}
	return 0;
}

int writeBinauralFirData(const BinauralFirData *pBinauralFirData, 
						 short nBrirPairs,
						 wobitbufHandle hBitstream)
{
	int i, j;

	wobitbuf_WriteBits(hBitstream, pBinauralFirData->Ntaps, 24); /* bsNumCoefs */
	for (i=0; i<nBrirPairs; i++)
	{
		for(j=0; j<pBinauralFirData->Ntaps; j++)
		{
			wobitbuf_WriteFloat(hBitstream, pBinauralFirData->pppTaps[0][i][j]); /* bsFirCoefLeft[pos][i] */
			wobitbuf_WriteFloat(hBitstream, pBinauralFirData->pppTaps[1][i][j]); /* bsFirCoefRight[pos][i] */
		}
	}
	wobitbuf_WriteBits(hBitstream, pBinauralFirData->allCutFreq, 32); /* bsAllCutFreq */
	if (pBinauralFirData->allCutFreq == 0)
	{
		for (i=0; i<nBrirPairs; i++)
		{
			wobitbuf_WriteFloat(hBitstream, pBinauralFirData->ppCutFreq[0][i]); /* bsCutFreqLeft[pos] */
			wobitbuf_WriteFloat(hBitstream, pBinauralFirData->ppCutFreq[1][i]); /* bsCutFreqRight[pos] */
		}
	}

	return 0;
}


/*-------------------------  FdBinauralRendererParam  --------------------------------*/

int readFdBinauralRendererParam(
	FdBinauralRendererParam *pFdBinauralRendererParam, 
	short nBrirPairs,
	robitbufHandle hBitstream)
{
	
	int k, b, nr, v, fftlength;
	int nBitNFilter=0, nBitNFft=0, nBitNBlk=0;
	int nBitQtdlLag;


	pFdBinauralRendererParam->flagHRIR = robitbuf_ReadBits(hBitstream, 1);  /* flagHRIR */
	pFdBinauralRendererParam->dInit	   = robitbuf_ReadBits(hBitstream, 10); /* kMax */
	pFdBinauralRendererParam->kMax     = robitbuf_ReadBits(hBitstream, 6);  /* kMax */
	pFdBinauralRendererParam->kConv    = robitbuf_ReadBits(hBitstream, 6);  /* kConv */
	pFdBinauralRendererParam->kAna     = robitbuf_ReadBits(hBitstream, 6);  /* kAna */

	/***************************** VoffBrirParam *****************************/

	nBitNFilter = robitbuf_ReadBits(hBitstream, 4);  /* nBitNFilter */
	nBitNFft = robitbuf_ReadBits(hBitstream, 3);  /* nBitNFft */
	nBitNBlk = robitbuf_ReadBits(hBitstream, 3);  /* nBitNBlk */

	/* Memory Allocation */
	pFdBinauralRendererParam->nFilter = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));
	pFdBinauralRendererParam->nFft = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));
	pFdBinauralRendererParam->nBlk = (unsigned int*)calloc(pFdBinauralRendererParam->kMax, sizeof(unsigned int));

	pFdBinauralRendererParam->ppppVoffCoeffLeftReal = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffLeftImag = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightReal = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));
	pFdBinauralRendererParam->ppppVoffCoeffRightImag = (float****)calloc(pFdBinauralRendererParam->kMax, sizeof(float***));

	for(k=0; k<(int)pFdBinauralRendererParam->kMax; k++)
	{
		pFdBinauralRendererParam->nFilter[k] = robitbuf_ReadBits(hBitstream, nBitNFilter);  /* nFilter */
		pFdBinauralRendererParam->nFft[k] = robitbuf_ReadBits(hBitstream, nBitNFft);  /* nFft */
		pFdBinauralRendererParam->nBlk[k] = robitbuf_ReadBits(hBitstream, nBitNBlk);  /* nBlk */

		/* Memory Allocation */
		pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightReal[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		pFdBinauralRendererParam->ppppVoffCoeffRightImag[k] = (float***)calloc(pFdBinauralRendererParam->nBlk[k], sizeof(float**));
		fftlength = (int)(pow(2.0f, pFdBinauralRendererParam->nFft[k]));
		for( b=0; b<(int)pFdBinauralRendererParam->nBlk[k]; b++)
		{		
			/* Memory Allocation */
			pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b] = (float**)calloc(nBrirPairs, sizeof(float*)); 
			pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b] = (float**)calloc(nBrirPairs, sizeof(float*));
			for( nr=0; nr < nBrirPairs; nr++)
			{
				/* Memory Allocation */
				pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr] = (float*)calloc(fftlength, sizeof(float));
				for( v=0; v< fftlength; v++)
				{
					pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr][v] = robitbuf_ReadFloat(hBitstream);  /* VoffCoeffLeftReal[k][b][nr][v] */
					pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr][v] = robitbuf_ReadFloat(hBitstream);  /* VoffCoeffLeftImag[k][b][nr][v] */
					pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr][v] = robitbuf_ReadFloat(hBitstream); /* VoffCoeffRightReal[k][b][nr][v] */
					pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr][v] = robitbuf_ReadFloat(hBitstream); /* VoffCoeffRightImag[k][b][nr][v] */
				}
			}
		}
	}

	/***************************** SfrBrirParam *****************************/
	pFdBinauralRendererParam->fcAna = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	pFdBinauralRendererParam->rt60 = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	pFdBinauralRendererParam->nrgLr = (float*)calloc(pFdBinauralRendererParam->kAna, sizeof(float));
	for(k=0; k<(int)pFdBinauralRendererParam->kAna; k++)
	{
		pFdBinauralRendererParam->fcAna[k] = robitbuf_ReadFloat(hBitstream);  /* fcAna[k] */
		pFdBinauralRendererParam->rt60[k] = robitbuf_ReadFloat(hBitstream);   /* rt60[k] */
		pFdBinauralRendererParam->nrgLr[k] = robitbuf_ReadFloat(hBitstream);  /* nrgLr[k] */
	}
	
	/***************************** QtdlBrirParam *****************************/
	/* Memory Allocation */
	pFdBinauralRendererParam->ppQtdlGainLeftReal = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainLeftImag = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightReal = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlGainRightImag = (float**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(float*));
	pFdBinauralRendererParam->ppQtdlLagLeft = (unsigned int**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(unsigned int*));
	pFdBinauralRendererParam->ppQtdlLagRight = (unsigned int**)calloc(pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv, sizeof(unsigned int*));
	for(k=0; k<(int)pFdBinauralRendererParam->kMax-(int)pFdBinauralRendererParam->kConv; k++)
	{
		nBitQtdlLag = robitbuf_ReadBits(hBitstream, 4);  /* nBitQtdlLag */
		/* Memory Allocation */
		pFdBinauralRendererParam->ppQtdlGainLeftReal[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainLeftImag[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightReal[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlGainRightImag[k] = (float*)calloc(nBrirPairs, sizeof(float));
		pFdBinauralRendererParam->ppQtdlLagLeft[k] = (unsigned int*)calloc(nBrirPairs, sizeof(unsigned int));
		pFdBinauralRendererParam->ppQtdlLagRight[k] = (unsigned int*)calloc(nBrirPairs, sizeof(unsigned int));
		for( nr=0; nr < nBrirPairs; nr++)
		{
			pFdBinauralRendererParam->ppQtdlGainLeftReal[k][nr] = robitbuf_ReadFloat(hBitstream);			/* ppQtdlGainLeftReal[k][nr] */
			pFdBinauralRendererParam->ppQtdlGainLeftImag[k][nr] = robitbuf_ReadFloat(hBitstream);			/* ppQtdlGainLeftImag[k][nr] */
			pFdBinauralRendererParam->ppQtdlGainRightReal[k][nr] = robitbuf_ReadFloat(hBitstream);			/* ppQtdlGainRightReal[k][nr] */
			pFdBinauralRendererParam->ppQtdlGainRightImag[k][nr] = robitbuf_ReadFloat(hBitstream);			/* ppQtdlGainRightImag[k][nr] */
			pFdBinauralRendererParam->ppQtdlLagLeft[k][nr] = robitbuf_ReadBits(hBitstream, nBitQtdlLag);	/* ppQtdlLagLeft[k][nr] */
			pFdBinauralRendererParam->ppQtdlLagRight[k][nr] = robitbuf_ReadBits(hBitstream, nBitQtdlLag);   /* ppQtdlLagRight[k][nr] */
		}
	}
	return 0;
}

int writeFdBinauralRendererParam(
	FdBinauralRendererParam *pFdBinauralRendererParam, 
	short nBrirPairs,
	wobitbufHandle hBitstream)
{

	unsigned int k, b, nr, v, fftlength;
	int nBitNFilter=0, nBitNFft=0, nBitNBlk=0;
	int tmpnBitNFilter=0, tmpnBitNFft=0, tmpnBitNBlk=0;
	int nBitQtdlLag;
	int tmpnBitQtdlLag = 0;


	wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->flagHRIR, 1);  /* flagHRIR */
	wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->dInit,   10);	/* dInit */
	wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->kMax,     6);  /* kMax */
	wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->kConv,    6);	/* kConv */
	wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->kAna,     6);	/* kAna */

	/***************************** VoffBrirParam *****************************/
	/* Find minimum bits for nFilter, nFft, and nBitBlk */
	for(k=0; k<pFdBinauralRendererParam->kMax; k++)
	{		
		tmpnBitNFilter = (int)(ceil(log10((float)pFdBinauralRendererParam->nFilter[k]+1)/log10(2.0f))); 
		if( nBitNFilter < tmpnBitNFilter ){
			nBitNFilter = tmpnBitNFilter;
		}
		tmpnBitNFft = (int)(ceil(log10((float)pFdBinauralRendererParam->nFft[k]+1)/log10(2.0f))+1);
		if( nBitNFft < tmpnBitNFft ){
			nBitNFft = tmpnBitNFft;
		}
		tmpnBitNBlk = (int)(ceil(log10((float)pFdBinauralRendererParam->nBlk[k]+1)/log10(2.0f))+1);
		if( nBitNBlk < tmpnBitNBlk ){
			nBitNBlk = tmpnBitNBlk;
		}
	}

	wobitbuf_WriteBits(hBitstream, nBitNFilter, 4);  /* nBitNFilter */
	wobitbuf_WriteBits(hBitstream, nBitNFft, 3);	 /* nBitNFft */
	wobitbuf_WriteBits(hBitstream, nBitNBlk, 3);     /* nBitNBlk */

	for(k=0; k<pFdBinauralRendererParam->kMax; k++)
	{
		wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->nFilter[k], nBitNFilter);  /* nFilter[k] */
		wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->nFft[k], nBitNFft);		/* nFft[k] */
		wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->nBlk[k], nBitNBlk);		/* nBlk[k] */
		fftlength = (int)(pow(2.0f, pFdBinauralRendererParam->nFft[k]));
		for( b=0; b<pFdBinauralRendererParam->nBlk[k]; b++)
		{
			for( nr=0; nr < (unsigned int)nBrirPairs; nr++)
			{
				for( v=0; v< fftlength; v++)
				{
					wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppppVoffCoeffLeftReal[k][b][nr][v]);	/* VoffCoeffLeftReal[k][b][nr][v] */
					wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppppVoffCoeffLeftImag[k][b][nr][v]);	/* ppppVoffCoeffLeftImag[k][b][nr][v] */
					wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppppVoffCoeffRightReal[k][b][nr][v]);	/* ppppVoffCoeffRightReal[k][b][nr][v] */
					wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppppVoffCoeffRightImag[k][b][nr][v]);	/* ppppVoffCoeffRightImag[k][b][nr][v] */
				}
			}
		}
	}

	/***************************** SfrBrirParam *****************************/
	for(k=0; k<pFdBinauralRendererParam->kAna; k++)
	{
		wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->fcAna[k]);	/* fcAna[k] */
		wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->rt60[k]);		/* rt60[k] */
		wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->nrgLr[k]);	/* nrgLr[k] */
	}
	
	/***************************** QtdlBrirParam *****************************/
	for(k=0; k<pFdBinauralRendererParam->kMax-pFdBinauralRendererParam->kConv; k++)
	{
		nBitQtdlLag = 0;
		for( nr=0; nr < (unsigned int) nBrirPairs; nr++)
		{
			tmpnBitQtdlLag =  (int)(ceil(log10((float)pFdBinauralRendererParam->ppQtdlLagLeft[k][nr])/log10(2.0f)));
			if( nBitQtdlLag < tmpnBitQtdlLag ){
				nBitQtdlLag = tmpnBitQtdlLag;
			}
			tmpnBitQtdlLag =  (int)(ceil(log10((float)pFdBinauralRendererParam->ppQtdlLagRight[k][nr])/log10(2.0f)));
			if( nBitQtdlLag < tmpnBitQtdlLag ){
				nBitQtdlLag = tmpnBitQtdlLag;
			}
		}
		wobitbuf_WriteBits(hBitstream, nBitQtdlLag, 4);     /* nBitNBlk */

		for( nr=0; nr < (unsigned int)nBrirPairs; nr++)
		{
			wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppQtdlGainLeftReal[k][nr]);			/* ppQtdlGainLeftReal[k][nr] */
			wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppQtdlGainLeftImag[k][nr]);			/* ppQtdlGainLeftImag[k][nr] */
			wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppQtdlGainRightReal[k][nr]);			/* ppQtdlGainRightReal[k][nr] */
			wobitbuf_WriteFloat(hBitstream, pFdBinauralRendererParam->ppQtdlGainRightImag[k][nr]);			/* ppQtdlGainRightImag[k][nr] */
			wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->ppQtdlLagLeft[k][nr], nBitQtdlLag);	/* ppQtdlLagLeft[k][nr] */	
			wobitbuf_WriteBits(hBitstream, pFdBinauralRendererParam->ppQtdlLagRight[k][nr], nBitQtdlLag);	/* ppQtdlLagRight[k][nr] */
		}
	}

	return 0;
};
/*-------------------------  TdBinauralRendererParam  --------------------------------*/

int readTdBinauralRendererParam(
	TdBinauralRendererParam *pTdBinauralRendererParam, 
	short nBrirPairs,
	robitbufHandle hBitstream)
{
	int i, j;

	pTdBinauralRendererParam->numChannel = nBrirPairs;

	pTdBinauralRendererParam->beginDelay = robitbuf_ReadBits(hBitstream, 16); /* bsDelay */

	pTdBinauralRendererParam->lenDirect = robitbuf_ReadBits(hBitstream, 16); /* bsDirectLen */
	if (pTdBinauralRendererParam->lenDirect > MAX_LENGTH_DIRECT_FILTER)
	{
		fprintf(stderr,"\nError : bsDirectLen should be <= %d", MAX_LENGTH_DIRECT_FILTER);
		return -1;
	}

	pTdBinauralRendererParam->numDiffuseBlock = robitbuf_ReadBits(hBitstream, 8); /* bsNbDiffuseBlocks */
	if (pTdBinauralRendererParam->numDiffuseBlock > MAX_NUM_DIFFUSE_BLOCKS)
	{
		fprintf(stderr,"\nError : bsNbDiffuseBlocks should be <= %d", MAX_NUM_DIFFUSE_BLOCKS);
		return -1;
	}

	for (i=0; i<nBrirPairs; i++)
	{
		pTdBinauralRendererParam->ppDirectFc[0][i] = 
			robitbuf_ReadFloat(hBitstream); /* bsFmaxDirectLeft[chan] */
		pTdBinauralRendererParam->ppDirectFc[1][i] = 
			robitbuf_ReadFloat(hBitstream); /* bsFmaxDirectRight[chan] */
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		pTdBinauralRendererParam->ppDiffuseFc[0][i] = 
			robitbuf_ReadFloat(hBitstream); /* bsFmaxDiffuseLeft[block] */
		pTdBinauralRendererParam->ppDiffuseFc[1][i] = 
			robitbuf_ReadFloat(hBitstream); /* bsFmaxDiffuseRight[block]; */
	}

	for (i=0; i<nBrirPairs; i++)
	{
		pTdBinauralRendererParam->pInvDiffuseWeight[i] = 
			robitbuf_ReadFloat(hBitstream); /* bsWeights[chan] */
	}

	for (i=0; i<nBrirPairs; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
			pTdBinauralRendererParam->pppTaps_direct[0][i][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFIRDirectLeft[chan][tap] */
			pTdBinauralRendererParam->pppTaps_direct[1][i][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFIRDirectRight[chan][tap]; */
		}
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
			pTdBinauralRendererParam->pppTaps_diffuse[i][0][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFIRDiffuseLeft[bloc][tap] */
			pTdBinauralRendererParam->pppTaps_diffuse[i][1][j] = 
				robitbuf_ReadFloat(hBitstream); /* bsFIRDiffuseRight[bloc][tap] */
		}
	}
	return 0;
};




int writeTdBinauralRendererParam(
	TdBinauralRendererParam *pTdBinauralRendererParam, 
	short nBrirPairs,
	wobitbufHandle hBitstream)
{
	int i, j;

	wobitbuf_WriteBits(hBitstream, pTdBinauralRendererParam->beginDelay, 16); /* bsDelay */
	wobitbuf_WriteBits(hBitstream, pTdBinauralRendererParam->lenDirect, 16); /* bsDirectLen */
	wobitbuf_WriteBits(hBitstream, pTdBinauralRendererParam->numDiffuseBlock, 8); /* bsNbDiffuseBlocks */

	for (i=0; i<nBrirPairs; i++)
	{

		wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->ppDirectFc[0][i]); /* bsFmaxDirectLeft[chan] */
		wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->ppDirectFc[1][i]); /* bsFmaxDirectRight[chan] */
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->ppDiffuseFc[0][i]); /* bsFmaxDiffuseLeft[block] */
		wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->ppDiffuseFc[1][i]); /* bsFmaxDiffuseRight[block]; */
	}

	for (i=0; i<nBrirPairs; i++)
	{
		wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->pInvDiffuseWeight[i]); /* bsWeights[chan] */
	}

	for (i=0; i<nBrirPairs; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
			wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->pppTaps_direct[0][i][j]); /* bsFIRDirectLeft[chan][tap] */
			wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->pppTaps_direct[1][i][j]); /* bsFIRDirectRight[chan][tap]; */
		}
	}

	for (i=0; i<pTdBinauralRendererParam->numDiffuseBlock; i++)
	{
		for(j=0; j<pTdBinauralRendererParam->lenDirect; j++)
		{
			wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->pppTaps_diffuse[i][0][j]); /* bsFIRDiffuseLeft[bloc][tap] */
			wobitbuf_WriteFloat(hBitstream, pTdBinauralRendererParam->pppTaps_diffuse[i][1][j]); /* bsFIRDiffuseRight[bloc][tap] */
		}
	}
	return 0;
};


/*-------------------------------- BinauralRepresentation --------------------------------------------*/

BinauralRepresentation * BinauralRepresentation_initAlloc()
{
	BinauralRepresentation *pBinauralRepresentation = (BinauralRepresentation *)malloc(sizeof(BinauralRepresentation));
	if (pBinauralRepresentation==NULL) 
	{
		fprintf(stderr,"\nError : Binaural Malloc Issue");
		return NULL;
	}

	pBinauralRepresentation->brirSamplingFrequency = 0;
	pBinauralRepresentation->isHoaData = 0;
	pBinauralRepresentation->hoaOrderBinaural = 0;										/* case HOA */
	pBinauralRepresentation->nBrirPairs = 0;
  memset(&pBinauralRepresentation->Setup_SpeakerConfig3d, 0, sizeof(Setup_SpeakerConfig3d));		/* case VL  */
	pBinauralRepresentation->binauralDataFormatID = -1;		/* 0 : raw FIR     1 : FD parameters     2 : TD parameters*/

	/* only one of the following 3 pointers may be used, depending on binauralDataFormatID */
	pBinauralRepresentation->pBinauralFirData = (BinauralFirData *)malloc(sizeof(BinauralFirData));
	if (pBinauralRepresentation->pBinauralFirData == NULL) 
	{
		fprintf(stderr,"\nError : Binaural Malloc Issue");
		return NULL;
	}

	pBinauralRepresentation->pFdBinauralRendererParam = (FdBinauralRendererParam *)malloc(sizeof(FdBinauralRendererParam));
	if (pBinauralRepresentation->pFdBinauralRendererParam == NULL) 
	{
		fprintf(stderr,"\nError : Binaural Malloc Issue");
		return NULL;
	}

	pBinauralRepresentation->pTdBinauralRendererParam = (TdBinauralRendererParam *)malloc(sizeof(TdBinauralRendererParam));
	if (pBinauralRepresentation->pTdBinauralRendererParam == NULL) 
	{
		fprintf(stderr,"\nError : Binaural Malloc Issue");
		return NULL;
	}



	return pBinauralRepresentation;
}

int BinauralRepresentation_setFIR_CICP_GEO(
		BinauralRepresentation *pBinauralRepresentation,
		unsigned long brirSamplingFrequency,
		short nBrirPairs,
		Setup_SpeakerConfig3d *pSpeakerConfig3d,
		BinauralFirData *pBinauralFirData)
{
	pBinauralRepresentation->binauralDataFormatID = 0; /* FIR */
	pBinauralRepresentation->brirSamplingFrequency = brirSamplingFrequency;
	pBinauralRepresentation->isHoaData = 0;
	pBinauralRepresentation->nBrirPairs = nBrirPairs;
	memcpy(&pBinauralRepresentation->Setup_SpeakerConfig3d, pSpeakerConfig3d, sizeof(Setup_SpeakerConfig3d));

	if (pBinauralRepresentation->pBinauralFirData != NULL)
		memcpy(pBinauralRepresentation->pBinauralFirData, pBinauralFirData, sizeof(BinauralFirData));

	return 0;
}

int BinauralRepresentation_setFIR_HOA(
	BinauralRepresentation *pBinauralRepresentation,
	unsigned long   brirSamplingFrequency,
	short hoaOrder,
	BinauralFirData *pBinauralFirData)
{
	pBinauralRepresentation->binauralDataFormatID = 0; /* FIR */
	pBinauralRepresentation->brirSamplingFrequency = brirSamplingFrequency;
	pBinauralRepresentation->isHoaData = 1; /* HOA */
	pBinauralRepresentation->hoaOrderBinaural = hoaOrder;
	pBinauralRepresentation->nBrirPairs = (hoaOrder+1)*(hoaOrder+1);
	pBinauralRepresentation->Setup_SpeakerConfig3d.speakerLayoutType = -1; /* config CICP */
	if (pBinauralRepresentation->pBinauralFirData != NULL)
		memcpy(pBinauralRepresentation->pBinauralFirData, pBinauralFirData, sizeof(BinauralFirData));

	return 0;
}

int BinauralRepresentation_setTdBinauralRendererParam(
	BinauralRepresentation *pBinauralRepresentation,
	TdBinauralRendererParam *pTdBinauralRendererParam)
{

	/* WARNING :   */
	/* brirSamplingFrequency, nBrirPairs, isHoaData, pBinauralRepresentation->Setup_SpeakerConfig3d .... is not set here !!!!!!!!!!! */
	/* it is assumed that these fields are already set, i.e., by the FIR representation */

	pBinauralRepresentation->binauralDataFormatID = 2;
	if (pBinauralRepresentation->pTdBinauralRendererParam != NULL) 
		memcpy(pBinauralRepresentation->pTdBinauralRendererParam, pTdBinauralRendererParam, sizeof(TdBinauralRendererParam));

	return 0;
}

/*-----------------------------  BinauralRendering   -----------------------------------------------*/

BinauralRendering * BinauralRendering_initAlloc()
{
	BinauralRendering *pBinauralRendering = (BinauralRendering *)malloc(sizeof(BinauralRendering));
	if (pBinauralRendering==NULL) 
	{
		fprintf(stderr,"\nError : Binaural Malloc Issue");
		return NULL;
	}

	pBinauralRendering->fileSignature = 0; 
	pBinauralRendering->fileVersion = 0;
	strcpy(pBinauralRendering->name,"");
	pBinauralRendering->useTrackingMode = 0;
	pBinauralRendering->numBinauralRepresentation = 0;

	/* set all BinauralRepresentation pointers to NULL */
	memset(pBinauralRendering->ppBinauralRepresentation, 
		0, 
		MAX_BINAURAL_REPRESENTATION * sizeof(BinauralRepresentation *) );

	return pBinauralRendering;
}

int BinauralRendering_setBinauralRepresentation(
	BinauralRendering *pBinauralRendering, 
	BinauralRepresentation *pBinauralRepresentation,
	short idx)
{
	BinauralRepresentation *pNewBinauralRepresentation;

	if (idx >= MAX_BINAURAL_REPRESENTATION)
	{
		fprintf(stderr,"\nError : MAX_BINAURAL_REPRESENTATION reached");
		return -1;
	}

	pNewBinauralRepresentation = BinauralRepresentation_initAlloc();

	/* 1) copy BinauralRepresentation "header" */
	/*memcpy( pNewBinauralRepresentation, 
		pBinauralRepresentation, 
		sizeof(BinauralRepresentation) ); */

  pNewBinauralRepresentation->binauralDataFormatID = pBinauralRepresentation->binauralDataFormatID;
  pNewBinauralRepresentation->brirSamplingFrequency = pBinauralRepresentation->brirSamplingFrequency;
  pNewBinauralRepresentation->hoaOrderBinaural = pBinauralRepresentation->hoaOrderBinaural;
  pNewBinauralRepresentation->isHoaData = pBinauralRepresentation->isHoaData;
  pNewBinauralRepresentation->nBrirPairs = pBinauralRepresentation->nBrirPairs;
  memcpy(&pNewBinauralRepresentation->Setup_SpeakerConfig3d, &pBinauralRepresentation->Setup_SpeakerConfig3d, sizeof(Setup_SpeakerConfig3d));

	/* 2) copy the FIR or low-level params depending on binauralDataFormatID */
	switch (pBinauralRepresentation->binauralDataFormatID)
	{
	case 0:  /* 0 : raw FIR */     
		memcpy( pNewBinauralRepresentation->pBinauralFirData, 
			pBinauralRepresentation->pBinauralFirData, 
			sizeof(BinauralFirData) );
		break;

	case 1: /* 1 : FD parameters */       
		memcpy( pNewBinauralRepresentation->pFdBinauralRendererParam, 
			pBinauralRepresentation->pFdBinauralRendererParam, 
			sizeof(FdBinauralRendererParam) ); 
		break;

	case 2: /* 2 : TD parameters*/
		memcpy( pNewBinauralRepresentation->pTdBinauralRendererParam, 
			pBinauralRepresentation->pTdBinauralRendererParam, 
			sizeof(TdBinauralRendererParam) );
		break;

	default:
		fprintf(stderr,"\nError : Unknown binauralDataFormatID");
		return -1;

	}

	pBinauralRendering->ppBinauralRepresentation[idx] = pNewBinauralRepresentation;

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

/* ---------------------------------------------------------------------------*/
int BinauralRendering_addRepresentationFromWav_CICP_GEO(
	BinauralRendering *pBinauralRendering, 
	const char* BRIRpath, 
	int cicpIndex,		
  char* geoFileName,
  int useTrackingMode)
{
	FILE    *pfIn = NULL;
	long int  numSamples_32s, numChannels_32s;
	double	Fs_signal = 0;
	AFILE *afp=NULL;
	char *suffix[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
	unsigned long brirSamplingFrequency;
	short nBrirPairs;
	int i_ear, i_chan, i_samp, cnt;
	char filename[500];
	BinauralRepresentation *pBinauralRepresentation;
	BinauralFirData *pBinauralFirData;
	Setup_SpeakerConfig3d Setup_SpeakerConfig3d = {0};
	float *pDataIn_32f;
	CICP2GEOMETRY_ERROR cicpError;
	CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
	int numChannels = 0;
	int numLFEs = 0;
	int cntLFE = 0;

	/* check params */
	if (pBinauralRendering == 0)
	{
		fprintf(stderr, "\nError ->   init pBinauralRendering first !\n");
		return -1;
	}
	if (BRIRpath == 0)
	{
		fprintf(stderr, "\nError ->   BRIRpath is empty !\n");
		return -1;
	}

	pBinauralRepresentation = BinauralRepresentation_initAlloc();
	pBinauralFirData = (BinauralFirData *)malloc(sizeof(BinauralFirData));
	pDataIn_32f = (float *) malloc(MAX_BRIR_SIZE*2*sizeof(float));

	/* create BRIR suffix from CICP or Geo config */
	if (cicpIndex == -1) 
	{
		cicpError = cicp2geometry_get_geometry_from_file(geoFileName,    /**<  in: filename of textfile containing CICP channel indices or geometry data  */
			AzElLfe,      /**< out: pointer to an array with CICP2GEOMETRY_CHANNEL_GEOMETRYs */
			&numChannels, /**< out: number of effective output channels for chosen reproduction setup (i.e. w/o LFEs) */
			&numLFEs      /**< out: number of LFEs in chosen reproduction setup  */
			);
		if (cicpError != CICP2GEOMETRY_OK)
		{
			fprintf(stderr, "\nError ->   error reading Geo input file !\n");
			return -1;
		}
	}
	else
	{
		cicpError = cicp2geometry_get_geometry_from_cicp(cicpIndex,    /**<  in: cicpIndex indicating the reproduction setup  */
			AzElLfe,      /**< out: pointer to an array with CICP2GEOMETRY_CHANNEL_GEOMETRYs */
			&numChannels, /**< out: number of effective output channels for chosen reproduction setup (i.e. w/o LFEs) */
			&numLFEs      /**< out: number of LFEs in chosen reproduction setup  */
			);
		if (cicpError != CICP2GEOMETRY_OK)
		{
			fprintf(stderr, "\nError ->   error reading CICP config !\n");
			return -1;
		}
	}

	/* set Setup_SpeakerConfig3d */
	set_speakerConfig3d_from_geometry(AzElLfe, numChannels, numLFEs, &Setup_SpeakerConfig3d, cicpIndex);
	
	/* construct BRIR suffix from hp config */
	for (i_chan = 0; i_chan < numChannels+numLFEs; ++i_chan)
	{
		suffix[i_chan] = (char *)malloc(sizeof(char)*20);
		if (AzElLfe[i_chan].LFE == 1) {
			cntLFE++;
			sprintf(suffix[i_chan],"LFE%d",cntLFE);
		}
		else {
			if (AzElLfe[i_chan].Az >= 0) {
				sprintf(suffix[i_chan],"A+%03d",AzElLfe[i_chan].Az);
			}
			else {
				sprintf(suffix[i_chan],"A-%03d",-AzElLfe[i_chan].Az);
			}
			if (AzElLfe[i_chan].El >= 0) {
				sprintf(&suffix[i_chan][5],"_E+%02d",AzElLfe[i_chan].El);
			}
			else {
				sprintf(&suffix[i_chan][5],"_E-%02d",-AzElLfe[i_chan].El);
			}
		}
	}

	/* Load filters into BinauralFirData */
	nBrirPairs = numChannels+numLFEs;
	for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
	{
		strcpy(filename, BRIRpath); /* path (including prefix) */
		strcat(filename, suffix[i_chan]); /* suffix */
		strcat(filename, ".wav"); /* extension */

		if ( (FileExist(filename) != 1) || (filename[0] == '\0') )
		{
			fprintf(stderr,"Cannot open file %s\n",filename);
			return -1;
		}

		afp = AFopnRead(filename, &numSamples_32s, &numChannels_32s, &Fs_signal, pfIn);
		numSamples_32s = numSamples_32s / numChannels_32s; 
		if ( afp == 0 )
		{
			fprintf(stderr,"Cannot open file %s\n",filename);
			return -1;
		}

		/* Check that wavfile is stereo */
		if (numChannels_32s != 2)
		{
			fprintf(stderr, "\nError : %s\n ->   is not stereo !\n", filename);
			return -1;
		}

		/* Get fs */
		if (i_chan == 0)
		{
			brirSamplingFrequency = (unsigned long)Fs_signal;
		}
		else if (Fs_signal != brirSamplingFrequency)
		{
			fprintf(stderr, "\nError : %s\n ->   wrong samplerate !\n", filename);
			return -1;
		}

		/* Get filtersize  */
		if (i_chan == 0)
		{
			pBinauralFirData->Ntaps = numSamples_32s;
			if (pBinauralFirData->Ntaps > MAX_BRIR_SIZE)
			{
				fprintf(stderr, "\nError : %s\n ->  max BRIR size (%d samples) reached !\n", filename, MAX_BRIR_SIZE);
				return -1;
			}
		}
		else /* Check that size is the same for all BRIRs */
		{
			if (numSamples_32s != pBinauralFirData->Ntaps)
			{
				fprintf(stderr, "\nError : %s\n ->   filter size is not correct !\n", filename);
				return -1;
			}
		}

		/* Load interleaved samples and copy IR */
		AFfReadData(afp, 0, pDataIn_32f, numSamples_32s*numChannels_32s);
		cnt = 0;
		for (i_samp = 0; i_samp <  numSamples_32s; ++i_samp)
		{
			for (i_ear = 0; i_ear < 2; ++i_ear)
			{
				pBinauralFirData->pppTaps[i_ear][i_chan][i_samp] = pDataIn_32f[cnt];
				cnt++;
			}
		}

	} /* loop i_chan */

	/* Complete BinauralFirData struct manually */
	pBinauralFirData->allCutFreq = 1;

	/* Add BinauralFirData to a BinauralRepresentation */
	BinauralRepresentation_setFIR_CICP_GEO(
		pBinauralRepresentation,
		brirSamplingFrequency,
		nBrirPairs,
		&Setup_SpeakerConfig3d,
		pBinauralFirData);

	/* Add BinauralRepresentation to BinauralRendering */
	if (BinauralRendering_setBinauralRepresentation(
		pBinauralRendering, 
		pBinauralRepresentation,
		pBinauralRendering->numBinauralRepresentation) != 0)
	{
		return -1;
	}
	else
	{
		pBinauralRendering->numBinauralRepresentation++;
	}
  pBinauralRendering->useTrackingMode = useTrackingMode;

	/* Free */
	free(pBinauralRepresentation);
	free(pBinauralFirData);
	free(pDataIn_32f);

	return 0;

};

/* ---------------------------------------------------------------------------*/
int BinauralRendering_addRepresentationFromWav_HOA(
	BinauralRendering *pBinauralRendering, 
	const char* BRIRpath, 
	int hoaOrder,
  int useTrackingMode)
{
	FILE    *pfIn = NULL;
	long int  numSamples_32s, numChannels_32s;
	double	Fs_signal = 0;
	AFILE *afp=NULL;
	char *suffix[MAX_NUM_BRIR_PAIRS];
	unsigned long brirSamplingFrequency;
	short nBrirPairs;
    int i_ear, i_chan, i_samp, cnt;
	char filename[500];
	BinauralRepresentation *pBinauralRepresentation;
	BinauralFirData *pBinauralFirData; 
	float *pDataIn_32f;
	int n, m;

	/* check params */
	if (pBinauralRendering == 0)
	{
		fprintf(stderr, "\nError ->   init pBinauralRendering first !\n");
		return -1;
	}
	if (BRIRpath == 0)
	{
		fprintf(stderr, "\nError ->   BRIRpath is empty !\n");
		return -1;
	}

	pBinauralRepresentation = BinauralRepresentation_initAlloc();
	pBinauralFirData = (BinauralFirData *)malloc(sizeof(BinauralFirData));
	pDataIn_32f = (float *) malloc(MAX_BRIR_SIZE*2*sizeof(float));

	/* create BRIR suffix from HOA order */
	nBrirPairs = (hoaOrder+1)*(hoaOrder+1);
	if (nBrirPairs > MAX_NUM_BRIR_PAIRS)
	{
		fprintf(stderr, "\nError ->  max number of HOA BRIRs is %d !\n", MAX_NUM_BRIR_PAIRS);
		return -1;
	}

	/* construct BRIR suffix from HOA order : "linear" component order */
	i_chan = 0;
	for (n=0; n<=hoaOrder; ++n)
	{
		for (m=-n; m<=n; ++m)
		{
			suffix[i_chan] = (char *)malloc(sizeof(char)*20);
			if (m < 0)
				sprintf(suffix[i_chan],"%d_%d%d-",hoaOrder,n,-m);
			else
				sprintf(suffix[i_chan],"%d_%d%d+",hoaOrder,n,m);

			i_chan++;
		}
	}

	/* Load filters into BinauralFirData */
	for (i_chan = 0; i_chan < nBrirPairs; ++i_chan)
	{
		strcpy(filename, BRIRpath); /* path (including prefix) */
		strcat(filename, suffix[i_chan]); /* suffix */
		strcat(filename, ".wav"); /* extension */

		if ( (FileExist(filename) != 1) || (filename[0] == '\0') )
		{
			fprintf(stderr,"Cannot open file %s\n",filename);
			return -1;
		}

		afp = AFopnRead(filename, &numSamples_32s, &numChannels_32s, &Fs_signal, pfIn);
		numSamples_32s = numSamples_32s / numChannels_32s; 
		if ( afp == 0 )
		{
			fprintf(stderr,"Cannot open file %s\n",filename);
			return -1;
		}

		/* Check that wavfile is stereo */
		if (numChannels_32s != 2)
		{
			fprintf(stderr, "\nError : %s\n ->   is not stereo !\n", filename);
			return -1;
		}

		/* Get fs */
		if (i_chan == 0)
		{
			brirSamplingFrequency = (unsigned long)Fs_signal;
		}
		else if (Fs_signal != brirSamplingFrequency)
		{
			fprintf(stderr, "\nError : %s\n ->   wrong samplerate !\n", filename);
			return -1;
		}

		/* Get filtersize  */
		if (i_chan == 0)
		{
			pBinauralFirData->Ntaps = numSamples_32s;
			if (pBinauralFirData->Ntaps > MAX_BRIR_SIZE)
			{
				fprintf(stderr, "\nError : %s\n ->  max BRIR size (%d samples) reached !\n", filename, MAX_BRIR_SIZE);
				return -1;
			}
		}
		else /* Check that size is the same for all BRIRs */
		{
			if (numSamples_32s != pBinauralFirData->Ntaps)
			{
				fprintf(stderr, "\nError : %s\n ->   filter size is not correct !\n", filename);
				return -1;
			}
		}

		/* Load interleaved samples and copy IR */
		AFfReadData(afp, 0, pDataIn_32f, numSamples_32s*numChannels_32s);
		cnt = 0;
		for (i_samp = 0; i_samp <  numSamples_32s; ++i_samp)
		{
			for (i_ear = 0; i_ear < 2; ++i_ear)
			{
				pBinauralFirData->pppTaps[i_ear][i_chan][i_samp] = pDataIn_32f[cnt];
				cnt++;
			}
		}

	} /* loop i_chan */

	/* Complete BinauralFirData struct manually */
	pBinauralFirData->allCutFreq = 1;

	/* Add BinauralFirData to a BinauralRepresentation */
	BinauralRepresentation_setFIR_HOA(
		pBinauralRepresentation,
		brirSamplingFrequency,
		hoaOrder,
		pBinauralFirData);

	/* Add BinauralRepresentation to BinauralRendering */
	if (BinauralRendering_setBinauralRepresentation(
		pBinauralRendering, 
		pBinauralRepresentation,
		pBinauralRendering->numBinauralRepresentation) != 0)
	{
		return -1;
	}
	else
	{
		pBinauralRendering->numBinauralRepresentation++;
	}
  pBinauralRendering->useTrackingMode = useTrackingMode;

	/* Free */
	free(pBinauralRepresentation);
	free(pBinauralFirData);
	free(pDataIn_32f);

	return 0;

};



/*-------------------  Read/Write binaural bitstream to/from BinauralRendering  -----------------------*/

int bitstreamRead(const char* filename, BinauralRendering* pBinauralRendering)
{
	FILE*	pBS = fopen(filename, "rb"); /* bitstream ptr */
	unsigned long ii;
	short i_representation;
	BinauralRepresentation *pNewBinauralRepresentation = NULL;

	unsigned char *bitbuf;
	unsigned long temp_uL;

	long	nBytes, nBits;
	robitbuf readbitBuffer;
	robitbufHandle hBitstream = &readbitBuffer;

	if (pBS == NULL)
	{
		fprintf(stderr,"\n Error in bitstreamRead() : cannot open bs file");
		return -1;
	}
	else{
	    fprintf(stderr,"Read bitstream from file: \n %s.\n", filename); 
	}

	/* load all bitstream file into the bitbuffer */
	fseek(pBS, 0L, SEEK_END);
	nBytes = ftell(pBS);
	nBits = 8 * nBytes;
	fseek(pBS, 0L, SEEK_SET); /* rewind */
	bitbuf = (unsigned char*)malloc(nBytes * sizeof(unsigned char));
	fread(bitbuf, 1, nBytes, pBS);
	fclose(pBS);
	robitbuf_Init(hBitstream, bitbuf, nBits, 0);


	/* Read BinauralRendering "header" */
	pBinauralRendering->fileSignature = robitbuf_ReadBits(hBitstream, 32); /* bsFileSignature */
	pBinauralRendering->fileVersion = (unsigned short) robitbuf_ReadBits(hBitstream, 8); /* bsFileVersion */
	temp_uL = robitbuf_ReadBits(hBitstream, 8); /* bsNumCharName */
	for (ii = 0; ii < temp_uL; ++ii)
	{
		pBinauralRendering->name[ii] = (char) robitbuf_ReadBits(hBitstream, 8); /* bsName[ii] */
	}
	pBinauralRendering->name[ii] = '\0'; /* add end of string */
	pBinauralRendering->useTrackingMode = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* useTrackingMode */
	pBinauralRendering->numBinauralRepresentation = (unsigned short) robitbuf_ReadBits(hBitstream, 4); /* bsNumBinauralDataRepresentation */

	/* Read BinauralRepresentations */
	pNewBinauralRepresentation = BinauralRepresentation_initAlloc();
	for (i_representation = 0; 
		i_representation < pBinauralRendering->numBinauralRepresentation; 
		i_representation++)
	{
		temp_uL = robitbuf_ReadBits(hBitstream, 5); /* brirSamplingFrequencyIndex */
		if (temp_uL == 0x1F)
		{
			pNewBinauralRepresentation->brirSamplingFrequency = robitbuf_ReadBits(hBitstream, 24); /* brirSamplingFrequency */
		}
		else
		{
			pNewBinauralRepresentation->brirSamplingFrequency = brirSamplingFrequencyTable[temp_uL];
		}
		pNewBinauralRepresentation->isHoaData = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* isHoaData */
		if (pNewBinauralRepresentation->isHoaData)
		{
			pNewBinauralRepresentation->hoaOrderBinaural = ReadEscapedValue(hBitstream,3,5,0); /* hoaOrderBinaural  */
			pNewBinauralRepresentation->nBrirPairs = (pNewBinauralRepresentation->hoaOrderBinaural+1)*(pNewBinauralRepresentation->hoaOrderBinaural+1);
		}
		else
		{
			/* load Setup_SpeakerConfig3d */
			readSpeakerConfig3d(&pNewBinauralRepresentation->Setup_SpeakerConfig3d, hBitstream);

			if (pNewBinauralRepresentation->Setup_SpeakerConfig3d.speakerLayoutType == 0)
			{
				pNewBinauralRepresentation->nBrirPairs = ReadEscapedValue(hBitstream,5,8,0) + 1; /* nBrirPairs  */
			}
			else
			{
				if (pNewBinauralRepresentation->Setup_SpeakerConfig3d.numSpeakers > 32767)
				{
					fprintf(stderr,"\nError : nBrirPairs is a signed 16-bit short number <= 32767");
					return -1;
				}
				pNewBinauralRepresentation->nBrirPairs = (short) pNewBinauralRepresentation->Setup_SpeakerConfig3d.numSpeakers;
			}
		}

		pNewBinauralRepresentation->binauralDataFormatID = (short) robitbuf_ReadBits(hBitstream, 2); /* binauralDataFormatID */
		robitbuf_ByteAlign(hBitstream);

		/* check nBrirPairs */
		if (pNewBinauralRepresentation->nBrirPairs > MAX_NUM_BRIR_PAIRS)
		{
			fprintf(stderr,"\nError : nBrirPairs should be <= %d", MAX_NUM_BRIR_PAIRS);
			return -1;
		}
		if ( (pNewBinauralRepresentation->isHoaData == 0) && 
			 (pNewBinauralRepresentation->nBrirPairs > CICP2GEOMETRY_MAX_LOUDSPEAKERS) )
		{
			fprintf(stderr,"\nError : nBrirPairs should be <= %d for channel-based configurations !", CICP2GEOMETRY_MAX_LOUDSPEAKERS);
			return -1;
		}

		switch (pNewBinauralRepresentation->binauralDataFormatID)
		{
		case 0:  /* 0 : raw FIR */     
			if ( readBinauralFirData(
				pNewBinauralRepresentation->pBinauralFirData, 
				pNewBinauralRepresentation->nBrirPairs,
				hBitstream) != 0) return -1;
			break;

		case 1: /* 1 : FD parameters */      
			if ( readFdBinauralRendererParam(
				pNewBinauralRepresentation->pFdBinauralRendererParam, 
				pNewBinauralRepresentation->nBrirPairs,
				hBitstream) != 0) return -1;  
			break;

		case 2: /* 2 : TD parameters*/
			if ( readTdBinauralRendererParam(
				pNewBinauralRepresentation->pTdBinauralRendererParam, 
				pNewBinauralRepresentation->nBrirPairs,
				hBitstream) != 0) return -1;
			break;

		default:
			fprintf(stderr,"\nError : Unknown binauralDataFormatID");
			return -1;

		}

		/* set representation in BinauralRendering */
		if ( BinauralRendering_setBinauralRepresentation(
			pBinauralRendering, 
			pNewBinauralRepresentation,
			i_representation) != 0) return -1;
	}
  free(bitbuf);
	return 0;
}

int bitstreamWrite(const char* filename, unsigned long fileSignature, unsigned short  fileVersion, BinauralRendering* pBinauralRendering)
{
	FILE*	pBS = fopen(filename, "wb");
	unsigned long ii;
	short i_representation;
	BinauralRepresentation *pBinauralRepresentation = NULL;

	unsigned char *bitbuf;

	long	nBytes, nBits;
	wobitbuf writebitBuffer;
	wobitbufHandle hBitstream = &writebitBuffer;
	unsigned short numCharName, brirSamplingFrequencyIdx;

	if (pBS == NULL)
	{
		fprintf(stderr,"\n Error in bitstreamWrite() : cannot open bs file");
		return -1;
	}
	else{
		fprintf(stderr,"Write to output bitstream file: \n %s.\n", filename); 
	}

	/* write all bitstream file into the bitbuffer */
	nBytes = BITSTREAM_MAX_NUM_MEGA_BYTES * 1024 * 1024;
	nBits = 8 * nBytes;
	bitbuf = (unsigned char*)malloc(nBytes * sizeof(unsigned char));
	wobitbuf_Init(hBitstream, bitbuf, nBits, 0);

	/* Write BinauralRendering "header" */
	wobitbuf_WriteBits(hBitstream, fileSignature, 32);                               /* bsFileSignature */
	wobitbuf_WriteBits(hBitstream, fileVersion, 8);                                  /* bsFileVersion */
	numCharName = (strlen(filename) < 255) ? (unsigned short)strlen(filename) : 255; /* max 255 char */
	wobitbuf_WriteBits(hBitstream, numCharName, 8);                                  /* bsNumCharName */
	for (ii = 0; ii < numCharName; ++ii)
	{
		wobitbuf_WriteBits(hBitstream, filename[ii], 8); /* bsName[ii] */
	}
	wobitbuf_WriteBits(hBitstream, pBinauralRendering->useTrackingMode, 1); /* useTrackingMode */
	wobitbuf_WriteBits(hBitstream, pBinauralRendering->numBinauralRepresentation, 4); /* bsNumBinauralDataRepresentation */

	/* Write BinauralRepresentations */
	for (i_representation=0; i_representation<pBinauralRendering->numBinauralRepresentation; ++i_representation)
	{
		pBinauralRepresentation = pBinauralRendering->ppBinauralRepresentation[i_representation];
		brirSamplingFrequencyIdx = getBrirSamplingFrequencyIdx(pBinauralRepresentation->brirSamplingFrequency); 
		wobitbuf_WriteBits(hBitstream, brirSamplingFrequencyIdx, 5); /* brirSamplingFrequencyIndex */
		if (brirSamplingFrequencyIdx == 0x1F)
		{
			wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->brirSamplingFrequency, 24); /* brirSamplingFrequency */
		}
		wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->isHoaData, 1); /* isHoaData */
		if (pBinauralRepresentation->isHoaData)
		{
			WriteEscapedValue(hBitstream, pBinauralRepresentation->hoaOrderBinaural, 3,5,0); /* hoaOrderBinaural  */
		}
		else
		{
			/* write Setup_SpeakerConfig3d */
			writeSpeakerConfig3d(&pBinauralRepresentation->Setup_SpeakerConfig3d, hBitstream);

			if (pBinauralRepresentation->Setup_SpeakerConfig3d.speakerLayoutType == 0)
			{
				WriteEscapedValue(hBitstream, pBinauralRepresentation->nBrirPairs - 1, 5,8,0); /* nBrirPairs (-1) */
			}
		}

		wobitbuf_WriteBits(hBitstream, pBinauralRepresentation->binauralDataFormatID, 2); /* binauralDataFormatID */
		wobitbuf_ByteAlign(hBitstream);

		switch (pBinauralRepresentation->binauralDataFormatID)
		{
		case 0:  /* 0 : raw FIR */     
			writeBinauralFirData(
				pBinauralRepresentation->pBinauralFirData, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream); 
			break;

		case 1: /* 1 : FD parameters */      
			writeFdBinauralRendererParam(
				pBinauralRepresentation->pFdBinauralRendererParam, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream); 
			break;

		case 2: /* 2 : TD parameters*/
			writeTdBinauralRendererParam(
				pBinauralRepresentation->pTdBinauralRendererParam, 
				pBinauralRepresentation->nBrirPairs,
				hBitstream);
			break;

		default:
			fprintf(stderr,"\nError : Unknown binauralDataFormatID");
			return -1;

		}
	}

	/* write bitbuffer to file */
	wobitbuf_ByteAlign(hBitstream);
	nBits = wobitbuf_GetBitsWritten(hBitstream);
	nBytes = nBits / 8;
	fwrite(bitbuf, nBytes, 1, pBS);

	fclose(pBS);
  free(bitbuf);
	return 0;

}
