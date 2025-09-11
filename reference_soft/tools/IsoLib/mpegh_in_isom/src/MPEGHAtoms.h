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

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

/*!
@header MPEGHAtoms
MPEGHAtoms defines specific MPEG-H Audio atoms for the file format.
@version 1.0
*/

#ifndef MPEGHAtoms_h
#define MPEGHAtoms_h

#include "MP4Atoms.h"

/* Types definition */
enum
{
  MP4MHAConfigurationAtomType     = MP4_FOUR_CHAR_CODE( 'm', 'h', 'a', 'C' ),
  MP4MpegHSampleEntryAtomTypeMHA1 = MP4_FOUR_CHAR_CODE( 'm', 'h', 'a', '1' ),
  MP4MpegHSampleEntryAtomTypeMHA2 = MP4_FOUR_CHAR_CODE( 'm', 'h', 'a', '2' ),
  MP4MpegHSampleEntryAtomTypeMHM1 = MP4_FOUR_CHAR_CODE( 'm', 'h', 'm', '1' ),
  MP4MpegHSampleEntryAtomTypeMHM2 = MP4_FOUR_CHAR_CODE( 'm', 'h', 'm', '2' ),
  MP4MpegHProfileAndLevelAtomType = MP4_FOUR_CHAR_CODE( 'm', 'h', 'a', 'P' )
};

typedef struct MP4MHAConfigurationAtom
{
  MP4_BASE_ATOM
  u8    configurationVersion;   /* uint(8) */
  u8    MPEGHAudioProfileLevel; /* uint(8) */
  u8    referenceChannelLayout; /* uint(8) */
  u16   mpegh3daConfigLength;   /* uint(16) */
  char* mpegh3daConfig;
} MP4MHAConfigurationAtom, *MP4MHAConfigurationAtomPtr;

typedef struct MP4MpegHSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char  reserved2[8];           /* uint(32)[2] */
  u32   reserved3;              /* uint(16) = 2 */
  u32   reserved4;              /* uint(16) = 16 */
  u32   reserved5;              /* uint(32) = 0 */
  u32   timeScale;              /* uint(16) copied from track! */
  u32   reserved6;              /* uint(16) = 0 */
} MP4MpegHSampleEntryAtom, *MP4MpegHSampleEntryAtomPtr;

typedef struct MP4MpegHProfileAndLevelAtom
{
  MP4_BASE_ATOM
  u8  numCompatibleSets;        /* uint(8) */
  u8* compatibleSetIndication;
} MP4MpegHProfileAndLevelAtom, *MP4MpegHProfileAndLevelAtomPtr;

/* Functions */
MP4_EXTERN ( MP4Err )
MP4ParseMPEGHAtom( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom );

MP4_EXTERN ( MP4Err )
MP4CreateMPEGHAtom( u32 atomType, MP4AtomPtr *outAtom );

MP4_EXTERN ( MP4Err )
MP4ParseMPEGHAtomUsingProtoList( MP4InputStreamPtr inputStream, u32* protoList, u32 defaultAtom, MP4AtomPtr *outAtom );

MP4_EXTERN ( MP4Err )
MP4CreateMHAConfigurationAtom( MP4MHAConfigurationAtomPtr *outAtom );

MP4_EXTERN ( MP4Err )
MP4CreateMpegHSampleEntryAtom( u32 atomType, MP4MpegHSampleEntryAtomPtr *outAtom );

MP4_EXTERN ( MP4Err )
MP4CreateMpegHProfileAndLevelAtom( MP4MpegHProfileAndLevelAtomPtr *outAtom );

#endif // MPEGHAtoms_h
