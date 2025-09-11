/*
 * tcc.c
 *
 *
 *	 This software module was originally developed by
 *
 *		Zylia Sp. z o.o.
 *
 *		Authors:
 *			Andrzej Ruminski ( andrzej.ruminski@zylia.pl )
 *			Lukasz Januszkiewicz ( lukasz.januszkiewicz@zylia.pl )
 *			Marzena Malczewska ( marzena.malczewska@zylia.pl )
 *
 *	 in the course of development of the ISO/IEC 23003-3 for reference purposes and its
 * 	 performance may not have been optimized. This software module is an implementation
 * 	 of one or more tools as specified by the ISO/IEC 23003-3 standard. ISO/IEC gives
 *	 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 *	 and make derivative works of this software module or modifications  thereof for use
 *	 in implementations or products claiming conformance to the ISO/IEC 23003-3 standard
 *	 and which satisfy any specified conformance criteria. Those intending to use this
 *	 software module in products are advised that its use may infringe existing patents.
 *	 ISO/IEC have no liability for use of this software module or modifications thereof.
 *	 Copyright is not released for products that do not conform to the ISO/IEC 23003-3
 *	 standard.
 *
 *	 Zylia Sp. z o.o. retains full right to modify and use the code for its
 *	 own purpose, assign or donate the code to a third party and to inhibit third parties
 *	 from using the code for products that do not conform to MPEG-related ITU Recommenda-
 *	 tions and/or ISO/IEC International Standards.
 *
 *	 This copyright notice must be included in all copies or derivative works.
 *
 *	 Copyright (c) ISO/IEC 2016.
 */

#include "tcc.h"
#include "tcc_struct.h"
#include "tcc_mathutils.h"
#include "tcc_decode_huff.h"
#include "tcc_decode_huff_tables.h"

#include "../src_tf/ct_sbrbitb.h"
#include "qmflib.h"

#include <string.h>



#define FREQUENCY_TRANSFORM_COEFFS  32
#define FREQUENCY_TRANSFORM_INDICES (FREQUENCY_TRANSFORM_COEFFS - 1)

#define AMPLITUDE_TRANSFORM_COEFFS  32
#define AMPLITUDE_TRANSFORM_INDICES (AMPLITUDE_TRANSFORM_COEFFS - 1)

#define TCC_FS 48000

#define DELAY_COMPENSATION 577
#define DELAY_COMPENSATION_MPS212 384



typedef struct {

	int isContinued;
	int segmentLength;
	int amplitudeQuantFlag;
	int amplitudeCoefficient[ TCC_MAX_COEFFICIENT_NUMBER ];
	int indexCoefficient[ TCC_MAX_COEFFICIENT_NUMBER ];
	int frequencyQuantFlag;
	int frequencyCoefficient[ TCC_MAX_COEFFICIENT_NUMBER ];
	int frequencyIndex[ TCC_MAX_COEFFICIENT_NUMBER ];
	int amplitudeIndex[ TCC_MAX_COEFFICIENT_NUMBER ];

} RAW_SEGMENT ;

typedef struct {

	int isContinued;
	int segmentLength;
	double* frequencyBuffer;
	double* amplitudeBuffer;

} DECODED_SEGMENT;




typedef struct _GOS_LIST_NODE{
	struct _GOS_LIST_NODE* next;

	int tccDataPresent;
	int numTrajectories;
	int channel;
	int frameNumber;
	RAW_SEGMENT* segments;

} GOS_LIST_NODE;

typedef struct {
	GOS_LIST_NODE* head;
	GOS_LIST_NODE* tail;
} GOS_LIST;

typedef struct _SEGMENT_NODE{
	DECODED_SEGMENT* segment;
	struct _SEGMENT_NODE * next;
} SEGMENT_NODE;

typedef struct {
	SEGMENT_NODE* head;
	SEGMENT_NODE* tail;
	int startFrame;
	int length;
	int isContinued;
	double lastPhaseValue;
	double lastFreqValue;
	double lastAmplValue;
	int newTrajectory;
} TRAJECTORY;

