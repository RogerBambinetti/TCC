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

#include "saocdeclib.h"
#include "saoc_getinfo.h"
#include "error.h"

static float pQmfReal[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_NUM_QMF_BANDS];
static float pQmfImag[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_NUM_QMF_BANDS];
static float pHybReal[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS];
static float pHybImag[SAOC_MAX_CHANNELS][SAOC_MAX_TIME_SLOTS][SAOC_MAX_HYBRID_BANDS];

static float pRenderMatFreq[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAMETER_BANDS] = {{{0}}};
static float pRenderMatTmp[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};

static float tmp_pDmxMat_preMix[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};
static float pDmxMatTmp[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] = {{0}};

int DecorrSeed[22] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1};

int saoc_GetNumber_of_OAM_Objects( char* inbsFilename,
																	int *numSaocDmxChannels,
																	int *numSaocDmxObjects,
																	int sacHeaderLen)
{  
	int numObjects, errorMessage, numPremixedChannels = 0;

	HANDLE_FILE_READER fileReader = NULL;
	HANDLE_BYTE_READER byteReader = NULL;
	BSR_INSTANCE*      pBsrInstance;
	HANDLE_S_BITINPUT  bitstream;

	fileReader = FileReaderOpen(inbsFilename);
	byteReader = FileReaderGetByteReader(fileReader);

	bitstream = s_bitinput_open(byteReader);
	CreateSAOCDecoder(&pBsrInstance);

	errorMessage = ReadSAOCSpecificConfig(bitstream,
																				pBsrInstance, 
																				sacHeaderLen,
																				numPremixedChannels);

	if (errorMessage != ERROR_NONE)
	{
		DestroySAOCDecoder(&pBsrInstance);
		s_bitinput_close(bitstream);
		return -1;
	}

	numObjects = pBsrInstance->saocSpecificConfig.bsNumSaocObjects;

	*numSaocDmxChannels = pBsrInstance->saocSpecificConfig.bsNumSaocDmxChannels;
	*numSaocDmxObjects = pBsrInstance->saocSpecificConfig.bsNumSaocDmxObjects;

	DestroySAOCDecoder(&pBsrInstance);
	s_bitinput_close(bitstream);

	return numObjects;
}

int saoc_GetSaocInputLayout(char* inbsFilename,
														int sacHeaderLen,
														int *inSaocCICPIndex,
														char inSaocGeoFilename[256],
														int *saocChannlPathFlag)
{  
	int errorMessage = ERROR_NONE;
	int numPremixedChannels = 0;

	HANDLE_FILE_READER fileReader = NULL;
	HANDLE_BYTE_READER byteReader = NULL;
	BSR_INSTANCE*     pBsrInstance;
	HANDLE_S_BITINPUT bitstream;

	fileReader = FileReaderOpen(inbsFilename);
	byteReader = FileReaderGetByteReader(fileReader);

	bitstream = s_bitinput_open(byteReader);
	CreateSAOCDecoder(&pBsrInstance);

	errorMessage = ReadSAOCSpecificConfig( bitstream,
		pBsrInstance,
		sacHeaderLen,
		numPremixedChannels);
	if (errorMessage != ERROR_NONE)
	{
		DestroySAOCDecoder(&pBsrInstance);
		s_bitinput_close(bitstream);
		return errorMessage;
	}

	if (pBsrInstance->saocSpecificConfig.bsNumSaocDmxChannels == 0)
	{
		*saocChannlPathFlag = 0;
		*inSaocCICPIndex    = -2; /*CICP_INVALID */
	} 
	else
	{
		*saocChannlPathFlag = 1;
		*inSaocCICPIndex = pBsrInstance->saocSpecificConfig.inSaocCICPIndex;
		if (*inSaocCICPIndex == -1)
		{
			sprintf(inSaocGeoFilename, "..\\..\\..\\..\\formatConverter\\formatConverterCmdl\\inputData\\inSaocGeoFilename.txt");
			cicp2geometry_write_geometry_to_file(inSaocGeoFilename,
				pBsrInstance->saocSpecificConfig.saocInputGeometryInfo,           
				pBsrInstance->saocSpecificConfig.bsNumSaocChannels,           
				pBsrInstance->saocSpecificConfig.bsNumSaocLFEs);
		}
	}

	DestroySAOCDecoder(&pBsrInstance);
	s_bitinput_close(bitstream);

	return errorMessage;
}



