/***********************************************************************************

This software module was originally developed by 

Orange Labs

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

Orange Labs retain full right to modify and use the code for its own purpose, 
assign or donate the code to a third party and to inhibit third parties 
from using the code for products that do not conform to MPEG-related 
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

/*----------------------------------------------------------------------------*/
#ifndef SPEAKER_CONFIG_3D_TD_H
#define SPEAKER_CONFIG_3D_TD_H
/*----------------------------------------------------------------------------*/


#include "cicp2geometry.h"
#include "readonlybitbuf.h"
#include "writeonlybitbuf.h"
#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

	/*---------------------  Utilities (from dmx_matrix_enc/dec.c)    ---------------------*/

	int ReadEscapedValue(robitbufHandle hBitStream, int count1, int count2, int count3);

	int WriteEscapedValue(wobitbufHandle hBitStream, unsigned int value, int count1, int count2, int count3);

  int saoc_ElevationAngleIdxToDegrees ( int idx, int direction, int precision );
  int saoc_AzimuthAngleIdxToDegrees ( int idx, int direction, int precision );


	/*--------------------------  Loudspeaker Configuration  ------------------------------*/

	typedef struct SpeakerDescription {
		unsigned short isCICPspeakerIdx;
		unsigned short CICPspeakerIdx;
		unsigned short ElevationClass;
		unsigned short ElevationAngleIdx; 
		unsigned short ElevationDirection; 
		unsigned short AzimuthAngleIdx;
		unsigned short AzimuthDirection; 
		unsigned short isLFE;
	} SpeakerDescription;

	typedef struct FlexibleSpeakerConfig {
		unsigned short angularPrecision;
		SpeakerDescription speakerDescription[CICP2GEOMETRY_MAX_LOUDSPEAKERS];
	} FlexibleSpeakerConfig;

	typedef struct Setup_SpeakerConfig3d
	{
		unsigned short speakerLayoutType;	/* 0 : CICPspeakerLayoutIdx, 1 : CICPspeakerIdx, 2 : FlexibleSpeakerConfig */
		short CICPspeakerLayoutIdx;								/* case 0 */
		unsigned short CICPspeakerIdx[CICP2GEOMETRY_MAX_LOUDSPEAKERS];      /* case 1 */
		FlexibleSpeakerConfig flexibleSpeakerConfig;						/* case 2 */

		/* Info filled in all 3 cases */
		unsigned long numSpeakers;	/* total number of speakers */
		int numChannels;			/* number of channels, not including LFEs */
		int numLFEs;				/* number of LFEs */
		CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS]; /* geometry */
	}
	Setup_SpeakerConfig3d;

	/**
	*   Read Setup_SpeakerConfig3d from bitstream 
	*   @param    pSpeakerConfig3d	: destination Setup_SpeakerConfig3d pointer (previously initialized)
	*   @param    hBitstream     	: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int readSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, robitbufHandle hBitstream);

	/**
	*   Read FlexibleSpeakerConfig from bitstream 
	*   @param    pFlexibleSpeakerConfig	: destination FlexibleSpeakerConfig pointer (previously initialized)
	*   @param    numSpeakers				: number of speakers in the config
	*   @param    hBitstream     			: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int readFlexibleSpeakerConfig(FlexibleSpeakerConfig *pFlexibleSpeakerConfig, int numSpeakers, robitbufHandle hBitstream);

	/**
	*   Write FlexibleSpeakerConfig to bitstream 
	*   @param    pFlexibleSpeakerConfig	: source FlexibleSpeakerConfig pointer (previously initialized)
	*   @param    numSpeakers				: number of speakers in the config
	*   @param    hBitstream     			: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int writeFlexibleSpeakerConfig(FlexibleSpeakerConfig *pFlexibleSpeakerConfig, int numSpeakers, wobitbufHandle hBitstream);

	/**
	*   Read SpeakerDescription from bitstream 
	*   @param    pSpeakerDescription	: destination SpeakerDescription pointer (previously initialized)
	*   @param    angularPrecision		: angular precision of the parent FlexibleSpeakerConfig
	*   @param    hBitstream     		: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int readSpeakerDescription(SpeakerDescription *pSpeakerDescription, int angularPrecision, robitbufHandle hBitstream);

	/**
	*   Write SpeakerDescription to bitstream 
	*   @param    pSpeakerDescription	: source SpeakerDescription pointer (previously initialized)
	*   @param    angularPrecision		: angular precision of the parent FlexibleSpeakerConfig
	*   @param    hBitstream     		: handle to the bit buffer
	*
	*   @return   ERROR_CODE
	*/
	int writeSpeakerDescription(SpeakerDescription *pSpeakerDescription, int angularPrecision, wobitbufHandle hBitstream);


	/**
	*   Set SpeakerDescription from geometry 
	*   @param    pSpeakerDescription	: destination SpeakerDescription pointer (previously initialized)
	*   @param    angularPrecision		: angular precision of the parent FlexibleSpeakerConfig
	*   @param    pGeometry     		: source CICP2GEOMETRY_CHANNEL_GEOMETRY pointer (previously initialised)
	*
	*   @return   ERROR_CODE
	*/
	int set_speakerDescription_from_geometry(SpeakerDescription *pSpeakerDescription, int angularPrecision, CICP2GEOMETRY_CHANNEL_GEOMETRY *pGeometry);

	/**
	*   Get geometry from a Setup_SpeakerConfig3d  
	*   @param    pSpeakerConfig3d	: source Setup_SpeakerConfig3d pointer
	*   @param    pGeometry			: destination CICP2GEOMETRY_CHANNEL_GEOMETRY array 
	*   @param    numChannels		: destination number of channels (excluding LFEs)
	*   @param    numLFEs			: destination number of LFEs
	*
	*   @return   ERROR_CODE
	*/
	int get_geometry_from_speakerConfig3d(
		Setup_SpeakerConfig3d *pSpeakerConfig3d,
		CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
		int *numChannels,
		int *numLFEs);

	/**
	*   Set Setup_SpeakerConfig3d from geometry
	*   @param    pGeometry			: source CICP2GEOMETRY_CHANNEL_GEOMETRY array
	*   @param    numChannels		: source number of channels (excluding LFEs)
	*   @param    numLFEs			: source number of LFEs
	*   @param    pSpeakerConfig3d	: destination Setup_SpeakerConfig3d pointer
	*   @param    cicpIndex     	: cicp index of input config (-1 if geo config)
	*
	*   @return   ERROR_CODE
	*/
	int set_speakerConfig3d_from_geometry(
		CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
		int numChannels,
		int numLFEs,
		Setup_SpeakerConfig3d *pSpeakerConfig3d,
		int cicpIndex);

	/**
	*   Find geometry in Setup_SpeakerConfig3d
	*   @param    pGeometry			: source CICP2GEOMETRY_CHANNEL_GEOMETRY pointer
	*   @param    pSpeakerConfig3d	: source Setup_SpeakerConfig3d pointer
	*   @param    ind	            : output indice of the geometry (-1 if not found) 
	*
	*   @return   ERROR_CODE
	*/
	int find_geometry_in_speakerConfig3d(
						CICP2GEOMETRY_CHANNEL_GEOMETRY *pGeometry,
						Setup_SpeakerConfig3d *pSpeakerConfig3d,
						int *ind);

  int writeSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, wobitbufHandle hBitstream);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_TD_H */