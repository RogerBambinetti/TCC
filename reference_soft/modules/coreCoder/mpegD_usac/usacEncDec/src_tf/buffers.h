/************************************************************************

This software module was originally developed by Fraunhofer IIS in the
course of development of the ISO/IEC 23008-3 for reference purposes and
its  performance may not have been optimized. This software module is an
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

Copyright (c) ISO/IEC 1998.

*************************************************************************/
#ifndef _buffers_h_
#define _buffers_h_

#include "allHandles.h"
#include "buffer.h"

HANDLE_BUFFER  CreateBuffer ( unsigned long bufferSize );
void DeleteBuffer (HANDLE_BUFFER handle) ;

void           FinishWriting ( HANDLE_BUFFER handle );

unsigned long  GetReadBitCnt( HANDLE_BUFFER handle );

unsigned long  GetWriteBitCnt( HANDLE_BUFFER handle );

long           GetBits ( int               n,
                         CODE_TABLE        code,
                         HANDLE_RESILIENCE hResilience,
                         HANDLE_ESC_INSTANCE_DATA    hEPInfo,
                         HANDLE_BUFFER     hVm );

long           GetBitsAhead ( int               n,
                              CODE_TABLE        code,
                              HANDLE_RESILIENCE hResilience,
                              HANDLE_ESC_INSTANCE_DATA    hEPInfo,
                              HANDLE_BUFFER     hVm );

HANDLE_BSBITBUFFER GetRemainingBufferBits ( HANDLE_BUFFER handle );

void SkipBits ( HANDLE_BUFFER handle, long numBits);

void           PrepareReading ( HANDLE_BUFFER handle );

void           PrepareWriting ( HANDLE_BUFFER handle );

void PutBits ( int               n,
               HANDLE_BUFFER     handle,
               unsigned long     value );

void           ResetReadBitCnt ( HANDLE_BUFFER handle );

void           ResetWriteBitCnt ( HANDLE_BUFFER handle );

void           RestoreBufferPointer ( HANDLE_BUFFER handle );

void           StoreBufferPointer ( HANDLE_BUFFER handle );

void           StoreDataInBuffer ( HANDLE_BUFFER  fromHandle,
                                   HANDLE_BUFFER  toHandle,
                                   unsigned short nrOfBits );

void TransferBits ( unsigned long     nrOfBits,
                    CODE_TABLE        code,
                    HANDLE_BUFFER     fromHandle,
                    HANDLE_BUFFER     toHandle,
                    HANDLE_ESC_INSTANCE_DATA    hEscInstanceData,
                    HANDLE_RESILIENCE hResilience );

void           setHuffdec2BitBuffer ( HANDLE_BSBITSTREAM  fixed_stream );

void            byte_align ( void ); /* make it void */
void            byte_align_on_bs (HANDLE_BSBITSTREAM bs);
#endif /* _buffers_h_  */
