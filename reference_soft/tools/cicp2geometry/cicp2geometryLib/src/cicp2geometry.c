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


/* system includes */
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>


/* locals includes */
#include "cicp2geometry.h"

/**********************************************************************//**

**************************************************************************/

enum {
  C2G_EMPTY   = 0, /**< no loudspeakers */
  C2G_MONO    = 1, /**<  mono   */
  C2G_STEREO  = 2, /**< stereo  */
  C2G_5_1     = 6, /**<   5.1   */
  C2G_7_1     = 8, /**<   7.1   */
  C2G_22_2    = 24 /**<  22.2   */
};

/**********************************************************************//**

**************************************************************************/

const CICP2GEOMETRY_CHANNEL_GEOMETRY CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLESIZE] =
{/* idx  az    el lfe screenRel   loudspeaker type             informative label (merely descriptive) */
  { 0,   30,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front                         */
  { 1,  -30,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front                        */
  { 2,    0,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre front                       */
  { 3,    0, -15,  1,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< Front LFE                          */
  { 4,  110,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left surround                      */
  { 5, -110,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround                     */
  { 6,   22,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front centre                  */
  { 7,  -22,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front centre                 */
  { 8,  135,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround left 1               */
  { 9, -135,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround right 1              */
  {10,  180,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear centre                        */
  {11,   -1,  -1, -1, -1  , CICP2GEOMETRY_LOUDSPEAKER_INVALID}, /**< reserved                         */
  {12,   -1,  -1, -1, -1  , CICP2GEOMETRY_LOUDSPEAKER_INVALID}, /**< reserved                         */
  {13,   90,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left side surround                 */
  {14,  -90,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right side surround                */
  {15,   60,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front wide                    */
  {16,  -60,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front wide                   */
  {17,   30,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical height 1       */
  {18,  -30,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical height 1      */
  {19,    0,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre front vertical height       */
  {20,  135,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left surround vertical height rear */
  {21, -135,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround vertical height rear*/
  {22,  180,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre vertical height rear        */
  {23,   90,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left vertical height side surround */
  {24,  -90,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right vertical height side surround*/
  {25,    0,  90,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< top centre surround                */
  {26,   45, -15,  1,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front LFE                     */
  {27,   45, -15,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical bottom         */
  {28,  -45, -15,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical bottom        */
  {29,    0, -15,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< centre front vertical bottom       */
  {30,  110,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left surround vertical height      */
  {31, -110,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right surround vertical height     */
  {32,   45,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front vertical height  2      */
  {33,  -45,  35,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front vertical height 2      */
  {34,   45,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left front 2                       */
  {35,  -45,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front 2                      */
  {36,  -45, -15,  1,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right front LFE                    */
  {37,   60,   0,  0,  1  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< left edge of display               */
  {38,  -60,   0,  0,  1  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< right edge of display              */
  {39,   30,   0,  0,  1  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< half-way btw. centre of display and left edge of display */
  {40,  -30,   0,  0,  1  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< half-way btw. centre of display and right edge of display */
  {41,  150,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround left 2               */
  {42, -150,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< rear surround right 2              */
#ifdef RM6_INTERNAL_CHANNEL
  {43,    0,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< internal channel center             */
  {44,    0, -15,  1,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< internal channel lfe                */
  {45,   30,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< internal channel left               */
  {46,  -30,   0,  0,  0  , CICP2GEOMETRY_LOUDSPEAKER_KNOWN}, /**< internal channel right              */
#endif
};


/**********************************************************************//**

**************************************************************************/

static int matchLoudspeakers( const CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe, const CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe ) {
  
  int error = 0;
  
  error = ( tmpAzElLfe.Az != AzElLfe.Az );
  error |= ( tmpAzElLfe.El != AzElLfe.El );
  error |= ( tmpAzElLfe.LFE != AzElLfe.LFE );
  error |= ( tmpAzElLfe.screenRelative != AzElLfe.screenRelative );

  return error;

}

/**********************************************************************//**

**************************************************************************/

static int matchLoudspeakerSetups( CICP2GEOMETRY_CHANNEL_GEOMETRY * const AzElLfe, 
                                   const CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS], 
                                   const int numChannels, 
                                   const int tmpNumChannels, 
                                   const int numLFEs, 
                                   const int tmpNumLFEs, 
                                   const int numTotalChannels ) {

  int error = 0;
  int ch = 0;

  if ( numChannels != tmpNumChannels || numLFEs != tmpNumLFEs ) {
    error = 1;
    return error;
  }
      
  for ( ch = 0; ch < numTotalChannels; ++ch ) {
    error = matchLoudspeakers( tmpAzElLfe[ch], AzElLfe[ch] );
    if ( error ) {
      break;
    }
  }

  return error;
}

/**********************************************************************//**

**************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_cicpIndex_from_geometry( CICP2GEOMETRY_CHANNEL_GEOMETRY * AzElLfe, int numChannels, int numLFEs, int * cicpIndex) {

  CICP2GEOMETRY_CHANNEL_GEOMETRY tmpAzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
  int tmpNumChannels = 0;
  int tmpNumLFEs = 0;
  int numTotalChannels = numChannels + numLFEs;
  int error = 0;

  switch (numTotalChannels) {

    case C2G_EMPTY:
      fprintf(stderr, "Error in cicp2geometry_get_cicpIndex_from_geometry: No channels given!\n");
      error = 1;
      break;

    case C2G_MONO:

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_1_0_0, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_1_0_0;
      }
      break;


    case C2G_STEREO:

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_2_0_0, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_2_0_0;
      }
      break;

    case C2G_5_1:

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_3_2_1, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_3_2_1;
      }
      break;

    case C2G_7_1:

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_5_2_1, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_5_2_1;
        break;
      }

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_3_4_1, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_3_4_1;
        break;
      }

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_5_2_1_ELEVATION, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      

      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_5_2_1_ELEVATION;
      }
      break;



    case C2G_22_2:

      cicp2geometry_get_geometry_from_cicp( CICP2GEOMETRY_CICP_11_11_2, tmpAzElLfe, &tmpNumChannels, &tmpNumLFEs );

      error = matchLoudspeakerSetups( AzElLfe, 
                                      tmpAzElLfe, 
                                      numChannels, 
                                      tmpNumChannels, 
                                      numLFEs, 
                                      tmpNumLFEs,
                                      numTotalChannels );
      
      if ( error == 0 ) {
        *cicpIndex = CICP2GEOMETRY_CICP_11_11_2;
      }
      break;


    default:
      fprintf(stderr, "Wrong number of channels!\n");
      error = 1;
      break;

  }

  if ( error ) {

    fprintf(stderr, "No matching cicp index available!\n");
    *cicpIndex = CICP2GEOMETRY_CICP_INVALID;

    return CICP2GEOMETRY_INVALID_CICP_INDEX;
  }

  return CICP2GEOMETRY_OK;

}

/**********************************************************************//**

**************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp( int cicpIndex, CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS], int * numChannels, int * numLFEs ) {

  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;

  switch ( cicpIndex ) {

    /*  CICP index 1: 1.0 - mono */
    case CICP2GEOMETRY_CICP_1_0_0:

      if ( AzElLfe != NULL )
      { 
        /* centre front */
        AzElLfe[0].Az  = 0;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;
      }

      *numChannels = 1;
      *numLFEs = 0;
      break;

      /*  CICP index 2: 2.0 - stereo */
    case CICP2GEOMETRY_CICP_2_0_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;
      }

      *numChannels = 2;
      *numLFEs = 0;
      break;

      /*  CICP index 3: 3.0 */
    case CICP2GEOMETRY_CICP_3_0_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;
      }

      *numChannels = 3;
      *numLFEs = 0;
      break;

      /*  CICP index 4: 4.0 */
    case CICP2GEOMETRY_CICP_3_1_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* rear centre */
        AzElLfe[3].Az  = 180;
        AzElLfe[3].El  = 0;
        AzElLfe[3].LFE = 0;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;
      }

      *numChannels = 4;
      *numLFEs = 0;
      break;

      /*  CICP index 5: 5.0 */
    case CICP2GEOMETRY_CICP_3_2_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* left surround */
        AzElLfe[3].Az  = 110;
        AzElLfe[3].El  = 0;
        AzElLfe[3].LFE = 0;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* right surround */
        AzElLfe[4].Az  = -110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;
      }

      *numChannels = 5;
      *numLFEs = 0;
      break;

      /*  CICP index 6: 5.1 */
    case CICP2GEOMETRY_CICP_3_2_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;
      }

      *numChannels = 5;
      *numLFEs = 1;
      break;


      /*  CICP index 7: 7.1 Front Centre */
    case CICP2GEOMETRY_CICP_5_2_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left front wide */
        AzElLfe[6].Az  = 60;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right front wide  */
        AzElLfe[7].Az  = -60;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;
      }

      *numChannels = 7;
      *numLFEs = 1;
      break;

      /*  CICP index 8: n.a. */
    case CICP2GEOMETRY_CICP_1_1:
      *numChannels = -1;
      *numLFEs = -1;
      break;

      /*  CICP index 9: 3.0 */
    case CICP2GEOMETRY_CICP_2_1_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* rear centre */
        AzElLfe[2].Az  = 180;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;
      }

      *numChannels = 3;
      *numLFEs = 0;
      break;


      /*  CICP index 10: 4.0 */
    case CICP2GEOMETRY_CICP_2_2_0:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* left surround */
        AzElLfe[2].Az  = 110;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* right surround */
        AzElLfe[3].Az  = -110;
        AzElLfe[3].El  = 0;
        AzElLfe[3].LFE = 0;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;
      }

      *numChannels = 4;
      *numLFEs = 0;
      break;


      /*  CICP index 11: 6.1 */
    case CICP2GEOMETRY_CICP_3_3_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* rear centre */
        AzElLfe[6].Az  = 180;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;
      }

      *numChannels = 6;
      *numLFEs = 1;
      break;

      /*  CICP index 12: 7.1 rear surround */
    case CICP2GEOMETRY_CICP_3_4_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* rear surround left */
        AzElLfe[6].Az  = 135;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* rear surround right */
        AzElLfe[7].Az  = -135;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;
      }

      *numChannels = 7;
      *numLFEs = 1;
      break;


      /*  CICP index 13: 22.2 */
    case CICP2GEOMETRY_CICP_11_11_2:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 60;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -60;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* left front LFE */
        AzElLfe[3].Az  = 45;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* rear left surround */
        AzElLfe[4].Az  = 135;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* rear right surround */
        AzElLfe[5].Az  = -135;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left front centre */
        AzElLfe[6].Az  = 30;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right front centre */
        AzElLfe[7].Az  = -30;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* rear centre */
        AzElLfe[8].Az  = 180;
        AzElLfe[8].El  = 0;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* right front LFE */
        AzElLfe[9].Az  = -45;
        AzElLfe[9].El  = -15;
        AzElLfe[9].LFE = 1;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* left side surround */
        AzElLfe[10].Az  = 90;
        AzElLfe[10].El  = 0;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* right side surround */
        AzElLfe[11].Az  = -90;
        AzElLfe[11].El  = 0;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[12].Az  = 45;
        AzElLfe[12].El  = 35;
        AzElLfe[12].LFE = 0;
        AzElLfe[12].cicpLoudspeakerIndex = -1;
        AzElLfe[12].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[12].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[13].Az  = -45;
        AzElLfe[13].El  = 35;
        AzElLfe[13].LFE = 0;
        AzElLfe[13].cicpLoudspeakerIndex = -1;
        AzElLfe[13].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[13].screenRelative = 0;

        /* centre front vertical height */
        AzElLfe[14].Az  = 0;
        AzElLfe[14].El  = 35;
        AzElLfe[14].LFE = 0;
        AzElLfe[14].cicpLoudspeakerIndex = -1;
        AzElLfe[14].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[14].screenRelative = 0;

        /* top centre surround */
        AzElLfe[15].Az  = 0;
        AzElLfe[15].El  = 90;
        AzElLfe[15].LFE = 0;
        AzElLfe[15].cicpLoudspeakerIndex = -1;
        AzElLfe[15].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[15].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[16].Az  = 135;
        AzElLfe[16].El  = 35;
        AzElLfe[16].LFE = 0;
        AzElLfe[16].cicpLoudspeakerIndex = -1;
        AzElLfe[16].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[16].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[17].Az  = -135;
        AzElLfe[17].El  = 35;
        AzElLfe[17].LFE = 0;
        AzElLfe[17].cicpLoudspeakerIndex = -1;
        AzElLfe[17].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[17].screenRelative = 0;

        /* left vertical height side surround */
        AzElLfe[18].Az  = 90;
        AzElLfe[18].El  = 35;
        AzElLfe[18].LFE = 0;
        AzElLfe[18].cicpLoudspeakerIndex = -1;
        AzElLfe[18].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[18].screenRelative = 0;

        /* left vertical height side surround */
        AzElLfe[19].Az  = -90;
        AzElLfe[19].El  = 35;
        AzElLfe[19].LFE = 0;
        AzElLfe[19].cicpLoudspeakerIndex = -1;
        AzElLfe[19].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[19].screenRelative = 0;

        /* centre vertical height rear */
        AzElLfe[20].Az  = 180;
        AzElLfe[20].El  = 35;
        AzElLfe[20].LFE = 0;
        AzElLfe[20].cicpLoudspeakerIndex = -1;
        AzElLfe[20].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[20].screenRelative = 0;

        /* centre front vertical bottom */
        AzElLfe[21].Az  = 0;
        AzElLfe[21].El  = -15;
        AzElLfe[21].LFE = 0;
        AzElLfe[21].cicpLoudspeakerIndex = -1;
        AzElLfe[21].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[21].screenRelative = 0;

        /* left front vertical bottom */
        AzElLfe[22].Az  = 45;
        AzElLfe[22].El  = -15;
        AzElLfe[22].LFE = 0;
        AzElLfe[22].cicpLoudspeakerIndex = -1;
        AzElLfe[22].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[22].screenRelative = 0;

        /* right front vertical bottom */
        AzElLfe[23].Az  = -45;
        AzElLfe[23].El  = -15;
        AzElLfe[23].LFE = 0;
        AzElLfe[23].cicpLoudspeakerIndex = -1;
        AzElLfe[23].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[23].screenRelative = 0;
      }

      *numChannels = 22;
      *numLFEs = 2;
      break;

      /*  CICP index 14: 7.1 front elevation */
    case CICP2GEOMETRY_CICP_5_2_1_ELEVATION:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* left front LFE */
        AzElLfe[3].Az  = 45;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[6].Az  = 30;
        AzElLfe[6].El  = 35;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[7].Az  = -30;
        AzElLfe[7].El  = 35;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;
      }

      *numChannels = 7;
      *numLFEs = 1;
      break;

      /*  CICP index 15: 10.2 */
    case CICP2GEOMETRY_CICP_5_5_2:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 45;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 135;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -135;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* right front LFE */
        AzElLfe[6].Az  = -45;
        AzElLfe[6].El  = -15;
        AzElLfe[6].LFE = 1;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* left side surround */
        AzElLfe[7].Az  = 90;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* right side surround */
        AzElLfe[8].Az  = -90;
        AzElLfe[8].El  = 0;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[9].Az  = 45;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[10].Az  = -45;
        AzElLfe[10].El  = 35;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* centre vertical height rear */
        AzElLfe[11].Az  = 180;
        AzElLfe[11].El  = 35;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;
      }

      *numChannels = 10;
      *numLFEs = 2;
      break;

      /*  CICP index 16: 9.1 */
    case CICP2GEOMETRY_CICP_5_4_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[6].Az  = 30;
        AzElLfe[6].El  = 35;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[7].Az  = -30;
        AzElLfe[7].El  = 35;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[8].Az  = 110;
        AzElLfe[8].El  = 35;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[9].Az  = -110;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;
      }

      *numChannels = 9;
      *numLFEs = 1;
      break;


      /*  CICP index 17: 11.1 */
    case CICP2GEOMETRY_CICP_6_5_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[6].Az  = 30;
        AzElLfe[6].El  = 35;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[7].Az  = -30;
        AzElLfe[7].El  = 35;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* centre front vertical height */
        AzElLfe[8].Az  = 0;
        AzElLfe[8].El  = 35;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[9].Az  = 110;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[10].Az  = -110;
        AzElLfe[10].El  = 35;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* top centre surround */
        AzElLfe[11].Az  = 0;
        AzElLfe[11].El  = 90;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;
      }

      *numChannels = 11;
      *numLFEs = 1;
      break;


      /*  CICP index 18: 13.1 */
    case CICP2GEOMETRY_CICP_6_7_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 110;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -110;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left surround */
        AzElLfe[6].Az  = 150;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right surround */
        AzElLfe[7].Az  = -150;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[8].Az  = 30;
        AzElLfe[8].El  = 35;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[9].Az  = -30;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* centre front vertical height */
        AzElLfe[10].Az  = 0;
        AzElLfe[10].El  = 35;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[11].Az  = 110;
        AzElLfe[11].El  = 35;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[12].Az  = -110;
        AzElLfe[12].El  = 35;
        AzElLfe[12].LFE = 0;
        AzElLfe[12].cicpLoudspeakerIndex = -1;
        AzElLfe[12].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[12].screenRelative = 0;

        /* top centre surround */
        AzElLfe[13].Az  = 0;
        AzElLfe[13].El  = 90;
        AzElLfe[13].LFE = 0;
        AzElLfe[13].cicpLoudspeakerIndex = -1;
        AzElLfe[13].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[13].screenRelative = 0;
      }

      *numChannels = 13;
      *numLFEs = 1;
      break;

      /*  CICP index 19: 7.1 + 4 */
    case CICP2GEOMETRY_CICP_7_4_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 135;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -135;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left surround */
        AzElLfe[6].Az  = 90;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right surround */
        AzElLfe[7].Az  = -90;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[8].Az  = 30;
        AzElLfe[8].El  = 35;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[9].Az  = -30;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[10].Az  = 135;
        AzElLfe[10].El  = 35;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[11].Az  = -135;
        AzElLfe[11].El  = 35;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;

      }

      *numChannels = 11;
      *numLFEs = 1;
      break;


    /*  CICP index 20: 9.1 + 4 */
    case CICP2GEOMETRY_CICP_9_4_1:

      if ( AzElLfe != NULL )
      { 
        /* left front */
        AzElLfe[0].Az  = 30;
        AzElLfe[0].El  = 0;
        AzElLfe[0].LFE = 0;
        AzElLfe[0].cicpLoudspeakerIndex = -1;
        AzElLfe[0].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[0].screenRelative = 0;

        /* right front */
        AzElLfe[1].Az  = -30;
        AzElLfe[1].El  = 0;
        AzElLfe[1].LFE = 0;
        AzElLfe[1].cicpLoudspeakerIndex = -1;
        AzElLfe[1].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[1].screenRelative = 0;

        /* centre front */
        AzElLfe[2].Az  = 0;
        AzElLfe[2].El  = 0;
        AzElLfe[2].LFE = 0;
        AzElLfe[2].cicpLoudspeakerIndex = -1;
        AzElLfe[2].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[2].screenRelative = 0;

        /* LFE */
        AzElLfe[3].Az  = 0;
        AzElLfe[3].El  = -15;
        AzElLfe[3].LFE = 1;
        AzElLfe[3].cicpLoudspeakerIndex = -1;
        AzElLfe[3].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[3].screenRelative = 0;

        /* left surround */
        AzElLfe[4].Az  = 135;
        AzElLfe[4].El  = 0;
        AzElLfe[4].LFE = 0;
        AzElLfe[4].cicpLoudspeakerIndex = -1;
        AzElLfe[4].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[4].screenRelative = 0;

        /* right surround */
        AzElLfe[5].Az  = -135;
        AzElLfe[5].El  = 0;
        AzElLfe[5].LFE = 0;
        AzElLfe[5].cicpLoudspeakerIndex = -1;
        AzElLfe[5].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[5].screenRelative = 0;

        /* left surround */
        AzElLfe[6].Az  = 90;
        AzElLfe[6].El  = 0;
        AzElLfe[6].LFE = 0;
        AzElLfe[6].cicpLoudspeakerIndex = -1;
        AzElLfe[6].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[6].screenRelative = 0;

        /* right surround */
        AzElLfe[7].Az  = -90;
        AzElLfe[7].El  = 0;
        AzElLfe[7].LFE = 0;
        AzElLfe[7].cicpLoudspeakerIndex = -1;
        AzElLfe[7].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[7].screenRelative = 0;

        /* left front vertical height */
        AzElLfe[8].Az  = 45;
        AzElLfe[8].El  = 35;
        AzElLfe[8].LFE = 0;
        AzElLfe[8].cicpLoudspeakerIndex = -1;
        AzElLfe[8].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[8].screenRelative = 0;

        /* right front vertical height */
        AzElLfe[9].Az  = -45;
        AzElLfe[9].El  = 35;
        AzElLfe[9].LFE = 0;
        AzElLfe[9].cicpLoudspeakerIndex = -1;
        AzElLfe[9].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[9].screenRelative = 0;

        /* left surround vertical height rear */
        AzElLfe[10].Az  = 135;
        AzElLfe[10].El  = 35;
        AzElLfe[10].LFE = 0;
        AzElLfe[10].cicpLoudspeakerIndex = -1;
        AzElLfe[10].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[10].screenRelative = 0;

        /* right surround vertical height rear */
        AzElLfe[11].Az  = -135;
        AzElLfe[11].El  = 35;
        AzElLfe[11].LFE = 0;
        AzElLfe[11].cicpLoudspeakerIndex = -1;
        AzElLfe[11].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[11].screenRelative = 0;

        /* left front */
        AzElLfe[12].Az  = 60;
        AzElLfe[12].El  = 0;
        AzElLfe[12].LFE = 0;
        AzElLfe[12].cicpLoudspeakerIndex = -1;
        AzElLfe[12].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[12].screenRelative = 1;

        /* right front */
        AzElLfe[13].Az  = -60;
        AzElLfe[13].El  = 0;
        AzElLfe[13].LFE = 0;
        AzElLfe[13].cicpLoudspeakerIndex = -1;
        AzElLfe[13].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;
        AzElLfe[13].screenRelative = 1;

      }

      *numChannels = 13;
      *numLFEs = 1;
      break;

    default:
      *numChannels = -1;
      *numLFEs = -1;
      fprintf(stderr, "ERROR: get_geometry_from_cicp() !\n");
      return CICP2GEOMETRY_INVALID_CICP_INDEX;
  }

  return cicpError;
}

/**********************************************************************//**

**************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_file( char* geometryInputFile, CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS], int * numChannels, int * numLFEs ) {
  
  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;
  FILE* fileHandle;
  char line[512] = {0};
  int i, ch;
  int verboseLvl = 0;

  if ( AzElLfe == NULL ) {
    if (verboseLvl >= 1) {
      fprintf( stderr, "Bad pointer in cicp2geometry_get_geometry_from_cicp_loudspeaker_index()!\n");
    }
    return CICP2GEOMETRY_BAD_POINTER;
  }

  /* open file */
  fileHandle = fopen(geometryInputFile, "r");    
  if ( !fileHandle )
  {
    if (verboseLvl >= 1) {
      fprintf(stderr,"Error: Unable to open channel list file (%s).\n",geometryInputFile);        
    }
    return CICP2GEOMETRY_INIT_ERROR;
  } else {
    if (verboseLvl >= 2) {
      fprintf(stdout, "\nInfo: Found channel list file: %s.\n", geometryInputFile );
    }
  }

  /* Init channel counters */
  *numChannels = 0;
  *numLFEs     = 0;
    
  /* Get new line */
  if (verboseLvl >= 2) {
    fprintf(stdout, "Info: Reading channel list file ... \n");    
  }
  ch = 0;
  while ( fgets(line, 511, fileHandle) != NULL )
  {
    char* pChar = line;
    i = 0;
      
    /* Add newline at end of line (for eof line), terminate string after newline */
    line[strlen(line)+1] = '\0';
    line[strlen(line)] = '\n';      
    
    /* Replace all white spaces with new lines for easier parsing */
    while ( pChar[i] != '\0') {

      if ( pChar[i] == ' ' || pChar[i] == '\t') {
        pChar[i] = '\n';            
      }

      i++;
    }
                
    /* Parse line */        
    pChar = line;
    while ( (*pChar) != '\0') {

      /****************************************************  
        SANITY CHECK: break when all loudspeakers are read
      *****************************************************/
      if( ( *numChannels > 0 ) && ( ch >= CICP2GEOMETRY_MAX_LOUDSPEAKERS ) ) {
        if (verboseLvl >= 1) {
          fprintf(stderr,"Error: Wrong channel list file format.\n");
        }
        cicpError = CICP2GEOMETRY_INVALID_GEO_FILE_SYNTAX;
        break;
      }

      while (  (*pChar) == '\n' || (*pChar) == '\r'  )
        pChar++;

      /***********************************************  
        FIRST ENTRY OF FILE: read number of channels
      ***********************************************/
      if (*numChannels == 0) {         

        *numChannels = atoi(pChar);

        if ( ( *numChannels < 1 ) || ( *numChannels > CICP2GEOMETRY_MAX_LOUDSPEAKERS ) ) { /* sanity check for reasonable number of channels */
          if (verboseLvl >= 1) {
            fprintf(stderr,"Error: Read wrong number of channels: %d. Valid range is [1 ... 32].\n",*numChannels);
          }
          *numChannels = CICP2GEOMETRY_MAX_LOUDSPEAKERS;
          cicpError = CICP2GEOMETRY_INVALID_GEO_FILE_SYNTAX;
        }
        else {
          if (verboseLvl >= 2) {
            fprintf(stdout,"Reading indices for %d channels.\n",*numChannels);
          }
        }

      } 
      else { /* read channel definition */

        /***********************************************  
             CICP LOUDSPEAKER INDEX
         ***********************************************/
        if( (*pChar) == 'c' ){ 

          int CicpChannelIndex = -1;

          pChar++; pChar++;

          while ( (*pChar) == '\n' || (*pChar) == '\r' )
            pChar++;

          /***************************************
             CICP LOUDSPEAKER INDEX SANITY CHECK
           ***************************************/

          CicpChannelIndex = atoi(pChar);

          /* init loudspeaker type */
          AzElLfe[ch].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;

          /******************************************
             GET GEOMETRY FROM CICP LOUDSPEAKER INDEX
           ******************************************/

          if ( cicp2geometry_get_geometry_from_cicp_loudspeaker_index( CicpChannelIndex , &AzElLfe[ch] ) ) {
            if ( cicpError == CICP2GEOMETRY_OK ) {
              cicpError = CICP2GEOMETRY_UNKNOWN_SPEAKER_INDEX;
            }
            if (verboseLvl >= 1) {
              fprintf(stderr, "Error in cicp2geometry_get_geometry_from_cicp_loudspeaker_index() !\n");
            }
          }

          /************************
                  LFE HANDLING
           ************************/
          if ( AzElLfe[ch].LFE == 1 ) {
             (*numLFEs)++;
          }

        }

        /***********************************************
              READ GEOMETRIC DATA
        ***********************************************/
        else if( (*pChar) == 'g' ) {

          pChar++;

          while (  (*pChar) == '\n' || (*pChar) == '\r'  )
            pChar++;

          while ( *(pChar) != ',' )
            pChar++;

          /* jump over semicolon */
          pChar++;

          /* skip new lines and carriage return */
          while (  (*pChar) == '\n' || (*pChar) == '\r'  )
            pChar++;

          /********************************
            READ GEOMETRY AND SANITY CHECKS
           ********************************/

          /* init loudspeaker type */
          AzElLfe[ch].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;

          /************
             AZIMUTH
           ************/

          AzElLfe[ch].Az = atoi(pChar);

          if ( AzElLfe[ch].Az < -180 || AzElLfe[ch].Az > 180 ) {

            AzElLfe[ch].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_INVALID;
            if (verboseLvl >= 1) {
              fprintf(stderr,"Error: Illegal value for Azimuth (Channel: %i, Azimuth %i).\n", ch, AzElLfe[ch].Az);
            }

            if ( cicpError == CICP2GEOMETRY_OK ) {
              cicpError = CICP2GEOMETRY_INVALID_AZIMITH;
            }

          }

          while ( *(pChar) != ',' )
            pChar++;

          /* jump over semicolon */
          pChar++;

          /* skip new lines and carriage return */
          while (  (*pChar) == '\n' || (*pChar) == '\r'  )
            pChar++;

          /************
            ELEVAVTION
           ************/
          AzElLfe[ch].El = atoi(pChar);

          if ( AzElLfe[ch].El < -90 || AzElLfe[ch].El > 90 ) {

            AzElLfe[ch].loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_INVALID;
            if (verboseLvl >= 1) {
              fprintf(stderr,"Error: Illegal value for Elevation (Channel: %i, Elevation %i).\n", ch, AzElLfe[ch].El);
            }

            if ( cicpError == CICP2GEOMETRY_OK ) {
              cicpError = CICP2GEOMETRY_INVALID_ELEVATION;
            }

          }

          while ( *(pChar) != ',' )
            pChar++;
   
          /* jump over semicolon */
          pChar++;

          /* skip new lines and carriage return */
          while (  (*pChar) == '\n' || (*pChar) == '\r'  )
            pChar++;

          /************
             LFE
          ************/
          AzElLfe[ch].LFE = atoi(pChar);

          if ( AzElLfe[ch].LFE == 1 ) {
             (*numLFEs)++;
          }
          else if ( AzElLfe[ch].LFE != 0 ) {
            if (verboseLvl >= 1) {
              fprintf(stderr,"Error: Illegal value for LFE.\n");
            }
            if ( cicpError == CICP2GEOMETRY_OK ) {
              cicpError = CICP2GEOMETRY_INVALID_LFE_FLAG;
            }

          }

          while ( *(pChar) != ',' )
            pChar++;
   
          /* jump over semicolon */
          pChar++;

          /* skip new lines and carriage return */
          while (  (*pChar) == '\n' || (*pChar) == '\r'  )
            pChar++;

          /*********************
          SCREEN RELATIVENESS
          *********************/
          AzElLfe[ch].screenRelative = atoi(pChar);

          if ( AzElLfe[ch].screenRelative != 0 && AzElLfe[ch].screenRelative != 1 ) {
            if (verboseLvl >= 1) {
              fprintf(stderr,"Error: Illegal value for screenRelative.\n");
            }
            if ( cicpError == CICP2GEOMETRY_OK ) {
              cicpError = CICP2GEOMETRY_INVALID_SCREEN_RELATIVE_FLAG;
            }

          }

          /* no CICP loudspeaker index available in case geometric representation is given */
          AzElLfe[ch].cicpLoudspeakerIndex = -1;

        }

        else {

          /*************************************
            INVALID CICP / GEOMETRY FILE SYNTAX
           *************************************/
          if (verboseLvl >= 1) {
            fprintf(stderr,"Error: Wrong channel list file format.\n");
          }
          if ( cicpError == CICP2GEOMETRY_OK ) {
            cicpError = CICP2GEOMETRY_INVALID_GEO_FILE_SYNTAX;
          }
          break;

        }

        ++ch;
      }

      /* Jump over parsed value */
      while (  (*pChar) != '\n' )
        pChar++;            

      /* Jump over new lines */
      while (  (*pChar) == '\n' || (*pChar) == '\r'  )
        pChar++;

    }
  }
 
  if( ch != (*numChannels) ){
    if (verboseLvl >= 1) {
      fprintf(stderr,"Error: Channel list file did contain %d lines for %d channels.\n",ch,*numChannels);
    }
    cicpError = CICP2GEOMETRY_INVALID_GEO_FILE_SYNTAX;
  }

  /* print values */
  if (verboseLvl >= 2) {
    fprintf(stderr,"\nread channel list = [\n\n");
    for ( i = 0; i < (*numChannels); i++ ) {
       /* fprintf(stderr,"%d \n",(*channel_vector)[i]); */
       fprintf(stderr,"Channel %i:\n\n\tCICP loudspeaker index: %i\n\tAzimuth: %i,\n\tElevation: %i,\n\tLFE flag: %i\n\tScreen Relative: %i\n\n\n", i, AzElLfe[i].cicpLoudspeakerIndex, AzElLfe[i].Az, AzElLfe[i].El, AzElLfe[i].LFE, AzElLfe[i].screenRelative );
    }
  
    fprintf(stderr,"]\n");
  }

  *numChannels -= *numLFEs;

  fclose(fileHandle);
  return cicpError;    
}

/**********************************************************************//**

**************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_get_geometry_from_cicp_loudspeaker_index( int cicpLoudspeakerIndex, CICP2GEOMETRY_CHANNEL_GEOMETRY * AzElLfe ) {

  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;

  if ( AzElLfe == NULL ) {
    fprintf( stderr, "Bad pointer in cicp2geometry_get_geometry_from_cicp_loudspeaker_index()!\n");
    return CICP2GEOMETRY_BAD_POINTER;
  }

  if ( cicpLoudspeakerIndex >= 0 && cicpLoudspeakerIndex < CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLESIZE ) {

    AzElLfe->Az  = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex].Az;
    AzElLfe->El  = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex].El;
    AzElLfe->LFE = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex].LFE;

    AzElLfe->cicpLoudspeakerIndex = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex].cicpLoudspeakerIndex;
    AzElLfe->screenRelative       = CICP2GEOMETRY_CICP_LOUDSPEAKER_TABLE[cicpLoudspeakerIndex].screenRelative;
    AzElLfe->loudspeakerType      = CICP2GEOMETRY_LOUDSPEAKER_KNOWN;

  }
  else {

    /* save the cicp loudspeaker even if it's not known */
    AzElLfe->cicpLoudspeakerIndex = cicpLoudspeakerIndex;

    AzElLfe->Az  = -1;
    AzElLfe->El  = -1;
    AzElLfe->LFE = -1;
    AzElLfe->screenRelative  = -1;

    AzElLfe->loudspeakerType = CICP2GEOMETRY_LOUDSPEAKER_UNKNOWN;
    cicpError = CICP2GEOMETRY_UNKNOWN_SPEAKER_INDEX;

  }

  return cicpError;

}

/**********************************************************************//**

**************************************************************************/

CICP2GEOMETRY_ERROR cicp2geometry_write_geometry_to_file( char*  geometryOutputFilename,
                                 CICP2GEOMETRY_CHANNEL_GEOMETRY  AzElLfe[CICP2GEOMETRY_MAX_LOUDSPEAKERS],           
                                                          int    numChannels,           
                                                          int    numLFEs                
                                                        ){
  CICP2GEOMETRY_ERROR cicpError = CICP2GEOMETRY_OK;
  FILE* fileHandle;
  int numChannelsTotal = numChannels + numLFEs;
  int ch = 0;

  /* sanity check */
  if ( NULL ==  AzElLfe ) {
    cicpError = CICP2GEOMETRY_BAD_POINTER;
    return cicpError;
  }

  /* open file */
  fileHandle = fopen(geometryOutputFilename, "wb");    
  
  if ( NULL == fileHandle ) {
    cicpError = CICP2GEOMETRY_INVALID_FILEHANDLE;
    return cicpError;
  }

  /* define number of entries */
  fprintf( fileHandle, "%i\n", numChannelsTotal );

  for ( ch = 0; ch < numChannelsTotal; ++ch ) {
    /* write cicp speaker index if available */
    if( AzElLfe[ch].cicpLoudspeakerIndex != -1 ) {
      fprintf( fileHandle, "c,%i\n", AzElLfe[ch].cicpLoudspeakerIndex );
    }
    else {
      /* write geometry info if necessary */
      fprintf( fileHandle, "g,%i,%i,%i,%i\n", AzElLfe[ch].Az, AzElLfe[ch].El, AzElLfe[ch].LFE, AzElLfe[ch].screenRelative );
    }
  }

  /* close file */
  if ( fclose(fileHandle) ) {
    cicpError = CICP2GEOMETRY_INVALID_FILEHANDLE;
  }

  return cicpError;
}



int cicp2geometry_compare_geometry(CICP2GEOMETRY_CHANNEL_GEOMETRY geoOne[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                   unsigned int numChannelsOne,
                                   CICP2GEOMETRY_CHANNEL_GEOMETRY geoTwo[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
                                   unsigned int numChannelsTwo,
                                   unsigned int tolerance)
{
  unsigned int i;
  int result = 0;

  if ( numChannelsOne != numChannelsTwo )
  {
    result = -1; 
    return result;
  }

  for(i = 0; i < numChannelsOne; i++)
  {
    if( abs(geoOne[i].Az - geoTwo[i].Az ) > (int)tolerance )
      result = -1;
    if( abs(geoOne[i].El - geoTwo[i].El ) > (int)tolerance )
      result = -1;

    if( geoOne[i].LFE != geoTwo[i].LFE )
      result = -1;

    if( geoOne[i].screenRelative != geoTwo[i].screenRelative )
      result = -1;

    if( result != 0)
      break;

  }

  return result;
}
