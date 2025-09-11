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


/*----------------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"

#include "math.h"
#include "binauralization.h"

/*----------------------------------------------------------------------------*/
void floatMemAlloc(float*** ptr_32f, int size1_32s, int size2_32s)
{
	int	ind_32s;

	*ptr_32f = (float**)malloc(size1_32s*sizeof(float*));
	
	for( ind_32s=0 ; ind_32s<size1_32s ; ind_32s++)
	{
		(*ptr_32f)[ind_32s] = (float*)malloc(size2_32s*sizeof(float));
	}
}

/*----------------------------------------------------------------------------*/
int maximumInteger(int i1, int i2)
{
	if(i1 > i2)
	{
		return i1;
	}
	else
	{
		return i2;
	}
}

/*----------------------------------------------------------------------------*/
int Set_32f (
	float		val_32f,
	float		*pDst_32f,
	int			len_32s)
{
	int	ind_32s;

	if(len_32s<0)
	{
		return -1;
	}

	for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++ )
    {
        *pDst_32f++ = val_32f;
    }
	return 0;
}

/*----------------------------------------------------------------------------*/
int Copy_32f (
	const float		*pSrc_32f,
    float			*pDst_32f,
    int				len_32s )
{
    int ind_32s;

    if(len_32s<0)
	{
		return -1;
	}


    for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++ )
    {
        pDst_32f[ind_32s] = pSrc_32f[ind_32s];
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int Add_32f_I(
	const float		*pSrc_32f,
	float			*pSrcDst_32f,
    int				len_32s )
{
    int ind_32s;

    if(len_32s<0)
	{
		return -1;
	}


    for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++ )
    {
        pSrcDst_32f[ind_32s] +=  pSrc_32f[ind_32s];
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int  MulC_32f ( 
	const float		*pSrc_32f,
    float			val_32f,
    float			*pDst_32f,
    int				len_32s )
{
    int ind_32s;
	
	if(len_32s<0)
	{
		return -1;
	}

    for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++)
    {
        pDst_32f[ind_32s] = pSrc_32f[ind_32s] * val_32f;

    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int MulC_32f_I(
	float		val_32f,
	float		*pSrcDst_32f,
	int			len_32s )
{
    int ind_32s;


    if(len_32s<0)
	{
		return -1;
	}

    for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++ )
    {
        pSrcDst_32f[ind_32s] *= val_32f;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int MulCAdd_32f(
	const float		*pSrc_32f,
    float			val_32f,
    float			*pDst_32f,
    int				len_32s )
{
    int ind_32s;

	if(len_32s<0)
	{
		return -1;
	}


    for ( ind_32s = 0 ; ind_32s < len_32s ; ind_32s++ )
    {
        pDst_32f[ind_32s] += pSrc_32f[ind_32s] * val_32f;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int MulPerm_32f(
    const float		*pSrc1_32fc,
    const float		*pSrc2_32fc,
    float			*pDst_32fc,
    int				len_32s )
{
    int ind_32s;

    *pDst_32fc++ = pSrc1_32fc++[0] * pSrc2_32fc++[0];
    *pDst_32fc++ = pSrc1_32fc++[0] * pSrc2_32fc++[0];

    for ( ind_32s = 1; ind_32s < len_32s ; ind_32s++ )
    {
        *pDst_32fc =    pSrc1_32fc[0] * pSrc2_32fc[0];
        *pDst_32fc++ =  *pDst_32fc - pSrc1_32fc[1] * pSrc2_32fc[1];

        *pDst_32fc =  *pSrc1_32fc++ * pSrc2_32fc[1];
        *pDst_32fc++ =  *pDst_32fc + *pSrc1_32fc++ * *pSrc2_32fc++;
        pSrc2_32fc++;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int MulAddPerm_32fc(
    const float		*pSrc1_32fc,
    const float		*pSrc2_32fc,
    float			*pDst_32fc,
    int				len_32s )
{
    int ind_32s;

    *pDst_32fc++ += *pSrc1_32fc++ * *pSrc2_32fc++;
    *pDst_32fc++ += *pSrc1_32fc++ * *pSrc2_32fc++;


    for ( ind_32s = 1; ind_32s < len_32s ; ind_32s++ )
    {
        *pDst_32fc =    *pDst_32fc + *pSrc1_32fc * *pSrc2_32fc;
        *pDst_32fc++ -=  pSrc1_32fc[1] * pSrc2_32fc[1];

        *pDst_32fc =    *pDst_32fc + *pSrc1_32fc++ * pSrc2_32fc[1];
        *pDst_32fc++ += *pSrc1_32fc++ * *pSrc2_32fc++;
        pSrc2_32fc++;
    }

	return ERROR_OK;
}

/*----------------------------------------------------------------------------*/
void FFT_Complex_to_PermFmt (
	float				*pXr_32f,
	float				*pXi_32f,
	float				*pPerm_32f,
	int					FFTsize_16s)
{
	int ind_16s;
	int NyquistInd_16s;

	if(!pPerm_32f)
	{
		pPerm_32f = (float*)malloc(FFTsize_16s*sizeof(float));
	}

	NyquistInd_16s = FFTsize_16s / 2;	/* N/2+1 (one-based) */

	/* First val : Freq 0  */
	pPerm_32f[0] = pXr_32f[0];

	/* Second val : Freq Nyquist  */
	pPerm_32f[1] = pXr_32f[NyquistInd_16s];


	/*  Third val to N */
	for(ind_16s=1 ; ind_16s<NyquistInd_16s ; ind_16s++)
	{
		/* Real */
		pPerm_32f[2*ind_16s] = pXr_32f[ind_16s];
		
		/* Im */
		pPerm_32f[2*ind_16s+1] = -1 * pXi_32f[ind_16s];
	}
}

/*----------------------------------------------------------------------------*/
void FFT_Complex_to_PermFmt_I (
	float				*pXr_32f,
	float				*pXi_32f,
	int					FFTsize_16s)
{
	int ind_16s;
	int NyquistInd_16s;

	float *tempBuf_32f = NULL;

	tempBuf_32f = (float*)malloc(FFTsize_16s*sizeof(float));

	Copy_32f(	pXr_32f,
				tempBuf_32f,
				FFTsize_16s);


	NyquistInd_16s = FFTsize_16s / 2;	/* N/2+1 (one-based) */

	/* First val : Freq 0  */
	pXr_32f[0] = tempBuf_32f[0];

	/* Second val : Freq Nyquist  */
	pXr_32f[1] = tempBuf_32f[NyquistInd_16s];


	/*  Third val to N */
	for(ind_16s=1 ; ind_16s<NyquistInd_16s ; ind_16s++)
	{
		/* Real */
		pXr_32f[2*ind_16s] = tempBuf_32f[ind_16s];
		
		/* Im */
		pXr_32f[2*ind_16s+1] = -1 * pXi_32f[ind_16s];
	}

	free(tempBuf_32f);
}

/*----------------------------------------------------------------------------*/
void FFT_PermFmt_to_Complex (
	float				*pPerm_32f,
	float				*pXr_32f,
	float				*pXi_32f,
	int					FFTsize_16s)
{
	int ind_16s;
	int NyquistInd_16s;

	if(!pXr_32f)
	{
		pXr_32f = (float*)malloc(FFTsize_16s*sizeof(float));
	}

	if(!pXi_32f)
	{
		pXi_32f = (float*)malloc(FFTsize_16s*sizeof(float));
	}

	NyquistInd_16s = FFTsize_16s / 2;	/* N/2+1 (one-based) */

	/* First val : Freq 0  */
	pXr_32f[0] = pPerm_32f[0];

	/* Second val : Freq Nyquist  */
	pXr_32f[NyquistInd_16s] = pPerm_32f[1];

	/*  Third val to N */
	for(ind_16s=1 ; ind_16s<NyquistInd_16s ; ind_16s++)
	{
		/* Real */
		pXr_32f[ind_16s] = pPerm_32f[2*ind_16s];
		pXr_32f[FFTsize_16s-ind_16s] = pXr_32f[ind_16s];
		
		/* Im */
		pXi_32f[ind_16s] = -1 * pPerm_32f[2*ind_16s+1];
		pXi_32f[FFTsize_16s-ind_16s] = -1* pXi_32f[ind_16s];
	}
}

/*----------------------------------------------------------------------------*/
void FFT_PermFmt_to_Complex_I (
	float				*pXr_32f,
	float				*pXi_32f,
	int					FFTsize_16s)
{
	int ind_16s;
	int NyquistInd_16s;
	float *tempBuf_32f = NULL;


	if(!pXr_32f)
	{
		return;
	}

	if(!pXi_32f)
	{
		pXi_32f = (float*)malloc(FFTsize_16s*sizeof(float));
	}

	tempBuf_32f = (float*)malloc(FFTsize_16s*sizeof(float));

	Copy_32f(	pXr_32f,
				tempBuf_32f,
				FFTsize_16s);

	NyquistInd_16s = FFTsize_16s / 2;	/* N/2+1 (one-based) */

	/* First val : Freq 0  */
	pXr_32f[0] = tempBuf_32f[0];

	/* Second val : Freq Nyquist  */
	pXr_32f[NyquistInd_16s] = tempBuf_32f[1];

	/*  Third val to N */
	for(ind_16s=1 ; ind_16s<NyquistInd_16s ; ind_16s++)
	{
		/* Real */
		pXr_32f[ind_16s] = tempBuf_32f[2*ind_16s];
		pXr_32f[FFTsize_16s-ind_16s] = pXr_32f[ind_16s];
		
		/* Im */
		pXi_32f[ind_16s] = -1 * tempBuf_32f[2*ind_16s+1];
		pXi_32f[FFTsize_16s-ind_16s] = -1* pXi_32f[ind_16s];
	}

	free(tempBuf_32f);
}

/*----------------------------------------------------------------------------*/
int BinauralStruct_initAlloc_32f (
	int						fftSize_16s,
	int						DirectLen_16s,
	int						DiffuseLen_16s,
	int						nbDiffuseBlocks_16s,
	int						numberOfInput_16s,
	int						numberOfOutput_16s,
	BinauralStruct_32f		**ppBinaural )

	/* Construction */
{
	int len_16s, indN_16s, indM_16s, indK_16s;
	int fftMinSize_16s;


	if( ppBinaural == NULL )
	{
		return ERROR_BAD_PARAM;
	}

	*ppBinaural = NULL;

	*ppBinaural = (BinauralStruct_32f*)malloc( sizeof( BinauralStruct_32f ) );

	if ( *ppBinaural == NULL )
	{
		return ERROR_INSUFFICIENT_MEM;
	}

	if(DirectLen_16s>BINAURAL_DIR_MAXLEN)
	{
		fprintf(stderr,"Direct len must be < %d\n", BINAURAL_DIR_MAXLEN);
		return ERROR_BAD_PARAM;
	}
	if(DiffuseLen_16s>BINAURAL_DIF_MAXLEN)
	{
		fprintf(stderr,"Diffuse len must be < %d\n", BINAURAL_DIF_MAXLEN);
		return ERROR_BAD_PARAM;
	}

	if(fftSize_16s>BINAURAL_FFT_MAXLEN)
	{
		fftSize_16s = BINAURAL_FFT_MAXLEN;
		fprintf(stderr,"FFT size is forced to %d (max)\n", BINAURAL_FFT_MAXLEN);
	}

	indN_16s = 1;
	do
	{
		 fftMinSize_16s = (int)pow(2.0, indN_16s);
		 indN_16s++;
	}
	while( !(fftMinSize_16s>=2*DirectLen_16s) );

	if(fftMinSize_16s>fftSize_16s)
	{
		(*ppBinaural)->m_FFTsize_16s = fftMinSize_16s;
	}
	else
	{
		(*ppBinaural)->m_FFTsize_16s = fftSize_16s;
	}

	(*ppBinaural)->m_indexWriteInputBuffer_16s = 0;
	(*ppBinaural)->m_filterSize_16s = 0;
	(*ppBinaural)->m_numberOfInput_16s = numberOfInput_16s;
	(*ppBinaural)->m_numberOfOutput_16s = numberOfOutput_16s;

	len_16s = (*ppBinaural)->m_FFTsize_16s;

	if(nbDiffuseBlocks_16s<0)
	{
		nbDiffuseBlocks_16s = 2 * DiffuseLen_16s / (*ppBinaural)->m_FFTsize_16s;

		if(len_16s % DiffuseLen_16s != 0)
		{
			nbDiffuseBlocks_16s++;
		}
	}

	(*ppBinaural)->m_nbDiffuseBlocks_16s = nbDiffuseBlocks_16s;


	(*ppBinaural)->m_ppOutputMemoryBuffer_32f =
		(float **)malloc( numberOfOutput_16s * sizeof( float *) );

	if( (*ppBinaural)->m_ppOutputMemoryBuffer_32f == NULL )
	{
		free(*ppBinaural);
		*ppBinaural = NULL;

		return ERROR_INSUFFICIENT_MEM;
	}

	for( indM_16s = 0; indM_16s < numberOfOutput_16s ; indM_16s++ )
	{
		(*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s] =
			(float *)malloc( len_16s * sizeof( float ) );

		Set_32f(
			0,
			(*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s],
			(*ppBinaural)->m_FFTsize_16s );

	}

	(*ppBinaural)->m_pppTapsDirect_32f =
		(float ***)malloc( numberOfOutput_16s * sizeof( float** ) );

	for( indM_16s = 0; indM_16s < numberOfOutput_16s ; indM_16s++ )
	{

		(*ppBinaural)->m_pppTapsDirect_32f[indM_16s] =
			(float **)malloc( numberOfInput_16s * sizeof( float* ) );


		if((*ppBinaural)->m_pppTapsDirect_32f[indM_16s] == NULL)
		{
			free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s] );
			free(*ppBinaural);
			*ppBinaural = NULL;

			return ERROR_INSUFFICIENT_MEM;
		}

		for ( indN_16s = 0; indN_16s < numberOfInput_16s ; indN_16s++ )
		{
			(*ppBinaural)->m_pppTapsDirect_32f[indM_16s][indN_16s] =
				(float *) malloc( len_16s * sizeof(float) );

			if((*ppBinaural)->m_pppTapsDirect_32f[indM_16s][indN_16s] == NULL)
			{

				free((*ppBinaural)->m_ppInputMemoryBuffer_32f);
				free((*ppBinaural)->m_pppTapsDirect_32f);
				free((*ppBinaural)->m_pppTapsDiffuse_32f);
				free((*ppBinaural)->m_ppOutputMemoryBuffer_32f);
				free(*ppBinaural);
				*ppBinaural = NULL;

				return ERROR_INSUFFICIENT_MEM;
			}
		}
	}

	(*ppBinaural)->m_pppTapsDiffuse_32f =
		(float ***)malloc( (*ppBinaural)->m_nbDiffuseBlocks_16s * sizeof( float** ) );

	for( indK_16s = 0; indK_16s < (*ppBinaural)->m_nbDiffuseBlocks_16s ; indK_16s++ )
	{

		(*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s] =
			(float **)malloc( numberOfOutput_16s * sizeof( float* ) );


		if((*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s] == NULL)
		{
			free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s] );
			free(*ppBinaural);
			*ppBinaural = NULL;

			return ERROR_INSUFFICIENT_MEM;
		}

		for ( indM_16s = 0; indM_16s < numberOfOutput_16s ; indM_16s++ )
		{
			(*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s][indM_16s] =
				(float *) malloc( len_16s * sizeof(float) );

			if((*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s][indM_16s] == NULL)
			{

				free((*ppBinaural)->m_ppInputMemoryBuffer_32f);
				free((*ppBinaural)->m_pppTapsDiffuse_32f);
				free((*ppBinaural)->m_pppTapsDirect_32f);
				free((*ppBinaural)->m_ppOutputMemoryBuffer_32f);

				free(*ppBinaural);
				*ppBinaural = NULL;

				return ERROR_INSUFFICIENT_MEM;
			}
		}
	}



	(*ppBinaural)->m_ppInputMemoryBuffer_32f =
		(float **)malloc( numberOfInput_16s * sizeof( float* ) );

	if((*ppBinaural)->m_ppInputMemoryBuffer_32f == NULL)
	{
		free( (*ppBinaural)->m_pppTapsDirect_32f );
		free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f );
		free(*ppBinaural);
		*ppBinaural = NULL;

		return ERROR_INSUFFICIENT_MEM;
	}

	for ( indN_16s = 0; indN_16s < numberOfInput_16s ; indN_16s++ )
	{
		(*ppBinaural)->m_ppInputMemoryBuffer_32f[indN_16s] =
			(float *)malloc( len_16s * sizeof(float) );


		if( (*ppBinaural)->m_ppInputMemoryBuffer_32f[indN_16s] == NULL )
		{
			for ( len_16s = 0 ; len_16s < indN_16s ; len_16s ++ )
			{
				free( (*ppBinaural)->
					m_ppInputMemoryBuffer_32f[len_16s] );

				free( (*ppBinaural)->m_pppTapsDirect_32f[indM_16s][len_16s] );
			}

			free( (*ppBinaural)->m_pppTapsDirect_32f[indM_16s][indN_16s] );
			free( (*ppBinaural)->m_ppInputMemoryBuffer_32f );
			free( (*ppBinaural)->m_pppTapsDirect_32f );
			free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f );

			free(*ppBinaural);
			*ppBinaural = NULL;

			return ERROR_INSUFFICIENT_MEM;
		}

		Set_32f(
			0,
			(*ppBinaural)->m_ppInputMemoryBuffer_32f[indN_16s],
			len_16s );
	}

	(*ppBinaural)->m_ppPrevInputMemoryBuffer_32f =
		(float **)malloc( ((*ppBinaural)->m_nbDiffuseBlocks_16s+1) * sizeof( float* ) );

	if((*ppBinaural)->m_ppPrevInputMemoryBuffer_32f == NULL)
	{
		free( (*ppBinaural)->m_pppTapsDirect_32f );
		free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f );

		free(*ppBinaural);
		*ppBinaural = NULL;

		return ERROR_INSUFFICIENT_MEM;
	}

	for ( indK_16s = 0; indK_16s < (*ppBinaural)->m_nbDiffuseBlocks_16s+1 ; indK_16s++ )
	{
		(*ppBinaural)->m_ppPrevInputMemoryBuffer_32f[indK_16s] =
			(float *)malloc( len_16s * sizeof(float) );


		if( (*ppBinaural)->m_ppPrevInputMemoryBuffer_32f[indK_16s] == NULL )
		{
			for ( len_16s = 0 ; len_16s < indN_16s ; len_16s ++ )
			{
				free( (*ppBinaural)->
					m_ppPrevInputMemoryBuffer_32f[indK_16s] );

				free( (*ppBinaural)->m_pppTapsDirect_32f[indM_16s][len_16s] );
			}


			free( (*ppBinaural)->m_ppInputMemoryBuffer_32f );
			free( (*ppBinaural)->m_pppTapsDirect_32f );

			for ( indM_16s = 0; indM_16s < numberOfOutput_16s ; indM_16s++ )
			{
				free( (*ppBinaural)->m_pppTapsDirect_32f[indM_16s][indN_16s] );
				free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s] );
			}

			free(*ppBinaural);
			*ppBinaural = NULL;

			return ERROR_INSUFFICIENT_MEM;
		}

		(*ppBinaural)->m_pDiffuseWeight_32f =
			(float *)malloc( (*ppBinaural)->m_numberOfInput_16s * sizeof(float) );

		Set_32f(
			0,
			(*ppBinaural)->m_ppPrevInputMemoryBuffer_32f[indK_16s],
			len_16s );

	}

	(*ppBinaural)->m_processedSamples_16s = 0;
	(*ppBinaural)->m_outputSamplesSize_16s = 0;
	(*ppBinaural)->m_memoryIndex_16s = 0;


	/* malloc m_ppDirectProcessSize_16s */
    (*ppBinaural)->m_ppDirectProcessSize_16s =
        (int **)malloc((*ppBinaural)->m_numberOfOutput_16s * sizeof(int *));

    if ((*ppBinaural)->m_ppDirectProcessSize_16s == NULL)
    {
        BinauralStruct_freeAlloc_32f(ppBinaural);
        return ERROR_INSUFFICIENT_MEM;
    }

    for (indM_16s = 0; indM_16s < (*ppBinaural)->m_numberOfOutput_16s; indM_16s++)
    {
        (*ppBinaural)->m_ppDirectProcessSize_16s[indM_16s] =
            (int *)malloc(numberOfInput_16s * sizeof(int));

        if ((*ppBinaural)->m_ppDirectProcessSize_16s[indM_16s] == NULL)
        {
            BinauralStruct_freeAlloc_32f(ppBinaural);
            return ERROR_INSUFFICIENT_MEM;
        }
    }

    /* malloc m_ppDiffuseProcessSize_16s */
    (*ppBinaural)->m_ppDiffuseProcessSize_16s =
        (int **)malloc((*ppBinaural)->m_nbDiffuseBlocks_16s * sizeof(int *));

    if ((*ppBinaural)->m_ppDiffuseProcessSize_16s == NULL)
    {
        BinauralStruct_freeAlloc_32f(ppBinaural);
        return ERROR_INSUFFICIENT_MEM;
    }

    for (indK_16s = 0; indK_16s < (*ppBinaural)->m_nbDiffuseBlocks_16s; indK_16s++)
    {
        (*ppBinaural)->m_ppDiffuseProcessSize_16s[indK_16s] =
            (int *)malloc(numberOfOutput_16s * sizeof(int));

        if ((*ppBinaural)->m_ppDiffuseProcessSize_16s[indK_16s] == NULL)
        {
            BinauralStruct_freeAlloc_32f(ppBinaural);
            return ERROR_INSUFFICIENT_MEM;
        }
    }
    
