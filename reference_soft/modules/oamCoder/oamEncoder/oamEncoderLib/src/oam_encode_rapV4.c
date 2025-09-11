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
#include "oam_encode_rapV4.h"
#include "oam_bitbuf_write.h"
#include "oam_common.h"
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


/**************************
 *    Static Functions    *
 **************************/

/**
 * @brief Inverse polygon transformation via linear interpolation
 * @param y_out    interpolation result
 * @param num_in   number of polygon points
 * @param x_in     polygon x-values
 * @param y_in     polygon y-values
 * @param num_out  number of interpolation points
 * @param x_out    x-values of the interpolation points
 *
 * The given array y_out must be large enough to hold num_out elements.
 * The x_in values must be in ascending order.
 * Positions outside the range of x_in will be extrapolated.
 */
static void interpol(float* y_out,
                     int num_in,
                     const float* x_in,
                     const float* y_in,
                     int num_out,
                     const float* x_out)
{
	int nin, nout;
	float alpha;

	nin = 0;
	for (nout = 0; nout < num_out; nout++)
	{
		while(nin+2 < num_in && x_in[nin+1] < x_out[nout])
			nin++;
		while(x_out[nout] < x_in[nin] && nin > 0)
			nin--;
		alpha = (x_out[nout] - x_in[nin]) / (x_in[nin+1] - x_in[nin]);
		y_out[nout] = (1-alpha) * y_in[nin] + alpha * y_in[nin+1];
	}
}


/**
 * @brief Allocate memory for a frame polygon data structure.
 * @param  num_objects         number of objects
 * @return StructOamPolChunk*  generated frame polygon data structure
 */
StructOamPolChunk* create_pol_chunk(int num_objects)
{
	int n;
	StructOamPolChunk* p = NULL;

	p = (StructOamPolChunk*)malloc(sizeof(StructOamPolChunk));
	p->pol          = (StructOamPolygon*)malloc(OAM_NUMBER_COMPONENTS * num_objects * sizeof(StructOamPolygon));
	p->num_objects  = num_objects;
	p->num_elements = OAM_NUMBER_COMPONENTS * num_objects;

	for (n = 0; n < p->num_elements; n++)
	{
		p->pol[n].pos     = NULL;
		p->pol[n].value   = NULL;
		p->pol[n].num     = 0;
		p->pol[n].bufsize = 0;
	}

	return p;
}


/**
 * @brief Deallocate memory for a frame polygon data structure.
 * @param  p                   frame polygon data structure
 * @return StructOamPolChunk*  NULL
 */
StructOamPolChunk* destroy_pol_chunk(StructOamPolChunk* p)
{
	int n;

	if (p == NULL)
		return NULL;

	for (n = 0; n < p->num_elements; n++)
	{
		free(p->pol[n].pos);
		free(p->pol[n].value);
	}
	free(p->pol);
	free(p);
	p = NULL;

	return p;
}


/**
 * @brief Memory size adjustment for a COAM frame structure.
 * @param  p           COAM frame structure
 * @param  num_points  number of polygon points
 * @return int         0: no error; -1: error
 *
 * This function checks the internal memory size of a COAM frame structure and
 * enlarges it, if necessary, to hold up to num_points polygon points.
 */
int realloc_pol_chunk(StructOamPolChunk* p, int num_points)
{
	int n, size;

	if (p == NULL || p->num_elements < 1)
		return 0;

	/* large enough? */
	if (p->pol[0].bufsize >= num_points)
		return 0;

	size = num_points * sizeof(float);
	for (n = 0; n < p->num_elements; n++)
	{
		p->pol[n].pos   = (float*)realloc(p->pol[n].pos,   size);
		p->pol[n].value = (float*)realloc(p->pol[n].value, size);
		if (p->pol[n].pos == NULL || p->pol[n].value == NULL)
		{
			perror("oam_pol_realloc()");
			exit(-1);
		}
		p->pol[n].bufsize = num_points;
	}

	return 0;
}


/**
 * @brief Get I-Frame data chunk via linear interpolation.
 * @param  chunk          result of the inverse polygon transformation
 * @param  p              I-Frame/D-Frame polygon content
 * @param  iframe_period  I-Frame period / chunk length
 */
