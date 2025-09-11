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

#include "hoa_matrix_dec.h"

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

static unsigned int ReadRange(robitbufHandle hBitstream, unsigned int alphabetSize)
{
  int nBits = integer_log2(alphabetSize);
  unsigned int nUnused = (1U << (nBits + 1)) - alphabetSize;
  unsigned int value = robitbuf_ReadBits(hBitstream, nBits);

  if (value >= nUnused) {
    value = (value << 1) - nUnused + robitbuf_ReadBits(hBitstream, 1);
  }

  return value;
}

int ReadEscapedValue(robitbufHandle hBitStream, int count1, int count2, int count3)
{
  unsigned int value = 0;

  unsigned int value1 = robitbuf_ReadBits(hBitStream, count1);
  unsigned int escape1 = (1U << count1) - 1;

  value = value1;
  if (value1 == escape1) {
    unsigned int value2 = robitbuf_ReadBits(hBitStream, count2);
    unsigned int escape2 = (1U << count2) - 1;

    value += value2;
    if (value2 == escape2) {
      unsigned int value3 = robitbuf_ReadBits(hBitStream, count3);

      value += value3;
    }
  }

  return value;
}

static double DecodeHoaGainValue(robitbufHandle hBitStream, CoderState* cs, int order)
{
	double gainValue = HOA_MATRIX_LINEAR_GAIN_ZERO;

	int nAlphabet = ((cs->maxGain[order] - cs->minGain[order]) << cs->precisionLevel) + 2;
	int gainValueIndex = ReadRange(hBitStream, nAlphabet);

	gainValue = cs->maxGain[order] - gainValueIndex / (double) (1 << cs->precisionLevel);

	if (gainValue < cs->minGain[order]) {
		gainValue = HOA_MATRIX_LINEAR_GAIN_ZERO;
	} else {
		gainValue = pow(10.0f, gainValue / 20.0f);
	}

	return gainValue;
}



