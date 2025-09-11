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

#ifndef __SAOC_DECORR_TABLES_H__
#define __SAOC_DECORR_TABLES_H__

#include "saoc_const.h"


static float PremixingMatrix_11_Decorr_22_Output[11][22] = {
  {0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
  {1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f}
};


static float PremixingMatrix_9_Decorr_22_Output[9][22] = {
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f}   
};

static float PremixingMatrix_7_Decorr_22_Output[7][22] = {
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f}
};

static float PremixingMatrix_5_Decorr_22_Output[5][22] = {
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f}
};

static float PostmixingMatrix_11_Decorr_22_Output[22][11] = {
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f},
  {.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f},
  {0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f},
  {0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f}
};

static float PostmixingMatrix_9_Decorr_22_Output[22][9] = {
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f}   
};

static float PostmixingMatrix_7_Decorr_22_Output[22][7] = {
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.25f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	.25f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.25f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	.5f,	0.f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f,	0.f,	0.f}   
};

static float PostmixingMatrix_5_Decorr_22_Output[22][5] = {
{0.f,	0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	0.f,	.25f},
{.25f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{0.f,	0.f,	.25f,	0.f,	0.f},
{0.f,	0.f,	.25f,	0.f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	0.f,	.25f},
{.25f,	0.f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	0.f,	0.f,	.25f,	0.f},
{0.f,	.17f,	0.f,	0.f,	0.f},
{.25f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	.25f,	0.f,	0.f},
{0.f,	0.f,	.25f,	0.f,	0.f}
};
/* ======================================================================================================================================================= */

static float PremixingMatrix_11_Decorr_14_Output[11][14] = {
   { 1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0,	0.0,	0.0,	0.0,	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0, 	0.0},
   { 0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	0.0,	1.0}
};


static float PremixingMatrix_7_Decorr_14_Output[7][14] = {
  {0.f,   0.f,   1.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f},   
  {1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.0}
};

static float PremixingMatrix_5_Decorr_14_Output[5][14] = {
  {0.f,   0.f,   1.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f},   
  {1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f},  
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.0}
};

static float PremixingMatrix_4_Decorr_14_Output[4][14] = {
  {0.f,   0.f,   1.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f},   
  {1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   1.f,   1.0}
};

static float PostmixingMatrix_11_Decorr_14_Output[14][11] = {
  { 1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.5,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0},
  { 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0}  
};

static float PostmixingMatrix_7_Decorr_14_Output[14][7] = {
  {0.f,   0.f,   0.f,   .5f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   .5f,   0.f,   0.f},   
  {.5f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   .5f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   .5f,   0.f,   0.f,   0.f,   0.f},   
  {.5f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   .5f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   .5f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   .5f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   .5f},   
  {0.f,   .5f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   .5f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   .5f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   .5f,   0.0}   
};

static float PostmixingMatrix_5_Decorr_14_Output[14][5] = {
  {0.f,   0.f,   .25f,  0.f,   0.f},   
  {0.f,   0.f,   .25f,  0.f,   0.f},   
  {.5f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f,   0.f},   
  {.5f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   .25f,  0.f,   0.f},   
  {0.f,   0.f,   .25f,  0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   .5f},   
  {0.f,   0.f,   0.f,   0.f,   .5f},   
  {0.f,   .25f,  0.f,   0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   .5f,   0.f},   
  {0.f,   0.f,   0.f,   .5f,   0.0}  
};

static float PostmixingMatrix_4_Decorr_14_Output[14][4] = {
  {0.f,   0.f,   .25f,  0.f},   
  {0.f,   0.f,   .25f,  0.f},   
  {.5f,   0.f,   0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f},   
  {.5f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   .25f,  0.f},   
  {0.f,   0.f,   .25f,  0.f},   
  {0.f,   0.f,   0.f,   .25f},   
  {0.f,   0.f,   0.f,   .25f},   
  {0.f,   .25f,  0.f,   0.f},   
  {0.f,   .25f,  0.f,   0.f},   
  {0.f,   0.f,   0.f,   .25f},   
  {0.f,   0.f,   0.f,   .25f} 
};
/* ======================================================================================================================================================= */

static float PremixingMatrix_6_Decorr_11_Output[6][11] = {
  {1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f},   
  {0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.0}
};

static float PremixingMatrix_5_Decorr_11_Output[5][11] = {
  {1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f},     
  {0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.0}
};

static float PremixingMatrix_4_Decorr_11_Output[4][11] = {
  {1.f,   1.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f},     
  {0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.0}
};