typedef struct _TRAJECTORY_NODE{
	TRAJECTORY* trajectory;
	struct _TRAJECTORY_NODE * next;
	struct _TRAJECTORY_NODE * prev;
} TRAJECTORY_NODE;

typedef struct {
	TRAJECTORY_NODE* head;
	TRAJECTORY_NODE* tail;
} TRAJECTORY_LIST;

typedef struct {
	TCC_CONFIG config;
	GOS_LIST gosList;
	int frameNumber;

	double amplitudeStep;
	double frequencyStep;

	int hop;

	float* delayCompensation[ USAC_MAX_NUM_OUT_CHANNELS ];

	TRAJECTORY_LIST trajectories[USAC_MAX_NUM_OUT_CHANNELS];

	QMFLIB_POLYPHASE_ANA_FILTERBANK* qmfFilterBank[ USAC_MAX_NUM_OUT_CHANNELS ];

} TCC_DECODER;


/* private functions */
static void parseSingleGOS( HANDLE_BIT_BUFFER bitStream, GOS_LIST_NODE* outSengment, int channel );
static void AddGOS( GOS_LIST* list, GOS_LIST_NODE* node );
static void printGos( GOS_LIST_NODE* segment );
static void printTrajectory( RAW_SEGMENT* segment );
static void addSegment( RAW_SEGMENT* segment, int channel );
static void decodeSegment( RAW_SEGMENT* segment, DECODED_SEGMENT* result );
static void freeSegmentNode( SEGMENT_NODE* segment );
static TCC_DECODER decoderHandler;


/**********************/
/*
 *
 * 	Init Tcc decoder with provided config
 *
 */
void initTccDecoder( TCC_CONFIG config ) {
	memset( &decoderHandler, 0, sizeof(TCC_DECODER) );
	decoderHandler.config = config;

	decoderHandler.amplitudeStep = 0.5 * log( pow(10.0, 0.05) );
	decoderHandler.frequencyStep = 2.0 * log( pow(2.0, 1.0/1200.0) );

	decoderHandler.hop = config.outFrameLenght / 8;

	QMFlib_InitAnaFilterbank(64, 0);

	{
		int i;
		for ( i = 0; i < config.numAffectedChannel; i++ ) {
			int delay = DELAY_COMPENSATION + ( config.stereoConfigIndex[i] == 3 ? DELAY_COMPENSATION_MPS212 : 0);

			QMFlib_OpenAnaFilterbank( &decoderHandler.qmfFilterBank[i] );
			/* for delay compensation from sbr buffer */
			decoderHandler.delayCompensation[i] = (float*)calloc( delay, sizeof(float) );

			if( config.tccMode[ i ] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) i++;
		}
	}

}


/*
 *
 * Parse data from TCC extension
 *
 */
void parseTccExtension( unsigned char* payload, int payloadLength ) {
	int i;

	if ( payload != NULL ) {

		/* creating bit buffer from imput buffer */
		HANDLE_BIT_BUFFER bitStream = (HANDLE_BIT_BUFFER)calloc( 1, sizeof(BIT_BUFFER) );
		initBitBuffer( bitStream, payload, payloadLength * 8);


		for( i = 0; i < decoderHandler.config.numAffectedChannel; i++ ) {
			GOS_LIST_NODE* currentGOS = (GOS_LIST_NODE*)calloc( 1, sizeof(GOS_LIST_NODE) );
			currentGOS->channel = i;
			currentGOS->frameNumber = decoderHandler.frameNumber;
			parseSingleGOS( bitStream, currentGOS, i );
#ifdef TCC_DEBUG
		    printGos( currentGOS );
			AddGOS( &decoderHandler.gosList, currentGOS );
#else
			free( currentGOS );
#endif

			if( decoderHandler.config.tccMode[i] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT )i++;
		}
		free( bitStream );
	}
	decoderHandler.frameNumber += 1;
}

/*
 *
 * CleanUp
 *
 */
