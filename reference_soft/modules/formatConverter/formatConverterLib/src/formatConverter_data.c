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

#include "formatConverter_data.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**********************************************************************************************************************************/

int setFormatConverterParamsPreAlloc( FORMAT_CONVERTER_PARAMS *formatConverter_params )
{

    int status = 0;

    if ( formatConverter_params == NULL )
    {
        return 1;
    }
    else
    {
                
        
        return status;
    }
    
}

/**********************************************************************************************************************************/

int setFormatConverterParams(   const float             *centerFrequenciesNormalized, 
                                const float             *azimuthDeviation,
                                const float             *elevationDeviation,
                                const float             *distance,
                                FORMAT_CONVERTER_PARAMS *formatConverter_params )
{

    int i,j,k;
    
    int status = 0;

    if ( formatConverter_params == NULL )    
        return 1;
    
    else
    {        
        /* freq domain params */
        if (formatConverter_params->formatConverterMode != FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN) 
        {
            /* apply equalization filters */
            formatConverter_params->applyEqFilters = 1;
            
            if (centerFrequenciesNormalized == NULL)         
                return 1;            

            if (   formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
                || formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT)
            {
                for (i=0; i < formatConverter_params->stftNumErbBands; i++) 
                {
                    formatConverter_params->centerFrequenciesNormalized[i] = centerFrequenciesNormalized[i];                
                }

            }
            else
            {
                for (i=0; i < formatConverter_params->numFreqBands; i++) 
                {
                    formatConverter_params->centerFrequenciesNormalized[i] = centerFrequenciesNormalized[i];                
                }
            }
            /* aeq limits */
            formatConverter_params->eqLimitMax = 8.f;
            formatConverter_params->eqLimitMin = -10.f;
            
        /* time domain params */
        } else {
            /* apply equalization filters */
            formatConverter_params->applyEqFilters = 0;
            
            /* aeq limits */
            formatConverter_params->eqLimitMax = 0.f;
            formatConverter_params->eqLimitMin = -0.f;
        }       
        
        /* switch off equalizer filters in case of generic setups with external DmxMtx */
        if (formatConverter_params->genericIOFmt) {
            formatConverter_params->applyEqFilters = 0;
        }

        /* ignore setup deviations in case of generic input/output setups */       
        if (formatConverter_params->genericIOFmt) {
            if (azimuthDeviation != NULL || elevationDeviation != NULL || distance != NULL) {
                fprintf(stderr,"INFO: azimuth, elevation, distance deviations are ignored for generic in/out formats!\n");
            }
        } else { /* not generic -> apply deviations */
            /* azimuthDeviation + elevationDeviation */
            if (azimuthDeviation == NULL || elevationDeviation == NULL) {
                formatConverter_params->randomFlag = 0;
            } else {
                formatConverter_params->randomFlag = 1;
                for (i=0; i < formatConverter_params->numOutputChans; i++) 
                {
                    formatConverter_params->azimuthElevationDeviation[i*2] = azimuthDeviation[i];
                    formatConverter_params->azimuthElevationDeviation[i*2+1] = elevationDeviation[i];
                }
            }
             
            /* distance */
            if (distance == NULL) {
                formatConverter_params->trimFlag = 0;
            } else {
                formatConverter_params->trimFlag = 1;
                for (i=0; i < formatConverter_params->numOutputChans; i++) 
                {
                    formatConverter_params->distance[i] = distance[i];
                }
            }
            formatConverter_params->trim_max = 1024;
        }

        /* init DMX matrix */
        for (i=0; i < formatConverter_params->numInputChans; i++)
        {
            for (j=0; j < formatConverter_params->numOutputChans; j++)                
            {
                formatConverter_params->dmxMtx[i][j] = 0.0000f;       
            }
        } 

        /* init DMX matrix freq dependent */
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
        for (i=0; i < formatConverter_params->numFreqBands; i++)
        {
            for (j=0; j < formatConverter_params->numInputChans; j++)                
            {
                for (k=0; k < formatConverter_params->numOutputChans; k++)                
                {
                    formatConverter_params->dmxMtxFreq[i][j][k] = 0.0000f;    
                }
            }
        } 
#endif
        formatConverter_params->dmxMtxIsSet = 0;

        /* init mix matrix */
        for (i=0; i < formatConverter_params->numInputChans; i++)
        {
            for (j=0; j < formatConverter_params->numInputChans; j++)                
            {
                formatConverter_params->mixMtx[i][j] = 0;       
            }
        }  
        
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
        /* init eq control matrix */
        for (i=0; i < formatConverter_params->numInputChans; i++)
        {
            for (j=0; j < 2; j++)                
            {
                formatConverter_params->eqControlMatrix[i][j] = 0;       
            }
        }  

        /* gainCompensationControlMatrix */
        for (i=0; i < formatConverter_params->numInputChans; i++)                
        {
            formatConverter_params->gainCompensationControlMatrix[i] = 0;       
        }
        formatConverter_params->gainCompensationControlMatrixSize = 0;
        formatConverter_params->gainCompensationValue = 1;
#endif
                                      
        return status;
    }
    
}

