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

MP4_EXTERN ( MP4Err )
MP4ParseMPEGHAtom( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom )
{
  return MP4ParseMPEGHAtomUsingProtoList( inputStream, NULL, 0, outAtom );
}

MP4_EXTERN ( MP4Err )
MP4CreateMPEGHAtom( u32 atomType, MP4AtomPtr *outAtom )
{
  MP4Err      err = MP4NoErr;
  MP4AtomPtr  newAtom;

  switch ( atomType )
  {
  case MP4MHAConfigurationAtomType:
    err = MP4CreateMHAConfigurationAtom( (MP4MHAConfigurationAtomPtr *) &newAtom );
    break;

  case MP4MpegHProfileAndLevelAtomType:
    err = MP4CreateMpegHProfileAndLevelAtom ( (MP4MpegHProfileAndLevelAtomPtr *) &newAtom );
    break;

  case MP4MpegHSampleEntryAtomTypeMHA1:
  case MP4MpegHSampleEntryAtomTypeMHA2:
  case MP4MpegHSampleEntryAtomTypeMHM1:
  case MP4MpegHSampleEntryAtomTypeMHM2:
    err = MP4CreateMpegHSampleEntryAtom( atomType, (MP4MpegHSampleEntryAtomPtr *) &newAtom );
    break;

  default:
    err = MP4CreateAtom( atomType, &newAtom );
    break;
  }

  if ( err == MP4NoErr )
  {
    *outAtom = newAtom;
  }

  return err;
}

MP4_EXTERN ( MP4Err )
MP4ParseMPEGHAtomUsingProtoList( MP4InputStreamPtr inputStream, u32* protoList, u32 defaultAtom, MP4AtomPtr *outAtom )
{
  MP4Err     err = MP4NoErr;
  long       bytesParsed = 0L;
  MP4Atom	   protoAtom;
  MP4AtomPtr atomProto = &protoAtom;
  MP4AtomPtr newAtom;
  char       typeString[ 8 ];
  char       msgString[ 80 ];
  u64        beginAvail;
  u64        consumedBytes;
  u32        useDefaultAtom;
    
  if ((inputStream == NULL) || (outAtom == NULL) )
  {
    BAILWITHERROR( MP4BadParamErr )
  }
    
  *outAtom = NULL;
  beginAvail = inputStream->available;
  useDefaultAtom = 0;
  inputStream->msg( inputStream, "{" );
  inputStream->indent++;
  err = MP4CreateBaseAtom( atomProto );                                                   if ( err ) goto bail;

  atomProto->streamOffset = inputStream->getStreamOffset( inputStream );

  /* atom size */
  err = inputStream->read32( inputStream, &atomProto->size, NULL );                       if ( err ) goto bail;
  if ( atomProto->size == 0 )
  {
    /* BAILWITHERROR( MP4NoQTAtomErr )  */
    u64 the_size;
    the_size = inputStream->available + 4;
    if (the_size >> 32)
    {
      atomProto->size = 1;
      atomProto->size64  = the_size + 8;
    }
    else atomProto->size = (u32) the_size;
  }

  if ((atomProto->size != 1) && ((atomProto->size - 4) > inputStream->available))
  {
    BAILWITHERROR( MP4BadDataErr )
  }

  bytesParsed += 4L;

  sprintf( msgString, "atom size is %d", atomProto->size );
  inputStream->msg( inputStream, msgString );

  /* atom type */
  err = inputStream->read32( inputStream, &atomProto->type, NULL );                       if ( err ) goto bail;
  bytesParsed += 4L;
  MP4TypeToString( atomProto->type, typeString );
  sprintf( msgString, "atom type is '%s'", typeString );
  inputStream->msg( inputStream, msgString );
  if ( atomProto->type == MP4ExtendedAtomType )
  {
    err = inputStream->readData( inputStream, 16, (char*) atomProto->uuid, NULL );        if ( err ) goto bail;
    bytesParsed += 16L;
  }

  /* large atom */
  if ( atomProto->size == 1 )
  {
    u32 size;
    err = inputStream->read32( inputStream, &size, NULL );                                if ( err ) goto bail;
    /* if ( size )
    BAILWITHERROR( MP4NoLargeAtomSupportErr ) */
    atomProto->size64 = size;
    atomProto->size64 <<= 32;
    err = inputStream->read32( inputStream, &size, NULL );                                if ( err ) goto bail;
    atomProto->size64 |= size;
    atomProto->size = 1;
    bytesParsed += 8L;
  }

  atomProto->bytesRead = (u32) bytesParsed;
  if ((atomProto->size != 1) && ( ((long) atomProto->size) < bytesParsed ))
  {
    BAILWITHERROR( MP4BadDataErr )
  }

  if ( protoList )
  {
    while ( *protoList  )
    {
      if ( *protoList == atomProto->type )
        break;
      protoList++;
    }
    if ( *protoList == 0 )
    {
      useDefaultAtom = 1;
    }
  }

  err = MP4CreateMPEGHAtom( useDefaultAtom ? defaultAtom : atomProto->type, &newAtom );   if ( err ) goto bail;
  sprintf( msgString, "atom name is '%s'", newAtom->name );
  inputStream->msg( inputStream, msgString );
  err = newAtom->createFromInputStream( newAtom, atomProto, (char*) inputStream );        if ( err ) goto bail;
  consumedBytes = beginAvail - inputStream->available;
  if ((atomProto->size != 1 ) && ( consumedBytes != atomProto->size ))
  {
    sprintf( msgString, "##### atom size is %d but parse used %lld bytes ####", atomProto->size, consumedBytes );
    inputStream->msg( inputStream, msgString );
    if (consumedBytes < atomProto->size)
    {
      u32 x;
      u32 i;
      for (i=0; i<(atomProto->size)-consumedBytes; i++) inputStream->read8(inputStream, &x, NULL );
    }
  }
  else if ((atomProto->size == 1 ) && ( consumedBytes != atomProto->size64 ))
  {
    sprintf( msgString, "##### atom size is %lld but parse used %lld bytes ####", atomProto->size64, consumedBytes );
    inputStream->msg( inputStream, msgString );
    if (consumedBytes < atomProto->size64)
    {
      u32 x;
      u64 i;
      for (i=0; i<(atomProto->size64)-consumedBytes; i++) inputStream->read8(inputStream, &x, NULL );
    }
  }

  *outAtom = newAtom;
  inputStream->indent--;
  inputStream->msg( inputStream, "}" );

bail:
  TEST_RETURN( err );
  return err;
}
