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

#include "iarFormatConverter_process.h"

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**********************************************************************************************************************************/

void iar_formatConverterProcess_passive_timeDomain( 
                                       const float                      **inputBuffer,
                                       float                            **outputBuffer,
                                       IAR_HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                       IAR_HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    int i,j,k;

    /* reset output buffer */
    for (i=0; i < hFormatConverter_params->formatConverterFrameSize;i++)
    {         
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            outputBuffer[j][i] = 0.f;            
        }        
    }
    
    /* apply dmx matrix */
    for (i=0; i < hFormatConverter_params->formatConverterFrameSize;i++)
    {
        for (j=0; j < hFormatConverter_params->numInputChans; j++)
        {           
            for (k=0; k < hFormatConverter_params->numOutputChans; k++)                
            {
                outputBuffer[k][i] = outputBuffer[k][i] + hFormatConverter_params->dmxMtx[j][k]*inputBuffer[j][i];            
            }
        }
    }    
}
    
/**********************************************************************************************************************************/

void iar_formatConverterProcess_passive_freqDomain( 
                                               const float                      **inputBuffer_real,
                                               const float                      **inputBuffer_imag,
                                               float                            **outputBuffer_real,
                                               float                            **outputBuffer_imag,
                                               IAR_HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                               IAR_HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    int i,j,k;    
    
    /* reset output buffer */
    for (i=0; i < hFormatConverter_params->numFreqBands;i++)
    {         
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            outputBuffer_real[j][i] = 0.f;            
            outputBuffer_imag[j][i] = 0.f;            
        }        
    }
    
    /* apply dmx matrix */
    for (i=0; i < hFormatConverter_params->numFreqBands;i++)
    {
        for (j=0; j < hFormatConverter_params->numInputChans; j++)
        {           
            for (k=0; k < hFormatConverter_params->numOutputChans; k++)                
            {
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
                outputBuffer_real[k][i] = outputBuffer_real[k][i] + hFormatConverter_params->dmxMtx[j][k]*inputBuffer_real[j][i];            
                outputBuffer_imag[k][i] = outputBuffer_imag[k][i] + hFormatConverter_params->dmxMtx[j][k]*inputBuffer_imag[j][i];            
#else
                outputBuffer_real[k][i] = outputBuffer_real[k][i] + hFormatConverter_params->dmxMtxFreq[i][j][k]*inputBuffer_real[j][i];            
                outputBuffer_imag[k][i] = outputBuffer_imag[k][i] + hFormatConverter_params->dmxMtxFreq[i][j][k]*inputBuffer_imag[j][i];                           
#endif
            }
        }
    }    
}
    
/**********************************************************************************************************************************/

void iar_formatConverterProcess_active_freqDomain_phaseAlign( 
                                               const float                        **inputBuffer_real,
                                               const float                        **inputBuffer_imag,
                                               float                              **outputBuffer_real,
                                               float                              **outputBuffer_imag,
                                               IAR_HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                               IAR_HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state,
                                               FILE                                *fpR3T)
{
    
    /* apply phase align dmx */
    iar_phaseAlignProcess(  hFormatConverter_state->handlePhaseAligner, 
                        inputBuffer_real, 
                        inputBuffer_imag, 
                        outputBuffer_real, 
                        outputBuffer_imag,
                        fpR3T); 
}


/**********************************************************************************************************************************/
void formatConverter_process_STFT(
                                    const float                       **inputBufferStft, 
                                    float                             **outputBufferStft,
                                    IAR_HANDLE_FORMAT_CONVERTER_PARAMS  FormatConverter_params, 
                                    IAR_HANDLE_FORMAT_CONVERTER_STATE   hFormatConverter_state,
                                    const int                           rendering3DType)
{
    activeDmxProcess_STFT(  hFormatConverter_state->handleActiveDmxStft,
                            inputBufferStft,
                            outputBufferStft,
                            rendering3DType);
}


/**********************************************************************************************************************************/
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
void iar_find_channel( 
                                                const float                      **inputBuffer_real,
                                                const float                      **inputBuffer_imag,
                                                float                            **outputBuffer_real,
                                                float                            **outputBuffer_imag,
                                                IAR_HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                                IAR_HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{

    int i, j;
    
    for(i=0; i < hFormatConverter_params->numInputChans; i++) {
        for(j=0; j < hFormatConverter_params->numFreqBands; j++) {
            outputBuffer_real[i][j] = inputBuffer_real[i][j];
            outputBuffer_imag[i][j] = inputBuffer_imag[i][j];
        }            
    }        
    
    for(i=0; i < hFormatConverter_params->eqControlMatrixSize; i++) {
        for(j=0; j < hFormatConverter_params->numFreqBands; j++) {
            outputBuffer_real[hFormatConverter_params->eqControlMatrix[i][0]][j] = (hFormatConverter_params->formatConverterParams_internal)->eq[hFormatConverter_params->eqControlMatrix[i][1]][j]*inputBuffer_real[hFormatConverter_params->eqControlMatrix[i][0]][j];
            outputBuffer_imag[hFormatConverter_params->eqControlMatrix[i][0]][j] = (hFormatConverter_params->formatConverterParams_internal)->eq[hFormatConverter_params->eqControlMatrix[i][1]][j]*inputBuffer_imag[hFormatConverter_params->eqControlMatrix[i][0]][j];
        }            
    } 
    
    for(i=0; i < hFormatConverter_params->gainCompensationControlMatrixSize; i++) {
        for(j=0; j < hFormatConverter_params->numFreqBands; j++) {
            outputBuffer_real[hFormatConverter_params->gainCompensationControlMatrix[i]][j] = hFormatConverter_params->gainCompensationValue*inputBuffer_real[hFormatConverter_params->gainCompensationControlMatrix[i]][j];
            outputBuffer_imag[hFormatConverter_params->gainCompensationControlMatrix[i]][j] = hFormatConverter_params->gainCompensationValue*inputBuffer_imag[hFormatConverter_params->gainCompensationControlMatrix[i]][j];
        }            
    }     
}
#endif 
/**********************************************************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */




