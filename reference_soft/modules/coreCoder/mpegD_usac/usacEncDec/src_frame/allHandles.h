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

Copyright (c) ISO/IEC 2000.

*************************************************************************/

#ifndef _allHandle_h_
#define _allHandle_h_

typedef struct tagAACDecoder *HANDLE_AACDECODER;
typedef struct tagUsacDecoder *HANDLE_USAC_DECODER;
typedef struct tagProgConfig *HANDLE_PROG_CONFIG;

#ifdef I2R_LOSSLESS
typedef struct  tagSLSDecoder *HANDLE_SLSDECODER;
#endif

/*
  VERSION 1 handle type definitions
*/
typedef struct tagBsBitBufferStruct *HANDLE_BSBITBUFFER;
typedef struct tagBsBitStreamStruct *HANDLE_BSBITSTREAM;
typedef struct tag_buffer           *HANDLE_BUFFER;
typedef struct tag_modulo_buffer    *HANDLE_MODULO_BUF_VM;

/*
  VERSION 2 handle type definitions
*/
typedef struct tag_resilience       *HANDLE_RESILIENCE;
typedef struct tag_hcr              *HANDLE_HCR;
typedef struct tagEscInstanceData   *HANDLE_ESC_INSTANCE_DATA;
typedef struct tag_concealment      *HANDLE_CONCEALMENT;

typedef struct tagEpConverter       *HANDLE_EPCONVERTER;


/*
  the overall handler
*/
typedef struct tagDecoderGeneral {
  HANDLE_BUFFER            hVm;
  HANDLE_BUFFER            hHcrSpecData;
  HANDLE_RESILIENCE        hResilience;
  HANDLE_HCR               hHcrInfo;
  HANDLE_ESC_INSTANCE_DATA hEscInstanceData;
  HANDLE_CONCEALMENT       hConcealment;
} T_DECODERGENERAL;
typedef struct tagDecoderGeneral *HANDLE_DECODER_GENERAL;

#ifdef DRC
typedef struct tagDRC *HANDLE_DRC;
#endif

#endif /* _allHandle_h_ */
