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
 *  File formatConverter_main.c
 *  of formatConverter cmdl app.
 *  
 */   

/**************************************************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "wavIO.h"

#include "formatConverter_main.h"
    
#define QMFLIB_MAX_CHANNELS 32
#define QMF_BANDS 64

#include "qmflib_hybfilter.h"
#include "qmflib.h"
#include "fftlib.h"
/**************************************************************************************************************************************************************************************/

#ifdef __APPLE__
#pragma mark MAIN 
#endif

int main(int argc, char *argv[])
{
       
    unsigned int i, j;
    int Nin_int = 0;
    int Nout_int = 0;   
    int errorFlag  = 0;    
    int frameNo    = -1;   
    
    unsigned long int NumberSamplesWritten = 0;
    unsigned long int NumberSamplesWritten_real = 0;
    unsigned long int NumberSamplesWritten_imag = 0;
    unsigned int isLastFrameWrite = 0;    
    unsigned int isLastFrameRead = 0;    
    
    char* inFilename               = NULL;
    char* outFilename              = NULL;
    char* inFilenameDmxMtx         = NULL;
    char* inFilenameAngleDeviation = NULL;
    char* inFilenameDistance       = NULL;
    char* inFilenameEqParams       = NULL;
    char* wavExtension             = ".wav";
    char* txtExtension             = ".txt";   

    char* inFilenameInputChannelList  = NULL;
    char* inFilenameOutputChannelList = NULL;
    char qmfInFilenameReal[200];
    char qmfInFilenameImag[200];
    char qmfOutFilenameReal[200];
    char qmfOutFilenameImag[200];
    char* outFilenameDmxMtx = NULL;
    char* outFilenameMixMtx = NULL; 
    char outFilenameTrimParams[200];        

    WAVIO_HANDLE hWavIO      = NULL;    
    WAVIO_HANDLE hWavIO_real = NULL;
    WAVIO_HANDLE hWavIO_imag = NULL;  
    
    unsigned int frameSize     = 1024;        
    int moduleDelay            = 0;
    int numSamplesToCompensate = 0;
    unsigned int nInChannels   = 0;
    unsigned int nOutChannels  = 0;
    unsigned int inSampleRate  = 48000;
    int outSampleRate          = -1;
    int bFillLastFrame         = 1;     
    int nSamplesPerChannelFilled;
    int nSamplesPerChannelFilled_real;
    int nSamplesPerChannelFilled_imag;
    int qmfIOFlag          = 0;
    int helpFlag           = 0;
    int dmxMtxInputFlag    = 0;
    int dmxMtxOutputFlag   = 0;
    int mixMtxOutputFlag   = 0;    
    int angleDeviationFlag = 0;    
    int distanceFlag       = 0;    
    int extEqFlag          = 0;    
    int inChannelVbapFlag  = 0;
    int outChannelVbapFlag = 0;
    unsigned int numUnknownCh       = 0;
    int *unknownCh_vec     = NULL;
    float *azimuthDeviationFromMatching = NULL;
    float *elevationDeviationFromMatching = NULL;
    CICP2GEOMETRY_CHANNEL_GEOMETRY* inputChConfig = NULL;        
    CICP2GEOMETRY_CHANNEL_GEOMETRY* outputChConfig = NULL;        

    FILE *pInFile            = NULL;
    FILE *pQmfInRealFile     = NULL;
    FILE *pQmfInImagFile     = NULL;
    FILE *pOutFile           = NULL;
    FILE *pQmfOutRealFile    = NULL;
    FILE *pQmfOutImagFile    = NULL;    
    FILE *pOutFileDmxMtx     = NULL;
    FILE *pOutFileMixMtx     = NULL;
    FILE *pOutFileTrimParams = NULL;  

    unsigned long nTotalSamplesPerChannel;
    unsigned long nTotalSamplesPerChannel_real;
    unsigned long nTotalSamplesPerChannel_imag;
    unsigned long nTotalSamplesWrittenPerChannel;    
    unsigned long nTotalSamplesWrittenPerChannel_real;    
    unsigned long nTotalSamplesWrittenPerChannel_imag;    
        
    unsigned int bytedepth_input  = 0;
    unsigned int bytedepth_output = 0;
    unsigned int bitdepth_output  = 0;
    
    float **inBuffer          = NULL;
    float **outBuffer         = NULL; 
    float **dmxMtx            = NULL;
    int   **mixMtx            = NULL;
    int   *trimDelays         = NULL;
    float *trimGains          = NULL;
    float *azimuthDeviation   = NULL;
    float *elevationDeviation = NULL;
    float *azimuthDeviationExt   = NULL;
    float *elevationDeviationExt = NULL;
    float *distance           = NULL;
       
    /* parameter definition */    
    HANDLE_FORMAT_CONVERTER_PARAMS  hFormatConverter_params     = NULL;
    HANDLE_FORMAT_CONVERTER_STATE   hFormatConverter_state      = NULL;
    FORMAT_CONVERTER_MODE           formatConverterMode;
    FORMAT_CONVERTER_INPUT_FORMAT   formatConverterInputFormat;
    FORMAT_CONVERTER_OUTPUT_FORMAT  formatConverterOutputFormat;
    int                             formatConverterDelay;
    float                           *formatConverterCenterFrequencies  = NULL; 
    float                           *formatConverterAzimuthDeviation   = NULL; 
    float                           *formatConverterElevationDeviation = NULL; 
    float                           *formatConverterDistance  = NULL; 
    
    /*** stft processing parameters ***/


    unsigned int                    stftFormatConverterFrameSize;
    unsigned int                    stftFormatConverterFFTLength;
    unsigned int                    stftFormatConverterNumErbBands;
    unsigned int                    stftFormatConverterNumTimeSlots;
    int                             *stftFormatConverterErbFreqIdx;
    float                           **inputBufferStft           = NULL;        /*real, imag interleaved, N/2+1 coefficient stored in imaginary part of first coefficient-> stft of length N results in N interleaved coefficients*/
    float                           **outputBufferStft          = NULL;
    float                           *stftWindow                 = NULL;
    float                           *stftWindowedSig            = NULL;
    float                           **stftTmpInputBufferTime    = NULL;
    float                           **stftTmpOutputBufferTime   = NULL;
    float                           *stftInputBufferTimeImag    = NULL;
    float                           *stftOutputBufferTimeReal   = NULL;
    float                           *stftOutputBufferTimeImag   = NULL;
    float                           *stftOutputBufferFreqReal   = NULL;
    float                           *stftOutputBufferFreqImag   = NULL;
    float                           *stftInputBufferFreqReal    = NULL;
    float                           *stftInputBufferFreqImag    = NULL;
    HANDLE_FFTLIB                   hFFT                        = NULL; 
    HANDLE_FFTLIB                   hIFFT                       = NULL; 


    /* default cmdl parameters */
    int inConfIdx = 0; /* (22.2 = 0, 14.0 = 1, 11.1 = 2, 9.0 = 3, 5.1 = 4) */
    int outConfIdx = 2; /* (10.1 = 0, 8.1 = 1, 5.1 = 2, 2.0 = 3, 1.0 = 4, MPEG-Rand5 = 5, MPEG-Rand10 = 6) */
    int dmxTypeIdx = 3; /* (passive TD = 0, passive FD = 1, active-PA FD (QMF) = 3, custom FD (QMF) = 4,  active FD (STFT) = 5, custom FD (STFT) = 6)) */
    int plotInfo = 1; /* (0: nearly silent, 1: print minimal information, 2: print all information) */
    int compensateDelayFlag = 0; /* set by -d cmdl flag */
    int phaseAlignStrength = -1;
    int adaptiveEQStrength = -1;

    int samplingRate                         = -1;
    unsigned int formatConverterNumInputChans         = 0;
    unsigned int formatConverterNumOutputChans        = 0;
    int formatConverterFrameSize             = 0;
    int formatConverterNumFreqBands          = 0;
    
    /* input/output buffers */
    float **inputBufferFreqQMF_real   = NULL;
    float **inputBufferFreqQMF_imag   = NULL;
    float **outputBufferFreqQMF_real  = NULL;
    float **outputBufferFreqQMF_imag  = NULL;
    float **inputBufferFreq_real      = NULL;
    float **inputBufferFreq_imag      = NULL;
    float **outputBufferFreq_real     = NULL;
    float **outputBufferFreq_imag     = NULL;    

    /**************************************************************************************************************************************************************************************/
    /* QMF Parameters */
    /**************************************************************************************************************************************************************************************/
    
#ifdef FORMATCONVERTER_LOWCOMPLEXITY
    int hybMode = 0; /* -1 = no hybrid, 0 = 71, 1 = 73, 2 = 77 */
#else
    int hybMode = 2; /* -1 = no hybrid, 0 = 71, 1 = 73, 2 = 77 */
#endif
    int ldMode = 0; /* don't use low delay QMF */
    int alignDelay = 1; /* 0: Align hybrid HF part with LF part, 1: no delay alignment for HF part (submission) */
    int numQMFBands = QMF_BANDS;
    int qmfDelay = 0;


    QMFLIB_POLYPHASE_ANA_FILTERBANK **pAnalysisQmf;
    QMFLIB_POLYPHASE_SYN_FILTERBANK **pSynthesisQmf;    
    QMFLIB_HYBRID_FILTER_STATE *hybFilterState; 
    QMFLIB_HYBRID_FILTER_MODE hybridMode;
    
    switch (hybMode) {
        case 0:
            hybridMode = QMFLIB_HYBRID_THREE_TO_TEN;
            break;
        case 2:
            hybridMode = QMFLIB_HYBRID_THREE_TO_SIXTEEN;
            break;
        case -1:
        default:
            hybridMode = QMFLIB_HYBRID_OFF;
            break;
    }
    
    /**************************************************************************************************************************************************************************************/
    /* Header */
    /**************************************************************************************************************************************************************************************/
    
    fprintf( stdout, "\n"); 
    fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*                             Format Converter Module                         *\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*                                  %s                                *\n", __DATE__);
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
    fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*******************************************************************************\n"); 
    fprintf( stdout, "\n");  
        
    /**************************************************************************************************************************************************************************************/
    /* ----------------- CMDL PARSING -----------------*/
    /**************************************************************************************************************************************************************************************/

#ifdef __APPLE__
#pragma mark CMDL_PARSING
#endif
        
    /* Commandline Parsing */       
    for ( i = 1; i < (unsigned int) argc; ++i )
    {
        if (!strcmp(argv[i],"-if"))      /* Required */
        {
            int nWavExtensionChars = 0;
            
            if ( argv[i+1] ) {
                inFilename = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename given.\n");
                return -1;
            }                
            nWavExtensionChars = strspn(wavExtension,inFilename);
            if ( nWavExtensionChars != 4 ) {
                qmfIOFlag = 1;
                strcpy(qmfInFilenameReal, inFilename);
                strcpy(qmfInFilenameImag, inFilename);
                strcat(qmfInFilenameReal, "_real.qmf");
                strcat(qmfInFilenameImag, "_imag.qmf");
            }                
            i++;
        }
        else if (!strcmp(argv[i],"-of"))   /* Required */
        {
            if (inFilename == NULL) {
                int nTxtExtensionChars = 0;
                
                if ( argv[i+1] ) {
                    outFilename = argv[i+1];
                }
                else {
                    fprintf(stderr, "No output filename given.\n");
                    return -1;
                }                    
                nTxtExtensionChars = strspn(txtExtension,outFilename);
                
                if ( nTxtExtensionChars != 4 ) {
                    fprintf(stderr, "Input filename missing (use -if option first) or TXT file extension missing for dmxMtx/mixMtx output.\n");
                    return -1;
                }                   
            } else {               
                int nWavExtensionChars = 0;
                
                if ( argv[i+1] ) {
                    outFilename = argv[i+1];
                }
                else {
                    fprintf(stderr, "No output filename given.\n");
                    return -1;
                }
                
                nWavExtensionChars = strspn(wavExtension,outFilename);
                if ( nWavExtensionChars != 4 ) {
                    strcpy(qmfOutFilenameReal, outFilename);
                    strcpy(qmfOutFilenameImag, outFilename);
                    strcat(qmfOutFilenameReal, "_real.qmf");
                    strcat(qmfOutFilenameImag, "_imag.qmf");
                    if ( !qmfIOFlag ) {
                        fprintf(stderr, "Mixed time/frequency domain input/output is NOT allowed!\n");
                        return -1;
                    }                    
                } else if (qmfIOFlag) {
                    fprintf(stderr, "Mixed time/frequency domain input/output is NOT allowed!\n");
                    return -1;
                } 
            }
            i++;
        } 
        else if (!strcmp(argv[i],"-cicpIn")) /* Required */
        {
            inConfIdx = atoi(argv[i+1]);
            i++;
        }  
        else if (!strcmp(argv[i],"-cicpOut")) /* Required */
        {
            outConfIdx = atoi(argv[i+1]);
            i++;
        }  
        else if (!strcmp(argv[i],"-inNch")) /* Required for generic format only */
        {
            Nin_int = atoi(argv[i+1]);
            i++;
        }  
        else if (!strcmp(argv[i],"-outNch")) /* Required for generic format only */
        {
            Nout_int = atoi(argv[i+1]);
            i++;
        }  
        else if (!strcmp(argv[i],"-dmx")) /* Optional */
        {
            dmxTypeIdx = atoi(argv[i+1]);
            i++;
        } 
        else if ( !strcmp(argv[i], "-pas") )
        {
          if ( dmxTypeIdx != FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN )
          {
            fprintf(stderr, "Warning: pas-switch only used on dmx-type 4!\n");
          }
          phaseAlignStrength = atoi( argv[i+1] );
          i++;
        }
        else if ( !strcmp( argv[i], "-aes") )
        {
          if ( dmxTypeIdx != FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN && dmxTypeIdx != FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT )
          {
            fprintf(stderr, "Warning: aes-switch only used on dmx-type 4 and 6!\n");
          }
          adaptiveEQStrength =  atoi( argv[i+1] );
          i++;
        }
        else if (!strcmp(argv[i],"-d")) /* Optional */
        {
            compensateDelayFlag = 1;
        }             
        else if (!strcmp(argv[i],"-info")) /* Optional */
        {
            plotInfo = atoi(argv[i+1]);
            i++;
        } 
        else if (!strcmp(argv[i],"-dmxMtx")) /* Optional */
        {
            int nTxtExtensionChars = 0;
            dmxMtxInputFlag = 1;
                
            if ( argv[i+1] ) {
                inFilenameDmxMtx = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for dmx matrix given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameDmxMtx);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for dmx matrix input.\n");
                return -1;
            }   
                i++;
        } 
        else if (!strcmp(argv[i],"-dmxMtxOutput")) /* Optional */
        {
            int nTxtExtensionChars = 0;
            dmxMtxOutputFlag = 1;

            if ( argv[i+1] ) {
                outFilenameDmxMtx = argv[i+1];
            }
            else {
                fprintf(stderr, "No filename for downmix matrix output given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,outFilenameDmxMtx);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for downmix matrix output file.\n");
                return -1;
            }                    
            i++;
        } 
        else if (!strcmp(argv[i],"-mixMtxOutput")) /* Optional */
        {
            int nTxtExtensionChars = 0;
            mixMtxOutputFlag = 1;

            if ( argv[i+1] ) {
                outFilenameMixMtx = argv[i+1];
            }
            else {
                fprintf(stderr, "No filename for mix matrix output given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,outFilenameMixMtx);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for mix matrix output file.\n");
                return -1;
            }                    
            i++;
        } 
        else if (!strcmp(argv[i],"-angleDev")) /* Optional */
        {                   
            int nTxtExtensionChars = 0;
            angleDeviationFlag = 1;
            
            if ( argv[i+1] ) {
                inFilenameAngleDeviation = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for angle deviations given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameAngleDeviation);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for angle deviations input.\n");
                return -1;
            }                    
            i++;
        } 
        else if (!strcmp(argv[i],"-dist")) /* Optional */
        {                   
            int nTxtExtensionChars = 0;
            distanceFlag = 1;
            
            if ( argv[i+1] ) {
                inFilenameDistance = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for loudspeaker distances given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameDistance);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for loudspeaker distances input.\n");
                return -1;
            }                    
            i++;
        }
        else if (!strcmp(argv[i],"-extEq")) /* Optional: external EQ parameters */
        {                   
            int nTxtExtensionChars = 0;
            extEqFlag = 1;
            
            if ( argv[i+1] ) {
                inFilenameEqParams = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for external EQ parameters given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameEqParams);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for EQ parameters input.\n");
                return -1;
            }                    
            i++;
        }
        else if (!strcmp(argv[i],"-inGeo")) /* needed if cicpIn==-1 */
        {                   
            int nTxtExtensionChars = 0;
           
            if ( argv[i+1] ) {
                inFilenameInputChannelList = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for input setup specification given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameInputChannelList);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for input setup specification file.\n");
                return -1;
            }                    
            i++;
        }
        else if (!strcmp(argv[i],"-outGeo")) /* needed if cicpOut==-1 */
        {                   
            int nTxtExtensionChars = 0;
           
            if ( argv[i+1] ) {
                inFilenameOutputChannelList = argv[i+1];
            }
            else {
                fprintf(stderr, "No input filename for output setup specification given.\n");
                return -1;
            }                    
            nTxtExtensionChars = strspn(txtExtension,inFilenameOutputChannelList);
            
            if ( nTxtExtensionChars != 4 ) {
                fprintf(stderr, "TXT file extension missing for output setup specification file.\n");
                return -1;
            }                    
            i++;
        }
        else if (!strcmp(argv[i],"-bitdepth")) /* Optional */
        {               
            bitdepth_output = atoi(argv[i+1]);
            if (bitdepth_output != 8 && bitdepth_output != 16 && bitdepth_output != 24 && bitdepth_output != 32) {
                fprintf(stderr, "Invalid bitdepth. Has to be 8,16,24 or 32!\n");
                return -1;
            }
            /* divide by 8 to get byte depth */
            bytedepth_output = bitdepth_output/8;
            i++;               
        } 
        else if (!strcmp(argv[i],"-h")) /* Optional */
        {
            helpFlag = 1;
        } 
        else if (!strcmp(argv[i],"-help")) /* Optional */
        {
            helpFlag = 1;
        }
        else {
            fprintf(stderr, "Invalid command line.\n");
            return -1;
        }
    }
    
    if ( ((inFilename == NULL) && (outFilename == NULL) && !( dmxMtxOutputFlag || mixMtxOutputFlag ) ) || helpFlag )
    {            
        if (!helpFlag) {
            fprintf( stderr, "Invalid input arguments!\n\n");
        }
        fprintf(stderr, "FormatConverterCmdl Usage:\n\n");
        fprintf(stderr, "Example (time domain interface):      <formatConverterCmdl -if in.wav -of out.wav -cicpIn 9 -cicpOut 1 -dmx 0 -d>\n");
        fprintf(stderr, "Example (random setup):               <formatConverterCmdl -if in.wav -of out.wav -cicpIn 9 -cicpOut 1 -angleDev dev.txt -dist dist.txt> \n");
        fprintf(stderr, "Example (frequency domain interface): <formatConverterCmdl -if in -of out -inConf 3 -outConf 2 -dmx 3 -dmxMtx dmxMtx.txt -info 2> \n");
        fprintf(stderr, "Example (dmx matrix output):          <formatConverterCmdl -cicpIn 13 -cicpOut 6 -dmxMtxOutput downmixMtxOut.txt\n");
        fprintf(stderr, "Example (mix matrix output):          <formatConverterCmdl -cicpIn 10 -cicpOut 2 -mixMtxOutput mixMtxOut.txt -dmxMtx dmxMtx.txt\n\n");
        
        fprintf(stderr, "-if       <input.wav>          Time domain input.\n");
        fprintf(stderr, "-if       <input>              64-band QMF domain input.\n");
        fprintf(stderr, "                                  Filename will be extended to <input>_real.qmf and <input>_imag.qmf.\n");
        fprintf(stderr, "-of       <output.wav>         Time domain output.\n"); 
        fprintf(stderr, "-of       <output>             64-band QMF domain output.\n");
        fprintf(stderr, "                                  Filename will be extended to <output>_real.qmf and <output>_imag.qmf.\n");
        fprintf(stderr, "-of       <output.txt>         dmx/mix matrix output (usage only in combination with -dmxMtx/-mixMtx option).\n");
        fprintf(stderr, "                                  Note: Mixed  time/frequency domain input/output will be rejected.\n");
        fprintf(stderr, "-cicpIn   <inputConfigIdx>     input config idx\n\
                                ( -1: LIST_OF_CHANNELS, 0: GENERIC, >0: CICP_index )\n");
        fprintf(stderr, "-cicpOut  <outputConfigIdx>    output config idx\n\
                                ( -1: LIST_OF_CHANNELS, 0: GENERIC, >0: CICP_index )\n");
        fprintf(stderr, "LIST_OF_CHANNELS signaling of setups will read from file (use -inGeo <inconf.txt> and/or -outGeo <outconf.txt>).\n");
        fprintf(stderr, "GENERIC inConf/outConf require an externally provided DMX matrix and number of input/ouput channels via -inNch/-outNch switches.\n");
        fprintf(stderr, "-dmx      <dmxType>            dmx type idx (passive TD = 0 (default), passive FD = 1, active-PA FD (QMF) = 3, custom FD (QMF) = 4 (needs -aes and -pas switches),\n");
        fprintf(stderr, "                                  active FD (STFT) = 5, custom FD (STFT) = 6 (needs -aes).\n");
        fprintf(stderr, "-aes      <0, 7>               Adaptive EQ Strength: Value between 0 (passive) and 7 (active).\n");
        fprintf(stderr, "-pas      <0, 7>               Phase Align Strength: Value between 0 (passive) and 7 (active).\n");
        fprintf(stderr, "-d                             automatically compensate algorithmic format converter delay (not possible for QMF domain input/output).\n");
        fprintf(stderr, "-dmxMtx   <dmxMtx.txt>         input external dmx matrix as text file (format: [inChans x outChans], linear gain).\n");
        fprintf(stderr, "-dmxMtxOutput <dmxMtxOut.txt>  output dmx matrix as text file (no audio processing, format: [inChans x outChans]).\n");
        fprintf(stderr, "-extEq    <eqParams.txt>       input external EQ parameters as text file.\n");
        fprintf(stderr, "-mixMtxOutput <mixMtxOut.txt>  output mix matrix as text file (no audio processing, format: [inChans x inChans]).\n");
        fprintf(stderr, "-angleDev <angles.txt>         input angle deviations of output setup as text file.\n");
        fprintf(stderr, "                                 format: [outChans x 2], col.1: azimuth deviation in [deg], col.2: elevation deviation in [deg].\n");
        fprintf(stderr, "-dist     <distances.txt>      input loudspeaker distances of output setup as text file.\n");
        fprintf(stderr, "                                 format: [outChans x 1], distance in [m].\n");
        fprintf(stderr, "                                 Note: trim_parameters.txt will be created containing gain[lin],delay[samples] per channel.\n");
        fprintf(stderr, "-bitdepth                      specify bit depth of output file (otherwise input bitdepth is used)\n");
        fprintf(stderr, "-info     <info>               info type (0: nearly silent, 1: print minimal information, 2: print all information).\n");                        
        return -1;
    }
    
    /**************************************************************************************************************************************************************************************/
    /* ----------------- OPEN FILES -----------------*/
    /**************************************************************************************************************************************************************************************/
    
