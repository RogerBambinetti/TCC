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

#ifndef _FORMATCONVERTER_PROCESS_H_
#define _FORMATCONVERTER_PROCESS_H_


#include "formatConverter.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**********************************************************************************************************************************/

void formatConverterProcess_passive_timeDomain( 
                                               const float                      **inputBuffer,
                                               float                            **outputBuffer,
                                               HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                               HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );

/**********************************************************************************************************************************/

void formatConverterProcess_passive_freqDomain( 
                                               const float                      **inputBuffer_real,
                                               const float                      **inputBuffer_imag,
                                               float                            **outputBuffer_real,
                                               float                            **outputBuffer_imag,
                                               HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                               HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
    
/**********************************************************************************************************************************/

void formatConverterProcess_active_freqDomain_phaseAlign( 
                                                         const float                      **inputBuffer_real,
                                                         const float                      **inputBuffer_imag,
                                                         float                            **outputBuffer_real,
                                                         float                            **outputBuffer_imag,
                                                         HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                                         HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
    
/**********************************************************************************************************************************/
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
void formatConverter_applyEqualizationFilters( 
                                                const float                      **inputBuffer_real,
                                                const float                      **inputBuffer_imag,
                                                float                            **outputBuffer_real,
                                                float                            **outputBuffer_imag,
                                                HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                                HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
#endif
/**********************************************************************************************************************************/
void formatConverter_process_STFT(
                                    const float                      **inputBufferStft, 
                                    float                            **outputBufferStft,
                                    HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params, 
                                    HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
/**********************************************************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif
