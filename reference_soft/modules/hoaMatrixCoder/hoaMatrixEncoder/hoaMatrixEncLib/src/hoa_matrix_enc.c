/***********************************************************************************
 
 This software module was originally developed by 
 
 Qualcomm Techonologies, Inc. (QTI)
 
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
 
 Qualcomm Techonologies, Inc. (QTI) retains full right to modify and use the code 
 for its own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include "hoa_matrix_enc.h"


static int getCeilLog2(unsigned int unX)
{
	unsigned int unTmp = 1;
	unsigned int unN = 0;

	if(unX==0)
		return 0;
	while( (unTmp<<(unN)) < unX)
	{
		unN++;
	}
	return unN;
}


static int integer_log2(unsigned int n)
{
	int ilog2 = 0;

	while (n > 1) {
		n >>= 1;
		++ilog2;
	}

	return ilog2;
}

static int WriteRange(wobitbufHandle hBitstream, unsigned int value, unsigned int alphabetSize, int* bitCount)
{
	int error = 0;

	int nBits = integer_log2(alphabetSize);
	unsigned int nUnused = (1U << (nBits + 1)) - alphabetSize;
	if (value < nUnused) {
		error = wobitbuf_WriteBits(hBitstream, value, nBits); if (error) return -1; *bitCount+=nBits;
	} else {
		value += nUnused;
		error = wobitbuf_WriteBits(hBitstream, value >> 1, nBits); if (error) return -1; *bitCount+=nBits;
		error = wobitbuf_WriteBits(hBitstream, value & 1, 1); if (error) return -1; *bitCount+=1;
	}

	return error;
}


static int WriteEscapedValue(wobitbufHandle hBitStream, unsigned int value, int count1, int count2, int count3, int *bitCount)
{
	int error = 0;

	if (value < (1U << count1) - 1) {
		error = wobitbuf_WriteBits(hBitStream, value, count1); if (error) return -1; *bitCount+=count1;
	} else {
		unsigned int escape1 = (1U << count1) - 1;
		error = wobitbuf_WriteBits(hBitStream, escape1, count1); if (error) return -1; *bitCount+=count1;
		value -= escape1;
		if (value < (1U << count2) - 1) {
			error = wobitbuf_WriteBits(hBitStream, value, count2); if (error) return -1; *bitCount+=count2;
		} else {
			unsigned int escape2 = (1U << count2) - 1;
			error = wobitbuf_WriteBits(hBitStream, escape2, count2); if (error) return -1; *bitCount+=count2;
			value -= escape2;
			assert(value < (1U << count3));
			error = wobitbuf_WriteBits(hBitStream, value, count3); if (error) return -1; *bitCount+=count3;
		}
	}

	return error;
}

static int EncodeHoaGainValue(wobitbufHandle hBitStream, CoderState* cs, float gain_dB, int order, int *bitCount)
{
	int error = 0;

		int numValues = ((cs->maxGain[order] - cs->minGain[order]) << cs->precisionLevel) + 2;
		int gainIndex;

		if (gain_dB == HOA_MATRIX_GAIN_ZERO) {
			gainIndex = numValues - 1; /* the largest gain index indicates HOA_MATRIX_GAIN_ZERO */
		} else {
			gainIndex = (int)((cs->maxGain[order] - gain_dB) * (float) (1 << cs->precisionLevel));
		}

		error = WriteRange(hBitStream, gainIndex, numValues, bitCount); if (error) return -1; 


	return 0;
}


