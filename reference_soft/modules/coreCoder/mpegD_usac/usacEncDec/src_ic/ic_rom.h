#ifndef __INTERNAL_CHANNEL_ROM_LIBRARY_HEADER__
#define __INTERNAL_CHANNEL_ROM_LIBRARY_HEADER__

#include "ic_defs.h"

/* #define RM6_INTERNAL_CHANNEL */
#define RM6_ICG_SYNTAX

extern			  IC_CONFIG   ICConfig;
extern const	IC_RULES	  ICRules[47];
extern			  IC_DMX		  ICDmx;  

extern float ic_gains[4];
extern float ic_EQ[4][100];
extern float * FLT[2];
extern float   GAB[2];

#endif