static void get_iframe_chunk(StructOamMultidata* chunk,
                             const StructOamMultidata* first,
                             const StructOamMultidata* last,
                             int iframe_period)
{
	int k, obj, cp, kout;
	float n_x[OAM_MAX_IFRAME_PERIOD];
	float n[2];
	float y[2];
	float y_out[OAM_NUMBER_COMPONENTS][OAM_MAX_IFRAME_PERIOD];
	int num_objects;

	for (k = 0; k < iframe_period; k++)
		n_x[k] = (float)(k+2);

	n[0] = 1.0f;
	n[1] = 1.0f + (float)iframe_period;
	num_objects = chunk->size1;
	for (obj = 1; obj <= num_objects; obj++)
	{
		for (cp = 1; cp <= OAM_NUMBER_COMPONENTS; cp++)
		{
			switch (cp)
			{
				case 1:
					y[0] = first->azimuth[obj-1];
					y[1] = last->azimuth[obj-1];
					break;
				case 2:
					y[0] = first->elevation[obj-1];
					y[1] = last->elevation[obj-1];
					break;
				case 3:
					y[0] = first->radius[obj-1];
					y[1] = last->radius[obj-1];
					break;
				case 4:
					y[0] = first->gain[obj-1];
					y[1] = last->gain[obj-1];
					break;
				case 5:
					y[0] = first->spread[obj-1];
					y[1] = last->spread[obj-1];
					break;
				case 6:
					y[0] = first->spread_height[obj-1];
					y[1] = last->spread_height[obj-1];
					break;
				case 7:
					y[0] = first->spread_depth[obj-1];
					y[1] = last->spread_depth[obj-1];
					break;
				case 8:
					y[0] = first->dynamic_object_priority[obj-1];
					y[1] = last->dynamic_object_priority[obj-1];
					break;
			}
			interpol(y_out[cp-1], 2, n, y, iframe_period, n_x);
		}
		for (k = 0; k < iframe_period; k++)
		{
			kout = oam_mat2D(num_objects, obj, k+1);
			chunk->azimuth[kout]   = y_out[0][k];
			chunk->elevation[kout] = y_out[1][k];
			chunk->radius[kout]    = y_out[2][k];
			chunk->gain[kout]      = y_out[3][k];
			chunk->spread[kout]    = y_out[4][k];
			chunk->spread_height[kout]   = y_out[5][k];
			chunk->spread_depth[kout]    = y_out[6][k];
			chunk->dynamic_object_priority[kout]  = y_out[7][k];
			chunk->sample[kout]    = 0;
		}
	}
}


/**
 * @brief Repeat the first sample of each object
 * @param  chunk        OAM data chunk
 * @param  num_samples  number repeated samples
 * @param  last         last sample values of the previous datachunk (output)
 */
void repeat_chunk(StructOamMultidata* chunk, int num_samples, int iframe_period, StructOamMultidata* last)
{
	int n, n0, dest, bytes;
	int num_objects;

	num_objects = chunk->size1;
	dest  = num_objects;
	bytes = num_objects * sizeof(float);

  if(num_samples > iframe_period){
    fprintf(stderr, "Too many dummy_frames for iframe_period.");
    exit(-1);
  }

	/* repeat the first sample of each object */
	for (n = 1; n < num_samples; n++)
	{
		memcpy(&chunk->azimuth[dest],   chunk->azimuth,   bytes);
		memcpy(&chunk->elevation[dest], chunk->elevation, bytes);
		memcpy(&chunk->radius[dest],    chunk->radius,    bytes);
		memcpy(&chunk->gain[dest],      chunk->gain,      bytes);
		memcpy(&chunk->spread[dest],    chunk->spread,    bytes);
		memcpy(&chunk->spread_height[dest],    chunk->spread_height,   bytes);
		memcpy(&chunk->spread_depth[dest],     chunk->spread_depth,    bytes);
		memcpy(&chunk->dynamic_object_priority[dest],  chunk->dynamic_object_priority,  bytes);
		memcpy(&chunk->sample[dest],    chunk->sample,    bytes);
		dest += num_objects;
	}

	/* set chunk size to the requested size */
	chunk->size2 = num_samples;
	chunk->num_elements = chunk->size1 * chunk->size2;

	/* set "last" values */
	n0 = num_objects * (num_samples - 1);
	for (n = 0; n < num_objects; n++)
	{
		last->azimuth[n]   = (float)(ROUND(chunk->azimuth[n0 + n]));
		last->elevation[n] = (float)(ROUND(chunk->elevation[n0 + n]));
		last->radius[n]    = (float)(ROUND(chunk->radius[n0 + n]));
		last->gain[n]      = (float)(ROUND(chunk->gain[n0 + n]));
		last->spread[n]    = (float)(ROUND(chunk->spread[n0 + n]));
		last->spread_height[n]    = (float)(ROUND(chunk->spread_height[n0 + n]));
		last->spread_depth[n]     = (float)(ROUND(chunk->spread_depth[n0 + n]));
		last->dynamic_object_priority[n]  = (float)(ROUND(chunk->dynamic_object_priority[n0 + n]));
	}
}


