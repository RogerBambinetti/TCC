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

#ifndef _FORMATCONVERTER_DATA_H_
#define _FORMATCONVERTER_DATA_H_


#include "formatConverter.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** Set Format Converter parameters, prior allocation.
       Set data of Format Converter parameter struct.
    \param formatConverter_params A pointer to Format Converter parameter struct
    \return Returns 0 on success, otherwise 1.    */        
int setFormatConverterParamsPreAlloc( FORMAT_CONVERTER_PARAMS *formatConverter_params );

/** Set Format Converter parameters.
       Set data of Format Converter parameter struct.
    \param centerFrequeciesNormalized normalized center frequency of each freq band.
    \param formatConverter_params A pointer to Format Converter parameter struct
    \return Returns 0 on success, otherwise 1.    */        
int setFormatConverterParams(   const float *centerFrequenciesNormalized, 
                                const float             *azimuthDeviation,
                                const float             *elevationDeviation,
                                const float             *distance, 
                                FORMAT_CONVERTER_PARAMS *formatConverter_params );


/** Allocate Format Converter parameter struct members.
    Allocate Format Converter parameter struct members.
    \param formatConverter_params A pointer to Format Converter parameter struct.    */
int allocateFormatConverterParams( FORMAT_CONVERTER_PARAMS *formatConverter_params );


/** Free Format Converter parameter struct members.
    Free Format Converter parameter struct members.
    \param formatConverter_params a pointer to Format Converter parameter struct    */
void freeFormatConverterParams( FORMAT_CONVERTER_PARAMS *formatConverter_params );

/** Set Format Converter state.
       Set data of Format Converter state struct.
    \param formatConverter_state A pointer to Format Converter state struct
    \return Returns 0 on success, otherwise 1.    */        
int setFormatConverterState(   const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                                FORMAT_CONVERTER_STATE   *formatConverter_state );      

/** Allocate Format Converter parameter struct members.
    Allocate Format Converter parameter struct members.
    \param formatConverter_params A pointer to Format Control parameter struct.    
    \param formatConverter_state A pointer to Format Control state struct.    */
int allocateFormatConverterState(
                                 const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                                 FORMAT_CONVERTER_STATE           *formatConverter_state );

/** Free Format Converter parameter struct members.
    Free Format Converter parameter struct members.
    \param formatConverter_params a pointer to Format parameter struct    
    \param formatConverter_state a pointer to Format state struct    */
int freeFormatConverterState(
                                const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                                FORMAT_CONVERTER_STATE           *formatConverter_state );      

/** Set input format or output format that is not in list of known formats.
    Each function call either sets input _or_ output format.  */
int formatConverterSetInOutFormat_internal(   
                                const int  inout, 
                                const int                numChannels,
                                const converter_chid_t   *channel_vector );

/** .
    returns 0 on success, converter_init() return value else
    \param formatConverter_params a pointer to Format parameter struct    */    
int formatConverterInit_internal( HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params );

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif


