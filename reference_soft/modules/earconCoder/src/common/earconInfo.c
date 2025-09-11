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



/* INCLUDES OF THIS PROJECT */
#include "earconInfo.h"
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

static int isError(enum EARCON_INFO_RETURN retval){
  return (retval != EARCON_INFO_RETURN_NO_ERROR);
}

static void earconInfo_Print_binary(
  FILE                     *       file,
  struct EARCON_INFO const * const earconInfo
){
  enum EARCON_INFO_RETURN retVal = EARCON_INFO_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! earconInfo ){
    retVal = EARCON_INFO_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT EARCON INFO */
  if( ! isError(retVal) ){
    unsigned int i;

    fwrite(&earconInfo->numEarcons, sizeof(unsigned int), 1, file);

    for( i = 0; i < earconInfo->numEarcons + 1; i++){
      fwrite(&earconInfo->earcons[i].isIndependent, sizeof(unsigned int), 1, file);
      fwrite(&earconInfo->earcons[i].id, sizeof(unsigned int), 1, file);
      fwrite(&earconInfo->earcons[i].type, sizeof(unsigned int), 1, file);
      fwrite(&earconInfo->earcons[i].active, sizeof(unsigned int), 1, file);
      fwrite(&earconInfo->earcons[i].positionType, sizeof(unsigned int), 1, file);

      if( earconInfo->earcons[i].positionType == 0){
        fwrite(&earconInfo->earcons[i].CICPspeakerIdx, sizeof(unsigned int), 1, file);
      } else {
        if( earconInfo->earcons[i].positionType == 1){
          fwrite(&earconInfo->earcons[i].azimuth, sizeof(unsigned int), 1, file);
          fwrite(&earconInfo->earcons[i].elevation, sizeof(unsigned int), 1, file);
          fwrite(&earconInfo->earcons[i].distance, sizeof(unsigned int), 1, file);
        } else {
          fwrite(&earconInfo->earcons[i].azimuth, sizeof(unsigned int), 1, file);
          fwrite(&earconInfo->earcons[i].elevation, sizeof(unsigned int), 1, file);
          fwrite(&earconInfo->earcons[i].distance, sizeof(unsigned int), 1, file);
        }
      }

      fwrite(&earconInfo->earcons[i].hasGain, sizeof(unsigned int), 1, file);
      if( earconInfo->earcons[i].hasGain ){
        fwrite(&earconInfo->earcons[i].gain, sizeof(unsigned int), 1, file);
      }

      fwrite(&earconInfo->earcons[i].hasTextLabel, sizeof(unsigned int), 1, file);

      if( earconInfo->earcons[i].hasTextLabel ){
        unsigned int n;

        fwrite(&earconInfo->earcons[i].textLabel.numLanguages, sizeof(unsigned int), 1, file);
        for( n = 0; n < earconInfo->earcons[i].textLabel.numLanguages; n++){
          char tmp[4] = {0};
          char tmpLabel[MAX_NUM_TEXT_DATA_LENGTH+1] = {0};
          memcpy(tmp, earconInfo->earcons[i].textLabel.language[n].language, sizeof(earconInfo->earcons[i].textLabel.language[n].language));
          memcpy(tmpLabel, earconInfo->earcons[i].textLabel.language[n].textData, sizeof(char) * earconInfo->earcons[i].textLabel.language[n].textDataLength);

          fwrite(tmp, sizeof(char), 4, file);
          fwrite(&earconInfo->earcons[i].textLabel.language[n].textDataLength, sizeof(unsigned int), 1, file);
          fwrite(tmpLabel, sizeof(char), earconInfo->earcons[i].textLabel.language[n].textDataLength, file);
        } /* for n */
      }
    } /* for i */
  }
}

/* ######################################################################*/
/* ######################## non-static functions ########################*/
/* ######################################################################*/