#ifdef __APPLE__
#pragma mark OPEN_FILES
#endif

    /* Open input file/-s */
    if ( !(mixMtxOutputFlag || dmxMtxOutputFlag) ) {        
        
        if ( qmfIOFlag ) {
            if (inFilename) {

                pQmfInRealFile  = fopen(qmfInFilenameReal, "rb");
                pQmfInImagFile  = fopen(qmfInFilenameImag, "rb");
            }
            if ( pQmfInRealFile == NULL || pQmfInImagFile == NULL ) {
                fprintf(stderr, "Could not open input files: (%s and/or %s).\n", qmfInFilenameReal, qmfInFilenameImag );
                return -1;
            } else {
                fprintf(stderr, "Found real input file: %s.\n", qmfInFilenameReal );
                fprintf(stderr, "Found imaginary input file: %s.\n", qmfInFilenameImag );
            }
        }
        else {
            if (inFilename) {
                pInFile  = fopen(inFilename, "rb");
            }
            if ( pInFile == NULL ) {
                fprintf(stderr, "Could not open input file: %s.\n", inFilename );
                return -1;
            } else {
                fprintf(stderr, "Found input file: %s.\n", inFilename );
            }
        }
        
        /* Open output file/-s */
        if ( qmfIOFlag ) {
            if (outFilename) {
                pQmfOutRealFile = fopen(qmfOutFilenameReal, "wb");
                pQmfOutImagFile = fopen(qmfOutFilenameImag, "wb");
            }
            if ( pQmfOutRealFile == NULL || pQmfOutImagFile == NULL ) {
                fprintf(stderr, "Could not open output files: (%s and/or %s).\n", qmfOutFilenameReal, qmfOutFilenameImag );
                return -1;
            } else {
                fprintf(stderr, "Found real output file: %s.\n", qmfOutFilenameReal );
                fprintf(stderr, "Found imaginary output file: %s.\n", qmfOutFilenameImag );
            }        
        }
        else {
            if (outFilename) {
                pOutFile = fopen(outFilename, "wb");
            }
            if ( pOutFile == NULL ) {
                fprintf(stderr, "Could not open output file: %s.\n", outFilename );
                return -1;
            } else {
                fprintf(stderr, "Found output file: %s.\n", outFilename );
            }
        }
        
    } else {
        fprintf(stderr,"INFO: mixMtx and/or dmxMtx will be written. Will not process audio signals.\n");
    }
    
    /**************************************************************************************************************************************************************************************/
    /* ----------------- OPEN FORMAT CONVERTER -----------------*/
    /**************************************************************************************************************************************************************************************/

