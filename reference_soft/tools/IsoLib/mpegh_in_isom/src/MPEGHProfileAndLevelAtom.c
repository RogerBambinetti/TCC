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
  MP4MpegHProfileAndLevelAtomPtr self = (MP4MpegHProfileAndLevelAtomPtr) s;

  if ( self == NULL )
  {
    BAILWITHERROR( MP4BadParamErr );
  }
  if ( self->compatibleSetIndication )
  {
    free( self->compatibleSetIndication );
    self->compatibleSetIndication = NULL;
  }

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
  MP4MpegHProfileAndLevelAtomPtr self = (MP4MpegHProfileAndLevelAtomPtr) s;
  u32 i;

  err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
  buffer += self->bytesWritten;

  PUT8( numCompatibleSets );
  for ( i = 0; i < self->numCompatibleSets; i++ )
  {
    PUT8_V( self->compatibleSetIndication[i] );
  }
  assert( self->bytesWritten == self->size );

bail:
  TEST_RETURN( err );
  return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
  MP4Err err = MP4NoErr;
  MP4MpegHProfileAndLevelAtomPtr self = (MP4MpegHProfileAndLevelAtomPtr) s;

  err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;

  self->size +=
    1 +                         /* u8   numCompatibleSets;   */
    self->numCompatibleSets;    /* u8*  compatibleSetIndication; */

bail:
  TEST_RETURN( err );
  return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
  MP4Err err = MP4NoErr;
  MP4MpegHProfileAndLevelAtomPtr self = (MP4MpegHProfileAndLevelAtomPtr) s;
  u8 entries;
  u8* p;

  if ( self == NULL )
  {
    BAILWITHERROR( MP4BadParamErr );
  }

  err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

  GET8( numCompatibleSets );
  
  if (self->numCompatibleSets > 0) {
    self->compatibleSetIndication = (u8 *) calloc( self->numCompatibleSets, sizeof(u8) );
    TESTMALLOC( self->compatibleSetIndication )
  
    for ( entries = 0, p = self->compatibleSetIndication; entries < self->numCompatibleSets; entries++, p++ )
    {
      u32 indication;
      GET8_V( indication );
      *p = indication;
    }
  }

bail:
  TEST_RETURN( err );
  return err;
}

MP4Err MP4CreateMpegHProfileAndLevelAtom( MP4MpegHProfileAndLevelAtomPtr *outAtom )
{
  MP4Err err;
  MP4MpegHProfileAndLevelAtomPtr self;

  self = (MP4MpegHProfileAndLevelAtomPtr) calloc( 1, sizeof(MP4MpegHProfileAndLevelAtom) );
  TESTMALLOC( self );

  err = MP4CreateBaseAtom( (MP4AtomPtr) self ); if ( err ) goto bail;
  self->type                  = MP4MpegHProfileAndLevelAtomType;
  self->name                  = "mpeg-h audio profile and level compatibility sets";
  self->createFromInputStream = (cisfunc) createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  *outAtom = self;

bail:
  TEST_RETURN( err );
  return err;
}

