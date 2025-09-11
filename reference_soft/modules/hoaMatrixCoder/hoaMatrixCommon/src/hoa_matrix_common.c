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

#include "hoa_matrix_common.h"


void CoderStateInit(CoderState* cs, float maxGain)
{ 
	int i;
	for (i=0; i < (HOA_MATRIX_MAX_HOA_ORDER+1);++i) {
		cs->minGain[i] = -1;
		cs->maxGain[i] = maxGain;
	}
	cs->precisionLevel = 0;
	cs->rawCodingNonzeros = 1;
}


int findSymmetricSpeakers(int outputCount, SpeakerInformation* outputConfig, int hasLfeRendering) 
{
	int i, j;
	int numPairs = 0;
	for (i = 0; i < outputCount; ++i) {
		outputConfig[i].originalPosition = (short) i;
		outputConfig[i].isAlreadyUsed = 0;
		outputConfig[i].symmetricPair = NULL;
	}


	for (i = 0; i < outputCount; ++i) {
		if (outputConfig[i].isAlreadyUsed) continue;

		if ((outputConfig[i].azimuth == 0) || (abs(outputConfig[i].azimuth) == 180)) {
			/* the speaker is in the center, it has no pair */    
			outputConfig[i].symmetricPair = NULL;
			outputConfig[i].pairType = SP_PAIR_CENTER;

			outputConfig[i].isAlreadyUsed = 1;
		} else {
			for (j = i + 1; j < outputCount; ++j) {
				if (outputConfig[j].isAlreadyUsed) continue;

				if ((outputConfig[i].isLFE == outputConfig[j].isLFE) &&
					(outputConfig[i].elevation == outputConfig[j].elevation) &&
					(outputConfig[i].azimuth == -outputConfig[j].azimuth)) {

						/* we found a symmetric L/R pair for speaker on position i */
						outputConfig[i].symmetricPair = &(outputConfig[j]);
						outputConfig[i].pairType = SP_PAIR_SYMMETRIC;
						if (0 == (outputConfig[i].isLFE & !hasLfeRendering)){
							numPairs++;
						}
						outputConfig[j].symmetricPair = NULL;
						outputConfig[j].pairType = SP_PAIR_NONE;						

						outputConfig[i].isAlreadyUsed = 1;
						outputConfig[j].isAlreadyUsed = 1;
						break;
				}
			}

			if (!outputConfig[i].isAlreadyUsed) {
				/* we did not found a symmetric L/R pair for speaker on position i */
				outputConfig[i].symmetricPair = NULL;
				outputConfig[i].pairType = SP_PAIR_SINGLE;

				outputConfig[i].isAlreadyUsed = 1;
			}
		}
	}

	for (i = 0; i < outputCount; ++i) {
		outputConfig[i].isAlreadyUsed = 0;
	}
	return numPairs;
}


void createSymSigns(int* symSigns, int hoaOrder)
{
	int n, m, k = 0;	
	for (n = 0; n<=hoaOrder; ++n) {
		for (m = -n; m<=n; ++m) 
			symSigns[k++] = ((m>=0)*2)-1;					
	} 	
}

void create2dBitmask(int* bitmask, int hoaOrder)
{
	int n, m, k = 0;
	bitmask[k++] = 0;
	for (n = 1; n<=hoaOrder; ++n) {
		for (m = -n; m<=n; ++m)  
			bitmask[k++] = abs(m)!=n;					
	} 	
}
