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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "speakerConfig3d.h"


/*---------------------  Utilities (from dmx_matrix_enc/dec.c)    ---------------------*/

int ReadEscapedValue(robitbufHandle hBitStream, int count1, int count2, int count3)
{
	unsigned int value = 0;

	unsigned int value1 = robitbuf_ReadBits(hBitStream, count1);
	unsigned int escape1 = (1U << count1) - 1;

	value = value1;
	if (value1 == escape1) {
		unsigned int value2 = robitbuf_ReadBits(hBitStream, count2);
		unsigned int escape2 = (1U << count2) - 1;

		value += value2;
		if (value2 == escape2) {
			unsigned int value3 = robitbuf_ReadBits(hBitStream, count3);

			value += value3;
		}
	}

	return value;
}

int WriteEscapedValue(wobitbufHandle hBitStream, unsigned int value, int count1, int count2, int count3)
{
	int error = 0;

	if (value < (1U << count1) - 1) {
		error = wobitbuf_WriteBits(hBitStream, value, count1); if (error) return -1;
	} else {
		unsigned int escape1 = (1U << count1) - 1;
		error = wobitbuf_WriteBits(hBitStream, escape1, count1); if (error) return -1;
		value -= escape1;
		if (value < (1U << count2) - 1) {
			error = wobitbuf_WriteBits(hBitStream, value, count2); if (error) return -1;
		} else {
			unsigned int escape2 = (1U << count2) - 1;
			error = wobitbuf_WriteBits(hBitStream, escape2, count2); if (error) return -1;
			value -= escape2;
			assert(value < (1U << count3));
			error = wobitbuf_WriteBits(hBitStream, value, count3); if (error) return -1;
		}
	}

	return error;
}




/*---------------------     Loudspeaker Configuration (parts from saoc_bitstream.c)     -------------*/

int saoc_AzimuthAngleIdxToDegrees ( int idx, int direction, int precision ) 
{
	int retVal = 0;
	if ( precision == 0 ) {
		retVal = idx * 5;
	} else {
		retVal = idx;
	}

	if(direction == 1) retVal *= -1;

	return retVal;
}

int saoc_ElevationAngleIdxToDegrees ( int idx, int direction, int precision ) 
{
	int retVal = 0;
	if ( precision == 0 ) {
		retVal = idx * 5;
	} else {
		retVal = idx;
	}

	if(direction == 1) retVal *= -1;

	return retVal;
}

int readSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, robitbufHandle hBitstream)
{
	unsigned int i;
	pSpeakerConfig3d->speakerLayoutType = (unsigned short) robitbuf_ReadBits(hBitstream, 2); /* speakerLayoutType */ 
	if ( pSpeakerConfig3d->speakerLayoutType == 0 ) 
	{
		pSpeakerConfig3d->CICPspeakerLayoutIdx = (short) robitbuf_ReadBits(hBitstream, 6); /* CICPspeakerLayoutIdx */ 
	} 
	else if (pSpeakerConfig3d->speakerLayoutType < 3)
	{
    pSpeakerConfig3d->CICPspeakerLayoutIdx = (short) -1;
		pSpeakerConfig3d->numSpeakers = ReadEscapedValue(hBitstream, 5, 8, 16 ) + 1; /* numSpeakers */
		if ( pSpeakerConfig3d->speakerLayoutType == 1 ) 
		{
			for ( i = 0; i < pSpeakerConfig3d->numSpeakers; i++ ) 
			{
				pSpeakerConfig3d->CICPspeakerIdx[i] = (unsigned short) robitbuf_ReadBits(hBitstream, 7); /* CICPspeakerIdx */
			}
		}
		if ( pSpeakerConfig3d->speakerLayoutType == 2 ) 
		{
			readFlexibleSpeakerConfig( 
				&(pSpeakerConfig3d->flexibleSpeakerConfig), 
				pSpeakerConfig3d->numSpeakers, 
				hBitstream );
		}
	}

	/* Fill geometry, numChannels and numLFEs ... */
	get_geometry_from_speakerConfig3d(
		pSpeakerConfig3d,
		pSpeakerConfig3d->pGeometry,
		&pSpeakerConfig3d->numChannels,
		&pSpeakerConfig3d->numLFEs);

	/* ... and set numSpeakers */
	pSpeakerConfig3d->numSpeakers = pSpeakerConfig3d->numChannels + pSpeakerConfig3d->numLFEs;

	return 0;
}

