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
 *  @file oam_common.h
 *  @brief Common functions and type definitions
 *  @author Christian Borss <christian.borss@iis.fraunhofer.de>
 *
 *  This header file contains declarations of functions and type definitions
 *  that are used by the object metadata encoder and decoder.
 *
 *  @date 26.06.2013 - first version
 *
 *  @version 0.1
 */


#ifndef  __OAM_COMMON_H__
#define  __OAM_COMMON_H__

#if defined _WIN32 || defined _WINDOWS
typedef unsigned __int64 uint64_t;
typedef unsigned __int16 uint16_t;
#else
#include <stdint.h>
#endif

#include <stdio.h>

#define OAM_BITS_AZI  8           /**< number of quantization bits for the azimuth angle */
#define OAM_BITS_ELE  6           /**< number of quantization bits for the elevation angle */
#define OAM_BITS_RAD  4           /**< number of quantization bits for the radius (logarithmic) */
#define OAM_BITS_GAIN 7           /**< number of quantization bits for the object gain */
#define OAM_BITS_SPREAD 7         /**< number of quantization bits for the spread angle */
#define OAM_BITS_SPREAD_WIDTH 7   /**< number of quantization bits for the spread width */
#define OAM_BITS_SPREAD_HEIGHT 5  /**< number of quantization bits for the spread height */
#define OAM_BITS_SPREAD_DEPTH 4   /**< number of quantization bits for the spread depth */
#define OAM_BITS_PRIORITY 3       /**< number of quantization bits for the dynamic object priority */

#define OAM_MAX_NUM_OBJECTS 128	/**< maximum number of objects */

#define OAM_NUMBER_COMPONENTS	8	/**< number of OAM components */

#define ADD_NONUNIFORMSPREAD 0

typedef enum {
	DYN_OBJ_PRIO_NOT_NEEDED   = 0,
	DYN_OBJ_PRIO_AVAILABLE    = 1
} DYNAMIC_OBJECT_PRIO;

/**
 * @brief OAM data structure
 *
 * This structure contains the data of an object metadata (OAM) file.
 * The OAM header is not part of this structure.
 */
typedef struct
{
	uint64_t* sample;	        /**< matrix with sample positions */
	float* azimuth;             /**< matrix with azimuth angles in degree */
	float* elevation;           /**< matrix with elevation angles in degree */
	float* radius;              /**< matrix with radius values in meter */
	float* gain;                /**< matrix with linear gain values */
	float* spread;              /**< matrix with spread angles in degrees */
	float* spread_height;       /**< matrix with spread height */
	float* spread_depth;        /**< matrix with spread depth */
	float* dynamic_object_priority; /**< matrix with priority values */
	int num_elements;           /**< number of elements that the buffer(s) can hold  */
	int size1;                  /**< number of rows  */
	int size2;                  /**< number of columns  */
} StructOamMultidata;


/**
 * @brief Polygon data structure
 *
 * This structure defines a polygon course.
 */
typedef struct
{
	float* pos;     /**< polygon x-values */
	float* value;	/**< polygon y-values */
	int num;        /**< number of points */
	int bufsize;    /**< buffer size */
} StructOamPolygon;


/**
 * @brief Frame polygon data structure
 *
 * This structure contains the complete data of a COAM frame
 * (either I-Frame or D-Frame).
 */
typedef struct
{
	StructOamPolygon* pol;  /**< polygon x-values */
	int num_objects;        /**< number of objects */
	int num_elements;       /**< buffer size */
} StructOamPolChunk;