/**********************************************************************************************************************************/

int allocateFormatConverterParams( FORMAT_CONVERTER_PARAMS *formatConverter_params )
{
    int i,j;
    
    /* dmx mtx */
    formatConverter_params->dmxMtx = (float **) calloc( formatConverter_params->numInputChans, sizeof ( float* ) );    
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        formatConverter_params->dmxMtx[i] = ( float* ) calloc ( formatConverter_params->numOutputChans, sizeof ( float ) ); 
    }
    
    /* dmx mtx freq dependent */
#ifdef FORMATCONVERTER_LOWCOMPLEXITY

    formatConverter_params->dmxMtxFreq = (float ***) calloc( formatConverter_params->numFreqBands, sizeof ( float** ) );    
    for( i=0; i < formatConverter_params->numFreqBands; i++ )
    {
        formatConverter_params->dmxMtxFreq[i] = ( float** ) calloc ( formatConverter_params->numInputChans, sizeof ( float* ) ); 
        for( j=0; j < formatConverter_params->numInputChans; j++ )
        {
            formatConverter_params->dmxMtxFreq[i][j] = ( float* ) calloc ( formatConverter_params->numOutputChans, sizeof ( float ) ); 
        }
    }

#endif
    
    /* mix mtx */
    formatConverter_params->mixMtx = (int **) calloc( formatConverter_params->numInputChans, sizeof ( int* ) );    
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        formatConverter_params->mixMtx[i] = ( int* ) calloc ( formatConverter_params->numInputChans, sizeof ( int ) ); 
    }
    
    /* trim parameters */
    formatConverter_params->trimGains = (float *) calloc( formatConverter_params->numOutputChans, sizeof ( float ) );
    formatConverter_params->trimDelays = (int *) calloc( formatConverter_params->numOutputChans, sizeof ( int ) );

#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    /* eqControlMatrix */
    formatConverter_params->eqControlMatrix = (int **) calloc( formatConverter_params->numInputChans, sizeof ( int* ) );    
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        formatConverter_params->eqControlMatrix[i] = ( int* ) calloc ( 2, sizeof ( int ) ); 
    }
#endif

    /* internal structure */
    formatConverter_params->formatConverterParams_internal = (converter_pr_t *) calloc( 1, sizeof ( converter_pr_t ) );    

    /* centerFrequenciesNormalized */
    if (  formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
        ||formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT)
    {
        formatConverter_params->centerFrequenciesNormalized = ( float* ) calloc ( formatConverter_params->stftNumErbBands, sizeof ( float ) ); 
    }
    else
    {
        formatConverter_params->centerFrequenciesNormalized = ( float* ) calloc ( formatConverter_params->numFreqBands, sizeof ( float ) ); 
    }
    
    /* azimuthElevationDeviation */
    formatConverter_params->azimuthElevationDeviation = ( float* ) calloc ( 2*formatConverter_params->numOutputChans, sizeof ( float ) ); 
    
    /* distance */
    formatConverter_params->distance = ( float* ) calloc ( formatConverter_params->numOutputChans, sizeof ( float ) ); 
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
    /* gainCompensationControlMatrix */
    formatConverter_params->gainCompensationControlMatrix = ( int* ) calloc ( formatConverter_params->numInputChans, sizeof ( int ) ); 
#endif

    return 0;
}

/**********************************************************************************************************************************/

