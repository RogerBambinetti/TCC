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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "oam_common.h"


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

int oam_mat2D(int dim1, int n1, int n2)
{
	return (n1 - 1) + dim1 * (n2 - 1);
}


int oam_mat2D_pol(int obj, int cp, int components)
{
	return components * (obj-1) + (cp-1);
}


StructOamMultidata* oam_multidata_create(int size1, int size2)
{
	StructOamMultidata* ret = (StructOamMultidata*)calloc(1, sizeof(StructOamMultidata));
	int num_elements = size1 * size2;

	ret->sample    = calloc(num_elements, sizeof(uint64_t));
	ret->azimuth   = calloc(num_elements, sizeof(float));
	ret->elevation = calloc(num_elements, sizeof(float));
	ret->radius    = calloc(num_elements, sizeof(float));
	ret->gain      = calloc(num_elements, sizeof(float));
	ret->spread    = calloc(num_elements, sizeof(float));
	ret->spread_height    = calloc(num_elements, sizeof(float));
	ret->spread_depth     = calloc(num_elements, sizeof(float));
	ret->dynamic_object_priority = calloc(num_elements, sizeof(float));

	if (ret->sample    == NULL ||
		ret->azimuth   == NULL ||
		ret->elevation == NULL ||
		ret->radius    == NULL ||
		ret->gain      == NULL ||
		ret->spread    == NULL ||
		ret->spread_height    == NULL ||
		ret->spread_depth    == NULL ||
		ret->dynamic_object_priority == NULL)
	{
		perror("oam_multidata_create()");
		exit(-1);
	}
	ret->size1 = size1;
	ret->size2 = size2;
	ret->num_elements = num_elements;

	return ret;
}


StructOamMultidata* oam_multidata_setempty(StructOamMultidata* multidata)
{
	if (multidata == NULL)
		return NULL;

	if (multidata->sample != NULL)
	{
		free(multidata->sample);
		multidata->sample = NULL;
	}
	if (multidata->azimuth != NULL)
	{
		free(multidata->azimuth);
		multidata->azimuth = NULL;
	}
	if (multidata->elevation != NULL)
	{
		free(multidata->elevation);
		multidata->elevation = NULL;
	}
	if (multidata->radius != NULL)
	{
		free(multidata->radius);
		multidata->radius = NULL;
	}
	if (multidata->gain != NULL)
	{
		free(multidata->gain);
		multidata->gain = NULL;
	}
	if (multidata->spread != NULL)
	{
		free(multidata->spread);
		multidata->spread = NULL;
	}
	if (multidata->spread_height != NULL)
	{
		free(multidata->spread_height);
		multidata->spread_height = NULL;
	}
	if (multidata->spread_depth != NULL)
	{
		free(multidata->spread_depth);
		multidata->spread_depth = NULL;
	}
	if (multidata->dynamic_object_priority != NULL)
	{
		free(multidata->dynamic_object_priority);
		multidata->dynamic_object_priority = NULL;
	}

	multidata->num_elements = 0;
	multidata->size1 = 0;
	multidata->size2 = 0;

	return multidata;
}


StructOamMultidata* oam_multidata_destroy(StructOamMultidata* multidata)
{
	multidata = oam_multidata_setempty(multidata);
	free(multidata);
	return NULL;
}


int oam_multidata_realloc(StructOamMultidata* multidata, int size1, int size2)
{
	int num_elements = size1 * size2;
	int size, size_int;

	if (size1 != multidata->size1)
	{
		fprintf(stderr, "Error: matrix dimension mismatch in oam_multidata_realloc()\n");
		return -1;
	}

	size     = num_elements * sizeof(float);
	size_int = num_elements * sizeof(uint64_t);
	multidata->sample    = (uint64_t*)realloc(multidata->sample, size_int);
	multidata->azimuth   = (float*)realloc(multidata->azimuth,   size);
	multidata->elevation = (float*)realloc(multidata->elevation, size);
	multidata->radius    = (float*)realloc(multidata->radius,    size);
	multidata->gain      = (float*)realloc(multidata->gain,      size);
	multidata->spread    = (float*)realloc(multidata->spread,    size);
	multidata->spread_height    = (float*)realloc(multidata->spread_height, size);
	multidata->spread_depth     = (float*)realloc(multidata->spread_depth,  size);
	multidata->dynamic_object_priority = (float*)realloc(multidata->dynamic_object_priority,    size);

	if (multidata->sample    == NULL ||
		multidata->azimuth   == NULL ||
		multidata->elevation == NULL ||
		multidata->radius    == NULL ||
		multidata->gain      == NULL ||
		multidata->spread    == NULL ||
		multidata->spread_height    == NULL ||
		multidata->spread_depth     == NULL ||
		multidata->dynamic_object_priority == NULL)
	{
		perror("oam_multidata_realloc()");
		exit(-1);
	}
	multidata->size2 = size2;
	multidata->num_elements = num_elements;

	return 0;
}