#ifdef FFT_VERSION2
	/* init fft handles */
	FFTlib_Create(&((*ppBinaural)->hFFT), (*ppBinaural)->m_FFTsize_16s, 1);
    FFTlib_Create(&((*ppBinaural)->hIFFT), (*ppBinaural)->m_FFTsize_16s, -1);
#endif

	return ERROR_OK;
}


/*----------------------------------------------------------------------------*/
void BinauralStruct_freeAlloc_32f (
	BinauralStruct_32f **ppBinaural)

	/* Destruction */
{
	int indN_16s, indM_16s, indK_16s;

	if ( ppBinaural == NULL )
	{
		return;
	}

	if ( *ppBinaural == NULL )
	{
		return;
	}

	if ((*ppBinaural)->m_pppTapsDirect_32f)
	{
		for ( indM_16s = 0 ;
			indM_16s < (*ppBinaural)->m_numberOfOutput_16s ;
			indM_16s ++ )
		{
			for ( indN_16s = 0 ;
				indN_16s < (*ppBinaural)->m_numberOfInput_16s ;
				indN_16s ++ )
			{
				free((*ppBinaural)->m_pppTapsDirect_32f[indM_16s][indN_16s]);
			}
			free((*ppBinaural)->m_pppTapsDirect_32f[indM_16s]);
		}
		free((*ppBinaural)->m_pppTapsDirect_32f);
	}

	if ((*ppBinaural)->m_ppInputMemoryBuffer_32f)
	{
		for ( indN_16s = 0 ;
			indN_16s < (*ppBinaural)->m_numberOfInput_16s ;
			indN_16s ++ )
		{
			free( (*ppBinaural)->m_ppInputMemoryBuffer_32f[indN_16s] );
		}
		free( (*ppBinaural)->m_ppInputMemoryBuffer_32f );
	}

	if ((*ppBinaural)->m_ppOutputMemoryBuffer_32f)
	{
		for ( indM_16s = 0 ;
			indM_16s < (*ppBinaural)->m_numberOfOutput_16s ;
			indM_16s ++ )
		{
			free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f[indM_16s] );
		}
		free( (*ppBinaural)->m_ppOutputMemoryBuffer_32f );
	}

	if ((*ppBinaural)->m_ppPrevInputMemoryBuffer_32f)
	{
		for ( indK_16s = 0 ;
			indK_16s < (*ppBinaural)->m_nbDiffuseBlocks_16s ;
			indK_16s ++ )
		{
			free( (*ppBinaural)->m_ppPrevInputMemoryBuffer_32f[indK_16s] );
		}
		free( (*ppBinaural)->m_ppPrevInputMemoryBuffer_32f );
	}

	if ((*ppBinaural)->m_pppTapsDiffuse_32f)
	{
		for ( indK_16s = 0 ;
			indK_16s < (*ppBinaural)->m_nbDiffuseBlocks_16s ;
			indK_16s ++ )
		{
			for ( indM_16s = 0 ;
				indM_16s < (*ppBinaural)->m_numberOfOutput_16s ;
				indM_16s ++ )
			{
				free((*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s][indM_16s]);
			}
			free((*ppBinaural)->m_pppTapsDiffuse_32f[indK_16s]);
		}
		free((*ppBinaural)->m_pppTapsDiffuse_32f);
	}