void freeFormatConverterParams( FORMAT_CONVERTER_PARAMS *formatConverter_params )
{
    int i,j;
    
    /* dmx mtx */
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        free( formatConverter_params->dmxMtx[i]); formatConverter_params->dmxMtx[i] = NULL;        
    }
    free( formatConverter_params->dmxMtx ); formatConverter_params->dmxMtx = NULL;
    
    /* dmx mtx freq dependent */
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
    for( i=0; i < formatConverter_params->numFreqBands; i++ )
    {
        for( j=0; j < formatConverter_params->numInputChans; j++ )
        {
            free( formatConverter_params->dmxMtxFreq[i][j]); formatConverter_params->dmxMtxFreq[i][j] = NULL;        
        }
        free( formatConverter_params->dmxMtxFreq[i] ); formatConverter_params->dmxMtxFreq[i] = NULL;
    }
    free( formatConverter_params->dmxMtxFreq ); formatConverter_params->dmxMtxFreq = NULL;
#endif
    
    /* mix mtx */
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        free( formatConverter_params->mixMtx[i]); formatConverter_params->mixMtx[i] = NULL;        
    }
    free( formatConverter_params->mixMtx ); formatConverter_params->mixMtx = NULL;

    /* trim parameters */
    free ( formatConverter_params->trimGains ); formatConverter_params->trimGains = NULL;
    free ( formatConverter_params->trimDelays ); formatConverter_params->trimDelays = NULL;
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    /* eqControlMatrix */
    for( i=0; i < formatConverter_params->numInputChans; i++ )
    {
        free( formatConverter_params->eqControlMatrix[i]); formatConverter_params->eqControlMatrix[i] = NULL;        
    }
    free( formatConverter_params->eqControlMatrix ); formatConverter_params->eqControlMatrix = NULL;    
#endif
    
    /* internal structure */
    free( formatConverter_params->formatConverterParams_internal ); formatConverter_params->formatConverterParams_internal = NULL;        
    
    /* centerFrequenciesNormalized */
    free( formatConverter_params->centerFrequenciesNormalized ); formatConverter_params->centerFrequenciesNormalized = NULL;        
    
    /* azimuthElevationDeviation */
    free( formatConverter_params->azimuthElevationDeviation ); formatConverter_params->azimuthElevationDeviation = NULL;        
    
    /* distance */
    free( formatConverter_params->distance ); formatConverter_params->distance = NULL;  
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
    /* gainCompensationControlMatrix */
    free( formatConverter_params->gainCompensationControlMatrix ); formatConverter_params->gainCompensationControlMatrix = NULL;  
#endif
    
}

/**********************************************************************************************************************************/

int setFormatConverterState(   const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                                FORMAT_CONVERTER_STATE           *formatConverter_state )
{
    int errorFlag = 0;

    switch (formatConverter_params->formatConverterMode)
    {
    case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
    case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:

        /* init phase align dmx */
        errorFlag = phaseAlignInit( 
            &formatConverter_state->handlePhaseAligner,
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RMO code */
            formatConverter_params->dmxMtx, 
#else
            formatConverter_params->dmxMtxFreq, 
#endif
            formatConverter_params->numInputChans, 
            formatConverter_params->numOutputChans,
            formatConverter_params->numFreqBands, 
            formatConverter_params->eqLimitMax, 
            formatConverter_params->eqLimitMin);
        break;
    
    case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
    case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:

        /* init active stft dmx */
        errorFlag = activeDmxStftInit(
            &formatConverter_state->handleActiveDmxStft,
            formatConverter_params->numInputChans, 
            formatConverter_params->numOutputChans,
            formatConverter_params->dmxMtxFreq,
            formatConverter_params->stftFrameSize,
            formatConverter_params->stftLength,
            formatConverter_params->eqLimitMax, 
            formatConverter_params->eqLimitMin,
            formatConverter_params->numFreqBands,
            formatConverter_params->stftNumErbBands,
            formatConverter_params->stftErbFreqIdx);
        break;
    }

    if(errorFlag!=0)    
        return -1;
    else 
        return 0;
}
                  
/**********************************************************************************************************************************/

