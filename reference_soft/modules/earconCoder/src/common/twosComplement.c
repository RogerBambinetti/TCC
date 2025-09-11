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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* INCLUDES OF THIS PROJECT */
#include "twosComplement.h"

/* OTHER INCLUDES */



/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/

#define TWOS_COMPLEMENT_MIN_VALUE(numBits) (- (int) (1<<(numBits-1)))
#define TWOS_COMPLEMENT_MAX_VALUE(numBits) ((1<<(numBits-1)) - 1)

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

/*! rounds value from float to int */
static int roundFloatToInt(float const val){
  if (val < 0.0f){
    return (int)(val - 0.5f);
  } else {
    return (int)(val + 0.5f);
  }
}

/*! converts two's complement to integer */
int twosComplementToInt(unsigned int const tc, size_t const numBits){

  const unsigned int signBitMask = (1<<(numBits - 1));
  const unsigned int signBit = (tc & signBitMask) != 0;
  int number;
  
  if( signBit ){
    unsigned int bitMask = 0;
    size_t n;
    for( n = 0; n < numBits; n++) bitMask += 1<<n;

    number = (int)(~( tc - 1) & bitMask);
    number = - number;
  } else {
    number = (int)tc;
  }

  return number;
}

/*! converts integer to two's complement */
unsigned int intToTwosComplement(int const number, size_t const numBits){
  const unsigned int absNumber = number < 0 ? -number : number;
  unsigned int tc = absNumber;
  
  assert( numBits <= sizeof(tc) * CHAR_BIT);
  assert( number >= TWOS_COMPLEMENT_MIN_VALUE(numBits) && number <= TWOS_COMPLEMENT_MAX_VALUE(numBits));
  
  if( number < 0 ){
    tc = ~tc;
    tc++;
  }

  return tc;
}

/*! converts two's complement to float in range [-1;1] */
float twosComplementToFloat(unsigned int const tc, size_t const numBits){
  const int intNumber = twosComplementToInt(tc, numBits);
  float number;
  if( intNumber >= 0 ){
    number = (float)intNumber / (float)TWOS_COMPLEMENT_MAX_VALUE(numBits);
  } else {
    number = -(float)intNumber / (float)(TWOS_COMPLEMENT_MIN_VALUE(numBits));
  }
  
  assert( number <= 1.0f && number >= -1.0f );
  
  return number;
}

/*! converts float in range [-1;1] to two's complement */
unsigned int floatToTwosComplement(float const number, size_t const numBits){
  int intNumber;
  
  assert( number <= 1.0f && number >= -1.0f );
  
  if( number >= 0 ){
    intNumber = roundFloatToInt(number * (float)TWOS_COMPLEMENT_MAX_VALUE(numBits));
  } else {
    intNumber = -roundFloatToInt(number * (float)(TWOS_COMPLEMENT_MIN_VALUE(numBits)));
  }
  
  return intToTwosComplement(intNumber, numBits);
}