int writeSpeakerConfig3d(Setup_SpeakerConfig3d *pSpeakerConfig3d, wobitbufHandle hBitstream)
{

	wobitbuf_WriteBits(hBitstream, pSpeakerConfig3d->speakerLayoutType, 2);    /* speakerLayoutType */ 
	switch ( pSpeakerConfig3d->speakerLayoutType ) 
	{
	case 0: /* CICPspeakerLayoutIdx */
		{
			wobitbuf_WriteBits(hBitstream, pSpeakerConfig3d->CICPspeakerLayoutIdx, 6); /* CICPspeakerLayoutIdx */ 
			break;
		}
	case 1: /* CICPspeakerIdx */
		{
			fprintf(stderr,"\nError writeSpeakerConfig3d() : speakerLayoutType %d not implemented yet", pSpeakerConfig3d->speakerLayoutType);
			return -1;
		}
	case 2:
		{
			WriteEscapedValue(hBitstream, pSpeakerConfig3d->numSpeakers - 1, 5,8,16); /* numSpeakers (-1) */
			writeFlexibleSpeakerConfig(&pSpeakerConfig3d->flexibleSpeakerConfig, 
				pSpeakerConfig3d->numSpeakers, 
			    hBitstream);
			break;
		}
	
	}
	
	return 0;
}

int readFlexibleSpeakerConfig(
	FlexibleSpeakerConfig *pFlexibleSpeakerConfig, 
	int numSpeakers, 
	robitbufHandle hBitstream)
{
	int i = 0;
	int alsoAddSymmetricPair;

	pFlexibleSpeakerConfig->angularPrecision = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* angularPrecision */

	while ( i < numSpeakers ) {

		int azimuth;

		readSpeakerDescription (
			&(pFlexibleSpeakerConfig->speakerDescription[i]), 
			pFlexibleSpeakerConfig->angularPrecision, 
			hBitstream );

		if(pFlexibleSpeakerConfig->speakerDescription[i].isCICPspeakerIdx) 
		{
			CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
			cicp2geometry_get_geometry_from_cicp_loudspeaker_index( 
				pFlexibleSpeakerConfig->speakerDescription[i].CICPspeakerIdx, 
				&AzElLfe);
			azimuth = AzElLfe.Az;
		}
		else 
		{
			azimuth = saoc_AzimuthAngleIdxToDegrees ( 
				pFlexibleSpeakerConfig->speakerDescription[i].AzimuthAngleIdx, 
				pFlexibleSpeakerConfig->speakerDescription[i].AzimuthDirection, 
				pFlexibleSpeakerConfig->angularPrecision );
		}

		if ( (azimuth != 0 ) && (azimuth != 180 ) /* && (the symmetric speaker is not yet in the list) */ ) {

			alsoAddSymmetricPair = robitbuf_ReadBits(hBitstream, 1); /* alsoAddSymmetricPair */ 

			if ( alsoAddSymmetricPair ) {
				/* (also add the speaker with the opposite AzimuthDirection); */
				i++;
				pFlexibleSpeakerConfig->speakerDescription[i] = pFlexibleSpeakerConfig->speakerDescription[i - 1];
				pFlexibleSpeakerConfig->speakerDescription[i].AzimuthDirection = 1 - pFlexibleSpeakerConfig->speakerDescription[i].AzimuthDirection;
			}
		}

		i++;
	}

	return 0;
}

int writeFlexibleSpeakerConfig(
	FlexibleSpeakerConfig *pFlexibleSpeakerConfig, 
	int numSpeakers, 
	wobitbufHandle hBitstream)
{
	int i = 0;
	/* int alsoAddSymmetricPair; */
	unsigned int data;

	data = (unsigned int) pFlexibleSpeakerConfig->angularPrecision;
	wobitbuf_WriteBits(hBitstream, data, 1); /* angularPrecision */

	while ( i < numSpeakers ) {
		int azimuth;

		if(pFlexibleSpeakerConfig->speakerDescription[i].isCICPspeakerIdx) 
		{
			CICP2GEOMETRY_CHANNEL_GEOMETRY AzElLfe; 
			cicp2geometry_get_geometry_from_cicp_loudspeaker_index( 
				pFlexibleSpeakerConfig->speakerDescription[i].CICPspeakerIdx, 
				&AzElLfe);
			azimuth = AzElLfe.Az;
		}
		else 
		{
			azimuth = saoc_AzimuthAngleIdxToDegrees ( 
				pFlexibleSpeakerConfig->speakerDescription[i].AzimuthAngleIdx, 
				pFlexibleSpeakerConfig->speakerDescription[i].AzimuthDirection, 
				pFlexibleSpeakerConfig->angularPrecision );
		}

		writeSpeakerDescription (
			&(pFlexibleSpeakerConfig->speakerDescription[i]), 
			pFlexibleSpeakerConfig->angularPrecision, 
			hBitstream );

		
		if ( (azimuth != 0 ) && (azimuth != 180 ) ) /* && (the symmetric speaker is not yet in the list) */ 
		{
			data = 0; /* NEVER add symmetrical pair */
			wobitbuf_WriteBits(hBitstream, data, 1);  /* alsoAddSymmetricPair */
		}

		i++;
	}

	return 0;
}

