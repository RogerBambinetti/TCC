/************************************************************************

This software module was originally developed by Fraunhofer IIS and 
VoiceAge Corp. in the course of development of the ISO/IEC 23008-3 for 
reference purposes and its  performance may not have been optimized. 
This software module is an implementation of one or more tools as 
specified by the ISO/IEC 23008-3 standard. ISO/IEC gives you a 
royalty-free, worldwide, non-exclusive,copyright license to copy, 
distribute, and make derivative works of this software module or 
modifications thereof for use in implementations or products claiming 
conformance to the ISO/IEC 23008-3 standard and which satisfy any specified 
conformance criteria. 
Those intending to use this software module in products are advised that 
its use may infringe existing patents.

ISO/IEC have no liability for use of this software module or 
modifications thereof. Copyright is not released for products that do
not conform to the ISO/IEC 23008-3 standard.

Fraunhofer IIS and VoiceAge Corp. retain full right to modify and use the 
code for its own purpose, assign or donate the code to a third party and 
to inhibit third parties from using the code for products that do not 
conform to MPEG-related ITU Recommendations and/or ISO/IEC International 
Standards.

This copyright notice must be included in all copies or derivative works.

Copyright (c) ISO/IEC 2008.

*************************************************************************/


#ifndef options_h
#define options_h


/* Define LAGW in order to use Gaussian Lag Windowing */
#define LAGW
/* Define COSINE_INTERP in order to use sample based cosine interpolation */
#define COSINE_INTERP
/* Define USE_CHOLESKY in order to use Cholesky instead of Levinson algorithm */
#define USE_CHOLESKY

/* AMR-WB+ input/output sampling rate filters */
/* Choice of filter should be selected according to the hardware specification */
/* Both can be selected if required */
/* FILTER_44khz allow 11025, 22050 and 44100 Hz sampling rates */
/* FILTER_48khz allow 8, 16, 24, 32 and 48 kHz sampling rates */
#define FILTER_44kHz
#define FILTER_48kHz

#endif