tSaocDec *saoc_DecoderOpen( int isInQmf,
													 int isOutQmf,
													 int nChannelsOut,
													 int numOutLFEs,
													 QMFLIB_HYBRID_FILTER_MODE hybridMode,
													 int hybridHFAlign,
													 unsigned int *nIn_WAV_IO_Channels,
													 unsigned int *nOut_WAV_IO_Channels,
													 char* inbsFilename,
													 int sacHeaderLen,
													 int outCICPIndex,
													 CICP2GEOMETRY_CHANNEL_GEOMETRY* saocOutputGeometryInfo,
													 int numPremixChannels,
													 int coreOutputFrameLength,
													 int *error_init)
{
	unsigned int i;
	int ch;
	HANDLE_FILE_READER fileReader = NULL;
	HANDLE_BYTE_READER byteReader = NULL;

	tSaocDec *dec = (tSaocDec*)calloc(1, sizeof(tSaocDec));
	*error_init  = 0;

	fileReader = FileReaderOpen(inbsFilename);
	byteReader = FileReaderGetByteReader(fileReader);

	dec->bitstreamIn = s_bitinput_open(byteReader);
	CreateSAOCDecoder(&dec->pBsrInstance);

	ReadSAOCSpecificConfig(dec->bitstreamIn,
		dec->pBsrInstance, 
		sacHeaderLen,
		numPremixChannels);

	PrintSAOCSpecificConfig(dec->bitstreamIn, dec->pBsrInstance);

	dec->pBsrInstance->saocSpecificConfig.bsFrameLength = (dec->pBsrInstance->saocSpecificConfig.bsDoubleFrameLengthFlag+1) * coreOutputFrameLength - 1;

	dec->LdMode             = dec->pBsrInstance->saocSpecificConfig.bsLowDelayMode;
	dec->nTimeSlots         = dec->pBsrInstance->saocSpecificConfig.bsFrameLength + 1;
	dec->sampleFreq         = dec->pBsrInstance->saocSpecificConfig.bsSamplingFrequency;

	dec->DecorrelationLevel = 0;
	dec->DecorrMethod       = dec->pBsrInstance->saocSpecificConfig.bsDecorrelationMethod;

	dec->nSaocChannels      = dec->pBsrInstance->saocSpecificConfig.bsNumSaocChannels;
	dec->nSaocDmxChannels   = dec->pBsrInstance->saocSpecificConfig.bsNumSaocDmxChannels;
	dec->nSaocObjects       = dec->pBsrInstance->saocSpecificConfig.bsNumSaocObjects;
	dec->nSaocDmxObjects    = dec->pBsrInstance->saocSpecificConfig.bsNumSaocDmxObjects;
	dec->nNumPreMixedChannels = dec->pBsrInstance->saocSpecificConfig.bsNumPreMixedChannels;
	dec->numInLFEs          = dec->pBsrInstance->saocSpecificConfig.bsNumSaocLFEs;
	dec->nChannelsIn        = dec->nSaocDmxChannels + dec->nSaocDmxObjects;

	dec->dualMode           = dec->pBsrInstance->saocSpecificConfig.bsDualMode;
	dec->numParamBands      = dec->pBsrInstance->saocSpecificConfig.bsNumBins;

	for (ch = 0; ch < dec->nSaocChannels + dec->nSaocObjects; ch++)
	{ 
		for (i = 0; i < (unsigned int)(dec->nSaocChannels + dec->nSaocObjects); i++)
		{
			dec->RelatedToMatrix[ch][i] = dec->pBsrInstance->saocSpecificConfig.bsRelatedTo[i][ch];
		}
	}

	dec->bandsLow     = (dec->dualMode == 1 ? dec->pBsrInstance->saocSpecificConfig.bsBandsLow : 0);
	dec->startHybBand = 0;

	dec->inSaocCICPIndex = dec->pBsrInstance->saocSpecificConfig.inSaocCICPIndex;
	for (i =0; i<CICP2GEOMETRY_MAX_LOUDSPEAKERS ; i++)
	{
		dec->saocInputGeometryInfo[i] = dec->pBsrInstance->saocSpecificConfig.saocInputGeometryInfo[i];
	}

	*nIn_WAV_IO_Channels     = dec->nChannelsIn  + dec->numInLFEs;
	*nOut_WAV_IO_Channels    = nChannelsOut + numOutLFEs;

	dec->nChannelsOut        = nChannelsOut;
	dec->numOutLFEs          = numOutLFEs;

	if (dec->LdMode==0)
	{
		dec->moduleDelay =  SAOC_SYSTEM_DELAY_HQ;
		if (isInQmf)
		{
			dec->moduleDelay -= SAOC_QMF_DELAY_ANA; 
		}
		if (isOutQmf)
		{
			dec->moduleDelay -= SAOC_QMF_DELAY_SYN; 
		}
	} 
	else 
	{
		dec->moduleDelay =  SAOC_SYSTEM_DELAY_LD;
		if (isInQmf)
		{
			dec->moduleDelay -= SAOC_QMF_DELAY_ANA_LD; 
		}
		if (isOutQmf)
		{
			dec->moduleDelay -= SAOC_QMF_DELAY_SYN_LD; 
		}
	}

	if (dec->sampleFreq < 27713.0)
	{
		dec->nQmfBands = 32;
	} 
	else if (dec->sampleFreq >= 55426.0) 
	{
		dec->nQmfBands = 128;
	} 
	else 
	{
		dec->nQmfBands = 64;
	}

	dec->frameSize   = dec->nTimeSlots  * dec->nQmfBands;
	dec->qmfSamplesPcDelay = (dec->LdMode==0 ? dec->nQmfBands * SAOC_PC_FILTERDELAY : 0);


	if (isInQmf) /* allocate buffer for qmf input */
	{       
		dec->qmfInRealBuffer = (float**) calloc(*nIn_WAV_IO_Channels, sizeof(float*));
		dec->qmfInImagBuffer = (float**) calloc(*nIn_WAV_IO_Channels, sizeof(float*));
		dec->tmpInRealQmfBuffer = (float**) calloc(*nIn_WAV_IO_Channels, sizeof(float*));
		dec->tmpInImagQmfBuffer = (float**) calloc(*nIn_WAV_IO_Channels, sizeof(float*));
		for (i = 0; i< *nIn_WAV_IO_Channels; i++) 
		{
			dec->qmfInRealBuffer[i] = (float*)calloc(dec->frameSize, sizeof(float));
			dec->qmfInImagBuffer[i] = (float*)calloc(dec->frameSize, sizeof(float));
			dec->tmpInRealQmfBuffer[i] = (float*)calloc(dec->frameSize + dec->moduleDelay, sizeof(float));
			dec->tmpInImagQmfBuffer[i] = (float*)calloc(dec->frameSize + dec->moduleDelay, sizeof(float));
			memset(dec->tmpInRealQmfBuffer[i],0,(dec->frameSize + dec->moduleDelay) * sizeof(float));
			memset(dec->tmpInImagQmfBuffer[i],0,(dec->frameSize + dec->moduleDelay) * sizeof(float));
		}
	} 
	else  /* allocate buffer for wav input */
	{      
		dec->inBuffer = (float**)calloc(*nIn_WAV_IO_Channels,sizeof(float*));
		dec->tmpInBuffer = (float**)calloc(*nIn_WAV_IO_Channels,sizeof(float*));
		for (i = 0; i< *nIn_WAV_IO_Channels; i++) 
		{
			dec->inBuffer[i] = (float*)calloc(dec->frameSize,sizeof(float));
			dec->tmpInBuffer[i] = (float*)calloc(dec->frameSize + dec->moduleDelay,sizeof(float));
			memset(dec->tmpInBuffer[i],0, (dec->frameSize + dec->moduleDelay) * sizeof(float));
		}
	}

	if (isOutQmf) /* allocate buffer for qmf output */
	{  
		dec->qmfOutRealBuffer = (float**) calloc(*nOut_WAV_IO_Channels, sizeof(float*));
		dec->qmfOutImagBuffer = (float**) calloc(*nOut_WAV_IO_Channels, sizeof(float*));
		for (i = 0; i< *nOut_WAV_IO_Channels; i++)
		{
			dec->qmfOutRealBuffer[i] = (float*)calloc(dec->frameSize, sizeof(float));
			dec->qmfOutImagBuffer[i] = (float*)calloc(dec->frameSize, sizeof(float));
		}
	} 
	else /* allocate buffer for wav output */
	{  
		dec->outBuffer = (float**)calloc(*nOut_WAV_IO_Channels,sizeof(float*));
		for (i=0; i< *nOut_WAV_IO_Channels; i++) 
		{
			dec->outBuffer[i] = (float*)calloc(dec->frameSize,sizeof(float));
		}
	}

	if (!isInQmf)  /* Init QMFlib Analysis */
	{
		QMFlib_InitAnaFilterbank(dec->nQmfBands, dec->LdMode);
		dec->pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK**) calloc(dec->nChannelsIn, sizeof(QMFLIB_POLYPHASE_ANA_FILTERBANK*));
		for (ch = 0; ch < dec->nChannelsIn; ch++)
		{
			QMFlib_OpenAnaFilterbank(&(dec->pAnalysisQmf[ch]));
		}
	}

	if (!isOutQmf) /* Init QMFLib Synthesis */
	{
		QMFlib_InitSynFilterbank(dec->nQmfBands, dec->LdMode);
		dec->pSynthesisQmf = (QMFLIB_POLYPHASE_SYN_FILTERBANK**) calloc(*nOut_WAV_IO_Channels, sizeof(QMFLIB_POLYPHASE_SYN_FILTERBANK*));
		for (ch = 0; ch < dec->nChannelsOut; ch++) {
			QMFlib_OpenSynFilterbank(&(dec->pSynthesisQmf[ch]));
		}
	}

	/* Hybrid filtering init */
	if (dec->LdMode==0) 
	{
		for (ch = 0; ch < dec->nChannelsIn; ch++) 
		{
			QMFlib_InitAnaHybFilterbank(&(dec->hybFilterState[ch]));
		}
	}

	if(dec->LdMode==0)
	{
		if (hybridMode == QMFLIB_HYBRID_THREE_TO_TEN) 
		{
			dec->nHybridBands = dec->nQmfBands - 3 + 10;
		} 
		else if (hybridMode == QMFLIB_HYBRID_THREE_TO_SIXTEEN)
		{ 
			dec->nHybridBands = dec->nQmfBands - 3 + 16;
		}
	}
	else
	{
		dec->nHybridBands = dec->nQmfBands;
	}

	SAOC_DecorrMpreMpostInit(dec->DecorrelationLevel,
		dec->nChannelsOut,
		dec->numOutLFEs,
		outCICPIndex,
		saocOutputGeometryInfo,
		&dec->numDecorrelators,
		dec->Mpre,
		dec->Mpost,
		dec->Hproto);

	for (i=0; i<dec->numDecorrelators; i++)
	{  
		*error_init = SAOC_SpatialDecDecorrelateCreate(&(dec->hDecorrDec[i]),dec->nHybridBands,0);

		SAOC_SpatialDecDecorrelateInit( dec->hDecorrDec[i],
			dec->nHybridBands,
			DecorrSeed[i], 
			0,     /* decorrType */
			0,     /* decorrConfig */
			hybridMode,
			dec->LdMode);
	}

	/* Initialize pP_dry for the fisrt interpolation step */
	for (ch = 0; ch < dec->nChannelsOut; ch++)
	{ 
		for (i = 0; i < (unsigned int)dec->numParamBands; i++)
		{
			dec->pP_dry[0][i][ch][ch] = 1.f;
		}
	}

	dec->isInQmf       = isInQmf;
	dec->isOutQmf      = isOutQmf;
	dec->hybridMode    = hybridMode;
	dec->hybridHFAlign = hybridHFAlign;

	return dec;
}


