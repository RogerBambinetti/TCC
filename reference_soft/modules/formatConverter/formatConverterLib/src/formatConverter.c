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

#include "formatConverter.h"
#include "formatConverter_process.h"
#include "formatConverter_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef max
#define max(a,b) (( a > b ) ? a : b)
#endif 

/**********************************************************************************************************************************/

int formatConverterSetInOutFormat(
                         const int                  inout, 
                         const int                  numChannels, 
                         const int                  *channel_vector)
{
    if( (inout == 0) || (inout == 1) ){
        return formatConverterSetInOutFormat_internal(inout, numChannels, channel_vector);
    }
    else{
        fprintf(stderr, "Error: Wrong use of API function! check parameters!\n");
        return -1;
    }
}

/**********************************************************************************************************************************/

int formatConverterMatchChConfig2Channels(
                            const CICP2GEOMETRY_CHANNEL_GEOMETRY     *chConfig,
                            const int                  inout,
                            const int                  numChannels,
                            const int                  numLfes,
                            const int                  printResult,
                            api_chid_t                 **channel_vec, 
                            unsigned int                        *numUnknownCh,
                            int                        **unknownCh_vec,
                            float                      **azOffset_vec, 
                            float                      **elOffset_vec)
{
    int k,l;
    unsigned int uk;
    int az,el,azstart,azend,elstart,elend;
    int *lfeIdx = NULL;

    const int azel[CH_LFE2+1][6] = {
        /*   azi    ele  azstart azend elstart elend */
        {      0,     0,    -7,     7,    -9,    20 },
        {     22,     0,     8,    22,    -9,    20 },
        {    -22,     0,   -22,    -8,    -9,    20 },
        {     30,     0,    23,    37,    -9,    20 },
        {    -30,     0,   -37,   -23,    -9,    20 },
        {     45,     0,    38,    52,    -9,    20 },
        {    -45,     0,   -52,   -38,    -9,    20 },
        {     60,     0,    53,    75,    -9,    20 },
        {    -60,     0,   -75,   -53,    -9,    20 },
        {     90,     0,    76,   100,   -45,    20 },
        {    -90,     0,  -100,   -76,   -45,    20 },
        {    110,     0,   101,   124,   -45,    20 },
        {   -110,     0,  -124,  -101,   -45,    20 },
        {    135,     0,   125,   142,   -45,    20 },
        {   -135,     0,  -142,  -125,   -45,    20 },
        {    150,     0,   143,   157,   -45,    20 },
        {   -150,     0,  -157,  -143,   -45,    20 },
        {    180,     0,   158,  -158,   -45,    20 },
        {      0,    35,   -10,    10,    21,    60 },
        {     45,    35,    38,    66,    21,    60 },
        {    -45,    35,   -66,   -38,    21,    60 },
        {     30,    35,    11,    37,    21,    60 },
        {    -30,    35,   -37,   -11,    21,    60 },
        {     90,    35,    67,   100,    21,    60 },
        {    -90,    35,  -100,   -67,    21,    60 },
        {    110,    35,   101,   124,    21,    60 },
        {   -110,    35,  -124,  -101,    21,    60 },
        {    135,    35,   125,   157,    21,    60 },
        {   -135,    35,  -157,  -125,    21,    60 },
        {    180,    35,   158,  -158,    21,    60 },
        {      0,    90,  -180,   180,    61,    90 },
        {      0,   -15,   -10,    10,   -45,   -10 },
        {     45,   -15,    11,    75,   -45,   -10 },
        {    -45,   -15,   -75,   -11,   -45,   -10 },
        {     45,     0,     0,     0,   -20,    20 }, /* LFE1 */
        {    -45,     0,     0,     0,   -20,    20 }, /* LFE2 */
    };

    /* no known channel used in the beginning */
    int channelUsed[CH_LFE1] = {0}; 

    /* sanity check: check number of LFE channels */
    l=0;
    for(k=0; k<numChannels; k++)
        if((chConfig[k]).LFE == 1)
            l++;
    if(l!=numLfes){
        fprintf(stderr,"Error: wrong number of LFEs found in channel config: found %d instead of %d!\n",l,numLfes);
        return 1;
    }

    /* allocate output vector for known channel IDs */
    *channel_vec = (int*)calloc(numChannels,sizeof(int));
    /* allocate output vector for azimuth offsets */
    *azOffset_vec = (float*)calloc(numChannels,sizeof(float));
    /* allocate output vector for elevation offsets */
    *elOffset_vec = (float*)calloc(numChannels,sizeof(float));
    /* all channels are unknown at the beginning */
    *numUnknownCh = numChannels;

    /* ==== match non-LFE channels to known channels ==== */
    for(k=0; k<numChannels; k++){
        /* initialize channel k as unknown channel, 0 offset */
        (*channel_vec)[k] = CH_EMPTY;
        (*azOffset_vec)[k] = 0.f;
        (*elOffset_vec)[k] = 0.f;        
#ifdef RM6_INTERNAL_CHANNEL
        if((chConfig[k]).LFE==0 && !(chConfig[k].cicpLoudspeakerIndex >= CH_I_CNTR && chConfig[k].cicpLoudspeakerIndex <= CH_I_RIGHT) ){ /* <== neither LFEs or internal channel only */
#else
        if((chConfig[k]).LFE==0){ /* <== non-LFEs only */
#endif
            /* search for matching sector. sectors are non-overlapping, thus only 
               1 sector may match (but multiple unknowns may match the 1 known). */
            for(l=0; l<CH_LFE1; l++){
                az = (chConfig[k]).Az; /* read az inside l-loop since we may overwrite it below */ 
                el = (chConfig[k]).El;
                azstart = azel[l][2];
                azend   = azel[l][3];
                elstart = azel[l][4];
                elend   = azel[l][5];  
                if( (azstart>0) && (azend<0) ){ /* +-180 deg. wrap around */
                    azend = azend + 360;
                    if(az<0)
                        az = az + 360;
                } 
                if( (az >= azstart) && (az <= azend) && (el >= elstart) && (el <= elend)){
                    /* multiple use of known channels only allowed in input setup */
                    if((channelUsed[l]==0) || (inout==0)) {
                        (*channel_vec)[k] = l;
                        channelUsed[l] = 1;
                        (*numUnknownCh)--;
                        (*azOffset_vec)[k] = (float) az-azel[l][0];
                        (*elOffset_vec)[k] = (float) el-azel[l][1];
                    }
                    else {
                        fprintf(stderr,"ch %d: FormatConverterChannelIndex %d already in use. Will not match a second time.\n",k,l);
                    }
                }
            }
        }
#ifdef RM6_INTERNAL_CHANNEL
      else if ( chConfig[k].cicpLoudspeakerIndex >= CH_I_CNTR && chConfig[k].cicpLoudspeakerIndex <= CH_I_RIGHT && chConfig[k].LFE ==0)
      {
        (*channel_vec)[k]  = chConfig[k].cicpLoudspeakerIndex;
        (*numUnknownCh)--;
        (*azOffset_vec)[k]  = 0;
        (*elOffset_vec)[k]  = 0;
      }
#endif
    } /* end of non-LFE matching */

    /* ==== match LFE channel(s) ==== */
    /* find the indices of the numLfes LFE(s) in the input config */
    if(numLfes>0){
        lfeIdx = (int*)calloc(numLfes,sizeof(int));
        l=0;
        for(k=0; k<numChannels; k++)
            if((chConfig[k]).LFE == 1){
                lfeIdx[l] = k;
                l++;
            }
    }
    /* 1 LFE channel in config: => assign LFE1 */
    if(numLfes==1){
        (*channel_vec)[lfeIdx[0]] = CH_LFE1;
        (*numUnknownCh)--;
        free(lfeIdx);
    }
    else if(numLfes>1) {
    /* multiple LFE channels in config: */
        /* calculate distance from all LFEs in config to known LFEs */
        float maxDist, optMaxDist;
        float *allDist = (float*)calloc(2*numLfes,sizeof(float));
        int optConfigLfeToLfe1,optConfigLfeToLfe2;
        for(l=0; l<numLfes; l++){ /* loop over LFEs in config */
            az = (chConfig[lfeIdx[l]]).Az;
            allDist[2*l] = (float) (az - azel[CH_LFE1][0]);
            allDist[2*l+1] = (float) (az - azel[CH_LFE2][0]);
        }
        /* evaluate maximum azimuth distance for all possible combinations 
           of 2 LFEs out of the LFEs in the config. map those 2 creating smallest max.distance */
        optMaxDist = 360; /* for tracking minimum max.distance */
        for(l=0; l<numLfes; l++)
            for(k=0; k<numLfes; k++)
                if(l==k)
                    continue; /* don't assign the same LFE to both known LFEs */
                else {
                    maxDist = max(allDist[2*l],allDist[2*k+1]);
                    if(maxDist<optMaxDist){ /* new best mapping so far found */
                        optConfigLfeToLfe1 = l;
                        optConfigLfeToLfe2 = k;
                        optMaxDist = maxDist;
                    }
                }
        if(!( (optConfigLfeToLfe1>=0)&&(optConfigLfeToLfe1<numLfes)&&(optConfigLfeToLfe2>=0)&&(optConfigLfeToLfe2<numLfes) )){
            fprintf(stderr,"Error: something really bad happened in LFE matching!\n");
            return 1;
        } else {
            (*channel_vec)[lfeIdx[optConfigLfeToLfe1]] = CH_LFE1;
            (*channel_vec)[lfeIdx[optConfigLfeToLfe2]] = CH_LFE2;
            (*numUnknownCh)--;
            (*numUnknownCh)--;
        }
        free(allDist);
        free(lfeIdx);
    } /* end of LFE matching */

    /* signal list of the (*numUnknownCh) unknown channels to the caller, ignore unmatched LFEs */
    *unknownCh_vec = (int*)calloc((*numUnknownCh),sizeof(int));
    l = 0;
    for(k=0; k<numChannels; k++)
        if((*channel_vec)[k] == CH_EMPTY){
            if( (chConfig[k]).LFE == 1 ){
                fprintf(stderr, "INFO: ignoring unmatched LFE channel: Channel %d\n",k);
                (*numUnknownCh)--;
                }
            else {
                (*unknownCh_vec)[l] = k;
                l++;
            }
        }

    /* print matching result if requested */
    if(printResult==1){
        if (inout==0)
            fprintf(stderr, "****\nparsed input config with %d channels: \n",numChannels);
        else
            fprintf(stderr, "****\nparsed output config with %d channels: \n",numChannels);
        for(k=0;k<numChannels;k++)
            fprintf(stderr, "ch %d of %d: FormatConverterChannelIndex = %d, azOffset = %f, elOffset = %f\n",\
                            k,numChannels,(*channel_vec)[k],(*azOffset_vec)[k],(*elOffset_vec)[k]);
        fprintf(stderr, "config has %d unknown channel(s).\n",*numUnknownCh);
        if(*numUnknownCh>0){
            if(inout==0)
                fprintf(stderr, "List of unknown indices in input config (starting at position 0): ");
            else
                fprintf(stderr, "List of unknown indices in output config (starting at position 0): ");
            for(uk=0;uk<*numUnknownCh;uk++)
                fprintf(stderr, "%d ", (*unknownCh_vec)[uk]);
            fprintf(stderr,"\n");
        }
        fprintf(stderr, "****\n");
    }

    return 0;
}

/**********************************************************************************************************************************/

int  formatConverterOpen(
                             const FORMAT_CONVERTER_MODE          formatConverterMode,
                             const FORMAT_CONVERTER_INPUT_FORMAT  formatConverterInputFormat,
                             const FORMAT_CONVERTER_OUTPUT_FORMAT formatConverterOutputFormat,
                             const int                            samplingRate,
                             const int                            frameSize,
                             const int                            numFreqBands,
                             const int                            stftNumErbBands,
                             const int                            *stftErbFreqIdx,
                             const int                            stftLength,
                             int                                  *numInputChans,
                             int                                  *numOutputChans,
                             int                                  *delaySamples,
                             HANDLE_FORMAT_CONVERTER_PARAMS       *phFormatConverter_params,
                             HANDLE_FORMAT_CONVERTER_STATE        *phFormatConverter_state )
{

    /**************************************************************************************/
    /* local variables */

    int status = 0;

    /* pointers to structs for allocation */
    FORMAT_CONVERTER_PARAMS    *formatConverter_params;
    FORMAT_CONVERTER_STATE     *formatConverter_state;

    /**************************************************************************************/
    /* allocation */

    formatConverter_params   = NULL;
    formatConverter_state    = NULL;
    
    /* allocate structs ... perhaps use iisCalloc(), will need utillib */        
    if ( ( formatConverter_params = (FORMAT_CONVERTER_PARAMS *)  calloc( 1, sizeof( FORMAT_CONVERTER_PARAMS ) ) ) == NULL )
    {
        status = 1;
    }
    if ( ( formatConverter_state = (FORMAT_CONVERTER_STATE *)   calloc( 1, sizeof( FORMAT_CONVERTER_STATE ) ) ) == NULL )
    {
        status = 1;
    }

    /**************************************************************************************/
    /* check input parameters */

    /* processing mode */
    switch ( formatConverterMode )
    {
        case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
            formatConverter_params->formatConverterMode = formatConverterMode;
            formatConverter_params->numFreqBands = 0;
            break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
            formatConverter_params->stftNumErbBands = stftNumErbBands;
            formatConverter_params->stftErbFreqIdx = stftErbFreqIdx;
            formatConverter_params->stftFrameSize = numFreqBands-1;
            formatConverter_params->stftLength = (numFreqBands-1) * 2;
        case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
            formatConverter_params->formatConverterMode = formatConverterMode;
            if (numFreqBands > 0) {
                formatConverter_params->numFreqBands = numFreqBands;
            } else {
                fprintf(stderr,"Error: formatConverterNumFreqBands = %d not supported.\n", numFreqBands );
            }
            break;
        default:
            formatConverter_params->formatConverterMode = FORMAT_CONVERTER_MODE_INVALID;
            fprintf(stderr,"Error: formatConverterMode currently %d not supported.\n", formatConverterMode );
            status = 1;
    }

    /* delay */
    formatConverter_params->formatConverterDelay = formatConverterGetDelaySamples(formatConverter_params->formatConverterMode);
    *delaySamples = formatConverter_params->formatConverterDelay;

    /* frameSize */
    if (frameSize > 0) {
        formatConverter_params->formatConverterFrameSize = frameSize;
    } else {
        fprintf(stderr,"Error: formatConverterFrameSize %d not supported.\n", formatConverter_params->formatConverterFrameSize );
        status = 1;
    } 

    /* generic formats (for use with external DMXmtx) */
    if(formatConverterInputFormat == FORMAT_CONVERTER_INPUT_FORMAT_GENERIC)
        if(formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) {
            fprintf(stderr,"Error: generic input format only allowed if output format is also generic.\n");
            status = 1;
        }
    if(formatConverterOutputFormat == FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC)
        if(formatConverterInputFormat != FORMAT_CONVERTER_INPUT_FORMAT_GENERIC) {
            fprintf(stderr,"Error: generic output format only allowed if input format is also generic.\n");
            status = 1;
        }

    /* input format */
    switch ( formatConverterInputFormat )
    {
        case FORMAT_CONVERTER_INPUT_FORMAT_5_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_5_2_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT:
        case FORMAT_CONVERTER_INPUT_FORMAT_10_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_22_2:
        case FORMAT_CONVERTER_INPUT_FORMAT_14_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_9_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_9_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_11_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC:
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC:
        case FORMAT_CONVERTER_INPUT_FORMAT_4_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_5_0:
        case FORMAT_CONVERTER_INPUT_FORMAT_6_1:
        case FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS:
        case FORMAT_CONVERTER_INPUT_FORMAT_GENERIC:
            formatConverter_params->formatConverterInputFormat = formatConverterInputFormat;
            break;
        default:
            formatConverter_params->formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_INVALID;
            fprintf(stderr,"Error: formatConverterInputFormat %d not supported.\n", formatConverterInputFormat );
            status = 1;
    }

    if( (formatConverter_params->formatConverterInputFormat != FORMAT_CONVERTER_INPUT_FORMAT_GENERIC) && 
        (formatConverter_params->formatConverterInputFormat != FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS) ) {
        /* numInputChans */
        formatConverter_params->numInputChans = formatConverterGetNumInputChans(formatConverter_params->formatConverterInputFormat);
        if (formatConverter_params->numInputChans != 0) {
            *numInputChans = formatConverter_params->numInputChans;
        } else {
            fprintf(stderr,"Error: numInputChans %d not supported.\n", formatConverter_params->numInputChans );
            status = 1;
        }
    } else {
        if (numInputChans != NULL) {
            formatConverter_params->numInputChans = *numInputChans;
            if(formatConverter_params->numInputChans != 0){

                if (formatConverter_params->formatConverterInputFormat == FORMAT_CONVERTER_INPUT_FORMAT_GENERIC)
                    fprintf(stderr,"generic input format: %d channels\n",formatConverter_params->numInputChans);
                else
                    fprintf(stderr,"input format: list of %d channels\n",formatConverter_params->numInputChans);

            } else {
                fprintf(stderr,"Error: 0 input channels not allowed!\n");
                status = 1;
            }
        } else {
            fprintf(stderr,"Error: numInputChans not provided (NULL ptr) for generic input format or list of input channels.\n");
            status = 1;
        }
    }

    /* output format */
    switch ( formatConverterOutputFormat )
    {
        case FORMAT_CONVERTER_OUTPUT_FORMAT_1_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_2_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_8_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_10_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_11_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_0:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_6_1:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS:
        case FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC:
            formatConverter_params->formatConverterOutputFormat = formatConverterOutputFormat;
            break;
        default:
            formatConverter_params->formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID;
            fprintf(stderr,"Error: formatConverterOutputFormat %d not supported.\n", formatConverterOutputFormat );
            status = 1;
    }

    /* numOutputChans */
    if( (formatConverter_params->formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) &&
        (formatConverter_params->formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS) ) {
        formatConverter_params->numOutputChans = formatConverterGetNumOutputChans(formatConverter_params->formatConverterOutputFormat);
        if (formatConverter_params->numOutputChans != 0) {
            *numOutputChans = formatConverter_params->numOutputChans;
        } else {
            fprintf(stderr,"Error: numOutputChans %d not supported.\n", formatConverter_params->numOutputChans );  
            status = 1;
        }
    } else {
        if (numOutputChans != NULL) {
            formatConverter_params->numOutputChans = *numOutputChans;
            if(formatConverter_params->numOutputChans != 0){   

                if(formatConverter_params->formatConverterOutputFormat == FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC){
                    formatConverter_params->genericIOFmt = 1;
                    fprintf(stderr,"generic output format: %d channels\n",formatConverter_params->numOutputChans);
                } else
                    fprintf(stderr,"output format: list of %d channels\n",formatConverter_params->numOutputChans);
           
            } else {
                fprintf(stderr,"Error: 0 output channels not allowed!\n");
                status = 1;
            }
        } else {
            fprintf(stderr,"Error: numOutputChans not provided (NULL ptr) for generic output format or list of output channels.\n");
            status = 1;
        }
    }

    /* sampling rate */
    if((samplingRate >= 8000) && (samplingRate <= 384000) )
        formatConverter_params->samplingRate = samplingRate;
    else {
        fprintf(stderr,"Error: samplingRate of %d not supported.\n", samplingRate );
        formatConverter_params->samplingRate = 0;
        status = 1;
    }

    /**************************************************************************************/

    /* on error clean up so far allocated structs and return */
    if ( status == 1 )
    {        
        free ( formatConverter_params );
        free ( formatConverter_state );
        return 1;
    }

    /**************************************************************************************/

    /* init of parameter members for allocation */

    if ( setFormatConverterParamsPreAlloc( formatConverter_params ) != 0 )
    {
        status = 1;
        fprintf(stderr,"Error in setFormatConverterParamsPreAlloc().\n" );
    }

    /* buffer allocation */
    if ( allocateFormatConverterParams( formatConverter_params ) != 0 )
    {
        status = 1;
        fprintf(stderr,"Error in allocateFormatConverterParams().\n" );
    }
    
    if ( allocateFormatConverterState( formatConverter_params, formatConverter_state ) != 0 )
    {
        status = 1;
        fprintf(stderr,"Error in allocateFormatConverterState().\n" );
    }

    /**************************************************************************************/

    if ( status == 0 )
    {
        /* if no error return allocated structs */
        *phFormatConverter_params = formatConverter_params;
        *phFormatConverter_state  = formatConverter_state;
    }
    else
    {
        /* otherwise clean up */
        formatConverterClose(
                            &formatConverter_params,
                            &formatConverter_state );
    }

    return status;
}

/**********************************************************************************************************************************/

int  formatConverterInit(
                             const float                      *centerFrequenciesNormalized,
                             const float                      *azimuthDeviation,
                             const float                      *elevationDeviation,
                             const float                      *distance,
                             HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                             HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    int errorFlag;    

    if ( hFormatConverter_params == NULL || hFormatConverter_state == NULL )
    {
        return 1;
    }    
    
    /* set format converter parameters */
    if ( setFormatConverterParams( centerFrequenciesNormalized, azimuthDeviation, elevationDeviation, distance, hFormatConverter_params ) != 0 )
    {
        fprintf(stderr,"Error in setFormatConverterParams().\n" );
        return 1;    
    }    
  
    if(!hFormatConverter_params->genericIOFmt) {
        int errorFlag;
        /* format converter init internal */   
        errorFlag = formatConverterInit_internal( hFormatConverter_params );
        if ( errorFlag != 0 )
        {
            fprintf(stderr,"Error in formatConverterInit_internal().\n" );
            return errorFlag;    
        } else {
            hFormatConverter_params->dmxMtxIsSet = 1;
        }
    } else {
        fprintf(stderr, "Generic input,output formats used. Skipping generation of DmxMtx.\n");
        hFormatConverter_params->dmxMtxIsSet = 0;
    }    


    /* set format converter states */
    errorFlag = setFormatConverterState( hFormatConverter_params, hFormatConverter_state ); 
    if(errorFlag!=0){
        fprintf(stderr, "Error: Setting format converter states failed.\n");
        return -1;
    }
    
    return 0;
}

/**********************************************************************************************************************************/

int  formatConverterProcess_timeDomain( 
                                const float                      **inputBuffer,
                                float                            **outputBuffer,
                                HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    
    if ( hFormatConverter_params == NULL || hFormatConverter_state == NULL )
        return 1;

    if( !hFormatConverter_params->dmxMtxIsSet) {
        fprintf(stderr,"Error in formatConverterProcess_timeDomain(): DMX matrix has not been set yet.\n");
        return 1;
    }
    
    switch (hFormatConverter_params->formatConverterMode) {
        case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
            formatConverterProcess_passive_timeDomain(inputBuffer, outputBuffer, hFormatConverter_params, hFormatConverter_state);
            break;
        default:
            fprintf(stderr,"Error in formatConverterProcess_timeDomain(): formatConverterMode invalid for time domain processing.\n" );
            return 1; 
            break;
    }
    
    return 0;
}

/**********************************************************************************************************************************/

int  formatConverterProcess_freqDomain( 
                                           const float                      **inputBuffer_real,
                                           const float                      **inputBuffer_imag,
                                           float                            **outputBuffer_real,
                                           float                            **outputBuffer_imag,
                                           HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                           HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    
    if ( hFormatConverter_params == NULL || hFormatConverter_state == NULL )
        return 1;

    if( !hFormatConverter_params->dmxMtxIsSet) {
        fprintf(stderr,"Error in formatConverterProcess_freqDomain(): DMX matrix has not been set yet.\n");
        return 1;
    }
    
    switch (hFormatConverter_params->formatConverterMode) {
        case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
            if (hFormatConverter_params->applyEqFilters) {
                formatConverter_applyEqualizationFilters(   inputBuffer_real, 
                                                            inputBuffer_imag, 
                                                            hFormatConverter_state->inputBufferFiltered_real, 
                                                            hFormatConverter_state->inputBufferFiltered_imag, 
                                                            hFormatConverter_params, hFormatConverter_state);
                formatConverterProcess_passive_freqDomain((const float**)hFormatConverter_state->inputBufferFiltered_real, (const float**)hFormatConverter_state->inputBufferFiltered_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
            } else {
                formatConverterProcess_passive_freqDomain(inputBuffer_real, inputBuffer_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
            }
#else
            formatConverterProcess_passive_freqDomain(inputBuffer_real, inputBuffer_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
#endif
            break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
            if (hFormatConverter_params->applyEqFilters) {
                formatConverter_applyEqualizationFilters(   inputBuffer_real, 
                                                            inputBuffer_imag, 
                                                            hFormatConverter_state->inputBufferFiltered_real, 
                                                            hFormatConverter_state->inputBufferFiltered_imag, 
                                                            hFormatConverter_params, hFormatConverter_state);
                
                formatConverterProcess_active_freqDomain_phaseAlign((const float**)hFormatConverter_state->inputBufferFiltered_real, (const float**)hFormatConverter_state->inputBufferFiltered_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
            } else {
                formatConverterProcess_active_freqDomain_phaseAlign(inputBuffer_real, inputBuffer_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
            }
#else
            formatConverterProcess_active_freqDomain_phaseAlign(inputBuffer_real, inputBuffer_imag, outputBuffer_real, outputBuffer_imag, hFormatConverter_params, hFormatConverter_state);
#endif
            break;
        default:
            fprintf(stderr,"Error in formatConverterProcess_freqDomain(): formatConverterMode invalid for frequency domain processing.\n" );
            return 1; 
            break;
    }    
    
    return 0;
    
}
    
/**********************************************************************************************************************************/
int  formatConverterProcess_freqDomain_STFT( 
                                const float                      **inputBufferStft, 
                                float                            **outputBufferStft,
                                HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params, 
                                HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state )
{
    formatConverter_process_STFT(inputBufferStft, outputBufferStft, hFormatConverter_params, hFormatConverter_state);
    
    return 0;
}

/**********************************************************************************************************************************/
int  formatConverterClose(
                              HANDLE_FORMAT_CONVERTER_PARAMS   *phFormatConverter_params,
                              HANDLE_FORMAT_CONVERTER_STATE    *phFormatConverter_state )
{
    
    int status = 0;

    if ( (*phFormatConverter_params != NULL ) && (*phFormatConverter_state != NULL ) )
    {       

        /* free buffers */
        freeFormatConverterState(
                                *phFormatConverter_params,
                                *phFormatConverter_state );

        freeFormatConverterParams( *phFormatConverter_params );

        /* free pointers to format converter structs */
        free( *phFormatConverter_params );        
        *phFormatConverter_params = NULL;     

        free( *phFormatConverter_state );        
        *phFormatConverter_state  = NULL;       
    
    }

    return status;
}

/**********************************************************************************************************************************/

int  formatConverterGetDmxMtx(
                                  float **dmxMtx,
                                  HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params)
{
    int i,j;
    
    if ( dmxMtx == NULL)
        return 1;    
    
    /* copy dmx matrix */
    for (i=0; i < hFormatConverter_params->numInputChans;i++)
    {
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)
        {           
            dmxMtx[i][j] = hFormatConverter_params->dmxMtx[i][j];            
        }
    }
    
    return 0;
}

/**********************************************************************************************************************************/

int directMapLFEs(
                float **dmxMtx,
                const int numInputChans,
                const CICP2GEOMETRY_CHANNEL_GEOMETRY *inputChConfig,
                const int numOutputChans,
                const CICP2GEOMETRY_CHANNEL_GEOMETRY *outputChConfig)
{
    int n,l;
    int numTargetLfes = 0;    
    int targetLfeChannelIdx[FORMAT_CONVERTER_MAX_CHANNELS];
    float targetLfeAz[FORMAT_CONVERTER_MAX_CHANNELS];
    if ( dmxMtx == NULL )
        return 1;    

    /* search for LFEs in target setup, store azimuths and output channel indices of LFEs */
    for(n = 0; n<numOutputChans; n++){
        if(outputChConfig[n].LFE == 1){
            targetLfeChannelIdx[numTargetLfes] = n;
            targetLfeAz[numTargetLfes] = (float)outputChConfig[n].Az;
            /* fprintf(stderr,"LFE in target setup (LFE idx %d): out channel %d, azimuth = %f\n",numTargetLfes,targetLfeChannelIdx[numTargetLfes],targetLfeAz[numTargetLfes]); */
            numTargetLfes++;
        }
    }

    if(numTargetLfes>0){
        /* look for LFEs in input and remap them */        
        for(n = 0; n<numInputChans; n++){
            if(inputChConfig[n].LFE == 1){ /* we only modify dmx matrix entries of input LFEs */
                /* find target LFE with minimum distance*/
                float az_in = (float)inputChConfig[n].Az;
                float az_dist = 1000.f;
                int min_az_dist_lfe_idx = 0;
                for(l = 0; l<numTargetLfes; l++){
                    float az_out = targetLfeAz[l];
                    float az_dist_tmp = (float)acos(cos((az_out-az_in)*M_PI/180.f)); /* acos(cos()) to avoid special handling of angle signs,wrap-around,etc...*/
                    if(az_dist_tmp < az_dist){
                        az_dist = az_dist_tmp;   /* new current min distance */
                        min_az_dist_lfe_idx = l; /* index of LFE with current min distance */
                    }
                }
                fprintf(stderr, "INFO: Target setup contains LFE(s). Mapping input LFE directly to target LFE instead of gVBAP.\n");
                /* fprintf(stderr,"LFE with minimum azimuth distance: %d, out channel %d, az_dist = %f\n",min_az_dist_lfe_idx,targetLfeChannelIdx[min_az_dist_lfe_idx],az_dist*180.f/M_PI); */

                /* change dmx matrix entry to directly map to min_az_dist LFE */
                for(l = 0; l<numOutputChans; l++)
                    dmxMtx[n][l] = 0.f;
                dmxMtx[n][targetLfeChannelIdx[min_az_dist_lfe_idx]] = 1.f;
            }
        }
    }
    return 0;
}

/**********************************************************************************************************************************/

int formatConverterPostprocessDmxMtx(
                                float **dmxMtx,
                                const int numInputChans,
                                const int numOutputChans)
{
    int i,j;
    float origEne,modEne,normFactor,maxGain;
    float thr = 0.3f; /* threshold for setting DMX gains to zero: 0.3=-10.5dB (approx 3/4 panpot) */

    if ( dmxMtx == NULL)
        return 1;    

    /* post-process gains for each input channel individually */
    for(i=0;i<numInputChans;i++){
        /* post-process only if there is at least one gain 
           larger than the threshold for the current input channel */
        maxGain = 0.f;
        for(j=0;j<numOutputChans;j++)
            if(dmxMtx[i][j] > maxGain)
                maxGain = dmxMtx[i][j];
        if(maxGain<=thr)
            continue;

        /* calculate energy before post-processing */
        origEne = 0.f;
        for(j=0;j<numOutputChans;j++)
            origEne += dmxMtx[i][j] * dmxMtx[i][j];          

        /* apply threshold */
        for(j=0;j<numOutputChans;j++){
            if(dmxMtx[i][j] <= thr)
                dmxMtx[i][j] = 0.f;
        }

        /* calculate power after modification */
        modEne = 0.f;
        for(j=0;j<numOutputChans;j++)
            modEne += dmxMtx[i][j] * dmxMtx[i][j];          

        /* normalize gains */
        normFactor = (float) sqrt(origEne/modEne);
        for(j=0;j<numOutputChans;j++){
            dmxMtx[i][j] = normFactor * dmxMtx[i][j];
        }
    }  
    return 0;
}

/**********************************************************************************************************************************/

int  formatConverterAddDmxMtx(
                              const float **dmxMtx,
                              HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                              HANDLE_FORMAT_CONVERTER_STATE  hFormatConverter_state)
{
    int i,j,k,l;
    int idx[FORMAT_CONVERTER_MAX_CHANNELS];
    phaseAligner* h     = hFormatConverter_state->handlePhaseAligner;
    activeDownmixer* h2 = hFormatConverter_state->handleActiveDmxStft;

    if ( dmxMtx == NULL )
        return 1;    

    if ( hFormatConverter_params->dmxMtxIsSet != 1 ) {
        fprintf(stderr,"Error: DMX matrix has not been set yet!\n");
        fprintf(stderr,"       Use formatConverterSetDmxMtx() instead of formatConverterAddDmxMtx()!\n");
        return 1;
    }
    
    /* add external dmx matrix: element-wise addition
       NOTE: the number of input/output channels must NOT change! 
    */    
    for (i=0; i < hFormatConverter_params->numInputChans; i++)
    {           
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            /* add update terms to current downmix matrix */
            hFormatConverter_params->dmxMtx[i][j] += dmxMtx[i][j];            
        }
    }     

#ifndef FORMATCONVERTER_LOWCOMPLEXITY    
    /* pass updated dmx matrix to phasealign DMX struct if running in phasealign DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
        if(h != NULL) {           
            fprintf(stderr,"mode == phasealign dmx => passing updated DMX matrix to phasealign dmx\n");
            phaseAlignSetDmxMtx(h, hFormatConverter_params->dmxMtx);
        }
    }
#else
    /* also update frequency dependent DMX matrix by adding external matrix values */
    for (i=0; i < hFormatConverter_params->numInputChans; i++)
    {           
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            for (k=0; k < hFormatConverter_params->numFreqBands; k++)                
            {
                /* Note: we are adding the frequency independent update terms to the freq-dep. matrix */
                hFormatConverter_params->dmxMtxFreq[k][i][j] += dmxMtx[i][j];            
            }
        }
    }  
    /* pass updated dmx matrix to phasealign DMX struct if running in phasealign DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
        if(h != NULL) {           
            fprintf(stderr,"mode == phasealign dmx => passing updated DMX matrix to phasealign dmx\n");
            phaseAlignSetDmxMtx(h, hFormatConverter_params->dmxMtxFreq);
        }
    }
    /* pass updated dmx matrix to STFT DMX struct if running in STFT DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
        hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT ) {
        if(h2 != NULL) {           
            fprintf(stderr,"mode == active dmx  stft => passing updated DMX matrix to stft dmx\n");
            activeDmxSetDmxMtx_STFT(h2, (const float ***)hFormatConverter_params->dmxMtxFreq);
        }
    }
#endif

    /* update mix matrix */
    /* initialize */
    for (i = 0; i < hFormatConverter_params->numInputChans; i++)
        for (j = 0; j < hFormatConverter_params->numInputChans; j++)
            hFormatConverter_params->mixMtx[i][j] = 0;
    /* fill it */
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

    return 0;
}
        
/**********************************************************************************************************************************/

int  formatConverterSetDmxMtx(
                              const float **dmxMtx,
                              HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                              HANDLE_FORMAT_CONVERTER_STATE  hFormatConverter_state)
{
    int i,j,k,l;
    int idx[FORMAT_CONVERTER_MAX_CHANNELS];
    phaseAligner* h = hFormatConverter_state->handlePhaseAligner;

    if ( dmxMtx == NULL )
        return 1;    
    
    /* apply external dmx matrix
       NOTE: the number of input/output channels must NOT change! 
    */    
    for (i=0; i < hFormatConverter_params->numInputChans; i++)
    {           
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            hFormatConverter_params->dmxMtx[i][j] = dmxMtx[i][j];            
        }
    }     

#ifndef FORMATCONVERTER_LOWCOMPLEXITY    
    /* pass dmx matrix to phasealign DMX struct if running in phasealign DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
        if(h != NULL) {           
            fprintf(stderr,"mode == phasealign dmx => passing external DMX matrix to phasealign dmx\n");
            phaseAlignSetDmxMtx(h, hFormatConverter_params->dmxMtx);
        }
    }
#else
    /* copy to frequency dependent DMX matrix */
    for (i=0; i < hFormatConverter_params->numInputChans; i++)
    {           
        for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
        {
            for (k=0; k < hFormatConverter_params->numFreqBands; k++)                
            {
                hFormatConverter_params->dmxMtxFreq[k][i][j] = hFormatConverter_params->dmxMtx[i][j];            
            }
        }
    }  
    /* pass dmx matrix to phasealign DMX struct if running in phasealign DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
        if(h != NULL) {           
            fprintf(stderr,"mode == phasealign dmx => passing external DMX matrix to phasealign dmx\n");
            phaseAlignSetDmxMtx(h, hFormatConverter_params->dmxMtxFreq);
        }
    }
#endif

    /* DMX matrix is valid now */
    hFormatConverter_params->dmxMtxIsSet = 1;
        
    /* internal EQs are switched off for external DMX matrix */
    hFormatConverter_params->applyEqFilters = 0;
    fprintf(stderr,"Format Converter Lib Info: internal EQ filters disabled for external DMX matrix.\n");  
    
    /* update mix matrix */
    /* initialize */
    for (i = 0; i < hFormatConverter_params->numInputChans; i++)
        for (j = 0; j < hFormatConverter_params->numInputChans; j++)
            hFormatConverter_params->mixMtx[i][j] = 0;
    /* fill it */
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

    return 0;
}

/**********************************************************************************************************************************/
int  formatConverterSetDmxMtx_STFT(const HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params, HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state)
{
    activeDownmixer *h;
    h =    (activeDownmixer*) hFormatConverter_state->handleActiveDmxStft;

    if (hFormatConverter_params->dmxMtxFreq == NULL)
        return -1;
    if (h!=NULL)
    {
        fprintf(stderr,"mode == active dmx stft => passing external DMX matrix to active dmx stft\n");
        activeDmxSetDmxMtx_STFT(h, (const float***) hFormatConverter_params->dmxMtxFreq);
    }

    return 0;
}
/**********************************************************************************************************************************/
int formatConverterSetEQs(
                         int numEQs,
                         eqParamsStruct *eqParams,
                         int *eqMap,
                         float *bands_nrm,      /* normalized freqs of processing bands */
                         float sfreq_Hz,
                         HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                         HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state)
{
    int i,j,k,m,n,numFreqBands,fftBand,fftBandStop;
    float eqGainsTmp[MAXBANDS][N_EQ];
#ifdef FORMATCONVERTER_LOWCOMPLEXITY  
    phaseAligner* h     = hFormatConverter_state->handlePhaseAligner;
    activeDownmixer* h2 = hFormatConverter_state->handleActiveDmxStft;
#endif

	if ( (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT) ||
         (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) ) 
	{
		numFreqBands = hFormatConverter_params->stftNumErbBands;
	}
	else
	{
		numFreqBands = hFormatConverter_params->numFreqBands;
	}

    /* sanity checks */
    if(!hFormatConverter_params->genericIOFmt) {
        fprintf(stderr,"ERROR: setting of external EQs only allowed for generic setups with external DMX matrices.\n");
        return 1;
    }
    if(numEQs>N_EQ){
        fprintf(stderr,"ERROR: currently only up to %d EQs allowed...\n",N_EQ);
        return 1;
    }
    for (i=0; i < hFormatConverter_params->numInputChans; i++){
        if(eqMap[i]>numEQs) {
            fprintf(stderr,"ERROR: eqMap signals EQ index larger than numEQs!\n");
            return 1;
        }
    }
#ifdef FORMATCONVERTER_LOWCOMPLEXITY  
    if(!hFormatConverter_params->dmxMtxIsSet) {
        fprintf(stderr,"ERROR: no DMX matrix set yet. Cannot apply EQs to DMX matrix.\n");
        return 1;  
    }
#endif

    /* calculate numEQs EQ responses from the numEQs eqParamsStructs */
    for(n=0; n<numEQs; n++){
        for (k=0; k < numFreqBands; k++) {

            float f = (float)fabs(bands_nrm[k])*sfreq_Hz/2.0f;

            /* init with first peak filter and global gain G[dB] */
            pkFilterParamsStruct pkParams = eqParams[n].pkFilterParams[0];
            eqGainsTmp[k][n] = peak_filter(pkParams.f, pkParams.q, pkParams.g, eqParams[n].G, f);
            /* apply remaining filters of peak filter cascade with 0dB global gain */
            for(m=1; m<eqParams[n].nPkFilter; m++){
                pkParams = eqParams[n].pkFilterParams[m];
                eqGainsTmp[k][n] *= peak_filter(pkParams.f, pkParams.q, pkParams.g, 0.f, f);
            }            
            /* fprintf(stderr,"EQ:%d, band:%d, gain:%f\n",n,k,eqGainsTmp[k][n]); */
        }
    }

#ifdef FORMATCONVERTER_LOWCOMPLEXITY  
    /* apply EQs to freq. dep. dmx matrix */
    for (i=0; i < hFormatConverter_params->numInputChans; i++)
    {
        int eqIdx = eqMap[i];           
        if (eqIdx > 0) { /* apply an EQ to this input channel */
            for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
                if ( (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT) ||
                     (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) ) 
                {
                    fftBand = 0;
                    for(k=0;k<h2->numErbBands;k++) {
                        fftBandStop = h2->erbFreqIdx[k];
                        while(fftBand < fftBandStop){
                           hFormatConverter_params->dmxMtxFreq[fftBand][i][j] = eqGainsTmp[k][eqIdx-1] * hFormatConverter_params->dmxMtx[i][j];             
                           fftBand++;
                        }
                 	}
                }
                else {
                    for (k=0; k < numFreqBands; k++)
                        hFormatConverter_params->dmxMtxFreq[k][i][j] = eqGainsTmp[k][eqIdx-1] * hFormatConverter_params->dmxMtx[i][j];            
                }
        } else {         /* no EQ applied to this input channel - simply copy dmxMtxFreq */
            for (j=0; j < hFormatConverter_params->numOutputChans; j++)                
                for (k=0; k < hFormatConverter_params->numFreqBands; k++)
                    hFormatConverter_params->dmxMtxFreq[k][i][j] = hFormatConverter_params->dmxMtx[i][j];            
        }
    }     

    /* pass EQed dmx matrix to phasealign DMX struct if running in phasealign DMX mode */
    if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN) {
        if(h != NULL) {           
            phaseAlignSetDmxMtx(h, hFormatConverter_params->dmxMtxFreq);
            fprintf(stderr,"Format Converter Lib Info: External EQs applied to DMX matrix\n");  
        }
    }
    else /* pass updated dmx matrix to STFT DMX struct if running in STFT DMX mode */
        if (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT ||
            hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) {
                if(h2 != NULL) {           
                    activeDmxSetDmxMtx_STFT(h2, (const float ***)hFormatConverter_params->dmxMtxFreq);
                    fprintf(stderr,"Format Converter Lib Info: External EQs applied to DMX matrix\n");  
                }
        }
        else {
            fprintf(stderr,"ERROR: setting of EQs is only allowed in FREQ_DOMAIN mode\n");
            return 1;
        }
#else   
    /* copy calculated EQ responses to internal EQ gains */
    for (j=0; j < numEQs; j++)                
        for (k=0; k < numFreqBands; k++)                
            hFormatConverter_params->formatConverterParams_internal->eq[j][k] = eqGainsTmp[k][j];

    /* fill eqControlMatrix */
    hFormatConverter_params->eqControlMatrixSize = 0;
    for (i=0; i < hFormatConverter_params->numInputChans; i++){
        int eqIdx = eqMap[i];           
        if (eqIdx > 0) { /* apply an EQ to the i-th input channel */  
            hFormatConverter_params->eqControlMatrix[hFormatConverter_params->eqControlMatrixSize][0] = i;
            hFormatConverter_params->eqControlMatrix[hFormatConverter_params->eqControlMatrixSize][1] = eqIdx-1;  
            hFormatConverter_params->eqControlMatrixSize++;     
        }       
    }

    /* filter gains and control matrix are ready: switch on externally provided EQs */
    hFormatConverter_params->applyEqFilters = 1;
    fprintf(stderr,"Format Converter Lib Info: EQ filters enabled using externally provided EQ settings.\n");  
#endif

    return 0;
}
 
/**********************************************************************************************************************************/

int  formatConverterGetMixMtx(
                                  int **mixMtx,
                                  HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params)
{
    int i,j;
    
    if ( mixMtx == NULL)
        return 1;   
    
    /* copy dmx matrix */
    for (i=0; i < hFormatConverter_params->numInputChans;i++)
    {
        for (j=0; j < hFormatConverter_params->numInputChans; j++)
        {           
            mixMtx[i][j] = hFormatConverter_params->mixMtx[i][j];            
        }
    }
    
    return 0;
}

/**********************************************************************************************************************************/

int  formatConverterGetTrimParams(
                                float *trimGains,
                                int *trimDelays,
                                HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params)
{
    int i;

    if (( trimGains == NULL ) || (trimDelays == NULL))
        return 1;   

    /* copy trim parameters */
    for (i=0; i < hFormatConverter_params->numOutputChans;i++){
        trimGains[i] = hFormatConverter_params->trimGains[i];
        trimDelays[i] = hFormatConverter_params->trimDelays[i];
    }
    
    return 0;
}
        
/**********************************************************************************************************************************/

int formatConverterGetDelaySamples( FORMAT_CONVERTER_MODE mode ){
   int delaySamples = -1;

   switch (mode) {
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
            delaySamples = 4096;
            break;
        case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
        case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
            delaySamples = 0;
            break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
            delaySamples = 256;
            break;
        default:
            break;
    }

    return delaySamples;

}

/**********************************************************************************************************************************/

int formatConverterDisplayParams( int mode, const HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params )
{
    int i, j, numFreqBands;
    
    if ( hFormatConverter_params == NULL )
    {
        fprintf(stderr,"Error in formatConverterDisplayParams(): NULL pointer.\n" );    
        return 1;
    }

    if ( (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT) ||
         (hFormatConverter_params->formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) ) 
    {
        numFreqBands = hFormatConverter_params->stftNumErbBands;
    }
    else
    {
        numFreqBands = hFormatConverter_params->numFreqBands;
    }
    
    fprintf(stderr,"\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
    fprintf(stderr,"\nFormat Converter parameters:\n");

    fprintf(stderr,"samplingRate:                 %d\n", hFormatConverter_params->samplingRate );
    
    
    switch ( hFormatConverter_params->formatConverterMode )
    {
        case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
            fprintf(stderr,"formatConverterMode:          PASSIVE TIME DOMAIN\n" );
            break;
        case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
            fprintf(stderr,"formatConverterMode:          PASSIVE FREQ DOMAIN\n" );
            break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
            fprintf(stderr,"formatConverterMode:          ACTIVE FREQ DOMAIN PHASE ALIGN LOW COMPLEXITY\n" );
#else
            fprintf(stderr,"formatConverterMode:          ACTIVE FREQ DOMAIN PHASE ALIGN\n" );
#endif
            break;
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
          fprintf(stderr, "formatConverterMode:          CUSTOM FREQ DOMAIN PHASE ALING\n" );
          break;
        case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
          fprintf(stderr, "formatConverterMode:          ACTIVE FREQ DOMAIN STFT\n" );
          break;
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
          fprintf(stderr, "formatConverterMode:          CUSTOM FREQ DOMAIN STFT\n" );
          break;

        case FORMAT_CONVERTER_MODE_INVALID:                         
        default:
            fprintf(stderr,"Fatal error. Unsupported formatConverterMode. This should have been catched before.\n" );
            break;            
    }
    
    switch ( hFormatConverter_params->formatConverterInputFormat )
    {
        case FORMAT_CONVERTER_INPUT_FORMAT_5_1:
            fprintf(stderr,"formatConverterInputFormat:   5.1\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_5_2_1:
            fprintf(stderr,"formatConverterInputFormat:   7.1 = 5.1 + 2 (upper L/R spks)\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1: 
            fprintf(stderr,"formatConverterInputFormat:   7.1 (surround L/R spks)\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT:
            fprintf(stderr,"formatConverterInputFormat:   7.1 (center L/R spks)\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_10_1:
            fprintf(stderr,"formatConverterInputFormat:   10.1 = 5.1 + 4 + T\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_22_2:
            fprintf(stderr,"formatConverterInputFormat:   22.2\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_14_0:
            fprintf(stderr,"formatConverterInputFormat:   14.0\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_9_1:
            fprintf(stderr,"formatConverterInputFormat:   9.1 = 5.1 + 4\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_9_0:
            fprintf(stderr,"formatConverterInputFormat:   9.0 = 5.0 + 4\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_11_1:
            fprintf(stderr,"formatConverterInputFormat:   11.1 = 5.1 + 5 + T\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_0:
            fprintf(stderr,"formatConverterInputFormat:   4.0 + 4\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0:
            fprintf(stderr,"formatConverterInputFormat:   4.0 + 4 + T\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC:
            fprintf(stderr,"formatConverterInputFormat:   3.0 (front center)\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC:
            fprintf(stderr,"formatConverterInputFormat:   3.0 (rear center)\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_0:
            fprintf(stderr,"formatConverterInputFormat:   4.0\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_5_0:
            fprintf(stderr,"formatConverterInputFormat:   5.0\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_6_1:
            fprintf(stderr,"formatConverterInputFormat:   6.1\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_GENERIC:
            fprintf(stderr,"formatConverterInputFormat:   generic\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS:
            fprintf(stderr,"formatConverterInputFormat:   list of channels\n" );
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_INVALID:                         
        default:
            fprintf(stderr,"Fatal error. Unsupported formatConverterInputFormat. This should have been catched before.\n" );
            break;            
    }
    
    switch ( hFormatConverter_params->formatConverterOutputFormat )
    {
        case FORMAT_CONVERTER_OUTPUT_FORMAT_1_0:
            fprintf(stderr,"formatConverterOutputFormat:  1.0\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_2_0:
            fprintf(stderr,"formatConverterOutputFormat:  2.0\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_1:
            fprintf(stderr,"formatConverterOutputFormat:  5.1\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1:
            fprintf(stderr,"formatConverterOutputFormat:  7.1 = 5.1 + 2 (upper L/R spks)\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1:
            fprintf(stderr,"formatConverterOutputFormat:  7.1 (surround L/R spks)\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT:
            fprintf(stderr,"formatConverterOutputFormat:  7.1 (center L/R spks) \n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_8_1:
            fprintf(stderr,"formatConverterOutputFormat:  8.1 (without mid-center)\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_10_1:
            fprintf(stderr,"formatConverterOutputFormat:  10.1 = 5.1 + 4 + T\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_1:
            fprintf(stderr,"formatConverterOutputFormat:  9.1 = 5.1 + 4\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_0:
            fprintf(stderr,"formatConverterOutputFormat:  9.0 = 5.0 + 4\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_11_1:
            fprintf(stderr,"formatConverterOutputFormat:  11.1 = 5.1 + 5 + T\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0:
            fprintf(stderr,"formatConverterOutputFormat:  4.0 + 4\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0:
            fprintf(stderr,"formatConverterOutputFormat:  4.0 + 4 + T\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC:
            fprintf(stderr,"formatConverterOutputFormat:  3.0 (front center)\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC:
            fprintf(stderr,"formatConverterOutputFormat:  3.0 (rear center)\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_0:
            fprintf(stderr,"formatConverterOutputFormat:  4.0\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_0:
            fprintf(stderr,"formatConverterOutputFormat:  5.0\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_6_1:
            fprintf(stderr,"formatConverterOutputFormat:  6.1 = 5.1 + rear center\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC:
            fprintf(stderr,"formatConverterOutputFormat:  generic\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS:
            fprintf(stderr,"formatConverterOutputFormat:  list of channels\n" );
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID:                         
        default:
            fprintf(stderr,"Fatal error. Unsupported formatConverterOutputFormat. This should have been catched before.\n" );
            break;            
    }
    
    fprintf(stderr,"numInputChans:                %d\n", hFormatConverter_params->numInputChans );
    fprintf(stderr,"numOutputChans:               %d\n", hFormatConverter_params->numOutputChans );
    fprintf(stderr,"delay [samples]:              %d\n", hFormatConverter_params->formatConverterDelay );
    fprintf(stderr,"frameSize:                    %d\n", hFormatConverter_params->formatConverterFrameSize );
    fprintf(stderr,"numFreqBands:                 %d\n", hFormatConverter_params->numFreqBands );
    fprintf(stderr,"eqLimitMax [dB]:              %2.1f\n", hFormatConverter_params->eqLimitMax );
    fprintf(stderr,"eqLimitMin [dB]:              %2.1f\n", hFormatConverter_params->eqLimitMin );
    fprintf(stderr,"applyEqFilters:               %d\n", hFormatConverter_params->applyEqFilters );      
    fprintf(stderr,"randomFlag:                   %d\n", hFormatConverter_params->randomFlag );      
    fprintf(stderr,"trimFlag:                     %d\n", hFormatConverter_params->trimFlag );   
    
    if (mode > 0) {       
        
        if(hFormatConverter_params->formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC){
            if (hFormatConverter_params->randomFlag != 0) {   
            
                fprintf(stderr,"\n");
                fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
            
                fprintf(stderr,"Angle Deviation [deg] =\n");
                for (i = 0; i < hFormatConverter_params->numOutputChans; i++) {
                    fprintf(stderr,"Channel %d: Azimuth Deviation: %3.1f, Elevation Deviation: %3.1f\n", \
                        i, hFormatConverter_params->azimuthElevationDeviation[i*2],\
                        hFormatConverter_params->azimuthElevationDeviation[i*2+1]);        
                }
                
            }        
            if (hFormatConverter_params->trimFlag != 0) {   
            
                fprintf(stderr,"\n");
                fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
                
                fprintf(stderr,"Distance [m] =\n");
                for (i = 0; i < hFormatConverter_params->numOutputChans; i++) {
                    fprintf(stderr,"Channel %d: Distance: %3.1f\n", i, hFormatConverter_params->distance[i]);        
                }
            
            }
        
            fprintf(stderr,"\n");    
            fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        
        
            fprintf(stderr,"in_out-Matrix =\n");   
        
            i = 0;
            while ((hFormatConverter_params->formatConverterParams_internal)->in_out_src[i] >= 0) {
                fprintf(stderr,"  %d   %d   %5.2f   %d\n", \
                  (hFormatConverter_params->formatConverterParams_internal)->in_out_src[i], \
                  (hFormatConverter_params->formatConverterParams_internal)->in_out_dst[i], \
                  (hFormatConverter_params->formatConverterParams_internal)->in_out_gain[i], \
                  (hFormatConverter_params->formatConverterParams_internal)->in_out_proc[i]);
                i++;
            }   
            fprintf(stderr,"\n");
            fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        } /* if(outfmt!=generic) */
                
        fprintf(stderr,"DMX-Matrix =\n");
        for (i = 0; i < hFormatConverter_params->numInputChans; i++) {
            for (j = 0; j < hFormatConverter_params->numOutputChans; j++) {
                fprintf(stderr,"%2.4f ",hFormatConverter_params->dmxMtx[i][j]);
            }
            fprintf(stderr,"\n");
        }
        fprintf(stderr,"\n");
        fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        
        fprintf(stderr,"MIX-Matrix =\n");
        for (i = 0; i < hFormatConverter_params->numInputChans; i++) {
            for (j = 0; j < hFormatConverter_params->numInputChans; j++) {
                fprintf(stderr,"%d ",hFormatConverter_params->mixMtx[i][j]);
            }
            fprintf(stderr,"\n");
        }
        fprintf(stderr,"\n");
        fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
        fprintf(stderr,"EQ-Control-Matrix =\n");
        for (i = 0; i < hFormatConverter_params->eqControlMatrixSize; i++) {
            for (j = 0; j < 2; j++) {
                fprintf(stderr,"%d ",hFormatConverter_params->eqControlMatrix[i][j]);
            }
            fprintf(stderr,"\n");
        }
        
        fprintf(stderr,"\n");
        fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
#endif
        
        fprintf(stderr,"EQ-Matrix =\n");
        for (i = 0; i < numFreqBands; i++) {
            for (j = 0; j < N_EQ; j++) {
                fprintf(stderr,"%1.4f ",(hFormatConverter_params->formatConverterParams_internal)->eq[j][i]);
            }
            fprintf(stderr,"\n");
        }    
        
#ifndef FORMATCONVERTER_LOWCOMPLEXITY /* RM0 code */
        fprintf(stderr,"\n");
        fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        
        fprintf(stderr,"Gain-Compensation-Control-Matrix =\n");
        for (i = 0; i < hFormatConverter_params->gainCompensationControlMatrixSize; i++) {
            fprintf(stderr,"%d ",hFormatConverter_params->gainCompensationControlMatrix[i]);
            fprintf(stderr,"%1.4f ",hFormatConverter_params->gainCompensationValue);            
            fprintf(stderr,"\n");
        }
#endif        
    }
    
    fprintf(stderr,"\n");    
    fprintf(stderr,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");

    return 0;
}

/**********************************************************************************************************************************/

const char *formatConverterGetInfo( void )
{
    return ( "Built " __DATE__ ", " __TIME__ );
}
  
/**********************************************************************************************************************************/

int formatConverterGetNumInputChans( const FORMAT_CONVERTER_INPUT_FORMAT formatConverterInputFormat )
{
    switch (formatConverterInputFormat) {
        case FORMAT_CONVERTER_INPUT_FORMAT_5_1:
            return 6;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_5_2_1:
            return 8;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1:
            return 8;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT:
            return 8;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_10_1:
            return 11;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_22_2:
            return 24;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_14_0:
            return 14;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_9_1:
            return 10;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_9_0:
            return 9;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_11_1:
            return 12;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_0:
            return 8;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0:
            return 9;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC:
            return 3;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC:
            return 3;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_4_0:
            return 4;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_5_0:
            return 5;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_6_1:
            return 7;
            break;
        case FORMAT_CONVERTER_INPUT_FORMAT_INVALID:
        default:
            return 0;
            break;
    }   
}

/**********************************************************************************************************************************/

int formatConverterGetNumOutputChans( const FORMAT_CONVERTER_OUTPUT_FORMAT formatConverterOutputFormat )
{
    switch (formatConverterOutputFormat) {
        case FORMAT_CONVERTER_OUTPUT_FORMAT_1_0:
            return 1;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_2_0:
            return 2;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_1:
            return 6;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1:
            return 8;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1:
            return 8;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT:
            return 8;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_8_1:
            return 9;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_10_1:
            return 11;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_1:
            return 10;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_9_0:
            return 9;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_11_1:
            return 12;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0:
            return 8;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0:
            return 9;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC:
            return 3;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC:
            return 3;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_4_0:
            return 4;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_5_0:
            return 5;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_6_1:
            return 7;
            break;
        case FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID:
        default:
            return 0;
            break;
    }    
}    

/**********************************************************************************************************************************/
int setCustomDownmixParameters( HANDLE_FORMAT_CONVERTER_STATE formatConverter_state, int adaptiveEQStrength, int phaseAlignStrength)
{
  phaseAlignSetAES( adaptiveEQStrength, formatConverter_state->handlePhaseAligner );
  phaseAlignSetPAS( phaseAlignStrength, formatConverter_state->handlePhaseAligner );
  return 0;
}

/**********************************************************************************************************************************/
int setCustomDownmixParameter_STFT( HANDLE_FORMAT_CONVERTER_STATE formatConverter_state, int adaptiveEQStrength)
{
  activeDmxSetAES( adaptiveEQStrength, formatConverter_state->handleActiveDmxStft);
  return 0;
}

/**********************************************************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */



