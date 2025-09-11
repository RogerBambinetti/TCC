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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "oam_decode_ldV1.h"
#include "oam_common.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define OAM_MAX_IFRAME_PERIOD 64


/**************************
 *    Static Functions    *
 **************************/

/**
 * @brief Range limitation of OAM data
 * @param x OAM Multidata structure
 */
static void limit_range(StructOamMultidata* x, int hasUniformSpread)
{
	float minval, maxval;
	int n, num_elements;

	num_elements = x->num_elements;

	/* azimuth */
	minval = -180;
	maxval =  180;
	for (n = 0; n < num_elements; n++)
		x->azimuth[n] = MIN(MAX(x->azimuth[n], minval), maxval);

	/* elevation */
	minval = -90;
	maxval =  90;
	for (n = 0; n < num_elements; n++)
		x->elevation[n] = MIN(MAX(x->elevation[n], minval), maxval);

	/* radius */
	minval =  0.5f;
	maxval = 16.f;
	for (n = 0; n < num_elements; n++)
		x->radius[n] = MIN(MAX(x->radius[n], minval), maxval);

	/* gain */
	minval = 0.004f;
	maxval = 5.957f;
	for (n = 0; n < num_elements; n++)
		x->gain[n] = MIN(MAX(x->gain[n], minval), maxval);

	/* spread */
	minval =    0;
	maxval =  180;
	for (n = 0; n < num_elements; n++)
		x->spread[n] = MIN(MAX(x->spread[n], minval), maxval);

	if( !hasUniformSpread ){

		/* spread height */
		minval =    0;
		maxval =   90;
		for (n = 0; n < num_elements; n++)
			x->spread_height[n] = MIN(MAX(x->spread_height[n], minval), maxval);

		/* spread depth */
		minval =    0.0f;
		maxval =   15.5f;
		for (n = 0; n < num_elements; n++)
			x->spread_depth[n] = MIN(MAX(x->spread_depth[n], minval), maxval);
	}

	/* dynamic object priority */
	minval =  0.0f;
	maxval =  7.0f;
	for (n = 0; n < num_elements; n++)
		x->dynamic_object_priority[n] = MIN(MAX(x->dynamic_object_priority[n], minval), maxval);

}

/**
 * @brief Range limitation of OAM data
 * @param x OAM Multidata structure
 */
static void limit_range_singleObject(StructOamMultidata* x, int hasUniformSpread, int n)
{
	float minval, maxval;

	/* azimuth */
	minval = -180;
	maxval =  180;
	x->azimuth[n] = MIN(MAX(x->azimuth[n], minval), maxval);

	/* elevation */
	minval = -90;
	maxval =  90;
	x->elevation[n] = MIN(MAX(x->elevation[n], minval), maxval);

	/* radius */
	minval =  0.5f;
	maxval = 16.f;
	x->radius[n] = MIN(MAX(x->radius[n], minval), maxval);

	/* gain */
	minval = 0.004f;
	maxval = 5.957f;
	x->gain[n] = MIN(MAX(x->gain[n], minval), maxval);

	/* spread */
	minval =    0;
	maxval =  180;
	x->spread[n] = MIN(MAX(x->spread[n], minval), maxval);

	if( !hasUniformSpread ){

		/* spread height */
		minval =    0;
		maxval =   90;
		x->spread_height[n] = MIN(MAX(x->spread_height[n], minval), maxval);

		/* spread depth */
		minval =    0.0f;
		maxval =   15.5f;
		x->spread_depth[n] = MIN(MAX(x->spread_depth[n], minval), maxval);
	}

	/* dynamic object priority */
	minval =  0.0f;
	maxval =  7.0f;
	x->dynamic_object_priority[n] = MIN(MAX(x->dynamic_object_priority[n], minval), maxval);

}

/**
 * @brief Revert the encoder's scaling of the OAM data values
 * @param x OAM Multidata structure
 */
static void descale_multidata(StructOamMultidata* x, int hasUniformSpread)
{
	int n, num_elements;

	num_elements = x->num_elements;

	/* azimuth */
	for (n = 0; n < num_elements; n++)
		x->azimuth[n] = x->azimuth[n] * 1.5f;

	/* elevation */
	for (n = 0; n < num_elements; n++)
		x->elevation[n] = x->elevation[n] * 3.0f;

	/* radius */
	for (n = 0; n < num_elements; n++)
		x->radius[n] = (float) ( pow(2.0, (x->radius[n] / 3.0)) / 2.0);

	/* gain */
	for (n = 0; n < num_elements; n++)
		x->gain[n] = (float)pow(10.0, (x->gain[n] - 32.0) / 40.0);

	/* spread */
	for (n = 0; n < num_elements; n++)
		x->spread[n] = x->spread[n] * 1.5f;

	if( !hasUniformSpread ){
		/* spread height */
		for (n = 0; n < num_elements; n++)
			x->spread_height[n] = x->spread_height[n] * 3.0f;
		/* spread depth */
		for (n = 0; n < num_elements; n++)
			x->spread_depth[n] = (float) ( pow(2.0, (x->spread_depth[n] / 3.0)) / 2.0) - 0.5f;
	}
}