/**
 * @brief Convert matrix indices [1..M,1..N] to 1D memory address
 * @param  dim1  size M of the matrix
 * @param  n1    row index [1..M]
 * @param  n2    column index [1..N]
 * @return int   1D array memory address
 *
 * This function converts Matlab-style 2D matrix indices to a 1D memory
 * address.
 *
 *  @date 24.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_mat2D(int dim1, int n1, int n2);

/**
 * @brief Convert polygon matrix indices [obj,cp] to a 1D memory address.
 * @param  obj  object index [1..N]
 * @param  cp   component index [1..4]
 * @return int  array offset
 *
 * This function is used to address the StructOamPolChunk->pol array.
 *
 *  @date 24.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_mat2D_pol(int obj, int cp, int numUsedComponents);


/**
 * @brief Allocate and initialize an oam_multidata structure.
 * @param  size1                number of objects
 * @param  size2                number of sample points
 * @return StructOamMultidata*  resulting oam_multidata structure
 *
 * This function is used to allocate and initialize an oam_multidata structure.
 * The resulting structure holds a NxM matrix in single-precission floating
 * point for each component (sample, azimuth, elevation, radius, gain).
 *
 *  @date 25.07.2013 - first version
 *
 *  @version 0.1
 */
StructOamMultidata* oam_multidata_create(int size1, int size2);


/**
 * @brief Clear the internal memory of an oam_multidata structure.
 * @param  multidata            oam_multidata structure that shall be cleared
 * @return StructOamMultidata*  pointer to the modified structure
 *
 * This function resets an oam_multidata structure to an empty structure.
 *
 *  @date 25.07.2013 - first version
 *
 *  @version 0.1
 */
StructOamMultidata* oam_multidata_setempty(StructOamMultidata* multidata);


/**
 * @brief Clear and free an oam_multidata structure.
 * @param  multidata			oam_multidata structure that shall be cleared and freed
 * @return StructOamMultidata*  pointer to the resulting structure (NULL)
 *
 * This function frees all memory that belongs to a an oam_multidata structure.
 *
 *  @date 25.07.2013 - first version
 *
 *  @version 0.1
 */
StructOamMultidata* oam_multidata_destroy(StructOamMultidata* multidata);


/**
 * @brief Enlarge an oam_multidata structure.
 * @param  multidata            oam_multidata structure that shall be enlarged
 * @param  size1                number of objects
 * @param  size2                number of sample points
 * @return StructOamMultidata*  pointer to the modified oam_multidata structure
 *
 * This function re-allocates the internal memory of an oam_multidata
 * structure and updates the size values (size1, size2, num_elements).
 * size1 and multidata->size1 must be equal.
 *
 *  @date 25.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_multidata_realloc(StructOamMultidata* multidata, int size1, int size2);


/**
 * @brief Add the content of an oam_multidata structure to another structure.
 * @param  y    oam_multidata structure that shall be processed (input and
 *              output)
 * @param  x    oam_multidata structure that shall be added
 * @return int  0: no error; 1: error
 *
 * This function adds the content of an oam_multidata structure to another
 * structure.
 *
 *  @date 25.07.2013 - first version
 *
 *  @version 0.1
 */
int oam_multidata_add(StructOamMultidata* y, const StructOamMultidata* x);


/**
 * @brief Open an OAM file for writing and write the OAM header.
 * @param  filename     OAM filename
 * @param  num_objects  number of objects
 * @return FILE*        open file descriptor
 *
 * This function opens an OAM file for writing and writes an OAM header
 * according to the given parameters.
 *
 *  @date 29.06.2013 - first version
 *
 *  @version 0.1
 */

FILE* oam_write_open(const char* filename, uint16_t num_objects, const unsigned int oam_version, const unsigned int hasDynamicObjectPriority, const unsigned int hasUniformSpread);

/**
 * @brief Append OAM data to an OAM file.
 * @param  file   OAM file descriptor
 * @param  chunk  OAM data chunk
 *
 * This function appends a given OAM data chunk to an OAM file.
 *
 *  @date 29.06.2013 - first version
 *
 *  @version 0.1
 */

void oam_write_process(FILE* file, const StructOamMultidata* chunk, const unsigned int oam_version, const unsigned int hasDynamicObjectPriority, const unsigned int hasUniformSpread);


/**
 * @brief Close OAM file.
 * @param  file  OAM file descriptor
 *
 * This function closes the given OAM file descriptor.
 *
 *  @date 29.06.2013 - first version
 *
 *  @version 0.1
 */
void oam_write_close(FILE* file);







#endif	/* __OAM_COMMON_H__ */