int allocateFormatConverterState(
                                const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                                FORMAT_CONVERTER_STATE           *formatConverter_state )
{

    int i;
    int status = 0;

    /* inputBufferFiltered_real + inputBufferFiltered_imag */
    if (formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN
     || formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN
     || formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN) {
        
        formatConverter_state->inputBufferFiltered_real = (float **) calloc( formatConverter_params->numInputChans, sizeof ( float* ) );
        formatConverter_state->inputBufferFiltered_imag = (float **) calloc( formatConverter_params->numInputChans, sizeof ( float* ) );
        for( i=0; i < formatConverter_params->numInputChans; i++ ) 
        {
            formatConverter_state->inputBufferFiltered_real[i] = ( float* ) calloc ( formatConverter_params->numFreqBands, sizeof ( float ) ); 
            formatConverter_state->inputBufferFiltered_imag[i] = ( float* ) calloc ( formatConverter_params->numFreqBands, sizeof ( float ) ); 
        }
        
    }
    
    return status;
}

/**********************************************************************************************************************************/

int freeFormatConverterState(
                            const FORMAT_CONVERTER_PARAMS    *formatConverter_params,
                            FORMAT_CONVERTER_STATE           *formatConverter_state )
{
    int i;
    int status = 0;

    if ( formatConverter_state == NULL )
    {
        return 1;
    }
    else    
    {     
        switch(formatConverter_params->formatConverterMode)
        {
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
        case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
            /* inputBufferFiltered_real + inputBufferFiltered_imag */
            if (formatConverter_params->formatConverterMode != FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN) {

                for( i=0; i < formatConverter_params->numInputChans; i++ )
                {
                    free( formatConverter_state->inputBufferFiltered_real[i]); formatConverter_state->inputBufferFiltered_real[i] = NULL;        
                    free( formatConverter_state->inputBufferFiltered_imag[i]); formatConverter_state->inputBufferFiltered_imag[i] = NULL;        
                }
                free( formatConverter_state->inputBufferFiltered_real ); formatConverter_state->inputBufferFiltered_real = NULL;
                free( formatConverter_state->inputBufferFiltered_imag ); formatConverter_state->inputBufferFiltered_imag = NULL;

            }

            /* free phase align dmx */
            if (formatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
                phaseAlignClose(formatConverter_state->handlePhaseAligner);
            }
            return status;
            break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
            /* free stft dmx */
            activeDmxClose_STFT(formatConverter_state->handleActiveDmxStft);
            break;
        }

    }

    return status;
}

/**********************************************************************************************************************************/

int formatConverterSetInOutFormat_internal(
                         const int                  inout, 
                         const int                  numChannels, 
                         const converter_chid_t     *channel_vector)
{
    if( (inout == 0) || (inout == 1) ){
        return converter_set_inout_format(inout, numChannels, channel_vector);
    }
    else{
        fprintf(stderr, "Error: Wrong use of API function! check parameters!\n");
        return -1;
    }
}

/**********************************************************************************************************************************/

