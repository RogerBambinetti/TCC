/*
 * tcc_struct.h
 *
 *  This files contains config struct for TCC decoder
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

#ifndef MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_STRUCT_H_
#define MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_STRUCT_H_

#include "../src_frame/obj_descr.h"           /* structs */
#include "../src_frame/usac_config.h"

#define TCC_MAX_COEFFICIENT_NUMBER 32
#define TCC_MAX_SEGMENT_LENGTH     32

/* Bit Streams defines */
#define BS_TCC_PRESENT     1

#define BS_TCC_NO_TRJ         3
#define BS_TCC_TRJ_ISCONT     1
#define BS_TCC_TRJ_LEN        2
#define BS_TCC_AMP_QUANT_FLAG 1
#define BS_TCC_AMP_DC 		  8
#define BS_TCC_AMP_COEFF      7
#define BS_TCC_AMP_INDEX      5
#define BS_TCC_FREQ_COEFF     7
#define BS_TCC_FREQ_INDEX     5
#define BS_TCC_FREQ_DC 		  11

#define BS_TCC_AMP_DC_OFFSET 32
#define BS_TCC_FREQ_DC_OFFSET 600


typedef enum {
	TCC_MODE_CPE_NOT_CPE                 = -1,
	TCC_MODE_CPE_NOT_PRESENT             = 0,
	TCC_MODE_CPE_ONE_FRAME_PER_ELEMENT   = 1,
	TCC_MODE_CPE_TWO_FRAMES_PER_ELEMENT  = 2
} TCC_CPE_MODE;


typedef struct {

	/* data from bitstream */
	int outFrameLenght;
	int samplingRate;

	int           channelMap[ USAC_MAX_NUM_OUT_CHANNELS ];
	int           stereoConfigIndex[ USAC_MAX_NUM_OUT_CHANNELS ];
	TCC_CPE_MODE  tccMode[USAC_MAX_NUM_OUT_CHANNELS];
	int           numAffectedChannel;

} TCC_CONFIG;


#endif /* MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_STRUCT_H_ */
