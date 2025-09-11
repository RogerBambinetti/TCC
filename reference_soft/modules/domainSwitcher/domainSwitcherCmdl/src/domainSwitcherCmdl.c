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
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wavIO.h"
#include "qmflib.h"
#include "qmflib_hybfilter.h"
#include "fftlib.h"
#include "domainSwitcherCmdl.h"

#define QMFLIB_MAX_CHANNELS 32
#define SUBBAND_BYTE_DEPTH 4

int main(int argc, char *argv[])
{
    WAVIO_HANDLE hWavIO = 0;
    WAVIO_HANDLE hWavIO_real = 0;
    WAVIO_HANDLE hWavIO_imag = 0;
    char* inFile        = 0;
    char inFileReal[200];
    char inFileImag[200];
    char* outFile       = 0;
    char outFileReal[200];
    char outFileImag[200];
    char* wavExtension  = ".wav";
    FILE *pInFileName   = 0;
    FILE *pInFileNameReal  = 0;
    FILE *pInFileNameImag  = 0;
    FILE *pOutFileName    = 0;
    FILE *pOutFileNameReal = 0;
    FILE *pOutFileNameImag  = 0;
    int error           = 0;
    unsigned int nInChannels  = 0;
    unsigned int InSampleRate = 0;
    unsigned int InBytedepth  = 0;
    unsigned long nTotalSamplesPerChannel = 0;
    unsigned long nTotalSamplesWrittenPerChannel = 0;
    int nSamplesPerChannelFilled = 0;
    float **inBufferReal = 0;
    float **inBufferImag = 0;
    float **outBufferReal = 0;
    float **outBufferImag = 0;
    int bSubBandInput = 0;
    int bSubBandOutput = 0;
    int subBandDomainMode = 0; /* 0 = QMF, 1 = STFT */
    int numBands = 0;
    int frameSize = 0;
    
    unsigned long nTotalSamplesReadPerChannel = 0;
    unsigned int isLastFrame = 0;
    
    /* QMF */
    int LdMode = 0;
    int hybridHFAlign = 1; /* 0: Align hybrid HF part with LF part, 1: no delay alignment for HF part */
    
    QMFLIB_POLYPHASE_ANA_FILTERBANK** pAnalysisQmf = 0;
    QMFLIB_POLYPHASE_SYN_FILTERBANK** pSynthesisQmf = 0;
    QMFLIB_HYBRID_FILTER_STATE hybFilterState[QMFLIB_MAX_CHANNELS];
    QMFLIB_HYBRID_FILTER_MODE hybridMode = QMFLIB_HYBRID_OFF;
    
    /* STFT */
    int fftlen    = 0;
    int stftFrameSize = 0;
    
    HANDLE_FFTLIB hFFTlibF = 0;
    HANDLE_FFTLIB hFFTlibB = 0;
    
    float *fftBufRe    = 0;
    float *fftBufIm    = 0;
    float *stftWindow			        = NULL;
    float *stftWindowedSignalInputRe    = NULL;
    float *stftWindowedSignalOutputRe	= NULL;
    float *stftWindowedSignalInputIm	= NULL;
    float *stftWindowedSignalOutputIm	= NULL;
    float **stftWindowedSignalInputTmp  = NULL;
    float **stftWindowedSignalOutputTmp = NULL;
    
    unsigned int i = 0;
    int flushingSamplesPerChannel = 0;
    int additionalDelay = 0;
    int bCompensateDelay = 0;
    int outputBitdepth = 0;
    int outputBytedepth = 4;
    int sample = 0;
    int channel = 0;
    int frame = 0;
    
    fprintf( stdout, "\n");
    fprintf( stdout, "******************** MPEG-H 3D Audio Coder - Edition 4.0 **********************\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*                              TF/FT Domain Switcher                          *\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*                                  %s                                *\n", __DATE__);
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*    This software may only be used in the development of the MPEG 3D Audio   *\n");
    fprintf( stdout, "*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *\n");
    fprintf( stdout, "*                                                                             *\n");
    fprintf( stdout, "*******************************************************************************\n");
    fprintf( stdout, "\n");
    
    /* Commandline Parsing */
    
    for ( i = 1; i < (unsigned int) argc; ++i )
    {
        if (!strcmp(argv[i],"-if"))      /* Required */
        {
            int long nWavExtensionChars = 0;
            
            if ( argv[i+1] ) {
                inFile = argv[i+1];
            }
            else {
                fprintf(stderr, "No output filename given.\n");
                return -1;
            }
            nWavExtensionChars = strspn(wavExtension,inFile);
            if ( nWavExtensionChars != 4 ) {
                bSubBandInput = 1;
                strcpy(inFileReal, inFile);
                strcpy(inFileImag, inFile);
            }
            i++;
        }
        else if (!strcmp(argv[i],"-of"))  /* Required */
        {
            int long nWavExtensionChars = 0;
            
            if ( argv[i+1] ) {
                outFile = argv[i+1];
            }
            else {
                fprintf(stderr, "No output filename given.\n");
                return -1;
            }
            
            nWavExtensionChars = strspn(wavExtension,outFile);
            if ( nWavExtensionChars != 4 ) {
                bSubBandOutput = 1;
                strcpy(outFileReal, outFile);
                strcpy(outFileImag, outFile);
            }
            i++;
        }
        else if (!strcmp(argv[i],"-ld"))  /* Optional */
        {
            LdMode = 1;
        }
        else if (!strcmp(argv[i],"-stft"))  /* Optional */
        {
            subBandDomainMode = 1;
        }
        else if (!strcmp(argv[i],"-compensateDelay"))  /* Optional */
        {
            bCompensateDelay = 1;
        }
        else if (!strcmp(argv[i],"-bands"))  /* Optional */
        {
            if ( argv[i+1] ) {
                numBands = atoi(argv[i+1]);
            }
            else {
                fprintf(stderr, "No number of qmf bands given.\n");
                return -1;
            }
            i++;
        }
        else if (!strcmp(argv[i],"-hybrid71"))  /* Optional */
        {
            hybridMode = QMFLIB_HYBRID_THREE_TO_TEN;
        }
        else if (!strcmp(argv[i],"-hybrid77"))  /* Optional */
        {
            hybridMode = QMFLIB_HYBRID_THREE_TO_SIXTEEN;
        }
        else if (!strcmp(argv[i],"-hybalign"))  /* Optional */
        {
            hybridHFAlign = 0;
        }
        else if (!strcmp(argv[i],"-additionalDelay"))  /* Optional */
        {
            if ( argv[i+1] ) {
                additionalDelay = atoi(argv[i+1]);
            }
            else {
                fprintf(stderr, "No additional delay given.\n");
                return -1;
            }
            i++;
        }
        else if (!strcmp(argv[i],"-outputBitdepth"))  /* Optional */
        {
            if ( argv[i+1] ) {
                outputBitdepth = atoi(argv[i+1]);
            }
            else {
                fprintf(stderr, "No bit depth given. Please select 16, 24 or 32 bits.\n");
                return -1;
            }
            
            switch( outputBitdepth ) {
                case 16:
                    outputBytedepth = 2;
                    break;
                    
                case 24:
                    outputBytedepth = 3;
                    break;
                    
                case 32:
                    outputBytedepth = 4;
                    break;
                    
                default:
                    fprintf(stderr, "Invalid bit depth setting. Please select 16, 24 or 32 bits.\n");
                    return -1;
                    break;
            }
            
            i++;
        }
        else {
            fprintf(stderr, "Invalid command line.\n");
            return -1;
        }
    }
    
    if ( argc < 4 || inFile == NULL || outFile == NULL )
    {
        fprintf(stderr, "-if <input.wav> or <input> \n\t the latter will be extended to <input>_real.qmf and <input>_imag.qmf or to <input>_real.stft and <input>_imag.stft dependent on -stft option\n");
        fprintf(stderr, "-of <output.wav> or <output> \n\t the latter will be extended to <output>_real.qmf and <output>_imag.qmf or to <output>_real.stft and <output>_imag.stft dependent on -stft option\n");
        fprintf(stderr, "-bands <num_qmf_bands>, \n\t default: 64\n");
        fprintf(stderr, "-stft, \n\t STFT256 subband domain processing according to ISO/IEC 23008-3:2015/AMD3 (default: QMF64)\n");
        fprintf(stderr, "-compensateDelay, \n\t remove the delay introduced by Subband Analysis and Synthesis, default: NO compensation\n");
        fprintf(stderr, "-additionalDelay, \n\t delay on top of qmf delay in samples, default: NO additional delay\n");
        fprintf(stderr, "-outputBitdepth, \n\t in case of qmf2wav - wav output bitdepth, default: 32 bit\n");
        return 1;
    }
    
    /* sanity check */
    if ( bSubBandInput == 1 && bSubBandOutput == 1 ) {
        fprintf(stderr, "Either input or output must be \".wav\". Otherwise, there is nothing to do.\n");
        return -1;
    }
    
    if (subBandDomainMode == 0) { /* QMF */
        
        frameSize = 64;
        if (numBands == 0) {
            numBands = 64;
        }
        
        if ( LdMode ) {
            flushingSamplesPerChannel = (int) (numBands * 2.5) + /* analysis */
            (int) (numBands * 1.5) ; /* synthesis */
        }
        else {
            flushingSamplesPerChannel = numBands * 5 +     /* analysis */
            numBands * 4 + 1 ; /* synthesis */
        }
        
        if ( QMFLIB_HYBRID_OFF != hybridMode ) {
            flushingSamplesPerChannel += 6 * numBands;
        }
        
    } else if (subBandDomainMode == 1) {
        
        fftlen = 512;
        frameSize = fftlen/2;
        numBands = fftlen/2;
        stftFrameSize = fftlen/2;
        stftWindow = stftWindow_512;
        
        /* init FFTLIB */
        FFTlib_Create(&hFFTlibF, fftlen, -1);
        FFTlib_Create(&hFFTlibB, fftlen, 1);
        
        flushingSamplesPerChannel = stftFrameSize;
        
    } else {
        return -1;
    }
    
    flushingSamplesPerChannel += additionalDelay;
    
    if ( bSubBandInput ) {
        if (subBandDomainMode == 0) {
            strcat(inFileReal, "_real.qmf");
            strcat(inFileImag, "_imag.qmf");
        } else if (subBandDomainMode == 1) {
            strcat(inFileReal, "_real.stft");
            strcat(inFileImag, "_imag.stft");
        }
        pInFileNameReal  = fopen(inFileReal, "rb");
        pInFileNameImag  = fopen(inFileImag, "rb");
    }
    else {
        pInFileName  = fopen(inFile, "rb");
    }
    
    if ( bSubBandOutput ) {
        if (subBandDomainMode == 0) {
            strcat(outFileReal, "_real.qmf");
            strcat(outFileImag, "_imag.qmf");
        } else if (subBandDomainMode == 1) {
            strcat(outFileReal, "_real.stft");
            strcat(outFileImag, "_imag.stft");
        }
        pOutFileNameReal = fopen(outFileReal, "wb");
        pOutFileNameImag = fopen(outFileImag, "wb");
    }
    else {
        pOutFileName = fopen(outFile, "wb");
    }
    
    if ( bCompensateDelay ) {
        error = wavIO_init(&hWavIO, frameSize, 0, -flushingSamplesPerChannel );
    }
    else {
        error = wavIO_init(&hWavIO, frameSize, 0, 0 );
    }
    
    /* wavIO for wav file */
    if ( 0 != error )
    {
        fprintf(stderr, "Error during initialization.\n");
        return -1;
    }
    
    /* wavIO for qmf file */
    if ( bSubBandInput || bSubBandOutput ) {
        error = wavIO_init(&hWavIO_real, numBands, 0, 0);
        if ( 0 != error )
        {
            fprintf(stderr, "Error during initialization.\n");
            return -1;
        }
        
        error = wavIO_init(&hWavIO_imag, numBands, 0, 0);
        if ( 0 != error )
        {
            fprintf(stderr, "Error during initialization.\n");
            return -1;
        }
    }
    
    if ( bSubBandInput ) {
        
        error = wavIO_openRead(hWavIO_real, pInFileNameReal, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening input subband real file\n");
            return -1;
        }
        
        error = wavIO_openRead(hWavIO_imag, pInFileNameImag, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening input subband imag file\n");
            return -1;
        }
        
    }
    
    /* wav input */
    if( !bSubBandInput ) {
        error = wavIO_openRead(hWavIO, pInFileName, &nInChannels, &InSampleRate, &InBytedepth, &nTotalSamplesPerChannel, &nSamplesPerChannelFilled);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening input wav\n");
            return -1;
        }
        outputBytedepth = InBytedepth;
    }
    
    /* wav output */
    if( !bSubBandOutput ) {
        error = wavIO_openWrite(hWavIO, pOutFileName, nInChannels, InSampleRate, outputBytedepth);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening output wav\n");
            return -1;
        }
    }
    
    if ( bSubBandOutput ) {
        error = wavIO_openWrite(hWavIO_real, pOutFileNameReal, nInChannels, InSampleRate, SUBBAND_BYTE_DEPTH);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening output subband\n");
            return -1;
        }
        
        error = wavIO_openWrite(hWavIO_imag, pOutFileNameImag, nInChannels, InSampleRate, SUBBAND_BYTE_DEPTH);
        if ( 0 != error )
        {
            fprintf(stderr, "Error opening output subband\n");
            return -1;
        }
    }
    
    /* allocate buffers */
    inBufferReal = (float**) calloc(nInChannels, sizeof(float*));
    inBufferImag = (float**) calloc(nInChannels, sizeof(float*));
    outBufferReal = (float**) calloc(nInChannels, sizeof(float*));
    outBufferImag = (float**) calloc(nInChannels, sizeof(float*));
    
    if (subBandDomainMode == 0) {
        
        if ( QMFLIB_HYBRID_OFF != hybridMode )
        {
            for ( i = 0; i < nInChannels; ++i )
                QMFlib_InitAnaHybFilterbank(&hybFilterState[i]);
        }
        
        pAnalysisQmf = (QMFLIB_POLYPHASE_ANA_FILTERBANK**) calloc(nInChannels, sizeof(QMFLIB_POLYPHASE_ANA_FILTERBANK*));
        pSynthesisQmf = (QMFLIB_POLYPHASE_SYN_FILTERBANK**) calloc(nInChannels, sizeof(QMFLIB_POLYPHASE_SYN_FILTERBANK*));
        
        /* Init QMFlib Analysis */
        QMFlib_InitAnaFilterbank(numBands, LdMode);
        
        /* Init QMFLib Synthesis */
        QMFlib_InitSynFilterbank(numBands, LdMode);
        
    } else if (subBandDomainMode == 1) {
        
        fftBufRe    = calloc(fftlen, sizeof(float));
        fftBufIm    = calloc(fftlen, sizeof(float));
        
        stftWindowedSignalInputRe  = (float*) calloc (fftlen, sizeof (float));
        stftWindowedSignalOutputRe = (float*) calloc (fftlen, sizeof (float));
        stftWindowedSignalInputIm   = (float*) calloc (fftlen, sizeof (float));
        stftWindowedSignalOutputIm  = (float*) calloc (fftlen, sizeof (float));
        
        stftWindowedSignalInputTmp = (float**) calloc(nInChannels, sizeof (float*));
        stftWindowedSignalOutputTmp = (float**) calloc(nInChannels, sizeof (float*));
        
    }
    
    for (i = 0; i < nInChannels; ++i )
    {
        inBufferReal[i] = calloc(numBands, sizeof(float));
        inBufferImag[i] = calloc(numBands, sizeof(float));
        outBufferReal[i] = calloc(numBands, sizeof(float));
        outBufferImag[i] = calloc(numBands, sizeof(float));
        
        if (subBandDomainMode == 0) {
            QMFlib_OpenAnaFilterbank( &pAnalysisQmf[i] );
            QMFlib_OpenSynFilterbank( &pSynthesisQmf[i] );
        } else if (subBandDomainMode == 1) {
            stftWindowedSignalInputTmp[i] = (float*) calloc(stftFrameSize, sizeof (float));
            stftWindowedSignalOutputTmp[i] = (float*) calloc(stftFrameSize, sizeof (float));
        }
    }
    
    do  /*loop over all frames*/
    {
        unsigned int  nSamplesReadPerChannel = 0;
        unsigned int  nSamplesToWritePerChannel = 0;
        unsigned int  nSamplesWrittenPerChannel = 0;
        unsigned int  nSamplesToWritePerChannel_real = 0;
        unsigned int  nSamplesWrittenPerChannel_real = 0;
        unsigned int  nSamplesToWritePerChannel_imag = 0;
        unsigned int  nSamplesWrittenPerChannel_imag = 0;
        unsigned int  nZerosPaddedBeginning = 0;
        unsigned int  nZerosPaddedEnd = 0;
        
        float mHybridReal[QMFLIB_MAX_HYBRID_BANDS] = { 0.0f };
        float mHybridImag[QMFLIB_MAX_HYBRID_BANDS] = { 0.0f };
        
        unsigned int j = 0, s = 0;
        
        /*********************/
        /* time domain input */
        /*********************/
        
        if ( !bSubBandInput ) {
            
            /* read frame */
            if ( !isLastFrame ) {
                wavIO_readFrame(hWavIO, inBufferReal, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
                nSamplesToWritePerChannel      = nSamplesReadPerChannel;
                nSamplesToWritePerChannel_real = nSamplesReadPerChannel;
                nSamplesToWritePerChannel_imag = nSamplesReadPerChannel;
                nTotalSamplesReadPerChannel   += nSamplesReadPerChannel;
            }
            
            /* Flush the subband transformation / Calculate flushing samples */
            if ( nSamplesReadPerChannel < (unsigned int) numBands ) {
                
                int samplesToAdd = 0;
                
                if ( flushingSamplesPerChannel < numBands ) {
                    if ( flushingSamplesPerChannel == 0 ) {
                        flushingSamplesPerChannel = -1;
                    }
                    else if ( flushingSamplesPerChannel < 0 ) {
                        samplesToAdd = flushingSamplesPerChannel + numBands;
                        nSamplesToWritePerChannel = samplesToAdd;
                    }
                    else {
                        samplesToAdd = flushingSamplesPerChannel;
                        flushingSamplesPerChannel = -1;
                        nSamplesToWritePerChannel = samplesToAdd;
                    }
                }
                else {
                    samplesToAdd = numBands;
                    nSamplesToWritePerChannel = samplesToAdd;
                }
                
                for ( channel = 0; channel < (int) nInChannels; ++channel ) {
                    for ( sample = nSamplesReadPerChannel; sample < numBands; ++sample ) {
                        inBufferReal[channel][sample] = 0.0f;
                    }
                }
            }
            
            /* Apply the subband transformation */
            for (j = 0; j < nInChannels; j++)
            {
                if (subBandDomainMode == 0) { /* QMF */
                    
                    QMFlib_CalculateAnaFilterbank(pAnalysisQmf[j],
                                                  inBufferReal[j],
                                                  outBufferReal[j],
                                                  outBufferImag[j],
                                                  LdMode);
                    
                } else if (subBandDomainMode == 1) { /* STFT */
                    
                    for(s=0;s<stftFrameSize;s++)
                    {
                        /*part of  previous frame*/
                        stftWindowedSignalInputRe[s]                 = stftWindowedSignalInputTmp[j][s] * stftWindow[s];
                        /*current frame*/
                        stftWindowedSignalInputRe[stftFrameSize + s] = stftWindow[stftFrameSize+s] * inBufferReal[j][s];
                        /*Update tmp buffer*/
                        stftWindowedSignalInputTmp[j][s]           = inBufferReal[j][s];
                    }
                    
                    /* Apply fft to input signal */
                    FFTlib_Apply(hFFTlibF,stftWindowedSignalInputRe, stftWindowedSignalInputIm, fftBufRe, fftBufIm);
                    
                    /* sparse representation */
                    outBufferReal[j][0] = fftBufRe[0];
                    outBufferImag[j][0] = fftBufRe[fftlen/2];
                    for(s=1;s<fftlen/2;s++)
                    {
                        outBufferReal[j][s] = fftBufRe[s];
                        outBufferImag[j][s] = fftBufIm[s];
                    }
                    
                }
            }
        }
        /************************/
        /* subband domain input */
        /************************/
        else {
            
            /* read qmf frame */
            wavIO_readFrame(hWavIO_real, inBufferReal, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
            wavIO_readFrame(hWavIO_imag, inBufferImag, &nSamplesReadPerChannel, &isLastFrame, &nZerosPaddedBeginning, &nZerosPaddedEnd);
            
            nSamplesToWritePerChannel      = nSamplesReadPerChannel;
            nSamplesToWritePerChannel_real = nSamplesReadPerChannel;
            nSamplesToWritePerChannel_imag = nSamplesReadPerChannel;
            nTotalSamplesReadPerChannel   += nSamplesReadPerChannel;
            
        }
        
        /**********************/
        /* time domain output */
        /**********************/
        
        /* Apply the inverse qmf transformation */
        if ( !bSubBandOutput ) {
            
            for (j = 0; j < nInChannels; j++) {
                
                if (subBandDomainMode == 0) { /* QMF */
                    
                    if ( QMFLIB_HYBRID_OFF != hybridMode )
                    {
                        /* Apply hybrid qmf and inverse hybrid qmf transformation */
                        QMFlib_ApplyAnaHybFilterbank(&hybFilterState[i], hybridMode, numBands, hybridHFAlign, inBufferReal[j], inBufferImag[j], mHybridReal, mHybridImag);
                        
                        QMFlib_ApplySynHybFilterbank(numBands, hybridMode, mHybridReal, mHybridImag, inBufferReal[j], inBufferImag[j]);
                    }
                    
                    QMFlib_CalculateSynFilterbank(pSynthesisQmf[j],
                                                  inBufferReal[j],
                                                  inBufferImag[j],
                                                  outBufferReal[j],
                                                  LdMode);
                    
                } else if (subBandDomainMode == 1) { /* STFT */
                    
                    /* full representation */
                    fftBufRe[0]        = inBufferReal[j][0];
                    fftBufRe[fftlen/2] = inBufferImag[j][0];
                    fftBufIm[0]        = 0;
                    fftBufIm[fftlen/2] = 0;
                    for(s=1;s<fftlen/2;s++)
                    {
                        fftBufRe[s] = inBufferReal[j][s];
                        fftBufIm[s] = inBufferImag[j][s];
                    }
                    
                    /*second half of spectrum*/
                    for(s=1;s<fftlen/2;s++)
                    {
                        fftBufRe[fftlen-s] = fftBufRe[s];
                        fftBufIm[fftlen-s] = -fftBufIm[s];
                    }
                    
                    /*Apply inverse fft*/
                    FFTlib_Apply(hFFTlibB,fftBufRe, fftBufIm, stftWindowedSignalOutputRe, stftWindowedSignalOutputIm);
                    
                    for(s=0;s<stftFrameSize;s++)
                    {
                        /* overlap add */
                        outBufferReal[j][s] = (stftWindowedSignalOutputRe[s] / ((float) fftlen)) * stftWindow[s] + stftWindowedSignalOutputTmp[j][s] * stftWindow[stftFrameSize+s];
                        /* update output buffer */
                        stftWindowedSignalOutputTmp[j][s] = stftWindowedSignalOutputRe[stftFrameSize + s] / ((float) fftlen);
                    }
                    
                }
            }
        }
        
        /* write frame - wav file */
        if ( !bSubBandOutput ) {
            wavIO_writeFrame(hWavIO, outBufferReal, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel);
        }
        else {
            wavIO_writeFrame(hWavIO_real, outBufferReal, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel_real);
            wavIO_writeFrame(hWavIO_imag, outBufferImag, nSamplesToWritePerChannel, &nSamplesWrittenPerChannel_imag);
        }
        
        flushingSamplesPerChannel -= ( numBands - nSamplesReadPerChannel );
        
        fprintf(stderr,"Processing frame: %d\n", frame++);
    }  while ( !isLastFrame || flushingSamplesPerChannel >= 0 );
    
    if ( bSubBandOutput ) {
        wavIO_updateWavHeader(hWavIO_real, &nTotalSamplesWrittenPerChannel);
        wavIO_updateWavHeader(hWavIO_imag, &nTotalSamplesWrittenPerChannel);
    }
    else {
        wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);
    }
    
    if ( bSubBandInput || bSubBandOutput ) {
        wavIO_close(hWavIO_real);
        wavIO_close(hWavIO_imag);
    }
    wavIO_close(hWavIO);
    
    if (nInChannels)
    {
        for (i=0; i< nInChannels; i++) {
            
            free(inBufferReal[i]);
            free(inBufferImag[i]);
            free(outBufferReal[i]);
            free(outBufferImag[i]);
            
            if (subBandDomainMode == 0) { /* QMF */
                
                QMFlib_CloseAnaFilterbank(pAnalysisQmf[i]);
                QMFlib_CloseSynFilterbank(pSynthesisQmf[i]);
                
            } else if (subBandDomainMode == 1) { /* STFT */
                
                free(stftWindowedSignalInputTmp[i]);
                free(stftWindowedSignalOutputTmp[i]);
                
            }
            
        }
        
        free(inBufferReal); inBufferReal = NULL;
        free(inBufferImag); inBufferImag = NULL;
        free(outBufferReal); outBufferReal = NULL;
        free(outBufferImag); outBufferImag = NULL;
        
        if (subBandDomainMode == 0) { /* QMF */
            
            free(pAnalysisQmf); pAnalysisQmf = NULL;
            free(pSynthesisQmf); pSynthesisQmf = NULL;
            
        } else if (subBandDomainMode == 1) { /* STFT */
            
            free(fftBufRe);
            free(fftBufIm);
            
            free(stftWindowedSignalInputRe); stftWindowedSignalInputRe = NULL;
            free(stftWindowedSignalOutputRe); stftWindowedSignalOutputRe = NULL;
            free(stftWindowedSignalInputIm); stftWindowedSignalInputIm = NULL;
            free(stftWindowedSignalOutputIm); stftWindowedSignalOutputIm = NULL;
            
            free(stftWindowedSignalInputTmp); stftWindowedSignalInputTmp = NULL;
            free(stftWindowedSignalOutputTmp); stftWindowedSignalOutputTmp = NULL;
            
            FFTlib_Destroy(&hFFTlibF);
            FFTlib_Destroy(&hFFTlibB);
            
        }
    }
    
    return 0;
}
