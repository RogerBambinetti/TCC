/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its performance may not have been optimized. This software module is an
implementation of one or more tools as specified by the ISO/IEC 23008-3
standard. ISO/IEC gives you a royalty-free, worldwide, non-exclusive,
copyright license to copy, distribute, and make derivative works of this 
software module or modifications thereof for use in implementations or
products claiming conformance to the ISO/IEC 23008-3 standard and which
satisfy any specified conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2018.

*************************************************************************/

/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/
/* SYSTEM INCLUDES */
#include <assert.h>

/* INCLUDES OF THIS PROJECT */
#include "printHelper.h"

/* OTHER INCLUDES */


/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

/* ######################################################################*/
/* ########################## static functions ##########################*/
/* ######################################################################*/

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/* works exactly like fprintf but allows for easy indentation of the whole printout */
int fprintf_indented(FILE * stream, unsigned int numSpacesIndentation, char const * const fmt, ...){
  int ret = 0;
  assert(fmt);
  {
    va_list args;
    unsigned int n;
    for( n = 0; n < numSpacesIndentation; n++) ret += fprintf(stream, " ");
    va_start (args, fmt);
    ret += vfprintf (stream, fmt, args);
    va_end (args);
  }

  return ret;
}