/**
 * @brief Revert the encoder's scaling of the OAM data values
 * @param x OAM Multidata structure
 */
static void descale_multidata_singleObject(StructOamMultidata* x, int hasUniformSpread, int n)
{
	/* azimuth */
  x->azimuth[n] = x->azimuth[n] * 1.5f;

	/* elevation */
  x->elevation[n] = x->elevation[n] * 3.0f;

	/* radius */
	x->radius[n] = (float) ( pow(2.0, (x->radius[n] / 3.0)) / 2.0);

	/* gain */
	x->gain[n] = (float)pow(10.0, (x->gain[n] - 32.0) / 40.0);

	/* spread */
	x->spread[n] = x->spread[n] * 1.5f;

	if( !hasUniformSpread ){
		/* spread height */
		x->spread_height[n] = x->spread_height[n] * 3.0f;
		/* spread depth */
		x->spread_depth[n] = (float) ( pow(2.0, (x->spread_depth[n] / 3.0)) / 2.0) - 0.5f;
	}
}

/**
 * @brief Parse a COAM I-Frame segment (single object)
 * @param  bitstream       bitstream structure; read position is updated (input/output)
 * @param  x               output values (azimuth, elevation, radius, gain, spread)
 * @param  fixed_val       fixed_... flag (azimuth, elevation, radius, gain, spread)
 * @param  flag_absolute   flag_absolute flag
 * @return int             0:success, -1:error
 */
static int single_dynamic_object_data(OamBitbufRead* bitstream,
                                      int* x,
                                      const int* fixed_val,
                                      int flag_absolute,
                                      int hasDynamicObjectPriority,
                                      int hasUniformSpread)
{
	/*default case with all components in use*/
	int wordsize[OAM_NUMBER_COMPONENTS] = {OAM_BITS_AZI, OAM_BITS_ELE, OAM_BITS_RAD, OAM_BITS_GAIN, OAM_BITS_SPREAD_WIDTH, OAM_BITS_SPREAD_HEIGHT, OAM_BITS_SPREAD_DEPTH, OAM_BITS_PRIORITY};
	char sig[OAM_NUMBER_COMPONENTS] = {'s', 's', 'u', 's', 'u', 'u', 'u', 'u'};
	int numUsedComponents = OAM_NUMBER_COMPONENTS;
	int n, num_bits, flag_val, bits;
	int ret = 0;

	/*different possibilities for components to read*/
	if ( !hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 5;
	}
	else if( hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 6;
		wordsize[5] = OAM_BITS_PRIORITY;
	}
	else if ( !hasDynamicObjectPriority && !hasUniformSpread ){
		numUsedComponents = 7;
	}

	if (flag_absolute)
	{
		for (n = 0; n < numUsedComponents; n++)
		{
			if (fixed_val[n] == 0){
				ret |= oam_bitstream_read(bitstream, &x[n], wordsize[n], sig[n]);	/* position_... */
			}
			else{
				x[n] = 0;
			}
		}
	}
	else
	{
		ret |= oam_bitstream_read(bitstream, &num_bits, 3, 'u');					/* num_bits (-2!!!) */
		num_bits += 2;
		for (n = 0; n < numUsedComponents; n++)
		{
			x[n] = 0;
			if (fixed_val[n] == 0)
			{
				ret |= oam_bitstream_read(bitstream, &flag_val, 1, 'u');			/* flag_... */
				if (flag_val)
				{
					if( wordsize[n] == OAM_BITS_AZI ) {
						bits = num_bits;
					}
					else {
						bits = MIN(num_bits, wordsize[n] + 1);
					}
					ret |= oam_bitstream_read(bitstream, &x[n], bits, 's');			/* position_..._difference */
				}
			}
		}
	}

	return ret;
}


/**
 * @brief Parse a COAM I-Frame
 * @param  bitstream    bitstream structure; read position is updated (input/output)
 * @param  x            output values (azimuth, elevation, radius, gain, spread)
 * @param  fixed_val    fixed_... flag (output)
 * @param  num_objects  number of objects
 * @return int          0:success, -1:error
 */