int oam_multidata_add(StructOamMultidata* y, const StructOamMultidata* x)
{
	int n;

	if (x->size1 != y->size1 || 
		x->size2 != y->size2 )
	{
		fprintf(stderr, "Error: matrix dimension mismatch in oam_multidata_add()\n");
		return -1;
	}

	for (n = 0; n < x->num_elements; n++)
	{
		y->sample[n]    += x->sample[n];
		y->azimuth[n]   += x->azimuth[n];
		y->elevation[n] += x->elevation[n];
		y->radius[n]    += x->radius[n];
		y->gain[n]      += x->gain[n];
		y->spread[n]    += x->spread[n];
		y->spread_height[n] += x->spread_height[n];
		y->spread_depth[n]  += x->spread_depth[n];
		y->dynamic_object_priority[n] += x->dynamic_object_priority[n];
	}

	return 0;
}


static void fatal_error(const char* msg, const char* parg)
{
	fprintf(stderr, "%s", msg);
	perror(parg);
	exit(-1);
}

FILE* oam_write_open(const char* filename, uint16_t num_objects, const unsigned int oam_version, const unsigned int hasDynamicObjectPriority, const unsigned int hasUniformSpread)
{
	FILE* file = NULL;
	char     id[]          = "OAM ";
	uint16_t version       = (uint16_t)oam_version;
	uint16_t tmpHasDynamicObjectPriority = (uint16_t)hasDynamicObjectPriority;
	uint16_t tmpHasUniformSpread = (uint16_t)hasUniformSpread;
	uint16_t num_channels  = 0;
	char     description[] = "Created by oam_write_open()     ";
	char obj_desc[64];
	int obj;

	file = fopen(filename, "wb");
	if (file == NULL)
		fatal_error("Error in oam_write_open().", filename);

	/* write header */
	if (fwrite(&id, sizeof(char), 4, file) != 4)
		fatal_error("Error in oam_write_open().", "fwrite()");
	if (fwrite(&version, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_write_open().", "fwrite()");
	if ( oam_version > 2 )
		if (fwrite(&tmpHasDynamicObjectPriority, sizeof(uint16_t), 1, file) != 1)
			fatal_error("Error in oam_write_open().", "fwrite()");
	if ( oam_version > 3 )
		if (fwrite(&tmpHasUniformSpread, sizeof(uint16_t), 1, file) != 1)
			fatal_error("Error in oam_write_open().", "fwrite()");
	if (fwrite(&num_channels, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_write_open().", "fwrite()");
	if (fwrite(&num_objects, sizeof(uint16_t), 1, file) != 1)
		fatal_error("Error in oam_write_open().", "fwrite()");
	if (fwrite(description, sizeof(char), 32, file) != 32)
		fatal_error("Error in oam_write_open().", "fwrite()");
	for (obj = 0; obj < num_objects; obj++)
	{
		memset(obj_desc, 0, 64);
		sprintf(obj_desc, "Object #%03d", obj);
		if (fwrite(obj_desc, sizeof(char), 64, file) != 64)
			fatal_error("Error in oam_write_open().", "fwrite()");
	}
	return file;
}


void oam_write_process(FILE* file, 
                       const StructOamMultidata* chunk, 
                       const unsigned int oam_version, 
                       const unsigned int hasDynamicObjectPriority, 
                       const unsigned int hasUniformSpread)
{
	float fdata[8];
	uint16_t obj_id;
	int n, num_objects, num_components;

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
			if ( hasDynamicObjectPriority ) {
				num_components = 6;
			}
			else {
				num_components = 5;
			}
			break;
		case 2:
			num_components = 5;
			break;
		case 1:
			num_components = 4;
			break;

		default:
			fatal_error("Error: Invalid OAM version given", "oam_write_process()");
	}

	num_objects = chunk->size1;
	for (n = 0; n < chunk->num_elements; n++)
	{
		obj_id = (uint16_t)(n % num_objects);
		fdata[0] = chunk->azimuth[n];
		fdata[1] = chunk->elevation[n];
		fdata[2] = chunk->radius[n];
		fdata[3] = chunk->gain[n];

		if (oam_version > 1) {
			fdata[4] = chunk->spread[n];
		}
		else {
			fdata[4] = 0.f;
		}

		if (oam_version > 2) {
			fdata[5] = chunk->dynamic_object_priority[n];
		}
		if (oam_version > 3){
			if( num_components == 7 ){ /*no dynamicObjectPriority and no uniformSpread */
				fdata[5] = chunk->spread_height[n];
				fdata[6] = chunk->spread_depth[n];
			}
			else if( num_components == 8 ){ /* dynamicObjectPriority and no uniformSpread */
				fdata[5] = chunk->spread_height[n];
				fdata[6] = chunk->spread_depth[n];
				fdata[7] = chunk->dynamic_object_priority[n];
			}
		}

		if (fwrite(&chunk->sample[n], sizeof(uint64_t), 1, file) != 1)
			fatal_error("Error in oam_write_process().", "fwrite()");
		if (fwrite(&obj_id, sizeof(uint16_t), 1, file) != 1)
			fatal_error("Error in oam_write_process().", "fwrite()");
		if (fwrite(fdata, sizeof(float), num_components, file) != num_components)
			fatal_error("Error in oam_write_process().", "fwrite()");
	}
}


void oam_write_close(FILE* file)
{
	fclose(file);
}