void saoc_DecoderApply( tSaocDec *self,
											 unsigned int      isLastFrame,
											 int               numOamPerFrame,
											 int               framePossOAM[32],
											 float             frameRenderMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS] ,
											 float             framePreMixMat[32][SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
											 int               coder_offset,
											 int               use_ren_mat_files,
											 float             ***pRenderMats,
											 float             *time_instances,
											 int               num_time_instances,
											 float             pRenderMat_ch[SAOC_MAX_RENDER_CHANNELS][SAOC_MAX_OBJECTS],
											 INVERSION_PARAMETERS inversionParam,
											 int               saocInputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
											 int               saocOutputCh_Mapping[SAOC_MAX_RENDER_CHANNELS],
											 int               drcFlag,
											 float***          drcMatrix,
											 int               *frameLength)

{
	int j, ch, ch1, obj, ts, qs, k;
	int nNumDmx, nNumSignals, nNumBins, nNumPremixedChannels ;

	float pOld[SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS];
	float pIoc[SAOC_MAX_OBJECTS][SAOC_MAX_OBJECTS][SAOC_MAX_PARAM_BANDS];
	float Dmx[SAOC_MAX_DMX_CHANNELS][SAOC_MAX_OBJECTS];

	int paramSet, nParamSets, env_pos;
	int pParamSlot[SAOC_MAX_PARAM_SETS];
	int repeatLastParamSet = 0;

	SAOCFRAME*        pFrameDataSaoc;

	if(!self->isInQmf) 
	{
		for (ch = 0; ch < self->nChannelsIn; ch++) 
		{
			for (ts = 0; ts < self->nTimeSlots; ts++) 
			{
				float tmp2QmfBuf[QMFLIB_MAX_NUM_QMF_BANDS] = {0}; 
				for (k=0; k < self->nQmfBands; k++) 
				{
					tmp2QmfBuf[k] = self->inBuffer[self->numInLFEs + ch][self->nQmfBands*ts+k];
				}

				QMFlib_CalculateAnaFilterbank(self->pAnalysisQmf[ch],
					tmp2QmfBuf, 
					pQmfReal[ch][ts],
					pQmfImag[ch][ts],
					self->LdMode);
			}       
		}
	} 
	else /*if(!self->isInQmf) { */
	{ 
		for (ch=0; ch<self->nChannelsIn; ch++) 
		{
			for (ts=0; ts< self->nTimeSlots; ts++) 
			{
				for (k=0; k< self->nQmfBands; k++) 
				{
					pQmfReal[ch][ts][k] = self->qmfInRealBuffer[self->numInLFEs + ch][self->nQmfBands*ts+k]; 
					pQmfImag[ch][ts][k] = self->qmfInImagBuffer[self->numInLFEs + ch][self->nQmfBands*ts+k]; 
				}
			} 
		}
	}/* end of if(!isInQmf) { */

	if(self->LdMode==0)
	{
		for (ch = 0; ch < self->nChannelsIn; ch++)
		{
			for (ts = 0; ts < self->nTimeSlots; ts++)
			{        
				QMFlib_ApplyAnaHybFilterbank(&self->hybFilterState[ch], 
					self->hybridMode,
					self->nQmfBands,
					self->hybridHFAlign,
					pQmfReal[ch][ts],
					pQmfImag[ch][ts],
					pHybReal[ch][ts], 
					pHybImag[ch][ts]); 
			}
		}
	}
	else
	{
		for (ch = 0; ch < self->nChannelsIn; ch++)
		{
			for (ts = 0; ts < self->nTimeSlots; ts++)
			{
				for (qs=0; qs< self->nQmfBands; qs++)
				{
					pHybReal[ch][ts][qs] = pQmfReal[ch][ts][qs];
					pHybImag[ch][ts][qs] = pQmfImag[ch][ts][qs];
				}
			}
		}
	}

	if (isLastFrame == SAOC_DECODE_BITSTREAM)
	{
		ReadSAOCFrame(self->bitstreamIn, self->pBsrInstance);
		PrintSAOCSpecificFrame(self->bitstreamIn, self->pBsrInstance);
	}

	pFrameDataSaoc = GetSAOCFrame(self->pBsrInstance);
	nParamSets  = pFrameDataSaoc->framingInfo.bsNumParamSets + 1;

	pParamSlot[0] = -1;
	for(paramSet = 0; paramSet < nParamSets; paramSet++)
	{
		pParamSlot[paramSet+1] = pFrameDataSaoc->framingInfo.bsParamSlots[paramSet];  
	}

	nNumDmx							 = self->nChannelsIn;
	nNumSignals					 = self->nSaocObjects + self->nSaocChannels;
	nNumPremixedChannels = self->nNumPreMixedChannels;
	nNumBins						 = self->pBsrInstance->saocSpecificConfig.bsNumBins; 
	*frameLength				 = self->pBsrInstance->saocSpecificConfig.bsFrameLength+1;

	repeatLastParamSet =  (pParamSlot[nParamSets] != *frameLength - 1 ? 1 : 0);

	for(paramSet=0; paramSet<nParamSets; paramSet++) 
	{
		env_pos = coder_offset + pParamSlot[paramSet+1];

		if (use_ren_mat_files) 
		{ 
			SAOC_getRenderMat(pRenderMatTmp,pRenderMats,num_time_instances,time_instances, env_pos);
		} 
		else 
		{
			if (self->nSaocObjects > 0) 
			{ 
				SAOC_Interpolate_oamRenMat(numOamPerFrame,
					framePossOAM,
					frameRenderMat,
					pRenderMatTmp,
					env_pos);
			}
		}

		/*  Make Ren Mat frequency dependent and combine ren mat for channels with the ren mat of objects */
		if (self->nSaocChannels > 0) 
		{
			for(ch=0; ch<self->nChannelsOut; ch++) 
			{
				for(j=0; j<self->nSaocChannels; j++) 
				{
					for(k=0; k<SAOC_MAX_PARAMETER_BANDS; k++) 
					{
						pRenderMatFreq[ch][j][k]=pRenderMat_ch[ch][j];
					}
				}
			}

			if (drcFlag == 1) /*  Apply DRC sequence for channels in parameter domain */
			{ 
				for(ch=0; ch<self->nChannelsOut; ch++) 
				{
					for(j=0; j<self->nSaocChannels; j++) 
					{
						for(k=0; k<self->numParamBands; k++) 
						{
							pRenderMatFreq[ch][j][k] = pRenderMatFreq[ch][j][k] * drcMatrix[pParamSlot[paramSet+1]][saocInputCh_Mapping[j]][k];
						}
					}
				}
			}
		}

		if (self->nSaocObjects > 0)
		{
			for(ch=0; ch<self->nChannelsOut; ch++) 
			{
				for(j=0; j<self->nSaocObjects; j++) 
				{
					for(k=0; k<SAOC_MAX_PARAMETER_BANDS; k++) 
					{
						pRenderMatFreq[ch][self->nSaocChannels + j][k]=pRenderMatTmp[ch][j];
					}
				}
			}

			if (drcFlag == 1) /*  Apply DRC sequence for objects in parameter domain */
			{ 
				for(ch=0; ch<self->nChannelsOut; ch++) 
				{
					for(j=self->nSaocChannels; j<self->nSaocChannels + self->nSaocObjects; j++) 
					{
						for(k=0; k<self->numParamBands; k++) 
						{
							pRenderMatFreq[ch][j][k] = pRenderMatFreq[ch][j][k] * drcMatrix[pParamSlot[paramSet+1]][self->numInLFEs + j][k];
						}
					}
				}
			}

			if (self->nNumPreMixedChannels > 0) 
			{
				SAOC_Interpolate_oamRenMat(numOamPerFrame,
					framePossOAM,
					framePreMixMat,
					tmp_pDmxMat_preMix,
					env_pos);
			}
		}

		GetSaocOld(pOld,pFrameDataSaoc,paramSet,nNumSignals,nNumBins);
		GetSaocIoc(pIoc,pFrameDataSaoc,paramSet,nNumSignals,nNumBins);
		SAOC_GetSaocDmxMat( Dmx,
												pFrameDataSaoc,
												paramSet,
												self->nSaocChannels,
												self->nSaocObjects,
												self->nSaocDmxChannels,
												self->nSaocDmxObjects,
												nNumPremixedChannels);

		if (self->nNumPreMixedChannels > 0) 
		{
			for(ch=0;ch<self->nSaocDmxObjects;ch++) 
			{
				for(obj=0;obj<self->nSaocObjects;obj++)	
				{
					pDmxMatTmp[ch][obj] = 0.0f;
					for(ch1=0;ch1<self->nNumPreMixedChannels;ch1++) 
					{
						pDmxMatTmp[ch][obj] += Dmx[self->nSaocDmxChannels + ch][self->nSaocChannels + ch1] * tmp_pDmxMat_preMix[ch1][obj];
					}
				}
			}
			for(ch=0;ch<self->nSaocDmxObjects;ch++) 
			{
				for(obj=0;obj<self->nSaocObjects;obj++)	
				{
					Dmx[self->nSaocDmxChannels + ch][self->nSaocChannels + obj] = pDmxMatTmp[ch][obj];
				}
			}
		}

		/* GSAOC parameter estimation */
		saoc_ParametersEstimation(pOld,
															pIoc,
															Dmx,
															self->Mpre,
															self->Mpost,
															self->Hproto,
															pRenderMatFreq,
															self->RelatedToMatrix,
															self->DecorrMethod,
															self->numDecorrelators,
															self->nChannelsOut,
															self->nSaocChannels,
															self->nSaocObjects,
															self->nSaocDmxChannels,
															self->nSaocDmxObjects,
															nNumBins,
															paramSet+1,
															inversionParam,
															self->dualMode,
															self->bandsLow,
															self->RG_matrix,
															self->pP_dry,
															self->pP_wet);

	} /* end for(paramSet=0;paramSet<nParamSets;paramSet++) { */

	/* GSAOC processing */
	saoc_Processing(Dmx,
									self->RG_matrix,
									self->pP_dry,
									self->pP_wet,
									self->Mpre,
									self->Mpost,
									self->hDecorrDec,
									self->DecorrMethod,
									self->numDecorrelators,
									self->startHybBand,
									self->nChannelsOut,
									self->LdMode,
									pHybReal,
									pHybImag,
									nNumDmx,
									nNumSignals,
									nNumBins,
									self->nHybridBands, 
									nParamSets,
									pParamSlot,
									*frameLength);

	if (self->LdMode==0) 
	{
		for (ch = 0; ch < self->nChannelsOut; ch++) 
		{
			for (ts = 0; ts < self->nTimeSlots; ts++) 
			{         
				QMFlib_ApplySynHybFilterbank( self->nQmfBands,
																			self->hybridMode,
																			pHybReal[ch][ts],
																			pHybImag[ch][ts],
																			pQmfReal[ch][ts], 
																			pQmfImag[ch][ts]); 
			}
		}
	} 
	else 
	{
		for (ts=0; ts< self->nTimeSlots; ts++) 
		{
			for (j = 0; j < self->nChannelsOut; j++) 
			{
				for (k=0; k< self->nQmfBands ; k++)
				{
					pQmfReal[j][ts][k] = pHybReal[j][ts][k];
					pQmfImag[j][ts][k] = pHybImag[j][ts][k];
				}
			}
		}
	}  

	if(!self->isOutQmf)
	{
		for (ch = 0; ch < self->nChannelsOut; ch++) 
		{
			for (ts = 0; ts < self->nTimeSlots; ts++) 
			{
				float tmpQmfBuf[QMFLIB_MAX_NUM_QMF_BANDS] = {0.0f}; 
				QMFlib_CalculateSynFilterbank(self->pSynthesisQmf[ch],
																			pQmfReal[ch][ts],
																			pQmfImag[ch][ts],
																			tmpQmfBuf, /* &outBuffer[ch][nQmfBands*ts], */
																			self->LdMode);
				for (qs=0; qs < self->nQmfBands; qs++) 
				{
					self->outBuffer[ saocOutputCh_Mapping[ch] ][self->nQmfBands*ts+qs] = tmpQmfBuf[qs];
				}
			}
		}
	}
	else 
	{
		for (ch = 0; ch < self->nChannelsOut; ch++) 
		{
			for (ts = 0; ts < self->nTimeSlots; ts++) 
			{
				for (qs=0; qs < self->nQmfBands; qs++) 
				{
					self->qmfOutRealBuffer[ saocOutputCh_Mapping[ch] ][self->nQmfBands*ts+qs] = pQmfReal[ch][ts][qs];
					self->qmfOutImagBuffer[ saocOutputCh_Mapping[ch] ][self->nQmfBands*ts+qs] = pQmfImag[ch][ts][qs];
				}
			}
		}
	}
}

