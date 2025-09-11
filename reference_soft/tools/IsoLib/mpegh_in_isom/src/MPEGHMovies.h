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
@header MPEGHMovies
MPEGHMovies defines specific functions to enable MPEG-H Audio support within libisomedia.
@version 1.0
*/

#ifndef MPEGHMovies_h
#define MPEGHMovies_h

#include "MP4Movies.h"

MP4_EXTERN ( MP4Err )
MP4NewMHAConfiguration( MP4Track theTrack,
                        MP4Handle sampleDescriptionH,
                        u32 dataReferenceIndex,
                        u32 decoderBufferSize,
                        u32 maxBitrate,
                        u32 avgBitrate,
                        u8  MPEGHProfileLevel,
                        u8  mpeghMask,
                        u8  referenceChannelLayout,
                        u8  numProfileAndLevelCompatibleSets,
                        u8* profileAndLevelCompatibleSets,
                        MP4Handle decoderSpecificInfoH );

MP4_EXTERN ( MP4Err )
MHAGetMediaDecoderConfig( MP4TrackReader theReader,
                          MP4Handle decoderConfigH );

MP4_EXTERN ( MP4Err )
MHAGetMediaDecoderInformation( MP4Media theMedia, 
                               u32 sampleDescIndex,
                               u32 *outConfigVersion,
                               u32 *outProfileLevel,
                               u32 *outRefChannelLayout,
                               u32 *outBufferSize,
                               u32 *outMaxBitrate,
                               u32 *outAvgBitrate,
                               u8  *outNumProfileAndLevelCompatibleSets,
                               MP4Handle outProfileAndLevelCompatibleSets,
                               MP4Handle specificInfoH );

MP4_EXTERN ( MP4Err )
MHAGetMediaDecoderType( MP4Media theMedia,
                        u32 sampleDescIndex,
                        u32 *outProfileLevel,
                        u32 *outBufferSize,
                        MP4Handle specificInfoH );

#endif // MPEGHMovies_h
