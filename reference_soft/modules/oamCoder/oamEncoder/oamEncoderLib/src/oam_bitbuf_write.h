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

/**
 * @file oam_bitbuf_write.h
 * @brief Bitstream write operations
 * @author Christian Borss <christian.borss@iis.fraunhofer.de>
 *
 * @date 08.08.2013 - first version
 *
 * @version 0.1
 */

#ifndef  __OAM_BITBUF_WRITE_H__
#define  __OAM_BITBUF_WRITE_H__

#include <stdio.h>
#include "writeonlybitbuf.h"


#define OAM_MAX_PBS_LEN	4096

#define OAMBITBUFWRITE_MAX_FILENAME_LENGTH 1024


/**
 * @brief Structure that holds a pseude bitstream.
 */
typedef struct
{
	int data[OAM_MAX_PBS_LEN];      /**< bitstream values */
	int size[OAM_MAX_PBS_LEN];      /**< bitstream word sizes */
	int num;                        /**< number of elements */
} OamPBS;


#define OAMBITBUFWRITE_MAX_FILENAME_LENGTH 1024


/**
 * @brief Add a value to a pseudo bitstream
 * @param  pbs       pseudo bitstream
 * @param  value     value that shall be added
 * @param  wordsize  word size in bits
 *
 * This function is used to appand a value to a pseudo bitstream.
 *
 *  @date 08.08.2013 - first version
 *
 *  @version 0.1
 */
void oam_bitbuf_add(OamPBS* pbs, int value, int wordsize);

void oam_bitbuf_concat(OamPBS* pbs, OamPBS* pbs2);

/**
 * @brief Write a pseudo bitstream to a file
 * @param  prefix     filename prefix
 * @param  frame_idx  frame index
 * @param  pbs        pseudo bitstream
 *
 * This function is used to write a pseudo bitstream to a file.
 *
 *  @date 08.08.2013 - first version
 *
 *  @version 0.1
 */
void oam_bitbuf_write(const char* prefix, int frame_idx, const OamPBS* pbs);

#endif