#ifdef __APPLE__
#pragma mark OPEN_FORMAT_CONVERTER
#endif
    

    /* converter type */
    switch (dmxTypeIdx) {
        case 0:
            formatConverterMode = FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN;
            /* qmf IO */
            if (qmfIOFlag) {
                fprintf(stderr,"error: dmxTypeIdx invalid for qmf domain IO mode!!!");
                return -1;
            }
            break;
        case 1:
            formatConverterMode = FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN;
            break;
        case 3:
            formatConverterMode = FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN;
            break;
        case 4:
            formatConverterMode = FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN;
            if ( phaseAlignStrength == -1 || adaptiveEQStrength == -1 )
            {
              fprintf(stderr, "Error: Please set both switches (-pas and -aes) when using custom downmix mode (-dmx 4).\n");
              return -1;
            }
            break;
        case 5:
            formatConverterMode = FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT;
            break;
        case 6:
            formatConverterMode = FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT;
            if (adaptiveEQStrength == -1 )
            {
              fprintf(stderr, "Error: Please set -aes switch when using custom downmix mode (-dmx 6).\n");
              return -1;
            }
            break;
        default:
            fprintf(stderr,"error: dmxTypeIdx invalid!!!");
            return -1;
            break;
    }
    

    /* input format */
    switch (inConfIdx) {
        case 0:
            formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_GENERIC;
            break;
        default:
            formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_LISTOFCHANNELS;
            break;
    }
    
    /* output format */
    switch (outConfIdx) {
        case 0:
            formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC;
            break;
        default:
            formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_LISTOFCHANNELS;
            break;
    }

    /* sanity check for generic input/output formats */
    if( formatConverterInputFormat == FORMAT_CONVERTER_INPUT_FORMAT_GENERIC || formatConverterOutputFormat == FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC ) {
        if( Nin_int == 0 || Nout_int == 0 ) {
            fprintf(stderr,"error: number of in/out channels missing for generic in/out format\n");
            return -1;
        }
    }

    switch(formatConverterMode)
    {
    case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
    case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
    case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
        {
            formatConverterFrameSize = 64;

            switch (hybMode) {
            case 0:
                formatConverterNumFreqBands = 71; 
                formatConverterCenterFrequencies = f_bands_nrm_71;
                break;
            case 2:
                formatConverterNumFreqBands = 77;
                formatConverterCenterFrequencies = f_bands_nrm_77;
                break;
            case -1:
                formatConverterNumFreqBands = 64;
                formatConverterCenterFrequencies = f_bands_nrm_64;
                break;
            case 1:
            default:
                fprintf(stderr,"error hybMode invalid!!!");
                return 1;
                break;
            }
            break;
        }
    case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
    case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
        {
            formatConverterFrameSize            = 1024;
            stftFormatConverterFrameSize        = 256; 
            formatConverterNumFreqBands            = stftFormatConverterFrameSize+1;
            stftFormatConverterFFTLength        = stftFormatConverterFrameSize*2;
            stftFormatConverterNumTimeSlots        = formatConverterFrameSize/stftFormatConverterFrameSize;
            stftFormatConverterNumErbBands        = 58;
            stftFormatConverterErbFreqIdx        = erb_freq_idx_256_58;
            formatConverterCenterFrequencies    = f_bands_nrm_stft_erb_58;
            stftWindow                            = stftWindow_512;

            break;
        }
    case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
        {
            formatConverterFrameSize = 1024;   
            break;
        }
    default:
        fprintf(stderr,"Error: unhandled formatConverterMode.");
        return -1;
    }

    frameSize = formatConverterFrameSize;      

    
/*******************************************************************/
/*    read output channel config (if not generic output setup)     */
/*******************************************************************/
    if((outConfIdx==-1) || (outConfIdx>0)){
        int numChannels, numLfes;
        api_chid_t *channel_vec = NULL;
        outputChConfig = (CICP2GEOMETRY_CHANNEL_GEOMETRY*)calloc(32,sizeof(CICP2GEOMETRY_CHANNEL_GEOMETRY));
 
        if(outConfIdx==-1){ /* setup from text file */
            if(inFilenameOutputChannelList==NULL){
                fprintf(stderr,"Error: Missing filename for output setup geometry file. Use -outGeo <file.txt> switch.\n");
                return -1;
            }
            errorFlag = cicp2geometry_get_geometry_from_file(inFilenameOutputChannelList, outputChConfig, &numChannels, &numLfes);
        } else {/* setup from CICP setup index */
            errorFlag = cicp2geometry_get_geometry_from_cicp(outConfIdx, outputChConfig, &numChannels, &numLfes);
        }
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during getting output setup geometry.\n");
            return -1;
        } 
        numChannels += numLfes;
        Nout_int = numChannels;

        /* match known channels in channel config */
        errorFlag = formatConverterMatchChConfig2Channels(outputChConfig,1, \
            numChannels,numLfes,1,&channel_vec,&numUnknownCh,&unknownCh_vec, \
            &azimuthDeviationFromMatching,&elevationDeviationFromMatching);
        if ( 0 != errorFlag ) 
        {
            fprintf(stderr, "Error during matching known channels in channel config.\n");
            return -1;
        }    
        if (numUnknownCh > 0){
            fprintf(stderr, "Unknwon channel(s) in output config. Obtaining gains from generalized VBAP.\n");
            formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_GENERIC;
            formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC;
            outChannelVbapFlag = 1;
        }
        /* apply channel config as output setup in format converter and clean up */
        if(formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC)
            formatConverterSetInOutFormat(1, Nout_int, channel_vec);
        free(channel_vec);free(unknownCh_vec);
    }

/*******************************************************************/
/*     read input channel config (if not generic input setup)      */
/*******************************************************************/
    if((inConfIdx==-1) || (inConfIdx>0)){
        int numChannels, numLfes;
        api_chid_t *channel_vec = NULL;
        float *azOffset_vec_dummy = NULL;
        float *elOffset_vec_dummy = NULL;
        inputChConfig = (CICP2GEOMETRY_CHANNEL_GEOMETRY*)calloc(32,sizeof(CICP2GEOMETRY_CHANNEL_GEOMETRY));

        if(inConfIdx==-1){ /* setup from text file */
            if(inFilenameInputChannelList==NULL){
                fprintf(stderr,"Error: Missing filename for input setup geometry file. Use -inGeo <file.txt> switch.\n");
                return -1;
            }
            errorFlag = cicp2geometry_get_geometry_from_file(inFilenameInputChannelList, inputChConfig, &numChannels, &numLfes);
        } else {/* setup from CICP setup index */
            errorFlag = cicp2geometry_get_geometry_from_cicp( inConfIdx, inputChConfig, &numChannels, &numLfes);
        }
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during getting input setup geometry.\n");
            return -1;
        }    
        numChannels += numLfes;
        Nin_int = numChannels;

        /* only try to match & apply input setup if we are not using generic setups */
        if(formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC) {
            /* match known channels in channel config */
            errorFlag = formatConverterMatchChConfig2Channels(inputChConfig,0, \
                numChannels,numLfes,1,&channel_vec,&numUnknownCh,&unknownCh_vec, \
                &azOffset_vec_dummy,&elOffset_vec_dummy);
            free(azOffset_vec_dummy); /* we don't care about offsets in the input configuration */
            free(elOffset_vec_dummy); /* we don't care about offsets in the input configuration */
            if ( 0 != errorFlag )
            {
            fprintf(stderr, "Error during matching known channels in channel config.\n");
                return -1;
            }    
            if ((numUnknownCh > 0) && (formatConverterOutputFormat != FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC)){
                fprintf(stderr, "Unknown channel(s) in input config. Obtaining gains from generalized VBAP.\n");
                inChannelVbapFlag = 1;
            }
            /* apply channel config as input setup in format converter and clean up */
            formatConverterSetInOutFormat(0, Nin_int, channel_vec);   
            if(!inChannelVbapFlag){
                free(unknownCh_vec);
                unknownCh_vec = NULL;
            }
            free(channel_vec);
        }
    }


/*******************************************************************/
/*                         INIT WAV IO                             */
/*******************************************************************/
    if((!dmxMtxOutputFlag) && (!mixMtxOutputFlag)) {

#ifdef __APPLE__
#pragma mark WAVIO_INIT 
#endif
  
    /* wavIO_init */
    if ( qmfIOFlag ) {
        if (frameSize != numQMFBands ) {
            fprintf(stderr, "Error: frameSize != numQMFBands.\n");
            return -1;
        }
        
        errorFlag = wavIO_init(&hWavIO_real,
                                frameSize,
                                0,
                                0);        
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_init real.\n");
            return -1;
        }
        
        errorFlag = wavIO_init(&hWavIO_imag,
                                frameSize,
                                0,
                                0);        
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_init imag.\n");
            return -1;
        }
    } else {        
        errorFlag = wavIO_init(&hWavIO,
                                frameSize,
                                bFillLastFrame,
                                numSamplesToCompensate);    
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_init.\n");
            return -1;
        }
    }
    
/*******************************************************************/
/*                      WAV IO openRead()                          */
/*******************************************************************/
    /* wavIO_openRead */
    if ( qmfIOFlag ) {
        errorFlag = wavIO_openRead(hWavIO_real,
                                    pQmfInRealFile,
                                    &nInChannels,
                                    &inSampleRate, 
                                    &bytedepth_input,
                                    &nTotalSamplesPerChannel_real, 
                                    &nSamplesPerChannelFilled_real);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openRead real.\n");
            return -1;
        }
        
        errorFlag = wavIO_openRead(hWavIO_imag,
                                    pQmfInImagFile,
                                    &nInChannels,
                                    &inSampleRate, 
                                    &bytedepth_input,
                                    &nTotalSamplesPerChannel_imag, 
                                    &nSamplesPerChannelFilled_imag);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openRead imag.\n");
            return -1;
        }        
    } else {   
        errorFlag = wavIO_openRead(hWavIO,
                                    pInFile,
                                    &nInChannels,
                                    &inSampleRate, 
                                    &bytedepth_input,
                                    &nTotalSamplesPerChannel, 
                                    &nSamplesPerChannelFilled);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openRead.\n");
            return -1;
        }
    }    
    
    }/* !dmxMtxOutputFlag, !mixMtxOutputFlag*/
    samplingRate = inSampleRate;
    outSampleRate = inSampleRate;


