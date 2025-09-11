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

/*
 *  File formatConverter_api.h
 *  of module formatConverter.
 *  
 */    

#ifndef _FORMATCONVERTER_API_H_
#define _FORMATCONVERTER_API_H_

#include "cicp2geometry.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** List of channels known to the format coverter DMX matrix generator. 
    NOTE: Keep this list in sync with implementation! */
typedef enum
{
    /* horizontal channels */
    API_CH_M_000 =      0,
    API_CH_M_L015 =     1,
    API_CH_M_R015 =     2,
    API_CH_M_L030 =     3,
    API_CH_M_R030 =     4,
    API_CH_M_L045 =     5,
    API_CH_M_R045 =     6,
    API_CH_M_L060 =     7,
    API_CH_M_R060 =     8,
    API_CH_M_L090 =     9,
    API_CH_M_R090 =     10,
    API_CH_M_L110 =     11,
    API_CH_M_R110 =     12,
    API_CH_M_L135 =     13,
    API_CH_M_R135 =     14,
    API_CH_M_L150 =     15,
    API_CH_M_R150 =     16,
    API_CH_M_180 =      17,
    /* height channels */
    API_CH_U_000 =      18,
    API_CH_U_L045 =     19,
    API_CH_U_R045 =     20,
    API_CH_U_L030 =     21,
    API_CH_U_R030 =     22,
    API_CH_U_L090 =     23,
    API_CH_U_R090 =     24,
    API_CH_U_L110 =     25,
    API_CH_U_R110 =     26,
    API_CH_U_L135 =     27,
    API_CH_U_R135 =     28,
    API_CH_U_180 =      29,
    /* top channel */
    API_CH_T_000 =      30,
    /* low channels */
    API_CH_L_000 =      31,
    API_CH_L_L045 =     32,
    API_CH_L_R045 =     33,
    /* low frequency effects */
    API_CH_LFE1 =       34,
    API_CH_LFE2 =       35,
    /* empty channel */
    API_CH_EMPTY =      -1
} api_chid_t;

/** Handle to Format Converter parameter struct.
    Holds Format Converter parameter data. */
typedef struct T_FORMAT_CONVERTER_PARAMS    *HANDLE_FORMAT_CONVERTER_PARAMS;

/** Handle to Format Converter state struct.
    Holds Format Converter state data. */
typedef struct T_FORMAT_CONVERTER_STATE    *HANDLE_FORMAT_CONVERTER_STATE;

/* parameters struct for peak filter definition */
typedef struct {
    float f;    /* peak frequency [Hz] */
    float q;    /* peak Q factor       */ 
    float g;    /* peak gain [dB]      */ 
} pkFilterParamsStruct;

/* parameters struct for EQ definition */
typedef struct {
    int nPkFilter;                        /* number of cascaded peak filters */
    float G;                              /* global gain [dB]                */
    pkFilterParamsStruct *pkFilterParams; /* pointer to nPkFilter-element array of pkFilterParamsStructs */
} eqParamsStruct;

/** Supported Format Converter processing modi.
    FORMAT_CONVERTER_MODE_INVALID = 0                    : invalid processing mode. \n
    FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN            : .  \n
    FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN            : .  \n
    FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN : .  \n*/    
typedef enum
{
    FORMAT_CONVERTER_MODE_INVALID = 0,
    FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN,
    FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN,
    FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN,
    FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN,
    FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT,
    FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT

} FORMAT_CONVERTER_MODE;
      
/** Supported Format Converter input formats (IIS channel order).
 FORMAT_CONVERTER_INPUT_FORMAT_INVALID = 0       : invalid input format. \n
 FORMAT_CONVERTER_INPUT_FORMAT_5_1               : 5.1 input format (default channel order).   \n
 FORMAT_CONVERTER_INPUT_FORMAT_5_2_1 \n
 FORMAT_CONVERTER_INPUT_FORMAT_7_1 \n
 FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT \n
 FORMAT_CONVERTER_INPUT_FORMAT_10_1 \n
 FORMAT_CONVERTER_INPUT_FORMAT_22_2              : 22.2 input format (IIS channel order).  \n
 FORMAT_CONVERTER_INPUT_FORMAT_14_0              : 14.0 input format (IIS channel order).  \n
 FORMAT_CONVERTER_INPUT_FORMAT_9_1 \n
 FORMAT_CONVERTER_INPUT_FORMAT_9_0               : 9.0 input format (IIS channel order).   \n
 FORMAT_CONVERTER_INPUT_FORMAT_11_1              : 11.1 input format (IIS channel order).  \n
 FORMAT_CONVERTER_INPUT_FORMAT_4_4_0 \n
 FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0 \n
 FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC \n
 FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC \n
 FORMAT_CONVERTER_INPUT_FORMAT_4_0 \n
 FORMAT_CONVERTER_INPUT_FORMAT_5_0 \n
 FORMAT_CONVERTER_INPUT_FORMAT_6_1  \n
 FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS \n    
 FORMAT_CONVERTER_INPUT_FORMAT_GENERIC \n*/    
