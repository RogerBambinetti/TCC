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

#include "MPEGHAtoms.h"

static void destroy( MP4AtomPtr s )
{
  MP4Err err = MP4NoErr;
  MP4MpegHSampleEntryAtomPtr self = (MP4MpegHSampleEntryAtomPtr) s;

  if ( self == NULL )
  {
    BAILWITHERROR( MP4BadParamErr );
  }

  DESTROY_ATOM_LIST_F( ExtensionAtomList );

  if ( self->super )
  {
    self->super->destroy( s );
  }

bail:
  TEST_RETURN( err );
  return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
  MP4Err err = MP4NoErr;
  MP4MpegHSampleEntryAtomPtr self = (MP4MpegHSampleEntryAtomPtr) s;

  err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
  buffer += self->bytesWritten;
  PUTBYTES( self->reserved, 6 );
  PUT16( dataReferenceIndex );
  PUTBYTES( self->reserved2, 8 );
  PUT16( reserved3 );
  PUT16( reserved4 );
  PUT32( reserved5 );
  PUT16( timeScale );
  PUT16( reserved6 );
  SERIALIZE_ATOM_LIST( ExtensionAtomList );
  assert( self->bytesWritten == self->size );

bail:
  TEST_RETURN( err );
  return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
  MP4Err err = MP4NoErr;
  MP4MpegHSampleEntryAtomPtr self = (MP4MpegHSampleEntryAtomPtr) s;

  err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
  self->size += 14 + (1*4)+(5*2);
  ADD_ATOM_LIST_SIZE( ExtensionAtomList );

bail:
  TEST_RETURN( err );
  return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
  MP4Err err = MP4NoErr;
  MP4MpegHSampleEntryAtomPtr self = (MP4MpegHSampleEntryAtomPtr) s;

  if ( self == NULL )
  {
    BAILWITHERROR( MP4BadParamErr );
  }

  err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

  GETBYTES( 6, reserved );
  GET16( dataReferenceIndex );
  GETBYTES( 8, reserved2 );
  GET16( reserved3 );
  GET16( reserved4 );
  GET32( reserved5 );
  GET16( timeScale );
  GET16( reserved6 );

  while (self->bytesRead < self->size)
  {
    MP4AtomPtr atm;
    err = MP4ParseMPEGHAtom( inputStream, &atm ); if (err) goto bail;
    self->bytesRead += atm->size;
    err = MP4AddListEntry( (void*) atm, self->ExtensionAtomList ); if (err) goto bail;
  }

bail:
  TEST_RETURN( err );
  return err;
}

MP4Err MP4CreateMpegHSampleEntryAtom( u32 atomType, MP4MpegHSampleEntryAtomPtr *outAtom )
{
  MP4Err err = MP4NoErr;
  MP4MpegHSampleEntryAtomPtr self;

  self = (MP4MpegHSampleEntryAtomPtr) calloc( 1, sizeof(MP4MpegHSampleEntryAtom) );
  TESTMALLOC( self );

  err = MP4CreateBaseAtom( (MP4AtomPtr) self ); if ( err ) goto bail;
  self->type                  = atomType;
  self->name                  = "mpeg-h audio sample entry";
  err = MP4MakeLinkedList( &self->ExtensionAtomList ); if (err) goto bail;
  self->createFromInputStream = (cisfunc) createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->reserved3             = 0;
  self->reserved4             = 16;
  self->timeScale             = 44100;
  *outAtom = self;

bail:
  TEST_RETURN( err );
  return err;
}