static float PostmixingMatrix_6_Decorr_11_Output[11][6] = {
  {.5f,   0.f,   0.f,   0.f,   0.f,   0.f},  
  {0.f,   .5f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   .5f,   0.f},  
  {0.f,   0.f,   .5f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   .5f,   0.f,   0.f},  
  {.5f,   0.f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   .5f,   0.f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   .5f,   0.f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   .5f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,   0.f,   0.f,   1.f},   
  {0.f,   0.f,   0.f,   0.f,   .5f,   0.0} 
};

static float PostmixingMatrix_5_Decorr_11_Output[11][5] = {
  {.5f,   0.f,   0.f,    0.f,   0.f},   
  {0.f,   .5f,   0.f,    0.f,   0.f},   
  {0.f,   0.f,   0.f,    .5f,   0.f},   
  {0.f,   0.f,   .25f,   0.f,   0.f},   
  {0.f,   0.f,   .25f,   0.f,   0.f},   
  {.5f,   0.f,   0.f,    0.f,   0.f},   
  {0.f,   .5f,   0.f,    0.f,   0.f},   
  {0.f,   0.f,   .25f,   0.f,   0.f},   
  {0.f,   0.f,   .25f,   0.f,   0.f},   
  {0.f,   0.f,   0.f,    0.f,   1.f},  
  {0.f,   0.f,   0.f,    .5f,   0.0} 
};

static float PostmixingMatrix_4_Decorr_11_Output[11][4] = {
  {.25f,   0.f,    0.f,   0.f},  
  {.25f,   0.f,    0.f,   0.f},  
  {0.f,   0.f,    .5f,   0.f},  
  {0.f,   .25f,   0.f,   0.f},  
  {0.f,   .25f,   0.f,   0.f},  
  {.25f,   0.f,    0.f,   0.f},  
  {.25f,   0.f,    0.f,   0.f},  
  {0.f,   .25f,   0.f,   0.f},  
  {0.f,   .25f,   0.f,   0.f},  
  {0.f,   0.f,    0.f,   1.f},  
  {0.f,   0.f,    .5f,   0.0} 
};

/* ======================================================================================================================================================= */

static float PremixingMatrix_5_Decorr_10_Output[5][10] = {
{1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	1.f,	0.f},
};

static float PremixingMatrix_4_Decorr_10_Output[4][10] = {
{1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f,	0.f},
};

static float PremixingMatrix_3_Decorr_10_Output[3][10] = {
{1.f,	1.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f,	0.f},
};

static float PremixingMatrix_2_Decorr_10_Output[2][10] = {
{1.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f,	0.f},
};

static float PostmixingMatrix_5_Decorr_10_Output[10][5] = {
{.5f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f},
{.5f,	0.f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	0.f,	.5f},
{0.f,	0.f,	.5f,	0.f,	0.f}
};

static float PostmixingMatrix_4_Decorr_10_Output[10][4] = {
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	.25f},
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	0.f,	.25f},
{0.f,	0.f,	.5f,	0.f}
};

static float PostmixingMatrix_3_Decorr_10_Output[10][3] = {
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
{0.f,	.5f,	0.f},
{0.f,	0.f,	.25f},
{0.f,	0.f,	.25f},
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
{0.f,	0.f,	.25f},
{0.f,	0.f,	.25f},
{0.f,	.5f,	0.f}
};

static float PostmixingMatrix_2_Decorr_10_Output[10][2] = {
{.17f,	0.f},
{.17f,	0.f},
{.17f,	0.f},
{0.f,	.25f},
{0.f,	.25f},
{.17f,	0.f},
{.17f,	0.f},
{0.f,	.25f},
{0.f,	.25f},
{.17f,	0.f},
};

/* ======================================================================================================================================================= */


/* ======================================================================================================================================================= */

static float PremixingMatrix_5_Decorr_9_Output[5][9] = {
  {1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f},  
  {0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   1.f,   0.f},  
  {0.f,   0.f,   0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   1.f},  
  {0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.0}
};

static float PremixingMatrix_3_Decorr_9_Output[3][9] = {
  {1.f,   1.f,   0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   1.f,   1.f,   0.f,   0.f,   1.f,   1.f},  
  {0.f,   0.f,   1.f,   0.f,   0.f,   0.f,   0.f,   0.f,   0.0}
};

static float PostmixingMatrix_5_Decorr_9_Output[9][5] = {
  {.5f,   0.f,   0.f,   0.f,   0.f},  
  {0.f,   .5f,   0.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   0.f,   1.f},  
  {0.f,   0.f,   .5f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   .5f,   0.f},  
  {.5f,   0.f,   0.f,   0.f,   0.f},  
  {0.f,   .5f,   0.f,   0.f,   0.f},  
  {0.f,   0.f,   .5f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   .5f,   0.0}
};