/*******************************************************************/
/*                   Format Converter open()                       */
/*******************************************************************/
    fprintf(stderr,"\nOpen Format Converter ... \n");

    /* open format converter */
    if ( formatConverterOpen(   formatConverterMode, 
                                formatConverterInputFormat,
                                formatConverterOutputFormat,
                                samplingRate,
                                formatConverterFrameSize,
                                formatConverterNumFreqBands,
                                stftFormatConverterNumErbBands,
                                stftFormatConverterErbFreqIdx,
                                stftFormatConverterFFTLength,
                                &Nin_int,
                                &Nout_int,
                                &formatConverterDelay,
                                &hFormatConverter_params, 
                                &hFormatConverter_state) != 0 )        
    {
        fprintf(stderr,"Error in Opening Format Converter. Exiting application. \n");
        return 1;
    }    
    formatConverterNumInputChans = (unsigned int)Nin_int;
    formatConverterNumOutputChans = (unsigned int)Nout_int;

    /* alloc memory */
    azimuthDeviation   = (float*)calloc(formatConverterNumOutputChans,sizeof(float));        
    elevationDeviation = (float*)calloc(formatConverterNumOutputChans,sizeof(float));        
    azimuthDeviationExt   = (float*)calloc(formatConverterNumOutputChans,sizeof(float));        
    elevationDeviationExt = (float*)calloc(formatConverterNumOutputChans,sizeof(float));        
    distance           = (float*)calloc(formatConverterNumOutputChans,sizeof(float));        
    dmxMtx = (float**)calloc(formatConverterNumInputChans,sizeof(float*));        
    mixMtx = (int**)calloc(formatConverterNumInputChans,sizeof(int*));           
    if (distanceFlag == 1) {
        trimDelays  = (int*)calloc(formatConverterNumOutputChans,sizeof(int));
        trimGains  = (float*)calloc(formatConverterNumOutputChans,sizeof(float));
    }
    for (i=0; i < formatConverterNumInputChans; i++)
    {
        dmxMtx[i] = (float*)calloc(formatConverterNumOutputChans,sizeof(float));
        mixMtx[i] = (int*)calloc(formatConverterNumInputChans,sizeof(int));
    }



    /* angle deviation */
    if (angleDeviationFlag == 1) {

        errorFlag = readAngleDeviationsFromFile( inFilenameAngleDeviation, azimuthDeviationExt, elevationDeviationExt, formatConverterNumOutputChans );
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during readAngleDeviationsFromFile.\n");
            return -1;
        } 
        if(azimuthDeviationFromMatching != NULL){ /* add deviations */
            for(i=0;i<formatConverterNumOutputChans;i++){
                azimuthDeviation[i]  = azimuthDeviationFromMatching[i] + azimuthDeviationExt[i];
                elevationDeviation[i] = elevationDeviationFromMatching[i] + elevationDeviationExt[i];
            }
        }
        formatConverterAzimuthDeviation   = azimuthDeviation;
        formatConverterElevationDeviation = elevationDeviation;
    } else {
        formatConverterAzimuthDeviation   = azimuthDeviationFromMatching;
        formatConverterElevationDeviation = elevationDeviationFromMatching;   
    }

    /* distances */
    if (distanceFlag == 1) {
        
        errorFlag = readDistancesFromFile( inFilenameDistance, distance, formatConverterNumOutputChans );
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during readDistancesFromFile.\n");
            return -1;
        }        
        formatConverterDistance = distance;
    } 
   
    /* init format converter */
    errorFlag = formatConverterInit(
                             formatConverterCenterFrequencies,
                             formatConverterAzimuthDeviation,
                             formatConverterElevationDeviation,
                             formatConverterDistance,
                             hFormatConverter_params,
                             hFormatConverter_state );
    if ( errorFlag != 0 )
    {
        if(errorFlag==-2){
            fprintf(stderr,"Initializing of Format Converter returned \"missing ");
            fprintf(stderr,"rule\". Falling back to gVBAP gains.\n");
            formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_GENERIC;
            formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC;
            outChannelVbapFlag = 1;

            /* close, open, initialize for generic input/output setups: */
            if ( formatConverterClose ( &hFormatConverter_params, &hFormatConverter_state ) != 0)   
            {
                fprintf(stderr,"Error in formatConverterClose. Exiting application. \n");
                return 1;
            }
            if ( formatConverterOpen(   formatConverterMode, 
                                formatConverterInputFormat,
                                formatConverterOutputFormat,
                                samplingRate,
                                formatConverterFrameSize,
                                formatConverterNumFreqBands,
                                stftFormatConverterNumErbBands,
                                stftFormatConverterErbFreqIdx,
                                stftFormatConverterFFTLength,
                                &Nin_int,
                                &Nout_int,
                                &formatConverterDelay,
                                &hFormatConverter_params, 
                                &hFormatConverter_state) != 0 )
            {
                fprintf(stderr,"Error in Opening Format Converter. Exiting application. \n");
                return 1;
            }    
            formatConverterNumInputChans = (unsigned int)Nin_int;
            formatConverterNumOutputChans = (unsigned int)Nout_int;

            errorFlag = formatConverterInit(
                             formatConverterCenterFrequencies,
                             formatConverterAzimuthDeviation,
                             formatConverterElevationDeviation,
                             formatConverterDistance,
                             hFormatConverter_params,
                             hFormatConverter_state );
            if(errorFlag!=0){
                fprintf(stderr,"Error initializing the format converter. Terminating.\n");
                return 1;
            }               

        }
        else if(errorFlag==-8){
            fprintf(stderr,"Initializing of Format Converter returned \"loudspeaker ");
            fprintf(stderr,"offsets out of bounds\". Falling back to gVBAP gains.\n");
            /* treat as unknown output setup, apply full VBAP fallback */
            formatConverterInputFormat = FORMAT_CONVERTER_INPUT_FORMAT_GENERIC;
            formatConverterOutputFormat = FORMAT_CONVERTER_OUTPUT_FORMAT_GENERIC;
            outChannelVbapFlag = 1;

            /* close, open, initialize for generic input/output setups: */
            if ( formatConverterClose ( &hFormatConverter_params, &hFormatConverter_state ) != 0)   
            {
                fprintf(stderr,"Error in formatConverterClose. Exiting application. \n");
                return 1;
            }
            if ( formatConverterOpen(   formatConverterMode, 
                                formatConverterInputFormat,
                                formatConverterOutputFormat,
                                samplingRate,
                                formatConverterFrameSize,
                                formatConverterNumFreqBands,
                                stftFormatConverterNumErbBands,
                                stftFormatConverterErbFreqIdx,
                                stftFormatConverterFFTLength,
                                &Nin_int,
                                &Nout_int,
                                &formatConverterDelay,
                                &hFormatConverter_params, 
                                &hFormatConverter_state) != 0 )        
            {
                fprintf(stderr,"Error in Opening Format Converter. Exiting application. \n");
                return 1;
            }    
            formatConverterNumInputChans = (unsigned int)Nin_int;
            formatConverterNumOutputChans = (unsigned int)Nout_int;

            errorFlag = formatConverterInit(
                             formatConverterCenterFrequencies,
                             formatConverterAzimuthDeviation,
                             formatConverterElevationDeviation,
                             formatConverterDistance,
                             hFormatConverter_params,
                             hFormatConverter_state );
            if(errorFlag!=0){
                fprintf(stderr,"Error initializing the format converter. Terminating.\n");
                return 1;
            }               
        }
        else{ /* unrecoverable error during init */
            fprintf(stderr,"Error in initializing Format Converter (err=%d). Exiting application. \n",errorFlag);
            return 1;
        }
    } /* formatConverter opened successfully. */

    if(dmxTypeIdx == 4 || dmxTypeIdx == 6)
    {
        switch(formatConverterMode)
        {
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
            setCustomDownmixParameters( hFormatConverter_state, adaptiveEQStrength, phaseAlignStrength);
            break;
        case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
            setCustomDownmixParameter_STFT( hFormatConverter_state, adaptiveEQStrength);
            break;
        default:;
        }
    }


    /* get trim paramters and write trim paramters to text file */
    if (distanceFlag == 1) {
        
        strcpy(outFilenameTrimParams, "trim_parameters.txt");
        pOutFileTrimParams = fopen(outFilenameTrimParams, "wb");
                
        /* get trim params */
        if (formatConverterGetTrimParams(trimGains, trimDelays, hFormatConverter_params) != 0)
        {
            fprintf(stderr,"Error trying to get trim parameters. Exiting application. \n");
            return -1;
        }  
       
        /* write file */
        fprintf(stderr,"\nWrite trim parameters (gain, delay: one line per channel) to %s... \n", outFilenameTrimParams);    
        for( i =0 ; i < formatConverterNumOutputChans; i++)
        {
            fprintf(pOutFileTrimParams,"%.10f, ", trimGains[i]);
            fprintf(pOutFileTrimParams,"%d \n", trimDelays[i]);
        }    
        fclose(pOutFileTrimParams); 
                
        /* free memory */
        free(trimDelays); trimDelays = NULL;
        free(trimGains); trimGains = NULL;
    }

    /* output channel gVBAP fallback */
    if (outChannelVbapFlag) {                  

        if(angleDeviationFlag == 1){ /* apply external offsets */
            for (i=0; i < formatConverterNumOutputChans; i++){ 
                outputChConfig[i].Az += (int) azimuthDeviationExt[i];
                outputChConfig[i].El += (int) elevationDeviationExt[i];
            }
        }

        /* obtain gVBAP DMX gains: */
        gVBAPRenderer_GetStaticGains(inputChConfig, Nin_int, outConfIdx, outputChConfig, Nout_int, dmxMtx);

        /* direct mapping of LFE(s) to LFE(s) if LFE(s) are present in target setup */
        directMapLFEs(dmxMtx, Nin_int, inputChConfig, Nout_int, outputChConfig);

        /* post-process gains to reduce phantom sources */        
        formatConverterPostprocessDmxMtx(dmxMtx,formatConverterNumInputChans,formatConverterNumOutputChans);

        fprintf(stderr, "\nApplying gVBAP DMX matrix ... changing MIX matrix ... \n");       
        errorFlag = formatConverterSetDmxMtx((const float **)dmxMtx, hFormatConverter_params, hFormatConverter_state);   
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterSetDmxMtx.\n");
            return -1;
        }
        errorFlag = formatConverterSetDmxMtx_STFT(hFormatConverter_params,  hFormatConverter_state);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterSetDmxMtx_STFT.\n");
            return -1;
        }
    }

    /* input channel gVBAP fallback -> add VBAP gains matrix to internally generated one */
    if (inChannelVbapFlag && (!outChannelVbapFlag)) {
        CICP2GEOMETRY_CHANNEL_GEOMETRY* unknownChannelsGeo = NULL;
        float **tmpDmxMtx = NULL;
        
        if(angleDeviationFlag == 1){ /* apply external offsets */
            for (i=0; i < formatConverterNumOutputChans; i++){ 
                outputChConfig[i].Az += (int) azimuthDeviationExt[i];
                outputChConfig[i].El += (int) elevationDeviationExt[i];
            }
        }

        /* allocate temporary memory */
        unknownChannelsGeo = (CICP2GEOMETRY_CHANNEL_GEOMETRY*)calloc(numUnknownCh,sizeof(CICP2GEOMETRY_CHANNEL_GEOMETRY));
        tmpDmxMtx = (float**)calloc(numUnknownCh,sizeof(float*));        
        for (i=0; i < (unsigned int) numUnknownCh; i++)
            tmpDmxMtx[i] = (float*)calloc(formatConverterNumOutputChans,sizeof(float));

        fprintf(stderr,"Obtaining DMX gains for %d unknown input channel(s)...\n",numUnknownCh);
        for(i=0; i<numUnknownCh; i++){
            unknownChannelsGeo[i].Az  = inputChConfig[unknownCh_vec[i]].Az;
            unknownChannelsGeo[i].El  = inputChConfig[unknownCh_vec[i]].El;
            unknownChannelsGeo[i].LFE = inputChConfig[unknownCh_vec[i]].LFE;
            fprintf(stderr,"%d[of %d]: Az=%d, El=%d, isLFE=%d\n",i,numUnknownCh, \
                unknownChannelsGeo[i].Az,unknownChannelsGeo[i].El,unknownChannelsGeo[i].LFE);
        }
        /* obtain gVBAP DMX gains: */
        gVBAPRenderer_GetStaticGains(unknownChannelsGeo, numUnknownCh, outConfIdx, outputChConfig, Nout_int, tmpDmxMtx);

        /* direct mapping of LFE(s) to LFE(s) if LFE(s) are present in target setup */
        directMapLFEs(tmpDmxMtx, numUnknownCh, unknownChannelsGeo, Nout_int, outputChConfig);

        /* copy gains for unknown channels into full-size DMX matrix */
        for(i=0;i<numUnknownCh;i++)
            for(j=0;j<formatConverterNumOutputChans;j++)
                dmxMtx[unknownCh_vec[i]][j]= tmpDmxMtx[i][j];
        /* post-process gains to reduce phantom sources */        
        formatConverterPostprocessDmxMtx(dmxMtx,formatConverterNumInputChans,formatConverterNumOutputChans);

        /* free memory */        
        if (unknownChannelsGeo != NULL) {
            free(unknownChannelsGeo);
            unknownChannelsGeo = NULL;
        }
        if (unknownCh_vec != NULL) {
            free(unknownCh_vec);
            unknownCh_vec = NULL;
        }
        if (tmpDmxMtx != NULL){
            for (i=0; i < numUnknownCh; i++)
                free(tmpDmxMtx[i]);                
            free(tmpDmxMtx);
            tmpDmxMtx = NULL;
        }

        fprintf(stderr, "\nAdding gVBAP DMX matrix ... changing MIX matrix ... \n");       
        errorFlag = formatConverterAddDmxMtx((const float **)dmxMtx, hFormatConverter_params, hFormatConverter_state);   
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterAddDmxMtx.\n");
            return -1;
        }
    }

    /* free offset angles memory. */
    if (azimuthDeviationFromMatching != NULL) {
        free(azimuthDeviationFromMatching);
        azimuthDeviationFromMatching = NULL;
    }
    if (elevationDeviationFromMatching != NULL) {
        free(elevationDeviationFromMatching);
        elevationDeviationFromMatching = NULL;
    }
    
    /* free in/output configs that are not needed any more. */
    if(outputChConfig != NULL){
        free(outputChConfig);
        outputChConfig = NULL;
    }
    if(inputChConfig != NULL){
        free(inputChConfig);
        inputChConfig = NULL;
    }

    /* external DMX matrix */
    if (dmxMtxInputFlag) {
        
        errorFlag = readDmxMtxFromFile( inFilenameDmxMtx, dmxMtx, formatConverterNumInputChans, formatConverterNumOutputChans );
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during readDmxMtxFromFile.\n");
            return -1;
        }
       
        fprintf(stderr, "\nApplying external DMX matrix ... changing MIX matrix ... \n");
        
        errorFlag = formatConverterSetDmxMtx((const float **)dmxMtx, hFormatConverter_params, hFormatConverter_state);   
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterSetDmxMtx.\n");
            return -1;
        }
        errorFlag = formatConverterSetDmxMtx_STFT(hFormatConverter_params,  hFormatConverter_state);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterSetDmxMtx_STFT.\n");
            return -1;
        }
    }

    /* external EQ parameters */
    if (extEqFlag == 1) {
        int si = 0;
        int numEQs = 0;
        eqParamsStruct *eqParams  = NULL;
        int *eqMap = NULL;

        errorFlag = readEqParamsFromFile( inFilenameEqParams, formatConverterNumInputChans, &numEQs, &eqParams, &eqMap );
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during readEqParamsFromFile.\n");
            return -1;
        }        

        fprintf(stderr, "\nApplying external EQ parameters...\n");     
        errorFlag = formatConverterSetEQs( numEQs, eqParams, eqMap, formatConverterCenterFrequencies, (float) samplingRate, hFormatConverter_params, hFormatConverter_state);
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during formatConverterSetEQs.\n");
            return -1;
        }

        /* free eqParams,eqMap */
        for (si = 0; si < numEQs; si++) {
          if (eqParams[si].pkFilterParams != NULL) {
            free(eqParams[si].pkFilterParams);
            eqParams[si].pkFilterParams = NULL;
          }
        }
        if(eqParams != NULL){
            free(eqParams);
            eqParams = NULL;
        }
        if(eqMap != NULL)
        {
            free(eqMap);
            eqMap = NULL;
        }
    }

    /**************************************************************************/
    /* ----------------- DELAY -----------------*/
    /**************************************************************************/    