#ifdef FFT_VERSION2
	/* destroy fft handles */
	FFTlib_Destroy(&((*ppBinaural)->hFFT));
    FFTlib_Destroy(&((*ppBinaural)->hIFFT));
#endif

	free(*ppBinaural);
	*ppBinaural = NULL;
}

/*----------------------------------------------------------------------------*/
void BinauralStruct_set_32f (
	float					***pppTapsDirect_32f,
	int						TapsDirectLen_16s,
	float					***pppTapsDiffuse_32f,
	int						TapsDiffuseLen_16s,
	int						nbOfDiffuseBlocks_16s,
	float					*pDiffuseWeight_32f,
	float					**ppRatioFreqDirect_32f,
	float					**ppRatioFreqDiffuse_32f,
	BinauralStruct_32f		*pBinaural )

	/* Construction */
{
	int indN_16s, indM_16s, indK_16s, blockLen_16s;
	float *pXi_32f = NULL;

	if (pBinaural == NULL)
	{
		return;
	}

	pBinaural->m_nbDiffuseBlocks_16s = nbOfDiffuseBlocks_16s;

	blockLen_16s = pBinaural->m_FFTsize_16s / 2;

	pBinaural->m_DirectSize_16s = TapsDirectLen_16s;
	pBinaural->m_DiffuseSize_16s = TapsDiffuseLen_16s;

	pBinaural->m_filterSize_16s = blockLen_16s;

	pXi_32f = (float*)malloc(pBinaural->m_FFTsize_16s*sizeof(float));

	for ( indM_16s = 0; indM_16s < pBinaural->m_numberOfOutput_16s ; indM_16s++ )
	{
		for ( indN_16s = 0; indN_16s < pBinaural->m_numberOfInput_16s ; indN_16s++ )
		{

			Copy_32f(
				pppTapsDirect_32f[indM_16s][indN_16s],
				pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s],
				pBinaural->m_filterSize_16s );

			Set_32f(
				0,
				&pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s][pBinaural->m_filterSize_16s],
				pBinaural->m_FFTsize_16s - pBinaural->m_filterSize_16s );

			Set_32f(
				0,
				pXi_32f,
				pBinaural->m_FFTsize_16s);
#ifndef FFT_VERSION2
			UHD_fft(pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s], pXi_32f, pBinaural->m_FFTsize_16s);
