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
 * @file oam_bitbuf_read.h
 * @brief Bitstream read operations
 * @author Christian Borss <christian.borss@iis.fraunhofer.de>
 *
 * @date 18.07.2013 - first version
 *
 * @version 0.1
 */

#ifndef  __OAM_BITBUF_READ_H__
#define  __OAM_BITBUF_READ_H__


#include <stdio.h>
#include "readonlybitbuf.h"


/**
 * @brief Structure that is used to access bitstream files.
 *
 * This data structure combines all metadata that is required to access a
 * bitstream file like the file descriptor, the current read position, and the
 * file size in bits.
 */
typedef struct
{
	FILE* fptr;                 /**< file descriptor */
	int num_bits;               /**< file size in bits */
	int offset;                 /**< current read position */
	unsigned char* data_read;   /**< current read position */
} OamBitbufRead;


#define OAMBITBUFREAD_MAX_FILENAME_LENGTH 1024


/**
 * @brief Function to open a bitstream file
 * @param bitstream structure which contains the file descriptor of the opened file etc.
 * @param prefix the bitstream file prefix
 * @param block_idx data block index
 * @return int 0:success, -1:error
 *
 * This function is used to open a bitream file and to update the
 * OamBitbufRead structure parameter accordingly.
 *
 *  @date 18.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_bitbuf_read_open(OamBitbufRead* bitstream, const char* prefix, int frame_idx);


/**
 * @brief Function to read a value from a bitstream file
 * @param bitstream structure which contains the file descriptor of the opened file etc.
 * @param val read value
 * @param wordsize size in bits to read
 * @return int 0:success, -1:error
 *
 * This function is used to open a bitream file and to update the
 * OamBitbufRead structure parameter accordingly.
 *
 *  @date 18.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_bitbuf_read_process(OamBitbufRead* bitstream, int *val, int wordsize);


/**
 * @brief Function to close a bitstream file
 * @param bitstream structure which contains the file descriptor of the opened file etc.
 *
 * This function is used to close a bitream file and to update the
 * OamBitbufRead structure parameter accordingly.
 *
 *  @date 18.07.2013 - first version
 *
 *  @version 0.1
 */
void oam_bitbuf_read_close(OamBitbufRead* bitstream);


/**
 * @brief Function to read a value from a bitstream file
 * @param bitstream structure which contains the file descriptor of the opened file etc.
 * @param val read value
 * @param wordsize size in bits to read
 * @param sig 's': signed, 'u': unsigned
 * @return int 0:success, -1:error
 *
 * This function is used to open a bitream file and to update the
 * OamBitbufRead structure parameter accordingly.
 *
 *  @date 19.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_bitstream_read(OamBitbufRead* bitstream, int *val, int wordsize, char sig);

#endif

