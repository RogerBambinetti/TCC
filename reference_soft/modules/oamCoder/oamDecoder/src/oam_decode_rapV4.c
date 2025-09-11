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
#include "oam_decode_rapV4.h"
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
	minval = -180.0f;
	maxval =  180.0f;
	for (n = 0; n < num_elements; n++)
		x->azimuth[n] = MIN(MAX(x->azimuth[n], minval), maxval);

	/* elevation */
	minval = -90.0f;
	maxval =  90.0f;
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
	minval =    0.0f;
	maxval =  180.0f;
	for (n = 0; n < num_elements; n++)
		x->spread[n] = MIN(MAX(x->spread[n], minval), maxval);

	if( !hasUniformSpread ){

		/* spread height */
		minval =  0.0f;
		maxval = 90.0f;
		for (n = 0; n < num_elements; n++)
			x->spread_height[n] = MIN(MAX(x->spread_height[n], minval), maxval);

		/* spread depth */
		minval =  0.0f;
		maxval = 15.5f;
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
static int realloc_pol_chunk(StructOamPolChunk* p, int num_points)
{
	int n, size;

	if (p == NULL || p->num_elements < 1)
		return 0;

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
 * @brief Inverse polygon transformation of a complete I-Frame/D-Frame.
 * @param  chunk          result of the inverse polygon transformation
 * @param  p              I-Frame/D-Frame polygon content
 * @param  iframe_period  I-Frame period / chunk length
 */
static void pol2chunk(StructOamMultidata* chunk, const StructOamPolChunk* p, int iframe_period, int hasDynamicObjectPriority, int hasUniformSpread)
{
	int k, obj, cp, num_in, kout;
	float n_x[OAM_MAX_IFRAME_PERIOD];
	float n[OAM_MAX_IFRAME_PERIOD + 1];
	float y[OAM_MAX_IFRAME_PERIOD + 1];
	float y_out[OAM_NUMBER_COMPONENTS][OAM_MAX_IFRAME_PERIOD];
	StructOamPolygon* pol;

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

	for (k = 0; k < iframe_period; k++)
		n_x[k] = (float)(k+1);

	for (obj = 1; obj <= p->num_objects; obj++)
	{
		for (cp = 1; cp <= numUsedComponents; cp++)
		{
			pol = &( p->pol[oam_mat2D_pol(obj,cp,numUsedComponents)] );
			num_in = pol->num;
			for (k = 0; k < num_in; k++)
			{
				n[k] = pol->pos[k] - 1;
				y[k] = pol->value[k];
			}
			interpol(y_out[cp-1], num_in, n, y, iframe_period, n_x);
		}
		for (k = 0; k < iframe_period; k++)
		{
			kout = oam_mat2D(p->num_objects, obj, k+1);
			chunk->azimuth[kout]   = y_out[0][k];
			chunk->elevation[kout] = y_out[1][k];
			chunk->radius[kout]    = y_out[2][k];
			chunk->gain[kout]      = y_out[3][k];
			chunk->spread[kout]    = y_out[4][k];
			chunk->sample[kout]    = 0;
			if (numUsedComponents == 8 ){
				chunk->spread_height[kout]    = y_out[5][k];
				chunk->spread_depth[kout]     = y_out[6][k];
				chunk->dynamic_object_priority[kout]  = y_out[7][k];
			}
			if (numUsedComponents == 7 ){
				chunk->spread_height[kout]    = y_out[5][k];
				chunk->spread_depth[kout]     = y_out[6][k];
			}
			if (numUsedComponents == 6 ){
				chunk->dynamic_object_priority[kout]  = y_out[5][k];
			}
		}
	}
}


/**
 * @brief Extract the polygon x-values from the given bitstream.
 * @param  bitstream       bitstream structure; read position is updated
 * @param  poly_pos        array with resulting x-values; given buffer must be
 *                         large enough to hold up to iframe_period-1 elements
 * @param  num_points      number of returned elements
 * @param  iframe_period   I-Frame period
 * @param  bits_per_point  word size of the x-values
 * @return int             0:success, -1:error
 */
static int read_offset_data(OamBitbufRead* bitstream, float* poly_pos, int* num_points, int iframe_period, int bits_per_point)
{
	int offset_wordsize;
	float len_bitfield;
	int bitfield_syntax;
	int k, bitfield_k, pos;

	*num_points = 0;
	len_bitfield = (float)(iframe_period - 1);
	offset_wordsize = (int)ceil(log(len_bitfield) / log(2.0f));
	if (oam_bitstream_read(bitstream, &bitfield_syntax, 1, 'u') != 0)					/* bitfield_syntax */
		return -1;
	if (bitfield_syntax)
	{
		for (k = 0; k < len_bitfield; k++)
		{
			if (oam_bitstream_read(bitstream, &bitfield_k, 1, 'u') != 0)				/* offset_bitfield */
				return -1;
			if (bitfield_k)
				poly_pos[(*num_points)++] = (float)(k + 2);		/* [2..iframe_period] */
		}
	}
	else
	{
		if (oam_bitstream_read(bitstream, num_points, bits_per_point, 'u') != 0)		/* num_points (-1 !!!) */
			return -1;
		*num_points = *num_points + 1;
		for (k = 0; k < *num_points; k++)
		{
			if (oam_bitstream_read(bitstream, &pos, offset_wordsize, 'u') != 0)			/* frame_offset */
				return -1;
			poly_pos[k] = (float)(pos + 2);						/* [2..iframe_period] */
		}
	}

	return 0;
}


/**
 * @brief Parse an COAM I-Frame
 * @param  bitstream       bitstream structure; read position is updated (input/output)
 * @param  pol_iframe      COAM frame structure that is returned (output)
 * @param  last            last sample values of the last I-Frame (input/output)
 * @param  iframe_period   I-Frame period (output)
 * @param  num_objects     number of objects (input)
 * @return int             0:success, -1:error
 */
static int get_pol_iframe(OamBitbufRead* bitstream,
						  StructOamPolChunk* p,
						  int* last,
						  int* iframe_period,
						  int num_objects,
						  int hasDynamicObjectPriority,
						  int hasUniformSpread)
{
	int x[OAM_NUMBER_COMPONENTS * OAM_MAX_NUM_OBJECTS];
	int wordsize[OAM_NUMBER_COMPONENTS] = {OAM_BITS_AZI, OAM_BITS_ELE, OAM_BITS_RAD, OAM_BITS_GAIN, OAM_BITS_SPREAD_WIDTH, OAM_BITS_SPREAD_HEIGHT, OAM_BITS_SPREAD_DEPTH, OAM_BITS_PRIORITY};
	char sig[OAM_NUMBER_COMPONENTS] = {'s', 's', 'u', 's', 'u', 'u', 'u', 'u'};
	int numUsedComponents = OAM_NUMBER_COMPONENTS;
	int cp, tmpCp, obj, kout, kout1;
	int common_val;
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

	if (oam_bitstream_read(bitstream, iframe_period, 6, 'u') != 0)		/* iframe_period (-1!!!) */
		return -1;
	*iframe_period += 1;
	if (realloc_pol_chunk(p, *iframe_period + 1))
	{
		fprintf(stderr, "Error in get_pol_iframe(): re-allocation of memory for I-Frame failed.\n");
		return -1;
	}

	if (num_objects > 1)
	{
		for (cp = 1; cp <= numUsedComponents; cp++)
		{
			if( (cp == 6 || cp == 7) && !hasUniformSpread ){ /* if nonUniformSpread, additional spread componentes are handled with cp == 5 */
				continue;
			}
			ret |= oam_bitstream_read(bitstream, &common_val, 1, 'u');		/* common_val */
			if (common_val)
			{
				kout1 = oam_mat2D_pol(1, cp, numUsedComponents);
				ret |= oam_bitstream_read(bitstream, &( x[kout1] ), wordsize[cp-1], sig[cp-1]);		/* default_val */
				for (obj = 1; obj <= num_objects; obj++)
				{
					kout = oam_mat2D_pol(obj, cp, numUsedComponents);
					x[kout] = x[kout1];
				}
				if ( !hasUniformSpread && cp == 5){ /* additional spread components */
					tmpCp = cp + 1;
					kout1 = oam_mat2D_pol(1, tmpCp, numUsedComponents);
					ret |= oam_bitstream_read(bitstream, &( x[kout1] ), wordsize[tmpCp-1], sig[tmpCp-1]);		/* default_val */
					for (obj = 1; obj <= num_objects; obj++)
					{
						kout = oam_mat2D_pol(obj, tmpCp, numUsedComponents);
						x[kout] = x[kout1];
					}

					tmpCp = cp + 2;
					kout1 = oam_mat2D_pol(1, tmpCp, numUsedComponents);
					ret |= oam_bitstream_read(bitstream, &( x[kout1] ), wordsize[tmpCp-1], sig[tmpCp-1]);		/* default_val */
					for (obj = 1; obj <= num_objects; obj++)
					{
						kout = oam_mat2D_pol(obj, tmpCp, numUsedComponents);
						x[kout] = x[kout1];
					}
				}
			}
			else
			{
				for (obj = 1; obj <= num_objects; obj++)
				{
					kout = oam_mat2D_pol(obj, cp, numUsedComponents);
					ret |= oam_bitstream_read(bitstream, &( x[kout] ), wordsize[cp-1], sig[cp-1]);	/* position_val */
					if ( !hasUniformSpread && cp == 5){
						kout = oam_mat2D_pol(obj, cp+1, numUsedComponents);
						ret |= oam_bitstream_read(bitstream, &( x[kout] ), wordsize[cp+1-1], sig[cp+1-1]);	/* position_val */
						kout = oam_mat2D_pol(obj, cp+2, numUsedComponents);
						ret |= oam_bitstream_read(bitstream, &( x[kout] ), wordsize[cp+2-1], sig[cp+2-1]);	/* position_val */
					}
				}
			}
		}
	}
	else	/* if (num_objects > 1) */
	{
		for (cp = 1; cp <= numUsedComponents; cp++)
		{
			kout1 = oam_mat2D_pol(1, cp, numUsedComponents);
			ret |= oam_bitstream_read(bitstream, &( x[kout1] ), wordsize[cp-1], sig[cp-1]);			/* position_val */
		}
	}

	for (obj = 1; obj <= num_objects; obj++)
	{
		for (cp = 1; cp <= numUsedComponents; cp++)
		{
			kout = oam_mat2D_pol(obj, cp, numUsedComponents);
			p->pol[kout].pos[0] = 1.0f;
			p->pol[kout].pos[1] = (float)(*iframe_period + 1);
			p->pol[kout].value[0] = (float)last[kout];
			p->pol[kout].value[1] = (float)x[kout];
			p->pol[kout].num = 2;
			last[kout] = x[kout];
		}
	}

	return 0;
}

/**
 * @brief  Reads differential value from bitstream to polygon structure (D-Frame)
 * @param  bitstream                 bitstream
 * @param  p                         polygon structure 
 * @param  iframe_period             period between i-frames
 * @param  bits_per_point            number ob bits per point
 * @param  objNo                     number of current object
 * @param  componentNo               number of current component
 * @param  numUsedComponents         number of used components
 * @param  hasDynamicObjectPriority  dynamic object priority flag
 */
static int get_diff_value(OamBitbufRead* bitstream, 
                           StructOamPolChunk* p, 
                           int iframe_period, 
                           int bits_per_point,
                           int objNo, 
                           int componentNo, 
                           int numUsedComponents,
                           int hasDynamicObjectPriority)
{
	StructOamPolygon* pol;
	int flag_val, num_bits, diff_val;
	int num_points;
	int k;
	int ret = 0;

	ret |= oam_bitstream_read(bitstream, &flag_val, 1, 'u');		/* flag_val */
	if (flag_val)
	{
		pol = &( p->pol[oam_mat2D_pol(objNo,componentNo, numUsedComponents)] );
		ret |= read_offset_data(bitstream,
								&( pol->pos[1] ),
								&num_points,
								iframe_period,
								bits_per_point);	/* offset_data() */
		if ( componentNo == numUsedComponents && hasDynamicObjectPriority) {
			ret |= oam_bitstream_read(bitstream, &num_bits, 2, 'u');	/* num_bits (-2 !!!) */
		}
		else {
			ret |= oam_bitstream_read(bitstream, &num_bits, 3, 'u');	/* num_bits (-2 !!!) */
		}
		num_bits += 2;
		for (k = 1; k <= num_points; k++)
		{
			ret |= oam_bitstream_read(bitstream, &diff_val, num_bits, 's');	/* differential_val */
			pol->value[k] = (float)diff_val;
		}
		pol->pos[k]   = (float)(iframe_period + 1);
		pol->value[k] = 0.0f;
		pol->num      = k + 1;
	}

	return ret;

}


/**
 * @brief Parse an COAM D-Frame
 * @param  bitstream       bitstream structure; read position is updated (input/output)
 * @param  pol_dframe      COAM frame structure that is returned (output)
 * @param  iframe_period   I-Frame period (input)
 * @param  num_objects     number of objects (input)
 * @return int             0:success, -1:error
 */
static int get_pol_dframe(OamBitbufRead* bitstream,
						  StructOamPolChunk* p,
						  int iframe_period,
						  int num_objects,
						  int hasDynamicObjectPriority,
						  int hasUniformSpread)
{

	int kout;
	int obj, cp;
	int bits_per_point, fixed_val;
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

	for (kout = 0; kout < p->num_elements; kout++)
	{
		p->pol[kout].pos[0] = 1.0f;
		p->pol[kout].pos[1] = (float)(iframe_period + 1);
		p->pol[kout].value[0] = 0.0f;
		p->pol[kout].value[1] = 0.0f;
		p->pol[kout].num = 2;
	}

	if (bitstream == NULL)
	{
		return 0;
	}

	ret |= oam_bitstream_read(bitstream, &bits_per_point, 4, 'u');				/* bits_per_point */
	for (cp = 1; cp <= numUsedComponents; cp++)		/* [azimuth elevation radius gain spread] */
	{
		if( (cp == 6 || cp == 7) && !hasUniformSpread ){ /* if nonUniformSpread, additional spread componentes are handled with cp == 5 */
			continue;
		}
		ret |= oam_bitstream_read(bitstream, &fixed_val, 1, 'u');				/* fixed_val */
		if (fixed_val == 0)
		{
			for (obj = 1; obj <= num_objects; obj++)
			{
				ret |= get_diff_value(bitstream, p, iframe_period, bits_per_point, obj, cp, numUsedComponents, hasDynamicObjectPriority);
				if( !hasUniformSpread && cp == 5){ /* repeat for non uniform spread components */
					ret |= get_diff_value(bitstream, p, iframe_period, bits_per_point, obj, cp+1, numUsedComponents, hasDynamicObjectPriority);
					ret |= get_diff_value(bitstream, p, iframe_period, bits_per_point, obj, cp+2, numUsedComponents, hasDynamicObjectPriority);
				}
			}
		}
	}

	return ret;
}


/******************************
 *    Non-Static Functions    *
 ******************************/

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
                          uint64_t* sample_counter)
{

	int n, obj;
	int has_differential_data;
	int iframe_period = 0;

	/* decode pseudo-bitstream and get polygon representation */
	if (get_pol_iframe(bs, pol_iframe, last, &iframe_period, num_objects, hasDynamicObjectPriority, hasUniformSpread))
	{
		fprintf(stderr, "Fatal error in oam_decode_rapV4(): can't parse I-Frame.\n");
		exit(-1);
	}
	if (realloc_pol_chunk(pol_dframe, iframe_period + 1))
	{
		fprintf(stderr, "Fatal error in oam_decode_rapV4(): re-allocation of memory for D-Frame failed.\n");
		exit(-1);
	}

	if (oam_bitstream_read(bs, &has_differential_data, 1, 'u') != 0)		/* has_differential_data */
	{
		fprintf(stderr, "Error in oam_decode_rapV4(): syntax error in bitstream.\n");
		exit(-1);
	}
	if (has_differential_data)
	{
		if (get_pol_dframe(bs, pol_dframe, iframe_period, num_objects, hasDynamicObjectPriority, hasUniformSpread))
		{
			fprintf(stderr, "Fatal error in oam_decode_rapV4(): can't parse D-Frame.\n");
			exit(-1);
		}
	}
	else
	{
		get_pol_dframe(NULL, pol_dframe, iframe_period, num_objects, hasDynamicObjectPriority, hasUniformSpread);
	}

	/* enlarge chunk buffers */
	if (oam_multidata_realloc(chunk, num_objects, iframe_period))
	{
		fprintf(stderr, "Fatal error in oam_decode_rapV4(): can't enlarge chunk buffers.\n");
		exit(-1);
	}
	if (oam_multidata_realloc(chunk_d, num_objects, iframe_period))
	{
		fprintf(stderr, "Fatal error in oam_decode_rapV4(): can't enlarge chunk_d buffers.\n");
		exit(-1);
	}

	/* transform polygone representation to time signals */
	pol2chunk(chunk,   pol_iframe, iframe_period, hasDynamicObjectPriority, hasUniformSpread);
	pol2chunk(chunk_d, pol_dframe, iframe_period, hasDynamicObjectPriority, hasUniformSpread);
	if (oam_multidata_add(chunk, chunk_d))
	{
		fprintf(stderr, "Fatal error in oam_decode_rapV4(): can't add data chunk.\n");
		exit(-1);
	}

	/* restore sampling positions */
	for (n = 0; n < chunk->size2; n++)
	{
		for (obj = 1; obj <= num_objects; obj++)
		{
			int kout = oam_mat2D(num_objects, obj, n+1);
			chunk->sample[kout] = coreBlocksize + ((*sample_counter + n) * oamBlocksize);
		}
	}
	*sample_counter += n;

	/* Revert the encoder's scaling of the OAM data values */
	descale_multidata(chunk, hasUniformSpread);

	/* clip over-shooting values */
	limit_range(chunk, hasUniformSpread);

}