int readSpeakerDescription(
	SpeakerDescription *pSpeakerDescription, 
	int angularPrecision, 
	robitbufHandle hBitstream)
{
	pSpeakerDescription->isCICPspeakerIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* isCICPspeakerIdx */ 
	if ( pSpeakerDescription->isCICPspeakerIdx ) 
	{
		pSpeakerDescription->CICPspeakerIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 7); /* CICPspeakerIdx */ 
	} 
	else 
	{
		pSpeakerDescription->ElevationClass = (unsigned short) robitbuf_ReadBits(hBitstream, 2); /* ElevationClass */ 
		if ( pSpeakerDescription->ElevationClass == 3 ) 
		{
			if ( angularPrecision == 0 ) 
			{
				pSpeakerDescription->ElevationAngleIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 5); /* ElevationAngleIdx */ 
			} 
			else 
			{
				pSpeakerDescription->ElevationAngleIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 7); /* ElevationAngleIdx */ 
			}

			if ( saoc_ElevationAngleIdxToDegrees ( 
				pSpeakerDescription->ElevationAngleIdx, 
				pSpeakerDescription->ElevationDirection,  
				angularPrecision ) != 0 ) 
			{
				pSpeakerDescription->ElevationDirection = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* ElevationDirection */ 
			}
		}

		if ( angularPrecision == 0 ) 
		{
			pSpeakerDescription->AzimuthAngleIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 6); /* AzimuthAngleIdx */ 
		} 
		else 
		{
			pSpeakerDescription->AzimuthAngleIdx = (unsigned short) robitbuf_ReadBits(hBitstream, 8); /* AzimuthAngleIdx */ 
		}

		if ( ( saoc_AzimuthAngleIdxToDegrees ( pSpeakerDescription->AzimuthAngleIdx, pSpeakerDescription->AzimuthDirection, angularPrecision ) != 0 ) &&
			( saoc_AzimuthAngleIdxToDegrees ( pSpeakerDescription->AzimuthAngleIdx, pSpeakerDescription->AzimuthDirection, angularPrecision ) != 180 ) ) 
		{
			pSpeakerDescription->AzimuthDirection = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* AzimuthDirection */ 
		}
		pSpeakerDescription->isLFE = (unsigned short) robitbuf_ReadBits(hBitstream, 1); /* isLFE */ 

	}
	return 0;
}

int writeSpeakerDescription(
	SpeakerDescription *pSpeakerDescription, 
	int angularPrecision, 
	wobitbufHandle hBitstream)
{
	unsigned int data;

	data = (unsigned int) pSpeakerDescription->isCICPspeakerIdx;
	wobitbuf_WriteBits(hBitstream, data, 1); /* isCICPspeakerIdx */ 

	if ( pSpeakerDescription->isCICPspeakerIdx ) 
	{
		data = (unsigned int) pSpeakerDescription->CICPspeakerIdx;
		wobitbuf_WriteBits(hBitstream, data, 7); /* CICPspeakerIdx */ 
	} 
	else 
	{
		data = (unsigned int) pSpeakerDescription->ElevationClass;
		wobitbuf_WriteBits(hBitstream, data, 2); /* ElevationClass */ 
		if ( pSpeakerDescription->ElevationClass == 3 ) 
		{
			if ( angularPrecision == 0 ) 
			{
				data = (unsigned int) pSpeakerDescription->ElevationAngleIdx;
				wobitbuf_WriteBits(hBitstream, data, 5); /* ElevationAngleIdx */ 
			} 
			else 
			{
				data = (unsigned int) pSpeakerDescription->ElevationAngleIdx;
				wobitbuf_WriteBits(hBitstream, data, 7); /* ElevationAngleIdx */ 
			}

			if ( saoc_ElevationAngleIdxToDegrees ( 
				pSpeakerDescription->ElevationAngleIdx, 
				pSpeakerDescription->ElevationDirection,  
				angularPrecision ) != 0 ) 
			{
				data = (unsigned int) pSpeakerDescription->ElevationDirection;
				wobitbuf_WriteBits(hBitstream, data, 1); /* ElevationDirection */ 
			}
		}

		if ( angularPrecision == 0 ) 
		{
			data = (unsigned int) pSpeakerDescription->AzimuthAngleIdx;
			wobitbuf_WriteBits(hBitstream, data, 6); /* AzimuthAngleIdx */ 
		} 
		else 
		{
			data = (unsigned int) pSpeakerDescription->AzimuthAngleIdx;
			wobitbuf_WriteBits(hBitstream, data, 8); /* AzimuthAngleIdx */ 
		}

		if ( ( saoc_AzimuthAngleIdxToDegrees ( pSpeakerDescription->AzimuthAngleIdx, pSpeakerDescription->AzimuthDirection, angularPrecision ) != 0 ) &&
			( saoc_AzimuthAngleIdxToDegrees ( pSpeakerDescription->AzimuthAngleIdx, pSpeakerDescription->AzimuthDirection, angularPrecision ) != 180 ) ) 
		{
			data = pSpeakerDescription->AzimuthDirection;
			wobitbuf_WriteBits(hBitstream, data, 1); /* AzimuthDirection */ 
		}
		data = (unsigned int) pSpeakerDescription->isLFE;
		wobitbuf_WriteBits(hBitstream, data, 1); /* isLFE */ 

	}
	return 0;
}