void tccClean() {
	int i;
	for( i = 0; i < decoderHandler.config.numAffectedChannel; ++i ) {

		/* clear trajectory buffers */
		TRAJECTORY_NODE* trajectoryNode = decoderHandler.trajectories[i].head;
		while( trajectoryNode != NULL) {
			TRAJECTORY* curTrajectory = trajectoryNode->trajectory;
			SEGMENT_NODE* segmentNode = curTrajectory->head;

			while( segmentNode != NULL ) {

				segmentNode = segmentNode->next;
				/* remove the non needed segment */
				freeSegmentNode( curTrajectory->head );
				curTrajectory->head = segmentNode;

			}
			{
				TRAJECTORY_NODE* temp = trajectoryNode->next;
	
				free( trajectoryNode->trajectory);
				free( trajectoryNode );
	
				trajectoryNode = temp;
			}
		}

		QMFlib_CloseAnaFilterbank( decoderHandler.qmfFilterBank[i] );
		free( decoderHandler.delayCompensation[i] );

		if( decoderHandler.config.tccMode[ i ] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) i++;

	}

}

/*
 *
 * Add tcc data to output buffer
 *
 */
void updateOutputBuffer( float** sampleBuffer, float*** qmfBufferReal, float*** qmfBufferImag ) {

	int  channel;
	float* timeDomainData = (float*)calloc( decoderHandler.config.outFrameLenght, sizeof(float) );

	/* for each channel make synhtesis */
	for( channel = 0 ; channel < decoderHandler.config.numAffectedChannel; channel++ ) {
		TRAJECTORY_NODE* trajectoryNode = decoderHandler.trajectories[channel].head;
		
		int channelOut = decoderHandler.config.channelMap[ channel ];

		memset( timeDomainData, 0, (decoderHandler.config.outFrameLenght * sizeof(float)) );

		/*
		if( qmfBufferImag != NULL ) {
			int i, j;
			for( i = 0 ; i < 64; i++ ) {
				for( j = 0; j < 64; j++ ) {
					qmfBufferReal[i][channelOut][j] = 0;
					qmfBufferImag[i][channelOut][j] = 0;
					if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
						int tempChannel = decoderHandler.config.channelMap[channel + 1];
						qmfBufferReal[i][tempChannel][j] = 0;
						qmfBufferImag[i][tempChannel][j] = 0;
					}
				}
			}
		}
		{
			int i;
			for( i = 0 ; i < decoderHandler.config.outFrameLenght; i++ ) {
				sampleBuffer[channelOut][i] = 0.0;
				if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
					int tempChannel = decoderHandler.config.channelMap[channel + 1];
					sampleBuffer[tempChannel][i] = 0.0;
				}
			}
		}*/

		
		while( trajectoryNode != NULL) {
			TRAJECTORY* curTrajectory = trajectoryNode->trajectory;
			SEGMENT_NODE* segmentNode = curTrajectory->head;
			int currentStart = curTrajectory->startFrame;
			int curFrame = (decoderHandler.frameNumber - 1) * 8;
			int i;

			while( segmentNode != NULL ) {

				if( segmentNode->segment->segmentLength + currentStart > curFrame ) break;

				curTrajectory->length -= segmentNode->segment->segmentLength;
				curTrajectory->startFrame += segmentNode->segment->segmentLength;

				currentStart += segmentNode->segment->segmentLength;
				segmentNode = segmentNode->next;

				/* remove the non needed segment */
				freeSegmentNode( curTrajectory->head );
				curTrajectory->head = segmentNode;

			}

			if( segmentNode != NULL ) {

				DECODED_SEGMENT* curSegment = segmentNode->segment;

				if( curTrajectory->newTrajectory == 1) {
					curTrajectory->newTrajectory = 0;
					curTrajectory->lastPhaseValue = 0.0;
					curTrajectory->lastFreqValue = curSegment->frequencyBuffer[0];
					curTrajectory->lastAmplValue = 0.0;

				}

				for ( i = 0; i < 8; i++ ) {

					int idx = curFrame + i - currentStart;

					curTrajectory->lastPhaseValue =
							sinSynth( curTrajectory->lastFreqValue, curSegment->frequencyBuffer[ idx ],
										curTrajectory->lastAmplValue, curSegment->amplitudeBuffer[ idx ],
										decoderHandler.hop, curTrajectory->lastPhaseValue,
										timeDomainData + i * decoderHandler.hop );

					curTrajectory->lastFreqValue = curSegment->frequencyBuffer[ idx ];
					curTrajectory->lastAmplValue = curSegment->amplitudeBuffer[ idx ];

				}

			} else {
				/* trajectory is not needed now */
				if( trajectoryNode->next != NULL ) trajectoryNode->next->prev = trajectoryNode->prev;
				if( trajectoryNode->prev != NULL ) trajectoryNode->prev->next = trajectoryNode->next;
				if( trajectoryNode->next == NULL ) {
					decoderHandler.trajectories[channel].tail = trajectoryNode->prev;
				}
				if( trajectoryNode->prev == NULL ) {
					decoderHandler.trajectories[channel].head = trajectoryNode->next;
				}

				{
					TRAJECTORY_NODE* temp = trajectoryNode->next;

					free( trajectoryNode->trajectory);
					free( trajectoryNode );

					trajectoryNode = temp;
				}
				continue;
			}

			trajectoryNode = trajectoryNode->next;
		}


		{
			int cnt;
			int delay =  DELAY_COMPENSATION +
					( decoderHandler.config.stereoConfigIndex[channel] == 3 ? DELAY_COMPENSATION_MPS212 : 0);
			for( cnt = 0; cnt < delay; cnt++ ) {
				sampleBuffer[channelOut][cnt] += decoderHandler.delayCompensation[channel][cnt] * 32768.f;

				if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
					int tempChannel = decoderHandler.config.channelMap[channel + 1];
					sampleBuffer[tempChannel][cnt] += decoderHandler.delayCompensation[channel][cnt] * 32768.f;
				}

			}
	
			/* add samples to output buffer */
			for( cnt = delay; cnt < decoderHandler.config.outFrameLenght; cnt++ ) {
				sampleBuffer[channelOut][cnt] += timeDomainData[ cnt - delay ] * 32768.f;
				if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
					int tempChannel = decoderHandler.config.channelMap[channel + 1];
					sampleBuffer[tempChannel][cnt] += timeDomainData[ cnt - delay ] * 32768.f;
				}
			}
			memcpy( decoderHandler.delayCompensation[channel],
					timeDomainData + decoderHandler.config.outFrameLenght - delay,
					sizeof(float) * delay );

		}

		/* QMF ANALISYS */
		if( qmfBufferImag != NULL ) {
			int resolution = 64;
			int cnt = 0;
			int idx = 0;
			int samplesToQmfAnalysis = decoderHandler.config.outFrameLenght;
			float* qmfReal = (float*)calloc( resolution, sizeof(float));
			float* qmfImag = (float*)calloc( resolution, sizeof(float));

			/* delay compensation for qmf analisys for stereConfigIndex == 3 */
			if( decoderHandler.config.stereoConfigIndex[channel] == 3 ) {
				samplesToQmfAnalysis -= DELAY_COMPENSATION_MPS212;
				for ( cnt = 0; cnt < DELAY_COMPENSATION_MPS212; cnt += resolution ) {

					memset( qmfReal, 0, sizeof(float) * resolution );
					memset( qmfImag, 0, sizeof(float) * resolution );

					QMFlib_CalculateAnaFilterbank( decoderHandler.qmfFilterBank[channel],
												   decoderHandler.delayCompensation[channel] + cnt + DELAY_COMPENSATION,
												   qmfReal,
												   qmfImag,
												   0);
					{
						int band;
						for ( band = 0; band < resolution; band++ ) {
							qmfBufferReal[ idx ] [ channelOut ][ band ] += qmfReal[band];
							qmfBufferImag[ idx ] [ channelOut ][ band ] += qmfImag[band];
							if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
								int tempChannel = decoderHandler.config.channelMap[channel + 1];
								qmfBufferReal[ idx ] [ tempChannel ][ band ] += qmfReal[band];
								qmfBufferImag[ idx ] [ tempChannel ][ band ] += qmfImag[band];
							}
						}
						idx++;
					}

				}

			}

			for ( cnt = 0; cnt < samplesToQmfAnalysis; cnt += resolution ){

				memset( qmfReal, 0, sizeof(float) * resolution );
				memset( qmfImag, 0, sizeof(float) * resolution );


				QMFlib_CalculateAnaFilterbank( decoderHandler.qmfFilterBank[channel],
											   timeDomainData + cnt,
											   qmfReal,
											   qmfImag,
											   0); /* LD mode */
				{
					int band;
					for ( band = 0; band < resolution; band++ ) {
						qmfBufferReal[ idx ] [ channelOut ][ band ] += qmfReal[band];
						qmfBufferImag[ idx ] [ channelOut ][ band ] += qmfImag[band];
						if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
							int tempChannel = decoderHandler.config.channelMap[channel + 1];
							qmfBufferReal[ idx ] [ tempChannel ][ band ] += qmfReal[band];
							qmfBufferImag[ idx ] [ tempChannel ][ band ] += qmfImag[band];
						}
					}
					idx++;
				}

			}
			free(qmfReal);
			free(qmfImag);
		}

		if( decoderHandler.config.tccMode[channel] == TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT ) {
			channel++;
		}
	}
	free(timeDomainData);

}