static int get_iframe(OamBitbufRead* bitstream,
                      int* x,
                      int* fixed_val,
                      int num_objects,
                      int hasDynamicObjectPriority,
                      int hasUniformSpread)
{
	/*default case with all components in use*/
	int wordsize[OAM_NUMBER_COMPONENTS] = {OAM_BITS_AZI, OAM_BITS_ELE, OAM_BITS_RAD, OAM_BITS_GAIN, OAM_BITS_SPREAD_WIDTH, OAM_BITS_SPREAD_HEIGHT, OAM_BITS_SPREAD_DEPTH, OAM_BITS_PRIORITY};
	char sig[OAM_NUMBER_COMPONENTS] = {'s', 's', 'u', 's', 'u', 'u', 'u', 'u'};
	int numUsedComponents = OAM_NUMBER_COMPONENTS;
	int common_val;
	int n, n0;
	int ret = 0;

	/*different possibilities for components to read*/
	if ( !hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 5;
	}
	else if( hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 6;
		wordsize[5] = OAM_BITS_PRIORITY;
	}
	else if ( !hasDynamicObjectPriority && !hasUniformSpread ){
		numUsedComponents = 7;
	}

	if (num_objects > 1)
	{
		for (n = 0; n < numUsedComponents; n++)
		{
			if( (n == 5 || n == 6) && !hasUniformSpread ){ /* additional spread components are handled with n == 4 */
				continue;
			}
			ret |= oam_bitstream_read(bitstream, &fixed_val[n], 1, 'u');				/* fixed_... */
			if ( !hasUniformSpread && n == 4){
				fixed_val[n+1] = fixed_val[n];
				fixed_val[n+2] = fixed_val[n];
			}
			if (fixed_val[n])
			{
				ret |= oam_bitstream_read(bitstream, &x[n], wordsize[n], sig[n]);		/* default_... */
				if ( !hasUniformSpread && n == 4){ /* additional spread values */
					ret |= oam_bitstream_read(bitstream, &x[n+1], wordsize[n+1], sig[n+1]);		/* default_... */
					ret |= oam_bitstream_read(bitstream, &x[n+2], wordsize[n+2], sig[n+2]);		/* default_... */
				}
				for (n0 = numUsedComponents; n0 < numUsedComponents * num_objects; n0 += numUsedComponents){
					x[n0+n] = x[n];
					if ( !hasUniformSpread && n == 4){ /* additional spread values */
						x[n0+n+1] = x[n+1];
						x[n0+n+2] = x[n+2];
					}
				}
			}
			else
			{
				ret |= oam_bitstream_read(bitstream, &common_val, 1, 'u');				/* common_... */
				if (common_val)
				{
					ret |= oam_bitstream_read(bitstream, &x[n], wordsize[n], sig[n]);	/* default_... */
					if ( !hasUniformSpread && n == 4){ /* additional spread values */
						ret |= oam_bitstream_read(bitstream, &x[n+1], wordsize[n+1], sig[n+1]);		/* default_... */
						ret |= oam_bitstream_read(bitstream, &x[n+2], wordsize[n+2], sig[n+2]);		/* default_... */
					}
					for (n0 = numUsedComponents; n0 < numUsedComponents * num_objects; n0 += numUsedComponents){
						x[n0+n] = x[n];
						if ( !hasUniformSpread && n == 4){ /* additional spread values */
							x[n0+n+1] = x[n+1];
							x[n0+n+2] = x[n+2];
						}
					}
				}
				else
				{
					for (n0 = 0; n0 < numUsedComponents * num_objects; n0 += numUsedComponents){
						ret |= oam_bitstream_read(bitstream, &x[n0+n], wordsize[n], sig[n]);	/* position_... */
						if ( !hasUniformSpread && n == 4){
							ret |= oam_bitstream_read(bitstream, &x[n0+n+1], wordsize[n+1], sig[n+1]);	/* position_... */
							ret |= oam_bitstream_read(bitstream, &x[n0+n+2], wordsize[n+2], sig[n+2]);	/* position_... */
						}
					}
				
				}
			}
		}
	}
	else
	{
		for (n = 0; n < numUsedComponents; n++)
		{
			ret |= oam_bitstream_read(bitstream, &x[n], wordsize[n], sig[n]);		/* position_... */
			fixed_val[n] = 0;
		}
	}

	return ret;
}


/**
 * @brief Parse a COAM D-Frame
 * @param  bitstream    bitstream structure; read position is updated (input/output)
 * @param  x            output values (azimuth, elevation, radius, gain, spread)
 * @param  fixed_val    fixed_... flag (azimuth, elevation, radius, gain, spread)
 * @param  num_objects  number of objects
 * @return int          0:success, -1:error
 */