/**
 * @brief Polygon course approximation
 * @param  y_out    approximation result (output)
 * @param  num_in   number of polygon points (input/output)
 * @param  x_in     x-values of the polygon course (input/output)
 * @param  y_in     y-values of the polygon course (input/output)
 * @param  num_out  number of target points (input)
 * @param  x_out    x-values of the target function (input)
 * @param  target   y-values of the target function (input)
 */
static void pol_approx(float* y_out,
                       int* num_in,
                       float* x_in,
                       float* y_in,
                       int num_out,
                       const float* x_out,
					   const float* target)
{
	float max_delta = (float)(2.0 / 3.0);	/* approximation accuracy */
	float diff_max = 0.0f;
	float diff;
	int nmax = 0;
	int n;

	/* linear interpolation */
	interpol(y_out, *num_in, x_in, y_in, num_out, x_out);

	/* find maximum deviation */
	for (n = 0; n < num_out; n++)
	{
		diff = (float)fabs(target[n] - y_out[n]);
		if (diff > diff_max)
		{
			diff_max = diff;
			nmax = n;
		}
	}

	/* abort criterion */
	if (diff_max <= max_delta)
		return;

	/* insert new polygon point */
	for (n = *num_in - 1; n >= 0 && x_in[n] > x_out[nmax]; n--)
	{
		x_in[n+1] = x_in[n];
		y_in[n+1] = y_in[n];
	}
	x_in[n+1] = x_out[nmax];
	y_in[n+1] = (float)(ROUND(target[nmax]));
	(*num_in)++;

	/* recursion: first half */
	pol_approx(y_out, num_in, x_in, y_in, nmax+1, x_out, target);

	/* recursion: second half */
	pol_approx(&y_out[nmax], num_in, x_in, y_in, num_out - nmax, &x_out[nmax], &target[nmax]);
}


/**
 * @brief Approximate a D-Frame data chunk by polygon courses
 * @param  p      approximated polygon courses
 * @param  chunk  scaled OAM data chunk
 */
static void dchunk2pol(StructOamPolChunk* p, const StructOamMultidata* chunk, int numUsedComponents)
{
	StructOamPolygon* pol;
	float* src;
	float y_out[OAM_MAX_IFRAME_PERIOD + 1];
	int num_in;
	float x_in[OAM_MAX_IFRAME_PERIOD + 1];
	float y_in[OAM_MAX_IFRAME_PERIOD + 1];
	int num_out;
	float x_out[OAM_MAX_IFRAME_PERIOD + 1];
	float target[OAM_MAX_IFRAME_PERIOD + 1];
	int obj, cp;
	int k, kin;
	int num_objects, iframe_period;

	/* interpolation positions */
	num_objects   = chunk->size1;
	iframe_period = chunk->size2;
	num_out = iframe_period + 1;
	for (k = 0; k < num_out; k++)
		x_out[k] = (float)(k+1);

	for (obj = 1; obj <= num_objects; obj++)
	{
		for (cp = 1; cp <= numUsedComponents; cp++)
		{
			/* set approximation target */
			switch (cp)
			{
				case 1:
					src = &( chunk->azimuth[obj-1] );
					break;
				case 2:
					src = &( chunk->elevation[obj-1] );
					break;
				case 3:
					src = &( chunk->radius[obj-1] );
					break;
				case 4:
					src = &( chunk->gain[obj-1] );
					break;
				case 5:
					src = &( chunk->spread[obj-1] );
					break;
			}
			if ( numUsedComponents == 6 && cp == 6 ){
				src = &( chunk->dynamic_object_priority[obj-1] );
			}
			if ( numUsedComponents == 7 ){
				switch (cp)
				{
					case 6:
						src = &( chunk->spread_height[obj-1] );
						break;
					case 7:
						src = &( chunk->spread_depth[obj-1] );
						break;
				}
			}
			if ( numUsedComponents == 8 ){
				switch (cp)
				{
					case 6:
						src = &( chunk->spread_height[obj-1] );
						break;
					case 7:
						src = &( chunk->spread_depth[obj-1] );
						break;
					case 8:
						src = &( chunk->dynamic_object_priority[obj-1] );
						break;
				}
			}
			kin = 0;
			for (k = 1; k < iframe_period; k++)
			{
				target[k] = src[kin];
				kin += num_objects;
			}
			target[0]             = 0.0f;
			target[iframe_period] = 0.0f;

			/* approximate target function via polygon courses */
			num_in = 2;
			x_in[0] = 1.0f;
			x_in[1] = 1.0f + iframe_period;
			y_in[0] = (float)(ROUND(target[0]));
			y_in[1] = (float)(ROUND(target[iframe_period]));
			pol_approx(y_out, &num_in, x_in, y_in, num_out, x_out, target);

			/* fill in the polygon structure */
			pol = &( p->pol[oam_mat2D_pol(obj,cp,numUsedComponents)] );
			for (k = 1; k < num_in - 1; k++)
			{
				pol->pos[k-1]   = x_in[k];
				pol->value[k-1] = y_in[k];
			}
			pol->num = num_in - 2;
		}
	}
}


