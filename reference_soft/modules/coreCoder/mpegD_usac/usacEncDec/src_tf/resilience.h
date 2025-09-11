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

Copyright (c) ISO/IEC 1996, 1997, 1998, 1999.

*************************************************************************/
#ifndef _resilience_h_
#define _resilience_h_


HANDLE_RESILIENCE CreateErrorResilience ( char* decPara, 
                                          int epFlag, 
                                          int aacSectionDataResilienceFlag,
                                          int aacScalefactorDataResilienceFlag,
                                          int aacSpectralDataResilienceFlag);

void DeleteErrorResilience( const HANDLE_RESILIENCE ptr);

char GetEPFlag ( const HANDLE_RESILIENCE ptr );

char GetLenOfLongestCwFlag ( const HANDLE_RESILIENCE ptr );

char GetLenOfScfDataFlag ( const HANDLE_RESILIENCE ptr );

char GetReorderSpecPreSortFlag ( const HANDLE_RESILIENCE handle );

char GetReorderSpecFlag ( const HANDLE_RESILIENCE ptr );

char GetConsecutiveReorderSpecFlag ( const HANDLE_RESILIENCE handle );

char GetRvlcScfFlag ( const HANDLE_RESILIENCE ptr );

char GetScfBitFlag ( const HANDLE_RESILIENCE ptr );

char GetScfConcealmentFlag ( const HANDLE_RESILIENCE ptr );

char GetVcb11Flag ( const HANDLE_RESILIENCE handle );

#endif /* _resilience_h_ */
