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
#ifndef BINAURALIZATION_32F_H
#define BINAURALIZATION_32F_H
/*----------------------------------------------------------------------------*/

#define BINAURAL_FFT_MAXLEN			16384 
#define BINAURAL_DIR_MAXLEN			8192 
#define BINAURAL_DIF_MAXLEN			BINAURAL_DIR_MAXLEN
#define BINAURAL_DIF_MAXBLOCKS		10

#define BINAURAL_NB_OUTPUT			2

#define ERROR_OK                    0
#define ERROR_BAD_PARAM            -1
#define ERROR_INSUFFICIENT_MEM     -2

#define FFT_VERSION2
#ifdef FFT_VERSION2
#include "fftlib.h" 
#else
#include "uhd_fft.h" 
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/**
	*    BinauralStruct_32f struct
	*   to BinauralStruct_32f parameters
	*/
	typedef struct
	{
#ifdef FFT_VERSION2
		/** handle to FFT */
		HANDLE_FFTLIB hFFT;
		/** handle to IFFT */
		HANDLE_FFTLIB hIFFT;
#endif

		/** Size of FFT, defined by filter length */
		int	m_FFTsize_16s;

		/** Direct filter size  */
		int m_DirectSize_16s;

		/** Diffuse filter size  */
		int m_DiffuseSize_16s;

		/** Samples processed for each direct block
			(= m_filterSize_16s if full band processing) */
		int **m_ppDirectProcessSize_16s;

		/** Samples processed for each diffuse block
			(= m_filterSize_16s if full band processing) */
		int **m_ppDiffuseProcessSize_16s;

		int m_SpecMulSize_16s;

		/**  Filter taps in the frequency domain. Buffer size is of size twice
		the size of the filter in the time domain  */
		float ***m_pppTapsDirect_32f;

		/** Memory for input data, same size of the filter size */
		float **m_ppInputMemoryBuffer_32f;

		/** Memory for Output data, same size of the filter size */
		float **m_ppOutputMemoryBuffer_32f;

		/** Processing filter size */
		int m_filterSize_16s;

		/** Number of Input = N */
		int m_numberOfInput_16s;

		/** Number of Output = M */
		int m_numberOfOutput_16s;

		/** Number of processed output still in memory */
		int m_outputSamplesSize_16s;

		/** Memory Index of processed samples still in memory */
		int m_memoryIndex_16s;

		/** Size of sample generate processed by one FFT Multiplication iFFT cycle.
		* Size = FFTSIZE - TapsLen + 1*/
		int m_processedSamples_16s;

		/** Number of Previous Input Buffer = K */
		int m_nbDiffuseBlocks_16s;   

		/** Index for diffuse filters computering */
		int m_indexWriteInputBuffer_16s;

		/** Memory for previous input data, size is defined by the upper variable */
		float **m_ppPrevInputMemoryBuffer_32f;

		/** Memory for previous input data, size is defined by the upper variable */
		float *m_pDiffuseWeight_32f;

		/**  Filter taps in the frequency domain. Buffer size is of size twice
		the size of the filter in the time domain  */
		float ***m_pppTapsDiffuse_32f;

	}
	BinauralStruct_32f;

	/*----------------------------------------------------------------------------*/
	/** Usefull functions for code comprehension */
	void floatMemAlloc (float ***ptr_32f, int size1_32s, int size2_32s);

	int Set_32f (float val_32f, float *pDst_32f, int len_32s);

	int Copy_32f (const float *pSrc_32f, float *pDst_32f, int len_32s);

	int Add_32f(const float	*pSrc1_32f, const float *pSrc2_32f, float *pDst_32f, int len_32s );

	int Add_32f_I(const float *pSrc_32f, float *pSrcDst_32f, int len_32s );

	int  MulC_32f( const float *pSrc_32f, float val_32f, float *pDst_32f, int len_32s );

	int MulC_32f_I( float val_32f, float *pSrcDst_32f, int len_32s );

	int MulCAdd_32f( const float	*pSrc_32f, float val_32f, float	*pDst_32f, int	len_32s );

	int MulPerm_32f( const float	 *pSrc1_32fc, const float *pSrc2_32fc, float*pDst_32fc, int	len_32s );

	int MulAddPerm_32fc( const float *pSrc1_32fc, const float *pSrc2_32fc, float *pDst_32fc, int len_32s );

	void FFT_Complex_to_PermFmt ( float *pXr_32f, float	*pXi_32f, float	*pPerm_32f, int	FFTsize_16s );

	void FFT_Complex_to_PermFmt_I ( float *pXr_32f, float *pXi_32f, int	FFTsize_16s );

	void FFT_PermFmt_to_Complex ( float	*pPerm_32f, float *pXr_32f, float *pXi_32f, int FFTsize_16s );

	void FFT_PermFmt_to_Complex_I ( float *pXr_32f, float *pXi_32f, int	FFTsize_16s );

	int maximumInteger(int i1, int i2);
	/*----------------------------------------------------------------------------*/


	/**
	*   Building and allocate of struct BinauralStruct_32f
	*   @param    fftSize         :  FFT size 
	*   @param    DirectLen_16s   :  direct filter size 
	*   @param    DiffuseLen_16s  :  diffuse filter size 
	*   @param    nbDiffuseBlocks_16s   :  number of diffuse blocks 
	*   @param    numberOfInput_16s		: number of Input signal to be filtered
	*   @param    numberOfOutput_16s	: number of Output signal
	*   @param    ppBinaural      : pointer to BinauralStruct_32f
	*
	*   @return   ERROR_CODE
	*/
	int BinauralStruct_initAlloc_32f (
		int								fftSize,
		int								DirectLen_16s,
		int								DiffuseLen_16s,
		int								nbDiffuseBlocks_16s,
		int								numberOfInput_16s,
		int								numberOfOutput_16s,
		BinauralStruct_32f				**ppBinaural );


	/**
	*   Free of struct BinauralStruct_32f allocated with initAlloc
	*
	*   @param    ppBinaural  :  pointer to BinauralStruct_32f
	*/
	void BinauralStruct_freeAlloc_32f( BinauralStruct_32f **ppBinaural );


	/**
	*   Set the filters taps for NumberOfFilters.
	*   @param    pppTapsDirect_32f     : direct filter taps 
	*   @param    TapsDirectLen_16s     : direct filter size 
	*   @param    pppTapsDiffuse_32f    : diffuse filter taps 
	*   @param    TapsDiffuseLen_16s    : diffuse filter size 
	*   @param    numberOfFilters_16s	: number of filters = number of Input signal
	to be filtered
	*   @param    ppTaps_32f            : filter taps in the time domain                           
	*   @param    pBinaural             : pointer to BinauralStruct_32f
	*/
	void  BinauralStruct_set_32f (
		float								***pppTapsDirect_32f,
		int									TapsDirectLen_16s,
		float								***pppTapsDiffuse_32f,
		int									TapsDiffuseLen_16s,
		int									nbOfDiffuseBlocks_16s,
		float								*pDiffuseWeight_32f,
		float								**ppRatioFreqDirect_32f,
		float								**ppRatioFreqDiffuse_32f,
		BinauralStruct_32f					*pBinaural );


	/**
	*   Process the binauralization. The filtering is done in the FFT domain using
	*   Overlap and Add method.
	*
	*   @param    ppSrc_32f      :   pointer to real input data
	*   @param    ppDst_32f      :   pointer to real output data
	*   @param    samplesLen_32s :   length of the sample to filter
	*   @param    pWork_32f      :   work buffer of size max filter length - 1
	*   @param    pBinaural      :   pointer to BinauralStruct_32f
	*
	*   @return   number of processed samples
	*/
	int BinauralStruct_processLowDelayOLA_32f(   
		float								**ppSrc_32f,
		float								**ppDst_32f,
		int									samplesLen_32s,
		float                               *pWork_32f,
		BinauralStruct_32f					*pBinaural );

	/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif /* BINAURALIZATION_32F_H */
