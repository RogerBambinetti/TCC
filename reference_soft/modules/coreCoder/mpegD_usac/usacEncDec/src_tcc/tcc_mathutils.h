/*
 * tcc_mathutils.h
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

#ifndef MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_MATHUTILS_H_
#define MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_MATHUTILS_H_

double iquant (int dqCoeff, double del, double thr);

void idct( double* buffer, int n );

double sinSynth( double f1, double f2,
				 double a1, double a2,
				 int hopSize,
				 double lastPhaseValue,
				 float* resultInternal);

#endif /* MODULES_CORECODER_MPEGD_USAC_USACENCDEC_SRC_TCC_TCC_MATHUTILS_H_ */
