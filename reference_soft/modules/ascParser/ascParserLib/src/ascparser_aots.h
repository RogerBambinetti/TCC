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
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef __ASCPARSER_AOTS_H__
#define __ASCPARSER_AOTS_H__

typedef enum _ASCPARSER_AUDIO_OBJECT_TYPE
{
  AOT_NULL_OBJECT      = 0, 
  AOT_AAC_MAIN         = 1,
  AOT_AAC_LC           = 2,
  AOT_AAC_SSR          = 3,
  AOT_AAC_LTP          = 4,
  AOT_SBR              = 5,
  AOT_AAC_SCAL         = 6,
  AOT_TWIN_VQ          = 7,
  AOT_CELP             = 8,
  AOT_HVXC             = 9,
  AOT_RSVD_10          = 10, /* (reserved)                                */
  AOT_RSVD_11          = 11, /* (reserved)                                */
  AOT_TTSI 	       = 12, /* TTSI Object                               */
  AOT_MAIN_SYNTH       = 13, /* Main Synthetic Object                     */
  AOT_WAV_TAB_SYNTH    = 14, /* Wavetable Synthesis Object                */
  AOT_GEN_MIDI         = 15, /* General MIDI Object                       */
  AOT_ALG_SYNTH_AUD_FX = 16, /* Algorithmic Synthesis and Audio FX Object */
  AOT_ER_AAC_LC        = 17, /* Error Resilient(ER) AAC LC Object         */
  AOT_RSVD_18          = 18, /* (reserved)                                */
  AOT_ER_AAC_LTP       = 19, /* Error Resilient(ER) AAC LTP Object        */
  AOT_ER_AAC_SCAL      = 20, /* Error Resilient(ER) AAC Scalable Object   */
  AOT_ER_TWIN_VQ       = 21, /* Error Resilient(ER) TwinVQ Object         */
  AOT_ER_BSAC          = 22, /* Error Resilient(ER) BSAC Object           */
  AOT_ER_AAC_LD        = 23, /* Error Resilient(ER) AAC LD Object         */
  AOT_ER_CELP          = 24, /* Error Resilient(ER) CELP Object           */
  AOT_ER_HVXC          = 25, /* Error Resilient(ER) HVXC Object           */
  AOT_ER_HILN          = 26, /* Error Resilient(ER) HILN Object           */
  AOT_ER_PARA          = 27, /* Error Resilient(ER) Parametric Object     */
  AOT_RSVD_28          = 28, /* might become SSC                          */
  AOT_PS               = 29, /* PS, Parametric Stereo (includes SBR)      */
  AOT_RSVD_30          = 30, /* (reserved)                                */
  AOT_ESCAPE           = 31, /* (reserved)                                */

  AOT_MP3ONMP4_L1      = 32, /* layer1 in mp4                             */
  AOT_MP3ONMP4_L2      = 33, /* layer2 in mp4                             */
  AOT_MP3ONMP4_L3      = 34, /* layer3 in mp4                             */
  AOT_RSVD_35          = 35, /* (reserved)                                */
  AOT_RSVD_36          = 36, /* (reserved)                                */
  AOT_RSVD_37          = 37, /* (reserved)                                */

  AOT_SAAS             = 42,

  AOT_MP2_AAC_MAIN     = 128,/* virtual AOT MP2 Main Profile              */
  AOT_MP2_AAC_LC       = 129,/* virtual AOT MP2 Low Complexity Profile    */
  AOT_MP2_AAC_SSR      = 130,/* virtual AOT MP2 Scalable Sampling Rate Profile */
  AOT_PLAIN_MP3        = 131,/* virtual AOT for plain mp3 decoding        */

  AOT_INVALID          = 555,
  AOT_DUMMY            = 556

} ASCPARSER_AUDIO_OBJECT_TYPE;

#endif