int set_speakerDescription_from_geometry(SpeakerDescription *pSpeakerDescription, 
										 int angularPrecision, 
										 CICP2GEOMETRY_CHANNEL_GEOMETRY *pGeometry)
{
	if (pGeometry->cicpLoudspeakerIndex != -1) 
	{
		pSpeakerDescription->isCICPspeakerIdx = 1;
		pSpeakerDescription->CICPspeakerIdx = pGeometry->cicpLoudspeakerIndex;
	}
	else
	{
		pSpeakerDescription->isCICPspeakerIdx = 0; 

		if (pGeometry->Az >= 0)
		{
			pSpeakerDescription->AzimuthDirection = 0;
			if (angularPrecision == 0)
				pSpeakerDescription->AzimuthAngleIdx = (unsigned short)(pGeometry->Az / 5.);
			else
				pSpeakerDescription->AzimuthAngleIdx = (unsigned short) pGeometry->Az;
		}
		else
		{
			pSpeakerDescription->AzimuthDirection = 1;
			if (angularPrecision == 0)
				pSpeakerDescription->AzimuthAngleIdx = (unsigned short)(-pGeometry->Az / 5.);
			else
				pSpeakerDescription->AzimuthAngleIdx = (unsigned short) -pGeometry->Az;
		}
			
		pSpeakerDescription->ElevationClass = 3; /* elevation set explicitly */

		if (pGeometry->El >= 0)
		{
			pSpeakerDescription->ElevationDirection = 0;
			if (angularPrecision == 0)
				pSpeakerDescription->ElevationAngleIdx = (unsigned short)(pGeometry->El / 5.);
			else
				pSpeakerDescription->ElevationAngleIdx = (unsigned short) pGeometry->El;
		}
		else
		{
			pSpeakerDescription->ElevationDirection = 1;
			if (angularPrecision == 0)
				pSpeakerDescription->ElevationAngleIdx = (unsigned short)(-pGeometry->El / 5.);
			else
				pSpeakerDescription->ElevationAngleIdx = (unsigned short) -pGeometry->El;
		}

		pSpeakerDescription->isLFE = pGeometry->LFE;
	}
	return 0;
}