#ifdef __APPLE__
#pragma mark DELAY
#endif
    
    /* module delay */      
    if(formatConverterMode != FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
    && formatConverterMode != FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT)
    {
        if (ldMode) {
            switch (hybMode) {
            case 0:
            case 1:
            case 2:
                qmfDelay = 640;
                break;
            case -1:
            default:
                qmfDelay = 256;
                break;
            }
            /* qmf IO */
            if (qmfIOFlag) {
                qmfDelay = qmfDelay - 256;
            }
        } else {
            switch (hybMode) {
            case 0:
            case 1:
            case 2:
                qmfDelay = 961;
                break;
            case -1:
            default:
                qmfDelay = 577;
                break;
            }
            /* qmf IO */
            if (qmfIOFlag) {
                qmfDelay = qmfDelay - 577;
            }
        }
    }

    if (formatConverterMode != FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN ) 
    {
        moduleDelay = formatConverterDelay+qmfDelay;
    } else {
        moduleDelay = formatConverterDelay;
    }
    
    if (plotInfo > 0) {
        
        /* display parameters */
        formatConverterDisplayParams( plotInfo-1, hFormatConverter_params );   

        fprintf(stderr,"Format Converter Lib  Info: %s \n",formatConverterGetInfo());  
        fprintf(stderr,"Format Converter Cmdl Info: %s \n",formatConverterCmdlGetInfo());      

        if ( !(mixMtxOutputFlag || dmxMtxOutputFlag) ) {        
            
            /* qmf IO */
            if (qmfIOFlag) {
                fprintf(stderr,"Format Converter Cmdl Info: QMF IO mode activated!\n");
                fprintf(stderr,"Format Converter Cmdl Info: Additional hybrid QMF Delay = %d samples\n",qmfDelay);
            } else {
                if (formatConverterMode != FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN ) 
                {
                    fprintf(stderr,"Format Converter Cmdl Info: QMF Delay = %d samples\n",qmfDelay);
                }
            }
            
            fprintf(stderr,"Format Converter Cmdl Info: Format Converter Delay: Netto = %d samples, Brutto = %d samples\n",formatConverterDelay,moduleDelay);        
            
            if (compensateDelayFlag && qmfIOFlag) {
                fprintf(stderr,"Format Converter Cmdl Info: Module delay of %d samples will NOT be compensated in QMF IO mode. Has to be compensated after inverse QMF. \n\n",moduleDelay);
            }           
            
        }        
    }
    
    if (!compensateDelayFlag || qmfIOFlag) {
        numSamplesToCompensate = 0;
    } else {
        numSamplesToCompensate = -moduleDelay;
    }     
    if ( (!qmfIOFlag) && ( !(mixMtxOutputFlag || dmxMtxOutputFlag) ) ) {
        wavIO_setDelay(hWavIO, numSamplesToCompensate);
    }

    /**************************************************************************/
    /*            DMX/MIX MATRIX output => no audio processing!               */
    /**************************************************************************/    
#ifdef __APPLE__
#pragma mark DMX_MIX_MATRIX 
#endif
    
    if (dmxMtxOutputFlag) {
        
        /* open file */
        pOutFileDmxMtx = fopen(outFilenameDmxMtx, "wb");
        
        if (pOutFileDmxMtx == NULL)
        {
            fprintf(stderr,"Error opening downmix matrix output file. Exiting application. \n");
                return 1;
        }

        /* get mix matrix */
        formatConverterGetDmxMtx(dmxMtx, hFormatConverter_params);
        
        /* write file */
        for( i =0 ; i < formatConverterNumInputChans; i++)
        {
            for( j =0 ; j < formatConverterNumOutputChans; j++)
            {
                fprintf(pOutFileDmxMtx,"%2.10f ", dmxMtx[i][j]);
            }
            fprintf(pOutFileDmxMtx,"\n");
        }    
        fprintf(stderr,"\nWrite DMX Matrix to file ... \n");
        fclose(pOutFileDmxMtx); 
        
        if(!mixMtxOutputFlag){ /* else: program will be closed after mix matrix has been written */
            /* free memory */
            for (i=0; i < formatConverterNumInputChans; i++) {
                free(dmxMtx[i]);
                free(mixMtx[i]);
            }
            free(dmxMtx);
            free(mixMtx);        
            free(azimuthDeviation);
            free(elevationDeviation);
            free(distance);
            
            /* close format converter */
            fprintf(stderr,"\nClose Format Converter ... ");
            if ( formatConverterClose ( &hFormatConverter_params, &hFormatConverter_state ) != 0)   
            {
                fprintf(stderr,"Error in formatConverterClose. Exiting application. \n");
                return 1;
            }
            fprintf(stderr," finished!\n");
            fprintf(stderr,"\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
          
            return 0;
        }
    }
    
    if (mixMtxOutputFlag) {
        
        /* open file */
        pOutFileMixMtx = fopen(outFilenameMixMtx, "wb");
        
        /* get mix matrix */
        formatConverterGetMixMtx(mixMtx, hFormatConverter_params);
        
        /* write file */
        for( i =0 ; i < formatConverterNumInputChans; i++)
        {
            for( j =0 ; j < formatConverterNumInputChans; j++)
            {
                fprintf(pOutFileMixMtx,"%d ", mixMtx[i][j]);
            }
            fprintf(pOutFileMixMtx,"\n");
        }    
        fprintf(stderr,"\nWrite MIX Matrix to file ... \n");
        fclose(pOutFileMixMtx); 
        
        /* free memory */
        for (i=0; i < formatConverterNumInputChans; i++) {
            free(dmxMtx[i]);
            free(mixMtx[i]);
        }
        free(dmxMtx);
        free(mixMtx);
        free(azimuthDeviation);
        free(elevationDeviation);
        free(distance);
        
        /* close format converter */
        fprintf(stderr,"\nClose Format Converter ... ");
        if ( formatConverterClose ( &hFormatConverter_params, &hFormatConverter_state ) != 0)   
        {
            fprintf(stderr,"Error in formatConverterClose. Exiting application. \n");
            return 1;
        }
        fprintf(stderr," finished!\n");
        fprintf(stderr,"\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
        
        return 0;
    }

/*******************************************************************/
/*                      WAV IO openWrite()                         */
/*******************************************************************/ 
    nOutChannels = formatConverterNumOutputChans;
    /* nOutChannels check */
    if ( nOutChannels == 0 ) {
        fprintf(stderr, "\nNo number of output channels specified! Using input format (%i channels).\n", nInChannels);
        nOutChannels = nInChannels;
    }
    
    /* bytedepth output */
    if (qmfIOFlag || bytedepth_output == 0) {
        bytedepth_output = bytedepth_input;
    }
    
    printf("\nFormatConverter Cmdl Info: bytedepth_input = %d, bytedepth_output = %d.\n", bytedepth_input,bytedepth_output);    
    
    /* wavIO_openWrite */
    if ( qmfIOFlag ) {
        errorFlag = wavIO_openWrite(hWavIO_real,
                                     pQmfOutRealFile,
                                     nOutChannels,
                                     outSampleRate,
                                     bytedepth_output);        
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openWrite real.\n");
            return -1;
        }     
        
        errorFlag = wavIO_openWrite(hWavIO_imag,
                                     pQmfOutImagFile,
                                     nOutChannels,
                                     outSampleRate,
                                     bytedepth_output);        
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openWrite imag.\n");
            return -1;
        }     
    } else {
        errorFlag = wavIO_openWrite(hWavIO,
                                     pOutFile,
                                     nOutChannels,
                                     outSampleRate,
                                     bytedepth_output);        
        if ( 0 != errorFlag )
        {
            fprintf(stderr, "Error during wavIO_openWrite.\n");
            return -1;
        }        
    }    
        
    if (formatConverterNumInputChans != nInChannels) {
        fprintf(stderr,"Error: formatConverterNumInputChans != nInChannels. \n");
        return 1; 
    }
    
    if (formatConverterNumOutputChans != nOutChannels) {
        fprintf(stderr,"Error: formatConverterNumOutputChans != nOutChannels. \n");
        return 1; 
    }
            
    /* alloc local buffers */
    if ( !qmfIOFlag ) {   
        inBuffer = (float**)calloc(nInChannels,sizeof(float*));
        outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
        
        for (i=0; i< (unsigned int) nInChannels; i++)
        {
            inBuffer[i] = (float*)calloc(frameSize,sizeof(float));
        }
        
        for (i=0; i< (unsigned int) nOutChannels; i++)
        {
            outBuffer[i] = (float*)calloc(frameSize,sizeof(float));
        }
    }
        
    /**************************************************************************/
    /* ----------------- INIT QMF -----------------*/
    /**************************************************************************/