#else
			FFTlib_Apply(pBinaural->hFFT, pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s], pXi_32f, pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s], pXi_32f);
#endif
			FFT_Complex_to_PermFmt_I((pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s]), pXi_32f, pBinaural->m_FFTsize_16s);

			if (pBinaural->m_ppInputMemoryBuffer_32f)
			{
				if (pBinaural->m_ppInputMemoryBuffer_32f[indN_16s])
				{
					Set_32f(
						0,
						pBinaural->m_ppInputMemoryBuffer_32f[indN_16s],
						pBinaural->m_FFTsize_16s );
				}
			}
		}

		if (pBinaural->m_ppOutputMemoryBuffer_32f)
		{
			Set_32f(
				0,
				pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
				pBinaural->m_FFTsize_16s );
		}
	}

	for ( indK_16s = 0; indK_16s < pBinaural->m_nbDiffuseBlocks_16s ; indK_16s++ )
	{
		for ( indM_16s = 0; indM_16s < pBinaural->m_numberOfOutput_16s ; indM_16s++ )
		{
			Copy_32f(
				pppTapsDiffuse_32f[indK_16s][indM_16s],
				pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s],
				TapsDiffuseLen_16s );

			Set_32f(
				0,
				&pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s][pBinaural->m_filterSize_16s],
				pBinaural->m_FFTsize_16s - pBinaural->m_filterSize_16s );

			Set_32f(0, pXi_32f, pBinaural->m_FFTsize_16s);