typedef enum
{
    FORMAT_CONVERTER_INPUT_FORMAT_INVALID = 0,
    FORMAT_CONVERTER_INPUT_FORMAT_5_1,
    FORMAT_CONVERTER_INPUT_FORMAT_5_2_1,
    FORMAT_CONVERTER_INPUT_FORMAT_7_1,
    FORMAT_CONVERTER_INPUT_FORMAT_7_1_ALT,
    FORMAT_CONVERTER_INPUT_FORMAT_10_1,
    FORMAT_CONVERTER_INPUT_FORMAT_22_2,
    FORMAT_CONVERTER_INPUT_FORMAT_14_0,
    FORMAT_CONVERTER_INPUT_FORMAT_9_1,
    FORMAT_CONVERTER_INPUT_FORMAT_9_0,
    FORMAT_CONVERTER_INPUT_FORMAT_11_1,
    FORMAT_CONVERTER_INPUT_FORMAT_4_4_0,
    FORMAT_CONVERTER_INPUT_FORMAT_4_4_T_0,
    FORMAT_CONVERTER_INPUT_FORMAT_3_0_FC,
    FORMAT_CONVERTER_INPUT_FORMAT_3_0_RC,
    FORMAT_CONVERTER_INPUT_FORMAT_4_0,
    FORMAT_CONVERTER_INPUT_FORMAT_5_0,
    FORMAT_CONVERTER_INPUT_FORMAT_6_1,
    FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS,
    FORMAT_CONVERTER_INPUT_FORMAT_GENERIC
} FORMAT_CONVERTER_INPUT_FORMAT;
  
/** Supported Format Converter output formats (IIS channel order).
 FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID = 0    : invalid output format. \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_1_0            : 1.0 output format (default channel order).  \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_2_0            : 2.0 output format (default channel order).  \n    
 FORMAT_CONVERTER_OUTPUT_FORMAT_5_1            : 5.1 output format (default channel order).  \n    
 FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_7_1 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_8_1            : 8.1 output format (IIS channel order).      \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_10_1           : 10.1 output format (IIS channel order).     \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_9_1 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_9_0 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_11_1 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_4_0 \n 
 FORMAT_CONVERTER_OUTPUT_FORMAT_5_0 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_6_1 \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS \n
 FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC \n*/
typedef enum
{
    FORMAT_CONVERTER_OUTPUT_FORMAT_INVALID = 0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_1_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_2_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_5_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_5_2_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_7_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_7_1_ALT,
    FORMAT_CONVERTER_OUTPUT_FORMAT_8_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_10_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_9_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_9_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_11_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_4_4_T_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_FC,
    FORMAT_CONVERTER_OUTPUT_FORMAT_3_0_RC,
    FORMAT_CONVERTER_OUTPUT_FORMAT_4_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_5_0,
    FORMAT_CONVERTER_OUTPUT_FORMAT_6_1,
    FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS,
    FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC
} FORMAT_CONVERTER_OUTPUT_FORMAT;


/**
   Match input/output channel configurations given by channel geometry data to channel labels 
   known to the format converter DMX matrix generator (= format converter initialization).
   \param chConfig geometry definition of setup to match
  */
int formatConverterMatchChConfig2Channels(
        const CICP2GEOMETRY_CHANNEL_GEOMETRY     *chConfig,       /* in:   channel configuration */
        const int                  inout,           /* in:   0: input config, 1: output config */
        const int                  numChannels,     /* in:   number of channels in channel config */
        const int                  numLfes,         /* in:   number of LFEs in channel config */
        const int                  printResult,     /* in:   1: print matching result for debugging/development */
        api_chid_t                 **channel_vec,   /* out:  vector containing channel enums */
        unsigned int               *numUnknownCh,   /* out:  number of unknown channels */
        int                        **unknownCh_vec, /* out:  vector containing indices(=position in setup) of channels unknown to the format converter */ 
        float                      **azOffset_vec,  /* out:  vector containing azimuth offsets in degrees */ 
        float                      **elOffset_vec   /* out:  vector containing elevation offsets in degrees */ 
);


/**
  * Set input format or output format that is not in list of known formats.
  * Each function call either sets input _or_ output format.
  */
int formatConverterSetInOutFormat(
        const int                  inout,           /* in:  0: set input format, 1: set output format */
        const int                  num_channels,    /* in:  number of channels in setup to define */
        const int                  *channel_vector  /* in:  vector containing channel enums */
);