#ifdef __APPLE__
#pragma mark INIT_QMF
#endif

    if (formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN 
        || formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN
        || formatConverterMode == FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN) 
    {

        /* alloc memory for input/output in frequency domain */

        /* input QMF */
        inputBufferFreqQMF_real = (float **) calloc( formatConverterNumInputChans, sizeof ( float* ) );    
        inputBufferFreqQMF_imag = (float **) calloc( formatConverterNumInputChans, sizeof ( float* ) );    
        for( i=0; i < formatConverterNumInputChans; i++ )
        {
            inputBufferFreqQMF_real[i] = ( float* ) calloc ( numQMFBands, sizeof ( float ) ); 
            inputBufferFreqQMF_imag[i] = ( float* ) calloc ( numQMFBands, sizeof ( float ) ); 
        }

        /* output QMF */
        outputBufferFreqQMF_real = (float **) calloc( formatConverterNumOutputChans, sizeof ( float* ) );    
        outputBufferFreqQMF_imag = (float **) calloc( formatConverterNumOutputChans, sizeof ( float* ) );    
        for( i=0; i < formatConverterNumOutputChans; i++ )
        {
            outputBufferFreqQMF_real[i] = ( float* ) calloc ( numQMFBands, sizeof ( float ) ); 
            outputBufferFreqQMF_imag[i] = ( float* ) calloc ( numQMFBands, sizeof ( float ) ); 
        }

        /* hybrid QMF */
        if (hybMode != -1) {            
            /* input full resolution */
            inputBufferFreq_real = (float **) calloc( formatConverterNumInputChans, sizeof ( float* ) );    
            inputBufferFreq_imag = (float **) calloc( formatConverterNumInputChans, sizeof ( float* ) );    
            for( i=0; i < formatConverterNumInputChans; i++ )
            {
                inputBufferFreq_real[i] = ( float* ) calloc ( formatConverterNumFreqBands, sizeof ( float ) ); 
                inputBufferFreq_imag[i] = ( float* ) calloc ( formatConverterNumFreqBands, sizeof ( float ) ); 
            }

            /* output full resolution */
            outputBufferFreq_real = (float **) calloc( formatConverterNumOutputChans, sizeof ( float* ) );    
            outputBufferFreq_imag = (float **) calloc( formatConverterNumOutputChans, sizeof ( float* ) );    
            for( i=0; i < formatConverterNumOutputChans; i++ )
            {
                outputBufferFreq_real[i] = ( float* ) calloc ( formatConverterNumFreqBands, sizeof ( float ) ); 
                outputBufferFreq_imag[i] = ( float* ) calloc ( formatConverterNumFreqBands, sizeof ( float ) ); 
            }            
        }     

        /****************/
        /* alloc memory */
        /****************/

        /* multi-channel QMF */
        pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK **) calloc( formatConverterNumInputChans, sizeof ( QMFLIB_POLYPHASE_ANA_FILTERBANK * ) );    
        pSynthesisQmf = (QMFLIB_POLYPHASE_SYN_FILTERBANK **) calloc( formatConverterNumOutputChans, sizeof ( QMFLIB_POLYPHASE_SYN_FILTERBANK * ) );    

        /* multi-channel hybrid QMF */
        hybFilterState = (QMFLIB_HYBRID_FILTER_STATE *) calloc( QMFLIB_MAX_CHANNELS, sizeof ( QMFLIB_HYBRID_FILTER_STATE ) );    

        /*****************/
        /* init analysis */
        /*****************/

        QMFlib_InitAnaFilterbank(numQMFBands, ldMode);
        for (i=0; i<formatConverterNumInputChans; i++) {

            /* QMF */
            QMFlib_OpenAnaFilterbank(&pAnalysisQmf[i]);



        }        

        for (i=0; i<QMFLIB_MAX_CHANNELS; i++) {    

            /* hybrid QMF */
            QMFlib_InitAnaHybFilterbank(&hybFilterState[i]);        

        }

        /*****************/
        /* init synthesis */
        /*****************/

        QMFlib_InitSynFilterbank(numQMFBands, ldMode);
        for (i=0; i<formatConverterNumOutputChans; i++) {    

            /* QMF */
            QMFlib_OpenSynFilterbank(&pSynthesisQmf[i]);         

        }

    }

    /**************************************************************************/
    /* ----------------- INIT STFT -----------------*/
    /**************************************************************************/
    if (formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
     || formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT )
    {

        stftWindowedSig = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        
        /*Initialize buffers for FFT*/
        stftInputBufferTimeImag     = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftOutputBufferTimeReal = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftOutputBufferTimeImag = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftInputBufferFreqReal  = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftInputBufferFreqImag  = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftOutputBufferFreqReal = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        stftOutputBufferFreqImag = (float*) calloc (stftFormatConverterFFTLength, sizeof (float));
        

        /*Initialize forward and backward FFT*/
        FFTlib_Create(&hFFT, stftFormatConverterFFTLength, 1);
        FFTlib_Create(&hIFFT, stftFormatConverterFFTLength, -1);

        inputBufferStft = (float **) calloc(formatConverterNumInputChans, sizeof (float*));            
        for(i=0; i<formatConverterNumInputChans; i++)
        {
            inputBufferStft[i] = (float *) calloc(stftFormatConverterFFTLength, sizeof (float) );    
        }

        outputBufferStft = (float **) calloc(formatConverterNumOutputChans, sizeof (float*));
        for(i=0;i<formatConverterNumOutputChans;i++)
        {
            outputBufferStft[i] = (float *) calloc(stftFormatConverterFFTLength, sizeof (float) );
        }

        stftTmpInputBufferTime = (float**) calloc(formatConverterNumInputChans, sizeof (float*));
        for(i=0;i<formatConverterNumInputChans;i++)
        {
            stftTmpInputBufferTime[i] = (float*) calloc(stftFormatConverterFrameSize, sizeof (float));
        }

        stftTmpOutputBufferTime = (float**) calloc(formatConverterNumOutputChans, sizeof (float*));
        for(i=0;i<formatConverterNumOutputChans;i++)
        {
            stftTmpOutputBufferTime[i] = (float*) calloc(stftFormatConverterFrameSize, sizeof (float));
        }

    }

    /**************************************************************************************************************************************************************************************/   
    /* ----------------- LOOP -----------------*/
    /**************************************************************************************************************************************************************************************/

#ifdef __APPLE__
#pragma mark PROCESS_LOOP
#endif
   
   {
    /* timing */
    clock_t startLap, stopLap, accuTime, accuTimeQMF, accuTimeSTFT;    
    double elapsed_time, elapsed_time_QMF, elapsed_time_STFT;
    float inputDuration;
    float ratio;
     
    /* aux stuff */
    unsigned int numSamplesStillToProcess = 0;
    unsigned int numFramesStillToProcess = 0;
    unsigned int frameSizeLastFrame = 0;
    accuTime = 0;
    accuTimeQMF = 0;
    accuTimeSTFT = 0;

    fprintf(stderr,"\nFormat Converter processing ... \n\n");

    do  /*loop over all frames*/
    {
        unsigned int samplesReadPerChannel = 0;
        unsigned int samplesToWritePerChannel = 0;
        unsigned int samplesWrittenPerChannel = 0;
        unsigned int samplesWrittenPerChannel_real = 0;
        unsigned int samplesWrittenPerChannel_imag = 0;
        unsigned int nZerosPaddedBeginning = 0;
        unsigned int nZerosPaddedEnd = 0;        
        frameNo++;
            
        /* qmf IO ??? */
        if (!qmfIOFlag) {
            
            /* read frame */
            if (!isLastFrameRead) {                    
                
                errorFlag = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrameRead,&nZerosPaddedBeginning,&nZerosPaddedEnd); 

                if (isLastFrameRead) {                     
                    numSamplesStillToProcess = samplesReadPerChannel + moduleDelay;
                    
                    if (numSamplesStillToProcess <= frameSize) {
                        frameSizeLastFrame = numSamplesStillToProcess;                      
                        numSamplesStillToProcess = 0;
                        numFramesStillToProcess = 0;
                        isLastFrameWrite = 1;
                    } else {
                        numSamplesStillToProcess = numSamplesStillToProcess-frameSize;
                        numFramesStillToProcess = (int) ceil(((double)numSamplesStillToProcess)/frameSize);
                        frameSizeLastFrame = numSamplesStillToProcess-(numFramesStillToProcess-1)*frameSize;                          
                    }               
                }
                
            } else {
                if (numFramesStillToProcess == 1) {
                    isLastFrameWrite = 1;
                    numFramesStillToProcess--;
                } else {
                    numFramesStillToProcess--;
                }
                
                /* reset input once */
                if (isLastFrameRead == 1) {
                    
                    isLastFrameRead = 2;
                    
                    for (i=0; i < (unsigned int) nInChannels; i++)
                    {
                        for (j=0; j < (unsigned int) frameSize; j++)
                        {
                            inBuffer[i][j] = 0.f;
                        }
                    }   
                }
            }          
            
        } else {                                    

            /* read frame */
            if (!isLastFrameRead) {   

              errorFlag = wavIO_readFrame(hWavIO_real,inputBufferFreqQMF_real,&samplesReadPerChannel,&isLastFrameRead,&nZerosPaddedBeginning,&nZerosPaddedEnd); 
              errorFlag = wavIO_readFrame(hWavIO_imag,inputBufferFreqQMF_imag,&samplesReadPerChannel,&isLastFrameRead,&nZerosPaddedBeginning,&nZerosPaddedEnd); 

              if (isLastFrameRead) {                     

                  numSamplesStillToProcess = samplesReadPerChannel + moduleDelay;
     
                  if (numSamplesStillToProcess <= frameSize) {
                      frameSizeLastFrame = numSamplesStillToProcess;                      
                      numSamplesStillToProcess = 0;
                      numFramesStillToProcess = 0;
                      isLastFrameWrite = 1;
                  } else {
                      numSamplesStillToProcess = numSamplesStillToProcess-frameSize;
                      numFramesStillToProcess = (int) ceil(((double)numSamplesStillToProcess)/frameSize);
                      frameSizeLastFrame = numSamplesStillToProcess-(numFramesStillToProcess-1)*frameSize;                          
                  }               
              }
            
            } else {

                if (numFramesStillToProcess == 1) {
                    isLastFrameWrite = 1;
                    numFramesStillToProcess--;
                } else {
                    numFramesStillToProcess--;
                }
                
                /* reset input once */
                if (isLastFrameRead == 1) {
                    
                    isLastFrameRead = 2;
                    
                    for (i=0; i < (unsigned int) nInChannels; i++)
                    {
                        for (j=0; j < (unsigned int) frameSize; j++)
                        {
                            inputBufferFreqQMF_real[i][j] = 0.f;
                            inputBufferFreqQMF_imag[i][j] = 0.f;
                        }
                    }   
                }
            } 
        }
        
        if (formatConverterMode != FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN ) 
        {
            switch (formatConverterMode) 
            {
            case FORMAT_CONVERTER_MODE_PASSIVE_TIME_DOMAIN:
            case FORMAT_CONVERTER_MODE_INVALID:
                fprintf(stderr,"Something weird happened. Exiting.\n");
                return -1;
                break;                  
            case FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN:
                fprintf(stderr,"Use custom frequency processing with PAS/AES parameters set to 0 for passive frequency domain processing. Exiting.\n");
                 return -1;
                 break;
            case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN:
            case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN:
                {
                    /* start lap timer */
                    startLap = clock();

                    for (i=0; i<formatConverterNumInputChans; i++) {

                        /* qmf IO */
                        if (!qmfIOFlag) {

                            /* QMF analysis */
                            QMFlib_CalculateAnaFilterbank(pAnalysisQmf[i],inBuffer[i],inputBufferFreqQMF_real[i],inputBufferFreqQMF_imag[i],ldMode);

                        }

                        /* hybrid QMF */
                        if (hybMode != -1) { 

                            QMFlib_ApplyAnaHybFilterbank(&hybFilterState[i], hybridMode, numQMFBands, alignDelay, 
                                inputBufferFreqQMF_real[i],inputBufferFreqQMF_imag[i],
                                inputBufferFreq_real[i],inputBufferFreq_imag[i]);

                        }                 
                    }

                    /* stop lap timer and accumulate */
                    stopLap = clock();
                    accuTimeQMF += stopLap - startLap;

                    /* start lap timer */
                    startLap = clock();

                    /* hybrid QMF */
                    if (hybMode != -1) { 

                        if ( formatConverterProcess_freqDomain( (const float**)inputBufferFreq_real, 
                            (const float**)inputBufferFreq_imag, 
                            outputBufferFreq_real, 
                            outputBufferFreq_imag, 
                            hFormatConverter_params, 
                            hFormatConverter_state) != 0 )
                        {
                            fprintf(stderr,"Error in formatConverterProcess_freqDomain. Exiting application. \n");
                            return 1;
                        }

                        /* QMF */
                    } else {

                        if ( formatConverterProcess_freqDomain( (const float**)inputBufferFreqQMF_real, 
                            (const float**)inputBufferFreqQMF_imag, 
                            outputBufferFreqQMF_real, 
                            outputBufferFreqQMF_imag, 
                            hFormatConverter_params, 
                            hFormatConverter_state) != 0 )
                        {
                            fprintf(stderr,"Error in formatConverterProcess_freqDomain. Exiting application. \n");
                            return 1;
                        }
                    }      

                    /* stop lap timer and accumulate */
                    stopLap = clock();
                    accuTime += stopLap - startLap;

                    /* start lap timer */
                    startLap = clock();

                    for (i=0; i<formatConverterNumOutputChans; i++) {

                        /* hybrid QMF */
                        if (hybMode != -1) { 

                            QMFlib_ApplySynHybFilterbank(numQMFBands, hybridMode, 
                                outputBufferFreq_real[i],outputBufferFreq_imag[i],
                                outputBufferFreqQMF_real[i],outputBufferFreqQMF_imag[i]); 

                        } 

                        /* qmf IO */
                        if (!qmfIOFlag) {

                            /* QMF synthesis */
                            QMFlib_CalculateSynFilterbank(pSynthesisQmf[i],outputBufferFreqQMF_real[i],outputBufferFreqQMF_imag[i],outBuffer[i],ldMode);

                        }
                    }             

                    /* stop lap timer and accumulate */
                    stopLap = clock();
                    accuTimeQMF += stopLap - startLap;
                    break;
                }
                case FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT:
                case FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT:
                    {
                        unsigned int s;
                        unsigned int realIdx, imagIdx;

                        for(j=0;j<stftFormatConverterNumTimeSlots;j++)
                        {
                            /* start lap timer */
                            startLap = clock();


                            /* Transform into frequency domain*/
                            for(i=0;i<formatConverterNumInputChans;i++)
                            {
        
                                for(s=0;s<stftFormatConverterFrameSize;s++)
                                {    
                                    /*part of  previous frame*/
                                    stftWindowedSig[s]                                    = stftTmpInputBufferTime[i][s] * stftWindow[s];
                                    /*current frame*/
                                    stftWindowedSig[stftFormatConverterFrameSize + s]    = stftWindow[stftFormatConverterFrameSize+ s] *inBuffer[i][j*stftFormatConverterFrameSize+s];
                                    /*Update tmp buffer*/
                                    stftTmpInputBufferTime[i][s]                        = inBuffer[i][j*stftFormatConverterFrameSize+s];
                                }

                                /* Apply fft to input signal */
                                FFTlib_Apply(hFFT,stftWindowedSig, stftInputBufferTimeImag, stftOutputBufferFreqReal, stftOutputBufferFreqImag);

                                /* Rearrange values for compact representation (real, imaginary parts interleaved) */
                                inputBufferStft[i][0] = stftOutputBufferFreqReal[0];
                                inputBufferStft[i][1] = stftOutputBufferFreqReal[stftFormatConverterFFTLength/2];

                                for(s=1;s<stftFormatConverterFFTLength/2;s++)
                                {
                                    realIdx = 2*s;
                                    imagIdx = 2*s+1;
                                    inputBufferStft[i][realIdx] = stftOutputBufferFreqReal[s];
                                    inputBufferStft[i][imagIdx] = stftOutputBufferFreqImag[s];
                                }
                            
                            }


                            /* stop lap timer and accumulate */
                            stopLap = clock();
                            accuTimeSTFT += stopLap - startLap;

                            /* start lap timer */
                            startLap = clock();
                            
                            /* do the actual format conversion */
                            formatConverterProcess_freqDomain_STFT((const float **)inputBufferStft, outputBufferStft, hFormatConverter_params, hFormatConverter_state);

                            /* stop lap timer and accumulate */
                            stopLap = clock();
                            accuTime += stopLap - startLap;

                            /* start lap timer */
                            startLap = clock();

                            /* Transform into time domain*/
                            for(i=0; i<formatConverterNumOutputChans; i++)
                            {

                                /*expand to full representation*/
                                stftInputBufferFreqReal[0] = outputBufferStft[i][0];
                                stftInputBufferFreqImag[0] = 0;
                                stftInputBufferFreqReal[stftFormatConverterFFTLength/2] = outputBufferStft[i][1];
                                stftInputBufferFreqImag[stftFormatConverterFFTLength/2] = 0;

                                /*first half of spectrum*/
                                for(s=1;s<stftFormatConverterFFTLength/2;s++)
                                {
                                    realIdx = 2*s;
                                    imagIdx = 2*s+1;
                                    stftInputBufferFreqReal[s] = outputBufferStft[i][realIdx];
                                    stftInputBufferFreqImag[s] = outputBufferStft[i][imagIdx];
                                }
                                
                                /*second half of spectrum*/
                                for(s=1;s<stftFormatConverterFFTLength/2;s++)
                                {
                                    realIdx = 2*s;
                                    imagIdx = 2*s+1;
                                    stftInputBufferFreqReal[stftFormatConverterFFTLength-s] = outputBufferStft[i][realIdx];
                                    stftInputBufferFreqImag[stftFormatConverterFFTLength-s] = -outputBufferStft[i][imagIdx];
                                }


                                /*Apply inverse fft*/
                                FFTlib_Apply(hIFFT,stftInputBufferFreqReal, stftInputBufferFreqImag, stftOutputBufferTimeReal, stftOutputBufferTimeImag);

                                for(s=0;s<stftFormatConverterFrameSize;s++)
                                {
                                    /* overlap add */
                                    outBuffer[i][j*stftFormatConverterFrameSize+s] = (stftOutputBufferTimeReal[s] / ((float) stftFormatConverterFFTLength)) * stftWindow[s] + stftTmpOutputBufferTime[i][s] * stftWindow[stftFormatConverterFrameSize+s];
                                    /* update output buffer */
                                    stftTmpOutputBufferTime[i][s] = stftOutputBufferTimeReal[stftFormatConverterFrameSize + s] / ((float) stftFormatConverterFFTLength);

                                }


                            }
                            /* stop lap timer and accumulate */
                            stopLap = clock();
                            accuTimeSTFT += stopLap - startLap;

                        }

                        break;
                    }
            }

            
                        
        } else {                                  
            
            /* start lap timer */
            startLap = clock();
            
            if (formatConverterProcess_timeDomain( (const float**)inBuffer, 
                                                  outBuffer, 
                                                  hFormatConverter_params, 
                                                  hFormatConverter_state) != 0 )   
            {
                fprintf(stderr,"Error in formatConverterProcess_timeDomain. Exiting application. \n");
                return 1;
            } 
            
            /* stop lap timer and accumulate */
            stopLap = clock();
            accuTime += stopLap - startLap;
            
        }           
        
        
        /* write frame */
        if (!qmfIOFlag) {
            
            if (isLastFrameWrite) {
                samplesToWritePerChannel = frameSizeLastFrame;
            } else {
                samplesToWritePerChannel = frameSize;
            }                
            
            errorFlag = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
            
            NumberSamplesWritten += samplesWrittenPerChannel;     
            
        } else {                           
            
            samplesToWritePerChannel = frameSize;               
            
            errorFlag = wavIO_writeFrame(hWavIO_real,outputBufferFreqQMF_real,samplesToWritePerChannel,&samplesWrittenPerChannel_real);
            errorFlag = wavIO_writeFrame(hWavIO_imag,outputBufferFreqQMF_imag,samplesToWritePerChannel,&samplesWrittenPerChannel_imag);
            
            NumberSamplesWritten_real += samplesWrittenPerChannel_real;     
            NumberSamplesWritten_imag += samplesWrittenPerChannel_imag;     
        }       
        
    } 
    while (!isLastFrameWrite);    
    
    fprintf(stderr,"Format Converter finished. \n");

    /* processing time */
    elapsed_time = ( (double) accuTime ) / CLOCKS_PER_SEC;
    elapsed_time_QMF = ( (double) accuTimeQMF ) / CLOCKS_PER_SEC;
    elapsed_time_STFT = ( (double) accuTimeSTFT ) / CLOCKS_PER_SEC;
    if (!qmfIOFlag) {
        inputDuration = (float) NumberSamplesWritten/outSampleRate;
    } else {
        inputDuration = (float) NumberSamplesWritten_real/outSampleRate;
    }   
    ratio = (float) (elapsed_time + elapsed_time_QMF) / inputDuration;
    
    if (plotInfo > 0) {        
        
        fprintf(stderr,"\nProcessed %d frames ( %.2f s ) in %.2f s.\n", frameNo, inputDuration, (float) (elapsed_time+elapsed_time_QMF+elapsed_time_STFT) );
        fprintf(stderr,"Processing duration / Input duration (formatConverterCmdl) = %4.2f %% .\n", (ratio * 100) ); 
        fprintf(stderr,"Processessing times: formatConverterLib = %.2f s, QMF = %.2f s, STFT %.2f s, ALL = %.2f s.\n", (float) elapsed_time, (float) elapsed_time_QMF, (float) elapsed_time_STFT, (float) (elapsed_time+elapsed_time_QMF+elapsed_time_STFT) );    
        
    }
   }
    
    /**************************************************************************************************************************************************************************************/
    /* ----------------- EXIT WAV IO ----------------- */
    /**************************************************************************************************************************************************************************************/

