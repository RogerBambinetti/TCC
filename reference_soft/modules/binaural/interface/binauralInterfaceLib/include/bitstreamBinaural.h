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


/*----------------------------------------------------------------------------*/
#ifndef INTERFACE_TD_H
#define INTERFACE_TD_H

/*----------------------------------------------------------------------------*/


#include "speakerConfig3d.h"
#include "readonlybitbuf.h"
#include "writeonlybitbuf.h"


#ifdef __cplusplus
extern "C" {
#endif

#define BITSTREAM_MAX_NUM_MEGA_BYTES    100		/* max number of MB supported when writing the bitstream */

#define MAX_BRIR_SIZE					262144	/* max BRIR size in samples (before parametrization) */

#define MAX_BINAURAL_REPRESENTATION		16		/* max number of BinauralRepresentations stored in a BinauralRendering struct */

#define MAX_NUM_BRIR_PAIRS		        100		/* max number of BRIRs stored in a BinauralRepresentation struct */
	/* WARNING : keep numBrirsPairs <= CICP2GEOMETRY_MAX_LOUDSPEAKERS except for HOA */

#define MAX_LENGTH_DIRECT_FILTER		8192	/* max direct filter size */
#define MAX_NUM_DIFFUSE_BLOCKS			10		/* max number of diffuse blocks */

#define NUM_QMFBANDS					64		/* max number of QMF bands */
#define MAX_LENGTH_BRIR_QMF				4098	/* max QMF domain BRIR length in samples - MAX_BRIR_SIZE/64+2 */
#define NUM_MAX_VOFF_BLK				100		/* max number of VOFF blocks */

	/*--------------------------  BinauralFirData  ------------------------------*/

	typedef struct BinauralFirData
	{
		int				Ntaps;			/* filter size */
		float			pppTaps[2][MAX_NUM_BRIR_PAIRS][MAX_BRIR_SIZE];		/* ptr to filter taps, access one tap by pppTaps[ear][channel][tap] */
		unsigned long   allCutFreq;
		float			ppCutFreq[2][MAX_NUM_BRIR_PAIRS];    /* ptr to filter cutoff frequencies, access one fc by ppCutFreq[ear][channel] */
	}
	BinauralFirData;

	/**
	*   Read BinauralFirData from bitstream 
	*   @param    pBinauralFirData	: destination BinauralFirData pointer (previously initialized)
	*   @param    nBrirPairs		: number of BRIR pairs
	*   @param    hBitstream     	: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int readBinauralFirData(
		BinauralFirData *pBinauralFirData, 
		short nBrirPairs,
		robitbufHandle hBitstream);

int writeBinauralFirData(const BinauralFirData *pBinauralFirData, 
						 short nBrirPairs,
						 wobitbufHandle hBitstream);

	/*-------------------------  FdBinauralRendererParam  --------------------------------*/

	typedef struct FdBinauralRendererParam 
	{
		unsigned int flagHRIR;				/* Indicates whether inputted impulse response is HRIR = 1 or BRIR = 0 */
		unsigned int dInit;
		unsigned int kMax;
		unsigned int kConv;
		unsigned int kAna;

		
		/* VOFF param. */
		unsigned int *nFilter;
		unsigned int *nFft;
		unsigned int *nBlk;
		float ****ppppVoffCoeffLeftReal; /*[NUM_QMFBANDS][NUM_MAX_VOFF_BLK][MAX_NUM_BRIR_PAIRS][MAX_LENGTH_BRIR_QMF];*/
		float ****ppppVoffCoeffLeftImag; /*[NUM_QMFBANDS][NUM_MAX_VOFF_BLK][MAX_NUM_BRIR_PAIRS][MAX_LENGTH_BRIR_QMF];*/
		float ****ppppVoffCoeffRightReal;/*[NUM_QMFBANDS][NUM_MAX_VOFF_BLK][MAX_NUM_BRIR_PAIRS][MAX_LENGTH_BRIR_QMF];*/
		float ****ppppVoffCoeffRightImag;/*[NUM_QMFBANDS][NUM_MAX_VOFF_BLK][MAX_NUM_BRIR_PAIRS][MAX_LENGTH_BRIR_QMF];*/
		
		/* SFR param. */
		float *fcAna;
		float *rt60;
		float *nrgLr;

		/* QTDL param. */
		float **ppQtdlGainLeftReal;
		float **ppQtdlGainLeftImag;
		float **ppQtdlGainRightReal;
		float **ppQtdlGainRightImag;
		unsigned int **ppQtdlLagLeft;
		unsigned int **ppQtdlLagRight;

	} FdBinauralRendererParam;


  int readFdBinauralRendererParam(
	FdBinauralRendererParam *pFdBinauralRendererParam, 
	short nBrirPairs,
	robitbufHandle hBitstream);

int writeFdBinauralRendererParam(
	FdBinauralRendererParam *pFdBinauralRendererParam, 
	short nBrirPairs,
	wobitbufHandle hBitstream);

	/*-------------------------  TdBinauralRendererParam  --------------------------------*/

	typedef struct TdBinauralRendererParam
	{
		int numChannel;				/* number of channels */
		int beginDelay;				/* indice of first sample for direct filters (0 being the first sample of original BRIR) */
		int lenDirect;				/* direct filter length (has to be a power of 2) */
		int numDiffuseBlock;        /* number of diffuse blocks */
		float ppDirectFc[2][MAX_NUM_BRIR_PAIRS];		/* direct Fc array, access one Fc by ppDirectFc[ear][channel] */
		float ppDiffuseFc[2][MAX_NUM_DIFFUSE_BLOCKS];		/* diffuse Fc array, access one Fc by ppDiffuseFc[ear][block] */
		float pInvDiffuseWeight[MAX_NUM_BRIR_PAIRS];   /* inverse diffuse weights array, access one inverse weigth by pInvDiffuseWeight[channel] */
		float pppTaps_direct[2][MAX_NUM_BRIR_PAIRS][MAX_LENGTH_DIRECT_FILTER];  /* direct filter taps, access one tap by pppTaps[ear][channel][tap] */
		float pppTaps_diffuse[MAX_NUM_DIFFUSE_BLOCKS][2][MAX_LENGTH_DIRECT_FILTER];   /* diffuse filter taps, access one tap by pppTaps[block][ear][tap] */
	} TdBinauralRendererParam;


	/**
	*   Read TdBinauralRendererParam from bitstream 
	*   @param    pTdBinauralRendererParam	: destination TdBinauralRendererParam pointer (previously initialized)
	*   @param    nBrirPairs				: number of BRIR pairs
	*   @param    hBitstream     			: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int readTdBinauralRendererParam(
		TdBinauralRendererParam *pTdBinauralRendererParam,
		short nBrirPairs,
		robitbufHandle hBitstream);

int writeTdBinauralRendererParam(
	TdBinauralRendererParam *pTdBinauralRendererParam, 
	short nBrirPairs,
	wobitbufHandle hBitstream);


	/*-------------------------------- BinauralRepresentation --------------------------------------------*/

	typedef struct BinauralRepresentation
	{
		unsigned long   brirSamplingFrequency;
		unsigned short  isHoaData;
		short			hoaOrderBinaural;							/* case HOA */
		short           nBrirPairs;
		Setup_SpeakerConfig3d Setup_SpeakerConfig3d;							/* case VL  */
		short           binauralDataFormatID;						/* 0 : raw FIR     1 : FD parameters     2 : TD parameters*/

		/* only one of the following 3 pointers may be allocated, depending on binauralDataFormatID */
		BinauralFirData         *pBinauralFirData;
		FdBinauralRendererParam *pFdBinauralRendererParam; 
		TdBinauralRendererParam *pTdBinauralRendererParam;

	}
	BinauralRepresentation;

	/**
	*   Build and init BinauralRepresentation
	*   @param    none
	*
	*   @return   pointer to BinauralRepresentation struct
	*/
	BinauralRepresentation * BinauralRepresentation_initAlloc();

	/**
	*   Set BinauralRepresentation FIR for CICP or GEO configurations
	*   @param  pBinauralRepresentation	: pointer to BinauralRepresentation struct
	*   @param	brirSamplingFrequency   : sampling frequency
	*   @param	nBrirPairs				: number of BRIR pairs
	*	@param  pSpeakerConfig3d	    : pointer to existing (and allocated) Setup_SpeakerConfig3d struct
	*   @param  pBinauralFirData		: pointer to existing (and allocated) BinauralFirData struct
	*
	*   @return   ERROR_CODE
	*/
	int BinauralRepresentation_setFIR_CICP_GEO(
		BinauralRepresentation *pBinauralRepresentation,
		unsigned long brirSamplingFrequency,
		short nBrirPairs,
		Setup_SpeakerConfig3d *pSpeakerConfig3d,
		BinauralFirData *pBinauralFirData);

	/**
	*   Set BinauralRepresentation FIR HOA
	*   @param  pBinauralRepresentation	: pointer to BinauralRepresentation struct
	*   @param	brirSamplingFrequency   : sampling frequency
	*   @param	hoaOrder				: HOA order
	*   @param  pBinauralFirData		: pointer to existing (and allocated) BinauralFirData struct
	*
	*   @return   ERROR_CODE
	*/
	int BinauralRepresentation_setFIR_HOA(
		BinauralRepresentation *pBinauralRepresentation,
		unsigned long   brirSamplingFrequency,
		short   hoaOrder,
		BinauralFirData *pBinauralFirData);

	/**
	*   Set BinauralRepresentation TdBinauralRendererParam
	*   @param  pBinauralRepresentation		: pointer to BinauralRepresentation struct
	*   @param  pTdBinauralRendererParam	: pointer to existing (and allocated) TdBinauralRendererParam struct
	*
	*   @return   ERROR_CODE
	*/
	int BinauralRepresentation_setTdBinauralRendererParam(
		BinauralRepresentation *pBinauralRepresentation,
		TdBinauralRendererParam *pTdBinauralRendererParam);



	/*-----------------------------  BinauralRendering   -----------------------------------------------*/

	typedef struct BinauralRendering
	{
		unsigned long   fileSignature; 
		unsigned short  fileVersion;
		char            name[257]; /* string with 256 char + \0*/
		unsigned short  useTrackingMode;
		unsigned short  numBinauralRepresentation;
		BinauralRepresentation *ppBinauralRepresentation[MAX_BINAURAL_REPRESENTATION];
	}
	BinauralRendering;

	/**
	*   Build and init BinauralRendering
	*   @param    none
	*
	*   @return   pointer to BinauralRendering struct
	*/
	BinauralRendering * BinauralRendering_initAlloc();

	/**
	*   Add existing representation to BinauralRendering
	*   @param    pBinauralRendering			: pointer to BinauralRendering struct
	*   @param    pBinauralRepresentation		: pointer to existing BinauralRepresentation struct
	*   @param	  idx							: index of the new representation in BinauralRendering
	*
	*   @return   ERROR_CODE
	*/
	int BinauralRendering_setBinauralRepresentation(
		BinauralRendering *pBinauralRendering, 
		BinauralRepresentation *pBinauralRepresentation,
		short idx);

	/**
	*   Load BRIRs wavefiles and add a FIR BinauralReprensentation into BinauralRendering.
	*	BRIR configuration is described by a CICP index OR by a geometric file.
	*   Filters must be stored in appropriate wavfiles.
	*
	*   @param    pBinauralRendering	: ptr to a BinauralRendering struct (previously initialized)
	*   @param    BRIRpath				: Path to BRIR wavfiles (up to prefix) (ex : "C:/Users/you/BRIR/Mid/IIS_BRIR_")
	*   @param    cicpIndex				: index of cicp configuration
	*   @param    geoFileName			: name of the geometric file (only used if cicpIndex = -1)
	*
	*   @return   0 if no error, 1 otherwise
	*/
	int BinauralRendering_addRepresentationFromWav_CICP_GEO(
		BinauralRendering *pBinauralRendering, 
		const char* BRIRpath, 
		int cicpIndex,		
	  char* geoFileName,
    int useTrackingMode);


	/**
	*   Load HOA BRIRs wavefiles and add a FIR BinauralReprensentation into BinauralRendering.
	*	BRIR configuration is described by the HOA order.
	*   Filters must be stored in appropriate wavfiles.
	*
	*   @param    pBinauralRendering	: ptr to a BinauralRendering struct (previously initialized)
	*   @param    BRIRpath				: Path to BRIR wavfiles (up to prefix) (ex : "C:/Users/you/BRIR/Mid/IIS_BRIR_")
	*   @param    hoaOrder				: HOA order
	*
	*   @return   0 if no error, 1 otherwise
	*/
	int BinauralRendering_addRepresentationFromWav_HOA(
		BinauralRendering *pBinauralRendering, 
		const char* BRIRpath, 
		int hoaOrder,
    int useTrackingMode);


	/*-------------------  Read/Write binaural bitstream to/from BinauralRendering  -----------------------*/

	/**
	*   Read binaural bitstream to BinauralRendering
	*   @param    filename				: name of the binaural bitstream
	*   @param    pBinauralRendering	: pointer to destination BinauralRendering struct (previoulsy initialized)
	*
	*   @return   ERROR_CODE
	*/
	int bitstreamRead(const char* filename, BinauralRendering* pBinauralRendering);

	/**
	*   Write binaural bitstream from BinauralRendering  
	*   @param    filename				: name of the binaural bitstream
	*   @param    pBinauralRendering	: pointer to source BinauralRendering struct (previoulsy initialized and filled)
	*
	*   @return   ERROR_CODE
	*/
	int bitstreamWrite(const char* filename, unsigned long fileSignature, unsigned short  fileVersion, BinauralRendering* pBinauralRendering);


#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_TD_H */