#ifndef FFT_VERSION2
			UHD_fft((pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s]), pXi_32f, pBinaural->m_FFTsize_16s);
#else
			FFTlib_Apply(pBinaural->hFFT, pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s], pXi_32f, pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s], pXi_32f);
#endif
			FFT_Complex_to_PermFmt_I((pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s]), pXi_32f, pBinaural->m_FFTsize_16s);

		}
	}

	for ( indN_16s = 0; indN_16s < pBinaural->m_numberOfInput_16s ; indN_16s++ )
	{
		pBinaural->m_pDiffuseWeight_32f[indN_16s] =  pDiffuseWeight_32f[indN_16s];
	}
	
	
	for (indM_16s = 0; indM_16s < pBinaural->m_numberOfOutput_16s; indM_16s++)
	{ 
		for (indN_16s = 0; indN_16s < pBinaural->m_numberOfInput_16s ; indN_16s++)
		{
			if (ppRatioFreqDirect_32f[indM_16s][indN_16s] > 1.0f)
				ppRatioFreqDirect_32f[indM_16s][indN_16s] = 1.0f;
			pBinaural->m_ppDirectProcessSize_16s[indM_16s][indN_16s] = (int)(ppRatioFreqDirect_32f[indM_16s][indN_16s] * pBinaural->m_filterSize_16s);
		}
	}


    for (indK_16s = 0; indK_16s < pBinaural->m_nbDiffuseBlocks_16s; indK_16s++)
    {
        for (indM_16s = 0; indM_16s < pBinaural->m_numberOfOutput_16s; indM_16s++)
        { 
            if (ppRatioFreqDiffuse_32f[indM_16s][indK_16s] > 1.0f)
                ppRatioFreqDiffuse_32f[indM_16s][indK_16s] = 1.0f;
            pBinaural->m_ppDiffuseProcessSize_16s[indK_16s][indM_16s] = (int)(ppRatioFreqDiffuse_32f[indM_16s][indK_16s] * pBinaural->m_filterSize_16s);
        }
    }

    pBinaural->m_SpecMulSize_16s = 2 * pBinaural->m_filterSize_16s;

    pBinaural->m_processedSamples_16s =
		pBinaural->m_FFTsize_16s -
		pBinaural->m_filterSize_16s;
	pBinaural->m_outputSamplesSize_16s = pBinaural->m_processedSamples_16s ;

	pBinaural->m_memoryIndex_16s = 0;

}