int DecodeHoaMatrix(int inputCount,
                    /*int outputIndex,*/
                    int outputCount,
                    SpeakerInformation* outputConfig,
                    robitbufHandle hBitStream,
                    double* hoaMatrix)                    
{
  int i, j; /*, k;*/
  int maxHoaOrder = sqrt(inputCount)-1;
  int nbitsHoaOrder = getCeilLog2(maxHoaOrder+1);
  int signMatrix[HOA_MATRIX_MAX_HOA_COEF_COUNT * HOA_MATRIX_MAX_SPEAKER_COUNT];
  int gainLimitPerHoaOrder = -1;
  int isFullMatrix = -1;
  int isNormalized = -1;
  int firstSparseOrder = -1;
  int isHoaCoefSparse[HOA_MATRIX_MAX_HOA_COEF_COUNT];
  int hasValue = 1;
  int lfeExist = 0;
  int hasLfeRendering = 0;
  int numPairs = 0;
  int zerothOrderAlwaysPositive = 0;
  int isAllValueSymmetric, isAnyValueSymmetric = 0;
  int isAllSignSymmetric, isAnySignSymmetric =  0;
  int pairIdx;
  int currentHoaOrder = 0;
  int precisionLevel = 0;
  CoderState cs;
  int minGain = 96;
  int maxGain = -96;
  int hasVerticalCoef = 0;
  int symSigns[HOA_MATRIX_MAX_HOA_COEF_COUNT];  
  int vertBitmask[HOA_MATRIX_MAX_HOA_COEF_COUNT]; 
  int valueSymmetricPairs[HOA_MATRIX_MAX_SPEAKER_COUNT/2]; 
  int signSymmetricPairs[HOA_MATRIX_MAX_SPEAKER_COUNT/2]; 
  int isSignSymmetric[HOA_MATRIX_MAX_SPEAKER_COUNT]; 
  int isValueSymmetric[HOA_MATRIX_MAX_SPEAKER_COUNT];
  double currentScalar = 1.0;
  createSymSigns(&symSigns[0], maxHoaOrder);
  create2dBitmask(&vertBitmask[0], maxHoaOrder);
    
  CoderStateInit(&cs, (float) maxGain);
  precisionLevel = robitbuf_ReadBits(hBitStream, 2);
  cs.precisionLevel = precisionLevel;
  /*assert(precisionLevel <= 3);*/
  isNormalized = robitbuf_ReadBits(hBitStream, 1);
  gainLimitPerHoaOrder = robitbuf_ReadBits(hBitStream, 1);
  if (1 == gainLimitPerHoaOrder) {
	  for (i = 0; i<(maxHoaOrder+1); ++i ) {
		  maxGain = -ReadEscapedValue(hBitStream, 3, 5, 6);	
		  minGain = -(ReadEscapedValue(hBitStream, 4, 5, 6) + 1 - maxGain);
		  cs.maxGain[i] = maxGain;
		  cs.minGain[i] = minGain;
	  }
  }
  else
  {
	  maxGain = -ReadEscapedValue(hBitStream, 3, 5, 6);	
	  minGain = -(ReadEscapedValue(hBitStream, 4, 5, 6) + 1- maxGain);
	  for (i = 0; i<(maxHoaOrder+1); ++i ) {
		  cs.maxGain[i] = maxGain;
		  cs.minGain[i] = minGain;
	  }
  }  

  isFullMatrix =  robitbuf_ReadBits(hBitStream, 1);
  if (0 == isFullMatrix) {
	  firstSparseOrder = robitbuf_ReadBits(hBitStream, nbitsHoaOrder);
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
  for (i=0; i< outputCount; ++i){
	  if (outputConfig[i].isLFE)
		  lfeExist = 1;
  }
  if (lfeExist) { hasLfeRendering = robitbuf_ReadBits(hBitStream, 1); }
  numPairs = findSymmetricSpeakers(outputCount, outputConfig, hasLfeRendering);
  for (i=0; i<numPairs; ++i) {
	  valueSymmetricPairs[i] = 0;
	  signSymmetricPairs[i] =  0;
  }
  zerothOrderAlwaysPositive = robitbuf_ReadBits(hBitStream, 1);
  isAllValueSymmetric  = robitbuf_ReadBits(hBitStream, 1);
  if (isAllValueSymmetric) {
	  for (i=0; i<numPairs; ++i) {
		  valueSymmetricPairs[i] = 1;		  
	  }
  }
  else {
	  isAnyValueSymmetric  = robitbuf_ReadBits(hBitStream, 1);
	  if (isAnyValueSymmetric) {
			  for (i=0; i<numPairs; ++i) {
				  valueSymmetricPairs[i] =  robitbuf_ReadBits(hBitStream, 1);
			  }
		  isAnySignSymmetric  = robitbuf_ReadBits(hBitStream, 1);
		  if (isAnySignSymmetric) {			   
			  for (i=0; i<numPairs; ++i) {				  
				  if (0==valueSymmetricPairs[i]){
					  signSymmetricPairs[i] = robitbuf_ReadBits(hBitStream, 1);
				  }
			  }
		  } 
	  }
	  else { /*isAnyValueSymmetric==0*/
		  isAllSignSymmetric  = robitbuf_ReadBits(hBitStream, 1);
		  if (isAllSignSymmetric) {
			  for (i=0; i<numPairs; ++i) {
				  signSymmetricPairs[i] = 1;
			  }
		  }
		  else{
			  isAnySignSymmetric  = robitbuf_ReadBits(hBitStream, 1);
			  if (isAnySignSymmetric) {				  
				  for (i=0; i<numPairs; ++i) {
					  signSymmetricPairs[i] = robitbuf_ReadBits(hBitStream, 1);
				  }
			  }
		  }
	  }
  }

  hasVerticalCoef = robitbuf_ReadBits(hBitStream, 1); 
  cs.rawCodingNonzeros = 1;

 j = 0;
  for (i=0; i<outputCount; ++i) {
	  isValueSymmetric[i] = 0;
	  isSignSymmetric[i] = 0;
	  if ((outputConfig[i].pairType == SP_PAIR_SYMMETRIC) && (outputConfig[i].symmetricPair != NULL)) {
		  if (0==(outputConfig[i].isLFE && (0==hasLfeRendering))) {
			  isValueSymmetric[i] = valueSymmetricPairs[j];
			  isSignSymmetric[i] = signSymmetricPairs[j++];			  
		  }
	  }	
  }


  /* decoding */
  for (i = 0; i < inputCount; ++i) {
	  currentHoaOrder = (int)ceil(sqrt(i+1)-1);
	  for (j = outputCount-1; j >= 0; --j) {
		  signMatrix[i * outputCount + j] = 1;
		  hoaMatrix[i * outputCount + j]  = HOA_MATRIX_LINEAR_GAIN_ZERO;
		  if ((vertBitmask[i] && hasVerticalCoef) || !vertBitmask[i]) {  
			  hasValue = 1;
			  if (0 == isValueSymmetric[j]) {
				  if ((hasLfeRendering && outputConfig[j].isLFE) || (!outputConfig[j].isLFE)) {
					  if (isHoaCoefSparse[i]){
						  hasValue = robitbuf_ReadBits(hBitStream, 1);
					  }
					  if (hasValue) {							 	
						  hoaMatrix[i * outputCount + j] = DecodeHoaGainValue(hBitStream, &cs, currentHoaOrder);	
						  if (0==isSignSymmetric[j]) {
							  if (hoaMatrix[i * outputCount + j] != HOA_MATRIX_LINEAR_GAIN_ZERO) {
								  if (currentHoaOrder || !zerothOrderAlwaysPositive) {
									  signMatrix[i * outputCount + j] = robitbuf_ReadBits(hBitStream, 1) * 2 - 1;									 
								  }			
							  }
						  }
						  else {/*  isSignSymmetric */
							  pairIdx = outputConfig[j].symmetricPair->originalPosition;
							  signMatrix[i * outputCount + j] = symSigns[i] * signMatrix[i * outputCount + pairIdx];							  
						  }
					  }
				  }
			  }
			  else{ /*isValueSymmetric*/
				  pairIdx = outputConfig[j].symmetricPair->originalPosition;			  
				  hoaMatrix[i * outputCount + j] = hoaMatrix[i * outputCount + pairIdx];			  
				  signMatrix[i * outputCount + j] = symSigns[i] * signMatrix[i * outputCount + pairIdx];				  					  
			  }
		  }
	  }
  }

  /* merge the rendering matrix with sign matrix and re-normalize */
  
  for (i = 0; i < inputCount; ++i) {
	  currentHoaOrder = (int)ceil(sqrt(i+1)-1);
	  currentScalar = 1.0f/sqrt(2*currentHoaOrder + 1.0f);
	  for (j = 0; j < outputCount; ++j) {
		   hoaMatrix[i * outputCount + j] *= signMatrix[i * outputCount + j];
		   hoaMatrix[i * outputCount + j] *= currentScalar;
	  }
  }

  if(1==isNormalized) {
	  currentScalar = 0.0;
	  /*compute Frobenius Norm:*/
	  for (i = 0; i < inputCount; ++i) {
		  for (j = 0; j < outputCount; ++j) {
			  if (!outputConfig[j].isLFE)
				  currentScalar += hoaMatrix[i * outputCount + j] * hoaMatrix[i * outputCount + j];
		  }
	  }
	  currentScalar = 1.0/sqrt(currentScalar);
	  /* normalize with inverse Frobenius Norm: */
	  for (i = 0; i < inputCount; ++i) {
		  for (j = 0; j < outputCount; ++j) {
			  if(!outputConfig[j].isLFE)
				  hoaMatrix[i * outputCount + j] *= currentScalar;
		  }
	  }
  }

  return 0;
}
