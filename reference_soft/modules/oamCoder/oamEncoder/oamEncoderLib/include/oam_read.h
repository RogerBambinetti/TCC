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
 * @file oam_read.h
 * @brief Read OAM file
 * @author Christian Borss <christian.borss@iis.fraunhofer.de>
 *
 * @date 08.08.2013 - first version
 *
 * @version 0.1
 */


#ifndef  __OAM_READ_H__
#define  __OAM_READ_H__


#include <stdio.h>
#include "oam_common.h"


/**
 * @brief Open an OAM file for reading and parse the OAM header.
 * @param  filename     OAM filename
 * @param  num_objects  number of objects
 * @return FILE*        open file descriptor
 *
 * This function opens an OAM file for reading and parses the OAM header.
 *
 *  @date 08.08.2013 - first version
 *
 *  @version 0.1
 */
FILE* oam_read_open(const char* filename, uint16_t* num_objects, uint16_t* oam_version, uint16_t* hasDynamicObjectPriority, uint16_t* hasUniformSpread);


/**
 * @brief Read an OAM data chunk
 * @param  file   OAM file descriptor
 * @param  chunk  OAM data chunk
 * @param  num    number of read elements; should be equal to chunk->num_elements
 *
 * This function reads a data chunk from an OAM file.
 *
 *  @date 08.08.2013 - first version
 *
 *  @version 0.1
 */
void oam_read_process(FILE* file, StructOamMultidata* chunk, int* num, const unsigned int oam_version, const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority, const unsigned int hasUniformSpread);

/**
 * @brief Close OAM file.
 * @param  file  OAM file descriptor
 *
 * This function closes the given OAM file descriptor.
 *
 *  @date 08.08.2013 - first version
 *
 *  @version 0.1
 */
void oam_read_close(FILE* file);

/**
 * @brief Read an OAM data chunk and scale it.
 * @param  chunk           OAM data chunk
 * @param  file            OAM file descriptor
 * @param  num_samples     number of samples to read
 * @param  replace_radius  replace radius flag
 */
int get_scaled_chunk(StructOamMultidata* chunk,
                            FILE* file,
                            int num_samples,
                            int replace_radius,
                            int sub_sample,
                            int oam_version,
                            DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority,
                            int hasUniformSpread);


#endif	/* __OAM_READ_H__ */