/**
 * @brief Get maximum number of polygon points for a given component
 * @param  p   polygons
 * @param  cp  component index (1: azimuth; 2: elevation; 3: radius; 4: gain; 5: spread)
 */
static int get_max_points_cp(StructOamPolChunk* p, int cp, int numUsedComponents)
{
	StructOamPolygon* pol;
	int num_objects = p->num_objects;
	int max_points = 0;
	int obj;

	for (obj = 1; obj <= num_objects; obj++)
	{
		pol = &( p->pol[oam_mat2D_pol(obj,cp,numUsedComponents)] );
		if (pol->num > max_points)
			max_points = pol->num;
	}
	return max_points;
}


/**
 * @brief Get maximum number of polygon points
 * @param  p              polygons (input)
 * @param  max_points_cp  array with maximum number of polygon points (output)
 */
static int get_max_points(StructOamPolChunk* p, int* max_points_cp, int numUsedComponents)
{
	int max_points = 0;
	int cp;

	for (cp = 1; cp <= numUsedComponents; cp++)
	{
		max_points_cp[cp-1] = get_max_points_cp(p, cp, numUsedComponents);
		max_points = MAX(max_points, max_points_cp[cp-1]);
	}

	return max_points;
}


/**
 * @brief Write "offset_data()" bitstream section
 * @param  pbs             pseudo bitstream
 * @param  pol             polygon representation
 * @param  iframe_period   I-Frame period
 * @param  bits_per_point  number of bits
 */
static void offset_data(OamPBS* pbs,
                        const StructOamPolygon* pol,
                        int iframe_period,
                        int bits_per_point)
{
	int num_points;
	int len_bitfield;
	int offset_wordsize;
	int len_direct;
	int bitfield_syntax;
	int bitfield[OAM_MAX_IFRAME_PERIOD - 1];
	int k, poly_pos;

	num_points = pol->num;
	len_bitfield = iframe_period - 1;
	offset_wordsize = ROUND(ceil(LOG2(MAX(1,len_bitfield))));
	len_direct = bits_per_point + num_points * offset_wordsize;
	if (len_bitfield < len_direct)
	{
		bitfield_syntax = 1;
		oam_bitbuf_add(pbs, bitfield_syntax, 1);					/* bitfield_syntax */
		memset(bitfield, 0, (iframe_period - 1) * sizeof(int));
		for (k = 0; k < pol->num; k++)
		{
			poly_pos = ROUND(pol->pos[k]) - 2;
			bitfield[poly_pos] = 1;
		}
		for (k = 0; k < iframe_period - 1; k++)
			oam_bitbuf_add(pbs, bitfield[k], 1);					/* offset_bitfield */
	}
	else
	{
		bitfield_syntax = 0;
		oam_bitbuf_add(pbs, bitfield_syntax, 1);					/* bitfield_syntax */
		oam_bitbuf_add(pbs, num_points - 1, bits_per_point);		/* num_points (-1 !!!) */
		for (k = 0; k < pol->num; k++)
		{
			poly_pos = ROUND(pol->pos[k]) - 2;
			oam_bitbuf_add(pbs, poly_pos, offset_wordsize);			/* frame_offset */
		}
	}
}


/**
 * @brief Determine the wordsize to store all pol->value values as signed integer
 * @param  pol  polygon representation
 */
