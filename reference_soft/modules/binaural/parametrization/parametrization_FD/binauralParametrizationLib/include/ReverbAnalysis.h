/***********************************************************************************

This software module was originally developed by 

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute

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

Fraunhofer IIS, ETRI, Yonsei University, WILUS Institute retain full right to modify 
and use the code for its own purpose, assign or donate the code to a third party and
to inhibit third parties from using the code for products that do not conform to 
MPEG-related ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works. 

Copyright (c) ISO/IEC 2014.

***********************************************************************************/

#ifndef _REVERBANALYSIS_H

  #define _REVERBANALYSIS_H


  #ifndef NULL
  #define NULL 0
  #endif

  #ifndef min
  #define min(a, b) (((a) < (b)) ? (a) : (b))
  #endif
  #ifndef max
  #define max(a, b) (((a) > (b)) ? (a) : (b))
  #endif

  #ifndef PI
  #define PI 3.14159265358979323846f
  #endif

/*--------------------------------*/

#ifndef BrirAnalysisStruct
  typedef struct BrirAnalysisStruct
  {
    float **timedomain_left;
    float **timedomain_right;
    float ***QMFdomain_left_real;
    float ***QMFdomain_right_real;
    float ***QMFdomain_left_imag;
    float ***QMFdomain_right_imag;
    int channels;
    int timedomain_samples;
    int QMF_Timeslots;
    int QMFbands;
    int fs_brirs;
  } BRIRANALYSISSTRUCT, *H_BRIRANALYSISSTRUCT;
#endif


  /*--------------------------------*/
#ifndef _BRIRMETADATASTRUCT_
#define _BRIRMETADATASTRUCT_
  typedef struct BRIRmetadataStruct 
  {
    int LateReverb_AnalysisBands;
    int noBands_transition;
    float *LateReverb_fc; 
    float *LateReverb_RT60; 
    float *LateReverb_energy; 
    int initDelay;	
    int *transition;
    int channels;
    float *azimuth;
    float *elevation;
    int numLFE;
    int *LFEchannels;
    int *filterlength_fft; 
    int *N_FFT;
    int *N_BLK;
    int *N_FRM;
    float ****FFTdomain_left_real;
    float ****FFTdomain_left_imag;
    float ****FFTdomain_right_real;
    float ****FFTdomain_right_imag;
    int numband_conv;
    float **TDL_gain_left_real;
    float **TDL_gain_left_imag;
    float **TDL_gain_right_real;
    float **TDL_gain_right_imag;
    int **TDL_delay_left;
    int **TDL_delay_right;
    unsigned int fs_BRIRs;
} BRIRMETADATASTRUCT, *H_BRIRMETADATASTRUCT; 
#endif

#ifndef RiffWavHeader
  typedef struct RiffWavHeader 
  {
    /* total length of the .wav Header: 44 Byte*/
    /* riff-section*/
    char riff_name[4];        /* 4 Byte, header-signature, contains "RIFF"*/
    unsigned int riff_length; /* 4 Byte, length of the rest of the header (36 Byte)+ < data_length> in Byte*/
    char riff_typ[4];         /* 4 Byte, contains the audio type, here "WAVE"*/

    /* format(fmt)-section*/
    char fmt_name[4];         /* 4 Byte, header-signature, contains "fmt "*/
    unsigned int fmt_length;  /* 4 Byte, length of the rest of the fmt header in Byte*/
    short format_tag;         /* 2 Byte, data format of the audio samples*/
    short channels;           /* 2 Byte, number of used channels*/
    unsigned int fs;          /* 4 Byte, sample frequency in Herz*/
    unsigned int bytes_per_sec; /* 4 Byte, < fs> * < block_align>*/
    short block_align;        /* 2 Byte, < channels> * (< bits/sample> / 8)*/
    short bpsample;           /* 2 Byte, quantizer bit depth, 8, 16 or 24 Bit*/

    /* data-section*/
    char data_name[4];        /* 4 Byte, header-signature, contains "data"*/
    unsigned long int data_length; /* 4 Byte, length of the data chunk in Byte*/
  } RIFFWAVHEADER; 
#endif

#define BINAURALPARAM_RIFF_HEADER_SIZE 36
#define BINAURALPARAM_NUM_OUTCHANNELS 2
#ifndef QMFLIB_NUMBANDS
#define QMFLIB_NUMBANDS 64
#endif

  
  float binauralParam_mean(float *data, int start, int l);
  void binauralParam_findmax(float *data, int l, float *max, int *idx);

  void binauralParam_analyzeLR(H_BRIRANALYSISSTRUCT h_BRIRs, H_BRIRMETADATASTRUCT h_BRIRmetadata, int end_analysis);
  void binauralParam_applyfilter(double *B, double *A, float *signal, int filterlength, int signallength, float ****signal_bandwise, int position, int channel, int numband);
  void binauralParam_estimate_noisefloor(float ****BRIRs_bandwise, int *startband, int *endband, int pos, int numband, int channel, float *noiselevel, int *endcalc, int minv, int fs);
  
  int binauralParam_find_last(float *data, float test, int modus, int start, int l_data);
  int binauralParam_find_first(float *data, float test, int modus, int start, int l_data);
  
  void binauralParam_getRT60(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *RT60);
  void binauralParam_getNRG(float ****BRIRs_bandwise, int *startband, int endcalc, int pos, int numband, int channel, int fs, float *NRG);
  void binauralParam_compensatedelay(float ****BRIRs_bandwise, int length, int numpos, int numch, int numbands);
  
  int binauralParam_readBRIRs_multipleWAV_writeGeoInfo(FILE *fbrirs_filelist, H_BRIRANALYSISSTRUCT h_BRIRs, char *brir_base_inpath, H_BRIRMETADATASTRUCT h_Metadata, char *cicp_file_path);
  void binauralParam_writeMetadata(H_BRIRMETADATASTRUCT h_BRIRmetadata, H_BRIRANALYSISSTRUCT h_BRIRs, FILE *fBRIRmetadata);
  int binauralParam_BRIRtoQMF(H_BRIRANALYSISSTRUCT h_BRIRs, int maxband, int min_length);

  void binauralParam_linearfit(H_BRIRMETADATASTRUCT h_BRIRmetadata, int startBand, int endBand, float* MixingTime);

#endif