int get_geometry_from_speakerConfig3d(
	Setup_SpeakerConfig3d *pSpeakerConfig3d,
	CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
	int *numChannels,
	int *numLFEs)
{

	unsigned int i;

	if (pSpeakerConfig3d->speakerLayoutType == 0 ) 
	{
		cicp2geometry_get_geometry_from_cicp(pSpeakerConfig3d->CICPspeakerLayoutIdx, 
			pGeometry, numChannels, numLFEs); 
	} 
	else if (pSpeakerConfig3d->speakerLayoutType < 3)
	{
		if (pSpeakerConfig3d->speakerLayoutType == 1 ) 
		{
			*numChannels = pSpeakerConfig3d->numSpeakers;
			*numLFEs = 0;
			for (i = 0; i<pSpeakerConfig3d->numSpeakers; i++) 
			{
				cicp2geometry_get_geometry_from_cicp_loudspeaker_index(pSpeakerConfig3d->CICPspeakerIdx[i], 
					&(pGeometry[i])); 
				if ( (pSpeakerConfig3d->CICPspeakerIdx[i]  == 3 ) || (pSpeakerConfig3d->CICPspeakerIdx[i]  == 26) )
				{
					(*numChannels)--;
					(*numLFEs)++;
				}
			} 
		} 
		else if (pSpeakerConfig3d->speakerLayoutType == 2 ) 
		{
			*numChannels = pSpeakerConfig3d->numSpeakers;
			*numLFEs = 0;
			for (i = 0; i<pSpeakerConfig3d->numSpeakers; i++) 
			{
				if(pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isCICPspeakerIdx) 
				{
					/* Bugfix CV */
					cicp2geometry_get_geometry_from_cicp_loudspeaker_index( /* pSpeakerConfig3d->CICPspeakerIdx[i], */
						pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].CICPspeakerIdx,
						&(pGeometry[i])); 
				}
				else 
				{
					pGeometry[i].cicpLoudspeakerIndex = -1;
					pGeometry[i].Az = saoc_AzimuthAngleIdxToDegrees(pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthAngleIdx, 
						pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].AzimuthDirection,  
						pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision);
					pGeometry[i].El = saoc_ElevationAngleIdxToDegrees(pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationAngleIdx, 
						pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].ElevationDirection,  
						pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision);

					pGeometry[i].LFE = pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isLFE;

				}
				if ( pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i].isLFE  !=0 ) 
				{
					(*numChannels)--;
					(*numLFEs)++;
				}
			} 
		}
	}
	return 0;
} 

int set_speakerConfig3d_from_geometry(
	CICP2GEOMETRY_CHANNEL_GEOMETRY pGeometry[CICP2GEOMETRY_MAX_LOUDSPEAKERS],
	int numChannels,
	int numLFEs,
	Setup_SpeakerConfig3d *pSpeakerConfig3d,
	int cicpIndex)
{
	unsigned int i;

	pSpeakerConfig3d->numChannels = numChannels;
	pSpeakerConfig3d->numLFEs = numLFEs;
	pSpeakerConfig3d->numSpeakers = numChannels + numLFEs;
	memcpy(pSpeakerConfig3d->pGeometry, 
		pGeometry, 
		pSpeakerConfig3d->numSpeakers * sizeof( CICP2GEOMETRY_CHANNEL_GEOMETRY ) );

	pSpeakerConfig3d->CICPspeakerLayoutIdx = cicpIndex;

	if (cicpIndex != -1)
	{
		pSpeakerConfig3d->speakerLayoutType = 0; /* case CICPspeakerLayoutIdx */
	}
	else 
	{
		pSpeakerConfig3d->speakerLayoutType = 2; /* case Flexible Speaker Config */
		pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision = 1; /* one degree precision */

		for (i=0; i<pSpeakerConfig3d->numSpeakers; i++)
		{
			set_speakerDescription_from_geometry(
				&pSpeakerConfig3d->flexibleSpeakerConfig.speakerDescription[i], 
			    pSpeakerConfig3d->flexibleSpeakerConfig.angularPrecision, 
				&pGeometry[i]);
		}
	}

	return 0;
}



int find_geometry_in_speakerConfig3d(
						CICP2GEOMETRY_CHANNEL_GEOMETRY *pGeometry,
						Setup_SpeakerConfig3d *pSpeakerConfig3d,
						int *ind)
{

	unsigned long i;
	*ind = -1;

	for (i=0; i<pSpeakerConfig3d->numSpeakers; i++)
	{
	
		if ( ( pGeometry->Az == pSpeakerConfig3d->pGeometry[i].Az ) &&
			 ( pGeometry->El == pSpeakerConfig3d->pGeometry[i].El ) &&
			 ( pGeometry->LFE == pSpeakerConfig3d->pGeometry[i].LFE ) )
		{
			 *ind = (int)i;
			 break;
		}
	}

	/* Ugly hack for unfound LFEs :
			if the exact expected coordinate of the LFE are not found in the list, 
			then the first LFE is taken ! */
	if ( (*ind == -1) && (pGeometry->LFE == 1) )
	{
		for (i=0; i<pSpeakerConfig3d->numSpeakers; i++)
		{
			if ( pGeometry->LFE == pSpeakerConfig3d->pGeometry[i].LFE ) 
			{
				*ind = (int)i;
				break;
			}
		}
	}

	return 0;
}
