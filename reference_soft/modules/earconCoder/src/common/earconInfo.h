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
#ifndef EARCON_INFO_H
#define EARCON_INFO_H
/* ######################################################################*/
/* ################################ includes ############################*/
/* ######################################################################*/

/* SYSTEM INCLUDES */
#include <stdio.h>

/* INCLUDES OF THIS PROJECT */

/* OTHER INCLUDES */

/* ######################################################################*/
/* ################################ defines #############################*/
/* ######################################################################*/
#define MAX_NUM_EARCONS (2<<7)
#define MAX_NUM_TEXT_DATA_LENGTH ((2<<8) - 1)
#define MAX_NUM_LANGUAGES ((2<<4) - 1)
/* ######################################################################*/
/* ################################# enums ##############################*/
/* ######################################################################*/

enum EARCON_INFO_RETURN {
  EARCON_INFO_RETURN_NO_ERROR = 0,
  EARCON_INFO_RETURN_UNKNOWN_ERROR
};

/* ######################################################################*/
/* ################################ structs #############################*/
/* ######################################################################*/

struct EARCON_TEXT_LABEL_LANGUAGE {
  char language[3];
  unsigned int textDataLength;
  char textData[MAX_NUM_TEXT_DATA_LENGTH];
};

struct EARCON_TEXT_LABEL {
  unsigned int numLanguages;
  struct EARCON_TEXT_LABEL_LANGUAGE language[MAX_NUM_LANGUAGES];
};

struct EARCON {
  unsigned int isIndependent;
  unsigned int id;
  unsigned int type;
  unsigned int active;
  unsigned int positionType;
  
  unsigned int CICPspeakerIdx;         /**< only used if positionType == 0 */

  unsigned int azimuth;                /**< only used if positionType != 0 */
  unsigned int elevation;              /**< only used if positionType != 0*/
  unsigned int distance;               /**< only used if positionType != 0 */

  unsigned int hasGain;
  unsigned int gain;                   /**< only used if earconHasGain == true */

  unsigned int hasTextLabel;
  struct EARCON_TEXT_LABEL textLabel;  /**< only used if hasTextLabel == true */
};

struct EARCON_INFO {
  unsigned int numEarcons;             /**< ATTENTION: actual number of earcons is numEarcons + 1 */

  struct EARCON earcons[MAX_NUM_EARCONS];
};

/* ######################################################################*/
/* ############################# functions ##############################*/
/* ######################################################################*/

/*! prints out content of the earcon info struct */
enum EARCON_INFO_RETURN earconInfo_Print(
  FILE                     *       file,
  struct EARCON_INFO const * const earconInfo,
  unsigned int               const numSpacesIndentation
);

#endif /* ifndef EARCON_INFO_H */