void saoc_DecoderClose(tSaocDec *self, 
											 unsigned int nIn_WAV_IO_Channels,
											 unsigned int nOut_WAV_IO_Channels
											 )
{

	unsigned int i;
	int ch;

	DestroySAOCDecoder(&self->pBsrInstance);
	s_bitinput_close(self->bitstreamIn);

	for (i=0; i<self->numDecorrelators; i++) 
	{  
		SAOC_SpatialDecDecorrelateDestroy(&(self->hDecorrDec[i]), self->nHybridBands);
	}

	/* free local buffers */
	if (self->isInQmf) 
	{
		for (i=0; i< nIn_WAV_IO_Channels; i++) 
		{
			free(self->qmfInRealBuffer[i]);
			free(self->qmfInImagBuffer[i]);
			free(self->tmpInRealQmfBuffer[i]);
			free(self->tmpInImagQmfBuffer[i]);
		}
	}
	else 
	{
		for (i=0; i< nIn_WAV_IO_Channels; i++)
		{
			free(self->inBuffer[i]);
			free(self->tmpInBuffer[i]);
		}
	}

	free(self->inBuffer);
	free(self->tmpInBuffer);
	free(self->qmfInRealBuffer);
	free(self->qmfInImagBuffer);
	free(self->tmpInRealQmfBuffer);
	free(self->tmpInImagQmfBuffer);


	if (self->isOutQmf)
	{
		for (i=0; i< nOut_WAV_IO_Channels; i++)
		{
			free(self->qmfOutRealBuffer[i]);
			free(self->qmfOutImagBuffer[i]);
		}
	} 
	else 
	{
		for (i=0; i< nOut_WAV_IO_Channels; i++)
		{
			free(self->outBuffer[i]);
		}
	}

	free(self->outBuffer);
	free(self->qmfOutRealBuffer);
	free(self->qmfOutImagBuffer);

	if (!self->isInQmf)
	{
		for (ch = 0; ch < self->nChannelsIn; ch++) 
		{
			QMFlib_CloseAnaFilterbank(self->pAnalysisQmf[ch]);
		} 
	}
	if (!self->isOutQmf)
	{
		for (ch = 0; ch < self->nChannelsOut; ch++) 
		{
			QMFlib_CloseSynFilterbank(self->pSynthesisQmf[ch]);
		}
	}
	free(self);
}