static int required_signed_bits(const StructOamPolygon* pol)
{
	int xmin, xmax;
	int minval, maxval;
	int bits;
	int value;
	int k, kmax;

	/* determine the minimum and maximum values of pol->value */
	value = ROUND(pol->value[0]);
	xmin = value;
	xmax = value;
	kmax = pol->num;
	for (k = 1; k < kmax; k++)
	{
		value = ROUND(pol->value[k]);
		xmin = MIN(xmin, value);
		xmax = MAX(xmax, value);
	}

	/* determine the required wordsize */
	bits = 2;
	minval = - (1<<(bits-1));
	maxval = ((1<<bits) - 1) + minval;
	while (xmin < minval || xmax > maxval)
	{
		bits++;
		minval = - (1<<(bits-1));
		maxval = ((1<<bits) - 1) + minval;
	}

	return bits;
}

/**
 * @brief  Adds differential value to pseudo bitstream (D-Frame)
 * @param  pbs                       pseudo bitstream
 * @param  pol_dframe                polygon representation 
 * @param  iframe_period             period between i-frames
 * @param  bits_per_point            number ob bits per point
 * @param  objNo                     number of current object
 * @param  componentNo               number of current component
 * @param  numUsedComponents         number of used components
 * @param  hasDynamicObjectPriority  dynamic object priority flag
 */
static void add_diff_value(OamPBS* pbs, 
                           StructOamPolChunk* pol_dframe, 
                           int iframe_period, 
                           int bits_per_point,
                           int objNo, 
                           int componentNo, 
                           int numUsedComponents,
                           int hasDynamicObjectPriority)
{

	int flag_val;
	StructOamPolygon* pol;
	int num_bits;
	int k;

	pol = &( pol_dframe->pol[oam_mat2D_pol(objNo,componentNo, numUsedComponents)] );
	flag_val = (pol->num > 0);
	oam_bitbuf_add(pbs, flag_val, 1);								/* flag_val */
	if (flag_val)
	{
		offset_data(pbs, pol, iframe_period, bits_per_point);		/* offset_data() */
		num_bits = required_signed_bits(pol);
		if (componentNo == numUsedComponents && hasDynamicObjectPriority) {
			oam_bitbuf_add(pbs, num_bits - 2, 2);						/* num_bits (-2 !!!) */
		}
		else {
			oam_bitbuf_add(pbs, num_bits - 2, 3);						/* num_bits (-2 !!!) */
		}
		for (k = 0; k < pol->num; k++)
		{
			oam_bitbuf_add(pbs, ROUND(pol->value[k]), num_bits);	/* differential_value */
		}
	}

}

/**
 * @brief Encode a scales OAM data chunk (D-Frame)
 * @param  pbs         pseudo bitstream
 * @param  chunk       scaled OAM data chunk (input)
 * @param  chunk_i     I-Frame data chunk (output)
 * @param  chunk_d     D-Frame data chunk (output)
 * @param  first       last sample values of the previous datachunk
 * @param  last        last sample values of the current datachunk
 * @param  pol_dframe  polygon representation (output)
 */