/** Open Format Converter.
    Allocates Format Converter structs.
    \param formatConverterMode Format Converter processing mode.
    \param formatConverterInputFormat Format Converter input format.
    \param formatConverterOutputFormat Format Converter output format.
    \param samplingRate The sampling rate [Hz].
    \param frameSize format fonverter frameSize in samples for time AND frequency domain processing.
    \param numFreqBands num frequency bands for freq domain processing.
    \param stftNumErbBands num processing bands for stft processing.
    \param stftErbFreqIdx stop indices of processing bands for stft processing.
    \param stftLength fft length.
    \param numInputChans Returns/reads number of input channels. Reads in case of generic input format.
    \param numOutputChans Returns/reads number of output channels. Reads in case of generic output format.
    \param delaySamples returns the format converter delay in samples.
    \param phFormatConverter_params A pointer to a handle to Format Converter parameter struct
    \param phFormatConverter_state A pointer to a handle to Format Converter state struct.
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterOpen(    
                        const FORMAT_CONVERTER_MODE          formatConverterMode,  
                        const FORMAT_CONVERTER_INPUT_FORMAT  formatConverterInputFormat,
                        const FORMAT_CONVERTER_OUTPUT_FORMAT formatConverterOutputFormat,
                        const int                            samplingRate,  
                        const int                            frameSize,  
                        const int                            numFreqBands,
                        const int                             stftNumErbBands,
                        const int                             *stftErbFreqIdx,
                        const int                             stftLength,
                        int                                  *numInputChans,
                        int                                  *numOutputChans,
                        int                                  *delaySamples,
                        HANDLE_FORMAT_CONVERTER_PARAMS       *phFormatConverter_params,
                        HANDLE_FORMAT_CONVERTER_STATE        *phFormatConverter_state );


/** Initialize Format Converter.
    Perform initialization of Format Converter with given parameters.
    \param centerFrequeciesNormalized required vector with normalized center frequencies of filterbank used for frequency domain processing.
    \param azimuthDeviation optional vector with azimuth deviations of output format speakers from specified default output format speakers. In [deg]. Disable with NULL ptr.
    \param elevationDeviation optional vector with elevation deviations of output format speakers from specified default output format speakers. In [deg]. Disable with NULL ptr.
    \param distance optional vector with distances of output format speakers from sweet spot position = Radius in [m]. Disable with NULL ptr.    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.    
    \return Returns 0 on success, otherwise an error code.    */
int  formatConverterInit(                         
                         const float                      *centerFrequeciesNormalized,
                         const float                      *azimuthDeviation,
                         const float                      *elevationDeviation,
                         const float                      *distance,
                         HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                         HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );


/** Format Converter processing in time domain.
    Performs time domain processing of format converter for one frame.
    \param inputBuffer inputBuffer with format (numInputChans x frameSize)
    \param outputBuffer outputBuffer with format (numOutputChans x frameSize).    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.    
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterProcess_timeDomain( 
                            const float                      **inputBuffer,
                            float                            **outputBuffer,
                            HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                            HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
    
/** Format Converter processing in frequency domain.
    Performs frequency domain processing of format converter for one frame.
    \param inputBuffer_real real inputBuffer with format (numInputChans x numFreqBands).
    \param inputBuffer_imag imaginary inputBuffer with format (numInputChans x numFreqBands).
    \param outputBuffer_real real outputBuffer with format (numOutputChans x numFreqBands).
    \param outputBuffer_imag imaginary outputBuffer with format (numOutputChans x numFreqBands).    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.    
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterProcess_freqDomain( 
                                const float                      **inputBuffer_real,
                                const float                      **inputBuffer_imag,
                                float                            **outputBuffer_real,
                                float                            **outputBuffer_imag,
                                HANDLE_FORMAT_CONVERTER_PARAMS   hFormatConverter_params,
                                HANDLE_FORMAT_CONVERTER_STATE    hFormatConverter_state );
 
int  formatConverterProcess_freqDomain_STFT( 
                                const float                       **inputBufferStft, 
                                float                             **outputBufferStft,
                                HANDLE_FORMAT_CONVERTER_PARAMS    hFormatConverter_params, 
                                HANDLE_FORMAT_CONVERTER_STATE     hFormatConverter_state );

/** Close Format Converter.
    Deallocates Format Converter structs.
    \param phFormatConverter_params A pointer to a handle to Format Converter parameter struct
    \param phFormatConverter_state A pointer to a handle to Format Converter state struct.  */
int  formatConverterClose(
                        HANDLE_FORMAT_CONVERTER_PARAMS   *phFormatConverter_params,
                        HANDLE_FORMAT_CONVERTER_STATE    *phFormatConverter_state );
    
    