int formatConverterInit_internal( HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params )
{   
    int i, j, k, l;
    int idx[FORMAT_CONVERTER_MAX_CHANNELS];
    float *randomization;
    float *trim;
    converter_status_t converterState;
    
    /* input format */
    switch ( hFormatConverter_params->formatConverterInputFormat )
    {
        case FORMAT_CONVERTER_INPUT_FORMAT_5_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_5_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_5_2_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_5_2_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_7_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_7_1_ALT;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_10_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_10_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_22_2:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_22_2;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_14_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_14_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_9_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_9_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_9_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_9_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_11_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_11_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_4_4_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_4_4_T_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_3_0_FC;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_3_0_RC;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_4_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_4_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_5_0:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_5_0;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_6_1:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_6_1;
            break;            
        case FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS:
            hFormatConverter_params->formatConverterInputFormat_internal = FORMAT_IN_LISTOFCHANNELS;
            break;            
        default:        
            fprintf(stderr,"Error: Mapping of formatConverterInputFormat to formatConverterInputFormat_internal currently not supported.\n" );            
            return 1;    
    }
    
    /* output format */
    switch ( hFormatConverter_params->formatConverterOutputFormat )
    {
        case FORMAT_CONVERTER_OUTPUT_FORMAT_1_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_2_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_2_0;
            break; 
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_5_1;
            break;   
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_5_2_1;
            break;   
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_7_1;
            break;   
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_7_1_ALT;
            break;   
        case FORMAT_CONVERTER_OUTPUT_FORMAT_8_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_8_1;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_10_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_10_1;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_9_1;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_9_0;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_11_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_11_1;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_4_4_0;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_4_4_T_0;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_3_0_FC;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_3_0_RC;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_4_0;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_0:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_5_0;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_6_1:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_6_1;
            break;            
        case FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS:
            hFormatConverter_params->formatConverterOutputFormat_internal = FORMAT_OUT_LISTOFCHANNELS;
            break;            
        default:        
            fprintf(stderr,"Error: Mapping of formatConverterOutputFormat to formatConverterOutputFormat_internal currently not supported.\n" );            
            return 1;    
    }    
    
    /* randomization */
    if (hFormatConverter_params->randomFlag) {
        randomization = hFormatConverter_params->azimuthElevationDeviation;
    } else {
        randomization = NULL;
    }
    
    /* trim */
    if (hFormatConverter_params->trimFlag) {
        trim = hFormatConverter_params->distance;
    } else {
        trim = NULL;
    }

    /* init internal structure */
    if (   hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT 
        || hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT )
    {
        converterState = converter_init( hFormatConverter_params->formatConverterParams_internal, 
            hFormatConverter_params->formatConverterInputFormat_internal, 
            hFormatConverter_params->formatConverterOutputFormat_internal,
            randomization, 
            (float) hFormatConverter_params->samplingRate, 
            hFormatConverter_params->formatConverterFrameSize,
            hFormatConverter_params->stftNumErbBands, 
            hFormatConverter_params->centerFrequenciesNormalized, 
            trim,
            hFormatConverter_params->trim_max);
    }
    else
    {
        converterState = converter_init( hFormatConverter_params->formatConverterParams_internal, 
            hFormatConverter_params->formatConverterInputFormat_internal, 
            hFormatConverter_params->formatConverterOutputFormat_internal,
            randomization, 
            (float) hFormatConverter_params->samplingRate, 
            hFormatConverter_params->formatConverterFrameSize,
            hFormatConverter_params->numFreqBands, 
            hFormatConverter_params->centerFrequenciesNormalized, 
            trim,
            hFormatConverter_params->trim_max);
    }

    switch (converterState) {
        case -1:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_FAILED\n" );
            break;
        case -2:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_MISSING_RULE\n" );
            break;
        case -3:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_INFORMAT\n" );
            break;
        case -4:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_OUTFORMAT\n" );
            break;
        case -5:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_SFREQ\n" );
            break;
        case -6:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_BLOCKLENGTH\n" );
            break;
        case -7:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_TRIM\n" );
            break;
        case -8:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_RANDOMIZATION\n" );
            break;
        case -9:
            fprintf(stderr,"Error in formatConverterInit_internal().converter_init() --> FORMAT_CONVERTER_STATUS_BANDS\n" );
            break;
        case 0:
        default:
            break;
    }
    
    if (converterState != 0) {        
        return converterState;
    }  

    /* copy trim parameters from internal struct */
    for (i = 0; i < hFormatConverter_params->numOutputChans; i++) {
        hFormatConverter_params->trimDelays[i] = (hFormatConverter_params->formatConverterParams_internal)->trim_delay[i];
        hFormatConverter_params->trimGains[i] = (hFormatConverter_params->formatConverterParams_internal)->trim_gain[i];
    }

    /* generate dmx matrix */
    if (hFormatConverter_params->formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_1_0) {    
        i = 0;
        while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
            hFormatConverter_params->dmxMtx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][(hFormatConverter_params->formatConverterParams_internal)->in_out_dst[i]] = (hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
            i++;
        }    
    } else {
        for (i=0; i < hFormatConverter_params->numInputChans; i++)
        {
            hFormatConverter_params->dmxMtx[i][0] = 0.0000f;              
        } 
        i = 0;
        while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
            hFormatConverter_params->dmxMtx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][0] += (hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
            i++;
        }          
    }
    
    /* generate mix matrix */ 
    for (i = 0; i < hFormatConverter_params->numOutputChans; i++) {
        k = 0;
        for (j = 0; j < hFormatConverter_params->numInputChans; j++) {
            if (hFormatConverter_params->dmxMtx[j][i] > 0.f) { 
                idx[k] = j;
                k++;
            }
        }
        for (j = 0; j < k; j++) {
            for (l = 0; l < k; l++) {
                hFormatConverter_params->mixMtx[idx[j]][idx[l]] = 1;
            }          
        }        
    } 
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY
    /* generate eq control matrix */
    hFormatConverter_params->eqControlMatrixSize = 0;
    for(i = 0; i < FORMAT_CONVERTER_MAX_CHANNELS; i++){
        idx[i] = 0;
    }        
    i = 0;
    while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
        if ((hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i] > 0 && idx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]] == 0) {
            idx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]] = 1;
            hFormatConverter_params->eqControlMatrix[hFormatConverter_params->eqControlMatrixSize][0] = (hFormatConverter_params->formatConverterParams_internal)->in_out_src[i];
            hFormatConverter_params->eqControlMatrix[hFormatConverter_params->eqControlMatrixSize][1] = (hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i]-1;  
            hFormatConverter_params->eqControlMatrixSize++;            
        }
        i++;
    }   