/*! prints out content of the earcon info struct */
enum EARCON_INFO_RETURN earconInfo_Print(
  FILE                     *       file,
  struct EARCON_INFO const * const earconInfo,
  unsigned int               const numSpacesIndentation
){
  enum EARCON_INFO_RETURN retVal = EARCON_INFO_RETURN_NO_ERROR;

  /* SANITY CHECKS */
  if( ! earconInfo ){
    retVal = EARCON_INFO_RETURN_UNKNOWN_ERROR;
  }

  /* PRINT EARCON INFO */
  if( ! isError(retVal) ){
    if (stdout == file) {
      unsigned int i;
      fprintf_indented(file, numSpacesIndentation, "numEarcons = %u\n", earconInfo->numEarcons);

      for( i = 0; i < earconInfo->numEarcons + 1; i++){
        fprintf_indented(file, numSpacesIndentation, "earconIsIndependent[%u] = %u\n", i, earconInfo->earcons[i].isIndependent);
        fprintf_indented(file, numSpacesIndentation, "earconID[%u] = %u\n", i, earconInfo->earcons[i].id);
        fprintf_indented(file, numSpacesIndentation, "earconType[%u] = %u\n", i, earconInfo->earcons[i].type);
        fprintf_indented(file, numSpacesIndentation, "earconActive[%u] = %u\n", i, earconInfo->earcons[i].active);
        fprintf_indented(file, numSpacesIndentation, "earconPositionType[%u] = %u\n", i, earconInfo->earcons[i].positionType);

        if( earconInfo->earcons[i].positionType == 0){
          fprintf_indented(file, numSpacesIndentation, "earcon_CICPspeakerIdx[%u] = %u\n", i, earconInfo->earcons[i].CICPspeakerIdx);
        } else {
          if( earconInfo->earcons[i].positionType == 1){
            fprintf_indented(file, numSpacesIndentation, "earcon_azimuth[%u] = %u\n", i, earconInfo->earcons[i].azimuth);
            fprintf_indented(file, numSpacesIndentation, "earcon_elevation[%u] = %u\n", i, earconInfo->earcons[i].elevation);
            fprintf_indented(file, numSpacesIndentation, "earcon_distance[%u] = %u\n", i, earconInfo->earcons[i].distance);
          } else {
            fprintf_indented(file, numSpacesIndentation, "earcon_azimuth[%u] = %u\n", i, earconInfo->earcons[i].azimuth);
            fprintf_indented(file, numSpacesIndentation, "earcon_elevation[%u] = %u\n", i, earconInfo->earcons[i].elevation);
            fprintf_indented(file, numSpacesIndentation, "earcon_distance[%u] = %u\n", i, earconInfo->earcons[i].distance);
          }
        }
    

        fprintf_indented(file, numSpacesIndentation, "earconHasGain[%u] = %u\n", i, earconInfo->earcons[i].hasGain);
        if( earconInfo->earcons[i].hasGain ){
          fprintf_indented(file, numSpacesIndentation, "earcon_gain[%u] = %u\n", i, earconInfo->earcons[i].gain);
        }

        fprintf_indented(file, numSpacesIndentation, "earconHasTextLabel[%u] = %u\n", i, earconInfo->earcons[i].hasTextLabel);

        if( earconInfo->earcons[i].hasTextLabel ){
          unsigned int n;
          fprintf_indented(file, numSpacesIndentation, "earconNumLanguages[%u] = %u\n", i, earconInfo->earcons[i].textLabel.numLanguages);
          for( n = 0; n < earconInfo->earcons[i].textLabel.numLanguages; n++){
            char tmp[4] = {0};
            char tmpLabel[MAX_NUM_TEXT_DATA_LENGTH+1] = {0};
            memcpy(tmp, earconInfo->earcons[i].textLabel.language[n].language, sizeof(earconInfo->earcons[i].textLabel.language[n].language));
            memcpy(tmpLabel, earconInfo->earcons[i].textLabel.language[n].textData, sizeof(char) * earconInfo->earcons[i].textLabel.language[n].textDataLength);
            fprintf_indented(file, numSpacesIndentation, "earconLanguage[%u][%u] = %s\n", i, n, tmp);
            fprintf_indented(file, numSpacesIndentation, "earconTextDataLength[%u][%u] = %u\n", i, n, earconInfo->earcons[i].textLabel.language[n].textDataLength);
            fprintf_indented(file, numSpacesIndentation, "earconTextData[%u][%u] = %s\n", i, n, tmpLabel);
          } /* for n */
        }
      } /* for i */
    } else {
      earconInfo_Print_binary(file, earconInfo);
    }
  }

  return retVal;
}