/*#define LOG(x) x */
#define LOG(x)


static void freeSegmentNode( SEGMENT_NODE* segment ) {

	free( segment->segment->amplitudeBuffer );
	free( segment->segment->frequencyBuffer );

	segment->segment->amplitudeBuffer = NULL;
	segment->segment->frequencyBuffer = NULL;

	free( segment->segment );
	segment->segment = NULL;

	free (segment);

}

/* parse GOS for one channel */
static void parseSingleGOS( HANDLE_BIT_BUFFER bitStream, GOS_LIST_NODE* gos, int channel ){

	gos->tccDataPresent = BufGetBits( bitStream, BS_TCC_PRESENT );

	if( gos->tccDataPresent == 1 ) {
		int i;
		gos->numTrajectories = BufGetBits( bitStream, BS_TCC_NO_TRJ ) + 1;
		LOG( fprintf(stderr, "GOS %d: number of trajectories %d\n", decoderHandler.frameNumber, gos->numTrajectories  ));

		gos->segments = ( RAW_SEGMENT* )calloc( gos->numTrajectories, sizeof(RAW_SEGMENT) );

		for( i = 0; i < gos->numTrajectories; i++ ) {
			int tempIndexBuffer[ TCC_MAX_SEGMENT_LENGTH ];
			int numAcCoefficient = 0;
			int r, k;
			gos->segments[i].isContinued   = BufGetBits( bitStream, BS_TCC_TRJ_ISCONT );
			gos->segments[i].segmentLength = ( BufGetBits(bitStream, BS_TCC_TRJ_LEN ) + 1 ) * 8;

			gos->segments[i].amplitudeQuantFlag = BufGetBits( bitStream, BS_TCC_AMP_QUANT_FLAG );
			/* read DC of amplitude */
			gos->segments[i].amplitudeCoefficient[0] = - ((long int)BufGetBits( bitStream, BS_TCC_AMP_DC ) + BS_TCC_AMP_DC_OFFSET);

			for ( r = 0; r < AMPLITUDE_TRANSFORM_INDICES; r++ ) {
				int decodedTemp = tccHuffmanDecode( bitStream, &tccTab_idx );
				if( decodedTemp == 0 ) {
					break;
				} else {
					tempIndexBuffer[r] = decodedTemp;
					gos->segments[i].amplitudeIndex[r + 1] = decodedTemp;
				}
			}

			for ( k = 0; k < r; k++ ) {
				gos->segments[i].amplitudeCoefficient[ tempIndexBuffer[k] ] = tccHuffmanDecode( bitStream, &tccTab_AC ) + 1;
			}

			/* frequencies */
			gos->segments[i].frequencyQuantFlag = BufGetBits( bitStream, 1 );

			gos->segments[i].frequencyCoefficient[0] = - ( (long int)BufGetBits( bitStream, BS_TCC_FREQ_DC ) + BS_TCC_FREQ_DC_OFFSET );

			for ( r = 0; r < FREQUENCY_TRANSFORM_INDICES; r++ ) {
				int decodedTemp = tccHuffmanDecode( bitStream, &tccTab_idx );
				if( decodedTemp == 0 ) {
					break;
				} else {
					tempIndexBuffer[r] = decodedTemp;
					gos->segments[i].frequencyIndex[r + 1] = decodedTemp;
				}
			}

			for ( k = 0; k < r; k++ ) {
				gos->segments[i].frequencyCoefficient[ tempIndexBuffer[k] ] = tccHuffmanDecode( bitStream, &tccTab_AC ) + 1;
			}

			addSegment( &gos->segments[i], channel );

		}

#ifndef TCC_DEBUG
			free( gos->segments );
#endif

	}

}