#else
    /* init */
    for (i=0; i < hFormatConverter_params->numFreqBands; i++) {
        for (j=0; j < hFormatConverter_params->numInputChans; j++) {
            for (k=0; k < hFormatConverter_params->numOutputChans; k++) {
                hFormatConverter_params->dmxMtxFreq[i][j][k] = 0.0000f;    
            }
        }
    } 
    /* generate frequency dependent dmx matrix and apply EQ's */
    if (hFormatConverter_params->formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_1_0) {    
        i = 0;
        while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
            if (hFormatConverter_params->applyEqFilters && (hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i] > 0) {

                if (   hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
                    || hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT)
                {
                    /* apply EQ filters with processing band resolution */
                    int * erbFreqIndices = hFormatConverter_params->stftErbFreqIdx;
                    int erb;

                    j = 0; /* frequency bin index */
                    for( erb = 0; erb < hFormatConverter_params->stftNumErbBands; erb++ ) {
                        while(j<erbFreqIndices[erb])
                        {
                            hFormatConverter_params->dmxMtxFreq[j][(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][(hFormatConverter_params->formatConverterParams_internal)->in_out_dst[i]] = (hFormatConverter_params->formatConverterParams_internal)->eq[(hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i]-1][erb]*(hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
                            j++;
                        }
                    }
                } 
                else
                {
                    for (j = 0; j < hFormatConverter_params->numFreqBands; j++) {
                        hFormatConverter_params->dmxMtxFreq[j][(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][(hFormatConverter_params->formatConverterParams_internal)->in_out_dst[i]] = (hFormatConverter_params->formatConverterParams_internal)->eq[(hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i]-1][j]*(hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
                    }
                }
            } else {
                for (j = 0; j < hFormatConverter_params->numFreqBands; j++) {
                    hFormatConverter_params->dmxMtxFreq[j][(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][(hFormatConverter_params->formatConverterParams_internal)->in_out_dst[i]] = (hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
                }
            }
            i++;
        }
    } else {
        i = 0;
        while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
            if (hFormatConverter_params->applyEqFilters && (hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i] > 0) {
                for (j = 0; j < hFormatConverter_params->numFreqBands; j++) {
                    hFormatConverter_params->dmxMtxFreq[j][(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][0] += (hFormatConverter_params->formatConverterParams_internal)->eq[(hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i]-1][j]*(hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
                }
            } else {
                for (j = 0; j < hFormatConverter_params->numFreqBands; j++) {
                    hFormatConverter_params->dmxMtxFreq[j][(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]][0] += (hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i];
                }
            }
            i++;
        }          
    }    
#endif
    
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
    /* generate gain compensation control matrix */
    hFormatConverter_params->gainCompensationValue = (hFormatConverter_params->formatConverterParams_internal)->gainCompValue;
    hFormatConverter_params->gainCompensationControlMatrixSize = 0;
    for(i = 0; i < FORMAT_CONVERTER_MAX_CHANNELS; i++){
        idx[i] = 0;
    }        
    i = 0;
    while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
        if ((hFormatConverter_params->formatConverterParams_internal)->in_out_gainComp[i] == 1 && idx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]] == 0) {
            idx[(hFormatConverter_params->formatConverterParams_internal)->in_out_src[i]] = 1;
            hFormatConverter_params->gainCompensationControlMatrix[hFormatConverter_params->gainCompensationControlMatrixSize] = (hFormatConverter_params->formatConverterParams_internal)->in_out_src[i];
            hFormatConverter_params->gainCompensationControlMatrixSize++;            
        }
        i++;
    } 
#endif
    
    return 0;
}

/**********************************************************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */
