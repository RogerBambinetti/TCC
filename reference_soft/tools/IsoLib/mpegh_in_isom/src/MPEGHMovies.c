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

#include "MPEGHMovies.h"
#include "MPEGHAtoms.h"
#include "MP4TrackReader.h"

enum
{
  MPEGHMaskWriteConfigBox           = (1 << 0), /* write configuration box (only for streaming format) */
  MPEGHMaskWriteBitrateBox          = (1 << 1), /* write bitrate box */
  MPEGHMaskUseStreamingFormat       = (1 << 2), /* use streaming format (MHM box) */
  MPEGHMaskUseMultipleStreamsFormat = (1 << 3)  /* use multiple streams format */
};

MP4_EXTERN ( MP4Err )
MP4NewMHAConfiguration( MP4Track theTrack,
                        MP4Handle sampleDescriptionH,
                        u32 dataReferenceIndex,
                        u32 decoderBufferSize,
                        u32 maxBitrate,
                        u32 avgBitrate,
                        u8  profileLevel,
                        u8  mpeghMask,
                        u8  referenceChannelLayout,
                        u8  numProfileAndLevelCompatibleSets,
                        u8* profileAndLevelCompatibleSets,
                        MP4Handle decoderSpecificInfoH )
{
  MP4Err                      err;
  GenericSampleEntryAtomPtr   entry = NULL;
  MP4MHAConfigurationAtomPtr  mhaConfig = NULL;
  MP4BitRateAtomPtr           bitRateAtom = NULL;
  MP4MpegHProfileAndLevelAtomPtr  mhaPAtom = NULL;
  MP4TrackAtomPtr             trak;
  MP4Media                    media;
  MP4MpegHSampleEntryAtomPtr  mpeghHSampleEntry;
  u32                         atomType;
  u32                         timeScale;
  u32                         infoSize;

  if ( theTrack == NULL || sampleDescriptionH == NULL )
  {
    BAILWITHERROR( MP4BadParamErr );
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if ( !( trak->newTrackFlags & MP4NewTrackIsAudio ) )
  {
    BAILWITHERROR( MP4BadParamErr );
  }

  switch( (mpeghMask & MPEGHMaskUseStreamingFormat) | (mpeghMask & MPEGHMaskUseMultipleStreamsFormat) )
  {
  case 0:
      atomType = MP4MpegHSampleEntryAtomTypeMHA1;
      break;

  case MPEGHMaskUseStreamingFormat:
      atomType = MP4MpegHSampleEntryAtomTypeMHM1;
      break;

  case MPEGHMaskUseMultipleStreamsFormat:
      atomType = MP4MpegHSampleEntryAtomTypeMHA2;
      break;

  case MPEGHMaskUseMultipleStreamsFormat + MPEGHMaskUseStreamingFormat:
      atomType = MP4MpegHSampleEntryAtomTypeMHM2;
      break;
  }

  err = MP4CreateMpegHSampleEntryAtom( atomType, (MP4MpegHSampleEntryAtomPtr*) &entry );  if (err) goto bail;

  mpeghHSampleEntry = (MP4MpegHSampleEntryAtomPtr)entry;
  err = MP4GetTrackMedia( theTrack, &media );                                             if (err) goto bail;
  err = MP4GetMediaTimeScale( media, &timeScale );                                        if (err) goto bail;
  mpeghHSampleEntry->timeScale = (timeScale <= 0xFFFF ? timeScale : 0);

  entry->dataReferenceIndex = dataReferenceIndex;
  err = MP4GetHandleSize( decoderSpecificInfoH, &infoSize );                              if (err) goto bail;

  if ((mpeghMask & MPEGHMaskWriteConfigBox) || (mpeghMask & MPEGHMaskUseStreamingFormat) == 0)
  {
    err = MP4CreateMHAConfigurationAtom( (MP4MHAConfigurationAtomPtr*) &mhaConfig );      if (err) goto bail;
    mhaConfig->configurationVersion   = 1;
    mhaConfig->MPEGHAudioProfileLevel = profileLevel;
    mhaConfig->referenceChannelLayout = referenceChannelLayout;
    mhaConfig->mpegh3daConfigLength   = infoSize;
    mhaConfig->mpegh3daConfig         = (char*)calloc(infoSize, 1);
    memcpy(mhaConfig->mpegh3daConfig, (char*)decoderSpecificInfoH[0], infoSize);

    err = MP4AddListEntry( (void*) mhaConfig, entry->ExtensionAtomList );                 if (err) goto bail;
  }

  if ( mpeghMask & MPEGHMaskWriteBitrateBox )
  {
    err = MP4CreateBitRateAtom( &bitRateAtom );                                           if (err) goto bail;
    bitRateAtom->buffersizeDB = decoderBufferSize;
    bitRateAtom->max_bitrate = maxBitrate;
    bitRateAtom->avg_bitrate = avgBitrate;
    err = MP4AddListEntry( (void*) bitRateAtom, entry->ExtensionAtomList );               if (err) goto bail;
  }

  if ( numProfileAndLevelCompatibleSets > 0 )
  {
    err = MP4CreateMpegHProfileAndLevelAtom ( &mhaPAtom );
    mhaPAtom->numCompatibleSets = numProfileAndLevelCompatibleSets;
    mhaPAtom->compatibleSetIndication = (u8*)calloc(numProfileAndLevelCompatibleSets, 1);
    memcpy(mhaPAtom->compatibleSetIndication, profileAndLevelCompatibleSets, numProfileAndLevelCompatibleSets);
    
    err = MP4AddListEntry( (void*) mhaPAtom, entry->ExtensionAtomList );                 if (err) goto bail;
  }

  err = atomPtrToSampleEntryH( sampleDescriptionH, (MP4AtomPtr) entry );                  if (err) goto bail;

bail:
  if ( mhaConfig )
  {
    if (entry) err = MP4DeleteListEntryAtom( entry->ExtensionAtomList, MP4MHAConfigurationAtomType );
    mhaConfig->destroy( (MP4AtomPtr) mhaConfig );
  }

  if ( bitRateAtom )
  {
    if (entry) err = MP4DeleteListEntryAtom( entry->ExtensionAtomList, MP4BitRateAtomType );
    bitRateAtom->destroy( (MP4AtomPtr) bitRateAtom );
  }

  if ( entry )
    entry->destroy( (MP4AtomPtr) entry );

  TEST_RETURN( err );
  return err;
}

MP4_EXTERN ( MP4Err )
MHAGetMediaDecoderConfig( MP4TrackReader theReader,
                          MP4Handle decoderConfigH )
{
  MP4Err                      err = MP4NoErr;
  u32                         handlerType, outDataReferenceIndex, size;
  MP4Media                    media;
  MP4Handle                   mediaDescriptionH;
  MP4AtomPtr                  sampleEntry;
  MP4MpegHSampleEntryAtomPtr  mpeghSampleEntry;
  MP4MHAConfigurationAtomPtr  mhaConfig = NULL;
  MP4BitRateAtomPtr           bitRateAtom;
  MP4MpegHProfileAndLevelAtomPtr  mhaPAtom;
  MP4TrackReaderPtr           reader = (MP4TrackReaderPtr) theReader;
  int                         outputBufferOffset = 0;
  MP4InputStreamPtr           is;

  err = MP4GetTrackMedia( reader->track, &media );                                                  if (err) goto bail;
  err = MP4GetMediaHandlerDescription( media, &handlerType, NULL );                                 if (err) goto bail;
  err = MP4NewHandle( 0, &mediaDescriptionH );                                                      if (err) goto bail;
  err = MP4GetMediaSampleDescription( media, 1, mediaDescriptionH, &outDataReferenceIndex );        if (err) goto bail;

  if (handlerType != MP4AudioHandlerType)
  {
    err = MP4InvalidMediaErr;
    if (err) goto bail;
  }

  err = MP4GetHandleSize( mediaDescriptionH, &size );                                               if (err) goto bail;
  err = MP4CreateMemoryInputStream( *mediaDescriptionH, size, &is );                                if (err) goto bail;
  err = MP4ParseMPEGHAtom( is, &sampleEntry );                                                      if (err) goto bail;

  mpeghSampleEntry = (MP4MpegHSampleEntryAtomPtr)sampleEntry;

  err = MP4GetListEntryAtom( mpeghSampleEntry->ExtensionAtomList, MP4MHAConfigurationAtomType, (MP4AtomPtr*) &mhaConfig );
  if ( err == MP4NotFoundErr )
  {
    if (mpeghSampleEntry->type == MP4MpegHSampleEntryAtomTypeMHM1 || mpeghSampleEntry->type == MP4MpegHSampleEntryAtomTypeMHM2)
    {
      /* mhaC atom is optional for mhm, we reset 'MP4NotFoundErr' to no error */
      err = MP4NoErr;
    }
    else
    {
      BAILWITHERROR( MP4InvalidMediaErr );
    }
  }

  if ( mhaConfig )
  {
    err = mhaConfig->calculateSize( (MP4Atom*) mhaConfig );                                         if (err) goto bail;
    err = MP4SetHandleSize( decoderConfigH, outputBufferOffset + mhaConfig->size );                 if (err) goto bail;
    if ( mhaConfig->size )
    {
      err = mhaConfig->serialize( (MP4Atom*) mhaConfig, *decoderConfigH + outputBufferOffset );     if (err) goto bail;
      outputBufferOffset += mhaConfig->size;
    }
  }

  err = MP4GetListEntryAtom( mpeghSampleEntry->ExtensionAtomList, MP4BitRateAtomType, (MP4AtomPtr*) &bitRateAtom );
  if ( err == MP4NotFoundErr )
  {
    /* Bitrate atom is optional, we reset 'MP4NotFoundErr' to no error */
    err = MP4NoErr;
  }

  if ( bitRateAtom )
  {
    err = bitRateAtom->calculateSize( (MP4Atom*) bitRateAtom );                                     if (err) goto bail;
    err = MP4SetHandleSize( decoderConfigH, outputBufferOffset + bitRateAtom->size );               if (err) goto bail;
    if ( bitRateAtom->size )
    {
      err = bitRateAtom->serialize( (MP4Atom*) bitRateAtom, *decoderConfigH + outputBufferOffset ); if (err) goto bail;
      outputBufferOffset += bitRateAtom->size;
    }
  }

  err = MP4GetListEntryAtom( mpeghSampleEntry->ExtensionAtomList, MP4MpegHProfileAndLevelAtomType, (MP4AtomPtr*) &mhaPAtom );
  if ( err == MP4NotFoundErr )
  {
    /* mhaP atom is optional, we reset 'MP4NotFoundErr' to no error */
    err = MP4NoErr;
  }

  if ( mhaPAtom )
  {
    err = mhaPAtom->calculateSize( (MP4Atom*) mhaPAtom );                                     if (err) goto bail;
    err = MP4SetHandleSize( decoderConfigH, outputBufferOffset + mhaPAtom->size );            if (err) goto bail;
    if ( mhaPAtom->size )
    {
      err = mhaPAtom->serialize( (MP4Atom*) mhaPAtom, *decoderConfigH + outputBufferOffset ); if (err) goto bail;
      outputBufferOffset += mhaPAtom->size;
    }
  }

bail:
  TEST_RETURN( err );
  return err;
}

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
                               MP4Handle specificInfoH )
{
  MP4Err                      err = MP4NoErr;
  u32                         handlerType, outDataReferenceIndex, size;
  MP4Handle                   mediaDescriptionH;
  MP4AtomPtr                  sampleEntry;
  MP4MpegHSampleEntryAtomPtr  mpeghSampleEntry;
  MP4MHAConfigurationAtomPtr  mhaConfig = NULL;
  MP4BitRateAtomPtr           bitRateAtom;
  MP4MpegHProfileAndLevelAtomPtr  mhaPAtom;
  int                         outputBufferOffset = 0;
  MP4InputStreamPtr           is;

  err = MP4GetMediaHandlerDescription( theMedia, &handlerType, NULL );                                 if (err) goto bail;
  err = MP4NewHandle( 0, &mediaDescriptionH );                                                      if (err) goto bail;
  err = MP4GetMediaSampleDescription( theMedia, 1, mediaDescriptionH, &outDataReferenceIndex );        if (err) goto bail;

  if (handlerType != MP4AudioHandlerType)
  {
    err = MP4InvalidMediaErr;
    if (err) goto bail;
  }

  err = MP4GetHandleSize( mediaDescriptionH, &size );                                               if (err) goto bail;
  err = MP4CreateMemoryInputStream( *mediaDescriptionH, size, &is );                                if (err) goto bail;
  err = MP4ParseMPEGHAtom( is, &sampleEntry );                                                      if (err) goto bail;

  mpeghSampleEntry = (MP4MpegHSampleEntryAtomPtr)sampleEntry;

  err = MP4GetListEntryAtom( mpeghSampleEntry->ExtensionAtomList, MP4MHAConfigurationAtomType, (MP4AtomPtr*) &mhaConfig ); 
  if ( err == MP4NotFoundErr )
  {
    if (mpeghSampleEntry->type == MP4MpegHSampleEntryAtomTypeMHM1 || mpeghSampleEntry->type == MP4MpegHSampleEntryAtomTypeMHM2)
    {
      /* mhaC atom is optional for mhm, we reset 'MP4NotFoundErr' to no error */
      err = MP4NoErr;
    }
    else
    {
      BAILWITHERROR( MP4InvalidMediaErr );
    }
  }

  if ( mhaConfig )
  {
    if (outConfigVersion)
    {
      *outConfigVersion = mhaConfig->configurationVersion;
    }
    if (outProfileLevel)
    {
      *outProfileLevel = mhaConfig->MPEGHAudioProfileLevel;
    }
    if (outRefChannelLayout)
    {
      *outRefChannelLayout = mhaConfig->referenceChannelLayout;
    }
    if (specificInfoH)
    {
      err = MP4SetHandleSize( specificInfoH, mhaConfig->mpegh3daConfigLength );                     if (err) goto bail;
      if ( mhaConfig->mpegh3daConfigLength )
      {
        memmove( *specificInfoH, mhaConfig->mpegh3daConfig, mhaConfig->mpegh3daConfigLength );
        err = MP4SetHandleSize( specificInfoH, mhaConfig->mpegh3daConfigLength );                   if (err) goto bail;
      }
      else
      {
        MP4SetHandleSize( specificInfoH, 0 );
      }
    }

    err = MP4GetListEntryAtom( mpeghSampleEntry->ExtensionAtomList, MP4BitRateAtomType, (MP4AtomPtr*) &bitRateAtom );
    if ( err == MP4NotFoundErr )
    {
      /* Bitrate atom is optional, we reset 'MP4NotFoundErr' to no error */
      err = MP4NoErr;
    }

    if ( bitRateAtom )
    {
      if (outBufferSize)
      {
        *outBufferSize = bitRateAtom->buffersizeDB;
      }
      if (outMaxBitrate)
      {
        *outMaxBitrate = bitRateAtom->max_bitrate;
      }
      if (outAvgBitrate)
      {
        *outAvgBitrate = bitRateAtom->avg_bitrate;
      }
    }
  }
  else
  {
    MP4SetHandleSize( specificInfoH, 0 );
  }

  err = MP4GetListEntryAtom(mpeghSampleEntry->ExtensionAtomList, MP4MpegHProfileAndLevelAtomType, (MP4AtomPtr*)&mhaPAtom);
  if (err == MP4NotFoundErr)
  {
      /* mhaP atom is optional, we reset 'MP4NotFoundErr' to no error */
      err = MP4NoErr;
  }

  if (mhaPAtom)
  {
      if (outNumProfileAndLevelCompatibleSets)
      {
          *outNumProfileAndLevelCompatibleSets = mhaPAtom->numCompatibleSets;
      }
      if (outProfileAndLevelCompatibleSets)
      {
          err = MP4SetHandleSize(outProfileAndLevelCompatibleSets, mhaPAtom->numCompatibleSets);                     if (err) goto bail;
          if (mhaPAtom->numCompatibleSets)
          {
              memmove(*outProfileAndLevelCompatibleSets, mhaPAtom->compatibleSetIndication, mhaPAtom->numCompatibleSets);
              err = MP4SetHandleSize(outProfileAndLevelCompatibleSets, mhaPAtom->numCompatibleSets);                   if (err) goto bail;
          }
          else
          {
              MP4SetHandleSize(outProfileAndLevelCompatibleSets, 0);
          }
      }
  }

bail:
  TEST_RETURN( err );
  return err;
}

MP4_EXTERN ( MP4Err )
MHAGetMediaDecoderType( MP4Media theMedia,
                        u32 sampleDescIndex,
                        u32 *outProfileLevel,
                        u32 *outBufferSize,
                        MP4Handle specificInfoH )
{
  return MHAGetMediaDecoderInformation(
    theMedia, sampleDescIndex,
    NULL, /* cfgversion */
    outProfileLevel,
    NULL, /* refChannelLayout */
    outBufferSize,
    NULL, /* max */
    NULL, /* avg */
    NULL,
    NULL,
    specificInfoH );
}