/*----------------------------------------------------------------------------*/
int BinauralStruct_processLowDelayOLA_32f (
	float						**ppSrc_32f,
	float						**ppDst_32f,
	int							samplesLen_32s,
	float						*pWork_32f,
	BinauralStruct_32f          *pBinaural )
{
	int srcDstIndex_16s;
	int len_16s=0, indN_16s, indM_16s, indK_16s, indR_16s;

	int IniSamplesLen_32s;
	float *pXi_32f = NULL;
	int numOutputSamples_32s = 0;


	if ( samplesLen_32s + pBinaural->m_memoryIndex_16s < pBinaural->m_processedSamples_16s )
	{
		for ( indN_16s = 0;
			indN_16s < pBinaural->m_numberOfInput_16s;
			indN_16s++ )
		{
			Copy_32f(
				ppSrc_32f[indN_16s],
				&pBinaural->
				m_ppInputMemoryBuffer_32f[indN_16s][
					pBinaural->m_memoryIndex_16s],
						samplesLen_32s );
		}

		pBinaural->m_memoryIndex_16s = 
			pBinaural->m_memoryIndex_16s + (int)samplesLen_32s;

		return numOutputSamples_32s;
	}
	else
	{
		IniSamplesLen_32s = samplesLen_32s + pBinaural->m_memoryIndex_16s;
		srcDstIndex_16s = 0;

		pXi_32f = (float*)malloc(pBinaural->m_FFTsize_16s*sizeof(float));

		while( IniSamplesLen_32s >= pBinaural->m_processedSamples_16s )
		{
			len_16s = (int)( IniSamplesLen_32s > pBinaural->m_processedSamples_16s ?
				pBinaural->m_processedSamples_16s : IniSamplesLen_32s );

			len_16s = len_16s - pBinaural->m_memoryIndex_16s;

			for (	indN_16s = 0;
				indN_16s < pBinaural->m_numberOfInput_16s;
				indN_16s++ )
			{
				Copy_32f(
					&ppSrc_32f[indN_16s][srcDstIndex_16s],
					&pBinaural->m_ppInputMemoryBuffer_32f[indN_16s][
					pBinaural->m_memoryIndex_16s],
					len_16s );
			}
				
			Set_32f(
				0,
				&pBinaural->
				m_ppInputMemoryBuffer_32f[0][
					pBinaural->m_processedSamples_16s],
						pBinaural->m_filterSize_16s );

			Set_32f(
				0,
				&pBinaural->m_ppPrevInputMemoryBuffer_32f[pBinaural->m_indexWriteInputBuffer_16s][
					pBinaural->m_processedSamples_16s],
						pBinaural->m_filterSize_16s );
		
			Set_32f(0, pXi_32f, pBinaural->m_FFTsize_16s);
#ifndef FFT_VERSION2
			UHD_fft((pBinaural->m_ppInputMemoryBuffer_32f[0]), pXi_32f, pBinaural->m_FFTsize_16s);
#else
			FFTlib_Apply(pBinaural->hFFT, pBinaural->m_ppInputMemoryBuffer_32f[0], pXi_32f, pBinaural->m_ppInputMemoryBuffer_32f[0], pXi_32f);
#endif
			FFT_Complex_to_PermFmt_I((pBinaural->m_ppInputMemoryBuffer_32f[0]), pXi_32f, pBinaural->m_FFTsize_16s);

			MulC_32f(
					pBinaural->m_ppInputMemoryBuffer_32f[0],
					pBinaural->m_pDiffuseWeight_32f[0],
					pBinaural->m_ppPrevInputMemoryBuffer_32f[pBinaural->m_indexWriteInputBuffer_16s],
					pBinaural->m_SpecMulSize_16s);
					
			for ( indN_16s = 1;
				indN_16s < pBinaural->m_numberOfInput_16s;
				indN_16s++ )
			{
				Set_32f(
					0,
					&pBinaural->
					m_ppInputMemoryBuffer_32f[indN_16s][
					pBinaural->m_processedSamples_16s],
					pBinaural->m_filterSize_16s );
			
				Set_32f(0, pXi_32f, pBinaural->m_FFTsize_16s);
#ifndef FFT_VERSION2
				UHD_fft((pBinaural->m_ppInputMemoryBuffer_32f[indN_16s]), pXi_32f, pBinaural->m_FFTsize_16s);
#else
				FFTlib_Apply(pBinaural->hFFT, pBinaural->m_ppInputMemoryBuffer_32f[indN_16s], pXi_32f, pBinaural->m_ppInputMemoryBuffer_32f[indN_16s], pXi_32f);
#endif
				FFT_Complex_to_PermFmt_I((pBinaural->m_ppInputMemoryBuffer_32f[indN_16s]), pXi_32f, pBinaural->m_FFTsize_16s);

				MulCAdd_32f(
					pBinaural->m_ppInputMemoryBuffer_32f[indN_16s],
					pBinaural->m_pDiffuseWeight_32f[indN_16s],
					pBinaural->m_ppPrevInputMemoryBuffer_32f[pBinaural->m_indexWriteInputBuffer_16s],
					pBinaural->m_SpecMulSize_16s);
			}

			for (	indM_16s = 0;
				indM_16s < pBinaural->m_numberOfOutput_16s;
				indM_16s++ )
			{

				Copy_32f(&pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s][
					pBinaural->m_processedSamples_16s],
						pWork_32f,
						pBinaural->m_filterSize_16s );

					Set_32f(
						0,
						pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
						pBinaural->m_FFTsize_16s);
					

					for (	indN_16s = 0;
							indN_16s < pBinaural->m_numberOfInput_16s;
							indN_16s++ )
					{
						MulAddPerm_32fc(
							pBinaural->m_pppTapsDirect_32f[indM_16s][indN_16s],
							pBinaural->m_ppInputMemoryBuffer_32f[indN_16s],
							pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
							pBinaural->m_ppDirectProcessSize_16s[indM_16s][indN_16s]);
					}
					
					indR_16s = (pBinaural->m_indexWriteInputBuffer_16s+pBinaural->m_nbDiffuseBlocks_16s)%(pBinaural->m_nbDiffuseBlocks_16s+1);

					for (	indK_16s = 0;
							indK_16s < pBinaural->m_nbDiffuseBlocks_16s;
							indK_16s++ )
					{
						
						MulAddPerm_32fc(
							pBinaural->m_pppTapsDiffuse_32f[indK_16s][indM_16s],
							pBinaural->m_ppPrevInputMemoryBuffer_32f[indR_16s],
							pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
                            pBinaural->m_ppDiffuseProcessSize_16s[indK_16s][indM_16s]);

						indR_16s = (indR_16s+pBinaural->m_nbDiffuseBlocks_16s) % (pBinaural->m_nbDiffuseBlocks_16s+1);
					}

					FFT_PermFmt_to_Complex_I((pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s]), pXi_32f, pBinaural->m_FFTsize_16s);
#ifndef FFT_VERSION2
					UHD_ifft((pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s]), pXi_32f, pBinaural->m_FFTsize_16s);
#else
					FFTlib_Apply(pBinaural->hIFFT, pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s], pXi_32f, pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s], pXi_32f);
					MulC_32f_I(
						1.0f/pBinaural->m_FFTsize_16s, 
						pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
						pBinaural->m_FFTsize_16s );