/* add segment to trajectory */
static void addSegment( RAW_SEGMENT* segment, int channel ) {
	DECODED_SEGMENT* decodedSegment = (DECODED_SEGMENT*)calloc( 1, sizeof( DECODED_SEGMENT ) );
	TRAJECTORY_NODE* trajectoryHandler = NULL;

	decodeSegment( segment, decodedSegment );

	/* now we have decoded trajectory */
	/* search for trajectory which can be continued */
	trajectoryHandler = decoderHandler.trajectories[channel].head;

	while( trajectoryHandler != NULL ) {

			if ( trajectoryHandler->trajectory->isContinued == 1 ) {
				if( trajectoryHandler->trajectory->startFrame + trajectoryHandler->trajectory->length
						== decoderHandler.frameNumber * 8 ) {

					SEGMENT_NODE* newNode = (SEGMENT_NODE*)calloc( 1, sizeof(SEGMENT_NODE) );
					newNode->segment = decodedSegment;
					newNode->next = NULL;
					trajectoryHandler->trajectory->tail->next = newNode;
					trajectoryHandler->trajectory->tail = newNode;
					trajectoryHandler->trajectory->isContinued = newNode->segment->isContinued;
					trajectoryHandler->trajectory->length =
							trajectoryHandler->trajectory->length + newNode->segment->segmentLength;
					return;
				}
			}
			trajectoryHandler = trajectoryHandler->next;
	}

	/* meet new trajectory */
	{
		SEGMENT_NODE* newNode = (SEGMENT_NODE*)calloc( 1, sizeof(SEGMENT_NODE) );
		TRAJECTORY_NODE* newTrajectoryNode = (TRAJECTORY_NODE*)calloc( 1, sizeof(TRAJECTORY_NODE) );
	
		newNode->segment = decodedSegment;
		newNode->next = NULL;
	
	
		newTrajectoryNode->trajectory = (TRAJECTORY*)calloc( 1, sizeof(TRAJECTORY) );
		newTrajectoryNode->trajectory->head = newNode;
		newTrajectoryNode->trajectory->tail = newNode;
		newTrajectoryNode->trajectory->isContinued = newNode->segment->isContinued;
		newTrajectoryNode->trajectory->length = newNode->segment->segmentLength;
		newTrajectoryNode->trajectory->startFrame = decoderHandler.frameNumber  * 8;
		newTrajectoryNode->trajectory->newTrajectory = 1;
	
		if( decoderHandler.trajectories[channel].head != NULL ) {
			decoderHandler.trajectories[channel].tail->next = newTrajectoryNode;
			newTrajectoryNode->prev = decoderHandler.trajectories[channel].tail;
			decoderHandler.trajectories[channel].tail = newTrajectoryNode;
		} else {
			decoderHandler.trajectories[channel].head = newTrajectoryNode;
			decoderHandler.trajectories[channel].tail = newTrajectoryNode;
			newTrajectoryNode->prev = NULL;
		}
		newTrajectoryNode->next = NULL;
	}
}