static void encode_chunk_dframe(OamPBS* pbs,
                                const StructOamMultidata* chunk,
                                StructOamMultidata* chunk_i,
                                StructOamMultidata* chunk_d,
                                const StructOamMultidata* first,
                                const StructOamMultidata* last,
                                StructOamPolChunk* pol_dframe,
                                const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority,
                                const int hasUniformSpread)
{
	int max_points;
	int max_points_cp[OAM_NUMBER_COMPONENTS];
	int num_objects, iframe_period;
	int bits_per_point;
	int cp, obj;
	int n;
	int fixed_val;

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

	num_objects   = chunk->size1;
	iframe_period = chunk->size2;

	/* set chunk_i and chunk_d size */
	chunk_i->size2 = chunk->size2;
	chunk_d->size2 = chunk->size2;
	chunk_i->num_elements = chunk->num_elements;
	chunk_d->num_elements = chunk->num_elements;

	/* Get I-Frame data chunk */
	get_iframe_chunk(chunk_i, first, last, iframe_period);

	/* Get D-Frame data chunk */
	for (n = 0; n < chunk->num_elements; n++)
	{
		chunk_d->azimuth[n]   = chunk->azimuth[n]   - chunk_i->azimuth[n];
		chunk_d->elevation[n] = chunk->elevation[n] - chunk_i->elevation[n];
		chunk_d->radius[n]    = chunk->radius[n]    - chunk_i->radius[n];
		chunk_d->gain[n]      = chunk->gain[n]      - chunk_i->gain[n];
		chunk_d->spread[n]    = chunk->spread[n]    - chunk_i->spread[n];
		chunk_d->spread_height[n]   = chunk->spread_height[n]  - chunk_i->spread_height[n];
		chunk_d->spread_depth[n]    = chunk->spread_depth[n]   - chunk_i->spread_depth[n];
		chunk_d->dynamic_object_priority[n]  = chunk->dynamic_object_priority[n]  - chunk_i->dynamic_object_priority[n];
	}

	/* approximate the D-Frame data chunk by polygon courses */
	dchunk2pol(pol_dframe, chunk_d, numUsedComponents);

	/* determine maximum number of polygon points */
	max_points = get_max_points(pol_dframe, max_points_cp, numUsedComponents);

	/* number of required bits for storing the number of polygon points */
	bits_per_point = ROUND(ceil(LOG2(MAX(1,max_points))));
	oam_bitbuf_add(pbs, bits_per_point, 4);										/* bits_per_point */


	/* 1 = azimuth; 2 = elevation; 3 = radius; 4 = gain; 5 = spread; 6 = dynamic_object_priority */
	for (cp = 1; cp <= numUsedComponents; cp++)
	{
		if( (cp == 6 || cp == 7) && !hasUniformSpread ){ /* if nonUniformSpread, additional spread componentes are handled with cp == 5 */
			continue;
		}
		if( cp == 5 && !hasUniformSpread ){ /* if nonUniformSpread, check additional spread components */
			if( (max_points_cp[5-1] == 0) && (max_points_cp[6-1] == 0) && (max_points_cp[7-1] == 0) ){
				fixed_val = 1;
			}
			else{
				fixed_val = 0;
			}
		}
		else{
			fixed_val = (max_points_cp[cp-1] == 0);
		}
		oam_bitbuf_add(pbs, fixed_val, 1);										/* fixed_val */
		if (!fixed_val)
		{
			for (obj = 1; obj <= num_objects; obj++)
			{
				add_diff_value(pbs, pol_dframe, iframe_period, bits_per_point, obj, cp, numUsedComponents, hasDynamicObjectPriority);
				if( !hasUniformSpread && cp == 5){ /* repeat for non uniform spread components */
					add_diff_value(pbs, pol_dframe, iframe_period, bits_per_point, obj, cp+1, numUsedComponents, hasDynamicObjectPriority);
					add_diff_value(pbs, pol_dframe, iframe_period, bits_per_point, obj, cp+2, numUsedComponents, hasDynamicObjectPriority);
				}
			}
		}
	}
}


/**
 * @brief Encode a scales OAM data chunk (I-Frame)
 * @param  pbs    pseudo bitstream
 * @param  chunk  scaled OAM data chunk
 * @param  last   last sample values of the previous datachunk (output)
 */