int EncodeHoaMatrix(int inputCount,
		    /*int outputIndex, */
					int outputCount,
					SpeakerInformation* outputConfig,
					int precisionLevel,
					wobitbufHandle hBitStream,
					double* hoaMatrix)					
{
	int maxHoaOrder = (int)sqrt((float)inputCount)-1;
	int gainLimitPerHoaOrder = 1; /* examples setting for RefSoft  */
	int isFullMatrix = 1;
	int firstSparseOrder = maxHoaOrder; 
	int isNormalized = 0;
	int i, j, k, sign1, sign2;
	double val1, val2;
	int valSymmetric, signSymmetric;  
	int nbitsHoaOrder = getCeilLog2(maxHoaOrder+1);
	int currentHoaOrder = 0; 
	double currentScalar;
	int isHoaCoefSparse[HOA_MATRIX_MAX_HOA_COEF_COUNT];
	int hasValue = 1;
	int lfeExist = 0;
	int hasLfeRendering = 0;
	int zerothOrderAlwaysPositive = 1;
	int numPairs = 0;
	int isSignSymmetric[HOA_MATRIX_MAX_SPEAKER_COUNT];
	int isValueSymmetric[HOA_MATRIX_MAX_SPEAKER_COUNT];
	int valueSymmetricPairs[HOA_MATRIX_MAX_SPEAKER_COUNT];
	int signSymmetricPairs[HOA_MATRIX_MAX_SPEAKER_COUNT];
	int pairIdx = 0; 
	int signMatrix[HOA_MATRIX_MAX_HOA_COEF_COUNT * HOA_MATRIX_MAX_SPEAKER_COUNT];
	int isAnySignSymmetric = 0;
	int isAllSignSymmetric = 0;
	int isAnyValueSymmetric = 0;
	int isAllValueSymmetric = 0;
	CoderState cs;
	int error = 0;

	double multiplier = (1 << precisionLevel);
	double invMultiplier = (1.0 / multiplier);
	double gain_dB;
	double minGain = 0.0; 
	double maxGain = -60.0; 
	int bitCount = 0;
	int numValueSymmetricPairs = 0;
	int numSignSymmetricPairs = 0;
	int hasVerticalCoef = 0;
	int symSigns[HOA_MATRIX_MAX_HOA_COEF_COUNT];  
	int vertBitmask[HOA_MATRIX_MAX_HOA_COEF_COUNT];  
	createSymSigns(&symSigns[0], maxHoaOrder);
	create2dBitmask(&vertBitmask[0], maxHoaOrder);
		
	
	if (inputCount > outputCount){
		int eqOrder = (int)ceil(sqrt((float)outputCount));
		if (maxHoaOrder >= eqOrder){		
			isFullMatrix = 0;
			firstSparseOrder = eqOrder;  /* examples setting for RefSoft  */
		}
	}
	
	/* check if Matrix is normalized*/
	 currentScalar = 0.0;
	 /*compute Frobenius Norm: */
	  for (i = 0; i < inputCount; ++i) {
		  for (j = 0; j < outputCount; ++j) {
			  if (!outputConfig[j].isLFE)
				  currentScalar += hoaMatrix[i * outputCount + j] * hoaMatrix[i * outputCount + j];
		  }
	  }
	  currentScalar = fabs(1.0-sqrt(currentScalar));	 
	  if (currentScalar<(1e-6)){
		  isNormalized = 1;
	  }

	/* convert the hoa matrix to dB domain and quantize it to the requested precisionLevel */
	CoderStateInit(&cs, (float)maxGain+1.0f);
	assert(precisionLevel <= 3);
	for (i = 0; i < inputCount; ++i) {
		currentHoaOrder = (int)ceil(sqrt(i+1.0f)-1);
		currentScalar = sqrt(2*currentHoaOrder + 1.0);
		for (j = 0; j < outputCount; ++j) {
			double value = hoaMatrix[i * outputCount + j];				
			value *= currentScalar;
			if (currentHoaOrder==0) {
				if (value < 0.0)			
					zerothOrderAlwaysPositive = 0;
			}			
			if (value != 0.0) {				
				gain_dB = (20.0 * log10(fabs(value)));
				signMatrix[i * outputCount + j] = (value > 0.0);
				gain_dB = (floor(gain_dB * multiplier + 0.5) * invMultiplier);
				gain_dB = min(gain_dB, minGain);
				if (gain_dB < maxGain) {
				  /*gain_dB = maxGain; */
					hoaMatrix[i * outputCount + j] = HOA_MATRIX_GAIN_ZERO;
				}
				else {
					hoaMatrix[i * outputCount + j] = gain_dB;
				}					
				if (vertBitmask[i] && (hoaMatrix[i * outputCount + j]!=HOA_MATRIX_GAIN_ZERO)) {
					hasVerticalCoef = 1;
				}
				
				if ((gain_dB>=maxGain) && (hasVerticalCoef || !vertBitmask[i])){
					if (gainLimitPerHoaOrder) {
						cs.minGain[currentHoaOrder] = min(cs.minGain[currentHoaOrder], (int)floor(gain_dB));
						cs.maxGain[currentHoaOrder] = max(cs.maxGain[currentHoaOrder], (int)ceil(gain_dB));
					}
					else {
						cs.minGain[0] = min(cs.minGain[0], (int)floor(gain_dB));
						cs.maxGain[0] = max(cs.maxGain[0], (int)ceil(gain_dB));
					}
				}
			} else {
				hoaMatrix[i * outputCount + j] = HOA_MATRIX_GAIN_ZERO;
				signMatrix[i * outputCount + j] = 1;		 
			}	
			if (outputConfig[j].isLFE) { 
				lfeExist = 1;
				if (hoaMatrix[i * outputCount + j] != HOA_MATRIX_GAIN_ZERO){
					hasLfeRendering = 1;
				}
			}
		}
	}  
	
	for (i = currentHoaOrder; i > -1;--i) { 
		if (cs.minGain[i] > cs.maxGain[i]){	
			firstSparseOrder = min(firstSparseOrder, i);
			cs.maxGain[i] =  0; 
			cs.minGain[i] =  -1; 	
		}else if(cs.minGain[i] == cs.maxGain[i]){
			cs.minGain[i] -=  1; 
		}
	}
	
	if (!gainLimitPerHoaOrder) {
		for (i = 1; i  < (currentHoaOrder+1); ++i) {
			cs.minGain[i] = cs.minGain[0];
			cs.maxGain[i] = cs.maxGain[0];
		}
	}

	numPairs = findSymmetricSpeakers(outputCount, outputConfig, hasLfeRendering);


	for (k = 0; k < outputCount; ++k) {
		if (outputConfig[k].isAlreadyUsed) continue;
		if ((!hasLfeRendering && outputConfig[k].isLFE) || (outputConfig[k].symmetricPair == SP_PAIR_CENTER))	  
		{
			isSignSymmetric[k] = 0;
			isValueSymmetric[k] = 0;
			continue;
		}
		valSymmetric = 1;
		signSymmetric = 1;

		j = outputConfig[k].symmetricPair->originalPosition;
		for (i = 0; i < inputCount; ++i) {	
			val1 = hoaMatrix[i * outputCount + k];
			val2 = hoaMatrix[i * outputCount + j];
			sign1 =  (2*signMatrix[i * outputCount + k])-1;
			sign2 =  (2*signMatrix[i * outputCount + j])-1;

			if (0 == ((val1==HOA_MATRIX_GAIN_ZERO) && (val2==HOA_MATRIX_GAIN_ZERO))){
			  /*if ((val1!=HOA_MATRIX_GAIN_ZERO) && (val2!=HOA_MATRIX_GAIN_ZERO)){ */
				if (sign1*val1 != (sign2 * symSigns[i] * val2)) {
					valSymmetric = 0;					
				}	
				if (sign1 != (symSigns[i] * sign2)) {
					signSymmetric = 0;					
				}			  
			}
		} 		
		isSignSymmetric[j] = 0;
		isSignSymmetric[k] = signSymmetric;
		signSymmetricPairs[pairIdx] = signSymmetric;
		numSignSymmetricPairs +=  signSymmetric;
		isValueSymmetric[j] = 0;
		isValueSymmetric[k] = valSymmetric;
		valueSymmetricPairs[pairIdx] = valSymmetric;		
		numValueSymmetricPairs += valSymmetric;
		pairIdx++;
		outputConfig[j].isAlreadyUsed = 1;
	}


	if (numValueSymmetricPairs == numPairs) {
		isAllValueSymmetric = 1;
		isAllSignSymmetric  = 0;
		isAnySignSymmetric  = 0;
		numSignSymmetricPairs = 0;
	}
	else if (numValueSymmetricPairs){
		isAnyValueSymmetric = 1;
		for (i=0;i<numPairs;++i) {
			if (signSymmetricPairs[i] && valueSymmetricPairs[i]){
				signSymmetricPairs[i] = 0;
				numSignSymmetricPairs--;
			}
		}
	} else {
		isAnyValueSymmetric=0;
	}


	isAllSignSymmetric = (numSignSymmetricPairs == numPairs);
	isAnySignSymmetric = numSignSymmetricPairs>0;

	if (0 == isFullMatrix) {	  
		for (i = 0; i<(firstSparseOrder*firstSparseOrder); ++i) {
			isHoaCoefSparse[i] = 0;
		}
		for (i; i< inputCount; ++i) {
			isHoaCoefSparse[i] = 1;
		}
	} else {
		for (i=0; i< inputCount; ++i) {
			isHoaCoefSparse[i] = 0;
		}
	}

	printf("precisionLevel %1d, precision %4.2f dB\n", precisionLevel, 1.0f / (1 << precisionLevel));
	if (gainLimitPerHoaOrder) {
		for (i = 0; i  < (currentHoaOrder+1); ++i) {
			printf("order %d minGain %3d maxGain %3d\n", i, cs.minGain[i], cs.maxGain[i]);
		}
	} else{ printf("minGain %3d maxGain %3d\n", cs.minGain[0], cs.maxGain[0]);}
	printf("\n");

	error = wobitbuf_WriteBits(hBitStream, precisionLevel, 2); if (error) return -1; bitCount += 2;
	error = wobitbuf_WriteBits(hBitStream, isNormalized, 1); if (error) return -1; bitCount += 1;
	error = wobitbuf_WriteBits(hBitStream, gainLimitPerHoaOrder, 1); if (error) return -1; bitCount += 1;
	if (1 == gainLimitPerHoaOrder) {
		for (i = 0; i < (1+currentHoaOrder);++i) { 
			error = WriteEscapedValue(hBitStream, (unsigned int) -cs.maxGain[i], 3, 5, 6, &bitCount); if (error) return -1; 
			error = WriteEscapedValue(hBitStream, (unsigned int) (-cs.minGain[i] - 1 + cs.maxGain[i]), 4, 5, 6, &bitCount); if (error) return -1; 
		}
	}
	else {	
		error = WriteEscapedValue(hBitStream, (unsigned int) -cs.maxGain[0], 3, 5, 6, &bitCount); if (error) return -1; 
		error = WriteEscapedValue(hBitStream, (unsigned int) (-cs.minGain[0] - 1 + cs.maxGain[0]), 4, 5, 6, &bitCount); if (error) return -1; 
	}
	error = wobitbuf_WriteBits(hBitStream, isFullMatrix, 1); if (error) return -1; bitCount += 1;
	if (0 == isFullMatrix) {
		error = wobitbuf_WriteBits(hBitStream, firstSparseOrder, nbitsHoaOrder); if (error) return -1; bitCount += nbitsHoaOrder;
	}
	if (lfeExist){ error = wobitbuf_WriteBits(hBitStream, hasLfeRendering, 1); if (error) return -1; bitCount++; }   
	error = wobitbuf_WriteBits(hBitStream, zerothOrderAlwaysPositive, 1); if (error) return -1;  bitCount++;    
	error = wobitbuf_WriteBits(hBitStream, isAllValueSymmetric, 1); if (error) return -1; bitCount++; 
	if (0==isAllValueSymmetric) {
		error = wobitbuf_WriteBits(hBitStream, isAnyValueSymmetric, 1); if (error) return -1; bitCount++; 
		if (isAnyValueSymmetric){
			for (i=0;i<numPairs;++i) {
				error = wobitbuf_WriteBits(hBitStream, valueSymmetricPairs[i], 1); if (error) return -1; bitCount++; 
			}
			error = wobitbuf_WriteBits(hBitStream, isAnySignSymmetric, 1); if (error) return -1; bitCount++; 
			if (isAnySignSymmetric) {
				if (isAnyValueSymmetric){
					for (i=0;i<numPairs;++i) {
						if (0==valueSymmetricPairs[i]) {
							error = wobitbuf_WriteBits(hBitStream, signSymmetricPairs[i], 1); if (error) return -1; bitCount++; 
						}
					}
				}
				else{
					for (i=0;i<numPairs;++i) {
						error = wobitbuf_WriteBits(hBitStream, signSymmetricPairs[i], 1); if (error) return -1; bitCount++; 
					}
				}
			}
		}
		else {	
			error = wobitbuf_WriteBits(hBitStream, isAllSignSymmetric, 1); if (error) return -1; bitCount++; 
			if (0==isAllSignSymmetric) {
				error = wobitbuf_WriteBits(hBitStream, isAnySignSymmetric, 1); if (error) return -1; bitCount++; 
				if (isAnySignSymmetric) {
					for (i=0;i<numPairs;++i) {
						error = wobitbuf_WriteBits(hBitStream, signSymmetricPairs[i], 1); if (error) return -1; bitCount++; 
					}
				}
			}
		}
	}


	error = wobitbuf_WriteBits(hBitStream, hasVerticalCoef, 1); if (error) return -1; bitCount++; 
	printf("bitCount of config %4d\n", bitCount);
	cs.precisionLevel = precisionLevel;


	/*////////////////////////////////////////////////////////////////////////////
	// End of Config - starting to encode the HOA matrix  
	////////////////////////////////////////////////////////////////////////////*/
	for (i = 0; i < inputCount; ++i) {			
		if ((vertBitmask[i] && hasVerticalCoef) || !vertBitmask[i]) {  
			currentHoaOrder = (int)ceil(sqrt(i+1.0f)-1);
			for (j = outputCount-1; j >= 0; --j) { 
				float value = (float) hoaMatrix[i * outputCount + j];
				hasValue = 1;
				if ( (hasLfeRendering && outputConfig[j].isLFE) || (!outputConfig[j].isLFE) ) {

					if (0==isValueSymmetric[j]) {					
						if (isHoaCoefSparse[i]){
							hasValue = (value != HOA_MATRIX_GAIN_ZERO);
							error = wobitbuf_WriteBits(hBitStream, hasValue, 1); 
							if (error) return -1; bitCount++; 
						}
					} 
					else { hasValue = 0; }
					if (hasValue) {					  
						error = EncodeHoaGainValue(hBitStream, &cs, value, currentHoaOrder, &bitCount); 
						if (error) return -1; 
						if (0==isSignSymmetric[j]) {
							if (value != HOA_MATRIX_GAIN_ZERO){
								if (currentHoaOrder || !zerothOrderAlwaysPositive) {
									error = wobitbuf_WriteBits(hBitStream, signMatrix[i * outputCount + j], 1); 
									if (error) return -1; bitCount++; 
								}
							}
						}
					}			  
				} 			
			}
		}
	}

	printf("\n");
	printf("total bitCount %4d\n", bitCount);
	printf("compressed to %4.2f bits per matrix element\n", (float) bitCount / (inputCount * outputCount));
	printf("\n");

	return error;
}
