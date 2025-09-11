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

#ifndef _USAC_INTERFACE_H_
#define _USAC_INTERFACE_H_

enum {
  LEN_USAC_INDEP_FLAG=1,
  LEN_CORE_MODE=1,
  LEN_ACELP_CORE_MODE_IDX=3,
  LEN_MODE=5,
  LEN_MODE_FULLBAND_LPD = 3,
  LEN_BPF_CONTROL=1,
  LEN_MODE_LPC=2,
  LEN_NOISE_FAC=3,
  LEN_NOISE_LEV=3,
  LEN_NOISE_OFF=5,
  LEN_GLOBAL_GAIN_TCX=7,
  LEN_COM_TW=1,
  LEN_TW_PRES=1,
  LEN_TW_RATIO=3,
  NUM_TW_NODES=16,
  LEN_RESET_ARIT_DEC=1,
  LEN_FAC_DATA_PRESENT=1,
  LEN_CORE_MODE_LAST=1,
  LEN_SHORT_FAC_FLAG=1,
  LEN_ELEMENT_LENGTH=16
};

typedef enum {
  CORE_MODE_FD,
  CORE_MODE_TD
} USAC_CORE_MODE;


#endif /* _USAC_INTERFACE_H_ */
