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

#ifndef __HOA_MATRIX_DEC_H__
#define __HOA_MATRIX_DEC_H__

#include "hoa_matrix_common.h"
#include "readonlybitbuf.h"


int DecodeHoaMatrix(int inputCount,
                    int outputCount,
                    SpeakerInformation* outputConfig,
                    robitbufHandle hBitStream,
                    double* hoaMatrix);  

int ReadEscapedValue(robitbufHandle hBitStream,
                     int count1,
                     int count2,
                     int count3);

#endif /* __HOA_MATRIX_DEC_H__ */