static void encode_chunk_iframe(OamPBS* pbs,
                                const StructOamMultidata* chunk,
                                StructOamMultidata* last,
                                const unsigned int iframe_period,
                                const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority,
                                const unsigned int hasUniformSpread)
{
	int azi[OAM_MAX_NUM_OBJECTS];
	int ele[OAM_MAX_NUM_OBJECTS];
	int rad[OAM_MAX_NUM_OBJECTS];
	int gain[OAM_MAX_NUM_OBJECTS];
	int spread[OAM_MAX_NUM_OBJECTS];
	int spread_height[OAM_MAX_NUM_OBJECTS];
	int spread_depth[OAM_MAX_NUM_OBJECTS];
	int dynamic_object_priority[OAM_MAX_NUM_OBJECTS];
	int num_objects, num_samples;
	int common_azimuth   = 1, 
	    common_elevation = 1,
	    common_radius    = 1,
	    common_gain      = 1,
	    common_spread    = 1,
	    common_dynamic_object_priority  = 1;
	int n, n0;

	/* quantize the floating point values */
	num_objects = chunk->size1;
	num_samples = iframe_period;

	n0 = num_objects * (num_samples - 1);
	for (n = 0; n < num_objects; n++)
	{
		azi[n]    = ROUND(chunk->azimuth[n0 + n]);
		ele[n]    = ROUND(chunk->elevation[n0 + n]);
		rad[n]    = ROUND(chunk->radius[n0 + n]);
		gain[n]   = ROUND(chunk->gain[n0 + n]);
		spread[n] = ROUND(chunk->spread[n0 + n]);
		spread_height[n] = ROUND(chunk->spread_height[n0 + n]);
		spread_depth[n]  = ROUND(chunk->spread_depth[n0 + n]);
		dynamic_object_priority[n] = ROUND(chunk->dynamic_object_priority[n0 + n]);
		last->azimuth[n]   = (float)azi[n];
		last->elevation[n] = (float)ele[n];
		last->radius[n]    = (float)rad[n];
		last->gain[n]      = (float)gain[n];
		last->spread[n]    = (float)spread[n];
		last->spread_height[n]   = (float)spread_height[n];
		last->spread_depth[n]    = (float)spread_depth[n];
		last->dynamic_object_priority[n]  = (float)dynamic_object_priority[n];
	}

	/* look out for common values */
	for (n = 1; n < num_objects; n++)
	{
		if (azi[n] != azi[0])
		{
			common_azimuth = 0;
			break;
		}
	}
	for (n = 1; n < num_objects; n++)
	{
		if (ele[n] != ele[0])
		{
			common_elevation = 0;
			break;
		}
	}
	for (n = 1; n < num_objects; n++)
	{
		if (rad[n] != rad[0])
		{
			common_radius = 0;
			break;
		}
	}
	for (n = 1; n < num_objects; n++)
	{
		if (gain[n] != gain[0])
		{
			common_gain = 0;
			break;
		}
	}
	for (n = 1; n < num_objects; n++)
	{
		if( hasUniformSpread ){
			if (spread[n] != spread[0])
			{
				common_spread = 0;
				break;
			}
		}
		else{
			if (spread[n] != spread[0] || spread_height[n] != spread_height[0] || spread_depth[n] != spread_depth[0])
			{
				common_spread = 0;
				break;
			}
		}
	}
	for (n = 1; n < num_objects; n++)
	{
		if (dynamic_object_priority[n] != dynamic_object_priority[0])
		{
			common_dynamic_object_priority = 0;
			break;
		}
	}

	oam_bitbuf_add(pbs, num_samples - 1, 6);					/* iframe_period  (-1 !!!) */
	if (num_objects > 1)
	{
		oam_bitbuf_add(pbs, common_azimuth, 1);					/* common_azimuth */
		if (common_azimuth)
		{
			oam_bitbuf_add(pbs, azi[0], OAM_BITS_AZI);			/* default_azimuth */
		}
		else
		{
			for (n = 0; n < num_objects; n++)
				oam_bitbuf_add(pbs, azi[n], OAM_BITS_AZI);		/* position_azimuth */
		}

		oam_bitbuf_add(pbs, common_elevation, 1);				/* common_elevation */
		if (common_elevation)
		{
			oam_bitbuf_add(pbs, ele[0], OAM_BITS_ELE);			/* default_elevation */
		}
		else
		{
			for (n = 0; n < num_objects; n++)
				oam_bitbuf_add(pbs, ele[n], OAM_BITS_ELE);		/* position_elevation */
		}

		oam_bitbuf_add(pbs, common_radius, 1);					/* common_radius */
		if (common_radius)
		{
			oam_bitbuf_add(pbs, rad[0], OAM_BITS_RAD);			/* default_radius */
		}
		else
		{
			for (n = 0; n < num_objects; n++)
				oam_bitbuf_add(pbs, rad[n], OAM_BITS_RAD);		/* position_radius */
		}

		oam_bitbuf_add(pbs, common_gain, 1);					/* common_gain */
		if (common_gain)
		{
			oam_bitbuf_add(pbs, gain[0], OAM_BITS_GAIN);		/* default_gain */
		}
		else
		{
			for (n = 0; n < num_objects; n++)
				oam_bitbuf_add(pbs, gain[n], OAM_BITS_GAIN);	/* gain_factor */
		}

		oam_bitbuf_add(pbs, common_spread, 1);					/* common_spread */
		if (common_spread)
		{
				if( hasUniformSpread ){
					oam_bitbuf_add(pbs, spread[0], OAM_BITS_SPREAD);		/* default_spread */
				}
				else{
					oam_bitbuf_add(pbs, spread[0], OAM_BITS_SPREAD);		/* default_spread width*/
					oam_bitbuf_add(pbs, spread_height[0], OAM_BITS_SPREAD_HEIGHT);		/* default_spread heigth */
					oam_bitbuf_add(pbs, spread_depth[0], OAM_BITS_SPREAD_DEPTH);		/* default_spread depth*/
				}
		}
		else
		{
			for (n = 0; n < num_objects; n++){
				if( hasUniformSpread ){
					oam_bitbuf_add(pbs, spread[n], OAM_BITS_SPREAD);	/* object_spread */
				}
				else{
					oam_bitbuf_add(pbs, spread[n], OAM_BITS_SPREAD);		/* default_spread width*/
					oam_bitbuf_add(pbs, spread_height[n], OAM_BITS_SPREAD_HEIGHT);		/* default_spread heigth */
					oam_bitbuf_add(pbs, spread_depth[n], OAM_BITS_SPREAD_DEPTH);		/* default_spread depth*/
				}
			}
		}

		if (hasDynamicObjectPriority != DYN_OBJ_PRIO_NOT_NEEDED)
		{
			oam_bitbuf_add(pbs, common_dynamic_object_priority, 1);					/* common_dynamic_object_priority */
			if (common_dynamic_object_priority)
			{
				oam_bitbuf_add(pbs, dynamic_object_priority[0], OAM_BITS_PRIORITY);		/* default_dynamic_object_priority */
			}
			else
			{
				for (n = 0; n < num_objects; n++)
					oam_bitbuf_add(pbs, dynamic_object_priority[n], OAM_BITS_PRIORITY);	/* dynamic_object_priority */
			}
		}

	}
	else
	{
		oam_bitbuf_add(pbs, azi[0],    OAM_BITS_AZI);		/* position_azimuth */
		oam_bitbuf_add(pbs, ele[0],    OAM_BITS_ELE);		/* position_elevation */
		oam_bitbuf_add(pbs, rad[0],    OAM_BITS_RAD);		/* position_radius */
		oam_bitbuf_add(pbs, gain[0],   OAM_BITS_GAIN);		/* gain_factor */
		oam_bitbuf_add(pbs, spread[0], OAM_BITS_SPREAD);		/* object_spread */
		if( !hasUniformSpread ){
			oam_bitbuf_add(pbs, spread_height[0], OAM_BITS_SPREAD_HEIGHT);		/* default_spread heigth */
			oam_bitbuf_add(pbs, spread_depth[0],  OAM_BITS_SPREAD_DEPTH);		/* default_spread depth*/
		}
		if (hasDynamicObjectPriority != DYN_OBJ_PRIO_NOT_NEEDED)
		{
			oam_bitbuf_add(pbs, dynamic_object_priority[0], OAM_BITS_PRIORITY);		/* dynamic_object_priority */
		}
	}
}


