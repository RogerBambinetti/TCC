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
 * @file oam_decode_rapV4.h
 * @brief RAP-V4 object metadata decoder
 * @author Christian Borss <christian.borss@iis.fraunhofer.de>
 *
 * @date 26.06.2013 - first version
 *
 * @version 0.1
 */


#ifndef  __OAM_DECODE_RAPV4_H__
#define  __OAM_DECODE_RAPV4_H__

#include "oam_bitbuf_read.h"
#include "oam_common.h"


StructOamPolChunk* create_pol_chunk(int num_objects);
StructOamPolChunk* destroy_pol_chunk(StructOamPolChunk* p);


void decode_higheff_frame(int num_objects,
                          int oamBlocksize,
                          int coreBlocksize,
                          unsigned int hasDynamicObjectPriority,
                          unsigned int hasUniformSpread,
                          StructOamPolChunk* pol_iframe,
                          StructOamPolChunk* pol_dframe,
                          StructOamMultidata* chunk_d,
                          int* last,
                          OamBitbufRead* bs,
                          StructOamMultidata* chunk,
                          uint64_t* sample_counter
);


/**
 * @brief Decode compressed object metadata
 * @param  prefix        Filename prefix of the compressed object metadata that
 *                       is written by the core decoder
 * @param  oam_filename  OAM filename
 * @param  num_objects   Number of objects (has to be provided by the core
 *                       decoder)
 * @param  blocksize     Sampling period of the object metadata in audio samples
 *
 * This is the toplevel function of the RAP-V4 object metadata decoder.
 *
 *  @date 29.06.2013 - updated version
 *
 *  @version 0.2
 */


#endif	/* __OAM_DECODE_RAPV4_H__ */