#ifdef __APPLE__
#pragma mark EXIT
#endif
    
    if ( !qmfIOFlag ) {

        errorFlag = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_updateWavHeader.\n");
            return -1;
        } else {
            assert(nTotalSamplesWrittenPerChannel==NumberSamplesWritten);            
            fprintf(stderr, "\nOutput file %s is written.\n", outFilename);
        }
        
        errorFlag = wavIO_close(hWavIO);
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_close.\n");
            return -1;
        }
        
        /* free buffers */
        for (i=0; i < (unsigned int) nInChannels; i++) {

            free(inBuffer[i]);
        }
        free(inBuffer);
        
        for (i=0; i < (unsigned int) nOutChannels; i++) {
            free(outBuffer[i]);
        }
        free(outBuffer);
    
    } else {
        
        errorFlag = wavIO_updateWavHeader(hWavIO_real, &nTotalSamplesWrittenPerChannel_real);
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_updateWavHeader real.\n");
            return -1;
        } else {
            assert(nTotalSamplesWrittenPerChannel_real==NumberSamplesWritten_real);            
            fprintf(stderr, "\nReal Output file %s is written.\n", qmfOutFilenameReal);
        }
        
        errorFlag = wavIO_updateWavHeader(hWavIO_imag, &nTotalSamplesWrittenPerChannel_imag);
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_updateWavHeader imag.\n");
            return -1;
        } else {
            assert(nTotalSamplesWrittenPerChannel_imag==NumberSamplesWritten_imag);            
            fprintf(stderr, "Imaginary Output file %s is written.\n", qmfOutFilenameImag);
        }        
    
        errorFlag = wavIO_close(hWavIO_real);  
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_close real.\n");
            return -1;
        }
        
        errorFlag = wavIO_close(hWavIO_imag);   
        if (errorFlag != 0 ) 
        {
            fprintf(stderr, "Error during wavIO_close imag.\n");
            return -1;
        }
    }
            
    /**************************************************************************************************************************************************************************************/
    /* ----------------- FORMAT CONVERTER CLOSE -----------------*/
    /**************************************************************************************************************************************************************************************/
    
    /* free memory */
    for (i=0; i < formatConverterNumInputChans; i++) {
        free(dmxMtx[i]);
        free(mixMtx[i]);
    }
    free(dmxMtx);
    free(mixMtx);
    free(azimuthDeviation);
    free(azimuthDeviationExt);
    free(elevationDeviation);
    free(elevationDeviationExt);
    free(distance);
    
    /* close format converter */
    fprintf(stderr,"\nClose Format Converter ...");    
    if ( formatConverterClose ( &hFormatConverter_params, &hFormatConverter_state ) != 0)   
    {
        fprintf(stderr,"Error in formatConverterClose. Exiting application. \n");
        return 1;
    }
    fprintf(stderr," finished!\n");    
    fprintf(stderr,"\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\n");
    
    /**************************************************************************************************************************************************************************************/
    /* ----------------- CLOSE QMF -----------------*/
    /**************************************************************************************************************************************************************************************/
    
    if (formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_PHASE_ALIGN
     || formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_PHASE_ALIGN
     || formatConverterMode == FORMAT_CONVERTER_MODE_PASSIVE_FREQ_DOMAIN) 
    {
     
        /* free memory for input/output in frequency domain */
        
        /* input QMF */
        for( i=0; i < formatConverterNumInputChans; i++ )
        {
            free( inputBufferFreqQMF_real[i]); inputBufferFreqQMF_real[i] = NULL;
            free( inputBufferFreqQMF_imag[i]); inputBufferFreqQMF_imag[i] = NULL;
        }
        free( inputBufferFreqQMF_real ); inputBufferFreqQMF_real = NULL;
        free( inputBufferFreqQMF_imag); inputBufferFreqQMF_imag = NULL;
        
        /* output QMF */
        for( i=0; i < formatConverterNumOutputChans; i++ )
        {
            free( outputBufferFreqQMF_real[i]); outputBufferFreqQMF_real[i] = NULL;
            free( outputBufferFreqQMF_imag[i]); outputBufferFreqQMF_imag[i] = NULL;
        }
        free( outputBufferFreqQMF_real ); outputBufferFreqQMF_real = NULL;
        free( outputBufferFreqQMF_imag ); outputBufferFreqQMF_imag = NULL;
        
        /* hybrid QMF */
        if (hybMode != -1) {             
            /* input full resolution */
            for( i=0; i < formatConverterNumInputChans; i++ )
            {
                free( inputBufferFreq_real[i]); inputBufferFreq_real[i] = NULL;
                free( inputBufferFreq_imag[i]); inputBufferFreq_imag[i] = NULL;
            }
            free( inputBufferFreq_real ); inputBufferFreq_real = NULL;
            free( inputBufferFreq_imag); inputBufferFreq_imag = NULL;
            
            /* output full resolution */
            for( i=0; i < formatConverterNumOutputChans; i++ )
            {
                free( outputBufferFreq_real[i]); outputBufferFreq_real[i] = NULL;
                free( outputBufferFreq_imag[i]); outputBufferFreq_imag[i] = NULL;
            }
            free( outputBufferFreq_real ); outputBufferFreq_real = NULL;
            free( outputBufferFreq_imag ); outputBufferFreq_imag = NULL;
        } 
                
        /* destroy analysis */
        for (i=0; i<formatConverterNumInputChans; i++) {
            
            /* QMF */
            QMFlib_CloseAnaFilterbank(pAnalysisQmf[i]); 
         
        }
                
        /* destroy synthesis */
        for (i=0; i<formatConverterNumOutputChans; i++) {  
            
            /* QMF */
            QMFlib_CloseSynFilterbank(pSynthesisQmf[i]); 
          
        }
        
        /* free multi-channel QMF */
        free( pAnalysisQmf ); pAnalysisQmf = NULL;
        free( pSynthesisQmf ); pSynthesisQmf = NULL;   
        
        /* free multi-channel hybrid QMF */
        free( hybFilterState ); hybFilterState = NULL;   
                
    }
    
    /**************************************************************************************************************************************************************************************/
    
    /**************************************************************************************************************************************************************************************/
    /* ----------------- CLOSE STFT -----------------*/
    /**************************************************************************************************************************************************************************************/
    
    if (formatConverterMode == FORMAT_CONVERTER_MODE_ACTIVE_FREQ_DOMAIN_STFT
     || formatConverterMode == FORMAT_CONVERTER_MODE_CUSTOM_FREQ_DOMAIN_STFT) 
    {

        free(stftWindowedSig); stftWindowedSig = NULL;
        free(stftInputBufferTimeImag); stftInputBufferTimeImag = NULL;
        free(stftOutputBufferTimeReal); stftOutputBufferTimeReal = NULL;
        free(stftOutputBufferTimeImag); stftOutputBufferTimeImag = NULL;
        free(stftOutputBufferFreqReal); stftOutputBufferFreqReal = NULL;
        free(stftOutputBufferFreqImag); stftOutputBufferFreqImag = NULL;
        free(stftInputBufferFreqReal); stftInputBufferFreqReal = NULL;
        free(stftInputBufferFreqImag); stftInputBufferFreqImag = NULL;
        
        for(i=0;i<formatConverterNumInputChans;i++)
        {
            free(stftTmpInputBufferTime[i]);
            free(inputBufferStft[i]);
        }
        free(stftTmpInputBufferTime); stftTmpInputBufferTime = NULL;
        free(inputBufferStft); inputBufferStft = NULL;
        

        for(i=0;i<formatConverterNumOutputChans;i++)
        {
            free(stftTmpOutputBufferTime[i]);
            free(outputBufferStft[i]);
        }
        free(stftTmpOutputBufferTime); stftTmpOutputBufferTime = NULL;
        free(outputBufferStft); outputBufferStft = NULL;

        /*free FFT*/
        FFTlib_Destroy(&hFFT);
        FFTlib_Destroy(&hIFFT);
    
    }
    /**************************************************************************************************************************************************************************************/
    return 0;
    
}