static void AddGOS( GOS_LIST* list, GOS_LIST_NODE* node ){
	if( list->head == NULL ) {
		list->head = node;
		list->tail = node;
		node->next = NULL;
	} else {
		list->tail->next = node;
		list->tail = node;
		node->next = NULL;
	}
}

/* decode trajectory */
static void decodeSegment( RAW_SEGMENT* segment, DECODED_SEGMENT* result ) {
	int i;

	result->isContinued = segment->isContinued;
	result->segmentLength = segment->segmentLength;

	result->amplitudeBuffer = (double*)calloc( segment->segmentLength, sizeof(double) );
	result->frequencyBuffer = (double*)calloc( segment->segmentLength, sizeof(double) );

	for( i = 0 ; i < segment->segmentLength; i++ ) {
		double ampStep  = decoderHandler.amplitudeStep * ( segment->amplitudeQuantFlag == 0 ? 1.0 : 2.0 );
		double freqStep = decoderHandler.frequencyStep * ( segment->frequencyQuantFlag == 0 ? 1.0 : 2.0 );
		result->amplitudeBuffer[i] = iquant( segment->amplitudeCoefficient[i], ampStep, 0.25 * ampStep );
		result->frequencyBuffer[i] = iquant( segment->frequencyCoefficient[i], freqStep, 0.25 * freqStep );
	}

	/* compute idct on buffers */
	idct( result->amplitudeBuffer, segment->segmentLength );
	idct( result->frequencyBuffer, segment->segmentLength );

	for( i = 0 ; i < segment->segmentLength; i++ ) {
		result->amplitudeBuffer[i] *= sqrt((float)segment->segmentLength);
		result->frequencyBuffer[i] *= sqrt((float)segment->segmentLength);

		result->amplitudeBuffer[i] = exp( result->amplitudeBuffer[i] );
		result->frequencyBuffer[i] = exp( result->frequencyBuffer[i] );

		/* scale frequency to sampling rate */
		result->frequencyBuffer[i] = result->frequencyBuffer[i] * TCC_FS/(float)decoderHandler.config.samplingRate;
	}

}