#endif
					Add_32f_I(
						pWork_32f,
						pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
						pBinaural->m_filterSize_16s );
			}

			pBinaural->m_indexWriteInputBuffer_16s = (pBinaural->m_indexWriteInputBuffer_16s+1)%(pBinaural->m_nbDiffuseBlocks_16s+1);	
			

			for (	indM_16s = 0;
				indM_16s < pBinaural->m_numberOfOutput_16s;
				indM_16s++ )
			{
				Copy_32f(
					pBinaural->m_ppOutputMemoryBuffer_32f[indM_16s],
					&ppDst_32f[indM_16s][numOutputSamples_32s],
					pBinaural->m_processedSamples_16s );
			}

			numOutputSamples_32s  += pBinaural->m_processedSamples_16s;
			IniSamplesLen_32s = IniSamplesLen_32s - pBinaural->m_processedSamples_16s;
			samplesLen_32s = samplesLen_32s - len_16s;
			srcDstIndex_16s = srcDstIndex_16s + len_16s;
			pBinaural->m_memoryIndex_16s = 0;
		}


		for ( indN_16s = 0;
			indN_16s < pBinaural->m_numberOfInput_16s;
			indN_16s++ )
		{
			Copy_32f(
				&ppSrc_32f[indN_16s][srcDstIndex_16s],
				pBinaural->m_ppInputMemoryBuffer_32f[indN_16s],
						samplesLen_32s );
		}

		pBinaural->m_memoryIndex_16s =  (int)samplesLen_32s;

		free(pXi_32f);

		return numOutputSamples_32s;
	}
}