static int get_dframe(OamBitbufRead* bitstream,
                      int* x,
                      const int* fixed_val,
                      int num_objects,
                      int hasDynamicObjectPriority,
                      int hasUniformSpread)
{
	int flag_absolute, has_object_data;
	int obj, n;
	int buf[OAM_NUMBER_COMPONENTS];
	int n0 = 0;
	int ret = 0;

	int numUsedComponents = 8;

	/*different possibilities for components to read*/
	if ( !hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 5;
	}
	else if( hasDynamicObjectPriority && hasUniformSpread ){
		numUsedComponents = 6;
	}
	else if ( !hasDynamicObjectPriority && !hasUniformSpread ){
		numUsedComponents = 7;
	}

	ret |= oam_bitstream_read(bitstream, &flag_absolute, 1, 'u');			/* flag_absolute */
	for (obj = 1; obj <= num_objects; obj++)
	{
		ret |= oam_bitstream_read(bitstream, &has_object_data, 1, 'u');		/* has_object_data */
		if (has_object_data)
		{
			if (flag_absolute)
			{
				ret |= single_dynamic_object_data(bitstream, &x[n0], fixed_val, flag_absolute, hasDynamicObjectPriority, hasUniformSpread);
			}
			else	/* DPCM */
			{
				ret |= single_dynamic_object_data(bitstream, buf, fixed_val, flag_absolute, hasDynamicObjectPriority, hasUniformSpread);
				for (n = 0; n < numUsedComponents; n++)
					x[n0+n] += buf[n];
			}
		}
		n0 += numUsedComponents;
	}

	if (ret)
	{
		fprintf(stderr, "Error in get_dframe(): invalid bit stream syntax.\n");
	}

	return ret;
}


/**
 * @brief Add decoded COAM frame to OAM file.
 * @param  chunk        frame data structure, pre-allocated by parent (input/output)
 * @param  file         OAM file descriptor
 * @param  x            decoded COAM frame
 * @param  num_objects  number of objects
 * @param  blocksize    sampling period of the object metadata in audio samples
 */
static void write_chunk(StructOamMultidata* chunk,
                        const int *x,
                        int num_objects,
                        int blocksize,
                        const unsigned int hasDynamicObjectPriority,
                        const unsigned int hasUniformSpread)
{
	int k = 0;
	int n;

	for (n = 0; n < num_objects; n++)
	{
		chunk->azimuth[n]   = (float)x[k++];
		chunk->elevation[n] = (float)x[k++];
		chunk->radius[n]    = (float)x[k++];
		chunk->gain[n]      = (float)x[k++];
		if ( hasUniformSpread ){
			chunk->spread[n]    = (float)x[k++];
		}else{
			chunk->spread[n]    = (float)x[k++];
			chunk->spread_height[n]  = (float)x[k++];
			chunk->spread_depth[n]  = (float)x[k++];
		}
		chunk->dynamic_object_priority[n]  =  hasDynamicObjectPriority ? (float)x[k++] : 0;
	}

	/* Revert the encoder's scaling of the OAM data values */
	descale_multidata(chunk, hasUniformSpread);

	/* clip over-shooting values */
	limit_range(chunk, hasUniformSpread);

	/* increase sampling positions */
	for (n = 0; n < num_objects; n++)
		chunk->sample[n] += blocksize;

}

/**
 * @brief Print an error message and exit with exit code -1.
 * @param  msg  error message
 */
static void fatal_error(const char* msg)
{
	fprintf(stderr, "%s", msg);
	exit(-1);
}


/******************************
 *    Non-Static Functions    *
 ******************************/

void decode_ld_frame(int num_objects,
                     int blocksize,
                     unsigned int hasDynamicObjectPriority,
                     unsigned int hasUniformSpread,
                     int* fixed_val,
                     int* x,
                     OamBitbufRead* bs,
                     StructOamMultidata* chunk)
{
	int has_intracoded_object_data;

	/* parse I-Frame / D-Frame */
	if (oam_bitstream_read(bs, &has_intracoded_object_data, 1, 'u'))			/* has_intracoded_object_data */
		fatal_error("Error in oam_decode_ldV1(): invalid bit stream syntax.\n");
	if (has_intracoded_object_data)
	{
		if (get_iframe(bs, x, fixed_val, num_objects, hasDynamicObjectPriority, hasUniformSpread))
			fatal_error("Error in oam_decode_ldV1(): can't decode I-Frame.\n");
	}
	else
	{
		if (get_dframe(bs, x, fixed_val, num_objects, hasDynamicObjectPriority, hasUniformSpread))
			fatal_error("Error in oam_decode_ldV1(): can't decode D-Frame.\n");
	}

	
	/* add decoded COAM frame to OAM file */
	write_chunk(chunk, x, num_objects, blocksize, hasDynamicObjectPriority, hasUniformSpread);


}


