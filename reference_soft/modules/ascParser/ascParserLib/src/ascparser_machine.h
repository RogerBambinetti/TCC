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

#ifndef __ASCPARSER__MACHINE_H
#define __ASCPARSER__MACHINE_H

#ifdef _MSC_VER
#include <basetsd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif 


#if defined _MSC_VER

typedef  signed char   INT8;
typedef  signed short  INT16;
typedef  signed int    INT32;
typedef  signed int    INT;
typedef  float         FLOAT;
typedef  double        DOUBLE;


#elif defined __GNUC__ && !defined __sparc__ && !defined __sparc_v9__

#if ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)) || (__GNUC__ > 4)

typedef  signed char   INT8;
typedef  signed short  INT16;
typedef  signed int    INT32;
typedef  signed int    INT;
typedef  float         FLOAT;
typedef  double        DOUBLE;

#else

typedef __attribute__((aligned(16))) signed char   INT8;
typedef __attribute__((aligned(16))) signed short  INT16;
typedef __attribute__((aligned(16))) signed int    INT32;
typedef __attribute__((aligned(16))) signed int    INT;
typedef __attribute__((aligned(16))) signed long   LONG;
typedef __attribute__((aligned(16))) float         FLOAT;
typedef __attribute__((aligned(16))) double        DOUBLE; 

#endif

#else 

typedef  signed char   INT8;
typedef  signed short  INT16;
typedef  signed int    INT32;
typedef  float         FLOAT;
typedef  double        DOUBLE;

#endif


/* typedef long            LONG; */
/* typedef unsigned long   ULONG; */
/* typedef unsigned int    UINT; */
/* typedef  signed int     INT; */
typedef  unsigned char  UINT8;
typedef  unsigned short UINT16;
typedef  unsigned int   UINT32;


#ifndef INT64
#ifndef WIN32
#define INT64  long long
#else
#define INT64 __int64
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