/** Get DMX Matrix of Format Converter.
    Extracts DMX Matrix from internal format converter struct.
    \param dmxMtx returns DMX Matrix in format (numInputChans x numOutputChans) --> correct memory allocation required!!!.    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterGetDmxMtx(
                               float **dmxMtx,
                               HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params);    

/** Postprocess DMX gains matrix. 
    Applies a threshold to MTX gains to avoid excessive use of phantom sources.
    Normalizes gains to preserve the same energy as before post-processing the matrix.
    \param dmxMtx DMX Matrix in format (numInputChans x numOutputChans).    
    \param numInputChans Number of input channels
    \param numOutputChans Number of output cannels
    \return Returns 0 on success, otherwise 1.    */
int formatConverterPostprocessDmxMtx(
                                float **dmxMtx,
                                const int numInputChans,
                                const int numOutputChans);

/** Directly map LFE input channel(s) to LFE output channel(s) if LFE(s) present in output setup.
    Replaces matrix entries for mapping LFE(s) if LFE(s) present in target.
    \param dmxMtx DMX Matrix in format (numInputChans x numOutputChans)
    \param numInputChans Number of input channels
    \param inputChConfig Geometry definition of input setup
    \param numOutputChans Number of output cannels
    \param outputChConfig Geometry definition of output setup
    \return Returns 0 on success, otherwise 1.    */
int directMapLFEs(
                float **dmxMtx,
                const int numInputChans,
                const CICP2GEOMETRY_CHANNEL_GEOMETRY *inputChConfig,
                const int numOutputChans,
                const CICP2GEOMETRY_CHANNEL_GEOMETRY *outputChConfig);

/** Set DMX Matrix of Format Converter.
    Overwrite internal DMX Matrix with external DMX Matrix.
    \param dmxMtx DMX Matrix in format (numInputChans x numOutputChans).    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.         
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterSetDmxMtx(
                                const float **dmxMtx,
                                HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                                HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state);

/** Add DMX Matrix to Currently Active DMX Matrix (element wise addition).
    \param dmxMtx DMX Matrix in format (numInputChans x numOutputChans).    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.         
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterAddDmxMtx(
                                const float **dmxMtx,
                                HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                                HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state);

/** Set EQs of Format Converter.
    \param numEQs number of EQ definitions
    \param eqParams pointer to numEQs eqParamsStructs
    \param eqMap vector (length: numInputChans) indicating which EQ to apply to which input, 0 = no EQ
    \param bands_nrm normalized processing band frequencies: 1.0 corresponds to fs/2
    \param sfreq_Hz sampling frequency in Hz
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \param hFormatConverter_state A handle to Format Converter state struct.         
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterSetEQs(
                             int numEQs,
                             eqParamsStruct *eqParams,
                             int *eqMap,
                             float *bands_nrm,
                             float sfreq_Hz,
                             HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params,
                             HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state);
   
/** Get Mix Matrix of Format Converter.
    Extracts Mix Matrix from internal format converter struct.
    \param mixMtx returns Mix Matrix in format (numInputChans x numInputChans) --> correct memory allocation required!!!.   .    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterGetMixMtx(
                                int **mixMtx,
                                HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params);

/** Get trim parameters of Format Converter.
    Extracts trim parameters (trim gains and delays) from internal format converter struct.
    \param trimGains returns gains in float vector of length numOutputChans --> correct memory allocation required!!!.   .    
    \param trimDelays returns delays in samples in int vector of length numOutputChans --> correct memory allocation required!!!.   .    
    \param hFormatConverter_params A handle to Format Converter parameter struct
    \return Returns 0 on success, otherwise 1.    */
int  formatConverterGetTrimParams(
                                float *trimGains,
                                int *trimDelays,
                                HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params);

    
/** Return processing delay of Format Converter.
    \param mode format converter processing mode.
    \return Returns delay in samples on success, otherwise -1.  */    
int formatConverterGetDelaySamples( FORMAT_CONVERTER_MODE mode );
    
/** Display Format Converter parameters.
    Displays certain Format Converter parameters.
    \param mode mode of format converter display (0: nearly silent, 1: print minimal information, 2: print all information).
    \param hFormatConverter_params A handle to Format Converter parameter struct.
    \return Returns 0 on success, otherwise 1.  */    
int formatConverterDisplayParams( int mode, const HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params );


/** Get build info of Format Converter.
    Build date and time of Format Converter.
    \return Returns string of build date and time.  */    
const char *formatConverterGetInfo( void );   


int setCustomDownmixParameters( HANDLE_FORMAT_CONVERTER_STATE formatConverter_state, int adaptiveEQStrength, int phaseAlignStrength);
int setCustomDownmixParameter_STFT( HANDLE_FORMAT_CONVERTER_STATE formatConverter_state, int adaptiveEQStrength);

int  formatConverterSetDmxMtx_STFT(const HANDLE_FORMAT_CONVERTER_PARAMS hFormatConverter_params, HANDLE_FORMAT_CONVERTER_STATE hFormatConverter_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


