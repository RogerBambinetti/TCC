/***********************************************************************************
 
 This software module was originally developed by 
 
 Samsung Electronics
 
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
 
 Samsung Electronics retains full right to modify and use the code for 
 their own purpose, assign or donate the code to a third party and to inhibit third 
 parties from using the code for products that do not conform to MPEG-related ITU 
 Recommendations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef IAR_ROM_H
#define IAR_ROM_H

#include <stdio.h>

#define IAR		1
#define R3T		1

/* TABLE : SPATIAL COLORATION FILTERS FOR HORIZONTAL CHANNELS */
extern float COLOR_180_110[71];
extern float COLOR_090_030[71];
extern float COLOR_060_110[71];
extern float COLOR_135_110[71];
extern float COLOR_090_110[71];
extern float COLOR_060_030[71];
/* TABLE : SPATIAL COLORATION FILTERS FOR HORIZONTAL CHANNELS, ERB bands for STFT */
extern float COLOR_180_110_StftErb[58];
extern float COLOR_090_030_StftErb[58];
extern float COLOR_060_110_StftErb[58];
extern float COLOR_135_110_StftErb[58];
extern float COLOR_090_110_StftErb[58];
extern float COLOR_060_030_StftErb[58];

/* TABLE : REQUIRED OUTPUT CHANNELS FOR ELEVATION RENDERING FOR isPossibleElev */
/* extern bool REQOUT[13][6]; */

/* TABLE : INITIAL SPATIAL LOCALIZATION PANNING COEFFICIENTS G_VH */
extern float GVH[13][6];

/* TABLE : INITIAL SPATIAL LOCALIZATION PANNING COEFFICIENTS G_VL */
extern float GVL[13][6];

/* TABLE : SPATIAL ELEVATION FILTER INITIAL VALUES (FOR THE 35 DEGREE IN ELEVATION */
extern float EQVF[71];
extern float EQVB[71];
extern float EQVFC[71];
extern float EQVBC[71];
extern float EQVOG[71];
extern float EQVS[71];
extern float EQBTM[71];
extern float EQVBA[71];
/* TABLE : SPATIAL ELEVATION FILTER INITIAL VALUES (FOR THE 35 DEGREE IN ELEVATION , ERB bands for STFT */
extern float EQVF_StftErb[58];
extern float EQVB_StftErb[58];
extern float EQVFC_StftErb[58];
extern float EQVBC_StftErb[58];
extern float EQVOG_StftErb[58];
extern float EQVS_StftErb[58];
extern float EQBTM_StftErb[58];
extern float EQVBA_StftErb[58];

extern int	 is4GVH[71];
extern int	 is4GVH_StftErb[58];
extern int	 topIn[13];
extern int	 midOut[6];
extern int	 D2;
extern int	 elv;

/* INDEX : FOR THE CONVENIENCE FOR ONLY HEIGHT CHANNELS */
typedef enum
{
	TFC=0,
	TFL,
	TFR,
	TFLA,
	TFRA,
	TSL,
	TSR,
	TBLA,
	TBRA,
	TBL,
	TBR,
	TBC,
	VOG
} H;

/* INDEX : FOR THE 5.1 CHANNEL SYSTEM */
typedef enum
{
	FL=0,
	FR,
	FC,
	SW,
	SL,
	SR
} C;

#endif  /* IAR_ROM_H */