int readAngleDeviationsFromFile( char* filename, float *azimuthDeviation, float *elevationDeviation, int numOutputChans )
{
    FILE* fileHandle;
    char line[512] = {0};
    int col = 0;
    int outputChannelIdx = 0;
    int i;
    
    /* open file */
    fileHandle = fopen(filename, "r");    
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open angle deviation input file\n");        
        return -1;
    } else {
        fprintf(stderr, "\nFound angle deviation input file: %s.\n", filename );
    }         
    
    /* Get new line */
    fprintf(stderr, "Reading angle deviation input file ... \n");    
    while ( fgets(line, 511, fileHandle) != NULL )
    {
        int i = 0;
        char* pChar = line;
        
        /* Add newline at end of line (for eof line), terminate string after newline */
        line[strlen(line)+1] = '\0';
        line[strlen(line)] = '\n';
        
        /* Replace all white spaces with new lines for easier parsing */
        while ( pChar[i] != '\0')
        {
            if ( pChar[i] == ' ' || pChar[i] == '\t')
                pChar[i] = '\n';            
            i++;
        }
        
        pChar = line;
        col = 0;
        
        /* Parse line */        
        while ( (*pChar) != '\0')
        {
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            if ( col == 2 ) {
                fprintf(stderr,"Angle deviation read from file has wrong format (too many columns)!\n");        
                return -1;
            }
            if ( outputChannelIdx == numOutputChans ) {
                fprintf(stderr,"Angle deviation read from file has wrong format (too many output channels)!\n");        
                return -1;
            }            
            
            if (col == 0) {
                azimuthDeviation[outputChannelIdx]   = (float) atof(pChar);
            } else if (col == 1) {
                elevationDeviation[outputChannelIdx] = (float) atof(pChar);
            }
        
            /* Jump over parsed float value */
            while (  (*pChar) != '\n' )
                pChar++;
            
            /* Jump over new lines */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            col++;            
        }        
        if (col != 2) {
            fprintf(stderr,"Angle deviation read from file has wrong format (too little columns)!\n"); 
            return -1;
        }
        outputChannelIdx++;       
    }
    
    if (outputChannelIdx != numOutputChans) {
        fprintf(stderr,"Angle deviation read from file has wrong format (too little rows)!\n"); 
        return -1;
    }
    
    /* print values */
    fprintf(stderr,"\nangleDeviation [deg] = [\n");
    for (i = 0; i < numOutputChans; i++) {
        fprintf(stderr,"%2.4f %2.4f\n",azimuthDeviation[i],elevationDeviation[i]);        
    }
    fprintf(stderr,"]\n");
    
    fclose(fileHandle);
    return 0;
}

int readDistancesFromFile( char* filename, float *distance, int numOutputChans )
{
    FILE* fileHandle;
    char line[512] = {0};
    int col = 0;
    int outputChannelIdx = 0;
    int i;
    
    /* open file */
    fileHandle = fopen(filename, "r");    
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open distance input file\n");        
        return -1;
    } else {
        fprintf(stderr, "\nFound distance input file: %s.\n", filename );
    }         
    
    /* Get new line */
    fprintf(stderr, "Reading distance input file ... \n");    
    while ( fgets(line, 511, fileHandle) != NULL )
    {
        int i = 0;
        char* pChar = line;
        
        /* Add newline at end of line (for eof line), terminate string after newline */
        line[strlen(line)+1] = '\0';
        line[strlen(line)] = '\n';
        
        /* Replace all white spaces with new lines for easier parsing */
        while ( pChar[i] != '\0')
        {
            if ( pChar[i] == ' ' || pChar[i] == '\t')
                pChar[i] = '\n';            
            i++;
        }
        
        pChar = line;

        col = 0;
        
        /* Parse line */        
        while ( (*pChar) != '\0')
        {
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            if ( col == 1 ) {
                fprintf(stderr,"Distance read from file has wrong format (too many columns)!\n");        
                return -1;
            }
            if ( outputChannelIdx == numOutputChans ) {
                fprintf(stderr,"Distance read from file has wrong format (too many output channels)!\n");        
                return -1;
            }
            distance[outputChannelIdx]   = (float) atof(pChar);
            
            /* Jump over parsed float value */
            while (  (*pChar) != '\n' )
                pChar++;
            
            /* Jump over new lines */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            col++;            
        }        
        if (col != 1) {
            fprintf(stderr,"Distance read from file has wrong format (too little columns)!\n"); 
            return -1;
        }
        outputChannelIdx++;       
    }
    
    if (outputChannelIdx != numOutputChans) {
        fprintf(stderr,"Distance read from file has wrong format (too little rows)!\n"); 
        return -1;
    }
    
    /* print values */
    fprintf(stderr,"\ndistance [m] = [\n");
    for (i = 0; i < numOutputChans; i++) {
        fprintf(stderr,"%2.4f\n",distance[i]);        
    }
    fprintf(stderr,"]\n");

    fclose(fileHandle);    
    return 0;
}

int readDmxMtxFromFile( char* filename, float **dmxMtx, int numInputChans, int numOutputChans )
{
    
    FILE* fileHandle;
    char line[512] = {0};
    int inputChannelIdx = 0;
    int outputChannelIdx = 0;
    int i, j;
    
    /* open file */
    fileHandle = fopen(filename, "r");    
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open DMX matrix input file: %s\n",filename);        
        return -1;
    } else {
        fprintf(stderr, "\nFound DMX matrix input file: %s\n", filename );
    }         
    
    /* Get new line */
    fprintf(stderr, "Reading DMX matrix from file ... \n");    
    while ( fgets(line, 511, fileHandle) != NULL )
    {
        int i = 0;
        char* pChar = line;
        
        /* Add newline at end of line (for eof line), terminate string after newline */
        line[strlen(line)+1] = '\0';
        line[strlen(line)] = '\n';
        
        /* Replace all white spaces with new lines for easier parsing */
        while ( pChar[i] != '\0')
        {
            if ( pChar[i] == ' ' || pChar[i] == '\t')
                pChar[i] = '\n';            
            i++;
        }
        
        pChar = line;
        outputChannelIdx = 0;
        
        /* Parse line */        
        while ( (*pChar) != '\0')
        {
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            if ( outputChannelIdx == numOutputChans ) {
                fprintf(stderr,"DMX matrix read from file has wrong format (too many output channels)!\n");        
                return -1;
            }
            if ( inputChannelIdx == numInputChans ) {
                fprintf(stderr,"DMX matrix read from file has wrong format (too many input channels)!\n");        
                return -1;
            }
            dmxMtx[inputChannelIdx][outputChannelIdx] = (float) atof(pChar);
            
            /* Jump over parsed float value */
            while (  (*pChar) != '\n' )
                pChar++;
            
            /* Jump over new lines */
            while (  (*pChar) == '\n' || (*pChar) == '\r'  )
                pChar++;
            
            outputChannelIdx++;            
        }        
        
        if (outputChannelIdx != numOutputChans) {
            fprintf(stderr,"DMX matrix read from file has wrong format (too little columns)!\n");    
            return -1;
        }
        
        inputChannelIdx++;
    }
    
    if (inputChannelIdx != numInputChans) {
        fprintf(stderr,"DMX matrix read from file has wrong format (too little rows)!\n");
        return -1;
    }
    
    /* print values */
    fprintf(stderr,"\ndmxMtx [linear] = [\n");
    for (i = 0; i < numInputChans; i++) {
        for (j = 0; j < numOutputChans; j++) {
            fprintf(stderr,"%2.4f ",dmxMtx[i][j]);
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"]\n");
    
    fclose(fileHandle);
    return 0;
    
} 

int readEqParamsFromFile( char *filename, int numCh, int *numEQs, eqParamsStruct **eqParams, int **eqMap )
{   
    FILE* fileHandle;
    char line[512] = {0};
    int numChEq, m, n;    

    /* open file */
    fileHandle = fopen(filename, "r");    
    if ( !fileHandle )
    {
        fprintf(stderr,"Unable to open EQ parameters input file\n");        
        return -1;
    } else {
        fprintf(stderr, "\nFound EQ parameters input file: %s.\n", filename );
    }         
  
    fprintf(stderr, "Reading EQ parameters from file ... \n");    

    /* read/check number of input channels */    
    if (fgets(line, 511, fileHandle) == NULL ){
        fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
    } else {
        numChEq = atoi(line);
        if( numChEq != numCh ){
            fprintf(stderr, "ERROR: EQ parameters file defines settings for %d input channels but FormatConverter expects %d\n",numChEq,numCh);            
            return -1;
        } else
            fprintf(stderr, "EQ parameters file defines settings for %d input channels\n",numChEq);
    }
       
    /* read number of EQ definitions */    
    if (fgets(line, 511, fileHandle) == NULL ){
        fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
    } else {
        *numEQs = atoi(line);
        if( *numEQs > numCh ){
            fprintf(stderr, "ERROR: more EQs than input channels defined\n");            
            return -1;
        } else
            fprintf(stderr, "EQ parameters file defines %d EQs\n",*numEQs);
    }

    /* allocate *numEQs eqParamsStructs */ 
    *eqParams = (eqParamsStruct*)calloc(*numEQs,sizeof(eqParamsStruct));        

    /* read *numEQs EQ definitions */ 
    for(n=0; n<*numEQs; n++){
        /* read global gain for current EQ */
        if (fgets(line, 511, fileHandle) == NULL )
            fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
        else
            (*eqParams)[n].G = (float) atof(line);
        /* read number of cascaded peak filters for current EQ */
        if (fgets(line, 511, fileHandle) == NULL )
            fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
        else
            (*eqParams)[n].nPkFilter = atoi(line);      
        /* allocate and read peak filters parameters for current EQ */
        (*eqParams)[n].pkFilterParams = (pkFilterParamsStruct*)calloc( (*eqParams)[n].nPkFilter, sizeof( pkFilterParamsStruct ) ); 
        for(m=0; m<(*eqParams)[n].nPkFilter; m++){
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].f = (float) atof(line);
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].q = (float) atof(line);
            if (fgets(line, 511, fileHandle) == NULL )
                fprintf(stderr, "\nERROR reading EQ parameters input file\n" );
            else
                (*eqParams)[n].pkFilterParams[m].g = (float) atof(line);
        }
    }
   
    /* allocate and read eqMap defining mapping of EQs to input channels */ 
    *eqMap = (int*)calloc(numChEq,sizeof(int));        
    for(n=0; n<numChEq; n++){
        if (fgets(line, 511, fileHandle) == NULL ){
            fprintf(stderr, "\nERROR reading EQ parameters input file: incomplete EqMap\n");
            return -1;
        } else {
            (*eqMap)[n] = atoi(line);
        }
    }

    /* print EQ parameters values ... */
    for(n=0; n<*numEQs; n++){
        fprintf(stderr,"\nEQ params %d of %d  : EQ gain    = %f dB\n",n+1,*numEQs,(*eqParams)[n].G);
        for(m=0; m<(*eqParams)[n].nPkFilter; m++){
            fprintf(stderr,"peak filter %d of %d: Pk_freq    = %f Hz\n",m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].f);
            fprintf(stderr,"peak filter %d of %d: Pk_qfactor = %f \n",  m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].q);
            fprintf(stderr,"peak filter %d of %d: Pk_gain    = %f dB\n",m+1, (*eqParams)[n].nPkFilter, (*eqParams)[n].pkFilterParams[m].g);
        }
    }
    fprintf(stderr,"\ninput channel -> EQ index map:\n");
    for(n=0; n<numChEq; n++)
        fprintf(stderr,"input ch %d: %d\n",n,(*eqMap)[n]);
    fprintf(stderr,"\n");

    fclose(fileHandle);
    return 0;   
} 

const char *formatConverterCmdlGetInfo( void )
{
    return ( "Built " __DATE__ ", " __TIME__ );
}

/**************************************************************************************************************************************************************************************/