/**
 * @brief Encode a scaled OAM data chunk
 * @param  pbs         pseudo bitstream
 * @param  chunk       scaled OAM data chunk (input)
 * @param  chunk_i     I-Frame data chunk (output)
 * @param  chunk_d     D-Frame data chunk (output)
 * @param  first       last sample values of the previous datachunk (input/output)
 * @param  last        last sample values of the current datachunk (input/output)
 * @param  pol_dframe  polygon representation (output)
 */
void encode_chunk(OamPBS* pbs,
                         const StructOamMultidata* chunk,
                         StructOamMultidata* chunk_i,
                         StructOamMultidata* chunk_d,
                         StructOamMultidata** first,
                         StructOamMultidata** last,
                         StructOamPolChunk* pol_dframe,
                         const unsigned int iframe_period,
                         const DYNAMIC_OBJECT_PRIO hasDynamicObjectPriority,
                         const unsigned int hasUniformSpread)
{
	StructOamMultidata* swap;
	int dsize0 = 6;
	int iframe_num;
	int has_differential_data;

	/* swap "first" and "last" */
	swap   = *first;
	*first = *last;
	*last  = swap;

	/* encode I-Frame and get new "last" vector */
	encode_chunk_iframe(pbs, chunk, *last, iframe_period, hasDynamicObjectPriority, hasUniformSpread);

	/* flag: has_differential_data */
	iframe_num = pbs->num;
	has_differential_data = 1;
	oam_bitbuf_add(pbs, has_differential_data, 1);			/* has_differential_data */

	/* encode D-Frame */
	encode_chunk_dframe(pbs, chunk, chunk_i, chunk_d, *first, *last, pol_dframe, hasDynamicObjectPriority, hasUniformSpread);

	/* remove empty D-Frame */
	if (pbs->num - iframe_num == dsize0)
	{
		pbs->num -= dsize0;
		has_differential_data = 0;
		oam_bitbuf_add(pbs, has_differential_data, 1);		/* has_differential_data */
	}
}



