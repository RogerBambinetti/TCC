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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "oam_read.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef DB
#define DB(x) (20.0 * log10((x)))
#endif
#ifndef LOG2
#define LOG2(x) (log((x)) / log(2.0))
#endif
#ifndef ROUND
#define ROUND(x) (((x)<(0))?(int)((x)-0.5f):(int)((x)+0.5f))
#endif

static void fatal_error(const char* msg, const char* parg)
{
	fprintf(stderr, "%s\n", msg);
	perror(parg);
	exit(-1);
}


static void syntax_error(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}

FILE* oam_read_open(const char* filename, uint16_t* num_objects, uint16_t* oam_version, uint16_t* hasDynamicObjectPriority, uint16_t* hasUniformSpread)
{
	char     id[4];
	uint16_t num_channels;
	char     description[32];
	char     obj_desc[64];
	char     chan_filename[64];
	int obj, chan;
	FILE* file = NULL;

	/* open file */
	file = fopen(filename, "rb");
	if (file == NULL)
		fatal_error("Error in oam_read_open().", filename);

	/* OAM Header: id */
	if (fread(&id, sizeof(char), 4, file) != 4)
		fatal_error("Error in oam_read_open().", "fread()");
	id[3] = '\0';
	if (strcmp(id, "OAM") != 0)
		syntax_error("Error in oam_read_open(): invalid OAM header.");

	/* OAM Header: version */
	if (fread(oam_version, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_read_open().", "fread()");
	if (*oam_version > 4)
		syntax_error("Error in oam_read_open(): invalid OAM header.");
	if (*oam_version > 2)
		if (fread(hasDynamicObjectPriority, sizeof(uint16_t), 1, file) != 1)
			syntax_error("Error in oam_read_open(): invalid OAM header.");
	if (*oam_version > 3) {
		if (fread(hasUniformSpread, sizeof(uint16_t), 1, file) != 1)
			syntax_error("Error in oam_read_open(): invalid OAM header.");
	}
	else {
		*hasUniformSpread = 1;
	}

	/* OAM Header: num_channels */
	if (fread(&num_channels, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_read_open().", "fread()");

	/* OAM Header: num_objects */
	if (fread(num_objects, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_read_open().", "fread()");

	/* OAM Header: description */
	if (fread(description, sizeof(char), 32, file) != 32)
		fatal_error("Error in oam_read_open().", "fread()");

	/* OAM Header: channel filenames */
	for (chan = 0; chan < num_channels; chan++)
	{
		if (fread(chan_filename, sizeof(char), 64, file) != 64)
			fatal_error("Error in oam_read_open().", "fread()");
	}

	/* OAM Header: object describtions */
	for (obj = 0; obj < *num_objects; obj++)
	{
		if (fread(obj_desc, sizeof(char), 64, file) != 64)
			fatal_error("Error in oam_read_open().", "fread()");
	}

	return file;
}

void oam_read_process(FILE* file, StructOamMultidata* chunk, int* num, const unsigned int oam_version, const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority, const unsigned int hasUniformSpread)
{
	float fdata[8];
	uint16_t obj_id;
	int n, num_objects;
	size_t num_components;

	switch (oam_version) {
		case 4:
			/*different possibilities for num_components*/
			if ( !hasDynamicObjectPriority && hasUniformSpread ){
				num_components = 5;
			}
			else if( hasDynamicObjectPriority && hasUniformSpread ){
				num_components = 6;
			}
			else if ( !hasDynamicObjectPriority && !hasUniformSpread ){
				num_components = 7;
			}
			else{
				num_components = 8;
			}
			break;
		case 3:
			if ( hasDynamicObjectPriority != DYN_OBJ_PRIO_AVAILABLE ) {
				num_components = 5;
			}
			else {
				num_components = 6;
			}
			break;
		case 2:
			num_components = 5;
			break;
		case 1:
			num_components = 4;
			break;
		default:
			fatal_error("Error: Invalid OAM version in header", "oam_read_process()");
	}

	num_objects = chunk->size1;

	for (n = 0; n < chunk->num_elements; n++)
	{
		if (fread(&chunk->sample[n], sizeof(uint64_t), 1, file) != 1)
			break;
		if (fread(&obj_id, sizeof(uint16_t), 1, file) != 1)
			fatal_error("Error in oam_read_process().", "fread()");
		if ((int)obj_id != (n % num_objects))
			syntax_error("Error in oam_read_process(): out-of-order object id");
		if (fread(fdata, sizeof(float), num_components, file) != num_components)
			fatal_error("Error in oam_read_process().", "fread()");
		chunk->azimuth[n]   = fdata[0];
		chunk->elevation[n] = fdata[1];
		chunk->radius[n]    = fdata[2];
		chunk->gain[n]      = fdata[3];

		if (oam_version > 1) {
			chunk->spread[n] = fdata[4];
		}
		else {
			chunk->spread[n] = 0.f;
		}

		if (oam_version > 2 && hasDynamicObjectPriority == DYN_OBJ_PRIO_AVAILABLE) {
			chunk->dynamic_object_priority[n] = fdata[5];
		}
		else {
			chunk->dynamic_object_priority[n] = 7.f;
		}

		if (oam_version > 3){
			if( num_components == 7 ){ /* no dynamicObjectPriority and no uniformSpread */
				chunk->spread_height[n]  =  fdata[5];
				chunk->spread_depth[n]  =  fdata[6];
			}
			else if( num_components == 8 ){ /* dynamicObjectPriority && no uniformSpread */
				chunk->spread_height[n] = fdata[5];
				chunk->spread_depth[n] = fdata[6];
				chunk->dynamic_object_priority[n] = fdata[7];
			}
			else{
				chunk->spread_height[n] = 0.0f;
				chunk->spread_depth[n] = 0.0f;
			}
		}
		if( ADD_NONUNIFORMSPREAD ){
			chunk->spread_height[n] = fdata[1]; /*10.0f+ 5.0f*(float)(n%2);*/
			chunk->spread_depth[n]  = fdata[2]; /*0.0f + 8.0f*(float)(n%2);*/
		}
	}
	*num = n;
}

/**
 * @brief range limitation
 * @param   x      input value
 * @param   bits   number of bits
 * @return  float  output value
 *
 * This function limits a value to the range that can be represented by an
 * unsigned integer of a given word size.
 */
static float bit_limit_unsigned(float x, int bits)
{
	float minval = 0.0f;
	float maxval = (float)((1<<bits) - 1);
	float y = MIN(MAX(x, minval), maxval);

	return y;
}


/**
 * @brief range limitation
 * @param   x      input value
 * @param   bits   number of bits
 * @return  float  output value
 *
 * This function limits a value to the range that can be represented by a
 * signed integer of a given word size.
 */
static float bit_limit_signed(float x, int bits)
{
	float minval = - (float)(1<<(bits-1));
	float maxval = (float)((1<<bits) - 1) + minval;
	float y = MIN(MAX(x, minval), maxval);

	return y;
}

/**
 * @brief Range limitation of scaled OAM data
 * @param x OAM Multidata structure
 */
static void integer_limit(StructOamMultidata* x)
{
	int bits;
	int n, num_elements;

	num_elements = x->num_elements;

	/* azimuth */
	bits = OAM_BITS_AZI;
	for (n = 0; n < num_elements; n++)
		x->azimuth[n] = bit_limit_signed(x->azimuth[n], bits);

	/* elevation */
	bits = OAM_BITS_ELE;
	for (n = 0; n < num_elements; n++)
		x->elevation[n] = bit_limit_signed(x->elevation[n], bits);

	/* radius */
	bits = OAM_BITS_RAD;
	for (n = 0; n < num_elements; n++)
		x->radius[n] = bit_limit_unsigned(x->radius[n], bits);

	/* gain */
	bits = OAM_BITS_GAIN;
	for (n = 0; n < num_elements; n++)
		x->gain[n] = bit_limit_signed(x->gain[n], bits);

	/* spread */
	bits = OAM_BITS_SPREAD;
	for (n = 0; n < num_elements; n++)
		x->spread[n] = bit_limit_unsigned(x->spread[n], bits);

	/* spread height*/
	bits = OAM_BITS_SPREAD_HEIGHT;
	for (n = 0; n < num_elements; n++)
		x->spread_height[n] = bit_limit_unsigned(x->spread_height[n], bits);

	/* spread depth*/
	bits = OAM_BITS_SPREAD_DEPTH;
	for (n = 0; n < num_elements; n++)
		x->spread_depth[n] = bit_limit_unsigned(x->spread_depth[n], bits);

	/* dynamic object priority */
	bits = OAM_BITS_PRIORITY;
	for (n = 0; n < num_elements; n++)
		x->dynamic_object_priority[n] = bit_limit_unsigned(x->dynamic_object_priority[n], bits);

}


/**
 * @brief Scale the OAM data values
 * @param x OAM Multidata structure
 */
static void scale_multidata(StructOamMultidata* x, int hasUniformSpread)
{
	int n, num_elements;

	num_elements = x->num_elements;

	/* azimuth: -180..180 */
	for (n = 0; n < num_elements; n++)
	{
		while (x->azimuth[n] <= 180.0f)
			x->azimuth[n] += 360.0f;
		while (x->azimuth[n] > 180.0f)
			x->azimuth[n] -= 360.0f;
	}

	/* azimuth */
	for (n = 0; n < num_elements; n++)
		x->azimuth[n] = x->azimuth[n] / 1.5f;

	/* elevation */
	for (n = 0; n < num_elements; n++)
		x->elevation[n] = x->elevation[n] / 3.0f;

	/* radius */
	for (n = 0; n < num_elements; n++)
		x->radius[n] = 3.0f * (float)LOG2(2.0 * x->radius[n]);   /* r0 = 2.^(-4/3:1/3:4) */

	/* gain */
	for (n = 0; n < num_elements; n++)
		x->gain[n] = 2.0f * (float)DB(x->gain[n]) + 32.0f;

	/* spread */
	for (n = 0; n < num_elements; n++)
		x->spread[n] = x->spread[n] / 1.5f;

	if( !hasUniformSpread ){
		/* spread height */
		for (n = 0; n < num_elements; n++)
			x->spread_height[n] = x->spread_height[n] / 3.0f;
		/* spread depth */
		for (n = 0; n < num_elements; n++)
			x->spread_depth[n] = 3.0f * (float)LOG2(2.0 * x->spread_depth[n] + 0.5f);
	}
	/* limit value range */
	integer_limit(x);
}


/**
 * @brief Sub-sample OAM data values
 * @param  x           OAM Multidata structure
 * @param  sub_sample  sub-sampling factor
 */
static void subsample_multidata(StructOamMultidata* x, int sub_sample)
{
	int num_objects, num_samples;
	int kin, kout;
	int k, obj;

	if (sub_sample == 1)
		return;

	num_objects = x->size1;
	num_samples = x->size2 / sub_sample;

	for (obj = 1; obj <= num_objects; obj++)
	{
		for (k = 0; k < num_samples; k++)
		{
			kin  = oam_mat2D(num_objects, obj, 1 + k * sub_sample);
			kout = oam_mat2D(num_objects, obj, 1 + k);
			x->azimuth[kout]   = x->azimuth[kin];
			x->elevation[kout] = x->elevation[kin];
			x->radius[kout]    = x->radius[kin];
			x->gain[kout]      = x->gain[kin];
			x->spread[kout]    = x->spread[kin];
			x->spread_height[kout] = x->spread_height[kin];
			x->spread_depth[kout]  = x->spread_depth[kin];
			x->dynamic_object_priority[kout]  = x->dynamic_object_priority[kin];
			x->sample[kout]    = x->sample[kin];
		}
	}

	x->size2        = num_samples;
	x->num_elements = x->size1 * x->size2;
}

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
                            int hasUniformSpread)
{
	float default_radius = 100.0f;
	int n, num;

	/* set chunk size to the requested size */
	chunk->size2 = num_samples * sub_sample;
	chunk->num_elements = chunk->size1 * chunk->size2;

	/* read OAM data chunk */
	oam_read_process(file, chunk, &num, oam_version, hasDynamicObjectPriority, hasUniformSpread);

	/* sub-sampling */
	subsample_multidata(chunk, sub_sample);

	/* replace "position_radius" values by a default value */
	if (replace_radius)
	{
		for (n = 0; n < num; n++)
			chunk->radius[n] = default_radius;
	}

	/* set chunk size that we actually got */
	chunk->size2 = num / chunk->size1;
	if (sub_sample > 1)
	{
		chunk->size2 = (chunk->size2 + sub_sample - 1) / sub_sample;
		num /= sub_sample;
	}
	chunk->num_elements = chunk->size1 * chunk->size2;

	/* scale OAM data chunk */
	scale_multidata(chunk, hasUniformSpread);

	return chunk->size2;
}

void oam_read_close(FILE* file)
{
	fclose(file);
}