static float PostmixingMatrix_3_Decorr_9_Output[9][3] = {
  {.25f,  	0.f,   0.f},  
  {.25f,  	0.f,   0.f},  
  {0.f,   0.f,   1.f},  
  {0.f,   .25f,  	0.f},  
  {0.f,   .25f,  	0.f},  
  {.25f,  	0.f,   0.f},  
  {.25f,  	0.f,   0.f},  
  {0.f,   .25f,  	0.f},  
  {0.f,   .25f,  	0.0} 
};

/* ======================================================================================================================================================= */
static float PremixingMatrix_4_Decorr_8_Output[4][8] = {
{1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f},
};

static float PremixingMatrix_3_Decorr_8_Output[3][8] = {
{1.f,	1.f,	0.f,	0.f,	0.f,	1.f,	1.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f},
};

static float PremixingMatrix_2_Decorr_8_Output[2][8] = {
{1.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f,	0.f},
};

static float PostmixingMatrix_2_Decorr_8_Output[8][2] = {
{.17f,	0.f},
{.17f,	0.f},
{.17f,	0.f},
{0.f,	.5f},
{0.f,	.5f},
{.17f,	0.f},
{.17f,	0.f},
{.17f,	0.f},
};


static float PostmixingMatrix_3_Decorr_8_Output[8][3] = {
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
{0.f,	.5f,	0.f},
{0.f,	0.f,	.5f},
{0.f,	0.f,	.5f},
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
{0.f,	.5f,	0.f},
};

static float PostmixingMatrix_4_Decorr_8_Output[8][4] = {
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f},
{0.f,	0.f,	0.f,	.5f},
{0.f,	0.f,	0.f,	.5f},
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	.5f,	0.f},
};
/* ======================================================================================================================================================= */
/* ======================================================================================================================================================= */
static float PremixingMatrix_4_Decorr_7_Output[4][7] = {
{1.f,	0.f,	0.f,	0.f,	0.f,	1.f,	0.f},
{0.f,	1.f,	0.f,	0.f,	0.f,	0.f,	1.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f},
};

static float PremixingMatrix_3_Decorr_7_Output[3][7] = {
{1.f,	1.f,	0.f,	0.f,	0.f,	1.f,	1.f},
{0.f,	0.f,	1.f,	0.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f},
};

static float PremixingMatrix_2_Decorr_7_Output[2][7] = {
{1.f,	1.f,	1.f,	0.f,	0.f,	1.f,	1.f},
{0.f,	0.f,	0.f,	1.f,	1.f,	0.f,	0.f},
};

static float PostmixingMatrix_2_Decorr_7_Output[7][2] = {
{.2f,	0.f},
{.2f,	0.f},
{.2f,	0.f},
{0.f,	.5f},
{0.f,	.5f},
{.2f,	0.f},
{.2f,	0.f},
};

static float PostmixingMatrix_3_Decorr_7_Output[7][3] = {
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
{0.f,	1.f,	0.f},
{0.f,	0.f,	.5f},
{0.f,	0.f,	.5f},
{.25f,	0.f,	0.f},
{.25f,	0.f,	0.f},
};


static float PostmixingMatrix_4_Decorr_7_Output[7][4] = {
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
{0.f,	0.f,	1.f,	0.f},
{0.f,	0.f,	0.f,	.5f},
{0.f,	0.f,	0.f,	.5f},
{.5f,	0.f,	0.f,	0.f},
{0.f,	.5f,	0.f,	0.f},
};


/* ======================================================================================================================================================= */

/* ======================================================================================================================================================= */

static float PremixingMatrix_3_Decorr_5_Output[3][5] = {
{1.f,	1.f,	0.f,	0.f,	0.f},
{0.f,	0.f,	1.f,	0.f,	0.f},
{0.f,	0.f,	0.f,	1.f,	1.f},
};


static float PremixingMatrix_2_Decorr_5_Output[3][5] = {
  {1.f,   1.f,   1.f,   0.f,   0.f},  
  {0.f,   0.f,   0.f,   1.f,   1.0}
};

static float PostmixingMatrix_3_Decorr_5_Output[5][3] = {
{.5f,	0.f,	0.f},
{.5f,	0.f,	0.f},
{0.f,	1.f,	0.f},
{0.f,	0.f,	.5f},
{0.f,	0.f,	.5f},
};

static float PostmixingMatrix_2_Decorr_5_Output[5][2] = {
  {.33f,   0.0f},   
  {.33f,   0.0f},   
  {.33f,   0.0f},   
  {0.0f,   0.5f},   
  {0.0f,   0.5f} 
};

/* ======================================================================================================================================================= */

static float PremixingMatrix_1_Decorr_2_Output[1][2] = {
  {1.f,   1.0}
};

static float PostmixingMatrix_1_Decorr_2_Output[2][1] = {
  {.5f},   
  {.5f} 
};

/* ======================================================================================================================================================= */
#endif