static void printTrajectory( RAW_SEGMENT* segment ) {
	int i;
	int numberOfCoef = 0;
	printf( "\nTRAJECTORY\n" );
	printf( "isContinued %d\n", segment->isContinued );
	printf( "segmentLength %d\n", segment->segmentLength );
	printf( "QUANT FLAG AMP %d FREQ %d\n", segment->amplitudeQuantFlag, segment->frequencyQuantFlag );
	printf( "FREQUENCY COEFFICIENT\n" );

	for ( i = 0 ; i < TCC_MAX_COEFFICIENT_NUMBER; i++ ) {
		if( segment->frequencyCoefficient[i] != 0  ) numberOfCoef++;
	}

	for ( i = 0 ; i < numberOfCoef; i++ ) {
		if( segment->frequencyCoefficient[segment->frequencyIndex[i]] != 0 ) {
			printf( "{ %d %d } ", segment->frequencyIndex[i], segment->frequencyCoefficient[segment->frequencyIndex[i]] );
		}
	}
	printf( "\n" );
	printf( "AMP COEFFICIENT\n" );

	numberOfCoef = 0;
	for ( i = 0 ; i < TCC_MAX_COEFFICIENT_NUMBER; i++ ) {
		if( segment->amplitudeCoefficient[i] != 0  ) numberOfCoef++;
	}
	for ( i = 0 ; i < numberOfCoef; i++ ) {
		if( segment->amplitudeCoefficient[segment->amplitudeIndex[i]] != 0 ) {
			printf( "{ %d %d } ", segment->amplitudeIndex[i], segment->amplitudeCoefficient[ segment->amplitudeIndex[i] ] );
		}
	}
	printf( "\n" );

}

static void printGos( GOS_LIST_NODE* gos) {

	printf( "------------------------------------\n" );
	printf( "FRAME %d : %d\n", gos->frameNumber, gos->tccDataPresent );
	if( gos->tccDataPresent ) {
		int i;
		printf( "Number of Trajectories %d\n", gos->numTrajectories );
		for ( i = 0; i < gos->numTrajectories; i++ ) {
			printTrajectory( &gos->segments[i] );

		}
	} else {
		printf( "No Tcc data\n" );
	}
}

